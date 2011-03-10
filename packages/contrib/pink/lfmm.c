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
 * File:		FMM.c
 *
 * Written by:		Ben Appleton
 *				ITEE, The University of Queensland
 *
 * Date:		November 2002
 *
 * Copyright:	Intended for open-source distribution
 *
*/

/*********************************************************************************************
 FMM.c
 ------

  DESCRIPTION:
  Support functions for algorithms based on Sethian's Fast Marching Method and
  Vladimirsky's Ordered Upwinding Method.

  HISTORY:
  Created by Ben Appleton (11/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "pde_toolbox.h"
#include "lfmm.h"

#ifndef FLT_MAX
#    define FLT_MAX 3.40282347e+38F
#endif

/***************************************** FUNCTIONS *****************************************/
/* HEAPSTRUCT_constructor:
	Allocates memory for the HEAPSTRUCT
*/
HEAPSTRUCT * HEAPSTRUCT_constructor(
	BVECT * dim				/* The image dimensions (needed for hash table) */
)
{
	int i, num_pixels;
	HEAPSTRUCT * heapStruct;

	num_pixels = BVECT_prod(dim);

	heapStruct = (HEAPSTRUCT *)malloc(sizeof(HEAPSTRUCT));
	heapStruct->heapEnd = -1;	/* Initially empty */
	heapStruct->log2HeapSize = (int)ceil(log(num_pixels + 1)/log(2.0));
	heapStruct->heapToImage = (int *)malloc(num_pixels * sizeof(int));
	heapStruct->imageToHeap = (int *)malloc(num_pixels * sizeof(int));

	/* Avoid duplications by labelling pixels that aren't in the heap as -1 */
	for (i = 0; i < num_pixels; i++) {
		heapStruct->imageToHeap[i] = -1;
	}

	return heapStruct;
}

/* HEAPSTRUCT_destructor:
	Deallocates memory for the HEAPSTRUCT
*/
int HEAPSTRUCT_destructor(
	HEAPSTRUCT * heapStruct		/* The object to free */
)
{
	free((void *)heapStruct->heapToImage);
	free((void *)heapStruct->imageToHeap);
	free((void *)heapStruct);

	return 0;
}


/* - heapAdd:
	Insert the new image point into the heap, shuffling it down into the correct
	position and updating the backpointer (imageToHeap) array.
*/
int heapAdd(
	HEAPSTRUCT *heapStruct,		/* The encapsulation of the heap */
	int imageIndex,				/* The image index of the point to add */
	BIMAGE *phi)				/* The distance function */
{
	int index;

	/* Check for duplicates */
	if (heapStruct->imageToHeap[imageIndex] != -1) return 1;

	/* Insert it at the end of the heap */
	heapStruct->heapEnd++;
	heapStruct->heapToImage[heapStruct->heapEnd] = imageIndex;
	heapStruct->imageToHeap[imageIndex] = heapStruct->heapEnd;

	/* Now shuffle this point up the heap to where it belongs */
	index = heapStruct->heapEnd;
	heapShuffleUp(heapStruct, index, phi);

	return 0;
}


/* - heapPull:
	Remove the specified point in the heap, promoting the end of the heap and shuffling
	it into place.
*/
int heapPull(
	HEAPSTRUCT *heapStruct,	/* The heap structure, containing the indexes */
	int index,				/* The heap index of the element to remove */
	BIMAGE *phi) 			/* The 'distance' function by which we sort */
{
	float old_phi, new_phi;
	int old_image_index;

	old_image_index = heapStruct->heapToImage[index];
	old_phi = phi->buf[old_image_index];

	/* Write the end of the heap over the node to replace it */
	heapStruct->heapToImage[index] = heapStruct->heapToImage[heapStruct->heapEnd];
	heapStruct->imageToHeap[heapStruct->heapToImage[index]] = index;	/* Update backpointer */
	heapStruct->heapEnd--;		/* Maintain the heapEnd offset */
	heapStruct->imageToHeap[old_image_index] = -1;		/* Deleted point flagged: not in heap */

	new_phi = phi->buf[heapStruct->heapToImage[index]];

	/* Shuffle this point to where it belongs */
	if (new_phi < old_phi) {
		heapShuffleUp(heapStruct, index, phi);
	} else {
		heapShuffleDown(heapStruct, index, phi);
	}

	return 0;
}


/* - heapShuffleUp:
	Given the index to a node that has just decreased in value, shuffle it up the heap to maintain
	the heap property.
*/
int heapShuffleUp(
	HEAPSTRUCT *heapStruct,		/* The heap structure, containing the fast indices */
	int index,					/* The index (in the heap) of the point to be shuffled */
	BIMAGE *phi)				/* The 'distance' function */
{
	int newIndex, swap;

	/* Shuffle it up the tree until it's in the right place */
	while(LSTB_TRUE) {
		if (index == 0) break;	/* At root of tree, therefore complete */

		/* Grab the parent's index */
		newIndex = (index - 1)/2;

		/* If out of order, swap */
		if ( phi->buf[heapStruct->heapToImage[index]]
			< phi->buf[heapStruct->heapToImage[newIndex]]) {

			swap = heapStruct->heapToImage[index];
			heapStruct->heapToImage[index] = heapStruct->heapToImage[newIndex];
			heapStruct->heapToImage[newIndex] = swap;

			/* Maintain backpointers */
			swap = heapStruct->imageToHeap[heapStruct->heapToImage[index]];
			heapStruct->imageToHeap[heapStruct->heapToImage[index]]
				= heapStruct->imageToHeap[heapStruct->heapToImage[newIndex]];
			heapStruct->imageToHeap[heapStruct->heapToImage[newIndex]] = swap;
		} else {
			break;
		}

		/* Follow it up the tree */
		index = newIndex;
	}
	return 0;
}


/* - heapShuffleDown:
	Given the index to a node that has just increased in value, shuffle it down the
	heap to maintain the heap property.
*/
int heapShuffleDown(
	HEAPSTRUCT *heapStruct,		/* The heap structure, containing the fast indices */
	int index,					/* The index (in the heap) of the point to be shuffled */
	BIMAGE *phi)				/* The 'distance' function */
{
	int swap;

	/* Shuffle it down the heap.  Path is s.t. the heap structure is maintained */
	while(LSTB_TRUE) {
		float leftVal, rightVal;
		int newIndex;

		if ((2*index + 1) > heapStruct->heapEnd) break;	/* End of heap, nothing more to do */

		leftVal = phi->buf[heapStruct->heapToImage[2*index+1]];

		/* Check if a right child exists */
		if ((2*index + 2) > heapStruct->heapEnd) {
			rightVal = FLT_MAX;
		} else {
			rightVal = phi->buf[heapStruct->heapToImage[2*index + 2]];
		}

		/* Test which child is smaller, and promote the smaller of the two */
		if (leftVal < rightVal) {
			newIndex = 2*index + 1;
		} else {
			newIndex = 2*index + 2;
		}

		/* If each child is larger than the current node, shuffling is complete */
		if (phi->buf[heapStruct->heapToImage[index]]
			< phi->buf[heapStruct->heapToImage[newIndex]])
			break;

		/* Swap offsets */
		swap = heapStruct->heapToImage[index];
		heapStruct->heapToImage[index] = heapStruct->heapToImage[newIndex];
		heapStruct->heapToImage[newIndex] = swap;

		/* Maintain backpointers */
		swap = heapStruct->imageToHeap[heapStruct->heapToImage[index]];
		heapStruct->imageToHeap[heapStruct->heapToImage[index]]
			= heapStruct->imageToHeap[heapStruct->heapToImage[newIndex]];
		heapStruct->imageToHeap[heapStruct->heapToImage[newIndex]] = swap;

		/* Update the index and repeat */
		index = newIndex;
	}

	return 0;
}

/* Constructors and destructors for vectors and matrices */
FLOATVECT * FLOATVECT_constructor(
	int length					/* The length of the vector to create */
)
{
	FLOATVECT * vect;

	vect = (FLOATVECT *)malloc(sizeof(FLOATVECT));
	vect->length = length;
	vect->buf = (float *)malloc(length * sizeof(float));

	return vect;
}
void FLOATVECT_destructor(
	FLOATVECT * vect
)
{
	free((void *)vect->buf);
	free((void *)vect);
}

FLOATMATRIX * FLOATMATRIX_constructor(
	int rows,					/* How many rows and columns in the matrix to create */
	int cols
)
{
	FLOATMATRIX * matrix;

	matrix = (FLOATMATRIX *)malloc(sizeof(FLOATMATRIX));
	matrix->rows = rows;
	matrix->cols = cols;
	matrix->buf = (float *)malloc(rows*cols*sizeof(float));

	return matrix;
}
void FLOATMATRIX_destructor(
	FLOATMATRIX * matrix
)
{
	free((void *)matrix->buf);
	free((void *)matrix);
}

/* Vector and matrix functions */
/* Multiply a vector by a scalar: dest = scale_factor * source */
void FLOATVECT_scale(
	FLOATVECT * dest,
	float scale_factor,
	FLOATVECT * source
)
{
	int i;

	for (i = 0; i < dest->length; i++) {
		dest->buf[i] = scale_factor * source->buf[i];
	}
}

/* dest = souce1 + source2 */
void FLOATVECT_add(
	FLOATVECT * dest,
	FLOATVECT * source1,
	FLOATVECT * source2
)
{
	int i;

	for (i = 0; i < dest->length; i++) {
		dest->buf[i] = source1->buf[i] + source2->buf[i];
	}
}

/* Dot product */
float FLOATVECT_dot(
	FLOATVECT * source1,
	FLOATVECT * source2
)
{
	int i;
	float dot_prod;

	dot_prod = 0;
	for (i = 0; i < source1->length; i++) {
		dot_prod += source1->buf[i] * source2->buf[i];
	}

	return dot_prod;
}

/* Matrix by scalar: dest = scale_factor * source */
void FLOATMATRIX_scale(
	FLOATMATRIX * dest,
	float scale_factor,
	FLOATMATRIX * source
)
{
	int i;
	int num_components = dest->rows * dest->cols;

	for (i = 0; i < num_components; i++) {
		dest->buf[i] = scale_factor * source->buf[i];
	}
}

/* dest = source1 + source2 */
void FLOATMATRIX_add(
	FLOATMATRIX * dest,
	FLOATMATRIX * source1,
	FLOATMATRIX * source2
)
{
	int i;
	int num_components = dest->rows * dest->cols;

	for (i = 0; i < num_components; i++) {
		dest->buf[i] = source1->buf[i] + source2->buf[i];
	}
}

/* dest = source1 * source2 */
void FLOATMATRIX_mul(
	FLOATMATRIX * dest,
	FLOATMATRIX * source1,
	FLOATMATRIX * source2
)
{
	/* Multidimensional arrays are indexed :
		A[i][j] = (A[i])[j] =  *(A + i*ni + j) = A[i*ni + j];
	*/
	int i, j, k;
	FLOATMATRIX * temp;

	temp = FLOATMATRIX_constructor(dest->rows, dest->cols);

	/* Inner-product formulation of matrix multiplication */
	for (i = 0; i < dest->rows; i++) {
	for (j = 0; j < dest->cols; j++) {
		temp->buf[i*2 + j] = 0;
		for (k = 0; k < source1->cols; k++) {
			temp->buf[i*2 + j] += source1->buf[i*2 + k] * source2->buf[k*2 + j];
		}
	}
	}

	/* Now copy over output */
	memcpy(dest->buf, temp->buf, dest->rows*dest->cols*sizeof(float));

	/* Clean up extra matrix */
	FLOATMATRIX_destructor(temp);
}

/* Computes the determinant of a 2x2 matrix. */
float FLOATMATRIX_det(
	FLOATMATRIX * source
)
{
	/* Check we've been given a 2x2 matrix */
	if (!(source->rows == 2 && source->cols == 2)) {
		LSTB_error("FLOATMATRIX_det() is specific to 2x2 matrices for now!\n");
		return 0.0;
	}

	/* Specialised determinant computation */
	return source->buf[0*2+0]*source->buf[1*2+1] - source->buf[0*2+1]*source->buf[1*2+0];
}

/* dest = source^-1.  Returns determinant.  Specific to 2x2 matrices for now. */
float FLOATMATRIX_inv(
	FLOATMATRIX * dest,
	FLOATMATRIX * source
)
{
	FLOATMATRIX * temp;
	float det;

	/* Check we've been given a 2x2 matrix */
	if (!(source->rows == 2 && source->cols == 2)) {
		LSTB_error("FLOATMATRIX_inv() is specific to 2x2 matrices for now!\n");
		return 0.0;
	}

	temp = FLOATMATRIX_constructor(2, 2);

	det = FLOATMATRIX_det(source);

	/* Specialised matrix inversion for 2x2 matrices */
	temp->buf[0*2+0] = source->buf[1*2+1] / det;
	temp->buf[0*2+1] = -source->buf[0*2+1] / det;
	temp->buf[1*2+0] = -source->buf[1*2+0] / det;
	temp->buf[1*2+1] = source->buf[0*2+0] / det;

	/* Now copy over output */
	memcpy(dest->buf, temp->buf, dest->rows*dest->cols*sizeof(float));

	/* Clean up extra matrix */
	FLOATMATRIX_destructor(temp);

	return det;
}

/* dest = source' */
void FLOATMATRIX_transpose(
	FLOATMATRIX * dest,
	FLOATMATRIX * source
)
{
	int i, j;
	FLOATMATRIX * temp;

	temp = FLOATMATRIX_constructor(dest->rows, dest->cols);

	/* Multidimensional arrays are indexed :
		A[i][j] = (A[i])[j] =  *(A + i*ni + j) = A[i*ni + j];
	*/
	for (i = 0; i < dest->rows; i++) {
	for (j = 0; j < dest->cols; j++) {
		temp->buf[i*dest->cols + j] = source->buf[j*dest->cols + i];
	}
	}

	/* Now copy over output */
	memcpy(dest->buf, temp->buf, dest->rows*dest->cols*sizeof(float));

	/* Clean up extra matrix */
	FLOATMATRIX_destructor(temp);
}

/* Matrix-vector */
/* dest = matrix * source */
void FLOATVECT_premul(
	FLOATVECT * dest,
	FLOATMATRIX * matrix,
	FLOATVECT * source
)
{
	int i, j;

	FLOATVECT * temp;

	temp = FLOATVECT_constructor(dest->length);

	/* Initially zero the destination vector */
	memset(temp->buf, 0, dest->length*sizeof(float));

	/* Multidimensional arrays are indexed :
		A[i][j] = (A[i])[j] =  *(A + i*ni + j) = A[i*ni + j];
	*/
	for (i = 0; i < matrix->rows; i++) {
		for (j = 0; j < matrix->cols; j++) {
			temp->buf[i] += matrix->buf[i*matrix->cols + j] * source->buf[j];
		}
	}

	/* Now copy over output */
	memcpy(dest->buf, temp->buf, dest->length*sizeof(float));

	/* Clean up extra matrix */
	FLOATVECT_destructor(temp);
}
/* dest = source * matrix */
void FLOATVECT_postmul(
	FLOATVECT * dest,
	FLOATVECT * source,
	FLOATMATRIX * matrix
)
{
	int i, j;

	FLOATVECT * temp;

	temp = FLOATVECT_constructor(dest->length);

	/* Initially zero the destination vector */
	memset(temp->buf, 0, dest->length*sizeof(float));

	/* Multidimensional arrays are indexed :
		A[i][j] = (A[i])[j] =  *(A + i*ni + j) = A[i*ni + j];
	*/
	for (i = 0; i < matrix->rows; i++) {
		for (j = 0; j < matrix->cols; j++) {
			temp->buf[j] += source->buf[i] * matrix->buf[i*matrix->cols + j];
		}
	}

	/* Now copy over output */
	memcpy(dest->buf, temp->buf, dest->length*sizeof(float));

	/* Clean up extra matrix */
	FLOATVECT_destructor(temp);
}


/*************************** Riemann surface related functions **********************/
/* RIE_INT_constructor:
	Constructor for integer coordinates on the Riemann surface for ln.
*/
RIE_INT * RIE_INT_constructor(
) {
	RIE_INT * this;

	this = (RIE_INT *)malloc(sizeof(RIE_INT));

	return this;
}


/* RIE_INT_destructor:
	A destructor for integer coordinates on the Riemann surface for ln
*/
void RIE_INT_destructor(
	RIE_INT * this
) {
	free((void *)this);
}


/* RIE_FLOAT_constructor:
	Constructor for float coordinates on the Riemann surface for ln.
*/
RIE_FLOAT * RIE_FLOAT_constructor(
) {
	RIE_FLOAT * this;

	this = (RIE_FLOAT *)malloc(sizeof(RIE_FLOAT));

	return this;
}


/* RIE_FLOAT_destructor:
	A destructor for float coordinates on the Riemann surface for ln.
*/
void RIE_FLOAT_destructor(
	RIE_FLOAT * this
) {
	free((void *)this);
}


/* RIE_SURF_constructor:
	Constructor for structure to hold parameters of Riemann surface for ln.
*/
RIE_SURF * RIE_SURF_constructor(
) {
	RIE_SURF * this;

	this = (RIE_SURF *)malloc(sizeof(RIE_SURF));

	return this;
}


/* RIE_SURF_destructor:
	Destructor for RIE_SURF structure.
*/
void RIE_SURF_destructor(
	RIE_SURF * this
) {
	free((void *)this);
}


/* RIE_INT_add:
	Offset a coordinate on the Riemann surface for ln.
*/
int RIE_INT_add(
	int dx, int dy,
	char restrict_to_surface,		/* Restrict the new point to the Riemann surface? */
	RIE_SURF * rie_surf,
	RIE_INT * this
) {
	RIE_INT new;

	/* Deal with special case */
	if (dx == 0 && dy == 0) return 0;

	/* Offset the point */
	new.x = this->x + dx;
	new.y = this->y + dy;
	new.z = this->z;			/* Default value */

	/* Check if we have to change layer */
	/* Simplify things by an approximation */
	if (new.x < rie_surf->source_x - rie_surf->source_radius
		&& this->x < rie_surf->source_x - rie_surf->source_radius) {
		if (new.y >= rie_surf->source_y && this->y < rie_surf->source_y) {
			new.z = this->z + 1;
		} else if (new.y < rie_surf->source_y && this->y >= rie_surf->source_y) {
			new.z = this->z - 1;
		} else {
			new.z = this->z;
		}
	}

	/* Restrict the coordinate to a valid surface point */
	if (restrict_to_surface)
		if (!RIE_INT_on_surface(&new, rie_surf))
			return 1;

	/* Copy temporary coordinate to output */
	memcpy(this, &new, sizeof(RIE_INT));

	return 0;
}


/* RIE_FLOAT_add:
	Offset a coordinate on the Riemann surface for ln.
*/
int RIE_FLOAT_add(
	float dx, float dy,
	char restrict_to_surface,		/* Restrict the new point to the Riemann surface? */
	RIE_SURF * rie_surf,
	RIE_FLOAT * this
) {
	RIE_FLOAT new;

	/* Deal with special case */
	if (dx == 0 && dy == 0) return 0;

	/* Offset the point */
	new.x = this->x + dx;
	new.y = this->y + dy;
	new.z = this->z;			/* Default value */

	/* Check if we have to change layer */
	/* Simplify things by an approximation */
	if (new.x < rie_surf->source_x && this->x < rie_surf->source_x) {
		if (new.y >= rie_surf->source_y && this->y < rie_surf->source_y) {
			new.z = this->z + 1;
		} else if (new.y < rie_surf->source_y && this->y >= rie_surf->source_y) {
			new.z = this->z - 1;
		} else {
			new.z = this->z;
		}
	}

	/* Restrict the coordinate to a valid surface point */
	if (restrict_to_surface)
		if (!RIE_FLOAT_on_surface(&new, rie_surf))
			return 1;

	/* Copy temporary coordinate to output */
	memcpy(this, &new, sizeof(RIE_FLOAT));

	return 0;
}


/* RIE_INT_to_index:
	Convert an integer Riemann coordinate to an index into an image buffer.
	Must be a valid coordinate!
*/
int RIE_INT_to_index(
	RIE_INT * this,
	RIE_SURF * rie_surf
) {
	int index;

	index = this->x + rie_surf->nx*(this->y + rie_surf->ny*this->z);

	return index;
}

/* index_to_RIE_INT:
	Convert an index into an image buffer to an integer coordinate
	on the Riemann surface.
*/
int index_to_RIE_INT(
	int index,
	RIE_INT * coord,
	RIE_SURF * rie_surf
)
{
	coord->x = index % rie_surf->nx;
	index /= rie_surf->nx;
	coord->y = index % rie_surf->ny;
	index /= rie_surf->ny;
	coord->z = index % rie_surf->num_revs;

	return 0;
}

/* RIE_INT_on_surface:
	Check if a given point lies on the Riemann surface (else out of bounds)
*/
char RIE_INT_on_surface(
	RIE_INT * this,
	RIE_SURF * rie_surf
) {
	/* Check outer bounds */
	if (this->x < 0 || this->x > rie_surf->nx - 1) {
		return LSTB_FALSE;
	}
	if (this->y < 0 || this->y > rie_surf->ny - 1) {
		return LSTB_FALSE;
	}
	if (this->z < 0 || this->z > rie_surf->num_revs - 1) {
		return LSTB_FALSE;
	}

	/* Check it doesn't clash with the seed point */
	if (LSTB_ABS(this->x - rie_surf->source_x) <= rie_surf->source_radius
		&& LSTB_ABS(this->y - rie_surf->source_y) <= rie_surf->source_radius) {
		return LSTB_FALSE;
	}

	return LSTB_TRUE;
}

/* RIE_FLOAT_on_surface:
	Check if a given point lies on the Riemann surface (else out of bounds)
*/
char RIE_FLOAT_on_surface(
	RIE_FLOAT * this,
	RIE_SURF * rie_surf
) {
	/* Check outer bounds */
	if (this->x < 0 || this->x > rie_surf->nx - 1) {
		return LSTB_FALSE;
	}
	if (this->y < 0 || this->y > rie_surf->ny - 1) {
		return LSTB_FALSE;
	}
	if (this->z < 0 || this->z > rie_surf->num_revs - 1) {
		return LSTB_FALSE;
	}

	/* Check it doesn't clash with the seed point */
	if (LSTB_ABS(this->x - rie_surf->source_x) <= rie_surf->source_radius &&
		LSTB_ABS(this->y - rie_surf->source_y) <= rie_surf->source_radius) {
		return LSTB_FALSE;
	}

	return LSTB_TRUE;
}
