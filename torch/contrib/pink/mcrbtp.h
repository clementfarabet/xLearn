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
#define RBTP_Black 0
#define RBTP_Red   1

typedef struct {
  double x;
  double y;
  double z;
} TypRbtpKey;
typedef int32_t TypRbtpAuxData;

#define EQUALKEY(j,k) ((j.x==k.x)&&(j.y==k.y)&&(j.z==k.z))
#define LESSKEY(j,k) ((j.x<k.x) || ((j.x==k.x)&&(j.y<k.y)) || ((j.x==k.x)&&(j.y==k.y)&&(j.z<k.z)))
#define COPYKEY(j,k) {j.x=k.x;j.y=k.y;j.z=k.z;}
#define PRINTKEY(j) printf("%g %g %g",j.x,j.y,j.z)

typedef struct RBTPELT {
  TypRbtpAuxData auxdata;
  TypRbtpKey key;
  char color;
  struct RBTPELT * left;
  struct RBTPELT * right;
  struct RBTPELT * parent;
} RbtpElt;

typedef struct {
  int32_t max;             /* taille max du rbtp (en nombre de points) */
  int32_t util;            /* nombre de points courant dans le rbtp */
  int32_t maxutil;         /* nombre de points utilises max (au cours du temps) */
  RbtpElt *root;        /* racine de l'arbre */
  RbtpElt *nil;         /* sentinelle et element dont l'adresse joue le role de NIL */
  RbtpElt *libre;       /* pile des cellules libres */
  RbtpElt elts[1];      /* tableau des elements physiques */
} Rbtp;

/* ============== */
/* prototypes     */
/* ============== */

extern Rbtp * CreeRbtpVide(
  int32_t taillemax);

extern void RbtpFlush(
  Rbtp * T);

extern int32_t RbtpVide(
  Rbtp * T);

extern void RbtpTermine(
  Rbtp * T);

extern void RbtpPrint(
  Rbtp * T);

extern RbtpElt * RbtpSearch(
  Rbtp * T, TypRbtpKey k);

extern RbtpElt * RbtpMinimum(
  Rbtp * T, RbtpElt * x);

extern RbtpElt * RbtpMaximum(
  Rbtp * T, RbtpElt * x);

extern RbtpElt * RbtpSuccessor(
  Rbtp * T, RbtpElt * x);

extern RbtpElt * RbtpInsert(
  Rbtp ** T, TypRbtpKey k, TypRbtpAuxData d);

extern void RbtpDelete(
  Rbtp * T, RbtpElt * z);

extern TypRbtpAuxData RbtpPopMin(
  Rbtp * T);

extern TypRbtpAuxData RbtpPopMax(
  Rbtp * T);

extern TypRbtpKey RbtpMinLevel(
  Rbtp * T);
#ifdef __cplusplus
}
#endif
