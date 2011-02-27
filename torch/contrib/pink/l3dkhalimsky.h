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

extern int32_t l3dkhalimskize(struct xvimage * i, struct xvimage **k, int32_t mode);
extern int32_t l3dcolor(struct xvimage * k);
extern int32_t l3dthin(struct xvimage * k, int32_t nsteps);
extern int32_t l3dskelsurf(struct xvimage * k, int32_t nsteps);
extern int32_t l3disthmus(struct xvimage * f);
extern int32_t l3dlabel(struct xvimage * f, struct xvimage * lab);
extern int32_t l3drecons(struct xvimage * f, index_t *tab, int32_t n);
extern int32_t l3dsphere(struct xvimage * k, index_t x0, index_t y0, index_t z0, double r);
extern int32_t l3dplane(struct xvimage * k, double a, double b, double c, double d);
extern int32_t l3dinvariants(struct xvimage *f, index_t *nbcc, index_t *nbcav, index_t *nbtun, index_t *euler);
extern int32_t l3dboundary(struct xvimage * f);
extern int32_t l3dborder(struct xvimage * f);
extern int32_t l3dseltype(struct xvimage * k, uint8_t d1, uint8_t d2, uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2);
extern int32_t l3dmakecomplex(struct xvimage * i);
extern int32_t l3dalpha(struct xvimage * i);
extern int32_t l3dbeta(struct xvimage * i);
#ifdef __cplusplus
}
#endif
