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
/****************************************************************
*
* Routine Name: ldistXXX - library call for dist
*
* Purpose:     Calcul du niveau des branches d'un arbre en n-connexit�
               (n = 4,8,6,18,26)
*
* Input:       X: Image binaire
*Output:       Image en niveaux de gris (entiers longs)
*
* Author: C�dric All�ne
*
****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <llabeltree.h>

#define MAX(a,b)  ((a)>(b)?(a):(b))


/* ==================================== */
int32_t llabeltree(struct xvimage *imgx,   /* donnee: image binaire */       
               int32_t connex,
               struct xvimage *res     /* resultat: distances (doit �tre allou�e) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llabeltree"
{ 
  if ((connex == 4) || (connex == 8))
    return llabeltree2d(imgx, connex, res);
  else
    return llabeltree3d(imgx, connex, res);
} // llabeltree


/* ==================================== */
int32_t llabeltree2d(struct xvimage *imgx,   /* donnee: image binaire */       
                 int32_t connex,
                 struct xvimage *res     /* resultat: distances (doit �tre allou�e) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llabeltree2d"
{ 
  int32_t rs = imgx->row_size;
  int32_t cs = imgx->col_size;
  int32_t N = rs * cs;           /* taille de l'image */
  uint8_t *X;                    /* pointeur sur l'image x */
  uint32_t *D;                   /* pointeur sur les distances */
  int32_t i, k, current_pixel;
  int32_t d_max = 0;

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != 1))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (datatype(res) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: result image type must be int32_t\n", F_NAME);
    return 0;
  }

  X = UCHARDATA(imgx);
  D = ULONGDATA(res);

  k = 0;
  
  for (i = 0; i < rs; i++) 
  {
    if (!X[i]) 
    {
      D[i] = 0;
    }
    else
    { /* X[i] != 0 */
      D[i] = 1; 
      k ++;
      current_pixel = i;
    }
  } /* for (i = 0; i < rs; i++) */
  
  if (k!=1)
  {
    fprintf (stderr, "%s: uniquement le point de d�part de la racine sur la premi�re ligne (y=0)\n", F_NAME);
    return 0;
  } /* if (k!=1) */

  for (i = rs; i < N; i++) 
  {
    if (!X[i]) 
    {
      D[i] = 0;
    }
    else
    { /* X[i] != 0 */
      D[i] = -1; 
    }
  } /* for (i = rs; i < N; i++) */

  switch (connex)
  {
    case 4: d_max = labelbranch2d4 (X, rs, N, current_pixel, 0, D); break;
    case 8: d_max = labelbranch2d8 (X, rs, N, current_pixel, 0, D); break;
    default: 
      fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

  return(d_max);
} // llabeltree2d()


/* ==================================== */
int32_t llabeltree3d(struct xvimage *imgx,   /* donnee: image binaire */       
                 int32_t connex,
                 struct xvimage *res     /* resultat: distances (doit �tre allou�e) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llabeltree3d"
{ 
  int32_t rs = imgx->row_size;
  int32_t cs = imgx->col_size;
  int32_t ds = depth(imgx);
  int32_t ps = rs * cs;          /* taille d'un plan */
  int32_t N = ps * ds;           /* taille de l'image */
  uint8_t *X;                    /* pointeur sur l'image x */
  uint32_t *D;                   /* pointeur sur les distances */
  int32_t i, k, current_pixel;
  int32_t d_max = 0;

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (datatype(res) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: result image type must be int32_t\n", F_NAME);
    return 0;
  }

  X = UCHARDATA(imgx);
  D = ULONGDATA(res);

  k = 0;
  
  for (i = 0; i < rs*cs; i++) 
  {
    if (!X[i]) 
    {
      D[i] = 0;
    }
    else
    { /* X[i] != 0 */
      D[i] = 1; 
      k ++;
      current_pixel = i;
    }
  } /* for (i = 0; i < rs*cs; i++) */
  
  if (k!=1)
  {
    fprintf (stderr, "%s: uniquement le point de d�part de la racine sur le plan d'origine (z=0)\n", F_NAME);
    return 0;
  } /* if (k!=1) */

  for (i = rs*cs; i < N; i++) 
  {
    if (!X[i]) 
    {
      D[i] = 0;
    }
    else
    { /* X[i] != 0 */
      D[i] = -1; 
    }
  } /* for (i = rs*cs; i < N; i++) */

  switch (connex)
  {
    case 6:  d_max = labelbranch3d6  (X, rs, ps, N, current_pixel, 0, D); break;
    case 18: d_max = labelbranch3d18 (X, rs, ps, N, current_pixel, 0, D); break;
    case 26: d_max = labelbranch3d26 (X, rs, ps, N, current_pixel, 0, D); break;
    default: 
      fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

  return(d_max);
} // llabeltree3d()


/* ==================================== */
int32_t labelbranch2d4(uint8_t *X,  /* donnee: pointeur sur l'image x */
                   int32_t rs,            /* donnee: taille ligne */
		   int32_t N,             /* donnee: taille image */
                   int32_t current_pixel, /* donnee: num�ro pixel courant */  
                   int32_t intersection,  /* donnee: flag d'intersection */  
                   uint32_t *D        /* resultat: pointeur sur distances */
) /* Etiquette les branches avec la 4-connexit� */
/* ==================================== */
{ 
  int32_t d, k, j, d_max, l;
      
  d = D[current_pixel];
  if (nbvois4 (X, current_pixel, rs, N) > 2)
  {
    if (intersection == 0) intersection = 1;
  }
  else if (intersection == 1)
  {
    intersection = 0;
    d++;
    D[current_pixel] = d;
  }
  d_max = d;
 
  for (k = 0; k < 8; k += 2)
  {
    j = voisin (current_pixel, k, rs, N);
    if ((j != -1) && (D[j] == -1))
    {
      if (nonbord (j, rs, N) == 0)
      {
        fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d\n", F_NAME, j%rs, j/rs);
        return 0;
      }
      D[j] = d;
      l = labelbranch2d4 (X, rs, N, j, intersection, D);
      d_max = MAX(d_max, l);
    }
  }

  return(d_max);
} // llabeltree2d4()

/* ==================================== */
int32_t labelbranch2d8(uint8_t *X,  /* donnee: pointeur sur l'image x */
                   int32_t rs,            /* donnee: taille ligne */
		   int32_t N,             /* donnee: taille image */
                   int32_t current_pixel, /* donnee: num�ro pixel courant */  
                   int32_t intersection,  /* donnee: flag d'intersection */  
                   uint32_t *D        /* resultat: pointeur sur distances */
) /* Etiquette les branches avec la 8-connexit� */
/* ==================================== */
{ 
  int32_t d, k, j, d_max, l;
      
  d = D[current_pixel];
  if (nbvois8 (X, current_pixel, rs, N) > 2)
  {
    if (intersection == 0) intersection = 1;
  }
  else if (intersection == 1)
  {
    intersection = 0;
    d++;
    D[current_pixel] = d;
  }
  d_max = d;

  for (k = 0; k < 8; k += 1)
  {
    j = voisin (current_pixel, k, rs, N);
    if ((j != -1) && (D[j] == -1)) 
    {
      if (nonbord (j, rs, N) == 0)
      {
        fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d\n", F_NAME, j%rs, j/rs);
        return 0;
      }
      D[j] = d;
      l = labelbranch2d8 (X, rs, N, j, intersection, D);
      d_max = MAX(d_max, l);
    }
  }

  return(d_max);
} // llabeltree2d8()

/* ==================================== */
int32_t labelbranch3d6(uint8_t *X,  /* donnee: pointeur sur l'image x */
                   int32_t rs,            /* donnee: taille ligne */
                   int32_t ps,            /* donnee: taille plan */
		   int32_t N,             /* donnee: taille image */
                   int32_t current_pixel, /* donnee: num�ro pixel courant */  
                   int32_t intersection,  /* donnee: flag d'intersection */  
                   uint32_t *D        /* resultat: pointeur sur distances */
) /* Etiquette les branches avec la 6-connexit� */
/* ==================================== */
{ 
  int32_t d, k, j, d_max, l;
      
  d = D[current_pixel];
  if (mctopo3d_nbvoiso6 (X, current_pixel, rs, ps, N) > 2)
  {
    if (intersection == 0) intersection = 1;
  }
  else if (intersection == 1)
  {
    intersection = 0;
    d++;
    D[current_pixel] = d;
  }
  d_max = d;
 
  for (k = 0; k < 10; k += 2)
  {
    j = voisin6 (current_pixel, k, rs, ps, N);
    if ((j != -1) && (D[j] == -1))
    {
      if (nonbord3d (j, rs, ps, N) == 0)
      {
        fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d z=%d\n", F_NAME, j%rs,(j%(rs*ps))/rs, j/(rs*ps));
        return 0;
      }
      D[j] = d;
      l = labelbranch3d6 (X, rs, ps, N, j, intersection, D);
      d_max = MAX(d_max, l);
    }
  }

  return(d_max);
} // llabeltree3d6()

/* ==================================== */
int32_t labelbranch3d18(uint8_t *X,  /* donnee: pointeur sur l'image x */
                    int32_t rs,            /* donnee: taille ligne */
                    int32_t ps,            /* donnee: taille plan */
		    int32_t N,             /* donnee: taille image */
                    int32_t current_pixel, /* donnee: num�ro pixel courant */  
                    int32_t intersection,  /* donnee: flag d'intersection */  
                    uint32_t *D        /* resultat: pointeur sur distances */
) /* Etiquette les branches avec la 18-connexit� */
/* ==================================== */
{ 
  int32_t d, k, j, d_max, l;
      
  d = D[current_pixel];
  if (mctopo3d_nbvoiso18 (X, current_pixel, rs, ps, N) > 2)
  {
    if (intersection == 0) intersection = 1;
  }
  else if (intersection == 1)
  {
    intersection = 0;
    d++;
    D[current_pixel] = d;
  }
  d_max = d;

  for (k = 0; k < 18; k += 1)
  {
    j = voisin18 (current_pixel, k, rs, ps, N);
    if ((j != -1) && (D[j] == -1))
    {
      if (nonbord3d (j, rs, ps, N) == 0)
      {
        fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d z=%d\n", F_NAME, j%rs,(j%(rs*ps))/rs, j/(rs*ps));
        return 0;
      }
      D[j] = d;
      l = labelbranch3d18 (X, rs, ps, N, j, intersection, D);
      d_max = MAX(d_max, l);
    }
  }

  return(d_max);
} // llabeltree3d18()

/* ==================================== */
int32_t labelbranch3d26(uint8_t *X,  /* donnee: pointeur sur l'image x */
                    int32_t rs,            /* donnee: taille ligne */
                    int32_t ps,            /* donnee: taille plan */
		    int32_t N,             /* donnee: taille image */
                    int32_t current_pixel, /* donnee: num�ro pixel courant */  
                    int32_t intersection,  /* donnee: flag d'intersection */  
                    uint32_t *D        /* resultat: pointeur sur distances */
) /* Etiquette les branches avec la 26-connexit� */
/* ==================================== */
{ 
  int32_t d, k, j, d_max, l;
      
  d = D[current_pixel];
  if (mctopo3d_nbvoiso26 (X, current_pixel, rs, ps, N) > 2)
  {
    if (intersection == 0) intersection = 1;
  }
  else if (intersection == 1)
  {
    intersection = 0;
    d++;
    D[current_pixel] = d;
  }
  d_max = d;

  for (k = 0; k < 26; k += 1)
  {
    j = voisin26 (current_pixel, k, rs, ps, N);
    if ((j != -1) && (D[j] == -1))
    {
      if (nonbord3d (j, rs, ps, N) == 0)
      {
        fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d z=%d\n", F_NAME, j%rs,(j%(rs*ps))/rs, j/(rs*ps));
        return 0;
      }
      D[j] = d;
      l = labelbranch3d26 (X, rs, ps, N, j, intersection, D);
      d_max = MAX(d_max, l);
    }
  }

  return(d_max);
} // llabeltree3d26()

