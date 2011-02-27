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
   Operateurs topologiques en 3d binaire et ndg

   mctopo3d_lhtkern3dbin - noyau homotopique binaire
      methode:  saturation par etapes (utilise 2 lifos)

   mctopo3d_lhtkern3d - noyau homotopique en niveaux de gris par abaissement
      methode: inodation par niveaux croissants - utilise une fah

   lhtkernu3d - noyau homotopique en niveaux de gris par elevation
      methode: inodation par niveaux croissants - utilise une fah

   lhthindelta3d - amincissement homotopique en niveaux de gris par etapes
      methode: memorise les points a abaisser et un niveau max d'abaissement
      utilise 2 lifos

   lhthickdelta3d - epaississement homotopique en niveaux de gris par etapes
      methode: memorise les points a elever et un niveau min d'elevation
      utilise 2 lifos

   llthin3d - amincissement par nivellement en niveaux de gris par etapes
      methode: memorise les points a abaisser et un niveau max d'abaissement
      utilise 2 lifos

   llthick3d - epaississement par nivellement en niveaux de gris par etapes
      methode: memorise les points a elever et un niveau max d'elevation
      utilise 2 lifos

   Michel Couprie -  juillet 1999 et janvier 2000

*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcfah.h>
#include <mctopo3d.h>
#include <mcutil.h>
#include <mcindic.h>
#include <lhtkern3d.h>

#define EN_FAH        0
#define EN_LIFO       0
#define PARANO

/*
#define DEBUG
#define VERBOSE
*/


#define ENCODE(y,a) (y|(a<<24))
#define DECODEX(y) (y&0x00ffffff)
#define DECODEA(y) ((y>>24)&0xff)

/* ******************************************************************************* */
/* ******************************************************************************* */
/*                            NOYAU HOMOTOPIQUE BINAIRE                            */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t mctopo3d_lhtkern3d_testabaisse6bin(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  if (mctopo3d_simple6(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = NDG_MIN;
  }

  return modifie;
} /* mctopo3d_lhtkern3d_testabaisse6bin() */

/* ==================================== */
int32_t testabaisse26bin(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  if (mctopo3d_simple26(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = NDG_MIN;
  }

  return modifie;
} /* testabaisse26bin() */

/* ==================================== */
int32_t mctopo3d_lhtkern3dbin(struct xvimage *image, int32_t connex, int32_t nimax)
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_lhtkern3dbin"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t d = depth(image);
  int32_t N = d * ps;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Lifo * LIFO1;
  Lifo * LIFO2;
  Lifo * LIFOtmp;
  int32_t niter;

  IndicsInit(N);
  mctopo3d_init_topo3d();
  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  if (nimax == -1) nimax = 2000000000;   

  /* ================================================ */
  /*            DEBUT ALGO                            */
  /* ================================================ */

    /* ========================================================= */
    /*   INITIALISATION DE LA LIFO: empile les points objet bord */
    /* ========================================================= */

  for (x = 0; x < N; x++)
  {
    if (F[x] && mctopo3d_bordext26(F, x, rs, ps, N))
    {
      LifoPush(LIFO1, x);
      Set(x, EN_LIFO);
    }
  }

  if (connex == 6)
  {
    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

    niter = 0;
    while (!LifoVide(LIFO1))
    {
      niter++; 
      if (niter > nimax) break;
      while (!LifoVide(LIFO1))
      {
        x = LifoPop(LIFO1);
        UnSet(x, EN_LIFO);
        if (mctopo3d_lhtkern3d_testabaisse6bin(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
        {
          for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
          {                                              /* pour empiler les voisins */
            y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
            if ((y != -1) && (F[y]) && (! IsSet(y, EN_LIFO)))
            {
              LifoPush(LIFO2, y);
              Set(y, EN_LIFO);
            } /* if y */
          } /* for k */      
        } /* if (mctopo3d_lhtkern3d_testabaisse6bin(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

      LIFOtmp = LIFO1; LIFO1 = LIFO2; LIFO2 = LIFOtmp;

    } /* while (! (LifoVide(LIFO1) ...)) */

  } /* if (connex == 6) */
  else
  if (connex == 26)
  {

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

    niter = 0;
    while (!LifoVide(LIFO1))
    {
      niter++; 
      if (niter > nimax) break;
      while (!LifoVide(LIFO1))
      {
        x = LifoPop(LIFO1);
        UnSet(x, EN_LIFO);
        if (testabaisse26bin(F, x, rs, ps, N))        /* modifie l'image le cas echeant */
        {
          for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
          {                                              /* pour empiler les voisins */
            y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
            if ((y != -1) && (F[y]) && (! IsSet(y, EN_LIFO)))
            {
              LifoPush(LIFO2, y);
              Set(y, EN_LIFO);
            } /* if y */
          } /* for k */      
        } /* if (testabaisse26bin(F, x, rs, N)) */
      } /* while (!LifoVide(LIFO1)) */

      LIFOtmp = LIFO1; LIFO1 = LIFO2; LIFO2 = LIFOtmp;

    } /* while (! (LifoVide(LIFO1) ...)) */

  } /* if (connex == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* mctopo3d_lhtkern3dbin() */

/* ******************************************************************************* */
/* ******************************************************************************* */
/*                            NOYAU HOMOTOPIQUE NDG                                */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t mctopo3d_lhtkern3d_testabaisse6(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_pdestr6(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26m(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* mctopo3d_lhtkern3d_testabaisse6() */

/* ==================================== */
int32_t testabaisse26(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_pdestr26(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26m(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testabaisse26() */

/* ==================================== */
int32_t testeleve6(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_pconstr6(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26p(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testeleve6() */

/* ==================================== */
int32_t testeleve26(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_pconstr26(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26p(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testeleve26() */

/* ==================================== */
int32_t mctopo3d_lhtkern3d(struct xvimage *image, struct xvimage *imagecond, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_lhtkern3d"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {
    fprintf(stderr, "%s: CreeFahVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points destructibles */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_pdestr6(F, x, rs, ps, N))) 
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pdestr6(F, x, rs, ps, N))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_pdestr26(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pdestr26(F, x, rs, ps, N))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connexmin == 6)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (mctopo3d_lhtkern3d_testabaisse6(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_pdestr6(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (mctopo3d_lhtkern3d_testabaisse6(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connexmin == 6) */
  else
  {   /* (connexmin == 26) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testabaisse26(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_pdestr26(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testabaisse26(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* mctopo3d_lhtkern3d() */

/* ==================================== */
int32_t lhtkernu3d(struct xvimage *image, struct xvimage *imagecond, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhtkernu3d"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {
    fprintf(stderr, "%s: CreeFahVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points constructibles */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_pconstr6(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pconstr6(F, x, rs, ps, N))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_pconstr26(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pconstr26(F, x, rs, ps, N))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connexmin == 6)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testeleve6(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_pconstr6(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testeleve6(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connexmin == 6) */
  else
  { /* (connexmin == 26) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testeleve26(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_pconstr26(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testeleve26(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* lhtkernu3d() */


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/*
          AMINCISSEMENT - EPAISSISSEMENT
 */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

/*
   principe de l'encodage du alpha- ou du delta-: 
   la valeur alpha- de x est placee dans les bits 24 a 31 de l'int32_t contenant x
                                                      24
   il reste donc 24 bits pour coder les pixels, soit 2   = 16 megapixels.
   (ex: image 4k x 4k)

   une valeur -1 pour nitermax signifie de laisser calculer jusqu'a stabilite (en fait
   2000000000 iterations) 
*/

/* ******************************************************************************* */
/* ******************************************************************************* */
/*                          AMINCISSEMENT HOMOTOPIQUE NDG                          */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t lhthindelta3d(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhthindelta3d"
{ 
  int32_t x, y, k;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nb plans */
  int32_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return 0;
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points destructibles avec leur delta-  */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_pdestr6(F, x, rs, ps, N))) 
        {
          a = mctopo3d_delta6m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pdestr6(F, x, rs, ps, N)) 
	{
          a = mctopo3d_delta6m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
  } /* if (connexmin == 6) */
  else
  { /* (connexmin == 26) */
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_pdestr26(F, x, rs, ps, N))) 
        {
          a = mctopo3d_delta26m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pdestr26(F, x, rs, ps, N)) 
	{
          a = mctopo3d_delta26m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
  } /* (connexmin == 26) */

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connexmin == 6)
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
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d(%x) ; y = %x\n", 
                x%rs, (x%ps)/rs, x/ps, F[x], a, a, y);
#endif
        if (mctopo3d_pdestr6(F, x, rs, ps, N))
	{
          F[x] = mcmax(mctopo3d_delta6m(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_pdestr6(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (mctopo3d_pdestr6(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_delta6m(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_delta6m(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_pdestr6(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_delta6m(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_delta6m(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connexmin == 6) */

  else

  { /* (connexmin == 26) */
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
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d(%x) ; y = %x\n", 
                x%rs, (x%ps)/rs, x/ps, F[x], a, a, y);
#endif
        if (mctopo3d_pdestr26(F, x, rs, ps, N))
	{
          F[x] = mcmax(mctopo3d_delta26m(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_pdestr26(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (mctopo3d_pdestr26(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_delta26m(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_delta26m(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_pdestr26(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_delta26m(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_delta26m(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return 1;
} /* lhthindelta3d() */

/* ******************************************************************************* */
/* ******************************************************************************* */
/*                          EPAISSISSEMENT HOMOTOPIQUE NDG                         */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t lhthickdelta3d(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhthickdelta3d"
{ 
  int32_t x, y, k;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nb plans */
  int32_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return 0;
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points constructibles avec leur delta+  */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_pconstr6(F, x, rs, ps, N))) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_delta6p(F, x, rs, ps, N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pconstr6(F, x, rs, ps, N)) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_delta6p(F, x, rs, ps, N)));
    }
  } /* if (connexmin == 6) */
  else
  { /* (connexmin == 26) */
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_pconstr26(F, x, rs, ps, N))) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_delta26p(F, x, rs, ps, N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_pconstr26(F, x, rs, ps, N)) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_delta26p(F, x, rs, ps, N)));
    }
  } /* (connexmin == 26) */

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connexmin == 6)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d\n", x%rs, (x%ps)/rs, x/ps, F[x], a);
#endif
        if (mctopo3d_pconstr6(F, x, rs, ps, N))
	{
          F[x] = mcmin(mctopo3d_delta6p(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_pconstr6(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (mctopo3d_pconstr6(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_delta6p(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_delta6p(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_pconstr6(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_delta6p(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_delta6p(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connexmin == 6) */

  else
 
  { /* (connexmin == 26) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points constructibles */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d\n", x%rs, (x%ps)/rs, x/ps, F[x], a);
#endif
        if (mctopo3d_pconstr26(F, x, rs, ps, N))
	{
          F[x] = mcmin(mctopo3d_delta26p(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_pconstr26(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (mctopo3d_pconstr26(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_delta26p(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_delta26p(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_pconstr26(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_delta26p(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_delta26p(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lhthickdelta3d() */


/* ******************************************************************************* */
/* ******************************************************************************* */
/*                          AMINCISSEMENT PAR NIVELLEMENT NDG                      */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t testlevabaisse6(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_plevdestr6(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26m(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testlevabaisse6() */

/* ==================================== */
int32_t testlevabaisse26(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_plevdestr26(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26m(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testlevabaisse26() */

/* ==================================== */
int32_t testleveleve6(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_plevconstr6(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26p(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testleveleve6() */

/* ==================================== */
int32_t testleveleve26(uint8_t *F, int32_t x, int32_t rs, int32_t ps, int32_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while (mctopo3d_plevconstr26(F, x, rs, ps, N)) 
  { 
    modifie = 1; 
    F[x] = mctopo3d_alpha26p(F, x, rs, ps, N); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* testleveleve26() */

/* ==================================== */
int32_t llvkern3d(struct xvimage *image, struct xvimage *imagecond, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "llvkern3d"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {
    fprintf(stderr, "%s: CreeFahVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points nivelables */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_plevdestr6(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevdestr6(F, x, rs, ps, N))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_plevdestr26(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevdestr26(F, x, rs, ps, N))
        {
          FahPush(FAH, x, F[x]);
          Set(x, EN_FAH);
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connexmin == 6)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testlevabaisse6(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_plevdestr6(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testlevabaisse6(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connexmin == 6) */
  else
  {   /* (connexmin == 26) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testlevabaisse26(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_plevdestr26(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testlevabaisse26(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* llvkern3d() */

/* ==================================== */
int32_t llvkernu3d(struct xvimage *image, struct xvimage *imagecond, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "llvkernu3d"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  Fah * FAH;

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {
    fprintf(stderr, "%s: CreeFahVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA FAH: empile tous les points nivelables */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_plevconstr6(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevconstr6(F, x, rs, ps, N))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
  }
  else
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_plevconstr26(F, x, rs, ps, N)))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevconstr26(F, x, rs, ps, N))
        {
          FahPush(FAH, x, NDG_MAX-F[x]);
          Set(x, EN_FAH);
	}
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  if (connexmin == 6)
  {
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testleveleve6(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_plevconstr6(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testleveleve6(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* if (connexmin == 6) */
  else
  { /* (connexmin == 26) */
    while (!FahVide(FAH))
    {
      x = FahPop(FAH);
      UnSet(x, EN_FAH);
      if (testleveleve26(F, x, rs, ps, N))         /* modifie l'image le cas echeant */
      {
        if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
        for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
        {                                       /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                /* non deja empiles */
          if ((y != -1) && (! IsSet(y, EN_FAH)))
          {
            if (((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_plevconstr26(F, y, rs, ps, N))) 
            {
              FahPush(FAH, y, NDG_MAX-F[y]);
              Set(y, EN_FAH);
            }
          } /* if y */
        } /* for k */      
      } /* if (testleveleve26(F, x, rs, ps, N)) */
    } /* while (!FahVide(FAH)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* llvkernu3d() */

/* ==================================== */
int32_t llthin3d(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "llthin3d"
{ 
  int32_t x, y, k;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nb plans */
  int32_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return 0;
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points nivelables avec leur alpha-  */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_plevdestr6(F, x, rs, ps, N))) 
        {
          a = mctopo3d_alpha26m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevdestr6(F, x, rs, ps, N)) 
	{
          a = mctopo3d_alpha26m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
  } /* if (connexmin == 6) */
  else
  { /* (connexmin == 26) */
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] < F[x]) && (mctopo3d_plevdestr26(F, x, rs, ps, N))) 
        {
          a = mctopo3d_alpha26m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevdestr26(F, x, rs, ps, N)) 
	{
          a = mctopo3d_alpha26m(F, x, rs, ps, N);
          LifoPush(LIFO1, ENCODE(x,a));
#ifdef DEBUG
          printf("Init : Push x = %d,%d,%d(%x) ; F[x] = %d ; a = %d(%x) ; ENCODE(x,a) = %x\n", 
                  x%rs, (x%ps)/rs, x/ps, x, F[x], a, a, ENCODE(x,a));
#endif
	}
    }
  } /* (connexmin == 26) */

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connexmin == 6)
  {
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points nivelables */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d(%x) ; y = %x\n", 
                x%rs, (x%ps)/rs, x/ps, F[x], a, a, y);
#endif
        if (mctopo3d_plevdestr6(F, x, rs, ps, N))
	{
          F[x] = mcmax(mctopo3d_alpha26m(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_plevdestr6(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (mctopo3d_plevdestr6(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_alpha26m(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_alpha26m(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_plevdestr6(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_alpha26m(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_alpha26m(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connexmin == 6) */

  else

  { /* (connexmin == 26) */
    while (!LifoVide(LIFO1) && (niter < nitermax))
    {
      niter++;

#ifdef VERBOSE
  fprintf(stderr, "%s: niter = %d\n", F_NAME, niter);
#endif

  /* --------------------------------------------------------- */
  /* 1ere demi iteration : on abaisse les points nivelables */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO1))
      {
        y = LifoPop(LIFO1);
        x = DECODEX(y);
        a = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d(%x) ; y = %x\n", 
                x%rs, (x%ps)/rs, x/ps, F[x], a, a, y);
#endif
        if (mctopo3d_plevdestr26(F, x, rs, ps, N))
	{
          F[x] = mcmax(mctopo3d_alpha26m(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmax(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_plevdestr26(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] < F[x])) && (mctopo3d_plevdestr26(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_alpha26m(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_alpha26m(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] < F[y])) && (mctopo3d_plevdestr26(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_alpha26m(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_alpha26m(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return 1;
} /* llthin3d() */

/* ******************************************************************************* */
/* ******************************************************************************* */
/*                          EPAISSISSEMENT PAR NIVELLEMENT NDG                     */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t llthick3d(struct xvimage *image, struct xvimage *imagecond, int32_t nitermax, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "llthick3d"
{ 
  int32_t x, y, k;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nb plans */
  int32_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *G;            /* l'image de conditionnement */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t a;

  if (nitermax == -1) nitermax = 2000000000;   

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    return 0;
  }

  if (imagecond != NULL)
  {
    G = UCHARDATA(imagecond);
    if ((rowsize(imagecond) != rs) || (colsize(imagecond) != cs) || (depth(imagecond) != ds))
    {
      fprintf(stderr, "%s: tailles image et imagecond incompatibles\n", F_NAME);
      return 0;
    }
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points nivelables avec leur alpha+  */
  /* ========================================================= */

  if (connexmin == 6)
  {
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_plevconstr6(F, x, rs, ps, N))) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_alpha26p(F, x, rs, ps, N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevconstr6(F, x, rs, ps, N)) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_alpha26p(F, x, rs, ps, N)));
    }
  } /* if (connexmin == 6) */
  else
  { /* (connexmin == 26) */
    if (imagecond != NULL)
    {
      for (x = 0; x < N; x++) 
        if ((G[x] > F[x]) && (mctopo3d_plevconstr26(F, x, rs, ps, N))) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_alpha26p(F, x, rs, ps, N)));
    }
    else
    {
      for (x = 0; x < N; x++) 
        if (mctopo3d_plevconstr26(F, x, rs, ps, N)) 
          LifoPush(LIFO1, ENCODE(x, mctopo3d_alpha26p(F, x, rs, ps, N)));
    }
  } /* (connexmin == 26) */

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connexmin == 6)
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
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d\n", x%rs, (x%ps)/rs, x/ps, F[x], a);
#endif
        if (mctopo3d_plevconstr6(F, x, rs, ps, N))
	{
          F[x] = mcmin(mctopo3d_alpha26p(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_plevconstr6(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (mctopo3d_plevconstr6(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_alpha26p(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_alpha26p(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_plevconstr6(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_alpha26p(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_alpha26p(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connexmin == 6) */

  else
 
  { /* (connexmin == 26) */
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
        printf("Pop x = %d,%d,%d ; F[x] = %d ; a = %d\n", x%rs, (x%ps)/rs, x/ps, F[x], a);
#endif
        if (mctopo3d_plevconstr26(F, x, rs, ps, N))
	{
          F[x] = mcmin(mctopo3d_alpha26p(F, x, rs, ps, N),a);
          if (imagecond != NULL) F[x] = mcmin(F[x],G[x]);
          LifoPush(LIFO2, x);
        } /* if (mctopo3d_plevconstr26(F, x, rs, ps, N)) */
      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) &&
            ((imagecond == NULL) || (G[x] > F[x])) && (mctopo3d_plevconstr26(F, x, rs, ps, N)))
        {
#ifdef DEBUG
          printf("Push x = %d,%d,%d ; F[x] = %d ; a- = %d\n", 
                  x%rs, (x%ps)/rs, x/ps, F[x], mctopo3d_alpha26p(F, x, rs, ps, N));
#endif
          LifoPush(LIFO1, ENCODE(x,mctopo3d_alpha26p(F,x,rs,ps,N)));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) &&
              ((imagecond == NULL) || (G[y] > F[y])) && (mctopo3d_plevconstr26(F, y, rs, ps, N)))
          {
#ifdef DEBUG
            printf("Push y = %d,%d,%d ; F[y] = %d ; a- = %d\n", 
                    y%rs, (y%ps)/rs, y/ps, F[y], mctopo3d_alpha26p(F, y, rs, ps, N));
#endif
            LifoPush(LIFO1, ENCODE(y,mctopo3d_alpha26p(F,y,rs,ps,N)));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* (connexmin == 26) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* llthick3d() */

/* ******************************************************************************* */
/* ******************************************************************************* */
/*                        RESTAURATION DE SURFACES NDG 3D                          */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t extensible6(
  uint8_t *F,            /* pointeur base image */
  int32_t x,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t ps,                      /* taille plan */
  int32_t N)                       /* taille image */
/* ==================================== */
/*
   Le point x est extensible au niveau nivext si : 
     i) x est separant
     ii) x possede dans son voisinage un point y verifiant :
         F[y] = nivext
         y separant pour le niveau F[x]
         y non separant pour tout niveau > F[x]

   La fonction retourne 0 si non extensible, le plus petit nivext trouve sinon.
*/
{
  int32_t k, y;
  int32_t nivext = 256;

  if (!mctopo3d_separant6(F, x, rs, ps, N)) return 0;

  for (k = 0; k < 26; k += 1)
  {
    y = voisin26(x, k, rs, ps, N);
    if (y != -1)
      if ((F[y] > F[x]) && mctopo3d_hfseparant6(F,y,F[x],rs,ps,N) && !mctopo3d_hfseparant6(F,y,F[x],rs,ps,N)) 
      {
        if (F[y] < nivext) nivext = F[y]; 
#ifdef DEBUG
        printf("extensible6: x = %d,%d,%d ; y = %d,%d,%d ; F[y] = %d \n", 
                x%rs, (x%ps)/rs, x/ps, y%rs, (y%ps)/rs, y/ps, F[y]);
#endif
      }
  }

  if (nivext == 256) return 0;
  return nivext;
} /* extensible6() */

/* ==================================== */
int32_t trouextensible6(
  uint8_t *F,            /* pointeur base image */
  int32_t x,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t ps,                      /* taille plan */
  int32_t N)                       /* taille image */
/* ==================================== */
{
  int32_t a;

  if (!mctopo3d_separant6(F, x, rs, ps, N)) return 0;
  a = mctopo3d_alpha26p(F, x, rs, ps, N);
  if (a == F[x]) return 0;
  if (!mctopo3d_hfseparant6(F, x, a, rs, ps, N)) return 0;
  return 1;
} /* trouextensible6() */

/* ==================================== */
int32_t lsurfacerestoration(struct xvimage *image, int32_t nitermax, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsurfacerestoration"
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t d = depth(image);
  int32_t N = d * ps;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t niter;                   /* nombre d'iterations effectuees */
  Lifo * LIFO1;
  Lifo * LIFO2;
  int32_t nivext;

  if (depth(image) == 1) 
  {
    fprintf(stderr, "%s: cette operation n'a de sens que pour des images volumiques\n", F_NAME);
    exit(0);
  }

  if (N > (1<<24)) 
  {
    fprintf(stderr, "%s: image trop grande (limite 16 M)\n", F_NAME);
    exit(0);
  }

  IndicsInit(N);

  mctopo3d_init_topo3d();

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) && (LIFO2 == NULL))
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DE LA LIFO: empile tous les points 
       extensibles avec leur delta+*/
  /* ========================================================= */

  if (connexmin == 6)
  {
    for (x = 0; x < N; x++) 
      if ((nivext = extensible6(F, x, rs, ps, N)))
      {
        LifoPush(LIFO1, ENCODE(x,nivext));
#ifdef DEBUG
        printf("INIT: Push x = %d,%d,%d ; F[x] = %d ; nivext = %d\n", 
                x%rs, (x%ps)/rs, x/ps, F[x], nivext);
#endif
      }
  }
  else
  {
    printf("connexmin = %d NYI \n", connexmin);
    return 0;
  }

  /* ================================================ */
  /*                  DEBUT ITERATIONS                */
  /* ================================================ */

  niter = 0;

  if (connexmin == 6)
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
        nivext = DECODEA(y);
        UnSet(x,EN_LIFO);
#ifdef DEBUG
        printf("Pop x = %d,%d,%d ; F[x] = %d ; nivext = %d\n", 
                x%rs, (x%ps)/rs, x/ps, F[x], nivext);
#endif

        if (mctopo3d_pconstr6(F, x, rs, ps, N))
	{
          F[x] = mcmin(mctopo3d_delta6p(F,x,rs,ps,N),nivext);
          LifoPush(LIFO2, x);
#ifdef DEBUG
          printf("========> ELEVE constr : %d\n", F[x]);
#endif
        } /* if (mctopo3d_pconstr6(F, x, rs, ps, N)) */

        else if (trouextensible6(F, x, rs, ps, N))
	{
          F[x] = mctopo3d_alpha26p(F, x, rs, ps, N);
          LifoPush(LIFO2, x);
#ifdef DEBUG
          printf("========> ELEVE trou : %d\n", F[x]);
#endif
        } /* if () */

      } /* while (!LifoVide(LIFO1)) */

  /* --------------------------------------------------------- */
  /* 2eme demi iteration : on empile les voisins extensibles   */
  /* --------------------------------------------------------- */

      while (!LifoVide(LIFO2))
      {
        x = LifoPop(LIFO2);
        if ((! IsSet(x,EN_LIFO)) && (nivext = extensible6(F, x, rs, ps, N)))
        {
          LifoPush(LIFO1, ENCODE(x,nivext));
          Set(x,EN_LIFO);
	}
        for (k = 0; k < 26; k += 1)            /* parcourt les voisins */
        {                                      /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);       /* non deja empiles */
          if ((y != -1) && (! IsSet(y,EN_LIFO)) && (nivext = extensible6(F,y,rs,ps,N)))
          {
            LifoPush(LIFO1, ENCODE(y,nivext));
            Set(y,EN_LIFO);
          } /* if y */
        } /* for k */      
      } /* while (!LifoVide(LIFO2)) */

    } /* while (! (LifoVide(LIFO1) ...)) */
  } /* if (connexmin == 6) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} /* lsurfacerestoration() */
