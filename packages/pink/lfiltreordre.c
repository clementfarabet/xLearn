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
/* filtre d'ordre sur un voisinage quelconque */
/* Michel Couprie - decembre 1997 */
/* update juin 2001: 3D */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>

/* =================================================
  Selection en temps moyen lineaire
  D'apres "Introduction a l'algorithmique", 
    T. Cormen, C. Leiserson, R. Rivest, pp. 152, Dunod Ed. 
   =================================================
*/

/* =============================================================== */
int32_t lfiltreordre_Partitionner(uint8_t *A, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[p] et les autres.
*/
{
  uint8_t t;
  uint8_t x = A[p];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (A[j] > x);
    do i++; while (A[i] < x);
    if (i < j) { t = A[i]; A[i] = A[j]; A[j] = t; }
    else return j;
  } /* while (1) */   
} /* lfiltreordre_Partitionner() */

/* =============================================================== */
int32_t lfiltreordre_PartitionStochastique(uint8_t *A, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[q] et les autres, avec q tire au hasard dans [p,r].
*/
{
  uint8_t t;
  int32_t q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  return lfiltreordre_Partitionner(A, p, r);
} /* lfiltreordre_PartitionStochastique() */

/* =============================================================== */
uint8_t lfiltreordre_SelectionStochastique (uint8_t * A, int32_t p, int32_t r, int32_t i)
/* =============================================================== */
/* 
  retourne la valeur de rang i dans le tableau A 
  entre l'indice m (compris) a l'indice n (compris) 
*/
{
  int32_t q, k; 
  if (p == r) return A[p];
  q = lfiltreordre_PartitionStochastique(A, p, r);
  k = q - p + 1;
  if (i <= k) return lfiltreordre_SelectionStochastique (A, p, q, i);
  else        return lfiltreordre_SelectionStochastique (A, q+1, r, i - k) ;
} /* lfiltreordre_SelectionStochastique() */

/* ==================================== */
int32_t lfiltreordre(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc, double r)
/* r : rang ramene a une echelle 0..1 (ex: 0.5 -> filtre median) */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
{
  int32_t rang;
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t i, j;                    /* index muet */
  register int32_t k, l;                    /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  int32_t rsm = rowsize(m);        /* taille ligne masque */
  int32_t csm = colsize(m);        /* taille colonne masque */
  int32_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  uint8_t *tab_es_val;   /* liste des valeurs des points de l'e.s. */
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "lfiltreordre: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"lfiltreordre() : malloc failed for H\n");
     return(0);
  }

  for (x = 0; x < N; x++) H[x] = F[x];

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  rang = (int32_t)((double)(nptb - 1) * r);
#ifdef VERBOSE
  printf("r = %g ; nptb = %d ; rang = %d\n", r, nptb, rang);
#endif
  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_val = (uint8_t *)calloc(1,nptb * sizeof(char));
  if ((tab_es_x == NULL) || (tab_es_y == NULL) || (tab_es_val == NULL))
  {  
     fprintf(stderr,"lfiltreordre() : malloc failed for tab_es\n");
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = i;
         tab_es_y[k] = j;
         k += 1;
      }

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
        tab_es_val[c] = H[l * rs + k];
      else
        tab_es_val[c] = 0;
    }
    F[y * rs + x] = lfiltreordre_SelectionStochastique(tab_es_val, 0, nptb - 1, rang);
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  free(tab_es_val);
  return 1;
} /* lfiltreordre() */

/* ==================================== */
int32_t lfiltreordre3d(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc, int32_t zc, double r)
/* r : rang ramene a une echelle 0..1 (ex: 0.5 -> filtre median) */
/* m : masque representant l'element structurant */
/* xc, yc, zc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
{
  int32_t rang;
  int32_t x,y,z;
  register int32_t i, j, k;                 /* index muet */
  register int32_t n, o, p;                 /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t ds = depth(f);           /* nb plans */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t N = ps * ds;             /* taille image */
  int32_t rsm = rowsize(m);        /* taille ligne masque */
  int32_t csm = colsize(m);        /* taille colonne masque */
  int32_t dsm = depth(m);          /* nb plans masque */
  int32_t psm = rsm * csm;         /* taille plan masque */
  int32_t Nm = psm * dsm;          /* taille masque */
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t *tab_es_z;               /* liste des coord. z des points de l'e.s. */
  uint8_t *tab_es_val;   /* liste des valeurs des points de l'e.s. */
  int32_t c;

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"lfiltreordre3d() : malloc failed for H\n");
     return(0);
  }

  for (x = 0; x < N; x++) H[x] = F[x];

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  rang = (int32_t)((double)(nptb - 1) * r);
#ifdef VERBOSE
  printf("r = %g ; nptb = %d ; rang = %d\n", r, nptb, rang);
#endif

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_z = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_val = (uint8_t *)calloc(1,nptb * sizeof(char));
  if ((tab_es_x == NULL) || (tab_es_y == NULL) || (tab_es_z == NULL) || (tab_es_val == NULL))
  {  
     fprintf(stderr,"lfiltreordre3d() : malloc failed for tab_es\n");
     return(0);
  }

  n = 0;
  for (k = 0; k < dsm; k += 1)
    for (j = 0; j < csm; j += 1)
      for (i = 0; i < rsm; i += 1)
        if (M[k * psm + j * rsm + i])
        {
           tab_es_x[n] = i;
           tab_es_y[n] = j;
           tab_es_z[n] = k;
           n += 1;
        }

  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    for (c = 0; c < nptb ; c += 1)
    {
      o = z + tab_es_z[c] - zc;
      p = y + tab_es_y[c] - yc;
      n = x + tab_es_x[c] - xc; 
      if ((o >= 0) && (o < ds) && (p >= 0) && (p < cs) && (n >= 0) && (n < rs))
        tab_es_val[c] = H[o * ps + p * rs + n];
      else
        tab_es_val[c] = 0;
    }
    F[z * ps + y * rs + x] = lfiltreordre_SelectionStochastique(tab_es_val, 0, nptb - 1, rang);
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  free(tab_es_z);
  free(tab_es_val);
  return 1;
} /* lfiltreordre3d() */
