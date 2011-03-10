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
Librairie mclin : 

fonctions de base d'algebre lineaire

Michel Couprie, mars 2002

Nov. 2006: cor. bug abs->fabs dans lin_solvebidiag
Mar. 2010: lin_solve et lin_solvebidiag et lin_solvetridiag renvoient un code err sur div. 0
*/

/*
conventions : 
  matrice n x m : n = nb lignes, m = nb colonnes
  element (i,j) : i = indice ligne, j = indice colonne
                  offset = i * m + j
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mclin.h>

/*
#define TESTLIN
#define DEBUG
*/

/* ==================================================================== */
/* ==================================================================== */
/* fonctions sur les matrices, vecteurs, systemes */
/* ==================================================================== */
/* ==================================================================== */

/* ==================================== */
double * lin_duplicate(double *A, int32_t n, int32_t m)
/* ==================================== */
/*! \fn double * lin_duplicate(double *A, int32_t n, int32_t m)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return matrice
    \brief retourne une copie de la matrice \b A
*/
#undef F_NAME
#define F_NAME "lin_duplicate"
{
  double *z; int32_t i;
  z = (double *)calloc(1,n * m * sizeof(double));
  if (z == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }
  for (i = 0; i < n * m; i++) z[i] = A[i];
  return z;
} // lin_duplicate()

/* ==================================== */
double * lin_zeros(int32_t n, int32_t m)
/* ==================================== */
/*! \fn double * lin_zeros(double * mat, int32_t n, int32_t m)
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return matrice
    \brief retourne une matrice dont tous les éléments sont a 0.0
*/
#undef F_NAME
#define F_NAME "lin_zeros"
{
  double *z; int32_t i;
  z = (double *)calloc(1,n * m * sizeof(double));
  if (z == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }
  for (i = 0; i < n * m; i++) z[i] = 0.0;
  return z;
} // lin_zeros()

/* ==================================== */
double * lin_ones(int32_t n, int32_t m)
/* ==================================== */
/*! \fn double * lin_ones(double * mat, int32_t n, int32_t m)
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return matrice
    \brief retourne une matrice dont tous les éléments sont a 1.0
*/
#undef F_NAME
#define F_NAME "lin_ones"
{
  double *z; int32_t i;
  z = (double *)calloc(1,n * m * sizeof(double));
  if (z == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }
  for (i = 0; i < n * m; i++) z[i] = 1.0;
  return z;
} // lin_ones()

/* ==================================== */
void lin_printmat(double * mat, int32_t n, int32_t m)
/* ==================================== */
/*! \fn void lin_printmat(double * mat, int32_t n, int32_t m)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \brief affichage matrice \b n x \b m
*/
{
  int32_t i, j;
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < m; j++)  
      printf("%6g   ", mat[i * m + j]);
    printf("\n");
  }          
  printf("\n");
} // lin_printmat()

/* ==================================== */
double lin_min(double * mat, int32_t n, int32_t m)
/* ==================================== */
/*! \fn double lin_min(double * mat, int32_t n, int32_t m)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return élément minimal de \b mat
    \brief retourne l'élément minimal de \b mat
*/
{
  int32_t i; double min = mat[0];
  for (i = 1; i < n*m; i++)
    if (min < mat[i]) min = mat[i];
  return min;
} // lin_min()

/* ==================================== */
double lin_max(double * mat, int32_t n, int32_t m)
/* ==================================== */
/*! \fn double lin_max(double * mat, int32_t n, int32_t m)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return élément maximal de \b mat
    \brief retourne l'élément maximal de \b mat
*/
{
  int32_t i; double max = mat[0];
  for (i = 1; i < n*m; i++)
    if (max < mat[i]) max = mat[i];
  return max;
} // lin_max()

/* ==================================== */
double lin_sum(double * mat, int32_t n, int32_t m)
/* ==================================== */
/*! \fn double lin_sum(double * mat, int32_t n, int32_t m)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return somme des éléments de \b mat
    \brief retourne la somme des éléments de \b mat
*/
{
  int32_t i; double sum = 0.0;
  for (i = 0; i < n*m; i++) sum += mat[i];
  return sum;
} // lin_sum()

/* ==================================== */
double lin_norme2(double * mat, int32_t n, int32_t m)
/* ==================================== */
/*! \fn double lin_norme2(double * mat, int32_t n, int32_t m)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \return norme2 des éléments de \b mat
    \brief retourne la racine carrée de la somme des carrés des éléments de \b mat
*/
{
  int32_t i; double sum = 0.0;
  for (i = 0; i < n*m; i++) sum += (mat[i] * mat[i]);
  return sqrt(sum);
} // lin_norme2()

/* ==================================== */
double lin_normevec2(double * vec, int32_t n)
/* ==================================== */
/*! \fn double lin_normevec2(double * mat, int32_t n)
    \param vec (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : taille vecteur
    \return norme2 des éléments de \b vec
    \brief retourne la racine carrée de la somme des carrés des éléments de \b vec
*/
{
  int32_t i; double sum = 0.0;
  for (i = 0; i < n; i++) sum += (vec[i] * vec[i]);
  return sqrt(sum);
} // lin_normevec2()

/* ==================================== */
void lin_transpose(double * mat, int32_t n)
/* ==================================== */
/*! \fn void lin_transpose(double * mat, int32_t n)
    \param mat (entrée) : pointeur sur le tableau des donnees
    \param n (entrée) : nombre de lignes et de colonnes
    \brief transpose la matrice carrée \b mat (sur place)
*/
{
  int32_t i, j; double t;
  for (i = 0; i < n; i++)
    for (j = i+1; j < n; j++)  
    {
      t = mat[i*n+j];
      mat[i*n+j] = mat[j*n+i];
      mat[j*n+i] = t;
    }
} // lin_transpose()

/* ==================================== */
void lin_copy(double * dest, double * source, int32_t n, int32_t m)
/* ==================================== */
/*! \fn lin_copy(double * dest, double * source, int32_t n, int32_t m)
    \param dest (sortie) : résultat de la copie
    \param source (entrée) : matrice à copier
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \brief copie la matrice source dans \b dest
    \warning la mémoire pour stocker le résultat \b dest doit avoir été allouée
*/
{
  memcpy(dest, source, n * m * sizeof(double));
} // lin_copy()

/* ==================================== */
void lin_multscal(double * A, double r, int32_t n, int32_t m)
/* ==================================== */
/*! \fn void lin_multscal(double * A, double r, int32_t n, int32_t m)
    \param A (entrée/sortie) : matrice
    \param r (entrée) : scalaire
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \brief multiplication de chaque element de la matrice \b A par le scalaire \b r
*/
{
  int32_t i;
  for (i = 0; i < n * m; i++) A[i] *= r;
} // lin_multscal()

/* ==================================== */
void lin_multpoint(double * A, double * B, int32_t n, int32_t m)
/* ==================================== */
/*! \fn lin_multpoint(double * A, double * B, int32_t n, int32_t m)
    \param A (entrée/sortie) : matrice
    \param B (entrée) : matrice
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \brief multiplication point à point de \b A par \b B - résultat dans \b A
*/
{
  int32_t i;
  for (i = 0; i < n * m; i++) A[i] *= B[i];
} // lin_multpoint()

/* ==================================== */
void lin_sub(double * A, double * B, int32_t n, int32_t m)
/* ==================================== */
/*! \fn lin_sub(double * A, double * B, int32_t n, int32_t m)
    \param A (entrée/sortie) : matrice
    \param B (entrée) : matrice
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \brief soustraction de \b B à \b A - résultat dans \b A
*/
{
  int32_t i;
  for (i = 0; i < n * m; i++) A[i] -= B[i];
} // lin_sub()

/* ==================================== */
void lin_add(double * A, double * B, int32_t n, int32_t m)
/* ==================================== */
/*! \fn lin_add(double * A, double * B, int32_t n, int32_t m)
    \param A (entrée/sortie) : matrice
    \param B (entrée) : matrice
    \param n (entrée) : nombre de lignes
    \param m (entrée) : nombre de colonnes
    \brief addition de \b B à \b A - résultat dans \b A
*/
{
  int32_t i;
  for (i = 0; i < n * m; i++) A[i] += B[i];
} // lin_add()

/* ==================================== */
void lin_mult(double * A, double * B , double * R, int32_t p, int32_t q, int32_t r)
/* ==================================== */
/*! \fn void lin_mult(double * A, double B , double * R, int32_t p, int32_t q, int32_t r)
    \param A (entrée) : opérande gauche
    \param B (entrée) : opérande droit
    \param R (sortie) : résultat (taille p x r) 
    \param p (entrée) : nb de lignes de A
    \param q (entrée) : nb de colonnes de A = nb de lignes de B
    \param r (entrée) : nb de colonnes de B
    \brief multiplication matrice \b A (taille \b p x \b q) par matrice \b B (taille \b q x \b r)
    \warning la mémoire pour stocker le résultat \b R doit avoir été allouée
*/
{
  double t;
  int32_t i, j, k;
  for (i = 0; i < p; i++)
    for (j = 0; j < r; j++)  
    {
      t = 0.0;
      for (k = 0; k < q; k++)  
        t += A[i * q + k] * B[k * r + j];
      R[i * r + j] = t;    
    }
} // lin_mult()

/* ==================================== */
void lin_multAtB(double * A, double * B , double * R, int32_t nA, int32_t mA, int32_t nB, int32_t mB)
/* ==================================== */
/*! \fn void lin_multAtB(double * A, double * B , double * R, int32_t nA, int32_t mA, int32_t nB, int32_t mB)
    \param A (entrée) : opérande gauche
    \param B (entrée) : opérande droit
    \param R (sortie) : résultat (taille mA x mB) 
    \param nA (entrée) : nb de lignes de A
    \param mA (entrée) : nb de colonnes de A
    \param nB (entrée) : nb de lignes de B
    \param mB (entrée) : nb de colonnes de B
    \brief multiplication de la matrice \b A transposée par matrice \b B
    \warning la mémoire pour stocker le résultat \b R doit avoir été allouée
    \warning on doit avoir: \b nA = \b nB (pas de vérification)
*/
{
  double t;
  int32_t i, j, k;
  for (i = 0; i < mA; i++)
    for (j = 0; j < mB; j++)  
    {
      t = 0.0;
      for (k = 0; k < nA; k++)  
        t += A[k * mA + i] * B[k * mB + j];
      R[i * mB + j] = t;    
    }
} // lin_multAtB()

/* ==================================== */
int32_t lin_decomposition_LUP(double * A, int32_t * pi, int32_t n)
/* ==================================== */
/*! \fn int32_t lin_decomposition_LUP(double * A, double * p, int32_t n)
    \param A (entrée/sortie) : opérande, et résultat (L et U dans la même matrice)
    \param pi (sortie) : permutation (sous la forme d'un vecteur d'indices de colonnes)
    \param n (entrée) : nb de lignes et de colonnes de A
    \return 0 si la matrice est singulière, 1 sinon
    \brief décomposition LUP de la matrice carrée A (cf. Cormen & al., "Introduction a l'algorithmique") 
    \warning la mémoire pour stocker le résultat \b p doit avoir été allouée
    \warning le contenu de la matrice A est effacé
*/
{
  double p, t;
  int32_t i, j, k, kp;
  for (i = 0; i < n; i++) pi[i] = i;

  for (k = 0; k < n; k++)
  {
    p = 0;
    for (i = k; i < n; i++)
    {
      t = mcabs(A[i*n+k]);
      if (t > p) { p = t; kp = i; }
    }
    if (p < MCLIN_EPSILON) return 0;
    t = pi[k]; pi[k] = pi[kp]; pi[kp] = t; // echange
    for (i = 0; i < n; i++)
    {  t = A[k*n+i]; A[k*n+i] = A[kp*n+i]; A[kp*n+i] = t; } // echange
    for (i = k+1; i < n; i++)
    {
      A[i*n+k] /= A[k*n+k];
      for (j = k+1; j < n; j++)
        A[i*n+j] -= (A[i*n+k] * A[k*n+j]);
    }    
  }
  for (k = 0; k < n; k++) if (mcabs(A[k*n+k]) < MCLIN_EPSILON) return 0; 
  return 1;
} // lin_decomposition_LUP()

/* ==================================== */
int32_t lin_decomposition_cholesky(double * a, double * l, int32_t n)
/* ==================================== */
/*! \fn int32_t lin_decomposition_cholesky(double * a, int32_t * l, int32_t n)
    \param a (entrée) : matrice à décomposer (supposée symétrique définie positive) 
    \param l (sortie) : résultat l (triangulaire inférieure)
    \param n (entrée) : nb de lignes et de colonnes de A
    \return 0 si la matrice est non définie positive, 1 sinon
    \brief décomposition de Cholesky de a, le résultat l est tel que l * l' = a
    \warning la mémoire pour stocker le résultat \b l doit avoir été allouée
*/
{
  int32_t i,j,k;
  double sum;
  double *p = (double *)calloc(1,n * sizeof(double));
  if (p == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }

  for (i=0; i<n; i++)
    for (j=i; j<n; j++)  
    {
      for (sum=a[i*n+j],k=i-1;k>=0;k--) sum -= a[i*n+k]*a[j*n+k];
      if (i == j) 
      {
	if (sum<=0.0)  
	{
	  printf("i = %d , j = %d, sum = %g\n", i, j, sum);
          return 0;
	}
	else 
	  p[i]=sqrt(sum); 
      }
      else 
      {
	a[j*n+i]=sum/p[i];
      }
    } // for i,j

  for (i=0; i<n; i++)  
  {
    for (j=i; j<n; j++)  
      if (i==j)
	l[i*n+i] = p[i];
      else
      {
	l[j*n+i]=a[j*n+i];  
	l[i*n+j]=0.0;
      }    
  }
  free(p);
  return 1;
} // lin_decomposition_cholesky()

/* ==================================== */
int32_t lin_inverse_gauss(double *TB, double *InvB, int32_t N)
/* ==================================== */
/*! \fn int32_t lin_inverse_gauss(double *TB, double *InvB, int32_t N)
    \param TB (entrée) : matrice à inverser (supposée triangulaire inférieure)
    \param InvB (sortie) : résultat
    \param N (entrée) : nb de lignes et de colonnes de TB
    \return 0 si la matrice est singulière, 1 sinon
    \brief inversion de la matrice TB par la méthode de Gauss-Jordan
    \warning la mémoire pour stocker le résultat \b InvB doit avoir été allouée
*/
#undef F_NAME
#define F_NAME "lin_inverse_gauss"
{
      int32_t k,i,j,p,q;
      double mult;
      double D,temp;
      double maxpivot;
      int32_t npivot;
      int32_t mB = N + 2;
      int32_t mA = (N+1) * 2;
      double *B = (double *)calloc((N+1) * (N+2), sizeof(double));
      double *A = (double *)calloc((N+1) * (N+1) * 2, sizeof(double));
      double eps = 10e-20;
      
      for(k=1;k<=N;k++)
	for(j=1;j<=N;j++)
	  B[k*mB+j]=TB[(k-1)*N+j-1];
      
      for (k=1;k<=N;k++)
      {
	for (j=1;j<=N+1;j++)
	  A[k*mA+j]=B[k*mB+j];
	for (j=N+2;j<=2*N+1;j++)
	  A[k*mA+j]=(double)0;
	A[k*mA + k-1+N+2]=(double)1;
      }
      for (k=1;k<=N;k++)
      {
	maxpivot=mcabs(A[k*mA+k]);
	npivot=k;
	for (i=k;i<=N;i++)
	  if (maxpivot>mcabs(A[i*mA+k])) // 2/3/2006 : cor. BUG ( < )
	  {
	    maxpivot=mcabs(A[i*mA+k]);
	    npivot=i;
	  }
	if (maxpivot>=eps)
	{      
	  if (npivot!=k)
	    for (j=k;j<=2*N+1;j++)
	    {
	      temp=A[npivot*mA+j];
	      A[npivot*mA+j]=A[k*mA+j];
	      A[k*mA+j]=temp;
	    } ;
	  D=A[k*mA+k];
	  for (j=2*N+1;j>=k;j--)
	    A[k*mA+j]=A[k*mA+j]/D;
	  for (i=1;i<=N;i++)
	  {
	    if (i!=k)
	    {
	      mult=A[i*mA+k];
	      for (j=2*N+1;j>=k;j--)
		A[i*mA+j]=A[i*mA+j]-mult*A[k*mA+j] ;
	    }
	  }
	}
	else
	{
	  fprintf(stderr, "%s: maxpivot ( %g ) < eps ( %g )\n", F_NAME, maxpivot, eps);
	  return 0;
	}
      }
      /* Copie le résultat dans la matrice InvB */
      for (k=1,p=0;k<=N;k++,p++)
	for (j=N+2,q=0;j<=2*N+1;j++,q++)
	  InvB[p*N+q]=A[k*mA+j];
      return 1;
} // lin_inverse_gauss()
    

void ROTATE(double * a, int32_t i, int32_t j, int32_t k, int32_t l, double tau, double s, int32_t N) 
{
  double g,h;
  g=a[i*N+j]; h=a[k*N+l]; a[i*N+j]=g-s*(h+g*tau);
  a[k*N+l]=h+s*(g-h*tau);
}

/* ==================================== */
int32_t lin_jacobi(double * A, int32_t n, double * D, double * V, int32_t nrot)      
/* ==================================== */
/*! \fn void lin_jacobi(double * A, int32_t N, double * D, double * V, int32_t nrot)
    \param A (entrée) : matrice à traiter
    \param n (entrée) : nb de lignes et de colonnes de A
    \param D (sortie) : valeurs propres
    \param V (sortie) : vecteurs propres
    \param nrot (entrée) : non utilisé
    \return 0 si erreur (trop d'itérations), 1 sinon
    \brief calcul des valeurs et vecteurs propres de a par la méthode de Jacobi
    \warning la matrice A doit être symétrique
    \warning la mémoire pour stocker les résultat \b D et \b V doit avoir été allouée
*/
{
	int32_t j, iq, ip, i;
	double tresh,theta,tau,t,sm,s,h,g,c;
        int32_t N = n+1; 
        double *a = (double *)calloc((n+1) * (n+1), sizeof(double));
        double *v = (double *)calloc((n+1) * (n+1), sizeof(double));
	double *b = (double *)calloc((n+1), sizeof(double));
	double *z = (double *)calloc((n+1), sizeof(double));
	double *d = (double *)calloc((n+1), sizeof(double));
	
	for (ip=1;ip<=n;ip++)  // transfert A -> a
	  for (iq=1;iq<=n;iq++) 
            a[ip*N+iq] = A[(ip-1)*n + iq-1];

	for (ip=1;ip<=n;ip++) {
	  for (iq=1;iq<=n;iq++) v[ip*N+iq]=0.0;
	  v[ip*N+ip]=1.0;
	}
	for (ip=1;ip<=n;ip++) {
	  b[ip]=d[ip]=a[ip*N+ip];
	  z[ip]=0.0;
	}
	nrot=0;
	for (i=1;i<=50;i++) {
	  sm=0.0;
	  for (ip=1;ip<=n-1;ip++) {
	    for (iq=ip+1;iq<=n;iq++)
	      sm += mcabs(a[ip*N+iq]);
	  }
	  if (sm == 0.0) goto fin;
	  if (i < 4)
	    tresh=0.2*sm/(n*n);
	  else
	    tresh=0.0;
	  for (ip=1;ip<=n-1;ip++) {
	    for (iq=ip+1;iq<=n;iq++) {
	      g=100.0*mcabs(a[ip*N+iq]);
	      if (i > 4 && mcabs(d[ip])+g == mcabs(d[ip])
		  && mcabs(d[iq])+g == mcabs(d[iq]))
		a[ip*N+iq]=0.0;
	      else if (mcabs(a[ip*N+iq]) > tresh) {
		h=d[iq]-d[ip];
		if (mcabs(h)+g == mcabs(h))
		  t=(a[ip*N+iq])/h;
		else {
		  theta=0.5*h/(a[ip*N+iq]);
		  t=1.0/(mcabs(theta)+sqrt(1.0+theta*theta));
		  if (theta < 0.0) t = -t;
		}
		c=1.0/sqrt(1+t*t);
		s=t*c;
		tau=s/(1.0+c);
		h=t*a[ip*N+iq];
		z[ip] -= h;
		z[iq] += h;
		d[ip] -= h;
		d[iq] += h;
		a[ip*N+iq]=0.0;
		for (j=1;j<=ip-1;j++) {
		  ROTATE(a,j,ip,j,iq,tau,s,N);
		  }
		for (j=ip+1;j<=iq-1;j++) {
		  ROTATE(a,ip,j,j,iq,tau,s,N);
		  }
		for (j=iq+1;j<=n;j++) {
		  ROTATE(a,ip,j,iq,j,tau,s,N);
		  }
		for (j=1;j<=n;j++) {
		  ROTATE(v,j,ip,j,iq,tau,s,N);
		  }
		++nrot;
	      }
	    }
	  }
	  for (ip=1;ip<=n;ip++) {
	    b[ip] += z[ip];
	    d[ip]=b[ip];
	    z[ip]=0.0;
	  }
	}
	printf("Too many iterations in routine JACOBI");
        return 0;
      fin:
	for (ip=1;ip<=n;ip++)  // transfert v -> V
	  for (iq=1;iq<=n;iq++) 
            V[(ip-1)*n + iq-1] = v[ip*N+iq];
	for (ip=1;ip<=n;ip++)  // transfert d -> D
          D[ip-1] = d[ip];
        free(a);
        free(b);
        free(z);
        free(v);
        free(d);
        return 1;
}

/* ==================================== */
void lin_permutmat(int32_t * pi, double * P, int32_t n)
/* ==================================== */
/*! \fn void lin_permutmat(double * pi, double * A, int32_t n)
    \param pi (entrée) : permutation (sous la forme d'un vecteur d'indices de colonnes)
    \param P (sortie) : matrice de permutation générée, sous forme de tableau n x n
    \param n (entrée) : nb de lignes et de colonnes de P
    \brief conversion d'une permutation en matrice de permutation
    \warning la mémoire pour stocker le résultat \b P doit avoir été allouée
*/
{
  int32_t i, j;
  
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (pi[i] == j) P[i*n+j] = 1; else P[i*n+j] = 0;
} // lin_permutmat()

/* ==================================== */
void lin_LU(double * A, double * L, double * U, int32_t n)
/* ==================================== */
/*! \fn void lin_LU(double * A, double * L, double * U, int32_t n)
    \param A (entrée) : matrice contenant une décomposition LU
    \param L (sortie) : matrice L, triangulaire inférieure unitaire, extraite de A
    \param U (sortie) : matrice U, triangulaire supérieure, extraite de A
    \param n (entrée) : nb de lignes et de colonnes de A
    \brief extraction des matrices L et U d'une matrice A contenant une décomposition LU
    \warning la mémoire pour stocker les résultats \b L, \b U doit avoir été allouée
*/
{
  int32_t i, j;
  
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (i > j)
      { L[i*n+j] = A[i*n+j]; U[i*n+j] = 0; }
      else if (i == j)
      { L[i*n+j] = 1; U[i*n+j] = A[i*n+j]; }
      else
      { U[i*n+j] = A[i*n+j]; L[i*n+j] = 0; }
} // lin_LU()

/* ==================================== */
void lin_solveLUP(double * LU, int32_t *P, double * b, double * x, int32_t n)
/* ==================================== */
/*! \fn void lin_solveLUP(double * LU, int32_t *P, double * b, double * x, int32_t n)
    \param LU (entrée) : matrice contenant la décomposition LU de A
    \param P (entrée) : permutation de la décomposition LUP de A
    \param b (entrée) : vecteur second membre du système
    \param x (sortie) : vecteur résultat
    \param n (entrée) : nb de lignes et de colonnes de L
    \brief résolution du système Ax = b, d'apres une décomposition LUP de A
    \warning la mémoire pour stocker le résultat \b x doit avoir été allouée
*/
{
  int32_t i, j;
  double t, *y = (double *)calloc(1,n * sizeof(double));
  if (y == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }

  for (i = 0; i < n; i++)
  {
    t = 0;
    for (j = 0; j < i; j++) t += LU[i*n+j] * y[j];
    y[i] = b[P[i]] - t;  
  }

  //  lin_printmat((double *)y, 3, 1);

  for (i = n-1; i >= 0; i--)
  {
    t = 0;
    for (j = i+1; j < n; j++) t += LU[i*n+j] * x[j];
    x[i] = (y[i] - t) / LU[i*n+i];  
    //printf("t = %g\n", t);
  }
  free(y);
} // lin_solveLUP()

/* ==================================== */
int32_t lin_solve(double * A, double * b, double * x, int32_t n)
/* ==================================== */
/*! \fn void lin_solve(double * A, double * b, double * x, int32_t n)
    \param A (entrée) : matrice du système
    \param b (entrée) : vecteur second membre du système
    \param x (sortie) : vecteur résultat
    \param n (entrée) : nb de lignes et de colonnes de A
    \brief résolution du système Ax = b par décomposition LU
    \warning la mémoire pour stocker le résultat \b x doit avoir été allouée
*/
#undef F_NAME
#define F_NAME "lin_solve"
{
  int32_t ret, *pi = (int32_t *)calloc(1,n * sizeof(int32_t));
  if (pi == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }
  ret = lin_decomposition_LUP((double *)A, (int32_t *)pi, n);
  if (ret == 0) 
  {
    free(pi);
    return 0;
  }
  lin_solveLUP((double *)A, (int32_t *)pi, (double *)b, (double *)x, n);
  free(pi);
  return 1;
} // lin_solve()

/* ==================================== */
int32_t lin_solvebidiag(double * A, double * b, double * x, int32_t n)
/* ==================================== */
/*! \fn void lin_solvebidiag(double * A, double * b, double * x, int32_t n)
    \param A (entrée) : matrice du système (doit être bi-diagonale supérieure)
    \param b (entrée) : vecteur second membre du système
    \param x (sortie) : vecteur résultat
    \param n (entrée) : nb de lignes et de colonnes de A
    \brief résolution du système Ax = b
    \warning la mémoire pour stocker le résultat \b x doit avoir été allouée
*/
#undef F_NAME
#define F_NAME "lin_solvebidiag"
{
  int32_t i;
  for (i = n-1; i >= 0; i--)
    if (fabs(A[i*n+i]) < MCLIN_EPSILON) 
      return 0;
  x[n-1] = b[n-1] / A[(n-1)*n + n-1];
  for (i = n-2; i >= 0; i--)
  {
    x[i] = (b[i] - (A[i*n + i+1] * x[i+1])) / A[i*n + i];
  }
  return 1;
} // lin_solvebidiag()

/* ==================================== */
int32_t lin_solvetridiag(double * A, double * b, double * x, int32_t n)
/* ==================================== */
/*! \fn void lin_solvetridiag(double * A, double * b, double * x, int32_t n)
    \param A (entrée) : matrice du système (doit être tri-diagonale)
    \param b (entrée) : vecteur second membre du système
    \param x (sortie) : vecteur résultat
    \param n (entrée) : nb de lignes et de colonnes de A
    \brief résolution du système Ax = b
    \warning la mémoire pour stocker le résultat \b x doit avoir été allouée
    \warning les contenus de A et de b seront modifiés
*/
#undef F_NAME
#define F_NAME "lin_solvetridiag"
{
  int32_t i;

  for (i = 1; i < n; i++)
  {
    A[(i)*n + i] = A[(i-1)*n + i] * A[(i)*n + i-1] - 
                   A[(i-1)*n + i-1] * A[(i)*n + i];
    if (i < n-1) A[(i)*n + i+1] = - A[(i-1)*n + i-1] * A[(i)*n + i+1];
    b[i] = A[(i)*n + i-1] * b[i-1] - A[(i-1)*n + i-1] * b[i];
    // A[(i)*n + i-1] = 0.0; // inutile pour la suite 
  }
  return lin_solvebidiag(A, b, x, n);
} // lin_solvetridiag()

/* ==================================== */
double lin_det2(double * m)
/* ==================================== */
{
  return m[0*2+0]*m[1*2+1] - m[0*2+1]*m[1*2+0];
}

/* ==================================== */
int32_t lin_invmat2(double * ma, double * mr)
/* ==================================== */
{
  double det = lin_det2( ma );
  if ( fabs( det ) < MCLIN_EPSILON ) 
  {
    printf("lin_invmat2: det = %g\n", det);
    return 0;
  }
  mr[0*2+0] =   ma[1*2+1] / det;
  mr[1*2+0] = - ma[1*2+0] / det;
  mr[0*2+1] = - ma[0*2+1] / det;
  mr[1*2+1] =   ma[0*2+0] / det;
  return 1;
} // lin_invmat2()

/* ==================================== */
double lin_det3(double * m)
/* ==================================== */
{
  return m[0*3+0]*m[1*3+1]*m[2*3+2] + m[0*3+1]*m[1*3+2]*m[2*3+0] + m[1*3+0]*m[2*3+1]*m[0*3+2] 
       - m[0*3+2]*m[1*3+1]*m[2*3+0] - m[0*3+1]*m[1*3+0]*m[2*3+2] - m[1*3+2]*m[2*3+1]*m[0*3+0];
}

/* ==================================== */
int32_t lin_invmat3(double *ma, double *mr)
/* ==================================== */
{
  double det = lin_det3( ma );
  if ( fabs( det ) < MCLIN_EPSILON ) return 0;
  mr[0*3+0] =  ( ma[1*3+1]*ma[2*3+2] - ma[1*3+2]*ma[2*3+1] ) / det;
  mr[0*3+1] = -( ma[0*3+1]*ma[2*3+2] - ma[2*3+1]*ma[0*3+2] ) / det;
  mr[0*3+2] =  ( ma[0*3+1]*ma[1*3+2] - ma[1*3+1]*ma[0*3+2] ) / det;
  mr[1*3+0] = -( ma[1*3+0]*ma[2*3+2] - ma[1*3+2]*ma[2*3+0] ) / det;
  mr[1*3+1] =  ( ma[0*3+0]*ma[2*3+2] - ma[2*3+0]*ma[0*3+2] ) / det;
  mr[1*3+2] = -( ma[0*3+0]*ma[1*3+2] - ma[1*3+0]*ma[0*3+2] ) / det;
  mr[2*3+0] =  ( ma[1*3+0]*ma[2*3+1] - ma[2*3+0]*ma[1*3+1] ) / det;
  mr[2*3+1] = -( ma[0*3+0]*ma[2*3+1] - ma[2*3+0]*ma[0*3+1] ) / det;
  mr[2*3+2] =  ( ma[0*3+0]*ma[1*3+1] - ma[0*3+1]*ma[1*3+0] ) / det;
  return 1;
} // lin_invmat3()

/* ==================================== */
int32_t lin_inverseLUP(double * A, double * R, int32_t n)
/* ==================================== */
/*! \fn 
    \param A (entrée) : matrice à inverser
    \param R (sortie) : résultat (inverse de A)
    \param n (entrée) : nb de lignes et de colonnes de A
    \return 0 si la matrice est singulière, 1 sinon
    \brief inversion de la matrice, d'apres une décomposition LUP de A
    \warning la mémoire pour stocker le résultat \b R doit avoir été allouée
    \warning le contenu de la matrice A est effacé
*/
{
  int32_t i, j, ret;
  int32_t *pi;
  double *x;
  double *e;

  pi = (int32_t *)calloc(1,n * sizeof(int32_t));
  if (pi == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }
  ret = lin_decomposition_LUP(A, pi, n);
  if (ret == 0) return 0;

  x = (double *)calloc(1,n * sizeof(double));
  e = (double *)calloc(n, sizeof(double));
  if ((x == NULL) || (e == NULL))
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(0);
  }
  for (i = 0; i < n; i++)
  {
    e[i] = 1; if (i > 0) e[i-1] = 0; // prepare e_i
    lin_solveLUP(A, pi, e, x, n);
    for (j = 0; j < n; j++) R[j*n+i] = x[j];
  }
  free(x);
  free(e);
  free(pi);
  return 1;
} // lin_inverseLUP()

/* ==================================== */
int32_t lin_trouvemin(double * x, double * d, double (*F)(double *, int32_t), int32_t n, double *precision)
/* ==================================== */
/*! \fn lin_trouvemin(double * x, double * d, double (*F)(double *, int32_t), int32_t n, double *precision)
    \param x (entrée/sortie) : valeur initiale et résultat (vecteur de taille n)
    \param d (entrée/sortie) : valeur initiale/finale du vecteur d'incréments (taille n)
    \param F (entrée) : fonction de R^n dans R dont on doit trouver le min
    \param n (entrée) : dimension du problème (taille des vecteurs x et d0)
    \param precision (entrée) : vecteur spécificant la précision voulue (valeur minimale de l'incrément) dans chacune des dimensions
    \return le nombre d'itérations plus 1 si la stabilité est atteinte, 0 sinon
    \brief trouve le minimum de la fonction F de R^n dans R, supposée convexe
*/
{
  const int32_t MAXITER = 500;
  int32_t i, k;
  double *newx = lin_zeros(n, 1);
  double Fplusd, Fmoinsd, Fprev, Fcur = (*F)(x, n);

  for (k = 0; k < MAXITER; k++)
  {
    Fprev = Fcur;
    for (i = 0; i < n; i++)
    {
      x[i] += d[i]; Fplusd = (*F)(x, n); x[i] -= d[i];
      if (Fplusd < Fcur) newx[i] = x[i] + d[i];
      else
      {
        x[i] -= d[i]; Fmoinsd = (*F)(x, n); x[i] += d[i];
        if (Fmoinsd < Fcur) newx[i] = x[i] - d[i];
        else
          d[i] = d[i] / 2;
      }
    } // for (i = 0; i < n; i++)
    // nouveau x = barycentre des nouvelles positions dans chaque dimension
    for (i = 0; i < n; i++)
      x[i] = ((n-1) * x[i] + newx[i]) / n; 
    // update Fcur
    Fcur = (*F)(x, n);
    // test stabilite
    for (i = 0; i < n; i++)
      if (d[i] > precision[i]) break; // sort du for i
    if (i == n) break; // sort du for k
  } // for (k = 0; k < MAXITER; k++)

  free(newx);
  if (k < MAXITER) return k+1; else return 0;
 
} // lin_trouvemin()

/* ==================================== */
int32_t lidentifyline(double *pbx, double *pby, int32_t npb, double *a, double *b)
/* ==================================== */
#undef F_NAME
#define F_NAME "lidentifyline"
/*
 Identifie les parametres (a,b) de l'equation y = ax + b d'une droite
 pour minimiser l'ecart (au sens des moindres carres)
 entre cette droite et les points contenus dans la liste de points (pbx,pby).
 Régression linéaire (voir http://en.wikipedia.org/wiki/Linear_regression ).
 */
{
  int32_t i, ret, noresult = 1;
  double *X, *Y, *XtX, *XtXi, *XtY, *RtXtY, *YtY, *R;

  if (npb < 2)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }  

  X = lin_zeros(npb, 2);
  Y = lin_zeros(npb, 1);
  XtX = lin_zeros(2, 2);
  XtXi = lin_zeros(2, 2);
  XtY = lin_zeros(2, 1);
  R = lin_zeros(2, 1);
  RtXtY = lin_zeros(1, 1);
  YtY = lin_zeros(1, 1);

  for (i = 0; i < npb; i++)
  {
    X[2*i] = 1.0;
    X[2*i + 1] = pbx[i];
    Y[i] = pby[i];
  }  
  lin_multAtB(X, X, XtX, npb, 2, npb, 2);
  ret = lin_invmat2(XtX, XtXi);
  if (ret != 0)
  { 
    noresult = 0;
    lin_multAtB(X, Y, XtY, npb, 2, npb, 1);
    lin_mult(XtXi, XtY, R, 2, 2, 1);
    *a = R[1];
    *b = R[0];
  }

  free(X);
  free(Y);
  free(XtX);
  free(XtXi);
  free(XtY);
  free(RtXtY);
  free(YtY);
  free(R);
  if (noresult) return 0;
  return 1;
} /* lidentifyline() */

/* ==================================== */
int32_t lidentifyparabola3(double *pbx, double *pby, int32_t npb, double *a, double *b, double *c)
/* ==================================== */
#undef F_NAME
#define F_NAME "lidentifyparabola"
/*
 Identifie les parametres (a,b,c) de l'equation y = ax^2 + bx + c d'une parabole, 
 pour minimiser l'ecart (au sens des moindres carres)
 entre cette parabole et les points contenus dans la liste de points (pbx,pby).
 Régression linéaire (voir http://en.wikipedia.org/wiki/Linear_regression ).
 */
{
  int32_t i, ret, noresult = 1;
  double *X, *Y, *XtX, *XtXi, *XtY, *RtXtY, *YtY, *R;

  if (npb < 3)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }  

  X = lin_zeros(npb, 3);
  Y = lin_zeros(npb, 1);
  XtX = lin_zeros(3, 3);
  XtXi = lin_zeros(3, 3);
  XtY = lin_zeros(3, 1);
  R = lin_zeros(3, 1);
  RtXtY = lin_zeros(1, 1);
  YtY = lin_zeros(1, 1);

  for (i = 0; i < npb; i++)
  {
    X[3*i] = 1.0;
    X[3*i + 1] = pbx[i];
    X[3*i + 2] = pbx[i] * pbx[i];
    Y[i] = pby[i];
  }  
  lin_multAtB(X, X, XtX, npb, 3, npb, 3);
  ret = lin_invmat3(XtX, XtXi);
  if (ret != 0)
  { 
    noresult = 0;
    lin_multAtB(X, Y, XtY, npb, 3, npb, 1);
    lin_mult(XtXi, XtY, R, 3, 3, 1);
    *a = R[2];
    *b = R[1];
    *c = R[0];
  }

  free(X);
  free(Y);
  free(XtX);
  free(XtXi);
  free(XtY);
  free(RtXtY);
  free(YtY);
  free(R);
  if (noresult) return 0;
  return 1;
} /* lidentifyparabola() */

/* ==================================== */
int32_t lidentifyparabola2(double *pbx, double *pby, int32_t npb, double *a, double *b)
/* ==================================== */
#undef F_NAME
#define F_NAME "lidentifyparabola"
/*
 Identifie les parametres (a,b,c) de l'equation y = ax^2 + b d'une parabole
 d'axe de symétrie vertical, pour minimiser l'ecart (au sens des moindres carres)
 entre cette parabole et les points contenus dans la liste de points (pbx,pby).
 Régression linéaire (voir http://en.wikipedia.org/wiki/Linear_regression ).
 */
{
  int32_t i, ret, noresult = 1;
  double *X, *Y, *XtX, *XtXi, *XtY, *RtXtY, *YtY, *R;

  if (npb < 3)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }  

  X = lin_zeros(npb, 2);
  Y = lin_zeros(npb, 1);
  XtX = lin_zeros(2, 2);
  XtXi = lin_zeros(2, 2);
  XtY = lin_zeros(2, 1);
  R = lin_zeros(2, 1);
  RtXtY = lin_zeros(1, 1);
  YtY = lin_zeros(1, 1);

  for (i = 0; i < npb; i++)
  {
    X[2*i] = 1.0;
    X[2*i + 1] = pbx[i] * pbx[i];
    Y[i] = pby[i];
  }  
  lin_multAtB(X, X, XtX, npb, 2, npb, 2);
  ret = lin_invmat2(XtX, XtXi);
  if (ret != 0)
  { 
    noresult = 0;
    lin_multAtB(X, Y, XtY, npb, 2, npb, 1);
    lin_mult(XtXi, XtY, R, 2, 2, 1);
    *a = R[1];
    *b = R[0];
  }

  free(X);
  free(Y);
  free(XtX);
  free(XtXi);
  free(XtY);
  free(RtXtY);
  free(YtY);
  free(R);
  if (noresult) return 0;
  return 1;
} /* lidentifyparabola() */

/* ==================================== */
int32_t lidentifyplane(double *pbx, double *pby, double *pbz, index_t npb, double *a, double *b, double *c, double *d, double *error)
/* ==================================== */
#undef F_NAME
#define F_NAME "lidentifyplane"
/*
 Identifie les parametres (a, b, c, d) de l'equation d'un plan 3D
 pour minimiser l'ecart (au sens des moindres carres)
 entre ce plan et les points contenus dans la liste de points (pbx,pby,pbz).
 Régression linéaire (voir http://en.wikipedia.org/wiki/Linear_regression ).
 */
{
  index_t i;
  int32_t ret, noresult = 1;
  double *X, *Y, *XtX, *XtXi, *XtY, *RtXtY, *YtY, *R;
  double err;

  if (npb < 3)
  {
    fprintf(stderr, "%s: not enough points\n", F_NAME);
    return 0;
  }  

  X = lin_zeros(npb, 3);
  Y = lin_zeros(npb, 1);
  XtX = lin_zeros(3, 3);
  XtXi = lin_zeros(3, 3);
  XtY = lin_zeros(3, 1);
  R = lin_zeros(3, 1);
  RtXtY = lin_zeros(1, 1);
  YtY = lin_zeros(1, 1);

  // L'équation cherchée est du type : ax + by + cz + d = 0.
  // Pour réduire le nombre d'inconnues à 3, on force l'un des 
  // trois paramètres a,b,c à -1.
  // Pour savoir lequel, on fait les trois calculs et l'on retient
  // celui qui donne l'erreur minimale.

  err = FLT_MAX;
  for (i = 0; i < npb; i++)
  {
    X[3*i] = 1.0;
    X[3*i + 1] = pbx[i];
    X[3*i + 2] = pby[i];
    Y[i] = pbz[i];
  }  
  lin_multAtB(X, X, XtX, npb, 3, npb, 3);
  ret = lin_invmat3(XtX, XtXi);
  if (ret != 0)
  { 
    noresult = 0;
    lin_multAtB(X, Y, XtY, npb, 3, npb, 1);
    lin_mult(XtXi, XtY, R, 3, 3, 1);
    // calcule l'erreur
    lin_multAtB(R, XtY, RtXtY, 3, 1, 3, 1);
    lin_multAtB(Y, Y, YtY, npb, 1, npb, 1);
    err = *YtY - *RtXtY;
    //    printf("cas 1 : erreur %g\n", err);
    *a = R[1];
    *b = R[2];
    *c = -1;
    *d = R[0];
  }

  for (i = 0; i < npb; i++)
  {
    X[3*i] = 1.0;
    X[3*i + 1] = pbz[i];
    X[3*i + 2] = pbx[i];
    Y[i] = pby[i];
  }  
  lin_multAtB(X, X, XtX, npb, 3, npb, 3);
  ret = lin_invmat3(XtX, XtXi);
  if (ret != 0)
  { 
    noresult = 0;
    lin_multAtB(X, Y, XtY, npb, 3, npb, 1);
    lin_mult(XtXi, XtY, R, 3, 3, 1);
    // calcule l'erreur
    lin_multAtB(R, XtY, RtXtY, 3, 1, 3, 1);
    lin_multAtB(Y, Y, YtY, npb, 1, npb, 1);
    //    printf("cas 2 : erreur %g\n", *YtY - *RtXtY);
    if (*YtY - *RtXtY < err)
    {
      err = *YtY - *RtXtY;
      *a = R[2];
      *b = -1;
      *c = R[1];
      *d = R[0];
    }
  }

  for (i = 0; i < npb; i++)
  {
    X[3*i] = 1.0;
    X[3*i + 1] = pby[i];
    X[3*i + 2] = pbz[i];
    Y[i] = pbx[i];
  }  
  lin_multAtB(X, X, XtX, npb, 3, npb, 3);
  ret = lin_invmat3(XtX, XtXi);
  if (ret != 0)
  { 
    noresult = 0;
    lin_multAtB(X, Y, XtY, npb, 3, npb, 1);
    lin_mult(XtXi, XtY, R, 3, 3, 1);
    // calcule l'erreur
    lin_multAtB(R, XtY, RtXtY, 3, 1, 3, 1);
    lin_multAtB(Y, Y, YtY, npb, 1, npb, 1);
    //    printf("cas 3 : erreur %g\n", *YtY - *RtXtY);
    if (*YtY - *RtXtY < err)
    {
      err = *YtY - *RtXtY;
      *a = -1;
      *b = R[1];
      *c = R[2];
      *d = R[0];
    }
  }
  *error = err;

  free(X);
  free(Y);
  free(XtX);
  free(XtXi);
  free(XtY);
  free(RtXtY);
  free(YtY);
  free(R);
  if (noresult) return 0;
  return 1;
} /* lidentifyplane() */

/* ============================================= */
/* ============================================= */
/* ============================================= */
/* TESTS */
/* ============================================= */
/* ============================================= */
/* ============================================= */

#ifdef TESTLIN5
int32_t main()
{
  int32_t ret;  
  double A[4][4] = {2, 3, 0, 0,
                    3, 3, 4, 0,
                    0, 5, 4, 2, 
                    0, 0, 3.4, -1};
  double B[4][4] = {2, 3, 0, 0,
                    3, 3, 4, 0,
                    0, 5, 4, 2, 
                    0, 0, 3.4, -1};
  double x[4];
  double b[4] = {.6, -3, 3, -5.4};
  int32_t pi[4];
  
  lin_printmat((double *)A, 4, 4);
  ret = lin_decomposition_LUP((double *)A, (int32_t *)pi, 4);
  printf("ret = %d\n", ret);
  lin_solveLUP((double *)A, (int32_t *)pi, (double *)b, (double *)x, 4);
  lin_printmat((double *)x, 1, 4);
  
  lin_printmat((double *)B, 4, 4);
  lin_solvetridiag((double *)B, (double *)b, (double *)x, 4);
  lin_printmat((double *)x, 1, 4);
}
#endif

#ifdef TESTLIN4
int32_t main()
{
  double X[4] = {3, 2, 10, 50};
  double D[4] = {1, 1, 3, 5};
  double P[4] = {0.001, 0.001, 0.01, 0.1};
  int32_t ret = lin_trouvemin(X, D, &lin_normevec2, 4, P);
  printf("ret = %d\n", ret);
  lin_printmat((double *)X, 4, 1);
  lin_printmat((double *)D, 4, 1);
}
#endif

#ifdef TESTLIN3
int32_t main()
{
  int32_t ret;  
  double A[4][4] = {10, 2, 3, 4,
                    2, 20, 5, 1,
                    3, 5, 20, 0,
                    4, 1, 0, 10};
  double L[4][4];
  double U[4][4];
  double R[4][4];
  
  lin_printmat((double *)A, 4, 4);
  ret = lin_decomposition_cholesky((double *)A, (double *)L, 4);
  printf("ret = %d\n", ret);
  lin_printmat((double *)L, 4, 4);
  lin_copy((double *)U, (double *)L, 4, 4);
  lin_transpose((double *)U, 4);
  lin_printmat((double *)U, 4, 4);
  lin_mult((double *)L, (double *)U, (double *)R, 4, 4, 4);
  lin_printmat((double *)R, 4, 4);
  lin_inverse_gauss((double *)L, (double *)R, 4);
  lin_printmat((double *)R, 4, 4);
  lin_mult((double *)L, (double *)R, (double *)U, 4, 4, 4);
  lin_printmat((double *)U, 4, 4);

}
#endif

#ifdef TESTLIN2
int32_t main()
{
  int32_t ret;  
  double A[4][4] = {2, 0, 2, .6,
                    3, 3, 4, -2,
                    5, 5, 4, 2, 
                    -1, -2, 3.4, -1};
  double B[4][4] = {2, 0, 2, .6,
                    3, 3, 4, -2,
                    5, 5, 4, 2, 
                    -1, -2, 3.4, -1};
  double C[4][4] = {2, 0, 2, .6,
                    3, 3, 4, -2,
                    5, 5, 4, 2, 
                    -1, -2, 3.4, -1};
  double L[4][4];
  double U[4][4];
  double R[4][4];
  int32_t pi[4];
  double x[4];
  double b[4] = {.6, -3, 3, -5.4};
  double c[4] = {1, 0, -1, 1};
  
  lin_printmat((double *)A, 4, 4);
  ret = lin_decomposition_LUP((double *)A, (int32_t *)pi, 4);
  printf("ret = %d\n", ret);

  lin_printmat((double *)A, 4, 4);
  lin_permutmat((int32_t *)pi, (double *)R, 4);
  lin_printmat((double *)R, 4, 4);

  lin_solveLUP((double *)A, (int32_t *)pi, (double *)b, (double *)x, 4);
  lin_printmat((double *)x, 1, 4);

  lin_mult((double *)B, (double *)x, (double *)b, 4, 4, 1);
  lin_printmat((double *)b, 1, 4);

  ret = lin_inverseLUP((double *)B, (double *)R, 4);
  lin_printmat((double *)R, 4, 4);
  lin_mult((double *)C, (double *)R, (double *)B, 4, 4, 4);
  lin_printmat((double *)B, 4, 4);  
}
#endif

#ifdef TESTLIN1
int32_t main()
{
  double A[2][3] = {1, 2, 3, 4, 5, 6};
  double B[2][3] = {1, 2, 1, 2, 1, 2};
  double C[2][2];
  double D[2][3];
  double v[3] = {2, 5, 8};
  double w[2];
  double E[3][3] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  double S[3][3] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  double P[3][3];
  double L[3][3];
  double U[3][3];
  double R[3][3];
  int32_t pi[3];
  double x[3];
  int32_t ret;  

  lin_printmat((double *)A, 2, 3);
  lin_printmat((double *)B, 3, 2);
  lin_printmat((double *)v, 1, 3);
  lin_printmat((double *)v, 3, 1);
  lin_mult((double *)A, (double *)B, (double *)C, 2, 3, 2);
  lin_printmat((double *)C, 2, 2);
  lin_mult((double *)B, (double *)A, (double *)D, 3, 2, 3);
  lin_printmat((double *)D, 3, 3);
  lin_mult((double *)A, (double *)v, (double *)w, 2, 3, 1);
  lin_printmat((double *)w, 1, 2);

  lin_printmat((double *)E, 3, 3);
  printf("det E = %g\n", lin_det3((double *)E));
  ret = lin_decomposition_LUP((double *)E, (int32_t *)pi, 3);
  printf("ret = %d\n", ret);
  lin_printmat((double *)E, 3, 3);
  lin_permutmat((int32_t *)pi, (double *)P, 3);
  lin_printmat((double *)P, 3, 3);
  lin_LU((double *)E, (double *)L, (double *)U, 3);
  lin_printmat((double *)L, 3, 3);
  lin_printmat((double *)U, 3, 3);
  lin_mult((double *)L, (double *)U, (double *)R, 3, 3, 3);
  lin_printmat((double *)R, 3, 3);
  lin_mult((double *)P, (double *)S, (double *)L, 3, 3, 3);
  lin_printmat((double *)L, 3, 3);
  lin_printmat((double *)v, 1, 3);
  lin_printmat((double *)E, 3, 3);
  lin_solveLUP((double *)E, (int32_t *)pi, (double *)v, (double *)x, 3);
  lin_printmat((double *)x, 1, 3);

}
#endif
