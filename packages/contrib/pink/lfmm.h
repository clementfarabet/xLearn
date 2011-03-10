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
 * File:		FMM.h
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
 FMM.h
 ------

  DESCRIPTION:
  Definitions for code relating to Sethian's Fast Marching Method,
  Vladimirsky's Ordered Upwinding Methods, etc.

  HISTORY:
  Created by Ben Appleton (11/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#ifndef FMM_H
#define FMM_H

#include "pde_toolbox_bimage.h"
#include "pde_toolbox_LSTB.h"

/******************************** Definitions *************************************/
/* Fast Marching Method node states */
#define KNOWNNODE	-1
#define FARNODE	0
#define TRIALNODE	1

/* Ordered Upwind Method node states */
/* #define FARNODE	0 */
#define CONSIDEREDNODE 1
#define ACCEPTEDFRONTNODE 2
#define ACCEPTEDNODE 3

/* Additional anisotropy constant due to choice of grid (equals maximum difference of distance between two endpoints
of an arc and the nearest point along it */
#define ANISOSF 0.5

/* Possible return values from update() */
#define NOCHANGE	0
#define DECREASED	-1
#define INCREASED	1
#define ADD		2

/* Define the backtracking step size and the precision of Heun's method */
#define BACKSTEPSIZE	0.1
#define HEUNPRECISION	0.01
#define HEUNITERATIONS  10
/* MAXSEQOVERWRITES must be > sqrt(2)/BACKSTEPSIZE */
#define MAXSEQOVERWRITES 20

/********************************* Data Structures *********************************/
/* Encapsulate the heap (index) objects */
typedef struct {
	int *heapToImage;		/* Array of offsets from heap to image */
	int *imageToHeap;		/* Array of offsets from image into heap */
	int log2HeapSize;		/* Size of heap (max size) */
	int heapEnd;			/* Offset of current end of heap */
} HEAPSTRUCT;

/* Encapsulate the algorithm state objects */
typedef struct {
	char *nodeState;		/* Array of node states */
	BIMAGE * label;			/* Labels to propagate */
} FMMSTATESTRUCT;

/* A simplex is a triangle of grid points */
typedef struct {
	BVECT * cPoint;			/* CONSIDEREDNODE point to evaluate */
	BVECT * afPoint1;		/* The two ACCEPTEDFRONTNODE points from which to compute the new distance */
	BVECT * afPoint2;
} SIMPLEX;

/* Vectors and matrices */
typedef struct {
	float * buf;		/* The vector */
	int length;			/* The length of the vector */
} FLOATVECT;

typedef struct {
	float * buf;		/* The matrix components */
	int rows;			/* The number of rows of the matrix */
	int cols;			/* The number of columns of the matrix */
} FLOATMATRIX;

/* Coordinates on Riemann surfaces */
typedef struct {
	int x, y, z;
} RIE_INT;

typedef struct {
	float x, y;
	int z;
} RIE_FLOAT;

typedef struct {
	int source_x, source_y;		/* The source point */
	int source_radius;			/* The radius of the source point */
	int nx, ny, num_revs;		/* Planar size and number of revolutions of the surface */
} RIE_SURF;

/* A simplex is a triangle of grid points */
typedef struct {
	RIE_INT * cPoint;			/* CONSIDEREDNODE point to evaluate */
	RIE_INT * afPoint1;			/* The two ACCEPTEDFRONTNODE points from which to compute the new distance */
	RIE_INT * afPoint2;
} RIE_SIMPLEX;

/********************************* Function Prototypes *********************************/
/* Constructors and destructors for heaps */
HEAPSTRUCT * HEAPSTRUCT_constructor(
	BVECT * dim				/* The image dimensions (needed for hash table) */
);
int HEAPSTRUCT_destructor(
	HEAPSTRUCT * heapStruct		/* The object to free */
);

int heapAdd(
	HEAPSTRUCT *heapStruct,		/* The encapsulation of the heap */
	int imageIndex,				/* The image index of the point to add */
	BIMAGE *phi					/* The distance function */
);

int heapPull(
	HEAPSTRUCT *heapStruct,	/* The heap structure, containing the indexes */
	int index,				/* The heap index of the element to remove */
	BIMAGE *phi 			/* The 'distance' function by which we sort */
);

int heapShuffleUp(
	HEAPSTRUCT *heapStruct,		/* The heap structure, containing the fast indices */
	int index,					/* The index (in the heap) of the point to be shuffled */
	BIMAGE *phi					/* The 'distance' function */
);

int heapShuffleDown(
	HEAPSTRUCT *heapStruct,		/* The heap structure, containing the fast indices */
	int index,					/* The index (in the heap) of the point to be shuffled */
	BIMAGE *phi					/* The 'distance' function */
);

/* Constructors for vectors and matrices */
FLOATVECT * FLOATVECT_constructor(
	int length					/* The length of the vector to create */
);
void FLOATVECT_destructor(
	FLOATVECT * vect
);

/* Multidimensional arrays are indexed :
	A[i][j] = (A[i])[j] =  *(A + i*ni + j) = A[i*ni + j];
*/
FLOATMATRIX * FLOATMATRIX_constructor(
	int rows,					/* How many rows and columns in the matrix to create */
	int cols
);
void FLOATMATRIX_destructor(
	FLOATMATRIX * matrix
);

/* Vector and matrix functions */
/* Multiply a vector by a scalar: dest = scale_factor * source */
void FLOATVECT_scale(
	FLOATVECT * dest,
	float scale_factor,
	FLOATVECT * source
);
/* dest = souce1 + source2 */
void FLOATVECT_add(
	FLOATVECT * dest,
	FLOATVECT * source1,
	FLOATVECT * source2
);
/* Dot product */
float FLOATVECT_dot(
	FLOATVECT * source1,
	FLOATVECT * source2
);

/* Matrix by scalar: dest = scale_factor * source */
void FLOATMATRIX_scale(
	FLOATMATRIX * dest,
	float scale_factor,
	FLOATMATRIX * source
);
/* dest = source1 + source2 */
void FLOATMATRIX_add(
	FLOATMATRIX * dest,
	FLOATMATRIX * source1,
	FLOATMATRIX * source2
);
/* dest = source1 * source2 */
void FLOATMATRIX_mul(
	FLOATMATRIX * dest,
	FLOATMATRIX * source1,
	FLOATMATRIX * source2
);
/* dest = source^-1.  Returns determinant.  Specific to 2x2 matrices for now. */
float FLOATMATRIX_inv(
	FLOATMATRIX * dest,
	FLOATMATRIX * source
);
/* dest = source' */
void FLOATMATRIX_transpose(
	FLOATMATRIX * dest,
	FLOATMATRIX * source
);

/* Matrix-vector */
/* dest = matrix * source */
void FLOATVECT_premul(
	FLOATVECT * dest,
	FLOATMATRIX * matrix,
	FLOATVECT * source
);
/* dest = source * matrix */
void FLOATVECT_postmul(
	FLOATVECT * dest,
	FLOATVECT * source,
	FLOATMATRIX * matrix
);

/* Riemann surface related functions */
RIE_INT * RIE_INT_constructor(void);
void RIE_INT_destructor(RIE_INT * this);
RIE_FLOAT * RIE_FLOAT_constructor(void);
void RIE_FLOAT_destructor(RIE_FLOAT * this);
RIE_SURF * RIE_SURF_constructor(void);
void RIE_SURF_destructor(RIE_SURF * this);
int RIE_INT_add(int dx, int dy, char restrict_to_surface, RIE_SURF * rie_surf, RIE_INT * this);
int RIE_FLOAT_add(float dx, float dy, char restrict_to_surface, RIE_SURF * rie_surf, RIE_FLOAT * this);
int RIE_INT_to_index(RIE_INT * this, RIE_SURF * rie_surf);
int index_to_RIE_INT(int index, RIE_INT * coord, RIE_SURF * rie_surf);
char RIE_INT_on_surface(RIE_INT * this, RIE_SURF * rie_surf);
char RIE_FLOAT_on_surface(RIE_FLOAT * this, RIE_SURF * rie_surf);

/* Major functions */
int geodist(
	BIMAGE * seeds,				/* Non-zero values form seeds (overwritten by Voronoi tessellation) */
	BIMAGE * g,					/* The isotropic but non-homogeneous metric */
	const char stopping,		/* The stopping criteria type */
	const float threshold,		/* The stopping threshold */
	BIMAGE * distance			/* The geodesic distance function (output) */
);

int tensordist(
	BIMAGE * seeds,				/* Non-zero values form seeds (overwritten by Voronoi tessellation) */
	METRIC2D * metric,			/* The SPD metric tensor field */
	const char stopping,		/* The stopping criteria type */
	const float threshold,		/* The stopping threshold */
	BIMAGE * distance,			/* The geodesic distance function (output) */
	int * backpointers			/* Network backpointers for when grad fails */
);

int compute_aniso_ratio(
	METRIC2D * metric				/* The SPD metric tensor field */
);

int geodesic(
	BVECT * start,				/* The start of the shortest path */
	BIMAGE * distance,			/* The distance function */
	BIMAGE * contour			/* The output contour */
);

int tensorgeodesic(
	BVECT * start,				/* The start of the shortest path */
	BIMAGE * distance,			/* The distance function */
	METRIC2D * metric,			/* The SPD metric field */
	int * backpointers,			/* Network descent backpointers */
	BIMAGE * contour			/* The output contour */
);

int gogac(
	float *in,					/* Input metric image (2D) - *not* radially weighted */
	int nx, int ny,				/* Image dimensions */
	int xCentre, int yCentre,	/* Centre of cut */
	float *out					/* Output image */
);

int oumgac(
	METRIC2D * in_metric,			/* The SPD metric field */
	int xCentre, int yCentre,	/* Source point */
	BIMAGE * out				/* The output image */
);

#endif

