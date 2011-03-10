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
 * File:		pde_toolbox_LSTB.h
 *
 * Written by:		Ben Appleton
 * 			ITEE, The University of Queensland
 *
 * Date:		October 2002
 *
 * Copyright:		Intended for open-source distribution
 *
*/

/*********************************************************************************************
 LSTB.h
 ------

  DESCRIPTION:
  Definitions for the Level Set ToolBox.

  HISTORY:
  Created by Ben Appleton (10/02)
  Contact: appleton@itee.uq.edu.au
**********************************************************************************************/

#ifndef LSTB_H
#define LSTB_H

#include "pde_toolbox_bimage.h"
/* #include "FMM.h" */

typedef void (pde_hook_func)(void);

/********************************* Data Structures *********************************/
/* The narrow band structure for direct iteration of the level set equation */
typedef struct {
	int *band;					/* Band: array of offsets into image */
	int innerSize;				/* The size of the inner-band set, not including the edge set */
	int edgeSize;				/* The size of the edge array */
	int outerSize;				/* The size of the outer set, needed for signs */
	int outerouterSize;			/* The size of the outerouter set, needed for outer adjacency signs */
	char *signs;				/* The signs of the associated band pixels, indexed by image index */
} NBSTRUCT;

/* The run-length encoding representation of the band for a single row */
typedef struct _RLENODE {
	int start;				/* The start of the run of KNOWN nodes */
	int end;				/* The end of the run of KNOWN nodes */
	struct _RLENODE *next;	/* The next run of KNOWN nodes, NULL if no more */
} RLENODE;

/* An array of the RLE representations for each row */
typedef struct {
	RLENODE **rowRootNode;	/* An array of pointers to the root node of each row */
	BVECT *dim;				/* The dimensions of this array */
} RLEARRAY;

/* The basic DL-interlaced format of the ldl arrays */
typedef struct {
	double D;
	double L;
} LDLCOMP;

/* An array of LDL decompositions */
/*
typedef struct {
	LDLCOMP *ldl;				//. The LDL data
	BVECT *dim;			// The dimension of this LDL decomposition - such that decomposition is along first
						// axis for low stride.  dim is same as that of image, except that we make the
						// decomposition axis the first one and shift the remainder along to give a dim
						// vector that is the same as that for the corresponding RLEARRAY
} LDLARRAY;
*/

/* The main AOS structure, being an array of RLE arrays */
typedef struct {
	RLENODE *freeNodes;			/* Free nodes for allocation (cyclic linked list) - must be kept perfectly clean! */
	RLEARRAY **rleArrayArray;		/* Array of N pointers to the RLEARRAYs - one for each dimension */
/*	LDLARRAY **ldlArrayArray; */		/* Array of N pointers to the LDLARRAYs - one for each dimension */
	int numNodes;				/* Total number of nodes */
} AOSSTRUCT;

/* The level set structure, holding all others inside */
typedef struct {
	NBSTRUCT * nb;				/* The narrow band structure */
	AOSSTRUCT * aos;			/* The additive operator splitting structure */
	BIMAGE * phi;				/* The phi function (level sets) */
	float thickness;			/* The thickness of the narrow band */
} LSSTRUCT;

/* GAC speed function parameter */
typedef struct {
	float alpha;				/* The area penalty coefficient */
} GAC_PARAM;

/* Sethian's speed function parameters */
typedef struct {
	float epsilon;			/* The viscosity coefficient */
	float beta;				/* The edge-attraction coefficient */
} SETHIAN_PARAM;

/* Gradient Vector Flow parameter */
typedef struct {
	float alpha;
} GVF_PARAM;

/* GOGAC_Polygon (optional output format for gogac routines) */
typedef struct {
	float * points;				/* Stored as (x0, y0, x1, y1, ...) */
	int num_points;				/* The number of points currently in the polygon */
	int allocated_length;		/* The allocated length of the points buffer (dynamic realloc by doubling/halving scheme) */
} GOGAC_Polygon;
#define GOGAC_POLYGON_MIN_ALLOCATED_LENGTH 2

/********************************* Function Prototypes *********************************/
/** Linked list utility functions **/
int initLinkedList(
	RLENODE *freeNodes,							/* The list of free nodes to set up as a cyclic list */
	int numNodes);								/* The number of nodes in the list */

RLENODE *mallocNode(
	RLENODE *freeNodes);						/* The list of free nodes */

int freeNode(
	RLENODE *node,								/* The node to be freed */
	RLENODE *freeNodes);						/* The list of free nodes */

/* Constructor and destructor for LSSTRUCT 'class' */
LSSTRUCT * LSSTRUCT_constructor(
	BVECT * dim, 					/* The requested dimensions */
	float thickness,				/* The thickness of the narrow band */
	char use_aos					/* Use AOS as well? */
);
void LSSTRUCT_destructor(LSSTRUCT * lsstruct);

/* Level set upsampler */
int LS_upsample(
	LSSTRUCT ** ls_ptr,					/* The level set object to upsample */
	BVECT * up_dim,						/* The upsampled image dimensions (not simple multiple) */
	int upsampling_rate				/* The upsampling rate (integer -> viable DS upsampling) */
);

/* N-D narrow band reinitialisation function */
int nbreinit(
	LSSTRUCT * ls				/* The level set object */
);

/* N-D level set iterator */
int lvlseg(
	void * in,							/* The general input image (may be scalar or vector for example) */
	LSSTRUCT * ls,						/* The level set object */
	/* Pointer to generic speed function */
	float (* outward_normal_speed) (
		void *,							/* Input image */
		BIMAGE *,						/* Phi function */
		BVECT *,						/* Coordinate at which to compute speed */
		void *							/* Void pointer to parameter list */
	),
	void * param_list,					/* Generic structure holding parameters */
	float iterationTime 				/* The time over which the PDE is run */
);

/** Utility functions for curve PDEs **/
/* Some default speed functions for level sets */
float LS_GAC_speed_function(
	void * in,								/* The input image */
	BIMAGE *phi,							/* The phi function */
	BVECT *coord,							/* The co-ordinate at which to compute the speed */
	void * param_list						/* Void pointer to structure containing parameters */
);

/* The speed function from Sethian's book */
float LS_sethian_speed_function(
	void * in,								/* The input image */
	BIMAGE *phi,							/* The phi function */
	BVECT *coord,							/* The co-ordinate at which to compute the speed */
	void * param_list						/* Void pointer to structure containing parameters */
);

/* The speed function for Gradient Vector Flows */
float LS_GVF_speed_function(
	void * in,								/* The input image */
	BIMAGE *phi,							/* The phi function */
	BVECT *coord,							/* The co-ordinate at which to compute the speed */
	void * param_list						/* Void pointer to structure containing parameters */
);

/* Compute the curvature of a level set */
float LS_curv(
	BIMAGE *phi,							/* The phi function */
	BVECT *pcoord);							/* The co-ordinate at which to compute the curvature */

/* Compute the attraction of a level set to an edge in an image */
float LS_attract(
	BIMAGE *P, 								/* The potential image */
	BIMAGE *phi, 							/* The phi function (level sets) */
	BVECT *pcoord);							/* The co-ordinate at which to compute the edge-attraction term */
float LS_vector_attract(
	BIMAGE * * in, 							/* The input image, typically a gradient image */
	BIMAGE *phi, 							/* The phi function (level sets) */
	BVECT *pcoord							/* The co-ordinate at which to compute the edge-attraction term */
);

/* N-D level set iterator by AOS */
int aosseg(
	BIMAGE *g,						/* The scalar metric image */
	LSSTRUCT * ls,					/* The level set data structure */
	float alpha, 					/* Balloon force coefficient */
	float iterationTime, 			/* The time over which the PDE is run */
	float timestep 					/* The iteration time step */
);

/** Level set I/O **/
/* Creates a level set from a contour and a vector of interior points */
int init_LS_contour(
		LSSTRUCT * ls,			/* Level set object to fill */
		BIMAGE *in,			/* Input image - !0 on contour, 0 elsewhere */
		BVECT * * sources,		/* Array of source (interior) BVECTs */
		int num_sources			/* Number of source points */
);

/* Create a hyperspherical level set */
int init_LS_sphere(
	LSSTRUCT * ls,				/* The level set object */
	float radius,				/* The radius of the hypersphere */
	BVECT * centre				/* The centre coordinate (int) of the hypersphere */
);

/* Accept a manually created level set and reinitialise, intialising data structures */
int init_LS_direct(
	LSSTRUCT * ls,				/* The level set object */
	BIMAGE * level_set			/* The initial level set */
);

/* Initialise the level set to a cubic lattice of spheres */
int init_LS_spherepacking(
	LSSTRUCT * ls,				/* The level set object */
	int spacing					/* The spacing between sphere centres */
);

int init_LS_border(
	LSSTRUCT * ls				/* The level set object */
);

/* Copies the zero level set contour to the output BIMAGE */
int output_LS_contour(
		LSSTRUCT * ls,			/* Level set object to fill */
		BIMAGE *out			/* Output image - 1 contour, 0 elsewhere */
);

/* Directly copies the level sets to the given output BIMAGE */
int output_LS_direct(
		LSSTRUCT * ls,			/* Level set object to fill */
		BIMAGE *out			/* Output image - 1 contour, 0 elsewhere */
);

/* Outputs the internal regions as -1.0, external as 1.0 */
int output_LS_region(
		LSSTRUCT * ls,			/* Level set object to fill */
		BIMAGE *out			/* Output image - 1 contour, 0 elsewhere */
);

/* GOGAC functions */
int gogac(
	float *in,					/* Input metric image (2D) - *not* radially weighted */
	int nx, int ny,				/* Image dimensions */
	int xCentre, int yCentre,	/* Centre of cut */
	float *out					/* Output image */
);

int gogacw(
	float *in,					/* Input metric image (2D) - *not* radially weighted */
	float *w,					/* Weighting function, usually 1/radius from seed point */
	int nx, int ny,				/* Image dimensions */
	int xCentre, int yCentre,	/* Centre of cut */
	float *out,					/* Output image */
	GOGAC_Polygon * p			/* Output GOGAC_polygon (optional) */
);


/** Printing functions **/
/* LSTBmsgs */
void LSTB_show_messages(void);
void LSTB_clear_messages(void);
int LSTB_add_message(const char * msg,...);
int LSTB_append_line(const char * buff);
char ** LSTB_get_messages(void);
int LSTB_get_num_messages(void);
/* LSTBmsg */
void LSTB_error(char *msg, ...);
void LSTB_enable_debug(void);
void LSTB_disable_debug(void);
int LSTB_is_debug_enabled(void);
void LSTB_debug(char *msg, ...);

/* GOGAC_Polygon methods */
GOGAC_Polygon * GOGAC_Polygon_constructor(
	void
);

void GOGAC_Polygon_destructor(
	GOGAC_Polygon * p
);

void GOGAC_Polygon_append_point(
	float * point,		/* x == point[0], y == point[1] */
	GOGAC_Polygon * p
);

/* Experimental code */
int etals(
	BIMAGE *in,							/* The general input image (may be normal, grad or metric for example) */
	LSSTRUCT * ls,						/* The level set object */
	/* Pointer to generic speed function */
	float (* outward_normal_speed) (
		void *,							/* Input image */
		BIMAGE *,						/* Phi function */
		BVECT *,						/* Coordinate at which to compute speed */
		void *							/* Void pointer to parameter list */
	),
	void * param_list,					/* Generic structure holding parameters */
	float iteration_time 				/* The time over which the PDE is run */
);

int contmaxflow(
	BIMAGE * g,					/* The isotropic but non-homogeneous metric */
	char * type,				/* The type of each vertex - normal, source or sink */
	float simulationtime,		/* The total time to run the simulation */
	BIMAGE * P,					/* The input/output P field */
	BIMAGE * * F,				/* The input/output F field */
	pde_hook_func * cbf,		/* callback e.g for display, can be NULL */
	double * Pbuf,				/* debug pressure buffer, can be NULL */
	double * Fx,				/* debug velocity buffer, can be NULL */
	double * Fy,				/* debug velocity buffer, can be NULL */
	double * Fz,				/* debug velocity buffer, can be NULL */
	BVECT * out_dim,			/* Dimensions of imview window */
	int period,					/* callback called every N */
	int num_threads
);

int maxflow(
	BIMAGE * g,							/* The input metric image (no need to radially weight!) */
	char * type,						/* The type of each vertex */
	BIMAGE * P,							/* The pressure function (output) */
	BIMAGE * * F						/* The flows (scaled to floating point) */
);

#endif

