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
/* operateurs de dilatation et d'erosion morphologique */
/* Michel Couprie - avril 1997 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <ldilateros.h>

/* ==================================== */
int32_t ldilatnum( struct xvimage *f, 
            struct xvimage *m, 
            int32_t xc, 
            int32_t yc)
/* dilatation numerique par un element structurant de taille quelconque */
/* attention algo non optimal - voir l'implementation de dilat */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilatnum"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t i, j;           /* index muet */
  register index_t k, l;           /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                     /* image de travail */
  int32_t sup;
  int32_t t;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
      return(0);
  }
  for (x = 0; x < N; x++) H[x] = F[x];

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    sup = NDG_MIN;
    for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
    {
      t = (int32_t)M[j * rsm + i];
      if (t)
      {
        l = y + j - yc;
        k = x + i - xc;
        if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && ((int32_t)H[l * rs + k] + t > sup))
           sup = (int32_t)H[l * rs + k] + t;
      }
    } 
    F[y * rs + x] = (uint8_t)mcmin(sup, NDG_MAX);
  }

  free(H);
  return 1;
} /* ldilatnum() */

/* ==================================== */
int32_t lerosnum( struct xvimage *f,
           struct xvimage *m,  
           int32_t xc,
           int32_t yc)
/* operateur d'erosion numerique par un element structurant de taille quelconque */
/* attention algo non optimal - voir l'implementation de dilat */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "lerosnum"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t i, j;                    /* index muet */
  index_t k, l;                    /* index muet */
  register index_t rs = rowsize(f);         /* taille ligne */
  register index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                     /* image de travail */
  int32_t inf;
  int32_t t;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
      return(0);
  }
  for (x = 0; x < N; x++) H[x] = F[x];

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    inf = NDG_MAX;
    for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
    {
      t = (int32_t)M[j * rsm + i];
      if (t)
      {
        l = y + j - yc;
        k = x + i - xc; 
        if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && ((int32_t)H[l * rs + k] - t < inf))
           inf = (int32_t)H[l * rs + k] - t;
      }
    } 
    F[y * rs + x] = (uint8_t)mcmax(inf, NDG_MIN);
  }

  free(H);
  return 1;
} /* lerosnum() */

/* ==================================== */
int32_t ldilateros_ldilatbin(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* operateur de dilatation binaire par un element structurant de taille quelconque */
/* ATTENTION: l'element structurant doit etre CONNEXE et REFLEXIF */
/* Michel Couprie - avril 1998 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees de l'origine de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilateros_ldilatbin"
{
  index_t x, y, z, w;              /* index muet de pixel */
  register index_t i, j;           /* index muet */
  register index_t k, l;           /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  index_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;
  int32_t frontiere;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  if (!M[yc * rsm + xc]) /* l'element structurant N'est PAS reflexif */
  {
    fprintf(stderr, "%s: l'element structurant doit etre reflexif\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s() : malloc failed for tab_es\n", F_NAME);
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
    /* verifie si (x,y) est un point de la frontiere de l'objet */
    w = y * rs + x;
    if (H[w])
    {
      frontiere = 0;
      for (k = 0; k < 8; k += 2) /* on cherche une frontiere 8-connexe */
      {
        z = voisin(w, k, rs, N);
        if ((z != -1) && (!H[z])) { frontiere = 1; break; }
      } /* for k... */

      if (!frontiere) 
        F[w] = NDG_MAX;
      else
      {
        for (c = 0; c < nptb ; c += 1)
        {
          l = y + tab_es_y[c] - yc;
          k = x + tab_es_x[c] - xc; 
          if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
	    F[l * rs + k] = NDG_MAX;
        }
      }
    }
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* ldilateros_ldilatbin() */

/* ==================================== */
int32_t ldilateros_lerosbin(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* operateur d'erosion binaire par un element structurant de taille quelconque */
/* ATTENTION: l'element structurant doit etre CONNEXE et REFLEXIF */
/* Michel Couprie - avril 1998 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees de l'origine de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilateros_lerosbin"
{
  index_t x, y, z, w;              /* index muet de pixel */
  register index_t i, j;           /* index muet */
  register index_t k, l;           /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  index_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;
  int32_t frontiere;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  if (!M[yc * rsm + xc]) /* l'element structurant N'est PAS reflexif */
  {
    fprintf(stderr, "%s: l'element structurant doit etre reflexif\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s() : malloc failed for tab_es\n", F_NAME);
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
    /* verifie si (x,y) est un point de la frontiere du fond */
    w = y * rs + x;
    if (!H[w])
    {
      frontiere = 0;
      for (k = 0; k < 8; k += 2) /* on cherche une frontiere 8-connexe */
      {
        z = voisin(w, k, rs, N);
        if ((z != -1) && (H[z])) { frontiere = 1; break; }
      } /* for k... */

      if (!frontiere) 
        F[w] = NDG_MIN;
      else
      {
        for (c = 0; c < nptb ; c += 1)
        {
          l = y + tab_es_y[c] - yc;
          k = x + tab_es_x[c] - xc; 
          if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs))
	    F[l * rs + k] = NDG_MIN;
        }
      }
    }
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* ldilateros_lerosbin() */

/* ==================================== */
int32_t ldilateros_ldilatfast(struct xvimage *f, uint8_t *mask) 
/* operateur de dilatation numerique par un element structurant de taille inferieure a 3x3 */
/* UNIQUEMENT POUR DES ELEMENTS STRUCTURANTS REFLEXIFS */
/* Michel Couprie - juillet 1996 */
/* mask : masque du 8-voisinage representant l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilateros_ldilatfast"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                     /* image de travail */
  uint8_t sup;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++) H[x] = F[x];

  for (x = 0; x < N; x++)
  {
    sup = H[x];                         /* l'ES est reflexif */
    for (k = 0; k < 8; k += 1)
    {
      if (mask[k])                      /* element structurant */
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (H[y] > sup)) sup = H[y];
      }
    } /* for k */
    F[x] = sup;
  }

  free(H);
  return 1;
} /* ldilateros_ldilatfast() */

#define BORDZERO

/* ==================================== */
int32_t ldilateros_lerosfast(struct xvimage *f, uint8_t *mask)
/* operateur d'erosion numerique  par un element structurant de taille inferieure a 3x3 */
/* UNIQUEMENT POUR DES ELEMENTS STRUCTURANTS REFLEXIFS */
/* Michel Couprie - juillet 1996 */
/* update avril 1999 : option BORDZERO */
/* mask : masque du 8-voisinage representant l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilateros_lerosfast"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                     /* image de travail */
  uint8_t inf;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
      return(0);
  }
  for (x = 0; x < N; x++) H[x] = F[x];

  for (x = 0; x < N; x++)
  {
    inf = H[x];                         /* l'ES est reflexif */
    for (k = 0; k < 8; k += 1)
    {
      if (mask[k])                      /* element structurant */
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (H[y] < inf)) inf = H[y];
#ifdef BORDZERO
        if (y == -1) inf = NDG_MIN;
#endif
      }
    } /* for k */
    F[x] = inf;
  }

  free(H);
  return 1;
} /* ldilateros_lerosfast() */

/* ==================================== */
int32_t ldilatbyte(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* operateur de dilatation numerique par un element structurant plan de taille quelconque */
/* Michel Couprie - juillet 1997 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilatbyte"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t i, j;                    /* index muet */
  register index_t k, l;                    /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  index_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  uint8_t sup;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s() : malloc failed for tab_es\n", F_NAME);
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = rsm - 1 - i;  /* symetrique de l'e.s. */
         tab_es_y[k] = csm - 1 - j;
         k += 1;
      }
  xc = rsm - 1 - xc;
  yc = csm - 1 - yc;

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    sup = NDG_MIN;
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && (H[l * rs + k] > sup))
	sup = H[l * rs + k];
    }
    F[y * rs + x] = sup;
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* ldilatbyte() */

/* ==================================== */
int32_t ldilatlong(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* operateur de dilatation numerique par un element structurant plan de taille quelconque */
/* Michel Couprie - juillet 1997 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilatlong"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t i, j;                    /* index muet */
  register index_t k, l;                    /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  index_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  int32_t *F = SLONGDATA(f);
  int32_t *H;                      /* image de travail */
  int32_t sup;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (int32_t *)calloc(1,N*sizeof(int32_t));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, 4*N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s() : malloc failed for tab_es\n", F_NAME);
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = rsm - 1 - i;  /* symetrique de l'e.s. */
         tab_es_y[k] = csm - 1 - j;
         k += 1;
      }
  xc = rsm - 1 - xc;
  yc = csm - 1 - yc;

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    sup = 0;
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && (H[l * rs + k] > sup))
	sup = H[l * rs + k];
    }
    F[y * rs + x] = sup;
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* ldilatlong() */

/* ==================================== */
int32_t ldilat2(struct xvimage *f, int32_t nptb, int32_t *tab_es_x, int32_t *tab_es_y, int32_t xc, int32_t yc)
/* nptb :                   nombre de points de l'e.s. */
/* tab_es_x :               liste des coord. x des points de l'e.s. */
/* tab_es_y :               liste des coord. y des points de l'e.s. */
/* xc, yc :                 coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilat2"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t k, l;           /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;            /* image de travail */
  uint8_t sup;
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    sup = NDG_MIN;
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && (H[l * rs + k] > sup))
	sup = H[l * rs + k];
    }
    F[y * rs + x] = sup;
  }

  free(H);

  return 1;
} /* ldilat2() */

/* ==================================== */
int32_t ldilat3(struct xvimage *f, int32_t nptb, int32_t *tab_es_x, int32_t *tab_es_y, int32_t *tab_es_z, int32_t xc, int32_t yc, int32_t zc)
/* nptb :                   nombre de points de l'e.s. */
/* tab_es_x :               liste des coord. x des points de l'e.s. */
/* tab_es_y :               liste des coord. y des points de l'e.s. */
/* tab_es_z :               liste des coord. z des points de l'e.s. */
/* xc, yc, zc :             coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilat3"
{
  index_t x, y, z;
  register index_t i, j, k;
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(f);           /* nb plans */
  index_t N = ds * ps;
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;            /* image de travail */
  uint8_t sup;
  int32_t c;

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    sup = NDG_MIN;
    for (c = 0; c < nptb ; c += 1)
    {
      k = z + tab_es_z[c] - zc;
      j = y + tab_es_y[c] - yc;
      i = x + tab_es_x[c] - xc; 
      if ((i >= 0) && (i < rs) && (j >= 0) && (j < cs) && (k >= 0) && (k < ds) && (H[k*ps + j*rs + i] > sup))
	sup = H[k*ps + j*rs + i];
    }
    F[z*ps + y*rs + x] = sup;
  }

  free(H);

  return 1;
} /* ldilat3() */

/* ==================================== */
int32_t lerosbyte(
        struct xvimage *f,
        struct xvimage *m,  
        int32_t xc,
        int32_t yc)
/* operateur d'erosion numerique par un element structurant plan de taille quelconque */
/* Michel Couprie - juillet 1997 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "lerosbyte"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t i, j;                    /* index muet */
  register index_t k, l;                    /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  index_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;                    /* image de travail */
  uint8_t inf;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s() : malloc failed for tab_es\n", F_NAME);
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = rsm - 1 - i;  /* symetrique de l'e.s. */
         tab_es_y[k] = csm - 1 - j;
         k += 1;
      }
  xc = rsm - 1 - xc;
  yc = csm - 1 - yc;

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    inf = NDG_MAX;
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && (H[l * rs + k] < inf))
	inf = H[l * rs + k];
    }
    F[y * rs + x] = inf;
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* lerosbyte() */

/* ==================================== */
int32_t leroslong(
        struct xvimage *f,
        struct xvimage *m,  
        int32_t xc,
        int32_t yc)
/* operateur d'erosion numerique par un element structurant plan de taille quelconque */
/* Michel Couprie - juillet 1997 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "leroslong"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t i, j;                    /* index muet */
  register index_t k, l;                    /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  index_t rsm = rowsize(m);        /* taille ligne masque */
  index_t csm = colsize(m);        /* taille colonne masque */
  index_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  int32_t *F = SLONGDATA(f);
  int32_t *H;                      /* image de travail */
  int32_t inf;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (int32_t *)calloc(1,N*sizeof(int32_t));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, 4*N);

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s() : malloc failed for tab_es\n", F_NAME);
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = rsm - 1 - i;  /* symetrique de l'e.s. */
         tab_es_y[k] = csm - 1 - j;
         k += 1;
      }
  xc = rsm - 1 - xc;
  yc = csm - 1 - yc;

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    inf = INT32_MAX;
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && (H[l * rs + k] < inf))
	inf = H[l * rs + k];
    }
    F[y * rs + x] = inf;
  }

  free(H);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* leroslong() */

/* ==================================== */
int32_t leros2(struct xvimage *f, int32_t nptb, int32_t *tab_es_x, int32_t *tab_es_y, int32_t xc, int32_t yc)
/* nptb :                   nombre de points de l'e.s. */
/* tab_es_x :               liste des coord. x des points de l'e.s. */
/* tab_es_y :               liste des coord. y des points de l'e.s. */
/* xc, yc :                 coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "leros2"
{
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t k, l;           /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;            /* image de travail */
  uint8_t inf;
  int32_t c;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    inf = NDG_MAX;
    for (c = 0; c < nptb ; c += 1)
    {
      l = y + tab_es_y[c] - yc;
      k = x + tab_es_x[c] - xc; 
      if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) && (H[l * rs + k] < inf))
	inf = H[l * rs + k];
    }
    F[y * rs + x] = inf;
  }

  free(H);
  return 1;
} /* leros2() */

/* ==================================== */
int32_t leros3(struct xvimage *f, int32_t nptb, int32_t *tab_es_x, int32_t *tab_es_y, int32_t *tab_es_z, int32_t xc, int32_t yc, int32_t zc)
/* nptb :                   nombre de points de l'e.s. */
/* tab_es_x :               liste des coord. x des points de l'e.s. */
/* tab_es_y :               liste des coord. y des points de l'e.s. */
/* tab_es_z :               liste des coord. z des points de l'e.s. */
/* xc, yc, zc :             coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "leros3"
{
  index_t x, y, z;
  register index_t i, j, k;
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ps = rs * cs;            /* taille plan */
  index_t ds = depth(f);           /* nb plans */
  index_t N = ds * ps;
  uint8_t *F = UCHARDATA(f);
  uint8_t *H;            /* image de travail */
  uint8_t inf;
  int32_t c;

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {  
     fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
     return(0);
  }

  memcpy(H, F, N);

  for (z = 0; z < ds; z++)
  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    inf = NDG_MAX;
    for (c = 0; c < nptb ; c += 1)
    {
      k = z + tab_es_z[c] - zc;
      j = y + tab_es_y[c] - yc;
      i = x + tab_es_x[c] - xc; 
      if ((i >= 0) && (i < rs) && (j >= 0) && (j < cs) && (k >= 0) && (k < ds) && (H[k*ps + j*rs + i] < inf))
	inf = H[k*ps + j*rs + i];
    }
    F[z*ps + y*rs + x] = inf;
  }

  free(H);

  return 1;
} /* leros3() */

/* ==================================== */
int32_t ldilateros_ldilat(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* operateur de dilatation numerique par un element structurant plan de taille quelconque */
/* Michel Couprie - juillet 1997 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilateros_ldilat"
{
  if (datatype(f) == VFF_TYP_1_BYTE) 
    return ldilatbyte(f, m, xc, yc);
  else if (datatype(f) == VFF_TYP_4_BYTE) 
    return ldilatlong(f, m, xc, yc);
  else
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    return 0;
  }
} // ldilateros_ldilat()

/* ==================================== */
int32_t ldilateros_leros(struct xvimage *f, struct xvimage *m, int32_t xc, int32_t yc)
/* operateur d'erosion numerique par un element structurant plan de taille quelconque */
/* Michel Couprie - juillet 1997 */
/* m : masque representant l'element structurant */
/* xc, yc : coordonnees du "centre" de l'element structurant */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldilateros_leros"
{
  if (datatype(f) == VFF_TYP_1_BYTE) 
    return lerosbyte(f, m, xc, yc);
  else if (datatype(f) == VFF_TYP_4_BYTE) 
    return leroslong(f, m, xc, yc);
  else
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    return 0;
  }
} // ldilateros_leros()
