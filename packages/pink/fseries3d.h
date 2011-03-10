/*------------------------------------------------------------------------
 *
 * Prototypes for the minimal version of the
 * Library of Image Analysis Routines
 *
 * From Imview
 *
 * Hugues Talbot	 4 Jan 2001
 *      
 *-----------------------------------------------------------------------*/

#ifndef FSERIES_3D_H
#define FSERIES_3D_H

#ifdef __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */


    /* 3d rectangular morphological operations */
/* prototypes from lfdilate3d.c - from Z-IMAGE function fdilate3D */
extern int lfdilate3d_rect(PIX_TYPE *inbuf, PIX_TYPE *outbuf, int ncol,
			   int nrow, int nslice, int dimx, int dimy, int dimz);
/* prototypes from lferode3d.c - from Z-IMAGE function ferode3D */
extern int lferode3d_rect(PIX_TYPE *inbuf, PIX_TYPE *outbuf, int ncol,
			  int nrow, int nslice, int dimx, int dimy, int dimz);
/* prototypes from lfopen3d.c - from Z-IMAGE function fopen3D */
extern int lfopen3d_rect(PIX_TYPE *inbuf, PIX_TYPE *outbuf, int ncol,
			 int nrow, int nslice, int dimx, int dimy, int dimz);
/* prototypes from lfclose3d.c - from Z-IMAGE function fclose3D */
extern int lfclose3d_rect(PIX_TYPE *inbuf, PIX_TYPE *outbuf, int ncol,
			  int nrow, int nslice, int dimx, int dimy, int dimz);
/* prototypes from rect3dmm.c, function in lfdilate3D.c etc */
extern void rect3dminmax(PIX_TYPE *in, int nx, int ny, int nz, int w, int b,
			 int d, void (*func) () );


/* 3d linear morphological operations */
/* this first few are not really 3d so should be removed at some stage soon */
/* prototypes from bresen3.c, function in glinemm2.c, glinemm3.c */
extern INT4_TYPE *bresenham3(int xstart, int ystart, int dx, int dy, 
			     int imwidth, int imheight, int *ol);
/* prototypes from glinemm2.c, function in ? */
extern void glineminmax2(PIX_TYPE *f, int nx, int ny, int k, int dx, int dy,
			 void (*operation)());
/* prototypes from glinemm3.c, function in lfdilate3D.c etc */
extern void glineminmax3(PIX_TYPE *f, int nx, int ny, int k, int dx, int dy,
			 void (*operation)());
/* prototypes from bresen3d.c, function in lfdilate3D.c etc */
extern INT4_TYPE *bresenham3d(int dx, int dy, int dz, 
			      int imwidth, int imheight, int imdepth, 
			      int *ol, int *period);

extern INT4_TYPE *bresenham3d_dir(int dx, int dy, int dz, 
                                  int imwidth, int imheight, int imdepth,
                                  int length,
                                  int *ol, int *period);

    /* prototypes from period3d.c, function in lfdilate3D.c etc */
extern INT4_TYPE *periodic3d(int dx, int dy, int dz, 
			     int imwidth, int imheight, int imdepth, 
			     int *ol, int *period);
/* prototypes from glinemm3d.c, function in lfdilate3D.c etc */
extern int glineminmax3d(PIX_TYPE *f, int nx, int ny, int nz, int k, 
			  int dx, int dy, int dz, void (*operation)(), 
			  INT4_TYPE *(*lineop)());

/* prototypes from lferode3d.c - from Z-IMAGE function ferode3D */
extern int lferode3d_line(PIX_TYPE *inbuf, PIX_TYPE *outbuf, 
			  int ncol, int nrow, int nslice, int length, 
			  int dx, int dy, int dz, int type);
/* prototypes from lfdilate3d.c - from Z-IMAGE function fdilate3D */
extern int lfdilate3d_line(PIX_TYPE *inbuf, PIX_TYPE *outbuf, 
			   int ncol, int nrow, int nslice, int length, 
			   int dx, int dy, int dz, int type);
/* prototypes from lfclose3d.c - from Z-IMAGE function fclose3D */
extern int lfclose3d_line(PIX_TYPE *inbuf, PIX_TYPE *outbuf, 
		   int ncol, int nrow, int nslice, int length, 
		   int dx, int dy, int dz, int type);
/* prototypes from lfopen3d.c - from Z-IMAGE function fopen3D */
extern int lfopen3d_line(PIX_TYPE *inbuf, PIX_TYPE *outbuf, 
		   int ncol, int nrow, int nslice, int length, 
                         int dx, int dy, int dz, int type);

    

#ifdef __cplusplus
}
#endif                          /* __cplusplus */

    
#endif // FSERIES_3D_H
