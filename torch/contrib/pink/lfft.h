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

/**************************************************************
 * kube_fft.h
 *
 * Forward and inverse discrete 2D Fourier transforms.
 *
 * This software is in the public domain.
 *
 **************************************************************/

typedef complex COMPLEX;
typedef dcomplex DCOMPLEX;

#ifndef ERROR
#define ERROR -1
#define NO_ERROR 0
#endif

#define FFT_FORWARD	0
#define FFT_INVERSE 1

#define PI      3.1415926535897932
#define TWOPI   6.2831853071795865 /* 2.0 * PI */
#define HALFPI  1.5707963267948966 /* PI / 2.0 */
#define PI8 	0.392699081698724 /* PI / 8.0 */
#define RT2 	1.4142135623731  /* sqrt(2.0) */
#define IRT2 	0.707106781186548  /* 1.0/sqrt(2.0) */

/* Perform forward 2D transform on a COMPLEX array. */
extern int forward_fft2f(COMPLEX *array, int rows, int cols);

/* Perform inverse 2D transform on a COMPLEX array. */
extern int inverse_fft2f(COMPLEX *array, int rows, int cols);

/* Perform forward 2D transform on a DCOMPLEX array. */
extern int forward_fft2d(DCOMPLEX *array, int rows, int cols);

/* Perform inverse 2D transform on a DCOMPLEX array. */
extern int inverse_fft2d(DCOMPLEX *array, int rows, int cols);

extern int32_t lfft(struct xvimage *image, int32_t dir);
extern int32_t lfft2(struct xvimage *image1, struct xvimage *image2, int32_t dir);
#ifdef __cplusplus
}
#endif
