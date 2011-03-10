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
 * File:		pde_toolbox.h
 *
 * Written by:		Image Analysis Group staff,
 * 			CSIRO Mathematical and Information Sciences.
 *
 * Date:		December 2002
 *
 *
 * CSIRO Mathematical and Information Sciences is the owner of all
 * copyright subsisting in the software contained in this file. It may
 * not be disclosed to, or used by, anyone without the prior approval
 * of the Chief, CSIRO Mathematical and Information Sciences.
 *
*/

/**********************************************************
 pde_toolbox.h
 -----------

 LIAR level include for the level set toolbox.

**********************************************************/

#ifndef PDE_TOOLBOX_H
#define PDE_TOOLBOX_H

#include "pde_toolbox_bimage.h"
#include "pde_toolbox_LSTB.h"
#include "pde_toolbox_defs.h"

int llsinitcontour(
	char *in,						/* The desired contours */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	int **source_list,				/* List of interior points to flood fill contours  */
	int num_sources,				/* Number of interior points */
	DBL_TYPE *out					/* Put the signed distance function here */
);

int llsinitsphere(
	DBL_TYPE *out,					/* Put the signed distance function here */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE radius,				/* Radius of sphere */
	int * centre_buf			/* Coordinate of centre of sphere */
);

int llsinitdirect(
	DBL_TYPE * in,					/* The initial (manually constructed) level sets */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE *out					/* The cleaned level sets */
);

int llsinitspherepacking(
	DBL_TYPE *out,					/* Put the signed distance function here */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	int spacing						/* The spacing between spheres */
);

int llsinitborder(
	DBL_TYPE *out,					/* Put the signed distance function here */
	int * dim_buf,					/* The image dimensions */
	int dim_length					/* The number of image dimesions */
);

int llsaosfast(
	/* Input image */
	DBL_TYPE *in,
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Level set creation parameters */
	DBL_TYPE radius,				/* Radius of sphere */
	int * centre_buf,				/* Centre coordinate of sphere */
	/* Metric creation parameters */
	DBL_TYPE epsilon,				/* Weak regularity constraint = length penalty coeff. */
	int blurring,					/* The scale of blurring used to compute edges */
	/* Evolution parameters */
	DBL_TYPE alpha,					/* Deflation coefficient (negative for inflation) */
	DBL_TYPE initial_time,			/* Simulation time at first scale */
	DBL_TYPE settling_time,			/* Simulation time at subsequent scales */
	DBL_TYPE timestep,				/* Iteration timestep */
	/* Number of scales (>=1) for multiscale */
	int num_scales,
	/* Output image AND format */
	DBL_TYPE *out,
	int output_format
);

int llsaossimple(
	/* Input metric image */
	DBL_TYPE *metric_buf,			/* Metric image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Initial level sets */
	DBL_TYPE *ls_buf,
	/* Evolution parameters */
	DBL_TYPE alpha,					/* Deflation coefficient (negative for inflation) */
	DBL_TYPE initial_time,			/* Simulation time at first scale */
	DBL_TYPE settling_time,			/* Simulation time at subsequent scales */
	DBL_TYPE timestep,				/* Iteration timestep */
	/* Number of scales (>=1) for multiscale */
	int num_scales,
	/* Output image AND format */
	DBL_TYPE *out,
	int output_format
);

int llslvlsimple(
	/* Input image */
	void * image_buf,				/* Image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Initial level sets */
	DBL_TYPE *ls_buf,
	/* Evolution parameters */
	int speed_func,					/* Which speed function to use */
	DBL_TYPE param1,				/* First parameter passed to speed function */
	DBL_TYPE param2,				/* Second parameter passed to speed function */
	DBL_TYPE seg_time,				/* Simulation time */
	/* Output image and format */
	DBL_TYPE *out,
	int output_format
);

/* Wrappers for the BIMAGE utility functions */
int lblur(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	int blurring,					/* The amount of blurring to perform */
	DBL_TYPE *out					/* The blurred output image */
);

int laosblur(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE blurring,					/* The amount of blurring to perform */
	DBL_TYPE *out					/* The blurred output image */
);

int labs_grad(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE *out					/* The blurred output image */
);

int lrad_grad(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	int * centre_buf,				/* Centre for radial gradient */
	DBL_TYPE *out					/* The gradient image */
);

int lradial_weighting(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	int * centre_buf,				/* The centre point */
	DBL_TYPE * out					/* The output image */
);

int lmetrify(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE normalisation,			/* The intensity normalisation factor */
	int p,							/* The power applied to the gradient values */
	DBL_TYPE epsilon,				/* Constant length-penalty coefficient */
	DBL_TYPE *out					/* The gradient image */
);

int ltensor_metric(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE normalisation,			/* The intensity normalisation factor */
	int p,							/* The power applied to the gradient values */
	DBL_TYPE epsilon,				/* Constant length-penalty coefficient */
	DBL_TYPE *abuf,					/* The SPD metric field g = */
	DBL_TYPE *bbuf,					/* [a b] */
	DBL_TYPE *cbuf					/* [b c] */
);

int lline_metric(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE normalisation,			/* Normalisation factor */
	DBL_TYPE epsilon,				/* Euclidean length penalty */
	DBL_TYPE *abuf,					/* The SPD metric field g = */
	DBL_TYPE *bbuf,					/* [a b] */
	DBL_TYPE *cbuf					/* [b c] */
);

int ldiffusivity(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE lambda,				/* Soft threshold for gradient significance */
	DBL_TYPE *out					/* The diffusivity image */
);

int laosdiffuse(
	/* Input image */
	DBL_TYPE *image_buf,			/* Image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Diffusivity map */
	DBL_TYPE *g_buf,
	/* Evolution parameters */
	DBL_TYPE simulation_time,		/* Simulation time */
	DBL_TYPE time_step,				/* Iteration timestep */
	/* Output image */
	DBL_TYPE *out
);

int laosdiffuse2(
	/* Input image */
	DBL_TYPE *image_buf,			/* Image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Evolution parameters */
	DBL_TYPE blurring,				/* The scale of blurring to find edges */
	DBL_TYPE lambda,				/* The threshold for edge significance */
	DBL_TYPE simulation_time,		/* Simulation time */
	DBL_TYPE time_step,				/* Iteration timestep */
	/* Output image */
	DBL_TYPE *out
);

int lreactdiffuse(
	/* Input image */
	DBL_TYPE *image_buf,			/* Image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Diffusivity map */
	DBL_TYPE *g_buf,
	/* Confidence map (non-negative) */
	DBL_TYPE *a_buf,
	/* Evolution parameters */
	DBL_TYPE simulation_time,		/* Simulation time */
	DBL_TYPE time_step,				/* Iteration timestep */
	/* Output image */
	DBL_TYPE *out
);

int lstereoinpaint(
	/* Input image */
	DBL_TYPE *image_buf,			/* Image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Diffusivity map */
	DBL_TYPE *g_buf,
	/* Confidence map (non-negative) */
	DBL_TYPE *a_buf,
	/* Evolution parameters */
	DBL_TYPE simulation_time,		/* Simulation time */
	DBL_TYPE time_step,				/* Iteration timestep */
	/* Output image */
	DBL_TYPE *out
);

int lfmmdist(
    /* Input image */
    INT4_TYPE * seed_in_buf,			/* Seeds from which to grow distance function */
    INT4_TYPE * seed_out_buf,		/* Result of growing seeds - may point to seed_in_buf */
    int32_t * dim_buf,					/* The image dimensions */
    int32_t dim_length,					/* The number of image dimesions */
    /* Metric image */
    DBL_TYPE * g_buf,
    /* Halting criteria */
    const char stopping,			/* The type of halting criteria */
    const float threshold,			/* The halting threshold */
    /* Output image */
    DBL_TYPE * distance_buf
);

int ltensordist(
	/* Input image */
	DBL_TYPE * seed_in_buf,			/* Seeds from which to grow distance function */
	DBL_TYPE * seed_out_buf,		/* Result of growing seeds - may point to seed_in_buf */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Metric components */
	DBL_TYPE * a_buf,				/* metric tensor = */
	DBL_TYPE * b_buf,				/* [a b] */
	DBL_TYPE * c_buf,				/* [b c] */
	/* Halting criteria */
	const char stopping,			/* The type of halting criteria */
	const float threshold,			/* The halting threshold */
	/* Output image */
	DBL_TYPE * distance_buf,
	DBL_TYPE * backpointers_DBL		/* DBL_TYPE representation of backpointers */
);

int lgeodesic(
	/* Distance image */
	DBL_TYPE * distance_buf,		/* The distance function buffer */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Starting point for descent */
	int * start_buf,
	/* Output contour image */
	DBL_TYPE * contour_buf
);

int ltensorgeodesic(
	/* Distance image */
	DBL_TYPE * distance_buf,		/* The distance function buffer */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Metric (2D) */
	DBL_TYPE * abuf,				/* g = */
	DBL_TYPE * bbuf,				/* [a b] */
	DBL_TYPE * cbuf,				/* [b c] */
	/* Network backpointers for when gradient descent fails */
	DBL_TYPE * backpointers_DBL,
	/* Starting point for descent */
	int * start_buf,
	/* Output contour image */
	DBL_TYPE * contour_buf
);

int lgogac(
	DBL_TYPE *metric,			/* Input metric image (2D) */
	int nx, int ny,				/* Image dimensions */
	int xCentre, int yCentre,	/* Centre of cut */
	DBL_TYPE *contour			/* Output contour image */
);

int lgogacw(
	DBL_TYPE *metric,			/* Input metric image (2D) */
	DBL_TYPE *weighting,		/* Weighting function (to *divide* metric) */
	int nx, int ny,				/* Image dimensions */
	int xCentre, int yCentre,	/* Centre of cut */
	DBL_TYPE *contour			/* Output contour image */
);

int loumgac(
	DBL_TYPE * a_buf,			/* Input metric image (2D) */
	DBL_TYPE * b_buf,			/* [a b] */
	DBL_TYPE * c_buf,			/* [b c] */
	int nx, int ny,				/* Image dimensions */
	int xCentre, int yCentre,	/* Centre of cut */
	DBL_TYPE *contour			/* Output contour image */
);

int lreadLSTBmsgs(void);

int lconderichef(
	DBL_TYPE * numerator,			/* The numerator coefficients.  Indexing is centred on 0 */
	DBL_TYPE * denominator,			/* The denominator coefficients.  Indexing is centred on 0 */
	DBL_TYPE sigma,					/* The scale parameter */
	int filter_order,				/* The order of the (one-sided) filter */
	int derivative_order			/* The derivative order (0, 1, 2) */
);

int lgaussblur(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE *out,					/* The blurred output image */
	DBL_TYPE *sigma_vect,			/* Scales of Gaussian */
	int order						/* Order of approximation [2, 4] */
);

#ifdef COMMENT
int lgaussblurGMP(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE *out,					/* The blurred output image */
	DBL_TYPE *sigma_vect,			/* Scales of Gaussian */
	int order						/* Order of approximation [2, 4] */
);
#endif

int lgradgauss(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE * * out,				/* The blurred output image.  Indexed as out[component][pixel] */
	DBL_TYPE *sigma_vect,			/* Scales of Gaussian */
	int order,						/* Order of approximation [2, 4] */
	char multi_comp					/* Return multi-component gradient? If not, take 2-norm of gradient vector */
);
int llapgauss(
	DBL_TYPE * in,					/* The input image */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	DBL_TYPE *out,					/* The blurred output image */
	DBL_TYPE *sigma_vect,			/* Scales of Gaussian */
	int order						/* Order of approximation [2, 4] */
);

/* Experimental code */
int letals(
	/* Input image */
	DBL_TYPE *image_buf,			/* Image data */
	int * dim_buf,					/* The image dimensions */
	int dim_length,					/* The number of image dimesions */
	/* Initial level sets */
	DBL_TYPE *ls_buf,
	/* Evolution parameters */
	int speed_func,					/* Which speed function to use */
	DBL_TYPE param1,				/* First parameter passed to speed function */
	DBL_TYPE param2,				/* Second parameter passed to speed function */
	DBL_TYPE seg_time,				/* Simulation time */
	/* Output image and format */
	DBL_TYPE *out,
	int output_format
);


int lcontmaxflow(
	DBL_TYPE * g,				/* The isotropic but non-homogeneous metric */
	int * dim_buf,				/* The image dimensions */
	int dim_length,
	DBL_TYPE * dbl_type,		/* Label image of node types */
	DBL_TYPE initial_time,		/* Simulation time at first scale */
	DBL_TYPE settling_time,		/* Simulation time at subsequent scales */
	DBL_TYPE * out,				/* The output */
	int num_scales,				/* The number of scales (>1 for multiscale) */
	pde_hook_func * cbf,		/* callback function e.g for display, can be NULL */
	DBL_TYPE * dbgP,			/* debug pressure (can be NULL) */
	DBL_TYPE * dbgFx,			/* debug velocity components (can be NULL) */
	DBL_TYPE * dbgFy,
	DBL_TYPE * dbgFz,
	int period,					/* frequency of call back (every N step...) */
	int num_threads
    );

int lmaxflow(
	DBL_TYPE * g,				/* The isotropic but non-homogeneous metric */
	int * dim_buf,				/* The image dimensions */
	int dim_length,				/* The number of image dimesions */
	DBL_TYPE * dbl_type,		/* Label image of node types */
	DBL_TYPE * out				/* The output */
);

#endif /* PDE_TOOLBOX_H */
