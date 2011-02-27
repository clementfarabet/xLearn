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
#ifndef _mcgeo_h
#include <mcgeo.h>
#endif

extern int32_t lellipticite(
         struct xvimage *img,    /* image de depart */
         int32_t connex);            /* 4, 8  */

extern int32_t lfenetreincluante(
        struct xvimage *img,    /* image de depart */
        int32_t connex);            /* 4, 8  */

extern int32_t lboxmin(struct xvimage *img);

extern int32_t lmatchrect(
        struct xvimage *img,    /* image de depart */
        int32_t connex,             /* 4, 8  */
        int32_t arrondi);           /* booleen */

extern int32_t lrectangleincluant(
        struct xvimage *img,    /* image de depart */
        int32_t connex);            /* 4, 8  */

extern int32_t lmatchellipse(
        struct xvimage *img,    /* image de depart */
        int32_t connex);            /* 4, 8  */

extern struct xvimage *lcar2pol(struct xvimage *img, double xc, double yc);

extern int32_t lpol2car(struct xvimage *img, struct xvimage *res, double xc, double yc);

extern struct xvimage *lsection(struct xvimage *img, 
                         double x0, double y0, double z0,
                         double x1, double y1, double z1,
                         double x2, double y2, double z2
				);

extern rectangle *AjusteRectangle(int32_t *pb, int32_t npb, double xc, double yc, double theta, double h, double w, int32_t rs);

extern rectangle *RectangleIncluant(int32_t *pb, int32_t npb, double xc, double yc, double theta, int32_t rs);

extern rectanglearrondi *AjusteRectangleArrondi(int32_t *pb, int32_t npb, double xc, double yc, 
						double theta, double h, double w, int32_t rs);

extern cercle *AjusteCercle(struct xvimage *img, double xc, double yc, double r);

extern cercle *AjusteCercle2(struct xvimage *img, double xc, double yc, double r);

#ifdef __cplusplus
}
#endif
