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
 * File:		geodesic.c
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
  A function to compute geodesics from distance functions.

  REFERENCES:
  J A Sethian, "Level Set Methods and Fast Marching Methods", Cambridge University Press,
  1996, 1999 (2nd ed.) - Chapter 8 "Efficient Schemes: Fast Marching Methods"

  HISTORY:
  Created by Ben Appleton (2/12/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "pde_toolbox.h"
#include "lfmm.h"

/********************************* DEFINES ***************************************/
/* Define the backtracking step size, precision, and maximum number of iterations */
static const float STEPSIZE = 0.25;
static const float PRECISION = 0.01;
static const int MAXITER = 10;

/*************************** FUNCTION PROTOTYPES *********************************/
static int interpolate_grad(
	BIMAGE * distance,		/* The discrete distance function */
	float * point,			/* The continuous point coordinates */
	float * grad			/* Output gradient vector */
);

static int interpolate_value(
	BIMAGE * distance,		/* The discrete distance function */
	float * point,			/* The continuous point coordinates */
	float * value
);

static int neighbour_cube(
	BIMAGE * distance,		/* The discrete distance function */
	float * point,			/* The continuous point coordinates */
	float * neighbour_val	/* The binary hypercube of neighbouring values */
);

static float contract2(
	float * tensor,			/* The 2^N contravariant tensor to contract */
	float * operator,		/* The array of N 2-covector operators */
	int N					/* The number of dimensions */
);

static int normalise_vector(
	float * vector,		/* The vector to normalise */
	int N				/* The length of the vector */
);


/******************************** FUNCTIONS **************************************/
/* geodesic:
*/
int geodesic(
	BVECT * start,				/* The start of the shortest path */
	BIMAGE * distance,			/* The distance function */
	BIMAGE * contour			/* The output contour */
)
{
	int i;
	BVECT * coord, * temp_coord;
	float * tracking_point, * new_point;
	float * new_grad, * old_grad, * av_grad, *temp_grad;

	/* Check that we've been given a valid starting point */
	for (i = 0; i < distance->dim->length; i++) {
		if (start->buf[i] < 0 || start->buf[i] > distance->dim->buf[i] - 1) {
			LSTB_debug("Starting point outside of image!\n");
			break;
		}
	}

	/* Initialise the coordinate structures */
	coord = BVECT_constructor(distance->dim->length);
	temp_coord = BVECT_constructor(distance->dim->length);
	tracking_point = (float *)malloc(distance->dim->length*sizeof(float));
	new_point = (float *)malloc(distance->dim->length*sizeof(float));
	new_grad = (float *)malloc(distance->dim->length*sizeof(float));
	old_grad = (float *)malloc(distance->dim->length*sizeof(float));
	av_grad = (float *)malloc(distance->dim->length*sizeof(float));
	temp_grad = (float *)malloc(distance->dim->length*sizeof(float));

	/* Initialise the (back)tracking point */
	for (i = 0; i < distance->dim->length; i++)
		tracking_point[i] = (float)start->buf[i];

	/* Label the starting point as part of the contour */
	contour->buf[bvectToInt(start, distance->dim)] = 1.0;

	/* Backtrack until a halting condition is reached */
	while(LSTB_TRUE) {
		int iteration;
		int return_val;
		char failure;
		float old_dist = LSTB_BIGNUM, new_dist;

		/** Back-estimate the gradient for stability **/
		/* Compute the gradient at the current point */
		interpolate_grad(distance, tracking_point, old_grad);
		memcpy(av_grad, old_grad, distance->dim->length * sizeof(float));
		normalise_vector(av_grad, distance->dim->length);

		/* Iteratively estimate average gradient */
		failure = LSTB_FALSE;
		for (iteration = 0; iteration < MAXITER; iteration++) {
			float temp;

			/* Compute the estimated new point */
			for (i = 0; i < distance->dim->length; i++)
				new_point[i] = tracking_point[i] - STEPSIZE * av_grad[i];

			/* Compute the gradient at this point */
			return_val = interpolate_grad(distance, new_point, new_grad);
			if (return_val != 0) {
				failure = LSTB_TRUE;
				break;
			}

			/* Compute the average of the old and new gradients */
			memcpy(temp_grad, av_grad, distance->dim->length * sizeof(float));
			for (i = 0; i < distance->dim->length; i++)
				av_grad[i] = (old_grad[i] + new_grad[i])/2.0;
			normalise_vector(av_grad, distance->dim->length);

			/* Check for convergence */
			temp = 0;
			for (i = 0; i < distance->dim->length; i++)
				temp += LSTB_SQR(av_grad[i] - temp_grad[i]);
			if (temp < LSTB_SQR(PRECISION)) break;
		}
		if (failure || iteration == MAXITER) {
			int min_index;

			/* Restart from nearest integer coordinate */
			for (i = 0; i < distance->dim->length; i++)
				tracking_point[i] = (int)(tracking_point[i] + 0.5);
			for (i = 0; i < coord->length; i++)
				coord->buf[i] = (int)tracking_point[i];
			old_dist = distance->buf[bvectToInt(coord, distance->dim)];

			/* Face-connected descent to neighbouring integer point */
			new_dist = LSTB_BIGNUM;
			min_index = 0;
			for (i = 0; i < distance->dim->length; i++) {
				int d;
				for (d = -1; d <= 1; d+=2) {
					int index;

					BVECT_copy(temp_coord, coord);
					temp_coord->buf[i] += d;

					/* Check bounds */
					if (temp_coord->buf[i] < 0 || temp_coord->buf[i] > distance->dim->buf[i] - 1)
						continue;

					/* Check value */
					index = bvectToInt(temp_coord, distance->dim);
					if (distance->buf[index] < new_dist) {
						new_dist = distance->buf[index];
						min_index = index;
					}
				}
			}

			/* Save the minimum index as the new point */
			intToBvect(min_index, coord, distance->dim);
			for (i = 0; i < distance->dim->length; i++)
				new_point[i] = (float)coord->buf[i];
		} else {
			/* Test to see if we went up the distance surface (halt) */
			interpolate_value(distance, new_point, &new_dist);
		}

		/* Check if we've reached a minima */
		if (new_dist >= old_dist) break;

		/* Draw a point on the image here */
		for (i = 0; i < distance->dim->length; i++)
			coord->buf[i] = (int)(new_point[i]+0.5);
		i = bvectToInt(coord, distance->dim);
		contour->buf[i] = 1.0;

		/* Copy state for next iteration */
		old_dist = new_dist;
		memcpy(tracking_point, new_point, distance->dim->length*sizeof(float));
	}

	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);
	free((void *)tracking_point);
	free((void *)new_point);
	free((void *)new_grad);
	free((void *)old_grad);
	free((void *)av_grad);
	free((void *)temp_grad);

	return 0;
}

/* interpolate_grad:
	Compute the gradient of the multi-linearly interpolated distance function at a continuous
	point.
*/
static int interpolate_grad(
	BIMAGE * distance,		/* The discrete distance function */
	float * point,			/* The continuous point coordinates */
	float * grad			/* Output gradient vector */
)
{
	int i;
	float * tensor, * tensor_copy;		/* The 2^N component local hypercube */
	float * operator;					/* The 2-covectors to apply to the tensor along each dimension */
	float * temp_2vect;					/* A temporary 2-vector */

	tensor = (float *)malloc((1<<distance->dim->length)*sizeof(float));
	tensor_copy = (float *)malloc((1<<distance->dim->length)*sizeof(float));
	temp_2vect = (float *)malloc(2 * sizeof(float));
	operator = (float *)malloc(distance->dim->length * 2 * sizeof(float));

	/* Extract the hypercube of neighbouring values */
	i = neighbour_cube(distance, point, tensor);
	/* If not all neighbouring values were available, drop out */
	if (i != 0) return 1;

	/* Initialise the array of operators */
	for (i = 0; i < distance->dim->length; i++) {
		operator[0 + 2*i] = 1 - (point[i] - (int)point[i]);
		operator[1 + 2*i] = (point[i] - (int)point[i]);
	}

	/* Compute each component of the gradient by setting up interpolatory operators
	   in each direction except for the derivative operator */
	for (i = 0; i < distance->dim->length; i++) {
		memcpy(tensor_copy, tensor, (1<<distance->dim->length)*sizeof(float));

		/* Set up the operator array */
		memcpy(temp_2vect, operator + 2*i, 2*sizeof(float));
		operator[0 + 2*i] = -1;
		operator[1 + 2*i] = 1;

		/* Compute this component of the gradient */
		grad[i] = contract2(tensor_copy, operator, distance->dim->length);

		/* Correct the temporary alteration to the operator array */
		memcpy(operator + 2*i, temp_2vect, 2*sizeof(float));
	}

	free((void *)tensor);
	free((void *)tensor_copy);
	free((void *)temp_2vect);
	free((void *)operator);

	return 0;
}

/* interpolate_value:
	Compute the value of the multi-linearly interpolated distance function at a continuous
	point.
*/
static int interpolate_value(
	BIMAGE * distance,		/* The discrete distance function */
	float * point,			/* The continuous point coordinates */
	float * value
)
{
	int i;
	float * tensor;						/* The 2^N component local hypercube */
	float * operator;					/* The 2-covectors to apply to the tensor along each dimension */

	tensor = (float *)malloc((1<<distance->dim->length)*sizeof(float));
	operator = (float *)malloc(distance->dim->length * 2 * sizeof(float));

	/* Extract the hypercube of neighbouring values */
	i = neighbour_cube(distance, point, tensor);
	/* If not all neighbouring values were available, drop out */
	if (i != 0) return 1;

	/* Initialise the array of operators */
	for (i = 0; i < distance->dim->length; i++) {
		operator[0 + 2*i] = 1 - (point[i] - (int)point[i]);
		operator[1 + 2*i] = (point[i] - (int)point[i]);
	}

	/* Apply the array of operators */
	*value = contract2(tensor, operator, distance->dim->length);

	free((void *)tensor);
	free((void *)operator);

	return 0;
}

/* neighbour_cube:
	Extract the hypercube of neighbouring values or flag as unavailable
*/
static int neighbour_cube(
	BIMAGE * distance,		/* The discrete distance function */
	float * point,			/* The continuous point coordinates */
	float * neighbour_val	/* The binary hypercube of neighbouring values */
)
{
	int i;
	BVECT * floor_coord, * temp_coord, * bin_coord, * bin_dim;

	/* Set up the coordinate structures */
	floor_coord = BVECT_constructor(distance->dim->length);
	bin_coord = BVECT_constructor(distance->dim->length);
	temp_coord = BVECT_constructor(distance->dim->length);
	bin_dim = BVECT_constructor(distance->dim->length);

	/* Set up the dimensions of the binary hypercube */
	for (i = 0; i < distance->dim->length; i++) bin_dim->buf[i] = 2;

	/* Establish the floored coordinates of the given continuous point */
	for (i = 0; i < distance->dim->length; i++) floor_coord->buf[i] = (int)point[i];

	/* Check that the local neighbourhood lies in the image range */
	for (i = 0; i < distance->dim->length; i++)
		if (point[i] < 0 || point[i] > distance->dim->buf[i] - 2)
			return 1;	/* Value not computable */

	/* Form the hypercube (sidelength 2) of local neighbours */
	for (i = 0, BVECT_zero(bin_coord); i < (1<<distance->dim->length); i++, BVECT_inc(bin_coord, bin_dim)) {
		int index;

		/* Compute the coordinate of the value to extract in temp_coord */
		BVECT_copy(temp_coord, floor_coord);
		BVECT_add(temp_coord, bin_coord);

		/* Extract the value and store it in the tensor */
		index = bvectToInt(temp_coord, distance->dim);
		neighbour_val[i] = distance->buf[index];
	}

	/* Free all allocated objects */
	BVECT_destructor(floor_coord);
	BVECT_destructor(temp_coord);
	BVECT_destructor(bin_coord);
	BVECT_destructor(bin_dim);

	return 0;
}

/* contract2:
	A specialised tensor contraction function, for N 2-vectors and a (2^N)-tensor.
	Overwrites the variable tensor!
*/
static float contract2(
	float * tensor,			/* The 2^N contravariant tensor to contract */
	float * operator,		/* The array of N 2-covector operators */
	int N					/* The number of dimensions */
)
{
	int n, i;

	/* Contract tensor in place along each dimension from largest stride to smallest */
	for (n = N-1; n >= 0; n--) {
		/* Contract in-place along this dimension */
		for (i = 0; i < (1<<n); i++) {
			tensor[i] = tensor[i]*operator[0 + 2*n] + tensor[i+(1<<n)]*operator[1 + 2*n];
		}
	}

	return tensor[0];
}

/* normalise_vector:
	Normalise a vector
*/
static int normalise_vector(
	float * vector,		/* The vector to normalise */
	int N				/* The length of the vector */
)
{
	int i;
	float magnitude;

	/* Compute the magnitude of the vector */
	magnitude = 0;
	for (i = 0; i < N; i++)
		magnitude += LSTB_SQR(vector[i]);
	magnitude = sqrt(magnitude);

	if (magnitude == 0.0) {
		return 1;
	}

	/* Now apply magnitude to normalise */
	for (i = 0; i < N; i++)
		vector[i] /= magnitude;

	return 0;
}
