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

#define UNDEFINED    0

#define PEAK         1
#define PLATEAU      2
#define WELL         3
#define CONST_FLOOR  4
#define CONV_FLOOR   5
#define DEST_CEILING 6
#define CONST_DEST   7
#define CONV_DEST    8
#define DIV_CEILING  9
#define DIV_CONST    10
#define SADDLE       11

#define FLOOR        12
#define DEST         13
#define DIV          14
#define CEILING      15
#define CONST        16
#define CONV         17

#define SIMPLE       1
#define MULTIPLE     2
#define SEPARANT     3
#define ISOLE        4
#define INTERIEUR    5

extern void nbtopo(uint8_t *img, index_t p, index_t rs, index_t N, 
                   int32_t *t4mm, int32_t *t4m, int32_t *t8p, int32_t *t8pp);
extern void nbtopo2(uint8_t *img, index_t p, index_t rs, index_t N,
                    int32_t *t8mm, int32_t *t8m, int32_t *t4p, int32_t *t4pp);
extern void nbtopoh(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N,
                    int32_t *t4mm, int32_t *t4m, int32_t *t8p, int32_t *t8pp);
extern void nbtopoh2(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N,
                    int32_t *t8mm, int32_t *t8m, int32_t *t4p, int32_t *t4pp);
extern void nbtopoh_l(int32_t *img, index_t p, int32_t h, index_t rs, index_t N,
                    int32_t *t4mm, int32_t *t4m, int32_t *t8p, int32_t *t8pp);
extern void nbtopoh2_l(int32_t *img, index_t p, int32_t h, index_t rs, index_t N,
                    int32_t *t8mm, int32_t *t8m, int32_t *t4p, int32_t *t4pp);
extern int32_t t8pp(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t8pp_l(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t t8p(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t4m(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t4mm(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t4pp(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t4pp_l(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t t4p(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t8m(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t t8mm(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t alpha8m(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t alpha4m(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t alpha8p(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t alpha4p(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t calpha8m(uint8_t *img, uint8_t *cnd, index_t p, index_t rs, index_t N);
extern uint8_t calpha4m(uint8_t *img, uint8_t *cnd, index_t p, index_t rs, index_t N);
extern uint8_t calpha8p(uint8_t *img, uint8_t *cnd, index_t p, index_t rs, index_t N);
extern uint8_t calpha4p(uint8_t *img, uint8_t *cnd, index_t p, index_t rs, index_t N);
extern uint8_t halpha8m(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N);
extern uint8_t halpha4m(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N);
extern uint8_t halpha8p(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N);
extern uint8_t halpha4p(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N);
extern int32_t alpha8m_l(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t alpha4m_l(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t alpha8p_l(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t alpha4p_l(int32_t *img, index_t p, index_t rs, index_t N);
extern uint8_t beta8m(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t beta4m(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t beta8p(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t beta4p(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t typetopo(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t typetopo8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t typetopobin(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t typetopobin8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pdestr(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pconstr(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pdestr4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pconstr4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pdestr8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pconstr8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t peakordestr4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t wellorconstr4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t peakordestr8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t wellorconstr8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t peak4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t well4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t peak8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t well8(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t delta4m(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t delta8m(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t delta4p(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t delta8p(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t lambdadestr4(uint8_t *F, index_t p, int32_t lambda, index_t rs, index_t N);
extern int32_t lambdaconstr4(uint8_t *F, index_t p, int32_t lambda, index_t rs, index_t N);
extern int32_t lambdadestr8(uint8_t *F, index_t p, int32_t lambda, index_t rs, index_t N);
extern int32_t lambdaconstr8(uint8_t *F, index_t p, int32_t lambda, index_t rs, index_t N);
extern int32_t pconv4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pconv8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pdiv4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t pdiv8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t saddle4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t saddle8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvois8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvois4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvoisc8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvoisc4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvois8neq(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvois4neq(int32_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvoisp8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvoisp4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvoism8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t nbvoism8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t extremite8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t extremite4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t extremite8m(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t extremite4m(uint8_t *img, index_t p, index_t rs, index_t N);
extern void top4(uint8_t *img, index_t p, index_t rs, index_t N, int32_t *t, int32_t *tb);
extern void top8(uint8_t *img, index_t p, index_t rs, index_t N, int32_t *t, int32_t *tb);
extern void top4_l(int32_t *img, index_t p, index_t rs, index_t N, int32_t *t, int32_t *tb);
extern void top8_l(int32_t *img, index_t p, index_t rs, index_t N, int32_t *t, int32_t *tb);
extern int32_t simple8(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t simple4(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t mask(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t maskmm(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t maskpp(uint8_t *img, index_t p, index_t rs, index_t N);
extern uint8_t maskmmh(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N);
extern uint8_t maskpph(uint8_t *img, index_t p, uint8_t h, index_t rs, index_t N);
extern int32_t t8(int32_t v);
extern int32_t t8b(int32_t v);
extern int32_t t4(int32_t v);
extern int32_t t4b(int32_t v);
extern int32_t ridge(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t bordext4(uint8_t *F, index_t x, index_t rs, index_t N);
extern int32_t bordext8(uint8_t *F, index_t x, index_t rs, index_t N);
extern int32_t curve4(uint8_t *img, index_t p, index_t rs, index_t N);
extern int32_t curve8(uint8_t *img, index_t p, index_t rs, index_t N);

extern int32_t separant4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N);                      /* taille image */
extern int32_t separant4h(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  index_t rs,                      /* taille rangee */
  index_t N);                      /* taille image */
extern int32_t hseparant4(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* parametre */
  index_t rs,                      /* taille rangee */
  index_t N);                      /* taille image */
extern int32_t separant8(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N);                      /* taille image */
extern int32_t museparant4(  
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N,                       /* taille image */ 
  int32_t mu);
extern int32_t nbtrans8(  
  uint8_t *F,            /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t N);                      /* taille image */ 
#ifdef __cplusplus
}
#endif
