/*
 * detect.c
 *
 * Detects all stars in an image.
 *
 *  Created on: Jun 10, 2016
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
 * Detects all stars in image. Will look through pixels in thresholded image
 * if there is an object present, detect entire object and find brightest
 * pixels. Create a sub-image centered at this pixel. Centroid it and determine
 * whether the object is a star is not. If it is a star save it to a linked
 * list, sorted by brightness
 *
 * Input:
 * 			img: a 1-dimensional array containing the image pixel values,
 * 				the x_ith, y_ith pixel can be accessed using
 * 					offset(DIMENSION, x_i, y_i)
 * 			th_img: a 1-dimensional array containing the thresholded values,
 * 				i.e. a 1 where the corresponding pixel value is above the
 * 				the threshold and 0 otherwise.
 *
 * Output:
 * 			A pointer to the root of a linked list. This list contains
 * 			the object parameters, sorted by brightness. (brightest first)
 */
struct Object_Node* detect_stars(int *img, int *th_img, int *th_pos, FILE * log ){
	int k;
	unsigned short numstars = 0;
	struct tuple position;
	struct Object *curr_star = NULL;
	struct Object_Node *root = NULL;
	struct Object_Node *temp;
	struct Object_Node *current;

	//UNCOMMENT BELOW FOR DEBUB
	/*
	fprintf(log, "%4s %4s %8s %8s  %8s  %8s   %8s    %8s\n",
			"xpos", "ypos", "max val", "xscent", "yscent",
			"sharp", "xicent", "yicent");
	*/
	//UNCOMMENT ABOVE FOR DEBUG

	for (k = 0; k < DIMENSION*DIMENSION; k++){
		if (th_pos[k] == 0) break;

		position.x = (int)floor(th_pos[k]/DIMENSION);
		if (position.x < (SIZE + BACK)||
				position.x > DIMENSION - (SIZE + BACK)) continue;
		position.y = th_pos[k] % DIMENSION;
		if (position.y < (SIZE + BACK)||
				position.y > DIMENSION - (SIZE + BACK)) continue;

		if (th_img[offset(DIMENSION, position.x, position.y)] != 1) continue;


		position = flood_fill( img, th_img, position );
		if (position.x == -1 || position.y == -1) continue;


		curr_star = extract_star( img, position, log );

		if (curr_star == NULL){
			continue;
		}


		//create a node to hold object and added to list based on brightness
		temp = (struct Object_Node*)malloc(1*sizeof(struct Object_Node));
		if (temp == NULL){
			continue; //ERROR
		}
		temp->obj = curr_star;
		numstars++;
		if (root == NULL || curr_star->maxval > root->obj->maxval ){
			temp->next = root;
			root = temp;
		}
		else {
			current = root;
			while(current->next != NULL
					&& curr_star->maxval < current->next->obj->maxval){
				current = current->next;
			}
			temp->next = current->next;
			current->next = temp;
		}
	}
	errorout[1] = numstars;
	return(root);
}

/*
 * Will take a position and the image and return a star if, there is a star at
 * this position. Will mark pixels checked, so that they will not be considered
 * agian.
 *
 * Input:
 * 			img: a 1-dimensional array containing the image pixel values,
 * 				the x_ith, y_ith pixel can be accessed using
 * 					offset(DIMENSION, x_i, y_i)
 * 			pos: a tuple giving the position to search around.
 *
 * Output:
 * 			A struct Object, containing the information about the star. If there
 * 			is no star at this position, return null.
 */
struct Object* extract_star( int *img, struct tuple pos, FILE * log ){
	struct Object *star;

	int xind, yind;
	float backarea, backval; //values for determining local background noise //HERE

	star = (struct Object*)malloc(1*sizeof(struct Object));
	if (star == NULL) return(star); //ERROR

	star->position[0] = pos.x;
	star->position[1] = pos.y;

	//find the average background
	backarea = 0; backval = 0; //HERE
	float image[SIZE*SIZE] = {0};
	for( xind = pos.x - (SIZE/2)-BACK; xind < pos.x + (SIZE/2)+BACK; xind++){
		for ( yind = pos.y - (SIZE/2)-BACK; yind < pos.y + (SIZE/2)+BACK; yind++){
			if ( (xind< pos.x - (SIZE/2) || xind > pos.x + (SIZE/2)-1)
					|| (yind < pos.y - (SIZE/2) || yind > pos.y + (SIZE/2)-1)){
				backval += (float)img[offset(DIMENSION, xind, yind)];
				backarea += 1;
			}
		}
	}
	//set max value for object
	star->maxval = (float)img[offset(DIMENSION, pos.x, pos.y)]
							  - (backval/backarea);

	//set postage stamp with background subtracted off
	for( xind = pos.x - (SIZE/2); xind < pos.x + (SIZE/2); xind++){
		for ( yind = pos.y - (SIZE/2); yind < pos.y + (SIZE/2); yind++){
			image[offset(SIZE, xind-(pos.x-(SIZE/2)), yind-(pos.y-(SIZE/2)))]
				  = (float)img[offset(DIMENSION, xind, yind)] - (backval/backarea);
		}
	}
	//UNCOMMENT BELOW FOR DEBUB
	/*
	fprintf(log, "%04d %04d %f ",
			star->position[0],
			star->position[1],
			star->maxval);
	*/
	//UNCOMMENT ABOVE FOR DEBUG

	star->center[0] = com( image, 0 );
	star->center[1] = com( image, 1 );

	//UNCOMMENT BELOW FOR DEBUB
	/*
	fprintf(log, "%f %f ", star->center[0], star->center[1]);
	*/
	//UNCOMMENT ABOVE FOR DEBUG

	//check if object is a star
	if (is_star( image, star, log ) == 0 || star->center[0] == -1 || star->center[1] == -1){
		//UNCOMMENT BELOW FOR DEBUB
		/*
		fprintf(log, "0 0\n");
		 */
		//UNCOMMENT ABOVE FOR DEBUG
		free(star);
		star = NULL;
		return (star);
	}

	//change center to correspond to full image
	star->center[0] = (float)star->position[0] +
			star->center[0] - SIZE/2;
	star->center[1] = (float)star->position[1] +
			star->center[1] - SIZE/2;

	//UNCOMMENT BELOW FOR DEBUB
	/*
	fprintf(log, "%f %f\n", star->center[0], star->center[1]);
	*/
	//UNCOMMENT ABOVE FOR DEBUG


	return(star);
}

/*
 * Will perform a 4- way flood fill algorithm starting at the given position.
 * In the process will find the brightest pixel. This will be the center of the
 * postage stamp of the object.
 *
 * Input:
 * 			img: a 1-dimensional array containing the image pixel values,
 * 				the x_ith, y_ith pixel can be accessed using
 * 					offset(DIMENSION, x_i, y_i)
 * 			th_img: a 1-dimensional array containing the thresholded values,
 * 				i.e. a 1 where the corresponding pixel value is above the
 * 				the threshold and 0 otherwise.
 * 			pos: a tuple giving the position to search around.
 *
 * Output:
 * 			A tuple giving the position of the objects's brightest pixel.
 */
struct tuple flood_fill( int *img, int *th_img, struct tuple start ){
	struct tuple max_pos, curr_pos, next_pos;
	int max_val, c;
	struct Queue *Q = NULL;

	//initialize queue.
	Q = (struct Queue*)malloc(sizeof(struct Queue));
	if (Q == NULL) return(start); //ERROR

	Q->front = Q->rear = NULL;

	max_pos.x = start.x;
	max_pos.y = start.y;
	max_val = img[offset(DIMENSION, max_pos.x, max_pos.y)];
	c = 0;
	enqueue( Q, max_pos );
	while (isEmpty(Q) == 0){
		if (c == DIMENSION*DIMENSION) {\
			while (isEmpty(Q) == 0) {
				dequeue(Q);
			}
			break;
		}
		curr_pos = getFront(Q);
		dequeue(Q);
		if (curr_pos.x == -1 && curr_pos.y == -1) continue;
		if (th_img[offset(DIMENSION, curr_pos.x, curr_pos.y)] != 1)
			continue;
		th_img[offset(DIMENSION, curr_pos.x, curr_pos.y)] = 2;

		//check if this value is greater than max_val
		if (img[offset(DIMENSION, curr_pos.x, curr_pos.y)] > max_val){
			max_pos.x = curr_pos.x;
			max_pos.y = curr_pos.y;
			max_val = img[offset(DIMENSION, curr_pos.x, curr_pos.y)];
			//printf("new: x pos: %d, y pos: %d\n", max_pos.x, max_pos.y);
		}

		if (curr_pos.x < (DIMENSION-(SIZE + BACK) - 1)){
			if (th_img[offset(DIMENSION, curr_pos.x+1, curr_pos.y)] == 1){
				next_pos.x = curr_pos.x+1;
				next_pos.y = curr_pos.y;
				enqueue(Q, next_pos);
			}
		}
		if (curr_pos.y > (SIZE + BACK)){
			if (th_img[offset(DIMENSION, curr_pos.x, curr_pos.y-1)] == 1){
				next_pos.x = curr_pos.x;
				next_pos.y = curr_pos.y-1;
				enqueue(Q, next_pos);
			}
		}
		if (curr_pos.y < (DIMENSION-(SIZE + BACK) - 1)){
			if (th_img[offset(DIMENSION, curr_pos.x, curr_pos.y+1)] == 1){
				next_pos.x = curr_pos.x;
				next_pos.y = curr_pos.y+1;
				enqueue(Q, next_pos);
			}
		}
		c += 1;
	}
	//clean up
	free(Q);
	Q = NULL;

	return (max_pos);
}

/*
 * Centroids a sub-image that is SIZE x SIZE pixel image using a center of
 * mass (COM) calculation.
 *
 * Input:
 * 		image: a pointer to a 1-d array representing a SIZE x SIZE pixel image.
 * 			The x_ith, y_ith pixel can be accessed using
 * 					offset(SIZE, x_i, y_i)
 * 		dim: 0 or 1 corresponding to the the x and y dim respectively.
 *
 * Output:
 * 		An float value giving the centroid in the appropriate dim.
 */
float com( float *image, int dim ){
	if (image == NULL ) return(-1); //ERROR
	float hist[SIZE] = {0};
	float num = 0, den = 0;
	int i,j;

	//create histogram
	for (i = 0; i < SIZE; i++){
		for (j = 0; j < SIZE; j++){
			if (dim == 0){
				hist[i] += image[offset(SIZE, i, j)];
			} else {
				hist[j] += image[offset(SIZE, i, j)];
			}
		}
	}

	//compute center of mass
	for (i = 0; i < SIZE; i++){
		num += hist[i]*(float)i;
		den += hist[i];
	}

	return((float)num/(float)(den));
}

/*
 * Determine if an object is star-like rather than a hot pixel, bad column,
 * or a cosmic. Based on cds.cern.ch/record/428221/files/0002435.pdf. Calls
 * sharp.m and shape.m to get shape and sharp values. If 0 < sharp <
 * 1, and 0 < shape < 1, then the object is a star.
 *
 * Input:
 *		image: a pointer to a 1-d array representing a SIZE x SIZE pixel image.
 * 			The x_ith, y_ith pixel can be accessed using
 * 					offset(SIZE, x_i, y_i)
 * 		obj: a struct (OBJECT) containing the information about the object.
 * 			(max val, position, and center)
 *
 * Output:
 *      An integer value representing if the postage stamp contains a star, 1
 *      if not.
 *
 */
int is_star( float *image, struct Object *obj, FILE * log ){
	if (image == NULL || obj == NULL) return (1); //ERROR
	float sharpval;

	int is_star = 1;

	//float shapeval = shape( image, obj );
	sharpval = sharp( image, obj );

	//UNCOMMENT BELOW FOR DEBUB
	/*
	//fprintf(log, "%f %f ", sharpval, shapeval);
	fprintf(log, "%f ", sharpval);
	*/
	//UNCOMMENT ABOVE FOR DEBUG

	if (sharpval <= SHARP_LOW_BND ||
			sharpval >= SHARP_UPP_BND ||
			sharpval != sharpval) is_star = 0;
	if (obj->center[0] < 0 ||
			obj->center[0] > SIZE ||
			obj->center[1] < 0 ||
			obj->center[1] > SIZE) is_star = 0;
	return (is_star);
}

/*
 * CURRENTLY NOT USED, RESULTS HAVE HIGHT ACCURACY WITHOUT.
 *
 * Calculate shape parameter based using method from
 * cds.cern.ch/record/428221/files/0002435.pdf. Determine the second
 * moments of the object using equation (6) and (7), then calculates the
 * shape based on equation (8).
 *
 * (6) hx^2 = sum(Iij(i - xc)^2)/sum(Iij), hy^2 = sum(Iij(j - yc)^2/sum(Iij)
 * (7) hxy = sum(Iij(i - xc)(j - yc))/sum(Iij)
 * (8) shape = 2*sqrt((hx-hy)^2 + 4hxy^2)/(hx+hy)
 *
 * Input:
 *		image: a pointer to a 1-d array representing a SIZE x SIZE pixel image.
 * 			The x_ith, y_ith pixel can be accessed using
 * 					offset(SIZE, x_i, y_i)
 * 		obj: a struct (OBJECT) containing the information about the object.
 * 			(max val, position, and center)
 *
 * Output:
 *       A value for the shape parameter.
 */
float shape( float *image, struct Object *obj ){
	if (image == NULL || obj == NULL) return (-1); //ERROR
	float hx2num = 0, hy2num = 0, hxynum = 0;
	float ints, den = 0;
	int i,j;
	float x, y;
	float hx, hy, hxy;

	//math stuff. See comment for equation.
	for (i = 0; i < SIZE; i++){
		for(j = 0; j < SIZE; j++){
			ints =  10 + image[offset(SIZE, i, j)];
			x = (float)i - obj->center[0];
			y = (float)j - obj->center[1];
			hx2num += ints*x*x;
			hy2num += ints*y*y;
			hxynum += ints*x*y;
			den += ints;
		}
	}

	hx = sqrt(hx2num/den);
	hy = sqrt(hy2num/den);
	hxy = hxynum/den;

	return (2*sqrt(pow((hx - hy),2) + 4*pow(hxy,2))/(hx + hy));
}

/*
 * Calculate sharp value based using method from
 * cds.cern.ch/record/428221/files/0002435.pdf. Determine G_0, the amplitude
 * of the fitted Gaussian, and I_0, the central intensity, using equation
 * (2), (3), (4). Then use equation (5) to determine the sharp parameter.
 *
 * (2) Gij = G_0*exp(-((i-xc)^2 + (j - yc)^2)/2*h^2) + b
 * (3) S = sum(Gij - Iij)^2 -> min
 * (4) I_0 = maxints - B, B = sum(wijIij)/sum=i0j0(wij), ij~=i0j0.
 * (5) sharp = I_0/G_0.
 *
 * The first part of the function uses a least squares approximation for Ax = b
 * where A is a 2 x 2 matrix containing exp(-((i-xc)^2 + (j - yc)^2) in the
 * first column and 1's in the second column, and b is the brightnesses at each
 * pixel. Equation solved using x = (transpose(A)A)^-1*transpose(A)b. Method
 * then derived to solve this using the idea that the matrices are small.
 *
 * Input:
 *		image: a pointer to a 1-d array representing a SIZE x SIZE pixel image.
 * 			The x_ith, y_ith pixel can be accessed using
 * 					offset(SIZE, x_i, y_i)
 * 		obj: a struct (OBJECT) containing the information about the object.
 * 			(max val, position, and center)
 *
 * Output:
 *       A value for the sharp parameter.
 */
float sharp( float *image, struct Object *obj ){
	if (image == NULL || obj == NULL) return (-1); //ERROR
	float y;
	int i,j;
	float a = 0, bc = 0, d = (float)(SIZE*SIZE);
	float XTIM[2] = {0};
	float G_0, I_0;
	float Bnum = 0, Bden = 0;

	//Calculate G_0 using a least squares fitting. Rather than reshape the
	//arrays, loops over all elements using two nested loops.
	for (i = 0; i < SIZE; i++){
		for (j = 0; j < SIZE; j++){
			y = (float)(exp(-(pow((float)i - obj->center[0],2) +
					pow((float)j - obj->center[1],2))/
					(2*HWIDTH*HWIDTH)));
			a += y*y;
			bc += y;
			XTIM[0] += y*image[offset(SIZE, i, j)];
			XTIM[1] += image[offset(SIZE, i, j)];
		}
	}

	G_0 = (d/(a*d - bc*bc))*XTIM[0]+(-bc/(a*d - bc*bc))*XTIM[1];

	//Calculate I_0
	for (i = 0; i < SIZE; i++){
		if (i == SIZE/2) { continue; }
		for (j = 0; j < SIZE; j++){
			if (j == SIZE/2) { continue; }
			Bnum += WEIGHT*image[offset(SIZE, i, j)];
			Bden += WEIGHT;
		}
	}

	I_0 = obj->maxval - Bnum/Bden;

	//return ratio
	return (I_0/G_0);
}
