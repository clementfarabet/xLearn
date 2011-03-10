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
  Trace un segment de droite dans une image.
  Utilise l'algorithme de Bresenham.
  D'apres Kenny Hoff 
    (http://wwwx.cs.unc.edu/~hoff/projects/comp235/bresline/bresen.html)

  Variante lbresenlist: le segment est retourne sous forme de liste de points 
  (int32_t *) lp, avec lp[0] = nb de points.

  Michel Couprie - juin 1998 

  Trace une conique - d'apres Foley and van Dam p 959, "2nd ed, revised 5th printing"
  implementation de Andrew W. Fitzgibbon (andrewfg@ed.ac.uk),
  http://www.robots.ox.ac.uk/~awf/graphics/bres-ellipse.html

  ATTENTION : aucun test de debordement de l'image n'est effectue.

*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>
#include <mcliste.h>
#include <lbresen.h>

#define DEBUG 0
#define DEBUGLIST

/* ================================================= */
void lbresen(uint8_t *F, int32_t rs, int32_t Ax, int32_t Ay, int32_t Bx, int32_t By)
/* ================================================= */
{
  int32_t dX = mcabs(Bx - Ax);
  int32_t dY = mcabs(By - Ay);
  int32_t P;
  int32_t dPp;
  int32_t dPm;
  int32_t x, y;

  if (dY <= dX) 
  {                   /* pente <= 1 */
    P = 2 * dY - dX;
    dPp = (2 * dY) - (2 * dX);
    dPm = 2 * dY;
    y = Ay;
    if (Ax <= Bx)
    {
      if (Ay <= By)
      {               /* octant Est-Nord */
        for (x = Ax; x <= Bx; x++)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { y++; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x <= Bx; x++) */
      }
      else /* Ay > By */
      {               /* octant Est-Sud */
        for (x = Ax; x <= Bx; x++)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { y--; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x <= Bx; x++) */
      }
    }
    else /* Ax > Bx */
    {
      if (Ay <= By)
      {               /* octant Ouest-Nord */
        for (x = Ax; x >= Bx; x--)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { y++; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x <= Bx; x--) */
      }
      else /* Ay > By */
      {               /* octant Ouest-Sud */
        for (x = Ax; x >= Bx; x--)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { y--; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x >= Bx; x--) */
      }
    }
  }
  else
  {                   /* pente > 1 */
    P = 2 * dX - dY;
    dPp = (2 * dX) - (2 * dY);
    dPm = 2 * dX;
    x = Ax;
    if (Ay <= By)
    {
      if (Ax <= Bx)
      {               /* octant Nord-Est */
        for (y = Ay; y <= By; y++)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { x++; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y <= By; y++) */
      }
      else /* Ax > Bx */
      {               /* octant Nord-Ouest */
        for (y = Ay; y <= By; y++)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { x--; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y <= By; y++) */
      }
    }
    else /* Ay > By */
    {
      if (Ax <= Bx)
      {               /* octant Sud-Est */
        for (y = Ay; y >= By; y--)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { x++; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y <= By; y--) */
      }
      else /* Ax > Bx */
      {               /* octant Sud-Ouest */
        for (y = Ay; y >= By; y--)
        {
          F[y * rs + x] = NDG_MAX;
          if (P > 0)
          { x--; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y >= By; y--) */
      }
    }
  }
} /* lbresen() */

/* ================================================= */
void lbresenlist(int32_t Ax, int32_t Ay, int32_t Bx, int32_t By, int32_t *lx, int32_t *ly, int32_t *n)
/* ================================================= */
/*
  resultat dans les listes lx, ly et n (n est le nb de points).
  lx et ly doivent etre prealablement allouees.
  n doit contenir au départ le nombre max de points autorisés.
  si le segment est plus long que cette valeur, lbresenlist se termine.
*/
{
  int32_t dX = mcabs(Bx - Ax);
  int32_t dY = mcabs(By - Ay);
  int32_t P;
  int32_t dPp;
  int32_t dPm;
  int32_t x, y;
  int32_t nn, nmax = *n;

  nn = 0;                 /* pour compter les points */

  if (dY <= dX) 
  {                   /* pente <= 1 */
    P = 2 * dY - dX;
    dPp = (2 * dY) - (2 * dX);
    dPm = 2 * dY;
    y = Ay;
    if (Ax <= Bx)
    {
      if (Ay <= By)
      {               /* octant Est-Nord */
        for (x = Ax; x <= Bx; x++)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { y++; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x <= Bx; x++) */
      }
      else /* Ay > By */
      {               /* octant Est-Sud */
        for (x = Ax; x <= Bx; x++)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { y--; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x <= Bx; x++) */
      }
    }
    else /* Ax > Bx */
    {
      if (Ay <= By)
      {               /* octant Ouest-Nord */
        for (x = Ax; x >= Bx; x--)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { y++; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x <= Bx; x--) */
      }
      else /* Ay > By */
      {               /* octant Ouest-Sud */
        for (x = Ax; x >= Bx; x--)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { y--; P += dPp; }
          else
          { P += dPm; }
        } /* for (x = Ax; x >= Bx; x--) */
      }
    }
  }
  else
  {                   /* pente > 1 */
    P = 2 * dX - dY;
    dPp = (2 * dX) - (2 * dY);
    dPm = 2 * dX;
    x = Ax;
    if (Ay <= By)
    {
      if (Ax <= Bx)
      {               /* octant Nord-Est */
        for (y = Ay; y <= By; y++)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { x++; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y <= By; y++) */
      }
      else /* Ax > Bx */
      {               /* octant Nord-Ouest */
        for (y = Ay; y <= By; y++)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { x--; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y <= By; y++) */
      }
    }
    else /* Ay > By */
    {
      if (Ax <= Bx)
      {               /* octant Sud-Est */
        for (y = Ay; y >= By; y--)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { x++; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y <= By; y--) */
      }
      else /* Ax > Bx */
      {               /* octant Sud-Ouest */
        for (y = Ay; y >= By; y--)
        {
	  if (nn >= nmax) return;
          lx[nn] = x;
          ly[nn++] = y;
          if (P > 0)
          { x--; P += dPp; }
          else
          { P += dPm; }
        } /* for (y = Ay; y >= By; y--) */
      }
    }
  }
  *n = nn;
} /* lbresenlist() */

/*

 CONIC  2D Bresenham-like conic drawer.
       CONIC(Sx,Sy, Ex,Ey, A,B,C,D,E,F) draws the conic specified
       by A x^2 + B x y + C y^2 + D x + E y + F = 0, between the
       start point (Sx, Sy) and endpoint (Ex,Ey).

 Author: Andrew W. Fitzgibbon (andrewfg@ed.ac.uk),
         Machine Vision Unit,
         Dept. of Artificial Intelligence,
         Edinburgh University,
 
 Date: 31-Mar-94
 Version 2: 6-Oct-95
      Bugfixes from Arne Steinarson <arst@ludd.luth.se>
*/

static int32_t DIAGx[] = {999, 1,  1, -1, -1, -1, -1,  1,  1};
static int32_t DIAGy[] = {999, 1,  1,  1,  1, -1, -1, -1, -1};
static int32_t SIDEx[] = {999, 1,  0,  0, -1, -1,  0,  0,  1};
static int32_t SIDEy[] = {999, 0,  1,  1,  0,  0, -1, -1,  0};

/* ================================================= */
int32_t getoctant(int32_t gx, int32_t gy)
/* ================================================= */
{
  /* Use gradient to identify octant. */
  int32_t upper = mcabs(gx)>mcabs(gy);
  if (gx>=0)                            /* Right-pointing */
    if (gy>=0)                          /*    Up */
      return 4 - upper;
    else                                /*    Down */
      return 1 + upper;
  else                                  /* Left */
    if (gy>0)                           /*    Up */
      return 5 + upper;
    else                                /*    Down */
      return 8 - upper;
}

/* ================================================= */
void plot(uint8_t *I, int32_t rs, int32_t cs, int32_t x, int32_t y, uint8_t color)
/* ================================================= */
{
  if ((x < 0) || (x >= rs) || (y < 0) || (y >= cs)) return;
  I[y*rs + x] = color;
} /* plot() */

/* ================================================= */
void lconic(uint8_t *I, int32_t rs, int32_t cs, int32_t xs, int32_t ys, int32_t xe, int32_t ye,
           int32_t A, int32_t B, int32_t C, int32_t D, int32_t E, int32_t F, int32_t xoffset, int32_t yoffset)
/* ================================================= */
{
  int32_t octant; 
  int32_t dxS, dyS, dxD, dyD;
  int32_t d,u,v;
  int32_t k1sign, k1, Bsign, k2, k3;
  int32_t gxe, gye, gx, gy;
  int32_t octantcount;
  int32_t x, y;
  int32_t tmp, tmpdk;

  A *= 4;
  B *= 4;
  C *= 4;
  D *= 4;
  E *= 4;
  F *= 4;

  if (DEBUG) fprintf(stderr,"lconic -- %d %d %d %d %d %d\n", xs, ys, xe, ye, xoffset, yoffset);
  if (DEBUG) fprintf(stderr,"lconic -- %d %d %d %d %d %d\n", A,B,C,D,E,F);

  /* Translate start point to origin... */
  /*
  F = A*xs*xs + B*xs*ys + C*ys*ys + D*xs + E*ys + F;
  D = D + 2 * A * xs + B * ys;
  E = E + B * xs + 2 * C * ys;
  */

  /* Work out starting octant */
  octant = getoctant(D,E);
  
  dxS = SIDEx[octant]; 
  dyS = SIDEy[octant]; 
  dxD = DIAGx[octant];
  dyD = DIAGy[octant];

  switch (octant) {
  case 1:
    d = A + B/2 + C/4 + D + E/2 + F;
    u = A + B/2 + D;
    v = u + E;
    break;
  case 2:
    d = A/4 + B/2 + C + D/2 + E + F;
    u = B/2 + C + E;
    v = u + D;
    break;
  case 3:
    d = A/4 - B/2 + C - D/2 + E + F;
    u = -B/2 + C + E;
    v = u - D;
    break;
  case 4:
    d = A - B/2 + C/4 - D + E/2 + F;
    u = A - B/2 - D;
    v = u + E;
    break;
  case 5:
    d = A + B/2 + C/4 - D - E/2 + F;
    u = A + B/2 - D;
    v = u - E;
    break;
  case 6:
    d = A/4 + B/2 + C - D/2 - E + F;
    u = B/2 + C - E;
    v = u - D;
    break;
  case 7:
    d = A/4 - B/2 + C + D/2 - E + F;
    u =  -B/2 + C - E;
    v = u + D;
    break;
  case 8:
    d = A - B/2 + C/4 + D - E/2 + F;
    u = A - B/2 + D;
    v = u - E;
    break;
  default:
    fprintf(stderr,"FUNNY OCTANT\n");
    abort();
  }
  
  k1sign = dyS*dyD;
  k1 = 2 * (A + k1sign * (C - A));
  Bsign = dxD*dyD;
  k2 = k1 + Bsign * B;
  k3 = 2 * (A + C + Bsign * B);

  /* Work out gradient at endpoint */
  gxe = xe - xs;
  gye = ye - ys;
  gx = 2*A*gxe +   B*gye + D;
  gy =   B*gxe + 2*C*gye + E;
  
  octantcount = getoctant(gx,gy) - octant;
  if (octantcount < 0)
    octantcount = octantcount + 8;
  else if (octantcount==0)
    if((xs>xe && dxD>0) || (ys>ye && dyD>0) ||
       (xs<xe && dxD<0) || (ys<ye && dyD<0))
      octantcount +=8;

  if (DEBUG)
    fprintf(stderr,"octantcount = %d\n", octantcount);
  
  x = xs;
  y = ys;
  
  while (octantcount > 0) {
    if (DEBUG)
      fprintf(stderr,"-- %d -------------------------\n", octant); 
    
    if (mcodd(octant)) {
      while (2*v <= k2) {
        /* Plot this point */
        plot(I, rs, cs, x+xoffset, y+yoffset, NDG_MAX);
        
        /* Are we inside or outside? */
        if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
        if (d < 0) {                    /* Inside */
          x = x + dxS;
          y = y + dyS;
          u = u + k1;
          v = v + k2;
          d = d + u;
        }
        else {                          /* outside */
          x = x + dxD;
          y = y + dyD;
          u = u + k2;
          v = v + k3;
          d = d + v;
        }
      }
    
      d = d - u + v/2 - k2/2 + 3*k3/8; 
      /* error (^) in Foley and van Dam p 959, "2nd ed, revised 5th printing" */
      u = -u + v - k2/2 + k3/2;
      v = v - k2 + k3/2;
      k1 = k1 - 2*k2 + k3;
      k2 = k3 - k2;
      tmp = dxS; dxS = -dyS; dyS = tmp;
    }
    else {                              /* Octant is even */
      while (2*u < k2) {
        /* Plot this point */
        plot(I, rs, cs, x+xoffset, y+yoffset, NDG_MAX);
        if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
        
        /* Are we inside or outside? */
        if (d > 0) {                    /* Outside */
          x = x + dxS;
          y = y + dyS;
          u = u + k1;
          v = v + k2;
          d = d + u;
        }
        else {                          /* Inside */
          x = x + dxD;
          y = y + dyD;
          u = u + k2;
          v = v + k3;
          d = d + v;
        }
      }
      tmpdk = k1 - k2;
      d = d + u - v + tmpdk;
      v = 2*u - v + tmpdk;
      u = u + tmpdk;
      k3 = k3 + 4*tmpdk;
      k2 = k1 + tmpdk;
      
      tmp = dxD; dxD = -dyD; dyD = tmp;
    }
    
    octant = (octant&7)+1;
    octantcount--;
  }

  /* Draw final octant until we reach the endpoint */
  if (DEBUG)
    fprintf(stderr,"-- %d (final) -----------------\n", octant); 
    
  if (mcodd(octant)) {
    while (2*v <= k2) {
      /* Plot this point */
      plot(I, rs, cs, x+xoffset, y+yoffset, NDG_MAX);

      if (x == xe && y == ye)
        break;
      if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
      
      /* Are we inside or outside? */
      if (d < 0) {                      /* Inside */
        x = x + dxS;
        y = y + dyS;
        u = u + k1;
        v = v + k2;
        d = d + u;
      }
      else {                            /* outside */
        x = x + dxD;
        y = y + dyD;
        u = u + k2;
        v = v + k3;
        d = d + v;
      }
    }
  }
  else {                                /* Octant is even */
    while ((2*u < k2)) {
      /* Plot this point */
      plot(I, rs, cs, x+xoffset, y+yoffset, NDG_MAX);

      if (x == xe && y == ye)
        break;
      if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
      
      /* Are we inside or outside? */
      if (d > 0) {                      /* Outside */
        x = x + dxS;
        y = y + dyS;
        u = u + k1;
        v = v + k2;
        d = d + u;
      }
      else {                            /* Inside */
        x = x + dxD;
        y = y + dyD;
        u = u + k2;
        v = v + k3;
        d = d + v;
      }
    }
  }
} /* lconic() */

/* ================================================= */
void lellipsearc(uint8_t *I, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
                 int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specifies and draws an ellipse arc in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Applies the translation (xoffset, yoffset) to the result.
*/
{
  int32_t xprod, tmp;
  int32_t A, B, C, D, E, F;
  
  if (DEBUG) fprintf(stderr,"lellipsearc -- %d %d %d %d %d %d\n", xp, yp, xq, yq, xoffset, yoffset);

  xprod = xp*yq - xq*yp;

  if (DEBUG) fprintf(stderr,"xprod = %d\n", xprod);

  if (xprod != 0)        /* if 0, the points are colinear ! */
  {
    if (xprod < 0)       /* exchange P and Q */
    {
      tmp = xp; xp = xq; xq = tmp;
      tmp = yp; yp = yq; yq = tmp;
      xprod = -xprod;
    } /* if (xprod < 0) */
    A = yp*yp + yq*yq;
    B = -2 * (xp*yp + xq*yq);
    C = xp*xp + xq*xq;
    D = 2 * yq * xprod;
    E = -2 * xq * xprod;
    F = 0;
    lconic(I, rs, cs, xp, yp, xq, yq, A, B, C, D, E, F, xoffset, yoffset);
  } /* if (xprod != 0) */
} /* lellipsearc() */

/* ================================================= */
void lellipse(uint8_t *I, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
              int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specifies and draws an ellipse in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Applies the translation (xoffset, yoffset) to the result.
*/
{
  if (DEBUG) fprintf(stderr,"lellipse -- %d %d %d %d %d %d\n", xp, yp, xq, yq, xoffset, yoffset);
  lellipsearc(I, rs, cs, xp, yp, xq, yq, xoffset, yoffset);
  lellipsearc(I, rs, cs, xq, yq, -xp, -yp, xoffset, yoffset);
  lellipsearc(I, rs, cs, -xp, -yp, -xq, -yq, xoffset, yoffset);
  lellipsearc(I, rs, cs, -xq, -yq, xp, yp, xoffset, yoffset);
} /* lellipse() */

/* ================================================= */
/* ================================================= */
/* sous la forme "list", une ellipse est definie comme une liste de triplets (y, xmin, xmax) */
/* y indexe les deux tableaux xmin et xmax */
/* si (xmin <= xmax), les points (xmin,y) et (xmax,y) sont sur l'ellipse */

/* ================================================= */
void plotlist(ellipse *ell, int32_t rs, int32_t cs, int32_t x, int32_t y)
/* ================================================= */
{
  if ((y < 0) || (y >= cs)) return;
  if (x < 0) { ell->xmin[y] = 0; return; }
  if (x >= rs) { ell->xmax[y] = rs - 1; return; }
  if (x < ell->xmin[y]) ell->xmin[y] = x;
  if (x > ell->xmax[y]) ell->xmax[y] = x;
} /* plotlist() */

/* ================================================= */
void lconiclist(ellipse *ell, int32_t rs, int32_t cs, int32_t xs, int32_t ys, int32_t xe, int32_t ye,
           int32_t A, int32_t B, int32_t C, int32_t D, int32_t E, int32_t F, int32_t xoffset, int32_t yoffset)
/* ================================================= */
{
  int32_t octant; 
  int32_t dxS, dyS, dxD, dyD;
  int32_t d,u,v;
  int32_t k1sign, k1, Bsign, k2, k3;
  int32_t gxe, gye, gx, gy;
  int32_t octantcount;
  int32_t x, y;
  int32_t tmp, tmpdk;

  A *= 4;
  B *= 4;
  C *= 4;
  D *= 4;
  E *= 4;
  F *= 4;

  if (DEBUG) fprintf(stderr,"lconic -- %d %d %d %d %d %d\n", xs, ys, xe, ye, xoffset, yoffset);
  if (DEBUG) fprintf(stderr,"lconic -- %d %d %d %d %d %d\n", A,B,C,D,E,F);

  /* Translate start point to origin... */
  /*
  F = A*xs*xs + B*xs*ys + C*ys*ys + D*xs + E*ys + F;
  D = D + 2 * A * xs + B * ys;
  E = E + B * xs + 2 * C * ys;
  */

  /* Work out starting octant */
  octant = getoctant(D,E);
  
  dxS = SIDEx[octant]; 
  dyS = SIDEy[octant]; 
  dxD = DIAGx[octant];
  dyD = DIAGy[octant];

  switch (octant) {
  case 1:
    d = A + B/2 + C/4 + D + E/2 + F;
    u = A + B/2 + D;
    v = u + E;
    break;
  case 2:
    d = A/4 + B/2 + C + D/2 + E + F;
    u = B/2 + C + E;
    v = u + D;
    break;
  case 3:
    d = A/4 - B/2 + C - D/2 + E + F;
    u = -B/2 + C + E;
    v = u - D;
    break;
  case 4:
    d = A - B/2 + C/4 - D + E/2 + F;
    u = A - B/2 - D;
    v = u + E;
    break;
  case 5:
    d = A + B/2 + C/4 - D - E/2 + F;
    u = A + B/2 - D;
    v = u - E;
    break;
  case 6:
    d = A/4 + B/2 + C - D/2 - E + F;
    u = B/2 + C - E;
    v = u - D;
    break;
  case 7:
    d = A/4 - B/2 + C + D/2 - E + F;
    u =  -B/2 + C - E;
    v = u + D;
    break;
  case 8:
    d = A - B/2 + C/4 + D - E/2 + F;
    u = A - B/2 + D;
    v = u - E;
    break;
  default:
    fprintf(stderr,"FUNNY OCTANT\n");
    abort();
  }
  
  k1sign = dyS*dyD;
  k1 = 2 * (A + k1sign * (C - A));
  Bsign = dxD*dyD;
  k2 = k1 + Bsign * B;
  k3 = 2 * (A + C + Bsign * B);

  /* Work out gradient at endpoint */
  gxe = xe - xs;
  gye = ye - ys;
  gx = 2*A*gxe +   B*gye + D;
  gy =   B*gxe + 2*C*gye + E;
  
  octantcount = getoctant(gx,gy) - octant;
  if (octantcount < 0)
    octantcount = octantcount + 8;
  else if (octantcount==0)
    if((xs>xe && dxD>0) || (ys>ye && dyD>0) ||
       (xs<xe && dxD<0) || (ys<ye && dyD<0))
      octantcount +=8;

  if (DEBUG)
    fprintf(stderr,"octantcount = %d\n", octantcount);
  
  x = xs;
  y = ys;
  
  while (octantcount > 0) {
    if (DEBUG)
      fprintf(stderr,"-- %d -------------------------\n", octant); 
    
    if (mcodd(octant)) {
      while (2*v <= k2) {
        /* Plot this point */
        plotlist(ell, rs, cs, x+xoffset, y+yoffset);
        
        /* Are we inside or outside? */
        if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
        if (d < 0) {                    /* Inside */
          x = x + dxS;
          y = y + dyS;
          u = u + k1;
          v = v + k2;
          d = d + u;
        }
        else {                          /* outside */
          x = x + dxD;
          y = y + dyD;
          u = u + k2;
          v = v + k3;
          d = d + v;
        }
      }
    
      d = d - u + v/2 - k2/2 + 3*k3/8; 
      /* error (^) in Foley and van Dam p 959, "2nd ed, revised 5th printing" */
      u = -u + v - k2/2 + k3/2;
      v = v - k2 + k3/2;
      k1 = k1 - 2*k2 + k3;
      k2 = k3 - k2;
      tmp = dxS; dxS = -dyS; dyS = tmp;
    }
    else {                              /* Octant is even */
      while (2*u < k2) {
        /* Plot this point */
        plotlist(ell, rs, cs, x+xoffset, y+yoffset);
        if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
        
        /* Are we inside or outside? */
        if (d > 0) {                    /* Outside */
          x = x + dxS;
          y = y + dyS;
          u = u + k1;
          v = v + k2;
          d = d + u;
        }
        else {                          /* Inside */
          x = x + dxD;
          y = y + dyD;
          u = u + k2;
          v = v + k3;
          d = d + v;
        }
      }
      tmpdk = k1 - k2;
      d = d + u - v + tmpdk;
      v = 2*u - v + tmpdk;
      u = u + tmpdk;
      k3 = k3 + 4*tmpdk;
      k2 = k1 + tmpdk;
      
      tmp = dxD; dxD = -dyD; dyD = tmp;
    }
    
    octant = (octant&7)+1;
    octantcount--;
  }

  /* Draw final octant until we reach the endpoint */
  if (DEBUG)
    fprintf(stderr,"-- %d (final) -----------------\n", octant); 
    
  if (mcodd(octant)) {
    while (2*v <= k2) {
      /* Plot this point */
      plotlist(ell, rs, cs, x+xoffset, y+yoffset);

      if (x == xe && y == ye)
        break;
      if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
      
      /* Are we inside or outside? */
      if (d < 0) {                      /* Inside */
        x = x + dxS;
        y = y + dyS;
        u = u + k1;
        v = v + k2;
        d = d + u;
      }
      else {                            /* outside */
        x = x + dxD;
        y = y + dyD;
        u = u + k2;
        v = v + k3;
        d = d + v;
      }
    }
  }
  else {                                /* Octant is even */
    while ((2*u < k2)) {
      /* Plot this point */
      plotlist(ell, rs, cs, x+xoffset, y+yoffset);

      if (x == xe && y == ye)
        break;
      if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
      
      /* Are we inside or outside? */
      if (d > 0) {                      /* Outside */
        x = x + dxS;
        y = y + dyS;
        u = u + k1;
        v = v + k2;
        d = d + u;
      }
      else {                            /* Inside */
        x = x + dxD;
        y = y + dyD;
        u = u + k2;
        v = v + k3;
        d = d + v;
      }
    }
  }
} /* lconiclist() */

/* ================================================= */
void lellipsearclist(ellipse *ell, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
                 int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specify and draw an ellipse arc in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Apply the translation (xoffset, yoffset) to the result.
*/
{
  int32_t xprod, tmp;
  int32_t A, B, C, D, E, F;
  
  if (DEBUG) fprintf(stderr,"lellipsearc -- %d %d %d %d %d %d\n", xp, yp, xq, yq, xoffset, yoffset);

  xprod = xp*yq - xq*yp;

  if (DEBUG) fprintf(stderr,"xprod = %d\n", xprod);

  if (xprod != 0)        /* if 0, the points are colinear ! */
  {
    if (xprod < 0)       /* exchange P and Q */
    {
      tmp = xp; xp = xq; xq = tmp;
      tmp = yp; yp = yq; yq = tmp;
      xprod = -xprod;
    } /* if (xprod < 0) */
    A = yp*yp + yq*yq;
    B = -2 * (xp*yp + xq*yq);
    C = xp*xp + xq*xq;
    D = 2 * yq * xprod;
    E = -2 * xq * xprod;
    F = 0;
    lconiclist(ell, rs, cs, xp, yp, xq, yq, A, B, C, D, E, F, xoffset, yoffset);
  } /* if (xprod != 0) */
} /* lellipsearclist() */

/* ================================================= */
ellipse *AllocEllipseList(int32_t rs, int32_t cs)
/* ================================================= */
{
  ellipse *ell = (ellipse *)calloc(1,sizeof(ellipse));
  if (ell == NULL)
  {   fprintf(stderr,"allocellipselist() : malloc failed\n");
      exit(0);
  }
  ell->rs = rs;
  ell->cs = cs;
  ell->xmin = (int32_t *)calloc(1,cs * sizeof(int32_t));
  ell->xmax = (int32_t *)calloc(1,cs * sizeof(int32_t));
  if ((ell->xmin == NULL) || (ell->xmax == NULL))
  {   fprintf(stderr,"allocellipselist() : malloc failed\n");
      exit(0);
  }
  return ell;
} /* AllocEllipseList() */

/* ================================================= */
void InitEllipseList(ellipse * ell)
/* ================================================= */
{
  int32_t i, rs = ell->rs, cs = ell->cs;
  for (i = 0; i < cs; i++) ell->xmin[i] = rs-1;
  for (i = 0; i < cs; i++) ell->xmax[i] = 0;
} /* InitEllipseList() */

/* ================================================= */
void FreeEllipseList(ellipse *ell)
/* ================================================= */
{
  free(ell->xmin);
  free(ell->xmax);
  free(ell);
} /* FreeEllipseList() */

/* ================================================= */
void lellipselist(ellipse *ell, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
              int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specify and draw an ellipse in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Applies the translation (xoffset, yoffset) to the result.
*/
{
  InitEllipseList(ell);
  if (DEBUG) fprintf(stderr,"lellipse -- %d %d %d %d %d %d\n", xp, yp, xq, yq, xoffset, yoffset);
  lellipsearclist(ell, rs, cs, xp, yp, xq, yq, xoffset, yoffset);
  lellipsearclist(ell, rs, cs, xq, yq, -xp, -yp, xoffset, yoffset);
  lellipsearclist(ell, rs, cs, -xp, -yp, -xq, -yq, xoffset, yoffset);
  lellipsearclist(ell, rs, cs, -xq, -yq, xp, yp, xoffset, yoffset);
} /* lellipselist() */

/* ================================================= */
void lplotfilledellipse(ellipse *ell, uint8_t *I)
/* ================================================= */
{
  int32_t x, y, rs, cs;
  rs = ell->rs;
  cs = ell->cs;
  for (y = 0; y < cs; y++)
  {
    if (ell->xmin[y] <= ell->xmax[y])
      for (x = ell->xmin[y]; x <= ell->xmax[y]; x++)
      {
        I[y*rs + x] = NDG_MAX;
      }
  }
}  // lplotfilledellipse()



/* ================================================= */
/* ================================================= */
/* sous la forme "liste", une ellipse est definie comme une liste de points */
/* on alloue 2*rs + 2*cs pour etre tranquille */

/* ================================================= */
void plotliste(Liste *lp, int32_t rs, int32_t x, int32_t y)
/* ================================================= */
{
  int32_t p = rs * y + x;
  if (ListeVide(lp) || ((p != lp->Pts[lp->Sp - 1]) && (p != lp->Pts[0])))
    ListePush(lp, p);
} /* plotliste() */

/* ================================================= */
void lconicliste(Liste *lp, int32_t rs, int32_t cs, int32_t xs, int32_t ys, int32_t xe, int32_t ye,
           int32_t A, int32_t B, int32_t C, int32_t D, int32_t E, int32_t F, int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  resultat dans la liste lp.
  lp doit etre prealablement alloue.
*/
{
  int32_t octant; 
  int32_t dxS, dyS, dxD, dyD;
  int32_t d,u,v;
  int32_t k1sign, k1, Bsign, k2, k3;
  int32_t gxe, gye, gx, gy;
  int32_t octantcount;
  int32_t x, y;
  int32_t tmp, tmpdk;

  A *= 4;
  B *= 4;
  C *= 4;
  D *= 4;
  E *= 4;
  F *= 4;

  if (DEBUG) fprintf(stderr,"lconic -- %d %d %d %d %d %d\n", xs, ys, xe, ye, xoffset, yoffset);
  if (DEBUG) fprintf(stderr,"lconic -- %d %d %d %d %d %d\n", A,B,C,D,E,F);

  /* Translate start point to origin... */
  /*
  F = A*xs*xs + B*xs*ys + C*ys*ys + D*xs + E*ys + F;
  D = D + 2 * A * xs + B * ys;
  E = E + B * xs + 2 * C * ys;
  */

  /* Work out starting octant */
  octant = getoctant(D,E);
  
  dxS = SIDEx[octant]; 
  dyS = SIDEy[octant]; 
  dxD = DIAGx[octant];
  dyD = DIAGy[octant];

  switch (octant) {
  case 1:
    d = A + B/2 + C/4 + D + E/2 + F;
    u = A + B/2 + D;
    v = u + E;
    break;
  case 2:
    d = A/4 + B/2 + C + D/2 + E + F;
    u = B/2 + C + E;
    v = u + D;
    break;
  case 3:
    d = A/4 - B/2 + C - D/2 + E + F;
    u = -B/2 + C + E;
    v = u - D;
    break;
  case 4:
    d = A - B/2 + C/4 - D + E/2 + F;
    u = A - B/2 - D;
    v = u + E;
    break;
  case 5:
    d = A + B/2 + C/4 - D - E/2 + F;
    u = A + B/2 - D;
    v = u - E;
    break;
  case 6:
    d = A/4 + B/2 + C - D/2 - E + F;
    u = B/2 + C - E;
    v = u - D;
    break;
  case 7:
    d = A/4 - B/2 + C + D/2 - E + F;
    u =  -B/2 + C - E;
    v = u + D;
    break;
  case 8:
    d = A - B/2 + C/4 + D - E/2 + F;
    u = A - B/2 + D;
    v = u - E;
    break;
  default:
    fprintf(stderr,"FUNNY OCTANT\n");
    abort();
  }
  
  k1sign = dyS*dyD;
  k1 = 2 * (A + k1sign * (C - A));
  Bsign = dxD*dyD;
  k2 = k1 + Bsign * B;
  k3 = 2 * (A + C + Bsign * B);

  /* Work out gradient at endpoint */
  gxe = xe - xs;
  gye = ye - ys;
  gx = 2*A*gxe +   B*gye + D;
  gy =   B*gxe + 2*C*gye + E;
  
  octantcount = getoctant(gx,gy) - octant;
  if (octantcount < 0)
    octantcount = octantcount + 8;
  else if (octantcount==0)
    if((xs>xe && dxD>0) || (ys>ye && dyD>0) ||
       (xs<xe && dxD<0) || (ys<ye && dyD<0))
      octantcount +=8;

  if (DEBUG)
    fprintf(stderr,"octantcount = %d\n", octantcount);
  
  x = xs;
  y = ys;
  
  while (octantcount > 0) {
    if (DEBUG)
      fprintf(stderr,"-- %d -------------------------\n", octant); 
    
    if (mcodd(octant)) {
      while (2*v <= k2) {
        /* Plot this point */
        plotliste(lp, rs, x+xoffset, y+yoffset);
        
        /* Are we inside or outside? */
        if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
        if (d < 0) {                    /* Inside */
          x = x + dxS;
          y = y + dyS;
          u = u + k1;
          v = v + k2;
          d = d + u;
        }
        else {                          /* outside */
          x = x + dxD;
          y = y + dyD;
          u = u + k2;
          v = v + k3;
          d = d + v;
        }
      }
    
      d = d - u + v/2 - k2/2 + 3*k3/8; 
      /* error (^) in Foley and van Dam p 959, "2nd ed, revised 5th printing" */
      u = -u + v - k2/2 + k3/2;
      v = v - k2 + k3/2;
      k1 = k1 - 2*k2 + k3;
      k2 = k3 - k2;
      tmp = dxS; dxS = -dyS; dyS = tmp;
    }
    else {                              /* Octant is even */
      while (2*u < k2) {
        /* Plot this point */
        plotliste(lp, rs, x+xoffset, y+yoffset);
        if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
        
        /* Are we inside or outside? */
        if (d > 0) {                    /* Outside */
          x = x + dxS;
          y = y + dyS;
          u = u + k1;
          v = v + k2;
          d = d + u;
        }
        else {                          /* Inside */
          x = x + dxD;
          y = y + dyD;
          u = u + k2;
          v = v + k3;
          d = d + v;
        }
      }
      tmpdk = k1 - k2;
      d = d + u - v + tmpdk;
      v = 2*u - v + tmpdk;
      u = u + tmpdk;
      k3 = k3 + 4*tmpdk;
      k2 = k1 + tmpdk;
      
      tmp = dxD; dxD = -dyD; dyD = tmp;
    }
    
    octant = (octant&7)+1;
    octantcount--;
  }

  /* Draw final octant until we reach the endpoint */
  if (DEBUG)
    fprintf(stderr,"-- %d (final) -----------------\n", octant); 
    
  if (mcodd(octant)) {
    while (2*v <= k2) {
      /* Plot this point */
      plotliste(lp, rs, x+xoffset, y+yoffset);

      if (x == xe && y == ye)
        break;
      if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
      
      /* Are we inside or outside? */
      if (d < 0) {                      /* Inside */
        x = x + dxS;
        y = y + dyS;
        u = u + k1;
        v = v + k2;
        d = d + u;
      }
      else {                            /* outside */
        x = x + dxD;
        y = y + dyD;
        u = u + k2;
        v = v + k3;
        d = d + v;
      }
    }
  }
  else {                                /* Octant is even */
    while ((2*u < k2)) {
      /* Plot this point */
      plotliste(lp, rs, x+xoffset, y+yoffset);

      if (x == xe && y == ye)
        break;
      if (DEBUG) fprintf(stderr,"x = %3d y = %3d d = %4d\n", x,y,d);
      
      /* Are we inside or outside? */
      if (d > 0) {                      /* Outside */
        x = x + dxS;
        y = y + dyS;
        u = u + k1;
        v = v + k2;
        d = d + u;
      }
      else {                            /* Inside */
        x = x + dxD;
        y = y + dyD;
        u = u + k2;
        v = v + k3;
        d = d + v;
      }
    }
  }
} /* lconicliste() */

/* ================================================= */
void lellipsearcliste(Liste *lp, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
                 int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specify and draw an ellipse arc in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Apply the translation (xoffset, yoffset) to the result.
*/
{
  int32_t xprod, tmp;
  int32_t A, B, C, D, E, F;
  
  if (DEBUG) fprintf(stderr,"lellipsearc -- %d %d %d %d %d %d\n", xp, yp, xq, yq, xoffset, yoffset);

  xprod = xp*yq - xq*yp;

  if (DEBUG) fprintf(stderr,"xprod = %d\n", xprod);

  if (xprod != 0)        /* if 0, the points are colinear ! */
  {
    if (xprod < 0)       /* exchange P and Q */
    {
      tmp = xp; xp = xq; xq = tmp;
      tmp = yp; yp = yq; yq = tmp;
      xprod = -xprod;
    } /* if (xprod < 0) */
    A = yp*yp + yq*yq;
    B = -2 * (xp*yp + xq*yq);
    C = xp*xp + xq*xq;
    D = 2 * yq * xprod;
    E = -2 * xq * xprod;
    F = 0;
    lconicliste(lp, rs, cs, xp, yp, xq, yq, A, B, C, D, E, F, xoffset, yoffset);
  } /* if (xprod != 0) */
} /* lellipsearcliste() */

/* ================================================= */
Liste *lellipseliste(int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
              int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specify and draw an ellipse in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Applies the translation (xoffset, yoffset) to the result.
*/
{
  Liste * lp = CreeListeVide(2*rs + 2*cs);
  lellipsearcliste(lp, rs, cs, xp, yp, xq, yq, xoffset, yoffset);
  lellipsearcliste(lp, rs, cs, xq, yq, -xp, -yp, xoffset, yoffset);
  lellipsearcliste(lp, rs, cs, -xp, -yp, -xq, -yq, xoffset, yoffset);
  lellipsearcliste(lp, rs, cs, -xq, -yq, xp, yp, xoffset, yoffset);
  return lp;
} /* lellipseliste() */

/* ================================================= */
void lellipseliste2(Liste *lp, int32_t rs, int32_t cs, int32_t xp, int32_t yp, int32_t xq, int32_t yq, 
                   int32_t xoffset, int32_t yoffset)
/* ================================================= */
/*
  Specify and draw an ellipse in terms of the endpoints 
  P = (xp, yp) and Q = (xq, yq) of two conjugate diameters
  of the ellipse. The endpoints are specified as offsets
  relative to the center of the ellipse, assumed to be
  the origin.
  Applies the translation (xoffset, yoffset) to the result.
  CAUTION: lp must be an allocated and empty "Liste" structure with size (2*rs + 2*cs)
*/
{
  lellipsearcliste(lp, rs, cs, xp, yp, xq, yq, xoffset, yoffset);
  lellipsearcliste(lp, rs, cs, xq, yq, -xp, -yp, xoffset, yoffset);
  lellipsearcliste(lp, rs, cs, -xp, -yp, -xq, -yq, xoffset, yoffset);
  lellipsearcliste(lp, rs, cs, -xq, -yq, xp, yp, xoffset, yoffset);
} /* lellipseliste2() */

/* ================================================= */
void lplotellipseliste(Liste *lp, uint8_t *I)
/* ================================================= */
{
  int32_t p, i;
  for (i = 0; i < lp->Sp; i++)
  {
    p = lp->Pts[i];
    I[p] = NDG_MAX;
  }
}  // lplotellipseliste()

