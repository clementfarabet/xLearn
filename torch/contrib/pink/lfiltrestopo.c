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
   Destruction des pics et des puits en 2d
   Michel Couprie - septembre 1996

   Destruction des pics et des puits en 3d
   Michel Couprie - janvier 2000

   Destructions des fils sombres et clairs en 3d ndg
   Michel Couprie - janvier 2000

   Filtre topologique alterne 2d (taf)
   Michel Couprie - octobre 2000 et decembre 2002

*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <mclifo.h>
#include <mcindic.h>
#include <lhtkern.h>
#include <llambdakern.h>
#include <lfiltrestopo.h>

#define EN_LIFO       0

#define VERBOSE

/* ==================================== */
void subimage(
  struct xvimage * image1,
  struct xvimage * image2)
/* difference de 2 images */
/* ==================================== */
{
  int32_t i;
  uint8_t *pt1, *pt2;
  int32_t rs, cs, d, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  d = depth(image1);
  N = rs * cs * d;

  for (pt1 = UCHARDATA(image1), pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
    *pt1 = *pt1 - *pt2;
} /* subimage() */

/* ==================================== */
int32_t ldespics(struct xvimage * image, struct xvimage * mask, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldespics"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  uint8_t *M;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != 1))
    {
      fprintf(stderr, "%s: tailles image et mask incompatibles\n", F_NAME);
      exit(0);
    }
    M = UCHARDATA(mask);
    if (connexmin == 8)
    {
      for (i = 0; i < N; i++)
        if (M[i] && (typetopo8(F, i, rs, N) == PEAK))
          F[i] = alpha4m(F, i, rs, N);
    }
    else
    {
      for (i = 0; i < N; i++)
        if (M[i] && (typetopo(F, i, rs, N) == PEAK))
          F[i] = alpha8m(F, i, rs, N);
    }
  }
  else
  {
    if (connexmin == 8)
    {
      for (i = 0; i < N; i++)
        if (typetopo8(F, i, rs, N) == PEAK)
          F[i] = alpha4m(F, i, rs, N);
    }
    else
    {
      for (i = 0; i < N; i++)
        if (typetopo(F, i, rs, N) == PEAK)
          F[i] = alpha8m(F, i, rs, N);
    }
  }
  return 1;
} /* ldespics() */

/* ==================================== */
int32_t ldespuits(struct xvimage * image, struct xvimage * mask, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldespuits"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  uint8_t *M;
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != 1))
    {
      fprintf(stderr, "%s: tailles image et mask incompatibles\n", F_NAME);
      exit(0);
    }
    M = UCHARDATA(mask);
    if (connexmin == 8)
    {
      for (i = 0; i < N; i++)
        if (M[i] && (typetopo8(F, i, rs, N) == WELL))
          F[i] = alpha8p(F, i, rs, N);
    }
    else
    {
      for (i = 0; i < N; i++)
        if (M[i] && (typetopo(F, i, rs, N) == WELL))
          F[i] = alpha4p(F, i, rs, N);
    }
  }
  else
  {
    if (connexmin == 8)
    {
      for (i = 0; i < N; i++)
        if (typetopo8(F, i, rs, N) == WELL)
          F[i] = alpha8p(F, i, rs, N);
    }
    else
    {
      for (i = 0; i < N; i++)
        if (typetopo(F, i, rs, N) == WELL)
          F[i] = alpha4p(F, i, rs, N);
    }
  }
  return 1;
} /* ldespuits() */

/* ==================================== */
int32_t ldespics3d(struct xvimage * image, struct xvimage * mask, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldespics3d"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  uint8_t *M;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */

  mctopo3d_init_topo3d();

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != ds))
    {
      fprintf(stderr, "%s: tailles image et mask incompatibles\n", F_NAME);
      return 0;
    }
    M = UCHARDATA(mask);
    if (connexmin == 6)
    {
      for (i = 0; i < N; i++)
        if (M[i] && (mctopo3d_peak6(F, i, rs, ps, N)))
          F[i] = mctopo3d_alpha26m(F, i, rs, ps, N);
    }
    else
    {
      for (i = 0; i < N; i++)
        if (M[i] && (mctopo3d_peak26(F, i, rs, ps, N)))
          F[i] = mctopo3d_alpha6m(F, i, rs, ps, N);
    }
  }
  else
  {
    if (connexmin == 6)
    {
      for (i = 0; i < N; i++)
        if (mctopo3d_peak6(F, i, rs, ps, N))
          F[i] = mctopo3d_alpha26m(F, i, rs, ps, N);      
    }
    else
    {
      for (i = 0; i < N; i++)
        if (mctopo3d_peak26(F, i, rs, ps, N))
          F[i] = mctopo3d_alpha6m(F, i, rs, ps, N);      
    }
  }

  mctopo3d_termine_topo3d();
  return 1;
} /* ldespics3d() */

/* ==================================== */
int32_t ldespuits3d(struct xvimage * image, struct xvimage * mask, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldespuits3d"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  uint8_t *M;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */

  mctopo3d_init_topo3d();

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != ds))
    {
      fprintf(stderr, "%s: tailles image et mask incompatibles\n", F_NAME);
      return 0;
    }
    M = UCHARDATA(mask);
    if (connexmin == 6)
    {
      for (i = 0; i < N; i++)
        if (M[i] && (mctopo3d_well6(F, i, rs, ps, N)))
          F[i] = mctopo3d_alpha6p(F, i, rs, ps, N);
    }
    else
    {
      for (i = 0; i < N; i++)
        if (M[i] && (mctopo3d_well26(F, i, rs, ps, N)))
          F[i] = mctopo3d_alpha26p(F, i, rs, ps, N);
    }
  }
  else
  {
    if (connexmin == 6)
    {
      for (i = 0; i < N; i++)
        if (mctopo3d_well6(F, i, rs, ps, N))
          F[i] = mctopo3d_alpha6p(F, i, rs, ps, N);      
    }
    else
    {
      for (i = 0; i < N; i++)
        if (mctopo3d_well26(F, i, rs, ps, N))
          F[i] = mctopo3d_alpha26p(F, i, rs, ps, N);      
    }
  }

  mctopo3d_termine_topo3d();
  return 1;
} /* ldespics3d() */

/* ==================================== */
int32_t ldesfilssombres3d(struct xvimage * image, struct xvimage * mask, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldesfilssombres3d"
{
  int32_t x;
  uint8_t *F = UCHARDATA(image);
  uint8_t *M;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */

  mctopo3d_init_topo3d();

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != ds))
    {
      fprintf(stderr, "%s: tailles image et mask incompatibles\n", F_NAME);
      return 0;
    }
    M = UCHARDATA(mask);
    if (connexmin == 6)
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (M[x] && (mctopo3d_filsombre6(F, x, rs, ps, N)))
          F[x] = mctopo3d_alpha6p(F, x, rs, ps, N);
    }
    else
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (M[x] && (mctopo3d_filsombre26(F, x, rs, ps, N)))
          F[x] = mctopo3d_alpha26p(F, x, rs, ps, N);
    }
  }
  else
  {
    if (connexmin == 6)
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (mctopo3d_filsombre6(F, x, rs, ps, N))
          F[x] = mctopo3d_alpha6p(F, x, rs, ps, N);
    }
    else
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (mctopo3d_filsombre26(F, x, rs, ps, N))
          F[x] = mctopo3d_alpha26p(F, x, rs, ps, N);
    }
  }

  mctopo3d_termine_topo3d();
  return 1;
} /* ldesfilssombres3d() */

/* ==================================== */
int32_t ldesfilsclairs3d(struct xvimage * image, struct xvimage * mask, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldesfilsclairs3d"
{
  int32_t x;
  uint8_t *F = UCHARDATA(image);
  uint8_t *M;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);       /* nombre plans */
  int32_t N = ds * ps;             /* taille image */

  mctopo3d_init_topo3d();

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  
  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != ds))
    {
      fprintf(stderr, "%s: tailles image et mask incompatibles\n", F_NAME);
      return 0;
    }
    M = UCHARDATA(mask);
    if (connexmin == 6)
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (M[x] && (mctopo3d_filclair6(F, x, rs, ps, N)))
          F[x] = mctopo3d_alpha26m(F, x, rs, ps, N);
    }
    else
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (M[x] && (mctopo3d_filclair26(F, x, rs, ps, N)))
          F[x] = mctopo3d_alpha6m(F, x, rs, ps, N);
    }
  }
  else
  {
    if (connexmin == 6)
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (mctopo3d_filclair6(F, x, rs, ps, N))
          F[x] = mctopo3d_alpha26m(F, x, rs, ps, N);
    }
    else
    {
      for (x = 0; x < N; x++) /* init : empile les points candidats */
        if (mctopo3d_filclair26(F, x, rs, ps, N))
          F[x] = mctopo3d_alpha6m(F, x, rs, ps, N);
    }
  }

  mctopo3d_termine_topo3d();
  return 1;
} /* ldesfilsclairs3d() */

/* ==================================== */
int32_t p_despics(struct xvimage * image, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "p_despics"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  int32_t ndes = 0;

  if (connexmin == 8)
  {
    for (i = 0; i < N; i++)
      if (typetopo8(F, i, rs, N) == PEAK)
        { F[i] = alpha4m(F, i, rs, N); ndes++; }
  }
  else
  {
    for (i = 0; i < N; i++)
      if (typetopo(F, i, rs, N) == PEAK)
        { F[i] = alpha8m(F, i, rs, N); ndes++; }
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: ndes = %d\n", F_NAME, ndes);      
#endif

  return ndes;
} /* p_despics() */

/* ==================================== */
int32_t p_despuits(struct xvimage * image, int32_t connexmin)
/* ==================================== */
#undef F_NAME
#define F_NAME "p_despuits"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  int32_t ndes = 0;

  if (connexmin == 8)
  {
    for (i = 0; i < N; i++)
      if (typetopo8(F, i, rs, N) == WELL)
        { F[i] = alpha8p(F, i, rs, N); ndes++; }
  }
  else
  {
    for (i = 0; i < N; i++)
      if (typetopo(F, i, rs, N) == WELL)
        { F[i] = alpha4p(F, i, rs, N); ndes++; }
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: ndes = %d\n", F_NAME, ndes);      
#endif

  return ndes;
} /* ldespuits() */

/* ==================================== */
int32_t p_deslambdapics(struct xvimage * image, int32_t connexmin, int32_t lambda)
/* ==================================== */
#undef F_NAME
#define F_NAME "p_deslambdapics"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  int32_t ndes = 0;

  if (connexmin == 8)
  {
    for (i = 0; i < N; i++)
      if (typetopo8(F, i, rs, N) == PEAK)
      {
        int32_t a =  alpha4m(F, i, rs, N);
        if ((F[i] - a) <= lambda) { F[i] = a;  ndes++; }
      }
  }
  else
  {
    for (i = 0; i < N; i++)
      if (typetopo(F, i, rs, N) == PEAK)
      {
        int32_t a =  alpha8m(F, i, rs, N);
        if ((F[i] - a) <= lambda) { F[i] = a;  ndes++; }
      }
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: ndes = %d\n", F_NAME, ndes);      
#endif

  return ndes;
} /* p_deslambdapics() */

/* ==================================== */
int32_t p_deslambdapuits(struct xvimage * image, int32_t connexmin, int32_t lambda)
/* ==================================== */
#undef F_NAME
#define F_NAME "p_deslambdapuits"
{
  int32_t i;
  uint8_t *F = UCHARDATA(image);
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  int32_t ndes = 0;

  if (connexmin == 8)
  {
    for (i = 0; i < N; i++)
      if (typetopo8(F, i, rs, N) == WELL)
      {
        int32_t a =  alpha8p(F, i, rs, N);
        if ((a - F[i]) <= lambda) { F[i] = a;  ndes++; }
      }
  }
  else
  {
    for (i = 0; i < N; i++)
      if (typetopo(F, i, rs, N) == WELL)
      {
        int32_t a =  alpha4p(F, i, rs, N);
        if ((a - F[i]) <= lambda) { F[i] = a;  ndes++; }
      }
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: ndes = %d\n", F_NAME, ndes);      
#endif

  return ndes;
} /* ldeslambdapuits() */

/* =============================================================== */
int32_t latf_old(struct xvimage * image, int32_t connexmin, int32_t rayonmin, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "latf"
{
  struct xvimage * copy;
  int32_t rayon;
  int32_t rs, cs, ds, ps, N;
  int32_t ndes;

  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);       /* nombre plans */
  N = ds * ps;             /* taille image */

  if (ds == 1) /* ======================= 2D ========================*/
  {
    if (rayonmin == 0) 
    {
      p_despics(image, connexmin);
      p_despuits(image, connexmin);
      rayonmin++;
    }
    copy = copyimage(image);
    if (copy == NULL)
    {
      fprintf(stderr, "%s: copyimage failed\n", F_NAME);
      return 0;
    }
    for (rayon = rayonmin; rayon <= rayonmax; rayon++)
    {
#ifdef VERBOSE
      fprintf(stderr, "%s: rayon = %d\n", F_NAME, rayon);      
#endif
      do
      {
        if (! lhthindelta(image, NULL, rayon, connexmin))
        {
          fprintf(stderr, "%s: lhthindelta failed\n", F_NAME);
          return 0;
        }
        ndes = p_despics(image, connexmin);
        if (! lhtkernu(image, copy, connexmin))
        {
          fprintf(stderr, "%s: lhtkernu failed\n", F_NAME);
          return 0;
        }
      } while (ndes);
      memcpy(UCHARDATA(copy), UCHARDATA(image), N);
      do
      {
        if (! lhthickdelta(image, NULL, rayon, connexmin))
        {
          fprintf(stderr, "%s: lhthickdelta failed\n", F_NAME);
          return 0;
        }
        ndes = p_despuits(image, connexmin);
        if (! lhtkern(image, copy, connexmin))
        {
          fprintf(stderr, "%s: lhtkern failed\n", F_NAME);
         return 0;
        }
      } while (ndes);
      if (rayon < rayonmax) memcpy(UCHARDATA(copy), UCHARDATA(image), N);
    } /* for (rayon = 1; rayon <= rayonmax; rayon++) */
  }
  else /* ============================== 3D ================================*/
  {
    fprintf(stderr, "%s: 3D Not Yet Implemented\n", F_NAME);
    return 0;
  }

  freeimage(copy);
  return 1;
} /* latf_old() */

#ifdef PURELY_AUTODUAL

/* =============================================================== */
int32_t ltaflambda(struct xvimage * image, int32_t connexmin, int32_t rayon, int32_t lambdapics, int32_t lambdapuits)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ltaflambda"
{
  struct xvimage * copy;
  struct xvimage * save;
  int32_t rs, cs, ds, ps, N;
  int32_t ndes;

  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);       /* nombre plans */
  N = ds * ps;             /* taille image */

  if (rayon == -1) rayon = 2000000000;   

  if (ds == 1) /* ======================= 2D ========================*/
  {
    copy = copyimage(image);
    save = copyimage(image);
    if ((copy == NULL) || (save == NULL))
    {
      fprintf(stderr, "%s: copyimage failed\n", F_NAME);
      return 0;
    }

    do
    {
      if (! lhthindelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthindelta failed\n", F_NAME);
        return 0;
      }
      ndes = p_deslambdapics(image, connexmin, lambdapics);
      if (! lhtkernu(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkernu failed\n", F_NAME);
        return 0;
      }
      subimage(copy, image);
      memcpy(UCHARDATA(image), UCHARDATA(save), N);

      if (! lhthickdelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthickdelta failed\n", F_NAME);
        return 0;
      }
      ndes += p_deslambdapuits(image, connexmin, lambdapuits);
      if (! lhtkern(image, save, connexmin))
      {
        fprintf(stderr, "%s: lhtkern failed\n", F_NAME);
        return 0;
      }
      subimage(image, copy);
      if (ndes) 
      {
        memcpy(UCHARDATA(copy), UCHARDATA(image), N);
        memcpy(UCHARDATA(save), UCHARDATA(image), N);
      }
    } while (ndes);
  }
  else /* ============================== 3D ================================*/
  {
    fprintf(stderr, "%s: 3D Not Yet Implemented\n", F_NAME);
    return 0;
  }

  freeimage(copy);
  freeimage(save);
  return 1;
} /* ltaflambda() */

#else 

/* =============================================================== */
int32_t ltaflambda(struct xvimage * image, int32_t connexmin, int32_t rayon, int32_t lambdapics, int32_t lambdapuits)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ltaflambda"
{
  struct xvimage * copy;
  int32_t rs, cs, ds, ps, N;
  int32_t ndes;

  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);       /* nombre plans */
  N = ds * ps;             /* taille image */

  if (rayon == -1) rayon = 2000000000;   

  if (ds == 1) /* ======================= 2D ========================*/
  {
    copy = copyimage(image);
    if (copy == NULL)
    {
      fprintf(stderr, "%s: copyimage failed\n", F_NAME);
      return 0;
    }

    do
    {
      if (! lhthindelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthindelta failed\n", F_NAME);
        return 0;
      }
      ndes = p_deslambdapics(image, connexmin, lambdapics);
      if (! lhtkernu(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkernu failed\n", F_NAME);
        return 0;
      }
      memcpy(UCHARDATA(copy), UCHARDATA(image), N);

      if (! lhthickdelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthickdelta failed\n", F_NAME);
        return 0;
      }
      ndes += p_deslambdapuits(image, connexmin, lambdapuits);
      if (! lhtkern(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkern failed\n", F_NAME);
        return 0;
      }
      if (ndes) memcpy(UCHARDATA(copy), UCHARDATA(image), N);
    } while (ndes);
  }
  else /* ============================== 3D ================================*/
  {
    fprintf(stderr, "%s: 3D Not Yet Implemented\n", F_NAME);
    return 0;
  }

  freeimage(copy);
  return 1;
} /* ltaflambda() */

#endif

/* =============================================================== */
int32_t ltaf(struct xvimage * image, int32_t connexmin, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ltaf"
{
  struct xvimage * copy;
  int32_t rs, cs, ds, ps, N;
  int32_t ndes;

  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);       /* nombre plans */
  N = ds * ps;             /* taille image */

  if (rayon == -1) rayon = 2000000000;   

  if (ds == 1) /* ======================= 2D ========================*/
  {
    copy = copyimage(image);
    if (copy == NULL)
    {
      fprintf(stderr, "%s: copyimage failed\n", F_NAME);
      return 0;
    }

    do
    {
      if (! lhthindelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthindelta failed\n", F_NAME);
        return 0;
      }
      ndes = p_despics(image, connexmin);
      if (! lhtkernu(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkernu failed\n", F_NAME);
        return 0;
      }
      memcpy(UCHARDATA(copy), UCHARDATA(image), N);

      if (! lhthickdelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthickdelta failed\n", F_NAME);
        return 0;
      }
      ndes += p_despuits(image, connexmin);
      if (! lhtkern(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkern failed\n", F_NAME);
        return 0;
      }
      if (ndes) memcpy(UCHARDATA(copy), UCHARDATA(image), N);
    } while (ndes);
  }
  else /* ============================== 3D ================================*/
  {
    fprintf(stderr, "%s: 3D Not Yet Implemented\n", F_NAME);
    return 0;
  }

  freeimage(copy);
  return 1;
} /* ltaf() */

/* =============================================================== */
int32_t ltlf(struct xvimage * image, int32_t connexmin, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ltlf"
{
  struct xvimage * copy;
  int32_t rs, cs, ds, ps, N;
  int32_t ndes;

  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);       /* nombre plans */
  N = ds * ps;             /* taille image */

  if (ds == 1) /* ======================= 2D ========================*/
  {
    copy = copyimage(image);
    if (copy == NULL)
    {
      fprintf(stderr, "%s: copyimage failed\n", F_NAME);
      return 0;
    }

    do
    {
      if (! lhthickdelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthickdelta failed\n", F_NAME);
        return 0;
      }
      ndes = p_despuits(image, connexmin);
      if (! lhtkern(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkern failed\n", F_NAME);
        return 0;
      }
      if (ndes) memcpy(UCHARDATA(copy), UCHARDATA(image), N);
    } while (ndes);
  }
  else /* ============================== 3D ================================*/
  {
    fprintf(stderr, "%s: 3D Not Yet Implemented\n", F_NAME);
    return 0;
  }

  freeimage(copy);
  return 1;
} /* ltlf() */

/* =============================================================== */
int32_t ltuf(struct xvimage * image, int32_t connexmin, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ltuf"
{
  struct xvimage * copy;
  int32_t rs, cs, ds, ps, N;
  int32_t ndes;

  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);       /* nombre plans */
  N = ds * ps;             /* taille image */

  if (ds == 1) /* ======================= 2D ========================*/
  {
    copy = copyimage(image);
    if (copy == NULL)
    {
      fprintf(stderr, "%s: copyimage failed\n", F_NAME);
      return 0;
    }

    do
    {
      if (! lhthindelta(image, NULL, rayon, connexmin))
      {
        fprintf(stderr, "%s: lhthindelta failed\n", F_NAME);
        return 0;
      }
      ndes = p_despics(image, connexmin);
      if (! lhtkernu(image, copy, connexmin))
      {
        fprintf(stderr, "%s: lhtkernu failed\n", F_NAME);
        return 0;
      }
      if (ndes) memcpy(UCHARDATA(copy), UCHARDATA(image), N);
    } while (ndes);
  }
  else /* ============================== 3D ================================*/
  {
    fprintf(stderr, "%s: 3D Not Yet Implemented\n", F_NAME);
    return 0;
  }

  freeimage(copy);
  return 1;
} /* ltuf() */
