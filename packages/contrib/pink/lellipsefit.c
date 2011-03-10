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
/* identification d'ellipse 
   d'après : 
Andrew W. Fitzgibbon, Maurizio Pilu, and Robert B. Fisher
Direct least-squares fitting of ellipses,
IEEE Transactions on Pattern Analysis and Machine Intelligence, 21(5), 476--480, May 1999
*/
/* 
Michel Couprie, mars 2002
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mcutil.h>
#include <mclin.h>
#include <lellipsefit.h>

/*
#define TESTLIN
#define DEBUG
*/
//#define ELLFIT_DEBUG
#define PI_4 0.78539816339744830962
#define EPSILON 1E-30

#define COMPUTE_DIST

/* ==================================================================== */
/* ==================================================================== */
/* identification d'ellipse */
/* ==================================================================== */
/* ==================================================================== */

double elldist(double d[6], double x, double y)
// distance algébrique du point (x,y) à l'ellipse définie par les paramètres d[0..5]
// = A,B,C,D,E,F de l'équation Ax^2 + Bxy + Cy^2 + Dx + Ey + F de l'ellipse.
// la distance est simplement Ax^2 + Bxy + Cy^2 + Dx + Ey + F
{
  return d[0]*x*x + d[1]*x*y + d[2]*y*y + d[3]*x + d[4]*y + d[5];
}

/* ==================================== */
int32_t lellipsefit(double * x, double * y, int32_t n, 
                double *x0, double *y0, double *xa, double *ya, double *xb, double *yb
               )
/* ==================================== */
/*! \fn int32_t lellipsefit(double * x, double * y, int32_t n, double *x0, double *y0, double *xa, double *ya, double *xb, double *yb)
    \param x (entrée) : liste des abcisses des n points
    \param y (entrée) : liste des ordonnées des n points
    \param n (entrée) : nb de points
    \param x0, y0 (sortie) : coordonnées du centre de l'ellipse
    \param xa, ya (sortie) : coordonnées d'une extrémité du grand axe
    \param xb, yb (sortie) : coordonnées d'une extrémité du petit axe
    \return 0 en cas d'erreur, 1 sinon
    \brief détermine l'ellipse la plus proche des points donnés en entrée (cf. Fitzgibbon & al 96)
*/
{
  double S[6][6], L[6][6], C[6][6], InvL[6][6], InvL_t[6][6], temp[6][6], V[6][6], sol[6][6];
  double Const[6][6] = {{0,0,-2,0,0,0}, {0,1,0,0,0,0}, {-2,0,0,0,0,0}, 
			{0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0} };
  double d[6];
  double t;
  int32_t i, j, k, ret;

  if (n < 6) 
  {
    printf("lin_ellipsefit: pas assez de points\n");  
    return 0; 
  }

  // normalisation des coordonnees
  /*  
  xmax = fabs(x[0]); for (k = 1; k < n; k++) if (xmax > fabs(x[k])) xmax = fabs(x[k]);
  ymax = fabs(y[0]); for (k = 1; k < n; k++) if (ymax > fabs(y[k])) ymax = fabs(y[k]);
  for (k = 0; k < n; k++) x[k] /= (xmax/2); 
  for (k = 0; k < n; k++) y[k] /= (ymax/2); 
  */

  // calcul de S
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * x[k] * x[k]; S[0][0] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * x[k] * y[k]; S[0][1] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * y[k] * y[k]; S[0][2] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * x[k]; S[0][3] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * y[k]; S[0][4] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k]; S[0][5] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * y[k] * y[k]; S[1][1] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * y[k] * y[k] * y[k]; S[1][2] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k] * y[k]; S[1][3] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * y[k] * y[k]; S[1][4] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * y[k]; S[1][5] = t;
  t = 0; for (k = 0; k < n; k++) t += y[k] * y[k] * y[k] * y[k]; S[2][2] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * y[k] * y[k]; S[2][3] = t;
  t = 0; for (k = 0; k < n; k++) t += y[k] * y[k] * y[k]; S[2][4] = t;
  t = 0; for (k = 0; k < n; k++) t += y[k] * y[k]; S[2][5] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * x[k]; S[3][3] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k] * y[k]; S[3][4] = t;
  t = 0; for (k = 0; k < n; k++) t += x[k]; S[3][5] = t;
  t = 0; for (k = 0; k < n; k++) t += y[k] * y[k]; S[4][4] = t;
  t = 0; for (k = 0; k < n; k++) t += y[k]; S[4][5] = t;
  S[5][5] = n;
  for (i = 1; i < 6; i++) for (j = 0; j < i; j++) S[i][j] = S[j][i];
#ifdef ELLFIT_DEBUG
  printf("S = \n");
  lin_printmat((double *)S, 6, 6);
#endif

  // inversion de S et multiplication par C
  ret = lin_decomposition_cholesky((double *)S, (double *)L, 6);
  if (ret == 0) 
  {
    printf("lin_ellipsefit: echec decomposition Cholesky\n");  
    return 0;
  }
#ifdef ELLFIT_DEBUG
  printf("L = \n");
  lin_printmat((double *)L, 6, 6);
#endif
  ret = lin_inverse_gauss((double *)L, (double *)InvL, 6);
  if (ret == 0) 
  {
    fprintf(stderr, "lin_ellipsefit: echec inversion Gauss-Jordan\n");  
    return 0;
  }
#ifdef ELLFIT_DEBUG
  printf("InvL = \n");
  lin_printmat((double *)InvL, 6, 6);
#endif
  lin_copy((double *)InvL_t, (double *)InvL, 6, 6);
  lin_transpose((double *)InvL_t, 6);
  lin_mult((double *)Const, (double *)InvL_t, (double *)temp, 6, 6, 6);
  lin_mult((double *)InvL, (double *)temp, (double *)C, 6, 6, 6);
#ifdef ELLFIT_DEBUG
  printf("C = \n");
  lin_printmat((double *)C, 6, 6);
#endif

  // recherche des valeurs et vecteurs propres
  ret = lin_jacobi((double *)C, 6, (double *)d, (double *)V, 0);
  if (ret == 0) 
  {
    printf("lin_ellipsefit: echec Jacobi\n");  
    return 0;
  }
#ifdef ELLFIT_DEBUG
  printf("d = \n");
  lin_printmat((double *)d, 1, 6);
  printf("V = \n");
  lin_printmat((double *)V, 6, 6);
#endif

  // normalisation
  lin_multAtB((double *)InvL, (double *)V, (double *)sol, 6, 6, 6, 6);
  for (j=0; j<6; j++)  /* Scan columns */
  {
    double mod = 0.0;
    for (i=0; i<6; i++)
      mod += sol[i][j] * sol[i][j];
    for (i=0; i<6; i++)
      sol[i][j] /= sqrt(mod); 
  }
#ifdef ELLFIT_DEBUG
  printf("sol (normalisee) = \n");
  lin_printmat((double *)sol, 6, 6);
#endif
  {
    int32_t solind=0;
    for (i=0; i<6; i++)
      if (d[i]<0 && fabs(d[i])>EPSILON)	
	solind = i;
    // Now fetch the right solution (result in d)
    for (j=0; j<6; j++)
      d[j] = sol[j][solind];
  }
  //#ifdef ELLFIT_DEBUG
#ifdef COMPUTE_DIST
  printf("d (sol. finale) = \n");
  lin_printmat((double *)d, 1, 6);
#endif

#ifdef COMPUTE_DIST
  {
    double dist, sumdist = 0;
    for (k = 0; k < n; k++) 
    { 
      dist = elldist(d, x[k], y[k]);
      sumdist += fabs(dist);
    }
    printf("dist. moy à l'ellipse = %g\n", sumdist/n);
  }
#endif

  // simplification de l'equation de l'ellipse
  // d'apres: "Aide memoire de mathematiques superieures", M. Vygodski, Ed. de Moscou.
  { double theta, t;
    double A, B, C, D, E, F, F2, tx0, ty0, a, b;
    
    A = d[0]; B = 0.5*d[1]; C = d[2]; D = 0.5*d[3]; E = 0.5*d[4]; F = d[5]; 
    // calcul du centre
    t = A * C - B * B;
    if (fabs(t) < EPSILON)
    {
      printf("A = %g B = %g C = %g t = %g EPSILON = %g\n", A, B, C, t, EPSILON);      
      printf("lin_ellipsefit: echec recuperation ellipse\n");      
      return 0;
    }
    tx0 = - (D * C - E * B) / t;
    ty0 = - (A * E - D * B) / t;

    F2 = A * tx0 * tx0 + 2 * B * tx0 * ty0 + C * ty0 * ty0 + 
         2 * D * tx0 + 2 * E * ty0 + F;
        
    if (fabs(A-C) < EPSILON) theta = PI_4;
    else theta = 0.5 * atan((2 * B) / (A - C));

    a =  A * cos(theta) * cos(theta) + 
	 C * sin(theta) * sin(theta);
    a = sqrt(- F2 / a);
    b =  A * sin(theta) * sin(theta) +
	 C * cos(theta) * cos(theta);
    b = sqrt(- F2 / b);

    //printf("a = %g ; b = %g ; x0 = %g ; y0 = %g\n", a, b, tx0, ty0);
    *x0 = tx0;
    *y0 = ty0;
    *xa = (a * cos(theta) + tx0);
    *ya = (a * sin(theta) + ty0);
    *xb = (-b * sin(theta) + tx0);
    *yb = (b * cos(theta) + ty0);
  }

  return 1;
} // lellipsefit()

/* ============================================= */
/* ============================================= */
/* ============================================= */


#ifdef TESTLIN3
int32_t main()
{
  int32_t ret; 
  double x[6] = {3, 0, -3, -2, 0, 3};
  double y[6] = {1, 2, 1, -1, -2, -1};
  ret = lellipsefit(x, y, 6);

} // main()
#endif
