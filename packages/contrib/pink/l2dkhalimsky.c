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
/* Operateurs agissant dans la grille de Khalimsky 2d */
/* Michel Couprie - mars 2000 

   l2dseltype: extraction d'éléments selon leur rang et leur type topologique
     Michel Couprie - avril 2007

   l2dborder: extraction de la frontière interne
   l2dboundary: extraction de la frontière interne
     Michel Couprie - août 2009
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcindic.h>
#include <mcutil.h>
#include <mckhalimsky2d.h>
#include <l2dkhalimsky.h>

#define PARANO

/*
#define STEP
*/
//#define VERBOSE

/* =============================================================== */
int32_t l2dmakecomplex(struct xvimage * i)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dmakecomplex"
/* 
   effectue la fermeture par inclusion de l'ensemble i
*/
{
  assert(datatype(i) == VFF_TYP_1_BYTE);

  if (depth(i) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  AjouteAlphacarre2d(i);

  return 1;

} /* l2dmakecomplex() */

/* =============================================================== */
int32_t l2dclosebeta(struct xvimage * i)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dclosebeta"
/* 
   effectue la fermeture par inclusion inverse de l'ensemble i
*/
{
  assert(datatype(i) == VFF_TYP_1_BYTE);

  if (depth(i) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  AjouteBetacarre2d(i);

  return 1;

} /* l2dclosebeta() */

/* =============================================================== */
int32_t l2dkhalimskize(struct xvimage * i, struct xvimage **k, int32_t mode)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dkhalimskize"
/* 
   passage de z2 a la grille de Khalimsky
   mode = 
     0 : elements maximaux seulement,
     4 : emulation de la 4-connexite, idem "miss", 
     8 : emulation de la 8-connexite, 
     10 : "hit", 
     1 : min ndg, 
     2 : max ndg,
     5 : gradient ndg 2, 
     6 : gradient ndg 4,
     7 : moyenne
     9 : reverse (Khalimsky -> Z2) : selection carres
*/
{
  if (depth(i) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  if ((mode == 0) || (mode == 1) || (mode == 2) || (mode == 5) || (mode == 6) || (mode == 7))
    *k = KhalimskizeNDG2d(i);
  else if (mode == 9)
    *k = DeKhalimskize2d(i);
  else
    *k = Khalimskize2d(i);
  if (*k == NULL)
  {
    fprintf(stderr, "%s: Khalimskize2d failed\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s terminee\n", F_NAME);
#endif

  if (mode == 4)
    Connex4Obj2d(*k);
  else if (mode == 8)
    Connex8Obj2d(*k);
  else if (mode == 10)
    SatureAlphacarre2d(*k);
  else if (mode == 1)
    ndgmin2d(*k);
  else if (mode == 2)
    ndgmax2d(*k);
  else if (mode == 5)
  {
    ndg4grad2d(*k);
    ndgminbeta2d(*k);
  }
  else if (mode == 6)
  {
    ndg4grad2d(*k);
    ndgmaxbeta2d(*k);
  }
  else if (mode == 7)
    ndgmoy2d(*k);

#ifdef VERBOSE
  fprintf(stderr, "%s terminee\n", F_NAME);
#endif

  return 1;

} /* l2dkhalimskize() */

/* =============================================================== */
int32_t l2dkhalimskize_noalloc(struct xvimage * i, struct xvimage *k, int32_t mode)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dkhalimskize_noalloc"
/* 
   passage de z2 a la grille de Khalimsky
   sans allocation de mémoire
   mode = 
     4 : emulation de la 4-connexite, idem "miss", 
     8 : emulation de la 8-connexite, 
     10 : "hit", 
     1 : min ndg, 
     2 : max ndg,
     5 : gradient ndg 2, 
     6 : gradient ndg 4,
     7 : moyenne
     9 : reverse (Khalimsky -> Z2) : selection carres

     \warning pas de vérification de taille
*/
{
  if (depth(i) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  if ((mode == 1) || (mode == 2) || (mode == 5) || (mode == 6) || (mode == 7))
    KhalimskizeNDG2d_noalloc(i, k);
  else if (mode == 9)
    DeKhalimskize2d_noalloc(i, k);
  else
    Khalimskize2d_noalloc(i, k);

#ifdef VERBOSE
  fprintf(stderr, "%s terminee\n", F_NAME);
#endif

  if (mode == 4)
    Connex4Obj2d(k);
  else if (mode == 8)
    Connex8Obj2d(k);
  else if (mode == 10)
    SatureAlphacarre2d(k);
  else if (mode == 1)
    ndgmin2d(k);
  else if (mode == 2)
    ndgmax2d(k);
  else if (mode == 5)
  {
    ndg4grad2d(k);
    ndgminbeta2d(k);
  }
  else if (mode == 6)
  {
    ndg4grad2d(k);
    ndgmaxbeta2d(k);
  }
  else if (mode == 7)
    ndgmoy2d(k);

#ifdef VERBOSE
  fprintf(stderr, "%s terminee\n", F_NAME);
#endif

  return 1;

} /* l2dkhalimskize_noalloc() */

/* =============================================================== */
int32_t l2dcolor(struct xvimage * k)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dcolor"
/* 
   coloriage des elements de k selon leur type 
*/
{
  if (depth(k) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  ColorieKh2d(k);
  return 1;
} /* l2dcolor() */

/* =============================================================== */
int32_t l2dthin(struct xvimage * k, int32_t nsteps)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dthin"
/* 
   amincissement homotopique 2d dans la grille de Khalimsky 
*/
{
  struct xvimage * kp;
  int32_t stablealpha, stablebeta;
  int32_t i;
  index_t rs, cs, N, x, y;
  uint8_t * K;
  uint8_t * KP;

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  if (depth(k) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  InitPileGrilles2d();

  kp = copyimage(k);
  if (kp == NULL)
  {   fprintf(stderr,"%s() : copyimage failed\n", F_NAME);
      return 0;
  }  
  KP = UCHARDATA(kp);

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 1; i <= nsteps; i++)
  {
#ifdef VERBOSE
    fprintf(stderr, "step %d\n", i);
#endif
    if (i % 2)
    {
      stablealpha = 1;
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (K[y * rs + x] && Alpha2Simple2d(k, x, y))
          { KP[y * rs + x] = 0; stablealpha = 0; }
      memcpy(K, KP, N);
    }
    else
    {
      stablebeta = 1;
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (K[y * rs + x] && Beta2Simple2d(k, x, y))
          { KP[y * rs + x] = 0; stablebeta = 0; }
      memcpy(K, KP, N);
    }
    if (stablealpha && stablebeta) break;
  }

  TerminePileGrilles2d();
  freeimage(kp);

  return 1;

} /* l2dthin() */

/* =============================================================== */
int32_t l2dlabel(struct xvimage * f, struct xvimage * lab, index_t *nlabels)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dlabel"
/*
  Etiquette les regions theta-connexes de l'ordre (non value') f. 
  Resultat dans lab (image "longint", allouee a l'avance)
*/
{
  index_t rs, cs, N;
  int32_t x, y, w;
  uint8_t *F;
  int32_t *LAB;
  Lifo * LIFO;
  index_t tab[27]; 
  int32_t n, k;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  *nlabels = 0;

  rs = rowsize(f);
  cs = colsize(f);
  N = rs * cs;
  F = UCHARDATA(f);
  LAB = SLONGDATA(lab);
  memset(LAB, 0, N*sizeof(int32_t));

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }
  
  for (x = 0; x < N; x++)
  {
    if (F[x] && !LAB[x]) 
    {
      *nlabels += 1;
      LAB[x] = *nlabels;
      LifoPush(LIFO, x);
      /* propage le label dans la region connexe */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = *nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
        Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = *nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
    } /* if (F[x] && !LAB[x]) */
  } /* for (x = 0; x < N; x++) */

#ifdef VERBOSE
  fprintf(stderr, "%s : %d composantes trouvees\n", *nlabels, F_NAME);
#endif

  LifoTermine(LIFO);
  return 1;
} /* l2dlabel() */

/* =============================================================== */
int32_t l2dlabelextrema(
      struct xvimage * f,   /* ordre value' original */
      int32_t minimum,          /* booleen */
      struct xvimage * lab, /* resultat: image de labels */
      index_t *nlabels)         /* resultat: nombre d'extrema traites + 1 (0 = non extremum) */
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dlabelextrema"
/*
  Etiquette les regions theta-connexes extremales de l'ordre value' f. 
  Resultat dans lab (image "longint", allouee a l'avance)
*/
{
  index_t rs, cs, N;
  int32_t x, y, w;
  uint8_t *F;
  int32_t *LAB, label;
  Lifo * LIFO;
  index_t tab[27]; 
  int32_t n, k;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  *nlabels = 0;

  rs = rowsize(f);
  cs = colsize(f);
  N = rs * cs;
  F = UCHARDATA(f);
  LAB = SLONGDATA(lab);

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }
  
  /* le LABEL initialement est mis a -1 */
  for (x = 0; x < N; x++) LAB[x] = -1;

  *nlabels = 0;

  for (x = 0; x < N; x++)
  {
    if (LAB[x] == -1)            /* on trouve un point x non etiquete */
    {
      *nlabels += 1;             /* on cree un numero d'etiquette */
      LAB[x] = *nlabels;
      LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        label = LAB[w];
        Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
        {
          y = tab[k];
          if ((label > 0) && 
              ( (minimum && (F[y] < F[w])) ||
                (!minimum && (F[y] > F[w]))
              )
             )
          {   /* w non dans un minimum (resp. maximum) */
            label = 0;
            *nlabels -= 1;
            LAB[w] = label;
            LifoPush(LIFO, w);
          } 
          else
          if (F[y] == F[w])
          {
            if (((label > 0) && (LAB[y] == -1)) ||
                ((label == 0) && (LAB[y] != 0)))
            {
              LAB[y] = label;
              LifoPush(LIFO, y);
            } /* if .. */
          } /* if F ... */
        } /* for k ... */
        Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
        {
          y = tab[k];
          if ((label > 0) && 
              ( (minimum && (F[y] < F[w])) ||
                (!minimum && (F[y] > F[w]))
              )
             )
          {   /* w non dans un minimum (resp. maximum) */
            label = 0;
            *nlabels -= 1;
            LAB[w] = label;
            LifoPush(LIFO, w);
          } 
          else
          if (F[y] == F[w])
          {
            if (((label > 0) && (LAB[y] == -1)) ||
                ((label == 0) && (LAB[y] != 0)))
            {
              LAB[y] = label;
              LifoPush(LIFO, y);
            } /* if .. */
          } /* if F ... */
        } /* for k ... */

      } /* while (! LifoVide(LIFO)) */
    } /* if (LAB[x] ... */
  } /* for (x = 0; x < N; x++) */

#ifdef VERBOSE
  fprintf(stderr, "2dlabels : %d composantes trouvees\n", *nlabels);
#endif

  LifoTermine(LIFO);

  *nlabels += 1; /* pour le niveau 0 */
  return 1;
} /* l2dlabelextrema() */

/* =============================================================== */
static int32_t simple(index_t w, int32_t *LAB, index_t rs, index_t cs)
/* =============================================================== */
#undef F_NAME
#define F_NAME "simple"
/* si le point w est theta-voisin d'une seule composante,
   retourne la valeur de cette composante
   sinon retourne 0
*/
{
  index_t tab[27]; 
  int32_t n, k, y;
  int32_t val = 0;

  Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
  for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
  {
    y = tab[k];
    if (LAB[y])
    {
      if ((val == 0) || (LAB[y] == val)) 
        val = LAB[y];
      else 
        return 0;
    } /* if (LAB[y]) */
  } /* for k ... */
  Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
  for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
  {
    y = tab[k];
    if (LAB[y])
    {
      if ((val == 0) || (LAB[y] == val)) 
        val = LAB[y];
      else 
        return 0;
    } /* if (LAB[y]) */
  } /* for k ... */

  return val;

} /* simple() */

/* =============================================================== */
int32_t l2dtopotess(struct xvimage * lab, struct xvimage * mask)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dtopotess"
/*
  Entree: lab contient les etiquettes des "graines" (composantes connexes)
  Entree: mask definit une geodesie (null si tout l'espace)
  Sortie: lab contient les regions de Topotess associees aux graines initiales

  Attention : on suppose que lab est nulle en dehors de mask
*/
{
  index_t rs, cs, N;
  int32_t x, y, w;
  uint8_t *MASK;
  int32_t *LAB;
  Lifo * LIFO;
  Lifo * LIFOAUX;
  Lifo * LIFOVAL;
  index_t tab[27]; 
  int32_t n, k;
  int32_t val;

  if (depth(lab) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (datatype(lab) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: bad image type for lab\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(lab);
  cs = colsize(lab);
  N = rs * cs;
  LAB = SLONGDATA(lab);

  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != 1))
    {
      fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
      return 0;
    }

    if (datatype(mask) != VFF_TYP_1_BYTE)
    {
      fprintf(stderr, "%s: bad image type for mask\n", F_NAME);
      return 0;
    }

    MASK = UCHARDATA(mask);
  }

  Indics1bitInit(N);

  LIFO = CreeLifoVide(N);
  LIFOAUX = CreeLifoVide(N);
  LIFOVAL = CreeLifoVide(N);
  if ((LIFO == NULL) || (LIFOAUX == NULL) || (LIFOVAL == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

if (mask == NULL)
{

  /* initialisation des LIFO: on empile tous les points simples du complementaire */
  /* ainsi que la valeur de la composante voisine */
  
  for (x = 0; x < N; x++)
  {
    if (!LAB[x] && (val = simple(x, LAB, rs, cs))) 
    {
      LifoPush(LIFO, x);
      LifoPush(LIFOVAL, val);
    }
  }

  /* iteration : traite les points simples et empile les nouveaux points simples du voisinage */

  while (! LifoVide(LIFO))
  {

    while (! LifoVide(LIFO)) /* traite les points simples */
    {
      w = LifoPop(LIFO);
      val = LifoPop(LIFOVAL);
      UnSet1(w);
      LAB[w] = val;
      LifoPush(LIFOAUX, w);
    }

    while (! LifoVide(LIFOAUX)) /* traite les voisins */
    {
      w = LifoPop(LIFOAUX);
      Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
      {
        y = tab[k];
        if (!LAB[y] && (! IsSet1(y)) && (val = simple(y, LAB, rs, cs))) 
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
      Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
      {
        y = tab[k];
        if (!LAB[y] && (! IsSet1(y)) && (val = simple(y, LAB, rs, cs))) 
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
    } /* while (! LifoVide(LIFOAUX)) */

  } /* while (! LifoVide(LIFO)) */

#ifdef PARANO  
  for (x = 0; x < N; x++)
    assert(LAB[x] || (val != simple(x, LAB, rs, cs)));
    // il reste des points simples
#endif

} /* if (mask == NULL) */
else
{

  /* initialisation des LIFO: on empile tous les points simples du complementaire */
  /* ainsi que la valeur de la composante voisine */
  
  for (x = 0; x < N; x++)
  {
    if (MASK[x] && (!LAB[x]) && (val = simple(x, LAB, rs, cs))) 
    {
      LifoPush(LIFO, x);
      LifoPush(LIFOVAL, val);
    }
  }

  /* iteration : traite les points simples et empile les nouveaux points simples du voisinage */

  while (! LifoVide(LIFO))
  {

    while (! LifoVide(LIFO)) /* traite les points simples */
    {
      w = LifoPop(LIFO);
      val = LifoPop(LIFOVAL);
      UnSet1(w);
      LAB[w] = val;
      LifoPush(LIFOAUX, w);
    }

    while (! LifoVide(LIFOAUX)) /* traite les voisins */
    {
      w = LifoPop(LIFOAUX);
      Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
      {
        y = tab[k];
        if (MASK[y] && (!LAB[y]) && (! IsSet1(y)) && (val = simple(y, LAB, rs, cs))) 
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
      Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
      {
        y = tab[k];
        if (MASK[y] && (!LAB[y]) && (! IsSet1(y)) && (val = simple(y, LAB, rs, cs))) 
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
    } /* while (! LifoVide(LIFOAUX)) */

  } /* while (! LifoVide(LIFO)) */

} /* else if (mask == NULL) */

  LifoTermine(LIFO);
  LifoTermine(LIFOAUX);
  LifoTermine(LIFOVAL);
  IndicsTermine();
  return 1;
} /* l2dtopotess() */

/* =============================================================== */
int32_t testlocal(index_t w, int32_t *LAB, index_t rs, index_t cs)
/* =============================================================== */
#undef F_NAME
#define F_NAME "testlocal"
/* si le point w est theta-voisin d'une seule etiquette != -1,
     retourne la valeur de cette etiquette
   si le point w est theta-voisin de plusieurs etiquettes != -1,
     retourne 0
   sinon retourne -1
*/
{
  index_t tab[27]; 
  int32_t n, k, y;
  int32_t val = -1;

  Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
  for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
  {
    y = tab[k];
    if (LAB[y] != -1)
    {
      if ((val == -1) || (LAB[y] == val)) 
        val = LAB[y];
      else 
        return 0;
    } /* if (LAB[y]) */
  } /* for k ... */
  Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
  for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
  {
    y = tab[k];
    if (LAB[y] != -1)
    {
      if ((val == -1) || (LAB[y] == val)) 
        val = LAB[y];
      else 
        return 0;
    } /* if (LAB[y]) */
  } /* for k ... */

  return val;

} /* testlocal() */

/* =============================================================== */
int32_t testlocalmask(index_t w, int32_t *LAB, uint8_t *MASK, index_t rs, index_t cs)
/* =============================================================== */
#undef F_NAME
#define F_NAME "testlocalmask"
/* si le point w est theta-voisin d'une seule etiquette != -1,
     retourne la valeur de cette etiquette
   si le point w est theta-voisin de plusieurs etiquettes != -1,
     retourne 0
   sinon retourne -1
*/
{
  index_t tab[27]; 
  int32_t n, k, y;
  int32_t val = -1;

  Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
  for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
  {
    y = tab[k];
    if (MASK[y] && (LAB[y] != -1))
    {
      if ((val == -1) || (LAB[y] == val)) 
        val = LAB[y];
      else 
        return 0;
    } /* if (LAB[y]) */
  } /* for k ... */
  Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
  for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
  {
    y = tab[k];
    if (MASK[y] && (LAB[y] != -1))
    {
      if ((val == -1) || (LAB[y] == val)) 
        val = LAB[y];
      else 
        return 0;
    } /* if (LAB[y]) */
  } /* for k ... */

  return val;

} /* testlocalmask() */

/* =============================================================== */
int32_t l2dvoronoi(struct xvimage * lab, struct xvimage * mask)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dvoronoi"
/*
  Entree: lab contient les etiquettes des "graines" (composantes connexes)
  Entree: mask definit une geodesie (null si tout l'espace)
  Sortie: lab contient les regions de Voronoi associees aux graines initiales

  Attention : on suppose que lab est nulle en dehors de mask
*/
{
  index_t rs, cs, N;
  int32_t x, y, w;
  uint8_t *MASK;
  int32_t *LAB;
  Lifo * LIFO;
  Lifo * LIFOAUX;
  Lifo * LIFOVAL;
  index_t tab[27]; 
  int32_t n, k;
  int32_t val;

  if (depth(lab) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (datatype(lab) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: bad image type for lab\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(lab);
  cs = colsize(lab);
  N = rs * cs;
  LAB = SLONGDATA(lab);

  if (mask)
  {
    if ((rowsize(mask) != rs) || (colsize(mask) != cs) || (depth(mask) != 1))
    {
      fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
      return 0;
    }

    if (datatype(mask) != VFF_TYP_1_BYTE)
    {
      fprintf(stderr, "%s: bad image type for mask\n", F_NAME);
      return 0;
    }

    MASK = UCHARDATA(mask);
  }

  Indics1bitInit(N);

  LIFO = CreeLifoVide(N);
  LIFOAUX = CreeLifoVide(N);
  LIFOVAL = CreeLifoVide(N);
  if ((LIFO == NULL) || (LIFOAUX == NULL) || (LIFOVAL == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++) if (LAB[x] == 0) LAB[x] = -1;

if (mask == NULL)
{

  /* initialisation des LIFO: on empile tous les points du complementaire */
  /* voisins d'au moins une etiquette != -1 ainsi que la nouvelle valeur */
  
  for (x = 0; x < N; x++)
  {
    if ((LAB[x] == -1) && ((val = testlocal(x, LAB, rs, cs)) != -1)) 
    {
      LifoPush(LIFO, x);
      LifoPush(LIFOVAL, val);
    }
  }

  /* iteration : traite les points empiles et empile les nouveaux points du voisinage */

  while (! LifoVide(LIFO))
  {

    while (! LifoVide(LIFO)) /* traite les points empiles */
    {
      w = LifoPop(LIFO);
      val = LifoPop(LIFOVAL);
      UnSet1(w);
      LAB[w] = val;
      LifoPush(LIFOAUX, w);
    }

    while (! LifoVide(LIFOAUX)) /* traite les voisins */
    {
      w = LifoPop(LIFOAUX);
      Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
      {
        y = tab[k];
        if ((LAB[y] == -1) && (! IsSet1(y)) && ((val = testlocal(y, LAB, rs, cs)) != -1))
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
      Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
      {
        y = tab[k];
        if ((LAB[y] == -1) && (! IsSet1(y)) && ((val = testlocal(y, LAB, rs, cs)) != -1))
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
    } /* while (! LifoVide(LIFOAUX)) */

  } /* while (! LifoVide(LIFO)) */

} /* if (mask == NULL) */
else
{ /* if (mask != NULL) */
  /* initialisation des LIFO: on empile tous les points du complementaire */
  /* voisins d'au moins une etiquette != -1 ainsi que la nouvelle valeur */
  
  for (x = 0; x < N; x++)
  {
    if (MASK[x] && (LAB[x] == -1) && ((val = testlocalmask(x, LAB, MASK, rs, cs)) != -1)) 
    {
      LifoPush(LIFO, x);
      LifoPush(LIFOVAL, val);
    }
  }

  /* iteration : traite les points empiles et empile les nouveaux points du voisinage */

  while (! LifoVide(LIFO))
  {

    while (! LifoVide(LIFO)) /* traite les points empiles */
    {
      w = LifoPop(LIFO);
      val = LifoPop(LIFOVAL);
      UnSet1(w);
      LAB[w] = val;
      LifoPush(LIFOAUX, w);
    }

#ifdef STEP
    { char buf[10]; printf("step\n"); writeimage(lab,"lab"); scanf("%s", buf);}
#endif

    while (! LifoVide(LIFOAUX)) /* traite les voisins */
    {
      w = LifoPop(LIFOAUX);
      Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
      {
        y = tab[k];
        if ( MASK[y] && (LAB[y] == -1) && (! IsSet1(y)) && 
             ((val = testlocalmask(y, LAB, MASK, rs, cs)) != -1)
           )
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
      Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
      for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
      {
        y = tab[k];
        if ( MASK[y] && (LAB[y] == -1) && (! IsSet1(y)) && 
             ((val = testlocalmask(y, LAB, MASK, rs, cs)) != -1)
           )
        {
          Set1(y);
          LifoPush(LIFO, y);
          LifoPush(LIFOVAL, val);
        } /* if (F[y] ... */
      } /* for k ... */
    } /* while (! LifoVide(LIFOAUX)) */

  } /* while (! LifoVide(LIFO)) */

  for (x = 0; x < N; x++) if (LAB[x] == -1) LAB[x] = 0;

} /* if (mask != NULL) */

  LifoTermine(LIFO);
  LifoTermine(LIFOAUX);
  LifoTermine(LIFOVAL);
  IndicsTermine();
  return 1;
} /* l2dvoronoi() */

/* =============================================================== */
int32_t l2dtopotessndg(struct xvimage * f)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dtopotessndg"
/*
  Implementation naive d'apres la definition
*/
{
  index_t rs, cs, N;
  index_t x;
  struct xvimage * fk;  /* seuil de F au niveau k */ 
  struct xvimage * fkp; /* seuil de F au niveau k+1 */
  struct xvimage * lab; /* pour les labels des CC de fkp */ 
  uint8_t *F;
  uint8_t *Fk;
  uint8_t *Fkp;
  int32_t *LAB;
  int32_t kp;
  index_t nlabels;
  index_t histo[NDG_MAX+1];

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (datatype(f) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  N = rs * cs;
  F = UCHARDATA(f);

  fk = copyimage(f);
  fkp = copyimage(f);
  Fk = UCHARDATA(fk);
  Fkp = UCHARDATA(fkp);
  memset(Fk, 255, N);  /* init Fk pour k = 0 */

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  LAB = SLONGDATA(lab);

  /* calcule histo */  
  memset(histo, 0, (NDG_MAX+1) * sizeof(index_t));
  for (x = 0; x < N; x++) histo[F[x]] += 1;

  /* monte au second niveau non vide */
  kp = 0; 
  while (!histo[kp]) kp++;
  kp++;

  for (; kp <= NDG_MAX; kp++) /* monte niveau par niveau */
  if (histo[kp])
  {

#ifdef VERBOSE
  fprintf(stderr, "  niveau %d\n", kp);
#endif

    /* seuil au niveau kp */
    for (x = 0; x < N; x++) Fkp[x] = ( (F[x]>=kp) ? 255 : 0 );

#ifdef STEP
    { char buf[10]; printf("kp=%d\n",kp); writeimage(f,"f"); writeimage(fk,"fk"); writeimage(fkp,"fkp"); scanf("%s", buf);}
#endif

    /* etiquette les composantes de fkp */
    if (! l2dlabel(fkp, lab, &nlabels))
    {
      fprintf(stderr, "%s: l2dlabel failed\n", F_NAME);
      return 0;
    }

    /* topotess binaire sur fkp geodesiquement a fk */
    if (! l2dtopotess(lab, fk))
    {
      fprintf(stderr, "%s: l2dvoronoi failed\n", F_NAME);
      return 0;
    }

#ifdef STEP
writeimage(lab, "lab");
#endif

    /* recupere dans fk les RV */
    for (x = 0; x < N; x++) if (LAB[x]) Fk[x] = 255; else Fk[x] = 0;

    /* remonte dans F les points de fk qui ont ete rajoutes */
    for (x = 0; x < N; x++) if (Fk[x] && !Fkp[x]) F[x] = kp;

  }

  freeimage(lab);
  freeimage(fk);
  freeimage(fkp);
  return 1;
} /* l2dtopotessndg() */

/* =============================================================== */
int32_t l2dtopotessndg_inverse(struct xvimage * f)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dtopotessndg_inverse"
/*
  Implementation naive d'apres la definition
*/
{
  index_t rs, cs, N;
  index_t x;
  struct xvimage * fk;  /* seuil de F au niveau k */ 
  struct xvimage * fkp; /* seuil de F au niveau k+1 */
  struct xvimage * lab; /* pour les labels des CC de fkp */ 
  uint8_t *F;
  uint8_t *Fk;
  uint8_t *Fkp;
  int32_t *LAB;
  int32_t k, kp;
  index_t nlabels;
  index_t histo[NDG_MAX+1];

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (datatype(f) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  N = rs * cs;
  F = UCHARDATA(f);

  fk = copyimage(f);
  fkp = copyimage(f);
  Fk = UCHARDATA(fk);
  Fkp = UCHARDATA(fkp);

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  LAB = SLONGDATA(lab);

  /* calcule histo */  
  memset(histo, 0, (NDG_MAX+1) * sizeof(index_t));
  for (x = 0; x < N; x++) histo[F[x]] += 1;

  /* cherche niveau max */
  k = NDG_MAX; 
  while (!histo[k]) k--;

  /* seuil au niveau max */
  for (x = 0; x < N; x++) Fkp[x] = ( (F[x]>=k) ? 255 : 0 );

  kp = k;
  for (k = k-1; k >= 0; k--) /* descend niveau par niveau */
  if (histo[k])
  {

#ifdef VERBOSE
  fprintf(stderr, "  niveau %d\n", k);
#endif

    /* seuil au niveau k */
    for (x = 0; x < N; x++) Fk[x] = ( (F[x]>=k) ? 255 : 0 );

#ifdef STEP
    { char buf[10]; printf("kp=%d\n",kp); writeimage(f,"f"); writeimage(fk,"fk"); writeimage(fkp,"fkp"); scanf("%s", buf);}
#endif

    /* etiquette les composantes de fkp */
    if (! l2dlabel(fkp, lab, &nlabels))
    {
      fprintf(stderr, "%s: l2dlabel failed\n", F_NAME);
      return 0;
    }

    /* topotess binaire sur fkp geodesiquement a fk */
    if (! l2dtopotess(lab, fk))
    {
      fprintf(stderr, "%s: l2dvoronoi failed\n", F_NAME);
      return 0;
    }

#ifdef STEP
writeimage(lab, "lab");
#endif

    /* recupere dans fk les RV */
    for (x = 0; x < N; x++) if (LAB[x]) Fk[x] = 255; else Fk[x] = 0;

    /* remonte dans F les points de fk qui ont ete rajoutes */
    for (x = 0; x < N; x++) if (Fk[x] && !Fkp[x]) F[x] = kp;

    /* nouveau fkp */
    for (x = 0; x < N; x++) Fkp[x] = ( (F[x]>=k) ? 255 : 0 );
    kp = k;
  }

  freeimage(lab);
  freeimage(fk);
  freeimage(fkp);
  return 1;
} /* l2dtopotessndg() */

/* =============================================================== */
int32_t l2dtopotessndgVS(struct xvimage * f)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l2dtopotessndgVS"
/*
  Implementation naive d'apres la definition de Vincent et Soille
*/
{
  index_t rs, cs, N;
  index_t x;
  struct xvimage * fk;  /* seuil de F au niveau k */ 
  struct xvimage * fkp; /* seuil de F au niveau k+1 */
  struct xvimage * lab; /* pour les labels des CC de fkp */ 
  struct xvimage * maxi; /* pour les maxima de F */ 
  uint8_t *F;
  uint8_t *Fk;
  uint8_t *Fkp;
  int32_t *LAB;
  int32_t *MAXI;
  int32_t kp;
  index_t nlabels;
  index_t histo[NDG_MAX+1];

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

  if (datatype(f) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  N = rs * cs;
  F = UCHARDATA(f);

  fk = copyimage(f);
  fkp = copyimage(f);
  Fk = UCHARDATA(fk);
  Fkp = UCHARDATA(fkp);

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  LAB = SLONGDATA(lab);

  maxi = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (maxi == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  MAXI = SLONGDATA(maxi);

  /* etiquette les maxima de f */
  if (! l2dlabelextrema(f, 0, maxi, &nlabels)) /* value of nlabels not used */
  {
    fprintf(stderr, "%s: l2dlabelextrema failed\n", F_NAME);
    return 0;
  }

  /* calcule histo */  
  memset(histo, 0, (NDG_MAX+1) * sizeof(index_t));
  for (x = 0; x < N; x++) histo[F[x]] += 1;

  /* cherche niveau max */
  kp = NDG_MAX; 
  while (!histo[kp]) kp--;

  /* seuil au niveau max */
  for (x = 0; x < N; x++) Fkp[x] = ( (F[x]>=kp) ? 255 : 0 );

  for (kp = kp - 1; kp >= NDG_MIN; kp--) /* descend niveau par niveau */
  if (histo[kp])
  {

#ifdef VERBOSE
  fprintf(stderr, "  niveau %d\n", kp);
#endif

    /* seuil au niveau kp */
    for (x = 0; x < N; x++) Fk[x] = ( (F[x]>=kp) ? 255 : 0 );

#ifdef STEP
    { char buf[10]; printf("kp=%d\n",kp); writeimage(fk,"fk"); writeimage(fkp,"fkp"); scanf("%s", buf);}
#endif

    /* etiquette les composantes de fkp */
    if (! l2dlabel(fkp, lab, &nlabels))
    {
      fprintf(stderr, "%s: l2dlabel failed\n", F_NAME);
      return 0;
    }

    /* topotess binaire sur fkp geodesiquement a fk */
    if (! l2dtopotess(lab, fk))
    {
      fprintf(stderr, "%s: l2dvoronoi failed\n", F_NAME);
      return 0;
    }

    /* fkp = fkp U {maxima de niveau kp} */
    for (x = 0; x < N; x++) 
      if (LAB[x] || (Fk[x] && MAXI[x])) Fkp[x] = 255; else Fkp[x] = 0;

  }

  /* recupere dans f le resultat fkp */
  for (x = 0; x < N; x++) F[x] = Fkp[x];

  freeimage(maxi);
  freeimage(lab);
  freeimage(fk);
  freeimage(fkp);
  return 1;
} /* l2dtopotessndgVS() */

/* =============================================================== */
int32_t l2dinvariants(struct xvimage *f, index_t *nbcc, index_t *nbtrous, index_t *euler)
/* =============================================================== */
/*
  Calculs des nombres de composantes connexes et trous.
*/
#undef F_NAME
#define F_NAME "l2dinvariants"
{
  index_t rs, cs, N;
  index_t x, y, w;
  uint8_t *F;
  struct xvimage * lab;
  int32_t *LAB;
  int32_t nlabels;
  Lifo * LIFO;
  index_t tab[9]; int32_t n, k;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: ne traite pas les images volumiques\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  N = rs * cs;
  F = UCHARDATA(f);
  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(0);
    
  }
  LAB = SLONGDATA(lab);

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {  
    fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
    return(0);
  }
  
  /* ============================================= */
  /* calcul de la caracteristique d'Euler-Poincare */
  /* ============================================= */
  
  *euler = EulerKh2d(f);

  /* ======================================== */
  /* calcul du nombre de composantes connexes */
  /* ======================================== */

  nlabels = 0;
  for (x = 0; x < N; x++)
  {
    if (F[x] && !LAB[x]) 
    {
      nlabels += 1;
      LAB[x] = nlabels;
      LifoPush(LIFO, x);
      /* propage le label dans la region connexe */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        Alphacarre2d(rs, cs, w%rs, w/rs, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
        Betacarre2d(rs, cs, w%rs, w/rs, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
    } /* if (F[x] && !LAB[x]) */
  } /* for (x = 0; x < N; x++) */
  *nbcc = nlabels;
  *nbtrous =  *nbcc - *euler;

  LifoTermine(LIFO);
  return 1;
} /* l2dinvariants() */

/* =============================================================== */
int32_t l2dboundary(struct xvimage * f)
/* =============================================================== */
/* 
   extrait la frontière interne
   def: {x in F | theta(x) inter Fbar neq emptyset}
*/
{
#undef F_NAME
#define F_NAME "l2dboundary"
  struct xvimage * g;
  index_t rs, cs;
  index_t x, y;
  uint8_t *F;
  uint8_t *G;
  index_t tab[8];
  int32_t n, u;

  rs = rowsize(f);
  cs = colsize(f);
  F = UCHARDATA(f);
  g = copyimage(f);
  if (g == NULL)
  {   fprintf(stderr,"%s: copyimage failed\n", F_NAME);
      return 0;
  }  
  G = UCHARDATA(g);
  razimage(f);

  for (y = 0; y < cs; y++)
    for (x = 0; x < rs; x++)
      if (G[y*rs + x])
      {
	Thetacarre2d(rs, cs, x, y, tab, &n);
	for (u = 0; u < n; u++)
	  if (G[tab[u]] == 0) 
	  {
	    F[y*rs + x] = NDG_MAX;
	    goto next;
	  }
      next:;
      } 
  
  freeimage(g);
  return 1;
} /* l2dboundary() */

/* =============================================================== */
int32_t l2dborder(struct xvimage * f)
/* =============================================================== */
/* 
   extrait la frontière interne
   def: closure{x in F | x free for F}
*/
{
#undef F_NAME
#define F_NAME "l2dborder"
  struct xvimage * g;
  index_t rs, cs;
  index_t x, y;
  uint8_t *F;
  uint8_t *G;

  assert(datatype(f) == VFF_TYP_1_BYTE);
  rs = rowsize(f);
  cs = colsize(f);
  F = UCHARDATA(f);
  g = copyimage(f);
  if (g == NULL)
  {   fprintf(stderr,"%s: copyimage failed\n", F_NAME);
      return 0;
  }  
  G = UCHARDATA(g);
  razimage(f);
  for (y = 0; y < cs; y++)
    for (x = 0; x < rs; x++)
      if (G[y*rs + x] && FaceLibre2d(g, x, y))
	F[y*rs + x] = VAL_OBJET;
  l2dmakecomplex(f);
  freeimage(g);
  return 1;
} /* l2dborder() */

/* =============================================================== */
int32_t l2dseltype(struct xvimage * k, uint8_t d1, uint8_t d2, uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2)
/* =============================================================== */
/* 
  Selects the elements x of the Khalimsky object \b k 
  which satisfy the following inequalities : 
  \b a1 <= a(x) <= \b a2
  \b b1 <= b(x) <= \b b2
  \b d1 <= d(x) <= \b d2
  where
  d(x) = dimension of x
  a(x) = number of elements under x of dimension d(x) - 1
  b(x) = number of elements over x of dimension d(x) + 1
*/
#undef F_NAME
#define F_NAME "l2dseltype"
{
  index_t rs, cs, N, i1, j1, i2, j2, x, y;
  int32_t a, b, d;
  uint8_t * K;
  struct xvimage * kp;
  uint8_t * KP;
  index_t tab[9]; int32_t n, u;

//#define DEBUG_l2dseltype
#ifdef DEBUG_l2dseltype
  printf("call %s %d %d %d %d %d %d\n", F_NAME, d1, d2, a1, a2, b1, b2);
#endif

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  kp = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  if (kp == NULL)
  {   
    fprintf(stderr,"%s : allocimage failed\n", F_NAME);
    return(0);
  }
  KP = UCHARDATA(kp);
  memset(KP, 0, N);

  for (j1 = 0; j1 < cs; j1++)
  for (i1 = 0; i1 < rs; i1++)
  {
    x = j1*rs + i1;
    d = DIM2D(i1,j1);
    if (K[x] && (d1 <= d) && (d <= d2))
    {
      Alphacarre2d(rs, cs, i1, j1, tab, &n);
      for (a = u = 0; u < n; u++) /* parcourt les eventuels alpha-voisins */
      {
	y = tab[u];
	i2 = y%rs; j2 = y/rs;
	if (K[y] && (DIM2D(i2,j2) == (d-1))) a++;
      }
      if ((a1 <= a) && (a <= a2))
      {
	Betacarre2d(rs, cs, i1, j1, tab, &n);
	for (b = u = 0; u < n; u++) /* parcourt les eventuels beta-voisins */
	{
	  y = tab[u];
	  i2 = y%rs; j2 = y/rs;
	  if (K[y] && (DIM2D(i2,j2) == (d+1))) b++;
	}
	if ((b1 <= b) && (b <= b2))
	{
	  KP[x] = NDG_MAX;
	}
      }
    }
  } // for k1, j1, i1
  for (x = 0; x < N; x++) K[x] = KP[x];
  freeimage(kp);
  return 1;
} /* l2dseltype() */
