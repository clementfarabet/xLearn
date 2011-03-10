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
 * File:		bimage_utils.c
 *
 * Written by:		Ben Appleton
 *
 * Date:		October 2002
 *
 *
 * 			Intended for open-source distribution
 *
*/

/*********************************************************************************************
 bimage_utils.c
 ------

  DESCRIPTION:
  A collection of image preprocessing functions

  HISTORY:
  Created by Ben Appleton (October 2002)
  Contact: appleton@itee.uq.edu.au

**********************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pde_toolbox.h"
#include "lfmm.h"

/**************************STATIC FUNCTION DEFINITIONS ***************************************/
static int upsample_row(
	float *in,						/* Input row to upsample */
	int in_length,					/* Length of input row */
	float *out,						/* Output row */
	int out_length,					/* Length of output row */
	int upsampling_rate				/* Upsampling rate */
);

/**************************** FUNCTION IMPLEMENTATIONS ***************************************/
/* BIMAGE_abs_grad:
	Compute the absolute gradient of an image.
	The gradients are computed via central differencing.
	in and out may point to the same image
*/
int BIMAGE_abs_grad(BIMAGE * in, BIMAGE *out) {
	float * temp;
	int num_pixels = BVECT_prod(in->dim);
	int index, temp_index, dimension, d;
	BVECT * coord, * temp_coord;

	coord = BVECT_constructor(in->dim->length);
	temp_coord = BVECT_constructor(in->dim->length);

	temp = (float *)malloc(num_pixels*sizeof(float));

	/* For each pixel in the image */
	BVECT_zero(coord);
	for (index = 0; index < num_pixels; index++, BVECT_inc(coord, in->dim)) {
		float sqr_grad;

		/* For all face-connected neighbours */
		sqr_grad = 0;
		for (dimension = 0; dimension < in->dim->length; dimension++) {
			float difference = 0;
			for (d = -1; d <= 1; d+=2) {
				/* Compute the new coordinate */
				BVECT_copy(temp_coord, coord);
				temp_coord->buf[dimension] += d;

				/* Reflect this point if it lies outside the image domain */
				if (temp_coord->buf[dimension] < 0 || temp_coord->buf[dimension] > in->dim->buf[dimension] - 1) {
					 difference += d*in->buf[index];
				} else {
					temp_index = bvectToInt(temp_coord, in->dim);
					difference += d*in->buf[temp_index];
				}
			}
			difference /= 2.0;	/* Central differencing gives 2*gradient */
			sqr_grad += difference * difference;
		}
		temp[index] = sqrt(sqr_grad);
	}

	/* Copy the gradient image to output */
	memcpy(out->buf, temp, num_pixels*sizeof(float));

	/* Free all allocated memory */
	free((void *)temp);
	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);

	return 0;
}


/* BIMAGE_rad_grad:
	Compute the radial gradient of an image.
	The gradients are computed via central differencing.
	in and out may point to the same image
*/
int BIMAGE_rad_grad(BIMAGE * in, BIMAGE *out, BVECT * centre) {
	float * temp;
	int num_pixels = BVECT_prod(in->dim);
	int index, temp_index, dimension, d;
	BVECT * coord, * temp_coord, * radial_vect;

	/* Allocate memory */
	coord = BVECT_constructor(in->dim->length);
	temp_coord = BVECT_constructor(in->dim->length);
	radial_vect = BVECT_constructor(in->dim->length);
	temp = (float *)malloc(num_pixels*sizeof(float));

	/* For each pixel in the image */
	BVECT_zero(coord);
	for (index = 0; index < num_pixels; index++, BVECT_inc(coord, in->dim)) {
		float rad_grad, r;

		BVECT_copy(radial_vect, coord);
		BVECT_sub(radial_vect, centre);
		r = sqrt(BVECT_sum_sqr(radial_vect));

		/* For all face-connected neighbours */
		rad_grad = 0;
		for (dimension = 0; dimension < in->dim->length; dimension++) {
			float difference = 0;
			for (d = -1; d <= 1; d+=2) {
				/* Compute the new coordinate */
				BVECT_copy(temp_coord, coord);
				temp_coord->buf[dimension] += d;

				/* Reflect this point if it lies outside the image domain */
				if (temp_coord->buf[dimension] < 0 || temp_coord->buf[dimension] > in->dim->buf[dimension] - 1) {
					 difference += d*in->buf[index];
				} else {
					temp_index = bvectToInt(temp_coord, in->dim);
					difference += d*in->buf[temp_index];
				}
			}
			difference /= 2.0;	/* Central differencing gives 2*gradient */
			rad_grad += radial_vect->buf[dimension] * difference / r;
		}
		temp[index] = rad_grad;
	}

	/* Copy the gradient image to output */
	memcpy(out->buf, temp, num_pixels*sizeof(float));

	/* Free all allocated memory */
	free((void *)temp);
	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);
	BVECT_destructor(radial_vect);

	return 0;
}


/* - BIMAGE_radial_weighting:
	Weight the metric radially for scale invariance (*= 1/r^(N-1), N the number of dimensions)
*/
void BIMAGE_radial_weighting(
	BIMAGE * g,					/* The original metric (overwrite) */
	BVECT * source				/* The coordinates of the source */
)
{
	int i, j, num_pixels;
	BVECT * coord;

	/* Precompute the number of pixels */
	num_pixels = BVECT_prod(g->dim);

	/* Allocate memory for coordinates */
	coord = BVECT_constructor(g->dim->length);

	for (i = 0; i < num_pixels; i++) {
		float dist;

		/* Compute the coordinate of this point */
		intToBvect(i, coord, g->dim);

		/* Compute the distance to the source */
		BVECT_sub(coord, source);	/* coord -= source */
		dist = sqrt(BVECT_sum_sqr(coord));

		/* Radial weight according to spatial dimensions */
		for (j = 0; j < g->dim->length - 1; j++) {
			g->buf[i] /= (dist + 0.1f);
		}
	}

	/* Free memory */
	BVECT_destructor(coord);
}


/* BIMAGE_blur:
	Blur an image by [1 2 1]/4 in each dimension 'blurring' times.
*/
int BIMAGE_blur(
	BIMAGE * in,
	BIMAGE * out,
	int blurring
) {
	int dimension, index, b;
	BVECT * coord, * temp_coord;
	/* Double buffering */
	float * inbuf, * outbuf, * swap;
	int num_pixels = BVECT_prod(in->dim);

	/* Check they have the same dimensions */
	if (!BVECT_compare(in->dim, out->dim)) return 1;

	coord = BVECT_constructor(in->dim->length);
	temp_coord = BVECT_constructor(in->dim->length);
	inbuf = (float *)malloc(BVECT_prod(in->dim)*sizeof(float));
	outbuf = (float *)malloc(BVECT_prod(in->dim)*sizeof(float));

	memcpy(inbuf, in->buf, BVECT_prod(in->dim)*sizeof(float));
	for (dimension = 0; dimension < in->dim->length; dimension++) {
		int stride;
		/* Compute the memory stride for this dimension */
		BVECT_zero(temp_coord);
		temp_coord->buf[dimension] = 1;
		stride = bvectToInt(temp_coord, in->dim);

		for (b = 0; b < blurring; b++) {

			BVECT_zero(coord);
			for (index = 0; index < num_pixels; index++, BVECT_inc(coord, in->dim)) {
				outbuf[index] = 0.0;
				if (coord->buf[dimension] >= 1) {
					outbuf[index] += inbuf[index - stride];
				} else {
					outbuf[index] += inbuf[index];
				}
				outbuf[index] += 2.0*inbuf[index];
				if (coord->buf[dimension] <= in->dim->buf[dimension] - 2) {
					outbuf[index] += inbuf[index + stride];
				} else {
					outbuf[index] += inbuf[index];
				}
				outbuf[index] /= 4.0;
			}

			/* Ping-pong the buffers */
			swap = inbuf;
			inbuf = outbuf;
			outbuf = swap;
		}
	}
	memcpy(out->buf, outbuf, BVECT_prod(in->dim)*sizeof(float));

	free((void *)inbuf);
	free((void *)outbuf);
	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);

	return 0;
}

/* BIMAGE_aosblur:
	Blur an image with an implicit (IIR-like) Gaussian, using a semi-implicit
	implementation of linear diffusion.
*/
int BIMAGE_aosblur(
	BIMAGE *in,						/* The image data to blur */
	BIMAGE *out,					/* The output image.  May point to in */
	float blurring 				/* The time over which the PDE is run */
)
{
	int num_pixels;
	float time_step;

	/* Intermediate calculation space when solving LDL' system */
	LDLCOMP * ldl_array;				/* Temporary array for LDL decompositions */
	float *intermediate_vector;			/* An intermediate vector used when inverting matrices */
	int max_axis_length;				/* The maximum axis length of the image */
	BVECT * row_coord, * row_dim, * coord;

	/* Loop variables */
	int i, dimension;
	float time;

	/* Compute the largest dividing timestep < 20 */
	time_step = blurring / (float)ceil(blurring/IM_AOS_BLUR_MAX_TIME_STEP);

	/* Setup memory and commonly used variables */
	num_pixels = BVECT_prod(in->dim);
	max_axis_length = BVECT_max(in->dim);
	row_coord = BVECT_constructor(in->dim->length - 1);
	row_dim = BVECT_constructor(in->dim->length - 1);
	coord = BVECT_constructor(in->dim->length);
	ldl_array = (LDLCOMP *)malloc(max_axis_length * sizeof(LDLCOMP));
	intermediate_vector = (float *)malloc(max_axis_length * sizeof(float));

	if (ldl_array == NULL || intermediate_vector == NULL) {
		LSTB_error("BIMAGE_aosblur.c out of memory!\n");
		return 1;
	}

	/* In-place algorithm, so copy input to output */
	memcpy(out->buf, in->buf, num_pixels*sizeof(float));

	for (time = 0; time < blurring; time += time_step) {
		/* For each dimension */
		for (dimension = 0; dimension < out->dim->length; dimension++) {
			int row, num_rows, stride, offset;
			float a, b;
			float scaling;

			/* Compute the image stride along this dimension (could be optimised, but simpler this way) */
			stride = 1;
			for (i = 0; i < dimension; i++)
				stride *= out->dim->buf[i];

			/* Determine the row space (dimensions of face of hyperslab) */
			for (i = 0; i < out->dim->length - 1; i++) {
				if (i < dimension) {
					row_dim->buf[i] = out->dim->buf[i];
				} else { /* if (i >= dimension) */
					row_dim->buf[i] = out->dim->buf[i+1];
				}
			}
			num_rows = BVECT_prod(row_dim);

			/* Compute the backward step matrix (symmetric tridiagonal) */
			memset(ldl_array, 0, max_axis_length*sizeof(LDLCOMP));
			scaling = time_step;
			for (offset = 1; offset < out->dim->buf[dimension] - 1; offset++) {
				ldl_array[offset].D = 1.0 + scaling * 2.0;
			}
			ldl_array[0].D = 1.0 + scaling;
			ldl_array[out->dim->buf[dimension]-1].D = 1.0 + scaling;
			for (offset = 0; offset < out->dim->buf[dimension] - 1; offset++) {
				ldl_array[offset].L = -scaling;
			}

			/* Compute the ldl decomposition of the backward step matrix */
			a = b = 0;	/* Cache */
			for (offset = 0; offset < out->dim->buf[dimension]; offset++) {
				ldl_array[offset].D -= b*b*a;
				a = ldl_array[offset].D;			/* Cache */
				ldl_array[offset].L /= a;
				b = ldl_array[offset].L;			/* Cache */
			}

			/* For each row along this dimension */
			for (row = 0, BVECT_zero(row_coord); row < num_rows; BVECT_inc(row_coord, row_dim), row++) {
				int row_base_index;

				/* Compute the row base index */
				for (i = 0; i < out->dim->length; i++) {
					if (i < dimension) {
						coord->buf[i] = row_coord->buf[i];
					} else if (i == dimension) {
						coord->buf[i] = 0;
					} else {
						coord->buf[i] = row_coord->buf[i-1];
					}
				}
				row_base_index = bvectToInt(coord, out->dim);

				/* Copy this row into a temporary array */
				for (offset = 0; offset < out->dim->buf[dimension]; offset++) {
					intermediate_vector[offset] = out->buf[row_base_index + stride*offset];
				}

				/* Solve the linear-implicit equation */
				/* Solve Lz = b */
				a = intermediate_vector[0];
				for (offset = 1; offset < out->dim->buf[dimension]; offset++) {
					intermediate_vector[offset] -= a*ldl_array[offset-1].L;
					a = intermediate_vector[offset];
				}

				/* Solve Dy = z */
				for (offset = 0; offset < out->dim->buf[dimension]; offset++) {
					intermediate_vector[offset] /= ldl_array[offset].D;
				}

				/* Solve L'x = b */
				a = intermediate_vector[out->dim->buf[dimension]-1];
				for (offset = out->dim->buf[dimension] - 2; offset >= 0; offset--) {
					intermediate_vector[offset] -= a*ldl_array[offset].L;
					a = intermediate_vector[offset];
				}

				/* Overwrite existing row */
				for (offset = 0; offset < out->dim->buf[dimension]; offset++) {
					out->buf[row_base_index + offset*stride] = intermediate_vector[offset];
				}
			}
		}
	}

	free((void *)ldl_array);
	free((void *)intermediate_vector);
	BVECT_destructor(row_coord);
	BVECT_destructor(row_dim);
	BVECT_destructor(coord);

	return 0;
}

/* BIMAGE_metrify:
	Convert a gradient image to a metric image by 1/(1+|grad|^p)
*/
int BIMAGE_metrify(
	BIMAGE * in,				/* The input gradient image */
	BIMAGE * out,				/* Output image, may point to input */
	float normalisation,		/* Normalisation factor */
	int p,						/* Should be 1 or 2 */
	float epsilon				/* Length penalty weight */
)
{
	int i;
	int num_pixels = BVECT_prod(in->dim);

	if (p < 1) p = 1;
	if (p > 2) p = 2;

	if (p == 1) {
		for (i = 0; i < num_pixels; i++) {
			out->buf[i] = 1.0 / (1.0 + in->buf[i]/normalisation) + epsilon;
		}
	}
	if (p == 2) {
		for (i = 0; i < num_pixels; i++) {
			out->buf[i] = 1.0 / (1.0 + LSTB_SQR(in->buf[i]/normalisation)) + epsilon;
		}
	}

	return 0;
}

/* BIMAGE_tensor_metric:
	Construct a metric tensor from a 2D image.  Used
	for segmentation by Ordered Upwind Methods.
*/
int BIMAGE_tensor_metric(
	BIMAGE * in,				/* Input image (pre blurred, but not grad'd) */
	METRIC2D * metric,			/* Output metric */
	float normalisation,		/* Normalisation factor */
	int p,						/* Should be 1 or 2 */
	float epsilon				/* Length penalty weight */
)
{
	int num_pixels = BVECT_prod(in->dim);
	int index, temp_index, dimension, d, i;
	BVECT * coord, * temp_coord;
	FLOATVECT * grad_vect;
	FLOATMATRIX * g,  * temp_matrix;

	/* This function only deals with 2D images */
	if (in->dim->length != 2) {
		LSTB_error("Image must be 2D!\n");
		return 1;
	}

	/* Enforce range of power p */
	if (p < 1) p = 1;
	if (p > 2) p = 2;

	/* Allocate memory */
	coord = BVECT_constructor(in->dim->length);
	temp_coord = BVECT_constructor(in->dim->length);
	grad_vect = FLOATVECT_constructor(in->dim->length);
	g = FLOATMATRIX_constructor(in->dim->length, in->dim->length);
	temp_matrix = FLOATMATRIX_constructor(in->dim->length, in->dim->length);

	/* For each pixel in the image */
	BVECT_zero(coord);
	for (index = 0; index < num_pixels; index++, BVECT_inc(coord, in->dim)) {
		float abs_grad;

		/* Compute grad from face-connected neighbours */
		for (dimension = 0; dimension < in->dim->length; dimension++) {
			float difference = 0;
			for (d = -1; d <= 1; d+=2) {
				/* Compute the new coordinate */
				BVECT_copy(temp_coord, coord);
				temp_coord->buf[dimension] += d;

				/* Reflect this point if it lies outside the image domain */
				if (temp_coord->buf[dimension] < 0 || temp_coord->buf[dimension] > in->dim->buf[dimension] - 1) {
					 difference += d*in->buf[index];
				} else {
					temp_index = bvectToInt(temp_coord, in->dim);
					difference += d*in->buf[temp_index];
				}
			}
			difference /= 2.0;	/* Central differencing gives 2*gradient */
			grad_vect->buf[dimension] = difference;
		}

		/* Extract magnitude */
		abs_grad = 0;
		for (i = 0; i < in->dim->length; ++i)
			abs_grad += LSTB_SQR(grad_vect->buf[i]);
		abs_grad = sqrt(abs_grad);

		if (abs_grad < LSTB_SMALLNUM) {
			metric->a->buf[index] = 1;
			metric->b->buf[index] = 0;
			metric->c->buf[index] = 1;
			continue;
		}

		/* Normalise */
		for (i = 0; i < in->dim->length; ++i) {
			grad_vect->buf[i] /= abs_grad;
		}

		/* Construct metric tensor */
		/* Here temp_matrix is eigenvectors of metric tensor as columns */
		temp_matrix->buf[0*2+0] = grad_vect->buf[0];
		temp_matrix->buf[1*2+0] = grad_vect->buf[1];
		temp_matrix->buf[0*2+1] = grad_vect->buf[1];
		temp_matrix->buf[1*2+1] = -grad_vect->buf[0];
		/* Eigenvalues of metric tensor */
		if (p == 1) {
				g->buf[1*2+1] = 1.0 / (1.0 + abs_grad/normalisation);
		} else {
				g->buf[1*2+1] = 1.0 / (1.0 + LSTB_SQR(abs_grad/normalisation));
		}
		g->buf[0*2+0] = 1.0;
		g->buf[1*2+0] = 0.0;
		g->buf[0*2+1] = 0.0;
		/* From desired eigen decomposition construct metric tensor */
		FLOATMATRIX_mul(g, temp_matrix, g);
		FLOATMATRIX_transpose(temp_matrix, temp_matrix);
		FLOATMATRIX_mul(g, g, temp_matrix);

		/* Add isotropic (and constant) length-penalty term for regularisation */
		g->buf[0*2+0] += LSTB_SQR(epsilon);
		g->buf[1*2+1] += LSTB_SQR(epsilon);
		/* Not the same as a strict length-penalty; but close? */

		/* Store */
		metric->a->buf[index] = g->buf[0*2+0];
		metric->b->buf[index] = g->buf[0*2+1];
		metric->c->buf[index] = g->buf[1*2+1];
	}

	/* Free all allocated memory */
	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);
	FLOATVECT_destructor(grad_vect);
	FLOATMATRIX_destructor(g);
	FLOATMATRIX_destructor(temp_matrix);

	return 0;
}


/* BIMAGE_line_metric:
	Construct a metric tensor from a 2D image.  This version
	is specialised for finding linear features.
*/
int BIMAGE_line_metric(
	BIMAGE * in,				/* Input image (pre blurred, but not grad'd) */
	METRIC2D * metric,			/* Output metric */
	float normalisation,		/* Normalisation factor */
	float epsilon				/* Euclidean length penalty */
)
{
	int num_pixels = BVECT_prod(in->dim);
	int N = in->dim->length;
	int index, temp_index, d, d1, d2, i;
	BVECT * coord, * temp_coord;
	BIMAGE * * Dvector;				/* Vector of first differentials */
	BIMAGE * * Dmatrix;				/* Matrix of second differentials */
	FLOATVECT * vect;
	FLOATMATRIX * matrix, * g;

	/* This function only deals with 2D images */
	if (N != 2) {
		LSTB_error("Image must be 2D!\n");
		return 1;
	}

	/* Allocate memory */
	coord = BVECT_constructor(N);
	temp_coord = BVECT_constructor(N);
	vect = FLOATVECT_constructor(N);
	matrix = FLOATMATRIX_constructor(N, N);
	g = FLOATMATRIX_constructor(N, N);

	/* Allocate differential images */
	Dvector = (BIMAGE * *)malloc(N * sizeof(BIMAGE *));
	for (d1 = 0; d1 < N; d1++) {
		Dvector[d1] = BIMAGE_constructor(in->dim);
	}
	Dmatrix = (BIMAGE * *)malloc(LSTB_SQR(N) * sizeof(BIMAGE *));
	for (d1 = 0; d1 < N; d1++) {
	for (d2 = 0; d2 <= d1; d2++) {
		Dmatrix[d1 + N*d2] = BIMAGE_constructor(in->dim);
	}
	}

	/* Compute the vector of first differentials */
	BVECT_zero(coord);
	for (index = 0; index < num_pixels; index++, BVECT_inc(coord, in->dim)) {
		/* Compute components of differential */
		for (d1 = 0; d1 < N; d1++) {
			float difference = 0;
			for (d = -1; d <= 1; d+=2) {
				/* Compute the new coordinate */
				BVECT_copy(temp_coord, coord);
				temp_coord->buf[d1] += d;

				/* Reflect this point if it lies outside the image domain */
				if (temp_coord->buf[d1] < 0 || temp_coord->buf[d1] > in->dim->buf[d1] - 1) {
					 difference += d*in->buf[index];
				} else {
					temp_index = bvectToInt(temp_coord, in->dim);
					difference += d*in->buf[temp_index];
				}
			}
			/* Central differencing gives 2*gradient */
			difference /= 2.0;

			/* Store */
			Dvector[d1]->buf[index] = difference;
		}
	}

	/* Compute the matrix of second differentials from the first diffs */
	BVECT_zero(coord);
	for (index = 0; index < num_pixels; index++, BVECT_inc(coord, in->dim)) {
		/* Compute second differential matrix from first differential vectors */
		for (d1 = 0; d1 < N; d1++) {
		for (d2 = 0; d2 <= d1; d2++) {
			float difference = 0;
			for (d = -1; d <= 1; d+=2) {
				/* Compute the new coordinate */
				BVECT_copy(temp_coord, coord);
				temp_coord->buf[d2] += d;

				/* Reflect this point if it lies outside the image domain */
				if (temp_coord->buf[d2] < 0 || temp_coord->buf[d2] > in->dim->buf[d2] - 1) {
					 difference += d*Dvector[d1]->buf[index];
				} else {
					temp_index = bvectToInt(temp_coord, in->dim);
					difference += d*Dvector[d1]->buf[temp_index];
				}
			}
			/* Central differencing gives 2*gradient */
			difference /= 2.0;

			/* Store */
			Dmatrix[d1 + N*d2]->buf[index] = difference;
		}
		}
	}

	/* Construct metric */
	for (index = 0; index < num_pixels; index++) {
		float a, b, c;
		float det, num, den, ratio;

		/* Extract second derivative matrix */
		a = Dmatrix[0+2*0]->buf[index];
		b = Dmatrix[1+2*0]->buf[index];
		c = Dmatrix[1+2*1]->buf[index];

		/* Compute ratio of eigenvalues and minor eigenvector */
		det = sqrt((a-c)*(a-c)/4.0 + b*b);
		num = (a+c)/2.0 + det;
		den = (a+c)/2.0 - det;
		/* Numerator should be the larger of the two */
		if (LSTB_ABS(num) < LSTB_ABS(den)) {
			float swap = num;
			num = den;
			den = swap;
		}
		/* Seek negative Laplacian for bright lines on dark backgrounds */
		if (num > 0) num = 0;
		
		/* Convert to absolutes now */
		num = LSTB_ABS(num);
		den = LSTB_ABS(den);

		if (den < LSTB_SMALLNUM) {
			/* 0 eigenvalue - ignore and hope it's ok */
			g->buf[0*2+0] = 1.0 + LSTB_SQR(epsilon);
			g->buf[1*2+0] = 0.0;
			g->buf[0*2+1] = 0.0;
			g->buf[1*2+1] = 1.0 + LSTB_SQR(epsilon);
		} else {
			ratio = num/den;

			/* Extract minor eigenvector by powering method */
			matrix->buf[0*N+0] = a;
			matrix->buf[0*N+1] = b;
			matrix->buf[1*N+0] = b;
			matrix->buf[1*N+1] = c;

			FLOATMATRIX_inv(matrix, matrix);

			for (i = 0; i < 5; i++) {
				int j;
				float max;

				FLOATMATRIX_mul(matrix, matrix, matrix);
				/* Normalise */
				max = LSTB_SMALLNUM;
				for (j = 0; j < N*N; j++) {
					if (LSTB_ABS(matrix->buf[j]) > max) max = LSTB_ABS(matrix->buf[j]);
				}
				for (j = 0; j < N*N; j++) {
					matrix->buf[j] /= max;
				}
			}

			/* Extract minor eigenvector */
			for (i = 0; i < N; i++) {
				vect->buf[i] = matrix->buf[i];
			}
			FLOATVECT_scale(vect, 1.0/sqrt(FLOATVECT_dot(vect, vect)), vect);

			/* Construct metric */
			matrix->buf[0*2+0] = vect->buf[0];
			matrix->buf[1*2+0] = vect->buf[1];
			matrix->buf[0*2+1] = vect->buf[1];
			matrix->buf[1*2+1] = -vect->buf[0];
			/* Eigenvalues of metric tensor */
			g->buf[0*2+0] = 1.0 / (1.0 + ratio/normalisation);
			/* Threshold ridiculous values */
			if (g->buf[0*2+0] < 0.01) g->buf[0*2+0] = 0.01;
			g->buf[1*2+0] = 0.0;
			g->buf[0*2+1] = 0.0;
			g->buf[1*2+1] = 1.0;
			/* From desired eigen decomposition construct metric tensor */
			FLOATMATRIX_mul(g, matrix, g);
			FLOATMATRIX_transpose(matrix, matrix);
			FLOATMATRIX_mul(g, g, matrix);

			/* Add isotropic (and constant) length-penalty term for regularisation */
			g->buf[0*2+0] += LSTB_SQR(epsilon);
			g->buf[1*2+1] += LSTB_SQR(epsilon);
		}

		metric->a->buf[index] = g->buf[0*2+0];
		metric->b->buf[index] = g->buf[1*2+0];
		metric->c->buf[index] = g->buf[1*2+1];
	}

	/* Free all allocated memory */
	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);
	FLOATVECT_destructor(vect);
	FLOATMATRIX_destructor(matrix);
	FLOATMATRIX_destructor(g);

	/* Free differential images */
	for (d1 = 0; d1 < N; d1++) {
		BIMAGE_destructor(Dvector[d1]);
	}
	free((void *)Dvector);

	for (d1 = 0; d1 < N; d1++) {
	for (d2 = 0; d2 <= d1; d2++) {
		BIMAGE_destructor(Dmatrix[d1 + N*d2]);
	}
	}
	free((void *)Dmatrix);

	return 0;
}

/* BIMAGE_diffusivity:
	Convert a gradient image to a diffusivity image, following Weickert's
	non-linear diffusion via AOS paper
*/
int BIMAGE_diffusivity(
	BIMAGE * in,				/* The input gradient image */
	BIMAGE * out,				/* Output image, may point to input */
	float lambda				/* The threshold for gradients to be significant */
)
{
	int i;
	int num_pixels = BVECT_prod(in->dim);
	float lambda4;

	/* Precalculate the fourth power of lambda */
	lambda4 = lambda*lambda;
	lambda4 = lambda4 * lambda4;

	for (i = 0; i < num_pixels; i++) {
		float s = in->buf[i];
		/* Pre-calculate the 4th power of s, the absolute gradient */
		s = s * s;
		s = s * s;
		out->buf[i] = 1.0 - exp(-3.315 * lambda4 / s);
	}

	return 0;
}


/* BIMAGE_downsample:
	Downsample a BIMAGE by the given integer factor
*/
int BIMAGE_downsample(
	BIMAGE *in,						/* Input image to downsample */
	BIMAGE *out,					/* Output image (already allocated) */
	int downsampling_rate,			/* Downsampling rate for each axis */
	char blur_flag					/* Do I blur for anti-aliasing? */
) {
	int in_index, out_index, i;
	int num_pixels_in = BVECT_prod(in->dim);
	int num_pixels_out = BVECT_prod(out->dim);
	BVECT * coord, * temp_coord;

	coord = BVECT_constructor(in->dim->length);
	temp_coord = BVECT_constructor(in->dim->length);

	if (blur_flag) {
		int scaling;

		/* For each input pixel, compute the corresponding output
			pixel coordinate, accumulate there and average later */
		BVECT_zero(coord);
		for (in_index = 0; in_index < num_pixels_in; in_index++, BVECT_inc(coord, in->dim)) {
			char bail_out = LSTB_FALSE;

			BVECT_copy(temp_coord, coord);

			/* Scale coordinate to obtain input pixel */
			for (i = 0; i < in->dim->length; i++) {
				temp_coord->buf[i] /= downsampling_rate;
				if (temp_coord->buf[i] > out->dim->buf[i] - 1) {
					bail_out = LSTB_TRUE;
				}
			}
			if (bail_out) continue;

			/* Convert to output index */
			out_index = bvectToInt(temp_coord, out->dim);
			out->buf[out_index] += in->buf[in_index];
		}

		/* Compute normalisation factor */
		scaling = 1;
		for (i = 0; i < in->dim->length; i++)
			scaling *= downsampling_rate;

		/* Normalise */
		for (out_index = 0; out_index < num_pixels_out; out_index++) {
			out->buf[out_index] /= scaling;
		}
	} else {
		/* For each output pixel, compute the corresponding input
			pixel coordinate and sample */
		BVECT_zero(coord);
		for (out_index = 0; out_index < num_pixels_out; out_index++, BVECT_inc(coord, out->dim)) {
			BVECT_copy(temp_coord, coord);
			/* Scale to obtain input pixel */
			for (i = 0; i < out->dim->length; i++) {
				temp_coord->buf[i] *= downsampling_rate;
			}

			/* Convert to input index */
			in_index = bvectToInt(temp_coord, in->dim);
			out->buf[out_index] = in->buf[in_index];
		}
	}

	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);

	return 0;
}


/* label_image_downsample:
	Downsample a label image by the given integer factor
	0 is background, all other labels take precedence
*/
int label_image_downsample(
	char * in,						/* Input image to downsample */
	BVECT * in_dim,					/* Input dimensions */
	char * out,						/* Output image (already allocated) */
	BVECT * out_dim,				/* Output dimensions */
	int downsampling_rate			/* Downsampling rate for each axis */
)
{
	int in_index, out_index, i;
	int num_pixels_in = BVECT_prod(in_dim);
	int num_pixels_out = BVECT_prod(out_dim);
	BVECT * coord, * temp_coord;

	coord = BVECT_constructor(in_dim->length);
	temp_coord = BVECT_constructor(in_dim->length);

	/* Output defaults to 0 */
	memset(out, 0, num_pixels_out * sizeof(char));

	/* For each input pixel accumulate to output */
	for (in_index = 0, BVECT_zero(coord); in_index < num_pixels_in; in_index++, BVECT_inc(coord, in_dim)) {
		char bail_out;

		BVECT_copy(temp_coord, coord);

		/* Scale coordinate to obtain input pixel */
		bail_out = LSTB_FALSE;
		for (i = 0; i < in_dim->length; i++) {
			temp_coord->buf[i] /= downsampling_rate;
			if (temp_coord->buf[i] > out_dim->buf[i] - 1) {
				bail_out = LSTB_TRUE;
			}
		}
		if (bail_out) continue;

		/* Convert to output index */
		out_index = bvectToInt(temp_coord, out_dim);
		if (in[in_index] != 0) out[out_index] = in[in_index];
	}

	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);

	return 0;
}


/* BIMAGE_upsample:
	Upsample a BIMAGE by the given integer factor
*/
int BIMAGE_upsample(
	BIMAGE *in,						/* Input image to upsample */
	BIMAGE *out,					/* Output image (already allocated) */
	int upsampling_rate,			/* Upsampling rate for each axis */
	char blur_flag					/* Interpolate? */
) {
	int in_index, out_index, i;
	int num_pixels_in = BVECT_prod(in->dim);
	int num_pixels_out = BVECT_prod(out->dim);
	BVECT * coord, * temp_coord;

	/* Create coordinate structures */
	coord = BVECT_constructor(in->dim->length);
	temp_coord = BVECT_constructor(in->dim->length);

	if (blur_flag) {
		int dimension;
		int row_index;
		BVECT * row;
		BVECT * row_dim;
		int stride;
		float * buf1, * buf2;
		int max_dim;

		/* Allocate memory */
		max_dim = BVECT_max(out->dim);
		buf1 = (float *)malloc(max_dim*sizeof(float));
		buf2 = (float *)malloc(max_dim*sizeof(float));
		row = BVECT_constructor(out->dim->length - 1);
		row_dim = BVECT_constructor(out->dim->length - 1);

		/* Copy input to subspace of output */
		BVECT_zero(coord);
		for (in_index = 0; in_index < num_pixels_in; in_index++, BVECT_inc(coord, in->dim)) {
			/* Convert between the two coordinate systems */
			out_index = bvectToInt(coord, out->dim);
			out->buf[out_index] = in->buf[in_index];
		}

		/* Upsample in place along each axis successively */
		for (dimension = 0; dimension < out->dim->length; dimension++) {
			int num_rows;

			/* Determine the dimensions of the current face of the hyperslab */
			for (i = 0; i < row_dim->length; i++) {
				if (i < dimension) {
					row_dim->buf[i] = out->dim->buf[i];
				} else {
					row_dim->buf[i] = out->dim->buf[i+1];
				}
			}

			/* Determine the stride along this dimension in the image */
			stride = 1;
			for (i = 0; i < dimension; i++) {
				stride *= out->dim->buf[i];
			}

			/* For every row along this dimension */
			num_rows = BVECT_prod(row_dim);
			BVECT_zero(row);
			for (row_index = 0; row_index < num_rows; row_index++, BVECT_inc(row, row_dim)) {
				int row_base;

				/* Determine the index of the base of this row in the output image */
				for (i = 0; i < row_dim->length; i++) {
					if (i < dimension) {
						coord->buf[i] = row->buf[i];
					} else {
						coord->buf[i+1] = row->buf[i];
					}
				}
				coord->buf[dimension] = 0;
				row_base = bvectToInt(coord, out->dim);

				/* Copy this row into a temporary buffer */
				for (i = 0; i < in->dim->buf[dimension]; i++) {
					buf1[i] = out->buf[row_base + i*stride];
				}

				/* Upsample this buffer */
				upsample_row(buf1, in->dim->buf[dimension], buf2, out->dim->buf[dimension], upsampling_rate);

				/* Copy the upsampled row back */
				for (i = 0; i < out->dim->buf[dimension]; i++) {
					out->buf[row_base + i*stride] = buf2[i];
				}
			}
		}

		/* Free all allocated memory */
		free((void *)buf1);
		free((void *)buf2);
		BVECT_destructor(row);
		BVECT_destructor(row_dim);
	} else {
		/* For each output pixel, compute the corresponding input
			pixel coordinate and nearest-neighbour sample */
		BVECT_zero(coord);
		for (out_index = 0; out_index < num_pixels_out; out_index++, BVECT_inc(coord, out->dim)) {
			BVECT_copy(temp_coord, coord);

			/* Scale to obtain input pixel */
			for (i = 0; i < out->dim->length; i++) {
				temp_coord->buf[i] /= upsampling_rate;
			}

			/* Convert to input index */
			in_index = bvectToInt(temp_coord, in->dim);
			out->buf[out_index] = in->buf[in_index];
		}
	}

	BVECT_destructor(coord);
	BVECT_destructor(temp_coord);

	return 0;
}

/* upsample_row:
	Given an input row, stride, length, output row, upsampling_rate,
	downsample from in to out
*/
static int upsample_row(
	float *in,						/* Input row to upsample */
	int in_length,					/* Length of input row */
	float *out,						/* Output row */
	int out_length,					/* Length of output row */
	int upsampling_rate				/* Upsampling rate */
) {
	int out_index;

	/* Upsample a single row */
	for (out_index = 0; out_index < out_length; out_index++) {
		float t, in_index;
		int floor_index, ceil_index;

		/* Compute input index for output point */
		in_index = (float)out_index / upsampling_rate;
		in_index -= 0.25;

		/* Compute integer and fractional components */
		floor_index = (int)in_index;
		if (floor_index < 0) {
			out[out_index] = in[0];
			continue;
		}
		ceil_index = floor_index + 1;
		if (ceil_index > in_length - 1) {
			out[out_index] = in[in_length - 1];
			continue;
		}
		t = in_index - floor_index;

		out[out_index] = in[floor_index]*(1.0 - t) + in[ceil_index]*t;
	}

	return 0;
}
