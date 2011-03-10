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

extern int32_t ldist(struct xvimage *img, int32_t connex, struct xvimage *res);
extern int32_t ldistbyte(struct xvimage *img, int32_t connex, struct xvimage *res);
extern int32_t ldistquad(struct xvimage *img, struct xvimage *res);
extern int32_t ldistvect(uint8_t *F, vect2Dint *L, index_t rs, index_t cs);
extern int32_t ldisteuc(struct xvimage *img, struct xvimage *res);
extern int32_t ldilatdisc(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lerosdisc(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lopendisc(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lclosedisc(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lchamfrein(struct xvimage *img, struct xvimage *res);
extern int32_t ldistquad3d(struct xvimage* ob, struct xvimage* res);
extern int32_t ldisteuc3d(struct xvimage* ob, struct xvimage* res);
extern int32_t ldilatball(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lerosball(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lopenball(struct xvimage* ob, int32_t r, int32_t mode);
extern int32_t lcloseball(struct xvimage* ob, int32_t r, int32_t mode);
extern struct xvimage* ldilatdiscloc(struct xvimage* f, int32_t mode);
extern struct xvimage* ldilatballloc(struct xvimage* f, int32_t mode);
extern int32_t ldistquadSaito(struct xvimage *img, struct xvimage *res);
extern int32_t ldistSaito(struct xvimage *img, struct xvimage *res);
extern int32_t ldistMeijster(struct xvimage *img, struct xvimage *res);
extern struct xvimage* lredt2d(struct xvimage* f);
extern struct xvimage* lredt3d(struct xvimage* f);
extern int32_t lsedt_meijster(struct xvimage *img, struct xvimage *res);
extern void SEDT_line(uint8_t *f, uint32_t *g, index_t rs, index_t cs);
extern void SEDT_column(uint32_t *f, uint32_t *g, index_t rs, index_t cs);
extern void SEDT3d_line(uint8_t *f, uint32_t *g, index_t rs, index_t cs, index_t ds);
extern void SEDT3d_column(uint32_t *f, uint32_t *g, index_t rs, index_t cs, index_t ds);
extern void SEDT3d_planes(uint32_t *f, uint32_t *g, index_t rs, index_t cs, index_t ds);
extern int32_t lskeleton_ST(struct xvimage* f, struct xvimage* res);
extern struct xvimage * lopeningfunction(struct xvimage *img, int32_t mode);
  extern float ldistsets(struct xvimage *img1, struct xvimage *img2, int32_t mode, float cut);
#ifdef __cplusplus
}
#endif
