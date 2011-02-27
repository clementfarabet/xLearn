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
 * File:		bimage.c
 *
 * Written by:		Ben Appleton
 * 			ITEE, The University of Queensland
 *
 * Date:		May 2002
 *
 * Copyright:		Intended for open-source distribution
 *
*/

/*********************************************************************************************
 bimage.c
 ------

  DESCRIPTION:
  Tools for the BIMAGE data structures.

  HISTORY:
  Created by Ben Appleton (5/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pde_toolbox.h"

#define BVECT_PRINT_TO_STDOUT

/***************************************** FUNCTIONS *****************************************/

/* Constructor and destructor for BVECT 'class' */
BVECT * BVECT_constructor(
	int length					/* The length of the BVECT */
) {
	BVECT * bvect;

	bvect = (BVECT *)malloc(sizeof(BVECT));
	bvect->length = length;
	bvect->buf = (int *)calloc(length, sizeof(int));

	return bvect;
}
void BVECT_destructor(BVECT * bvect) {
	free((void *)bvect->buf);
	free((void *)bvect);
}


/* BVECT_prod:
	Return the product of a vector
*/
int BVECT_prod(BVECT *in) {
	int i, returnValue;

	returnValue = 1;
	for (i = 0; i < in->length; i++) {
		returnValue *= in->buf[i];
	}

	return returnValue;
}

/* prod:
	Some programs still need this function - sigh!
*/
int bimage_prod(BVECT * in) {
	int i, returnValue;

	returnValue = 1;
	for (i = 0; i < in->length; i++) {
		returnValue *= in->buf[i];
	}

	return returnValue;
} /*bimage_prod*/

/* BVECT_sum:
	Return the sum of a vector
*/
int BVECT_sum(BVECT *in) {
	int i, returnValue;

	returnValue = 0;
	for (i = 0; i < in->length; i++) {
		returnValue += in->buf[i];
	}

	return returnValue;
}

/* BVECT_sum_sqr:
	Return the square of the 2-norm of a vector
*/
float BVECT_sum_sqr(BVECT * in) {
	int i;
	float return_val;

	return_val = 0;
	for (i = 0; i < in->length; i++)
		return_val += in->buf[i]*in->buf[i];

	return return_val;
}

/* BVECT_add:
	Add two BVECTs, overwriting first
*/
int BVECT_add(BVECT *dest, BVECT *source) {
	int i;

	if (dest->length != source->length) return 1;

	for (i = 0; i < dest->length; i++)
		dest->buf[i] += source->buf[i];

	return 0;
}

/* BVECT_sub:
	Subtract (BVECTs) source from destination, overwriting destination
*/
int BVECT_sub(BVECT *dest, BVECT *source) {
	int i;

	if (dest->length != source->length) return 1;

	for (i = 0; i < dest->length; i++)
		dest->buf[i] -= source->buf[i];

	return 0;
}

/* BVECT_inc:
	Alter the given coordinate to increment the corresponding image index
*/
int BVECT_inc(BVECT * in, BVECT * dim) {
	int i;

	for (i = 0; i < dim->length; i++) {
		in->buf[i]++;
		if (in->buf[i] == dim->buf[i]) {
			in->buf[i] = 0;
		} else break;
	}

	return 0;
}

/* BVECT_dec:
	Alter the given coordinate to decrement the corresponding image index
*/
int BVECT_dec(BVECT * in, BVECT * dim) {
	int i;

	for (i = 0; i < dim->length; i++) {
		in->buf[i]--;
		if (in->buf[i] == -1) {
			in->buf[i] = dim->buf[i] - 1;
		} else break;
	}

	return 0;
}

/* BVECT_zero:
	Set this BVECT to all 0
*/
int BVECT_zero(BVECT * in) {
	memset(in->buf, 0, in->length*sizeof(int));
	return 0;
}

/* BVECT_copy:
	Copy source to destination
*/
int BVECT_copy(BVECT * dest, BVECT * source) {			/* dest = source by value */
	if (dest->length != source->length) return 1;

	memcpy(dest->buf, source->buf, dest->length * sizeof(int));

	return 0;
}

/* BVECT_compare:
	Return a > b
*/
char BVECT_compare(BVECT * a, BVECT * b) {
	int i;

	if (a->length != b->length) return LSTB_FALSE;

	for (i = 0; i < a->length; i++) {
		if (a->buf[i] != b->buf[i]) return LSTB_FALSE;
	}

	return LSTB_TRUE;
}

/* BVECT_print:
	Print a BVECT
*/
int BVECT_print(BVECT * in) {
	int i;

	#ifdef BVECT_PRINT_TO_STDOUT
	printf("(");
	for (i = 0; i < in->length; i++) {
		printf("%i", in->buf[i]);
		if (i != in->length - 1) printf(", ");
	}
	printf(")");
	#else
	LSTB_debug("(");
	for (i = 0; i < in->length; i++) {
		LSTB_debug("%i", in->buf[i]);
		if (i != in->length - 1) LSTB_debug(", ");
	}
	LSTB_debug(")");
	#endif

	return 0;
}

/* BVECT_max:
	Return the maximum component of a BVECT
*/
int BVECT_max(
	BVECT * in
) {
	int return_val, i;

	return_val = in->buf[0];
	for (i = 1; i < in->length; i++) {
		if (return_val < in->buf[i]) return_val = in->buf[i];
	}

	return return_val;
}

/* BVECT_min:
	Return the minimum component of a BVECT
*/
int BVECT_min(
	BVECT * in
) {
	int return_val, i;

	return_val = in->buf[0];
	for (i = 1; i < in->length; i++) {
		if (return_val > in->buf[i]) return_val = in->buf[i];
	}

	return return_val;
}

/* bvectToInt:
	Convert a coordinate to an index (from arbitrary base to integer).
*/
int bvectToInt(BVECT *coord, BVECT *dim) {
	int index = 0, i;

	for (i = dim->length - 1; i >= 0; i--) {
		index *= dim->buf[i];
		index += coord->buf[i];
	}

	return index;
}

/* intToBvect:
	Convert an index to a coordinate.  Arbitrary base conversion.
*/
int intToBvect(
	int index,								/* The input index */
	BVECT *pcoord,								/* The output coordinate */
	BVECT *dim)								/* The dimensions (conversion parameters) */
	{
	int i;

	for (i = 0; i < pcoord->length; i++) {					/* For each dimension */
		pcoord->buf[i] = index % dim->buf[i];
		index /= dim->buf[i];
	}

	return 0;
}

/* Blank image constructor for BIMAGE 'class'  */
BIMAGE * BIMAGE_constructor(
	BVECT * dim 					/* The requested dimensions */
) {
	BIMAGE * bimage;
	int i;

	bimage = (BIMAGE *)malloc(sizeof(BIMAGE));
	bimage->buf = (float *)calloc(BVECT_prod(dim), sizeof(float));
	bimage->dim = (BVECT *)malloc(sizeof(BVECT));
	bimage->dim->length = dim->length;
	bimage->dim->buf = (int *)malloc(dim->length*sizeof(int));
	memcpy(bimage->dim->buf, dim->buf, dim->length*sizeof(int));

	/* Set up the step vector, default to unit steps */
	bimage->steps = (float *)malloc(dim->length*sizeof(float));
	for (i = 0; i < dim->length; i++) bimage->steps[i] = 1.0;

	return bimage;
}

/* Copy constructor for BIMAGE 'class'  */
BIMAGE * BIMAGE_constructor_BIMAGE(
	BIMAGE * in 					/* The requested dimensions */
)
{
	return BIMAGE_constructor_float(in->buf, in->dim);
}

/* Templates would be nice! */
BIMAGE * BIMAGE_constructor_double(
	double *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
) {
	BIMAGE * bimage;
	int i;
	int num_pixels;

	bimage = (BIMAGE *)malloc(sizeof(BIMAGE));
	bimage->buf = (float *)malloc(BVECT_prod(dim)*sizeof(float));
	num_pixels = BVECT_prod(dim);
	for (i = 0; i < num_pixels; i++) {
		bimage->buf[i] = (float)in[i];
	}
	bimage->dim = (BVECT *)malloc(sizeof(BVECT));
	bimage->dim->length = dim->length;
	bimage->dim->buf = (int *)malloc(dim->length*sizeof(int));
	memcpy(bimage->dim->buf, dim->buf, dim->length*sizeof(int));

	/* Set up the step vector, default to unit steps */
	bimage->steps = (float *)malloc(dim->length*sizeof(float));
	for (i = 0; i < dim->length; i++) bimage->steps[i] = 1.0;

	return bimage;
}

BIMAGE * BIMAGE_constructor_float(
	float *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
) {
	BIMAGE * bimage;
	int i;

	bimage = (BIMAGE *)malloc(sizeof(BIMAGE));
	bimage->buf = (float *)malloc(BVECT_prod(dim)*sizeof(float));
	memcpy(bimage->buf, in, BVECT_prod(dim)*sizeof(float));
	bimage->dim = (BVECT *)malloc(sizeof(BVECT));
	bimage->dim->length = dim->length;
	bimage->dim->buf = (int *)malloc(dim->length*sizeof(int));
	memcpy(bimage->dim->buf, dim->buf, dim->length*sizeof(int));

	/* Set up the step vector, default to unit steps */
	bimage->steps = (float *)malloc(dim->length*sizeof(float));
	for (i = 0; i < dim->length; i++) bimage->steps[i] = 1.0;

	return bimage;
}

BIMAGE * BIMAGE_constructor_int(
	int *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
){
	BIMAGE * bimage;
	int i;
	int num_pixels;

	bimage = (BIMAGE *)malloc(sizeof(BIMAGE));
	bimage->buf = (float *)malloc(BVECT_prod(dim)*sizeof(float));
	num_pixels = BVECT_prod(dim);
	for (i = 0; i < num_pixels; i++) {
		bimage->buf[i] = (float)in[i];
	}
	bimage->dim = (BVECT *)malloc(sizeof(BVECT));
	bimage->dim->length = dim->length;
	bimage->dim->buf = (int *)malloc(dim->length*sizeof(int));
	memcpy(bimage->dim->buf, dim->buf, dim->length*sizeof(int));

	/* Set up the step vector, default to unit steps */
	bimage->steps = (float *)malloc(dim->length*sizeof(float));
	for (i = 0; i < dim->length; i++) bimage->steps[i] = 1.0;

	return bimage;
}

BIMAGE * BIMAGE_constructor_char(
	char *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
) {
	BIMAGE * bimage;
	int i;
	int num_pixels;

	bimage = (BIMAGE *)malloc(sizeof(BIMAGE));
	bimage->buf = (float *)malloc(BVECT_prod(dim)*sizeof(float));
	num_pixels = BVECT_prod(dim);
	for (i = 0; i < num_pixels; i++) {
		bimage->buf[i] = (float)in[i];
	}
	bimage->dim = (BVECT *)malloc(sizeof(BVECT));
	bimage->dim->length = dim->length;
	bimage->dim->buf = (int *)malloc(dim->length*sizeof(int));
	memcpy(bimage->dim->buf, dim->buf, dim->length*sizeof(int));

	/* Set up the step vector, default to unit steps */
	bimage->steps = (float *)malloc(dim->length*sizeof(float));
	for (i = 0; i < dim->length; i++) bimage->steps[i] = 1.0;

	return bimage;
}

void BIMAGE_destructor(BIMAGE * bimage) {
	free((void *)bimage->steps);
	free((void *)bimage->dim->buf);
	free((void *)bimage->dim);
	free((void *)bimage->buf);
	free((void *)bimage);
}
