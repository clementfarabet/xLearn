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
/* histogramme d'une image en niveaux de gris */
/* histogramme bivariable */
/* Michel Couprie - juillet 1996, novembre 1999 */
/* update 6/4/2006 John Chaussard : cor. bug */
/* update 11/7/2010 MC & Mohamed Amine Salem : histogrammes d'orientations */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcimage.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <lhisto.h>

//#define DEBUG_lseuilhisto
#define VERBOSE

/* ==================================== */
int32_t lhisto(struct xvimage *image, struct xvimage *mask, index_t *histo)
/* ==================================== */
// WARNING : histo is an array [0..255] of index_t that must have been allocated
{
  int32_t i;
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ds = depth(image);       /* nombre plans */
  index_t nb = nbands(image);      /* nombre bandes */
  index_t N = rs * cs * ds * nb;   /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  uint8_t *M;

  for (i = 0; i <= NDG_MAX; i++) histo[i] = 0;

  if (mask == NULL)
    for (x = 0; x < N; x++) histo[SOURCE[x]] += 1;
  else
  {
    M = UCHARDATA(mask);
    for (x = 0; x < N; x++) if (M[x]) histo[SOURCE[x]] += 1;
  }
  return(1);
} /* lhisto() */

/* ==================================== */
int32_t lhisto1(struct xvimage *image, index_t *histo)
/* ==================================== */
// WARNING : histo is an array [0..255] of index_t that must have been allocated
{
  int32_t i;
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ds = depth(image);       /* nombre plans */
  index_t nb = nbands(image);      /* nombre bandes */
  index_t N = rs * cs * ds * nb;   /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */

  for (i = 0; i <= NDG_MAX; i++) histo[i] = 0;
  for (x = 0; x < N; x++) histo[SOURCE[x]] += 1;
  return(1);
} /* lhisto1() */

/* ==================================== */
int32_t lhisto2(struct xvimage *image1, struct xvimage *image2, 
            struct xvimage *mask, struct xvimage *histo)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lhisto2"
  int32_t i;
  index_t x;                        /* index muet de pixel */
  index_t rs = rowsize(image1);     /* taille ligne */
  index_t cs = colsize(image1);     /* taille colonne */
  index_t ds = depth(image1);       /* nombre plans */
  index_t nb = nbands(image1);      /* nombre bandes */
  index_t N = rs * cs * ds * nb;    /* taille image */
  uint8_t *SOURCE1 = UCHARDATA(image1);      /* l'image de depart 1 */
  uint8_t *SOURCE2 = UCHARDATA(image2);      /* l'image de depart 2 */
  uint8_t *M;
  int32_t *H = SLONGDATA(histo);
  index_t rsh = rowsize(histo);
  index_t nh = rsh * colsize(histo);

  if ((rowsize(image2) != rs) || (colsize(image2) != cs) || (depth(image2) != ds) || (nbands(image2) != nb))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if ((datatype(image1) != VFF_TYP_1_BYTE) || (datatype(image2) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  for (i = 0; i < nh; i++) H[i] = 0;

  if (mask == NULL)
    for (x = 0; x < N; x++) H[SOURCE2[x] * rsh + SOURCE1[x]] += 1;
  else
  {
    M = UCHARDATA(mask);
    for (x = 0; x < N; x++) if (M[x]) H[SOURCE2[x] * rsh + SOURCE1[x]] += 1;
  }
  return(1);
}  /* lhisto2() */

/* ==================================== */
int32_t lhistolong(struct xvimage *image, struct xvimage *mask, index_t **histo, int32_t *size)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lhistolong"
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ds = depth(image);       /* nombre plans */
  index_t nb = nbands(image);      /* nombre bandes */
  index_t N = rs * cs * ds * nb;   /* taille image */
  int32_t *F = SLONGDATA(image);      /* l'image de depart */
  uint8_t *M;
  int32_t s;

  s = F[0];
  for (x = 1; x < N; x++) 
    if ((int32_t)(F[x]) > s) 
      s = (int32_t)(F[x]);
  s += 1;      /* pour la valeur 0 */
  *size = s;

  *histo = (index_t *)calloc(s, sizeof(index_t));
  if (*histo == NULL)
  {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    return 0;
  }

  if (mask == NULL)
  {
    for (x = 0; x < N; x++) (*histo)[F[x]] += 1;
  }
  else
  {
    M = UCHARDATA(mask);
    for (x = 0; x < N; x++) if (M[x]) (*histo)[F[x]] += 1;
  }

  return(1);
} // histolong()

/* ==================================== */
int32_t lhistofloat(struct xvimage *image, struct xvimage *mask, index_t **histo, int32_t *size, 
		float *Sincr, float *Smin, float *Smax)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lhistofloat"
#define NBINS 256
  int32_t i;
  index_t x;                       /* index muet de pixel */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t ds = depth(image);       /* nombre plans */
  index_t nb = nbands(image);      /* nombre bandes */
  index_t N = rs * cs * ds * nb;   /* taille image */
  float *F = FLOATDATA(image);     /* l'image de depart */
  uint8_t *M;
  float smin, smax, s, sincr;

  if (mask == NULL)
  {
    smin = smax = F[0];
    for (x = 1; x < N; x++) 
      if (F[x] > smax) smax = F[x]; else
	if (F[x] < smin) smin = F[x];
  }
  else
  {
    M = UCHARDATA(mask);
    for (x = 0; x < N; x++) 
      if (M[x])
      { smin = smax = F[x]; break; }
    for (; x < N; x++) 
      if (M[x])
      {
	if (F[x] > smax) smax = F[x]; else
	  if (F[x] < smin) smin = F[x];
      }
  } // else mask == NULL

#ifdef VERBOSE
  printf("%s: min=%g ; max=%g\n", F_NAME, smin, smax);
#endif  

  *size = NBINS;
  *Sincr = sincr = (smax - smin) / NBINS;
  *Smin = smin;
  *Smax = smax;

  *histo = (index_t *)calloc(NBINS, sizeof(index_t));
  if (*histo == NULL)
  {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    return 0;
  }

  if (mask == NULL)
    for (x = 0; x < N; x++)
    {
      for (s = smin, i = 0; i < NBINS; s += sincr, i++)
	if (F[x] < s) break;
      (*histo)[i-1] += 1;
    }
  else
  {
    M = UCHARDATA(mask);
    for (x = 0; x < N; x++)
    {
      if (M[x])
      {
	for (s = smin, i = 0; i < NBINS; s += sincr, i++)
	  if (F[x] < s) break;
	(*histo)[i-1] += 1;
      }
    }
  }

  return(1);
} // lhistofloat()

/* ==================================== */
void lhistcompact(index_t *histo, int32_t n)
/* ==================================== */
/* n niveaux de gris consecutifs sont compactes en un seul */
{
  int32_t i, t, i0 = -1;

  for (i = NDG_MIN; i <= NDG_MAX; i++) 
  {
    if ((i % n) == 0) 
    { 
      if (i0 != -1) histo[i0] = t; 
      t = histo[i]; 
      i0++; 
      histo[i0] = 0;
    } 
    else
      t += histo[i]; 
  }
  histo[i0] += t;
  for (i = i0 + 1; i <= NDG_MAX; i++) histo[i] = 0;
}

/* ==================================== */
int32_t lhistsum(index_t *histo)
/* ==================================== */
{
  int32_t i, t;

  t = 0;
  for (i = NDG_MIN; i <= NDG_MAX; i++) t += histo[i]; 
  return t;
}

/* ==================================== */
int32_t lhistdilat(index_t *histo, int32_t n)
/* ==================================== */
/* dilatation numerique par un es. plan, centre, de taille 2n+1 */
{
#undef F_NAME
#define F_NAME "lhistdilat"
  int32_t i, j, m;
  index_t *h;

  h = (index_t *)calloc(1,(NDG_MAX + 1) * sizeof(index_t));
  if (h == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return 0;
  }

  for (i = NDG_MIN; i <= NDG_MAX; i++) 
  {
    m = histo[i];
    for (j = 1; j <= n; j++)
    {
      if (((i - j) >= NDG_MIN) && (histo[i - j] > m)) m = histo[i - j];
      if (((i + j) <= NDG_MAX) && (histo[i + j] > m)) m = histo[i + j];
    }
    h[i] = m; 
  }
  for (i = NDG_MIN; i <= NDG_MAX; i++) histo[i] = h[i];
  free(h);
  return 1;
}

/* ==================================== */
void labelextr1d(int32_t *F, int32_t n, uint8_t *E)
/* ==================================== */
/*
  renvoie dans E les etiquettes des points de F : 
    1 : MAX
    2 : MIN
    3 : ni MAX ni MIN
*/
{
  int32_t i;
  for (i = 0; i < n; i++) E[i] = 0;
  E[0] = NONMIN | NONMAX;
  for (i = 1; i < n; i++) 
    if (F[i] < F[i-1])      E[i] |= NONMAX;
    else if (F[i] > F[i-1]) E[i] |= NONMIN;
    else                    E[i] |= E[i-1];
  E[n-1] = NONMIN | NONMAX;
  for (i = n-2; i >= 0; i--)
    if (F[i] < F[i+1])      E[i] |= NONMAX;
    else if (F[i] > F[i+1]) E[i] |= NONMIN;
    else                    E[i] |= E[i+1];
} /* labelextr1d() */

/* ==================================== */
int32_t lhisto2image(index_t *histo, int32_t size, struct xvimage **image)
/* ==================================== */
/*
  Genere une image 2d representant un histogramme. 
  Les barres verticales representent les "bins",
  les barres horizontales donnent l'echelle (1 barre tous les 1000, 10000... pixels)
*/
{
#undef F_NAME
#define F_NAME "lhisto2image"
  int32_t i, j, hmax, hbin, tailleclasse;
  int32_t rs, cs;
  uint8_t *I;
  int32_t interbar;
  const int32_t SizeX=512;
  const int32_t SizeY=256;

  // Creation d'une image pour l'histogramme
  if (size<SizeX)
    rs = size;
  else
    rs = SizeX;
  cs = SizeY;
  *image = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  if (*image == NULL)
  {   fprintf(stderr,"%s: allocimage failed\n", F_NAME);
      return 0;
  }
  I = UCHARDATA((*image));
  memset(I, NDG_MAX, rs*cs);

  // calcule la taille d'une classe
  tailleclasse=size/rs;
  // Compactifie l'histogramme (diminue le nb de classes)
  if (tailleclasse != 1) {
    for (i=0; i<rs; i++) {
      for (j=0; j<tailleclasse; j++) {
	histo[i] = histo[i*tailleclasse+j];
      }
    }
    size = rs;
  }

  // calcule la hauteur max d'un bin
  hmax = 0;
  for (i = 0; i < size; i++) if (histo[i] > hmax) hmax = histo[i];
  if (hmax <= 50000) interbar = 1000;
  else if (hmax <= 500000) interbar = 10000;
  else interbar = 100000;

  for (i = 0; i < size; i++) // trace les bins
  {
    hbin = (int32_t)(((double)histo[i] * (double)(SizeY-6)) / (double)hmax);

    for (j = 0; j < hbin; j++)
      I[(cs-j-1)*rs + i] = NDG_MIN;
  } // for (i = 0; i <= NDG_MAX; i++)
  
  i = 1;
  hbin = (int32_t)(((double)interbar * (double)i * (double)(SizeY-6)) / (double)hmax);
  while (hbin < SizeY)
  {
    for (j = 0; j <size; j++)
    I[(cs-hbin-1)*rs + j] = NDG_MIN;
    i++;
    hbin = (int32_t)(((double)interbar * (double)i * (double)(SizeY-6)) / (double)hmax);
  }

#ifdef VERBOSE
  printf("interbar = %d pixels\n", interbar);
#endif

  return(1);
} /* lhisto2image() */

/* ========================================================= */
/* ========================================================= */
/* ========================================================= */
// Pour les histogrammes d'orientations
/* ========================================================= */
/* ========================================================= */
/* ========================================================= */

#define ORIENT_EPS 1e-20

/* ==================================== */
static double azimuth(float x, float y) 
/* ==================================== */
// calcule l'azimuth d'un vecteur dont les coordonnées x, y sont en paramètres
// résultat : angle en degrés entre 0 et 180 (ou -1 si vecteur trop petit)
{
  double phi, cosphi;
	
  if ((mcabs(x) < ORIENT_EPS) && (mcabs(y) < ORIENT_EPS))
    return -1;
  else
  {
    cosphi=(x / sqrt(x*x+y*y));
    phi= acos (cosphi);
    //    printf("azimmmm: %f\n", (180*phi/M_PI));
    return (180*phi/M_PI);
  }
	
}

/* ==================================== */
static double elevation(float x, float y, float z) 
/* ==================================== */
// calcule l'élévation d'un vecteur dont les coordonnées x, y, z sont en paramètres
// résultat : angle en degrés entre 0 et 180 (ou -1 si vecteur trop petit)
{
  double theta, sintheta;
	
  if ((mcabs(x) < ORIENT_EPS) && (mcabs(y) < ORIENT_EPS) && (mcabs(z) < ORIENT_EPS))
    return -1;
  else
  {
    sintheta=(z / sqrt( x*x + y*y + z*z ));	
    theta= asin(sintheta);
    //printf("elevvv: %f\n", (180*theta/M_PI)+90);
    return (180*theta/M_PI+90);
  } 
}

/* ==================================== */
int32_t lhistoazimuth(struct xvimage * field, int32_t nbins, index_t **histo)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lhistoazimuth"
  index_t rs = rowsize(field);     /* taille ligne */
  index_t cs = colsize(field);     /* taille colonne */
  index_t ds = depth(field);       /* nombre plans */
  index_t ps = rs * cs;            /* taille plan */
  index_t N = ps * ds;             /* taille image */
  float * F;
  int32_t bin; 
  index_t x, y, z; 
  double az, wbin = 180.0/nbins;

  F = FLOATDATA(field);
  *histo = (index_t *)calloc(nbins, sizeof(index_t));
  assert(*histo != NULL);
  assert(datatype(field) == VFF_TYP_FLOAT);

  if (ds == 1)
  {
    assert(nbands(field) == 2);
    for (y=0; y<cs; y++)
    for (x=0; x<rs; x++)
    {   
      az = azimuth(F[y*rs+x], F[y*rs+x+N]);
      if (az >= 0)
      {
	bin = (int32_t)floor(az/wbin);
	(*histo)[bin] = (*histo)[bin] + 1;
      }
    }
  }
  else
  {
    assert(nbands(field) == 3);
    for (z=0; z<ds; z++)
    for (y=0; y<cs; y++)
    for (x=0; x<rs; x++)
    {   
      az = azimuth(F[z*ps+y*rs+x], F[z*ps+y*rs+x+N]);
      if (az >= 0)
      {
	bin = (int32_t)floor(az/wbin);
	(*histo)[bin] = (*histo)[bin] + 1;
      }
    }
  }
  return(1);
} // lhistoazimuth()

/* ==================================== */
int32_t lhistoelevation(struct xvimage * field, int32_t nbins, index_t **histo)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lhistoelevation"
  index_t rs = rowsize(field);     /* taille ligne */
  index_t cs = colsize(field);     /* taille colonne */
  index_t ds = depth(field);       /* nombre plans */
  index_t ps = rs * cs;            /* taille plan */
  index_t N = ps * ds;             /* taille image */
  float * F;
  int32_t bin; 
  index_t x, y, z; 
  double ev, wbin = 180.0/nbins;

  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 3);
  F = FLOATDATA(field);

  *histo = (index_t *)calloc(nbins, sizeof(index_t));
  assert(*histo != NULL);

  for (z=0; z<ds; z++)
  for (y=0; y<cs; y++)
  for (x=0; x<rs; x++)
  {   
    ev = elevation(F[z*ps+y*rs+x], F[z*ps+y*rs+x+N], F[z*ps+y*rs+x+N+N]);
    if (ev >= 0)
    {
      bin = (int32_t)floor(ev/wbin);
      (*histo)[bin] = (*histo)[bin] + 1;
    }
  }
  return(1);
} // lhistoelevation()

/* ==================================== */
int32_t lhisto_distance_modulo_raw (index_t * A, index_t * B, int32_t n)
/* ==================================== */
{ //traduit (en gros) le nombre de deplacements necessaires pour pouvoir obtenir l'histogramme B a partir de l'histogramme A
  int32_t i, j, h_dist, h_dist2, d ;
  int32_t prefixsum[n], temp[n];
	
  prefixsum[0] = A[0] - B[0];
  h_dist = mcabs(prefixsum[0]);
  for (i=1; i<n; i++)
  {
    prefixsum[i] = prefixsum[i-1] + A[i] - B[i];
    h_dist = h_dist + mcabs(prefixsum[i]);
  }
  
  for (;;)
  {
    d = 0;
    for (j=0; j<n; j++)
      if (prefixsum[j] > 0) { d = prefixsum[j]; break; }
    for (; j<n; j++)
      if ((prefixsum[j] > 0) && (prefixsum[j] < d))
	d = prefixsum[j];
    h_dist2 = 0;
    for (j=0; j<n; j++)
    {
      temp[j]= prefixsum [j] - d;
      h_dist2 = h_dist2+ mcabs(temp[j]);
    }
    if (h_dist2 < h_dist)
    {
      h_dist = h_dist2 ;
      for(j=0; j<n; j++)
	prefixsum[j]=temp[j];
    }
    else{break;}
  }
  
  for (;;)
  {
    d = 0;
    for (j=0; j<n; j++)
      if (prefixsum[j] < 0) { d = prefixsum[j]; break; }
    for (; j<n; j++)
      if ((prefixsum[j] < 0) && (prefixsum[j] > d))
	d= prefixsum[j];
    h_dist2 = 0;
    for (j=0; j<n; j++)
    {
      temp[j]= prefixsum [j] - d;
      h_dist2 = h_dist2+ mcabs(temp[j]);
    }
    if (h_dist2 < h_dist)
    {
      h_dist = h_dist2 ;
      for(j=0; j<n; j++)
	prefixsum[j]=temp[j];
    }
    else{break;}
  }
  return h_dist;
} // lhisto_distance_modulo_raw()

/* ==================================== */
int32_t lhisto_distance_ordinal_raw (index_t * A, index_t * B, int32_t n)
/* ==================================== */
{ //traduit (en gros) le nombre de deplacements necessaires pour pouvoir obtenir l'histogramme B a partir de l'histogramme A
  int32_t i, h_dist;
  int32_t prefixsum;
	
  prefixsum = 0;
  h_dist = 0;
  
  for (i=0; i<n; i++)
  {
    prefixsum = prefixsum + A[i] - B[i];
    h_dist = h_dist + mcabs(prefixsum);
  }
	
  return h_dist;
} // lhisto_distance_ordinal_raw()

/* ==================================== */
double lhisto_distance_modulo (index_t * A, index_t * B, int32_t n)
/* ==================================== */
{ 
// traduit (en gros) le nombre de deplacements necessaires pour pouvoir obtenir l'histogramme B a partir de l'histogramme A
// version normalisee
  int32_t i, j;
  double h_dist, h_dist2, d ;
  double prefixsum[n], temp[n];
  double nA, nB;

  for (i=0, nA=nB=0; i<n; i++) { nA += A[i]; nB += B[i]; }

  prefixsum[0] = nB*A[0] - nA*B[0];
  h_dist = mcabs(prefixsum[0]);
  for (i=1; i<n; i++)
  {
    prefixsum[i] = prefixsum[i-1] + nB*A[i] - nA*B[i];
    h_dist = h_dist + mcabs(prefixsum[i]);
  }
  
  for (;;)
  {
    d = 0;
    for (j=0; j<n; j++)
      if (prefixsum[j] > 0) { d = prefixsum[j]; break; }
    for (; j<n; j++)
      if ((prefixsum[j] > 0) && (prefixsum[j] < d))
	d = prefixsum[j];
    h_dist2 = 0;
    for (j=0; j<n; j++)
    {
      temp[j]= prefixsum [j] - d;
      h_dist2 = h_dist2+ mcabs(temp[j]);
    }
    if (h_dist2 < h_dist)
    {
      h_dist = h_dist2 ;
      for(j=0; j<n; j++)
	prefixsum[j]=temp[j];
    }
    else{break;}
  }
  
  for (;;)
  {
    d = 0;
    for (j=0; j<n; j++)
      if (prefixsum[j] < 0) { d = prefixsum[j]; break; }
    for (; j<n; j++)
      if ((prefixsum[j] < 0) && (prefixsum[j] > d))
	d= prefixsum[j];
    h_dist2 = 0;
    for (j=0; j<n; j++)
    {
      temp[j]= prefixsum [j] - d;
      h_dist2 = h_dist2+ mcabs(temp[j]);
    }
    if (h_dist2 < h_dist)
    {
      h_dist = h_dist2 ;
      for(j=0; j<n; j++)
	prefixsum[j]=temp[j];
    }
    else{break;}
  }
  return h_dist / (nA * nB);
} // lhisto_distance_modulo()

/* ==================================== */
double lhisto_distance_ordinal (index_t * A, index_t * B, int32_t n)
/* ==================================== */
{ //traduit (en gros) le nombre de deplacements necessaires pour pouvoir obtenir l'histogramme B a partir de l'histogramme A
  int32_t i;
  double h_dist, prefixsum;
  double nA, nB;

  for (i=0, nA=nB=0; i<n; i++) { nA += A[i]; nB += B[i]; }
	
  prefixsum = 0;
  h_dist = 0;
  
  for (i=0; i<n; i++)
  {
    prefixsum = prefixsum + nB*A[i] - nA*B[i];
    h_dist = h_dist + mcabs(prefixsum);
  }
	
  return h_dist / (nA * nB);
} // lhisto_distance_ordinal()

/* ==================================== */
int32_t lseuilhisto (struct xvimage *image, struct xvimage *masque, double p)
/* ==================================== */
// trouve le seuil s tel que image seuillée à s comporte (à peu près) une proportion p de pixels nuls, et applique ce seuil à image. Si le masque est non NULL, cette opération ne concerne que les points non nuls de masque. 
{  
#undef F_NAME
#define F_NAME "lseuilhisto"
  index_t rs = rowsize(image);
  index_t cs = colsize(image);
  index_t ds = depth(image);
  index_t i, nbpts, n, N = rs * cs * ds;
  uint8_t *I = UCHARDATA(image);
  index_t * histo;
  int32_t seuil;
  uint8_t *M;

#ifdef DEBUG_lseuilhisto
  printf("%s: p=%g\n", F_NAME, p);
#endif
  assert(p >= 0); assert(p <= 1);
 
  histo = (index_t *)calloc((NDG_MAX - NDG_MIN + 1), sizeof(index_t));
  assert(histo != NULL);

  if (masque != NULL)
  {
    M = UCHARDATA(masque);
    nbpts = 0;
    for (i = 0; i < N; i++) if (M[i]) { histo[I[i]]++; nbpts++; }
  }
  else
  {
    for (i = 0; i < N; i++) histo[I[i]]++;
    nbpts = N;
  }
  
  nbpts = (int32_t)(p * nbpts);
  n = 0;
  for (i = NDG_MIN; i <= NDG_MAX; i++) 
  {
    n += histo[i];
    if (n >= nbpts) { seuil = i; break; }
  }

  for (i = 0; i < N; i++) if (I[i] >= seuil) I[i] = NDG_MAX; else I[i] = NDG_MIN;
  free(histo);
  return 1;
} // lseuilhisto()

/* ==================================== */
int32_t lcountvalues(struct xvimage *image, struct xvimage *mask)
/* ==================================== */
#undef F_NAME
#define F_NAME "lcountvalues"
{
  int32_t count = 0;
  index_t *histo;
  int32_t i, s;

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    histo = (index_t *)calloc(1,(NDG_MAX - NDG_MIN + 1) * sizeof(index_t));
    if (histo == NULL)
    {
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return -1;
    }
    if (! lhisto(image, mask, histo))
    {
      fprintf(stderr, "%s: function lhisto failed\n", F_NAME);
      return -1;
    }
    for (i = NDG_MIN; i <= NDG_MAX; i++) if (histo[i]) count++;
    free(histo);
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    if (! lhistolong(image, mask, &histo, &s))
    {
      fprintf(stderr, "%s: function lhistolong failed\n", F_NAME);
      return -1;
    }
    for (i = 0; i < s; i++) if (histo[i]) count++;
    free(histo);
  }
  return(count);
} /* lcountvalues() */
