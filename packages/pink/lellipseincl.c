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
* Routine Name: lsmallestenclosingellipse
*
* Purpose: 
* Input:
* Output:
* Written By: Yskandar Hamam et Michel Couprie - mai 2002
* D'apres l'article 'Acceleration of D-optimum Design Algorithms by
* Removing Non-optimal Support Points', Luc Pronzato, Rapport de recherche
* I3S/RR-2002-05-FR, Labo. I3S de Sophia Antipolis UMR 6070, Mars 2002
*
****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mcutil.h>
#include <mclin.h>
#include <lellipseincl.h>

#define PI_4 0.78539816339744830962

/* ==================================== */
int32_t lellipseincl(double * X1, double * X2, int32_t np, 
                 double *R, double *S, double *T, double *U, double *V, double *Z
               )
/* ==================================== */
/*! \fn int32_t lellipseincl(double * X1, double * X2, int32_t np, double *R, double *S, double *T, double *U, double *V, double *Z)
    \param X1 (entrée) : liste des abcisses des n points
    \param X2 (entrée) : liste des ordonnées des n points
    \param np (entrée) : nb de points
    \param R,S,T,U,V,Z (sortie) : parametres de l'équation de l'ellipse (Rxx + Syy + 2Txy + 2Ux + 2Vy + Z = 0)
    \return 0 en cas d'erreur, 1 sinon
    \brief détermine la plus petite ellipse incluant les points donnés en entrée (cf. Pronzato 2002)
*/
{
#define LOCALEPS 1e-2
  double delta;
  double alpha0, alpha;
  double *E0, *E, *Ej, *W, *zmz, *xe, *ye, *c, *cc, *tmp, *C, *A, *M;
  double xx, xy, yy, x, y;
  double err, val, sW;
  int32_t iter, k, ret;

  delta = lin_max(X1, np, 1) - lin_min(X1, np, 1) + lin_max(X2, np, 1) - lin_min(X2, np, 1);
  alpha0 = 1;                        // initialisation pour le pas alpha(eq.7)
  E0 = lin_ones(np, 1); 
  lin_multscal(E0, (1.0/np), np, 1); // initialisation de la probabilite
  E = lin_duplicate(E0, np, 1);
  Ej = lin_duplicate(E0, np, 1);
  W = lin_duplicate(E0, np, 1);      // initialisation des poids
  zmz = lin_zeros(np,1);
  xe = lin_duplicate(X1, np, 1);
  ye = lin_duplicate(X2, np, 1);
  err = 10*LOCALEPS;                 // pour faire au moins une iteration
  iter = 0;
  while ((err>LOCALEPS) && (iter<10000)) 
  {
    iter = iter + 1;
    alpha = alpha0 / iter;           // eq.7
    lin_copy(xe, X1, np, 1);
    lin_copy(ye, X2, np, 1);
    lin_multpoint(xe, E, np, 1);
    lin_multpoint(ye, E, np, 1);
    lin_mult(xe, X1, &xx, 1, np, 1); // Calcul de M (cf eq.1)
    lin_mult(xe, X2, &xy, 1, np, 1);
    lin_mult(ye, X2, &yy, 1, np, 1);
    x = lin_sum(xe, np, 1);
    y = lin_sum(ye, np, 1);
    c = lin_zeros(2, 1); c[0] = x; c[1] = y; // centre de l'ellipse      
    M = lin_zeros(2, 2);
    M[0] = xx; M[1] = M[2] = xy; M[3] = yy;
    //#ifdef TESTSMELL
    //lin_printmat(M, 2, 2);
    //#endif
    C = lin_zeros(2, 2);
    lin_mult(c, c, C, 2, 1, 2);
    lin_sub(M, C, 2, 2);
    A = lin_zeros(2, 2);
    ret = lin_invmat2(M, A);
    if (ret == 0)
    {
      fprintf(stderr, "lsmallestenclosingellipse: lin_invmat2 failed\n");
      return 0;
    }
    lin_multscal(A, 0.5, 2, 2);

    // mise a jour des poids et probas
    cc = lin_zeros(2, 1);
    tmp = lin_zeros(2, 1);
    for (k = 0; k < np; k++)
    {
      cc[0] = X1[k]; cc[1] = X2[k];
      lin_sub(cc, c, 2, 1);
      lin_mult(A, cc, tmp, 2, 2, 1);
      lin_mult(cc, tmp, &(zmz[k]), 1, 2, 1);
    }
    val = lin_max(zmz, np, 1);
    err = (val - 1.0) / delta;      // Calcul de l'erreur relative
    if (err > LOCALEPS)
    {
      lin_multpoint(W, zmz, np, 1);
      sW = lin_sum(W, np, 1);
      lin_copy(Ej, W, np, 1);
      lin_multscal(Ej, alpha/sW, np, 1);
      lin_multscal(E, 1 - alpha, np, 1);
      lin_add(E, Ej, np, 1);
    }
  } // while ((err>LOCALEPS) && (iter<10000)) 
#ifdef DEBUG
  printf("iter = %d\n", iter);
#endif
  err = val - 1.0;
  lin_multscal(A, 1.0/(1.0+err), 2, 2);
   
  // passage de la representation: (p-c)' * A * (p-c) = 0 de l'ellipse
  // a la representation: rxx + syy + 2txy + 2ux + 2vy + z = 0
  *R = A[0];
  *S = A[3];
  *T = A[1];
  *U = - (A[0] * c[0] + A[1] * c[1]);
  *V = - (A[1] * c[0] + A[3] * c[1]);
  *Z = c[0]*c[0]*A[0] + 2*c[0]*c[1]*A[1] + c[1]*c[1]*A[3] - 1;

#ifdef TESTSMELL
lin_printmat(A, 2, 2);
lin_printmat(c, 2, 1);
#endif
  free(E0); 
  free(E);
  free(Ej);
  free(W);
  free(zmz);
  free(xe);
  free(ye);
  free(c);
  free(cc);
  free(tmp);
  free(C);
  free(A);
  free(M);
  return 1;   
} // lellipseincl()

#ifdef TESTSMELL
int32_t main()
{
  double x0, y0, xa, ya, xb, yb;
  double X1[20] = {
    0.9501,
    0.2311,
    0.6068,
    0.4860,
    0.8913,
    0.7621,
    0.4565,
    0.0185,
    0.8214,
    0.4447,
    0.6154,
    0.7919,
    0.9218,
    0.7382,
    0.1763,
    0.4057,
    0.9355,
    0.9169,
    0.4103,
    0.8936
  };

  double X2[20] = {
    0.0579,
    0.3529,
    0.8132,
    0.0099,
    0.1389,
    0.2028,
    0.1987,
    0.6038,
    0.2722,
    0.1988,
    0.0153,
    0.7468,
    0.4451,
    0.9318,
    0.4660,
    0.4186,
    0.8462,
    0.5252,
    0.2026,
    0.6721
  };

  //A =[3.0164    0.3653
  //    0.3653    4.2040];


  //c =[0.5942
  //    0.4791];

  lellipseincl((double *)X1, (double *)X2, 20, &x0, &y0, &xa, &ya, &xb, &yb);
  
  printf("x0 = %g ; y0 = %g ; xa = %g ; ya = %g ; xb = %g ; yb = %g\n", x0, y0, xa, ya, xb, yb);

}
#endif
