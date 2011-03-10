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
   Operateurs agissant dans la grille de Khalimsky 3d:

   l3dkhalimskize: pour passer d'un objet de Z3 a un objet de H3
   l3dcolor: attribution de niveaux de gris differents suivant le rang d'un element
   l3dthin: amincissement homotopique
   l3dskelsurf: squelette surfacique
   l3disthmus: detruit les isthmes 1D (T = 2 ; Tb = 1)
   l3dlabel: etiquette les regions theta-connexes de l'image
   l3drecons: reconstruction geodesique (au sens du theta-voisinage)
     Michel Couprie - novembre 1999 

   l3dinvariants: calculs des nombres de composantes connexes, cavites et tunnels
     Michel Couprie - avril 2001

   l3dsphere: dicretisation d'une sphere
   l3dplane: dicretisation d'un plan
     Michel Couprie - avril 2002

   l3dborder: extraction de la frontière interne
     Michel Couprie - décembre 2004

   l3dseltype: extraction d'éléments selon leur rang et leur type topologique
     Michel Couprie - avril 2007

   l3dbeta: beta-dilatation
   l3dalpha: alpha-dilatation
     Michel Couprie - juillet 2007
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcutil.h>
#include <mckhalimsky3d.h>
#include <l3dkhalimsky.h>

/*
#define VERBOSE
#define DEBUGISTHMUS
*/

/* =============================================================== */
int32_t l3dkhalimskize(struct xvimage * i, struct xvimage **k, int32_t mode)
/* =============================================================== */
/* 
   passage de z3 a la grille de Khalimsky
   mode = 
     0 : "hit", 
     1 : min ndg, 
     2 : max ndg,
     3 : emulation de la 26-connexite, 
     4 : emulation de la 6-connexite, idem "miss"
     5 : moyenne
     6 : reverse (Khalimsky -> Z3) : selection cubes

*/
#undef F_NAME
#define F_NAME "l3dkhalimskize"
{
#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  if ((mode == 1) || (mode == 2) || (mode == 5))
    *k = KhalimskizeNDG3d(i);
  else if (mode == 6)
    *k = DeKhalimskize3d(i);
  else
    *k = Khalimskize3d(i);
  if (*k == NULL)
  {
    fprintf(stderr, "%s: Khalimskize3d failed\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  fprintf(stderr, "%s: Khalimskisation terminee\n", F_NAME);
#endif

  if (mode == 4)
    Connex6Obj3d(*k);
  else if (mode == 3)
    Connex26Obj3d(*k);
  else if (mode == 0)
    SatureAlphacarre3d(*k);
  else if (mode == 1)
    ndgmin3d(*k);
  else if (mode == 2)
    ndgmax3d(*k);
  else if (mode == 5)
    ndgmoy3d(*k);

  return 1;

} /* l3dkhalimskize() */

/* =============================================================== */
int32_t l3dmakecomplex(struct xvimage * i)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l3dmakecomplex"
/* 
   effectue la fermeture par inclusion de l'ensemble i
*/
{
  AjouteAlphacarre3d(i);
  return 1;
} /* l3dmakecomplex() */

/* =============================================================== */
int32_t l3dalpha(struct xvimage * i)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l3dalpha"
/* 
   alpha-dilatation (idem fermeture par inclusion) 
*/
{
  AjouteAlphacarre3d(i);
  return 1;
} /* l3dalpha() */

/* =============================================================== */
int32_t l3dbeta(struct xvimage * i)
/* =============================================================== */
#undef F_NAME
#define F_NAME "l3dbeta"
/* 
   beta-dilatation
*/
{
  AjouteBetacarre3d(i);
  return 1;
} /* l3dbeta() */

/* =============================================================== */
int32_t l3dcolor(struct xvimage * k)
/* =============================================================== */
/* 
   coloriage des elements de k selon leur type 
*/
#undef F_NAME
#define F_NAME "l3dcolor"
{
#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  ColorieKh3d(k);
  return 1;
} /* l3dcolor() */

/* =============================================================== */
int32_t l3dplane(struct xvimage * k, double a, double b, double c, double d)
/* =============================================================== */
/* 
  Draws into the Khalimsky volume \b k, the discretization of the plane 
  defined by the equation: \b a x + \b b y + \b c z + \b d = 0
*/
#undef F_NAME
#define F_NAME "l3dplane"
{
  index_t rs, cs, ps, ds, N;
  uint8_t * K;
  index_t x, y, z;             // coordinates in the continuous space
  index_t xx, yy, zz;          // coordinates in the khalimsky space
  double A, X, X1, X2, X3, X4;

  if (a * b * c == 0)
  {
    fprintf(stderr, "%s: a*b*c must be non-zero\n", F_NAME);
    return 0;
  }

  if ((a * b == 0) || (a * c == 0) || (c * b == 0))
  {
    fprintf(stderr, "%s: this case is not yet implemented\n", F_NAME);
    return 0;
  }

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  A = (mcabs(a) + mcabs(b) + mcabs(c)) / 2;

  for (z = 0; z < (ds+1)/2; z++)
  for (y = 0; y < (cs+1)/2; y++)
  for (x = 0; x < (rs+1)/2; x++)
  {
    //cube
    X = a * x + b * y + c * z + d;
    xx = 2 * x + 1; yy = 2 * y + 1; zz = 2 * z + 1; 
    if ((-A < X) && (X < A)) K[zz*ps + yy*rs + xx] = NDG_CUBE3D;
    //singleton
    X = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    xx = 2 * x; yy = 2 * y; zz = 2 * z; 
    if (X == 0) K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
    //interx
    X1 = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X2 = a * (x+0.5) + b * (y-0.5) + c * (z-0.5) + d;
    xx = 2 * x + 1; yy = 2 * y; zz = 2 * z; 
    if (((X1 == 0) && (X2 == 0)) || (X1 * X2 < 0)) K[zz*ps + yy*rs + xx] = NDG_INTER3DX;
    //intery
    X1 = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X2 = a * (x-0.5) + b * (y+0.5) + c * (z-0.5) + d;
    xx = 2 * x; yy = 2 * y + 1; zz = 2 * z; 
    if (((X1 == 0) && (X2 == 0)) || (X1 * X2 < 0)) K[zz*ps + yy*rs + xx] = NDG_INTER3DY;
    //interz
    X1 = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X2 = a * (x-0.5) + b * (y-0.5) + c * (z+0.5) + d;
    xx = 2 * x; yy = 2 * y; zz = 2 * z + 1; 
    if (((X1 == 0) && (X2 == 0)) || (X1 * X2 < 0)) K[zz*ps + yy*rs + xx] = NDG_INTER3DZ;
    //carrexy
    X1 = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X2 = a * (x-0.5) + b * (y+0.5) + c * (z-0.5) + d;
    X3 = a * (x+0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X4 = a * (x+0.5) + b * (y+0.5) + c * (z-0.5) + d;
    xx = 2 * x + 1; yy = 2 * y + 1; zz = 2 * z; 
    if ((X1 * X2 < 0) || (X1 * X3 < 0) || (X1 * X4 < 0) ||
        (X2 * X3 < 0) || (X2 * X4 < 0) || (X3 * X4 < 0))
      K[zz*ps + yy*rs + xx] = NDG_CARRE3DXY;
    //carrexz
    X1 = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X2 = a * (x-0.5) + b * (y-0.5) + c * (z+0.5) + d;
    X3 = a * (x+0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X4 = a * (x+0.5) + b * (y-0.5) + c * (z+0.5) + d;
    xx = 2 * x + 1; yy = 2 * y; zz = 2 * z + 1; 
    if ((X1 * X2 < 0) || (X1 * X3 < 0) || (X1 * X4 < 0) ||
        (X2 * X3 < 0) || (X2 * X4 < 0) || (X3 * X4 < 0))
      K[zz*ps + yy*rs + xx] = NDG_CARRE3DXZ;
    //carreyz
    X1 = a * (x-0.5) + b * (y-0.5) + c * (z-0.5) + d;
    X2 = a * (x-0.5) + b * (y-0.5) + c * (z+0.5) + d;
    X3 = a * (x-0.5) + b * (y+0.5) + c * (z-0.5) + d;
    X4 = a * (x-0.5) + b * (y+0.5) + c * (z+0.5) + d;
    xx = 2 * x; yy = 2 * y + 1; zz = 2 * z + 1; 
    if ((X1 * X2 < 0) || (X1 * X3 < 0) || (X1 * X4 < 0) ||
        (X2 * X3 < 0) || (X2 * X4 < 0) || (X3 * X4 < 0))
      K[zz*ps + yy*rs + xx] = NDG_CARRE3DYZ;
  } // for z, y, x
  return 1;
} /* l3dplane() */

/* =============================================================== */
int32_t l3dsphere(struct xvimage * k, index_t x0, index_t y0, index_t z0, double r)
/* =============================================================== */
/* 
  Draws into the Khalimsky volume \b k, the discretized sphere of center 
  \b x0, \b y0, \b z0 and of radius \b r.
*/
#undef F_NAME
#define F_NAME "l3dsphere"
{
  index_t rs, cs, ps, ds, N;
  uint8_t * K;
  index_t x, y, z;             // coordinates in the continuous plane
  index_t xmin, ymin, zmin, xmax, ymax, zmax;
  index_t xx, yy, zz, x00, y00, z00; // coord. in the khalimsky space
  double t, t_, r2 = r*r;
  index_t tab[27]; int32_t i, n;

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  x00 = x0 * 2;
  y00 = y0 * 2;
  z00 = z0 * 2;
  zmin = ymin = xmin = -((index_t)r+1);
  zmax = ymax = xmax = (index_t)r+1;

#define D2(x,y,z) (double)((x)*(x)+(y)*(y)+(z)*(z))

  for (z = zmin; z <= zmax; z++)
  for (y = ymin; y <= ymax; y++)
  {
    zz = z * 2 + z00;
    yy = y * 2 + y00;
    for (x = xmin; x < 0 ; x++)
    {
      xx = x * 2 + x00;
      t_ = D2(x,y,z);
      t = D2((x+1),y,z);
      if (t_ == r2) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
      }
      if ((t_ > r2) && (t < r2)) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx+1 >= 0) && (xx+1 < rs))
          K[zz*ps + yy*rs + xx+1] = NDG_INTER3DX;
        Betacarre3d(rs, cs, ds, xx+1, yy, zz, tab, &n);
        for (i = 0; i < n; i++) K[tab[i]] = NDG_MAX;          
      }
      t_ = t;
    } // for x 
    for (x = 0; x < xmax ; x++)
    {
      xx = x * 2 + x00;
      t_ = D2(x,y,z);
      t = D2((x+1),y,z);
      if (t_ == r2) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
      }
      if ((t_ < r2) && (t > r2)) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx+1 >= 0) && (xx+1 < rs))
          K[zz*ps + yy*rs + xx+1] = NDG_INTER3DX;
        Betacarre3d(rs, cs, ds, xx+1, yy, zz, tab, &n);
        for (i = 0; i < n; i++) K[tab[i]] = NDG_MAX;          
      }
      t_ = t;
    } // for x 

    if (t == r2) 
    {
      xx = xmax * 2 + x00;
      if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
        K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
    }

  } // for y, z

  for (y = ymin; y <= ymax; y++)
  for (x = xmin; x <= xmax; x++)
  {
    xx = x * 2 + x00;
    yy = y * 2 + y00;
    for (z = zmin; z < 0 ; z++)
    {
      zz = z * 2 + z00;
      t_ = D2(x,y,z);
      t = D2(x,y,(z+1));
      if (t_ == r2) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
      }
      if ((t_ > r2) && (t < r2)) 
      {
        if ((zz+1 >= 0) && (zz+1 < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[(zz+1)*ps + yy*rs + xx] = NDG_INTER3DZ;
        Betacarre3d(rs, cs, ds, xx, yy, zz+1, tab, &n);
        for (i = 0; i < n; i++) K[tab[i]] = NDG_MAX;          
      }
      t_ = t;
    } // for z
    for (z = 0; z < zmax ; z++)
    {
      zz = z * 2 + z00;
      t_ = D2(x,y,z);
      t = D2(x,y,(z+1));
      if (t_ == r2) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
      }
      if ((t_ < r2) && (t > r2)) 
      {
        if ((zz+1 >= 0) && (zz+1 < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[(zz+1)*ps + yy*rs + xx] = NDG_INTER3DZ;
        Betacarre3d(rs, cs, ds, xx, yy, zz+1, tab, &n);
        for (i = 0; i < n; i++) K[tab[i]] = NDG_MAX;          
      }
      t_ = t;
    } // for z

    if (t == r2) 
    {
      zz = zmax * 2 + z00;
      if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
        K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
    }

  } // for y, x

  for (z = zmin; z <= zmax; z++)
  for (x = xmin; x <= xmax; x++)
  {
    zz = z * 2 + z00;
    xx = x * 2 + x00;
    for (y = ymin; y < 0 ; y++)
    {
      yy = y * 2 + y00;
      t_ = D2(x,y,z);
      t = D2(x,(y+1),z);
      if (t_ == r2) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
      }
      if ((t_ > r2) && (t < r2)) 
      {
        if ((zz >= 0) && (zz < ds) && (yy+1 >= 0) && (yy+1 < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + (yy+1)*rs + xx] = NDG_INTER3DY;
        Betacarre3d(rs, cs, ds, xx, yy+1, zz, tab, &n);
        for (i = 0; i < n; i++) K[tab[i]] = NDG_MAX;          
      }
      t_ = t;
    } // for y
    for (y = 0; y < ymax ; y++)
    {
      yy = y * 2 + y00;
      t_ = D2(x,y,z);
      t = D2(x,(y+1),z);
      if (t_ == r2) 
      {
        if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
      }
      if ((t_ < r2) && (t > r2)) 
      {
        if ((zz >= 0) && (zz < ds) && (yy+1 >= 0) && (yy+1 < cs) && (xx >= 0) && (xx < rs))
          K[zz*ps + (yy+1)*rs + xx] = NDG_INTER3DY;
        Betacarre3d(rs, cs, ds, xx, yy+1, zz, tab, &n);
        for (i = 0; i < n; i++) K[tab[i]] = NDG_MAX;          
      }
      t_ = t;
    } // for y

    if (t == r2) 
    {
      yy = ymax * 2 + y00;
      if ((zz >= 0) && (zz < ds) && (yy >= 0) && (yy < cs) && (xx >= 0) && (xx < rs))
        K[zz*ps + yy*rs + xx] = NDG_SINGL3D;
    }

  } // for x, z
  return 1;
} /* l3dsphere() */

/* =============================================================== */
int32_t l3dthin(struct xvimage * k, int32_t nsteps)
/* =============================================================== */
/* 
   amincissement homotopique 3d dans la grille de Khalimsky 
*/
#undef F_NAME
#define F_NAME "l3dthin"
{
  struct xvimage * kp;
  int32_t stablealpha, stablebeta, i;
  index_t x, y, z;
  index_t rs, cs, ps, d, N;
  uint8_t * K;
  uint8_t * KP;

  rs = rowsize(k);
  cs = colsize(k);
  d = depth(k);
  ps = rs * cs;
  N = ps * d;
  K = UCHARDATA(k);

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  InitPileGrilles3d();

  kp = copyimage(k);
  if (kp == NULL)
  {   fprintf(stderr,"%s : copyimage failed\n", F_NAME);
      return 0;
  }  
  KP = UCHARDATA(kp);

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 1; i <= nsteps; i++)
  {
#ifdef VERBOSE
    fprintf(stderr, "step %d\n", i);
#endif
    if (i % 2)
    {
      stablealpha = 1;
      for (z = 0; z < d; z++)
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (K[z * ps + y * rs + x] && Alpha3Simple3d(k, x, y, z))
          { KP[z * ps + y * rs + x] = 0; stablealpha = 0; }
      memcpy(K, KP, N);
    }
    else
    {
      stablebeta = 1;
      for (z = 0; z < d; z++)
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (K[z * ps + y * rs + x] && Beta3Simple3d(k, x, y, z))
          { KP[z * ps + y * rs + x] = 0; stablebeta = 0; }
      memcpy(K, KP, N);
    }
    if (stablealpha && stablebeta) break;
  }

  TerminePileGrilles3d();
  freeimage(kp);
  return 1;

} /* l3dthin() */

/* =============================================================== */
int32_t l3dskelsurf(struct xvimage * k, int32_t nsteps)
/* =============================================================== */
/* 
   squelette surfacique 3d dans la grille de Khalimsky 
*/
#undef F_NAME
#define F_NAME "l3dskelsurf"
{
  struct xvimage * kp;
  int32_t stablealpha, stablebeta, i;
  index_t x, y, z;
  index_t rs, cs, ps, d, N;
  uint8_t * K;
  uint8_t * KP;

  rs = rowsize(k);
  cs = colsize(k);
  d = depth(k);
  ps = rs * cs;
  N = ps * d;
  K = UCHARDATA(k);

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  InitPileGrilles3d();

  kp = copyimage(k);
  if (kp == NULL)
  {   fprintf(stderr,"%s : copyimage failed\n", F_NAME);
      return 0;
  }  
  KP = UCHARDATA(kp);

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 1; i <= nsteps; i++)
  {
#ifdef VERBOSE
    fprintf(stderr, "step %d\n", i);
#endif
    if (i % 2)
    {
      stablealpha = 1;
      for (z = 0; z < d; z++)
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (K[z * ps + y * rs + x] && Alpha3Simple3d(k, x, y, z) && !Surfend3d(k, x, y, z))
          { KP[z * ps + y * rs + x] = 0; stablealpha = 0; }
      memcpy(K, KP, N);
    }
    else
    {
      stablebeta = 1;
      for (z = 0; z < d; z++)
      for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
        if (K[z * ps + y * rs + x] && Beta3Simple3d(k, x, y, z) && !Surfend3d(k, x, y, z))
          { KP[z * ps + y * rs + x] = 0; stablebeta = 0; }
      memcpy(K, KP, N);
    }
    if (stablealpha && stablebeta) break;
  }

  TerminePileGrilles3d();
  freeimage(kp);

  return 1;

} /* l3dskelsurf() */

/* =============================================================== */
int32_t l3disthmus(struct xvimage * f)
/* =============================================================== */
/* 
   detruit les isthmes 1D (T = 2 ; Tb = 1)
*/
#undef F_NAME
#define F_NAME "l3disthmus"
{
  struct xvimage * g;
  struct xvimage * fp;
  index_t rs, cs, ds, ps, N;
  index_t x;
  uint8_t *F;
  uint8_t *FP;

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  g = allocimage(NULL, 7, 7, 7, VFF_TYP_1_BYTE);
  if (g == NULL)
  {
    fprintf(stderr,"%s : malloc failed\n", F_NAME);
    exit(0);
  }

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  F = UCHARDATA(f);
  ps = rs * cs;
  N = ps * ds;

  fp = copyimage(f);
  if (fp == NULL)
  {   fprintf(stderr,"%s : copyimage failed\n", F_NAME);
      return 0;
  }  
  FP = UCHARDATA(fp);

  for (x = 0; x < N; x++)
  {
    if (FP[x]) 
    {
      int32_t tb;
      int32_t t = T3d(fp, x % rs, (x % ps) / rs, x / ps, g);
      if (t == 2)
      {
        tb = Tbar3d(fp, x % rs, (x % ps) / rs, x / ps, g);
        if (tb == 1) F[x] = 0;
      }
#ifdef DEBUGISTHMUS
      printf("point %d %d %d : t = %d ; [tb = %d](si t==2) ; new val = %d\n",
              x % rs, (x % ps) / rs, x / ps, t, tb, F[x]);
#endif
    }
  } 
  
  freeimage(g);
  freeimage(fp);
  return 1;
} /* l3disthmus() */

/* =============================================================== */
int32_t l3dlabel(struct xvimage * f, struct xvimage * lab)
/* =============================================================== */
/*
  Etiquette les regions theta-connexes de l'image f. 
  Resultat dans lab (image "longint", allouee a l'avance)
*/
#undef F_NAME
#define F_NAME "l3dlabel"
{
  index_t rs, cs, ds, ps, N;
  index_t x, y, w;
  uint8_t *F;
  int32_t *LAB;
  index_t nlabels = 0;
  Lifo * LIFO;
  index_t tab[27]; int32_t n, k;

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  ps = rs * cs;
  N = ps * ds;
  F = UCHARDATA(f);
  LAB = SLONGDATA(lab);

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

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
      return(0);
  }
  
  for (x = 0; x < N; x++)
  {
    if (F[x] && !LAB[x]) 
    {
      nlabels += 1;
      LAB[x] = nlabels;
      LifoPush(LIFO, x);
      /* propage le label dans la region connexe */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        Alphacarre3d(rs, cs, ds, w%rs, (w%ps)/rs, w/ps, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
        Betacarre3d(rs, cs, ds, w%rs, (w%ps)/rs, w/ps, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
    } /* if (F[x] && !LAB[x]) */
  } /* for (x = 0; x < N; x++) */

#ifdef VERBOSE
  fprintf(stderr, "%s : %d composantes trouvees\n", F_NAME, nlabels);
#endif

  LifoTermine(LIFO);
  return 1;
} /* l3dlabel() */

/* =============================================================== */
int32_t l3drecons(struct xvimage * f, index_t *tab, int32_t n)
/* =============================================================== */
/* 
  Reconstruction geodesique (au sens du theta-voisinage) de l'ensemble
  represente par la liste de points dans (tab, n) dans l'ensemble 
  represente par l'image binaire f.
*/
#undef F_NAME
#define F_NAME "l3drecons"
{
  struct xvimage * fp;
  index_t rs, cs, ds, ps, N;
  index_t x, y;
  uint8_t *F;
  uint8_t *FP;
  Lifo * LIFO1;
  Lifo * LIFO2;
  Lifo * LIFO3;
  index_t vois[27];
  int32_t nv, i;

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  F = UCHARDATA(f);
  ps = rs * cs;
  N = ps * ds;

  fp = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if (fp == NULL)
  {   
    fprintf(stderr,"%s : allocimage failed\n", F_NAME);
    return(0);
  }
  FP = UCHARDATA(fp);
  memset(FP, 0, N);

  LIFO1 = CreeLifoVide(N/16);     /* insuffisant en theorie, mais rarement en pratique */
  LIFO2 = CreeLifoVide(N/16);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  { 
    fprintf(stderr,"%s : CreeLifoVide failed\n", F_NAME);
    return(0);
  }

  for (i = 0; i < n; i++)
  {
#ifdef PARANO
    if (tab[i] >= N)
    { 
      fprintf(stderr,"%s : point coord. out of range\n", F_NAME);
      return(0);
    }
#endif
    FP[tab[i]] = 255;
    LifoPush(LIFO1, tab[i]);
  }

  while (!LifoVide(LIFO1))
  {
    while (!LifoVide(LIFO1))
    {
      x = LifoPop(LIFO1);
      Alphacarre3d(rs, cs, ds, x%rs, (x%ps)/rs, x/ps, vois, &nv);
      for (i = 0; i < nv; i++)
      {
        y = vois[i];
        if (F[y] && !FP[y])
        {
          FP[y] = 255;
          LifoPush(LIFO2, y);
	}
      }
      Betacarre3d(rs, cs, ds, x%rs, (x%ps)/rs, x/ps, vois, &nv);
      for (i = 0; i < nv; i++)
      {
        y = vois[i];
        if (F[y] && !FP[y])
        {
          FP[y] = 255;
          LifoPush(LIFO2, y);
	}
      }
    } /* while (!LifoVide(LIFO1)) */
    LIFO3 = LIFO2;
    LIFO2 = LIFO1;
    LIFO1 = LIFO3;
  } /* while (!LifoVide(LIFO1)) */
  
  memcpy(UCHARDATA(f), UCHARDATA(fp), N);
  freeimage(fp);
  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return 1;
} /* l3drecons() */


/* =============================================================== */
int32_t l3dinvariants(struct xvimage *f, index_t *nbcc, index_t *nbcav, index_t *nbtun, index_t *euler)
/* =============================================================== */
/*
  Calculs des nombres de composantes connexes, cavites et tunnels.
*/
#undef F_NAME
#define F_NAME "l3dinvariants"
{
  index_t rs, cs, ds, ps, N;
  index_t x, y, w;
  uint8_t *F;
  struct xvimage * lab;
  int32_t *LAB;
  index_t nlabels;
  Lifo * LIFO;
  index_t tab[27]; int32_t n, k;

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  ps = rs * cs;
  N = ps * ds;
  F = UCHARDATA(f);
  lab = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(0);
    
  }
  LAB = SLONGDATA(lab);

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {  
    fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
    return(0);
  }
  
  /* ============================================= */
  /* calcul de la caracteristique d'Euler-Poincare */
  /* ============================================= */
  
  *euler = EulerKh3d(f);

  /* ======================================== */
  /* calcul du nombre de composantes connexes */
  /* ======================================== */

  nlabels = 0;
  for (x = 0; x < N; x++)
  {
    if (F[x] && !LAB[x]) 
    {
      nlabels += 1;
      LAB[x] = nlabels;
      LifoPush(LIFO, x);
      /* propage le label dans la region connexe */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        Alphacarre3d(rs, cs, ds, w%rs, (w%ps)/rs, w/ps, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
        Betacarre3d(rs, cs, ds, w%rs, (w%ps)/rs, w/ps, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
    } /* if (F[x] && !LAB[x]) */
  } /* for (x = 0; x < N; x++) */
  *nbcc = nlabels;
  
  /* ======================================== */
  /* calcul du nombre de cavites */
  /* ======================================== */

  for (x = 0; x < N; x++) F[x] = ((F[x]==0) ? 255 : 0);  

  nlabels = 0;
  for (x = 0; x < N; x++)
  {
    if (F[x] && !LAB[x]) 
    {
      nlabels += 1;
      LAB[x] = nlabels;
      LifoPush(LIFO, x);
      /* propage le label dans la region connexe */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        Alphacarre3d(rs, cs, ds, w%rs, (w%ps)/rs, w/ps, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels alpha-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
        Betacarre3d(rs, cs, ds, w%rs, (w%ps)/rs, w/ps, tab, &n);
        for (k = 0; k < n; k++) /* parcourt les eventuels beta-voisins */
        {
          y = tab[k];
          if (F[y] && !LAB[y])
          {
            LAB[y] = nlabels;
            LifoPush(LIFO, y);
          } /* if (F[y] ... */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
    } /* if (F[x] && !LAB[x]) */
  } /* for (x = 0; x < N; x++) */
  *nbcav = nlabels - 1;
  *nbtun =  *nbcc - *euler + *nbcav;

  LifoTermine(LIFO);
  return 1;
} /* l3dinvariants() */

/* =============================================================== */
int32_t l3dboundary(struct xvimage * f)
/* =============================================================== */
/* 
   extrait la frontière interne
   def: {x in F | theta(x) inter Fbar neq emptyset}
*/
{
#undef F_NAME
#define F_NAME "l3dboundary"
  struct xvimage * g;
  index_t rs, cs, ds, ps, N;
  index_t x, y, z;
  uint8_t *F;
  uint8_t *G;
  index_t tab[26];
  int32_t n, u;

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  F = UCHARDATA(f);
  ps = rs * cs;
  N = ps * ds;

  g = copyimage(f);
  if (g == NULL)
  {   fprintf(stderr,"%s: copyimage failed\n", F_NAME);
      return 0;
  }  
  G = UCHARDATA(g);
  memset(F, 0, N);

  for (z = 0; z < ds; z++)
    for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
	if (G[z*ps + y*rs + x])
        {
	  Thetacarre3d(rs, cs, ds, x, y, z, tab, &n);
	  for (u = 0; u < n; u++)
	    if (G[tab[u]] == 0) 
	    {
	      F[z*ps + y*rs + x] = NDG_MAX;
	      goto next;
	    }
	next:;
	} 
  
  freeimage(g);
  return 1;
} /* l3dboundary() */

/* =============================================================== */
int32_t l3dborder(struct xvimage * f)
/* =============================================================== */
/* 
   extrait la frontière interne
   def: closure{x in F | x free for F}
*/
{
#undef F_NAME
#define F_NAME "l3dborder"
  struct xvimage * g;
  index_t rs, cs, ds, ps;
  index_t x, y, z;
  uint8_t *F;
  uint8_t *G;

  assert(datatype(f) == VFF_TYP_1_BYTE);
  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  ps = rs * cs;
  F = UCHARDATA(f);
  g = copyimage(f);
  if (g == NULL)
  {   fprintf(stderr,"%s: copyimage failed\n", F_NAME);
      return 0;
  }  
  G = UCHARDATA(g);
  razimage(f);
  for (z = 0; z < ds; z++)
    for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
	if (G[z*ps + y*rs + x] && FaceLibre3d(g, x, y, z))
	  F[z*ps + y*rs + x] = VAL_OBJET;
  l3dmakecomplex(f);
  freeimage(g);
  return 1;
} /* l2dborder() */

/* =============================================================== */
int32_t l3dseltype(struct xvimage * k, uint8_t d1, uint8_t d2, uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2)
/* =============================================================== */
/* 
  Selects the elements x of the Khalimsky volume \b k 
  which satisfy the following inequalities : 
  \b a1 <= a(x) <= \b a2
  \b b1 <= b(x) <= \b b2
  \b d1 <= d(x) <= \b d2
  where
  d(x) = dimension of x
  a(x) = number of elements under x of dimension d(x) - 1
  b(x) = number of elements over x of dimension d(x) + 1
*/
#undef F_NAME
#define F_NAME "l3dseltype"
{
  index_t rs, cs, ps, ds, N, i1, j1, k1, i2, j2, k2, x, y;
  uint8_t * K;
  struct xvimage * kp;
  uint8_t * KP;
  index_t tab[27]; int32_t n, u, a, b, d;

  rs = rowsize(k);
  cs = colsize(k);
  ds = depth(k);
  ps = rs * cs;
  N = ps * ds;
  K = UCHARDATA(k);

  kp = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if (kp == NULL)
  {   
    fprintf(stderr,"%s : allocimage failed\n", F_NAME);
    return(0);
  }
  KP = UCHARDATA(kp);
  memset(KP, 0, N);

  for (k1 = 0; k1 < ds; k1++)
  for (j1 = 0; j1 < cs; j1++)
  for (i1 = 0; i1 < rs; i1++)
  {
    x = k1*ps + j1*rs + i1;
    d = DIM3D(i1,j1,k1);
    if (K[x] && (d1 <= d) && (d <= d2))
    {
      Alphacarre3d(rs, cs, ds, i1, j1, k1, tab, &n);
      for (a = u = 0; u < n; u++) /* parcourt les eventuels alpha-voisins */
      {
	y = tab[u];
	i2 = y%rs; j2 = (y%ps)/rs; k2 = y/ps;
	if (K[y] && (DIM3D(i2,j2,k2) == (d-1))) a++;
      }
      if ((a1 <= a) && (a <= a2))
      {
	Betacarre3d(rs, cs, ds, i1, j1, k1, tab, &n);
	for (b = u = 0; u < n; u++) /* parcourt les eventuels beta-voisins */
	{
	  y = tab[u];
	  i2 = y%rs; j2 = (y%ps)/rs; k2 = y/ps;
	  if (K[y] && (DIM3D(i2,j2,k2) == (d+1))) b++;
	}
	if ((b1 <= b) && (b <= b2))
	{
	  KP[x] = NDG_MAX;
	}
      }
    }
  } // for k1, j1, i1
  for (x = 0; x < N; x++) K[x] = KP[x];
  freeimage(kp);
  return 1;
} /* l3dseltype() */
