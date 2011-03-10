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
   Operateurs de calcul du noyau homotopique par abaissement et elevation,
   d'amincissement et d'epaississement homotopique.

   Operateurs de calcul du noyau de nivellement par abaissement et elevation,
   d'amincissement et d'epaississement de nivellement.

   Operateurs de restauration de cretes.

   Michel Couprie -  janvier 1998
 
   Update mai 1999: conditionnement par une image en niveaux de gris G
                    un point destructible x ne peut etre abaisse a une valeur < G[x]
                    si un point (destructible ou pas) est tel que F[x] < G[x] a l'origine,
                    alors il conserve sa valeur

   Udate juin 1999: noyau avec une seul fah
                    amincicement, epaissisement:
                      V1: avec 2 lifos en partant des extrema 
                      V2: avec 2 lifos, memorisation du delta- (valeur min a laquelle 
                          un point destructible peut etre abaisse)
                    restauration de cretes

   Update janvier 2000: options COL2 et EXTENSIBLE_TOPO pour la restauration de cretes

   Update juillet 2000: amincissement avec reduction d'anisotropie

   Update aout 2000: reconstruction preservant la dynamique 

   Update avril 2003: extension de CrestRestoration (ajout de imcond)
*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef HP
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfah.h>
#include <mclifo.h>
#include <mctopo.h>
#include <mcindic.h>
#include <mcutil.h>
#include <lhtkern.h>
#include <llabelextrema.h>

/*
#define ANIMATE
*/

/*#define COL2*/                     /* pour la restauration de cretes */
#define COLMULTI                 /* pour la restauration de cretes */
#define EXTENSIBLE_TOPO          /* pour la restauration de cretes */
#define EXTENSIBLE_GEO           /* pour la restauration de cretes */
#define EXTENSIBLE_MARK          /* pour la restauration de cretes */
#define COND_TRUE 1              /* pour la restauration de cretes */
#define EXTENSIBLE 2

#define EN_FAH       0
#define EN_LIFO      0

#define PARANO

/*
#define DEBUG
#define DEBUGISO
#define VERBOSE
*/

/* ==================================== */
int32_t testabaisse4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testabaisse4 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (pdestr4(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8m(F,x,rs,N); /* alpha8m : sic */
  }

#ifdef DEBUG
if (modifie) printf("========> ABAISSE : %d\n", F[x]);
#endif

  return modifie;
} /* testabaisse4() */

/* ==================================== */
int32_t testabaisse8(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testabaisse8 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (pdestr8(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8m(F,x,rs,N); 
  }

#ifdef DEBUG
if (modifie) printf("========> ABAISSE : %d\n", F[x]);
#endif

  return modifie;
} /* testabaisse8() */

/* ==================================== */
int32_t testeleve4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testeleve4 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (pconstr4(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8p(F,x,rs,N); /* alpha8m : sic */
  }

#ifdef DEBUG
if (modifie) printf("========> ELEVE : %d\n", F[x]);
#endif

  return modifie;
} /* testeleve4() */

/* ==================================== */
int32_t testeleve8(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testeleve8 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (pconstr8(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8p(F,x,rs,N); 
  }

#ifdef DEBUG
if (modifie) printf("========> ELEVE : %d\n", F[x]);
#endif

  return modifie;
} /* testeleve8() */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/*
          NOYAUX HOMOTOPIQUES
 */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/* ==================================== */
int32_t lhtkern(struct xvimage *image, struct xvimage *imagecond, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhtkern"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFahVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /* INITIALISATION DE LA FAH: empile tous les points destructibles */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (pdestr4(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pdestr4(F,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (pdestr8(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pdestr8(F,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connex == 4)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testabaisse4(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (pdestr4(F,y,rs,N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testabaisse4(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testabaisse8(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (pdestr8(F,y,rs,N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testabaisse8(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 8) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* lhtkern() */

/* ==================================== */
int32_t lhtkernu(struct xvimage *image, struct xvimage *imagecond, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhtkernu"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFahVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points destructibles */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((F[x] < G[x]) && (pconstr4(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pconstr4(F,x,rs,N))  
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((F[x] < G[x]) && (pconstr8(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pconstr8(F,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
  }


  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connex == 4)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testeleve4(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (F[y] < G[y])) && (pconstr4(F,y,rs,N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testeleve4(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testeleve8(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (F[y] < G[y])) && (pconstr8(F,y,rs,N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testeleve8(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 8) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* lhtkernu() */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/*
          AMINCISSEMENT - EPAISSISSEMENT
 */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/*
   principe de l'encodage du alpha-: 
   la valeur alpha- de x est placee dans les bits 24 a 31 de l'int32_t contenant x
                                                      24
   il reste donc 24 bits pour coder les pixels, soit 2   = 16 megapixels.
   (ex: image 4k x 4k)

   une valeur -1 pour nitermax signifie de laisser calculer jusqu'a stabilite (en fait
   2000000000 iterations) 
*/

#define ENCODE(y,a) (y|(a<<24))
#define DECODEX(y) (y&0x00ffffff)
#define DECODEA(y) ((y>>24)&0xff)

/* ==================================== */
int32_t lhthinalpha(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhthinalpha"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

#ifdef VERBOSE
  fprintf(stderr, "%s: nitermax = %d\n", F_NAME, nitermax);
#endif

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points destructibles avec leur alpha-  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (pdestr4(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pdestr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (pdestr8(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pdestr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pdestr4(F, x, rs, N))
	{
          F[x] = mcmax(alpha8m(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pdestr4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (pdestr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (pdestr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pdestr8(F, x, rs, N))
	{
          F[x] = mcmax(alpha8m(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pdestr8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (pdestr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (pdestr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lhthinalpha() */

/* ==================================== */
int32_t lhthindelta(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhthindelta"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

#ifdef VERBOSE
  fprintf(stderr, "%s: nitermax = %d\n", F_NAME, nitermax);
#endif

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points destructibles avec leur delta-  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (pdestr4(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,delta4m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pdestr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta4m(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (pdestr8(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,delta8m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pdestr8(F,x,rs,N)) 
        {
#ifdef DEBUG
          printf("init : push %d,%d, delta = %d\n", x%rs, x/rs, delta8m(F,x,rs,N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta8m(F,x,rs,N)));
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pdestr4(F, x, rs, N))
	{
          F[x] = mcmax(delta4m(F, x, rs, N),a);
#ifdef DEBUG
          printf("Abaisse x a %d\n", F[x]);
#endif
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pdestr4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (pdestr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta4m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta4m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (pdestr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta4m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta4m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pdestr8(F, x, rs, N))
	{
          F[x] = mcmax(delta8m(F, x, rs, N),a);
#ifdef DEBUG
          printf("Abaisse x a %d\n", F[x]);
#endif
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pdestr8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (pdestr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta8m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta8m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (pdestr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta8m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta8m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lhthindelta() */

/* ==================================== */
int32_t lhthickalpha(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhthickalpha"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points constructibles avec leur alpha+  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (pconstr4(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pconstr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (pconstr8(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pconstr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr4(F, x, rs, N))
	{
          F[x] = mcmin(alpha8p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (pconstr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (pconstr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr8(F, x, rs, N))
	{
          F[x] = mcmin(alpha8p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (pconstr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (pconstr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lhthickalpha() */

/* ==================================== */
int32_t lhthickdelta(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhthickdelta"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points constructibles avec leur delta+  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (pconstr4(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,delta4p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pconstr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta4p(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (pconstr8(F,x,rs,N))) LifoPush(LIFO1, ENCODE(x,delta8p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (pconstr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta8p(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr4(F, x, rs, N))
	{
          F[x] = mcmin(delta4p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (pconstr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta4p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta4p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (pconstr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta4p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta4p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr8(F, x, rs, N))
	{
          F[x] = mcmin(delta8p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (pconstr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta8p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta8p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (pconstr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta8p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta8p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lhthickdelta() */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/*
          OPERATEURS DE NIVELLEMENT
 */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/* ==================================== */
int32_t testnivabaisse4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testnivabaisse4 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (peakordestr4(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8m(F,x,rs,N); /* alpha8m : sic */
  }

#ifdef DEBUG
if (modifie) printf("========> ABAISSE : %d\n", F[x]);
#endif

  return modifie;
} /* testnivabaisse4() */

/* ==================================== */
int32_t testnivabaisse8(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testnivabaisse8 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (peakordestr8(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8m(F,x,rs,N); 
  }

#ifdef DEBUG
if (modifie) printf("========> ABAISSE : %d\n", F[x]);
#endif

  return modifie;
} /* testnivabaisse8() */

/* ==================================== */
int32_t testniveleve4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testniveleve4 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (wellorconstr4(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8p(F,x,rs,N); /* alpha8m : sic */
  }

#ifdef DEBUG
if (modifie) printf("========> ELEVE : %d\n", F[x]);
#endif

  return modifie;
} /* testniveleve4() */

/* ==================================== */
int32_t testniveleve8(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

#ifdef DEBUG
printf("testniveleve8 : point %d (%d %d), val = %d\n", x, x%rs, x/rs, F[x]);
#endif

  while (wellorconstr8(F,x,rs,N)) 
  { 
    modifie = 1; 
    F[x] = alpha8p(F,x,rs,N); 
  }

#ifdef DEBUG
if (modifie) printf("========> ELEVE : %d\n", F[x]);
#endif

  return modifie;
} /* testniveleve8() */

/* ==================================== */
int32_t llvkern(struct xvimage *image, struct xvimage *imagecond, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llvkern"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFahVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points destructibles ou pics */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (peakordestr4(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (peakordestr4(F,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (peakordestr8(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (peakordestr8(F,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, F[x]);
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connex == 4)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testnivabaisse4(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (peakordestr4(F,y,rs,N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testnivabaisse4(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testnivabaisse8(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (peakordestr8(F,y,rs,N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testnivabaisse8(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 8) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* llvkern() */

/* ==================================== */
int32_t llvkernu(struct xvimage *image, struct xvimage *imagecond, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llvkernu"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFahVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points destructibles ou puits */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((F[x] < G[x]) && (wellorconstr4(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (wellorconstr4(F,x,rs,N))  
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((F[x] < G[x]) && (wellorconstr8(F,x,rs,N))) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (wellorconstr8(F,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, NDG_MAX-F[x]);
	}
    }
  }


  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connex == 4)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testniveleve4(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (F[y] < G[y])) && (wellorconstr4(F,y,rs,N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testniveleve4(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testniveleve8(F, x, rs, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (F[y] < G[y])) && (wellorconstr8(F,y,rs,N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testniveleve8(F, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 8) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* llvkernu() */

/* ==================================== */
int32_t llthin(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llthin"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points destructibles avec leur alpha-  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (peakordestr4(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (peakordestr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (peakordestr8(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (peakordestr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles et les pics */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (peakordestr4(F, x, rs, N))
	{
          F[x] = alpha8m(F, x, rs, N);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (peakordestr4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (peakordestr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (peakordestr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (peakordestr8(F, x, rs, N))
	{
          F[x] = alpha8m(F, x, rs, N);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (peak8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (peakordestr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (peakordestr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* llthin() */

/* ==================================== */
int32_t llthindelta(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llthindelta"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points destructibles avec leur delta-  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (peakordestr4(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,delta4m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (peakordestr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta4m(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (peakordestr8(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,delta8m(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (peakordestr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta8m(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles et les pics */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pdestr4(F, x, rs, N))
	{
          F[x] = mcmax(delta4m(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pdestr4(F, x, rs, N)) */
        else if (peak4(F, x, rs, N))
	{
          F[x] = alpha8m(F, x, rs, N);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (peak4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (peakordestr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta4m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta4m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (peakordestr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta4m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta4m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points destructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pdestr8(F, x, rs, N))
	{
          F[x] = mcmax(delta8m(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pdestr8(F, x, rs, N)) */
        else if (peak8(F, x, rs, N))
	{
          F[x] = alpha8m(F, x, rs, N);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (peak8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (peakordestr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta8m(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta8m(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (peakordestr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta8m(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta8m(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* llthindelta() */

/* ==================================== */
int32_t llthick(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llthick"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points nivelables avec leur alpha+  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (wellorconstr4(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (wellorconstr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (wellorconstr8(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (wellorconstr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points nivelables */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (wellorconstr4(F, x, rs, N))
	{
          F[x] = mcmin(alpha8p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (wellorconstr4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (wellorconstr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (wellorconstr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points nivelables */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (wellorconstr8(F, x, rs, N))
	{
          F[x] = mcmin(alpha8p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (wellorconstr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], alpha8p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,alpha8p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (wellorconstr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], alpha8p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,alpha8p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* llthick() */

/* ==================================== */
int32_t llthickdelta(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llthickdelta"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t incr_vois;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (connex == 4) incr_vois = 2; else incr_vois = 1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return(0);
    }
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points constructibles avec leur delta+  */
  /* ========================================================= */

  if (connex == 4)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (wellorconstr4(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,delta4p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (wellorconstr4(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta4p(F,x,rs,N)));
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (wellorconstr8(F,x,rs,N))) 
          LifoPush(LIFO1, ENCODE(x,delta8p(F,x,rs,N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (wellorconstr8(F,x,rs,N)) LifoPush(LIFO1, ENCODE(x,delta8p(F,x,rs,N)));
    }
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr4(F, x, rs, N))
	{
          F[x] = mcmin(delta4p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr4(F, x, rs, N)) */
        else if (well4(F, x, rs, N))
	{
          F[x] = alpha8p(F, x, rs, N);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (well4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (wellorconstr4(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta4p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta4p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (wellorconstr4(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta4p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta4p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr8(F, x, rs, N))
	{
          F[x] = mcmin(delta8p(F, x, rs, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (pconstr8(F, x, rs, N)) */
        else if (well8(F, x, rs, N))
	{
          F[x] = alpha8p(F, x, rs, N);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (well8(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (wellorconstr8(F,x,rs,N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, x/rs, F[x], delta8p(F, x, rs, N));
#endif
          LifoPush(LIFO1, ENCODE(x,delta8p(F,x,rs,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (wellorconstr8(F,y,rs,N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, y/rs, F[y], delta8p(F, y, rs, N));
#endif
            LifoPush(LIFO1, ENCODE(y,delta8p(F,y,rs,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* llthickdelta() */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/*
          RESTORATION DE CRETES
 */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/* ==================================== */
int32_t nbvoiss8(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* retourne le nb de 8-voisins de p de niveau superieur ou egal a h */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) >= val)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) >= val)) n++;
        if ((p>=rs) && (*(ptr-rs) >= val)) n++;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) >= val)) n++;
        if ((p%rs!=0) && (*(ptr-1) >= val)) n++;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) >= val)) n++;
        if ((p<N-rs) && (*(ptr+rs) >= val)) n++;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) >= val)) n++;
        return n;
} /* nbvoiss8() */

/* ==================================== */
int32_t nbvoisi8(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* retourne le nb de 8-voisins de p de niveau inferieur strictement a h */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) < val)) n++;
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) < val)) n++;
        if ((p>=rs) && (*(ptr-rs) < val)) n++;
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) < val)) n++;
        if ((p%rs!=0) && (*(ptr-1) < val)) n++;
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) < val)) n++;
        if ((p<N-rs) && (*(ptr+rs) < val)) n++;
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) < val)) n++;
        return n;
} /* nbvoisi8() */

/* ==================================== */
int32_t nbvoiss4(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* retourne le nb de 4-voisins de p niveau superieur ou egal a h */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) >= val)) n++;
        if ((p>=rs) && (*(ptr-rs) >= val)) n++;
        if ((p%rs!=0) && (*(ptr-1) >= val)) n++;
        if ((p<N-rs) && (*(ptr+rs) >= val)) n++;
        return n;
} /* nbvoiss4() */

/* ==================================== */
int32_t nbvoisi4(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* retourne le nb de 4-voisins de p niveau inferieur strictement a h */
/* ==================================== */
{
	register uint8_t val = h;
	register uint8_t * ptr = img+p;
	register int32_t n = 0;

        if ((p%rs!=rs-1) && (*(ptr+1) < val)) n++;
        if ((p>=rs) && (*(ptr-rs) < val)) n++;
        if ((p%rs!=0) && (*(ptr-1) < val)) n++;
        if ((p<N-rs) && (*(ptr+rs) < val)) n++;
        return n;
} /* nbvoisi4() */

/* ==================================== */
int32_t nbcomps8(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  uint8_t h,             /* hauteur de la coupe */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* ==================================== */
{
	int32_t t4mm, t4m, t8p, t8pp;

        if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
                return 0;

        nbtopoh(img, p, h, rs, N, &t4mm, &t4m, &t8p, &t8pp);
	return t8p;
} /* nbcomps8() */

/* ==================================== */
int32_t phi8m(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* retourne max{F(y), y dans gamma8*(x) / F(y) <= F(x)} si gamma8*(x) / F(y) <= F(x) non vide,
   F(x) sinon 
*/
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register uint8_t phi = NDG_MIN;
	register uint8_t * ptr = img+p;

        if ((p%rs!=rs-1) && (*(ptr+1) <= val) && (*(ptr+1) > phi)) phi = *(ptr+1);
        if (((p%rs!=rs-1)&&(p>=rs)) && (*(ptr+1-rs) <= val) && (*(ptr+1-rs) > phi)) phi = *(ptr+1-rs);
        if ((p>=rs) && (*(ptr-rs) <= val) && (*(ptr-rs) > phi)) phi = *(ptr-rs);
        if (((p>=rs)&&(p%rs!=0)) && (*(ptr-rs-1) <= val) && (*(ptr-rs-1) > phi)) phi = *(ptr-rs-1);
        if ((p%rs!=0) && (*(ptr-1) <= val) && (*(ptr-1) > phi)) phi = *(ptr-1);
        if (((p%rs!=0)&&(p<N-rs)) && (*(ptr-1+rs) <= val) && (*(ptr-1+rs) > phi)) phi = *(ptr-1+rs);
        if ((p<N-rs) && (*(ptr+rs) <= val) && (*(ptr+rs) > phi)) phi = *(ptr+rs);
        if (((p<N-rs)&&(p%rs!=rs-1)) && (*(ptr+rs+1) <= val) && (*(ptr+rs+1) > phi)) phi = *(ptr+rs+1);
        if (phi == NDG_MIN) phi = val;
        return phi;
} /* phi8m() */

/* ==================================== */
int32_t courbe4(  /* teste si un point est un point de courbe - minima 4-connexes
	         ie- si pour une coupe <= img[p] on a T = 2 
                 et deux voisins exactement dans la coupe */
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* ==================================== */
{	
  int32_t t4mm, t4m, t8p, t8pp;
  int32_t k, q;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  nbtopo(img, p, rs, N, &t4mm, &t4m, &t8p, &t8pp);
  if ((t8p == 2) && (nbvoiss8(img, p, img[p], rs, N) == 2)) return 1;
  for (k = 0; k < 8; k += 1)
  {
    q = voisin(p, k, rs, N);
    if ((q != -1) && (img[q] <= img[p]))
    {
      nbtopoh(img, p, img[q], rs, N, &t4mm, &t4m, &t8p, &t8pp);
      if ((t8p == 2) && (nbvoiss8(img, p, img[q], rs, N) == 2)) return 1;
    }
  }	
  return 0;
} /* courbe4() */

/* ==================================== */
int32_t courbe8(  /* teste si un point est un point de courbe - minima 8-connexes
	         ie- si pour une coupe <= img[p] on a T = 2 
                 et deux voisins exactement dans la coupe */
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* ==================================== */
{	
  int32_t t8mm, t8m, t4p, t4pp;
  int32_t k, q;

  if ((p%rs==rs-1)||(p<rs)||(p%rs==0)||(p>=N-rs)) /* point de bord */
     return 0;
  nbtopo2(img, p, rs, N, &t8mm, &t8m, &t4p, &t4pp);
  if ((t4p == 2) && (nbvoiss4(img, p, img[p], rs, N) == 2)) return 1;
  for (k = 0; k < 8; k += 1)
  {
    q = voisin(p, k, rs, N);
    if ((q != -1) && (img[q] <= img[p]))
    {
      nbtopoh2(img, p, img[q], rs, N, &t8mm, &t8m, &t4p, &t4pp);
      if ((t4p == 2) && (nbvoiss4(img, p, img[q], rs, N) == 2)) return 1;
    }
  }	
  return 0;
} /* courbe8() */

/* ==================================== */
int32_t extensible4(
  uint8_t *F,            /* pointeur base image */
  int32_t x,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* ==================================== */
/*
   Le point x est extensible au niveau nivext si : 
     i) x est separant
     ii) x possede dans son voisinage un point y verifiant :
         F[y] = nivext
         y separant pour le niveau F[x]
         y non separant pour tout niveau > F[x]
         y marque COND_TRUE

   La fonction retourne 0 si non extensible, nivext sinon.
*/
{
  int32_t k, y;
  int32_t nivext = 0;

  if (!separant4(F, x, rs, N)) return 0;

  for (k = 0; k < 8; k += 1)
  {
    y = voisin(x, k, rs, N);
    if ((y != -1) && (IsSet(y,COND_TRUE)))
    {
      if ((F[y] > F[x]) && 
          hseparant4(F,y,F[x]-1,rs,N) && !hseparant4(F,y,F[x],rs,N)) 
        if (F[y] > nivext) nivext = F[y]; 
    }
  }

  return nivext;
} /* extensible4() */

/* ==================================== */
int32_t colextensible4(
  uint8_t *F,            /* pointeur base image */
  int32_t x,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N)                       /* taille image */
/* ==================================== */
{
  int32_t k, y;
  int32_t nbplus = 0;

  for (k = 0; k < 8; k += 1)
  {
    y = voisin(x, k, rs, N);
    if (y != -1)
    {
#ifdef EXTENSIBLE_TOPO
      if ((F[y] > F[x]) && !hseparant4(F, y, F[x], rs, N)) nbplus++; 
#endif
#ifdef EXTENSIBLE_GEO
      if ((F[y] > F[x]) && (nbvoiss8(F, y, F[x]+1, rs, N) <= 1)) nbplus++;
#endif
#ifdef EXTENSIBLE_MARK
      if ((F[y] > F[x]) && IsSet(y,COND_TRUE)) nbplus++;
#endif
    }
  }

#ifdef COL2
  return (nbplus == 2);
#endif
#ifdef COLMULTI
  return (nbplus >= 2);
#endif
} /* colextensible4() */

/* ==================================== */
int32_t lcrestrestoration(struct xvimage *image, struct xvimage *imcond, int32_t nitermax, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lcrestrestoration"
{ 
  int32_t x;                       /* index muet de pixel */
  uint32_t y;              /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de condition */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t a;
  int32_t stable;  
#ifdef ANIMATE
#define NITER1STEP 4
  int32_t nstep;
  char animname[64];
#endif

  if (nitermax == -1) nitermax = 2000000000;   

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return(0);
  }

  IndicsInit(N);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {
    fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
    return(0);
  }

  if (imcond == NULL)
  {
    for (x = 0; x < N; x++) Set(x,COND_TRUE);
  }
  else
  {
    G = UCHARDATA(imcond);
    for (x = 0; x < N; x++) 
      if (G[x]) Set(x,COND_TRUE);
  } // if (imcond != NULL)

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */
#ifdef SATURE
do /* repetition de toute la procedure jusqu'a stabilite */
{  /* cette boucle globale est rendue necessaire par le fait qu'un */
   /* point extensible ne peut etre caracterise que dans un 124-voisinage */
  stable = 1;
#endif
  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points 
       extensibles avec leur delta+ */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++) 
      if ((a = extensible4(F,x,rs,N)))
      {
#ifdef DEBUG
        printf("INIT: Push x = %d,%d ; F[x] = %d ; a = %d\n", 
                x%rs, x/rs, F[x], a);
#endif
        LifoPush(LIFO1, ENCODE(x,a));
      }
  }
  else
  {
    printf("connex = %d NYI \n", connex);
    return 0;
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

#ifdef ANIMATE
  nstep = 0;
  strcpy(animname, "ANIM");
#endif

  if (connex == 4)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on eleve les points constructibles et les points selle */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d ; F[x] = %d ; a = %d\n", x%rs, x/rs, F[x], a);
#endif
        if (pconstr4(F, x, rs, N))
	{
          F[x] = mcmin(delta4p(F,x,rs,N),a);
          Set(x,COND_TRUE);
          stable = 0;
          LifoPush(LIFO2, x);
#ifdef DEBUG
          printf("========> ELEVE constr : %d\n", F[x]);
#endif
        } /* if (pconstr4(F, x, rs, N)) */
#ifdef COL2
        else if ((saddle4(F, x, rs, N) == 2) && colextensible4(F, x, rs, N))
#else
#ifdef COLMULTI
        else if ((saddle4(F, x, rs, N) >= 2) && colextensible4(F, x, rs, N))
#else
        else if (saddle4(F, x, rs, N))
#endif
#endif
	{
          F[x] = alpha8p(F, x, rs, N);
          Set(x,COND_TRUE);
          stable = 0;
          LifoPush(LIFO2, x);
#ifdef DEBUG
          printf("========> ELEVE col : %d\n", F[x]);
#endif
        } /* if (saddle4(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins extensibles   */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) && (a = extensible4(F, x, rs, N)))
        {
          LifoPush(LIFO1, ENCODE(x,a));
          Set(x,EN_LIFO);
#ifdef DEBUG
          printf("Push x = %d,%d ; F[x] = %d ; a = %d\n", 
                  x%rs, x/rs, F[x], a);
#endif
	}
        for (k = 0; k < 8; k += 1)             /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) && (a = extensible4(F,y,rs,N)))
          {
            LifoPush(LIFO1, ENCODE(y,a));
            Set(y,EN_LIFO);
#ifdef DEBUG
          printf("Push y = %d,%d ; F[y] = %d ; a = %d\n", 
                  y%rs, y/rs, F[y], a);
#endif
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

#ifdef ANIMATE
    if (niter % NITER1STEP == 0)
    {
      animname[4] = '0' + (char)((nstep / 1000) % 10); 
      animname[5] = '0' + (char)((nstep / 100) % 10); 
      animname[6] = '0' + (char)((nstep / 10) % 10); 
      animname[7] = '0' + (char)(nstep % 10); 
      animname[8] = '\0';
      writeimage(image, animname);
      nstep++;
    }
#endif

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */

  } /* if (connex == 8) */

#ifdef SATURE
} while (stable == 0);
#endif

  if (imcond != NULL)
  {
    for (x = 0; x < N; x++) if IsSet(x,COND_TRUE) G[x] = NDG_MAX;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lcrestrestoration() */

/* ==================================== */
int32_t lptextensible(struct xvimage *image, struct xvimage *imcond, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptextensible"
{ 
  int32_t k, x, y;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de condition */
  Lifo * LIFO1;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  LIFO1 = CreeLifoVide(N);
  if (LIFO1 == NULL)
  {
    fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
    return(0);
  }

  IndicsInit(N);

  if (imcond == NULL)
  {
    for (x = 0; x < N; x++) Set(x,COND_TRUE);
  }
  else
  {
    G = UCHARDATA(imcond);
    for (x = 0; x < N; x++) 
      if (G[x]) 
      {
        Set(x,COND_TRUE);
        LifoPush(LIFO1, x);
      }
    while (!LifoVide(LIFO1))
    {
      x = LifoPop(LIFO1);
      for (k = 0; k < 8; k += 1)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (! IsSet(y,COND_TRUE)) && (F[y] >= F[x]) && separant4(F, y, rs, N))
	{
          Set(y,COND_TRUE);
          LifoPush(LIFO1, y);
	} 
      }
    } // while (!LifoVide(LIFO1))
  } // if (imcond != NULL)

  if (connex == 4)
  {
    for (x = 0; x < N; x++) 
      if (extensible4(F, x, rs, N)) Set(x,EXTENSIBLE);
  }
  else
  { /* if (connex == 8) */
    printf("connex = %d NYI \n", connex);
    return 0;
  } /* if (connex == 8) */

  for (x = 0; x < N; x++) 
    if IsSet(x,EXTENSIBLE) F[x] = NDG_MAX; else F[x] = 0;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  LifoTermine(LIFO1);
  return(1);
} /* lptextensible() */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/*
          RECONSTRUCTION PRESERVANT LA DYNAMIQUE 
 */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/* version preservant la dynamique des maxima */

/* ==================================== */
int32_t modifiable4(uint8_t *F, uint8_t *G, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  return ( ((F[x] > G[x]) && pdestr4(F, x, rs, N)) ||
           ((F[x] < G[x]) && wellorconstr4(F, x, rs, N))
         );
} /* modifiable4() */

/* ==================================== */
int32_t testmodifie4(uint8_t *F, uint8_t *G, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  if (F[x] > G[x])
  {
    while ((F[x] > G[x]) && (pdestr4(F,x,rs,N)))
    { 
      modifie = 1; 
      F[x] = alpha8m(F,x,rs,N);
      if (F[x] < G[x]) F[x] = G[x]; 
    }
    return modifie;
  }

  if (F[x] < G[x])
  {
    while ((F[x] < G[x]) && (wellorconstr4(F,x,rs,N)))
    { 
      modifie = 1; 
      F[x] = alpha8p(F,x,rs,N); /* alpha8: sic */
      if (F[x] > G[x]) F[x] = G[x]; 
    }
    return modifie;
  }

  return modifie;
} /* testmodifie4() */

/* ==================================== */
int32_t modifiable8(uint8_t *F, uint8_t *G, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  return ( ((F[x] > G[x]) && pdestr8(F, x, rs, N)) ||
           ((F[x] < G[x]) && wellorconstr8(F, x, rs, N))
         );
} /* modifiable8() */

/* ==================================== */
int32_t testmodifie8(uint8_t *F, uint8_t *G, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  if (F[x] > G[x])
  {
    while ((F[x] > G[x]) && (pdestr8(F,x,rs,N)))
    { 
      modifie = 1; 
      F[x] = alpha8m(F,x,rs,N);
      if (F[x] < G[x]) F[x] = G[x]; 
    }
    return modifie;
  }

  if (F[x] < G[x])
  {
    while ((F[x] < G[x]) && (wellorconstr8(F,x,rs,N)))
    { 
      modifie = 1; 
      F[x] = alpha8p(F,x,rs,N); 
      if (F[x] > G[x]) F[x] = G[x]; 
    }
    return modifie;
  }

  return modifie;
} /* testmodifie8() */

/* ==================================== */
int32_t ldynrecons(struct xvimage *image, struct xvimage *imagecond, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldynrecons"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    return(0);
  }

  G = UCHARDATA(imagecond);
  if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != 1))
  {
    fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
    return(0);
  }

  IndicsInit(N);

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFahVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points modifiables */
  /* ========================================================= */

  if (connex == 4)
  {
    for (x = 0; x < N; x++) 
      if (modifiable4(F,G,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, mcabs(F[x]-G[x]));
	}
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++) 
      if (modifiable8(F,G,x,rs,N)) 
        {
          Set(x, EN_FAH);
          FahPush(FAH, x, mcabs(F[x]-G[x]));
	}
  }
  else
  {
    fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
    return(0);
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connex == 4)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testmodifie4(F, G, x, rs, N))       /* modifie l'image le cas echeant */
      {
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)) && (modifiable4(F,G,y,rs,N)))
          {
            FahPush(FAH, y, F[y]);
            Set(y, EN_FAH);
          }
        } /* for k */      
      } /* if (testmodifie4(F, G, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 4) */
  else
  { /* if (connex == 8) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testmodifie8(F, G, x, rs, N))       /* modifie l'image le cas echeant */
      {
        for (k = 0; k < 8; k += 1) /* parcourt les voisins en 8-connexite */
        {                                      /* pour empiler les voisins */
          y = voisin(x, k, rs, N);             /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)) && (modifiable8(F,G,y,rs,N)))
          {
            FahPush(FAH, y, F[y]);
            Set(y, EN_FAH);
          }
        } /* for k */      
      } /* if (testmodifie8(F, G, x, rs, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connex == 8) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* ldynrecons() */
