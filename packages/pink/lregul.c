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
   Operateur de regulation haute ou basse
   d'apres: "Topological Approach to Image Segmentation" 
            G. Bertrand, J.C. Everat, M. Couprie 

   entrees: K, un leveling kernel au sens des minima;
            mode, un entier egal a 0 pour la regularisation basse, 
                  a 1 pour la haute, et a 2 pour les deux.

   regul haute : 
     K[x] - K[R] < (K[x] - K[Rp]) * alpha 
       devient:
     mu < 0           avec
     mu = K[x] - K[R] - (K[x] - K[Rp]) * alpha

   regul basse : 
     K[x] - K[R] < (PSI[R] - K[R]) * alpha
       devient:
     nu < 0           avec
     nu = K[x] - K[R] - (PSI[R] - K[R]) * alpha

   pour la version hierarchique, on encode avec l'index du point la direction du minimum
   voisin concerne : 
     x       ->        minimum = voisin EST
     x + N   ->        minimum = voisin NORD
     x + 2N  ->        minimum = voisin OUEST
     x + 3N  ->        minimum = voisin SUD
*/

/* Michel Couprie - aout 1997 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mclifo.h>
#include <mcindic.h>
#include <mccodimage.h>
#include <lregul.h>
#include <mcfah.h>

#define MINIMUM      0
#define UPPER        1
#define TRAITE       2
#define CANDIDAT     3
/*
#define DEBUG
#define TRACE
*/

/* ==================================== */
int32_t lregul(struct xvimage * kernel, int32_t mode, double alpha, int32_t *nbreg)
/* ==================================== */
{
  int32_t k, kk;
  uint32_t x, y, w, yy;
  uint32_t nbmin, min1, min2;
  uint8_t *K;
  uint8_t *PSI;
  int32_t rs, cs, d, N;
  Lifo * LIFO;
  uint8_t psi;
  int32_t flag;
  uint8_t altmin;
  Fah * FAH;
  uint32_t nbupper;
  int32_t mu, nu, numax;
  int32_t dir1, dir2;

  if (depth(kernel) != 1) 
  {
    fprintf(stderr, "lregul: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  rs = rowsize(kernel);
  cs = colsize(kernel);
  N = rs * cs;
  K = UCHARDATA(kernel);  

  PSI = (uint8_t *)calloc(1,N * sizeof(char));
  if (PSI == NULL) 
  {
    fprintf(stderr, "lregul: malloc failed for PSI\n");
    exit(0);
  }

  IndicsInit(N);

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "lregul() : CreeLifoVide failed\n");
      return(0);
  }

  *nbreg = 0;

  /* ---------------------------------------------------------- */
  /* etiquette les 4-minima avec le label MINIMUM recueille le PSI associe */
  /* ---------------------------------------------------------- */

  for (x = 0; x < N; x++)
  {
    if (!IsSet(x, TRAITE))
    {
      psi = NDG_MIN;
      flag = MINIMUM;

      Set(x, CANDIDAT);           /* 1er parcours */
      LifoPush(LIFO, x);
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        for (k = 0; k < 8; k += 2)
        {
          y = voisin(w, k, rs, N);
          if (y != -1)
          {
            if (K[y] < K[w]) /* w non dans un minimum */
              flag = UPPER;
            else
            if (K[y] > K[w])
	    {
              if (K[y] > psi) 
                psi = K[y];
	    }
            else /* K[y] == K[w] */ 
            if (!IsSet(y, CANDIDAT))
            {
              Set(y, CANDIDAT);
              LifoPush(LIFO, y);
            } /* if K ... */
          } /* if (y != -1) */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */

      Set(x, TRAITE);           /* 2nd parcours */
      Set(x, flag);
      PSI[x] = psi;
      LifoPush(LIFO, x);
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        for (k = 0; k < 8; k += 2)
        {
          y = voisin(w, k, rs, N);
          if (y != -1)
          {
            if ((K[y] == K[w]) && (!IsSet(y, TRAITE)))
            {
              Set(y, TRAITE);
              Set(y, flag);
              PSI[y] = psi;
              LifoPush(LIFO, y);
            } /* if K ... */
          } /* if (y != -1) */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */

    } /* if (LABEL[x] == 0) */
  } /* for (x = 0; x < N; x++) */

#ifdef DEBUG
  for (x = 0; x < N; x++)
  {
    if (x % rs == 0) printf("\n");
    if (IsSet(x, MINIMUM)) printf("%3d ", PSI[x]); else printf("%3d ", 0);
  }
  printf("\n");
#endif

  nbupper = 0;
  for (x = 0; x < N; x++) if (IsSet(x, UPPER)) nbupper++;
  FAH = CreeFahVide(nbupper + 1);
  if (FAH == NULL)
  {   fprintf(stderr, "lregul() : CreeFahVide failed\n");
      return(0);
  }

  /* ---------------------------------------------------------- */
  /* regularisation basse */
  /* ---------------------------------------------------------- */

  if ((mode == 0) || (mode == 2))
  {
    uint8_t valminmin;
    uint32_t minmin;
    int32_t irreg;

    FahPush(FAH, -1, 0);
    for (x = 0; x < N; x++)          
    {
      if (IsSet(x, UPPER))
      {
        FahPush(FAH, x, K[x]);
      } /* if (IsSet(x, UPPER)) */
    } /* for (x = 0; x < N; x++) */
    (void)FahPop(FAH);

    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
#ifdef TRACE
printf("FahPop(x=%d,%d; K[x]=%d)\n", x%rs, x/rs, K[x]);
#endif
      psi = NDG_MIN;
      irreg = 0;
      valminmin = NDG_MAX;
      for (k = 0; k < 8; k += 2)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (IsSet(y, MINIMUM)))
        {
          if (K[y] < valminmin) { minmin = y; valminmin = K[y]; }
          if (PSI[y] > psi) { psi = PSI[y]; }
          nu = (int32_t)(K[x] - K[y] - (PSI[y] - K[y]) * alpha);
          if (nu < 0) irreg = 1;
        } /* if ((y != -1) && (IsSet(y, MINIMUM))) */
      } /* for k ... */

      /* si le point x est irregulier : 
         1/ on l'abaisse au niveau du plus petit minimum du 4-voisinage ; 
         2/ on abaisse tous les minima du 4-voisinage au niveau du minimum le plus bas ;
         3/ on met les PSI des minima du 4-voisinage a la valeur du PSI le plus haut.
      */
      if (irreg)
      {
        *nbreg += 1;        
        for (k = 0; k < 8; k += 2)    /* 1er parcours des minima du voisinage */
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (IsSet(y, MINIMUM)))
	  {
            altmin = K[y];
            UnSet(y, MINIMUM);
            LifoPush(LIFO, y);
            while (! LifoVide(LIFO))
            {
              w = LifoPop(LIFO);
              K[w] = valminmin;
              PSI[w] = psi;
              for (kk = 0; kk < 8; kk += 2)
              {
                yy = voisin(w, kk, rs, N);
                if ((yy != -1) && (K[yy] == altmin) && (IsSet(yy, MINIMUM)))
                { 
                  UnSet(yy, MINIMUM);
                  LifoPush(LIFO, yy);
                } /* if (y != -1) */
              } /* for k ... */
            } /* while (! LifoVide(LIFO)) */
	  } /* ((y != -1) && (IsSet(y, MINIMUM)) && (K[y] > K[x])) */
        } /* for k ... */
 
        K[x] = valminmin;
        PSI[x] = psi;
        LifoPush(LIFO, x);
        Set(x, MINIMUM);
        while (! LifoVide(LIFO)) /* 2nd parcours pour remarquer MINIMUM */
        {
          w = LifoPop(LIFO);
          for (kk = 0; kk < 8; kk += 2)
          {
            yy = voisin(w, kk, rs, N);
            if ((yy != -1) && (K[yy] == valminmin) && (!IsSet(yy, MINIMUM)))
            { 
              LifoPush(LIFO, yy);
              Set(yy, MINIMUM);
            } /* if (y != -1) */
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
      } /* if (irreg) */
    } /* while (!FahVide(FAH)) */
  } /* if ((mode == 0) || (mode == 2)) */

  /* ---------------------------------------------------------- */
  /* regularisation haute */
  /* ---------------------------------------------------------- */

  if ((mode == 1) || (mode == 2))
  {
    FahPush(FAH, -1, 0);
    for (x = 0; x < N; x++)          
    {
      if (IsSet(x, UPPER))
      {
        /* on ne s'interesse qu'aux upper-points separant deux minima 
           d'altitudes differentes */
        nbmin = 0;
        for (k = 0; k < 8; k += 2)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (IsSet(y, MINIMUM)))
	  {
            if (nbmin == 0) 
            {
              nbmin++;
              min1 = y;
              dir1 = k;
	    }
            else
            if (K[y] != K[min1])
	    {
              nbmin++;
              min2 = y;
              dir2 = k;
              break;              
	    }
	  }
        } /* for k ... */        
        if (nbmin == 2)
	{
          mu = (int32_t)(K[x] - K[min1] - (K[x] - K[min2]) * alpha);
          if (mu < 0)
	  {
            FahPush(FAH, x + (dir1 / 2) * N, K[x]);
#ifdef TRACE
printf("FahPush(x=%d,%d; dir=%d; mu=%d; K[x]=%d; K[R]=%d; K[Rp]=%d)\n", 
                x%rs, x/rs, dir1, mu, K[x], K[min1], K[min2]);
#endif
	  }
          mu = (int32_t)(K[x] - K[min2] - (K[x] - K[min1]) * alpha);
          if (mu < 0)
	  {
            FahPush(FAH, x + (dir2 / 2) * N, K[x]);
#ifdef TRACE
printf("FahPush(x=%d,%d; dir=%d; mu=%d; K[x]=%d; K[R]=%d; K[Rp]=%d)\n", 
                x%rs, x/rs, dir2, mu, K[x], K[min2], K[min1]);
#endif
	  }
	} /* if (nbmin == 2) */
      } /* if (IsSet(x, UPPER)) */
    } /* for (x = 0; x < N; x++) */
    (void)FahPop(FAH);

    while (!FahVide(FAH))
    {
#ifdef TRACE
psi = FahNiveau(FAH); 
#endif
      x = FahPop(FAH);
      if (x < N)            dir1 = 0;
      else if (x < 2 * N) { dir1 = 2; x -= N; }
      else if (x < 3 * N) { dir1 = 4; x -= 2 * N; }
      else                { dir1 = 6; x -= 3 * N; }
#ifdef TRACE
printf("FahPop(x=%d,%d; dir=%d, lv=%d)\n", x%rs, x/rs, dir1, psi);
#endif
      
      min1 = voisin(x, dir1, rs, N);
      if (IsSet(min1, MINIMUM))
      {                                /* regularise le minimum min1 */
#ifdef TRACE
printf("Regularise %d,%d\n", min1%rs, min1/rs);
#endif
        *nbreg += 1;
        altmin = K[min1];
        K[min1] = K[x];
        UnSetAll(min1);  /* reset (notamment) les flags MINIMUM et TRAITE */
        LifoPush(LIFO, min1);
        while (! LifoVide(LIFO))
        {
          w = LifoPop(LIFO);
          for (k = 0; k < 8; k += 2)
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (K[y] == altmin) && (IsSet(y, TRAITE)))
            { 
              K[y] = K[x];
              UnSetAll(y);  /* reset (notamment) les flags MINIMUM et TRAITE */
              LifoPush(LIFO, y);
            } /* if (y != -1) */
          } /* for k ... */
        } /* while (! LifoVide(LIFO)) */
      } /* if (IsSet(y, MINIMUM)) */
    } /* while (!FahVide(FAH)) */
  } /* if ((mode != 1) || (mode == 2)) */

  LifoTermine(LIFO);
  return 1;
}
