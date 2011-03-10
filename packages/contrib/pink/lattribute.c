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
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <mccodimage.h>
#include <mclifo.h>
#include <mctopo.h>
#include <mcutil.h>
#include <mclin.h>
#include <lattribute.h>

/****************************************************************
*
* Routine Name: lattribute - library call for attribute
*
* Purpose: etiquetage des extrema d'une image 2d (en 4 ou 8 connexite) 
*          par un de ses attributs (surface, perimetre, circularite)
*
*          la circularite est definie dans le continu par le rapport :
*                                        2
*                   4 . PI . Surf / Perim        et vaut 1 pour un disque.
*
* Input:
* Output:
* Written By: Michel Couprie - mars 1998
*
* Remarques:
*   l'efficacite peut etre notablement amelioree en sortant les tests (switch, etc)
*   des boucles - a faire
*
* Modifications:
*
* 28/3/98 : ajout de l'attribut TROUS (nb de trous)
* 07/4/98 : ajout de l'attribut EXCEN (excentricite)
* 01/5/98 : ajout de l'attribut ORIEN (orientation)
*
*   les attributs EXCEN, ORIEN sont bases sur le calcul des moments et de la     
*   "best fit ellipse" : 
*     EXCEN est le rapport de la longueur du petit axe par la longueur du grand axe,
*     normalise entre 0 et 255,
*     ORIEN est l'angle que fait le grand axe avec la direction horizontale, en 
*     degres entre 0 et 179.
*
* 03/01/00 : redefinition de l'attribut PERIM:
*            l'ancienne version comptait les pixels de bord (en comptant plusieurs fois les
*            points de croisement) 
*            on calcule desormais la longueur effective du bord "1D" du complexe cellulaire
*            associe a l'objet 
*
* 17/05/08 : ajout des attributs VDIAM et HDIAM (vertical and horizontal diameter)
*
* 30/11/09 : lattribute3d
****************************************************************/

#define NONMARQUE -1
#define MARQUE    -2
#define NONEXTREM  0

/*
#define VERBOSE
#define DEBUGTROU
#define DEBUGEXCEN
#define DEBUGORIEN
*/

#define K1 (256.0/21.0)
#define K2 (13924.0/757.0)
#define EPSILON 1E-6

static int32_t excentricity(double mx1, double my1, double mx2, double my2, double mxy2, int32_t n)
{
  double Mx2, My2, Mxy2, delta;
  double lambda1, lambda2;

  /* moments centres d'ordre 2 (variances et covariances) */
  Mx2 = mx2 - mx1 * mx1 / n;  
  My2 = my2 - my1 * my1 / n;  
  Mxy2 = mxy2 - mx1 * my1 / n;

  /* calcul des valeurs propres de la matrice des covariances */
  delta = (Mx2 - My2) * (Mx2 - My2) + 4 * Mxy2 * Mxy2;
  lambda1 = (Mx2 + My2 + sqrt(delta)) / 2.0;
  lambda2 = (Mx2 + My2 - sqrt(delta)) / 2.0;
#ifdef DEBUGEXCEN
  printf("Mx2 = %g ; My2 = %g ; Mxy2 = %g ; delta = %g\n", Mx2, My2, Mxy2, delta);
  printf("lambda1 = %g ; lambda2 = %g\n", lambda1, lambda2);
#endif
  if (lambda2 < 0.0)
  {
    fprintf(stderr, "excentricity : valeur propre negative : %g\n", lambda2);
    return 0;
  }
  return 255 - (int32_t)(lambda2 * 255 / lambda1);
} /* excentricity() */

static int32_t orientation(double mx1, double my1, double mx2, double my2, double mxy2, int32_t n)
{
  double Mx2, My2, Mxy2, delta;
  double lambda1;
  double x, y, a;
  int32_t sign, theta;

  /* moments centres d'ordre 2 (variances et covariances) */
  Mx2 = mx2 - mx1 * mx1 / n;  
  My2 = my2 - my1 * my1 / n;  
  Mxy2 = mxy2 - mx1 * my1 / n;

  /* calcul des valeurs propres de la matrice des covariances */
  delta = (Mx2 - My2) * (Mx2 - My2) + 4 * Mxy2 * Mxy2;
  lambda1 = (Mx2 + My2 + sqrt(delta)) / 2.0;
#ifdef DEBUGORIEN
  printf("Mx2 = %g ; My2 = %g ; Mxy2 = %g ; lambda1 = %g\n", Mx2, My2, Mxy2, lambda1);
#endif
  if (mcabs(lambda1 - Mx2) < EPSILON) return 0; 
  a = Mxy2 / (lambda1 - Mx2);
  if (a < 0) sign = -1; else sign = 1; 
  a = a * a;
  x = sqrt(a / (1 + a));
  y = sign * sqrt(1 / (1 + a));
#ifdef DEBUGORIEN
  printf("x = %g ; y = %g ; atan2(y,x) = %g\n", x, y, atan2(y,x));
#endif
  theta = arrondi( (atan2(y,x) / M_PI) * 180.0 );
  if (theta < 0) return theta + 180; else return theta;
} /* orientation() */

/* ==================================== */
int32_t lattribute(
        struct xvimage *img, /* image de depart */
        int32_t connex,          /* 4, 8  */
        int32_t typregion,       /* = <LABMIN | LABMAX | LABPLATEAU> */
        int32_t attrib,          /* 0: surface, 1: perimetre, 2: circularite, 3: nb. trous, 
                                4: excentricite, 5: orientation, 6: diamètre vertical, 7: diamètre horizontal */
        int32_t seuil,           /* en dessous (<=) de seuil, l'attribut est mis a 0 */
        struct xvimage *lab, /* resultat: image d'attributs */
        int32_t *nlabels)        /* resultat: nombre de regions traitees */
/* ==================================== */
#undef F_NAME
#define F_NAME "lattribute"
{
  int32_t k, l;
  index_t w, x, y, z;
  uint8_t *SOURCE = UCHARDATA(img);
  int32_t *LABEL = SLONGDATA(lab);
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t d = depth(img);
  index_t N = rs * cs;          /* taille image */
  Lifo * LIFO;
  int32_t label;
  int32_t area;
  int32_t perim;
  int32_t min, max;
  int32_t val_attrib;
  double mx1, my1; // cumuls des variables x et y
  double mx2, my2, mxy2; // cumuls des x^2, y^2 et xy
  int32_t incr_vois;

  if (datatype(lab) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(lab) != rs) || (colsize(lab) != cs) || (depth(lab) != d))
  {
    fprintf(stderr, "%s: tailles images incompatibles\n", F_NAME);
    return 0;
  }

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

  /* le LABEL initialement est mis a NONMARQUE */
  for (x = 0; x < N; x++) LABEL[x] = NONMARQUE;

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
    {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  *nlabels = 0;

if ((typregion == LABMIN) || (typregion == LABMAX))
{
  for (x = 0; x < N; x++)
  {
    if (LABEL[x] == NONMARQUE)   /* on trouve un point x non etiquete */
    {
      *nlabels += 1;
      LABEL[x] = MARQUE;
#ifdef DEBUGTROU
printf("AMORCE p=%d,%d h=%d set LABEL = %d\n", x%rs, x/rs, SOURCE[x], LABEL[x]);
#endif
      switch (attrib)            /* on initialise les attributs de cette composante */
      {
	case AREA: val_attrib = 0; break;
	case PERIM: val_attrib = 0; break;
	case TROUS: val_attrib = 0; break;
	case CIRC: area = perim = 0; break;
	case EXCEN: 
	case ORIEN: area = 0; mx1 = my1 = mx2 = my2 = mxy2 = 0.0; break;
        case VDIAM: min = cs-1; max = 0; break;
        case HDIAM: min = rs-1; max = 0; break;
        default: 
          fprintf(stderr, "%s: mauvais attribut: %d\n", F_NAME, attrib);
          return 0;
      } /* switch (attrib) */
      LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        label = LABEL[w];
        if (label == MARQUE)     /* c'est une propagation de "marquage" : pour l'instant, */
	{                        /* on croit qu'on est dans un extremum */         
          switch (attrib)
          {
  	    case AREA: val_attrib++; break;
  	    case VDIAM: if (w/rs < min) min = w/rs; else if (w/rs > max) max = w/rs; break;
  	    case HDIAM: if (w%rs < min) min = w%rs; else if (w%rs > max) max = w%rs; break;
  	    case EXCEN:
  	    case ORIEN: area++; mx1 += w%rs; my1 += w/rs; mxy2 += (w%rs) * (w/rs);
                        mx2 += (w%rs) * (w%rs); my2 += (w/rs) * (w/rs); break;
  	    case PERIM: 
              if (w%rs==rs-1) val_attrib++; /* point de bord */
              if (w<rs)       val_attrib++; /* point de bord */
              if (w%rs==0)    val_attrib++; /* point de bord */
              if (w>=N-rs)    val_attrib++; /* point de bord */
              for (k = 0; k < 8; k += 2) /* 4-connexite obligatoire */
              {
                y = voisin(w, k, rs, N);
                if ((y != -1) && (SOURCE[y] != SOURCE[w])) val_attrib++;
              } /* for k */
            break;
            case CIRC:
              area++;
              if (w%rs==rs-1) perim++; /* point de bord */
              if (w<rs)       perim++; /* point de bord */
              if (w%rs==0)    perim++; /* point de bord */
              if (w>=N-rs)    perim++; /* point de bord */
              for (k = 0; k < 8; k += 2) /* 4-connexite obligatoire */
              {
                y = voisin(w, k, rs, N);
                if ((y != -1) && (SOURCE[y] != SOURCE[w])) perim++;
              } /* for k */
              break;
	  } /* switch (attrib) */
          for (k = 0; k < 8; k += incr_vois)
          {
            y = voisin(w, k, rs, N);
            if (y != -1)
            {
              if (((typregion == LABMIN) && (SOURCE[y] < SOURCE[w])) || 
                  ((typregion == LABMAX) && (SOURCE[y] > SOURCE[w])))
              {   /* w non dans un minimum (resp. maximum) */
                if (label == MARQUE)
		{
                  label = NONEXTREM;
                  *nlabels -= 1;
                  LABEL[w] = label;
                  LifoPush(LIFO, w);
		}
              } 
              else
              if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE))
              {
                LABEL[y] = label;
#ifdef DEBUGTROU
printf(" p=%d,%d h=%d set LABEL = %d\n", y%rs, y/rs, SOURCE[y], LABEL[y]);
#endif
                LifoPush(LIFO, y);
                if (attrib == TROUS)
	        {
                  int32_t masque = 0, imasque = 1;
                  /* fabrique le masque des voisins de y MARQUES */
                  for (l = 0; l < 8; l += 1)
                  {
                    z = voisin(y, l, rs, N);
                    if ((z != -1) && (LABEL[z] == MARQUE))
                      masque |= imasque; 
                    imasque = imasque << 1;
                  } /* for k ... */
#ifdef DEBUGTROU
printf(" p=%d,%d h=%d masque=%x t4m=%d t8b=%d\n", y%rs, y/rs, SOURCE[y], masque, t4(masque), t8b(masque));
#endif
                  if (connex == 4) 
                    { val_attrib += (t4(masque) - 1); if (t8b(masque) == 0) val_attrib--; }
                  else
                    { val_attrib += (t8(masque) - 1); if (t4b(masque) == 0) val_attrib--; }
	        } /* if (attrib == TROUS) */
              } /* if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE)) */
            } /* if (y != -1) */
          } /* for k ... */
	} /* if (label == MARQUE) */
        else                           /* propagation de "demarquage" */
	{
          for (k = 0; k < 8; k += incr_vois)
          {
            y = voisin(w, k, rs, N);
            if (y != -1)
            {
              if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] != NONEXTREM))
              {
                LABEL[y] = NONEXTREM;
                LifoPush(LIFO, y);
              } /* if .. */
            } /* if (y != -1) */
          } /* for k ... */
	} /* else if (label == MARQUE) */
      } /* while (! LifoVide(LIFO)) */

      if (label == MARQUE)
      {
        if (attrib == CIRC) 
	{
          val_attrib = (int32_t)(256 * 4 * M_PI * (double)area / (double)(perim * perim));
          if (val_attrib > 255)
	  {
            fprintf(stderr, "WARNING: indice de circularite > 255 : %d, a=%d, p=%d\n", 
                            val_attrib, area, perim);
            val_attrib = 255;
	  }
	}
        if (attrib == EXCEN) val_attrib = excentricity(mx1, my1, mx2, my2, mxy2, area);
        if (attrib == ORIEN) val_attrib = orientation(mx1, my1, mx2, my2, mxy2, area);
        if (attrib == VDIAM) val_attrib = max - min + 1;
        if (attrib == HDIAM) val_attrib = max - min + 1;
        if (val_attrib <= seuil) val_attrib = 0;
#ifdef VERBOSE
printf("valeur attribut = %d\n", val_attrib);
#endif
        LifoPush(LIFO, x);         /* on re-parcourt le plateau pour propager l'attribut */
        LABEL[x] = val_attrib;
        while (! LifoVide(LIFO))
        {
          w = LifoPop(LIFO);
          for (k = 0; k < 8; k += incr_vois)
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (LABEL[y] == MARQUE)) 
            {
              LABEL[y] = val_attrib;
              LifoPush(LIFO, y);
	    }
          } /* for k ... */
	} /* while (! LifoVide(LIFO)) */
      } /* if (label == MARQUE) */

    } /* if (LABEL[x] != -1) */
  } /* for (x = 0; x < N; x++) */
} /* if ((typregion == LABMIN) || (typregion == LABMAX)) */
else
{
  if (attrib == TROUS)
  {
    fprintf(stderr, "%s: attribut TROUS non compatible avec typreg = PLA\n", F_NAME);
    return 0;
  }
  for (x = 0; x < N; x++)
  {
    if (LABEL[x] == NONMARQUE)
    {
      *nlabels += 1;
      LABEL[x] = *nlabels;
      switch (attrib)            /* on initialise les attributs de cette composante */
      {
	case AREA: val_attrib = 0; break;
        case VDIAM: min = cs-1; max = 0; break;
        case HDIAM: min = rs-1; max = 0; break;
	case PERIM: val_attrib = 0; break;
	case CIRC: area = perim = 0; break;
	case EXCEN: 
	case ORIEN: area = 0; mx1 = my1 = mx2 = my2 = mxy2 = 0.0; break;
        default: 
          fprintf(stderr, "%s: mauvais attribut: %d\n", F_NAME, attrib);
          return 0;
      } /* switch (attrib) */
      LifoPush(LIFO, x);
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        switch (attrib)
        {
	  case AREA: val_attrib++; break;
  	  case VDIAM: if (w/rs < min) min = w/rs; else if (w/rs > max) max = w/rs; break;
  	  case HDIAM: if (w%rs < min) min = w%rs; else if (w%rs > max) max = w%rs; break;
	  case EXCEN:
	  case ORIEN: area++; mx1 += w%rs; my1 += w/rs; mxy2 += (w%rs) * (w/rs);
                      mx2 += (w%rs) * (w%rs); my2 += (w/rs) * (w/rs); break;
  	  case PERIM: 
            if (w%rs==rs-1) val_attrib++; /* point de bord */
            if (w<rs)       val_attrib++; /* point de bord */
            if (w%rs==0)    val_attrib++; /* point de bord */
            if (w>=N-rs)    val_attrib++; /* point de bord */
            for (k = 0; k < 8; k += 2) /* 4-connexite obligatoire */
            {
              y = voisin(w, k, rs, N);
              if ((y != -1) && (SOURCE[y] != SOURCE[w])) val_attrib++;
            } /* for k */
          break;
          case CIRC:
            area++;
            if (w%rs==rs-1) perim++; /* point de bord */
            if (w<rs)       perim++; /* point de bord */
            if (w%rs==0)    perim++; /* point de bord */
            if (w>=N-rs)    perim++; /* point de bord */
            for (k = 0; k < 8; k += 2) /* 4-connexite obligatoire */
            {
              y = voisin(w, k, rs, N);
              if ((y != -1) && (SOURCE[y] != SOURCE[w])) perim++;
            } /* for k */
            break;
	} /* switch (attrib) */

        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (LABEL[y] == NONMARQUE) && (SOURCE[y] == SOURCE[w]))
          {
            LABEL[y] = MARQUE;
            LifoPush(LIFO, y);
          } /* if y ... */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */

      if (attrib == CIRC) 
      {
        val_attrib = (int32_t)(256 * 4 * M_PI * (double)area / (double)(perim * perim));
        if (val_attrib > 255)
	{
          fprintf(stderr, "WARNING: indice de circularite > 255 : %d, a=%d, p=%d\n", 
                          val_attrib, area, perim);
          val_attrib = 255;
	}
      }
      if (attrib == EXCEN) val_attrib = excentricity(mx1, my1, mx2, my2, mxy2, area);
      if (attrib == ORIEN) val_attrib = orientation(mx1, my1, mx2, my2, mxy2, area);
      if (attrib == VDIAM) val_attrib = max - min + 1;
      if (attrib == HDIAM) val_attrib = max - min + 1;
      if (val_attrib <= seuil) val_attrib = 0;

      LifoPush(LIFO, x);         /* on re-parcourt le plateau pour propager l'attribut */
      LABEL[x] = val_attrib;
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (LABEL[y] == MARQUE)) 
          {
            LABEL[y] = val_attrib;
            LifoPush(LIFO, y);
	  }
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
    } /* if (LABEL[x] == NONMARQUE) */
  } /* for (x = 0; x < N; x++) */
} /* else if ((typregion == LABMIN) || (typregion == LABMAX)) */

  LifoTermine(LIFO);

  *nlabels += 1; /* pour le niveau 0 */
  return(1);
} // lattribute()

/* ==================================== */
int32_t lattribute3d(
        struct xvimage *img, /* image de depart */
        int32_t connex,          /* 6, 18, 26  */
        int32_t typregion,       /* = <LABMIN | LABMAX | LABPLATEAU> */
        int32_t attrib,          /* 0: surface */
        int32_t seuil,           /* en dessous (<=) de seuil, l'attribut est mis a 0 */
        struct xvimage *lab,     /* resultat: image d'attributs */
        int32_t *nlabels)        /* resultat: nombre de regions traitees */
/* ==================================== */
#undef F_NAME
#define F_NAME "lattribute3d"
{
  int32_t k;
  index_t w, x, y;
  uint8_t *SOURCE = UCHARDATA(img);
  int32_t *LABEL = SLONGDATA(lab);
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img);
  index_t ps = rs * cs;
  index_t N = ps * ds;
  Lifo * LIFO;
  int32_t label;
  int32_t val_attrib;

  if (datatype(lab) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(lab) != rs) || (colsize(lab) != cs) || (depth(lab) != ds))
  {
    fprintf(stderr, "%s: tailles images incompatibles\n", F_NAME);
    return 0;
  }

  /* le LABEL initialement est mis a NONMARQUE */
  for (x = 0; x < N; x++) LABEL[x] = NONMARQUE;

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
    {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  *nlabels = 0;

if ((typregion == LABMIN) || (typregion == LABMAX))
{
  for (x = 0; x < N; x++)
  {
    if (LABEL[x] == NONMARQUE)   /* on trouve un point x non etiquete */
    {
      *nlabels += 1;
      LABEL[x] = MARQUE;
      switch (attrib)            /* on initialise les attributs de cette composante */
      {
	case AREA: val_attrib = 0; break;
        default: 
          fprintf(stderr, "%s: bad attribute: %d\n", F_NAME, attrib);
          return 0;
      } /* switch (attrib) */
      LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        label = LABEL[w];
        if (label == MARQUE)     /* c'est une propagation de "marquage" : pour l'instant, */
	{                        /* on croit qu'on est dans un extremum */     
          switch (attrib)
          {
  	    case AREA: val_attrib++; break;
	  } /* switch (attrib) */
	  switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
              if (y != -1)
              {
               if (((typregion == LABMIN) && (SOURCE[y] < SOURCE[w])) || 
                   ((typregion == LABMAX) && (SOURCE[y] > SOURCE[w])))
               {   /* w non dans un minimum (resp. maximum) */
                if (label == MARQUE)
		{
                  label = NONEXTREM;
                  *nlabels -= 1;
                  LABEL[w] = label;
                  LifoPush(LIFO, w);
		}
               }   
	       else if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE))
               {
                LABEL[y] = label;
                LifoPush(LIFO, y);
               } /* if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE)) */
	      } /* if (y != -1) */
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
              if (y != -1)
              {
               if (((typregion == LABMIN) && (SOURCE[y] < SOURCE[w])) || 
                   ((typregion == LABMAX) && (SOURCE[y] > SOURCE[w])))
               {   /* w non dans un minimum (resp. maximum) */
                if (label == MARQUE)
		{
                  label = NONEXTREM;
                  *nlabels -= 1;
                  LABEL[w] = label;
                  LifoPush(LIFO, w);
		}
               }   
	       else if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE))
               {
                LABEL[y] = label;
                LifoPush(LIFO, y);
               } /* if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE)) */
	      } /* if (y != -1) */
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
              if (y != -1)
              {
               if (((typregion == LABMIN) && (SOURCE[y] < SOURCE[w])) || 
                   ((typregion == LABMAX) && (SOURCE[y] > SOURCE[w])))
               {   /* w non dans un minimum (resp. maximum) */
                if (label == MARQUE)
		{
                  label = NONEXTREM;
                  *nlabels -= 1;
                  LABEL[w] = label;
                  LifoPush(LIFO, w);
		}
               }   
	       else if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE))
               {
                LABEL[y] = label;
                LifoPush(LIFO, y);
               } /* if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] == NONMARQUE)) */
	      } /* if (y != -1) */
	    } // for k
	    break;
	  } // switch (connex)
	} /* if (label == MARQUE) */
        else                           /* propagation de "demarquage" */
	{
	  switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
              if (y != -1)
              {
                if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] != NONEXTREM))
                {
                  LABEL[y] = NONEXTREM;
                  LifoPush(LIFO, y);
                } /* if .. */
	      } /* if (y != -1) */
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
              if (y != -1)
              {
                if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] != NONEXTREM))
                {
                  LABEL[y] = NONEXTREM;
                  LifoPush(LIFO, y);
                } /* if .. */
	      } /* if (y != -1) */
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
              if (y != -1)
              {
                if ((SOURCE[y] == SOURCE[w]) && (LABEL[y] != NONEXTREM))
                {
                  LABEL[y] = NONEXTREM;
                  LifoPush(LIFO, y);
                } /* if .. */
	      } /* if (y != -1) */
	    } // for k
	    break;
	  } // switch (connex)
	} /* else if (label == MARQUE) */
      } /* while (! LifoVide(LIFO)) */

      if (label == MARQUE)
      {
        if (val_attrib <= seuil) val_attrib = 0;
        LifoPush(LIFO, x);         /* on re-parcourt le plateau pour propager l'attribut */
        LABEL[x] = val_attrib;
        while (! LifoVide(LIFO))
        {
          w = LifoPop(LIFO);

	  switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
              if ((y != -1) && (LABEL[y] == MARQUE)) 
              {
                LABEL[y] = val_attrib;
                LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
              if ((y != -1) && (LABEL[y] == MARQUE)) 
              {
                LABEL[y] = val_attrib;
                LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
              if ((y != -1) && (LABEL[y] == MARQUE)) 
              {
                LABEL[y] = val_attrib;
                LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  } // switch (connex)
	} /* while (! LifoVide(LIFO)) */
      } /* if (label == MARQUE) */

    } /* if (LABEL[x] != -1) */
  } /* for (x = 0; x < N; x++) */
} /* if ((typregion == LABMIN) || (typregion == LABMAX)) */
else
{
  for (x = 0; x < N; x++)
  {
    if (LABEL[x] == NONMARQUE)
    {
      *nlabels += 1;
      LABEL[x] = *nlabels;
      switch (attrib)            /* on initialise les attributs de cette composante */
      {
	case AREA: val_attrib = 0; break;
        default: 
          fprintf(stderr, "%s: bad attribute: %d\n", F_NAME, attrib);
          return 0;
      } /* switch (attrib) */
      LifoPush(LIFO, x);
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        switch (attrib)
        {
	  case AREA: val_attrib++; break;
	} /* switch (attrib) */

	switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
	      if ((y != -1) && (LABEL[y] == NONMARQUE) && (SOURCE[y] == SOURCE[w]))
	      {
		LABEL[y] = MARQUE;
		LifoPush(LIFO, y);
	      } /* if y ... */
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
	      if ((y != -1) && (LABEL[y] == NONMARQUE) && (SOURCE[y] == SOURCE[w]))
	      {
		LABEL[y] = MARQUE;
		LifoPush(LIFO, y);
	      } /* if y ... */
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
	      if ((y != -1) && (LABEL[y] == NONMARQUE) && (SOURCE[y] == SOURCE[w]))
	      {
		LABEL[y] = MARQUE;
		LifoPush(LIFO, y);
	      } /* if y ... */
	    } // for k
	    break;
	  } // switch (connex)
      } /* while (! LifoVide(LIFO)) */
      if (val_attrib <= seuil) val_attrib = 0;
      LifoPush(LIFO, x);         /* on re-parcourt le plateau pour propager l'attribut */
      LABEL[x] = val_attrib;
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);

	switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
	      if ((y != -1) && (LABEL[y] == MARQUE)) 
	      {
		LABEL[y] = val_attrib;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
	      if ((y != -1) && (LABEL[y] == MARQUE)) 
	      {
		LABEL[y] = val_attrib;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
	      if ((y != -1) && (LABEL[y] == MARQUE)) 
	      {
		LABEL[y] = val_attrib;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  } // switch (connex)
      } /* while (! LifoVide(LIFO)) */
    } /* if (LABEL[x] == NONMARQUE) */
  } /* for (x = 0; x < N; x++) */
} /* else if ((typregion == LABMIN) || (typregion == LABMAX)) */

  LifoTermine(LIFO);

  *nlabels += 1; /* pour le niveau 0 */
  return(1);
} // lattribute3d()

#define LP_MARK0 -1
#define LP_MARK1 -2
#define LP_MARK2 -3

/* ==================================== */
int32_t lplanarity(
        struct xvimage *img,     /* image de depart */
        int32_t connex,          /* 6, 18, 26  */
        struct xvimage *res,     /* resultat: image d'attributs */
        int32_t *nlabels)        /* resultat: nombre de regions traitees */
/* ==================================== */
#undef F_NAME
#define F_NAME "lplanarity"
{
  int32_t k;
  index_t w, x, y, areacomp;
  uint8_t *SOURCE = UCHARDATA(img);
  float *RES = FLOATDATA(res);
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img);
  index_t ps = rs * cs;
  index_t N = ps * ds;
  Lifo * LIFO;
  float planar;
  double a, b, c, d, error, *px, *py, *pz;

  if (datatype(res) != VFF_TYP_FLOAT) 
  {
    fprintf(stderr, "%s: result image must be of type VFF_TYP_FLOAT\n", F_NAME);
    return 0;
  }

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  /* le RES initialement est mis a LP_MARK0 */
  for (x = 0; x < N; x++) RES[x] = LP_MARK0;

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
    {   fprintf(stderr, "%s: CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  *nlabels = 0;
  for (x = 0; x < N; x++)
  {
    if (SOURCE[x] && (RES[x] == LP_MARK0))
    {
      *nlabels += 1;
      RES[x] = LP_MARK1;
      areacomp = 0;
      LifoPush(LIFO, x);
      while (! LifoVide(LIFO)) // 1er parcours de la composante - compte les points (areacomp)
      {
        w = LifoPop(LIFO);
	areacomp++;
	switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK0) && SOURCE[y])
	      {
		RES[y] = LP_MARK1;
		LifoPush(LIFO, y);
	      } /* if y ... */
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK0) && SOURCE[y])
	      {
		RES[y] = LP_MARK1;
		LifoPush(LIFO, y);
	      } /* if y ... */
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK0) && SOURCE[y])
	      {
		RES[y] = LP_MARK1;
		LifoPush(LIFO, y);
	      } /* if y ... */
	    } // for k
	    break;
	  } // switch (connex)
      } /* while (! LifoVide(LIFO)) */

      // init 2eme passe
      px = (double *)malloc(areacomp*sizeof(double)); assert(px != NULL);
      py = (double *)malloc(areacomp*sizeof(double)); assert(py != NULL);
      pz = (double *)malloc(areacomp*sizeof(double)); assert(pz != NULL);

      LifoPush(LIFO, x);     /* on parcourt le plateau une 2eme fois pour collecter les points */
      RES[x] = LP_MARK2;
      areacomp = 0;
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
	px[areacomp] = (double)(w % rs);
	py[areacomp] = (double)((w % ps) / rs);
	pz[areacomp] = (double)(w / ps);
	areacomp++;

	switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK1)) 
	      {
		RES[y] = LP_MARK2;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK1)) 
	      {
		RES[y] = LP_MARK2;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK1)) 
	      {
		RES[y] = LP_MARK2;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  } // switch (connex)
      } /* while (! LifoVide(LIFO)) */

      // init 3eme passe - calcul attribut planar
      if (!lidentifyplane(px, py, pz, areacomp, &a, &b, &c, &d, &error))
      {
	fprintf(stderr, "%s: lidentifyplane failed\n", F_NAME);
	return 0;
      }
      free(px); free(py); free(pz);
      planar = (float)(error / areacomp);

      LifoPush(LIFO, x);     /* on parcourt le plateau une 3eme fois pour propager la valeur */
      RES[x] = planar;
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);

	switch (connex)
	  {
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK2)) 
	      {
		RES[y] = planar;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK2)) 
	      {
		RES[y] = planar;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, ps, N);
	      if ((y != -1) && (RES[y] == LP_MARK2)) 
	      {
		RES[y] = planar;
		LifoPush(LIFO, y);
	      }
	    } // for k
	    break;
	  } // switch (connex)
      } /* while (! LifoVide(LIFO)) */

    } /* if (SOURCE[x] && (RES[x] == LP_MARK0)) */
  } /* for (x = 0; x < N; x++) */

  LifoTermine(LIFO);
  return(1);
} // lplanarity()

