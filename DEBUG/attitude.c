/*
 * attitude.c
 *
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
 * Takes the position info for the previous image and this image and computes
 * the change in position. Will identify the x_delta, y_delta, and r_delta
 * (rotation change). Will out put this information as a binary string.
 *
 * Input:
 * 			prev_stars: A struct Stars object that contains two list of the x and y
 * 				coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 				See util.h for struct description.
 * 			new_stars: A struct Stars object that contains two list of the x and y
 * 				coordinates of the NUM_STARS_TRACK stars, grouped in sets of three.
 * 				See util.h for struct description.
 *
 * Output:
 * 			none
 *
 * Note: may want to add a check to see if it worked
 */
int attitude_correction( struct Stars* prev_stars,
		struct Stars* curr_stars, FILE * log ){
	if (prev_stars == NULL || curr_stars == NULL) {
		return (1); //ERROR
	}
	int i, index, tracked_r = 0, tracked_x = 0, tracked_y = 0;
	float delta_z = 0, delta_x = 0, delta_y = 0, delta_temp;
	double theta_1, theta_2, theta_temp;
	float tempx, tempy, newx, newy;
	FILE * out;


	for ( i = 0; i < NUM_STAR_TRACK; i++){
		index = i*3;
		if (curr_stars->xcen[index] == -1) continue;
		//position of first star of each group set as origin

		//align first star in the two images
		theta_1 = atan2(prev_stars->ycen[index] - prev_stars->ycen[index+1],
				prev_stars->xcen[index] - prev_stars->xcen[index+1]);

		theta_2 = atan2(curr_stars->ycen[index] - curr_stars->ycen[index+1],
				curr_stars->xcen[index] - curr_stars->xcen[index+1]);

		theta_temp = (theta_2 - theta_1); //+ theta_0 which is the x_0 line on the detector
		if (theta_temp > EXPT_ROT || theta_temp < (-1*EXPT_ROT)) continue;
		delta_z += (float)theta_temp; //+ theta_0 which is the x_0 line on the detector
		tracked_r += 1;

		//align first star in the two images, first make center of image the origin
		tempx = curr_stars->xcen[index] - DIMENSION/2 + 1/2;
		tempy = curr_stars->ycen[index] - DIMENSION/2 + 1/2;
		//rotation
		newx = (float)(tempx*cos(theta_temp) +
				tempy*sin(theta_temp) + DIMENSION/2 - 1/2);
		newy = (float)(-tempx*sin(theta_temp) +
				tempy*cos(theta_temp) + DIMENSION/2 - 1/2);

		delta_temp = newy - prev_stars->ycen[index];
		if (delta_temp > EXPT_DRT || delta_temp < (-1*EXPT_DRT)) continue;
		delta_y += delta_temp;
		tracked_y += 1;

		delta_temp = newx - prev_stars->xcen[index];
		if (delta_temp > EXPT_DRT || delta_temp < (-1*EXPT_DRT)) continue;
		delta_x += delta_temp;
		tracked_x += 1;


	}

	delta_z = delta_z/(float)tracked_r;
	delta_x = delta_x/(float)tracked_x;
	delta_y = delta_y/(float)tracked_y;
	errorout[4] = (int)delta_x;
	errorout[5] = (int)delta_y;
	errorout[6] = (int)delta_z;

	if (delta_x != delta_x || delta_y != delta_y || delta_z != delta_z) {
		delta_x = 0;
		delta_y = 0;
		delta_z = 0;

		int positions[3] = {(int)delta_x, (int)delta_y, (int)delta_z};

		//UNCOMMENT BELOW FOR DEBUB
		fprintf(log, "pixels mas\n%f %d\n%f %d\n%f %d\n",
				delta_x, positions[0], delta_y, positions[1], delta_z, positions[2]);
		//UNCOMMENT ABOVE FOR DEBUG
		return(1);
	}

	int positions[3] = {(int)(delta_x * MAS_PER_PIX),
			(int)(delta_y * MAS_PER_PIX),
			(int)(delta_z * MAS_PER_RAD)};

	//UNCOMMENT BELOW FOR DEBUB
	fprintf(log, "pixels   mas\n%f %d\n%f %d\n%f %d\n",
			delta_x, positions[0], delta_y, positions[1], delta_z, positions[2]);
	//UNCOMMENT ABOVE FOR DEBUG

	return (0);
}

