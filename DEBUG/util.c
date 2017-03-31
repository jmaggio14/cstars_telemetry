/*
 * util.c
 *
 *  Created on: Jul 20, 2016
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
 * Prints the max value of each star found.
 *
 * Input:
 * 			root: a pointer to the root of a linked list. This list contains
 * 				the object parameters, sorted by brightness. (brightest first)
 *
 * Output:
 * 			none
 */
void print_list( struct Object_Node* root, FILE * log ){
	fprintf(log, "maxval xpos ypos\n");
	struct Object_Node *current;
	current = root;
	while(current!= NULL){
		fprintf(log, "%03.4f ",current->obj->maxval);
		fprintf(log, "%03.4f ",current->obj->center[0]);
		fprintf(log, "%03.4f\n",current->obj->center[1]);
		current = current->next;
	}
}

/*
 * Prints the max value of each star found.
 *
 * Input:
 * 			root: a pointer to the root of a linked list. This list contains
 * 				the object parameters, sorted by brightness. (brightest first)
 *
 * Output:
 * 			A null pointer
 */
void clean_up( struct Object_Node* root ){
	struct Object_Node *temp;
	while( root != NULL ){
		temp = root;
		root = root->next;
		temp->next = NULL;
		free(temp->obj);
		temp->obj = NULL;
		free(temp);
		temp = NULL;
	}
	return;
}

/*
 * Calculates distance between two object. Will need to be revised to account
 * for distortion
 *
 * Input:
 * 			x1: the x position of the first object
 * 			x2: the x position of the second object
 * 			y1: the y position of the first object
 * 			y2: the y position of the second object
 *
 * Output:
 * 			A float value that is that distance between the object.
 */
float distance( float x1, float x2, float y1, float y2 ){
	return ((float)sqrt( pow( x2 - x1, 2 ) + pow( y2 - y1, 2 )));
}

/*
 * A utility function that gives the array position in the 1-d array
 * representation of a 2-d array.
 *
 * Input:
 * 			len: the number of pixels in each row
 * 			x: the desired x position
 * 			y: the desired y position
 *
 * Output:
 * 			An integer value that is the index of the value in a
 * 			1-dimensional array.
 */
int offset( int len, int x, int y){
	return ( ( x * len ) + y );
}

/*
 * Will add a coordinate position to the end of a queue.
 *
 * Input:
 * 			Q: a pointer the the queue
 * 			Coordinate: a tuple holding the x, y values to be added
 *
 * Output:
 * 			none, added something for error checking purposes
 *
 */
void enqueue(struct Queue *Q,  struct tuple coordinate ){
	struct Q_Node* temp = (struct Q_Node*)malloc(1*sizeof(struct Q_Node));
	if (temp == NULL) return; //ERROR

	temp->data = coordinate;
	temp->next = NULL;
	if(Q->front == NULL && Q->rear == NULL){
		Q->front = Q->rear = temp;
	}
	else {
		Q->rear->next = temp;
		Q->rear = temp;
	}
	return;
}

/*
 * Will remove the first object in the queue
 *
 * Input:
 * 			Q: a pointer the the queue
 *
 * Output:
 * 			none, added something for error checking purposes
 *
 */
void dequeue(struct Queue *Q) {
	if (Q == NULL) return; //ERROR
	struct Q_Node* temp = Q->front;
	if(Q->front == NULL){
		fprintf(stderr, "QUEUE is empty.");
		return;
	}
	if(Q->front == Q->rear){
		Q->front = Q->rear = NULL;
	}
	else{
		Q->front = Q->front->next;
	}
	free(temp);
	return;
}

/*
 * Will return value at front of queue
 *
 * Input:
 * 			Q: a pointer the the queue
 *
 * Output:
 * 			A tuple containing an x and y coordinate
 *
 */
struct tuple getFront(struct Queue *Q) {
	struct tuple temp;
	if(Q->front == NULL) {
		temp.x = -1; temp.y = -1; //ERROR
	}
	else{
		temp = Q->front->data;
	}
	return (temp);
}

/*
 * Checks to see if queue is empty
 *
 * Input:
 * 			Q: a pointer the the queue
 *
 * Output:
 * 			An integer value, 1 is queue is empty, otherwise 0.
 *
 */
int isEmpty(struct Queue *Q) {
	if(Q->front == NULL){
		return (1);
	}
	return (0);
}
