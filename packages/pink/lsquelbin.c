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
/* operateur de calcul du squelette 2D binaire */
/* simule 2 fifos avec une */
/* Michel Couprie -  mars 1998 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfifo.h>
#include <mctopo.h>
#include <mcindic.h>
#include <lsquelbin.h>

#define EN_FIFO       0
#define PARANO
/*
#define DEBUG
*/

int32_t nord(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
{
  return (img[voisin(p, NORD, rs, N)] == 0);
} // nord()

int32_t ouest(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
{
  return (img[voisin(p, OUEST, rs, N)] == 0);
} // ouest()

int32_t sud(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
{
  return (img[voisin(p, SUD, rs, N)] == 0);
} // sud()

int32_t est(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
{
  return (img[voisin(p, EST, rs, N)] == 0);
} // est()

/* ==================================== */
int32_t lsquelbin(struct xvimage *image, int32_t connex, int32_t niseuil)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fifo * FIFO;
  int32_t t, tb;
  int32_t iter;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lsquelbin: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  IndicsInit(N);

  FIFO = CreeFifoVide(N);

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION FIFO: empile tous les points bord de l'objet non bord d'image */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && nord(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && sud(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && est(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && ouest(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  if (connex == 8)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && nord(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && sud(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && est(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x] != 0) && nonbord(x,rs,N) && nbvoisc4(F, x, rs, N) && ouest(F, x, rs, N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  {
    fprintf(stderr, "lsquelbin: bad connexity: %d\n", connex);
    return 0;
  }
  FifoPush(FIFO, -1); // marqueur de fin d'iteration 

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  iter = 0;
  if (niseuil == -1) niseuil = 2000000000; // very big positive integer
  if (connex == 4)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      } 
      else
      {
        UnSet(x, EN_FIFO);
        top4(F, x, rs, N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois4(F, x, rs, N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && (F[y]) && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F, x, rs, N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  }
  else
  if (connex == 8)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      }
      else
      {
        UnSet(x, EN_FIFO);
        top8(F, x, rs, N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois8(F, x, rs, N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && (F[y]) && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F, x, rs, N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FifoTermine(FIFO);
  return(1);
} // lsquelbin()

/* ==================================== */
int32_t lhthinbin(struct xvimage *image,
               int32_t connex, int32_t niseuil, int32_t niter)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fifo * FIFO;
  int32_t t, tb;
  int32_t iter;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lhthinbin: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  IndicsInit(N);

  FIFO = CreeFifoVide(N);

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION FIFO: empile tous les points bord de l'objet non bord d'image */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  if (connex == 8)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  {
    fprintf(stderr, "lhthinbin: bad connexity: %d\n", connex);
    return 0;
  }
  FifoPush(FIFO, -1); // marqueur de fin d'iteration 

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  iter = 0;
  if (niseuil == -1) niseuil = 2000000000; // very big positive integer
  if (niter == -1) niter = 2000000000; // very big positive integer
  if (connex == 4)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      } 
      else
      {
        UnSet(x, EN_FIFO);
        top4(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois4(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 4)
  else
  if (connex == 8)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      }
      else
      {
        UnSet(x, EN_FIFO);
        top8(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois8(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 8)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */
 menage:
  IndicsTermine();
  FifoTermine(FIFO);

  return(1);
} // lhthinbin()

/* ==================================== */
int32_t lhthinbincontr(struct xvimage *image, struct xvimage *contr, 
               int32_t connex, int32_t niseuil, int32_t niter)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *C = UCHARDATA(contr);      /* l'image de contrainte */
  Fifo * FIFO;
  int32_t t, tb;
  int32_t iter;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lhthinbin: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  IndicsInit(N);

  FIFO = CreeFifoVide(N);

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION FIFO: empile tous les points bord de l'objet non bord d'image */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  if (connex == 8)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]==0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  {
    fprintf(stderr, "lhthinbin: bad connexity: %d\n", connex);
    return 0;
  }
  FifoPush(FIFO, -1); // marqueur de fin d'iteration 

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  iter = 0;
  if (niseuil == -1) niseuil = 2000000000; // very big positive integer
  if (niter == -1) niter = 2000000000; // very big positive integer
  if (connex == 4)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      } 
      else
      {
        UnSet(x, EN_FIFO);
        top4(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois4(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && (C[y]==0) && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 4)
  else
  if (connex == 8)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      }
      else
      {
        UnSet(x, EN_FIFO);
        top8(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois8(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && (C[y]==0) && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 8)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */
 menage:
  IndicsTermine();
  FifoTermine(FIFO);
  return(1);
} // lhthinbincontr()

/* ==================================== */
int32_t lhthickbin(struct xvimage *image,
               int32_t connex, int32_t niseuil, int32_t niter)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fifo * FIFO;
  int32_t t, tb;
  int32_t iter;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lhthickbin: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  IndicsInit(N);

  FIFO = CreeFifoVide(N);

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  // inverse l'image
  for (x = 0; x < N; x++) F[x] = NDG_MAX - F[x];

  /* ========================================================= */
  /*   INITIALISATION FIFO: empile tous les points bord de l'objet non bord d'image */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  if (connex == 8)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  {
    fprintf(stderr, "lhthickbin: bad connexity: %d\n", connex);
    return 0;
  }
  FifoPush(FIFO, -1); // marqueur de fin d'iteration 

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  iter = 0;
  if (niseuil == -1) niseuil = 2000000000; // very big positive integer
  if (niter == -1) niter = 2000000000; // very big positive integer
  if (connex == 4)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      } 
      else
      {
        UnSet(x, EN_FIFO);
        top4(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois4(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 4)
  else
  if (connex == 8)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      }
      else
      {
        UnSet(x, EN_FIFO);
        top8(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois8(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 8)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */
 menage:
  // inverse l'image
  for (x = 0; x < N; x++) F[x] = NDG_MAX - F[x];
  IndicsTermine();
  FifoTermine(FIFO);
  return(1);
} // lhthickbin()

/* ==================================== */
int32_t lhthickbincontr(struct xvimage *image, struct xvimage *contr, 
               int32_t connex, int32_t niseuil, int32_t niter)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *C = UCHARDATA(contr);      /* l'image de contrainte */
  Fifo * FIFO;
  int32_t t, tb;
  int32_t iter;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lhthickbin: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  IndicsInit(N);

  FIFO = CreeFifoVide(N);

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  // inverse l'image
  for (x = 0; x < N; x++) F[x] = NDG_MAX - F[x];

  /* ========================================================= */
  /*   INITIALISATION FIFO: empile tous les points bord de l'objet non bord d'image */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  if (connex == 8)
  {
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && nord(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && sud(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && est(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
    for (x = 0; x < N; x++)
    {
      if ((F[x]!=0)&&(C[x]!=0) && nonbord(x,rs,N) && nbvoisc4(F,x,rs,N) && ouest(F,x,rs,N))
      {
        FifoPush(FIFO, x); 
        Set(x, EN_FIFO); 
      }
    } /* for x */
  }
  else
  {
    fprintf(stderr, "lhthickbin: bad connexity: %d\n", connex);
    return 0;
  }
  FifoPush(FIFO, -1); // marqueur de fin d'iteration 

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  iter = 0;
  if (niseuil == -1) niseuil = 2000000000; // very big positive integer
  if (niter == -1) niter = 2000000000; // very big positive integer
  if (connex == 4)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      } 
      else
      {
        UnSet(x, EN_FIFO);
        top4(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois4(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && C[y] && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 4)
  else
  if (connex == 8)
  {
    while (! (FifoVide(FIFO)))
    {
      x = FifoPop(FIFO);
      if (x == -1)
      {
        iter ++;
        if (iter >= niter) goto menage;
        if (FifoVide(FIFO)) break;
        FifoPush(FIFO, -1);
      }
      else
      {
        UnSet(x, EN_FIFO);
        top8(F,x,rs,N, &t, &tb); 
        if ((t == 1) && (tb == 1) && ((iter < niseuil) || (nbvois8(F,x,rs,N) != 1)))
        {
          F[x] = 0;
          for (k = 0; k < 8; k += 1)  /* parcourt les 8 voisins */
          {
            y = voisin(x, k, rs, N);
            if ((y != -1) && F[y] && C[y] && (!IsSet(y, EN_FIFO)) && nonbord(y,rs,N))
            {
              FifoPush(FIFO, y); 
              Set(y, EN_FIFO); 
            }
          } /* for k */
        } /* if ((t == 1) && (tb == 1) && !extremite8(F,x,rs,N)) */
      } /* else */
    } /* while (! (FifoVide(FIFO))) */
  } // if (connex == 8)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */
 menage:
  // inverse l'image
  for (x = 0; x < N; x++) F[x] = NDG_MAX - F[x];
  IndicsTermine();
  FifoTermine(FIFO);
  return(1);
} // lhthickbincontr()

