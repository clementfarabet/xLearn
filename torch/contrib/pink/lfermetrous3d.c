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
   Operateur de fermeture de trous en 3d binaire

   Michel Couprie - d'apres Gilles Bertrand - decembre 1999 

   Methode : Soit X l'objet initial dont on veut fermer les trous.
             Soit Y un objet simplement connexe tel que X c Y (on prend
             pour Y un pave englobant X).

             On repete jusqu'a stabilite:
               - choisir un point x de Y \ X tel que Tbar(x,Y) = 1
               - faire Y = Y \ {x}

             Pour le choix de x, on se base sur une information de distance: 
             on prend en priorite les points les plus eloignes de l'objet.

             Pour controler la "taille" des trous a laisser ouverts, on 
             supprime egalement les points x de Y \ X tels que Tbar(x,Y) = 2
             et dist(x,X) > tailletrous.

   Operateur de fermeture de trous en 3d niveaux de gris

   Michel Couprie - d'apres Gilles Bertrand - janvier 2000 

   Methode : Soit X l'image ndg originale.
             Soit Y une image valant NDG_MAX partout sauf en un point de 
             chacun des minima regionaux de X (on a alors Y(m) = X(m))

             On repete jusqu'a stabilite:
               - choisir un point x tel que Y(x) > X(x) et T--(x,Y) = 1
               - faire Y = mcmax(alpha-(x,Y), X(x))

   MC Update juin 2001 : generation d'una animation (flag ANIMATE)

   MC Update 2008 : lfermetrous3dbin2

   MC Update jan. 2011: lfermetrous3dbin_table, lfermetrous3dbin2_table
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfahpure.h>
#include <mctopo3d.h>
#include <mctopo3d_table.h>
#include <mcutil.h>
#include <mcindic.h>
#include <mcgeo.h>
#include <ldist.h>
#include <lcrop.h>
#include <llabelextrema.h>
#include <lfermetrous3d.h>

/*
#define DEBUG
#define DEBUG_MARCIN
*/

//#define VERBOSE
/* 
#define ANIMATE
*/

#define VAL_NULLE   0
#define VAL_X     255
#define VAL_Y_X   254
#define VAL_Y_X_M 253

#define EN_FAHP      0

#ifdef ANIMATE
void dump_anime(uint8_t *F, uint8_t *A, index_t N, index_t rs, index_t ps)
{
  index_t x, k, j, n;

  for (x = 0; x < N; x++)
  {
    if (F[x] == 0) A[x] = 0; 
    else
    {
      n = 0;
      for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
      {
        j = voisin6(x, k, rs, ps, N);
        if ((j != -1) && (F[j]>=F[x])) n++;
      }    
      if (n < 6)
      {
        if (F[x] == VAL_X) A[x] = 255; 
        else A[x] = 254;
      }
      else A[x] = 0;
    }
  }
}
#endif

// locally defined and used functions
int32_t lfermetrous3dbin_notable(struct xvimage *image, int32_t connex, int32_t tailletrous);
int32_t lfermetrous3dbin_table(struct xvimage *image, int32_t connex, int32_t tailletrous);
int32_t lfermetrous3dbin2_notable(struct xvimage *in, struct xvimage *g, int32_t connex, int32_t tailletrous);
int32_t lfermetrous3dbin2_table(struct xvimage *in, struct xvimage *g, int32_t connex, int32_t tailletrous);

// public
/* ==================================== */
int32_t lfermetrous3dbin(struct xvimage *in, int32_t connex, int32_t tailletrous)
/* ==================================== */
{
  if (connex == 26) return lfermetrous3dbin_table(in, connex, tailletrous); 
  else              return lfermetrous3dbin_notable(in, connex, tailletrous); 
}

// public
/* ==================================== */
int32_t lfermetrous3dbin2(struct xvimage *in, struct xvimage *g, int32_t connex, int32_t tailletrous)
/* ==================================== */
{
  if (connex == 26) return lfermetrous3dbin2_table(in, g, connex, tailletrous); 
  else              return lfermetrous3dbin2_notable(in, g, connex, tailletrous); 
}

/* ==================================== */
int32_t lfermetrous3dbin_notable(struct xvimage *image, int32_t connex, int32_t tailletrous)
/* ==================================== */
#undef F_NAME
#define F_NAME "lfermetrous3dbin_notable"
{ 
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t z;                       /* index muet (generalement un voisin de y) */
  int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(image);       /* nombre plans */
  index_t N = ds * ps;             /* taille image */
  struct xvimage *dist;        /* pour la fonction distance */
  uint8_t *F = UCHARDATA(image);
  uint32_t *D;
  Fahp * FAHP;                   /* fahp pour le controle de l'ordre de traitement des points */
  int32_t xmin, xmax, ymin, ymax, zmin, zmax; /* le pave englobant */
  int32_t tbar;
#ifdef ANIMATE
  int32_t anime_dist = 0;
  int32_t anime_step;
  char animname[64];
  struct xvimage *animimage;
  uint8_t *A;
#endif

#ifdef DEBUG
  fprintf(stderr, "%s: connex = %d, tailletrous = %d\n", F_NAME, connex, tailletrous);
#endif

  mctopo3d_init_topo3d();

  if (tailletrous == -1) tailletrous = 1000000000;

  FAHP = CreeFahpVide(N);
  if (FAHP == NULL)
  {   fprintf(stderr, "%s: CreeFahpVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* calcul fonction distance */

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;
  if (! lsedt_meijster(image, dist))
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return 0;
  }
  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;

  {
    double d;
    for (x = 0; x < N; x++) 
    {
      d = sqrt((double)(D[x]));
      D[x] = (uint32_t)arrondi(d);
    }
  }

  /* calcul du pave englobant */

  xmin = rs - 1; xmax = 0;
  ymin = cs - 1; ymax = 0;
  zmin = ds - 1; zmax = 0;
  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    if (F[z * ps + y * rs + x])
    {
      F[z * ps + y * rs + x] = VAL_X;      /* marque X au passage */
      if (x < xmin) xmin = x; else if (x > xmax) xmax = x;
      if (y < ymin) ymin = y; else if (y > ymax) ymax = y;
      if (z < zmin) zmin = z; else if (z > zmax) zmax = z;
    } /* if (F[z * ps + y * rs + x]) */
  } /* for z y x ... */

  if ((xmin == 0) || (ymin == 0) || (zmin == 0) || 
      (xmax == rs-1) || (ymax == cs-1) || (zmax == ds-1))
  {
    fprintf(stderr, "%s: there are object points on the border\n", F_NAME);
    return(0);
  }

  /* marquage de Y \ X */

  for (z = zmin; z <= zmax; z++)
  for (y = ymin; y <= ymax; y++)
  for (x = xmin; x <= xmax; x++)
  {
    if (!F[z * ps + y * rs + x]) 
      F[z * ps + y * rs + x] = VAL_Y_X;    
  } /* for z y x ... */

  if (connex == 6)
  {
    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      if (F[x] == VAL_Y_X)
      {
	tbar = mctopo3d_tbar6h(F, x, VAL_Y_X_M, rs, ps, N); 
	if (((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))))
        {
	  assert(D[x] <= FAHP_NPRIO);
	  FahpPush(FAHP, x, mcmax(0,(FAHP_NPRIO-D[x])));
	  F[x] = VAL_Y_X_M;
#ifdef ANIMATE
	  if (D[x] > anime_dist) anime_dist = D[x];
#endif
	}  
      } // if (F[x] == VAL_Y_X)
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

#ifdef ANIMATE
    printf("distance = %d\n", anime_dist);
    anime_step = 0;
    strcpy(animname, "ANIM");
    animimage = copyimage(image);
    A = UCHARDATA(animimage);

    animname[4] = '0' + (char)((anime_step / 1000) % 10); 
    animname[5] = '0' + (char)((anime_step / 100) % 10); 
    animname[6] = '0' + (char)((anime_step / 10) % 10); 
    animname[7] = '0' + (char)(anime_step % 10); 
    animname[8] = '\0';
    dump_anime(F, A, N, rs, ps);
    writeimage(animimage, animname);
    anime_step++;
#endif

    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
#ifdef ANIMATE
      if (D[x] < anime_dist) 
      {
        anime_dist = D[x];
        printf("distance = %d\n", anime_dist); 

        animname[4] = '0' + (char)((anime_step / 1000) % 10); 
        animname[5] = '0' + (char)((anime_step / 100) % 10); 
        animname[6] = '0' + (char)((anime_step / 10) % 10); 
        animname[7] = '0' + (char)(anime_step % 10); 
        animname[8] = '\0';
        dump_anime(F, A, N, rs, ps);
        writeimage(animimage, animname);
        anime_step++;
      }
#endif
      F[x] = VAL_Y_X;
      tbar = mctopo3d_tbar6h(F, x, VAL_Y_X_M, rs, ps, N); 
      if ((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous)))
      {
        F[x] = VAL_NULLE;
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (F[y] == VAL_Y_X))
          {
	    assert(D[y] <= FAHP_NPRIO);
            FahpPush(FAHP, y, mcmax(0,(FAHP_NPRIO-D[y])));
            F[y] = VAL_Y_X_M;
          } /* if y */
        } /* for k */      
      } /* if (tbareqone6h(F, x, VAL_Y_X_M, rs, ps, N)) */
    } /* while (!FahpVide(FAHP)) */

#ifdef ANIMATE
    animname[4] = '0' + (char)((anime_step / 1000) % 10); 
    animname[5] = '0' + (char)((anime_step / 100) % 10); 
    animname[6] = '0' + (char)((anime_step / 10) % 10); 
    animname[7] = '0' + (char)(anime_step % 10); 
    animname[8] = '\0';
    dump_anime(F, A, N, rs, ps);
    writeimage(animimage, animname);
    anime_step++;
#endif

  } /* if (connex == 6) */
  else if (connex == 26)
  {
    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      if (F[x] == VAL_Y_X)
      {
	tbar = mctopo3d_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
	if (((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))))
        {
	  assert(D[x] <= FAHP_NPRIO);
	  FahpPush(FAHP, x, mcmax(0,(FAHP_NPRIO-D[x])));
	  F[x] = VAL_Y_X_M;
#ifdef ANIMATE
	  if (D[x] > anime_dist) anime_dist = D[x];
#endif
	}
      }
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

#ifdef ANIMATE
    printf("distance = %d\n", anime_dist);
    anime_step = 0;
    strcpy(animname, "ANIM");
    animimage = copyimage(image);
    A = UCHARDATA(animimage);

    animname[4] = '0' + (char)((anime_step / 1000) % 10); 
    animname[5] = '0' + (char)((anime_step / 100) % 10); 
    animname[6] = '0' + (char)((anime_step / 10) % 10); 
    animname[7] = '0' + (char)(anime_step % 10); 
    animname[8] = '\0';
    dump_anime(F, A, N, rs, ps);
    writeimage(animimage, animname);
    anime_step++;
#endif

    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
#ifdef ANIMATE
      if (D[x] < anime_dist) 
      {
        anime_dist = D[x];
        printf("distance = %d\n", anime_dist); 

        animname[4] = '0' + (char)((anime_step / 1000) % 10); 
        animname[5] = '0' + (char)((anime_step / 100) % 10); 
        animname[6] = '0' + (char)((anime_step / 10) % 10); 
        animname[7] = '0' + (char)(anime_step % 10); 
        animname[8] = '\0';
        dump_anime(F, A, N, rs, ps);
        writeimage(animimage, animname);
        anime_step++;
      }
#endif
      F[x] = VAL_Y_X;
      tbar = mctopo3d_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
      if ((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))) 
      {
        F[x] = VAL_NULLE;
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (F[y] == VAL_Y_X))
          {
	    assert(D[y] <= FAHP_NPRIO);
            FahpPush(FAHP, y, mcmax(0,(FAHP_NPRIO-D[y])));
            F[y] = VAL_Y_X_M;
          } /* if y */
        } /* for k */      
      } /* if (tbareqone26h(F, x, VAL_Y_X_M, rs, ps, N)) */
    } /* while (!FahpVide(FAHP)) */

#ifdef ANIMATE
    animname[4] = '0' + (char)((anime_step / 1000) % 10); 
    animname[5] = '0' + (char)((anime_step / 100) % 10); 
    animname[6] = '0' + (char)((anime_step / 10) % 10); 
    animname[7] = '0' + (char)(anime_step % 10); 
    animname[8] = '\0';
    dump_anime(F, A, N, rs, ps);
    writeimage(animimage, animname);
    anime_step++;
#endif

  } /* if (connex == 26) */
  else
  {
    fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
    return 0;
  }

  for (x = 0; x < N; x++) if (F[x] != VAL_NULLE) F[x] = VAL_X;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  FahpTermine(FAHP);
  freeimage(dist);
  return(1);
} /* lfermetrous3dbin_notable() */

/* ==================================== */
int32_t lfermetrous3dbin2_notable(struct xvimage *image, struct xvimage *guide, int32_t connex, int32_t tailletrous)
/* ==================================== */
#undef F_NAME
#define F_NAME "lfermetrous3dbin2_notable"
// same as lfermetrous3dbin except that the distance map is computed from image 'guide'
{ 
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t z;                       /* index muet (generalement un voisin de y) */
  int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(image);       /* nombre plans */
  index_t N = ds * ps;             /* taille image */
  struct xvimage *dist;        /* pour la fonction distance */
  uint8_t *F = UCHARDATA(image);
  uint32_t *D;
  Fahp * FAHP;                   /* fahp pour le controle de l'ordre de traitement des points */
  int32_t xmin, xmax, ymin, ymax, zmin, zmax; /* le pave englobant */
  int32_t tbar;

  mctopo3d_init_topo3d();

  if (tailletrous == -1) tailletrous = 1000000000;

  FAHP = CreeFahpVide(N);
  if (FAHP == NULL)
  {   fprintf(stderr, "%s: CreeFahpVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* calcul fonction distance */

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;
  if (! lsedt_meijster(guide, dist))
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return 0;
  }
  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;

  {
    double d;
    for (x = 0; x < N; x++) 
    {
      d = sqrt((double)(D[x]));
      D[x] = (uint32_t)arrondi(d);
    }
  }

  /* calcul du pave englobant */

  xmin = rs - 1; xmax = 0;
  ymin = cs - 1; ymax = 0;
  zmin = ds - 1; zmax = 0;
  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    if (F[z * ps + y * rs + x])
    {
      F[z * ps + y * rs + x] = VAL_X;      /* marque X au passage */
      if (x < xmin) xmin = x; else if (x > xmax) xmax = x;
      if (y < ymin) ymin = y; else if (y > ymax) ymax = y;
      if (z < zmin) zmin = z; else if (z > zmax) zmax = z;
    } /* if (F[z * ps + y * rs + x]) */
  } /* for z y x ... */

  if ((xmin == 0) || (ymin == 0) || (zmin == 0) || 
      (xmax == rs-1) || (ymax == cs-1) || (zmax == ds-1))
  {
    fprintf(stderr, "%s: there are object points on the border\n", F_NAME);
    return(0);
  }

  /* marquage de Y \ X */

  for (z = zmin; z <= zmax; z++)
  for (y = ymin; y <= ymax; y++)
  for (x = xmin; x <= xmax; x++)
  {
    if (!F[z * ps + y * rs + x]) 
      F[z * ps + y * rs + x] = VAL_Y_X;    
  } /* for z y x ... */

  if (connex == 6)
  {
    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      if (F[x] == VAL_Y_X)
      {
	tbar = mctopo3d_tbar6h(F, x, VAL_Y_X_M, rs, ps, N); 
	if (((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))))
        {
	  assert(D[x] <= FAHP_NPRIO);
	  FahpPush(FAHP, x, mcmax(0,(FAHP_NPRIO-D[x])));
	  F[x] = VAL_Y_X_M;
	}
      }
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
      F[x] = VAL_Y_X;
      tbar = mctopo3d_tbar6h(F, x, VAL_Y_X_M, rs, ps, N); 
      if ((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous)))
      {
        F[x] = VAL_NULLE;
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (F[y] == VAL_Y_X))
          {
	    assert(D[y] <= FAHP_NPRIO);
            FahpPush(FAHP, y, mcmax(0,(FAHP_NPRIO-D[y])));
            F[y] = VAL_Y_X_M;
          } /* if y */
        } /* for k */      
      } /* if (tbareqone6h(F, x, VAL_Y_X_M, rs, ps, N)) */
    } /* while (!FahpVide(FAHP)) */

  } /* if (connex == 6) */
  else if (connex == 26)
  {
    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      if (F[x] == VAL_Y_X)
      {
	tbar = mctopo3d_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
	if (((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))))
        {
	  assert(D[x] <= FAHP_NPRIO);
	  FahpPush(FAHP, x, mcmax(0,(FAHP_NPRIO-D[x])));
	  F[x] = VAL_Y_X_M;
	}
      }
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
      F[x] = VAL_Y_X;
      tbar = mctopo3d_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
      if ((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))) 
      {
        F[x] = VAL_NULLE;
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (F[y] == VAL_Y_X))
          {
	    assert(D[y] <= FAHP_NPRIO);
            FahpPush(FAHP, y, mcmax(0,(FAHP_NPRIO-D[y])));
            F[y] = VAL_Y_X_M;
          } /* if y */
        } /* for k */      
      } /* if (tbareqone26h(F, x, VAL_Y_X_M, rs, ps, N)) */
#ifdef DEBUG_MARCIN
      else
      {
	printf("On garde le point %d,%d,%d\n", x % rs, (x % ps) / rs, x / ps);
      }
#endif
    } /* while (!FahpVide(FAHP)) */

  } /* if (connex == 26) */
  else
  {
    fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
    return 0;
  }

  for (x = 0; x < N; x++) if (F[x] != VAL_NULLE) F[x] = VAL_X;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  FahpTermine(FAHP);
  freeimage(dist);
  return(1);
} /* lfermetrous3dbin2_notable() */

/* ==================================== */
int32_t lfermetrous3dbin_table(struct xvimage *image, int32_t connex, int32_t tailletrous)
/* ==================================== */
#undef F_NAME
#define F_NAME "lfermetrous3dbin_table"
// version with tabulated connectivity numbers (only 26 connectivity for object)
{ 
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t z;                       /* index muet (generalement un voisin de y) */
  int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(image);       /* nombre plans */
  index_t N = ds * ps;             /* taille image */
  struct xvimage *dist;            /* pour la fonction distance */
  uint8_t *F = UCHARDATA(image);
  uint32_t *D;
  Fahp * FAHP;                   /* fahp pour le controle de l'ordre de traitement des points */
  int32_t xmin, xmax, ymin, ymax, zmin, zmax; /* le pave englobant */
  int32_t tbar;

#ifdef DEBUG
  fprintf(stderr, "%s: connex = %d, tailletrous = %d\n", F_NAME, connex, tailletrous);
#endif

  mctopo3d_table_init_topoCN3d();

  if (tailletrous == -1) tailletrous = 1000000000;

  FAHP = CreeFahpVide(N);
  if (FAHP == NULL)
  {   fprintf(stderr, "%s: CreeFahpVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* calcul fonction distance */

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;
  if (! lsedt_meijster(image, dist))
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return 0;
  }
  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;

  {
    double d;
    for (x = 0; x < N; x++) 
    {
      d = sqrt((double)(D[x]));
      D[x] = (uint32_t)arrondi(d);
    }
  }

  /* calcul du pave englobant */

  xmin = rs - 1; xmax = 0;
  ymin = cs - 1; ymax = 0;
  zmin = ds - 1; zmax = 0;
  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    if (F[z * ps + y * rs + x])
    {
      F[z * ps + y * rs + x] = VAL_X;      /* marque X au passage */
      if (x < xmin) xmin = x; else if (x > xmax) xmax = x;
      if (y < ymin) ymin = y; else if (y > ymax) ymax = y;
      if (z < zmin) zmin = z; else if (z > zmax) zmax = z;
    } /* if (F[z * ps + y * rs + x]) */
  } /* for z y x ... */

  if ((xmin == 0) || (ymin == 0) || (zmin == 0) || 
      (xmax == rs-1) || (ymax == cs-1) || (zmax == ds-1))
  {
    fprintf(stderr, "%s: there are object points on the border\n", F_NAME);
    return(0);
  }

  /* marquage de Y \ X */

  for (z = zmin; z <= zmax; z++)
  for (y = ymin; y <= ymax; y++)
  for (x = xmin; x <= xmax; x++)
  {
    if (!F[z * ps + y * rs + x]) 
      F[z * ps + y * rs + x] = VAL_Y_X;    
  } /* for z y x ... */
  if (connex == 26)
  {
    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      if (F[x] == VAL_Y_X)
      {
	tbar = mctopo3d_table_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
	if (((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))))
        {
	  assert(D[x] <= FAHP_NPRIO);
	  FahpPush(FAHP, x, mcmax(0,(FAHP_NPRIO-D[x])));
	  F[x] = VAL_Y_X_M;
	}
      }
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */


    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
      F[x] = VAL_Y_X;
      tbar = mctopo3d_table_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
      if ((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))) 
      {
        F[x] = VAL_NULLE;
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (F[y] == VAL_Y_X))
          {
	    assert(D[y] <= FAHP_NPRIO);
            FahpPush(FAHP, y, mcmax(0,(FAHP_NPRIO-D[y])));
            F[y] = VAL_Y_X_M;
          } /* if y */
        } /* for k */      
      } /* if (tbareqone26h(F, x, VAL_Y_X_M, rs, ps, N)) */
    } /* while (!FahpVide(FAHP)) */

  } /* if (connex == 26) */
  else
  {
    fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
    return 0;
  }

  for (x = 0; x < N; x++) if (F[x] != VAL_NULLE) F[x] = VAL_X;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_table_termine_topoCN3d();
  FahpTermine(FAHP);
  freeimage(dist);
  return(1);
} /* lfermetrous3dbin_table() */

/* ==================================== */
int32_t lfermetrous3dbin2_table(struct xvimage *image, struct xvimage *guide, int32_t connex, int32_t tailletrous)
/* ==================================== */
#undef F_NAME
#define F_NAME "lfermetrous3dbin2_table"
// same as lfermetrous3dbin_table except that the distance map is computed from image 'guide'
// version with tabulated connectivity numbers 
{ 
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t z;                       /* index muet (generalement un voisin de y) */
  int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(image);       /* nombre plans */
  index_t N = ds * ps;             /* taille image */
  struct xvimage *dist;        /* pour la fonction distance */
  uint8_t *F = UCHARDATA(image);
  uint32_t *D;
  Fahp * FAHP;                   /* fahp pour le controle de l'ordre de traitement des points */
  int32_t xmin, xmax, ymin, ymax, zmin, zmax; /* le pave englobant */
  int32_t tbar;

  mctopo3d_table_init_topoCN3d();

  if (tailletrous == -1) tailletrous = 1000000000;

  FAHP = CreeFahpVide(N);
  if (FAHP == NULL)
  {   fprintf(stderr, "%s: CreeFahpVide failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* calcul fonction distance */

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;
  if (! lsedt_meijster(guide, dist))
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return 0;
  }
  for (x = 0; x < N; x++) // inverse l'image
    if (F[x]) F[x] = 0; else F[x] = NDG_MAX;

  {
    double d;
    for (x = 0; x < N; x++) 
    {
      d = sqrt((double)(D[x]));
      D[x] = (uint32_t)arrondi(d);
    }
  }

  /* calcul du pave englobant */

  xmin = rs - 1; xmax = 0;
  ymin = cs - 1; ymax = 0;
  zmin = ds - 1; zmax = 0;
  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    if (F[z * ps + y * rs + x])
    {
      F[z * ps + y * rs + x] = VAL_X;      /* marque X au passage */
      if (x < xmin) xmin = x; else if (x > xmax) xmax = x;
      if (y < ymin) ymin = y; else if (y > ymax) ymax = y;
      if (z < zmin) zmin = z; else if (z > zmax) zmax = z;
    } /* if (F[z * ps + y * rs + x]) */
  } /* for z y x ... */

  if ((xmin == 0) || (ymin == 0) || (zmin == 0) || 
      (xmax == rs-1) || (ymax == cs-1) || (zmax == ds-1))
  {
    fprintf(stderr, "%s: there are object points on the border\n", F_NAME);
    return(0);
  }

  /* marquage de Y \ X */

  for (z = zmin; z <= zmax; z++)
  for (y = ymin; y <= ymax; y++)
  for (x = xmin; x <= xmax; x++)
  {
    if (!F[z * ps + y * rs + x]) 
      F[z * ps + y * rs + x] = VAL_Y_X;    
  } /* for z y x ... */

  if (connex == 26)
  {
    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      if (F[x] == VAL_Y_X)
      {
	tbar = mctopo3d_table_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
	if (((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))))
        {
	  assert(D[x] <= FAHP_NPRIO);
	  FahpPush(FAHP, x, mcmax(0,(FAHP_NPRIO-D[x])));
	  F[x] = VAL_Y_X_M;
	}
      }
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
      F[x] = VAL_Y_X;
      tbar = mctopo3d_table_tbar26h(F, x, VAL_Y_X_M, rs, ps, N); 
      if ((tbar == 1) || ((tbar > 1) && (D[x] > tailletrous))) 
      {
        F[x] = VAL_NULLE;
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (F[y] == VAL_Y_X))
          {
	    assert(D[y] <= FAHP_NPRIO);
            FahpPush(FAHP, y, mcmax(0,(FAHP_NPRIO-D[y])));
            F[y] = VAL_Y_X_M;
          } /* if y */
        } /* for k */      
      } /* if (tbareqone26h(F, x, VAL_Y_X_M, rs, ps, N)) */
#ifdef DEBUG_MARCIN
      else
      {
	printf("On garde le point %d,%d,%d\n", x % rs, (x % ps) / rs, x / ps);
      }
#endif
    } /* while (!FahpVide(FAHP)) */

  } /* if (connex == 26) */
  else
  {
    fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
    return 0;
  }

  for (x = 0; x < N; x++) if (F[x] != VAL_NULLE) F[x] = VAL_X;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_table_termine_topoCN3d();
  FahpTermine(FAHP);
  freeimage(dist);
  return(1);
} /* lfermetrous3dbin2_table() */


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GRAYSCALE CASE
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/* ==================================== */
int32_t lfermetrous3d_testabaisse6(uint8_t *F, uint8_t *P, index_t x, index_t rs, index_t ps, index_t N)
/* ==================================== */
{
  int32_t modifie = 0;

  while ((P[x] > F[x]) && (mctopo3d_t6mm(P, x, rs, ps, N) == 1))
  { 
    modifie = 1; 
    P[x] = mcmax(F[x],mctopo3d_alpha26m(P, x, rs, ps, N)); /* mctopo3d_alpha26m : sic */
  }

  return modifie;
} /* lfermetrous3d_testabaisse6() */

/* ==================================== */
int32_t lfermetrous3d(struct xvimage *image, int32_t connex, int32_t tailletrous)
/* ==================================== */
#undef F_NAME
#define F_NAME "lfermetrous3d"
{ 
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t z;                       /* index muet (generalement un voisin de y) */
  int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(image);       /* nombre plans */
  index_t N = ds * ps;             /* taille image */
  struct xvimage *p;               /* pour l'image englobante */
  uint8_t *P;
  struct xvimage *l;               /* pour l'image de labels */
  int32_t *L;
  uint8_t *F = UCHARDATA(image);
  Fahp * FAHP;                     /* fahp pour le controle de l'ordre de traitement des points */
  int32_t tbar;
  int32_t nminima;
  uint8_t *dejavu;

#ifdef DEBUG
  fprintf(stderr, "%s: connex = %d, tailletrous = %d\n", F_NAME, connex, tailletrous);
#endif

  if (tailletrous == -1) tailletrous = 1000000000;

  mctopo3d_init_topo3d();

  FAHP = CreeFahpVide(N);
  if (FAHP == NULL)
  {   fprintf(stderr, "%s: CreeFahpVide failed\n", F_NAME);
      return(0);
  }

  l = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (l == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  L = SLONGDATA(l);

  p = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if (p == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  P = UCHARDATA(p);

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  if (connex == 6)
  {
    if (!llabelextrema(image, connex, LABMIN, l, &nminima))
    {   
      fprintf(stderr, "%s: llabelextrema failed\n", F_NAME);
      return 0;
    }

    dejavu = (uint8_t *)calloc(nminima, sizeof(char));
    if (dejavu == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }

    /* met 1 voxel par cavite au niveau de la cavite */
    for (x = 0; x < N; x++) 
      if (L[x] && !dejavu[L[x]])
        { P[x] = F[x]; dejavu[L[x]] = 1; }
      else
        P[x] = NDG_MAX;

    /* met le cadre a 0 */
    for (x = 0; x < rs; x++)
    for (y = 0; y < cs; y++) 
      P[0 * ps + y * rs + x] = NDG_MIN;          /* plan z = 0 */
    for (x = 0; x < rs; x++)
    for (y = 0; y < cs; y++) 
      P[(ds-1) * ps + y * rs + x] = NDG_MIN;     /* plan z = ds-1 */

    for (x = 0; x < rs; x++)
    for (z = 0; z < ds; z++) 
      P[z * ps + 0 * rs + x] = NDG_MIN;          /* plan y = 0 */
    for (x = 0; x < rs; x++)
    for (z = 0; z < ds; z++) 
      P[z * ps + (cs-1) * rs + x] = NDG_MIN;     /* plan y = cs-1 */

    for (y = 0; y < cs; y++)
    for (z = 0; z < ds; z++) 
      P[z * ps + y * rs + 0] = NDG_MIN;          /* plan x = 0 */
    for (y = 0; y < cs; y++)
    for (z = 0; z < ds; z++) 
      P[z * ps + y * rs + (rs-1)] = NDG_MIN;     /* plan x = rs-1 */

    free(dejavu);
    freeimage(l);
    IndicsInit(N);

    /* ========================================================= */
    /*   INITIALISATION DE LA FAHP */
    /* ========================================================= */

    for (x = 0; x < N; x++)
    {
      tbar = mctopo3d_t6mm(P, x, rs, ps, N); 
      if ((P[x] > F[x]) && (tbar == 1))
      {
        FahpPush(FAHP, x, F[x]);
        Set(x,EN_FAHP);
      } /* if */
    } /* for (x = 0; x < N; x++) */

    /* ================================================ */
    /*                  DEBUT SATURATION                */
    /* ================================================ */

    while (!FahpVide(FAHP))
    {
      x = FahpPop(FAHP);
      UnSet(x,EN_FAHP);
      if (lfermetrous3d_testabaisse6(F, P, x, rs, ps, N))
      {
        for (k = 0; k < 26; k += 1)        /* parcourt les voisins en 26-connexite */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && !IsSet(y,EN_FAHP))
          {
            FahpPush(FAHP, y, F[y]);
            Set(y,EN_FAHP);
          } /* if y */
        } /* for k */      
      } /* if (testabaisse ... */
    } /* while (!FahpVide(FAHP)) */
  } /* if (connex == 6) */
  else
  {
    fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
    return 0;
  }

  memcpy(F, P, N);

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  FahpTermine(FAHP);
  freeimage(p);
  return(1);
} /* lfermetrous3d() */
