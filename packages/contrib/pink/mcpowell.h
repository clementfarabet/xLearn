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
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ensemble {
  double *X;
  int32_t n;
  double *Y;
  int32_t m;
  double *Tmp;
} ensemble;

#define SQRT_DBL_EPSILON   1e-8		/* should be >=< sqrt(DBL_EPSILON) */


#define    M_OK  	0
#define    M_FOUND  	0
#define    M_NOT_FOUND	1
#define    M_FAILURE	2
#define    M_MAX_ITER	3
#define    M_LAST_CODE	4

#undef min
#undef max
#define max(a,b) error_max_function_is_ambigous_use_mcmax_instead
#define min(a,b) error_min_function_is_ambigous_use_mcmin_instea
#define abs(b) error_abs_function_is_ambigous_use_mcabs_instead
#define sign(b) error_sign_function_is_ambigous_use_mcsign_instead

  
#undef mcabs
#define mcabs(x)		((x)>=0?(x):-(x))

#undef mcmax
#define mcmax(a,b)        ((a)<(b)?(b):(a))	/* WARNING: side effects */

#undef mcmin
#define mcmin(a,b)        ((a)>(b)?(b):(a))

#undef mcsign
#define mcsign(x)		((x)>0?1.0:((x)<0?-1.0:0.0))

typedef double (*FUNCe)(double, ensemble *);		/* f: R -> R */
typedef double (*FUNCn)(double, struct xvimage *, struct xvimage *);      
typedef double (*PFNe)(double *, ensemble *);		/* f: R^N -> R */
typedef double (*PFNn)(double *, struct xvimage * image1, struct xvimage * image2);

/* prototypes */

extern int32_t powell(PFNe f, ensemble *ens, double p[], int32_t n, double tol,
		  double scale, int32_t maxiter, double *fmin);

extern int32_t powell_num(PFNn f, struct xvimage * image1, 
			  struct xvimage * image2, double p[], int32_t n, 
			  double tol, double scale, int32_t maxiter, 
			  double *fmin);


#ifdef __cplusplus
}
#endif
