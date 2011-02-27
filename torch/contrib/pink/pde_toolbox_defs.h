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
 * File:		pde_toolbox_defs.h
 *
 * Written by:		Ben Appleton
 * 			ITEE, The University of Queensland
 *
 * Date:		October 2002
 *
*/

/*********************************************************************************************
 pde_defs.h
 ------

  DESCRIPTION:
  Definitions for pde functions.

  HISTORY:
  Created by Ben Appleton (10/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#ifndef PDE_DEFS_H
#define PDE_DEFS_H

#include <stdint.h>

/* these can change */
//typedef  uint32_t  INT4_TYPE; !!! CHANGED by MC nov. 16, 2009
typedef  int32_t  INT4_TYPE;
typedef  float    DBL_TYPE;

/* My defines, used by my pde functions */
#ifndef LSTB_SIGN
#define LSTB_SIGN(X) (((X) > 0) ? 1 : ( ((X) < 0) ? -1 : 0 ))
#endif

#ifndef LSTB_ABS
#define LSTB_ABS(X) (((X) >= 0) ? (X) : -(X))
#endif

#ifndef LSTB_SQR
#define LSTB_SQR(X) ((X)*(X))
#endif

#ifndef LSTB_MIN
#define LSTB_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#ifndef LSTB_MAX
#define LSTB_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif

#define LSTB_TRUE -1
#define LSTB_FALSE 0

#define LSTB_BIGNUM 1000000
#define LSTB_SMALLNUM 0.000001

/* Define the different level set output types  */
#define LSDIRECT -1						/* Copy level set data directly, no reinitialisation */
#define LSFULL 0						/* Perform a full reinitialisation (slow!) */
#define LSREGION 1						/* Output the internal region */
#define LSCONTOUR 2						/* Output the contour only (fast!) */

/* Define the different speed functions */
#define SETHIAN_SPEED_FUNC 0			/* The speed function from Sethian's book */
#define GAC_SPEED_FUNC 1				/* The speed function for Geodesic Active Contours */
#define GVF_SPEED_FUNC 2				/* The speed function used for Gradient Vector Flows */

/* Stopping criteria */
#define NOSTOPPING 0
#define STOPONMETRIC 1
#define STOPONDISTANCE 2

/* Define how far from the border to place the box level set in init_LS_border */
#define INIT_LS_BORDER_DIST 5.0

#define IM_AOS_BLUR_MAX_TIME_STEP 20.0

/* Bits used to store vertex types in maximum flow algorithm */
#define MAXFLOW_SINK -1
#define MAXFLOW_NORMAL 0
#define MAXFLOW_SOURCE 1


/* TEST CODE
	Hard coded weightings for directions
	For use when data dimensions are not directly comparable
*/
/* #define USE_DIRECTION_WEIGHTINGS */
#ifdef USE_DIRECTION_WEIGHTINGS
static const float direction_weighting[] = {1.0f, 1.0f, 1.0f, 1.0f};
#endif

#endif
