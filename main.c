/*
 * main.c
 *
 * Opens either a ascii or binary image and runs star detection, star tracking,
 * and attitude correction.
 *
 *  Created on: Oct 24, 2016
 *      Author: pgi8114
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "parameters.h"
#include "util.h"

#include <time.h>

#define INPUT_IMAGE_PIPE "/dev/xillybus_image"


unsigned short errorout[1024];

int main(int argc, char *argv[]){
	int f;
	int frame_number = 0;
	while(1){
		frame_number = frame_number + 1;
		for ( f = 0; f < 1024; f++){
				errorout[f] = 0;
			}
		read_binary( argc, argv, frame_number);
		send_uart();
		printf("errno: %d\n",errno);
	}
	return (0);
}

int read_binary(int argc, char *argv[], int frame_number){
	printf("%d\n",frame_number);
	int k, i;
	unsigned short bar[DIMENSION*DIMENSION];
	int *img;
	int *th_img;
	int *th_pos; //added

	//allocate memory for image and thresholdded image

	img = (int *)malloc(sizeof(int*)*DIMENSION*DIMENSION);
	th_img = (int *)malloc(sizeof(int*)*DIMENSION*DIMENSION);
	th_pos = (int *)malloc(sizeof(int*)*DIMENSION*DIMENSION);
	if (img == NULL || th_img == NULL || th_pos == NULL) {
		return (1);
	}

	//open the file
	// file = fopen(INPUT_IMAGE_PIPE, "rb");

	//IMPLEMENTING non-blocking fifo --> should exit if there isn't an issue
	// int image_fifo = open(INPUT_IMAGE_PIPE,O_RDONLY);
	// off_t eof_loc = lseek(image_fifo,0,SEEK_END);/
	// printf("EOF: %zd\n",eof_loc );
	// ssize_t x = read(image_fifo, &bar, eof_loc);
	// close(image_fifo);
	FILE* image_fifo = fopen(INPUT_IMAGE_PIPE,"rb");
	if(image_fifo == NULL) return 1;
	size_t x = fread(bar,2,DIMENSION*DIMENSION,image_fifo);
	fclose(image_fifo);

	// for(rad=0;rad<x; rad++) printf("value:%hu index: %d  frame: %d\n",bar[rad],rad,frame_number);
	if(x == -1 && errno == EAGAIN){
		printf("NON-BLOCK WORKS\n");
		return 1;
	}
	else if(x != DIMENSION*DIMENSION){
		printf("INCORRECT IMAGE SIZE -- discarding\n");
		printf("x:%zd correct size:%d\n",x,DIMENSION*DIMENSION );
		return 1;
	}
	else{
		printf("IMAGE READ SUCCESSFUL\n");
		//save image to 1D array that can be accessed using offset(size, x, y)
		i = 0;
	 	for (k = 0; k < DIMENSION*DIMENSION; k++){
			img[k] = (int)bar[k];
			if (img[k] >= THRESHOLD){
				th_img[k] = 1;
				th_pos[i] = k;
				i++;
			}
			else th_img[k] = 0;
		}

   	if (i < DIMENSION*DIMENSION-1) th_pos[i+1] = -1;


		process_im(img, th_img, th_pos, 1, frame_number);
		free(img);
		free(th_img);
		free(th_pos);
	}
	return(0);
}

int read_int(int alpha, int beta, char *fn[]){
	int k, i;
	int *img;
	int *th_img;
	int *th_pos;
	FILE * file;

	//allocate memory for image and thresholded image
	img = (int *)malloc(sizeof(int*)*DIMENSION*DIMENSION);
	th_img = (int *)malloc(sizeof(int*)*DIMENSION*DIMENSION);
	th_pos = (int *)malloc(sizeof(int*)*DIMENSION*DIMENSION);
	if (img == NULL || th_img == NULL || th_pos == NULL) {
		return (1);
	}

	//open the file
	file = fopen(fn[1], "r");
	if (file == NULL){
		return (1);
	}
	i = 0;
	//save image to 1D array that can be accessed using offset(size, x, y)
	for (k = 0; k < DIMENSION*DIMENSION; k++){
		if (!fscanf(file, "%d", &img[k])){
			return (1); //do something more here.
		}
		//at each pixel check is the value is greater than the threshold, mark if so
		if (img[k] >= THRESHOLD){
			th_img[k] = 1; //check this
			th_pos[i] = k;
			i++;
		}
		else th_img[k] = 0;
	}

   	if (i < DIMENSION*DIMENSION-1) th_pos[i+1] = -1;

	fclose(file);

	process_im(img, th_img, th_pos, alpha, beta);

	free(img);
	free(th_img);
	free(th_pos);
	return(0);
}


/*
 * Will take the image that is read in and process it.
 * 		1) Detect all stars in image - store as linked list. If this is the
 * 			first image in the series got to step 5
 * 		2) Read in positions form previous file.
 * 		3) Match stars using SSS congruence. If no stars are matched, go to
 * 			Step 2 looking at second position file.
 * 		4) Calculate attitude changes.
 * 		5) Determine what stars to save and save stars to position file.
 *
 * 	As a not f_o, f_s, f_p, and f_a log files will not be used in final version,
 * 	they are soley for testing puposes.
 */
int process_im(int *img, int *th_img, int *th_pos, int alpha, int beta){
	//handle output files. Set to null for release
	FILE* f_o = NULL;
	// FILE* f_s = NULL;
	FILE* f_p = NULL;
	FILE* f_a = NULL;

	//UNCOMMENT BELOW FOR DEBUB
	/*
	char f_o_name[80], f_s_name[80], f_p_name[80], f_a_name[80];
	//objects 	-> Parameters of objects found
	//detect	-> Sorted max vals of all stars:
	//position	-> Positions from file + new positions
	//attitude	-> Calculated telemetry
	FILE* f_o; FILE* f_s; FILE* f_p; FILE* f_a;
	sprintf(f_o_name, "data/object_%03d_%03d.txt", alpha, beta);
	f_o = fopen(f_o_name, "w");
	sprintf(f_s_name, "data/detect_%03d_%03d.txt", alpha, beta);
	f_s = fopen(f_s_name, "w");
	sprintf(f_p_name, "data/position_%03d_%03d.txt", alpha, beta);
	f_p = fopen(f_p_name, "w");
	sprintf(f_a_name, "data/attitude_%03d_%03d.txt", alpha, beta);
	f_a = fopen(f_a_name, "w");
	if (f_o == NULL || f_s == NULL || f_p == NULL ||f_a == NULL) return (1);
	*/
	//UNCOMMENT ABOVE FOR DEBUG

	int k;
	struct Object_Node* root;
	struct Stars* new_stars = NULL;
	struct Stars* prev_stars = NULL;
	int flag = 0;
	int newflag = 0;

   	errorout[0] = beta;

	//detect all stars in the image, store as linked list that is sorted by brightness
	root = detect_stars( img, th_img, th_pos, f_o );
	if (root == NULL){
		return (1);
	}
	//UNCOMMENT BELOW FOR DEBUB
	/*
	print_list( root, f_s );
	*/
	//UNCOMMENT ABOVE FOR DEBUG

	//determine if first file: if it is just set stars
	if ( beta == 1){
		new_stars = choose_stars( root, beta );
	} else {
		//otherwise, load previous positions
		prev_stars = get_previous_stars( beta, flag );
		if (prev_stars == NULL) errorout[2] = 1;
		//track stars and get new positions
		new_stars = track_stars( prev_stars, root, f_p );


		if (new_stars == NULL){ //try again
			flag = 1;
			free(prev_stars);
			prev_stars = get_previous_stars( beta, flag );
			//track stars and get new positions
			new_stars = track_stars( prev_stars, root, f_p );
			if (new_stars == NULL){
				errorout[3] = 1;
				flag = 0;
				newflag = 1;
			}
		}else{
		}

		//check if we need to choose new stars
		//determine and output attitude corrections
		if ( new_stars != NULL) {
			//UNCOMMENT BELOW FOR DEBUB
			/*
			for (k = 0; k < NUM_STAR_TRACK*3; k++){
				fprintf(f_p, "%f %f %f %f\n", prev_stars->xcen[k],
						prev_stars->ycen[k],
						new_stars->xcen[k],
						new_stars->ycen[k]);
			}
			*/
			//UNCOMMENT ABOVE FOR DEBUG
			if (attitude_correction( prev_stars, new_stars, f_a ) == 1){
				flag = 1;
			}
			for ( k = 0; k < NUM_STAR_TRACK*3; k++){
				if (new_stars->xcen[k] < OUTER ||
					new_stars->xcen[k] > DIMENSION - OUTER ||
					new_stars->ycen[k] < OUTER ||
					new_stars->ycen[k] > DIMENSION - OUTER){
					newflag = 0;
				}
			}
		} else newflag = 0;

		if (newflag == 0){
			free(new_stars);
			new_stars = choose_stars( root, beta );
			errorout[7] = 1;
		}

	}
	set_stars( new_stars, flag );
	save_pststamps(new_stars, img);
	//clean up memory
	free( prev_stars );
	prev_stars = NULL;
	free( new_stars );
	new_stars = NULL;
	clean_up( root );
	//UNCOMMENT BELOW FOR DEBUB
	/*
	fclose(f_o);
	fclose(f_s);
	fclose(f_p);
	fclose(f_a);
	*/
	//UNCOMMENT ABOVE FOR DEBUG
	return(0);
}
