/*
 * track.c
 *
 *  Created on: Oct 24, 2016
 *      Author: pgi8114
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>

#include "parameters.h"
#include "util.h"

/*
 * Using list of objects, choose the brightest sets of three stars that are
 * within an inner bound. Save their center and brightness to a list.
 *
 * Input:
 * 			root: a pointer to the root of a linked list. This list contains
 * 				the object parameters, sorted by brightness. (brightest first)
 * 			counter: current image number
 *
 * Output:
 * 			A struct Stars object that contains two list of the x and y
 * 			coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 			See util.h for struct description.
 */
struct Stars* choose_stars( struct Object_Node* root, int counter ){
	int i, num_set = 0;
	struct Stars *stars;
	stars = (struct Stars*)malloc(1*sizeof(struct Stars));
	if (stars == NULL) return NULL;  //ERROR

	stars->frame = counter;

	struct Object_Node *current;

	current = root;
	while(current!= NULL && num_set < NUM_STAR_TRACK*3 ){
		if ( current->obj->position[0] > INNER &&
				current->obj->position[0] < DIMENSION - INNER &&
				current->obj->position[1] > INNER &&
				current->obj->position[1] < DIMENSION - INNER &&
				current->obj->maxval < 4000) {
			stars->xcen[num_set] = current->obj->center[0];
			stars->ycen[num_set] = current->obj->center[1];
			stars->xpos[num_set] = current->obj->position[0];
			stars->ypos[num_set] = current->obj->position[1];
			num_set++;
		}
		current = current->next;
	}

	if (num_set < NUM_STAR_TRACK*3) {
		current = root;  //ERROR?
		while(current!= NULL && num_set < NUM_STAR_TRACK*3 ){
			if ((current->obj->position[0] <= INNER ||
					current->obj->position[0] >= DIMENSION - INNER ||
					current->obj->position[1] <= INNER ||
					current->obj->position[1] >= DIMENSION - INNER) &&
					( current->obj->position[0] > OUTER &&
							current->obj->position[0] < DIMENSION - OUTER &&
							current->obj->position[1] > OUTER &&
							current->obj->position[1] < DIMENSION - OUTER) ){
				if (current->obj->maxval < 4000){
					stars->xcen[num_set] = current->obj->center[0];
					stars->ycen[num_set] = current->obj->center[1];
					stars->xpos[num_set] = current->obj->position[0];
					stars->ypos[num_set] = current->obj->position[1];
				num_set++;
			}}
			current = current->next;
		}
	}

	if (num_set < NUM_STAR_TRACK*3) {
		current = root; //ERROR?
		while(current!= NULL && num_set < NUM_STAR_TRACK*3 ){
			if ( current->obj->position[0] <= OUTER ||
					current->obj->position[0] >= DIMENSION - OUTER ||
					current->obj->position[1] <= OUTER ||
					current->obj->position[1] >= DIMENSION - OUTER) {
				if (current->obj->maxval < 4000){
					stars->xcen[num_set] = current->obj->center[0];
					stars->ycen[num_set] = current->obj->center[1];
					stars->xpos[num_set] = current->obj->position[0];
					stars->ypos[num_set] = current->obj->position[1];
				num_set++;
				}
			}
			current = current->next;
		}
	}
	if (num_set < NUM_STAR_TRACK*3) {  //ERROR
		for ( i = num_set; i < NUM_STAR_TRACK*3; i++){
			stars->xcen[num_set] = -1;
			stars->ycen[num_set] = -1;
			stars->xpos[num_set] = -1;
			stars->ypos[num_set] = -1;
		}
	}
	return (stars);
}

/*
 * Previous stars saved as list in text file. x pos, y pos, brightness. Scans
 * file and saves position in 1D array. each value is accessible using offset
 * function.
 *
 * Input:
 * 			none
 *
 * Output:
 * 			A struct Stars object that contains two list of the x and y
 * 			coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 			See util.h for struct description.
 */
struct Stars* get_previous_stars( int counter, int flag ){
	int k;
	FILE *file;
	int check;
	struct Stars *stars;

	if (flag == 1){ //get main file
		file = fopen(POS_FN_SEC, "r");
		if (file == NULL) return NULL;  //ERROR
		if (!fscanf(file, "%d", &check)){
			return NULL;  //ERROR
			fclose(file);
		}
		if (check < counter - FRAME_OFFSET ){
			fclose(file);
			return NULL;
		}
	}else { //get secondary file
		file = fopen(POS_FN_MAIN, "r");
		if (file == NULL) return NULL;  //ERROR
		if (!fscanf(file, "%d", &check)){
			fclose(file);
			return NULL;  //ERROR
		}
	}

	stars = (struct Stars*)malloc(1*sizeof(struct Stars));
	if (stars == NULL) return NULL;  //ERROR

	stars->frame = check;

	for (k = 0; k < NUM_STAR_TRACK*3; k++){
		//stars[k] = 0;
		if (!fscanf(file, "%f", &stars->xcen[k])){
			free(stars);
			stars = NULL;
			return (stars); //ERROR
		}
		stars->xpos[k] = 0;
		if (!fscanf(file, "%f", &stars->ycen[k])){
			free(stars);
			stars = NULL;
			return (stars); //ERROR
		}
		stars->ypos[k] = 0;
	}
	fclose(file);
	return (stars);
}

/*
 * Takes in a list of star positions, calculates distances between them and
 * searches for matching stars in current image.
 *
 * Input:
 * 			prev_stars: a pointer to a 1 dimensional array where values can be
 * 				accessed using
 * 					offset(NUM_STAR_TRACK, i, j)
 * 				where i is the ith star is the list and j is the value desired.
 * 				The value j = 0 will give x center, j = 1 will give y center,
 * 				and j = 2 will give the brightness of the star.
 * 			root: a pointer to the root of a linked list. This list contains
 * 				the object parameters, sorted by brightness. (brightest first)
 *
 * Output:
 * 			A struct Stars object that contains two list of the x and y
 * 			coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 			See util.h for struct description.
 */
struct Stars* track_stars( struct Stars* prev_stars,
		struct Object_Node* root, FILE * log ){

    int i,j,index;

    //will store previous stars in groups of three.
    float prev_dist[NUM_STAR_TRACK*3] = {0};

    struct Object_Node *first_star;
    struct Object_Node *second_star;

    float temp_d1;

    int starflag;
    int num_matched = 0;

	struct Stars *stars;

	struct Stars *new_stars;

    //make sure enough stars are detected
    if (root == NULL || root->next == NULL) return NULL;  //ERROR
    if (root->next->next == NULL) return NULL;  //ERROR

    if (prev_stars == NULL) return NULL;  //ERROR

	new_stars = (struct Stars*)malloc(sizeof(struct Stars));
	if( new_stars == NULL ) return NULL;

	new_stars->frame = 0;

	//UNCOMMENT BELOW FOR DEBUB
	/*
    fprintf(log, "Calculated triangular distances:\n");
	*/
	//UNCOMMENT ABOVE FOR DEBUG
    for (i = 0; i < NUM_STAR_TRACK; i++){
    	for (j = 0; j < 3; j++){
    		index = offset(3, i, j);
    		if (j == 2){
    			prev_dist[index] = distance( prev_stars->xcen[index],
    					prev_stars->xcen[index-2],
						prev_stars->ycen[index],
						prev_stars->ycen[index-2]);
    			//UNCOMMENT BELOW FOR DEBUB
    			/*
    			fprintf(log, "%f \n", prev_dist[index]);
    			*/
    			//UNCOMMENT ABOVE FOR DEBUG
    		}
    		else {
    			prev_dist[index] = distance( prev_stars->xcen[index],
    					prev_stars->xcen[index+1],
						prev_stars->ycen[index],
						prev_stars->ycen[index+1]);
    			//UNCOMMENT BELOW FOR DEBUB
    			/*
    			fprintf(log, "%f ", prev_dist[index]);
    			*/
    			//UNCOMMENT ABOVE FOR DEBUG
    		}
    	}
    }

    for (i = 0; i < NUM_STAR_TRACK; i++){
    	index = i*3;
    	first_star = root;
    	second_star = NULL;
    	stars = NULL;
    	starflag = 1;
    	while (first_star->next->next != NULL && starflag == 1){
    		second_star = first_star->next;
    		while (second_star->next != NULL && starflag == 1){
    			temp_d1 = distance( first_star->obj->center[0],
    					second_star->obj->center[0],
						first_star->obj->center[1],
						second_star->obj->center[1]);
    			if (temp_d1 < prev_dist[index] + LPIX &&
    					temp_d1 > prev_dist[index] - LPIX ){
    				stars = search_third_star( first_star, second_star, 0,
    						prev_dist[index+1], prev_dist[index+2]);
    				if (stars != NULL) starflag = 0;
    			}
    			else if (temp_d1 < prev_dist[index+1] + LPIX &&
    					temp_d1 > prev_dist[index+1] - LPIX){
    				stars = search_third_star( first_star, second_star, 1,
    						prev_dist[index], prev_dist[index+2]);
    				if (stars != NULL) starflag = 0;
    			}
    			else if (temp_d1 < prev_dist[index+2] + LPIX &&
    					temp_d1 > prev_dist[index+2] - LPIX){
    				stars = search_third_star( first_star, second_star, 2,
    						prev_dist[index], prev_dist[index+1]);
    				if (stars != NULL) starflag = 0;
    			}
    			second_star = second_star->next;
    		}
    		first_star = first_star->next;
    	}
    	if (stars != NULL){
    		for (j = 0; j < 3; j++){
    			new_stars->xcen[index + j] = stars->xcen[j];
    			new_stars->ycen[index + j] = stars->ycen[j];
    		}
    		num_matched++;
    		free(stars);
    	}
    	else {
    		//UNCOMMENT BELOW FOR DEBUB
    		/*
    		fprintf(log, "NO MATCH");
    		 */
    		//UNCOMMENT ABOVE FOR DEBUG
    		for (j = 0; j < 3; j++){
    			new_stars->xcen[index + j] = -1;
    			new_stars->ycen[index + j] = -1;
    		}
    	}
    }
    if (num_matched == 0){
    	free( new_stars );
    	new_stars = NULL;
    }
	//UNCOMMENT BELOW FOR DEBUB
	/*
    fprintf(log, "\n");
     */
    //UNCOMMENT ABOVE FOR DEBUG
	return (new_stars);

}

/*
 * A utility function used by track_stars to find the third star after two have
 * been identified. Will call match_stars to get output.
 *
 * Input:
 * 			first_star: a pointer to the node object containing the first star
 * 			second_star: a pointer to the node object containing the first star
 * 			size: a value to identify where side the first two stars correspond
 * 				to.
 * 			prev_dist_a: the distance value for the second side.
 * 			prev_dist_b: the distance value for the third side.
 *
 * Output:
 * 			A struct Stars object that contains two list of the x and y
 * 			coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 			See util.h for struct description.
 */
struct Stars* search_third_star( struct Object_Node *first_star,
		struct Object_Node *second_star, int side, float prev_dist_a,
		float prev_dist_b ){

	if (second_star == NULL) return NULL;

	struct Object_Node *third_star;
    third_star = second_star->next;
    float temp_d2, temp_d3;
    int starflag = 1;
    struct Stars *stars = NULL;

    while (third_star != NULL && starflag == 1){
        temp_d3 =  distance( first_star->obj->center[0],
        		third_star->obj->center[0],
        		first_star->obj->center[1],
				third_star->obj->center[1]);
        temp_d2 =  distance( third_star->obj->center[0],
        		second_star->obj->center[0],
        		third_star->obj->center[1],
				second_star->obj->center[1]);
        if (temp_d3 < prev_dist_a + LPIX && temp_d3 > prev_dist_a - LPIX &&
        		temp_d2 < prev_dist_b + LPIX && temp_d2 > prev_dist_b - LPIX ){
        	starflag = 0;
        	if ( side == 0 ){
        		stars = match_stars( second_star, first_star, third_star );
        	}
        	else if ( side == 1 ){
        		stars = match_stars( third_star, first_star, second_star );
        	}
        	else {
        		stars = match_stars( first_star, third_star, second_star );
        	}
        }
        else if (temp_d2 < prev_dist_a + LPIX && temp_d2 > prev_dist_a - LPIX &&
        		temp_d3 < prev_dist_b + LPIX && temp_d3 > prev_dist_b - LPIX ){
            starflag = 0;
        	if ( side == 0 ){
        		stars = match_stars( first_star, second_star, third_star );
        	}
        	else if ( side == 1 ){
        		stars = match_stars( third_star, second_star, first_star );
        	}
        	else {
        		stars = match_stars( second_star, third_star, first_star );
        	}
        }
        third_star = third_star->next;
    }

    return (stars);
}

/*
 * A utility function used by track_stars and earch_third_star to order the
 * stars according to the previous star positions.
 *
 * Input:
 * 			first_star: a pointer to the node object containing the first star
 * 			second_star: a pointer to the node object containing the first star
 * 			third_star: a pointer to the node object containing the first star
 * 				(all ordered to match order of previous stars.)
 *
 * Output:
 * 			A struct Stars object that contains two list of the x and y
 * 			coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 			See util.h for struct description.
 */
struct Stars* match_stars( struct Object_Node *first_star,
		struct Object_Node *second_star, struct Object_Node *third_star ){

	struct Stars *stars;

	if(first_star == NULL ||
			second_star == NULL ||
			third_star == NULL) return NULL;//ERROR

	stars = (struct Stars*)malloc(1*sizeof(struct Stars));
	if (stars == NULL) return NULL; //ERROR

	stars->xcen[0] = first_star->obj->center[0];
	stars->ycen[0] = first_star->obj->center[1];
	stars->xcen[1] = second_star->obj->center[0];
	stars->ycen[1] = second_star->obj->center[1];
	stars->xcen[2] = third_star->obj->center[0];
	stars->ycen[2] = third_star->obj->center[1];
	stars->xpos[0] = first_star->obj->position[0];
	stars->ypos[0] = first_star->obj->position[1];
	stars->xpos[1] = second_star->obj->position[0];
	stars->ypos[1] = second_star->obj->position[1];
	stars->xpos[2] = third_star->obj->position[0];
	stars->ypos[2] = third_star->obj->position[1];

	return (stars);
}

/*
 * Saves a list of object parameters to a file
 *
 * Input:
 * 			stars: A struct Stars object that contains two list of the x and y
 * 				coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 				See util.h for struct description.
 * 			flag: 0 or 1 corresponding to what filename to save stars in.
 *
 * Output:
 * 			none
 *
 * Note: may want to add a check to see if it worked
 */
void set_stars( struct Stars* stars, int flag ){
	int i;
	FILE * file;

	if (stars == NULL) return;  //ERROR
	if (flag == 1){
		file = fopen(POS_FN_SEC, "w+");
	} else {
		file = fopen(POS_FN_MAIN, "w+");
	}

	if (file == NULL) return; //ERROR

	fprintf(file, "%d\n", stars->frame);
	for (i = 0; i < NUM_STAR_TRACK*3; i++){
		fprintf(file, "%f %f\n", stars->xcen[i], stars->ycen[i]);
	}
	fclose(file);

	return;
}

void save_pststamps( struct Stars* stars, int *img ){
	int i,j,k;
	for (i = 0; i < NUM_STAR_TRACK*3; i++){
		errorout[8+i] = stars->xpos[i];
		errorout[17+i] = stars->ypos[i];
		for (j = 0; j < SIZE; j ++){
			for (k = 0; k < SIZE; k ++){
				errorout[25+i*100+j*10+k] = img[offset(DIMENSION,stars->xpos[i]-SIZE+j, stars->ypos[i]-SIZE+k)];
			}
		}
	}
}
