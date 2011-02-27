/*
 * File:		fseries.h
 *
 
 *
*/

/**********************************************************
 fseries.h
 -----------
 
**********************************************************/  

#ifndef LIAR_FSERIES_H
#define LIAR_FSERIES_H

/* f-series directory */
#define PERIODIC	0
#define BRESENHAM	1



/* prototypes from genfmin.c, function in lfdilate.c etc */
extern void genfmin_CHAR(PIX_TYPE *f,PIX_TYPE *g,PIX_TYPE *h,INT4_TYPE *p,
			 unsigned int nx, unsigned int K);
extern void genfmin_char(PIX_TYPE *f,PIX_TYPE *g,PIX_TYPE *h,INT4_TYPE *p,
		    unsigned int nx, unsigned int K);

extern void genfmin_INT4(INT4_TYPE *f,INT4_TYPE *g,INT4_TYPE *h,INT4_TYPE *p,
			 unsigned int nx,unsigned int K);
extern void genfmin_int4(INT4_TYPE *f,INT4_TYPE *g,INT4_TYPE *h,INT4_TYPE *p,
			 unsigned int nx,unsigned int K);

extern void genfmin_long(LONG_TYPE *f,LONG_TYPE *g,LONG_TYPE *h,INT4_TYPE *p,
			 unsigned int nx,unsigned int K);

extern void genfmin_DBL(DBL_TYPE *f,DBL_TYPE *g,DBL_TYPE *h,INT4_TYPE *p,
			unsigned int nx,unsigned int K);
extern void genfmin_dbl(DBL_TYPE *f,DBL_TYPE *g,DBL_TYPE *h,INT4_TYPE *p,
			unsigned int nx,unsigned int K);


/* prototypes from genfmax.c, function in lfdilate.c etc */
extern void genfmax_CHAR(PIX_TYPE *f,PIX_TYPE *g,PIX_TYPE *h,INT4_TYPE *p,
			 unsigned int nx, unsigned int K);
extern void genfmax(PIX_TYPE *f,PIX_TYPE *g,PIX_TYPE *h,INT4_TYPE *p,
		    unsigned int nx, unsigned int K); 

extern void genfmax_INT4(INT4_TYPE *f,INT4_TYPE *g,INT4_TYPE *h,INT4_TYPE *p,
			 unsigned int nx, unsigned int K);
extern void genfmax_int4(INT4_TYPE *f,INT4_TYPE *g,INT4_TYPE *h,INT4_TYPE *p,
			 unsigned int nx, unsigned int K);

extern void genfmax_DBL(DBL_TYPE *f,DBL_TYPE *g,DBL_TYPE *h,INT4_TYPE *p,
			unsigned int nx,unsigned int K);
extern void genfmax_dbl(DBL_TYPE *f,DBL_TYPE *g,DBL_TYPE *h,INT4_TYPE *p,
			unsigned int nx,unsigned int K);




#endif // LIAR_FSERIES_H


