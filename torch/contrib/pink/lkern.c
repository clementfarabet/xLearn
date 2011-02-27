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
/* operateur de calcul du noyau homotopique par abaissement 
   pour la 1/2 topologie "moins" 

   on suppose que le cadre de l'image de depart est a 0, et que les autres
   points de l'image sont tous > 0. la valeur 0 represente -infini.

*/
/* Michel Couprie -  octobre 1998 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfahpure.h>
#include <mctopo.h>
#include <mcindic.h>
#include <llabelextrema.h>
#include <lkern.h>

#define EN_FAHP       0
#define EN_FAHP2      1
#define PARANO
/*
#define DEBUG
#define VERIF
*/

#define VERBOSE

/* ==================================== */
int32_t essentiel(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t t4mm, t4m, t8p, t8pp;

  nbtopo(F, x, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  return (t8p != 1) && (t8p != 2);
} /* essentiel() */

/* ==================================== */
void saturation(int32_t rs, int32_t cs, int32_t N, uint8_t *F, Fahp * FAHP)
/* ==================================== */
{
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t t4mm, t4m, t8p, t8pp;
  int32_t level;
#ifdef DEBUG
  uint8_t oldFx;
#endif
  while (!FahpVide(FAHP))
  {
    level = FahpNiveau(FAHP);
    x = FahpPop(FAHP);
    UnSet(x, EN_FAHP);
    UnSet(x, EN_FAHP2);
    if ((F[x] > 0) && (level <= NDG_MAX))
    {
      nbtopoh(F, x, 1, rs, N, &t4mm, &t4m, &t8p, &t8pp);
      if (t4mm == 1)
      {
        if (pdestr4(F, x, rs, N))
        {
#ifdef DEBUG
oldFx = F[x];
#endif
          if (!ridge(F, x, rs, N)) F[x] = 0; else F[x] = alpha8m(F, x, rs, N);
#ifdef DEBUG
printf("point %d (%d %d), val = %d --> %d\n", x, x%rs, x/rs, oldFx, F[x]);
#endif
          for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
          {                                       /* pour empiler les voisins */
            y = voisin(x, k, rs, N);             /* non deja empiles */
            if ((y != -1) && (F[y] > 0) && (! IsSet(y, EN_FAHP)))
            {
              FahpPush(FAHP, y, F[y]);
              Set(y, EN_FAHP);
            } /* if y */
          } /* for k */
        } /* if (pdestr4(F, x, rs, N)) */
      } /* if (t4mm == 1) */
      else                               /* t4mm != 1 */
      {
        if (pdestr4(F, x, rs, N))        /* point divergent */
        {
          F[x] = alpha8m(F, x, rs, N);
          for (k = 0; k < 8; k += 1)   /* parcourt les voisins en 8-connexite */
          {                                       /* pour empiler les voisins */
            y = voisin(x, k, rs, N);                      /* non deja empiles */
            if ((y != -1) && (F[y] > 0) && (! IsSet(y, EN_FAHP)))
            {
              FahpPush(FAHP, y, F[y]);
              Set(y, EN_FAHP);
            } /* if y */
          } /* for k */
        } /* if (pdestr4(F, x, rs, N)) */
      } /* else if (t4mm == 1) */
    } /* ((F[x] > 0) && (F[x] <= NDG_MAX)) */
  } /* while (! (FifoVide(FAHP) ...)) */
} /* saturation() */

/* ==================================== */
int32_t lkern(struct xvimage *image, int32_t connex)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahp * FAHP;
  int32_t t4mm, t4m, t8p, t8pp;
#ifdef DEBUG
  uint8_t oldFx;
#endif

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lkern: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  IndicsInit(N);

  FAHP = CreeFahpVide(N);
  if (FAHP == NULL)
  {   fprintf(stderr, "lkern() : CreeFahpVide failed\n");
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* empile tous les voisins des points du bord */

  for (x = rs + 1, y = (cs-2) * rs + 1; x < 2*rs-1; x++, y++)
  {
    FahpPush(FAHP, x, F[x]); Set(x, EN_FAHP);
    FahpPush(FAHP, y, F[y]); Set(y, EN_FAHP);
  }

  for (x = 2*rs+1 , y = 3*rs-2; x < (cs-2)*rs+1; y += rs, x += rs)
  {
    FahpPush(FAHP, x, F[x]); Set(x, EN_FAHP);
    FahpPush(FAHP, y, F[y]); Set(y, EN_FAHP);
  }

  saturation(rs, cs, N, F, FAHP);

  /* met a 0 les puits et les interieurs de plateaux (T-- = 0) */
  /* et empile les voisins */

  for (x = 0; x < N; x++)
  if ((x%rs != rs-1) && (x >= rs) && (x%rs != 0) && (x < N-rs) && /* non point de bord */
      (F[x] != 0))
  {
    nbtopo(F, x, rs, N, &t4mm, &t4m, &t8p, &t8pp);
    if (t4mm == 0)
    {
      F[x] = 0;
      for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
      {                                       /* pour empiler les voisins */
        y = voisin(x, k, rs, N);             /* non deja empiles */
        if ((y != -1) && (F[y] > 0) && (! IsSet(y, EN_FAHP)))
        {
          FahpPush(FAHP, y, F[y]);
          Set(y, EN_FAHP);
        } /* if y */
      } /* for k */
    } /* if (t4mm == 0) */
    saturation(rs, cs, N, F, FAHP);
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahpTermine(FAHP);
  return(1);
}
