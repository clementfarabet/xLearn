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
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mclifo.h>
#include <mclin.h>
#include <mcutil.h>
#include <lmoments.h>
#include <llabelextrema.h>

/****************************************************************
*
* Routine Name: lmoments - library call for moments
*
* Purpose: calcul des indicateurs de forme bases sur les moments
* Input:
* Output:
* Written By: Michel Couprie - novembre 1998
*
* Update april 1999 : version lmomentslab pour une image de labels
* Update nov. 2009 : versions 3d et listes de points
*
****************************************************************/

//#define DEBUG
//#define VERBOSE

#define EPSILON 1E-6

/* ==================================== */
int32_t limagemoments(
        struct xvimage *img,    /* image de depart */
        Indicateur * indic)     /* resultat */
/* ==================================== */
#undef F_NAME
#define F_NAME "limagemoments"
// WARNING : not tested
{
  int32_t i;
  int32_t rs = rowsize(img);
  int32_t cs = colsize(img);
  int32_t N = rs * cs;          /* taille image */
  uint8_t *F = UCHARDATA(img);
  int32_t area;
  double mx1, my1, mxy1, mx2, my2, mxy2;
  double Mxy1, Mx2, My2, Mxy2, delta;
  double lambda1, lambda2;
  double xx, yy, xc, yc;
  double theta;
  double gdiam;

  if (depth(img) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  area = 0; mx1 = my1 = mxy1 = mx2 = my2 = mxy2 = 0.0;
  for (i = 0; i < N; i++)
    if (F[i])
    {    
      area++; mx1 += i%rs; my1 += i/rs; mxy1 += ((i%rs) * (i/rs));
      mxy2 += (i%rs) * (i/rs); mx2 += (i%rs) * (i%rs); my2 += (i/rs) * (i/rs);
    }

  indic->area = area;
  xc = indic->xcentre = mx1/area;
  yc = indic->ycentre = my1/area;

     /* moments centres d'ordre 1 */
  Mxy1 = (mxy1 - (mx1 * yc)) - (my1 * xc) + (area * xc * yc);
      
     /* moments centres d'ordre 2 */
  Mx2 = mx2 - mx1 * mx1 / area;  
  My2 = my2 - my1 * my1 / area;  
  Mxy2 = mxy2 - mx1 * my1 / area;

     /* calcul des longueurs des demi-axes */
  delta = (Mx2 - My2) * (Mx2 - My2) + 4 * Mxy2 * Mxy2;
  lambda1 = sqrt(2 * (Mx2 + My2 + sqrt(delta)) / area);
  lambda2 = sqrt(2 * (Mx2 + My2 - sqrt(delta)) / area);
#ifdef DEBUG
printf("Mx2 = %g ; My2 = %g ; Mxy2 = %g ; delta = %g\n", Mx2, My2, Mxy2, delta);
printf("lambda1 = %g ; lambda2 = %g\n", lambda1, lambda2);
#endif
  if (lambda2 < 0.0)
  {
    fprintf(stderr, "%s: valeur propre negative : %g\n", F_NAME, lambda2);
    return 0;
  }

  if (mcabs(Mx2 - My2) < EPSILON) 
    theta = 0.0;
  else
  {
    double mi1, mi2, theta2;
    theta = 0.5 * atan( 2 * Mxy1 / (Mx2 - My2));
    /* la direction est determinee a pi/2 pres */
    /* on calcule les moments d'inertie pour theta et theta + pi/2 */
    /* pour lever l'ambiguite */
    mi1 = cos(theta) * cos(theta) * My2 + 
      sin(theta) * sin(theta) * Mx2 -
      2 *sin(theta) * cos(theta) * Mxy1;
    theta2 = theta + (M_PI / 2);
    mi2 = cos(theta2) * cos(theta2) * My2 + 
      sin(theta2) * sin(theta2) * Mx2 - 
      2 * sin(theta2) * cos(theta2) * Mxy1;
#ifdef DEBUG
printf("mi1 = %g ; mi2 = %g ; theta1 = %g ; theta2 = %g\n", 
        mi1, mi2, theta * 180 / M_PI, theta2 * 180 / M_PI);
#endif
    if (mi2 < mi1) theta = theta2;
  }
  indic->gaxe = lambda1;
  indic->excen = lambda2 / lambda1;
  indic->orien = theta;

  gdiam = 0.0; /* calcule le plus grand demi-diametre */
  for (i = 0; i < N; i++)
  {
    xx = (double)(i%rs);
    yy = (double)(i/rs);
    theta = (xc - xx)*(xc - xx) + (yc - yy)*(yc - yy);
    if (theta > gdiam) gdiam = theta;
  } // for (i = 0; i < N; i++)
  indic->gdiam = sqrt(gdiam);

  return(1);
} // limagemoments

/* ==================================== */
int32_t lmoments(
        struct xvimage *img,    /* image de depart */
        int32_t connex,             /* 4, 8  */
        int32_t minimum,            /* booleen */
        Indicateur ** tabindic, /* tableau des resultats */
        int32_t *nlabels)           /* resultat: nombre d'extrema traites */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmoments"
{
  int32_t k;
  int32_t w, x, y;
  int32_t rs = rowsize(img);
  int32_t cs = colsize(img);
  int32_t d = depth(img);
  int32_t N = rs * cs;          /* taille image */
  struct xvimage * labels;
  int32_t *LABEL;
  int32_t label;
  Lifo * LIFO;
  int32_t area;
  double mx1, my1, mxy1, mx2, my2, mxy2;
  double Mxy1, Mx2, My2, Mxy2, delta;
  double lambda1, lambda2;
  int32_t incr_vois;
  double xx, yy, xc, yc;
  double theta;
  double gdiam;
  Indicateur * tab;
  int32_t n;

  if (depth(img) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

  labels = allocimage(NULL, rs, cs, d, VFF_TYP_4_BYTE);
  if (labels == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  LABEL = SLONGDATA(labels);

  if (!llabelextrema(img, connex, minimum, labels, &n))
  {
    fprintf(stderr, "%s: llabelextrema failed\n", F_NAME);
    return 0;
  }

  tab = (Indicateur *)calloc(1,(n - 1) * sizeof(Indicateur));
  if (tab == NULL)
  {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    return 0;
  }

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)
  {
    if (LABEL[x])
    {
#ifdef DEBUG
printf("\npoint %d %d : nouvelle composante\n", x%rs, x/rs);
#endif
      label = LABEL[x];
      LABEL[x] = n+1; /* marquage provisoire */
      area = 0; mx1 = my1 = mxy1 = mx2 = my2 = mxy2 = 0.0;
      LifoPush(LIFO, x);         /* on va parcourir 2x la comp. a laquelle appartient x */
      while (! LifoVide(LIFO))   /* premier parcours */
      {
        w = LifoPop(LIFO);
        area++; mx1 += w%rs; my1 += w/rs; mxy1 += ((w%rs) * (w/rs));
        mxy2 += (w%rs) * (w/rs); mx2 += (w%rs) * (w%rs); my2 += (w/rs) * (w/rs);
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (LABEL[y] == label))
	  {
            LABEL[y] = n+1;
            LifoPush(LIFO, y);
	  }
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */

      tab[label-1].area = area;
      xc = tab[label-1].xcentre = mx1/area;
      yc = tab[label-1].ycentre = my1/area;

      /* moments centres d'ordre 1 */
      Mxy1 = (mxy1 - (mx1 * yc)) - (my1 * xc) + (area * xc * yc);
      
      /* moments centres d'ordre 2 */
      Mx2 = mx2 - mx1 * mx1 / area;  
      My2 = my2 - my1 * my1 / area;  
      Mxy2 = mxy2 - mx1 * my1 / area;

      /* calcul des longueurs des demi-axes */
      delta = (Mx2 - My2) * (Mx2 - My2) + 4 * Mxy2 * Mxy2;
      lambda1 = sqrt(2 * (Mx2 + My2 + sqrt(delta)) / area);
      lambda2 = sqrt(2 * (Mx2 + My2 - sqrt(delta)) / area);
#ifdef DEBUG
printf("Mx2 = %g ; My2 = %g ; Mxy2 = %g ; delta = %g\n", Mx2, My2, Mxy2, delta);
printf("lambda1 = %g ; lambda2 = %g\n", lambda1, lambda2);
#endif
      if (lambda2 < 0.0)
      {
        fprintf(stderr, "%s: valeur propre negative : %g\n", F_NAME, lambda2);
        return 0;
      }

      if (mcabs(Mx2 - My2) < EPSILON) 
        theta = 0.0;
      else
      {
        double mi1, mi2, theta2;
        theta = 0.5 * atan( 2 * Mxy1 / (Mx2 - My2));
        /* la direction est determinee a pi/2 pres */
        /* on calcule les moments d'inertie pour theta et theta + pi/2 */
        /* pour lever l'ambiguite */
        mi1 = cos(theta) * cos(theta) * My2 + 
              sin(theta) * sin(theta) * Mx2 -
              2 *sin(theta) * cos(theta) * Mxy1;
        theta2 = theta + (M_PI / 2);
        mi2 = cos(theta2) * cos(theta2) * My2 + 
              sin(theta2) * sin(theta2) * Mx2 - 
              2 * sin(theta2) * cos(theta2) * Mxy1;
#ifdef DEBUG
printf("mi1 = %g ; mi2 = %g ; theta1 = %g ; theta2 = %g\n", 
        mi1, mi2, theta * 180 / M_PI, theta2 * 180 / M_PI);
#endif
        if (mi2 < mi1) theta = theta2;
      }

      tab[label-1].gaxe = lambda1;
      tab[label-1].excen = lambda2 / lambda1;
      tab[label-1].orien = theta;

      gdiam = 0.0;
      LABEL[x] = 0;
      LifoPush(LIFO, x);         /* on re-parcourt la composante en demarquant */      
      while (! LifoVide(LIFO))   /* et on calcule le plus grand demi-diametre */
      {
        w = LifoPop(LIFO);
        xx = (double)(w%rs);
        yy = (double)(w/rs);
        theta = (xc - xx)*(xc - xx) + (yc - yy)*(yc - yy);
        if (theta > gdiam) gdiam = theta;
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (LABEL[y] == n + 1)) 
          {
            LABEL[y] = 0;
            LifoPush(LIFO, y);
	  }
        } /* for k ... */          
      } /* while (! LifoVide(LIFO)) */
      tab[label-1].gdiam = sqrt(gdiam);
    } /* if (LABEL[x]) */
  } /* for (x = 0; x < N; x++) */

  *nlabels = n - 1; /* designe le nombre de composantes (etiquettes non nulles) */
  *tabindic = tab;

  LifoTermine(LIFO);

  freeimage(labels);

  return(1);
} // lmoments

/* ==================================== */
int32_t lmomentslab(
        struct xvimage *labels, /* image de depart (cc etiquetees) */
        int32_t connex,             /* 4, 8  */
        int32_t nlabels,            /* nombre de composantes */
        Indicateur ** tabindic) /* tableau des resultats */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmomentslab"
{
  int32_t k;
  int32_t w, x, y;
  int32_t rs = rowsize(labels);
  int32_t cs = colsize(labels);
  int32_t d = depth(labels);
  int32_t N = rs * cs;          /* taille image */
  int32_t *LABEL = SLONGDATA(labels);
  int32_t label;
  Lifo * LIFO;
  int32_t area;
  double mx1, my1, mxy1, mx2, my2, mxy2;
  double Mxy1, Mx2, My2, Mxy2, delta;
  double lambda1, lambda2;
  int32_t incr_vois;
  double xx, yy, xc, yc;
  double theta;
  double gdiam;
  Indicateur * tab;

  if (d != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */

  tab = (Indicateur *)calloc(1,nlabels * sizeof(Indicateur));
  if (tab == NULL)
  {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    return 0;
  }

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
    {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)
  {
    if (LABEL[x])
    {
#ifdef DEBUG
printf("\npoint %d %d : nouvelle composante\n", x%rs, x/rs);
#endif
      label = LABEL[x];
      LABEL[x] = nlabels+1; /* marquage provisoire */
      area = 0; mx1 = my1 = mxy1 = mx2 = my2 = mxy2 = 0.0;
      LifoPush(LIFO, x);         /* on va parcourir 2x la comp. a laquelle appartient x */
      while (! LifoVide(LIFO))   /* premier parcours */
      {
        w = LifoPop(LIFO);
        area++; mx1 += w%rs; my1 += w/rs; mxy1 += ((w%rs) * (w/rs));
        mxy2 += (w%rs) * (w/rs); mx2 += (w%rs) * (w%rs); my2 += (w/rs) * (w/rs);
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (LABEL[y] == label))
	  {
            LABEL[y] = nlabels+1;
            LifoPush(LIFO, y);
	  }
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */

      tab[label-1].area = area;
      xc = tab[label-1].xcentre = mx1/area;
      yc = tab[label-1].ycentre = my1/area;

      /* moments centres d'ordre 1 */
      Mxy1 = (mxy1 - (mx1 * yc)) - (my1 * xc) + (area * xc * yc);
      
      /* moments centres d'ordre 2 */
      Mx2 = mx2 - mx1 * mx1 / area;  
      My2 = my2 - my1 * my1 / area;  
      Mxy2 = mxy2 - mx1 * my1 / area;

      /* calcul des longueurs des demi-axes */
      delta = (Mx2 - My2) * (Mx2 - My2) + 4 * Mxy2 * Mxy2;
      lambda1 = sqrt(2 * (Mx2 + My2 + sqrt(delta)) / area);
      lambda2 = sqrt(2 * (Mx2 + My2 - sqrt(delta)) / area);
#ifdef DEBUG
printf("Mx2 = %g ; My2 = %g ; Mxy2 = %g ; delta = %g\n", Mx2, My2, Mxy2, delta);
printf("lambda1 = %g ; lambda2 = %g\n", lambda1, lambda2);
#endif
      if (lambda2 < 0.0)
      {
        fprintf(stderr, "%s: valeur propre negative : %g\n", F_NAME, lambda2);
        return 0;
      }

      if (mcabs(Mx2 - My2) < EPSILON) 
        theta = 0.0;
      else
      {
        double mi1, mi2, theta2;
        theta = 0.5 * atan( 2 * Mxy1 / (Mx2 - My2));
        /* la direction est determinee a pi/2 pres */
        /* on calcule les moments d'inertie pour theta et theta + pi/2 */
        /* pour lever l'ambiguite */
        mi1 = cos(theta) * cos(theta) * My2 + 
              sin(theta) * sin(theta) * Mx2 -
              2 *sin(theta) * cos(theta) * Mxy1;
        theta2 = theta + (M_PI / 2);
        mi2 = cos(theta2) * cos(theta2) * My2 + 
              sin(theta2) * sin(theta2) * Mx2 - 
              2 * sin(theta2) * cos(theta2) * Mxy1;
#ifdef DEBUG
printf("mi1 = %g ; mi2 = %g ; theta1 = %g ; theta2 = %g\n", 
        mi1, mi2, theta * 180 / M_PI, theta2 * 180 / M_PI);
#endif
        if (mi2 < mi1) theta = theta2;
      }

      tab[label-1].gaxe = lambda1;
      tab[label-1].excen = lambda2 / lambda1;
      tab[label-1].orien = theta;

      gdiam = 0.0;
      LABEL[x] = 0;
      LifoPush(LIFO, x);         /* on re-parcourt la composante en demarquant */      
      while (! LifoVide(LIFO))   /* et on calcule le plus grand demi-diametre */
      {
        w = LifoPop(LIFO);
        xx = (double)(w%rs);
        yy = (double)(w/rs);
        theta = (xc - xx)*(xc - xx) + (yc - yy)*(yc - yy);
        if (theta > gdiam) gdiam = theta;
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (LABEL[y] == nlabels + 1)) 
          {
            LABEL[y] = 0;
            LifoPush(LIFO, y);
	  }
        } /* for k ... */          
      } /* while (! LifoVide(LIFO)) */
      tab[label-1].gdiam = sqrt(gdiam);
    } /* if (LABEL[x]) */
  } /* for (x = 0; x < N; x++) */

  *tabindic = tab;

  LifoTermine(LIFO);

  return(1);
} // lmomentslab

/* ==================================== */
void PrintIndicateur(Indicateur I)
/* ==================================== */
{
  printf("area = %d \t", I.area);
  printf("centre = (%d,%d) \t", (int32_t)I.xcentre, (int32_t)I.ycentre);
  printf("excen = %g \t", I.excen);
  printf("orien = %g \t", I.orien);
  printf("gdiam = %g \t", I.gdiam);
  printf("\n");
}

/* ==================================== */
int32_t llistemoments2d(
	double *X, double *Y, int32_t N,
        Indicateur * indic)     /* resultat */
/* ==================================== */
// WARNING : not tested
#undef F_NAME
#define F_NAME "llistemoments2d"
{
  int32_t i;
  double mx1, my1, mxy1, mx2, my2, mxy2;
  double Mxy1, Mx2, My2, Mxy2, delta;
  double lambda1, lambda2;
  double xx, yy, xc, yc;
  double theta;
  double gdiam;

  mx1 = my1 = mxy1 = mx2 = my2 = mxy2 = 0.0;
  for (i = 0; i < N; i++)
  {
     mx1 += X[i]; my1 += Y[i]; mxy1 += (X[i] * Y[i]);
     mxy2 += X[i] * Y[i]; mx2 += X[i] * X[i]; my2 += Y[i] * Y[i];
  }

  indic->area = N;
  xc = indic->xcentre = mx1/N;
  yc = indic->ycentre = my1/N;

  /* moments centres d'ordre 1 */
  Mxy1 = (mxy1 - (mx1 * yc)) - (my1 * xc) + (N * xc * yc);
      
  /* moments centres d'ordre 2 */
  Mx2 = mx2 - mx1 * mx1 / N;  
  My2 = my2 - my1 * my1 / N;  
  Mxy2 = mxy2 - mx1 * my1 / N;

  /* calcul des longueurs des demi-axes */
  delta = (Mx2 - My2) * (Mx2 - My2) + 4 * Mxy2 * Mxy2;
  lambda1 = sqrt(2 * (Mx2 + My2 + sqrt(delta)) / N);
  lambda2 = sqrt(2 * (Mx2 + My2 - sqrt(delta)) / N);
#ifdef DEBUG
printf("Mx2 = %g ; My2 = %g ; Mxy2 = %g ; delta = %g\n", Mx2, My2, Mxy2, delta);
printf("lambda1 = %g ; lambda2 = %g\n", lambda1, lambda2);
#endif
  if (lambda2 < 0.0)
  {
    fprintf(stderr, "%s : valeur propre negative : %g\n", F_NAME, lambda2);
    return 0;
  }

  if (mcabs(Mx2 - My2) < EPSILON) 
    theta = 0.0;
  else
  {
    double mi1, mi2, theta2;
    theta = 0.5 * atan( 2 * Mxy1 / (Mx2 - My2));
    /* la direction est determinee a pi/2 pres */
    /* on calcule les moments d'inertie pour theta et theta + pi/2 */
    /* pour lever l'ambiguite */
    mi1 = cos(theta) * cos(theta) * My2 + 
      sin(theta) * sin(theta) * Mx2 -
      2 *sin(theta) * cos(theta) * Mxy1;
    theta2 = theta + (M_PI / 2);
    mi2 = cos(theta2) * cos(theta2) * My2 + 
      sin(theta2) * sin(theta2) * Mx2 - 
      2 * sin(theta2) * cos(theta2) * Mxy1;
#ifdef DEBUG
printf("mi1 = %g ; mi2 = %g ; theta1 = %g ; theta2 = %g\n", 
        mi1, mi2, theta * 180 / M_PI, theta2 * 180 / M_PI);
#endif
    if (mi2 < mi1) theta = theta2;
  }
  indic->gaxe = lambda1;
  indic->excen = lambda2 / lambda1;
  indic->orien = theta;

  gdiam = 0.0; /* calcule le plus grand demi-diametre */
  for (i = 0; i < N; i++)
  {
    xx = X[i];
    yy = Y[i];
    theta = (xc - xx)*(xc - xx) + (yc - yy)*(yc - yy);
    if (theta > gdiam) gdiam = theta;
  } // for (i = 0; i < N; i++)
  indic->gdiam = sqrt(gdiam);

  return(1);
} // llistemoments2d

/* ==================================== */
int32_t ldirectionsprincipales2d(
	double *X, double *Y, int32_t N,
        double *xc, double *yc,      /* resultat */
        double *dx1, double *dy1,    /* resultat */
        double *dx2, double *dy2)    /* resultat */
/* ==================================== */
#undef F_NAME
#define F_NAME "directionsprincipales2d("
{
  int32_t i, ret;
  double mx1, my1;  // cumuls des variables x et y
  double mx2, my2, mxy2; // cumuls des x^2, y^2 et xy
  double cov[2][2]; // pour la matrice de covariance
  double D[2];      // pour les valeurs propres
  double V[2][2];   // pour les vecteurs propres

  mx1 = my1 = mx2 = my2 = mxy2 = 0.0;
  for (i = 0; i < N; i++)
  {
     mx1 += X[i]; my1 += Y[i];
     mx2 += X[i] * X[i]; my2 += Y[i] * Y[i];
     mxy2 += X[i] * Y[i]; 
  }

  // moments centres d'ordre 2 (matrice de variance - covariance)
  cov[0][0] = mx2 - mx1 * mx1 / N;  
  cov[1][1] = my2 - my1 * my1 / N;  
  cov[1][0] = cov[0][1] = mxy2 - mx1 * my1 / N;

  ret = lin_jacobi((double *)cov, 2, (double *)D, (double *)V, 0);      
  if (ret == 0) return 0;

#ifdef DEBUG
  printf("D = \n");
  lin_printmat((double *)D, 1, 2);
  printf("V = \n");
  lin_printmat((double *)V, 2, 2);
#endif

  *xc = mx1 / N; *yc = my1 / N;

  if (mcabs(D[1]) > mcabs(D[0])) 
  { 
    *dx1 = V[0][1]; *dy1 = V[1][1];
    *dx2 = V[0][0]; *dy2 = V[1][0];
  }
  else
  { 
    *dx1 = V[0][0]; *dy1 = V[1][0];
    *dx2 = V[0][1]; *dy2 = V[1][1];
  }

  return(1);
} // ldirectionsprincipales2d

/* ==================================== */
int32_t ldirectionsprincipales3d(
	double *X, double *Y, double *Z, int32_t N,
        double *xc, double *yc, double *zc,      /* resultat */
        double *dx1, double *dy1, double *dz1,   /* resultat */
        double *dx2, double *dy2, double *dz2,   /* resultat */
        double *dx3, double *dy3, double *dz3)   /* resultat */
/* ==================================== */
#undef F_NAME
#define F_NAME "directionsprincipales3d("
{
  int32_t i, ret;
  double mx1, my1, mz1; // cumuls des variables x, y et z
  double mx2, my2, mz2; // cumuls des x^2, y^2 et z^2
  double mxy2, myz2, mxz2; // cumuls des xy, yz et xz
  double cov[3][3]; // pour la matrice de covariance
  double D[3];      // pour les valeurs propres
  double V[3][3];   // pour les vecteurs propres
  double tmp;

  mx1 = my1 = mz1 = mx2 = my2 = mz2 = mxy2 = myz2 = mxz2 = 0.0; 
  for (i = 0; i < N; i++)
  {
     mx1 += X[i]; my1 += Y[i]; mz1 += Z[i];
     mx2 += X[i] * X[i]; my2 += Y[i] * Y[i]; mz2 += Z[i] * Z[i];
     mxy2 += X[i] * Y[i]; myz2 += Y[i] * Z[i]; mxz2 += X[i] * Z[i]; 
  }

  // moments centres d'ordre 2 (matrice de variance - covariance)
  cov[0][0] = mx2 - mx1 * mx1 / N;  
  cov[1][1] = my2 - my1 * my1 / N;  
  cov[2][2] = mz2 - mz1 * mz1 / N;  
  cov[1][0] = cov[0][1] = mxy2 - mx1 * my1 / N;
  cov[2][0] = cov[0][2] = mxz2 - mx1 * mz1 / N;
  cov[1][2] = cov[2][1] = myz2 - my1 * mz1 / N;

  ret = lin_jacobi((double *)cov, 3, (double *)D, (double *)V, 0);      
  if (ret == 0) return 0;

#ifdef DEBUG
  printf("cov = \n");
  lin_printmat((double *)cov, 3, 3);
  printf("D = \n");
  lin_printmat((double *)D, 1, 3);
  printf("V = \n");
  lin_printmat((double *)V, 3, 3);
#endif


  *xc = mx1 / N; *yc = my1 / N; *zc = mz1 / N;

  if (mcabs(D[1]) > mcabs(D[0])) 
  { 
    tmp = D[1]; D[1] = D[0]; D[0] = tmp;
    tmp = V[0][1]; V[0][1] = V[0][0]; V[0][0] = tmp;
    tmp = V[1][1]; V[1][1] = V[1][0]; V[1][0] = tmp;
    tmp = V[2][1]; V[2][1] = V[2][0]; V[2][0] = tmp;
  }
  if (mcabs(D[2]) > mcabs(D[1])) 
  { 
    tmp = D[2]; D[2] = D[1]; D[1] = tmp;
    tmp = V[0][2]; V[0][2] = V[0][1]; V[0][1] = tmp;
    tmp = V[1][2]; V[1][2] = V[1][1]; V[1][1] = tmp;
    tmp = V[2][2]; V[2][2] = V[2][1]; V[2][1] = tmp;
  }
  if (mcabs(D[1]) > mcabs(D[0])) 
  { 
    tmp = D[1]; D[1] = D[0]; D[0] = tmp;
    tmp = V[0][1]; V[0][1] = V[0][0]; V[0][0] = tmp;
    tmp = V[1][1]; V[1][1] = V[1][0]; V[1][0] = tmp;
    tmp = V[2][1]; V[2][1] = V[2][0]; V[2][0] = tmp;
  }

#ifdef DEBUG
  printf("D' = \n");
  lin_printmat((double *)D, 1, 3);
  printf("V' = \n");
  lin_printmat((double *)V, 3, 3);
#endif

  *dx1 = V[0][0]; *dy1 = V[1][0]; *dz1 = V[2][0];
  *dx2 = V[0][1]; *dy2 = V[1][1]; *dz2 = V[2][1];
  *dx3 = V[0][2]; *dy3 = V[1][2]; *dz3 = V[2][2];

  return(1);
} // ldirectionsprincipales3d
