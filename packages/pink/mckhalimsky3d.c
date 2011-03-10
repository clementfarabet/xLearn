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
/* 
   Librairie mckhalimsky3d

   Michel Couprie  --  novembre 1998

   Les beta-terminaux de la grille de khalimski sont codees par les points
   dont les trois coordonnees sont impaires, les alpha-terminaux (singletons)
   par les points dont les trois coordonnees sont paires.


   Pour les fonctions XAlphacarre3d, etc, 
   le codage du alpha-voisinage est le suivant pour un cube:

                               22          16          24
                              6           2           8
                           18          14          20

                               11           5          13
                              0           .           1
                           10           4          12

                               23          17          25
                              7           3           9
                           19          15          21

   et pour un carre:

     4   2   6

     0   .   1

     5   3   7

   et pour un intervalle:

     0   .   1

  update janvier 2008 - simple_26_att pairesimple_26_*

*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <bdd.h>
#include <mckhalimsky3d.h>

/*
#define TEST_SIMPLICITE_RECURSIF
*/
#define TEST_SIMPLICITE_TABULE

/*
#define DEBUGCTC
#define DEBUGNEW
#define DEBUGES
#define DEBUGCS
#define DEBUGP0CA
#define DEBUGNP
#define DEBUGELL
#define DEBUGECS
#define DEBUGCD
*/

uint8_t CxAlpha2s[256] = {
0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0
};

uint8_t CyAlpha2s[256] = {
0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0
};

uint8_t CzAlpha2s[256] = {
0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0
};

uint8_t IxAlpha2s[4] = {0, 1, 1, 0};

uint8_t IyAlpha2s[4] = {0, 1, 1, 0};

uint8_t IzAlpha2s[4] = {0, 1, 1, 0};

uint8_t IxBeta2s[256] = {
0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0
};

uint8_t IyBeta2s[256] = {
0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0
};

uint8_t IzBeta2s[256] = {
0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0
};

uint8_t CxBeta2s[4] = {0, 1, 1, 0};

uint8_t CyBeta2s[4] = {0, 1, 1, 0};

uint8_t CzBeta2s[4] = {0, 1, 1, 0};

/* en global : la pile des grilles pour l'allocation "maison" */

#define NBGRILLES3D 12

static struct xvimage * PileGrilles3d[NBGRILLES3D];
static int32_t IndexPileGrilles3d;

/* ========================================== */
void InitPileGrilles3d()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < NBGRILLES3D; i++)
    PileGrilles3d[i] = allocimage(NULL, GRS3D, GCS3D, GDS3D, VFF_TYP_1_BYTE);
  IndexPileGrilles3d = 0;
} /* InitPileGrilles3d() */

/* ========================================== */
void TerminePileGrilles3d()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < NBGRILLES3D; i++)
    freeimage(PileGrilles3d[i]);
} /* TerminePileGrilles3d() */

/* ========================================== */
struct xvimage * AllocGrille3d()
/* ========================================== */
{
  struct xvimage * g;
  if (IndexPileGrilles3d >= NBGRILLES3D)
  {   
    fprintf(stderr, "AllocGrille3d() : pile pleine\n");
    exit(0);
  }
  g = PileGrilles3d[IndexPileGrilles3d];
  IndexPileGrilles3d++;
  return g;
} /* AllocGrille3d() */

/* ========================================== */
void FreeGrille3d()
/* ========================================== */
{
  if (IndexPileGrilles3d <= 0)
  {   
    fprintf(stderr, "FreeGrille3d() : pile vide\n");
    exit(0);
  }
  IndexPileGrilles3d --;
} /* FreeGrille3d() */

/* ========================================================================== */
/* FONCTIONS POUR LE 3D */
/* ========================================================================== */

/* ==================================== */
struct xvimage * Khalimskize3d(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale
   b: resultat - chaque pixel de o est devenu un beta-terminal de b
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  index_t ods = depth(o);
  index_t ops = ors * ocs;
  uint8_t *O = UCHARDATA(o);
  struct xvimage *b;
  index_t brs, bcs, bds, bps, bN;
  uint8_t *B;
  index_t i, j, k;

  brs = 2 * ors + 1;
  bcs = 2 * ocs + 1;
  bds = 2 * ods + 1;
  bps = brs * bcs;
  bN = bps * bds;
  
  b = allocimage(NULL, brs, bcs, bds, VFF_TYP_1_BYTE);
  if (b == NULL)
  {   fprintf(stderr,"Khalimskize3d() : malloc failed\n");
      return NULL;
  }
  B = UCHARDATA(b);

  memset(B, VAL_NULLE, bN); /* init a VAL_NULLE */

  for (k = 0; k < ods; k++)
    for (j = 0; j < ocs; j++)
      for (i = 0; i < ors; i++)
        if (O[k * ops + j * ors + i])
          B[(2*k+1) * bps + (2*j+1) * brs + (2*i+1)] = VAL_OBJET;

  return b;
} /* Khalimskize3d() */

/* ==================================== */
struct xvimage * KhalimskizeNDG3d(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale
   b: resultat - chaque pixel de o est devenu un beta-terminal de b
                 et sa valeur de gris a ete transmise. les ndg des
                 autres points sont a 0.
*/
{
#undef F_NAME
#define F_NAME "KhalimskizeNDG3d"
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  index_t ods = depth(o);
  index_t ops = ors * ocs;
  struct xvimage *b;
  index_t brs, bcs, bds, bps, bN;
  index_t i, j, k;

  brs = 2 * ors + 1;
  bcs = 2 * ocs + 1;
  bds = 2 * ods + 1;
  bps = brs * bcs;
  bN = bps * bds;
  
  b = allocimage(NULL, brs, bcs, bds, datatype(o));
  if (b == NULL)
  {   fprintf(stderr,"%s: malloc failed\n", F_NAME);
      return NULL;
  }

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *O = UCHARDATA(o);
    uint8_t *B = UCHARDATA(b);
    memset(B, 0, bN*sizeof(uint8_t));
    for (k = 0; k < ods; k++)
      for (j = 0; j < ocs; j++)
	for (i = 0; i < ors; i++)
	  B[(2*k+1) * bps + (2*j+1) * brs + (2*i+1)] = O[k * ops + j * ors + i];
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *O = SLONGDATA(o);
    int32_t *B = SLONGDATA(b);
    memset(B, 0, bN*sizeof(uint32_t));
    for (k = 0; k < ods; k++)
      for (j = 0; j < ocs; j++)
	for (i = 0; i < ors; i++)
	  B[(2*k+1) * bps + (2*j+1) * brs + (2*i+1)] = O[k * ops + j * ors + i];
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *O = FLOATDATA(o);
    float *B = FLOATDATA(b);
    memset(B, 0, bN*sizeof(float));
    for (k = 0; k < ods; k++)
      for (j = 0; j < ocs; j++)
	for (i = 0; i < ors; i++)
	  B[(2*k+1) * bps + (2*j+1) * brs + (2*i+1)] = O[k * ops + j * ors + i];
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
  return b;
} /* KhalimskizeNDG3d() */

/* ==================================== */
void ndgmin3d(struct xvimage *b)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "ndgmin3d"
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  index_t ps = rs * cs;
  index_t N = ps * ds;
  struct xvimage *bp;
  index_t i, j, k;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *B;
    uint8_t *BP;
    B = UCHARDATA(b);
    bp = copyimage(b);
    BP = UCHARDATA(bp);
    memset(BP, NDG_MAX, N);

    for (k = 1; k < ds; k += 2)
      for (j = 1; j < cs; j += 2)
	for (i = 1; i < rs; i += 2)
	{
	  BP[k * ps + j * rs + i] = B[k * ps + j * rs + i];
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) BP[tab[u]] = mcmin(BP[tab[u]],B[k * ps + j * rs + i]);
	}
    memcpy(B, BP, N*sizeof(uint8_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *B;
    int32_t *BP;
    B = SLONGDATA(b);
    bp = copyimage(b);
    BP = SLONGDATA(bp);
    for (k = 1; k < N; k += 1) BP[k] = INT32_MAX;

    for (k = 1; k < ds; k += 2)
      for (j = 1; j < cs; j += 2)
	for (i = 1; i < rs; i += 2)
	{
	  BP[k * ps + j * rs + i] = B[k * ps + j * rs + i];
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) BP[tab[u]] = mcmin(BP[tab[u]],B[k * ps + j * rs + i]);
	}
    memcpy(B, BP, N*sizeof(uint32_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *B;
    float *BP;
    B = FLOATDATA(b);
    bp = copyimage(b);
    BP = FLOATDATA(bp);
    for (k = 1; k < N; k += 1) BP[k] = FLT_MAX;

    for (k = 1; k < ds; k += 2)
      for (j = 1; j < cs; j += 2)
	for (i = 1; i < rs; i += 2)
	{
	  BP[k * ps + j * rs + i] = B[k * ps + j * rs + i];
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) BP[tab[u]] = mcmin(BP[tab[u]],B[k * ps + j * rs + i]);
	}
    memcpy(B, BP, N*sizeof(float));
    freeimage(bp);
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }

} /* ndgmin3d() */

/* ==================================== */
void ndgmax3d(struct xvimage *b)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "ndgmax3d"
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  index_t ps = rs * cs;
  index_t N = ps * ds;
  struct xvimage *bp;
  index_t i, j, k;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *B;
    uint8_t *BP;
    B = UCHARDATA(b);
    bp = copyimage(b);
    BP = UCHARDATA(bp);
    memset(BP, NDG_MIN, N);

    for (k = 1; k < ds; k += 2)
      for (j = 1; j < cs; j += 2)
	for (i = 1; i < rs; i += 2)
	{
	  BP[k * ps + j * rs + i] = B[k * ps + j * rs + i];
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) BP[tab[u]] = mcmax(BP[tab[u]],B[k * ps + j * rs + i]);
	}
    memcpy(B, BP, N*sizeof(uint8_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *B;
    int32_t *BP;
    B = SLONGDATA(b);
    bp = copyimage(b);
    BP = SLONGDATA(bp);
    for (k = 1; k < N; k += 1) BP[k] = 0;

    for (k = 1; k < ds; k += 2)
      for (j = 1; j < cs; j += 2)
	for (i = 1; i < rs; i += 2)
	{
	  BP[k * ps + j * rs + i] = B[k * ps + j * rs + i];
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) BP[tab[u]] = mcmax(BP[tab[u]],B[k * ps + j * rs + i]);
	}
    memcpy(B, BP, N*sizeof(uint32_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *B;
    float *BP;
    B = FLOATDATA(b);
    bp = copyimage(b);
    BP = FLOATDATA(bp);
    for (k = 1; k < N; k += 1) BP[k] = 0.0;

    for (k = 1; k < ds; k += 2)
      for (j = 1; j < cs; j += 2)
	for (i = 1; i < rs; i += 2)
	{
	  BP[k * ps + j * rs + i] = B[k * ps + j * rs + i];
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) BP[tab[u]] = mcmax(BP[tab[u]],B[k * ps + j * rs + i]);
	}
    memcpy(B, BP, N*sizeof(float));
    freeimage(bp);
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
} /* ndgmax3d() */

/* ==================================== */
void ndgmoy3d(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H3 dans R^+ dont
          seules les valeurs des beta-terminaux (cubes) sont significatives.
  Sortie: une fonction kp de H3 dans R^+.
          Tous les points x non beta-terminaux 
          ont recu la valeur moy{k[y], y beta-terminal dans betacarre[x]}
*/
{
#undef F_NAME
#define F_NAME "ndgmoy3d"
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  index_t ds = depth(k);
  index_t ps = rs * cs;
  index_t x, y, z;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (datatype(k) == VFF_TYP_1_BYTE)
  {
    uint8_t *K = UCHARDATA(k);
    uint32_t nb;
    uint32_t sum;

    for (z = 0; z < ds; z++)
      for (y = 0; y < cs; y++)
	for (x = 0; x < rs; x++)
	  if (!CUBE3D(x,y,z))
	  {
	    nb = sum = 0;
	    Betacarre3d(rs, cs, ds, x, y, z, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (CUBE3D((tab[u]%rs),((tab[u]%ps)/rs),(tab[u]/ps)))
	      {
		sum += K[tab[u]];
		nb++;
	      }
	    K[z * ps + y * rs + x] = (uint8_t)(sum/nb);
	  }
  }
  else if (datatype(k) == VFF_TYP_4_BYTE)
  {
    int32_t *K = SLONGDATA(k);
    int32_t nb;
    int32_t sum;

    for (z = 0; z < ds; z++)
      for (y = 0; y < cs; y++)
	for (x = 0; x < rs; x++)
	  if (!CUBE3D(x,y,z))
	  {
	    nb = sum = 0;
	    Betacarre3d(rs, cs, ds, x, y, z, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (CUBE3D((tab[u]%rs),((tab[u]%ps)/rs),(tab[u]/ps)))
	      {
		sum += K[tab[u]];
		nb++;
	      }
	    K[z * ps + y * rs + x] = (uint32_t)(sum/nb);
	  }
  }
  else if (datatype(k) == VFF_TYP_FLOAT)
  {
    float *K = FLOATDATA(k);
    uint32_t nb;
    float sum;

    for (z = 0; z < ds; z++)
      for (y = 0; y < cs; y++)
	for (x = 0; x < rs; x++)
	  if (!CUBE3D(x,y,z))
	  {
	    nb = sum = 0;
	    Betacarre3d(rs, cs, ds, x, y, z, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (CUBE3D((tab[u]%rs),((tab[u]%ps)/rs),(tab[u]/ps)))
	      {
		sum += K[tab[u]];
		nb++;
	      }
	    K[z * ps + y * rs + x] = (float)(sum/nb);
	  }
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }

} /* ndgmoy3d() */

/* ==================================== */
struct xvimage * DeKhalimskize3d(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale dans la grille de Khalimsky
   r: resultat - chaque beta-terminal de k devient un voxel de r
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  index_t ods = depth(o);
  index_t ops = ors * ocs;
  uint8_t *O = UCHARDATA(o);
  struct xvimage *r;
  index_t rrs, rcs, rds, rps, rN;
  uint8_t *R;
  index_t i, j, k;  

  rrs = ors / 2;
  rcs = ocs / 2;
  rds = ods / 2;
  rps = rrs * rcs;
  rN = rps * rds;
  
  r = allocimage(NULL, rrs, rcs, rds, VFF_TYP_1_BYTE);
  if (r == NULL)
  {   fprintf(stderr,"DeKhalimskize3d : allocimage failed\n");
      return NULL;
  }
  R = UCHARDATA(r);

  memset(R, VAL_NULLE, rN); /* init a VAL_NULLE */

  for (k = 1; k < ods; k += 2)
    for (j = 1; j < ocs; j += 2)
      for (i = 1; i < ors; i += 2)
	R[(k/2) * rps + (j/2) * rrs + (i/2)] = O[k*ops + j*ors + i];
  // version "binaire":
      //      if (O[j * ors + i])
      //        R[(j/2) * rrs + (i/2)] = VAL_OBJET;

  return r;
} /* DeKhalimskize3d() */

/* ==================================== */
void DeKhalimskize3dOLD(struct xvimage *b, struct xvimage * r)
/* ==================================== */
/*            
   b: image originale dans la grille de Khalimsky
   r: resultat
   sous-echantillonne en gardant seulement les valeurs des beta-terminaux
   (cubes, 3 coord. impaires)
*/
{
  index_t brs = rowsize(b);
  index_t bcs = colsize(b);
  index_t bds = depth(b);
  index_t bps = brs * bcs;
  index_t rrs = rowsize(r);
  index_t rcs = colsize(r);
  index_t rds = depth(r);
  index_t rps = rrs * rcs;
  index_t rN = rps * rds;
  uint8_t *B = UCHARDATA(b);
  uint8_t *R = UCHARDATA(r);
  index_t i, j, k;

  if ((rrs != brs / 2) || (rcs != bcs / 2) || (rds != bds / 2))
  {
    fprintf(stderr, "DeKhalimskize3d: incompatible sizes\n");
    exit(0);
  }

  memset(R, VAL_NULLE, rN); /* init a VAL_NULLE */

  for (k = 0; k < rds; k++)
    for (j = 0; j < rcs; j++)
      for (i = 0; i < rrs; i++)
      {
        R[k * rps + j * rrs + i] = B[(k+k+1) * bps + (j+j+1) * brs + (i+i+1)];
//printf("%d %d %d [%d] <- %d %d %d [%d]\n", 
	//       k, j, i, k * rps + j * rrs + i, 
	//       (k+k+1), (j+j+1), (i+i+1), (k+k+1) * bps + (j+j+1) * brs + (i+i+1));
      }
} /* DeKhalimskize3d() */

/* ==================================== */
void Dual3d(struct xvimage *bd, struct xvimage *b)
/* ==================================== */
/*            
   b: image originale
   bd: resultat - dual de b
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  index_t ps = rs * cs;
  uint8_t *B = UCHARDATA(b);
  uint8_t *BD = UCHARDATA(bd);
  index_t bdrs = rs+1;
  index_t bdps = bdrs * (cs+1);
  index_t i, j, k;

  if ((rowsize(bd) != rs + 1) || (rowsize(bd) != rs + 1) || (rowsize(bd) != rs + 1))
  {   fprintf(stderr,"Dual3d() : incompatible sizes\n");
      exit(0);
  }

  memset(BD, VAL_NULLE, (rs+1)*(cs+1)*(ds+1)); /* init a VAL_NULLE */

  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        BD[(k+1) * bdps + (j+1) * bdrs + (i+1)] = B[k*ps + j*rs + i];
} /* Dual3d() */

/* ==================================== */
void InitKhalimsky3d(struct xvimage *b, uint32_t c)
/* ==================================== */
/*            
   c : config 3x3x3 codee sur 27 bits
   k: resultat - chaque pixel de c est devenu un beta-terminal de k
*/
{
  index_t brs = rowsize(b);
  index_t bcs = colsize(b);
  index_t bds = depth(b);
  index_t bps = brs * bcs;
  index_t bN = bps * bds;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k, ii, jj, kk;
  int32_t n;

  memset(B, VAL_NULLE, bN); /* init a VAL_NULLE */

  for (n = 0; n < GRS3D*GCS3D*GDS3D; n++)
  {
    i = n % GRS3D;
    j = (n % (GRS3D*GCS3D)) / GRS3D;
    k = n / (GRS3D*GCS3D);
    ii = 2 * i + 1;
    jj = 2 * j + 1;
    kk = 2 * k + 1;
    B[kk*bps + jj*brs + ii] = ((c&1)?VAL_OBJET:VAL_NULLE);
    c = c >> 1;
  } /* for n */
} /* InitKhalimsky3d() */

/* ==================================== */
void Alphacarre3d(index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab l'alpha-adherence de (i,j,k) privee de (i,j,k), dans H3.
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  index_t ps = rs * cs;
#ifdef DEBUG
printf("Alphacarre3d\n");
#endif
  *n = 0;
  if (i % 2)
  {
    if (i>0)      tab[(*n)++] = k*ps + j*rs + i-1;      
    if ((i+1)<rs) tab[(*n)++] = k*ps + j*rs + i+1;      
  }
  if (j % 2)
  {
    if (j>0)      tab[(*n)++] = k*ps + (j-1)*rs + i;      
    if ((j+1)<cs) tab[(*n)++] = k*ps + (j+1)*rs + i;      
  }
  if (k % 2)
  {
    if (k>0)      tab[(*n)++] = (k-1)*ps + j*rs + i;      
    if ((k+1)<ds) tab[(*n)++] = (k+1)*ps + j*rs + i;      
  }
  if ((i%2) + (j%2) == 2)
  {
    if ((i>0)&&(j>0))           tab[(*n)++] = k*ps + (j-1)*rs + i-1;      
    if ((i>0)&&((j+1)<cs))      tab[(*n)++] = k*ps + (j+1)*rs + i-1;      
    if (((i+1)<rs)&&(j>0))      tab[(*n)++] = k*ps + (j-1)*rs + i+1;
    if (((i+1)<rs)&&((j+1)<cs)) tab[(*n)++] = k*ps + (j+1)*rs + i+1;
  }
  if ((i%2) + (k%2) == 2)
  {
    if ((i>0)&&(k>0))           tab[(*n)++] = (k-1)*ps + j*rs + i-1;      
    if ((i>0)&&((k+1)<ds))      tab[(*n)++] = (k+1)*ps + j*rs + i-1;      
    if (((i+1)<rs)&&(k>0))      tab[(*n)++] = (k-1)*ps + j*rs + i+1;
    if (((i+1)<rs)&&((k+1)<ds)) tab[(*n)++] = (k+1)*ps + j*rs + i+1;
  }
  if ((k%2) + (j%2) == 2)
  {
    if ((k>0)&&(j>0))           tab[(*n)++] = (k-1)*ps + (j-1)*rs + i;      
    if ((k>0)&&((j+1)<cs))      tab[(*n)++] = (k-1)*ps + (j+1)*rs + i;      
    if (((k+1)<ds)&&(j>0))      tab[(*n)++] = (k+1)*ps + (j-1)*rs + i;
    if (((k+1)<ds)&&((j+1)<cs)) tab[(*n)++] = (k+1)*ps + (j+1)*rs + i;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if ((i>0)&&(j>0)&&(k>0))                tab[(*n)++] = (k-1)*ps + (j-1)*rs + i-1;
    if ((i>0)&&((j+1)<cs)&&(k>0))           tab[(*n)++] = (k-1)*ps + (j+1)*rs + i-1;
    if (((i+1)<rs)&&(j>0)&&(k>0))           tab[(*n)++] = (k-1)*ps + (j-1)*rs + i+1;
    if (((i+1)<rs)&&((j+1)<cs)&&(k>0))      tab[(*n)++] = (k-1)*ps + (j+1)*rs + i+1;
    if ((i>0)&&(j>0)&&((k+1)<ds))           tab[(*n)++] = (k+1)*ps + (j-1)*rs + i-1;
    if ((i>0)&&((j+1)<cs)&&((k+1)<ds))      tab[(*n)++] = (k+1)*ps + (j+1)*rs + i-1;
    if (((i+1)<rs)&&(j>0)&&((k+1)<ds))      tab[(*n)++] = (k+1)*ps + (j-1)*rs + i+1;
    if (((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)) tab[(*n)++] = (k+1)*ps + (j+1)*rs + i+1;
  }
} /* Alphacarre3d() */

/* ==================================== */
void Betacarre3d(index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab la beta-adherence de (i,j,k) privee de (i,j,k), dans H3.
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  index_t ps = rs * cs;
#ifdef DEBUG
printf("Betacarre3d\n");
#endif
  *n = 0;
  if (i % 2 == 0)
  {
    if (i>0)      tab[(*n)++] = k*ps + j*rs + i-1;      
    if ((i+1)<rs) tab[(*n)++] = k*ps + j*rs + i+1;      
  }
  if (j % 2 == 0)
  {
    if (j>0)      tab[(*n)++] = k*ps + (j-1)*rs + i;      
    if ((j+1)<cs) tab[(*n)++] = k*ps + (j+1)*rs + i;      
  }
  if (k % 2 == 0)
  {
    if (k>0)      tab[(*n)++] = (k-1)*ps + j*rs + i;      
    if ((k+1)<ds) tab[(*n)++] = (k+1)*ps + j*rs + i;      
  }
  if ((i%2) + (j%2) == 0)
  {
    if ((i>0)&&(j>0))           tab[(*n)++] = k*ps + (j-1)*rs + i-1;      
    if ((i>0)&&((j+1)<cs))      tab[(*n)++] = k*ps + (j+1)*rs + i-1;      
    if (((i+1)<rs)&&(j>0))      tab[(*n)++] = k*ps + (j-1)*rs + i+1;
    if (((i+1)<rs)&&((j+1)<cs)) tab[(*n)++] = k*ps + (j+1)*rs + i+1;
  }
  if ((i%2) + (k%2) == 0)
  {
    if ((i>0)&&(k>0))           tab[(*n)++] = (k-1)*ps + j*rs + i-1;      
    if ((i>0)&&((k+1)<ds))      tab[(*n)++] = (k+1)*ps + j*rs + i-1;      
    if (((i+1)<rs)&&(k>0))      tab[(*n)++] = (k-1)*ps + j*rs + i+1;
    if (((i+1)<rs)&&((k+1)<ds)) tab[(*n)++] = (k+1)*ps + j*rs + i+1;
  }
  if ((k%2) + (j%2) == 0)
  {
    if ((k>0)&&(j>0))           tab[(*n)++] = (k-1)*ps + (j-1)*rs + i;      
    if ((k>0)&&((j+1)<cs))      tab[(*n)++] = (k-1)*ps + (j+1)*rs + i;      
    if (((k+1)<ds)&&(j>0))      tab[(*n)++] = (k+1)*ps + (j-1)*rs + i;
    if (((k+1)<ds)&&((j+1)<cs)) tab[(*n)++] = (k+1)*ps + (j+1)*rs + i;
  }
  if ((i%2) + (j%2) + (k%2) == 0)
  {
    if ((i>0)&&(j>0)&&(k>0))                tab[(*n)++] = (k-1)*ps + (j-1)*rs + i-1;
    if ((i>0)&&((j+1)<cs)&&(k>0))           tab[(*n)++] = (k-1)*ps + (j+1)*rs + i-1;
    if (((i+1)<rs)&&(j>0)&&(k>0))           tab[(*n)++] = (k-1)*ps + (j-1)*rs + i+1;
    if (((i+1)<rs)&&((j+1)<cs)&&(k>0))      tab[(*n)++] = (k-1)*ps + (j+1)*rs + i+1;
    if ((i>0)&&(j>0)&&((k+1)<ds))           tab[(*n)++] = (k+1)*ps + (j-1)*rs + i-1;
    if ((i>0)&&((j+1)<cs)&&((k+1)<ds))      tab[(*n)++] = (k+1)*ps + (j+1)*rs + i-1;
    if (((i+1)<rs)&&(j>0)&&((k+1)<ds))      tab[(*n)++] = (k+1)*ps + (j-1)*rs + i+1;
    if (((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)) tab[(*n)++] = (k+1)*ps + (j+1)*rs + i+1;
  }
} /* Betacarre3d() */

/* ==================================== */
void Thetacarre3d(index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab la theta-adherence de (i,j,k) privee de (i,j,k), dans H3.
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  index_t tab2[26]; int32_t n2, l;
  Alphacarre3d(rs, cs, ds, i, j, k, tab, n);
  Betacarre3d(rs, cs, ds, i, j, k, tab2, &n2);
  for (l = 0; l < n2; l++) tab[*n+l] = tab2[l];
  *n = *n + n2;
} /* Thetacarre3d() */

/* ==================================== */
uint32_t XAlphacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  retourne l'alpha-adherence de (i,j) privee de (i,j), dans l'objet X,
  sous la forme d'un tableau de bits:
  singl: 0 bits
  inter: 2 bits
  carre: 8 bits
  cube : 26 bits
*/
{
  index_t ps = rs * cs;
  uint32_t b = 0; /* accumulateur pour les bits */
  int32_t b1 = 1;         /* un bit a decaler */

  if (i % 2)
  {
    if ((i>0)      && X[k*ps + j*rs + i-1]) b |= b1; b1 = b1 << 1;
    if (((i+1)<rs) && X[k*ps + j*rs + i+1]) b |= b1; b1 = b1 << 1;
  }
  if (j % 2)
  {
    if ((j>0)      && X[k*ps + (j-1)*rs + i]) b |= b1; b1 = b1 << 1;
    if (((j+1)<cs) && X[k*ps + (j+1)*rs + i]) b |= b1; b1 = b1 << 1;
  }
  if (k % 2)
  {
    if ((k>0)      && X[(k-1)*ps + j*rs + i]) b |= b1; b1 = b1 << 1;
    if (((k+1)<ds) && X[(k+1)*ps + j*rs + i]) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))           && X[k*ps + (j-1)*rs + i-1]) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((j+1)<cs))      && X[k*ps + (j+1)*rs + i-1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(j>0))      && X[k*ps + (j-1)*rs + i+1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((j+1)<cs)) && X[k*ps + (j+1)*rs + i+1]) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))           && X[(k-1)*ps + j*rs + i-1]) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((k+1)<ds))      && X[(k+1)*ps + j*rs + i-1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(k>0))      && X[(k-1)*ps + j*rs + i+1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((k+1)<ds)) && X[(k+1)*ps + j*rs + i+1]) b |= b1; b1 = b1 << 1;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))           && X[(k-1)*ps + (j-1)*rs + i]) b |= b1; b1 = b1 << 1;
    if (((k>0)&&((j+1)<cs))      && X[(k-1)*ps + (j+1)*rs + i]) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&(j>0))      && X[(k+1)*ps + (j-1)*rs + i]) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&((j+1)<cs)) && X[(k+1)*ps + (j+1)*rs + i]) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))                && X[(k-1)*ps + (j-1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&(k>0))           && X[(k-1)*ps + (j+1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&(k>0))           && X[(k-1)*ps + (j-1)*rs + i+1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))      && X[(k-1)*ps + (j+1)*rs + i+1]) b |= b1; b1 = b1<<1;
    if (((i>0)&&(j>0)&&((k+1)<ds))           && X[(k+1)*ps + (j-1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))      && X[(k+1)*ps + (j+1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))      && X[(k+1)*ps + (j-1)*rs + i+1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)) && X[(k+1)*ps + (j+1)*rs + i+1]) b |= b1; b1 = b1<<1;
  }
  return b;
} /* XAlphacarre3d() */

/* ==================================== */
uint32_t XBetacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  retourne la beta-adherence de (i,j) privee de (i,j), dans l'objet X,
  sous la forme d'un tableau de bits:
  cube : 0 bits
  carre: 2 bits
  inter: 8 bits
  singl: 26 bits
*/
{
  index_t ps = rs * cs;
  uint32_t b = 0; /* accumulateur pour les bits */
  int32_t b1 = 1;         /* un bit a decaler */

  if (i % 2 == 0)
  {
    if ((i>0)      && X[k*ps + j*rs + i-1]) b |= b1; b1 = b1 << 1;
    if (((i+1)<rs) && X[k*ps + j*rs + i+1]) b |= b1; b1 = b1 << 1;
  }
  if (j % 2 == 0)
  {
    if ((j>0)      && X[k*ps + (j-1)*rs + i]) b |= b1; b1 = b1 << 1;
    if (((j+1)<cs) && X[k*ps + (j+1)*rs + i]) b |= b1; b1 = b1 << 1;
  }
  if (k % 2 == 0)
  {
    if ((k>0)      && X[(k-1)*ps + j*rs + i]) b |= b1; b1 = b1 << 1;
    if (((k+1)<ds) && X[(k+1)*ps + j*rs + i]) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) == 0)
  {
    if (((i>0)&&(j>0))           && X[k*ps + (j-1)*rs + i-1]) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((j+1)<cs))      && X[k*ps + (j+1)*rs + i-1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(j>0))      && X[k*ps + (j-1)*rs + i+1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((j+1)<cs)) && X[k*ps + (j+1)*rs + i+1]) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (k%2) == 0)
  {
    if (((i>0)&&(k>0))           && X[(k-1)*ps + j*rs + i-1]) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((k+1)<ds))      && X[(k+1)*ps + j*rs + i-1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(k>0))      && X[(k-1)*ps + j*rs + i+1]) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((k+1)<ds)) && X[(k+1)*ps + j*rs + i+1]) b |= b1; b1 = b1 << 1;
  }
  if ((k%2) + (j%2) == 0)
  {
    if (((k>0)&&(j>0))           && X[(k-1)*ps + (j-1)*rs + i]) b |= b1; b1 = b1 << 1;
    if (((k>0)&&((j+1)<cs))      && X[(k-1)*ps + (j+1)*rs + i]) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&(j>0))      && X[(k+1)*ps + (j-1)*rs + i]) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&((j+1)<cs)) && X[(k+1)*ps + (j+1)*rs + i]) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) + (k%2) == 0)
  {
    if (((i>0)&&(j>0)&&(k>0))                && X[(k-1)*ps + (j-1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&(k>0))           && X[(k-1)*ps + (j+1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&(k>0))           && X[(k-1)*ps + (j-1)*rs + i+1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))      && X[(k-1)*ps + (j+1)*rs + i+1]) b |= b1; b1 = b1<<1;
    if (((i>0)&&(j>0)&&((k+1)<ds))           && X[(k+1)*ps + (j-1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))      && X[(k+1)*ps + (j+1)*rs + i-1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))      && X[(k+1)*ps + (j-1)*rs + i+1]) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)) && X[(k+1)*ps + (j+1)*rs + i+1]) b |= b1; b1 = b1<<1;
  }
  return b;
} /* XBetacarre3d() */

/* ==================================== */
uint32_t XhAlphacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, int32_t h)
/* ==================================== */
/* 
  retourne l'alpha-adherence de (i,j) privee de (i,j), dans l'objet X (coupe niveau h),
  sous la forme d'un tableau de bits:
  singl: 0 bits
  inter: 2 bits
  carre: 8 bits
  cube : 26 bits
*/
{
  index_t ps = rs * cs;
  uint32_t b = 0; /* accumulateur pour les bits */
  int32_t b1 = 1;         /* un bit a decaler */

  if (i % 2)
  {
    if ((i>0)      && (X[k*ps + j*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if (((i+1)<rs) && (X[k*ps + j*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
  }
  if (j % 2)
  {
    if ((j>0)      && (X[k*ps + (j-1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if (((j+1)<cs) && (X[k*ps + (j+1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
  }
  if (k % 2)
  {
    if ((k>0)      && (X[(k-1)*ps + j*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if (((k+1)<ds) && (X[(k+1)*ps + j*rs + i]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))           && (X[k*ps + (j-1)*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((j+1)<cs))      && (X[k*ps + (j+1)*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(j>0))      && (X[k*ps + (j-1)*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((j+1)<cs)) && (X[k*ps + (j+1)*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))           && (X[(k-1)*ps + j*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((k+1)<ds))      && (X[(k+1)*ps + j*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(k>0))      && (X[(k-1)*ps + j*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((k+1)<ds)) && (X[(k+1)*ps + j*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))           && (X[(k-1)*ps + (j-1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if (((k>0)&&((j+1)<cs))      && (X[(k-1)*ps + (j+1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&(j>0))      && (X[(k+1)*ps + (j-1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&((j+1)<cs)) && (X[(k+1)*ps + (j+1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))                && (X[(k-1)*ps + (j-1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&(k>0))           && (X[(k-1)*ps + (j+1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&(k>0))           && (X[(k-1)*ps + (j-1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))      && (X[(k-1)*ps + (j+1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
    if (((i>0)&&(j>0)&&((k+1)<ds))           && (X[(k+1)*ps + (j-1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))      && (X[(k+1)*ps + (j+1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))      && (X[(k+1)*ps + (j-1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)) && (X[(k+1)*ps + (j+1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
  }
  return b;
} /* XhAlphacarre3d() */

/* ==================================== */
uint32_t XhBetacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, int32_t h)
/* ==================================== */
/* 
  retourne l'alpha-adherence de (i,j) privee de (i,j), dans l'objet X (coupe niveau h),
  sous la forme d'un tableau de bits:
  cube : 0 bits
  carre: 2 bits
  inter: 8 bits
  singl: 26 bits
*/
{
  index_t ps = rs * cs;
  uint32_t b = 0; /* accumulateur pour les bits */
  int32_t b1 = 1;         /* un bit a decaler */

  if (i % 2 == 0)
  {
    if ((i>0)      && (X[k*ps + j*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if (((i+1)<rs) && (X[k*ps + j*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
  }
  if (j % 2 == 0)
  {
    if ((j>0)      && (X[k*ps + (j-1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if (((j+1)<cs) && (X[k*ps + (j+1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
  }
  if (k % 2 == 0)
  {
    if ((k>0)      && (X[(k-1)*ps + j*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if (((k+1)<ds) && (X[(k+1)*ps + j*rs + i]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) == 0)
  {
    if (((i>0)&&(j>0))           && (X[k*ps + (j-1)*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((j+1)<cs))      && (X[k*ps + (j+1)*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(j>0))      && (X[k*ps + (j-1)*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((j+1)<cs)) && (X[k*ps + (j+1)*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (k%2) == 0)
  {
    if (((i>0)&&(k>0))           && (X[(k-1)*ps + j*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if (((i>0)&&((k+1)<ds))      && (X[(k+1)*ps + j*rs + i-1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&(k>0))      && (X[(k-1)*ps + j*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
    if ((((i+1)<rs)&&((k+1)<ds)) && (X[(k+1)*ps + j*rs + i+1]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((k%2) + (j%2) == 0)
  {
    if (((k>0)&&(j>0))           && (X[(k-1)*ps + (j-1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if (((k>0)&&((j+1)<cs))      && (X[(k-1)*ps + (j+1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&(j>0))      && (X[(k+1)*ps + (j-1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
    if ((((k+1)<ds)&&((j+1)<cs)) && (X[(k+1)*ps + (j+1)*rs + i]>=h)) b |= b1; b1 = b1 << 1;
  }
  if ((i%2) + (j%2) + (k%2) == 0)
  {
    if (((i>0)&&(j>0)&&(k>0))                && (X[(k-1)*ps + (j-1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&(k>0))           && (X[(k-1)*ps + (j+1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&(k>0))           && (X[(k-1)*ps + (j-1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))      && (X[(k-1)*ps + (j+1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
    if (((i>0)&&(j>0)&&((k+1)<ds))           && (X[(k+1)*ps + (j-1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))      && (X[(k+1)*ps + (j+1)*rs + i-1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))      && (X[(k+1)*ps + (j-1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)) && (X[(k+1)*ps + (j+1)*rs + i+1]>=h)) b |= b1; b1 = b1<<1;
  }
  return b;
} /* XhBetacarre3d() */

/* ==================================== */
void SetXAlphacarre3d(uint32_t b, uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  X est une grille de K de taille 3x3x3, (i,j,k) les coord. d'un point dans X, 
  b est la representation binaire du alphacarre du point.
  La fonction remplit la grille X conformement a ces informations.
*/
{
  index_t ps = rs * cs;

  if (i % 2)
  {
    if ((i>0)      && (b & (1<<0))) X[k*ps + j*rs + i-1] = VAL_OBJET;
    if (((i+1)<rs) && (b & (1<<1))) X[k*ps + j*rs + i+1] = VAL_OBJET;
  }
  if (j % 2)
  {
    if ((j>0)      && (b & (1<<2))) X[k*ps + (j-1)*rs + i] = VAL_OBJET;
    if (((j+1)<cs) && (b & (1<<3))) X[k*ps + (j+1)*rs + i] = VAL_OBJET;
  }
  if (k % 2)
  {
    if ((k>0)      && (b & (1<<4))) X[(k-1)*ps + j*rs + i] = VAL_OBJET;
    if (((k+1)<ds) && (b & (1<<5))) X[(k+1)*ps + j*rs + i] = VAL_OBJET;
  }
  if ((i%2) + (j%2) == 2)
  {
    if ((i>0)&&(j>0)           && (b&(1<<6))) X[k*ps + (j-1)*rs + i-1]=VAL_OBJET;
    if ((i>0)&&((j+1)<cs)      && (b&(1<<7))) X[k*ps + (j+1)*rs + i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(j>0)      && (b&(1<<8))) X[k*ps + (j-1)*rs + i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((j+1)<cs) && (b&(1<<9))) X[k*ps + (j+1)*rs + i+1]=VAL_OBJET;
  }
  if ((i%2) + (k%2) == 2)
  {
    if ((i>0)&&(k>0)           && (b&(1<<10))) X[(k-1)*ps + j*rs + i-1]=VAL_OBJET;
    if ((i>0)&&((k+1)<ds)      && (b&(1<<11))) X[(k+1)*ps + j*rs + i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(k>0)      && (b&(1<<12))) X[(k-1)*ps + j*rs + i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((k+1)<ds) && (b&(1<<13))) X[(k+1)*ps + j*rs + i+1]=VAL_OBJET;
  }
  if ((k%2) + (j%2) == 2)
  {
    if ((k>0)&&(j>0)           && (b&(1<<14))) X[(k-1)*ps + (j-1)*rs + i]=VAL_OBJET;
    if ((k>0)&&((j+1)<cs)      && (b&(1<<15))) X[(k-1)*ps + (j+1)*rs + i]=VAL_OBJET;
    if (((k+1)<ds)&&(j>0)      && (b&(1<<16))) X[(k+1)*ps + (j-1)*rs + i]=VAL_OBJET;
    if (((k+1)<ds)&&((j+1)<cs) && (b&(1<<17))) X[(k+1)*ps + (j+1)*rs + i]=VAL_OBJET;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if ((i>0)&&(j>0)&&(k>0)           && (b&(1<<18))) X[(k-1)*ps+(j-1)*rs+i-1]=VAL_OBJET;
    if ((i>0)&&((j+1)<cs)&&(k>0)      && (b&(1<<19))) X[(k-1)*ps+(j+1)*rs+i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(j>0)&&(k>0)      && (b&(1<<20))) X[(k-1)*ps+(j-1)*rs+i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((j+1)<cs)&&(k>0) && (b&(1<<21))) X[(k-1)*ps+(j+1)*rs+i+1]=VAL_OBJET;
    if ((i>0)&&(j>0)&&((k+1)<ds)      && (b&(1<<22))) X[(k+1)*ps+(j-1)*rs+i-1]=VAL_OBJET;
    if ((i>0)&&((j+1)<cs)&&((k+1)<ds) && (b&(1<<23))) X[(k+1)*ps+(j+1)*rs+i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(j>0)&&((k+1)<ds) && (b&(1<<24))) X[(k+1)*ps+(j-1)*rs+i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)&&(b&(1<<25)))X[(k+1)*ps+(j+1)*rs+i+1]=VAL_OBJET;
  }
} /* SetXAlphacarre3d() */

/* ==================================== */
void SetXBetacarre3d(uint32_t b, uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  X est une grille de K de taille 3x3x3, (i,j,k) les coord. d'un point dans X, 
  b est la representation binaire du betacarre du point.
  La fonction remplit la grille X conformement a ces informations.
*/
{
  index_t ps = rs * cs;

  if (i % 2 == 0)
  {
    if ((i>0)      && (b & (1<<0))) X[k*ps + j*rs + i-1] = VAL_OBJET;
    if (((i+1)<rs) && (b & (1<<1))) X[k*ps + j*rs + i+1] = VAL_OBJET;
  }
  if (j % 2 == 0)
  {
    if ((j>0)      && (b & (1<<2))) X[k*ps + (j-1)*rs + i] = VAL_OBJET;
    if (((j+1)<cs) && (b & (1<<3))) X[k*ps + (j+1)*rs + i] = VAL_OBJET;
  }
  if (k % 2 == 0)
  {
    if ((k>0)      && (b & (1<<4))) X[(k-1)*ps + j*rs + i] = VAL_OBJET;
    if (((k+1)<ds) && (b & (1<<5))) X[(k+1)*ps + j*rs + i] = VAL_OBJET;
  }
  if ((i%2) + (j%2) == 0)
  {
    if ((i>0)&&(j>0)           && (b&(1<<6))) X[k*ps + (j-1)*rs + i-1]=VAL_OBJET;
    if ((i>0)&&((j+1)<cs)      && (b&(1<<7))) X[k*ps + (j+1)*rs + i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(j>0)      && (b&(1<<8))) X[k*ps + (j-1)*rs + i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((j+1)<cs) && (b&(1<<9))) X[k*ps + (j+1)*rs + i+1]=VAL_OBJET;
  }
  if ((i%2) + (k%2) == 0)
  {
    if ((i>0)&&(k>0)           && (b&(1<<10))) X[(k-1)*ps + j*rs + i-1]=VAL_OBJET;
    if ((i>0)&&((k+1)<ds)      && (b&(1<<11))) X[(k+1)*ps + j*rs + i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(k>0)      && (b&(1<<12))) X[(k-1)*ps + j*rs + i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((k+1)<ds) && (b&(1<<13))) X[(k+1)*ps + j*rs + i+1]=VAL_OBJET;
  }
  if ((k%2) + (j%2) == 0)
  {
    if ((k>0)&&(j>0)           && (b&(1<<14))) X[(k-1)*ps + (j-1)*rs + i]=VAL_OBJET;
    if ((k>0)&&((j+1)<cs)      && (b&(1<<15))) X[(k-1)*ps + (j+1)*rs + i]=VAL_OBJET;
    if (((k+1)<ds)&&(j>0)      && (b&(1<<16))) X[(k+1)*ps + (j-1)*rs + i]=VAL_OBJET;
    if (((k+1)<ds)&&((j+1)<cs) && (b&(1<<17))) X[(k+1)*ps + (j+1)*rs + i]=VAL_OBJET;
  }
  if ((i%2) + (j%2) + (k%2) == 0)
  {
    if ((i>0)&&(j>0)&&(k>0)           && (b&(1<<18))) X[(k-1)*ps+(j-1)*rs+i-1]=VAL_OBJET;
    if ((i>0)&&((j+1)<cs)&&(k>0)      && (b&(1<<19))) X[(k-1)*ps+(j+1)*rs+i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(j>0)&&(k>0)      && (b&(1<<20))) X[(k-1)*ps+(j-1)*rs+i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((j+1)<cs)&&(k>0) && (b&(1<<21))) X[(k-1)*ps+(j+1)*rs+i+1]=VAL_OBJET;
    if ((i>0)&&(j>0)&&((k+1)<ds)      && (b&(1<<22))) X[(k+1)*ps+(j-1)*rs+i-1]=VAL_OBJET;
    if ((i>0)&&((j+1)<cs)&&((k+1)<ds) && (b&(1<<23))) X[(k+1)*ps+(j+1)*rs+i-1]=VAL_OBJET;
    if (((i+1)<rs)&&(j>0)&&((k+1)<ds) && (b&(1<<24))) X[(k+1)*ps+(j-1)*rs+i+1]=VAL_OBJET;
    if (((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds)&&(b&(1<<25)))X[(k+1)*ps+(j+1)*rs+i+1]=VAL_OBJET;
  }
} /* SetXBetacarre3d() */

/* ==================================== */
int32_t Precede3d(index_t i, index_t j, index_t k, index_t x, index_t y, index_t z)
/* ==================================== */
{
  if (mcabs(i-x)>1) return 0;
  if (mcabs(j-y)>1) return 0;
  if (mcabs(k-z)>1) return 0;
  if ((i != x) && (i%2==1)) return 0;
  if ((j != y) && (j%2==1)) return 0;
  if ((k != z) && (k%2==1)) return 0;
  return 1;
} /* Precede3d() */

/* ==================================== */
int32_t NbPred3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/*
  cette fonction compte les predecesseurs minimaux dans l'objet B (le cardinal de betapoint).
*/
{
  index_t ps = rs * cs;
  int32_t u, v, tu, tv, n, nn;
  index_t tab[GRS3D*GCS3D*GDS3D];

  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  nn = n;

#ifdef DEBUGNP
printf("NbPred3d (%d %d %d) n = %d  :  ",i,j,k,n);
for (u = 0; u < n; u++) printf("(%d,%d,%d) ", tab[u]%rs, (tab[u]%ps)/rs, tab[u]/ps );
printf("\n");
#endif

  for (u = 0; u < n; u++)
  {
    tu = tab[u];
    if ((tu != -1) && !B[tu])
    { tab[u] = -1;  nn--; }
    else if (tu != -1)
      for (v = u+1; v < n; v++) /* elimine les pred. non minimaux */
      {
        tv = tab[v];
        if ((tv != -1) && B[tv])
        {
          if (Precede3d(tv%rs,(tv%ps)/rs,tv/ps,tu%rs,(tu%ps)/rs,tu/ps)) {tab[u] = -1; nn--;break;}
          if (Precede3d(tu%rs,(tu%ps)/rs,tu/ps,tv%rs,(tv%ps)/rs,tv/ps)) {tab[v] = -1; nn--;}
        }
      }
  }
#ifdef DEBUGNP
printf("NbPred3d (%d %d %d) n = %d ; nn = %d  :  ",i,j,k,n,nn);
for (u = 0; u < n; u++) if (tab[u] != -1) printf("(%d,%d,%d) ", tab[u]%rs, (tab[u]%ps)/rs, tab[u]/ps );
printf("\n");
#endif
  
  return nn;
} /* NbPred3d() */

/* ==================================== */
int32_t Succede3d(index_t i, index_t j, index_t k, index_t x, index_t y, index_t z)
/* ==================================== */
{
  if (mcabs(i-x)>1) return 0;
  if (mcabs(j-y)>1) return 0;
  if (mcabs(k-z)>1) return 0;
  if ((i != x) && (i%2==0)) return 0;
  if ((j != y) && (j%2==0)) return 0;
  if ((k != z) && (k%2==0)) return 0;
  return 1;
} /* Succede3d() */

/* ==================================== */
int32_t NbSucc3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/*
  cette fonction compte les successeurs maximaux dans l'objet B (le cardinal de alphapoint).
*/
{
  index_t ps = rs * cs;
  int32_t u, v, tu, tv, n, nn;
  index_t tab[GRS3D*GCS3D*GDS3D];

  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
  nn = n;

  for (u = 0; u < n; u++)
  {
    tu = tab[u];
    if ((tu != -1) && !B[tu])
      nn--;
    else
      for (v = u+1; v < n; v++) /* elimine les succ. non maximaux */
      {
        tv = tab[v];
        if ((tv != -1) && B[tv])
        {
          if (Succede3d(tv%rs,(tv%ps)/rs,tv/ps,tu%rs,(tu%ps)/rs,tu/ps)) {tab[u] = -1; nn--; break;}
          if (Succede3d(tu%rs,(tu%ps)/rs,tu/ps,tv%rs,(tv%ps)/rs,tv/ps)) {tab[v] = -1; nn--;}
	}
      }
  }
#ifdef DEBUGNS
printf("NbSucc3d (%d %d %d) n = %d ; nn = %d\n",i,j,k,n,nn);
#endif
  
  return nn;
} /* NbPred3d() */

/* ==================================== */
int32_t BetaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
{
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (!B[k*rs*cs + j*rs + i]) return 0;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) return 0;
  return 1;
} /* BetaTerminal3d() */

/* ==================================== */
int32_t AlphaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
{
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (!B[k*rs*cs + j*rs + i]) return 0;
  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) return 0;
  return 1;
} /* AlphaTerminal3d() */

/* ==================================== */
int32_t ExactementUnBetaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds)
/* ==================================== */
{
  index_t i, ps = rs * cs, N = ps * ds;
  int32_t n = 0;

  for (i = 0; i < N; i++)
    if ((B[i]) && BetaTerminal3d(B, rs, cs, ds, i%rs, (i%ps)/rs, i/ps))
      if (++n > 1) return 0;
  if (n < 1) return 0; else return 1;
} /* ExactementUnBetaTerminal3d() */

/* ==================================== */
void SatureAlphacarre3d(struct xvimage *b)
/* ==================================== */
/*
  b contient un objet de Z3 represente dans la grille de Khalimsky
  par uniquement des points beta-terminaux.
  Cette fonction met a VAL_OBJET l'alpha-adherence de ces points. 
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

#ifdef DEBUGSD
printf("SatureAlphacarre3d \n");
#endif
  for (k = 1; k < ds; k += 2)
    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
        if (B[k*ps + j*rs + i])
        {
          Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
          for (u = 0; u < n; u++) B[tab[u]] = VAL_OBJET;
        }
} /* SatureAlphacarre3d() */

/* ==================================== */
void AjouteAlphacarre3d(struct xvimage *b)
/* ==================================== */
/*
  retourne dans b l'ensemble des points de l'alphacarre de tous les éléments de b
 */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
	if (B[k*ps + j*rs + i])
	{
	  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) B[tab[u]] = VAL_OBJET;
	}
} /* AjouteAlphacarre3d() */

/* ==================================== */
void AjouteBetacarre3d(struct xvimage *b)
/* ==================================== */
/*
  retourne dans b l'ensemble des points de l'alphacarre de tous les éléments de b
 */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
	if (B[k*ps + j*rs + i])
	{
	  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
	  for (u = 0; u < n; u++) B[tab[u]] = VAL_OBJET;
	}
} /* AjouteBetacarre3d() */

/* ==================================== */
void EffaceLiensBetaLibres3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n;

  do 
  {
    n = 0;
    for (k = 0; k < ds; k += 1)
      for (j = 0; j < cs; j += 1)
        for (i = 0; i < rs; i += 1)
          if (B[k*ps + j*rs + i] && (NbPred3d(B, rs, cs, ds, i, j, k) == 1))
	  {
            B[k*ps + j*rs + i] = 0;
            n++;
	  }
  } while (n > 0);
} /* EffaceLiensBetaLibres3d() */

/* ==================================== */
void MaxAlpha3d(struct xvimage *k)
/* ==================================== */
/*
  pour chaque element x, faire K[x] = mcmax{K[y] | y in alpha(x)}
 */
#undef F_NAME
#define F_NAME "MaxAlpha3d"
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  index_t ds = depth(k);
  index_t ps = rs * cs;
  index_t x, y, z;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (datatype(k) == VFF_TYP_1_BYTE)
  {
    uint8_t *K = UCHARDATA(k);
    uint8_t M;
    for (z = 0; z < ds; z += 1)
      for (y = 0; y < cs; y += 1)
	for (x = 0; x < rs; x += 1)
	{
	  M = K[z * ps + y * rs + x];
	  Alphacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++) 
	    if (K[tab[u]] > M) M = K[tab[u]];
	  K[z * ps + y * rs + x] = M;
	}
  }
  else if (datatype(k) == VFF_TYP_4_BYTE)
  {
    int32_t *K = SLONGDATA(k);
    int32_t M;
    for (z = 0; z < ds; z += 1)
      for (y = 0; y < cs; y += 1)
	for (x = 0; x < rs; x += 1)
	{
	  M = K[z * ps + y * rs + x];
	  Alphacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++) 
	    if (K[tab[u]] > M) M = K[tab[u]];
	  K[z * ps + y * rs + x] = M;
	}
  }
  else if (datatype(k) == VFF_TYP_FLOAT)
  {
    float *K = FLOATDATA(k);
    float M;
    for (z = 0; z < ds; z += 1)
      for (y = 0; y < cs; y += 1)
	for (x = 0; x < rs; x += 1)
        {
	  M = K[z * ps + y * rs + x];
	  Alphacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++) 
	    if (K[tab[u]] > M) M = K[tab[u]];
	  K[z * ps + y * rs + x] = M;
	}
  }
  else
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
} /* MaxAlpha3d() */

/* ==================================== */
void MaxBeta3d(struct xvimage *k)
/* ==================================== */
/*
  pour chaque element x, faire K[x] = mcmax{K[y] | y in beta(x)}
 */
#undef F_NAME
#define F_NAME "MaxBeta3d"
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  index_t ds = depth(k);
  index_t ps = rs * cs;
  index_t x, y, z;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  if (datatype(k) == VFF_TYP_1_BYTE)
  {
    uint8_t *K = UCHARDATA(k);
    uint8_t M;
    for (z = 0; z < ds; z += 1)
      for (y = 0; y < cs; y += 1)
	for (x = 0; x < rs; x += 1)
	{
	  M = K[z * ps + y * rs + x];
	  Betacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++) 
	    if (K[tab[u]] > M) M = K[tab[u]];
	  K[z * ps + y * rs + x] = M;
	}
  }
  else if (datatype(k) == VFF_TYP_4_BYTE)
  {
    int32_t *K = SLONGDATA(k);
    int32_t M;
    for (z = 0; z < ds; z += 1)
      for (y = 0; y < cs; y += 1)
	for (x = 0; x < rs; x += 1)
	{
	  M = K[z * ps + y * rs + x];
	  Betacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++) 
	    if (K[tab[u]] > M) M = K[tab[u]];
	  K[z * ps + y * rs + x] = M;
	}
  }
  else if (datatype(k) == VFF_TYP_FLOAT)
  {
    float *K = FLOATDATA(k);
    float M;
    for (z = 0; z < ds; z += 1)
      for (y = 0; y < cs; y += 1)
	for (x = 0; x < rs; x += 1)
        {
	  M = K[z * ps + y * rs + x];
	  Betacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++) 
	    if (K[tab[u]] > M) M = K[tab[u]];
	  K[z * ps + y * rs + x] = M;
	}
  }
  else
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
} /* MaxBeta3d() */

/* ==================================== */
void EffaceLiensAlphaLibres3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n;

  do 
  {
    n = 0;
    for (k = 0; k < ds; k += 1)
      for (j = 0; j < cs; j += 1)
        for (i = 0; i < rs; i += 1)
          if (B[k*ps + j*rs + i] && (NbSucc3d(B, rs, cs, ds, i, j, k) == 1))
	  {
            B[k*ps + j*rs + i] = 0;
            n++;
	  }
  } while (n > 0);
} /* EffaceLiensAlphaLibres3d() */

/* ==================================== */
void EffaceLiensBiLibres3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n;

  do 
  {
    n = 0;
    for (k = 0; k < ds; k += 1)
      for (j = 0; j < cs; j += 1)
        for (i = 0; i < rs; i += 1)
          if (B[k*ps + j*rs + i] && 
               ((NbPred3d(B, rs, cs, ds, i, j, k) == 1) ||
                (NbSucc3d(B, rs, cs, ds, i, j, k) == 1)
               )
             )
	  {
            B[k*ps + j*rs + i] = 0;
            n++;
	  }
  } while (n > 0);
} /* EffaceLiensBiLibres3d() */

/* ==================================== */
index_t EffaceLiensBetaLibresNonMarques3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n, m;

  m = 0;
  do 
  {
    n = 0;
    for (k = 0; k < ds; k += 1)
      for (j = 0; j < cs; j += 1)
        for (i = 0; i < rs; i += 1)
          if ((B[k*ps + j*rs + i] == VAL_OBJET) && (NbPred3d(B, rs, cs, ds, i, j, k) == 1))
	  {
            B[k*ps + j*rs + i] = 0;
            n++;
	  }
    m += n;
  } while (n > 0);
  return m;
} /* EffaceLiensBetaLibresNonMarques3d() */

/* ==================================== */
index_t EffaceLiensAlphaLibresNonMarques3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n, m;

  m = 0;
  do 
  {
    n = 0;
    for (k = 0; k < ds; k += 1)
      for (j = 0; j < cs; j += 1)
        for (i = 0; i < rs; i += 1)
          if ((B[k*ps + j*rs + i] == VAL_OBJET) && (NbSucc3d(B, rs, cs, ds, i, j, k) == 1))
	  {
            B[k*ps + j*rs + i] = 0;
            n++;
	  }
    m += n;
  } while (n > 0);
  return m;
} /* EffaceLiensAlphaLibresNonMarques3d() */

/* ==================================== */
void EffaceLiensBiLibresNonMarques3d(struct xvimage *b)
/* ==================================== */
{
  index_t n;
  do 
  {
    n = 0;
    n += EffaceLiensBetaLibresNonMarques3d(b);
    n += EffaceLiensAlphaLibresNonMarques3d(b);
  } while (n > 0);
} /* EffaceLiensBiLibresNonMarques3d() */

/* ==================================== */
void Connex26Obj3d(struct xvimage *b)
/* ==================================== */
{
  SatureAlphacarre3d(b);
  EffaceLiensBetaLibres3d(b);
} /* Connex26Obj3d() */

/* ==================================== */
void Connex6Obj3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k, x, y, z;
  int32_t u, n;
  index_t tab[GRS3D*GCS3D*GDS3D];

  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        if (!CUBE3D(i,j,k))
        {
          Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
          for (u = 0; u < n; u++)
          {
            x = tab[u] % rs;
            y = (tab[u] % ps) / rs;
            z = tab[u] / ps;
            if ((CUBE3D(x,y,z)) && !B[tab[u]]) goto skip;
          }
          B[k*ps+j*rs+i] = VAL_OBJET;
skip: ;
        }
} /* Connex6Obj3d() */

/* ==================================== */
void ColorieKh3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[k*ps + j*rs + i])
        {
          if (CUBE3D (i,j,k)) B[k*ps + j*rs + i] = NDG_CUBE3D;
          else if (CARRE3DXY(i,j,k)) B[k*ps + j*rs + i] = NDG_CARRE3DXY;
          else if (CARRE3DYZ(i,j,k)) B[k*ps + j*rs + i] = NDG_CARRE3DYZ;
          else if (CARRE3DXZ(i,j,k)) B[k*ps + j*rs + i] = NDG_CARRE3DXZ;
          else if (INTER3DX(i,j,k)) B[k*ps + j*rs + i] = NDG_INTER3DX;
          else if (INTER3DY(i,j,k)) B[k*ps + j*rs + i] = NDG_INTER3DY;
          else if (INTER3DZ(i,j,k)) B[k*ps + j*rs + i] = NDG_INTER3DZ;
          else if (SINGL3D(i,j,k)) B[k*ps + j*rs + i] = NDG_SINGL3D;
        }
} /* ColorieKh3d() */

/* ==================================== */
index_t EulerKh3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n3=0, n2=0, n1=0, n0=0;

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[k*ps + j*rs + i])
        {
          if (CUBE3D (i,j,k)) n3++;
          else if (CARRE3D(i,j,k)) n2++;
          else if (INTER3D(i,j,k)) n1++;
          else if (SINGL3D(i,j,k)) n0++;
        }
  return n0 - n1 + n2 - n3;
} /* EulerKh3d() */

/* ==================================== */
index_t EulerKh3dVal(struct xvimage *b, uint8_t Val)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n3=0, n2=0, n1=0, n0=0;

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[k*ps + j*rs + i] == Val)
        {
          if (CUBE3D (i,j,k)) n3++;
          else if (CARRE3D(i,j,k)) n2++;
          else if (INTER3D(i,j,k)) n1++;
          else if (SINGL3D(i,j,k)) n0++;
        }
  return n0 - n1 + n2 - n3;
} /* EulerKh3dVal() */

/* ==================================== */
void CopieAlphacarre3d(uint8_t *G, uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t ps = rs * cs;
  index_t x = i % 2;
  index_t y = j % 2;
  index_t z = k % 2;
  int32_t gps = GRS3D*GCS3D;
  int32_t grs = GRS3D;

#ifdef DEBUG
printf("CopieAlphacarre3d\n");
#endif
  memset(G, VAL_NULLE, GRS3D*GCS3D*GDS3D); /* init a VAL_NULLE */
  if (i % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) G[z*gps+y*grs+x-1] = VAL_OBJET;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) G[z*gps+y*grs+x+1] = VAL_OBJET;
  }
  if (j % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) G[z*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) G[z*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if (k % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) G[(z-1)*gps+y*grs+x] = VAL_OBJET;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) G[(z+1)*gps+y*grs+x] = VAL_OBJET;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) G[z*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) G[z*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) G[z*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) G[z*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) G[(z-1)*gps+y*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) G[(z+1)*gps+y*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) G[(z-1)*gps+y*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) G[(z+1)*gps+y*grs+x+1] = VAL_OBJET;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) G[(z-1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) G[(z-1)*gps+(y+1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) G[(z+1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) G[(z+1)*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) G[(z-1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) G[(z-1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) G[(z-1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) G[(z-1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) G[(z+1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) G[(z+1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) G[(z+1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) G[(z+1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
} /* CopieAlphacarre3d() */

/* ==================================== */
void CopieThetacarre3d(uint8_t *G, uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  Copie le thetacarre du point i,j,k de B dans l'image G.
  G doit etre de taille 7x7x7.
*/    
{
  index_t ps = rs * cs;
  index_t x = GRS3D+(i%2);
  index_t y = GCS3D+(j%2);
  index_t z = GDS3D+(k%2);
  int32_t grs = 2 * GRS3D + 1;
  int32_t gps = grs * (2 * GCS3D + 1);
  int32_t gN = gps * (2 * GDS3D + 1);

#ifdef DEBUGCTC
printf("CopieThetacarre3d(rs=%d cs=%d ds=%d i=%d j=%d k=%d)\n", rs, cs, ds, i, j, k);
#endif
  memset(G, VAL_NULLE, gN); /* init a VAL_NULLE */

  /* copie le Alphacarre */
  if (i % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) G[z*gps+y*grs+x-1] = VAL_OBJET;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) G[z*gps+y*grs+x+1] = VAL_OBJET;
  }
  if (j % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) G[z*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) G[z*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if (k % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) G[(z-1)*gps+y*grs+x] = VAL_OBJET;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) G[(z+1)*gps+y*grs+x] = VAL_OBJET;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) G[z*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) G[z*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) G[z*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) G[z*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) G[(z-1)*gps+y*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) G[(z+1)*gps+y*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) G[(z-1)*gps+y*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) G[(z+1)*gps+y*grs+x+1] = VAL_OBJET;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) G[(z-1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) G[(z-1)*gps+(y+1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) G[(z+1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) G[(z+1)*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) G[(z-1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) G[(z-1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) G[(z-1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) G[(z-1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) G[(z+1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) G[(z+1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) G[(z+1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) G[(z+1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }

  /* copie le Betacarre */
  if ((i+1) % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) G[z*gps+y*grs+x-1] = VAL_OBJET;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) G[z*gps+y*grs+x+1] = VAL_OBJET;
  }
  if ((j+1) % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) G[z*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) G[z*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if ((k+1) % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) G[(z-1)*gps+y*grs+x] = VAL_OBJET;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) G[(z+1)*gps+y*grs+x] = VAL_OBJET;
  }
  if (((i+1)%2) + ((j+1)%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) G[z*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) G[z*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) G[z*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) G[z*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
  if (((i+1)%2) + ((k+1)%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) G[(z-1)*gps+y*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) G[(z+1)*gps+y*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) G[(z-1)*gps+y*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) G[(z+1)*gps+y*grs+x+1] = VAL_OBJET;
  }
  if (((k+1)%2) + ((j+1)%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) G[(z-1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) G[(z-1)*gps+(y+1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) G[(z+1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) G[(z+1)*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if (((i+1)%2) + ((j+1)%2) + ((k+1)%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) G[(z-1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) G[(z-1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) G[(z-1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) G[(z-1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) G[(z+1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) G[(z+1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) G[(z+1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) G[(z+1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
} /* CopieThetacarre3d() */

/* ==================================== */
void CopieThetacarreCompl3d(uint8_t *G, uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  Copie le thetacarre du point i,j,k du complementaire de B dans l'image G.
  G doit etre de taille 7x7x7.
*/    
{
  index_t ps = rs * cs;
  index_t x = GRS3D+(i%2);
  index_t y = GCS3D+(j%2);
  index_t z = GDS3D+(k%2);
  int32_t grs = 2 * GRS3D + 1;
  int32_t gps = grs * (2 * GCS3D + 1);
  int32_t gN = gps * (2 * GDS3D + 1);

#ifdef DEBUGCTC
printf("CopieThetacarre3d(rs=%d cs=%d ds=%d i=%d j=%d k=%d)\n", rs, cs, ds, i, j, k);
#endif
  memset(G, VAL_NULLE, gN); /* init a VAL_NULLE */

  /* copie le Alphacarre */
  if (i % 2)
  {
    if (!(i>0)||!B[k*ps+j*rs+i-1]) G[z*gps+y*grs+x-1] = VAL_OBJET;
    if (!((i+1)<rs)||!B[k*ps+j*rs+i+1]) G[z*gps+y*grs+x+1] = VAL_OBJET;
  }
  if (j % 2)
  {
    if (!(j>0)||!B[k*ps+(j-1)*rs+i]) G[z*gps+(y-1)*grs+x] = VAL_OBJET;
    if (!((j+1)<cs)||!B[k*ps+(j+1)*rs+i]) G[z*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if (k % 2)
  {
    if (!(k>0)||!B[(k-1)*ps+j*rs+i]) G[(z-1)*gps+y*grs+x] = VAL_OBJET;
    if (!((k+1)<ds)||!B[(k+1)*ps+j*rs+i]) G[(z+1)*gps+y*grs+x] = VAL_OBJET;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (!((i>0)&&(j>0))||!B[k*ps+(j-1)*rs+i-1]) G[z*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((j+1)<cs))||!B[k*ps+(j+1)*rs+i-1]) G[z*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(j>0))||!B[k*ps+(j-1)*rs+i+1]) G[z*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((j+1)<cs))||!B[k*ps+(j+1)*rs+i+1]) G[z*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (!((i>0)&&(k>0))||!B[(k-1)*ps+j*rs+i-1]) G[(z-1)*gps+y*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((k+1)<ds))||!B[(k+1)*ps+j*rs+i-1]) G[(z+1)*gps+y*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(k>0))||!B[(k-1)*ps+j*rs+i+1]) G[(z-1)*gps+y*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((k+1)<ds))||!B[(k+1)*ps+j*rs+i+1]) G[(z+1)*gps+y*grs+x+1] = VAL_OBJET;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (!((k>0)&&(j>0))||!B[(k-1)*ps+(j-1)*rs+i]) G[(z-1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (!((k>0)&&((j+1)<cs))||!B[(k-1)*ps+(j+1)*rs+i]) G[(z-1)*gps+(y+1)*grs+x] = VAL_OBJET;
    if (!(((k+1)<ds)&&(j>0))||!B[(k+1)*ps+(j-1)*rs+i]) G[(z+1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (!(((k+1)<ds)&&((j+1)<cs))||!B[(k+1)*ps+(j+1)*rs+i]) G[(z+1)*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (!((i>0)&&(j>0)&&(k>0))||!B[(k-1)*ps+(j-1)*rs+i-1]) G[(z-1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((j+1)<cs)&&(k>0))||!B[(k-1)*ps+(j+1)*rs+i-1]) G[(z-1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(j>0)&&(k>0))||!B[(k-1)*ps+(j-1)*rs+i+1]) G[(z-1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((j+1)<cs)&&(k>0))||!B[(k-1)*ps+(j+1)*rs+i+1]) G[(z-1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
    if (!((i>0)&&(j>0)&&((k+1)<ds))||!B[(k+1)*ps+(j-1)*rs+i-1]) G[(z+1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((j+1)<cs)&&((k+1)<ds))||!B[(k+1)*ps+(j+1)*rs+i-1]) G[(z+1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(j>0)&&((k+1)<ds))||!B[(k+1)*ps+(j-1)*rs+i+1]) G[(z+1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))||!B[(k+1)*ps+(j+1)*rs+i+1]) G[(z+1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }

  /* copie le Betacarre */
  if ((i+1) % 2)
  {
    if (!(i>0)||!B[k*ps+j*rs+i-1]) G[z*gps+y*grs+x-1] = VAL_OBJET;
    if (!((i+1)<rs)||!B[k*ps+j*rs+i+1]) G[z*gps+y*grs+x+1] = VAL_OBJET;
  }
  if ((j+1) % 2)
  {
    if (!(j>0)||!B[k*ps+(j-1)*rs+i]) G[z*gps+(y-1)*grs+x] = VAL_OBJET;
    if (!((j+1)<cs)||!B[k*ps+(j+1)*rs+i]) G[z*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if ((k+1) % 2)
  {
    if (!(k>0)||!B[(k-1)*ps+j*rs+i]) G[(z-1)*gps+y*grs+x] = VAL_OBJET;
    if (!((k+1)<ds)||!B[(k+1)*ps+j*rs+i]) G[(z+1)*gps+y*grs+x] = VAL_OBJET;
  }
  if (((i+1)%2) + ((j+1)%2) == 2)
  {
    if (!((i>0)&&(j>0))||!B[k*ps+(j-1)*rs+i-1]) G[z*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((j+1)<cs))||!B[k*ps+(j+1)*rs+i-1]) G[z*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(j>0))||!B[k*ps+(j-1)*rs+i+1]) G[z*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((j+1)<cs))||!B[k*ps+(j+1)*rs+i+1]) G[z*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
  if (((i+1)%2) + ((k+1)%2) == 2)
  {
    if (!((i>0)&&(k>0))||!B[(k-1)*ps+j*rs+i-1]) G[(z-1)*gps+y*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((k+1)<ds))||!B[(k+1)*ps+j*rs+i-1]) G[(z+1)*gps+y*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(k>0))||!B[(k-1)*ps+j*rs+i+1]) G[(z-1)*gps+y*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((k+1)<ds))||!B[(k+1)*ps+j*rs+i+1]) G[(z+1)*gps+y*grs+x+1] = VAL_OBJET;
  }
  if (((k+1)%2) + ((j+1)%2) == 2)
  {
    if (!((k>0)&&(j>0))||!B[(k-1)*ps+(j-1)*rs+i]) G[(z-1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (!((k>0)&&((j+1)<cs))||!B[(k-1)*ps+(j+1)*rs+i]) G[(z-1)*gps+(y+1)*grs+x] = VAL_OBJET;
    if (!(((k+1)<ds)&&(j>0))||!B[(k+1)*ps+(j-1)*rs+i]) G[(z+1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (!(((k+1)<ds)&&((j+1)<cs))||!B[(k+1)*ps+(j+1)*rs+i]) G[(z+1)*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if (((i+1)%2) + ((j+1)%2) + ((k+1)%2) == 3)
  {
    if (!((i>0)&&(j>0)&&(k>0))||!B[(k-1)*ps+(j-1)*rs+i-1]) G[(z-1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((j+1)<cs)&&(k>0))||!B[(k-1)*ps+(j+1)*rs+i-1]) G[(z-1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(j>0)&&(k>0))||!B[(k-1)*ps+(j-1)*rs+i+1]) G[(z-1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((j+1)<cs)&&(k>0))||!B[(k-1)*ps+(j+1)*rs+i+1]) G[(z-1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
    if (!((i>0)&&(j>0)&&((k+1)<ds))||!B[(k+1)*ps+(j-1)*rs+i-1]) G[(z+1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (!((i>0)&&((j+1)<cs)&&((k+1)<ds))||!B[(k+1)*ps+(j+1)*rs+i-1]) G[(z+1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if (!(((i+1)<rs)&&(j>0)&&((k+1)<ds))||!B[(k+1)*ps+(j-1)*rs+i+1]) G[(z+1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if (!(((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))||!B[(k+1)*ps+(j+1)*rs+i+1]) G[(z+1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
} /* CopieThetacarreCompl3d() */

/* ==================================== */
void CopieBetacarreDual3d(uint8_t *G, uint8_t *B, 
                           index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t ps = rs * cs;
  index_t x = (i+1) % 2;
  index_t y = (j+1) % 2;
  index_t z = (k+1) % 2;
  int32_t gps = GRS3D*GCS3D;
  int32_t grs = GRS3D;

#ifdef DEBUG
printf("CopieBetacarreDual3d\n");
#endif
  memset(G, VAL_NULLE, GRS3D*GCS3D*GDS3D); /* init a VAL_NULLE */
  if ((i+1) % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) G[z*gps+y*grs+x-1] = VAL_OBJET;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) G[z*gps+y*grs+x+1] = VAL_OBJET;
  }
  if ((j+1) % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) G[z*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) G[z*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if ((k+1) % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) G[(z-1)*gps+y*grs+x] = VAL_OBJET;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) G[(z+1)*gps+y*grs+x] = VAL_OBJET;
  }
  if (((i+1)%2) + ((j+1)%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) G[z*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) G[z*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) G[z*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) G[z*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
  if (((i+1)%2) + ((k+1)%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) G[(z-1)*gps+y*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) G[(z+1)*gps+y*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) G[(z-1)*gps+y*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) G[(z+1)*gps+y*grs+x+1] = VAL_OBJET;
  }
  if (((k+1)%2) + ((j+1)%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) G[(z-1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) G[(z-1)*gps+(y+1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) G[(z+1)*gps+(y-1)*grs+x] = VAL_OBJET;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) G[(z+1)*gps+(y+1)*grs+x] = VAL_OBJET;
  }
  if (((i+1)%2) + ((j+1)%2) + ((k+1)%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) G[(z-1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) G[(z-1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) G[(z-1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) G[(z-1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) G[(z+1)*gps+(y-1)*grs+x-1] = VAL_OBJET;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) G[(z+1)*gps+(y+1)*grs+x-1] = VAL_OBJET;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) G[(z+1)*gps+(y-1)*grs+x+1] = VAL_OBJET;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) G[(z+1)*gps+(y+1)*grs+x+1] = VAL_OBJET;
  }
} /* CopieBetacarreDual3d() */

/* ==================================== */
void MarqueThetacarre3d(uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  Marque (met a la valeur VAL_MARQUE) le thetacarre du point i,j,k de B.
*/    
{
  index_t ps = rs * cs;

  /* marque le Alphacarre */
  if (i % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) B[k*ps+j*rs+i-1] = VAL_MARQUE;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) B[k*ps+j*rs+i+1] = VAL_MARQUE;
  }
  if (j % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) B[k*ps+(j-1)*rs+i] = VAL_MARQUE;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) B[k*ps+(j+1)*rs+i] = VAL_MARQUE;
  }
  if (k % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) B[(k-1)*ps+j*rs+i] = VAL_MARQUE;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) B[(k+1)*ps+j*rs+i] = VAL_MARQUE;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) B[k*ps+(j-1)*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) B[k*ps+(j+1)*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) B[k*ps+(j-1)*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) B[k*ps+(j+1)*rs+i+1] = VAL_MARQUE;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) B[(k-1)*ps+j*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) B[(k+1)*ps+j*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) B[(k-1)*ps+j*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) B[(k+1)*ps+j*rs+i+1] = VAL_MARQUE;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) B[(k-1)*ps+(j-1)*rs+i] = VAL_MARQUE;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) B[(k-1)*ps+(j+1)*rs+i] = VAL_MARQUE;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) B[(k+1)*ps+(j-1)*rs+i] = VAL_MARQUE;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) B[(k+1)*ps+(j+1)*rs+i] = VAL_MARQUE;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) B[(k-1)*ps+(j-1)*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) B[(k-1)*ps+(j+1)*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) B[(k-1)*ps+(j-1)*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) B[(k-1)*ps+(j+1)*rs+i+1] = VAL_MARQUE;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) B[(k+1)*ps+(j-1)*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) B[(k+1)*ps+(j+1)*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) B[(k+1)*ps+(j-1)*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) B[(k+1)*ps+(j+1)*rs+i+1] = VAL_MARQUE;
  }

  /* marque le Betacarre */
  if ((i+1) % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) B[k*ps+j*rs+i-1] = VAL_MARQUE;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) B[k*ps+j*rs+i+1] = VAL_MARQUE;
  }
  if ((j+1) % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) B[k*ps+(j-1)*rs+i] = VAL_MARQUE;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) B[k*ps+(j+1)*rs+i] = VAL_MARQUE;
  }
  if ((k+1) % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) B[(k-1)*ps+j*rs+i] = VAL_MARQUE;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) B[(k+1)*ps+j*rs+i] = VAL_MARQUE;
  }
  if (((i+1)%2) + ((j+1)%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) B[k*ps+(j-1)*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) B[k*ps+(j+1)*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) B[k*ps+(j-1)*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) B[k*ps+(j+1)*rs+i+1] = VAL_MARQUE;
  }
  if (((i+1)%2) + ((k+1)%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) B[(k-1)*ps+j*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) B[(k+1)*ps+j*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) B[(k-1)*ps+j*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) B[(k+1)*ps+j*rs+i+1] = VAL_MARQUE;
  }
  if (((k+1)%2) + ((j+1)%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) B[(k-1)*ps+(j-1)*rs+i] = VAL_MARQUE;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) B[(k-1)*ps+(j+1)*rs+i] = VAL_MARQUE;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) B[(k+1)*ps+(j-1)*rs+i] = VAL_MARQUE;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) B[(k+1)*ps+(j+1)*rs+i] = VAL_MARQUE;
  }
  if (((i+1)%2) + ((j+1)%2) + ((k+1)%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) B[(k-1)*ps+(j-1)*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) B[(k-1)*ps+(j+1)*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) B[(k-1)*ps+(j-1)*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) B[(k-1)*ps+(j+1)*rs+i+1] = VAL_MARQUE;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) B[(k+1)*ps+(j-1)*rs+i-1] = VAL_MARQUE;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) B[(k+1)*ps+(j+1)*rs+i-1] = VAL_MARQUE;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) B[(k+1)*ps+(j-1)*rs+i+1] = VAL_MARQUE;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) B[(k+1)*ps+(j+1)*rs+i+1] = VAL_MARQUE;
  }
} /* MarqueThetacarre3d() */

/* ==================================== */
int32_t CardThetacarre3d(uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
  retourne le cardinal du thetacarre du point i,j,k de B.
  OBSOLETE - utiliser CardThetaCarre3d
*/    
{
  index_t ps = rs * cs;
  int32_t n = 0;

  /* Alphacarre */
  if (i % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) n++;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) n++;
  }
  if (j % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) n++;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) n++;
  }
  if (k % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) n++;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) n++;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) n++;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) n++;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) n++;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) n++;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) n++;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) n++;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) n++;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) n++;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) n++;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) n++;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) n++;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) n++;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) n++;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) n++;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) n++;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) n++;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) n++;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) n++;
  }

  /* Betacarre */
  if ((i+1) % 2)
  {
    if ((i>0)&&B[k*ps+j*rs+i-1]) n++;
    if (((i+1)<rs)&&B[k*ps+j*rs+i+1]) n++;
  }
  if ((j+1) % 2)
  {
    if ((j>0)&&B[k*ps+(j-1)*rs+i]) n++;
    if (((j+1)<cs)&&B[k*ps+(j+1)*rs+i]) n++;
  }
  if ((k+1) % 2)
  {
    if ((k>0)&&B[(k-1)*ps+j*rs+i]) n++;
    if (((k+1)<ds)&&B[(k+1)*ps+j*rs+i]) n++;
  }
  if (((i+1)%2) + ((j+1)%2) == 2)
  {
    if (((i>0)&&(j>0))&&B[k*ps+(j-1)*rs+i-1]) n++;
    if (((i>0)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i-1]) n++;
    if ((((i+1)<rs)&&(j>0))&&B[k*ps+(j-1)*rs+i+1]) n++;
    if ((((i+1)<rs)&&((j+1)<cs))&&B[k*ps+(j+1)*rs+i+1]) n++;
  }
  if (((i+1)%2) + ((k+1)%2) == 2)
  {
    if (((i>0)&&(k>0))&&B[(k-1)*ps+j*rs+i-1]) n++;
    if (((i>0)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i-1]) n++;
    if ((((i+1)<rs)&&(k>0))&&B[(k-1)*ps+j*rs+i+1]) n++;
    if ((((i+1)<rs)&&((k+1)<ds))&&B[(k+1)*ps+j*rs+i+1]) n++;
  }
  if (((k+1)%2) + ((j+1)%2) == 2)
  {
    if (((k>0)&&(j>0))&&B[(k-1)*ps+(j-1)*rs+i]) n++;
    if (((k>0)&&((j+1)<cs))&&B[(k-1)*ps+(j+1)*rs+i]) n++;
    if ((((k+1)<ds)&&(j>0))&&B[(k+1)*ps+(j-1)*rs+i]) n++;
    if ((((k+1)<ds)&&((j+1)<cs))&&B[(k+1)*ps+(j+1)*rs+i]) n++;
  }
  if (((i+1)%2) + ((j+1)%2) + ((k+1)%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i-1]) n++;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i-1]) n++;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&B[(k-1)*ps+(j-1)*rs+i+1]) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&B[(k-1)*ps+(j+1)*rs+i+1]) n++;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i-1]) n++;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i-1]) n++;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&B[(k+1)*ps+(j-1)*rs+i+1]) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&B[(k+1)*ps+(j+1)*rs+i+1]) n++;
  }
  return n;
} /* CardThetacarre3d() */

/* ==================================== */
int32_t CardThetaCarre3d(uint8_t *B, 
                     index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k,
                     uint8_t v)
/* ==================================== */
/* 
   retourne le nombre d'elements dans le thetacarre de (i,j,k) 
   dont la valeur est egale a v
*/
{
  index_t ps = rs * cs;
  int32_t n = 0;

  /* Alphacarre */
  if (i % 2)
  {
    if ((i>0)&&(B[k*ps+j*rs+i-1]==v)) n++;
    if (((i+1)<rs)&&(B[k*ps+j*rs+i+1]==v)) n++;
  }
  if (j % 2)
  {
    if ((j>0)&&(B[k*ps+(j-1)*rs+i]==v)) n++;
    if (((j+1)<cs)&&(B[k*ps+(j+1)*rs+i]==v)) n++;
  }
  if (k % 2)
  {
    if ((k>0)&&(B[(k-1)*ps+j*rs+i]==v)) n++;
    if (((k+1)<ds)&&(B[(k+1)*ps+j*rs+i]==v)) n++;
  }
  if ((i%2) + (j%2) == 2)
  {
    if (((i>0)&&(j>0))&&(B[k*ps+(j-1)*rs+i-1]==v)) n++;
    if (((i>0)&&((j+1)<cs))&&(B[k*ps+(j+1)*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(j>0))&&(B[k*ps+(j-1)*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((j+1)<cs))&&(B[k*ps+(j+1)*rs+i+1]==v)) n++;
  }
  if ((i%2) + (k%2) == 2)
  {
    if (((i>0)&&(k>0))&&(B[(k-1)*ps+j*rs+i-1]==v)) n++;
    if (((i>0)&&((k+1)<ds))&&(B[(k+1)*ps+j*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(k>0))&&(B[(k-1)*ps+j*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((k+1)<ds))&&(B[(k+1)*ps+j*rs+i+1]==v)) n++;
  }
  if ((k%2) + (j%2) == 2)
  {
    if (((k>0)&&(j>0))&&(B[(k-1)*ps+(j-1)*rs+i]==v)) n++;
    if (((k>0)&&((j+1)<cs))&&(B[(k-1)*ps+(j+1)*rs+i]==v)) n++;
    if ((((k+1)<ds)&&(j>0))&&(B[(k+1)*ps+(j-1)*rs+i]==v)) n++;
    if ((((k+1)<ds)&&((j+1)<cs))&&(B[(k+1)*ps+(j+1)*rs+i]==v)) n++;
  }
  if ((i%2) + (j%2) + (k%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&(B[(k-1)*ps+(j-1)*rs+i-1]==v)) n++;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&(B[(k-1)*ps+(j+1)*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&(B[(k-1)*ps+(j-1)*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&(B[(k-1)*ps+(j+1)*rs+i+1]==v)) n++;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&(B[(k+1)*ps+(j-1)*rs+i-1]==v)) n++;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&(B[(k+1)*ps+(j+1)*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&(B[(k+1)*ps+(j-1)*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&(B[(k+1)*ps+(j+1)*rs+i+1]==v)) n++;
  }

  /* Betacarre */
  if ((i+1) % 2)
  {
    if ((i>0)&&(B[k*ps+j*rs+i-1]==v)) n++;
    if (((i+1)<rs)&&(B[k*ps+j*rs+i+1]==v)) n++;
  }
  if ((j+1) % 2)
  {
    if ((j>0)&&(B[k*ps+(j-1)*rs+i]==v)) n++;
    if (((j+1)<cs)&&(B[k*ps+(j+1)*rs+i]==v)) n++;
  }
  if ((k+1) % 2)
  {
    if ((k>0)&&(B[(k-1)*ps+j*rs+i]==v)) n++;
    if (((k+1)<ds)&&(B[(k+1)*ps+j*rs+i]==v)) n++;
  }
  if (((i+1)%2) + ((j+1)%2) == 2)
  {
    if (((i>0)&&(j>0))&&(B[k*ps+(j-1)*rs+i-1]==v)) n++;
    if (((i>0)&&((j+1)<cs))&&(B[k*ps+(j+1)*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(j>0))&&(B[k*ps+(j-1)*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((j+1)<cs))&&(B[k*ps+(j+1)*rs+i+1]==v)) n++;
  }
  if (((i+1)%2) + ((k+1)%2) == 2)
  {
    if (((i>0)&&(k>0))&&(B[(k-1)*ps+j*rs+i-1]==v)) n++;
    if (((i>0)&&((k+1)<ds))&&(B[(k+1)*ps+j*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(k>0))&&(B[(k-1)*ps+j*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((k+1)<ds))&&(B[(k+1)*ps+j*rs+i+1]==v)) n++;
  }
  if (((k+1)%2) + ((j+1)%2) == 2)
  {
    if (((k>0)&&(j>0))&&(B[(k-1)*ps+(j-1)*rs+i]==v)) n++;
    if (((k>0)&&((j+1)<cs))&&(B[(k-1)*ps+(j+1)*rs+i]==v)) n++;
    if ((((k+1)<ds)&&(j>0))&&(B[(k+1)*ps+(j-1)*rs+i]==v)) n++;
    if ((((k+1)<ds)&&((j+1)<cs))&&(B[(k+1)*ps+(j+1)*rs+i]==v)) n++;
  }
  if (((i+1)%2) + ((j+1)%2) + ((k+1)%2) == 3)
  {
    if (((i>0)&&(j>0)&&(k>0))&&(B[(k-1)*ps+(j-1)*rs+i-1]==v)) n++;
    if (((i>0)&&((j+1)<cs)&&(k>0))&&(B[(k-1)*ps+(j+1)*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(j>0)&&(k>0))&&(B[(k-1)*ps+(j-1)*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&(k>0))&&(B[(k-1)*ps+(j+1)*rs+i+1]==v)) n++;
    if (((i>0)&&(j>0)&&((k+1)<ds))&&(B[(k+1)*ps+(j-1)*rs+i-1]==v)) n++;
    if (((i>0)&&((j+1)<cs)&&((k+1)<ds))&&(B[(k+1)*ps+(j+1)*rs+i-1]==v)) n++;
    if ((((i+1)<rs)&&(j>0)&&((k+1)<ds))&&(B[(k+1)*ps+(j-1)*rs+i+1]==v)) n++;
    if ((((i+1)<rs)&&((j+1)<cs)&&((k+1)<ds))&&(B[(k+1)*ps+(j+1)*rs+i+1]==v)) n++;
  }
  return n;
} /* CardThetaCarre3d() */

/* ========================================================================== */
/* ========================================================================== */
/* Simplicite, amincissement - 1ere definition GB (novembre 1998) */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
index_t EffaceBetaTerminauxSimples3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  struct xvimage *g;
  uint8_t *G;  
  index_t n = 0;

  g = AllocGrille3d();
  G = UCHARDATA(g);  
#ifdef DEBUGECS
printf("EffaceBetaTerminauxSimples3d : niveau %d\n", IndexPileGrilles3d);
#endif
  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if ((B[k*ps+j*rs+i]) && BetaTerminal3d(B, rs, cs, ds, i, j, k))
        {
#ifdef DEBUGCD
printf("Avant CopieAlphacarre : %d,%d,%d\n", rowsize(g), colsize(g), depth(g));
#endif
          CopieAlphacarre3d(G, B, rs, cs, ds, i, j, k);
#ifdef DEBUGCD
printf("Apres CopieAlphacarre : %d,%d,%d\n", rowsize(g), colsize(g), depth(g));
#endif
#ifdef DEBUGECS
printf("EffaceBetaTerminauxSimples3d(%d) : teste %d,%d,%d\n", IndexPileGrilles3d, i, j, k);
#endif
          EffaceLiensBetaLibres3d(g);
          if (EnsembleSimple3d(g)) 
          { 
            B[k*ps+j*rs+i] = VAL_NULLE; n++; 
#ifdef DEBUGECS
printf("EffaceBetaTerminauxSimples3d(%d) : efface %d,%d,%d\n", IndexPileGrilles3d, i, j, k);
#endif
          }
        }
  FreeGrille3d();
  return n;
} /* EffaceBetaTerminauxSimples3d() */

/* ==================================== */
int32_t EnsembleSimple3d(struct xvimage *b)
/* ==================================== */
{
#ifdef DEBUGES
printf("EnsembleSimple3d :\n");
printimage(b);
#endif
  if (ExactementUnBetaTerminal3d(UCHARDATA(b), rowsize(b), colsize(b), depth(b))) return 1;
  if (EffaceBetaTerminauxSimples3d(b) == 0) return 0;
  EffaceLiensBetaLibres3d(b);
  return EnsembleSimple3d(b);
} /* EnsembleSimple3d() */

/* ==================================== */
int32_t BetaTerminalSimple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  int32_t ret;

  if (!B[k*ps+j*rs+i]) return 0;
  if (!BetaTerminal3d(B, rs, cs, ds, i, j, k)) return 0;
  g = AllocGrille3d();
  G = UCHARDATA(g);
  CopieAlphacarre3d(G, B, rs, cs, ds, i, j, k);
  EffaceLiensBetaLibres3d(g);
  ret = EnsembleSimple3d(g);
  FreeGrille3d();
  return ret;
} /* BetaTerminalSimple3d() */

/* ========================================================================== */
/* ========================================================================== */
/* Simplicite, amincissement - 2ere definition GB (decembre 1998) */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
int32_t ContientUnSeulElement3d(struct xvimage *b)
/* ==================================== */
{
  index_t N = rowsize(b) * colsize(b) * depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, n = 0;
  for (i = 0; i < N; i++) if (B[i]) { n++; if (n > 1) return 0; }
  return (n == 1);
} /* ContientUnSeulElement3d() */

/* ==================================== */
int32_t Alpha1Simple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, n, nn = 0;

#ifdef DEBUGNEW
printf("Alpha1Simple3d : %d %d %d", i, j, k);
if (rowsize(b) == 3) printimage(b);
#endif

  if (!B[k*ps+j*rs+i]) return 0;
  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

#ifdef DEBUGNEW
printf("Alpha1Simple3d : nn = %d\n\n", n);
#endif

  return (nn == 1);
} /* Alpha1Simple3d() */

/* ==================================== */
int32_t Beta1Simple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, n, nn = 0;

#ifdef DEBUGNEW
printf("Beta1Simple3d : %d %d %d", i, j, k);
if (rowsize(b) == 3) printimage(b);
#endif

  if (!B[k*ps+j*rs+i]) return 0;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

#ifdef DEBUGNEW
printf("Beta1Simple3d : nn = %d\n\n", n);
#endif

  return (nn == 1);
} /* Beta1Simple3d() */

/* ==================================== */
index_t EffaceAlpha1Simples3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ps * ds;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k, nbsimples = 0;

  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        if (B[k*ps+j*rs+i])
          if (Alpha1Simple3d(b, i, j, k)) B[k*ps+j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceAlpha1Simples3d() */

/* ==================================== */
index_t EffaceBeta1Simples3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ps * ds;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k, nbsimples = 0;

  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        if (B[k*ps+j*rs+i])
          if (Beta1Simple3d(b, i, j, k)) B[k*ps+j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceBeta1Simples3d() */

/* ==================================== */
int32_t Ensemble1Contractile3d(struct xvimage *b)
/* ==================================== */
{
  int32_t stabilite;

#ifdef DEBUGNEW
printf("Ensemble1Contractile3d : avant effacement :\n");
if (rowsize(b) == 3) printimage(b);
#endif

  do
  {
    stabilite = 1;
    if (EffaceBeta1Simples3d(b)) stabilite = 0;
    if (EffaceAlpha1Simples3d(b)) stabilite = 0;
  } while (!stabilite);

#ifdef DEBUGNEW
printf("Ensemble1Contractile3d : apres effacement :\n");
if (rowsize(b) == 3) printimage(b);
#endif

  return ContientUnSeulElement3d(b);
} /* Ensemble1Contractile3d() */

/* ==================================== */
int32_t Alpha2Simple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  index_t tab;  /* resultats des tests */

#ifndef TEST_SIMPLICITE_RECURSIF
#ifndef TEST_SIMPLICITE_TABULE
  fprintf(stderr, "Alpha2Simple3d: ERR0: Il faut choisir !!!\n");
  exit(0);
#endif
#endif

#ifdef DEBUGNEW
printf("Alpha2Simple3d : %d %d %d\n", i, j, k);
if (rowsize(b) == 3) printimage(b);
#endif

  if (!B[k*ps+j*rs+i]) return 0;

#ifdef TEST_SIMPLICITE_RECURSIF
  g = AllocGrille3d();
  G = UCHARDATA(g);
  CopieAlphacarre3d(G, B, rs, cs, ds, i, j, k);
  rec = Ensemble1Contractile3d(g);

#ifdef DEBUGNEW
printf("Alpha2Simple3d : retourne %d\n\n", rec);
#endif

  FreeGrille3d();
#endif

#ifdef TEST_SIMPLICITE_TABULE
  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
        g = AllocGrille3d();   /* pas de table : version recursive */ 
        G = UCHARDATA(g);
        CopieAlphacarre3d(G, B, rs, cs, ds, i, j, k);
        tab = Ensemble1Contractile3d(g);
        FreeGrille3d();
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        tab = CzAlpha2s[XAlphacarre3d(B, rs, cs, ds, i, j, k)];
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        tab = CyAlpha2s[XAlphacarre3d(B, rs, cs, ds, i, j, k)];
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        tab = IxAlpha2s[XAlphacarre3d(B, rs, cs, ds, i, j, k)];
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        tab = CxAlpha2s[XAlphacarre3d(B, rs, cs, ds, i, j, k)];
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        tab = IyAlpha2s[XAlphacarre3d(B, rs, cs, ds, i, j, k)];
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        tab = IzAlpha2s[XAlphacarre3d(B, rs, cs, ds, i, j, k)];
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
        tab = 0;
      }
    }
  }
#endif

#ifdef TEST_SIMPLICITE_RECURSIF
#ifdef TEST_SIMPLICITE_TABULE
  if ((rec && !tab) || (!rec && tab))
  {
    int32_t d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
    fprintf(stderr, "Alpha2Simple3d: ERR2: rec = %d ; tab = %d ; i = %d ; j = %d ; k = %d ; XAlpha = %d\n", 
           rec, tab, i, j, k, d);
    fprintf(stderr, "Cx = %d ; Cy = %d ; Cz = %d\n", CxAlpha2s[d], CyAlpha2s[d], CzAlpha2s[d]);
    printimage(g);
    exit(0);
  }
#endif
#endif

#ifdef TEST_SIMPLICITE_TABULE
  return tab;
#endif
#ifdef TEST_SIMPLICITE_RECURSIF
  return rec;
#endif
} /* Alpha2Simple3d() */

/* ==================================== */
int32_t Beta2Simple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  index_t tab;  /* resultats des tests */

#ifndef TEST_SIMPLICITE_RECURSIF
#ifndef TEST_SIMPLICITE_TABULE
  fprintf(stderr, "Beta2Simple3d: ERR0: Il faut choisir !!!\n");
  exit(0);
#endif
#endif

#ifdef DEBUGNEW
printf("Beta2Simple3d : %d %d %d\n", i, j, k);
if (rowsize(b) == 3) printimage(b);
#endif

  if (!B[k*ps+j*rs+i]) return 0;

#ifdef TEST_SIMPLICITE_RECURSIF

  g = AllocGrille3d();
  G = UCHARDATA(g);
  CopieBetacarreDual3d(G, B, rs, cs, ds, i, j, k);
  rec = Ensemble1Contractile3d(g);

#ifdef DEBUGNEW
printf("Beta2Simple3d : retourne %d\n\n", rec);
#endif

  FreeGrille3d();
#endif

#ifdef TEST_SIMPLICITE_TABULE
  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
        tab = 0;
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        tab = CzBeta2s[XBetacarre3d(B, rs, cs, ds, i, j, k)];
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        tab = CyBeta2s[XBetacarre3d(B, rs, cs, ds, i, j, k)];
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        tab = IxBeta2s[XBetacarre3d(B, rs, cs, ds, i, j, k)];
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        tab = CxBeta2s[XBetacarre3d(B, rs, cs, ds, i, j, k)];
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        tab = IyBeta2s[XBetacarre3d(B, rs, cs, ds, i, j, k)];
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        tab = IzBeta2s[XBetacarre3d(B, rs, cs, ds, i, j, k)];
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
        g = AllocGrille3d();   /* pas de table : version recursive */ 
        G = UCHARDATA(g);
        CopieBetacarreDual3d(G, B, rs, cs, ds, i, j, k);
        tab = Ensemble1Contractile3d(g);
        FreeGrille3d();
      }
    }
  }
#endif

#ifdef TEST_SIMPLICITE_RECURSIF
#ifdef TEST_SIMPLICITE_TABULE
  if ((rec && !tab) || (!rec && tab))
  {
    int32_t d = XBetacarre3d(B, rs, cs, ds, i, j, k);
    fprintf(stderr, "Beta2Simple3d: ERR2: rec = %d ; tab = %d ; i = %d ; j = %d ; k = %d ; XBeta = %d\n", 
           rec, tab, i, j, k, d);
    fprintf(stderr, "Ix = %d ; Iy = %d ; Iz = %d\n", IxBeta2s[d], IyBeta2s[d], IzBeta2s[d]);
    printimage(g);
    exit(0);
  }
#endif
#endif

#ifdef TEST_SIMPLICITE_TABULE
  return tab;
#endif
#ifdef TEST_SIMPLICITE_RECURSIF
  return rec;
#endif
} /* Beta2Simple3d() */

/* ==================================== */
index_t EffaceAlpha2Simples3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ps * ds;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k, nbsimples = 0;

  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        if (B[k*ps+j*rs+i])
          if (Alpha2Simple3d(b, i, j, k)) B[k*ps+j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceAlpha2Simples3d() */

/* ==================================== */
index_t EffaceBeta2Simples3d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ps * ds;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k, nbsimples = 0;

  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        if (B[k*ps+j*rs+i])
          if (Beta2Simple3d(b, i, j, k)) B[k*ps+j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceBeta2Simples3d() */

/* ==================================== */
int32_t Ensemble2Contractile3d(struct xvimage *b)
/* ==================================== */
{
  int32_t stabilite;

#ifdef DEBUGNEW
printf("Ensemble2Contractile3d : avant effacement :\n");
if (rowsize(b) == 3) printimage(b);
#endif

  do
  {
    stabilite = 1;
    if (EffaceBeta2Simples3d(b)) stabilite = 0;
    if (EffaceAlpha2Simples3d(b)) stabilite = 0;
  } while (!stabilite);

#ifdef DEBUGNEW
printf("Ensemble2Contractile3d : apres effacement :\n");
if (rowsize(b) == 3) printimage(b);
#endif

  return ContientUnSeulElement3d(b);
} /* Ensemble2Contractile3d() */

/* ==================================== */
int32_t Alpha3Simple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
    Renvoie VRAI si e point (i,j,k) est alpha-simple pour l'ordre b,
            FAUX sinon

    Version efficace (BDD)
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  int32_t ret;
  uint32_t d;

  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
	d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleCube(
          d&0x1,       d&0x2,       d&0x4,       d&0x8,    
          d&0x10,      d&0x20,      d&0x40,      d&0x80,
          d&0x100,     d&0x200,     d&0x400,     d&0x800,  
          d&0x1000,    d&0x2000,    d&0x4000,    d&0x8000,
          d&0x10000,   d&0x20000,   d&0x40000,   d&0x80000, 
          d&0x100000,  d&0x200000,  d&0x400000,  d&0x800000,
          d&0x1000000, d&0x2000000
	  );
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleCarreZ(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleCarreY(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleInterX(d&0x1, d&0x2);
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleCarreX(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleInterY(d&0x1, d&0x2);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleInterZ(d&0x1, d&0x2);
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
        ret = 0;
      }
    }
  }
  return ret;
} /* Alpha3Simple3d() */

/* ==================================== */
int32_t Beta3Simple3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
    Renvoie VRAI si e point (i,j,k) est beta-simple pour l'ordre b,
            FAUX sinon

    Version efficace (BDD)
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  int32_t ret;
  uint32_t d;

  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
        ret = 0;
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddBeta3SimpleCarreZ(d&0x1, d&0x2);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddBeta3SimpleCarreY(d&0x1, d&0x2);
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddBeta3SimpleInterX(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddBeta3SimpleCarreX(d&0x1, d&0x2);
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddBeta3SimpleInterY(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddBeta3SimpleInterZ(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
	d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddAlpha3SimpleCube(            /* Alpha3,Cube : sic (dualite) */
          d&0x1,       d&0x2,       d&0x4,       d&0x8,    
          d&0x10,      d&0x20,      d&0x40,      d&0x80,
          d&0x100,     d&0x200,     d&0x400,     d&0x800,  
          d&0x1000,    d&0x2000,    d&0x4000,    d&0x8000,
          d&0x10000,   d&0x20000,   d&0x40000,   d&0x80000, 
          d&0x100000,  d&0x200000,  d&0x400000,  d&0x800000,
          d&0x1000000, d&0x2000000
	  );
      }
    }
  }

  return ret;
} /* Beta3Simple3d() */

/* ==================================== */
int32_t Alpha3Simpleh3d(struct xvimage *b, index_t i, index_t j, index_t k, int32_t h)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  int32_t ret;
  uint32_t d;

  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
	d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleCube(
          d&0x1,       d&0x2,       d&0x4,       d&0x8,    
          d&0x10,      d&0x20,      d&0x40,      d&0x80,
          d&0x100,     d&0x200,     d&0x400,     d&0x800,  
          d&0x1000,    d&0x2000,    d&0x4000,    d&0x8000,
          d&0x10000,   d&0x20000,   d&0x40000,   d&0x80000, 
          d&0x100000,  d&0x200000,  d&0x400000,  d&0x800000,
          d&0x1000000, d&0x2000000
	  );
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleCarreZ(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleCarreY(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleInterX(d&0x1, d&0x2);
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleCarreX(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleInterY(d&0x1, d&0x2);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        d = XhAlphacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleInterZ(d&0x1, d&0x2);
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
        ret = 0;
      }
    }
  }
  return ret;
} /* Alpha3Simpleh3d() */

/* ==================================== */
int32_t Beta3Simpleh3d(struct xvimage *b, index_t i, index_t j, index_t k, int32_t h)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  int32_t ret;
  uint32_t d;

  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
        ret = 0;
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddBeta3SimpleCarreZ(d&0x1, d&0x2);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddBeta3SimpleCarreY(d&0x1, d&0x2);
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddBeta3SimpleInterX(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddBeta3SimpleCarreX(d&0x1, d&0x2);
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddBeta3SimpleInterY(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddBeta3SimpleInterZ(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80);
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
	d = XhBetacarre3d(B, rs, cs, ds, i, j, k, h);
        ret = bddAlpha3SimpleCube(            /* Alpha3,Cube : sic (dualite) */
          d&0x1,       d&0x2,       d&0x4,       d&0x8,    
          d&0x10,      d&0x20,      d&0x40,      d&0x80,
          d&0x100,     d&0x200,     d&0x400,     d&0x800,  
          d&0x1000,    d&0x2000,    d&0x4000,    d&0x8000,
          d&0x10000,   d&0x20000,   d&0x40000,   d&0x80000, 
          d&0x100000,  d&0x200000,  d&0x400000,  d&0x800000,
          d&0x1000000, d&0x2000000
	  );
      }
    }
  }

  return ret;
} /* Beta3Simpleh3d() */

/* ==================================== */
int32_t Simpleh3d(struct xvimage *b, index_t i, index_t j, index_t k, int32_t h)
/* ==================================== */
{
  return Beta3Simpleh3d(b, i, j, k, h) || Alpha3Simpleh3d(b, i, j, k, h);
} /* Simpleh3d() */

/* ==================================== */
int32_t Alpha3Simple3dDefinitionRecursive(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  int32_t ret;

#ifdef DEBUGNEW
printf("Alpha3Simple3d : %d %d %d\n", i, j, k);
if (rowsize(b) == 3) printimage(b);
#endif

  if (SINGL3D(i,j,k)) return 0;
  g = AllocGrille3d();
  G = UCHARDATA(g);
  CopieAlphacarre3d(G, B, rs, cs, ds, i, j, k);
  if (INTER3D(i,j,k))
    ret = ContientUnSeulElement3d(g);
  else
    ret = Ensemble2Contractile3d(g);
  FreeGrille3d();

#ifdef DEBUGNEW
printf("Alpha3Simple3d : retourne %d\n", ret);
#endif

  return ret;
} /* Alpha3Simple3dDefinitionRecursive() */

/* ==================================== */
int32_t Beta3Simple3dDefinitionRecursive(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  int32_t ret;

#ifdef DEBUGNEW
printf("Beta3Simple3d : %d %d %d\n", i, j, k);
if (rowsize(b) == 3) printimage(b);
#endif

  if (CUBE3D(i,j,k)) return 0;
  g = AllocGrille3d();
  G = UCHARDATA(g);
  CopieBetacarreDual3d(G, B, rs, cs, ds, i, j, k);
  if (CARRE3D(i,j,k))
    ret = ContientUnSeulElement3d(g);
  else
    ret = Ensemble2Contractile3d(g);
  FreeGrille3d();

#ifdef DEBUGNEW
printf("Beta3Simple3d : retourne %d\n", ret);
#endif

  return ret;
} /* Beta3Simple3dDefinitionRecursive() */

/* ========================================================================== */
/* ========================================================================== */
/* connexite, courbe simple */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
static void PropageRec(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t tab[GRS3D*GCS3D*GDS3D]; 
  int32_t u, n;
  index_t ps = rs*cs;

  B[k * rs * cs + j * rs + i] = VAL_MARQUE;
  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) 
    if (B[tab[u]] == VAL_OBJET) 
      PropageRec(B, rs, cs, ds, tab[u] % rs, (tab[u] % ps) / rs, tab[u] / ps);  
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) 
    if (B[tab[u]] == VAL_OBJET) 
      PropageRec(B, rs, cs, ds, tab[u] % rs, (tab[u] % ps) / rs, tab[u] / ps);  
} // PropageRec()

/* ==================================== */
int32_t Connexe3d(struct xvimage *b)
/* ==================================== */
/*
  Suppose que tous les points objet ont pour valeur VAL_OBJET.
  On repere un point objet (si non existe, retour 1: l'ensemble vide est connexe)
  et on lance la propagation recursive du marqueur VAL_MARQUE dans la comp. connexe
  du point.
  Enfin, on verifie que tous les points ont ete marques VAL_MARQUE.

  Attention: cette fonction laisse les points a la valeur VAL_MARQUE.
*/

{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ds * ps;
  uint8_t *B = UCHARDATA(b);
  index_t i;

  for (i = 0; i < N; i++)
    if (B[i] == VAL_OBJET)
    {
      PropageRec(B, rs, cs, ds, i % rs, (i % ps) / rs, i / ps);
      break;
    }
  if (i == N) return 1; /* ens. vide connexe */

  for (i = 0; i < N; i++) 
    if ((B[i] != VAL_MARQUE) && (B[i] != VAL_NULLE)) return 0;
  return 1;
} /* Connexe3d() */

/* ==================================== */
static void PropageRecVal(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, uint8_t Val)
/* ==================================== */
{
  index_t tab[GRS3D*GCS3D*GDS3D]; 
  int32_t u, n;
  index_t ps = rs*cs;

  B[k * rs * cs + j * rs + i] = VAL_MARQUE;
  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) 
    if (B[tab[u]] == Val) 
      PropageRecVal(B, rs, cs, ds, tab[u] % rs, (tab[u] % ps) / rs, tab[u] / ps, Val);  
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) 
    if (B[tab[u]] == Val) 
      PropageRecVal(B, rs, cs, ds, tab[u] % rs, (tab[u] % ps) / rs, tab[u] / ps, Val);  
} // PropageRecVal()

/* ==================================== */
int32_t Connexe3dVal(struct xvimage *b, uint8_t Val)
/* ==================================== */
/*
  Suppose que tous les points objet ont pour valeur Val.
  On repere un point objet (si non existe, retour 1: l'ensemble vide est connexe)
  et on lance la propagation recursive du marqueur VAL_MARQUE dans la comp. connexe
  du point.
  Enfin, on verifie que tous les points ont ete marques VAL_MARQUE.

  Attention: cette fonction laisse les points a la valeur VAL_MARQUE.
*/

{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ds * ps;
  uint8_t *B = UCHARDATA(b);
  index_t i;

  for (i = 0; i < N; i++)
    if (B[i] == Val)
    {
      PropageRecVal(B, rs, cs, ds, i % rs, (i % ps) / rs, i / ps, Val);
      break;
    }
  if (i == N) return 1; /* ens. vide connexe */

  for (i = 0; i < N; i++) 
    if (B[i] == Val) return 0;
  return 1;
} /* Connexe3dVal() */

/* ==================================== */
int32_t Prop0CycleBetaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/*
  cette fonction verifie que l'ensemble des successeurs de i,j,k est compose d'exactement 
  deux alpha-terminaux
*/
{
  index_t ps = rs * cs;
  int32_t u, v, tu, tv, n, nn;
  index_t tab[GRS3D*GCS3D*GDS3D];

  Alphacarre3d(rs, cs, ds, i, j, k, tab, &n);
  if (n < 2) return 0;
  nn = n;

  for (u = 0; u < n; u++)
  {
    tu = tab[u];
    if ((tu != -1) && !B[tu])
    { tab[u] = -1; if (--nn < 2) return 0; }
    else
      for (v = u+1; v < n; v++) /* elimine les succ. non maximaux */
      {
        tv = tab[v];
        if ((tv != -1) && B[tv])
        {
          if (Succede3d(tv%rs,(tv%ps)/rs,tv/ps,tu%rs,(tu%ps)/rs,tu/ps)) 
          { tab[u] = -1; if(--nn<2) return 0; break; }
          if (Succede3d(tu%rs,(tu%ps)/rs,tu/ps,tv%rs,(tv%ps)/rs,tv/ps)) 
          { tab[v] = -1; nn--; }
	}
      }
  }
  if (nn > 2) return 0;
  for (u = 0; u < n; u++)
  {
    tu = tab[u];
    if ((tu != -1) && !AlphaTerminal3d(B, rs, cs, ds, tu%rs, (tu%ps)/rs, tu/ps)) return 0;
  }
  return 1;
} /* Prop0CycleBetaTerminal3d() */

/* ==================================== */
int32_t Prop0CycleAlphaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
/*
  cette fonction verifie que l'ensemble des predecesseurs de i,j,k est compose d'exactement 
  deux cliques
*/
{
  index_t ps = rs * cs;
  int32_t u, v, tu, tv, n, nn;
  index_t tab[GRS3D*GCS3D*GDS3D];

  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
#ifdef DEBUGP0CA
printf("Prop0CycleAlphaTerminal3d (%d %d %d) n = %d \n",i,j,k,n);
#endif
  if (n < 2) return 0;
  nn = n;

  for (u = 0; u < n; u++)
  {
    tu = tab[u];
    if ((tu != -1) && !B[tu])
    { tab[u] = -1; if (--nn < 2) return 0; }
    else
      for (v = u+1; v < n; v++) /* elimine les pred. non minimaux */
      {
        tv = tab[v];
        if ((tv != -1) && B[tv])
        {
          if (Precede3d(tv%rs,(tv%ps)/rs,tv/ps,tu%rs,(tu%ps)/rs,tu/ps)) {tab[u]=-1;if(--nn<2)return 0;break;}
          if (Precede3d(tu%rs,(tu%ps)/rs,tu/ps,tv%rs,(tv%ps)/rs,tv/ps)) {tab[v] = -1; nn--;}
	}
      }
  }
#ifdef DEBUGP0CA
printf("Prop0CycleAlphaTerminal3d (%d %d %d) nn = %d  : ",i,j,k,nn);
for (u = 0; u < n; u++)
{
  tu = tab[u];
  if (tu != -1) printf("(%d,%d,%d) ", tu%rs, (tu%ps)/rs, tu/ps);
}
printf("\n");
#endif
  if (nn > 2) return 0;
  for (u = 0; u < n; u++)
  {
    tu = tab[u];
    if ((tu != -1) && !BetaTerminal3d(B, rs, cs, ds, tu%rs, (tu%ps)/rs, tu/ps)) return 0;
  }
  return 1;
} /* Prop0CycleAlphaTerminal3d() */

/* ==================================== */
int32_t CourbeSimple3dOld(struct xvimage *b)   /* obsolete */
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n = 0;

  if (!Connexe3d(b)) return 0;

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[k * ps + j * rs + i])
        {
          n++;
          if (BetaTerminal3d(B, rs, cs, ds, i, j, k) && !Prop0CycleBetaTerminal3d(B, rs, cs, ds, i, j, k)) 
            return 0;
          if (AlphaTerminal3d(B, rs, cs, ds, i, j, k) && !Prop0CycleAlphaTerminal3d(B, rs, cs, ds, i, j, k)) 
            return 0;
        }
  if (n == 0) return 0;
  return 1;
}  /* CourbeSimple3dOld() */

/* ==================================== */
int32_t CourbeSimple3d(struct xvimage *b)
/* ==================================== */
/*
  Teste si b contient une courbe simple fermee, ie.
    pour tout x, le card. de thetacarre(x) vaut 2.
  Attention : on ne teste pas si les deux theta-voisins d'un point sont 
    non theta-voisins entre eux.
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n = 0;

#ifdef DEBUGCS
printf("CourbeSimple3d :\n");
printimage(b);
#endif

  if (!Connexe3d(b)) return 0;

#ifdef DEBUGCS
printf("CourbeSimple3d : connexite ok\n");
#endif

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[k * ps + j * rs + i])
        {
          n++;
          if (CardThetacarre3d(B, rs, cs, ds, i, j, k) != 2) return 0;
        } 
  if (n == 0) return 0;
  return 1;
}  /* CourbeSimple3d() */

/* ==================================== */
int32_t CourbeSimpleOuverte3d(struct xvimage *b)
/* ==================================== */
/*
  Teste si b contient une courbe simple ouverte.
  Un point isole est une courbe simple ouverte par convention.
  Sinon, tous les points sauf 2 ont un thetacarre de cardinal 2,
    les 2 autres ont un thetacarre de cardinal 1.
  Attention : on ne teste pas si les deux theta-voisins d'un point sont 
    non theta-voisins entre eux.
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  index_t n = 0;    /* nombre total de points */
  index_t nend = 0; /* nombre total de points extremites */
  int32_t ctc;

#ifdef DEBUGCS
printf("CourbeSimple3d :\n");
printimage(b);
#endif

  if (!Connexe3d(b)) return 0;

#ifdef DEBUGCS
printf("CourbeSimple3d : connexite ok\n");
#endif

  for (k = 0; k < ds; k += 1)
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[k * ps + j * rs + i])
        {
          n++;
          ctc = CardThetacarre3d(B, rs, cs, ds, i, j, k);
          if (ctc > 2) return 0;
          if (ctc == 1) nend++;
        } 
  if (n == 0) return 0;
  if (n == 1) return 1;
  if (nend == 2) return 1;
  return 0;
}  /* CourbeSimpleOuverte3d() */

/* ==================================== */
int32_t Arbre3d(struct xvimage *b)
/* ==================================== */
/*
  Teste si b contient un arbre. 
  Methode: retire des points extremites de courbe jusqu'a stabilite. 
  S'il ne reste qu'un point, c'est qu'on avait un arbre.
  ATTENTION: detruit les donnees de b.
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j, k;
  int32_t stabilite;

  do
  {
    stabilite = 1;
    for (k = 0; k < ds; k += 1)
      for (j = 0; j < cs; j += 1)
        for (i = 0; i < rs; i += 1)
          if (B[k * ps + j * rs + i])
            if (CardThetacarre3d(B, rs, cs, ds, i, j, k) == 1)
	    {
              B[k * ps + j * rs + i] = 0;
              stabilite = 0;
	    }
  } while (!stabilite);

  return ContientUnSeulElement3d(b);
}  /* Arbre3d() */

/* ========================================================================== */
/* ========================================================================== */
/* nombres topologiques */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
int32_t NbCompConnexe3d(struct xvimage *b)
/* ==================================== */
/*
  Suppose que tous les points objet ont pour valeur VAL_OBJET.
  On repere un point objet non marque (si non existe, retour 0)
  et on lance la propagation recursive du marqueur VAL_MARQUE dans la comp. connexe
  du point. Le compteur de composantes est incremente, et on recommence.
  
  Attention: cette fonction laisse les points a la valeur VAL_MARQUE.
*/

{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  index_t N = ds * ps;
  uint8_t *B = UCHARDATA(b);
  int32_t i, nbcomp = 0;

  for (i = 0; i < N; i++)
    if (B[i] == VAL_OBJET)
    {
      PropageRec(B, rs, cs, ds, i % rs, (i % ps) / rs, i / ps);
      nbcomp++;
    }
  return nbcomp;
} /* NbCompConnexe3d() */

/* ==================================== */
int32_t T3d(struct xvimage *b, index_t i, index_t j, index_t k, struct xvimage *g)
/* ==================================== */
/* 
  Retourne le nombre de composantes connexes du
    thetacarre du point i,j,k de b.
  g (image temporaire) doit etre allouee de taille 7x7x7.
*/    
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  uint8_t *G = UCHARDATA(g);

  CopieThetacarre3d(G, B, rs, cs, ds, i, j, k);
  return NbCompConnexe3d(g);
} /* T3D() */

/* ==================================== */
int32_t Tbar3d(struct xvimage *b, index_t i, index_t j, index_t k, struct xvimage *g)
/* ==================================== */
/* 
  Retourne le nombre de composantes connexes du
    thetacarre du point i,j,k du complementaire de b.
  g (image temporaire) doit etre allouee de taille 7x7x7.
*/    
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  uint8_t *G = UCHARDATA(g);

  CopieThetacarreCompl3d(G, B, rs, cs, ds, i, j, k);
  return NbCompConnexe3d(g);
} /* Tbar3D() */

/* ========================================================================== */
/* ========================================================================== */
/* Points terminaux de surface (idem arbre, mais appelle des BDD) */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
int32_t Surfend3d_SANSBDD(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;

  g = allocimage(NULL, 7, 7, 7, VFF_TYP_1_BYTE);
  G = UCHARDATA(g);
  CopieThetacarre3d(G, B, rs, cs, ds, i, j, k); 
  return Arbre3d(g);  
}

/* ==================================== */
int32_t Surfend3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
/* 
    Renvoie VRAI si e point (i,j,k) est un bord de surface dans l'ordre b,
            FAUX sinon
*/
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  int32_t ret;
  uint32_t d, e;

  if (i%2)               /* coord x impaire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 1 1 : cube */
	d = XAlphacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddSurfendCube(
          d&0x1,       d&0x2,       d&0x4,       d&0x8,    
          d&0x10,      d&0x20,      d&0x40,      d&0x80,
          d&0x100,     d&0x200,     d&0x400,     d&0x800,  
          d&0x1000,    d&0x2000,    d&0x4000,    d&0x8000,
          d&0x10000,   d&0x20000,   d&0x40000,   d&0x80000, 
          d&0x100000,  d&0x200000,  d&0x400000,  d&0x800000,
          d&0x1000000, d&0x2000000
	  );
      }
      else                   /* coord z paire */
      {                      /* 1 1 2 : carreZ */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k); /* 8 bits */
        e = XBetacarre3d(B, rs, cs, ds, i, j, k);  /* 2 bits */
        ret = bddSurfendCarreZ(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80,
                               e&0x1, e&0x2);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 1 2 1 : carreY */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k); /* 8 bits */
        e = XBetacarre3d(B, rs, cs, ds, i, j, k);  /* 2 bits */
        ret = bddSurfendCarreY(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80,
                               e&0x1, e&0x2);
      }
      else                   /* coord z paire */
      {                      /* 1 2 2 : interX */
        e = XAlphacarre3d(B, rs, cs, ds, i, j, k); /* 2 bits */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);  /* 8 bits */
        ret = bddSurfendCarreX(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80,
                               e&0x1, e&0x2);
      }
    }
  }
  else                   /* coord x paire */
  {
    if (j%2)               /* coord y impaire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 1 1 : carreX */
        d = XAlphacarre3d(B, rs, cs, ds, i, j, k); /* 8 bits */
        e = XBetacarre3d(B, rs, cs, ds, i, j, k);  /* 2 bits */
        ret = bddSurfendCarreX(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80,
                               e&0x1, e&0x2);
      }
      else                   /* coord z paire */
      {                      /* 2 1 2 : interY */
        e = XAlphacarre3d(B, rs, cs, ds, i, j, k); /* 2 bits */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);  /* 8 bits */
        ret = bddSurfendCarreY(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80,
                               e&0x1, e&0x2);
      }
    }
    else                   /* coord y paire */
    {
      if (k%2)               /* coord z impaire */
      {                      /* 2 2 1 : interZ */
        e = XAlphacarre3d(B, rs, cs, ds, i, j, k); /* 2 bits */
        d = XBetacarre3d(B, rs, cs, ds, i, j, k);  /* 8 bits */
        ret = bddSurfendCarreZ(d&0x1, d&0x2, d&0x4, d&0x8, d&0x10, d&0x20, d&0x40, d&0x80,
                               e&0x1, e&0x2);
      }
      else                   /* coord z paire */
      {                      /* 2 2 2 : singl */
	d = XBetacarre3d(B, rs, cs, ds, i, j, k);
        ret = bddSurfendCube(
          d&0x1,       d&0x2,       d&0x4,       d&0x8,    
          d&0x10,      d&0x20,      d&0x40,      d&0x80,
          d&0x100,     d&0x200,     d&0x400,     d&0x800,  
          d&0x1000,    d&0x2000,    d&0x4000,    d&0x8000,
          d&0x10000,   d&0x20000,   d&0x40000,   d&0x80000, 
          d&0x100000,  d&0x200000,  d&0x400000,  d&0x800000,
          d&0x1000000, d&0x2000000
	  );
      }
    }
  }
  return ret;
} /* Surfend3d() */

/* ========================================================================== */
/* ========================================================================== */
/* TEST POINT ET PAIRE SIMPLE PAR COLLAPSE */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
int32_t FaceLibre3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
// Détermine si la face (i,j,k) est libre dans le complexe b, c'est-a-dire si 
// elle est strictement incluse dans exactement une face de b.
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, n, nn = 0;

  if (!B[k*ps+j*rs+i]) return 0;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

  return (nn == 1);
} /* FaceLibre3d() */

/* ==================================== */
int32_t PaireLibre3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
// Détermine si la face (i,j,k) est libre dans le complexe b, c'est-a-dire si 
// elle est strictement incluse dans exactement une face de b.
// Si non, retourne -1.
// Si oui, retourne la face contenant (i,j,k).
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, uu, n, nn = 0;

  if (!B[k*ps+j*rs+i]) return -1;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) { nn++; uu = u; }
  if (nn != 1) return -1;
  return tab[uu];
} /* PaireLibre3d() */

/* ==================================== */
int32_t Collapse3d(struct xvimage *b, index_t i, index_t j, index_t k)
/* ==================================== */
#undef F_NAME
#define F_NAME "Collapse3d"
// Checks whether the face g = (i,j,k) is a free face for the complex b.
// If it is not, return -1.
// If it is, it forms a free pair with a face f which contains it. 
// These two faces are removed from b, and the face f is returned.
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t ps = rs * cs;
  index_t ds = depth(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, uu, n, nn = 0;

  if (!B[k*ps+j*rs+i]) return -1;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) { nn++; uu = u; }
  if (nn != 1) return -1;
  B[tab[uu]] = B[k*ps+j*rs+i] = VAL_NULLE;
  return tab[uu];
} /* Collapse3d() */

/* ==================================== */
int32_t FaceLibre3d_1(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
// Détermine si la face (i,j,k) est libre dans le complexe B, c'est-a-dire si 
// elle est strictement incluse dans exactement une face de B.
{
  index_t ps = rs * cs;
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, n, nn = 0;

  if (!B[k*ps+j*rs+i]) return 0;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

  return (nn == 1);
} /* FaceLibre3d_1() */

/* ==================================== */
int32_t Collapse3d_1(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k)
/* ==================================== */
#undef F_NAME
#define F_NAME "Collapse3d_1"
{
  index_t ps = rs * cs;
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, uu, n, nn = 0;

  if (!B[k*ps+j*rs+i]) return -1;
  Betacarre3d(rs, cs, ds, i, j, k, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) { nn++; uu = u; }
  if (nn != 1) return -1;
  B[tab[uu]] = B[k*ps+j*rs+i] = VAL_NULLE;
  return tab[uu];
} /* Collapse3d_1() */

#define MASK_OBJ  0x01
#define MASK_MRK  0x02

#define NN 343
static uint8_t BB[NN];

/* ==================================== */
int32_t simple_26_att(   /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,             /* index du point */
  index_t rs,            /* taille rangee */
  index_t ps,            /* taille plan */
  index_t N)             /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "simple_26_att"
#define RS 7
#define CS 7
#define DS 7
#define PS 49
{
  index_t i, j, k, ii, jj, kk, iu, ju, ku;
  index_t tab[GRS3D*GCS3D*GDS3D];
  int32_t u, n, eu, x, xx, na;
  int32_t lifo[GRS3D*GCS3D*GDS3D];
  int32_t lp;

  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;

  // calcule l'attachement du voxel p - resultat dans BB
  memset(BB, 0, NN);
  Alphacarre3d(RS, CS, DS, RS/2, CS/2, DS/2, tab, &n);
  for (u = 0; u < n; u++) BB[tab[u]] = MASK_MRK;

  ii = p % rs; jj = (p % ps) / rs; kk = p / ps;
  for (k = 0; k < 3; k++)
    for (j = 0; j < 3; j++)
      for (i = 0; i < 3; i++)
	if (img[(kk+k-1) * ps + (jj+j-1) * rs + ii+i-1])
	  if ((k != 1) || (j != 1) || (i != 1))
	  {
	    Alphacarre3d(RS, CS, DS, 2*i+1, 2*j+1, 2*k+1, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (BB[tab[u]] == MASK_MRK)
		BB[tab[u]] = MASK_OBJ;
	  }  

  na = 0; // pour taille attachement
  Alphacarre3d(RS, CS, DS, RS/2, CS/2, DS/2, tab, &n);
  for (u = 0; u < n; u++) 
    if (BB[tab[u]] == MASK_MRK) BB[tab[u]] = 0; 
    else if (BB[tab[u]] == MASK_OBJ) na++;

  // collapse de l'attachement
  lp = 0; // pointeur de lifo
  for (u = 0; u < n; u++) // init collapse
  {
    eu = tab[u];
    if (BB[eu])
    {
      i = eu % RS; j = (eu % PS) / RS; k = eu / PS;
      if (FaceLibre3d_1(BB, RS, CS, DS, i, j, k))
      {
	lifo[lp++] = eu;
	BB[eu] |= MASK_MRK;
      }
    }
  } // for u
  while (lp != 0) // collapse glouton
  {
    x = lifo[--lp];
    BB[x] &= ~MASK_MRK;
    i = x % RS; j = (x % PS) / RS; k = x / PS;
    xx = Collapse3d_1(BB, RS, CS, DS, i, j, k);
    if (xx != -1)
    {
      na -= 2;
      ii = xx % RS; jj = (xx % PS) / RS; kk = xx / PS;
      Alphacarre3d(RS, CS, DS, ii, jj, kk, tab, &n);
      for (u = 0; u < n; u += 1)
      {
        eu = tab[u];
	iu = eu % RS; ju = (eu % PS) / RS; ku = eu / PS;
	if (BB[eu] && !(BB[eu]&MASK_MRK) && FaceLibre3d_1(BB, RS, CS, DS, iu, ju, ku))
	{
	  lifo[lp++] = eu;
	  BB[eu] |= MASK_MRK;
	}
      }
    }
  } // while lp

  if (na == 1) return 1;
  return 0;

} /* simple_26_att() */

#undef NN
#define NN 441
static uint8_t B[NN];
static index_t tab[NN];

/* ==================================== */
int32_t pairesimple_26_x(
  uint8_t *img,          /* pointeur base image */
  index_t p,             /* index du point (l'autre element de la paire est son voisin x+1) */
  index_t rs,            /* taille rangee */
  index_t ps,            /* taille plan */
  index_t N)             /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "pairesimple_26_x"
#undef RS
#undef CS
#undef DS
#undef PS
#define RS 9
#define CS 7
#define DS 7
#define PS 63
{
  index_t i, j, k, ii, jj, kk, iu, ju, ku, eu, x, xx;
  int32_t u, n, na;
  int32_t lifo[GRS3D*GCS3D*5];
  int32_t lp;

  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs >= rs-2))     /* premiere ou (avant-)derniere ligne */
    return 0;

  // calcule l'attachement de la paire p/px+1 - resultat dans B

  memset(B, 0, NN);
  // marque les attachements des 2 voxels
  Alphacarre3d(RS, CS, DS, 3, 3, 3, tab, &n);
  for (u = 0; u < n; u++) B[tab[u]] = MASK_MRK;
  Alphacarre3d(RS, CS, DS, 5, 3, 3, tab, &n);
  for (u = 0; u < n; u++) B[tab[u]] = MASK_MRK;
  B[3*PS + 3*RS + 4] = 0;

  ii = p % rs; jj = (p % ps) / rs; kk = p / ps;
  for (k = 0; k < 3; k++)
    for (j = 0; j < 3; j++)
      for (i = 0; i < 4; i++)
	if (img[(kk+k-1) * ps + (jj+j-1) * rs + ii+i-1])
	  if ((k != 1) || (j != 1) || ((i != 1) && (i != 2)))
	  {
	    Alphacarre3d(RS, CS, DS, 2*i+1, 2*j+1, 2*k+1, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (B[tab[u]] == MASK_MRK)
		B[tab[u]] = MASK_OBJ;
	  }  

  Alphacarre3d(RS, CS, DS, 3, 3, 3, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]] == MASK_MRK) B[tab[u]] = 0; 
  Alphacarre3d(RS, CS, DS, 5, 3, 3, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]] == MASK_MRK) B[tab[u]] = 0; 
  na = 0; // pour taille attachement
  for (i = 0; i < NN; i++) if (B[i]) tab[na++] = i;

  // collapse de l'attachement
  lp = 0; // pointeur de lifo
  for (u = 0; u < na; u++) // init collapse
  {
    eu = tab[u];
    if (B[eu])
    {
      i = eu % RS; j = (eu % PS) / RS; k = eu / PS;
      if (FaceLibre3d_1(B, RS, CS, DS, i, j, k))
      {
	lifo[lp++] = eu;
	B[eu] |= MASK_MRK;
      }
    }
  } // for u
  while (lp != 0) // collapse glouton
  {
    x = lifo[--lp];
    B[x] &= ~MASK_MRK;
    i = x % RS; j = (x % PS) / RS; k = x / PS;
    xx = Collapse3d_1(B, RS, CS, DS, i, j, k);
    if (xx != -1)
    {
      na -= 2;
      ii = xx % RS; jj = (xx % PS) / RS; kk = xx / PS;
      Alphacarre3d(RS, CS, DS, ii, jj, kk, tab, &n);
      for (u = 0; u < n; u += 1)
      {
        eu = tab[u];
	iu = eu % RS; ju = (eu % PS) / RS; ku = eu / PS;
	if (B[eu] && !(B[eu]&MASK_MRK) && FaceLibre3d_1(B, RS, CS, DS, iu, ju, ku))
	{
	  lifo[lp++] = eu;
	  B[eu] |= MASK_MRK;
	}
      }
    }
  } // while lp

  if (na == 1) return 1;
  return 0;

} /* pairesimple_26_x() */

/* ==================================== */
int32_t pairesimple_26_y(
  uint8_t *img,          /* pointeur base image */
  index_t p,             /* index du point (l'autre element de la paire est son voisin y+1) */
  index_t rs,            /* taille rangee */
  index_t ps,            /* taille plan */
  index_t N)             /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "pairesimple_26_y"
#undef RS
#undef CS
#define RS 7
#define CS 9
{
  index_t i, j, k, ii, jj, kk, iu, ju, ku, eu, x, xx;
  int32_t u, n, na;
  int32_t lifo[GRS3D*GCS3D*5];
  int32_t lp;

  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-(2*rs)) ||  /* premiere ou (avant-)derniere colonne */
      (p%rs == 0) || (p%rs >= rs-1))     /* premiere ou derniere ligne */
    return 0;

  // calcule l'attachement de la paire p/py+1 - resultat dans B

  memset(B, 0, NN);
  // marque les attachements des 2 voxels
  Alphacarre3d(RS, CS, DS, 3, 3, 3, tab, &n);
  for (u = 0; u < n; u++) B[tab[u]] = MASK_MRK;
  Alphacarre3d(RS, CS, DS, 3, 5, 3, tab, &n);
  for (u = 0; u < n; u++) B[tab[u]] = MASK_MRK;
  B[3*PS + 4*RS + 3] = 0;

  ii = p % rs; jj = (p % ps) / rs; kk = p / ps;
  for (k = 0; k < 3; k++)
    for (j = 0; j < 4; j++)
      for (i = 0; i < 3; i++)
	if (img[(kk+k-1) * ps + (jj+j-1) * rs + ii+i-1])
	  if ((k != 1) || (i != 1) || ((j != 1) && (j != 2)))
	  {
	    Alphacarre3d(RS, CS, DS, 2*i+1, 2*j+1, 2*k+1, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (B[tab[u]] == MASK_MRK)
		B[tab[u]] = MASK_OBJ;
	  }  

  Alphacarre3d(RS, CS, DS, 3, 3, 3, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]] == MASK_MRK) B[tab[u]] = 0; 
  Alphacarre3d(RS, CS, DS, 3, 5, 3, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]] == MASK_MRK) B[tab[u]] = 0; 
  na = 0; // pour taille attachement
  for (i = 0; i < NN; i++) if (B[i]) tab[na++] = i;

  // collapse de l'attachement
  lp = 0; // pointeur de lifo
  for (u = 0; u < n; u++) // init collapse
  {
    eu = tab[u];
    if (B[eu])
    {
      i = eu % RS; j = (eu % PS) / RS; k = eu / PS;
      if (FaceLibre3d_1(B, RS, CS, DS, i, j, k))
      {
	lifo[lp++] = eu;
	B[eu] |= MASK_MRK;
      }
    }
  } // for u
  while (lp != 0) // collapse glouton
  {
    x = lifo[--lp];
    B[x] &= ~MASK_MRK;
    i = x % RS; j = (x % PS) / RS; k = x / PS;
    xx = Collapse3d_1(B, RS, CS, DS, i, j, k);
    if (xx != -1)
    {
      na -= 2;
      ii = xx % RS; jj = (xx % PS) / RS; kk = xx / PS;
      Alphacarre3d(RS, CS, DS, ii, jj, kk, tab, &n);
      for (u = 0; u < n; u += 1)
      {
        eu = tab[u];
	iu = eu % RS; ju = (eu % PS) / RS; ku = eu / PS;
	if (B[eu] && !(B[eu]&MASK_MRK) && FaceLibre3d_1(B, RS, CS, DS, iu, ju, ku))
	{
	  lifo[lp++] = eu;
	  B[eu] |= MASK_MRK;
	}
      }
    }
  } // while lp

  if (na == 1) return 1;
  return 0;

} /* pairesimple_26_y() */

/* ==================================== */
int32_t pairesimple_26_z(
  uint8_t *img,          /* pointeur base image */
  index_t p,             /* index du point (l'autre element de la paire est son voisin z+1) */
  index_t rs,            /* taille rangee */
  index_t ps,            /* taille plan */
  index_t N)             /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "pairesimple_26_z"
#undef CS
#undef DS
#undef PS
#define CS 7
#define DS 9
#define PS 49
{
  index_t i, j, k, ii, jj, kk, iu, ju, ku, eu, x, xx;
  int32_t u, n, na;
  int32_t lifo[GRS3D*GCS3D*5];
  int32_t lp;

  if ((p < ps) || (p >= N-(2*ps)) ||     /* premier ou (avant-)dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs >= rs-1))     /* premiere ou derniere ligne */
    return 0;

  // calcule l'attachement de la paire p/pz+1 - resultat dans B

  memset(B, 0, NN);
  // marque les attachements des 2 voxels
  Alphacarre3d(RS, CS, DS, 3, 3, 3, tab, &n);
  for (u = 0; u < n; u++) B[tab[u]] = MASK_MRK;
  Alphacarre3d(RS, CS, DS, 3, 3, 5, tab, &n);
  for (u = 0; u < n; u++) B[tab[u]] = MASK_MRK;
  B[4*PS + 3*RS + 3] = 0;

  ii = p % rs; jj = (p % ps) / rs; kk = p / ps;
  for (k = 0; k < 4; k++)
    for (j = 0; j < 3; j++)
      for (i = 0; i < 3; i++)
	if (img[(kk+k-1) * ps + (jj+j-1) * rs + ii+i-1])
	  if ((i != 1) || (j != 1) || ((k != 1) && (k != 2)))
	  {
	    Alphacarre3d(RS, CS, DS, 2*i+1, 2*j+1, 2*k+1, tab, &n);
	    for (u = 0; u < n; u++) 
	      if (B[tab[u]] == MASK_MRK)
		B[tab[u]] = MASK_OBJ;
	  }  

  Alphacarre3d(RS, CS, DS, 3, 3, 3, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]] == MASK_MRK) B[tab[u]] = 0; 
  Alphacarre3d(RS, CS, DS, 3, 3, 5, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]] == MASK_MRK) B[tab[u]] = 0; 
  na = 0; // pour taille attachement
  for (i = 0; i < NN; i++) if (B[i]) tab[na++] = i;

  // collapse de l'attachement
  lp = 0; // pointeur de lifo
  for (u = 0; u < n; u++) // init collapse
  {
    eu = tab[u];
    if (B[eu])
    {
      i = eu % RS; j = (eu % PS) / RS; k = eu / PS;
      if (FaceLibre3d_1(B, RS, CS, DS, i, j, k))
      {
	lifo[lp++] = eu;
	B[eu] |= MASK_MRK;
      }
    }
  } // for u
  while (lp != 0) // collapse glouton
  {
    x = lifo[--lp];
    B[x] &= ~MASK_MRK;
    i = x % RS; j = (x % PS) / RS; k = x / PS;
    xx = Collapse3d_1(B, RS, CS, DS, i, j, k);
    if (xx != -1)
    {
      na -= 2;
      ii = xx % RS; jj = (xx % PS) / RS; kk = xx / PS;
      Alphacarre3d(RS, CS, DS, ii, jj, kk, tab, &n);
      for (u = 0; u < n; u += 1)
      {
        eu = tab[u];
	iu = eu % RS; ju = (eu % PS) / RS; ku = eu / PS;
	if (B[eu] && !(B[eu]&MASK_MRK) && FaceLibre3d_1(B, RS, CS, DS, iu, ju, ku))
	{
	  lifo[lp++] = eu;
	  B[eu] |= MASK_MRK;
	}
      }
    }
  } // while lp

  if (na == 1) return 1;
  return 0;

} /* pairesimple_26_z() */
