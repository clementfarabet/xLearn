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
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mclifo.h>
/* #define PERF */
#ifdef PERF
#include <mcchrono.h>
#endif
#include <mcutil.h>
#include <mcindic.h>
#include <mcprobas.h>
#include <math.h>
#include <lpropgeo.h>
/*
#define DEBUG
*/
/****************************************************************
*
* Routine Name: lpropgeo - library call for propgeo
*
* Purpose: propagation geodesique dans des composantes 4 ou 8 connexes
*
* Input: img1, image en ndg
*        mask, image en ndg (plateaux - la valeur 0 est pour l'arriere-plan)
*              ou image de labels (int32_t)
*        connex, 4 ou 8
*        function:
*          MIN1: retourne un point correspondant a un minimum par composante
*          MIN:  propage le min
*          MAX1: retourne un point correspondant a un maximum par composante
*          MAX:  propage le max
*          MOY1: retourne un point par composante dont la valeur est proche de la moyenne
*          MOY:  propage la moyenne des ndg ponderee par la surface
*          MINB: propage le min du bord exterieur de la composante
*          MAXB: propage le max du bord exterieur de la composante
*          MOYB: propage la moyenne du bord exterieur de la composante
*          RANDB: remplit la composante de valeurs aleatoires, selon une loi normale
*                 de moyenne MOYB
* Output: img1
* Written By: Michel Couprie -- juin 1997
*          update avril 1998: MINB, MAXB, MOYB
*          update juillet 98: debug
*            les points frontieres sont des points qui ont un label = 0 dans l'image masque
*          update juillet 98: valeur taboue (pour les points frontiere)
*          update avril 99: RANDB
*
****************************************************************/

#define TRAITE1   0
#define TRAITE2   1
#define MINIMUM   2
#define MAXIMUM   3
#define MOYEN     4

#define VALEURTABOUE 0

/* ==================================== */
int32_t lpropgeo(
        struct xvimage *img1,
        struct xvimage *mask, 
        int32_t connex, 
        int32_t function
)
/* ==================================== */
#undef F_NAME
#define F_NAME "lpropgeo"
{
  int32_t k;
  int32_t w;
  int32_t x;
  int32_t y;
  uint8_t min;
  uint8_t max;
  uint8_t moy;
  uint8_t minb;
  uint8_t maxb;
  uint8_t moyb;
  uint8_t valcomp;
  int32_t valcomplong;
  int32_t mintrouve;
  int32_t maxtrouve;
  int32_t moytrouve;
  int32_t incr_vois;
  int32_t incr_voisb;
  int32_t sumndg;        /* pour calculer la moyenne sur la composante */
  int32_t surf;          /* pour calculer la surface de la composante */
  int32_t sumndgb;       /* pour calculer la moyenne sur le bord */
  int32_t sumndgc;       /* pour calculer la variance sur le bord */
  int32_t surfb;         /* pour calculer la surface du bord */
  double moybd;      /* moyenne sur le bord */
  double devb;       /* deviation (ecart-type) sur le bord */
  int32_t tmp;
  uint8_t *SOURCE = UCHARDATA(img1);
  uint8_t *MASK = UCHARDATA(mask);
  int32_t *MASKLONG = SLONGDATA(mask);
  int32_t rs = rowsize(img1);
  int32_t cs = colsize(img1);
  int32_t N = rs * cs;
  Lifo * LIFO;
#ifdef PERF
  chrono chrono1;
#endif

  if (depth(img1) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (img1->data_storage_type != VFF_TYP_1_BYTE)
    return lpropgeolong(img1, mask, connex, function);

  if ((rowsize(mask) != rs) || (colsize(mask) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  switch (connex)
  {
    case 4: incr_vois = 2; incr_voisb = 1; break;
    case 8: incr_vois = 1; incr_voisb = 2; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

#ifdef PERF
  /* pour l'analyse de performances */
  start_chrono(&chrono1);
#endif

  IndicsInit(N);

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  if (mask->data_storage_type == VFF_TYP_1_BYTE)
  {
    for (x = 0; x < N; x++)       /* recherche dans toute l'image */
    {
      if (!MASK[x]) SOURCE[x] = NDG_MIN;
      if (MASK[x] && (!IsSet(x, TRAITE1)))   /* on est dans une CC non traitee */
      {
        valcomp = MASK[x];
        LifoPush(LIFO, x);
        Set(x, TRAITE1);
        min = SOURCE[x];
        max = SOURCE[x];
        minb = NDG_MAX;
        maxb = NDG_MIN;
        sumndg = 0;
        surf = 0;
        sumndgb = 0;
        sumndgc = 0;
        surfb = 0;

        while (! LifoVide(LIFO))  /* parcourt la CC */
        {
          w = LifoPop(LIFO);
          if ((SOURCE[w] < min)) min = SOURCE[w];
          if ((SOURCE[w] > max)) max = SOURCE[w];
          surf += 1;
          sumndg += SOURCE[w];
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASK[y] == valcomp) && (!IsSet(y, TRAITE1)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE1);
            } 
          } /* for k ... */
          for (k = 0; k < 8; k += incr_voisb) /* parcourt les voisins dans le complementaire */
          {
            y = voisin(w, k, rs, N);
#ifdef VALEURTABOUE
            if ((y != -1) && (MASK[y] == 0) && (!IsSet(y, TRAITE1)) && (SOURCE[y] != VALEURTABOUE))
#else
            if ((y != -1) && (MASK[y] == 0) && (!IsSet(y, TRAITE1)))
#endif
            {
              if ((SOURCE[y] < minb)) minb = SOURCE[y];
              if ((SOURCE[y] > maxb)) maxb = SOURCE[y];
              surfb += 1;
              sumndgb += SOURCE[y];
              sumndgc += SOURCE[y] * SOURCE[y];
              Set(y, TRAITE1);
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
        moy = (uint8_t)(sumndg/surf);
        if (surfb > 0) { moyb = (uint8_t)(sumndgb/surfb); moybd = ((double)sumndgb)/surfb; }
        else           { moyb = 0; moybd = 0.0; }
        devb = sqrt(((double)sumndgc)/surfb - ((double)sumndgb * sumndgb)/(surfb * surfb));

#ifdef DEBUG
printf("composante %d - surf=%d ; sumndg=%d ; surfb=%d ; sumndgb=%d ; max=%d\n", 
        valcomplong, surf, sumndg, surfb, sumndgb, max);
#endif

        LifoPush(LIFO, x);
        Set(x, TRAITE2);
        mintrouve = 0;
        maxtrouve = 0;
        moytrouve = 0;
        while (! LifoVide(LIFO))  /* RE-parcourt la CC */
        {
          w = LifoPop(LIFO);
          switch (function)
          {
            case MIN1:
              if ((SOURCE[w] == min) && !mintrouve) { Set(w, MINIMUM); mintrouve = 1; }
              break;
            case MAX1:
              if ((SOURCE[w] == max) && !maxtrouve) { Set(w, MAXIMUM); maxtrouve = 1; }
              break;
            case MOY1:
              if ((SOURCE[w] == moy) && !moytrouve) { Set(w, MOYEN); moytrouve = 1; }
              break;
            case MIN:
              SOURCE[w] = min;
              break;
            case MAX:
              SOURCE[w] = max;
              break;
            case MOY:
              SOURCE[w] = moy;
              break;
            case MINB:
              SOURCE[w] = minb;
              break;
            case MAXB:
              SOURCE[w] = maxb;
              break;
            case MOYB:
              SOURCE[w] = moyb;
              break;
            case RANDB:
              tmp = (int32_t)Normal(moybd, devb);
              if (tmp > (int32_t)NDG_MAX) tmp = (int32_t)NDG_MAX;
              if (tmp < (int32_t)NDG_MIN) tmp = (int32_t)NDG_MIN;
              SOURCE[w] = (uint8_t)tmp;
              break;
	    default:
              fprintf(stderr, "%s: bad argument\n", F_NAME);
              return(0);
          }
          for (k = 0; k < 8; k += incr_voisb) /* parcourt les voisins dans le complementaire */
          {                                   /* pour les demarquer (TRAITE1) */
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASK[y] == 0) && (IsSet(y, TRAITE1)))
              UnSet(y, TRAITE1);
          } /* for k ... */
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASK[y] == valcomp) && (!IsSet(y, TRAITE2)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE2);
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
  
      } /* if (MASK[x] && (!IsSet(x, TRAITE1))) */

    } /* for (x = 0; x < N; x++) */
  } /* if (mask->data_storage_type == VFF_TYP_1_BYTE) */
  else if (mask->data_storage_type == VFF_TYP_4_BYTE)
  {
    for (x = 0; x < N; x++)       /* recherche dans toute l'image */
    {
      if (!MASKLONG[x]) SOURCE[x] = NDG_MIN;
      if (MASKLONG[x] && (!IsSet(x, TRAITE1)))   /* on est dans une CC non traitee */
      {
        valcomplong = MASKLONG[x];
        LifoPush(LIFO, x);
        Set(x, TRAITE1);
#ifdef DEBUG
printf("Set TRAITE1 %d (%d,%d)\n",x, x%rs, x/rs);
#endif
        min = SOURCE[x];
        max = SOURCE[x];
        sumndg = 0;
        surf = 0;
        sumndgb = 0;
        sumndgc = 0;
        surfb = 0;
  
        while (! LifoVide(LIFO))  /* parcourt la CC */
        {
          w = LifoPop(LIFO);
          if ((SOURCE[w] < min)) min = SOURCE[w];
          if ((SOURCE[w] > max)) max = SOURCE[w];
          surf += 1;
          sumndg += SOURCE[w];
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASKLONG[y] == valcomplong) && (!IsSet(y, TRAITE1)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE1);
#ifdef DEBUG
printf("Set TRAITE1 %d (%d,%d)\n",y, y%rs, y/rs);
#endif
            } 
          } /* for k ... */
          for (k = 0; k < 8; k += incr_voisb) /* parcourt les voisins dans le complementaire */
          {
            y = voisin(w, k, rs, N);
#ifdef VALEURTABOUE
            if ((y != -1) && (MASKLONG[y] == 0) && (!IsSet(y, TRAITE1)) && (SOURCE[y] != VALEURTABOUE))
#else
            if ((y != -1) && (MASKLONG[y] == 0) && (!IsSet(y, TRAITE1)))
#endif
            {
              if ((SOURCE[y] < minb)) minb = SOURCE[y];
              if ((SOURCE[y] > maxb)) maxb = SOURCE[y];
              surfb += 1;
              sumndgb += SOURCE[y];
              sumndgc += SOURCE[y] * SOURCE[y];
              Set(y, TRAITE1);
#ifdef DEBUG
printf("Set TRAITE1 %d (%d,%d)\n",y, y%rs, y/rs);
#endif
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
        moy = (uint8_t)(sumndg/surf);
        if (surfb > 0) { moyb = (uint8_t)(sumndgb/surfb); moybd = ((double)sumndgb)/surfb; }
        else           { moyb = 0; moybd = 0.0; }
        devb = sqrt(((double)sumndgc)/surfb - ((double)sumndgb * sumndgb)/(surfb * surfb));

#ifdef DEBUG
printf("composante %d - surf=%d ; sumndg=%d ; surfb=%d ; sumndgb=%d ; max=%d\n", 
        valcomplong, surf, sumndg, surfb, sumndgb, max);
#endif
  
        LifoPush(LIFO, x);
        Set(x, TRAITE2);
        mintrouve = 0;
        maxtrouve = 0;
        moytrouve = 0;
        while (! LifoVide(LIFO))  /* RE-parcourt la CC */
        {
          w = LifoPop(LIFO);
          switch (function)
          {
            case MIN1:
              if ((SOURCE[w] == min) && !mintrouve) { Set(w, MINIMUM); mintrouve = 1; }
              break;
            case MAX1:
              if ((SOURCE[w] == max) && !maxtrouve) { Set(w, MAXIMUM); maxtrouve = 1; }
              break;
            case MOY1:
              if ((SOURCE[w] == moy) && !moytrouve) { Set(w, MOYEN); moytrouve = 1; }
              break;
            case MIN:
              SOURCE[w] = min;
              break;
            case MAX:
              SOURCE[w] = max;
              break;
            case MOY:
              SOURCE[w] = moy;
              break;
            case MINB:
              SOURCE[w] = minb;
              break;
            case MAXB:
              SOURCE[w] = maxb;
              break;
            case MOYB:
              SOURCE[w] = moyb;
              break;
            case RANDB:
              tmp = (int32_t)Normal(moybd, devb);
              if (tmp > (int32_t)NDG_MAX) tmp = (int32_t)NDG_MAX;
              if (tmp < (int32_t)NDG_MIN) tmp = (int32_t)NDG_MIN;
              SOURCE[w] = (uint8_t)tmp;
              break;
	    default:
              fprintf(stderr, "%s: bad argument\n", F_NAME);
              return(0);
          }
          for (k = 0; k < 8; k += incr_voisb) /* parcourt les voisins dans le complementaire */
          {                                   /* pour les demarquer (TRAITE1) */
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASKLONG[y] == 0) && (IsSet(y, TRAITE1)))
	    {
              UnSet(y, TRAITE1);
#ifdef DEBUG
printf("UnSet TRAITE1 %d (%d,%d)\n",y, y%rs, y/rs);
#endif
	    }
          } /* for k ... */
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASKLONG[y] == valcomplong) && (!IsSet(y, TRAITE2)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE2);
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
  
      } /* if (MASKLONG[x] && (!IsSet(x, TRAITE1))) */

    } /* for (x = 0; x < N; x++) */
  } /* if (mask->data_storage_type == VFF_TYP_4_BYTE) */
  else
  {   fprintf(stderr, "%s: bad storage type\n", F_NAME);
      return(0);
  }

  if (function == MIN1)
    for (x = 0; x < N; x++)
      if (IsSet(x, MINIMUM))
        SOURCE[x] = NDG_MAX;
      else
        SOURCE[x] = NDG_MIN;

  if (function == MAX1)
    for (x = 0; x < N; x++)
      if (IsSet(x, MAXIMUM))
        SOURCE[x] = NDG_MAX;
      else
        SOURCE[x] = NDG_MIN;

  if (function == MOY1)
    for (x = 0; x < N; x++)
      if (IsSet(x, MOYEN))
        SOURCE[x] = NDG_MAX;
      else
        SOURCE[x] = NDG_MIN;

  LifoTermine(LIFO);
  IndicsTermine();
  return(1);
} /* lpropgeo() */

/* ==================================== */
int32_t lpropgeolong(
        struct xvimage *img1,
        struct xvimage *mask, 
        int32_t connex, 
        int32_t function
)
/* ==================================== */
#undef F_NAME
#define F_NAME "lpropgeolong"
{
  int32_t k;
  int32_t w;
  int32_t x;
  int32_t y;
  int32_t min;
  int32_t max;
  int32_t moy;
  uint8_t valcomp;
  int32_t valcomplong;
  int32_t mintrouve;
  int32_t maxtrouve;
  int32_t moytrouve;
  int32_t incr_vois;
  int32_t incr_voisb;
  int32_t sumndg;        /* pour calculer la moyenne sur la composante */
  int32_t surf;          /* pour calculer la surface de la composante */
  int32_t *SOURCE = SLONGDATA(img1);
  uint8_t *MASK = UCHARDATA(mask);
  int32_t *MASKLONG = SLONGDATA(mask);
  int32_t rs = rowsize(img1);
  int32_t cs = colsize(img1);
  int32_t N = rs * cs;
  Lifo * LIFO;
#ifdef PERF
  chrono chrono1;
#endif

  if (depth(img1) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (img1->data_storage_type != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: datatype for arg1 must be int32_t\n", F_NAME);
    return 0;
  }

  if ((rowsize(mask) != rs) || (colsize(mask) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  switch (connex)
  {
    case 4: incr_vois = 2; incr_voisb = 1; break;
    case 8: incr_vois = 1; incr_voisb = 2; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

#ifdef PERF
  /* pour l'analyse de performances */
  start_chrono(&chrono1);
#endif

  IndicsInit(N);

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  if (mask->data_storage_type == VFF_TYP_1_BYTE)
  {
    for (x = 0; x < N; x++)       /* recherche dans toute l'image */
    {
      if (!MASK[x]) SOURCE[x] = 0;
      if (MASK[x] && (!IsSet(x, TRAITE1)))   /* on est dans une CC non traitee */
      {
        valcomp = MASK[x];
        LifoPush(LIFO, x);
        Set(x, TRAITE1);
        min = SOURCE[x];
        max = SOURCE[x];
        surf = 0;

        while (! LifoVide(LIFO))  /* parcourt la CC */
        {
          w = LifoPop(LIFO);
          if ((SOURCE[w] < min)) min = SOURCE[w];
          if ((SOURCE[w] > max)) max = SOURCE[w];
          surf += 1;
          sumndg += SOURCE[w];
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASK[y] == valcomp) && (!IsSet(y, TRAITE1)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE1);
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
        moy = (int32_t)(sumndg/surf);

        LifoPush(LIFO, x);
        Set(x, TRAITE2);
        mintrouve = 0;
        maxtrouve = 0;
        moytrouve = 0;
        while (! LifoVide(LIFO))  /* RE-parcourt la CC */
        {
          w = LifoPop(LIFO);
          switch (function)
          {
            case MIN:
              SOURCE[w] = min;
              break;
            case MAX:
              SOURCE[w] = max;
              break;
            case MOY:
              SOURCE[w] = moy;
              break;
	    default:
              fprintf(stderr, "%s: bad argument\n", F_NAME);
              return(0);
          }
          for (k = 0; k < 8; k += incr_voisb) /* parcourt les voisins dans le complementaire */
          {                                   /* pour les demarquer (TRAITE1) */
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASK[y] == 0) && (IsSet(y, TRAITE1)))
              UnSet(y, TRAITE1);
          } /* for k ... */
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASK[y] == valcomp) && (!IsSet(y, TRAITE2)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE2);
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
  
      } /* if (MASK[x] && (!IsSet(x, TRAITE1))) */

    } /* for (x = 0; x < N; x++) */
  } /* if (mask->data_storage_type == VFF_TYP_1_BYTE) */
  else if (mask->data_storage_type == VFF_TYP_4_BYTE)
  {
    for (x = 0; x < N; x++)       /* recherche dans toute l'image */
    {
      if (!MASKLONG[x]) SOURCE[x] = NDG_MIN;
      if (MASKLONG[x] && (!IsSet(x, TRAITE1)))   /* on est dans une CC non traitee */
      {
        valcomplong = MASKLONG[x];
        LifoPush(LIFO, x);
        Set(x, TRAITE1);
#ifdef DEBUG
printf("Set TRAITE1 %d (%d,%d)\n",x, x%rs, x/rs);
#endif
        min = SOURCE[x];
        max = SOURCE[x];
        sumndg = 0;
        surf = 0;
  
        while (! LifoVide(LIFO))  /* parcourt la CC */
        {
          w = LifoPop(LIFO);
          if ((SOURCE[w] < min)) min = SOURCE[w];
          if ((SOURCE[w] > max)) max = SOURCE[w];
          surf += 1;
          sumndg += SOURCE[w];
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASKLONG[y] == valcomplong) && (!IsSet(y, TRAITE1)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE1);
#ifdef DEBUG
printf("Set TRAITE1 %d (%d,%d)\n",y, y%rs, y/rs);
#endif
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
        moy = (int32_t)(sumndg/surf);
  
        LifoPush(LIFO, x);
        Set(x, TRAITE2);
        mintrouve = 0;
        maxtrouve = 0;
        moytrouve = 0;
        while (! LifoVide(LIFO))  /* RE-parcourt la CC */
        {
          w = LifoPop(LIFO);
          switch (function)
          {
            case MIN:
              SOURCE[w] = min;
              break;
            case MAX:
              SOURCE[w] = max;
              break;
            case MOY:
              SOURCE[w] = moy;
              break;
	    default:
              fprintf(stderr, "%s: bad argument\n", F_NAME);
              return(0);
          }
          for (k = 0; k < 8; k += incr_voisb) /* parcourt les voisins dans le complementaire */
          {                                   /* pour les demarquer (TRAITE1) */
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASKLONG[y] == 0) && (IsSet(y, TRAITE1)))
	    {
              UnSet(y, TRAITE1);
#ifdef DEBUG
printf("UnSet TRAITE1 %d (%d,%d)\n",y, y%rs, y/rs);
#endif
	    }
          } /* for k ... */
          for (k = 0; k < 8; k += incr_vois) /* parcourt les voisins */
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (MASKLONG[y] == valcomplong) && (!IsSet(y, TRAITE2)))
            {
              LifoPush(LIFO, y);
              Set(y, TRAITE2);
            } 
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
  
      } /* if (MASKLONG[x] && (!IsSet(x, TRAITE1))) */

    } /* for (x = 0; x < N; x++) */
  } /* if (mask->data_storage_type == VFF_TYP_4_BYTE) */
  else
  {   fprintf(stderr, "%s: bad storage type\n", F_NAME);
      return(0);
  }

  LifoTermine(LIFO);
  IndicsTermine();
  return(1);
} /* lpropgeolong() */
