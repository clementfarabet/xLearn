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

extern int32_t scn_solvespline(double *x, double *y, int32_t n, 
			    double *Z0, double *Z1, double *Z2, double *Z3);
extern int32_t scn_solvespline_noalloc(double *x, double *y, int32_t n, 
		        double *Z0, double *Z1, double *Z2, double *Z3,
			double *M, double *P, double *z, double *A, double*B);
extern int32_t scn_solvespline1(double *y, int32_t n, 
			     double *Z0, double *Z1, double *Z2, double *Z3);
extern int32_t scn_solveclosedspline(double *x, double *y, int32_t n, 
			          double *Z0, double *Z1, double *Z2, double *Z3);
extern int32_t scn_samplespline(double *x, double *y, int32_t n, int32_t m, double *X, double *Y);
extern int32_t scn_samplespline3d(double *x, double *y, double *z, int32_t n, int32_t m, double *X, double *Y, double *Z);
extern int32_t scn_curvatures(double *x, double *y, int32_t n, int32_t m, double *sk, double *rhok);
extern int32_t scn_curvatures3d(double *x, double *y, double *z, int32_t n, int32_t m, double *sk, double *rhok);
extern int32_t scn_approxcurve(int32_t *X, int32_t *Y, int32_t N, double deltamax, int32_t *Z, int32_t *n, 
                            double *C0, double *C1, double *C2, double *C3,
                            double *D0, double *D1, double *D2, double *D3);
extern int32_t scn_approxcurve3d(int32_t *X, int32_t *Y, int32_t *Z, int32_t N, double deltamax, 
                              int32_t *C, int32_t *n, 
                              double *C0, double *C1, double *C2, double *C3,
                              double *D0, double *D1, double *D2, double *D3,
			      double *E0, double *E1, double *E2, double *E3);
extern double scn_lengthspline(double *X0, double *X1, double *X2, double *X3, 
			       double *Y0, double *Y1, double *Y2, double *Y3, 
			       int32_t nctrl);
extern double scn_lengthspline3d(double *X0, double *X1, double *X2, double *X3, 
				 double *Y0, double *Y1, double *Y2, double *Y3, 
				 double *Z0, double *Z1, double *Z2, double *Z3,
				 int32_t nctrl);
#ifdef __cplusplus
}
#endif
