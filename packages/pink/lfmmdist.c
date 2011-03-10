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
 * File:		lfmmdist.c
 *
 * Written by:		Image Analysis Group staff,
 * 			CSIRO Mathematical and Information Sciences.
 *
 * Date:		November 2002
 *
 * CSIRO Mathematical and Information Sciences is the owner of all
 * copyright subsisting in the software contained in this file. It may
 * not be disclosed to, or used by, anyone without the prior approval
 * of the Chief, CSIRO Mathematical and Information Sciences.
 *
*/

/*********************************************************************************************
 lfmmdist.c
 ------

  DESCRIPTION:
  A wrapper for geodist, which computes geodesic distance transforms from multiple seeds.

  HISTORY:
  Created by Ben Appleton (28/11/02)
  Contact: appleton@itee.uq.edu.au

  Modified for Pink by Hugues Talbot	 7 Jul 2009
**********************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pde_toolbox.h"
#include "lfmm.h"

int lfmmdist(
	/* Input image */
	INT4_TYPE  * seed_in_buf,			/* Seeds from which to grow distance function */
	INT4_TYPE  * seed_out_buf,		/* Result of growing seeds - may point to seed_in_buf */
	int32_t * dim_buf,					/* The image dimensions */
	int32_t   dim_length,					/* The number of image dimesions */
	/* Metric image */
	DBL_TYPE    * g_buf,
	/* Halting criteria */
	const char stopping,			/* The type of halting criteria */
	const float threshold,			/* The halting threshold */
	/* Output image */
	DBL_TYPE * distance_buf
)
{
	BVECT * dim;
	BIMAGE * seeds, * g, * distance;
	int i;
	int num_pixels;

	/* Set up a BVECT to describe the image dimensions */
	dim = BVECT_constructor(dim_length);
	memcpy(dim->buf, dim_buf, dim_length*sizeof(int));
	num_pixels = BVECT_prod(dim);

	/* Create the BIMAGEs from the image data */
	seeds = BIMAGE_constructor_int((int32_t*)seed_in_buf, dim);
	g = BIMAGE_constructor_float(g_buf, dim);
	distance = BIMAGE_constructor(dim);

	/* Call the geodist function from the level set toolbox */
	geodist(seeds, g, stopping, threshold, distance);

	/* Display any LSTB_error or LSTB_debug messages */
	lreadLSTBmsgs();

	/* Convert back to expected output */
	for (i = 0; i < num_pixels; i++) {
		distance_buf[i] = (DBL_TYPE)distance->buf[i];
		seed_out_buf[i] = (INT4_TYPE)seeds->buf[i];
	}

	/* Free everything */
	BVECT_destructor(dim);
	BIMAGE_destructor(seeds);
	BIMAGE_destructor(g);
	BIMAGE_destructor(distance);

	return 0;
}

