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

#ifndef _MCIMAGE_H
#include <mcimage.h>
#endif

#define MCLIN_EPSILON 1E-10

extern double * lin_zeros(int32_t n, int32_t m);
extern double * lin_ones(int32_t n, int32_t m);
extern void lin_printmat(double * mat, int32_t n, int32_t m);
extern double lin_min(double * mat, int32_t n, int32_t m);
extern double lin_max(double * mat, int32_t n, int32_t m);
extern double lin_sum(double * mat, int32_t n, int32_t m);
extern double lin_norme2(double * mat, int32_t n, int32_t m);
extern double lin_normevec2(double * vec, int32_t n);
extern void lin_transpose(double * mat, int32_t n);
extern void lin_copy(double * dest, double * source, int32_t n, int32_t m);
extern double * lin_duplicate(double *A, int32_t n, int32_t m);
extern void lin_add(double * A, double * B, int32_t n, int32_t m);
extern void lin_sub(double * A, double * B, int32_t n, int32_t m);
extern void lin_multscal(double * A, double r, int32_t n, int32_t m);
extern void lin_multpoint(double * A, double * B, int32_t n, int32_t m);
extern void lin_mult(double * A, double * B , double * R, int32_t p, int32_t q, int32_t r);
extern void lin_multAtB(double * A, double * B , double * R, int32_t nA, int32_t mA, int32_t nB, int32_t mB);
extern int32_t lin_decomposition_LUP(double * A, int32_t * pi, int32_t n);
extern int32_t lin_decomposition_cholesky(double * a, double * l, int32_t n);
extern int32_t lin_inverse_gauss(double *TB, double *InvB, int32_t N);
extern void ROTATE(double * a, int32_t i, int32_t j, int32_t k, int32_t l, double tau, double s, int32_t N);
extern int32_t lin_jacobi(double * A, int32_t n, double * D, double * V, int32_t nrot);
extern void lin_permutmat(int32_t * pi, double * P, int32_t n);
extern void lin_LU(double * A, double * L, double * U, int32_t n);
extern void lin_solveLUP(double * LU, int32_t *P, double * b, double * x, int32_t n);
extern int32_t lin_solve(double * A, double * b, double * x, int32_t n);
extern int32_t lin_solvebidiag(double * A, double * b, double * x, int32_t n);
extern int32_t lin_solvetridiag(double * A, double * b, double * x, int32_t n);
extern double lin_det2(double * m);
extern int32_t lin_invmat2(double * ma, double * mr);
extern double lin_det3(double * m);
extern int32_t lin_invmat3(double * ma, double * mr);
extern int32_t lin_inverseLUP(double * A, double * R, int32_t n);
extern int32_t lellipsefit(double * x, double * y, int32_t n, 
                       double *x0, double *y0, double *xa, double *ya,
					   double *xb, double *yb);
extern int32_t lin_trouvemin(double * x, double * d, double (*F)(double *, int32_t), int32_t n, double *precision);
extern int32_t lidentifyline(double *pbx, double *pby, int32_t npb, double *a, double *b);
extern int32_t lidentifyparabola2(double *pbx, double *pby, int32_t npb, double *a, double *b);
extern int32_t lidentifyparabola3(double *pbx, double *pby, int32_t npb, double *a, double *b, double *c);
extern int32_t lidentifyplane(double *pbx, double *pby, double *pbz, index_t npb, double *a, double *b, double *c, double *d, double *error);

#ifdef __cplusplus
}
#endif
