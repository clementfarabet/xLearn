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
/* graphic primitives */
/* Michel Couprie - octobre 2002 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <lbresen.h>
#include <ldraw.h>

//#define DEBUG_DL3

/* ==================================== */
static double dist3(double x1, double y1, double z1, double x2, double y2, double z2)
/* ==================================== */
{
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

/* ==================================== */
int32_t ldrawline(struct xvimage * image1, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
/* ==================================== */
/* draws a straight line segment between two points */
#undef F_NAME
#define F_NAME "ldrawline"
{
  int32_t rs, cs;
  uint8_t *F;

  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);

  if ((x1 < 0) || (x1 >= rs) ||  (y1 < 0) || (y1 >= cs) ||
      (x2 < 0) || (x2 >= rs) ||  (y2 < 0) || (y2 >= cs)) 
    return 1; // do nothing

  lbresen(F, rs, x1, y1, x2, y2);

  return 1;
} // ldrawline()

/* ==================================== */
int32_t ldrawline3d(struct xvimage * image1, int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2)
/* ==================================== */
/* draws a 3D straight line segment between two points */
/* NAIVE ALGORITHM - TO IMPROVE !!!! */
#undef F_NAME
#define F_NAME "ldrawline3d"
{
  int32_t i, rs, cs, ds, ps, x, y, z;
  uint8_t *F;

  double len =  dist3(x1, y1, z1, x2, y2, z2);
  int32_t NBSAMPLES = (int32_t)(10 * len);
  if (NBSAMPLES == 0) return 1; // do nothing

#ifdef DEBUG_DL3
  printf("%s: %d %d %d   %d %d %d\n", F_NAME, x1, y1, z1, x2, y2, z2);
#endif

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs * cs;
  F = UCHARDATA(image1);
    
  for (i = 0; i <= NBSAMPLES; i++)
  {
    x = x1 + (i * (x2 - x1)) / NBSAMPLES;
    y = y1 + (i * (y2 - y1)) / NBSAMPLES;
    z = z1 + (i * (z2 - z1)) / NBSAMPLES;
    if (!((x<0) || (x>=rs) || (y<0) || (y>=cs) || (z<0) || (z>=ds)))
      F[z*ps + y*rs + x] = NDG_MAX;
  }

  return 1;
} // ldrawline3d()

/* ==================================== */
int32_t ldrawline2(struct xvimage * image1)
/* ==================================== */
/* draws a straight line segment between the two first points found in image */
#undef F_NAME
#define F_NAME "ldrawline2"
{
  int32_t i, first = -1, last;
  uint8_t *F;
  int32_t rs, cs, N;

  rs = rowsize(image1);
  cs = colsize(image1);
  N = rs * cs;
  F = UCHARDATA(image1);
  
  for (i = 0; i < N; i++)
  {
    if (F[i])
    {
      if (first == -1) first = i;
      else {last = i; break;}
    }
  }
  lbresen(F, rs, first%rs, first/rs, last%rs, last/rs);

  return 1;
} // ldrawline2()

/* =============================================================== */
void ldrawfilledellipse(struct xvimage * image, double R, double S, double T, double U, double V, double Z)
/* =============================================================== */
/*
    \param image (entrée/sortie) : l'image où dessiner l'ellipse 
    \param R,S,T,U,V,Z (entrée) : parametres de l'équation de l'ellipse (Rxx + Syy + 2Txy + 2Ux + 2Vy + Z = 0)
*/
#undef F_NAME
#define F_NAME "ldrawfilledellipse"
{
  int32_t rs, cs, N, i, j;
  uint8_t *F;
  rs = rowsize(image);
  cs = colsize(image);
  N = rs * cs;
  F = UCHARDATA(image);
  
  memset(F, 0, N);
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    if (R*i*i + S*j*j + 2*T*i*j + 2*U*i + 2*V*j + Z <= 0)
      F[j * rs + i] = NDG_MAX;
  }

} // ldrawfilledellipse()

/* ==================================== */
void ldrawcubic1(struct xvimage * image1, double *x, double *y, int32_t nseg, double sx, double sy)
/* ==================================== */
/* draws a cubic line segment */
#undef F_NAME
#define F_NAME "ldrawcubic1"
{
  int32_t i, rs, cs, x1, y1, x2, y2;
  uint8_t *F;
  double X, Y, r = 1.0 / nseg, t = 0.0, t2, t3;

  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);
  
  X = x[0]*sx;
  Y = y[0]*sy;
  x1 = arrondi(X);
  y1 = arrondi(Y);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = sx * (x[0] + t*x[1] +  t2*x[2] +  t3*x[3]);
    Y = sy * (y[0] + t*y[1] +  t2*y[2] +  t3*y[3]);
    x2 = arrondi(X);
    y2 = arrondi(Y);
    ldrawline(image1, x1, y1, x2, y2);
    x1 = x2; y1 = y2;
  }
} // ldrawcubic1()

/* ==================================== */
void ldrawcubic2(struct xvimage * image1, double *x, double *y, int32_t nseg, double tmin, double tmax)
/* ==================================== */
/* draws a cubic line segment */
#undef F_NAME
#define F_NAME "ldrawcubic2"
{
  int32_t i, rs, cs, x1, y1, x2, y2;
  uint8_t *F;
  double X, Y, r = (tmax - tmin) / nseg, t = tmin, t2, t3;

  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);
  
  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
    ldrawline(image1, x1, y1, x2, y2);
    x1 = x2; y1 = y2;
  }
} // ldrawcubic2()

/* ==================================== */
void ldrawcubic3d(
		  struct xvimage * image1,         // image (entree/sortie)  
		  double *x, double *y, double *z, // trois polynomes de degre 3 
		  int32_t nseg,                    // pas de discretisation 
		  double tmin, double tmax)        // bornes pour le coeff. des polynomes
/* ==================================== */
/* draws a cubic line segment */
#undef F_NAME
#define F_NAME "ldrawcubic3d"
{
  int32_t i, rs, cs, ds, x1, y1, z1, x2, y2, z2;
  uint8_t *F;
  double X, Y, Z, r = (tmax - tmin) / nseg, t = tmin, t2, t3;

#ifdef DEBUG_DC3
  printf("%s: %d %d\n", F_NAME, tmin, tmax);
#endif

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  F = UCHARDATA(image1);
  
  t2 = t*t; t3 = t2 * t;
  X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
  Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
  Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
  x1 = arrondi(X);
  y1 = arrondi(Y);
  z1 = arrondi(Z);
  for (i = 0; i < nseg; i++)
  {
    t += r; t2 = t*t; t3 = t2 * t;
    X = x[0] + t*x[1] +  t2*x[2] +  t3*x[3];
    Y = y[0] + t*y[1] +  t2*y[2] +  t3*y[3];
    Z = z[0] + t*z[1] +  t2*z[2] +  t3*z[3];
    x2 = arrondi(X);
    y2 = arrondi(Y);
    z2 = arrondi(Z);
    ldrawline3d(image1, x1, y1, z1, x2, y2, z2);
    x1 = x2; y1 = y2; z1 = z2;
  }
} // ldrawcubic3d()

/* ==================================== */
void ldrawtangents2d(
		     struct xvimage *field,    // image de sortie (le champs de vecteurs)
		     double *x, double *y,     // deux polynomes de degre 3 
		     int32_t nseg,             // pas de discretisation 
		     double tmin, double tmax) // bornes pour le coeff. des polynomes
/* ==================================== */
/* saves, in a vector field structure, the tangent vectors of a cubic curve */
#undef F_NAME
#define F_NAME "ldrawtangents2d"
{
  int32_t N, rs, cs, x1, y1;
  double X, Y, r, t, t2, t3;
  float * F;

  assert(nseg > 0); assert(tmax >= tmin);
  assert(depth(field) == 1);
  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 2);

  r = (tmax - tmin) / nseg;
  rs = rowsize(field);
  cs = colsize(field);
  N = rs * cs;
  F = FLOATDATA(field);

  for (t = tmin; t <= tmax; t += r)
  {
    t2 = t * t; t3 = t2 * t;
    X = x[0] + t*x[1] + t2*x[2] + t3*x[3];
    Y = y[0] + t*y[1] + t2*y[2] + t3*y[3];

    x1 = arrondi(X);
    y1 = arrondi(Y);
		  
    F[y1*rs+x1] = x[1] + 2*t*x[2] + 3*t2*x[3];
    F[y1*rs+x1+N] = y[1] + 2*t*y[2] + 3*t2*y[3];
  }
} // ldrawtangents2d()

/* ==================================== */
void ldrawtangents3d(
		     struct xvimage *field,           // image de sortie (le champs de vecteurs)
		     double *x, double *y, double *z, // trois polynomes de degre 3 
		     int32_t nseg,                    // pas de discretisation 
		     double tmin, double tmax)        // bornes pour le coeff. des polynomes
/* ==================================== */
/* saves, in a vector field structure, the tangent vectors of a cubic curve */
#undef F_NAME
#define F_NAME "ldrawtangents3d"
{
  int32_t N, rs, cs, ds, ps, x1, y1, z1;
  double X, Y, Z, r, t, t2, t3;
  float * F;

  assert(nseg > 0); assert(tmax >= tmin);
  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 3);

  r = (tmax - tmin) / nseg;
  rs = rowsize(field);
  cs = colsize(field);
  ds = depth(field);
  ps = rs * cs;
  N = ps * ds;
  F = FLOATDATA(field);

  for (t = tmin; t <= tmax; t += r)
  {
    t2 = t * t; t3 = t2 * t;
    X = x[0] + t*x[1] + t2*x[2] + t3*x[3];
    Y = y[0] + t*y[1] + t2*y[2] + t3*y[3];
    Z = z[0] + t*z[1] + t2*z[2] + t3*z[3];

    x1 = arrondi(X);
    y1 = arrondi(Y);
    z1 = arrondi(Z);
		  
    F[z1*ps+y1*rs+x1] = x[1] + 2*t*x[2] + 3*t2*x[3];
    F[z1*ps+y1*rs+x1+N] = y[1] + 2*t*y[2] + 3*t2*y[3];
    F[z1*ps+y1*rs+x1+N+N] = z[1] + 2*t*z[2] + 3*t2*z[3];
  }
} // ldrawtangents3d()

#define EPS_DRAWVECT 1e-5

/* ==================================== */
struct xvimage *ldrawfield2d(struct xvimage *field, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield2d"
{
  struct xvimage *image;
  int32_t N, rs, cs, x1, y1;
  double X, Y, t;
  float * F;

  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 2);
  assert(depth(field) == 1);

  rs = rowsize(field);
  cs = colsize(field);
  N = rs * cs;
  F = FLOATDATA(field);

  image = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  assert(image != NULL);

  for (y1 = 0; y1 < cs; y1++)
  for (x1 = 0; x1 < rs; x1++)
  {
    X = F[y1*rs+x1];
    Y = F[y1*rs+x1+N];
    t = sqrt(X*X + Y*Y);
    if (t > EPS_DRAWVECT)
      ldrawline(image, x1, y1, arrondi((x1+(len*X))), arrondi((y1+(len*Y))));
  }
  return image;
} // ldrawfield2d()

/* ==================================== */
void ldrawfield2dlist(int32_t npoints, int32_t *X, int32_t *Y, double *tx, double *ty, struct xvimage *image, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield2dlist"
{
  int32_t i;
  double t, x, y;

  for (i = 0; i < npoints; i++)
  {
    x = tx[i];
    y = ty[i];
    t = sqrt(x*x + y*y);
    if (t > EPS_DRAWVECT)
      ldrawline(image, X[i], Y[i], arrondi((X[i]+(len*x))), arrondi((Y[i]+(len*y))));
  }
} // ldrawfield2dlist()

/* ==================================== */
struct xvimage *ldrawfield3d(struct xvimage *field, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield3d"
{
  struct xvimage *image;
  int32_t N, rs, cs, ds, ps, x1, y1, z1;
  double X, Y, Z, t;
  float * F;

  assert(datatype(field) == VFF_TYP_FLOAT);
  assert(nbands(field) == 3);

  rs = rowsize(field);
  cs = colsize(field);
  ds = depth(field);
  ps = rs * cs;
  N = ps * ds;
  F = FLOATDATA(field);

  image = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  assert(image != NULL);

  for (z1 = 0; z1 < ds; z1++)
  for (y1 = 0; y1 < cs; y1++)
  for (x1 = 0; x1 < rs; x1++)
  {
    X = F[z1*ps+y1*rs+x1];
    Y = F[z1*ps+y1*rs+x1+N];
    Z = F[z1*ps+y1*rs+x1+N+N];
    t = sqrt(X*X + Y*Y + Z*Z);
    if (t > EPS_DRAWVECT)
      ldrawline3d(image, x1, y1, z1, arrondi((x1+(len*X))), arrondi((y1+(len*Y))), arrondi((z1+(len*Z))));
  }
  return image;
} // ldrawfield3d()

/* ==================================== */
void ldrawfield3dlist(int32_t npoints, int32_t *X, int32_t *Y, int32_t *Z, double *tx, double *ty, double *tz, struct xvimage *image, double len)
/* ==================================== */
/* draws lines in output image that represent vectors in input field */
#undef F_NAME
#define F_NAME "ldrawfield3dlist"
{
  int32_t i;
  double t, x, y, z;

  for (i = 0; i < npoints; i++)
  {
    x = tx[i];
    y = ty[i];
    z = tz[i];
    t = sqrt(x*x + y*y + z*z);
    //printf("i=%d, t=%g\n", i, t);
    if (t > EPS_DRAWVECT)
    {
      //printf("drawing line %d %d %d - %d %d %d\n", X[i], Y[i], Z[i], arrondi((X[i]+(len*x))), arrondi((Y[i]+(len*y))), arrondi((Z[i]+(len*z))));
      ldrawline3d(image, X[i], Y[i], Z[i], arrondi((X[i]+(len*x))), arrondi((Y[i]+(len*y))), arrondi((Z[i]+(len*z))));
    }
  }
} // ldrawfield3dlist()

/* ==================================== */
int32_t ldrawball(struct xvimage * image1, double r, double xc, double yc, double zc)
/* ==================================== */
/* draws a euclidean ball */
#undef F_NAME
#define F_NAME "ldrawball"
{
  int32_t i, j, k, rs, cs, ds, ps;
  uint8_t *F;
  double r2, x, y, z;
  
  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs*cs;
  F = UCHARDATA(image1);
  
  r2 = r * r;
  for (k = 0; k < ds; k++)
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    x = xc - i; 
    y = yc - j; 
    z = zc - k; 
    if (x * x + y * y + z * z <= r2)
      F[k * ps + j * rs + i] = NDG_MAX;
  }

  return 1;
} // ldrawball()

/* ==================================== */
void ldrawdisc(struct xvimage * image1, double r, double xc, double yc)
/* ==================================== */
/* draws a euclidean disc */
#undef F_NAME
#define F_NAME "ldrawdisc"
{
  int32_t i, j, rs, cs;
  uint8_t *F;
  double r2, x, y;
  
  rs = rowsize(image1);
  cs = colsize(image1);
  F = UCHARDATA(image1);
  
  r2 = r * r;
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    x = xc - i; 
    y = yc - j; 
    if (x * x + y * y <= r2)
      F[j * rs + i] = NDG_MAX;
  }
} // ldrawdisc()

/* ==================================== */
int32_t ldrawtorus(struct xvimage * image1, double c, double a, double xc, double yc, double zc)
/* ==================================== */
/* draws a torus of equation (c-sqrt((xc-x)^2+(yc-y)^2))^2+(zc-z)^2 <= a^2 */
#undef F_NAME
#define F_NAME "ldrawtorus"
{
  int32_t i, j, k, rs, cs, ds, ps;
  uint8_t *F;
  double t, a2, x, y, z;
  
  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  ps = rs*cs;
  F = UCHARDATA(image1);
  
  a2 = a * a;
  for (k = 0; k < ds; k++)
  for (j = 0; j < cs; j++)
  for (i = 0; i < rs; i++)
  {
    x = xc - i; 
    y = yc - j; 
    z = zc - k;
    t = c - sqrt(x*x + y*y);
    if (t*t + z*z <= a2)
      F[k * ps + j * rs + i] = NDG_MAX;
  }

  return 1;
} // ldrawtorus()
