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
Librairie mcgeo : 

fonctions pour les transformations geometriques

Michel Couprie, mars 1997 
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mcutil.h>
#include <mcgeo.h>

/*
#define TESTGEO
#define DEBUG
*/

/* ==================================================================== */
/* ==================================================================== */
/* fonctions sur les points, droites etc */
/* ==================================================================== */
/* ==================================================================== */

/* ==================================== */
double detpq_qr(mcgeo_point p, mcgeo_point q, mcgeo_point r)
/* ==================================== */
{
  double pqx, pqy, qrx, qry;
  pqx = q.x - p.x; pqy = q.y - p.y;
  qrx = r.x - q.x; qry = r.y - q.y;
  return pqx * qry - pqy * qrx;
} /* detpq_pr() */

/* ==================================== */
int32_t estsitue(mcgeo_point p, mcgeo_point q, mcgeo_point r)
/* ==================================== */
/* prend la valeur 1, 0 ou -1 selon que p est situe a gauche, sur ou a droite */
/* de la droite orientee qr */
{
  double d = detpq_qr(p, q, r);
  if (d > 0.0) return 1;
  if (d < 0.0) return -1;
  return 0;
} /* estsitue() */

/* ==================================== */
int32_t estagauche(mcgeo_point p, mcgeo_point q, mcgeo_point r)
/* ==================================== */
{
  return (estsitue(p, q, r) == 1);
} /* estagauche() */

/* ==================================== */
int32_t estadroite(mcgeo_point p, mcgeo_point q, mcgeo_point r)
/* ==================================== */
{
  return (estsitue(p, q, r) == -1);
} /* estadtroite() */

/* ==================================== */
double carrenorme(mcgeo_point p)
/* ==================================== */
{
  return p.x * p.x + p.y * p.y;
} /* carrenorme() */

/* ==================================== */
double carredistance(mcgeo_point p, mcgeo_point q)
/* ==================================== */
{
  double dx, dy; 
  dx = p.x - q.x; dy = p.y - q.y; 
  return dx * dx + dy * dy;
} /* carredistance() */

/* ==================================== */
double dist2(double x1, double y1, double x2, double y2)
/* ==================================== */
{
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/* ==================================== */
double dist3(double x1, double y1, double z1, double x2, double y2, double z2)
/* ==================================== */
{
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

/* ==================================== */
double cosangle(mcgeo_point p, mcgeo_point q, mcgeo_point r)
/* ==================================== */
/* calcule le cos de l'angle entre les vecteurs pq et pr */
{
  double pqx, pqy, prx, pry;
  pqx = q.x - p.x; pqy = q.y - p.y;
  prx = r.x - p.x; pry = r.y - p.y;
  return (pqx * prx + pqy * pry) / sqrt((pqx*pqx + pqy*pqy) * (prx*prx + pry*pry));
} /* cosangle() */

/* ==================================== */
static double direction(mcgeo_point p1, mcgeo_point p2, mcgeo_point p3)
/* ==================================== */
/* calcule le produit en croix (p3-p1) x (p2-p1) */
{
  return ((p3.x-p1.x) * (p2.y-p1.y)) - ((p2.x-p1.x) * (p3.y-p1.y)) ;
} /* direction() */

/* ==================================== */
double sursegment(mcgeo_point p1, mcgeo_point p2, mcgeo_point p3)
/* ==================================== */
/* calcule le produit en croix (p3-p1) x (p2-p1) */
{
  return 
    (mcmin(p1.x,p2.x) <= p3.x) &&
    (p3.x <= mcmax(p1.x,p2.x)) &&
    (mcmin(p1.y,p2.y) <= p3.y) &&
    (p3.y <= mcmax(p1.y,p2.y));
} /* sursegment() */

/* ==================================== */
int32_t sontsecants(mcgeo_point p1, mcgeo_point p2, mcgeo_point p3, mcgeo_point p4)
/* ==================================== */
/* retourne 1 si les segments [p1,p2] et [p3,p4] sont secants, 0 sinon */
/* d'apres Cormen & al., "Introduction a l'algorithmique" 2e. Ed. p. 905 */
{
  double d1 = direction(p3, p4, p1);
  double d2 = direction(p3, p4, p2);
  double d3 = direction(p1, p2, p3);
  double d4 = direction(p1, p2, p4);

  if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
     ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
    return 1;
  if ((d1 == 0) && sursegment(p3, p4, p1)) return 1;
  if ((d2 == 0) && sursegment(p3, p4, p2)) return 1;
  if ((d3 == 0) && sursegment(p1, p2, p3)) return 1;
  if ((d4 == 0) && sursegment(p1, p2, p4)) return 1;
  return 0;
} /* sontsecants() */

/* ==================================== */
double distpointdroite(mcgeo_point p, droite d)
/* ==================================== */
/* 
   Calcule la distance d'un point a une droite.
   La droite d a pour coefficients (a, b, c) 
   La perpendiculaire a d passant par p est: (-b, a, b xp - a yp) 
   On calcule le point d'intersection m et la distance pm.
*/
{
  double a, b, c, cc, x, y, det, dx, dy;
  a = d.a; b = d.b; c = d.c;
  det = a * a + b * b;
  cc = b * p.x - a * p.y;
  x = (b * cc - c * a) / det;
  y = (- b * c - cc * a) / det;
  dx = x - p.x;
  dy = y - p.y;
  return sqrt(dx * dx + dy * dy);
} /* distpointdroite() */

/* ==================================== */
double distpointsegment(mcgeo_point p, segment s)
/* ==================================== */
/* 
   Calcule la distance d'un point a un segment (voir distpointdroite). 
*/
{
  double a, b, c, cc, x, y, det, dx, dy, d1, d2, x1, x2;
  a = s.a; b = s.b; c = s.c;
  det = a * a + b * b;
  cc = b * p.x - a * p.y;
  x = (b * cc - c * a) / det;
  y = (- b * c - cc * a) / det;

  /* verifie si le point d'intersection appartient au segment */
  x1 = mcmin(s.x1,s.x2); x2 = mcmax(s.x1,s.x2);
  if ((x >= x1) && (x <= x2))
  {
    dx = x - p.x;
    dy = y - p.y;
    return sqrt(dx * dx + dy * dy);
  }
  /* sinon on calcule les distances entre p et s.x1, s.x2 et on prend le min */
  dx = s.x1 - p.x;
  dy = s.y1 - p.y;
  d1 = sqrt(dx * dx + dy * dy);
  dx = s.x2 - p.x;
  dy = s.y2 - p.y;
  d2 = sqrt(dx * dx + dy * dy);
  return mcmin(d1,d2);
} /* distpointsegment() */

/* ==================================== */
double distpointcercle(mcgeo_point p, cercle c)
/* ==================================== */
/* 
   Calcule la distance d'un point a un cercle.
*/
{
  double r = c.r, x = p.x - c.x0, y = p.y - c.y0;
  double d = sqrt(x*x + y*y);
  if (d > r) return d-r;
  return r-d;
} /* distpointcercle() */

/* ==================================== */
double distpointrectangle(mcgeo_point p, rectangle *r)
/* ==================================== */
/* 
   Calcule la distance d'un point a un rectangle.
*/
{
  double d, dd;

  d =  distpointsegment(p, r->s1);
  dd = distpointsegment(p, r->s2); d = mcmin(d,dd);
  dd = distpointsegment(p, r->s3); d = mcmin(d,dd);
  dd = distpointsegment(p, r->s4); return mcmin(d,dd);
} /* distpointrectangle() */

/* ==================================== */
int32_t initrectangle(rectangle *r)
/* ==================================== */
{
  double l = r->w, L = r->h, theta = r->angle, x = r->centre.x, y = r->centre.y;
  double d, alpha;

  if (L < MCGEO_EPSILON) return 0;
  d = sqrt(l*l + L*L) / 2.0;
  alpha = atan(l/L);

  r->s1.x1 = x + d * cos(theta + alpha);  r->s1.y1 = y + d * sin(theta + alpha);
  r->s2.x1 = x + d * cos(theta - alpha);  r->s2.y1 = y + d * sin(theta - alpha);
  r->s3.x1 = x - d * cos(theta + alpha);  r->s3.y1 = y - d * sin(theta + alpha);
  r->s4.x1 = x - d * cos(theta - alpha);  r->s4.y1 = y - d * sin(theta - alpha);
  r->s1.x2 = r->s2.x1;  r->s1.y2 = r->s2.y1;
  r->s2.x2 = r->s3.x1;  r->s2.y2 = r->s3.y1;
  r->s3.x2 = r->s4.x1;  r->s3.y2 = r->s4.y1;
  r->s4.x2 = r->s1.x1;  r->s4.y2 = r->s1.y1;

  if (equdroite(r->s1.x1, r->s1.y1, r->s1.x2, r->s1.y2, &(r->s1.a), &(r->s1.b), &(r->s1.c)) == 0) return 0;
  if (equdroite(r->s2.x1, r->s2.y1, r->s2.x2, r->s2.y2, &(r->s2.a), &(r->s2.b), &(r->s2.c)) == 0) return 0;
  if (equdroite(r->s3.x1, r->s3.y1, r->s3.x2, r->s3.y2, &(r->s3.a), &(r->s3.b), &(r->s3.c)) == 0) return 0;
  if (equdroite(r->s4.x1, r->s4.y1, r->s4.x2, r->s4.y2, &(r->s4.a), &(r->s4.b), &(r->s4.c)) == 0) return 0;

  return 1;
} /* initrectangle() */

/* ==================================================================== */
/* ==================================================================== */
/* fonctions pour le 3d */
/* ==================================================================== */
/* ==================================================================== */

/* ==================================== */
int32_t point3cmp (point3 p1, point3 p2)
/* ==================================== */
/* 
   Retourne 0 si p1 == p2, -1 si p1 < p2 et +1 si p1 > p2
   (ordre lexicographique)
*/
#undef F_NAME
#define F_NAME "point3cmp"
{
  if (p1.x > p2.x) return 1; 
  if (p1.x < p2.x) return -1;
  if (p1.y > p2.y) return 1; 
  if (p1.y < p2.y) return -1;
  if (p1.z > p2.z) return 1; 
  if (p1.z < p2.z) return -1;
  return 0;
} /* point3cmp() */

/* ==================================== */
double distpointdroite3(point3 p, point3 p1, point3 p2)
/* ==================================== */
/* 
   Calcule la distance d'un point a une droite.
   La droite d est specifiee par les deux points p1, p2
   Soit b = p2 - p1, les points x de la droite d'expriment 
   comme x = p1 + s.b, avec s dans R
   On calcule le projete orthogonal f de p sur la droite, 
   tout d'abord s = (p - p1).b / b.b et f = p1 + s.b
   Le resultat cherche est la distance entre f et p.
*/
#undef F_NAME
#define F_NAME "distpointdroite3"
{
  double dx, dy, dz, bx, by, bz, x, y, z, n, nb, s;
  bx = p2.x - p1.x; by = p2.y - p1.y; bz = p2.z - p1.z;
  x = p.x - p1.x; y = p.y - p1.y; z = p.z - p1.z;
  nb = bx * bx + by * by + bz * bz;
  if (nb < MCGEO_EPSILON)
  {
    fprintf(stderr, "warning: %s: failed\n", F_NAME);
    return 0.0;
  }
  n = bx * x + by * y + bz * z;
  s = n / nb;
  dx = p1.x + s * bx - p.x;
  dy = p1.y + s * by - p.y;
  dz = p1.z + s * bz - p.z;
  return sqrt(dx * dx + dy * dy + dz * dz);
} /* distpointdroite3() */

/* ==================================================================== */
/* ==================================================================== */
/* fonctions sur les matrices, vecteurs, systemes */
/* ==================================================================== */
/* ==================================================================== */

/* ==================================== */
void printmat(
  double * mat,
  int32_t n,
  int32_t m)
/* ==================================== */
{
  int32_t i, j;
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < m; j++)  
      printf("%6g   ", mat[i * m + j]);
    printf("\n");
  }          
  printf("\n");
}

/* ==================================== */
void printvec(
  double * v,
  int32_t n)
/* ==================================== */
{
  int32_t i;
  for (i = 0; i < n; i++)
  {
    printf("%6g", v[i]);
    printf("\n");
  }          
  printf("\n");
}

/* ==================================== */
double det2(
  mat22 m)
/* ==================================== */
{
  return m[0][0]*m[1][1] - m[0][1]*m[1][0];
}

/* ==================================== */
double det3(
  mat33 m)
/* ==================================== */
{
  return m[0][0]*m[1][1]*m[2][2] + m[0][1]*m[1][2]*m[2][0] + m[1][0]*m[2][1]*m[0][2] 
       - m[0][2]*m[1][1]*m[2][0] - m[0][1]*m[1][0]*m[2][2] - m[1][2]*m[2][1]*m[0][0];
}

/* ==================================== */
int32_t solsyst2(
  mat22 m,
  vec2 b,
  vec2 sol)
/* ==================================== */
{
  mat22 m1;
  double d, d1, d2;
  int32_t i;
  
  d = det2(m);
  if (((d >= 0) && (d < MCGEO_EPSILON)) || ((d <= 0) && (-d < MCGEO_EPSILON))) return 0;

  for (i = 0; i < 2; i++) m1[i][0] = b[i]; 
  for (i = 0; i < 2; i++) m1[i][1] = m[i][1];
  d1 = det2(m1);

  for (i = 0; i < 2; i++) m1[i][1] = b[i]; 
  for (i = 0; i < 2; i++) m1[i][0] = m[i][0];
  d2 = det2(m1);

  sol[0] = d1 / d; 
  sol[1] = d2 / d; 
  return 1;
} // solsyst2()
  
/* ==================================== */
int32_t solsyst3(
  mat33 m,
  vec3 b,
  vec3 sol)
/* ==================================== */
{
  mat33 m1;
  double d, d1, d2, d3;
  int32_t i, j;
  
  d = det3(m);
  if (((d >= 0) && (d < MCGEO_EPSILON)) || ((d <= 0) && (-d < MCGEO_EPSILON))) return 0;

  for (i = 0; i < 3; i++) m1[i][0] = b[i]; 
  for (i = 0; i < 3; i++)
    for (j = 1; j < 3; j++)  
      m1[i][j] = m[i][j];
  d1 = det3(m1);

  for (i = 0; i < 3; i++) m1[i][1] = b[i]; 
  for (i = 0; i < 3; i++) m1[i][0] = m[i][0];
  d2 = det3(m1);

  for (i = 0; i < 3; i++) m1[i][2] = b[i]; 
  for (i = 0; i < 3; i++) m1[i][1] = m[i][1];
  d3 = det3(m1);

  sol[0] = d1 / d; 
  sol[1] = d2 / d; 
  sol[2] = d3 / d; 
  return 1;
} // solsyst3()

/* ==================================== */
int32_t invmat2(
  mat22 ma,
  mat22 mr)
/* ==================================== */
{
  double det = det2( ma );
  if ( fabs( det ) < MCGEO_EPSILON ) return 0;
  mr[0][0] =   ma[1][1] / det;
  mr[1][0] = - ma[1][0] / det;
  mr[0][1] = - ma[0][1] / det;
  mr[1][1] =   ma[0][0] / det;
  return 1;
} // invmat2()

/* ==================================== */
int32_t invmat3(
  mat33 ma,
  mat33 mr)
/* ==================================== */
{
  double det = det3( ma );
  if ( fabs( det ) < MCGEO_EPSILON ) return 0;
  mr[0][0] =  ( ma[1][1]*ma[2][2] - ma[1][2]*ma[2][1] ) / det;
  mr[0][1] = -( ma[0][1]*ma[2][2] - ma[2][1]*ma[0][2] ) / det;
  mr[0][2] =  ( ma[0][1]*ma[1][2] - ma[1][1]*ma[0][2] ) / det;
  mr[1][0] = -( ma[1][0]*ma[2][2] - ma[1][2]*ma[2][0] ) / det;
  mr[1][1] =  ( ma[0][0]*ma[2][2] - ma[2][0]*ma[0][2] ) / det;
  mr[1][2] = -( ma[0][0]*ma[1][2] - ma[1][0]*ma[0][2] ) / det;
  mr[2][0] =  ( ma[1][0]*ma[2][1] - ma[2][0]*ma[1][1] ) / det;
  mr[2][1] = -( ma[0][0]*ma[2][1] - ma[2][0]*ma[0][1] ) / det;
  mr[2][2] =  ( ma[0][0]*ma[1][1] - ma[0][1]*ma[1][0] ) / det;
  return 1;
} // invmat3()
  
/* ==================================== */
void multmat2vec2(
  mat22 m,
  vec2 v,
  vec2 sol)
/* ==================================== */
{
  double t;
  int32_t i, j;
  for (i = 0; i < 2; i++)
  {
    t = 0.0;
    for (j = 0; j < 2; j++)  
      t += m[i][j] * v[j];
    sol[i] = t;    
  }          
} // multmat2vec2()

/* ==================================== */
void multmat3vec3(
  mat33 m,
  vec3 v,
  vec3 sol)
/* ==================================== */
{
  double t;
  int32_t i, j;
  for (i = 0; i < 3; i++)
  {
    t = 0.0;
    for (j = 0; j < 3; j++)  
      t += m[i][j] * v[j];
    sol[i] = t;    
  }          
} // multmat3vec3()
  
/* ==================================== */
void multmat2mat2(
  mat22 m1,
  mat22 m2,
  mat22 mr)
/* ==================================== */
{
  double t;
  int32_t i, j, k;
  for (i = 0; i < 2; i++)
    for (j = 0; j < 2; j++)  
    {
      t = 0.0;
      for (k = 0; k < 2; k++)  
        t += m1[i][k] * m2[k][j];
      mr[i][j] = t;    
    }
} // multmat2mat2()
  
/* ==================================== */
void multmat3mat3(
  mat33 m1,
  mat33 m2,
  mat33 mr)
/* ==================================== */
{
  double t;
  int32_t i, j, k;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)  
    {
      t = 0.0;
      for (k = 0; k < 3; k++)  
        t += m1[i][k] * m2[k][j];
      mr[i][j] = t;    
    }
} // multmat3mat3()

/* ==================================== */
int32_t equdroite(
  double x1,
  double y1,
  double x2,
  double y2,
  double * a,
  double * b,
  double * c)
/* ==================================== */
{
  if (fabs(y2 - y1) > fabs(x2 - x1))
  {
    *a = 1.0; 
    *b = - (x2 - x1) / (y2 - y1);
    *c = - (*b * y1 + x1);
    return 1;
  }
  else if (x2 != x1)
  {
    *b = 1.0; 
    *a = - (y2 - y1) / (x2 - x1);
    *c = - (*a * x1 + y1);
    return 1;
  }
  else return 0;
}

/* ==================================== */
int32_t inittriangle(triangle *t)
/* ==================================== */
{
  double tmp;

  if (equdroite(t->xa, t->ya, t->xc, t->yc, &(t->a1), &(t->b1), &(t->c1)) == 0) return 0;
  if (equdroite(t->xb, t->yb, t->xc, t->yc, &(t->a2), &(t->b2), &(t->c2)) == 0) return 0;
  if (equdroite(t->xa, t->ya, t->xb, t->yb, &(t->a3), &(t->b3), &(t->c3)) == 0) return 0;

  t->s1 = t->a1 * t->xb + t->b1 * t->yb + t->c1;
  t->s2 = t->a2 * t->xa + t->b2 * t->ya + t->c2;
  t->s3 = t->a3 * t->xc + t->b3 * t->yc + t->c3;

  tmp = t->xa; if (t->xb < tmp) tmp = t->xb; if (t->xc < tmp) tmp = t->xc; t->xmin = tmp;
  tmp = t->ya; if (t->yb < tmp) tmp = t->yb; if (t->yc < tmp) tmp = t->yc; t->ymin = tmp;
  tmp = t->xa; if (t->xb > tmp) tmp = t->xb; if (t->xc > tmp) tmp = t->xc; t->xmax = tmp;
  tmp = t->ya; if (t->yb > tmp) tmp = t->yb; if (t->yc > tmp) tmp = t->yc; t->ymax = tmp;

  return 1;
}

/* ==================================== */
int32_t danstriangle(
  triangle *t,
  double x,
  double y)
/* ==================================== */
/* retourne 1 si le point (x,y) est dans le triangle t1, 0 sinon */
{
  if ((t->a1 * x + t->b1 * y + t->c1) * t->s1 < 0.0) return 0;
  if ((t->a2 * x + t->b2 * y + t->c2) * t->s2 < 0.0) return 0;
  if ((t->a3 * x + t->b3 * y + t->c3) * t->s3 < 0.0) return 0;
  return 1;
}

/* ==================================== */
void printtriangle(
  triangle *t)
/* ==================================== */
{
  printf("xa = %g, ya = %g, xb = %g, yb = %g, xc = %g, yc = %g\n",
          t->xa , t->ya, t->xb, t->yb, t->xc, t->yc);
  printf("a1 = %g, a2 = %g, a3 = %g\nb1 = %g, b2 = %g, b3 = %g\nc1 = %g, c2 = %g, c3 = %g\n",
          t->a1, t->a2, t->a3, t->b1, t->b2, t->b3, t->c1, t->c2, t->c3);
  printf("s1 = %g, s2 = %g, s3 = %g\n",
          t->s1, t->s2, t->s3);            
  printf("xmin = %g, ymin = %g, xmax = %g, ymax = %g\n\n",
          t->xmin, t->ymin, t->xmax, t->ymax);   
}

/* ==================================== */
int32_t dansdisque(
  double x,
  double y,
  double xc,
  double yc,
  double r)
/* ==================================== */
/* retourne 1 si le point (x,y) est dans le disque de centre (xc,yc) et de rayon r, 0 sinon */
{
  if (sqrt(((x - xc) * (x - xc)) + ((y - yc) * (y - yc))) <= r)
       return 1;
  else return 0;
}

/* ===================================================================
Identification de ((x0, y0), r), centre et rayon du cercle minimisant
la somme des distances algebriques de n points (xi, yi) au cercle.
*/
/* ==================================== */
int32_t identcercle(
  int32_t n,
  double *x,
  double *y,
  double *x0,
  double *y0,
  double *r)
/* ==================================== */
{
  int32_t i;
  double X[11];
  double t;
  mat33 m;
  vec3 b;
  vec3 sol;

  for (i = 1; i <= 10; i++)
    X[i] = 0.0;

  for (i = 0; i < n; i++)
  {
    t = x[i] * x[i] + y[i] * y[i];
    X[1] += t * t;
    X[2] += t * x[i];
    X[3] += t * y[i];
    X[4] += t;
    X[5] += x[i] * x[i];
    X[6] += y[i] * y[i];
    X[8] += x[i] * y[i];
    X[9] += x[i];
    X[10] += y[i];
  } /* for i */

  b[0] = - X[2];
  b[1] = - X[3];
  b[2] = - X[4];
  
  m[0][0] = X[5];
  m[0][1] = X[8];
  m[0][2] = X[9];
  m[1][0] = X[8];
  m[1][1] = X[6];
  m[1][2] = X[10];
  m[2][0] = X[9];
  m[2][1] = X[10];
  m[2][2] = n;
  
  if (solsyst3(m, b, sol) == 0)
  {
    printf("identcercle: pas de solution au systeme\n");
    return 0;
  }

  *x0 = -sol[0] / 2;
  *y0 = -sol[1] / 2;
  *r = *x0 * *x0 + *y0 * *y0 - sol[2];
  if (*r < 0)
  {
    printf("identcercle: le carre du rayon est < 0\n");
    return 0;
  }
  *r = sqrt(*r);

  return 1;
} /* identcercle() */

/* ==================================== */
int32_t writescene(scene *scn, char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writescene"
{
  FILE *fd = NULL;
  int32_t i, j, nobj;

  fd = fopen(filename, "w");
  if (!fd)
  {
    fprintf(stderr, "%s: bad file name: %s\n", F_NAME, filename);
    return 0;
  }

  nobj = scn->nobj;
  fprintf(fd, "3Dscene %d\n", nobj);

  for (i = 0; i < nobj; i++)
  {
    object *obj = scn->tabobj[i];
    switch (obj->objtype)
    {
    case OBJTYPE_LINE: fprintf(fd, "line %d\n", obj->npoints); break;
    case OBJTYPE_CLOSEDLINE: fprintf(fd, "closedline %d\n", obj->npoints); break;
    case OBJTYPE_SPLINE: fprintf(fd, "spline %d\n", obj->npoints); break;
    case OBJTYPE_CLOSEDSPLINE: fprintf(fd, "closedspline %d\n", obj->npoints); break;
    default:
      fprintf(stderr, "%s: bad object typs: %d\n", F_NAME, obj->objtype);
      return 0;
    } // switch (obj->objtype)

    for (j = 0; j < obj->npoints; j++)
      fprintf(fd, "%lf %lf %lf\n", obj->points[j].x, obj->points[j].y, obj->points[j].z);
  } // for (i = 0; i < nobj; i++)
  fclose(fd);
  return 1;
} // writescene()

/* =============================================================== */
static object * loadline(FILE *fd)
/* =============================================================== */
#undef F_NAME
#define F_NAME "loadline"
{
  int32_t npoints, j;
  double x, y, z;
  object *obj;

  fscanf(fd, "%d", &npoints);
  obj = (object *)calloc(1,sizeof(object));
  if (obj == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  obj->objtype = OBJTYPE_LINE;
  obj->npoints = npoints;
  obj->points = (point3 *)calloc(1,npoints * sizeof(point3));
  if (obj->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    fscanf(fd, "%lf%lf%lf", &x, &y, &z);
    obj->points[j].x = x;
    obj->points[j].y = y;
    obj->points[j].z = z;
  }
  return obj;
} // loadline()

/* =============================================================== */
static object * loadclosedline(FILE *fd)
/* =============================================================== */
#undef F_NAME
#define F_NAME "loadclosedline"
{
  int32_t npoints, j;
  double x, y, z;
  object *obj;

  fscanf(fd, "%d", &npoints);
  obj = (object *)calloc(1,sizeof(object));
  if (obj == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  obj->objtype = OBJTYPE_CLOSEDLINE;
  obj->npoints = npoints;
  obj->points = (point3 *)calloc(1,npoints * sizeof(point3));
  if (obj->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    fscanf(fd, "%lf%lf%lf", &x, &y, &z);
    obj->points[j].x = x;
    obj->points[j].y = y;
    obj->points[j].z = z;
  }
  return obj;
} // loadclosedline()

/* =============================================================== */
static object * loadspline(FILE *fd)
/* =============================================================== */
#undef F_NAME
#define F_NAME "loadspline"
{
  int32_t npoints, j;
  double x, y, z;
  object *obj;

  fscanf(fd, "%d", &npoints);
  obj = (object *)calloc(1,sizeof(object));
  if (obj == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  obj->objtype = OBJTYPE_SPLINE;
  obj->npoints = npoints;
  obj->points = (point3 *)calloc(1,npoints * sizeof(point3));
  if (obj->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    fscanf(fd, "%lf%lf%lf", &x, &y, &z);
    obj->points[j].x = x;
    obj->points[j].y = y;
    obj->points[j].z = z;
  }
  return obj;
} // loadspline()

/* =============================================================== */
static object * loadclosedspline(FILE *fd)
/* =============================================================== */
#undef F_NAME
#define F_NAME "loadclosedspline"
{
  int32_t npoints, j;
  double x, y, z;
  object *obj;

  fscanf(fd, "%d", &npoints);
  obj = (object *)calloc(1,sizeof(object));
  if (obj == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  obj->objtype = OBJTYPE_CLOSEDSPLINE;
  obj->npoints = npoints;
  obj->points = (point3 *)calloc(1,npoints * sizeof(point3));
  if (obj->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    fscanf(fd, "%lf%lf%lf", &x, &y, &z);
    obj->points[j].x = x;
    obj->points[j].y = y;
    obj->points[j].z = z;
  }
  return obj;
} // loadclosedspline()

/* ==================================== */
scene * readscene(char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "readscene"
{
  FILE *fd = NULL;
  int32_t j, nobj, ret;
  scene *scn;
  char buf[1024];

  fd = fopen(filename, "r");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return NULL;
  }

  ret = fscanf(fd, "%s", buf);
  if (strncmp(buf, "3Dscene", 7) != 0)
  {
    fprintf(stderr, "%s : bad file format 1 : %s\n", F_NAME, buf);
    return NULL;
  }

  ret = fscanf(fd, "%d", &nobj);
  if (ret != 1)
  {
    fprintf(stderr, "%s : bad file format 2\n", F_NAME);
    return NULL;
  }

  scn = (scene *)calloc(1,sizeof(scene));
  if (scn == NULL)
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return NULL;
  }

  scn->nobj = nobj;
  scn->tabobj = (object **)calloc(1,nobj * sizeof(object *));
  if (scn->tabobj == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return NULL;
  }

  for (j = 0; j < nobj; j++)
  {
    ret = fscanf(fd, "%s", buf);
    if (ret != 1)
    {
      fprintf(stderr, "%s : bad file format 3\n", F_NAME);
      return NULL;
    }

    if (strncmp(buf, "line", 4) == 0) scn->tabobj[j] = loadline(fd);
    else if (strncmp(buf, "closedline", 10) == 0) scn->tabobj[j] = loadclosedline(fd);
    else if (strncmp(buf, "spline", 6) == 0) scn->tabobj[j] = loadspline(fd);
    else if (strncmp(buf, "closedspline", 12) == 0) scn->tabobj[j] = loadclosedspline(fd);
    else
    {
      fprintf(stderr, "%s : bad object type : %s\n", F_NAME, buf);
      return NULL;
    }
  } // for (j = 0; j < nobj; j++)
  fclose(fd);
  return scn;
} // readscene()

/* =============================================================== */
object * copyline(object *o)
/* =============================================================== */
#undef F_NAME
#define F_NAME "copyline"
{
  int32_t npoints, j;
  object *oc;

  npoints = o->npoints;
  oc = (object *)calloc(1,sizeof(object));
  if (oc == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  oc->objtype = OBJTYPE_LINE;
  oc->npoints = npoints;
  oc->points = (point3 *)calloc(1,npoints * sizeof(point3));

  if (oc->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    oc->points[j].x = o->points[j].x;
    oc->points[j].y = o->points[j].y;
    oc->points[j].z = o->points[j].z;
  }

  return oc;
} // copyline()

/* =============================================================== */
object * copyclosedline(object *o)
/* =============================================================== */
#undef F_NAME
#define F_NAME "copyclosedline"
{
  int32_t npoints, j;
  object *oc;

  npoints = o->npoints;
  oc = (object *)calloc(1,sizeof(object));
  if (oc == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  oc->objtype = OBJTYPE_CLOSEDLINE;
  oc->npoints = npoints;
  oc->points = (point3 *)calloc(1,npoints * sizeof(point3));
  if (oc->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    oc->points[j].x = o->points[j].x;
    oc->points[j].y = o->points[j].y;
    oc->points[j].z = o->points[j].z;
  }
  return oc;
} // copyclosedline()

/* =============================================================== */
object * copyspline(object *o)
/* =============================================================== */
#undef F_NAME
#define F_NAME "copyspline"
{
  int32_t npoints, j;
  object *oc;

  npoints = o->npoints;
  oc = (object *)calloc(1,sizeof(object));
  if (oc == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  oc->objtype = OBJTYPE_SPLINE;
  oc->npoints = npoints;
  oc->points = (point3 *)calloc(1,npoints * sizeof(point3));

  if (oc->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    oc->points[j].x = o->points[j].x;
    oc->points[j].y = o->points[j].y;
    oc->points[j].z = o->points[j].z;
  }

  return oc;
} // copyspline()

/* =============================================================== */
object * copyclosedspline(object *o)
/* =============================================================== */
#undef F_NAME
#define F_NAME "copyclosedspline"
{
  int32_t npoints, j;
  object *oc;

  npoints = o->npoints;
  oc = (object *)calloc(1,sizeof(object));
  if (oc == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  oc->objtype = OBJTYPE_CLOSEDSPLINE;
  oc->npoints = npoints;
  oc->points = (point3 *)calloc(1,npoints * sizeof(point3));
  if (oc->points == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    exit(1);
  }
  for (j = 0; j < npoints; j++)
  {
    oc->points[j].x = o->points[j].x;
    oc->points[j].y = o->points[j].y;
    oc->points[j].z = o->points[j].z;
  }
  return oc;
} // copyclosedspline()

/* ==================================== */
scene * copyscene(scene * s)
/* ==================================== */
#undef F_NAME
#define F_NAME "copyscene"
{
  scene * sc;
  int32_t i, nobj = s->nobj;
  object *obj;

  sc = (scene *)calloc(1,sizeof(scene));
  if (sc == NULL)
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return NULL;
  }

  sc->nobj = nobj;
  sc->tabobj = (object **)calloc(1,nobj * sizeof(object *));
  if (sc->tabobj == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return NULL;
  }
 
  for (i = 0; i < nobj; i++)
  {
    obj = s->tabobj[i];
    switch (obj->objtype)
    {
    case OBJTYPE_LINE: sc->tabobj[i] = copyline(obj); break;
    case OBJTYPE_CLOSEDLINE: sc->tabobj[i] = copyclosedline(obj); break;
    case OBJTYPE_SPLINE: sc->tabobj[i] = copyspline(obj); break;
    case OBJTYPE_CLOSEDSPLINE: sc->tabobj[i] = copyclosedspline(obj); break;
    default:
      fprintf(stderr, "%s: bad object typs: %d\n", F_NAME, obj->objtype);
      return 0;
    } // switch (obj->objtype)
  } // for (i = 0; i < nobj; i++)

  return sc;
} // copyscene()

/* ============================================= */
/* ============================================= */
/* ============================================= */

#ifdef TESTGEO
int32_t main()
{
  mat33 m, mr, mp;
  vec3 b;
  vec3 sol;

  mat22 m2, mr2, mp2;
  vec2 b2;
  vec2 sol2;

  triangle t;

  mesh * msh;

  m[0][0] = 1.0;
  m[0][1] = 2.0;
  m[0][2] = 3.0;
  m[1][0] = 1.0;
  m[1][1] = 5.0;
  m[1][2] = 6.0;
  m[2][0] = 2.0;
  m[2][1] = 3.0;
  m[2][2] = 1.0;
  b[0] = 10.0;
  b[1] = 5.0;
  b[2] = 15.0;

  printmat((double *)m, 3, 3);
  printvec(b, 3);

  if (solsyst3(m, b, sol) == 0)
    printf("determinant nul\n");

  printvec(sol, 3);

  multmat3vec3(m, sol, b);

  printvec(b, 3);

  invmat3(m, mr);

  printmat((double *)mr, 3, 3);

  multmat3mat3(m, mr, mp);

  printmat((double *)mp, 3, 3);

  m2[0][0] = 1.0;
  m2[0][1] = 2.0;
  m2[1][0] = 1.0;
  m2[1][1] = 5.0;
  b2[0] = 10.0;
  b2[1] = 5.0;

  printmat((double *)m2, 2, 2);
  printvec(b2, 2);

  if (solsyst2(m2, b2, sol2) == 0)
    printf("determinant nul\n");

  printvec(sol2, 2);

  multmat2vec2(m2, sol2, b2);

  printvec(b2, 2);

  invmat2(m2, mr2);

  printmat((double *)mr2, 2, 2);

  multmat2mat2(m2, mr2, mp2);

  printmat((double *)mp2, 2, 2);

  t.xa = 1.0;
  t.ya = 1.0;
  t.xb = 2.0;
  t.yb = 1.0;
  t.xc = 1.0;
  t.yc = 2.0;

  if (inittriangle(&t) == 0)
    printf("mauvais triangle\n");

  printtriangle(&t);

  msh = readmesh("essai.msh");
  printmesh(msh);  
  freemesh(msh);  

}
#endif
