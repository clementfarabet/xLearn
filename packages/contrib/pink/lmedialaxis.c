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
Fonctions pour l'axe médian exact ou approché, et
pour la fonction bissectrice.

Michel Couprie - mai 2002
Rita Zrour - juin 2004
  axe médian euclidien exact (algo de Rémy-Thiel) et fonction bissectrice (lmedialaxis_lbisector_Rita)
Michel Couprie - juillet 2004 - révision lmedialaxis_lbisector
Laurent Najman - lmedialaxis_lbisector_talbot
Michel Couprie - juillet 2004 - révision lmedialaxis_lbisector_talbot
Michel Couprie - août 2005 - variante lmedialaxis_lbisector Coeurjolly 
Michel Couprie - novembre 2008 - lambda medial axis
*/

//#define OLD

//#define CHRONO

#define MODIF_MC
#define MEDAX_MEYER
//#define MEDAX_MC
#define VARIANTE_GB
//#define VARIANTE_DC

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#ifdef HP
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <ltopotypes.h>
#include <mcutil.h>
#include <mcgeo.h>
#include <ldist.h>
#include <lmedialaxis.h>
#include <lballincl.h>

#ifdef CHRONO
#include <mcchrono.h>
#endif

//#define DEBUG

//#define COUNT
#ifdef COUNT
int32_t countds = 0;
double sumpoints = 0;
#endif

//#define STATVOR

#ifdef STATVOR
int32_t countvor = 0;
int32_t countpts = 0;
int32_t countmult = 0;
int32_t maxdwnstr = 0;
#endif

//#define HISTVOR

#ifdef HISTVOR
#define sizevorhisto 100
int32_t vorhisto[sizevorhisto];
int32_t extvorhisto[sizevorhisto];
#endif

//#define VERBOSE

//#define EXACT
//ATTENTION: algorithme naif extremement couteux !

#define INFINI 1000000000

static double norm(vect2Dint v) { return sqrt((double)(v.x*v.x + v.y*v.y)); }
static double pscal(vect2Dint u, vect2Dint v) { return (double)(u.x*v.x + u.y*v.y); }

/* ==================================== */
int32_t lmedialaxis_lmedax_talbot(struct xvimage *img,   /* donnee: image binaire */       
                  struct xvimage *res    /* resultat: centres des boules max (approx) */
)
/* ==================================== */
/* 
  ********** NON VALIDE ***********
  Algo de H. Talbot
  Les points de l'axe median sont values par la distance au complementaire
  Le calcul des vecteurs pointant sur le plus proche point de bord est fait
  suivant l'algo de Danielsson 4SED
*/
#undef F_NAME
#define F_NAME "lmedialaxis_lmedax_talbot"
{ 
  int32_t rs = img->row_size;
  int32_t cs = img->col_size;
  int32_t N= rs * cs;            /* taille de l'image */
  uint8_t *F;          /* pointeur sur l'image */
  uint32_t *M;          /* donnees de l'image resultat */
  vect2Dint *L;              /* tableau de vecteur associe a un point de l'image */
  int32_t h, i, j, k;
  vect2Dint v1;
  double cosmax, tmp, norm_i, norm_j, dij, sqrt_2 = sqrt(2), cosB = cos(M_PI/8);

  if (depth(img) != 1)
  {
    fprintf(stderr, "%s: only for 2D images\n", F_NAME);
    return 0;
  }

#define INFINI 1000000000

  L = (vect2Dint *)calloc(1,N*sizeof(vect2Dint));
  F = UCHARDATA(img);
  for (i = 0; i < N; i++) 
    F[i] = 255-F[i];
  if (! ldistvect(F, L, rs, cs))
  {
    fprintf(stderr, "%s: ldistvect failed\n", F_NAME);
    exit(1);
  }
  for (i = 0; i < N; i++) 
    F[i] = 255-F[i];

  M = ULONGDATA(res);
  for (i = 0; i < N; i++) 
    M[i] = norm(L[i]);

  M = ULONGDATA(res);
  for (i = 0; i < N; i++) if (F[i])
  {
    printf("i=(%d,%d); L[i]=(%d,%d)\n", i%rs, i/rs, L[i].x, L[i].y);
    norm_i = norm(L[i]);
    // trouver j, le voisin de i dans la direction la plus proche de L[i]
    // (aval de i)
    // angle min <=> cos max
    cosmax = -2.0;
    for (k = 0; k < 8; k += 1)
    {
      h = voisin(i, k, rs, N);
      if ((h != -1) && (norm_i>norm(L[h])))
      { 
        v1.x = h%rs - i%rs; 
        v1.y = h/rs - i/rs;
        tmp = pscal(v1, L[i]) / (norm(v1) * norm_i);
	//printf("scal = %lg, n1 = %lg, n2 = %lg, tmp = %lg - ", pscal(v1,L[i]), norm(v1), norm_i, tmp);
        if (tmp > cosmax) { 
	  cosmax = tmp; 
	  j = k; 
	  if (k%2) 
	    dij = sqrt_2; 
	  else dij = 1.0; 
	}
      }
    } // for (k = 0; k < 8; k += 1)
    j = (j + 4) % 8; // cherche l'amont dans la direction opposee
    j = voisin(i, j, rs, N);
    norm_j = norm(L[j]);
    printf("j=(%d,%d); norm_j = %g  norm_i = %g  cosmax = %g; dij = %g; d*c = %g\n", 
        j%rs, j/rs, norm_j, norm_i, cosmax, dij, dij*cosB);
    if ((double)(norm_j - norm_i) < ((double)(dij) * cosB)) 
      //M[i] = L[i].x * L[i].x + L[i].y * L[i].y;
      M[i]=255;
    else 
      M[i] = 0;
  }
  free(L);
  return(1);
} // lmedialaxis_lmedax_talbot()

/* ==================================== */
int32_t lmedialaxis_lmedax_meyer(struct xvimage *img,   /* donnee: image binaire */       
                 struct xvimage *res    /* resultat: centres des boules max (approx) */
)
/* ==================================== */
/* 
  Methode de Meyer (approximation)
*/
#undef F_NAME
#define F_NAME "lmedialaxis_lmedax_meyer"
{ 
  int32_t rs = img->row_size;
  int32_t cs = img->col_size;
  int32_t N= rs * cs;            /* taille de l'image */
  uint8_t *F = UCHARDATA(img);          /* pointeur sur l'image */
  uint32_t *M = ULONGDATA(res);          /* donnees de l'image resultat */
  uint32_t *D;          /* distance */
  uint32_t *H;          /* tampon */
  struct xvimage *inv;
  uint8_t *I;          /* image inverse */
  int32_t x, y, k;
  uint32_t inf, sup;
  int32_t incr_vois = 2;

  if (depth(img) != 1)
  { fprintf(stderr, "%s: only for 2D images\n", F_NAME);
    return 0;
  }
  inv = copyimage(img);
  I = UCHARDATA(inv);
  for (x = 0; x < N; x++) if (I[x]) I[x] = 0; else I[x] = NDG_MAX;
  if (! ldisteuc(inv, res)) // calcule la fonction distance
  {
    fprintf(stderr, "%s: ldist failed\n", F_NAME);
    return(0);
  }
  freeimage(inv);
  H = (uint32_t *)calloc(1,N*sizeof(int32_t));
  if (H == NULL)
  { fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
    return(0);
  }
  D = (uint32_t *)calloc(1,N*sizeof(int32_t));
  if (D == NULL)
  { fprintf(stderr,"%s() : malloc failed for D\n", F_NAME);
    return(0);
  }
  memcpy(D, M, N*sizeof(int32_t));

  // erosion unitaire
  memcpy(H, M, N*sizeof(int32_t));
  for (x = 0; x < N; x++)
  {
    inf = H[x]; /* l'ES est reflexif */
    for (k = 0; k < 8; k += incr_vois)
    {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (H[y] < inf)) inf = H[y];
    } /* for k */
    M[x] = inf;
  }
  // dilatation unitaire
  memcpy(H, M, N*sizeof(int32_t));
  for (x = 0; x < N; x++)
  {
    sup = H[x]; /* l'ES est reflexif */
    for (k = 0; k < 8; k += incr_vois)
    {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (H[y] > sup)) sup = H[y];
    } /* for k */
    M[x] = sup;
  }
  free(H);
  // detection des centres de BM
  for (x = 0; x < N; x++)
    if (F[x] && (M[x] < D[x])) M[x] = D[x]; else M[x] = 0;  
  free(D);
  return(1);
} // lmedialaxis_lmedax_meyer()

/* ==================================== */
int32_t lmedialaxis_lmedax_meyer3d(struct xvimage *img,   /* donnee: image binaire */       
                   struct xvimage *res    /* resultat: centres des boules max (approx) */
)
/* ==================================== */
/* 
  Methode de Meyer (approximation)
*/
#undef F_NAME
#define F_NAME "lmedialaxis_lmedax_meyer3d"
{ 
  int32_t rs = rowsize(img);
  int32_t cs = colsize(img);
  int32_t ds = depth(img);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;                            /* taille de l'image */
  uint8_t *F = UCHARDATA(img);          /* pointeur sur l'image */
  uint32_t *M = ULONGDATA(res);          /* donnees de l'image resultat */
  uint32_t *D;          /* distance */
  uint32_t *H;          /* tampon */
  struct xvimage *inv;
  uint8_t *I;          /* image inverse */
  int32_t x, y, k;
  uint32_t inf, sup;

  inv = copyimage(img);
  I = UCHARDATA(inv);
  for (x = 0; x < N; x++) if (I[x]) I[x] = 0; else I[x] = NDG_MAX;
  if (! ldisteuc3d(inv, res)) // calcule la fonction distance
  {
    fprintf(stderr, "%s: ldist failed\n", F_NAME);
    return(0);
  }
  freeimage(inv);
  H = (uint32_t *)calloc(1,N*sizeof(int32_t));
  if (H == NULL)
  { fprintf(stderr,"%s() : malloc failed for H\n", F_NAME);
    return(0);
  }
  D = (uint32_t *)calloc(1,N*sizeof(int32_t));
  if (D == NULL)
  { fprintf(stderr,"%s() : malloc failed for D\n", F_NAME);
    return(0);
  }
  memcpy(D, M, N*sizeof(int32_t));

  // erosion unitaire
  memcpy(H, M, N*sizeof(int32_t));
  for (x = 0; x < N; x++)
  {
    inf = H[x]; /* l'ES est reflexif */
    for (k = 0; k <= 10; k += 2)
    {
        y = voisin6(x, k, rs, ps, N);
        if ((y != -1) && (H[y] < inf)) inf = H[y];
    } /* for k */
    M[x] = inf;
  }
  // dilatation unitaire
  memcpy(H, M, N*sizeof(int32_t));
  for (x = 0; x < N; x++)
  {
    sup = H[x]; /* l'ES est reflexif */
    for (k = 0; k <= 10; k += 2)
    {
        y = voisin6(x, k, rs, ps, N);
        if ((y != -1) && (H[y] > sup)) sup = H[y];
    } /* for k */
    M[x] = sup;
  }
  free(H);
  // detection des centres de BM
  for (x = 0; x < N; x++)
    if (F[x] && (M[x] < D[x])) M[x] = D[x]; else M[x] = 0;  
  free(D);
  return(1);
} // lmedialaxis_lmedax_meyer3d()

/* ==================================== */
int32_t lmedialaxis_lmedax_mc(struct xvimage *f)
/* ==================================== */
#undef F_NAME
#define F_NAME "lmedialaxis_lmedax_mc"
{
  int32_t i;
  int32_t rs = rowsize(f);        /* taille ligne f */
  int32_t cs = colsize(f);        /* taille colonne f */
  int32_t ds = depth(f);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;            /* taille image f */
  uint8_t *F = UCHARDATA(f);
  struct xvimage *tmp1 = copyimage(f);
  struct xvimage *tmp2 = copyimage(f);
  struct xvimage *res = copyimage(f);
  uint8_t *T1 = UCHARDATA(tmp1);
  uint8_t *T2 = UCHARDATA(tmp2);
  uint8_t *R = UCHARDATA(res);
  struct xvimage *dist;
  uint32_t *D;
  int32_t r, rmax = 0;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(0);
  }
  D = ULONGDATA(dist);

  if (ds == 1)
  {
      for (i = 0; i < N; i++) if (F[i]) F[i] = 0; else F[i] = NDG_MAX; // inverse
      ldisteuc(f, dist);
      for (i = 0; i < N; i++) if (F[i]) F[i] = 0; else F[i] = NDG_MAX; // inverse
      for (i = 0; i < N; i++) if (D[i] > rmax) rmax = D[i];
      freeimage(dist);
#ifdef VERBOSE
        printf("rmax = %d\n", rmax);
#endif

      memset(R, 0, N);
      for (r = 0; r < rmax; r++)
      {
#ifdef VERBOSE
        printf("r = %d\n", r);
#endif
        if (r+1 > NDG_MAX)
        {   
          fprintf(stderr, "%s: ball radius > %d\n", F_NAME, NDG_MAX);
          return(0);
        }
        for (i = 0; i < N; i++) T1[i] = NDG_MAX;
#ifdef EXACT
	// ATTENTION: algorithme naif extremement couteux !
        for (k = 1; k <= rmax; k++)
	{
          copy2image(tmp2, f);
          lopendisc(tmp2, r+k, 0);
          for (i = 0; i < N; i++) if (T2[i]) T2[i] = NDG_MIN; else T2[i] = NDG_MAX; // inverse
          ldilatdisc(tmp2, r, 0);
          for (i = 0; i < N; i++) T1[i] = T1[i] && T2[i];
	}
        for (i = 0; i < N; i++) T2[i] = T1[i];
#else
        copy2image(tmp2, f);
        lopendisc(tmp2, r+1, 0);
        for (i = 0; i < N; i++) if (T2[i]) T2[i] = NDG_MIN; else T2[i] = NDG_MAX; // inverse
        ldilatdisc(tmp2, r, 0);
#endif
        copy2image(tmp1, f);
        lerosdisc(tmp1, r, 0);
        for (i = 0; i < N; i++) 
          if (T1[i] && T2[i]) // intersection
            R[i] = r+1;
      } // for r
  }
  else // 3D
  {
      for (i = 0; i < N; i++) if (F[i]) F[i] = 0; else F[i] = NDG_MAX; // inverse
      ldisteuc3d(f, dist);
      for (i = 0; i < N; i++) if (F[i]) F[i] = 0; else F[i] = NDG_MAX; // inverse
      for (i = 0; i < N; i++) if (D[i] > rmax) rmax = D[i];
      freeimage(dist);
#ifdef VERBOSE
        printf("rmax = %d\n", rmax);
#endif

      memset(R, 0, N);
      for (r = 0; r < rmax; r++)
      {
#ifdef VERBOSE
        printf("r = %d\n", r);
#endif
        if (r+1 > NDG_MAX)
        {   
          fprintf(stderr, "%s: ball radius > %d\n", F_NAME, NDG_MAX);
          return(0);
        }
        for (i = 0; i < N; i++) T1[i] = NDG_MAX;
#ifdef EXACT
	// ATTENTION: algorithme naif extremement couteux !
        for (k = 1; k <= rmax; k++)
	{
          copy2image(tmp2, f);
          lopenball(tmp2, r+k, 0);
          for (i = 0; i < N; i++) if (T2[i]) T2[i] = NDG_MIN; else T2[i] = NDG_MAX; // inverse
          ldilatball(tmp2, r, 0);
          for (i = 0; i < N; i++) T1[i] = T1[i] && T2[i];
	}
        for (i = 0; i < N; i++) T2[i] = T1[i];
#else
        copy2image(tmp2, f);
        lopenball(tmp2, r+1, 0);
        for (i = 0; i < N; i++) if (T2[i]) T2[i] = NDG_MIN; else T2[i] = NDG_MAX; // inverse
        ldilatball(tmp2, r, 0);
#endif
        copy2image(tmp1, f);
        lerosball(tmp1, r, 0);
        for (i = 0; i < N; i++) 
          if (T1[i] && T2[i]) // intersection
            R[i] = r+1;
      } // for r
  }

  copy2image(f, res);  
  freeimage(tmp1);
  freeimage(tmp2);
  freeimage(res);
  return 1;
} // lmedialaxis_lmedax_mc()


/* ==================================== */
/* ==================================== */
/* ==================================== */

// Data structures and functions for the Remy-Thiel algorithm

/* ==================================== */
/* ==================================== */
/* ==================================== */

/*
from the article: " Exact Medial Axis With Euclidean Distance"
  algorithm to compute the medial axis based on the look-up table. It reads the look-up table from
  the file $PINK/src/tables/TabRemyThiel.txt 
  
  Written by: Rita Zrour
  created: 05/04/2004
  last modified: 29/07/2004
*/

typedef int32_t * LookUpTable;
typedef int32_t * Tabuff;
typedef int32_t * TabDTg;

typedef struct  Weighting{
    int32_t x, y, z, RR;
}  Weighting;

typedef Weighting *MaskG;	      	//used to store Mglut

typedef struct{
    int32_t x, y, z;
} Coordinates;

typedef struct {		//used to store all possible neighbors of (1/8) Z2 or (1/48) Z3
    Coordinates neig[48];
} Neighbors;

////////////// Function definitions

int32_t callcheckR(int32_t distance,MaskG M);
int32_t RadiusMax(uint32_t * gg, int32_t rs, int32_t cs, int32_t ds);
int32_t lmedialaxis_ApplySymmetries(int32_t x, int32_t y, int32_t x1, int32_t y1, int32_t rs, int32_t cs, Neighbors *Mg);
int32_t lmedialaxis_ApplySymmetries3d(int32_t x, int32_t y, int32_t z, int32_t x1, int32_t y1, int32_t z1, int32_t rs, int32_t cs, int32_t ds, Neighbors *Mg);
int32_t CallMedial(int32_t x, int32_t y, uint32_t *image, int32_t rs, int32_t cs, MaskG MgL, LookUpTable Lut, int32_t nbcollut, int32_t rrmax);
int32_t CallMedial3d(int32_t x, int32_t y, int32_t z, uint32_t *image, int32_t rs, int32_t cs, int32_t ds, MaskG MgL, LookUpTable Lut, int32_t nbcollut, int32_t rrmax);

/*
Function that calculates the number of entry we want from MgLut
It takes the value of DT as a criterion to determine how many vectors are needed 
*/

//----------------------------------
int32_t callcheckR(int32_t distance,MaskG M)   
//----------------------------------
{
  int32_t count=0;
  while (distance>=(M[count].RR)) count++;
  return count;
}

/* Function that returns the maximum radius in the distance image*/
//----------------------------------
int32_t RadiusMax(uint32_t * gg, int32_t rs, int32_t cs, int32_t ds)  //rs=width of image  cs=height of image
//----------------------------------
{
    int32_t i, n = rs*cs*ds;
    uint32_t max = 0;
    for (i = 0; i < n; i++) if (gg[i] > max) max = gg[i];
    return max;
}

//----------------------------------
int32_t lmedialaxis_ApplySymmetries(int32_t x, int32_t y, int32_t x1, int32_t y1, int32_t rs, int32_t cs, Neighbors *Mg)
//----------------------------------
// ATTENTION : roles de x et y inversés (cf. Rita)
{
  int32_t vectx[8], vecty[8], i;
  int32_t count=0;

  vectx[0] = x;   vecty[0] = y;
  vectx[1] = y;   vecty[1] = x;
  vectx[2] = y;   vecty[2] = -x;
  vectx[3] = x;   vecty[3] = -y;
  vectx[4] = -x;  vecty[4] = -y;
  vectx[5] = -y;  vecty[5] = -x;
  vectx[6] = -y;  vecty[6] = x;
  vectx[7] = -x;  vecty[7] = y;

  for (i = 0; i <= 7; i++)
  {
    if ((x1 + vectx[i] >= 0) && (x1 + vectx[i] < cs) && // sic
        (y1 + vecty[i] >= 0) && (y1 + vecty[i] < rs))
    {
      Mg->neig[count].x = vectx[i];
      Mg->neig[count].y = vecty[i];
      count=count+1;
    }
  }
  return count;	// this is important to know how many vector are present in Mg
} // lmedialaxis_ApplySymmetries()

//----------------------------------
int32_t lmedialaxis_ApplySymmetries3d(int32_t x, int32_t y, int32_t z, int32_t x1, int32_t y1, int32_t z1, int32_t rs, int32_t cs, int32_t ds, Neighbors *Mg)
//----------------------------------
// ATTENTION : roles de x, y, z inversés (cf. Rita)
{
  int32_t vectx[48], vecty[48], vectz[48], i;
  int32_t count = 0;

  vectx[ 0] = x;   vecty[ 0] = y;   vectz[ 0] = z;
  vectx[ 1] = x;   vecty[ 1] = z;   vectz[ 1] = y;
  vectx[ 2] = y;   vecty[ 2] = x;   vectz[ 2] = z;
  vectx[ 3] = y;   vecty[ 3] = z;   vectz[ 3] = x;
  vectx[ 4] = z;   vecty[ 4] = x;   vectz[ 4] = y;
  vectx[ 5] = z;   vecty[ 5] = y;   vectz[ 5] = x;

  vectx[ 6] = -x;  vecty[ 6] = y;   vectz[ 6] = z;
  vectx[ 7] = -x;  vecty[ 7] = z;   vectz[ 7] = y;
  vectx[ 8] = -y;  vecty[ 8] = x;   vectz[ 8] = z;
  vectx[ 9] = -y;  vecty[ 9] = z;   vectz[ 9] = x;
  vectx[10] = -z;  vecty[10] = x;   vectz[10] = y;
  vectx[11] = -z;  vecty[11] = y;   vectz[11] = x;

  vectx[12] = x;   vecty[12] = -y;  vectz[12] = z;
  vectx[13] = x;   vecty[13] = -z;  vectz[13] = y;
  vectx[14] = y;   vecty[14] = -x;  vectz[14] = z;
  vectx[15] = y;   vecty[15] = -z;  vectz[15] = x;
  vectx[16] = z;   vecty[16] = -x;  vectz[16] = y;
  vectx[17] = z;   vecty[17] = -y;  vectz[17] = x;

  vectx[18] = x;   vecty[18] = y;   vectz[18] = -z;
  vectx[19] = x;   vecty[19] = z;   vectz[19] = -y;
  vectx[20] = y;   vecty[20] = x;   vectz[20] = -z;
  vectx[21] = y;   vecty[21] = z;   vectz[21] = -x;
  vectx[22] = z;   vecty[22] = x;   vectz[22] = -y;
  vectx[23] = z;   vecty[23] = y;   vectz[23] = -x;

  vectx[24] = -x;  vecty[24] = -y;  vectz[24] = z;
  vectx[25] = -x;  vecty[25] = -z;  vectz[25] = y;
  vectx[26] = -y;  vecty[26] = -x;  vectz[26] = z;
  vectx[27] = -y;  vecty[27] = -z;  vectz[27] = x;
  vectx[28] = -z;  vecty[28] = -x;  vectz[28] = y;
  vectx[29] = -z;  vecty[29] = -y;  vectz[29] = x;

  vectx[30] = -x;  vecty[30] = y;   vectz[30] = -z;
  vectx[31] = -x;  vecty[31] = z;   vectz[31] = -y;
  vectx[32] = -y;  vecty[32] = x;   vectz[32] = -z;
  vectx[33] = -y;  vecty[33] = z;   vectz[33] = -x;
  vectx[34] = -z;  vecty[34] = x;   vectz[34] = -y;
  vectx[35] = -z;  vecty[35] = y;   vectz[35] = -x;

  vectx[36] = x;   vecty[36] = -y;  vectz[36] = -z;
  vectx[37] = x;   vecty[37] = -z;  vectz[37] = -y;
  vectx[38] = y;   vecty[38] = -x;  vectz[38] = -z;
  vectx[39] = y;   vecty[39] = -z;  vectz[39] = -x;
  vectx[40] = z;   vecty[40] = -x;  vectz[40] = -y;
  vectx[41] = z;   vecty[41] = -y;  vectz[41] = -x;

  vectx[42] = -x;  vecty[42] = -y;  vectz[42] = -z;
  vectx[43] = -x;  vecty[43] = -z;  vectz[43] = -y;
  vectx[44] = -y;  vecty[44] = -x;  vectz[44] = -z;
  vectx[45] = -y;  vecty[45] = -z;  vectz[45] = -x;
  vectx[46] = -z;  vecty[46] = -x;  vectz[46] = -y;
  vectx[47] = -z;  vecty[47] = -y;  vectz[47] = -x;

  for (i = 0; i < 48; i++)
  {
    if((x1 + vectx[i] >= 0) && (x1 + vectx[i] < ds) && // sic
       (y1 + vecty[i] >= 0) && (y1 + vecty[i] < cs) &&
       (z1 + vectz[i] >= 0) && (z1 + vectz[i] < rs))
    {
      Mg->neig[count].x = vectx[i];
      Mg->neig[count].y = vecty[i];
      Mg->neig[count].z = vectz[i];
      count=count+1;
    }
  }
  return count;		// this is important to know how many vector are present in Mg
} // lmedialaxis_ApplySymmetries3d()

/*This function takes a point in the initial image find all possible neighbors in Mglut;
calls find neighbors to check for each vector in Mglut its neighbors; puts the point in the 
correct position in LutColumn; returns 1 if the point is a medial axis point and 0 if not*/

//----------------------------------
int32_t CallMedial(int32_t x, int32_t y, uint32_t *image, int32_t rs, int32_t cs, MaskG MgL, LookUpTable Lut, int32_t nbcollut, int32_t rrmax)
//----------------------------------
{
  int32_t e, nb, t, j, dist, xx, yy;
  Neighbors MgN1;
  dist = (int32_t)(image[x*rs + y]); // sic
  e = callcheckR(rrmax, MgL);

  for(t = 0; t < e; t++) //t determine the position of the vector in Mglut
  {	
    nb = lmedialaxis_ApplySymmetries(MgL[t].x, MgL[t].y, x, y, rs, cs, &MgN1);
    for(j = 0; j < nb; j++)
    {
      xx = x + MgN1.neig[j].x; 
      yy = y + MgN1.neig[j].y; 
      if (image[xx*rs + yy] >= Lut[t*nbcollut + dist]) return 0;
    }
  }	
  return 1;
} // CallMedial()

//----------------------------------
int32_t CallMedial3d(int32_t x, int32_t y, int32_t z, uint32_t *image, int32_t rs, int32_t cs, int32_t ds, MaskG MgL, LookUpTable Lut, int32_t nbcollut, int32_t rrmax)
//----------------------------------
{
  int32_t e, nb, t, j, dist, xx, yy, zz, ps = rs*cs;
  Neighbors MgN1;
  dist = (int32_t)(image[x*ps + y*rs + z]); // sic
  e = callcheckR(rrmax, MgL);

  for(t = 0; t < e; t++) //t determine the position of the vector in Mglut
  {	
    nb = lmedialaxis_ApplySymmetries3d(MgL[t].x, MgL[t].y, MgL[t].z, x, y, z, rs, cs, ds, &MgN1);
    for(j = 0; j < nb; j++)
    {
      xx = x + MgN1.neig[j].x; 
      yy = y + MgN1.neig[j].y; 
      zz = z + MgN1.neig[j].z; 
      if (image[xx*ps + yy*rs + zz] >= Lut[t*nbcollut + dist]) return 0;
    }
  }	
  return 1;
} // CallMedial3d()

/* ==================================== */
int32_t lmedialaxis_lmedax_Remy_Thiel(struct xvimage *ImageDist, struct xvimage *ImageMedial)
/* ==================================== */
#undef F_NAME
#define F_NAME "lmedialaxis_lmedax_Remy_Thiel"
/*
from the article: " Exact Medial Axis With Euclidean Distance"
  algorithm to compute the medial axis based on the look-up table
  $PINK/src/tables/TabRemyThiel.txt   

  Written by: Rita Zrour
  created: 05/04/2004
  last modified: 24/07/2004
  revision Michel Couprie: 10/08/2004

  Input : ImageDist, an exact squared Euclidean distance map
  Output : ImageMedial, the weighted medial axis
*/
{
  int32_t rs = rowsize(ImageDist);        /* taille ligne f */
  int32_t cs = colsize(ImageDist);        /* taille colonne f */
  int32_t ds = depth(ImageDist);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;            /* taille image f */
  uint32_t *image1;
  uint32_t *imagemedial;
  FILE *fd = NULL;

#ifdef CHRONO
  chrono Chrono1;
  //luts
  start_chrono(&Chrono1);
#endif

  if ((datatype(ImageDist) != VFF_TYP_4_BYTE) || (datatype(ImageMedial) != VFF_TYP_4_BYTE))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  if ((rowsize(ImageMedial) != rs) || (colsize(ImageMedial) != cs) || (depth(ImageMedial) != ds))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    

  image1 = ULONGDATA(ImageDist);
  imagemedial = ULONGDATA(ImageMedial);
  memset(imagemedial, 0, N*sizeof(int32_t));

  if (ds == 1) // 2D
  {
    int32_t i, j, numb, rmax, rknown, distance;
    char tablefilename[512];
    LookUpTable LutColumn1;
    MaskG MgL1;

    sprintf(tablefilename, "%s/src/tables/TabRemyThiel.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table\n", F_NAME);
      return(0);
    }
    fscanf(fd,"%d",&numb); // Number of different directions
    MgL1 = (MaskG)calloc(1,numb * sizeof(struct Weighting));
    if (MgL1 == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < numb; i++)
    {
      fscanf (fd, "%d %d %d", &(MgL1[i].x), &(MgL1[i].y), &(MgL1[i].RR));
    }
    fscanf(fd,"%d",&rknown);
    LutColumn1 = (LookUpTable)calloc(1,numb * rknown * sizeof(int32_t));
    if (LutColumn1 == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < numb; i++)
    {
      for (j = 0; j < rknown; j++)
	fscanf(fd,"%d",&(LutColumn1[i*rknown + j]));
    }
    fclose (fd);

    rmax= RadiusMax(image1, rs, cs, ds);
#ifdef VERBOSE
    printf("Maximum radius in the image=%d\n",rmax);
#endif
    if (rmax > rknown)
    {   
      fprintf(stderr, "%s: rmax = %d > %d (max permitted by current table)\n", F_NAME, rmax, rknown);
      return(0);
    }

#ifdef CHRONO
    //rmax
    printf("%d,", rmax);
    //luts
    printf("%g,", ((double)read_chrono(&Chrono1)) / 1000000.0);
    //ma
    start_chrono(&Chrono1);
#endif

    /*Finding the medial Axis points*/    
    for(i = 0; i < cs; i++) // sic
    {
      for(j = 0; j < rs; j++)
      {
	if(image1[i*rs + j]!=0)
	{
	  distance=image1[i*rs + j];
	  if (CallMedial(i, j, image1, rs, cs, MgL1, LutColumn1, rknown, rmax))	 
	    imagemedial[i*rs + j] = distance;
	}
	else 
	  imagemedial[i*rs + j] = 0;
      }
    }
#ifdef CHRONO
      //ma
      printf("%g,", ((double)read_chrono(&Chrono1)) / 1000000.0);
#endif

  }
  else // 3D
  {
    int32_t i, j, k, numb, rmax, rknown, distance, ps = rs*cs;
    char tablefilename[512];
    LookUpTable LutColumn1;
    MaskG MgL1;

    sprintf(tablefilename, "%s/src/tables/TabRemyThiel3d.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table\n", F_NAME);
      return(0);
    }
    fscanf(fd,"%d",&numb); // Number of different directions
    MgL1 = (MaskG)calloc(1,numb * sizeof(struct Weighting));
    if (MgL1 == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < numb; i++)
    {
      fscanf (fd, "%d %d %d %d", &(MgL1[i].x), &(MgL1[i].y), &(MgL1[i].z), &(MgL1[i].RR));
    }
    fscanf(fd,"%d",&rknown);
    LutColumn1 = (LookUpTable)calloc(1,numb * rknown * sizeof(int32_t));
    if (LutColumn1 == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < numb; i++)
    {
      for (j = 0; j < rknown; j++)
	fscanf(fd,"%d",&(LutColumn1[i*rknown + j]));
    }
    fclose (fd);

    rmax= RadiusMax(image1, rs, cs, ds);
#ifdef VERBOSE
    printf("Maximum radius in the image=%d\n",rmax);
#endif
    if (rmax > rknown)
    {   
      fprintf(stderr, "%s: rmax = %d > %d (max permitted by current table)\n", F_NAME, rmax, rknown);
      return(0);
    }

#ifdef CHRONO
    //rmax
    printf("%d,", rmax);
    //luts
    printf("%g,", ((double)read_chrono(&Chrono1)) / 1000000.0);
    //ma
    start_chrono(&Chrono1);
#endif

    /*Finding the medial Axis points*/
    for (i = 0; i < ds; i++) // sic
    {
      for (j = 0; j < cs; j++)
      {
	for (k = 0; k < rs; k++)
        {
	  if (image1[i*ps + j*rs + k] != 0)
	  {
	    distance=image1[i*ps + j*rs + k];
	    if (CallMedial3d(i, j, k, image1, rs, cs, ds, MgL1, LutColumn1, rknown, rmax))	 
	      imagemedial[i*ps + j*rs + k] = distance;
	  }
	}
      }
    }
#ifdef CHRONO
      //ma
      printf("%g,", ((double)read_chrono(&Chrono1)) / 1000000.0);
#endif
  }

  return 1;
} // lmedialaxis_lmedax_Remy_Thiel()

/* ==================================== */
struct xvimage *lmedialaxis_lmedialaxis(struct xvimage *f, int32_t mode)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lmedialaxis_lmedialaxis"
  int32_t i;
  int32_t rs = rowsize(f);        /* taille ligne f */
  int32_t cs = colsize(f);        /* taille colonne f */
  int32_t ds = depth(f);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;            /* taille image f */
  uint8_t *F = UCHARDATA(f);
  struct xvimage *medial;
  uint32_t *D;

  medial = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (medial == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(NULL);
  }
  D = ULONGDATA(medial);

    if ((mode == 0) && (ds == 1))
    {
      if (! lmedialaxis_lmedax_meyer(f, medial))
	//if (! lmedialaxis_lmedax_talbot(f, medial))
      {
        fprintf(stderr, "%s: lmedialaxis_lmedax_meyer failed\n", F_NAME);
        return(NULL);
      }
    }
    else
    if ((mode == 0) && (ds > 1))
    {
      if (! lmedialaxis_lmedax_meyer3d(f, medial))
      {
        fprintf(stderr, "%s: lmedialaxis_lmedax_meyer3d failed\n", F_NAME);
        return(NULL);
      }
    }
    else
    if ((mode == 1) && (ds == 1)) // Saito-Toriwaki's skeleton
    {
      struct xvimage *dist;

      fprintf(stderr, "%s: WARNING: mode 1 not validated\n", F_NAME);

      dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
      if (dist == NULL)
      {   
	fprintf(stderr, "%s: allocimage failed\n", F_NAME);
	return(NULL);
      }
      if (! lsedt_meijster(f, dist)) // calcule la fonction distance
      {
        fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
        return(NULL);
      }
      if (! lskeleton_ST(dist, medial))
      {
        fprintf(stderr, "%s: lskeleton_ST failed\n", F_NAME);
        return(NULL);
      }
      freeimage(dist);
    }
    else
    if (mode == 3)
    {
      struct xvimage *dist;
      dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
      if (dist == NULL)
      {   
	fprintf(stderr, "%s: allocimage failed\n", F_NAME);
	return(NULL);
      }
      if (! lsedt_meijster(f, dist)) // calcule la fonction distance
      {
        fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
        return(NULL);
      }
      if (! lmedialaxis_lmedax_Remy_Thiel(dist, medial))
      {
        fprintf(stderr, "%s: lmedialaxis_lmedax_Remy_Thiel failed\n", F_NAME);
        return(NULL);
      }
      freeimage(dist);
    }
    else
    if ((mode == 4) && (ds == 1))
    {
      struct xvimage *dist;
      uint32_t *T1;
      for (i = 0; i < N; i++) if (F[i]) F[i] = NDG_MIN; else F[i] = NDG_MAX; // inverse et seuille a 1 
      if (! ldist(f, 4, medial)) // calcule la fonction distance
      {
        fprintf(stderr, "%s: ldist failed\n", F_NAME);
        return(NULL);
      }
      dist = copyimage(medial);
      if (! lt4pp(dist))           // pour les maxima de la fonction distance
      {
        fprintf(stderr, "%s: function lt4pp failed\n", F_NAME);
        return(NULL);
      }
      T1 = ULONGDATA(dist);
      for (i = 0; i < N; i++) 
        if ((T1[i] == 0) && (F[i] == 0)) ; // F[i] = D[i]; 
        else D[i] = 0; 
      freeimage(dist);
    }
    else
    if ((mode == 8) && (ds == 1))
    {
      struct xvimage *dist;
      uint32_t *T1;
      for (i = 0; i < N; i++) if (F[i]) F[i] = NDG_MIN; else F[i] = NDG_MAX; // inverse et seuille a 1 
      if (! ldist(f, 8, medial)) // calcule la fonction distance
      {
        fprintf(stderr, "%s: ldist failed\n", F_NAME);
        return(NULL);
      }
      dist = copyimage(medial);
      if (! lt8pp(dist))           // pour les maxima de la fonction distance
      {
        fprintf(stderr, "%s: function lt8pp failed\n", F_NAME);
        return(NULL);
      }
      T1 = ULONGDATA(dist);
      for (i = 0; i < N; i++) 
        if ((T1[i] == 0) && (F[i] == 0)) ; // F[i] = D[i]; 
        else D[i] = 0; 
      freeimage(dist);
    }
    else
    if (mode == 6)
    {
      struct xvimage *dist;
      uint32_t *T1;
      for (i = 0; i < N; i++) if (F[i]) F[i] = NDG_MIN; else F[i] = NDG_MAX; // inverse et seuille a 1 
      if (! ldist(f, 6, medial)) // calcule la fonction distance
      {
        fprintf(stderr, "%s: ldist failed\n", F_NAME);
        return(NULL);
      }
      dist = copyimage(medial);
      if (! lt6pp(dist))           // pour les maxima de la fonction distance
      {
        fprintf(stderr, "%s: function lt6pp failed\n", F_NAME);
        return(NULL);
      }
      T1 = ULONGDATA(dist);
      for (i = 0; i < N; i++) 
        if ((T1[i] == 0) && (F[i] == 0)) ; // F[i] = D[i]; 
        else D[i] = 0; 
      freeimage(dist);
    }
    else
    if (mode == 26)
    {
      struct xvimage *dist;
      uint32_t *T1;
      for (i = 0; i < N; i++) if (F[i]) F[i] = NDG_MIN; else F[i] = NDG_MAX; // inverse et seuille a 1 
      if (! ldist(f, 26, medial)) // calcule la fonction distance
      {
        fprintf(stderr, "%s: ldist failed\n", F_NAME);
        return(NULL);
      }
      dist = copyimage(medial);
      if (! ltopotypes_t26pp(dist))           // pour les maxima de la fonction distance
      {
        fprintf(stderr, "%s: function ltopotypes_t26pp failed\n", F_NAME);
        return(NULL);
      }
      T1 = ULONGDATA(dist);
      for (i = 0; i < N; i++) 
        if ((T1[i] == 0) && (F[i] == 0)) ; // F[i] = D[i]; 
        else D[i] = 0; 
      freeimage(dist);
    }
    else
    {
      fprintf(stderr, "%s: mode %d not yet implemented\n", F_NAME, mode);
      return(NULL);
    }
#ifdef RETIRE_BORDS
    if (depth(f) == 1)  // cas des bords: pas d'AM (inconnu)
    {
      for (i = 0; i < rs; i++) 
      { D[i] = 0; D[(cs-1)*rs + i] = 0; }
      for (i = 0; i < cs; i++)
      { D[i*rs] = 0; D[i*rs + rs-1] = 0; }
    }
    else
    {
      int32_t x, y, z;
      for (x = 0; x < rs; x++)
      for (y = 0; y < cs; y++) 
        D[0 * ps + y * rs + x] = 0;          /* plan z = 0 */
      for (x = 0; x < rs; x++)
      for (y = 0; y < cs; y++) 
        D[(ds-1) * ps + y * rs + x] = 0;     /* plan z = ds-1 */

      for (x = 0; x < rs; x++)
      for (z = 0; z < ds; z++) 
        D[z * ps + 0 * rs + x] = 0;          /* plan y = 0 */
      for (x = 0; x < rs; x++)
      for (z = 0; z < ds; z++) 
        D[z * ps + (cs-1) * rs + x] = 0;     /* plan y = cs-1 */

      for (y = 0; y < cs; y++)
      for (z = 0; z < ds; z++) 
        D[z * ps + y * rs + 0] = 0;          /* plan x = 0 */
      for (y = 0; y < cs; y++)
      for (z = 0; z < ds; z++) 
        D[z * ps + y * rs + (rs-1)] = 0;     /* plan x = rs-1 */
    }
#endif
  return medial;
} // lmedialaxis_lmedialaxis()

/* ==================================== */
int32_t lmedialaxis_lmedialaxisbin(struct xvimage *f, int32_t mode)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lmedialaxisbin"
  int32_t i;
  int32_t N = rowsize(f) * colsize(f) * depth(f);
  uint8_t *F = UCHARDATA(f);
  struct xvimage *dist;
  uint32_t *D;

  dist = lmedialaxis_lmedialaxis(f, mode);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: lmedialaxis failed\n", F_NAME);
    return(0);
  }
  D = ULONGDATA(dist);
  for (i = 0; i < N; i++) if (D[i]) F[i] = NDG_MAX; else F[i] = NDG_MIN;
  return 1;
} // lmedialaxis_lmedialaxisbin()

/* ==================================== */
/* ==================================== */
/* ==================================== */

// Data structures and functions for the bisector

/* ==================================== */
/* ==================================== */
/* ==================================== */

typedef double * ImageAngle;

struct  Point2D{
    int32_t xCoor, yCoor;
};
struct  Point3D{
    int32_t xCoor, yCoor, zCoor;
};
typedef struct Point2D * ListPoint2D;
typedef struct Point3D * ListPoint3D;

typedef int32_t *tabulateCTg;

/* Function that returns the cosine of the maximum angle */
/* ==================================== */
double MaximumAngle(int32_t xc, int32_t yc, ListPoint2D LPoints, int32_t count)
/* ==================================== */
{
  int32_t i, j, x1, y1, x2, y2;
  double cosi, MaxAngle;

#ifdef COUNT
  sumpoints += count;
  countds++;
#endif

  if (count == 1)
    MaxAngle = 1.0;
  else
  {
    MaxAngle = 100.00;
    for (i = 0; i < count-1; i++)
    {
      x1 = LPoints[i].xCoor; y1 = LPoints[i].yCoor;
      for(j = i+1; j < count; j++)
      {
	x2 = LPoints[j].xCoor; y2 = LPoints[j].yCoor;
	cosi = ((xc-x1)*(xc-x2) + (yc-y1)*(yc-y2)) /
	  sqrt(((xc-x1)*(xc-x1) + (yc-y1)*(yc-y1)) * ((xc-x2)*(xc-x2) + (yc-y2)*(yc-y2)));	
	if (cosi<MaxAngle) MaxAngle=cosi;
      }
    }	
  }
  return MaxAngle;
} // MaximumAngle()

/* Function that returns the cosine of the maximum angle in 3d */
/* ==================================== */
double MaximumAngle3d(int32_t xc, int32_t yc, int32_t zc, ListPoint3D LPoints, int32_t count)
/* ==================================== */
{
  int32_t i, j, x1, y1, z1, x2, y2, z2;
  double cosi, MaxAngle;

#ifdef COUNT
  sumpoints += count;
  countds++;
#endif

  if (count == 1)
    MaxAngle = 1.0;
  else
  {
    MaxAngle = 100.00;
    for (i = 0; i < count-1; i++)
    {
      x1 = LPoints[i].xCoor; y1 = LPoints[i].yCoor; z1 = LPoints[i].zCoor;
      for(j = i+1; j < count; j++)
      {
	x2 = LPoints[j].xCoor; y2 = LPoints[j].yCoor; z2 = LPoints[j].zCoor;
	cosi = ((xc-x1)*(xc-x2) + (yc-y1)*(yc-y2) + (zc-z1)*(zc-z2)) /
	       sqrt(((xc-x1)*(xc-x1) + (yc-y1)*(yc-y1) + (zc-z1)*(zc-z1)) * 
		    ((xc-x2)*(xc-x2) + (yc-y2)*(yc-y2) + (zc-z2)*(zc-z2)));	
	if (cosi<MaxAngle) MaxAngle=cosi;
      }
    }	
  }
  return MaxAngle;
} // MaximumAngle3d()

//----------------------------------
int32_t lmedialaxis_ApplySymmetriesB(int32_t x, int32_t y, int32_t x1, int32_t y1, int32_t rs, int32_t cs, Neighbors *Mg)
//----------------------------------
{
  int32_t vectx[8], vecty[8], i;
  int32_t count=0;

  vectx[0] = x;   vecty[0] = y;
  vectx[1] = y;   vecty[1] = x;
  vectx[2] = y;   vecty[2] = -x;
  vectx[3] = x;   vecty[3] = -y;
  vectx[4] = -x;  vecty[4] = -y;
  vectx[5] = -y;  vecty[5] = -x;
  vectx[6] = -y;  vecty[6] = x;
  vectx[7] = -x;  vecty[7] = y;

  for (i = 0; i <= 7; i++)
  {
    if((x1 + vectx[i] >= 0) && (x1 + vectx[i] < rs) &&
       (y1 + vecty[i] >= 0) && (y1 + vecty[i] < cs))
    {
      Mg->neig[count].x = vectx[i];
      Mg->neig[count].y = vecty[i];
      count=count+1;
    }
  }
  return count;	// this is important to know how many vector are present in Mg
} // lmedialaxis_ApplySymmetriesB()

//----------------------------------
int32_t lmedialaxis_ApplySymmetriesB3d(int32_t x, int32_t y, int32_t z, int32_t x1, int32_t y1, int32_t z1, int32_t rs, int32_t cs, int32_t ds, Neighbors *Mg)
//----------------------------------
{
  int32_t vectx[48], vecty[48], vectz[48], i;
  int32_t count = 0;

  vectx[ 0] = x;   vecty[ 0] = y;   vectz[ 0] = z;
  vectx[ 1] = x;   vecty[ 1] = z;   vectz[ 1] = y;
  vectx[ 2] = y;   vecty[ 2] = x;   vectz[ 2] = z;
  vectx[ 3] = y;   vecty[ 3] = z;   vectz[ 3] = x;
  vectx[ 4] = z;   vecty[ 4] = x;   vectz[ 4] = y;
  vectx[ 5] = z;   vecty[ 5] = y;   vectz[ 5] = x;

  vectx[ 6] = -x;  vecty[ 6] = y;   vectz[ 6] = z;
  vectx[ 7] = -x;  vecty[ 7] = z;   vectz[ 7] = y;
  vectx[ 8] = -y;  vecty[ 8] = x;   vectz[ 8] = z;
  vectx[ 9] = -y;  vecty[ 9] = z;   vectz[ 9] = x;
  vectx[10] = -z;  vecty[10] = x;   vectz[10] = y;
  vectx[11] = -z;  vecty[11] = y;   vectz[11] = x;

  vectx[12] = x;   vecty[12] = -y;  vectz[12] = z;
  vectx[13] = x;   vecty[13] = -z;  vectz[13] = y;
  vectx[14] = y;   vecty[14] = -x;  vectz[14] = z;
  vectx[15] = y;   vecty[15] = -z;  vectz[15] = x;
  vectx[16] = z;   vecty[16] = -x;  vectz[16] = y;
  vectx[17] = z;   vecty[17] = -y;  vectz[17] = x;

  vectx[18] = x;   vecty[18] = y;   vectz[18] = -z;
  vectx[19] = x;   vecty[19] = z;   vectz[19] = -y;
  vectx[20] = y;   vecty[20] = x;   vectz[20] = -z;
  vectx[21] = y;   vecty[21] = z;   vectz[21] = -x;
  vectx[22] = z;   vecty[22] = x;   vectz[22] = -y;
  vectx[23] = z;   vecty[23] = y;   vectz[23] = -x;

  vectx[24] = -x;  vecty[24] = -y;  vectz[24] = z;
  vectx[25] = -x;  vecty[25] = -z;  vectz[25] = y;
  vectx[26] = -y;  vecty[26] = -x;  vectz[26] = z;
  vectx[27] = -y;  vecty[27] = -z;  vectz[27] = x;
  vectx[28] = -z;  vecty[28] = -x;  vectz[28] = y;
  vectx[29] = -z;  vecty[29] = -y;  vectz[29] = x;

  vectx[30] = -x;  vecty[30] = y;   vectz[30] = -z;
  vectx[31] = -x;  vecty[31] = z;   vectz[31] = -y;
  vectx[32] = -y;  vecty[32] = x;   vectz[32] = -z;
  vectx[33] = -y;  vecty[33] = z;   vectz[33] = -x;
  vectx[34] = -z;  vecty[34] = x;   vectz[34] = -y;
  vectx[35] = -z;  vecty[35] = y;   vectz[35] = -x;

  vectx[36] = x;   vecty[36] = -y;  vectz[36] = -z;
  vectx[37] = x;   vecty[37] = -z;  vectz[37] = -y;
  vectx[38] = y;   vecty[38] = -x;  vectz[38] = -z;
  vectx[39] = y;   vecty[39] = -z;  vectz[39] = -x;
  vectx[40] = z;   vecty[40] = -x;  vectz[40] = -y;
  vectx[41] = z;   vecty[41] = -y;  vectz[41] = -x;

  vectx[42] = -x;  vecty[42] = -y;  vectz[42] = -z;
  vectx[43] = -x;  vecty[43] = -z;  vectz[43] = -y;
  vectx[44] = -y;  vecty[44] = -x;  vectz[44] = -z;
  vectx[45] = -y;  vecty[45] = -z;  vectz[45] = -x;
  vectx[46] = -z;  vecty[46] = -x;  vectz[46] = -y;
  vectx[47] = -z;  vecty[47] = -y;  vectz[47] = -x;

  for (i = 0; i < 48; i++)
  {
    if((x1 + vectx[i] >= 0) && (x1 + vectx[i] < rs) &&
       (y1 + vecty[i] >= 0) && (y1 + vecty[i] < cs) &&
       (z1 + vectz[i] >= 0) && (z1 + vectz[i] < ds))
    {
      Mg->neig[count].x = vectx[i];
      Mg->neig[count].y = vecty[i];
      Mg->neig[count].z = vectz[i];
      count++;
    }
  }
  return count;		// this is important to know how many vector are present in Mg
} // lmedialaxis_ApplySymmetriesB3d()

/* ==================================== */
double lmedialaxis_ComputeAngle(int32_t x, int32_t y, uint32_t *image,
		    int32_t rs, int32_t cs, 
		    int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		    ListPoint2D Aval)
/* ==================================== */
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval - 
//          logiquement locale, passé en paramètre pour éviter allocations/libérations
{
#undef F_NAME
#define F_NAME "lmedialaxis_ComputeAngle"
  int32_t nb, i, j, xx, yy, rr, counter, k, c, ti, d, nbdec;
  double maxangle;
  Neighbors MgN1;
  int32_t xnew, ynew;

  int32_t X[5] = {x, x-1, x+1, x, x};
  int32_t Y[5] = {y, y, y, y-1, y+1};

  counter=0;

#ifdef STATVOR
  countpts++;
#endif

  for (k = 0; k < 5; k++) // k indexes the point in test and its 4 neighbors
  {
    if (X[k] < rs && Y[k] < cs && X[k] >= 0 && Y[k] >= 0)
    {
      i = (int32_t)(image[Y[k]*rs + X[k]]);
      if (i == 0) goto endfor;
#ifdef VARIANTE_GB
      if (i > image[y*rs + x]) goto endfor;
#endif
#ifdef PARANO
      if (i >= nval)
      {
	fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, i, nval);
	exit(0);
      }    
#endif
      ti = TabIndDec[i];
      nbdec = TabIndDec[i+1] - ti;
      for(d = 0; d < nbdec; d++)
      {
	xnew = ListDecs[ti+d].x; ynew = ListDecs[ti+d].y;
	nb = lmedialaxis_ApplySymmetriesB(xnew, ynew, X[k], Y[k], rs, cs, &MgN1);	
	for (j = 0; j < nb; j++)
        {
	  xx = X[k] + MgN1.neig[j].x; yy = Y[k] + MgN1.neig[j].y; 
	  rr = (int32_t)(image[yy*rs + xx]);
	  
	  if (rr == 0) // if one zero is found
	  { 
	    for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	      if (xx==Aval[c].xCoor && yy==Aval[c].yCoor) goto skip;
	    Aval[counter].xCoor=xx; Aval[counter].yCoor=yy; counter++;		
#ifdef STATVOR
	    if (k == 0) countvor++;
#endif
	  skip: ;
	  } // if (rr == 0)
	} // for (j = 0; j < nb; j++)
      } // for(d = 0; d < nbdec; d++)
    } // if
#ifdef VARIANTE_DC
    if ((k == 0) && (counter > 1)) break;
#endif
#ifdef STATVOR
    if ((k == 0) && (counter > 1)) countmult++;
    if ((k == 0) && (counter > maxdwnstr)) maxdwnstr = counter; 
#endif
#ifdef HISTVOR
    if (k == 0) vorhisto[counter] += 1;
#endif
  endfor: ;
  } // for(k = 0; k < 5; k++)

#ifdef HISTVOR
  extvorhisto[counter] += 1;
#endif

#ifdef PARANO
  if (counter == 0) 
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d\n", counter, x, y);
    exit(0);
  }
#endif

  maxangle = MaximumAngle(x, y, Aval, counter);

  return maxangle;
} // lmedialaxis_ComputeAngle()
 
/* ==================================== */
double lmedialaxis_ComputeAngle_8(int32_t x, int32_t y, uint32_t *image,
		    int32_t rs, int32_t cs, 
		    int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		    ListPoint2D Aval)
/* ==================================== */
// VARIANTE avec les 8 voisins
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval - 
//          logiquement locale, passé en paramètre pour éviter allocations/libérations
{
#undef F_NAME
#define F_NAME "lmedialaxis_ComputeAngle"
  int32_t nb, i, j, xx, yy, rr, counter, k, c, ti, d, nbdec;
  double maxangle;
  Neighbors MgN1;
  int32_t xnew, ynew;
  int32_t X[9] = {x, x-1, x+1, x, x, x-1, x-1, x+1, x+1};
  int32_t Y[9] = {y, y, y, y-1, y+1, y-1, y+1, y-1, y+1};

  counter=0;

  for(k = 0; k < 9; k++) // k indexes the point in test and its 8 neighbors
  {
    if (X[k] < rs && Y[k] < cs && X[k] >= 0 && Y[k] >= 0)
    {
      i = (int32_t)(image[Y[k]*rs + X[k]]);
      if (i == 0) goto endfor;
#ifdef VARIANTE_GB
      if (i > image[y*rs + x]) goto endfor;
#endif
#ifdef PARANO
      if (i >= nval)
      {
	fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, i, nval);
	exit(0);
      }    
#endif
      ti = TabIndDec[i];
      nbdec = TabIndDec[i+1] - ti;
      for(d = 0; d < nbdec; d++)
      {
	xnew = ListDecs[ti+d].x; ynew = ListDecs[ti+d].y;
	nb = lmedialaxis_ApplySymmetriesB(xnew, ynew, X[k], Y[k], rs, cs, &MgN1);	
	for (j = 0; j < nb; j++)
        {
	  xx = X[k] + MgN1.neig[j].x; yy = Y[k] + MgN1.neig[j].y;
	  rr = (int32_t)(image[yy*rs + xx]);
	  
	  if (rr == 0) // if one zero is found
	  { 
	    for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	      if (xx==Aval[c].xCoor && yy==Aval[c].yCoor) goto stoploops;
	    Aval[counter].xCoor=xx; Aval[counter].yCoor=yy; counter++;		
	  stoploops: ;
	  } // if (rr == 0)
	} // for (j = 0; j < nb; j++)
      } // for(d = 0; d < nbdec; d++)
    } // if
  endfor: ;
  } // for(k = 0; k < 9; k++)

#ifdef PARANO
  if (counter == 0) 
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d\n", counter, x, y);
    exit(0);
  }
#endif

  maxangle = MaximumAngle(x, y, Aval, counter);

  return maxangle;
} // lmedialaxis_ComputeAngle_8()

/* ==================================== */
double lmedialaxis_ComputeAngle3d (int32_t x, int32_t y, int32_t z, uint32_t *image,
		       int32_t rs, int32_t cs, int32_t ds, 
		       int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		       ListPoint3D Aval)
/* ==================================== */
// x, y, z : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval - 
//          logiquement locale, passé en paramètre pour éviter allocations/libérations
{
#undef F_NAME
#define F_NAME "lmedialaxis_ComputeAngle3d"
  int32_t nb, i, j, xx, yy, zz, rr, counter, k, c, ti, d, nbdec, ps = rs*cs;
  double maxangle;
  Neighbors MgN1;
  int32_t xnew, ynew, znew;

  int32_t X[7] = {x, x-1, x+1, x,   x,   x,   x};
  int32_t Y[7] = {y, y,   y,   y-1, y+1, y,   y};
  int32_t Z[7] = {z, z,   z,   z,   z,   z-1, z+1};

#ifdef STATVOR
  countpts++;
#endif

  counter = 0;

  for (k = 0; k < 7; k++) // k indexes the point in test and its 6 neighbors
  {
    if (X[k] < rs && Y[k] < cs && Z[k] < ds && X[k] >= 0 && Y[k] >= 0 && Z[k] >= 0)
    {
      i = (int32_t)(image[Z[k]*ps + Y[k]*rs + X[k]]);
      if (i == 0) goto endfor;
#ifdef VARIANTE_GB
      if (i > image[z*ps + y*rs + x]) goto endfor;
#endif
#ifdef PARANO
      if (i >= nval)
      {
	fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, i, nval);
	exit(0);
      }    
#endif
      ti = TabIndDec[i];
      nbdec = TabIndDec[i+1] - ti;
      for (d = 0; d < nbdec; d++)
      {
	xnew = ListDecs[ti+d].x; // direction of search to reach the first zero (CTg table)
	ynew = ListDecs[ti+d].y;
	znew = ListDecs[ti+d].z;
	nb = lmedialaxis_ApplySymmetriesB3d(xnew, ynew, znew, X[k], Y[k], Z[k], rs, cs, ds, &MgN1);	
	for (j = 0; j < nb; j++)
        {
	  xx = X[k] + MgN1.neig[j].x; 
	  yy = Y[k] + MgN1.neig[j].y; 
	  zz = Z[k] + MgN1.neig[j].z; 
	  rr = (int32_t)(image[zz*ps + yy*rs + xx]);
	  
	  if (rr == 0) // if one zero is found
	  { 
	    for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	      if ((xx == Aval[c].xCoor) && (yy == Aval[c].yCoor) && (zz == Aval[c].zCoor))
		goto stoploops;
	    Aval[counter].xCoor = xx;
	    Aval[counter].yCoor = yy;
	    Aval[counter].zCoor = zz;
	    counter++;		
#ifdef STATVOR
	    if (k == 0) countvor++;
#endif
	  stoploops: ;
	  } // if (rr == 0)
	} // for (j = 0; j < nb; j++)
      } // for (d = 0; d < nbdec; d++)
    } // if
#ifdef STATVOR
    if ((k == 0) && (counter > 1)) countmult++;
    if ((k == 0) && (counter > maxdwnstr)) maxdwnstr = counter; 
#endif
#ifdef HISTVOR
    if (k == 0) vorhisto[counter] += 1;
#endif
  endfor: ;
  } // for (k = 0; k < 7; k++)

#ifdef HISTVOR
  extvorhisto[counter] += 1;
#endif

#ifdef PARANO
  if (counter == 0) 
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d,%d\n", counter, x, y, z);
    exit(0);
  }
#endif

  maxangle = MaximumAngle3d(x, y, z, Aval, counter);

  return maxangle;
} // lmedialaxis_ComputeAngle3d()

/* ==================================== */
int32_t lmedialaxis_lbisector(struct xvimage *id, struct xvimage *im, struct xvimage *ia)
/* ==================================== */
/*
   Calcule la fonction bissectrice pour les points non nuls de l'image 'im'. 
   'id' est une carte de distance euclidienne quadratique de l'objet. 
   Les angles calculés (entre 0 et pi) seront stockés dans 'ia'. 
   L'image 'ia' (type float) doit être allouée à l'avance. 
*/
{
#undef F_NAME
#define F_NAME "lmedialaxis_lbisector"
  int32_t i, j, k, nval, npoints, npointsmax;
  int32_t rs = rowsize(id);
  int32_t cs = colsize(id);
  int32_t ds = depth(id);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;
  double angle;	
  int32_t *TabIndDec;
  Coordinates *ListDecs;
  FILE *fd=NULL;
  uint32_t *imagedist = ULONGDATA(id);
  uint8_t *imagemask = UCHARDATA(im);
  float *imageangle = FLOATDATA(ia);
  char tablefilename[512];
  int32_t distmax; 

#ifdef HISTVOR
  for (i = 0; i < sizevorhisto; i++) vorhisto[i] = extvorhisto[i] = 0;
#endif

  if (datatype(id) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: distance image must be int32_t\n", F_NAME);
    return 0;
  }    

  if ((rowsize(im) != rs) || (colsize(im) != cs) || (depth(im) != ds) ||
      (rowsize(ia) != rs) || (colsize(ia) != cs) || (depth(ia) != ds))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    
  if (datatype(im) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: mask image must be byte\n", F_NAME);
    return 0;
  }    
  if (datatype(ia) != VFF_TYP_FLOAT)
  {
    fprintf(stderr, "%s: angle image must be float\n", F_NAME);
    return 0;
  }    

  distmax = 0;  // calcule la distance max dans l'image de distance
  for (i = 0; i < N; i++)
    if ((imagemask[i] != 0) && (imagedist[i] > distmax)) distmax = imagedist[i];
  distmax++;

  if (ds == 1) // 2D
  {
    ListPoint2D Aval;
    Aval= (struct Point2D *)calloc(1,N*sizeof(struct Point2D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }

    /* Reading the bissector table from the files */
    sprintf(tablefilename, "%s/src/tables/TabBisector_1.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }

    fscanf(fd,"%d",&nval);
    if (distmax >= nval)
    {
      fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
      exit(0);
    }
    TabIndDec = (int32_t *)calloc(1,(distmax+2) * sizeof(int32_t));
    if (TabIndDec == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i <= distmax+1; i++) fscanf(fd,"%d", &TabIndDec[i]);
    npointsmax = TabIndDec[distmax];
    fclose(fd);

    sprintf(tablefilename, "%s/src/tables/TabBisector_2.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }
    fscanf(fd,"%d",&npoints);
#ifdef PARANO
    if (npointsmax >= npoints)
    {
      fprintf(stderr, "%s: bisector table overflow for npoints: %d >= %d\n", F_NAME, npointsmax, npoints);
      exit(0);
    }
#endif
    ListDecs = (Coordinates *)calloc(1,npointsmax * sizeof(Coordinates));
    if (ListDecs == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < npointsmax; i++) fscanf(fd,"%d%d", &ListDecs[i].x, &ListDecs[i].y);
    fclose (fd);
#ifdef VERBOSE
printf("distmax = %d ; nval = %d ; npointsmax = %d ; npoints = %d\n", distmax, nval, npointsmax, npoints);
#endif
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
      {
	if (imagemask[j*rs + i] != 0)
        {
	  angle = lmedialaxis_ComputeAngle(i, j, imagedist, rs, cs, TabIndDec, nval, ListDecs, Aval);
	  imageangle[j*rs + i] = (float)acos(angle);				
	}
	else
	  imageangle[j*rs + i] = 0.0;
      }
    free(Aval);
  } // if (ds == 1)
  else // 3D
  {
    ListPoint3D Aval;
    Aval= (struct Point3D *)calloc(1,N*sizeof(struct Point3D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }

    /* Reading the bissector table from the file */
    sprintf(tablefilename, "%s/src/tables/TabBisector3d_1.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table\n", F_NAME);
      return(0);
    }

    fscanf(fd,"%d",&nval);
    if (distmax >= nval)
    {
      fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
      exit(0);
    }
    TabIndDec = (int32_t *)calloc(1,(distmax+2) * sizeof(int32_t));
    if (TabIndDec == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i <= distmax+1; i++) fscanf(fd,"%d", &TabIndDec[i]);
    npointsmax = TabIndDec[distmax];
    fclose(fd);

    sprintf(tablefilename, "%s/src/tables/TabBisector3d_2.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }
    fscanf(fd,"%d",&npoints);
#ifdef PARANO
    if (npointsmax >= npoints)
    {
      fprintf(stderr, "%s: bisector table overflow for npoints: %d >= %d\n", F_NAME, npointsmax, npoints);
      exit(0);
    }
#endif
    ListDecs = (Coordinates *)calloc(1,npoints * sizeof(Coordinates));
    if (ListDecs == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < npointsmax; i++) fscanf(fd,"%d%d%d", &ListDecs[i].x, &ListDecs[i].y, &ListDecs[i].z);
    fclose (fd);
#ifdef VERBOSE
printf("distmax = %d ; nval = %d ; npointsmax = %d ; npoints = %d\n", distmax, nval, npointsmax, npoints);
#endif

    for (k = 0; k < ds; k++)
      for (j = 0; j < cs; j++)
	for (i = 0; i < rs; i++)
        {
	  if (imagemask[k*ps + j*rs + i] != 0)
          {
	    angle = lmedialaxis_ComputeAngle3d(i, j, k, imagedist, rs, cs, ds, TabIndDec, nval, ListDecs, Aval);
	    imageangle[k*ps + j*rs + i] = (float)acos(angle);				
	  }
	  else
	    imageangle[k*ps + j*rs + i] = 0.0;
	}

    free(Aval);
  } // else (3D)


#ifdef COUNT
  printf("mean card. of ext. downstream = %g\n", sumpoints/countds);
#endif

#ifdef STATVOR
  //  printf("%d %d %d %d %d\n", 
  //         countpts, countvor, countmult, maxdwnstr, countmult*maxdwnstr);
  printf("%d %d\n", 
         countpts, countvor);
#endif
#ifdef HISTVOR
  i = 1; 
  printf("histo voronoi : \n"); 
  while ((i < sizevorhisto) && (vorhisto[i] > 0))
  {
    printf("%d %d\n", i, vorhisto[i]);
    i++;
  }
  printf("\n"); 
  printf("histo extended voronoi : \n"); 
  i = 1; 
  while ((i < sizevorhisto) && (extvorhisto[i] > 0))
  {
    printf("%d %d\n", i, extvorhisto[i]);
    i++;
  }
  printf("\n"); 
#endif

  free(TabIndDec);
  free(ListDecs);
  return 1;
} // lmedialaxis_lbisector()
 
/* ==================================== */
double lmedialaxis_ComputeAngle_Rita(
			int32_t x, int32_t y, uint32_t *image,
			int32_t rs, int32_t cs, int32_t *distarray,
			int32_t *xarray, int32_t *yarray, int32_t number, 
			ListPoint2D Aval)
/* ==================================== */
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// distarray, xarray, yarray : la "table" pour calculer l'aval
// number : la taille de ladite "table"
// Aval : tableau pour stocker les points de l'aval - 
//  logiquement locale, passé en paramètre pour éviter allocations/libérations
{
  int32_t nb, i, j, xx, yy, rr, counter, k, c;
  double maxangle;
  int32_t dist[5];
  Neighbors MgN1;
  int32_t xnew, ynew;

  int32_t X[5] = {x, x-1, x+1, x, x};
  int32_t Y[5] = {y, y, y, y-1, y+1};

  dist[0] = (int32_t)(image[X[0] * rs+Y[0]]);
  dist[1] = (int32_t)(image[X[1] * rs+Y[1]]);
  dist[2] = (int32_t)(image[X[2] * rs+Y[2]]);
  dist[3] = (int32_t)(image[X[3] * rs+Y[3]]);
  dist[4] = (int32_t)(image[X[4] * rs+Y[4]]);

  counter=0;

  for(k = 0; k < 5; k++) // k indicate the point in test and its 4 neighbors
  {
    if(dist[k]==0 || X[k]>=rs || Y[k]>=cs) goto endfor;
    for(i = 0; i < number; i++)
    {
      if (dist[k] == distarray[i])
      {	
	xnew = xarray[i]; // direction of search to reach the first zero (CTg table)
	ynew = yarray[i];
			
	nb=lmedialaxis_ApplySymmetriesB(xnew, ynew, X[k], Y[k], rs, cs, &MgN1);	
	for (j = 0; j < nb; j++)
	{
	  xx = X[k] + MgN1.neig[j].x;
	  yy = Y[k] + MgN1.neig[j].y;
	  rr = (int32_t)(image[xx*rs+yy]);
					
	  if (rr == 0) // if one zero is found
	  { 
            for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	      if (xx==Aval[c].xCoor && yy==Aval[c].yCoor)
		goto stoploops;
	    Aval[counter].xCoor=xx;
	    Aval[counter].yCoor=yy;
	    counter++;		
	  }
	stoploops: ;
	}
      }
    }
  endfor: ;
  }

  maxangle=MaximumAngle(x,y,Aval,counter);
  return maxangle;
} // lmedialaxis_ComputeAngle_Rita()

/* ==================================== */
int32_t lmedialaxis_lbisector_Rita(struct xvimage *id, struct xvimage *im, struct xvimage *ia)
/* ==================================== */
/*
   Calcule la fonction bissectrice pour les points non nuls de l'image 'im'. 
   Les angles calculés (entre 0 et pi) seront stockés dans 'ia'. 
   L'image 'ia' (type float) doit être allouée à l'avance. 
*/
{
#undef F_NAME
#define F_NAME "lmedialaxis_lbisector_Rita"
  int32_t i, j, numb;
  int32_t rs, cs, N;
  double angle;	
  ListPoint2D Aval;
  tabulateCTg distCTg;
  tabulateCTg YcoodCTg;
  tabulateCTg XcoodCTg;
  FILE *fd=NULL;
  uint32_t *imagedist;
  uint8_t *imagemask;
  float *imageangle;
  char tablefilename[512];

  if (depth(id) != 1)
  {
    fprintf(stderr, "%s: only for 2D images\n", F_NAME);
    return 0;
  }    
  if (datatype(id) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: distance image must be int32_t\n", F_NAME);
    return 0;
  }    
  imagedist = ULONGDATA(id);
  rs = rowsize(id);
  cs = colsize(id);
  N = rs * cs;

  if ((rowsize(im) != rs) || (colsize(im) != cs) || (depth(im) != 1) ||
      (rowsize(im) != rs) || (colsize(im) != cs) || (depth(im) != 1))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    
  if (datatype(im) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: mask image must be byte\n", F_NAME);
    return 0;
  }    
  if (datatype(ia) != VFF_TYP_FLOAT)
  {
    fprintf(stderr, "%s: angle image must be float\n", F_NAME);
    return 0;
  }    
  imagemask = UCHARDATA(im);
  imageangle = FLOATDATA(ia);

  Aval= (struct Point2D *)calloc(1,N*sizeof(struct Point2D));

  /* Reading the CTg table from the file */
  sprintf(tablefilename, "%s/src/tables/TabBisector.txt", getenv("PINK"));
  fd = fopen (tablefilename, "r");
  if (fd == NULL) 
  {   
    fprintf(stderr, "%s: error while opening table\n", F_NAME);
    return(0);
  }

  fscanf(fd,"%d",&numb);
  distCTg= (int32_t *)calloc(1,numb*sizeof(int32_t));
  YcoodCTg= (int32_t *)calloc(1,numb*sizeof(int32_t));
  XcoodCTg= (int32_t *)calloc(1,numb*sizeof(int32_t));
  for (i = 0; i < numb; i++)
  {
    fscanf(fd,"%d %d %d",&distCTg[i],&XcoodCTg[i],&YcoodCTg[i]);
  }
  fclose (fd);

  for (i = 0; i < cs; i++)
  {
    for (j = 0; j < rs; j++)
    {
      if (imagemask[i*rs + j] != 0)
      {
	angle = lmedialaxis_ComputeAngle_Rita(i,j,imagedist,rs,cs,distCTg,XcoodCTg,YcoodCTg,numb,Aval);
	imageangle[i*rs + j] = (float)acos(angle);				
      }
      else
	imageangle[i*rs + j] = 0.0;
    }
  }
  free(Aval);
  free(distCTg);
  free(YcoodCTg);
  free(XcoodCTg);
  return 1;
} // lmedialaxis_lbisector_Rita()

/* =============================================================== */
int32_t lmedialaxis_lbisector_talbot(struct xvimage * image, struct xvimage *angles) 
/* =============================================================== */
{
#undef F_NAME
#define F_NAME "lmedialaxis_lbisector_talbot"
  int32_t N, i, j, rs, cs;
  uint8_t *F;
  float *A;
  vect2Dint *L;              /* tableau de vecteur associe a un point de l'image */
  double theta;
  struct Point2D LPoints[5];

  if (depth(image) != 1)
  {
    fprintf(stderr, "%s: Only 2D images supported\n", F_NAME);
    return 0;
  }
  if (datatype(image) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: mask image must be byte\n", F_NAME);
    return 0;
  }    
  if (datatype(angles) != VFF_TYP_FLOAT)
  {
    fprintf(stderr, "%s: angles image must be float\n", F_NAME);
    return 0;
  }    

  rs = rowsize(image);
  cs = colsize(image);
  N = rowsize(image) * colsize(image);
  F = UCHARDATA(image);
  A = FLOATDATA(angles);

  if ((rowsize(angles) != rs) || (colsize(angles) != cs) || (depth(angles) != 1))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    

  L = (vect2Dint *)calloc(1,N * sizeof(vect2Dint));
  if (L == NULL)
  {   
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return 0;
  }

  for (i = 0; i < N; i++) // inverse l'image
    if (F[i]) F[i] = 0; else F[i] = NDG_MAX;
   
  if (! ldistvect(F, L, rs, cs))
  {
    fprintf(stderr, "%s: ldistvect failed\n", F_NAME);
    return 0;
  }

  for (i = 0; i < N; i++) // inverse l'image
    if (F[i]) F[i] = 0; else F[i] = NDG_MAX;
  for (j = 1; j < cs-1; j++)
  {
    for (i = 1; i < rs-1; i++)
    {
      if (F[i+j*rs] != 0) 
      {
#ifdef MODIF_MC
        LPoints[0].xCoor = i+L[i+j*rs].x; 
	LPoints[0].yCoor = j+L[i+j*rs].y; 
	// +1,0
        LPoints[1].xCoor = i+L[i+1+j*rs].x; 
	LPoints[1].yCoor = j+L[i+1+j*rs].y; 
	// -1,0
        LPoints[2].xCoor = i+L[i-1+j*rs].x; 
	LPoints[2].yCoor = j+L[i-1+j*rs].y; 
	// 0,-1
        LPoints[3].xCoor = i+L[i+(j-1)*rs].x; 
	LPoints[3].yCoor = j+L[i+(j-1)*rs].y; 
	// 0,+1
        LPoints[4].xCoor = i+L[i+(j+1)*rs].x; 
	LPoints[4].yCoor = j+L[i+(j+1)*rs].y; 

	theta = MaximumAngle(i, j, LPoints, 5);
	A[i+j*rs] = acos(theta);
#else
	vect2Dint v1,v2;
	double acosthetamax;
	v1.x = L[i+j*rs].x; 
	v1.y = L[i+j*rs].y; 
	// +1,0
	v2.x = L[i+1+j*rs].x; 
	v2.y = L[i+1+j*rs].y; 
	acosthetamax = acos((double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y))));
	// -1,0
	v2.x = L[i-1+j*rs].x; 
	v2.y = L[i-1+j*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
	// 0,-1
	v2.x = L[i+(j-1)*rs].x; 
	v2.y = L[i+(j-1)*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
	// 0,+1
	v2.x = L[i+(j+1)*rs].x; 
	v2.y = L[i+(j+1)*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
#ifdef CONNEX8
	// 1,1
	v2.x = L[i+1+(j+1)*rs].x; 
	v2.y = L[i+1+(j+1)*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
	// 1,-1
	v2.x = L[i+1+(j-1)*rs].x; 
	v2.y = L[i+1+(j-1)*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
	// -1,-1
	v2.x = L[i-1+(j-1)*rs].x; 
	v2.y = L[i-1+(j-1)*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
	// -1,+1
	v2.x = L[i-1+(j+1)*rs].x; 
	v2.y = L[i-1+(j+1)*rs].y; 
	theta = (double)(v1.x*v2.x + v1.y*v2.y)
	  / sqrt((double)((v1.x*v1.x + v1.y*v1.y)*(v2.x*v2.x + v2.y*v2.y)));
	acosthetamax = mcmax(acos(theta), acosthetamax);
#endif
	A[i+j*rs] = acosthetamax;
#endif
      } // if
    } // for i
  } // for j
  free(L);
  return 1;
} // lmedialaxis_lbisector_talbot()

/* ==================================== */
int32_t lmedialaxis_Downstream(int32_t x, int32_t y, uint32_t *image,
		int32_t rs, int32_t cs, 
		int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		ListPoint2D Aval)
/* ==================================== */
// Calcule l'aval du point (x,y)
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval
// Retourne le nombre de points de l'aval
{
#undef F_NAME
#define F_NAME "lmedialaxis_Downstream"
  int32_t nb, i, j, xx, yy, rr, counter, c, ti, d, nbdec;
  Neighbors MgN1;
  int32_t xnew, ynew;

  counter=0;

  i = (int32_t)(image[y*rs + x]);
  if (i == 0) return 0;
  ti = TabIndDec[i];
  nbdec = TabIndDec[i+1] - ti;
  for(d = 0; d < nbdec; d++)
  {
    xnew = ListDecs[ti+d].x; ynew = ListDecs[ti+d].y;
    nb = lmedialaxis_ApplySymmetriesB(xnew, ynew, x, y, rs, cs, &MgN1);	
    for (j = 0; j < nb; j++)
    {
      xx = x + MgN1.neig[j].x; yy = y + MgN1.neig[j].y; 
      rr = (int32_t)(image[yy*rs + xx]);
      if (rr == 0) // if one zero is found
      { 
	for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	  if (xx==Aval[c].xCoor && yy==Aval[c].yCoor) goto skip;
	Aval[counter].xCoor=xx; Aval[counter].yCoor=yy; counter++;
#define VARIANTE
#ifdef VARIANTE
	return counter;
#endif
      skip: ;
      } // if (rr == 0)
    } // for (j = 0; j < nb; j++)
  } // for(d = 0; d < nbdec; d++)
  return counter;
} // lmedialaxis_Downstream()

/* ==================================== */
int32_t lmedialaxis_Downstream3d(int32_t x, int32_t y, int32_t z, uint32_t *image,
		int32_t rs, int32_t cs, int32_t ds, 
		int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		ListPoint3D Aval)
/* ==================================== */
// Calcule l'aval du point (x,y, z)
// x, y, z : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval
// Retourne le nombre de points de l'aval
{
#undef F_NAME
#define F_NAME "lmedialaxis_Downstream3d"
  int32_t nb, i, j, xx, yy, zz, rr, counter, c, ti, d, nbdec;
  int32_t ps = rs*cs;
  Neighbors MgN1;
  int32_t xnew, ynew, znew;

  counter=0;

  i = (int32_t)(image[z*ps + y*rs + x]);
  if (i == 0) return 0;
  ti = TabIndDec[i];
  nbdec = TabIndDec[i+1] - ti;
  for(d = 0; d < nbdec; d++)
  {
    xnew = ListDecs[ti+d].x; 
    ynew = ListDecs[ti+d].y; 
    znew = ListDecs[ti+d].z;
    nb = lmedialaxis_ApplySymmetriesB3d(xnew, ynew, znew, x, y, z, rs, cs, ds, &MgN1);	
    for (j = 0; j < nb; j++)
    {
      xx = x + MgN1.neig[j].x; 
      yy = y + MgN1.neig[j].y; 
      zz = z + MgN1.neig[j].z; 
      rr = (int32_t)(image[zz*ps + yy*rs + xx]);
      if (rr == 0) // if one zero is found
      { 
	for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	  if (xx==Aval[c].xCoor && yy==Aval[c].yCoor && zz==Aval[c].zCoor) goto skip;
	Aval[counter].xCoor=xx; 
	Aval[counter].yCoor=yy; 
	Aval[counter].zCoor=zz; 
	counter++;
      skip: ;
      } // if (rr == 0)
    } // for (j = 0; j < nb; j++)
  } // for(d = 0; d < nbdec; d++)
  return counter;
} // lmedialaxis_Downstream3d()

/* ==================================== */
int32_t lmedialaxis_lprintdownstream(struct xvimage *id)
/* ==================================== */
/*
  Imprime le downstream de chaque point objet (pour tests)
*/
{
#undef F_NAME
#define F_NAME "lmedialaxis_lprintdownstream"
  int32_t i, j, k, nval, npoints, npointsmax;
  int32_t rs = rowsize(id);
  int32_t cs = colsize(id);
  int32_t ds = depth(id);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;
  int32_t *TabIndDec;
  Coordinates *ListDecs;
  FILE *fd=NULL;
  uint32_t *imagedist = ULONGDATA(id);
  char tablefilename[512];
  int32_t distmax; 
  int32_t card_downstream, n;

  if (datatype(id) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: distance image must be int32_t\n", F_NAME);
    return 0;
  }    

  distmax = 0;  // calcule la distance max dans l'image de distance
  for (i = 0; i < N; i++)
    if (imagedist[i] > distmax) distmax = imagedist[i];
  distmax++;

  if (ds == 1) // 2D
  {
    ListPoint2D Aval;
    Aval= (struct Point2D *)calloc(1,N*sizeof(struct Point2D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }
    /* Reading the bissector table from the files */
    sprintf(tablefilename, "%s/src/tables/TabBisector_1.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }

    fscanf(fd,"%d",&nval);
    if (distmax >= nval)
    {
      fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
      exit(0);
    }
    TabIndDec = (int32_t *)calloc(1,(distmax+2) * sizeof(int32_t));
    if (TabIndDec == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i <= distmax+1; i++) fscanf(fd,"%d", &TabIndDec[i]);
    npointsmax = TabIndDec[distmax];
    fclose(fd);

    sprintf(tablefilename, "%s/src/tables/TabBisector_2.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }
    fscanf(fd,"%d",&npoints);
    ListDecs = (Coordinates *)calloc(1,npointsmax * sizeof(Coordinates));
    if (ListDecs == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < npointsmax; i++) fscanf(fd,"%d%d", &ListDecs[i].x, &ListDecs[i].y);
    fclose (fd);

    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
      {
	card_downstream = lmedialaxis_Downstream(i, j, imagedist, rs, cs, TabIndDec, nval, ListDecs, Aval);
	if ((imagedist[j*rs + i]) && card_downstream)
	{
	  printf("[%d,%d,%d] : ", i, j, 0);//BUG
	  for (n = 0; n < card_downstream; n++) 
	    printf("(%d,%d,%d) ", Aval[n].xCoor, Aval[n].yCoor, 0);//BUG
	  printf("\n");	  
	}
      }
    free(Aval);
  } // if (ds == 1)
  else // 3D
  {
    ListPoint3D Aval;
    Aval= (struct Point3D *)calloc(1,N*sizeof(struct Point3D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }
    /* Reading the bissector table from the file */
    sprintf(tablefilename, "%s/src/tables/TabBisector3d_1.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table\n", F_NAME);
      return(0);
    }

    fscanf(fd,"%d",&nval);
    if (distmax >= nval)
    {
      fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
      exit(0);
    }
    TabIndDec = (int32_t *)calloc(1,(distmax+2) * sizeof(int32_t));
    if (TabIndDec == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i <= distmax+1; i++) fscanf(fd,"%d", &TabIndDec[i]);
    npointsmax = TabIndDec[distmax];
    fclose(fd);

    sprintf(tablefilename, "%s/src/tables/TabBisector3d_2.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }
    fscanf(fd,"%d",&npoints);
#ifdef PARANO
    if (npointsmax >= npoints)
    {
      fprintf(stderr, "%s: bisector table overflow for npoints: %d >= %d\n", F_NAME, npointsmax, npoints);
      exit(0);
    }
#endif
    ListDecs = (Coordinates *)calloc(1,npoints * sizeof(Coordinates));
    if (ListDecs == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < npointsmax; i++) fscanf(fd,"%d%d%d", &ListDecs[i].x, &ListDecs[i].y, &ListDecs[i].z);
    fclose (fd);

    for (k = 0; k < ds; k++)
      for (j = 0; j < cs; j++)
	for (i = 0; i < rs; i++)
        {
	  card_downstream = lmedialaxis_Downstream3d(i, j, k, imagedist, rs, cs, ds, TabIndDec, nval, ListDecs, Aval);
	  if ((imagedist[k*ps + j*rs + i]) && card_downstream)
	  {
	    printf("[%d,%d,%d] : ", i, j, k);
	    for (n = 0; n < card_downstream; n++) 
	      printf("(%d,%d,%d) ", Aval[n].xCoor, Aval[n].yCoor, Aval[n].zCoor);
	    printf("\n");	  
	  }
	}
    free(Aval);
  } // else (3D)

  free(TabIndDec);
  free(ListDecs);
  return 1;
} // lmedialaxis_lprintdownstream3d()

/* ==================================== */
/* ==================================== */
/* ==================================== */

// Data structures and functions for the lambda-medial axis

/* ==================================== */
/* ==================================== */
/* ==================================== */

struct  DPoint2D{
    double xCoor, yCoor;
};
struct  DPoint3D{
    double xCoor, yCoor, zCoor;
};
typedef struct DPoint2D * ListDPoint2D;
typedef struct DPoint3D * ListDPoint3D;

/* Function that returns the maximum diameter of the point set */
/* ==================================== */
double MaximumDiameter(ListDPoint2D LPoints, int32_t count)
/* ==================================== */
{
  int32_t i, j;
  double x1, y1, x2, y2;
  double tmp, MaxDiam;

  MaxDiam = 0.0;

  if (count > 1)
  {
    for (i = 0; i < count-1; i++)
    {
      x1 = LPoints[i].xCoor; y1 = LPoints[i].yCoor;
      for(j = i+1; j < count; j++)
      {
	x2 = LPoints[j].xCoor; y2 = LPoints[j].yCoor;
	tmp = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
	if (tmp > MaxDiam) MaxDiam=tmp;
      }
    }	
  }
  return MaxDiam;
} // MaximumDiameter()

/* Function that returns the maximum diameter of the point set in 3D */
/* ==================================== */
double MaximumDiameter3d(ListDPoint3D LPoints, int32_t count)
/* ==================================== */
{
  int32_t i, j;
  double x1, y1, z1, x2, y2, z2;
  double tmp, MaxDiam;

  MaxDiam = 0.0;

  if (count > 1)
  {
    for (i = 0; i < count-1; i++)
    {
      x1 = LPoints[i].xCoor; y1 = LPoints[i].yCoor; z1 = LPoints[i].zCoor;
      for(j = i+1; j < count; j++)
      {
	x2 = LPoints[j].xCoor; y2 = LPoints[j].yCoor; z2 = LPoints[j].zCoor;
	tmp = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
	if (tmp > MaxDiam) MaxDiam=tmp;
      }
    }	
  }
  return MaxDiam;
} // MaximumDiameter3d()

/* ==================================== */
int32_t Extendedlmedialaxis_Downstream(int32_t x, int32_t y, uint32_t *image,
		    int32_t rs, int32_t cs, 
		    int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		    ListDPoint2D Aval)
/* ==================================== */
// Calcule l'aval étendu du point (x,y)
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval étendu
// Retourne le nombre de points de l'aval étendu
{
#undef F_NAME
#define F_NAME "Extendedlmedialaxis_Downstream"
  int32_t nb, i, j, xx, yy, rr, counter, k, c, ti, d, nbdec;
  Neighbors MgN1;
  int32_t xnew, ynew;

  int32_t X[5] = {x, x-1, x+1, x, x};
  int32_t Y[5] = {y, y, y, y-1, y+1};

  counter=0;

  for (k = 0; k < 5; k++) // k indexes the point in test and its 4 neighbors
  {
    if (X[k] < rs && Y[k] < cs && X[k] >= 0 && Y[k] >= 0)
    {
      i = (int32_t)(image[Y[k]*rs + X[k]]);
      if (i == 0) goto endfor;
      if (i > image[y*rs + x]) goto endfor;
#ifdef PARANO
      if (i >= nval)
      {
	fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, i, nval);
	exit(0);
      }    
#endif
      ti = TabIndDec[i];
      nbdec = TabIndDec[i+1] - ti;
      for(d = 0; d < nbdec; d++)
      {
	xnew = ListDecs[ti+d].x; ynew = ListDecs[ti+d].y;
	nb = lmedialaxis_ApplySymmetriesB(xnew, ynew, X[k], Y[k], rs, cs, &MgN1);	
	for (j = 0; j < nb; j++)
        {
	  xx = X[k] + MgN1.neig[j].x; yy = Y[k] + MgN1.neig[j].y; 
	  rr = (int32_t)(image[yy*rs + xx]);
	  
	  if (rr == 0) // if one zero is found
	  { 
	    for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	      if (xx==Aval[c].xCoor && yy==Aval[c].yCoor) goto skip;
	    Aval[counter].xCoor=xx; Aval[counter].yCoor=yy; counter++;		
	  skip: ;
	  } // if (rr == 0)
	} // for (j = 0; j < nb; j++)
      } // for(d = 0; d < nbdec; d++)
    } // if
  endfor: ;
  } // for(k = 0; k < 5; k++)

#ifdef PARANO
  if (counter == 0) 
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d\n", counter, x, y);
    exit(0);
  }
#endif

  return counter;
} // Extendedlmedialaxis_Downstream()

/* ==================================== */
int32_t Extendedlmedialaxis_Downstream3d (int32_t x, int32_t y, int32_t z, uint32_t *image,
		       int32_t rs, int32_t cs, int32_t ds, 
		       int32_t *TabIndDec, int32_t nval, Coordinates *ListDecs,
		       ListDPoint3D Aval)
/* ==================================== */
// Calcule l'aval étendu du point (x,y,z)
// x, y, z : le point de base
// image : la carte de distance euclidinenne quadratique
// TabIndDec, nval, ListDecs : la "table" pour calculer l'aval
// Aval : tableau pour stocker les points de l'aval étendu
// Retourne le nombre de points de l'aval étendu
{
#undef F_NAME
#define F_NAME "Extendedlmedialaxis_Downstream3d"
  int32_t nb, i, j, xx, yy, zz, rr, counter, k, c, ti, d, nbdec, ps = rs*cs;
  Neighbors MgN1;
  int32_t xnew, ynew, znew;

  int32_t X[7] = {x, x-1, x+1, x,   x,   x,   x};
  int32_t Y[7] = {y, y,   y,   y-1, y+1, y,   y};
  int32_t Z[7] = {z, z,   z,   z,   z,   z-1, z+1};

  counter = 0;

  for (k = 0; k < 7; k++) // k indexes the point in test and its 6 neighbors
  {
    if (X[k] < rs && Y[k] < cs && Z[k] < ds && X[k] >= 0 && Y[k] >= 0 && Z[k] >= 0)
    {
      i = (int32_t)(image[Z[k]*ps + Y[k]*rs + X[k]]);
      if (i == 0) goto endfor;
      if (i > image[z*ps + y*rs + x]) goto endfor;
#ifdef PARANO
      if (i >= nval)
      {
	fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, i, nval);
	exit(0);
      }    
#endif
      ti = TabIndDec[i];
      nbdec = TabIndDec[i+1] - ti;
      for (d = 0; d < nbdec; d++)
      {
	xnew = ListDecs[ti+d].x; // direction of search to reach the first zero (CTg table)
	ynew = ListDecs[ti+d].y;
	znew = ListDecs[ti+d].z;
	nb = lmedialaxis_ApplySymmetriesB3d(xnew, ynew, znew, X[k], Y[k], Z[k], rs, cs, ds, &MgN1);	
	for (j = 0; j < nb; j++)
        {
	  xx = X[k] + MgN1.neig[j].x; 
	  yy = Y[k] + MgN1.neig[j].y; 
	  zz = Z[k] + MgN1.neig[j].z; 
	  rr = (int32_t)(image[zz*ps + yy*rs + xx]);
	  
	  if (rr == 0) // if one zero is found
	  { 
	    for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
	      if ((xx == Aval[c].xCoor) && (yy == Aval[c].yCoor) && (zz == Aval[c].zCoor))
		goto stoploops;
	    Aval[counter].xCoor = xx;
	    Aval[counter].yCoor = yy;
	    Aval[counter].zCoor = zz;
	    counter++;		
	  stoploops: ;
	  } // if (rr == 0)
	} // for (j = 0; j < nb; j++)
      } // for (d = 0; d < nbdec; d++)
    } // if
  endfor: ;
  } // for (k = 0; k < 7; k++)

#ifdef PARANO
  if (counter == 0) 
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d,%d\n", counter, x, y, z);
    exit(0);
  }
#endif

  return counter;
} // Extendedlmedialaxis_Downstream3d()
 
/* ==================================== */
int32_t llambdamedialaxis(struct xvimage *dist, struct xvimage *lambda)
/* ==================================== */
/*
   Calcule la fonction "lambda-axe médian discret" de l'objet 
   dont la carte de distance euclidienne au carré est dans 'dist'.
   L'image 'lambda' (type float) doit être allouée à l'avance. 
 
*/
{
#undef F_NAME
#define F_NAME "llambdamedialaxis"
  int32_t i, j, k, nval, npoints, npointsmax;
  int32_t rs = rowsize(dist);
  int32_t cs = colsize(dist);
  int32_t ds = depth(dist);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;
  int32_t *TabIndDec;
  Coordinates *ListDecs;
  FILE *fd=NULL;
  char tablefilename[512];
  int32_t distmax; 
  int32_t card_aval;
  uint32_t *imagedist;
  float *imagelambda;
  double c_x, c_y, c_z, c_r;

  if (datatype(dist) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: distance image must be long\n", F_NAME);
    return 0;
  }    

  if (datatype(lambda) != VFF_TYP_FLOAT)
  {
    fprintf(stderr, "%s: lambda image must be float\n", F_NAME);
    return 0;
  }    

  if ((rowsize(lambda) != rs) || (colsize(lambda) != cs) || (depth(lambda) != ds))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    

  imagedist = ULONGDATA(dist);
  imagelambda = FLOATDATA(lambda);
  razimage(lambda); // pour stocker le résulat

  distmax = 0;  // calcule la distance max dans l'image de distance
  for (i = 0; i < N; i++)
    if ((imagedist[i] > distmax)) distmax = imagedist[i];
  distmax++;

  if (ds == 1) // 2D
  {
    ListDPoint2D Aval;
    Aval= (struct DPoint2D *)calloc(N, sizeof(struct DPoint2D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }
    /* Reading the bisector table from the files */
    sprintf(tablefilename, "%s/src/tables/TabBisector_1.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }

    fscanf(fd,"%d",&nval);
    if (distmax >= nval)
    {
      fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
      exit(0);
    }
    TabIndDec = (int32_t *)calloc(1,(distmax+2) * sizeof(int32_t));
    if (TabIndDec == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i <= distmax+1; i++) fscanf(fd,"%d", &TabIndDec[i]);
    npointsmax = TabIndDec[distmax];
    fclose(fd);

    sprintf(tablefilename, "%s/src/tables/TabBisector_2.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }
    fscanf(fd,"%d",&npoints);
#ifdef PARANO
    if (npointsmax >= npoints)
    {
      fprintf(stderr, "%s: bisector table overflow for npoints: %d >= %d\n", F_NAME, npointsmax, npoints);
      exit(0);
    }
#endif
    ListDecs = (Coordinates *)calloc(1,npointsmax * sizeof(Coordinates));
    if (ListDecs == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < npointsmax; i++) fscanf(fd,"%d%d", &ListDecs[i].x, &ListDecs[i].y);
    fclose (fd);
#ifdef VERBOSE
printf("distmax = %d ; nval = %d ; npointsmax = %d ; npoints = %d\n", distmax, nval, npointsmax, npoints);
#endif

    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
      {
	if (imagedist[j*rs + i] != 0)
        {
	  card_aval = Extendedlmedialaxis_Downstream(i, j, imagedist, rs, cs, 
				 TabIndDec, nval, ListDecs, Aval);	  

		compute_min_disk_with_border_constraint((double *)Aval, card_aval, NULL, 0, &c_x, &c_y, &c_r);
	  imagelambda[j*rs + i] = (float)c_r;

	}
      }
    free(Aval);
  } // if (ds == 1)
  else // 3D
  {
    ListDPoint3D Aval;
    Aval= (struct DPoint3D *)calloc(1,N*sizeof(struct DPoint3D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }
    /* Reading the bissector table from the file */
    sprintf(tablefilename, "%s/src/tables/TabBisector3d_1.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table\n", F_NAME);
      return(0);
    }

    fscanf(fd,"%d",&nval);
    if (distmax >= nval)
    {
      fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
      exit(0);
    }
    TabIndDec = (int32_t *)calloc(1,(distmax+2) * sizeof(int32_t));
    if (TabIndDec == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i <= distmax+1; i++) fscanf(fd,"%d", &TabIndDec[i]);
    npointsmax = TabIndDec[distmax];
    fclose(fd);

    sprintf(tablefilename, "%s/src/tables/TabBisector3d_2.txt", getenv("PINK"));
    fd = fopen (tablefilename, "r");
    if (fd == NULL) 
    {   
      fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tablefilename);
      return(0);
    }
    fscanf(fd,"%d",&npoints);
#ifdef PARANO
    if (npointsmax >= npoints)
    {
      fprintf(stderr, "%s: bisector table overflow for npoints: %d >= %d\n", F_NAME, npointsmax, npoints);
      exit(0);
    }
#endif
    ListDecs = (Coordinates *)calloc(1,npoints * sizeof(Coordinates));
    if (ListDecs == NULL) 
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return(0);
    }
    for (i = 0; i < npointsmax; i++) fscanf(fd,"%d%d%d", &ListDecs[i].x, &ListDecs[i].y, &ListDecs[i].z);
    fclose (fd);
#ifdef VERBOSE
printf("distmax = %d ; nval = %d ; npointsmax = %d ; npoints = %d\n", distmax, nval, npointsmax, npoints);
#endif

    for (k = 0; k < ds; k++)
      for (j = 0; j < cs; j++)
	for (i = 0; i < rs; i++)
	{
	  if (imagedist[k*ps + j*rs + i] != 0)
	  {
	    card_aval = Extendedlmedialaxis_Downstream3d(i, j, k, imagedist, rs, cs, ds, 
					   TabIndDec, nval, ListDecs, Aval);

	    compute_min_sphere_with_border_constraint((double *)Aval, card_aval, NULL, 0, &c_x, &c_y, &c_z, &c_r);
	    imagelambda[k*ps + j*rs + i] = (float)c_r;

	  }
	}

    free(Aval);
  } // else (3D)

  free(TabIndDec);
  free(ListDecs);
  return 1;
} // llambdamedialaxis()

/* ==================================== */
int32_t Extendedlmedialaxis_DownstreamLambdaPrime(
  int32_t x, int32_t y, 
  uint32_t *image, uint32_t *vor,
  int32_t rs, int32_t cs, 
  ListDPoint2D Aval)
/* ==================================== */
// Calcule l'aval étendu du point (x,y)
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// vor : le "voronoi labelling"
// Aval : tableau pour stocker les points de l'aval étendu
// Retourne le nombre de points de l'aval étendu
{
#undef F_NAME
#define F_NAME "Extendedlmedialaxis_DownstreamLambdaPrime"
  int32_t i, j, k, xx, yy;
  int32_t X[5] = {x, x-1, x+1, x, x};
  int32_t Y[5] = {y, y, y, y-1, y+1};
  int32_t counter=0;

  for (k = 0; k < 5; k++) // k indexes the point in test and its 4 neighbors
  {
    if (X[k] < rs && Y[k] < cs && X[k] >= 0 && Y[k] >= 0)
    {
      j = Y[k]*rs + X[k];
      i = (int32_t)(image[j]);
      if (i == 0) goto endfor;
      if (i > image[y*rs + x]) goto endfor;
      xx = vor[j] % rs;
      yy = vor[j] / rs;
      //      for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
      //	if ((xx!=Aval[c].xCoor) || (yy!=Aval[c].yCoor)) goto skip;
      Aval[counter].xCoor=xx; Aval[counter].yCoor=yy; counter++;		
    } // if
  endfor: ;
  } // for(k = 0; k < 5; k++)

#ifdef PARANO
  if (counter == 0) 
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d\n", counter, x, y);
    exit(0);
  }
#endif

  return counter;
} // Extendedlmedialaxis_DownstreamLambdaPrime()
 
/* ==================================== */
int32_t Extendedlmedialaxis_Downstream3dLambdaPrime(
  int32_t x, int32_t y, int32_t z,
  uint32_t *image, uint32_t *vor,
  int32_t rs, int32_t cs, int32_t ds,
  ListDPoint3D Aval)
/* ==================================== */
// Calcule l'aval étendu du point (x,y)
// x, y : le point de base
// image : la carte de distance euclidinenne quadratique
// vor : le "voronoi labelling"
// Aval : tableau pour stocker les points de l'aval étendu
// Retourne le nombre de points de l'aval étendu
{
#undef F_NAME
#define F_NAME "Extendedlmedialaxis_Downstream3dLambdaPrime"
  int32_t i, j, k, xx, yy, zz, ps = rs*cs;
  int32_t X[7] = {x, x-1, x+1, x,   x,   x,   x};
  int32_t Y[7] = {y, y,   y,   y-1, y+1, y,   y};
  int32_t Z[7] = {z, z,   z,   z,   z,   z+1, z-1};
  int32_t counter=0;

  for (k = 0; k < 7; k++) // k indexes the point in test and its 6 neighbors
  {
    if ((X[k] < rs) && (Y[k] < cs) && (Z[k] < ds) && (X[k] >= 0) && (Y[k] >= 0) && (Z[k] >= 0))
    {
      j = Z[k]*ps + Y[k]*rs + X[k];
      i = (int32_t)(image[j]);
      if (i == 0) goto endfor;
      if (i > image[z*ps + y*rs + x]) goto endfor;
      xx = vor[j] % rs;
      yy = (vor[j] % ps) / rs;
      zz = (vor[j] / ps);
      //      for (c = 0; c < counter; c++) // check:if the value already exists, no need to store it
      //	if ((xx!=Aval[c].xCoor) || (yy!=Aval[c].yCoor)) goto skip;
      Aval[counter].xCoor=xx; Aval[counter].yCoor=yy; Aval[counter].zCoor=zz; counter++;
      //    skip: ;
    } // if
  endfor: ;
  } // for(k = 0; k < 7; k++)

#ifdef PARANO
  if (counter == 0)
  {
    printf("ERROR (BUG) - Please report - count = %d point %d,%d\n", counter, x, y);
    exit(0);
  }
#endif

  return counter;
} // Extendedlmedialaxis_Downstream3dLambdaPrime()

/* ==================================== */
int32_t llambdaprimemedialaxis(struct xvimage *dist, struct xvimage *vor, struct xvimage *lambda)
/* ==================================== */
/*
   Calcule la fonction "lambda'-axe médian discret" de l'objet 
   dont la carte de distance euclidienne au carré est dans 'dist',
   et le voronoi labelling dans vor.
   L'image 'lambda' (type float) doit être allouée à l'avance. 
 
*/
{
#undef F_NAME
#define F_NAME "llambdaprimemedialaxis"
  int32_t i, j, k;
  int32_t rs = rowsize(dist);
  int32_t cs = colsize(dist);
  int32_t ds = depth(dist);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;
  int32_t card_aval;
  uint32_t *imagedist;
  uint32_t *imagevor;
  float *imagelambda;
  double c_x, c_y, c_r;

  if (datatype(dist) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: distance image must be long\n", F_NAME);
    return 0;
  }    

  if (datatype(vor) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: vor. lab. image must be long\n", F_NAME);
    return 0;
  }    

  if (datatype(lambda) != VFF_TYP_FLOAT)
  {
    fprintf(stderr, "%s: lambda image must be float\n", F_NAME);
    return 0;
  }    

  if ((rowsize(lambda) != rs) || (colsize(lambda) != cs) || (depth(lambda) != ds))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    

  if ((rowsize(vor) != rs) || (colsize(vor) != cs) || (depth(vor) != ds))
  {
    fprintf(stderr, "%s: imcompatible image sizes\n", F_NAME);
    return 0;
  }    

  imagedist = ULONGDATA(dist);
  imagevor = ULONGDATA(vor);
  imagelambda = FLOATDATA(lambda);
  razimage(lambda); // pour stocker le résulat

  if (ds == 1) // 2D
  {
    ListDPoint2D Aval;
    Aval= (struct DPoint2D *)calloc(1,N*sizeof(struct DPoint2D)); // LARGEMENT SURDIMENSIONE
    if (Aval == NULL)
    {   
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }

    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
      {
	if (imagedist[j*rs + i] != 0)
        {
	  card_aval = Extendedlmedialaxis_DownstreamLambdaPrime(i, j, imagedist, imagevor, rs, cs, Aval);	  

          compute_min_disk_with_border_constraint((double *)Aval, card_aval, NULL, 0, &c_x, &c_y, &c_r);
	  imagelambda[j*rs + i] = (float)c_r;

	}
      }
    free(Aval);
  } // if (ds == 1)
  else // 3D
  {
    ListDPoint3D Aval3d;
    double c_x, c_y, c_z, c_r;
    Aval3d= (struct DPoint3D *)calloc(1,N*sizeof(struct DPoint3D)); // LARGEMENT SURDIMENSIONE
    if (Aval3d == NULL)
    {
      fprintf(stderr, "%s: malloc failed\n", F_NAME);
      return 0;
    }

    for (k = 0; k < ds; k++)
      for (j = 0; j < cs; j++)
        for (i = 0; i < rs; i++)
      {
	if (imagedist[k*ps + j*rs + i] != 0)
        {
	  card_aval = Extendedlmedialaxis_Downstream3dLambdaPrime(i, j, k, imagedist, imagevor, rs, cs, ds, Aval3d);

          compute_min_sphere_with_border_constraint((double *)Aval3d, card_aval, NULL, 0, &c_x, &c_y, &c_z, &c_r);
	  imagelambda[k*ps + j*rs + i] = (float)c_r;

	}
      }
    free(Aval3d);
  } // else (3D)

  return 1;
} // llambdaprimemedialaxis()
