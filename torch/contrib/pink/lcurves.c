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
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <lcurves.h>

/* ====================================================================== */
static void trouve2voisins(int32_t i, int32_t rs, int32_t ps, int32_t N, int32_t connex, uint8_t *F, int32_t *v1, int32_t *v2)
/* ====================================================================== */
// retourne dans v1 et v2 les 2 voisins de i qui sont des points objet (F)
{
#undef F_NAME
#define F_NAME "trouve2voisins"
  int32_t j, k, n = 0;
  switch (connex)
  {
  case 4:
    for (k = 0; k < 8; k += 2)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 8:
    for (k = 0; k < 8; k += 1)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 6:
    for (k = 0; k <= 10; k += 2)
    {
      j = voisin6(i, k, rs, ps, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 18:
    for (k = 0; k < 18; k += 1)
    {
      j = voisin18(i, k, rs, ps, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 26:
    for (k = 0; k < 26; k += 1)
    {
      j = voisin26(i, k, rs, ps, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  default:
    fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
    exit(0);
  } // switch (connex)
  if (n != 2) printf("%s: error n = %d != 2; point %d %d %d\n", F_NAME, n, 
		     i % rs, (i % ps) / rs, i / ps);
  assert(n == 2);
} // trouve2voisins()

/* ====================================================================== */
static int32_t trouve1voisin(int32_t i, int32_t rs, int32_t ps, int32_t N, int32_t connex, uint8_t *F)
/* ====================================================================== */
// retourne un voisin de i qui est un point objet (F), ou -1 si non trouvé
{
#undef F_NAME
#define F_NAME "trouve1voisin"
  int32_t j, k = 0;
  switch (connex)
  {
  case 4:
    for (k = 0; k < 8; k += 2)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 8:
    for (k = 0; k < 8; k += 1)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 6:
    for (k = 0; k <= 10; k += 2)
    {
      j = voisin6(i, k, rs, ps, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 18:
    for (k = 0; k < 18; k += 1)
    {
      j = voisin18(i, k, rs, ps, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 26:
    for (k = 0; k < 26; k += 1)
    {
      j = voisin26(i, k, rs, ps, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  default:
    fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
    exit(0);
  } // switch (connex)
  return -1;
} // trouve1voisin()

/* ====================================================================== */
static int32_t is_end(int32_t x, uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex)
/* ====================================================================== */
{
  switch (connex)
  {
  case 4:
    if (nbvois4(F, x, rs, N) == 1) return 1; else return 0;
  case 8:
    if (nbvois8(F, x, rs, N) == 1) return 1; else return 0;
  case 6:
    if (mctopo3d_nbvoiso6(F, x, rs, ps, N) == 1) return 1; else return 0;
  case 18:
    if (mctopo3d_nbvoiso18(F, x, rs, ps, N) == 1) return 1; else return 0;
  case 26:
    if (mctopo3d_nbvoiso26(F, x, rs, ps, N) == 1) return 1; else return 0;
  default: assert(0);
  }
} // is_end()

/* ========================================== */
int32_t lcurves_extractcurve(
  uint8_t *B,        // entrée/sortie : pointeur base image
  int32_t i,         // entrée : index du point de départ
  int32_t rs,        // entrée : taille rangee
  int32_t N,         // entrée : taille image
  int32_t connex,    // entrée : 4 ou 8
  int32_t ** X,      // sortie : points
  int32_t ** Y,
  int32_t * npoints) // sortie : nombre de points
/* ========================================== */
// extrait de l'image B la courbe débutant au point extrémité i
{
#undef F_NAME
#define F_NAME "lcurves_extractcurve"
  int32_t n = 0;     // compte le nombre de points
  int32_t v1, v2, ii, jj;

  ii = i;
  assert(is_end(ii, B, rs, 1, N, connex)); n++;
  jj = trouve1voisin(ii, rs, 1, N, connex, B); n++;
  while (!is_end(jj, B, rs, 1, N, connex))
  {
    trouve2voisins(jj, rs, 1, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
  }
  *npoints = n;
  *X = (int32_t *)malloc(n * sizeof(int32_t)); assert(*X != NULL); 
  *Y = (int32_t *)malloc(n * sizeof(int32_t)); assert(*Y != NULL); 
  n = 0;
  ii = i;
  (*X)[n] = ii % rs;
  (*Y)[n] = ii / rs;
  jj = trouve1voisin(ii, rs, 1, N, connex, B); n++;
  (*X)[n] = jj % rs;
  (*Y)[n] = jj / rs;
  while (!is_end(jj, B, rs, 1, N, connex))
  {
    trouve2voisins(jj, rs, 1, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
    (*X)[n] = jj % rs;
    (*Y)[n] = jj / rs;
  }  
  return 1;
} // lcurves_extractcurve()

/* ========================================== */
int32_t lcurves_extractcurve3d(
  uint8_t *B,        // entrée/sortie : pointeur base image
  int32_t i,         // entrée : index du point de départ
  int32_t rs,        // entrée : taille rangee
  int32_t ps,        // entrée : taille plan
  int32_t N,         // entrée : taille image
  int32_t connex,    // entrée : 6, 18 ou 26
  int32_t ** X,      // sortie : points
  int32_t ** Y,
  int32_t ** Z,
  int32_t * npoints) // sortie : nombre de points
/* ========================================== */
// extrait de l'image B la courbe débutant au point extrémité i
{
#undef F_NAME
#define F_NAME "lcurves_extractcurve3d"
  int32_t n = 0;     // compte le nombre de points
  int32_t v1, v2, ii, jj;

  ii = i;
  assert(is_end(ii, B, rs, ps, N, connex)); n++;
  jj = trouve1voisin(ii, rs, ps, N, connex, B); n++;
  while (!is_end(jj, B, rs, ps, N, connex))
  {
    trouve2voisins(jj, rs, ps, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
  }
  *npoints = n;
  *X = (int32_t *)malloc(n * sizeof(int32_t)); assert(*X != NULL); 
  *Y = (int32_t *)malloc(n * sizeof(int32_t)); assert(*Y != NULL); 
  *Z = (int32_t *)malloc(n * sizeof(int32_t)); assert(*Z != NULL); 
  n = 0;
  ii = i;
  (*X)[n] = ii % rs;
  (*Y)[n] = (ii % ps) / rs;
  (*Z)[n] = ii / ps;
  jj = trouve1voisin(ii, rs, ps, N, connex, B); n++;
  (*X)[n] = jj % rs;
  (*Y)[n] = (jj % ps) / rs;
  (*Z)[n] = jj / ps;
  while (!is_end(jj, B, rs, ps, N, connex))
  {
    trouve2voisins(jj, rs, ps, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
    (*X)[n] = jj % rs;
    (*Y)[n] = (jj % ps) / rs;
    (*Z)[n] = jj / ps;
  }  
  return 1;
} // lcurves_extractcurve3d()
