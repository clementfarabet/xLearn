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
#include "mcliste.h"
typedef struct {
  int32_t rs, cs;
  int32_t *xmin;
  int32_t *xmax;
} ellipse;

#define InEllipse(ell,x,y) (((x)>=(ell)->xmin[(y)])&&((x)<=(ell)->xmax[(y)]))

extern void      lbresen(uint8_t *I, int32_t rs, int32_t Ax, int32_t Ay, int32_t Bx, int32_t By);
extern void      lbresenlist(int32_t Ax, int32_t Ay, int32_t Bx, int32_t By, int32_t *lx, int32_t *ly, int32_t *n);
extern ellipse * AllocEllipseList(int32_t rs, int32_t cs);
extern void      InitEllipseList(ellipse * ell);
extern void      FreeEllipseList(ellipse *ell);
extern void      lconic(uint8_t *I, int32_t rs, int32_t cs, int32_t xs, int32_t ys, int32_t xe, int32_t ye,
                      int32_t A, int32_t B, int32_t C, int32_t D, int32_t E, int32_t F, int32_t xoffset, int32_t yoffset);
extern void      lellipsearc(uint8_t *I, int32_t rs, int32_t cs,
                      int32_t xp, int32_t yp, int32_t xq, int32_t yq, int32_t xoffset, int32_t yoffset);
extern void      lellipse(uint8_t *I, int32_t rs, int32_t cs, 
                      int32_t xp, int32_t yp, int32_t xq, int32_t yq, int32_t xoffset, int32_t yoffset);
extern void      lellipsearclist(ellipse *ell, int32_t rs, int32_t cs,
                      int32_t xp, int32_t yp, int32_t xq, int32_t yq, int32_t xoffset, int32_t yoffset);
extern void      lellipselist(ellipse *ell, int32_t rs, int32_t cs, 
                      int32_t xp, int32_t yp, int32_t xq, int32_t yq, int32_t xoffset, int32_t yoffset);
extern void lplotfilledellipse(ellipse *ell, uint8_t *I);
extern void lellipsearcliste(Liste *lp, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
			     int32_t xoffset, int32_t yoffset);
extern Liste *lellipseliste(int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
			    int32_t xoffset, int32_t yoffset);
extern void lellipseliste2(Liste *lp, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
			    int32_t xoffset, int32_t yoffset);
extern void lplotellipseliste(Liste *lp, uint8_t *I);
#ifdef __cplusplus
}
#endif
