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
   ** FILE
   **   powell.c - Brent-Powell conjugate directions minimization method
   **
   ** DESCRIPTION
   **   performs a minimization of a multidimensional user supplied function
   **
   ** v 1.7  19 Sep. 1997
   **   STATIC ARRAY VERSION
   **   Added maxiter parameter
   **
   **   (C) Copyright Dario Bressanini
   **   Dipartimento di Scienze Matematiche Fisiche e Chimiche 
   **   Universita' dell'Insubria, sede di Como
   **   E-mail: dario@fis.unico.it

Reference:
Brent, R. P. (1973) Algorithms for Minimization without Derivatives. Prentice-Hall, Englewood-Cliffs, New Jersey, USA

 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcpowell.h>

//#define VERBOSE 
#define MAXDIM		100

static double pold[MAXDIM];
static double pextrap[MAXDIM];
static double new_dir[MAXDIM];
static double dir[MAXDIM][MAXDIM];

/*
   ** FILE
   **   linmin.c - line minimization of a N dimensional function
   **
   ** DESCRIPTION
   **   This routine perform the minimization of a R^N->R
   **   along a user-specified direction.
   **
   ** USES:
   **   minimize.c
   **
   ** HISTORY
   **   v 1.0    1 feb 1993     start from Numerical Recipes Version
   **   v 1.1   17 Jun 1993     further modified.
   **   v 1.2    6 Jun 1997     little cleanup (no longer use defs.h)
   **                              STATIC ARRAY VERSION
   **
   ** AUTHOR
   **   (C) Copyright Dario Bressanini
   **   Dipartimento di Scienze Matematiche Fisiche e Chimiche 
   **   Universita' dell'Insubria, sede di Como
   **   E-mail: dario@fis.unico.it
 */

#define PTOL	1e-6		/* doesn't need high accuracy */

static PFNe func;		/* this stores the function name */
static PFNn func_num;		/* this stores the function name */
static int32_t ndim;		/* number of dimensions */
static double *point;		/* store the address of a vector */
static double *direc;		/* ............................. */
static double ptrial[MAXDIM];	/* trial point. */

/*
   ** FILE
   **   minimize.c - minimization of 1-dimensional functions 
   **
   ** DESCRIPTION
   **   this file contains various minimization routines to perform
   **   a minimization of a user supplied function
   **
   ** HISTORY
   **   v 1.0    1 feb 1993     start from Numerical Recipes Version
   **   v 1.1   17 Jun 1993     further modified.
   **   v 1.2    6 Jun 1997     little cleanup (no longer use defs.h)
   **                              STATIC ARRAY VERSION
   **
   ** AUTHOR
   **   (C) Copyright Dario Bressanini
   **   Dipartimento di Scienze Matematiche Fisiche e Chimiche 
   **   Universita' dell'Insubria, sede di Como
   **   E-mail: dario@fis.unico.it
 */

/*-----------------------------------------------------------------------*/

#define GOLD 	1.618033988	/* golden section, (sqrt(5)+1)/2 */
#define RGOLD	(GOLD - 1.0)	/* 0.618034 */
#define CGOLD	(1.0 - RGOLD)	/* 0.3819660 */

#define MAX_ITER	30	/* max. number of iteration */

/*
   ** Try to bracket a minimum of the user supplied function f() given
   ** two initial points *x0 and *x1.
   ** First look for the minimum inside the interval. If not found
   ** go downhill until found or until MAX_ITER iteration have been done
   ** If bracketing succedeed the triple is returned in *x0, *x1 and *x2.
   ** Simpler than the one given in Numerical Recipes but sufficient for
   ** most purposes
   **
   ** RETURN
   **   M_FOUND: bracketing OK.
   **   M_MAX_ITER: too many iterations performed.
 */

#define NUM_DIV		8	/* number of divisions of interval */

int32_t bracket(FUNCe f, ensemble *ens, double *x0, double *x1, double *x2)
{
    double ax, bx, cx;
    double fa, fb, fc;
    double x[NUM_DIV + 1], fx[NUM_DIV + 1];	/* start from 0 */
    int32_t i = 0;


    fa = f(*x0,ens);
    fb = f(*x1,ens);
    if (fa > fb) {		/* must be fa > fb; in case switch so we */
	ax = *x0;		/* can go downhill */
	bx = *x1;
    } else {
	ax = *x1;
	bx = *x0;
	fb = fa;		/* we need only the lowest */
    }

    /* first search inside (ax,bx) dividing the interval in NUM_DIV steps */
    for (i = 0; i < NUM_DIV + 1; i++) {
	x[i] = ax + i * (bx - ax) / ((double) NUM_DIV);
	fx[i] = f(x[i],ens);
    }
    for (i = 1; i < NUM_DIV; i++)
	if (fx[i] < fx[i - 1] && fx[i] < fx[i + 1]) {	/* found!! */
	    *x0 = x[i - 1];
	    *x1 = x[i];
	    *x2 = x[i + 1];
	    return M_FOUND;
	}
    /* not found inside, so go downhill */

    cx = bx + GOLD * (bx - ax);
    fc = f(cx,ens);
    while (fb >= fc) {
	ax = bx;
	bx = cx;
	cx = bx + GOLD * (bx - ax);
	fb = fc;
	fc = f(cx,ens);
	if (i++ == MAX_ITER)	/* too many iterations */
	    return M_MAX_ITER;
	/* printf("c %15.10f  fc %15.10f\n",cx,fc); */
    }

    *x0 = ax;
    *x1 = bx;
    *x2 = cx;
    return M_FOUND;
} // bracket()

int32_t bracket_num(FUNCn f, struct xvimage * image1, struct xvimage * image2, double *x0, double *x1, double *x2)
{
    double ax, bx, cx;
    double fa, fb, fc;
    double x[NUM_DIV + 1], fx[NUM_DIV + 1];	/* start from 0 */
    int32_t i = 0;


    fa = f(*x0, image1, image2);
    fb = f(*x1, image1, image2);
    if (fa > fb) {		/* must be fa > fb; in case switch so we */
	ax = *x0;		/* can go downhill */
	bx = *x1;
    } else {
	ax = *x1;
	bx = *x0;
	fb = fa;		/* we need only the lowest */
    }

    /* first search inside (ax,bx) dividing the interval in NUM_DIV steps */
    for (i = 0; i < NUM_DIV + 1; i++) {
	x[i] = ax + i * (bx - ax) / ((double) NUM_DIV);
	fx[i] = f(x[i], image1, image2);
    }
    for (i = 1; i < NUM_DIV; i++)
	if (fx[i] < fx[i - 1] && fx[i] < fx[i + 1]) {	/* found!! */
	    *x0 = x[i - 1];
	    *x1 = x[i];
	    *x2 = x[i + 1];
	    return M_FOUND;
	}
    /* not found inside, so go downhill */

    cx = bx + GOLD * (bx - ax);
    fc = f(cx, image1, image2);
    while (fb >= fc) {
	ax = bx;
	bx = cx;
	cx = bx + GOLD * (bx - ax);
	fb = fc;
	fc = f(cx, image1, image2);
	if (i++ == MAX_ITER)	/* too many iterations */
	    return M_MAX_ITER;
	/* printf("c %15.10f  fc %15.10f\n",cx,fc); */
    }

    *x0 = ax;
    *x1 = bx;
    *x2 = cx;
    return M_FOUND;
} // bracket_num()

/*
   ** DESCRIPTION
   **   find the minimum of a user supplied functions using brent's method
 */

#define TINY  1.0e-10

int32_t brent(FUNCe f, ensemble *ens, double ax, double bx, double cx, double tol,
	  double *xmin, double *fmin)
{
    double a, b, d, e, u, v, w, x;
    double p, q, r;
    double fx, fu, fv, fw;
    double xm, tol1, tol2;
    double temp;
    int32_t i, flag;

    /* tol = mcmax(tol, SQRT_DBL_EPSILON); */

    a = mcmin(ax, cx);		/* a and b must be in ascending order */
    b = mcmax(ax, cx);
    x = w = v = bx;
    d = e = 0.0;		/* e: distance moved on the step before last */
    fv = fw = fx = f(x, ens);
    for (i = 0; i < MAX_ITER; i++) {
	xm = (a + b) / 2.0;
	tol1 = tol * mcabs(x) + TINY;	/* this avoids searching for 0 */
	tol2 = 2 * tol1;
	if (mcabs(x - xm) <= (tol2 - (b - a) / 2.0)) {
	    *xmin = x;
	    *fmin = fx;
	    return M_FOUND;
	}
	flag = 0;
	if (mcabs(e) > tol1) {	/* try parabolic fit */
	    r = (x - w) * (fx - fv);
	    q = (x - v) * (fx - fw);
	    p = (x - v) * q - (x - w) * r;
	    q = 2 * (q - r);
	    if (q > 0.0)
		p = -p;
	    q = mcabs(q);
	    temp = e;
	    e = d;
	    /* now check if parabolic fit is ok */
	    if (mcabs(p) < mcabs(.5 * q * temp) || (p > q * (a - x) && p < q * (b - x))) {
		d = p / q;
		u = x + d;
		if (u - a < tol2 || b - u < tol2)
		    d = tol1 * mcsign(xm - x);
		flag = 1;
	    }
	}
	if (flag == 0) {
	    if (x >= xm)
		e = a - x;
	    else
		e = b - x;
	    d = CGOLD * e;
	}
	if (mcabs(d) >= tol1)
	    u = x + d;
	else
	    u = x + tol1 * mcsign(d);

	fu = f(u, ens);
	if (fu <= fx) {
	    if (u >= x)
		a = x;
	    else
		b = x;
	    v = w;
	    fv = fw;
	    w = x;
	    fw = fx;
	    x = u;
	    fx = fu;
	} else {
	    if (u < x)
		a = u;
	    else
		b = u;

	    if (fu <= fw || w == x) {
		v = w;
		fv = fw;
		w = u;
		fw = fu;
	    } else if (fu <= fv || v == x || v == w) {
		v = u;
		fv = fu;
	    }
	}
    }
    *xmin = x;
    *fmin = fx;
    return M_MAX_ITER;
} // brent()

int32_t brent_num(FUNCn f, struct xvimage * image1, struct xvimage * image2, 
		  double ax, double bx, double cx, double tol,
		  double *xmin, double *fmin)
{
    double a, b, d, e, u, v, w, x;
    double p, q, r;
    double fx, fu, fv, fw;
    double xm, tol1, tol2;
    double temp;
    int32_t i, flag;

    /* tol = mcmax(tol, SQRT_DBL_EPSILON); */

    a = mcmin(ax, cx);		/* a and b must be in ascending order */
    b = mcmax(ax, cx);
    x = w = v = bx;
    d = e = 0.0;		/* e: distance moved on the step before last */
    fv = fw = fx = f(x, image1, image2);
    for (i = 0; i < MAX_ITER; i++) {
	xm = (a + b) / 2.0;
	tol1 = tol * mcabs(x) + TINY;	/* this avoids searching for 0 */
	tol2 = 2 * tol1;
	if (mcabs(x - xm) <= (tol2 - (b - a) / 2.0)) {
	    *xmin = x;
	    *fmin = fx;
	    return M_FOUND;
	}
	flag = 0;
	if (mcabs(e) > tol1) {	/* try parabolic fit */
	    r = (x - w) * (fx - fv);
	    q = (x - v) * (fx - fw);
	    p = (x - v) * q - (x - w) * r;
	    q = 2 * (q - r);
	    if (q > 0.0)
		p = -p;
	    q = mcabs(q);
	    temp = e;
	    e = d;
	    /* now check if parabolic fit is ok */
	    if (mcabs(p) < mcabs(.5 * q * temp) || (p > q * (a - x) && p < q * (b - x))) {
		d = p / q;
		u = x + d;
		if (u - a < tol2 || b - u < tol2)
		    d = tol1 * mcsign(xm - x);
		flag = 1;
	    }
	}
	if (flag == 0) {
	    if (x >= xm)
		e = a - x;
	    else
		e = b - x;
	    d = CGOLD * e;
	}
	if (mcabs(d) >= tol1)
	    u = x + d;
	else
	    u = x + tol1 * mcsign(d);

	fu = f(u, image1, image2);
	if (fu <= fx) {
	    if (u >= x)
		a = x;
	    else
		b = x;
	    v = w;
	    fv = fw;
	    w = x;
	    fw = fx;
	    x = u;
	    fx = fu;
	} else {
	    if (u < x)
		a = u;
	    else
		b = u;

	    if (fu <= fw || w == x) {
		v = w;
		fv = fw;
		w = u;
		fw = fu;
	    } else if (fu <= fv || v == x || v == w) {
		v = u;
		fv = fu;
	    }
	}
    }
    *xmin = x;
    *fmin = fx;
    return M_MAX_ITER;
} // brent_num()

/*-------------------------------------------------------------------------*/


/*
   ** This function acts as interface between a monodimensional minimization
   ** function such as brent and a multidimensional function
   ** This function is called from monodimensional routines and uses globals
   ** point[] and direc[] are duplicate
   ** pointers to the real arrays received by linmin, same for (*func)()
 */

static double f1dim(double x, ensemble *e)
{
    int32_t j;

    for (j = 0; j < ndim; j++)
	ptrial[j] = point[j] + x * direc[j];

    return func(ptrial, e);
} // f1dim()

static double f1dim_num(double x, struct xvimage * image1, struct xvimage * image2)
{
    int32_t j;

    for (j = 0; j < ndim; j++)
	ptrial[j] = point[j] + x * direc[j];

    return func_num(ptrial, image1, image2);
} // f1dim_num()


/*
   ** minimize a user supplied function of 'n' variables along the
   ** direction specified by dirv[]
   ** if M_FOUND, set the point vector p to the new point and reset the
   ** direction vector to the new displacement vector
   ** the value of the function at the minimum is returned in *ymin
   **
   ** RETURN
   **   M_FOUND         minimum found
   **   int32_t from bracket() or brent()
 */

int32_t linmcmin(PFNe f, ensemble *ens, double p[], double dirv[], int32_t n, double *fmin)
{
    double x1, x2, x3, xmin;
    int32_t i;
    int32_t code;

    ndim = n;			/* save the number of dimensions */
    point = p;			/* copy the pointers */
    direc = dirv;		/* so that f1dim can find them */
    func = f;

    x1 = 0.0;			/* MUST CHANGE THIS.... */
    x2 = 0.1;
    if ((code = bracket(f1dim, ens, &x1, &x2, &x3)) != M_FOUND)
	return code;

    if ((code = brent(f1dim, ens, x1, x2, x3, PTOL, &xmin, fmin)) != M_FOUND)
	return code;

    for (i = 0; i < ndim; i++) {
	dirv[i] *= xmin;	/* update direction vector */
	p[i] += dirv[i];	/* construct the vector result */
    }

    return M_FOUND;
} // linmcmin()

int32_t linmin_num(PFNn f, struct xvimage * image1, struct xvimage * image2, double p[], double dirv[], int32_t n, double *fmin)
{
    double x1, x2, x3, xmin;
    int32_t i;
    int32_t code;

    ndim = n;			/* save the number of dimensions */
    point = p;			/* copy the pointers */
    direc = dirv;		/* so that f1dim can find them */
    func_num = f;

    x1 = 0.0;			/* MUST CHANGE THIS.... */
    x2 = 0.1;
    if ((code = bracket_num(f1dim_num, image1, image2, &x1, &x2, &x3)) != M_FOUND)
	return code;

    if ((code = brent_num(f1dim_num, image1, image2, x1, x2, x3, PTOL, &xmin, fmin)) != M_FOUND)
	return code;

    for (i = 0; i < ndim; i++) {
	dirv[i] *= xmin;	/* update direction vector */
	p[i] += dirv[i];	/* construct the vector result */
    }

    return M_FOUND;
} // linmin_num()


/*--------------------------------------------------------------------------*/

/* n is the number of dimensions */
/* dir is the matrix of column directions vectors */

int32_t powell(PFNe f, ensemble * ens, double p[], int32_t n, double tol,
	   double scale, int32_t maxiter, double *fmin)
{
    double new_value, cur_value, prev_value, trial_value;
    double max_decr;		/* max. function decrease */
    double tmp;
    int32_t i, j, iter, flag;
    int32_t go_down;		/* direction of descent */

    //    double e, s2, sm2, weff;	/* used for weight() */

    if (n > MAXDIM) {
	printf("***ERROR:powell: Too many parameters. Max = %d\n", MAXDIM);
	exit(-1);
    }
    for (i = 0; i < n; i++)	/* set up scale lenght */
	for (j = 0; j < n; j++)
	    dir[i][j] = ((i == j) ? scale : 0.0);

    new_value = f(p,ens);		/* compute initial value */
    for (i = 0; i < n; i++)	/* save the old point */
	pold[i] = p[i];
    
    for (iter = 0; iter < maxiter; iter++) 
    {
      go_down = -1;		/* impossible direction */
      max_decr = 0.0;
      cur_value = new_value;
      /*
      ** now loop over the direction set and try to minimize along each one
      */

      for (i = 0; i < n; i++) 
      {
	for (j = 0; j < n; j++) new_dir[j] = dir[j][i];
	prev_value = new_value;
	flag = linmcmin(f, ens, p, new_dir, n, &new_value);
	if (flag != M_FOUND)
	  // printf("dir %d ener %.13g\n", i, new_value);
	  ;
	else if (prev_value - new_value > max_decr) 
	{
	  max_decr = prev_value - new_value;
	  go_down = i;
	}
      }

      if (2 * mcabs(cur_value - new_value) <= tol * (mcabs(cur_value) + mcabs(new_value))) {
	*fmin = new_value;
	return M_FOUND;	/* found minimum. p[] contains the point */
      }
      if (go_down == -1) {
	printf("***ERROR, go_down = -1\n");
	printf("cur_value %f new_value %f max_decr %f\n", cur_value,
	       new_value, max_decr);
	printf("check %f <= %f\n", 2 * mcabs(cur_value - new_value),
	       tol * (mcabs(cur_value) + mcabs(new_value)));
	exit(1);
      }
      /*
       * construct the extrapolated point and the average direction moved
       */
      for (i = 0; i < n; i++) {
	pextrap[i] = 2 * p[i] - pold[i];	/* extrapolated point */
	new_dir[i] = p[i] - pold[i];
	pold[i] = p[i];
      }
      trial_value = f(pextrap,ens);
      if (trial_value >= cur_value)	/* don't use new direction */
	continue;		/* jump to next iteration */

      tmp = (cur_value - new_value - max_decr) * (cur_value - new_value - max_decr);
      tmp = tmp * 2 * (cur_value - 2 * new_value + trial_value);
      tmp = tmp - max_decr *
	(cur_value - trial_value) * (cur_value - trial_value);
      if (tmp < 0.0) {	/* good direction */
	flag = linmcmin(f,ens, p, new_dir, n, &new_value);	/* move along it */
	for (i = 0; i < n; i++)	/* update dir */
	  dir[i][go_down] = new_dir[i];
      }
#ifdef VERBOSE 
      printf("Cost: %g ; Params : ", new_value);
      for (i = 0; i < n; i++) printf("%g ", p[i]);
      printf("\n");
#endif
    }
    /* performed maxiter iterations. Assume not found */

    *fmin = new_value;
    return M_NOT_FOUND;
} // powell()



int32_t powell_num(PFNn f, struct xvimage * image1, struct xvimage * image2, 
		   double p[], int32_t n, double tol, double scale, int32_t maxiter, 
		   double *fmin)
{
    double new_value, cur_value, prev_value, trial_value;
    double max_decr;		/* max. function decrease */
    double tmp;
    int32_t i, j, iter, flag;
    int32_t go_down;		/* direction of descent */

    //    double e, s2, sm2, weff;	/* used for weight() */

    if (n > MAXDIM) {
	printf("***ERROR:powell: Too many parameters. Max = %d\n", MAXDIM);
	exit(-1);
    }
    for (i = 0; i < n; i++)	/* set up scale lenght */
	for (j = 0; j < n; j++)
	    dir[i][j] = ((i == j) ? scale : 0.0);

    new_value = f(p, image1, image2);	/* compute initial value */
    for (i = 0; i < n; i++)	/* save the old point */
	pold[i] = p[i];
    
    for (iter = 0; iter < maxiter; iter++) 
    {
      go_down = -1;		/* impossible direction */
      max_decr = 0.0;
      cur_value = new_value;
      /*
      ** now loop over the direction set and try to minimize along each one
      */
      for (i = 0; i < n; i++) 
      {
	for (j = 0; j < n; j++) new_dir[j] = dir[j][i];
	prev_value = new_value;
	flag = linmin_num(f, image1, image2, p, new_dir, n, &new_value);
	if (flag != M_FOUND)
	  // printf("dir %d ener %.13g\n", i, new_value);
	  ;
	else if (prev_value - new_value > max_decr) 
	{
	  max_decr = prev_value - new_value;
	  go_down = i;
	}
      }

      if (2 * mcabs(cur_value - new_value) <= tol * (mcabs(cur_value) + mcabs(new_value))) {
	*fmin = new_value;
	return M_FOUND;	/* found minimum. p[] contains the point */
      }

      if (go_down == -1) {
//#define DO_NOT_EXIT
#ifdef DO_NOT_EXIT
	return M_FOUND;
#else
	printf("***ERROR, go_down = -1\n");
	printf("cur_value %f new_value %f max_decr %f\n", cur_value,
	       new_value, max_decr);
	printf("check %f <= %f\n", 2 * mcabs(cur_value - new_value),
	       tol * (mcabs(cur_value) + mcabs(new_value)));
	exit(1);
#endif
      }
      /*
       * construct the extrapolated point and the average direction moved
       */
      for (i = 0; i < n; i++) {
	pextrap[i] = 2 * p[i] - pold[i];	/* extrapolated point */
	new_dir[i] = p[i] - pold[i];
	pold[i] = p[i];
      }

      trial_value = f(pextrap, image1, image2);
      if (trial_value >= cur_value)	/* don't use new direction */
	continue;		/* jump to next iteration */

      tmp = (cur_value - new_value - max_decr) * (cur_value - new_value - max_decr);
      tmp = tmp * 2 * (cur_value - 2 * new_value + trial_value);
      tmp = tmp - max_decr *
	(cur_value - trial_value) * (cur_value - trial_value);
      if (tmp < 0.0) {	/* good direction */
	flag = linmin_num(f, image1, image2, p, new_dir, n, &new_value);	/* move along it */
	for (i = 0; i < n; i++)	/* update dir */
	  dir[i][go_down] = new_dir[i];
      }
#ifdef VERBOSE 
      printf("Cost: %g ; Params : ", new_value);
      for (i = 0; i < n; i++) printf("%g ", p[i]);
      printf("\n");
#endif
    }
    /* performed maxiter iterations. Assume not found */

    *fmin = new_value;
    return M_NOT_FOUND;
} // powell_num()



/*-------------------------------------------------------------------*/

#ifdef TEST
#include <math.h>

double test1(double x)
{
    return 3 * x * x + 40 * sin(x);
}

double test2(double x, ensemble *ens)
{
    return 3 * x * x + 40 * sin(x);
}

double test(FUNCe f, double x0, double x1)
{
    double x, x2, fx;

    printf("------------\n");
    if (bracket(f, NULL,  &x0, &x1, &x2) != M_FOUND)
	printf("Can't bracket\n");
    else {
	printf("Bracket x0 %f x1 %f x2 %f \n", x0, x1, x2);

	if (brent(f, NULL, x0, x1, x2, 1e-15, &x, &fx) != M_FOUND)
	    printf("brent Can't find the minimum\n");
	else
	    printf("brent minimum f(%.14f)=%.14f\n", x, fx);
    }
    return x;
}

int32_t main()
{
    double x;

    x = test(test2, 1.8, 1.9);
    printf("min %f\n", x);
    x = test(test2, 0, 1);
    printf("min %f\n", x);

    exit(0);
}

#endif				/* TEST */

#ifdef TEST1

#include <stdio.h>
#include <math.h>

double test1(double x[], ensemble *ens)
{
    return -sin(x[0]) * sin(x[1]) * exp(-x[0] * x[0] + x[0] + 2 + x[1] * x[1]);
}

main()
{
    double fmin, f;
    int32_t i;
    double x[2], d[2];

    x[0] = -3.0;
    x[1] = 3;
    f = test1(x, NULL);
    printf("initial value %.8f\n", f);
    for (i = 0; i < 10; i++) {
	d[0] = 1;
	d[1] = 0;
	linmcmin(test1, NULL, x, d, 2, &fmin);
	printf("i %i x %.13f %.13f d %.6g %.6g f %.13f\n",
	       i, x[0], x[1], d[0], d[1], fmin);
	d[0] = 0;
	d[1] = 1;
	linmcmin(test1, NULL, x, d, 2, &fmin);
	printf("i %i x %.13f %.13f d %.6g %.6g f %.13f\n",
	       i, x[0], x[1], d[0], d[1], fmin);
    }

    printf("x %.13f %.13f d %.6g %.6g f %.13f\n", x[0], x[1], d[0], d[1], fmin);
}
#endif

#ifdef TEST2
#include <math.h>

double test1(double x[], ensemble *ens)
{
    return -sin(x[0] / 2) * sin(x[1] / 2);
}


int32_t main()
{
    double fmin;

    double p[10];

    p[0] = p[1] = 0.1;

    //int32_t powell(FUNCe f, ensemble *ens, double p[], int32_t n, double tol,
    //	         double scale, int32_t maxiter, double *fmin)

    powell(test1, NULL, p, 2, 1e-8, 0.1, 10000, &fmin);
    printf("x1 % .13f x2 % .13f fmin % .13f\n", p[0], p[1], fmin);
    exit(1);
}

#endif
