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
  recalage rigide :
  - de contours simples (courbes fermées)
  - d'images numériques

  Michel Couprie - juillet 2002 - avril 2005 - novembre 2008 
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <mclin.h>
#include <mcgeo.h>
#include <mcpowell.h>
#include <lrecalagerigide.h>

#define VERBOSE
const int32_t MAXITER = 50;

/* ==================================== */
static double distcarre(double x1, double y1, double x2, double y2)
/* ==================================== */
{
  return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

/* ==================================== */
double lrecalagerigide_Delta(double *X, int32_t n, double *Y, int32_t m)
/* ==================================== */
/*! \fn double lrecalagerigide_Delta(double *X, int32_t n, double *Y, int32_t m)
    \param X (entrée) : matrice n * 2 contenant le premier ensemble de points
    \param n (entrée) : nombre de points dans X
    \param Y (entrée) : matrice m * 2 contenant le second ensemble de points
    \param m (entrée) : nombre de points dans X
    \return distance de X à Y
    \brief calcule une "distance" de X à Y, définie comme la somme des carrés des distances
       des points de X à l'ensemble Y.
*/
{
  double tmin, t, x1, x2, delta = 0.0;
  int32_t i, j;
  for (i = 0; i < n; i++)
  {
    x1 = X[i*2 + 0];
    x2 = X[i*2 + 1];
    tmin = distcarre(x1, x2, Y[0], Y[1]);
    for (j = 1; j < m; j++)
    {
      t = distcarre(x1, x2, Y[j*2 + 0], Y[j*2 + 1]);
      if (t < tmin) tmin = t;
    } // for (j = 1; j < m; j++)
    delta += tmin;
  } // for (i = 0; i < n; i++)
  return delta;
} // lrecalagerigide_Delta()

/* ==================================== */
void lrecalagerigide_Tgamma(double *X, int32_t n, double *Gamma, double *R)
/* ==================================== */
/*! \fn double lrecalagerigide_Tgamma(double *X, int32_t n, double *Y, int32_t m)
    \param X (entrée) : matrice n * 2 contenant un ensemble de points
    \param n (entrée) : nombre de points dans X
    \param Gamma (entrée) : paramètres d'une transformation affine linéaire
    \param R (sortie) : résultat
    \brief applique la transformation affine linéaire définie par Gamma aux points de X
    \warning R doit etre initialise aux memes dimensions que X (n,2)
*/
{
  int32_t i;
  double Rot[2][2];
  
  // rotation
  Rot[0][0] = cos(Gamma[2]);  
  Rot[0][1] = -sin(Gamma[2]);  
  Rot[1][0] = sin(Gamma[2]);  
  Rot[1][1] = cos(Gamma[2]);  
  lin_mult((double *)X, (double *)Rot, (double *)R, n, 2, 2);
  // homotheties
  for (i = 0; i < n; i++)
  {
    R[i*2 + 0] = R[i*2 + 0] * Gamma[0]; // hx
    R[i*2 + 1] = R[i*2 + 1] * Gamma[1]; // hy
  } // for (i = 0; i < n; i++)
  // translations
  for (i = 0; i < n; i++)
  {
    R[i*2 + 0] = R[i*2 + 0] + Gamma[3]; // tx
    R[i*2 + 1] = R[i*2 + 1] + Gamma[4]; // ty
  } // for (i = 0; i < n; i++)
} // lrecalagerigide_Tgamma() 

/* ==================================== */
double lrecalagerigide_F(double *G, ensemble *ens)
/* ==================================== */
{
  lrecalagerigide_Tgamma(ens->X, ens->n, G, ens->Tmp);
  return lrecalagerigide_Delta(ens->Tmp, ens->n, ens->Y, ens->m);  
} // lrecalagerigide_F()

/* ==================================== */
double *lrecalagerigide_lrecalagerigide2d(double *X, int32_t n, double *Y, int32_t m)
/* ==================================== */
/*! \fn double *lrecalagerigide_lrecalagerigide2d(double *X, int32_t n, double *Y, int32_t m)
    \param X (entrée/sortie) : matrice n * 2 contenant le premier ensemble de points
    \param n (entrée) : nombre de points dans X
    \param Y (entrée) : matrice m * 2 contenant le second ensemble de points
    \param m (entrée) : nombre de points dans Y
    \return vecteur des paramètres de la déformation recalant X à Y  
    \brief identifie les paramètres de la déformation affine linéaire recalant au mieux X à Y et
       applique cette déformation à X.
       Cette déformation est la composée (dans cet ordre) d'une rotation d'angle theta ;
       d'une  homothétie dans les directions x,y de facteurs (hx, hy)
       et d'une translation de vecteur (tx, ty).
       Le résultat de l'application de cette déformation à X est stockée dans X en sortie.
*/
{
  double *Gamma, fmin;
  double G[5] = {1.0, 1.0, 0.0, 0.0, 0.0}; // hx, hy, theta, tx, ty
  int32_t i;
  double BX1, BX2, BY1, BY2; // barycentres
  ensemble ens;
  const double PRECISION = 1e-6;

  // identifie les barycentres et normalise les coordonnées
  BX1 = BX2 = BY1 = BY2 = 0.0;
  for (i = 0; i < n; i++) { BX1 += X[2*i]; BX2 += X[2*i + 1]; }
  for (i = 0; i < m; i++) { BY1 += Y[2*i]; BY2 += Y[2*i + 1]; }
  BX1 /= n; BX2 /= n; BY1 /= m; BY2 /= m;

  for (i = 0; i < n; i++) { X[2*i] -= BX1; X[2*i + 1] -= BX2; }
  for (i = 0; i < n; i++) { Y[2*i] -= BY1; Y[2*i + 1] -= BY2; }

  ens.X = X; ens.n = n; ens.Y = Y; ens.m = m; 
  ens.Tmp = (double *)calloc(1, 2 * mcmax(n,m) * sizeof(double));

  powell(lrecalagerigide_F, &ens, G, 5, PRECISION, 0.1, MAXITER, &fmin);

  G[3] += BY1; G[4] += BY2;
  lrecalagerigide_Tgamma(X, n, G, ens.Tmp);
  G[3] -= BX1; G[4] -= BX2;

  for (i = 0; i < n+n; i++) X[i] = ens.Tmp[i]; 

  Gamma = (double *)calloc(1, 5 * sizeof(double));
  memcpy(Gamma, G, 5 * sizeof(double));
  free(ens.Tmp);
  return Gamma;
} // lrecalagerigide_lrecalagerigide2d()

/* ==================================== */
static double distcarre3d(double x1, double y1, double z1, double x2, double y2, double z2)
/* ==================================== */
{
  return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1);
}

/* ==================================== */
double lrecalagerigide_Delta3d(double *X, int32_t n, double *Y, int32_t m)
/* ==================================== */
/*! \fn double lrecalagerigide_Delta3d(double *X, int32_t n, double *Y, int32_t m)
    \param X (entrée) : matrice n * 3 contenant le premier ensemble de points
    \param n (entrée) : nombre de points dans X
    \param Y (entrée) : matrice m * 3 contenant le second ensemble de points
    \param m (entrée) : nombre de points dans X
    \return distance de X à Y
    \brief calcule une "distance" de X à Y, définie comme la somme des carrés des distances
       des points de X à l'ensemble Y.
*/
{
  double tmin, t, x1, x2, x3, delta = 0.0;
  int32_t i, j;
  for (i = 0; i < n; i++)
  {
    x1 = X[i*3 + 0];
    x2 = X[i*3 + 1];
    x3 = X[i*3 + 2];
    tmin = distcarre3d(x1, x2, x3, Y[0], Y[1], Y[2]);
    for (j = 1; j < m; j++)
    {
      t = distcarre3d(x1, x2, x3, Y[j*3 + 0], Y[j*3 + 1], Y[j*3 + 2]);
      if (t < tmin) tmin = t;
    } // for (j = 1; j < m; j++)
    delta += tmin;
  } // for (i = 0; i < n; i++)
  return delta;
} // lrecalagerigide_Delta3d()

/* ==================================== */
void lrecalagerigide_Tgamma3d(double *X, int32_t n, double *Gamma, double *R)
/* ==================================== */
/*! \fn double lrecalagerigide_Tgamma3d(double *X, int32_t n, double *Y, int32_t m)
    \param X (entrée) : matrice n * 2 contenant un ensemble de points
    \param n (entrée) : nombre de points dans X
    \param Gamma (entrée) : paramètres d'une transformation affine linéaire
    \param R (sortie) : résultat
    \brief applique la transformation affine linéaire définie par Gamma aux points de X
    \warning R doit etre initialise aux memes dimensions que X (n,2)
*/
{
  int32_t i;
  double Rot1[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
  double Rot2[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
  double Rot[3][3];

  // rotation autour de l'axe z
  Rot1[0][0] = cos(Gamma[3]);  
  Rot1[0][1] = -sin(Gamma[3]);  
  Rot1[1][0] = sin(Gamma[3]);  
  Rot1[1][1] = cos(Gamma[3]);  
  Rot1[2][2] = 1.0;  
  // rotation autour de l'axe y
  Rot2[0][0] = cos(Gamma[4]);  
  Rot2[0][2] = -sin(Gamma[4]);  
  Rot2[2][0] = sin(Gamma[4]);  
  Rot2[2][2] = cos(Gamma[4]);  
  Rot2[1][1] = 1.0;  
  lin_mult((double *)Rot1, (double *)Rot2, (double *)Rot, 3, 3, 3);
  lin_mult((double *)X, (double *)Rot, (double *)R, n, 3, 3);

  // homotheties
  for (i = 0; i < n; i++)
  {
    R[i*3 + 0] = R[i*3 + 0] * Gamma[0]; // hx
    R[i*3 + 1] = R[i*3 + 1] * Gamma[1]; // hy
    R[i*3 + 2] = R[i*3 + 2] * Gamma[2]; // hz
  } // for (i = 0; i < n; i++)
  // translations
  for (i = 0; i < n; i++)
  {
    R[i*3 + 0] = R[i*3 + 0] + Gamma[5]; // tx
    R[i*3 + 1] = R[i*3 + 1] + Gamma[6]; // ty
    R[i*3 + 2] = R[i*3 + 2] + Gamma[7]; // tz
  } // for (i = 0; i < n; i++)
} // lrecalagerigide_Tgamma3d() 

/* ==================================== */
double lrecalagerigide_F3d(double *G, ensemble *ens)
/* ==================================== */
{
  lrecalagerigide_Tgamma3d(ens->X, ens->n, G, ens->Tmp);
  return lrecalagerigide_Delta3d(ens->Tmp, ens->n, ens->Y, ens->m);  
} // lrecalagerigide_F3d()

/* ==================================== */
double *lrecalagerigide3d(double *X, int32_t n, double *Y, int32_t m)
/* ==================================== */
/*! \fn double *lrecalagerigide3d(double *X, int32_t n, double *Y, int32_t m)
    \param X (entrée/sortie) : matrice n * 3 contenant le premier ensemble de points
    \param n (entrée) : nombre de points dans X
    \param Y (entrée) : matrice m * 3 contenant le second ensemble de points
    \param m (entrée) : nombre de points dans Y
    \return vecteur des paramètres de la déformation recalant X à Y  
    \brief identifie les paramètres de la déformation affine linéaire recalant au mieux X à Y et
       applique cette déformation à X.
       Cette déformation est la composée (dans cet ordre) d'une rotation d'angle theta ;
       d'une homothétie dans les directions x,y,z de facteurs (hx, hy, hz) ; 
       et d'une translation de vecteur (tx, ty, tz).
       Le résultat de l'application de cette déformation à X est stockée dans X en sortie.
*/
{
  double *Gamma, fmin;
  double G[8] = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // hx, hy, hz, theta, phi, tx, ty, tz
  int32_t i;
  double BX1, BX2, BX3, BY1, BY2, BY3; // barycentres
  ensemble ens;
  const double PRECISION = 1e-6;

  // identifie les barycentres et normalise les coordonnées
  BX1 = BX2 = BX3 = BY1 = BY2 = BY3 = 0.0;
  for (i = 0; i < n; i++) { BX1 += X[3*i]; BX2 += X[3*i + 1]; BX3 += X[3*i + 2]; }
  for (i = 0; i < m; i++) { BY1 += Y[3*i]; BY2 += Y[3*i + 1]; BY3 += Y[3*i + 2]; }
  BX1 /= n; BX2 /= n; BX3 /= n; BY1 /= m; BY2 /= m; BY3 /= m;

  for (i = 0; i < n; i++) { X[3*i] -= BX1; X[3*i + 1] -= BX2; X[3*i + 2] -= BX3; }
  for (i = 0; i < m; i++) { Y[3*i] -= BY1; Y[3*i + 1] -= BY2; Y[3*i + 2] -= BY3; }

  ens.X = X; ens.n = n; ens.Y = Y; ens.m = m;
  ens.Tmp = (double *)calloc(1, 3 * mcmax(n,m) * sizeof(double));
  

  powell(lrecalagerigide_F3d, &ens, G, 8, PRECISION, 0.1, MAXITER, &fmin);

  G[5] += BY1; G[6] += BY2; G[7] += BY3;
  lrecalagerigide_Tgamma3d(X, n, G, ens.Tmp);
  G[5] -= BX1; G[6] -= BX2; G[7] -= BX3;

  for (i = 0; i < 3*n; i++) X[i] = ens.Tmp[i]; 

  Gamma = (double *)calloc(1, 8 * sizeof(double));
  memcpy(Gamma, G, 8 * sizeof(double));
  free(ens.Tmp);
  return Gamma;
} // lrecalagerigide3d()

/* ============================================================= */
/* ============================================================= */
/* recalage global (affine) pour deux images numériques */
/* ============================================================= */
/* ============================================================= */

static double SEUIL2;

/* ==================================== */
double lrecalagerigide_F_num(double *G, struct xvimage * image1, struct xvimage * image2)
/* ==================================== */
// fonction de l'évaluation d'erreur pour 2 images numériques
//
// G contient les paramètres d'une transformation affine :
// G[0] = hx
// G[1] = hy (homothetie)
// G[2] = theta (rotation)
// G[3] = tx
// G[4] = ty (translation)
//
// dans image2 on a les coordonnées (xmin,ymin,xmax,ymax) d'un rectangle 
// dont seuls les points seront utilisés.
//
// calcule la somme des carrés des différences entre G(image1) et image2
// WARNING : les images doivent être de même taille - pas de vérification
// WARNING : utilise (par effet de bord) la variable globale SEUIL2 (à changer)
{
  int32_t i, x, y, Xm, Ym, XM, YM;
  int32_t rs = rowsize(image1);
  int32_t cs = colsize(image1);
  int32_t N = rs * cs;
  uint8_t *I1 = UCHARDATA(image1);
  uint8_t *I2 = UCHARDATA(image2);
  double RH[2][2];
  double X, Y, tXm, tXM, t;
  double ErrorQuad = 0.0;
  
#define SANS_ZOOM
#ifdef SANS_ZOOM
  // rotation 
  RH[0][0] = cos(G[2]);  
  RH[0][1] = -sin(G[2]);  
  RH[1][0] = sin(G[2]);  
  RH[1][1] = cos(G[2]);  
#else
  // rotation et homothetie
  RH[0][0] = cos(G[2]) * G[0];  
  RH[0][1] = -sin(G[2]) * G[0];  
  RH[1][0] = sin(G[2]) * G[1];  
  RH[1][1] = cos(G[2]) * G[1];  
#endif

  for (i = 0; i < N; i++)
//#define ESSAI
#ifdef ESSAI
      if (I1[i] >= SEUIL2)
#endif
  {
    x = i % rs;
    y = i / rs;
    X = RH[0][0] * x + RH[0][1] * y + G[3];
    Y = RH[1][0] * x + RH[1][1] * y + G[4];
    // encadrement de la cible
    Xm = (int32_t)floor(X);
    XM = Xm + 1;
    Ym = (int32_t)floor(Y);
    YM = Ym + 1;
    // cible dans le rectangle ?
    if ((Xm >= image2->xmin) && (Ym >= image2->ymin) && (Xm <= image2->xmax) && (Ym <= image2->ymax) &&
	(XM >= image2->xmin) && (YM >= image2->ymin) && (XM <= image2->xmax) && (YM <= image2->ymax))
    {
      // calcule valeur interpolée dans I2
      tXm = I2[Ym*rs + Xm] * (XM-X) + I2[Ym*rs + XM] * (X-Xm);
      tXM = I2[YM*rs + Xm] * (XM-X) + I2[YM*rs + XM] * (X-Xm);
      t = tXm * (YM - Y) + tXM * (Y - Ym); 
      // calcule l'erreur quadratique et accumule
      t = t - I1[y*rs + x];
      t = t * t;
#ifndef ESSAI
      if (t <= SEUIL2)
#endif
	ErrorQuad = ErrorQuad + t;
    }
  } // for (i = 0; i < N; i++)

  return ErrorQuad;
} // lrecalagerigide_F_num()

/* ==================================== */
int32_t lrecalagerigide_lrecalagerigide2d_num(struct xvimage * image1, struct xvimage * image2, double *G, double seuil, double precision)
/* ==================================== */
/*! \fn double *lrecalagerigide_lrecalagerigide2d_num(struct xvimage * image1, struct xvimage * image2)
    \param image1 (entrée/sortie) : première image
    \param image2 (entrée) : seconde image
    \param G (entrée/sortie) : vecteur des paramètres de la déformation recalant image1 à image2 - en entrée : une déformation initiale
    \param seuil (en entrée) : l'erreur ponctuelle quadratique est comparée à seuil^2, au delà elle n'est pas prise en compte (outlier)
    \param précision (en entrée) : arrêt de l'algo quand la différence entre les coûts globaux P et C aux étapes précedente et courante est inférieure à (P+C)*precision/2
    \return entier 1 si ok, 0 sinon
    \brief identifie les paramètres de la déformation affine linéaire recalant au mieux 
       image1 à image2.
       Cette déformation est la composée (dans cet ordre) d'une rotation 
       d'angle theta ; d'une homothétie dans les directions x,y de facteurs (hx, hy), 
       et d'une translation de vecteur (tx, ty).
*/
#undef F_NAME
#define F_NAME "lrecalagerigide_lrecalagerigide2d_num"
{
  double fmin;

#ifdef ESSAI
  SEUIL2 = seuil; // globale utilisée par lrecalagerigide_F_num 
#else
  SEUIL2 = seuil * seuil; // globale utilisée par lrecalagerigide_F_num - pour éliminer les outliers
#endif

  if (powell_num(lrecalagerigide_F_num, image1, image2, G, 5, precision, 0.1, MAXITER, &fmin) == M_NOT_FOUND)
  {
    fprintf(stderr, "%s() : powell_num failed\n", F_NAME);
    return(0);    
  }

  return 1;
} // lrecalagerigide_lrecalagerigide2d_num()
