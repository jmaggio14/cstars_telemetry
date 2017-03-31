/*
 * parameters.h
 *
 *  Created on: Oct 24, 2016
 *      Author: pgi8114
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

//values that are predefined
#define DIMENSION 1024 		//Size of image
#define THRESHOLD 1197		//1100?//Threshold based on noise and star brightness

#define INNER 400 			//Number of pixels from border to look for stars
#define OUTER 200 			//Number of pixels away from center that stars can drift

#define SIZE 10				//Size of a postage-stamp
#define BACK 4				//Size of background border around postage-stamp

#define HWIDTH 0.8493218 	//((fwhm)/(2*sqrt(2*log(2)))), where fwhm = 2
#define WEIGHT 1 			//Test if this has effect. may need to change to array in sharp

#define SHAPE_UPP_BND 1     //Bounds for star checking parameters
#define SHAPE_LOW_BND 0
#define SHARP_UPP_BND 1.5
#define SHARP_LOW_BND 0

#define NUM_STAR_TRACK 3	//Number of groups of 3 stars to track.

#define POS_FN_MAIN "star_pos_001.txt"	//File where previous star positions are stored
#define POS_FN_SEC "star_pos_002.txt"	//File where previous star positions are stored
#define FRAME_OFFSET 3		// Number of previous frames to store when no match found
#define LPIX 1				//Deviation allowed on distance matching
#define EXPT_DRT 7 		//plus/min 17 pix
#define EXPT_ROT 0.5  	//plus/min pi/6

#define MAS_PER_PIX 21093.75 	//pixel conversion to milli-arc-second.
#define MAS_PER_RAD 206264806.247 	//radian conversion to milli-arc-second.

#endif /* PARAMETERS_H_ */
