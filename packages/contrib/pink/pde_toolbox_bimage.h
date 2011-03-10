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
 * File:		pde_toolbox_bimage.h
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
 bimage.h
 ------

  DESCRIPTION:
  Definition of the BIMAGE (Ben's image) data structure.  Not particularly general, simply
  arbitary-dimensional.  Should be trivial to wrap into image processing systems.

  HISTORY:
  Created by Ben Appleton (5/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#ifndef BIMAGE_H
#define BIMAGE_H

/* For simplifying some of the code, we'll fix the maximum number of dimensions */
#define MAXDIMS 32

/********************************* Data Structures *********************************/
/* The structures used to represent N-D images */
typedef struct {
	int *buf;			/* The vector */
	int length;			/* The length of the vector */
} BVECT;

typedef struct {
	float *buf;			/* The buffer in [x + nx*(y + ny*(z + ...))] format */
	BVECT *dim;			/* The dimensions of the image */
	float *steps;			/* The physical size of each step in each axis */
} BIMAGE;

/* 2D metric tensor (spatially varying) */
typedef struct {
	BIMAGE * a;				/* SPD metric field in the form */
	BIMAGE * b;				/* [a b] */
	BIMAGE * c;				/* [b c] */
	BIMAGE * local_aniso_ratio;		/* anisotropy ratio of this point's metric */
	BIMAGE * regional_aniso_ratio;	/* anisotropy ratio of largest regional neighbour */
} METRIC2D;

/********************************* Function Prototypes *********************************/
/* Constructor and destructor for BVECT 'class' */
BVECT * BVECT_constructor(
	int length					/* The length of the BVECT */
);
void BVECT_destructor(BVECT * bvect);

/* Utility functions for BVECTs */
int bvectToInt(
	BVECT *coord, 					/* The coordinate to convert */
	BVECT *dim);					/* The dimensions (conversion parameters) */

int intToBvect(
	int index,					/* The input index */
	BVECT *pcoord,					/* The output coordinate */
	BVECT *dim);					/* The dimensions (conversion parameters) */

int BVECT_prod(BVECT *in);					/* Return the product of the vector */
int prod(BVECT *in);						/* Return the product of the vector */
int BVECT_sum(BVECT *in);					/* Return the sum of the vector */
float BVECT_sum_sqr(BVECT * in);				/* Compute the sum of squares */
int BVECT_add(BVECT *dest, BVECT *source);			/* dest += source */
int BVECT_sub(BVECT *dest, BVECT *source);			/* dest -= source */
int BVECT_inc(BVECT * in, BVECT * dim);				/* Increment image index */
int BVECT_dec(BVECT * in, BVECT * dim);				/* Decrement image index */
int BVECT_zero(BVECT * in);							/* Set to 0 vector */
int BVECT_copy(BVECT * dest, BVECT * source);		/* dest = source by value */
char BVECT_compare(BVECT * a, BVECT * b);			/* Return a > b */
int BVECT_print(BVECT * in);					/* Print a BVECT */
int BVECT_max(BVECT * in);						/* Return maximum component */
int BVECT_min(BVECT * in);						/* Return minimum component */

/* Constructors and destructor for BIMAGE 'class' */
BIMAGE * BIMAGE_constructor(
	BVECT * dim 					/* The requested dimensions */
);
BIMAGE * BIMAGE_constructor_BIMAGE(
	BIMAGE * in 					/* The requested dimensions */
);
BIMAGE * BIMAGE_constructor_double(
	double *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
);
BIMAGE * BIMAGE_constructor_float(
	float *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
);
BIMAGE * BIMAGE_constructor_int(
	int *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
);
BIMAGE * BIMAGE_constructor_char(
	char *in,					/* Input image to copy */
	BVECT * dim 					/* The requested dimensions */
);
void BIMAGE_destructor(BIMAGE * bimage);

/* Preprocessing functions for BIMAGEs */
/* Blur in to out, in == out ok */
int BIMAGE_abs_grad(BIMAGE * in, BIMAGE *out);

/* Radial gradient given centre point */
int BIMAGE_rad_grad(
	BIMAGE * in,
	BIMAGE *out,
	BVECT * centre
);

/* Weight the metric radially for scale invariance (*= 1/r^(N-1), N the number of dimensions) */
void BIMAGE_radial_weighting(
	BIMAGE * g,					/* The original metric (overwrite) */
	BVECT * source				/* The coordinates of the source */
);

/* Gaussian blur by [1 2 1]/4 'blurring' times along each axis */
int BIMAGE_blur(
	BIMAGE * in,
	BIMAGE * out,
	int blurring
);

/* Blur an image with an implicit (IIR-like) Gaussian, using an AOS
	implementation of linear diffusion.
*/
int BIMAGE_aosblur(
	BIMAGE *in,						/* The image data to blur */
	BIMAGE *out,					/* The output image.  May point to in */
	float blurring 				/* The time over which the PDE is run */
);

/* Image IIR filtering with reflective boundary conditions */
int iirsymconv(
	double *in,						/* The image data to blur */
	double *out,					/* The output image.  May point to in */
	BVECT * dim,					/* Image dimensions */
	int dimension,					/* The dimension along which to apply the filter */
	double * numerator,				/* The symmetric, odd-length numerator coefficient vectors */
	int numerator_length,			/* The (odd!) number of numerator coefficients */
	char symmetric,					/* Is the numerator symmetric (true) or asymmetric (false) ? */
	double * ldl_matrix,			/* The LDL decomposition of the denominator */
	int denominator_length			/* The (odd!) number of denominator coefficients */
);

/* Batch LDL decomposition for filtering with reflective boundary conditions */
int LDLbatchdecomp(
	double * denominator,			/* The symmetric denominator to decompose */
	int denominator_length,			/* The length of the denominator */
	double * ldl_matrix,			/* The decomposition (preallocated) */
	int n,							/* The image dimensions */
	double * old_ldl_matrix,		/* Old LDL decomposition for head-start (or NULL) */
	int old_n
);

/* Deriche's constant-time-per-pixel Gaussian blur, extended to reflective boundaries */
int conderichef(
	double * numerator,						/* The numerator of the filter */
	double * denominator,					/* The denominator of the filter */
	double sigma,							/* The desired scale of the Gaussian */
	int filter_order,						/* The order of the filter approximation */
	int derivative_order					/* The order of the derivative */
);
int gaussblur(
	double *in,						/* The image data to blur */
	double *out,					/* The output image.  May point to in */
	BVECT * dim,					/* The image dimensions */
	double * sigma_vect,			/* The scale parameter for each axis */
	int order						/* The order of Gaussian approximation [2, 4] */
);

#ifdef COMMENT
int gaussblurGMP(
	double *in,						/* The image data to blur */
	double *out,					/* The output image.  May point to in */
	BVECT * dim,					/* The image dimensions */
	double * sigma_vect,			/* The scale parameter for each axis */
	int order						/* The order of Gaussian approximation [2, 4] */
);
#endif

int gradgauss(
	double *in,						/* The image data */
	double * * out,					/* The output image.  For single-component may point to in */
	BVECT * dim,					/* The image dimensions */
	double * sigma_vect,			/* The scale parameter for each axis */
	int order,						/* The order of Gaussian approximation [2, 4] */
	char multi_comp					/* Return multi-component gradient? If not, take 2-norm of gradient vector */
);
int lapgauss(
	double *in,						/* The image data */
	double *out,					/* The output image.  May point to in */
	BVECT * dim,					/* The image dimensions */
	double * sigma_vect,			/* The scale parameter for each axis */
	int order						/* The order of Gaussian approximation [2, 4] */
);


/* Convert a gradient image to a metric image by 1/(1+|grad|^p) */
int BIMAGE_metrify(
	BIMAGE * in,				/* The input gradient image */
	BIMAGE * out,				/* Output image, may point to input */
	float normalisation,		/* Normalisation factor */
	int p,						/* Should be 1 or 2 */
	float epsilon				/* Length penalty weight */
);

/* Create a tensor metric from a blurred image.  For segmentation. */
int BIMAGE_tensor_metric(
	BIMAGE * in,				/* Input image (pre blurred, but not grad'd) */
	METRIC2D * metric,			/* Output metric */
	float normalisation,		/* Normalisation factor */
	int p,						/* Should be 1 or 2 */
	float epsilon				/* Length penalty weight */
);

/* Create a tensor metric from a blurred line image. */
int BIMAGE_line_metric(
	BIMAGE * in,				/* Input image (pre blurred, but not grad'd) */
	METRIC2D * metric,			/* Output metric */
	float normalisation,		/* Normalisation factor */
	float epsilon				/* Euclidean length penalty */
);

/* Convert a gradient image to a diffusivity image */
int BIMAGE_diffusivity(
	BIMAGE * in,				/* The input gradient image */
	BIMAGE * out,				/* Output image, may point to input */
	float lambda				/* The threshold for gradients to be significant */
);

/** Weickert et. al.'s AOS diffusion scheme **/
/* Constant (w.r.t. time) diffusivities */
int aosdiffuse(
	BIMAGE *image,					/* The image data to smooth */
	BIMAGE *g,						/* The scalar diffusivity image */
	float simulation_time, 			/* The time over which the PDE is run */
	float timestep 					/* The iteration time step */
);
/* Time-varying diffusivities */
int aosdiffuse2(
	BIMAGE *image,					/* The image data to diffuse */
	float blurring,					/* The scale of blurring to find edges */
	float lambda,					/* The threshold for edge significance */
	float simulation_time, 			/* The time over which the PDE is run */
	float timestep 					/* The iteration time step */
);

/* General reaction-diffusion image restoration (isotropic) */
int reactdiffuse(
	BIMAGE *inimage,				/* The scalar field to diffuse */
	BIMAGE *image,					/* The output scalar field */
	BIMAGE *g,						/* The scalar diffusivity image */
	BIMAGE *alpha,					/* The confidence image (non-negative!) */
	float simulation_time, 			/* The time over which the PDE is run */
	float timestep 					/* The iteration time step */
);

/* Stereo inpainting by a reaction-diffusion equation */
int stereoinpaint(
	BIMAGE *inimage,				/* The scalar field to diffuse */
	BIMAGE *image,					/* The output scalar field */
	BIMAGE *g,						/* The scalar diffusivity image */
	BIMAGE *alpha,					/* The confidence image (non-negative!) */
	float simulation_time, 			/* The time over which the PDE is run */
	float timestep 					/* The iteration time step */
);

/* Image downsampling and upsampling.  Requires both images allocated with correct
dimensions (out->dim = floor(in->dim / or *  downsampling_rate))
*/
int BIMAGE_downsample(
	BIMAGE *in,						/* Input image to downsample */
	BIMAGE *out,					/* Output image (already allocated) */
	int downsampling_rate,			/* Downsampling rate for each axis */
	char blur_flag					/* Do I blur for anti-aliasing? */
);

int label_image_downsample(
	char * in,						/* Input image to downsample */
	BVECT * in_dim,					/* Input dimensions */
	char * out,						/* Output image (already allocated) */
	BVECT * out_dim,				/* Output dimensions */
	int downsampling_rate			/* Downsampling rate for each axis */
);

int BIMAGE_upsample(
	BIMAGE *in,						/* Input image to upsample */
	BIMAGE *out,					/* Output image (already allocated) */
	int upsampling_rate,			/* Upsampling rate for each axis */
	char blur_flag					/* Interpolate? */

);

#endif

