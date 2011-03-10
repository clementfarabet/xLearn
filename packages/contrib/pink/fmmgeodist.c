/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/
/*
 * File:		geodist.c
 *
 * Written by:	Ben Appleton
 *				ITEE, The University of Queensland
 *
 * Date:		November 2002
 *
 * Copyright:	Intended for open-source distribution
 *
*/

/*********************************************************************************************
 geodist.c
 ------

  DESCRIPTION:
  A function to compute geodesic distances.  Performs competitive FMM from an arbitrary number
  of seeds.

  REFERENCES:
  J A Sethian, "Level Set Methods and Fast Marching Methods", Cambridge University Press,
  1996, 1999 (2nd ed.) - Chapter 8 "Efficient Schemes: Fast Marching Methods"

  HISTORY:
  Created by Ben Appleton (28/11/02)
  Contact: appleton@itee.uq.edu.au / h.talbot@esiee.fr
**********************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "pde_toolbox.h"
#include "lfmm.h"

/*************************** FUNCTION PROTOTYPES *********************************/
static char updateDistance(
	BIMAGE * distance,			/* The distance image */
	BIMAGE * g,					/* The metric */
	FMMSTATESTRUCT * stateStruct,	/* The state of the algorithm */
	BVECT * coord				/* The point to update */
);

static int initContour(
	BIMAGE * distance,				/* The distance function to initialise */
	BIMAGE * g,						/* The metric */
	HEAPSTRUCT *heapStruct,			/* The heap objects */
	FMMSTATESTRUCT *stateStruct 	/* The state objects */
);

/* geodist:
*/
int geodist(
	BIMAGE * seeds,				/* Non-zero values form seeds (overwritten by Voronoi tessellation) */
	BIMAGE * g,					/* The isotropic but non-homogeneous metric */
	const char stopping,		/* The stopping criteria type */
	const float threshold,		/* The stopping threshold */
	BIMAGE * distance			/* The geodesic distance function (output) */
)
{
	int i;
	HEAPSTRUCT * heapStruct;
	FMMSTATESTRUCT * stateStruct;
	BVECT * coord, * tempCoord;

	/* Allocate memory for coordinates */
	coord = BVECT_constructor(g->dim->length);
	tempCoord = BVECT_constructor(g->dim->length);

	/* Initialise the heap structure */
	heapStruct = HEAPSTRUCT_constructor(g->dim);

	/* Initialise the state structure */
	stateStruct = (FMMSTATESTRUCT *)malloc(sizeof(FMMSTATESTRUCT));
	stateStruct->nodeState = (char *)calloc(BVECT_prod(g->dim), sizeof(char));	/* Calloc == default to FARNODE */
	stateStruct->label = seeds;		/* Use the seeds image for label propagation */

	/* Initialise the contour */
	initContour(distance, g, heapStruct, stateStruct);

	/* Now begin fast marching outwards */
	while(LSTB_TRUE) {
		int tempHeapIndex, imageIndex, tempImageIndex;
		char returnVal;
		int d;

		/* Check if the heap is empty */
		if (heapStruct->heapEnd < 0) break;

		/* Grab the image index of the heap's root */
		imageIndex = heapStruct->heapToImage[0];

		/* Early stopping criteria */
		if (stopping == STOPONMETRIC) {
			if (g->buf[imageIndex] > threshold) break;
		} else if (stopping == STOPONDISTANCE) {
			if (distance->buf[imageIndex] > threshold) break;
		} /* else no stopping */

		/* Decompose the root's image index into pixel co-ordinates */
		intToBvect(imageIndex, coord, distance->dim);

		/* Remove the root (making it KNOWNNODE) and promote a new one */
		stateStruct->nodeState[imageIndex] = KNOWNNODE;
		heapPull(heapStruct, 0, distance);

		/* Update each face-connected neighbour's distance, state and the heap correspondingly */
		for (i = 0; i < distance->dim->length; i++) {		/* For each dimension */
			for (d = -1; d <= 1; d+=2) {			/* Plus or minus one in this direction */
				/* Compute the neighbour's coordinate and image index */
				BVECT_copy(tempCoord, coord);
				tempCoord->buf[i] += d;
				if (tempCoord->buf[i] < 0 || tempCoord->buf[i] > distance->dim->buf[i] - 1) continue;
				tempImageIndex = bvectToInt(tempCoord, g->dim);

				returnVal = updateDistance(distance, g, stateStruct, tempCoord);

				if (returnVal == ADD) {
					heapAdd(heapStruct, tempImageIndex, distance);
				} else if (returnVal == DECREASED) {
					tempHeapIndex = heapStruct->imageToHeap[tempImageIndex];
					heapShuffleUp(heapStruct, tempHeapIndex, distance);
				} else if (returnVal == INCREASED) {
					tempHeapIndex = heapStruct->imageToHeap[tempImageIndex];
					heapShuffleDown(heapStruct, tempHeapIndex, distance);
				}
			}
		}
	}

	/* Free all allocated memory */
	HEAPSTRUCT_destructor(heapStruct);
	BVECT_destructor(coord);
	BVECT_destructor(tempCoord);
	free((void *)stateStruct->nodeState);
	free((void *)stateStruct);

	return 0;
}


/* - updateDistance:
	Updates the current point based on the neighbouring known nodes.
	Returns a flag indicating the necessary modification to the heap.
*/
static char updateDistance(
	BIMAGE * distance,				/* The distance image */
	BIMAGE * g,						/* The metric */
	FMMSTATESTRUCT * stateStruct,	/* The state of the algorithm */
	BVECT * coord					/* The point to update */
)
{
	int i, d;
	float a, c;						/* Quadratic coefficients to compute distance */
	float meanDist;
	float oldDist, newDist, discriminant;
	float minNeighbourDist;			/* Label inherited from lowest distance neighbour */
	int returnVal = NOCHANGE;		/* Default is that no change occurred */
	BVECT tempCoord, *ptempCoord;
	float neighbourDist[MAXDIMS];	/* Temporary buffers, to save the pain of freeing at every return */
	int buffer[MAXDIMS];
	int index, neighbourDistLength;

	/* Create the coordinate structure - from static memory rather than dynamic (faster?) */
	ptempCoord = &tempCoord;
	ptempCoord->length = distance->dim->length;
	ptempCoord->buf = (int *)buffer;

	/* WARNING: No longer checking bounds, unnecessary if called correctly */

	index = bvectToInt(coord, distance->dim);

	/* If already KNOWNNODE, leave alone */
	if (stateStruct->nodeState[index] == KNOWNNODE) return NOCHANGE;

	/* Store the old distance for detecting changes */
	if (stateStruct->nodeState[index] == TRIALNODE) {
		oldDist = distance->buf[index];
	} else { /* FARNODE */
		oldDist = (float)LSTB_BIGNUM;
	}

	/* If FARNODE, set to TRIALNODE and make a note to add it to the heap later */
	if (stateStruct->nodeState[index] == FARNODE) returnVal = ADD;
	stateStruct->nodeState[index] = TRIALNODE;

	/* Initialise the distance list */
	neighbourDistLength = 0;
	minNeighbourDist = LSTB_BIGNUM;
	for (i = 0; i < distance->dim->length; i++) neighbourDist[i] = LSTB_BIGNUM;

	/* Find the least-distance neighbour along each axis, where they exist */
	for (i = 0; i < distance->dim->length; i++) {
		for (d = -1; d <= 1; d += 2) {
			int tempIndex;

			BVECT_copy(ptempCoord, coord);
			ptempCoord->buf[i] += d;			/* Step in the given direction */

			/* Skip this point if it lies outside the image domain */
			if (ptempCoord->buf[i] < 0 || ptempCoord->buf[i] > distance->dim->buf[i] - 1) continue;

			tempIndex = bvectToInt(ptempCoord, distance->dim);

			/* Now update the distance */
			if (stateStruct->nodeState[tempIndex] == KNOWNNODE)
				if (distance->buf[tempIndex] < neighbourDist[neighbourDistLength]) {
					neighbourDist[neighbourDistLength] = distance->buf[tempIndex];

					/* Propagate labels */
					if (neighbourDist[neighbourDistLength] < minNeighbourDist) {
						minNeighbourDist = neighbourDist[neighbourDistLength];
						stateStruct->label->buf[index] = stateStruct->label->buf[tempIndex];
					}
				}
		}

		/* Move along to the next element in the distance array if we found one here */
		if (neighbourDist[neighbourDistLength] != LSTB_BIGNUM) neighbourDistLength++;
	}

	/* Now solve the quadratic in neighbourDistLength dimensions */
	c = 0;
	/* Work with mean-corrected distances to reduce rounding errors! */
	meanDist = 0.0;
	for (i = 0; i < neighbourDistLength; i++) {
		meanDist += neighbourDist[i] / (float)neighbourDistLength;
	}
	for (i = 0; i < neighbourDistLength; i++) {
		float t = neighbourDist[i] - meanDist;
		c += LSTB_SQR(t);
	}
	a = neighbourDistLength;
	c -= g->buf[index];

	discriminant = - 4.0 * a * c;
	if (discriminant >= 0) {
		newDist = sqrt(discriminant) / (2.0 * a) + meanDist;
	} else {
		LSTB_debug("Discriminant = %f\n", discriminant);
		newDist = meanDist;
	}
	
	/* Only apply changes if they reduce the distance */
	if (newDist < oldDist) {
		distance->buf[index] = newDist;
	}

	/* Check what to return */
	if (returnVal == ADD) return ADD;
	if (newDist < oldDist) return DECREASED;
	if (newDist > oldDist) return INCREASED;
	/* Default */
	return NOCHANGE;
}


/* - initContour:
	Initialises the seeds as TRIALs.
*/
static int initContour(
	BIMAGE * distance,				/* The distance function to initialise */
	BIMAGE * g,						/* The metric */
	HEAPSTRUCT *heapStruct,			/* The heap objects */
	FMMSTATESTRUCT *stateStruct 	/* The state objects */
)
{
	int num_pixels;
	int index;

	heapStruct->heapEnd = -1;

	/* Full image sweep, setting labels to 0 dist and initialise as TRIALs */
	num_pixels = BVECT_prod(distance->dim);
	for (index = 0; index < num_pixels; index++) {
		/* If there's no label here, continue */
		if (stateStruct->label->buf[index] == 0) continue;

		/* Set distance to 0 */
		distance->buf[index] = 0.0;
		stateStruct->nodeState[index] = TRIALNODE;
		heapAdd(heapStruct, index, distance);
	}

	return 0;
}

