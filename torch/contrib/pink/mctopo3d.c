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
Librairie mctopo3D : 

Calcul des nombres topologiques en 3D

Version calculant les nombres de connexité T et Tb directement
d'après la definition de G. Bertrand [Ber94].

[Ber94] G. Bertrand, "Simple points, topological numbers and geodesic
neighborhoods in cubic grids", Pattern Recognition Letters, 
Vol. 15, pp. 1003-1011, 1994.

Michel Couprie 1998-2007

Update nov. 2006 : modif mctopo3d_geodesic_neighborhood pour compatibilité 64 bits
Update nov. 2007 : modif mctopo3d_nbcomp pour compatibilité 64 bits
Update sep. 2009 : ajout des tests is_on_frame()
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mclifo.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <mctopo3d.h>

/* globales privees */
static Lifo * LIFO_topo3d1 = NULL;
static Lifo * LIFO_topo3d2 = NULL;
static voxel cube_topo3d[27];
static voxel cubec_topo3d[27];


static inline int32_t is_on_frame(index_t p, index_t rs, index_t ps, index_t N)
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 1;
  else
    return 0;
}
  
/* ========================================== */
void mctopo3d_init_topo3d()
/* ========================================== */
#undef F_NAME
#define F_NAME "mctopo3d_init_topo3d"
{
  LIFO_topo3d1 = CreeLifoVide(27);
  LIFO_topo3d2 = CreeLifoVide(27);
  if ((LIFO_topo3d1 == NULL) || (LIFO_topo3d2 == NULL))
  {   
    fprintf(stderr, "mccube() : CreeLifoVide failed\n");
    exit(0);
  }
  mctopo3d_construitcube(cube_topo3d);
  mctopo3d_construitcube(cubec_topo3d);
} /* mctopo3d_init_topo3d() */

/* ========================================== */
void mctopo3d_termine_topo3d()
/* ========================================== */
{
  LifoTermine(LIFO_topo3d1);
  LifoTermine(LIFO_topo3d2);
} /* mctopo3d_termine_topo3d() */

/* ========================================== */
void mctopo3d_construitcube(voxel * cube)
/* ========================================== */
{
  uint8_t x,y,z,u,v,w;
  pvoxel p;
  for (z = 0; z < 3; z++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
      {
      	p = &(cube[encode(x,y,z)]);
        p->x = x;
        p->y = y;
        p->z = z;
        p->n = encode(x,y,z);
      	p->val = 0;
      	p->lab = 0;
      	p->lab2 = 0;
      	p->n6v = 0;
      	p->n12v = 0;
      	p->n8v = 0;
      	p->n18v = 0;
      	p->n26v = 0;

        if ((x == 1) && (y == 1) && (z == 1))  p->type = centre;
        else
          if (mcabs(1-x)+mcabs(1-y)+mcabs(1-z) == 1) p->type = face;
          else
            if (mcabs(1-x)+mcabs(1-y)+mcabs(1-z) <= 2) p->type = arete;
            else
              p->type = coin;

      	for (w = 0; w < 3; w++)
       	  for (v = 0; v < 3; v++)
      	    for (u = 0; u < 3; u++)
      	    {
              if (mcabs(u-x)+mcabs(v-y)+mcabs(w-z) == 1)
	      {
                p->v6[p->n6v++] = &(cube[encode(u,v,w)]);
                p->v18[p->n18v++] = &(cube[encode(u,v,w)]);
                p->v26[p->n26v++] = &(cube[encode(u,v,w)]);
	      }
              else
              if (mcmax(mcabs(u-x), mcmax(mcabs(v-y), mcabs(w-z))) == 1)
              {
                if (mcabs(u-x)+mcabs(v-y)+mcabs(w-z) <= 2)
		{
                  p->v12[p->n12v++] = &(cube[encode(u,v,w)]);
                  p->v18[p->n18v++] = &(cube[encode(u,v,w)]);
                  p->v26[p->n26v++] = &(cube[encode(u,v,w)]);
		}
                else 
		{
                  p->v8[p->n8v++] = &(cube[encode(u,v,w)]);
                  p->v26[p->n26v++] = &(cube[encode(u,v,w)]);
		}
              }
      	    }  /* for w v u */
      } /* for z y x */
} /* mctopo3d_construitcube() */

/* ========================================== */
uint32_t mctopo3d_encodecube()
/* ========================================== */
{
  uint8_t n;
  pvoxel p;
  uint32_t i = 0;

  for (n = 0; n < 27; n++)
  {
    p = &(cube_topo3d[n]);
    if (p->val) i = i | (1<<n);        
  } /* for n */
  return i;
} /* mctopo3d_encodecube() */

/* ========================================== */
void mctopo3d_geodesic_neighborhood(voxel * cube, uint8_t connex, uint8_t s)
/* ========================================== */
#undef F_NAME
#define F_NAME "mctopo3d_geodesic_neighborhood("
/* 
  met a 1 le champ lab des points appartenant au voisinage geodesique d'ordre s du point central,
  met a 0 les autres
*/
{
  uint8_t n;
  pvoxel p, pp, pc;
  Lifo * LIFOtmp;
  
  if ((LIFO_topo3d1 == NULL) || (LIFO_topo3d2 == NULL))
  { 
    fprintf(stderr, "%s: LIFO_topo3d1 and LIFO_topo3d2 must be allocated\n", F_NAME); 
    exit(0); 
  }

  if (s < 1) 
  { 
    fprintf(stderr, "%s: order %d not allowed (must be > 0)\n", F_NAME, s); 
    exit(0); 
  }
  
  /* met a 0 le champ lab des points du cube */
  for (n = 0; n < 27; n++) cube[n].lab = 0;

  /* met a 1 le champ lab des voisins de valeur 1 du point central pc */
  pc = &(cube[13]);
  for (n = 0; n < pc->n6v; n++)
  {
    p = pc->v6[n];
    if (p->val == 1)
    { p->lab = 1; LifoPush(LIFO_topo3d1, (int32_t)(p-pc)); }
  }
  if (connex > 6)
    for (n = 0; n < pc->n12v; n++)
    {
      p = pc->v12[n];
      if (p->val == 1)
      { p->lab = 1; LifoPush(LIFO_topo3d1, (int32_t)(p-pc)); }
    }
  if (connex > 18)
    for (n = 0; n < pc->n8v; n++)
    {
      p = pc->v8[n];
      if (p->val == 1)
      { p->lab = 1; LifoPush(LIFO_topo3d1, (int32_t)(p-pc)); }
    }
  s--;

  while (s > 0)
  {
    while (!LifoVide(LIFO_topo3d1))
    {
      p = pc + LifoPop(LIFO_topo3d1);
      /* met a 1 le champ lab des voisins de valeur 1 du point p (sauf pc) */
      for (n = 0; n < p->n6v; n++)
      {
        pp = p->v6[n];
        if ((pp != pc) && (pp->val == 1) && (pp->lab == 0))
        { pp->lab = 1; LifoPush(LIFO_topo3d2, (int32_t)(pp-pc)); }
      }
      if (connex > 6)
        for (n = 0; n < p->n12v; n++)
        {
          pp = p->v12[n];
          if ((pp != pc) && (pp->val == 1) && (pp->lab == 0))
          { pp->lab = 1; LifoPush(LIFO_topo3d2, (int32_t)(pp-pc)); }
        }
      if (connex > 18)
        for (n = 0; n < p->n8v; n++)
        {
          pp = p->v8[n];
          if ((pp != pc) && (pp->val == 1) && (pp->lab == 0))
          { pp->lab = 1; LifoPush(LIFO_topo3d2, (int32_t)(pp-pc)); }
        }
    } /* while (!LifoVide(LIFO_topo3d1)) */
    s--;
    LIFOtmp = LIFO_topo3d1;
    LIFO_topo3d1 = LIFO_topo3d2;
    LIFO_topo3d2 = LIFOtmp;
  } /* while (s > 0) */

  LifoFlush(LIFO_topo3d1);
  
} /* mctopo3d_geodesic_neighborhood() */

/* ========================================== */
void mctopo3d_G6(voxel * cube)
/* ========================================== */
{
  mctopo3d_geodesic_neighborhood(cube, 6, 2);	
} /* mctopo3d_G6() */

/* ========================================== */
void mctopo3d_G6p(voxel * cube)
/* ========================================== */
{
  mctopo3d_geodesic_neighborhood(cube, 6, 3);	
} /* mctopo3d_G6p() */

/* ========================================== */
void mctopo3d_G18(voxel * cube)
/* ========================================== */
{
  mctopo3d_geodesic_neighborhood(cube, 18, 2);	
} /* mctopo3d_G18() */

/* ========================================== */
void mctopo3d_G26(voxel * cube)
/* ========================================== */
{
  mctopo3d_geodesic_neighborhood(cube, 26, 1);	
} /* mctopo3d_G26() */

/* ========================================== */
uint8_t mctopo3d_nbcomp(voxel * cube, uint8_t connex)
/* ========================================== */
/*
  retourne le nombre de composantes connexes de l'objet marque par un lab=1 
*/
{
  uint8_t ncc;
  uint32_t n, v, m;
  pvoxel p,pp;

  ncc = 0;
  for (n = 0; n < 27; n++) cube[n].lab2 = 0;
  for (n = 0; n < 27; n++)
  {
    p = &(cube[n]);
    if ((p->lab == 1) && (p->lab2 == 0))
    {
      ncc++;
      p->lab2 = ncc;
      LifoPush(LIFO_topo3d1, n);
      while (!LifoVide(LIFO_topo3d1))
      {
	m = LifoPop(LIFO_topo3d1);
	p = &(cube[m]);
        for (v = 0; v < p->n6v; v++)
        {
          pp = p->v6[v];
          if ((pp->lab == 1) && (pp->lab2 == 0))
	  {
            pp->lab2 = ncc;
            LifoPush(LIFO_topo3d1, pp->n);
	  }
        } /* for v */
        if (connex > 6)
          for (v = 0; v < p->n12v; v++)
          {
            pp = p->v12[v];
            if ((pp->lab == 1) && (pp->lab2 == 0)) 
	    {
              pp->lab2 = ncc;
              LifoPush(LIFO_topo3d1, pp->n);
	    }
          } /* for v */
        if (connex > 18)
          for (v = 0; v < p->n8v; v++)
          {
            pp = p->v8[v];
            if ((pp->lab == 1) && (pp->lab2 == 0))
	    {
              pp->lab2 = ncc;
              LifoPush(LIFO_topo3d1, pp->n);
	    }
          } /* for v */
      } /* while (!LifoVide(LIFO_topo3d1)) */
    } /* if */
  } /* for n */

  return ncc;
} /* mctopo3d_nbcomp() */

/* ========================================== */
uint8_t mctopo3d_nbvois6(voxel * cube)
/* ========================================== */
/*
  retourne le nombre de 6-voisins du point central appartenant a l'objet
*/
{
  uint8_t v, mctopo3d_nbvois;
  pvoxel p,pp;

  mctopo3d_nbvois = 0;
  p = &(cube[13]);
  for (v = 0; v < p->n6v; v++)
  {
    pp = p->v6[v];
    if (pp->val) mctopo3d_nbvois++;
  }

  return mctopo3d_nbvois;
} /* mctopo3d_nbvois6() */

/* ========================================== */
uint8_t mctopo3d_nbvois26(voxel * cube)
/* ========================================== */
/*
  retourne le nombre de 26-voisins du point central appartenant a l'objet
*/
{
  uint8_t v, mctopo3d_nbvois;
  pvoxel p,pp;

  mctopo3d_nbvois = 0;
  p = &(cube[13]);
  for (v = 0; v < p->n26v; v++)
  {
    pp = p->v26[v];
    if (pp->val) mctopo3d_nbvois++;
  }

  return mctopo3d_nbvois;
} /* mctopo3d_nbvois26() */

/* ========================================== */
int32_t mctopo3d_nbvoisc6(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 6-voisins du point central de niveau nul
*/
{
  int32_t mctopo3d_nbvois = 0;
  if ((i%rs!=rs-1) && !B[i+1])    mctopo3d_nbvois++;
  if (((i%ps)>=rs) && !B[i-rs])   mctopo3d_nbvois++;
  if ((i%rs!=0) && !B[i-1])       mctopo3d_nbvois++;
  if (((i%ps)<ps-rs) && !B[i+rs]) mctopo3d_nbvois++;
  if ((i>=ps) && !B[i-ps])        mctopo3d_nbvois++;
  if ((i<N-ps) && !B[i+ps])      mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoisc6() */

/* ========================================== */
int32_t mctopo3d_nbvoisc18(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 18-voisins du point central de niveau nul
*/
{
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && !B[ps+i+1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && !B[ps+i-rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && !B[ps+i-1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && !B[ps+i+rs]) mctopo3d_nbvois++;
  if (((i<N-ps)) && !B[ps+i]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && !B[i+1]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && !B[i+1-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && !B[i-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && !B[i-rs-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)) && !B[i-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && !B[i-1+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && !B[i+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && !B[i+rs+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && !B[-ps+i+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && !B[-ps+i-rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && !B[-ps+i-1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && !B[-ps+i+rs]) mctopo3d_nbvois++;
  if (((i>=ps)) && !B[-ps+i]) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoisc18() */

/* ========================================== */
int32_t mctopo3d_nbvoisc26(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 26-voisins du point central de niveau nul
*/
{
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && !B[ps+i+1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && !B[ps+i+1-rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && !B[ps+i-rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)&&(i%rs!=0)) && !B[ps+i-rs-1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && !B[ps+i-1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && !B[ps+i-1+rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && !B[ps+i+rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && !B[ps+i+rs+1]) mctopo3d_nbvois++;
  if (((i<N-ps)) && !B[ps+i]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && !B[i+1]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && !B[i+1-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && !B[i-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && !B[i-rs-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)) && !B[i-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && !B[i-1+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && !B[i+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && !B[i+rs+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && !B[-ps+i+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && !B[-ps+i+1-rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && !B[-ps+i-rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)&&(i%rs!=0)) && !B[-ps+i-rs-1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && !B[-ps+i-1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && !B[-ps+i-1+rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && !B[-ps+i+rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && !B[-ps+i+rs+1]) mctopo3d_nbvois++;
  if (((i>=ps)) && !B[-ps+i]) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoisc26() */

/* ========================================== */
int32_t mctopo3d_nbvoiso6(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 6-voisins du point central de niveau NON nul
*/
{
  int32_t mctopo3d_nbvois = 0;
  if ((i%rs!=rs-1) && B[i+1])    mctopo3d_nbvois++;
  if (((i%ps)>=rs) && B[i-rs])   mctopo3d_nbvois++;
  if ((i%rs!=0) && B[i-1])       mctopo3d_nbvois++;
  if (((i%ps)<ps-rs) && B[i+rs]) mctopo3d_nbvois++;
  if ((i>=ps) && B[i-ps])        mctopo3d_nbvois++;
  if ((i<N-ps) && B[i+ps])      mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoiso6() */

/* ========================================== */
int32_t mctopo3d_nbvoiso18(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 18-voisins du point central de niveau NON nul
*/
{
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && B[ps+i+1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && B[ps+i-rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && B[ps+i-1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && B[ps+i+rs]) mctopo3d_nbvois++;
  if (((i<N-ps)) && B[ps+i]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && B[i+1]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && B[i+1-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && B[i-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && B[i-rs-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)) && B[i-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && B[i-1+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && B[i+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && B[i+rs+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && B[-ps+i+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && B[-ps+i-rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && B[-ps+i-1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && B[-ps+i+rs]) mctopo3d_nbvois++;
  if (((i>=ps)) && B[-ps+i]) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoiso18() */

/* ========================================== */
int32_t mctopo3d_nbvoiso26(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 26-voisins du point central de niveau NON nul
*/
{
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && B[ps+i+1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && B[ps+i+1-rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && B[ps+i-rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)&&(i%rs!=0)) && B[ps+i-rs-1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && B[ps+i-1]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && B[ps+i-1+rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && B[ps+i+rs]) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && B[ps+i+rs+1]) mctopo3d_nbvois++;
  if (((i<N-ps)) && B[ps+i]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && B[i+1]) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && B[i+1-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && B[i-rs]) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && B[i-rs-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)) && B[i-1]) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && B[i-1+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && B[i+rs]) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && B[i+rs+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && B[-ps+i+1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && B[-ps+i+1-rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && B[-ps+i-rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)&&(i%rs!=0)) && B[-ps+i-rs-1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && B[-ps+i-1]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && B[-ps+i-1+rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && B[-ps+i+rs]) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && B[-ps+i+rs+1]) mctopo3d_nbvois++;
  if (((i>=ps)) && B[-ps+i]) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoiso26() */

/* ========================================== */
int32_t mctopo3d_nbvoislab6(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 6-voisins du point central de même label
*/
{
  int32_t lab = B[i];
  int32_t mctopo3d_nbvois = 0;
  if ((i%rs!=rs-1) && (B[i+1]==lab))    mctopo3d_nbvois++;
  if (((i%ps)>=rs) && (B[i-rs]==lab))   mctopo3d_nbvois++;
  if ((i%rs!=0) && (B[i-1]==lab))       mctopo3d_nbvois++;
  if (((i%ps)<ps-rs) && (B[i+rs]==lab)) mctopo3d_nbvois++;
  if ((i>=ps) && (B[i-ps]==lab))        mctopo3d_nbvois++;
  if ((i<N-ps) && (B[i+ps]==lab))      mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoislab6() */

/* ========================================== */
int32_t mctopo3d_nbvoislab18(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 18-voisins du point central de même label
*/
{
  int32_t lab = B[i];
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && (B[ps+i+1]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && (B[ps+i-rs]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && (B[ps+i-1]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && (B[ps+i+rs]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)) && (B[ps+i]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && (B[i+1]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && (B[i+1-rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && (B[i-rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && (B[i-rs-1]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)) && (B[i-1]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && (B[i-1+rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && (B[i+rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && (B[i+rs+1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && (B[-ps+i+1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && (B[-ps+i-rs]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && (B[-ps+i-1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && (B[-ps+i+rs]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)) && (B[-ps+i]==lab)) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoislab18() */

/* ========================================== */
int32_t mctopo3d_nbvoislab26(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 26-voisins du point central de même label
*/
{
  int32_t lab = B[i];
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && (B[ps+i+1]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && (B[ps+i+1-rs]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && (B[ps+i-rs]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)&&(i%rs!=0)) && (B[ps+i-rs-1]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && (B[ps+i-1]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && (B[ps+i-1+rs]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && (B[ps+i+rs]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && (B[ps+i+rs+1]==lab)) mctopo3d_nbvois++;
  if (((i<N-ps)) && (B[ps+i]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && (B[i+1]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && (B[i+1-rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && (B[i-rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && (B[i-rs-1]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)) && (B[i-1]==lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && (B[i-1+rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && (B[i+rs]==lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && (B[i+rs+1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && (B[-ps+i+1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && (B[-ps+i+1-rs]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && (B[-ps+i-rs]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)&&(i%rs!=0)) && (B[-ps+i-rs-1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && (B[-ps+i-1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && (B[-ps+i-1+rs]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && (B[-ps+i+rs]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && (B[-ps+i+rs+1]==lab)) mctopo3d_nbvois++;
  if (((i>=ps)) && (B[-ps+i]==lab)) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoislab26() */

/* ========================================== */
int32_t mctopo3d_nbvoislabc6(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 6-voisins du point central de label différent et non nul
*/
{
  int32_t lab = B[i];
  int32_t mctopo3d_nbvois = 0;
  if ((i%rs!=rs-1) && B[i+1] && (B[i+1]!=lab))    mctopo3d_nbvois++;
  if (((i%ps)>=rs) && B[i-rs] && (B[i-rs]!=lab))   mctopo3d_nbvois++;
  if ((i%rs!=0) && B[i-1] && (B[i-1]!=lab))       mctopo3d_nbvois++;
  if (((i%ps)<ps-rs) && B[i+rs] && (B[i+rs]!=lab)) mctopo3d_nbvois++;
  if ((i>=ps) && B[i-ps] && (B[i-ps]!=lab))        mctopo3d_nbvois++;
  if ((i<N-ps) && B[i+ps] && (B[i+ps]!=lab))      mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoislabc6() */

/* ========================================== */
int32_t mctopo3d_nbvoislabc18(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 18-voisins du point central de label différent et non nul
*/
{
  int32_t lab = B[i];
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && B[ps+i+1] && (B[ps+i+1]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && B[ps+i-rs] && (B[ps+i-rs]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && B[ps+i-1] && (B[ps+i-1]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && B[ps+i+rs] && (B[ps+i+rs]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)) && B[ps+i] && (B[ps+i]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && B[i+1] && (B[i+1]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && B[i+1-rs] && (B[i+1-rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && B[i-rs] && (B[i-rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && B[i-rs-1] && (B[i-rs-1]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)) && B[i-1] && (B[i-1]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && B[i-1+rs] && (B[i-1+rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && B[i+rs] && (B[i+rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && B[i+rs+1] && (B[i+rs+1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && B[-ps+i+1] && (B[-ps+i+1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && B[-ps+i-rs] && (B[-ps+i-rs]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && B[-ps+i-1] && (B[-ps+i-1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && B[-ps+i+rs] && (B[-ps+i+rs]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)) && B[-ps+i] && (B[-ps+i]!=lab)) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoislabc18() */

/* ========================================== */
int32_t mctopo3d_nbvoislabc26(
  int32_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ========================================== */
/*
  retourne le nombre de 26-voisins du point central de label différent et non nul
*/
{
  int32_t lab = B[i];
  int32_t mctopo3d_nbvois = 0;
  if (((i<N-ps)&&(i%rs!=rs-1)) && B[ps+i+1] && (B[ps+i+1]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && B[ps+i+1-rs] && (B[ps+i+1-rs]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)) && B[ps+i-rs] && (B[ps+i-rs]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps>=rs)&&(i%rs!=0)) && B[ps+i-rs-1] && (B[ps+i-rs-1]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)) && B[ps+i-1] && (B[ps+i-1]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && B[ps+i-1+rs] && (B[ps+i-1+rs]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)) && B[ps+i+rs] && (B[ps+i+rs]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && B[ps+i+rs+1] && (B[ps+i+rs+1]!=lab)) mctopo3d_nbvois++;
  if (((i<N-ps)) && B[ps+i] && (B[ps+i]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)) && B[i+1] && (B[i+1]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=rs-1)&&(i%ps>=rs)) && B[i+1-rs] && (B[i+1-rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)) && B[i-rs] && (B[i-rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps>=rs)&&(i%rs!=0)) && B[i-rs-1] && (B[i-rs-1]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)) && B[i-1] && (B[i-1]!=lab)) mctopo3d_nbvois++;
  if (((i%rs!=0)&&(i%ps<ps-rs)) && B[i-1+rs] && (B[i-1+rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)) && B[i+rs] && (B[i+rs]!=lab)) mctopo3d_nbvois++;
  if (((i%ps<ps-rs)&&(i%rs!=rs-1)) && B[i+rs+1] && (B[i+rs+1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)) && B[-ps+i+1] && (B[-ps+i+1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) && B[-ps+i+1-rs] && (B[-ps+i+1-rs]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)) && B[-ps+i-rs] && (B[-ps+i-rs]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps>=rs)&&(i%rs!=0)) && B[-ps+i-rs-1] && (B[-ps+i-rs-1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)) && B[-ps+i-1] && (B[-ps+i-1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%rs!=0)&&(i%ps<ps-rs)) && B[-ps+i-1+rs] && (B[-ps+i-1+rs]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)) && B[-ps+i+rs] && (B[-ps+i+rs]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) && B[-ps+i+rs+1] && (B[-ps+i+rs+1]!=lab)) mctopo3d_nbvois++;
  if (((i>=ps)) && B[-ps+i] && (B[-ps+i]!=lab)) mctopo3d_nbvois++;
  return mctopo3d_nbvois;
} /* mctopo3d_nbvoislabc26() */

/* ========================================== */
uint8_t mctopo3d_T6(voxel * cube)
/* ========================================== */
{
  mctopo3d_G6(cube);
  return mctopo3d_nbcomp(cube, 6);
} /* mctopo3d_T6() */

/* ========================================== */
uint8_t mctopo3d_T6p(voxel * cube)
/* ========================================== */
{
  mctopo3d_G6p(cube);
  return mctopo3d_nbcomp(cube, 6);
} /* mctopo3d_T6p() */

/* ========================================== */
uint8_t mctopo3d_T18(voxel * cube)
/* ========================================== */
{
  mctopo3d_G18(cube);
  return mctopo3d_nbcomp(cube, 18);
} /* mctopo3d_T18() */

/* ========================================== */
uint8_t mctopo3d_T26(voxel * cube)
/* ========================================== */
{
  mctopo3d_G26(cube);
  return mctopo3d_nbcomp(cube, 26);
} /* mctopo3d_T26() */

#ifdef __GNUC__
static uint8_t simple(voxel * cube, voxel * cubec, uint8_t connex) __attribute__ ((unused));
#endif
/* ========================================== */
static uint8_t simple(voxel * cube, voxel * cubec, uint8_t connex)
/* ========================================== */
#undef F_NAME
#define F_NAME "simple"
{
  switch (connex)
  {
    case 6: return (uint8_t)((mctopo3d_T6(cube) == 1) && (mctopo3d_T26(cubec) == 1));
    case 18: return (uint8_t)((mctopo3d_T18(cube) == 1) && (mctopo3d_T6p(cubec) == 1));
    case 26: return (uint8_t)((mctopo3d_T26(cube) == 1) && (mctopo3d_T6(cubec) == 1));
    default: 
      fprintf(stderr, "simple: mauvaise connexite : %d\n", connex); 
      exit(0); 
  } /* switch (connex) */
} /* simple() */

/* ==================================== */
static void preparecubes(
  uint8_t *B,            /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
  Transfere le voisinage de i pour l'image 3d img dans les 
  structures cube_topo3d (vois. original) et cubec_topo3d (complementaire).
  ATTENTION: i ne doit pas etre un point de bord (test a faire avant).
 */
{

  /* plan "HAUT" (+ps) */
  if (B[ps+i+1])    cube_topo3d[23].val = 1; else cube_topo3d[23].val = 0;
  if (B[ps+i+1-rs]) cube_topo3d[20].val = 1; else cube_topo3d[20].val = 0;
  if (B[ps+i-rs])   cube_topo3d[19].val = 1; else cube_topo3d[19].val = 0;
  if (B[ps+i-rs-1]) cube_topo3d[18].val = 1; else cube_topo3d[18].val = 0;
  if (B[ps+i-1])    cube_topo3d[21].val = 1; else cube_topo3d[21].val = 0;
  if (B[ps+i-1+rs]) cube_topo3d[24].val = 1; else cube_topo3d[24].val = 0;
  if (B[ps+i+rs])   cube_topo3d[25].val = 1; else cube_topo3d[25].val = 0;
  if (B[ps+i+rs+1]) cube_topo3d[26].val = 1; else cube_topo3d[26].val = 0;
  if (B[ps+i])      cube_topo3d[22].val = 1; else cube_topo3d[22].val = 0;
  /* plan "COURANT" () */
  if (B[i+1])       cube_topo3d[14].val = 1; else cube_topo3d[14].val = 0;
  if (B[i+1-rs])    cube_topo3d[11].val = 1; else cube_topo3d[11].val = 0;
  if (B[i-rs])      cube_topo3d[10].val = 1; else cube_topo3d[10].val = 0;
  if (B[i-rs-1])    cube_topo3d[9].val = 1; else cube_topo3d[9].val = 0;
  if (B[i-1])       cube_topo3d[12].val = 1; else cube_topo3d[12].val = 0;
  if (B[i-1+rs])    cube_topo3d[15].val = 1; else cube_topo3d[15].val = 0;
  if (B[i+rs])      cube_topo3d[16].val = 1; else cube_topo3d[16].val = 0;
  if (B[i+rs+1])    cube_topo3d[17].val = 1; else cube_topo3d[17].val = 0;
  if (B[i])         cube_topo3d[13].val = 1; else cube_topo3d[13].val = 0;
  /* plan "BAS" (-ps) */
  if (B[-ps+i+1])    cube_topo3d[5].val = 1; else cube_topo3d[5].val = 0;
  if (B[-ps+i+1-rs]) cube_topo3d[2].val = 1; else cube_topo3d[2].val = 0;
  if (B[-ps+i-rs])   cube_topo3d[1].val = 1; else cube_topo3d[1].val = 0;
  if (B[-ps+i-rs-1]) cube_topo3d[0].val = 1; else cube_topo3d[0].val = 0;
  if (B[-ps+i-1])    cube_topo3d[3].val = 1; else cube_topo3d[3].val = 0;
  if (B[-ps+i-1+rs]) cube_topo3d[6].val = 1; else cube_topo3d[6].val = 0;
  if (B[-ps+i+rs])   cube_topo3d[7].val = 1; else cube_topo3d[7].val = 0;
  if (B[-ps+i+rs+1]) cube_topo3d[8].val = 1; else cube_topo3d[8].val = 0;
  if (B[-ps+i])      cube_topo3d[4].val = 1; else cube_topo3d[4].val = 0;
  
  for (i = 0; i < 27; i++) 
    if (cube_topo3d[i].val == 1) cubec_topo3d[i].val = 0; else cubec_topo3d[i].val = 1;
} /* preparecubes() */

/* ==================================== */
static void preparecubesh(
  uint8_t *img,          /* pointeur base image */
  index_t i,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
  Transfere le voisinage de i pour l'image 3d img seuillee au niveau h dans les 
  structures cube_topo3d (vois. original) et cubec_topo3d (complementaire).
  ATTENTION: i ne doit pas etre un point de bord (test a faire avant).
 */
{
  /* plan "ARRIERE" (+ps) */
  if (img[ps+i+1]>=h)    cube_topo3d[17].val = 1; else cube_topo3d[17].val = 0;
  if (img[ps+i+1-rs]>=h) cube_topo3d[26].val = 1; else cube_topo3d[26].val = 0;
  if (img[ps+i-rs]>=h)   cube_topo3d[25].val = 1; else cube_topo3d[25].val = 0;
  if (img[ps+i-rs-1]>=h) cube_topo3d[24].val = 1; else cube_topo3d[24].val = 0;
  if (img[ps+i-1]>=h)    cube_topo3d[15].val = 1; else cube_topo3d[15].val = 0;
  if (img[ps+i-1+rs]>=h) cube_topo3d[6].val = 1; else cube_topo3d[6].val = 0;
  if (img[ps+i+rs]>=h)   cube_topo3d[7].val = 1; else cube_topo3d[7].val = 0;
  if (img[ps+i+rs+1]>=h) cube_topo3d[8].val = 1; else cube_topo3d[8].val = 0;
  if (img[ps+i]>=h)      cube_topo3d[16].val = 1; else cube_topo3d[16].val = 0;
  /* plan "COURANT" () */
  if (img[i+1]>=h)       cube_topo3d[14].val = 1; else cube_topo3d[14].val = 0;
  if (img[i+1-rs]>=h)    cube_topo3d[23].val = 1; else cube_topo3d[23].val = 0;
  if (img[i-rs]>=h)      cube_topo3d[22].val = 1; else cube_topo3d[22].val = 0;
  if (img[i-rs-1]>=h)    cube_topo3d[21].val = 1; else cube_topo3d[21].val = 0;
  if (img[i-1]>=h)       cube_topo3d[12].val = 1; else cube_topo3d[12].val = 0;
  if (img[i-1+rs]>=h)    cube_topo3d[3].val = 1; else cube_topo3d[3].val = 0;
  if (img[i+rs]>=h)      cube_topo3d[4].val = 1; else cube_topo3d[4].val = 0;
  if (img[i+rs+1]>=h)    cube_topo3d[5].val = 1; else cube_topo3d[5].val = 0;
  if (img[i]>=h)         cube_topo3d[13].val = 1; else cube_topo3d[13].val = 0;
  /* plan "AVANT" (-ps) */
  if (img[-ps+i+1]>=h)    cube_topo3d[11].val = 1; else cube_topo3d[11].val = 0;
  if (img[-ps+i+1-rs]>=h) cube_topo3d[20].val = 1; else cube_topo3d[20].val = 0;
  if (img[-ps+i-rs]>=h)   cube_topo3d[19].val = 1; else cube_topo3d[19].val = 0;
  if (img[-ps+i-rs-1]>=h) cube_topo3d[18].val = 1; else cube_topo3d[18].val = 0;
  if (img[-ps+i-1]>=h)    cube_topo3d[9].val = 1; else cube_topo3d[9].val = 0;
  if (img[-ps+i-1+rs]>=h) cube_topo3d[0].val = 1; else cube_topo3d[0].val = 0;
  if (img[-ps+i+rs]>=h)   cube_topo3d[1].val = 1; else cube_topo3d[1].val = 0;
  if (img[-ps+i+rs+1]>=h) cube_topo3d[2].val = 1; else cube_topo3d[2].val = 0;
  if (img[-ps+i]>=h)      cube_topo3d[10].val = 1; else cube_topo3d[10].val = 0;
  
  for (i = 0; i < 27; i++) 
    if (cube_topo3d[i].val == 1) cubec_topo3d[i].val = 0; else cubec_topo3d[i].val = 1;
} /* preparecubesh() */

/* ==================================== */
static void preparecubesh_l(
  int32_t *img,          /* pointeur base image */
  index_t i,                       /* index du point */
  int32_t h,                      /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
  Transfere le voisinage de i pour l'image 3d img seuillee au niveau h dans les 
  structures cube_topo3d (vois. original) et cubec_topo3d (complementaire).
  ATTENTION: i ne doit pas etre un point de bord (test a faire avant).
 */
{
  /* plan "ARRIERE" (+ps) */
  if (img[ps+i+1]>=h)    cube_topo3d[17].val = 1; else cube_topo3d[17].val = 0;
  if (img[ps+i+1-rs]>=h) cube_topo3d[26].val = 1; else cube_topo3d[26].val = 0;
  if (img[ps+i-rs]>=h)   cube_topo3d[25].val = 1; else cube_topo3d[25].val = 0;
  if (img[ps+i-rs-1]>=h) cube_topo3d[24].val = 1; else cube_topo3d[24].val = 0;
  if (img[ps+i-1]>=h)    cube_topo3d[15].val = 1; else cube_topo3d[15].val = 0;
  if (img[ps+i-1+rs]>=h) cube_topo3d[6].val = 1; else cube_topo3d[6].val = 0;
  if (img[ps+i+rs]>=h)   cube_topo3d[7].val = 1; else cube_topo3d[7].val = 0;
  if (img[ps+i+rs+1]>=h) cube_topo3d[8].val = 1; else cube_topo3d[8].val = 0;
  if (img[ps+i]>=h)      cube_topo3d[16].val = 1; else cube_topo3d[16].val = 0;
  /* plan "COURANT" () */
  if (img[i+1]>=h)       cube_topo3d[14].val = 1; else cube_topo3d[14].val = 0;
  if (img[i+1-rs]>=h)    cube_topo3d[23].val = 1; else cube_topo3d[23].val = 0;
  if (img[i-rs]>=h)      cube_topo3d[22].val = 1; else cube_topo3d[22].val = 0;
  if (img[i-rs-1]>=h)    cube_topo3d[21].val = 1; else cube_topo3d[21].val = 0;
  if (img[i-1]>=h)       cube_topo3d[12].val = 1; else cube_topo3d[12].val = 0;
  if (img[i-1+rs]>=h)    cube_topo3d[3].val = 1; else cube_topo3d[3].val = 0;
  if (img[i+rs]>=h)      cube_topo3d[4].val = 1; else cube_topo3d[4].val = 0;
  if (img[i+rs+1]>=h)    cube_topo3d[5].val = 1; else cube_topo3d[5].val = 0;
  if (img[i]>=h)         cube_topo3d[13].val = 1; else cube_topo3d[13].val = 0;
  /* plan "AVANT" (-ps) */
  if (img[-ps+i+1]>=h)    cube_topo3d[11].val = 1; else cube_topo3d[11].val = 0;
  if (img[-ps+i+1-rs]>=h) cube_topo3d[20].val = 1; else cube_topo3d[20].val = 0;
  if (img[-ps+i-rs]>=h)   cube_topo3d[19].val = 1; else cube_topo3d[19].val = 0;
  if (img[-ps+i-rs-1]>=h) cube_topo3d[18].val = 1; else cube_topo3d[18].val = 0;
  if (img[-ps+i-1]>=h)    cube_topo3d[9].val = 1; else cube_topo3d[9].val = 0;
  if (img[-ps+i-1+rs]>=h) cube_topo3d[0].val = 1; else cube_topo3d[0].val = 0;
  if (img[-ps+i+rs]>=h)   cube_topo3d[1].val = 1; else cube_topo3d[1].val = 0;
  if (img[-ps+i+rs+1]>=h) cube_topo3d[2].val = 1; else cube_topo3d[2].val = 0;
  if (img[-ps+i]>=h)      cube_topo3d[10].val = 1; else cube_topo3d[10].val = 0;
  
  for (i = 0; i < 27; i++) 
    if (cube_topo3d[i].val == 1) cubec_topo3d[i].val = 0; else cubec_topo3d[i].val = 1;
} /* preparecubesh_l() */

/* ==================================== */
static void preparecubeslab(
  int32_t *img,                    /* pointeur base image */
  index_t i,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
  Transfere le voisinage de i pour l'image 3d img dans les 
  structures cube_topo3d (vois. original) et cubec_topo3d (complementaire).
  Les points pris en compte sont ceux qui ont le même label que i.
  ATTENTION: i ne doit pas etre un point de bord (test a faire avant).
*/
{
  int32_t lab = img[i];
  /* plan "ARRIERE" (+ps) */
  if (img[ps+i+1]==lab)    cube_topo3d[17].val = 1; else cube_topo3d[17].val = 0;
  if (img[ps+i+1-rs]==lab) cube_topo3d[26].val = 1; else cube_topo3d[26].val = 0;
  if (img[ps+i-rs]==lab)   cube_topo3d[25].val = 1; else cube_topo3d[25].val = 0;
  if (img[ps+i-rs-1]==lab) cube_topo3d[24].val = 1; else cube_topo3d[24].val = 0;
  if (img[ps+i-1]==lab)    cube_topo3d[15].val = 1; else cube_topo3d[15].val = 0;
  if (img[ps+i-1+rs]==lab) cube_topo3d[6].val = 1; else cube_topo3d[6].val = 0;
  if (img[ps+i+rs]==lab)   cube_topo3d[7].val = 1; else cube_topo3d[7].val = 0;
  if (img[ps+i+rs+1]==lab) cube_topo3d[8].val = 1; else cube_topo3d[8].val = 0;
  if (img[ps+i]==lab)      cube_topo3d[16].val = 1; else cube_topo3d[16].val = 0;
  /* plan "COURANT" () */
  if (img[i+1]==lab)       cube_topo3d[14].val = 1; else cube_topo3d[14].val = 0;
  if (img[i+1-rs]==lab)    cube_topo3d[23].val = 1; else cube_topo3d[23].val = 0;
  if (img[i-rs]==lab)      cube_topo3d[22].val = 1; else cube_topo3d[22].val = 0;
  if (img[i-rs-1]==lab)    cube_topo3d[21].val = 1; else cube_topo3d[21].val = 0;
  if (img[i-1]==lab)       cube_topo3d[12].val = 1; else cube_topo3d[12].val = 0;
  if (img[i-1+rs]==lab)    cube_topo3d[3].val = 1; else cube_topo3d[3].val = 0;
  if (img[i+rs]==lab)      cube_topo3d[4].val = 1; else cube_topo3d[4].val = 0;
  if (img[i+rs+1]==lab)    cube_topo3d[5].val = 1; else cube_topo3d[5].val = 0;
  if (img[i]==lab)         cube_topo3d[13].val = 1; else cube_topo3d[13].val = 0;
  /* plan "AVANT" (-ps) */
  if (img[-ps+i+1]==lab)    cube_topo3d[11].val = 1; else cube_topo3d[11].val = 0;
  if (img[-ps+i+1-rs]==lab) cube_topo3d[20].val = 1; else cube_topo3d[20].val = 0;
  if (img[-ps+i-rs]==lab)   cube_topo3d[19].val = 1; else cube_topo3d[19].val = 0;
  if (img[-ps+i-rs-1]==lab) cube_topo3d[18].val = 1; else cube_topo3d[18].val = 0;
  if (img[-ps+i-1]==lab)    cube_topo3d[9].val = 1; else cube_topo3d[9].val = 0;
  if (img[-ps+i-1+rs]==lab) cube_topo3d[0].val = 1; else cube_topo3d[0].val = 0;
  if (img[-ps+i+rs]==lab)   cube_topo3d[1].val = 1; else cube_topo3d[1].val = 0;
  if (img[-ps+i+rs+1]==lab) cube_topo3d[2].val = 1; else cube_topo3d[2].val = 0;
  if (img[-ps+i]==lab)      cube_topo3d[10].val = 1; else cube_topo3d[10].val = 0;
  
  for (i = 0; i < 27; i++) 
    if (cube_topo3d[i].val == 1) cubec_topo3d[i].val = 0; else cubec_topo3d[i].val = 1;
} /* preparecubesh_l() */


/* ******************************************************************************* */
/* ******************************************************************************* */
/*                               PRIMITIVES 3D BINAIRES                            */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
void mctopo3d_top6(                   /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  preparecubes(img, p, rs, ps, N);
  *t = mctopo3d_T6(cube_topo3d);
  *tb = mctopo3d_T26(cubec_topo3d);
} /* mctopo3d_top6() */

/* ==================================== */
void mctopo3d_top18(                   /* pour un objet en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  preparecubes(img, p, rs, ps, N);
  *t = mctopo3d_T18(cube_topo3d);
  *tb = mctopo3d_T6p(cubec_topo3d);
} /* mctopo3d_top18() */

/* ==================================== */
void mctopo3d_top26(                   /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *t,
  int32_t *tb)                     /* resultats */
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  preparecubes(img, p, rs, ps, N);
  *t = mctopo3d_T26(cube_topo3d);
  *tb = mctopo3d_T6(cubec_topo3d);
} /* mctopo3d_top26() */

/* ==================================== */
int32_t mctopo3d_simple6(                   /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple6"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubes(img, p, rs, ps, N);
  return ((mctopo3d_T6(cube_topo3d) == 1) && (mctopo3d_T26(cubec_topo3d) == 1));
} /* mctopo3d_simple6() */

/* ==================================== */
int32_t mctopo3d_simple18(                  /* pour un objet en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple18"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubes(img, p, rs, ps, N);
  return ((mctopo3d_T18(cube_topo3d) == 1) && (mctopo3d_T6p(cubec_topo3d) == 1));
} /* mctopo3d_simple18() */

/* ==================================== */
int32_t mctopo3d_simple26(                  /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple26"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubes(img, p, rs, ps, N);
  return ((mctopo3d_T26(cube_topo3d) == 1) && (mctopo3d_T6(cubec_topo3d) == 1));
} /* mctopo3d_simple26() */

/* ==================================== */
int32_t mctopo3d_simple6h(                   /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple6h"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, h, rs, ps, N);
  return ((mctopo3d_T6(cube_topo3d) == 1) && (mctopo3d_T26(cubec_topo3d) == 1));
} /* mctopo3d_simple6h() */

/* ==================================== */
int32_t mctopo3d_simple18h(                  /* pour un objet en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple18h"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, h, rs, ps, N);
  return ((mctopo3d_T18(cube_topo3d) == 1) && (mctopo3d_T6p(cubec_topo3d) == 1));
} /* mctopo3d_simple18h() */

/* ==================================== */
int32_t mctopo3d_simple26h(                  /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple26h"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, h, rs, ps, N);
  return ((mctopo3d_T26(cube_topo3d) == 1) && (mctopo3d_T6(cubec_topo3d) == 1));
} /* mctopo3d_simple26h() */

/* ==================================== */
int32_t mctopo3d_simple6lab(                   /* pour un objet en 6-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple6lab"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubeslab(img, p, rs, ps, N);
  return ((mctopo3d_T6(cube_topo3d) == 1) && (mctopo3d_T26(cubec_topo3d) == 1));
} /* mctopo3d_simple6lab() */

/* ==================================== */
int32_t mctopo3d_simple18lab(                  /* pour un objet en 18-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple18lab"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubeslab(img, p, rs, ps, N);
  return ((mctopo3d_T18(cube_topo3d) == 1) && (mctopo3d_T6p(cubec_topo3d) == 1));
} /* mctopo3d_simple18lab() */

/* ==================================== */
int32_t mctopo3d_simple26lab(                  /* pour un objet en 26-connexite */
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
#undef F_NAME
#define F_NAME "mctopo3d_simple26lab"
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubeslab(img, p, rs, ps, N);
  return ((mctopo3d_T26(cube_topo3d) == 1) && (mctopo3d_T6(cubec_topo3d) == 1));
} /* mctopo3d_simple26lab() */

/* ==================================== */
int32_t mctopo3d_tbar6h(               /* pour un objet en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return -1;
  preparecubesh(img, p, h, rs, ps, N);
  return mctopo3d_T26(cubec_topo3d);
} /* mctopo3d_tbar6h() */

/* ==================================== */
int32_t mctopo3d_tbar26h(              /* pour un objet en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* seuil */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return -1;
  preparecubesh(img, p, h, rs, ps, N);
  return mctopo3d_T6(cubec_topo3d);
} /* mctopo3d_tbar26h() */

/* ========================================== */
uint8_t mctopo3d_P_simple(voxel * cube, voxel * cubep, voxel * cubec, uint8_t connex)
/* ========================================== */
#undef F_NAME
#define F_NAME "mctopo3d_P_simple"
/*
  cube contient X
  cubep contient P
  cubec (auxiliaire) n'a pas besoin d'etre initialise
  d'apres: "Some topological properties of surfaces in Z3", G. Bertrand & R. Malgouyres
           Theoreme 6
*/
{
  uint8_t n;
  uint8_t v;
  pvoxel x;  /* point central de cube */
  pvoxel y;  /* point de cube */
  pvoxel xc; /* point central de cubec */
  pvoxel yc; /* point de cubec */
  pvoxel xp; /* point central de cubep */
  pvoxel yp; /* point de cubep */

  for (n = 0; n < 27; n++) if (cube[n].val == 1) cubec[n].val = 0; else cubec[n].val = 1;

  switch (connex) /* teste la condition 2 (theoreme 6) */
  {
    case 6:  
      if (mctopo3d_T26(cubec) != 1) return 0; 
      break;
    case 18: 
      if (mctopo3d_T6p(cubec) != 1) return 0; 
      break;
    case 26: 
      if (mctopo3d_T6(cubec) != 1) return 0; 
      break;
    default: 
      fprintf(stderr, "mctopo3d_P_simple: mauvaise connexite : %d\n", connex); 
      exit(0); 
  } /* switch (connex) */
  
  x = &(cube[13]);
  xc = &(cubec[13]);
  xp = &(cubep[13]);
  switch (connex) /* teste la condition 4 (theoreme 6) */
  {
    case 6: 
      for (n = 0; n < x->n26v; n++)
      {
        yp = xp->v26[n];
        if (yp->val)
        {
          yc = xc->v26[n];
          v = yc->val;
          yc->val = 1;
          if (mctopo3d_T26(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n26v; n++) */
      break;
    case 18: 
      for (n = 0; n < x->n6v; n++)
      {
        yp = xp->v6[n];
        if (yp->val)
        {
          yc = xc->v6[n];
          v = yc->val;
          yc->val = 1;
          if (mctopo3d_T6p(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n6v; n++) */
      break;
    case 26: 
      for (n = 0; n < x->n6v; n++)
      {
        yp = xp->v6[n];
        if (yp->val)
        {
          yc = xc->v6[n];
          v = yc->val;
          yc->val = 1;
          if (mctopo3d_T6(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n6v; n++) */
      break;
    default: 
      fprintf(stderr, "mctopo3d_P_simple: mauvaise connexite : %d\n", connex); 
      exit(0); 
  } /* switch (connex) */
  
  for (n = 0; n < 27; n++) /* calcule et range dans cubec l'ensemble R = X - P  */
  {
    y = &(cube[n]);
    yp = &(cubep[n]);
    yc = &(cubec[n]);
    if (y->val && !yp->val) yc->val = 1; else yc->val = 0;
  } /* for (n = 0; n < 27; n++) */

  switch (connex) /* teste la condition 1 (theoreme 6) */
  {
    case 6:  
      if (mctopo3d_T6(cubec) != 1) return 0;
      break;
    case 18: 
      if (mctopo3d_T18(cubec) != 1) return 0;
      break;
    case 26: 
      if (mctopo3d_T26(cubec) != 1) return 0;
      break;
    default: 
      fprintf(stderr, "mctopo3d_P_simple: mauvaise connexite : %d\n", connex); 
      exit(0); 
  } /* switch (connex) */
  
  switch (connex) /* teste la condition 3 (theoreme 6) */
  {
    case 6: 
      for (n = 0; n < x->n6v; n++)
      {
        yp = xp->v6[n];
        if (yp->val)
        {
          yc = xc->v6[n];
          v = yc->val;
          yc->val = 1;
          if (mctopo3d_T6(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n6v; n++) */
      break;
    case 18: 
      for (n = 0; n < x->n18v; n++)
      {
        yp = xp->v18[n];
        if (yp->val)
        {
          yc = xc->v18[n];
          v = yc->val;
          yc->val = 1;
          if (mctopo3d_T18(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n18v; n++) */
      break;
    case 26: 
      for (n = 0; n < x->n26v; n++)
      {
        yp = xp->v26[n];
        if (yp->val)
        {
          yc = xc->v26[n];
          v = yc->val;
          yc->val = 1;
          if (mctopo3d_T26(cubec) != 1) return 0;
          yc->val = v;
        } /* if (yp->val) */
      } /* for (n = 0; n < x->n26v; n++) */
      break;
    default: 
      fprintf(stderr, "mctopo3d_P_simple: mauvaise connexite : %d\n", connex); 
      exit(0); 
  } /* switch (connex) */
  return 1;
} /* mctopo3d_P_simple() */





/* ******************************************************************************* */
/* ******************************************************************************* */
/*                                  PRIMITIVES 3D NDG                              */
/* ******************************************************************************* */
/* ******************************************************************************* */

/* ==================================== */
int32_t mctopo3d_pdestr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return ((mctopo3d_T26(cube_topo3d) == 1) && (mctopo3d_T6(cubec_topo3d) == 1));
} /* mctopo3d_pdestr6() */

/* ==================================== */
int32_t mctopo3d_pdestr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return ((mctopo3d_T6p(cube_topo3d) == 1) && (mctopo3d_T18(cubec_topo3d) == 1));
} /* mctopo3d_pdestr18() */

/* ==================================== */
int32_t mctopo3d_pdestr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return ((mctopo3d_T6(cube_topo3d) == 1) && (mctopo3d_T26(cubec_topo3d) == 1));
} /* mctopo3d_pdestr26() */

/* ==================================== */
int32_t mctopo3d_plevdestr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return (mctopo3d_T6(cubec_topo3d) == 1);
} /* mctopo3d_plevdestr6() */

/* ==================================== */
int32_t mctopo3d_plevdestr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return (mctopo3d_T18(cubec_topo3d) == 1);
} /* mctopo3d_plevdestr18() */

/* ==================================== */
int32_t mctopo3d_plevdestr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return (mctopo3d_T26(cubec_topo3d) == 1);
} /* mctopo3d_plevdestr26() */

/* ==================================== */
int32_t mctopo3d_pconstr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return ((mctopo3d_T26(cube_topo3d) == 1) && (mctopo3d_T6(cubec_topo3d) == 1));
} /* mctopo3d_pconstr6() */

/* ==================================== */
int32_t mctopo3d_pconstr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return ((mctopo3d_T6p(cube_topo3d) == 1) && (mctopo3d_T18(cubec_topo3d) == 1));
} /* mctopo3d_pconstr18() */

/* ==================================== */
int32_t mctopo3d_pconstr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return ((mctopo3d_T6(cube_topo3d) == 1) && (mctopo3d_T26(cubec_topo3d) == 1));
} /* mctopo3d_pconstr26() */

/* ==================================== */
int32_t mctopo3d_plevconstr6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return (mctopo3d_T26(cube_topo3d) == 1);
} /* mctopo3d_plevconstr6() */

/* ==================================== */
int32_t mctopo3d_plevconstr18(                  /* pour des minima en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return (mctopo3d_T6p(cube_topo3d) == 1);
} /* mctopo3d_plevconstr18() */

/* ==================================== */
int32_t mctopo3d_plevconstr26(                  /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return (mctopo3d_T6(cube_topo3d) == 1);
} /* mctopo3d_plevconstr26() */

/* ==================================== */
int32_t mctopo3d_peak6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return (mctopo3d_T26(cube_topo3d) == 0);
} /* mctopo3d_peak6() */

/* ==================================== */
int32_t mctopo3d_peak26(                    /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return (mctopo3d_T6(cube_topo3d) == 0);
} /* mctopo3d_peak26() */

/* ==================================== */
int32_t mctopo3d_well6(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return (mctopo3d_T6(cubec_topo3d) == 0);
} /* mctopo3d_well6() */

/* ==================================== */
int32_t mctopo3d_well26(                    /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return (mctopo3d_T26(cubec_topo3d) == 0);
} /* mctopo3d_well26() */

/* ==================================== */
uint8_t mctopo3d_alpha26m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[x] dans le 26-voisinage de x, */
/* ou img[x] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register index_t q;
	register uint8_t v;
	register int32_t mctopo3d_alpha = NDG_MIN - 1;
        register int32_t k;

        for (k = 0; k < 26; k += 1)
        {
          q = voisin26(p, k, rs, ps, N);
          if ((q != -1) && ((v=img[q]) < val) && ((int32_t)v > mctopo3d_alpha)) mctopo3d_alpha = (int32_t)v;
	}
        if (mctopo3d_alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)mctopo3d_alpha;
} /* mctopo3d_alpha26m() */

/* ==================================== */
int32_t mctopo3d_alpha26m_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[x] dans le 26-voisinage de x, */
/* ou img[x] si pas de telles valeurs */
/* ==================================== */
{
	register int32_t val = *(img+p);
	register index_t q;
	register int32_t v;
	register int32_t mctopo3d_alpha = NDG_MIN - 1;
        register int32_t k;

        for (k = 0; k < 26; k += 1)
        {
          q = voisin26(p, k, rs, ps, N);
          if ((q != -1) && ((v=img[q]) < val) && ((int32_t)v > mctopo3d_alpha)) mctopo3d_alpha = (int32_t)v;
	}
        if (mctopo3d_alpha == NDG_MIN - 1) 
          return val;
        else
          return (int32_t)mctopo3d_alpha;
} /* mctopo3d_alpha26m_l() */

/* ==================================== */
uint8_t mctopo3d_alpha6m(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* retourne le sup des valeurs < img[x] dans le 6-voisinage de x, */
/* ou img[x] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register index_t q;
	register uint8_t v;
	register int32_t mctopo3d_alpha = NDG_MIN - 1;
        register int32_t k;

        for (k = 0; k <= 10; k += 2)
        {
          q = voisin6(p, k, rs, ps, N);
          if ((q != -1) && ((v=img[q]) < val) && ((int32_t)v > mctopo3d_alpha)) mctopo3d_alpha = (int32_t)v;
	}
        if (mctopo3d_alpha == NDG_MIN - 1) 
          return val;
        else
          return (uint8_t)mctopo3d_alpha;
} /* mctopo3d_alpha6m() */

/* ==================================== */
uint8_t mctopo3d_alpha26p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* retourne le inf des valeurs > img[x] dans le 26-voisinage de x, */
/* ou img[x] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register index_t q;
	register uint8_t v;
	register int32_t mctopo3d_alpha = NDG_MAX + 1;
        register int32_t k;

        for (k = 0; k < 26; k += 1)
        {
          q = voisin26(p, k, rs, ps, N);
          if ((q != -1) && ((v=img[q]) > val) && ((int32_t)v < mctopo3d_alpha)) mctopo3d_alpha = (int32_t)v;
	}
        if (mctopo3d_alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)mctopo3d_alpha;
} /* mctopo3d_alpha26p() */

/* ==================================== */
uint8_t mctopo3d_alpha6p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* retourne le inf des valeurs > img[x] dans le 6-voisinage de x, */
/* ou img[x] si pas de telles valeurs */
/* ==================================== */
{
	register uint8_t val = *(img+p);
	register index_t q;
	register uint8_t v;
	register int32_t mctopo3d_alpha = NDG_MAX + 1;
        register int32_t k;

        for (k = 0; k <= 10; k += 2)
        {
          q = voisin6(p, k, rs, ps, N);
          if ((q != -1) && ((v=img[q]) > val) && ((int32_t)v < mctopo3d_alpha)) mctopo3d_alpha = (int32_t)v;
	}
        if (mctopo3d_alpha == NDG_MAX + 1) 
          return val;
        else
          return (uint8_t)mctopo3d_alpha;
} /* mctopo3d_alpha6p() */

/* ==================================== */
uint8_t mctopo3d_delta6m( 
/* retourne la valeur max. a laquelle p est destructible - minima 6-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (mctopo3d_pdestr6(img, p, rs, ps, N)) img[p] = mctopo3d_alpha26m(img, p, rs, ps, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* mctopo3d_delta6m() */

/* ==================================== */
uint8_t mctopo3d_delta26m( 
/* retourne la valeur max. a laquelle p est destructible - minima 26-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (mctopo3d_pdestr26(img, p, rs, ps, N)) img[p] = mctopo3d_alpha26m(img, p, rs, ps, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* mctopo3d_delta26m() */

/* ==================================== */
uint8_t mctopo3d_delta6p( 
/* retourne la valeur min. a laquelle p est constructible - minima 6-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (mctopo3d_pconstr6(img, p, rs, ps, N)) img[p] = mctopo3d_alpha26p(img, p, rs, ps, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* mctopo3d_delta6p() */

/* ==================================== */
uint8_t mctopo3d_delta26p( 
/* retourne la valeur min. a laquelle p est constructible - minima 26-connexes */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{	
  uint8_t ret, sav = img[p];
  while (mctopo3d_pconstr26(img, p, rs, ps, N)) img[p] = mctopo3d_alpha26p(img, p, rs, ps, N);
  ret = img[p];
  img[p] = sav;
  return ret;
} /* mctopo3d_delta26p() */

/* ==================================== */
int32_t mctopo3d_separant6(  /* teste si un point est separant - minima 6-connexes
	                     ie- s'il est separant pour une coupe <= img[p] */
  uint8_t *img,                    /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  index_t k, q;

  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;

  preparecubesh(img, p, img[p], rs, ps, N);
  if (mctopo3d_T6(cubec_topo3d) >= 2) return 1;

  for (k = 0; k < 26; k += 1)
  {
    q = voisin26(p, k, rs, ps, N);
    if ((q != -1) && (img[q] <= img[p]))
    {
      preparecubesh(img, p, img[q], rs, ps, N);
      if (mctopo3d_T6(cubec_topo3d) >= 2) return 1;
    }
  }	
  return 0;
} /* mctopo3d_separant6() */

/* ==================================== */
int32_t mctopo3d_hseparant6(  /* teste si un point est hseparant - minima 6-connexes
	         ie- s'il est separant pour la coupe h */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* parametre */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;

  preparecubesh(img, p, h, rs, ps, N);
  if (mctopo3d_T6(cubec_topo3d) >= 2) return 1;
  return 0;
} /* mctopo3d_hseparant6() */

/* ==================================== */
int32_t mctopo3d_hfseparant6(  /* teste si un point est hfseparant - minima 6-connexes
	         ie- s'il est separant pour une coupe c telle que h < c <= img[p] */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,                       /* parametre */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  index_t k, q;

  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;

  preparecubesh(img, p, img[p], rs, ps, N);
  if (mctopo3d_T6(cubec_topo3d) >= 2) return 1;

  for (k = 0; k < 26; k += 1)
  {
    q = voisin26(p, k, rs, ps, N);
    if ((q != -1) && (img[q] > h) && (img[q] <= img[p]))
    {
      preparecubesh(img, p, img[q], rs, ps, N);
      if (mctopo3d_T6(cubec_topo3d) >= 2) return 1;
    }
  }	
  return 0;
} /* mctopo3d_hfseparant6() */

/* ==================================== */
int32_t mctopo3d_filsombre6(                /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
   pour la coupe K (>img[p]), le point doit etre
   - soit un point isole (T == 1; Tb == 0)
   - soit une extremite de courbe (T == 1; Tb == 1 et card(K) = 1)
   - soit un point de courbe (T == 1; Tb == 2 et card(K) = 2)
   - soit un point de croisement de courbe (T == 1; Tb == n et card(K) = n)

*/
{
  int32_t T, Tb, Nb;
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  T = mctopo3d_T26(cube_topo3d);
  if (T != 1) return 0;
  Tb = mctopo3d_T6(cubec_topo3d);
  if (Tb == 0) return 1;
  Nb = mctopo3d_nbvois6(cubec_topo3d);
  if (Tb > 0) return (Nb == Tb);
  return 0;
} /* mctopo3d_filsombre6() */

/* ==================================== */
int32_t mctopo3d_filsombre26(               /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
   pour la coupe K (>img[p]), le point doit etre
   - soit un point isole (T == 1; Tb == 0)
   - soit une extremite de courbe (T == 1; Tb == 1 et card(K) = 1)
   - soit un point de courbe (T == 1; Tb == 2 et card(K) = 2)
   - soit un point de croisement de courbe (T == 1; Tb == n et card(K) = n)

*/
{
  int32_t T, Tb, Nb;
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  T = mctopo3d_T6(cube_topo3d);
  if (T != 1) return 0;
  Tb = mctopo3d_T26(cubec_topo3d);
  if (Tb == 0) return 1;
  Nb = mctopo3d_nbvois26(cubec_topo3d);
  if (Tb > 0) return (Nb == Tb);
  return 0;
} /* mctopo3d_filsombre26() */

/* ==================================== */
int32_t mctopo3d_filclair6(                /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
   pour la coupe K (>=img[p]), le point doit etre
   - soit un point isole (Tb == 1; T == 0)
   - soit une extremite de courbe (Tb == 1; T == 1 et card(K) = 1)
   - soit un point de courbe (Tb == 1; T == 2 et card(K) = 2)
   - soit un point de croisement de courbe (Tb == 1; T == n et card(K) = n)
*/
{
  int32_t T, Tb, Nb;
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  Tb = mctopo3d_T6(cubec_topo3d);
  if (Tb != 1) return 0;
  T = mctopo3d_T26(cube_topo3d);
  if (T == 0) return 1;
  Nb = mctopo3d_nbvois26(cube_topo3d);
  if (T > 0) return (Nb == T);
  return 0;
} /* mctopo3d_filclair6() */

/* ==================================== */
int32_t mctopo3d_filclair26(                /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
/*
   pour la coupe K (>=img[p]), le point doit etre
   - soit un point isole (Tb == 1; T == 0)
   - soit une extremite de courbe (Tb == 1; T == 1 et card(K) = 1)
   - soit un point de courbe (Tb == 1; T == 2 et card(K) = 2)
   - soit un point de croisement de courbe (Tb == 1; T == n et card(K) = n)
*/
{
  int32_t T, Tb, Nb;
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  Tb = mctopo3d_T26(cubec_topo3d);
  if (Tb != 1) return 0;
  T = mctopo3d_T6(cube_topo3d);
  if (T == 0) return 1;
  Nb = mctopo3d_nbvois6(cube_topo3d);
  if (T > 0) return (Nb == T);
  return 0;
} /* mctopo3d_filclair26() */

/* ==================================== */
int32_t mctopo3d_t6mm(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return mctopo3d_T6(cubec_topo3d);
} /* mctopo3d_t6mm() */

/* ==================================== */
int32_t mctopo3d_t6m(                   /* pour des minima en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return mctopo3d_T6(cubec_topo3d);
} /* mctopo3d_t6m() */

/* ==================================== */
int32_t mctopo3d_t26mm(                   /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return mctopo3d_T26(cubec_topo3d);
} /* mctopo3d_t26mm() */

/* ==================================== */
int32_t mctopo3d_t26m(                   /* pour des minima en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return mctopo3d_T26(cubec_topo3d);
} /* mctopo3d_t26m() */

/* ==================================== */
int32_t mctopo3d_t6pp(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return mctopo3d_T6(cube_topo3d);
} /* mctopo3d_t6pp() */

/* ==================================== */
int32_t mctopo3d_t6p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return mctopo3d_T6(cube_topo3d);
} /* mctopo3d_t6p() */

/* ==================================== */
int32_t mctopo3d_t26pp(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p]+1, rs, ps, N);
  return mctopo3d_T26(cube_topo3d);
} /* mctopo3d_t26pp() */

/* ==================================== */
int32_t mctopo3d_t26p(
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh(img, p, img[p], rs, ps, N);
  return mctopo3d_T26(cube_topo3d);
} /* mctopo3d_t26p() */

/* ==================================== */
int32_t mctopo3d_t26pp_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh_l(img, p, img[p]+1, rs, ps, N);
  return mctopo3d_T26(cube_topo3d);
} /* mctopo3d_t26pp_l() */

/* ==================================== */
int32_t mctopo3d_t6pp_l(
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  if ((p < ps) || (p >= N-ps) ||         /* premier ou dernier plan */
      (p%ps < rs) || (p%ps >= ps-rs) ||  /* premiere ou derniere colonne */
      (p%rs == 0) || (p%rs == rs-1))     /* premiere ou derniere ligne */
    return 0;
  preparecubesh_l(img, p, img[p]+1, rs, ps, N);
  return mctopo3d_T6(cube_topo3d);
} /* mctopo3d_t6pp_l() */

/* ==================================== */
void mctopo3d_nbtopoh3d26_l( /* pour les minima en 26-connexite */ 
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *mctopo3d_t6p,
  int32_t *mctopo3d_t26mm)
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  preparecubesh_l(img, p, h, rs, ps, N);
  *mctopo3d_t6p = mctopo3d_T6(cube_topo3d);
  *mctopo3d_t26mm = mctopo3d_T26(cubec_topo3d);
} /* mctopo3d_nbtopoh3d26_l() */

/* ==================================== */
void mctopo3d_nbtopoh3d6_l( /* pour les minima en 6-connexite */ 
  int32_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  int32_t h,
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N,                       /* taille image */
  int32_t *mctopo3d_t26p,
  int32_t *mctopo3d_t6mm)
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  preparecubesh_l(img, p, h, rs, ps, N);
  *mctopo3d_t26p = mctopo3d_T26(cube_topo3d);
  *mctopo3d_t6mm = mctopo3d_T6(cubec_topo3d);
} /* mctopo3d_nbtopoh3d6_l() */

/* ==================================== */
int32_t mctopo3d_bordext6(uint8_t *F, index_t x, index_t rs, index_t ps, index_t N)
/* ==================================== */
/* teste si x a un 6-voisin a 0 */
{
  index_t k, y;
  for (k = 0; k <= 10; k += 2) /* parcourt les voisins en 6-connexite */
  {
    y = voisin6(x, k, rs, ps, N);
    if ((y != -1) && (F[y] == 0)) return 1;
  } /* for k */      
  return 0;
} /* mctopo3d_bordext6() */

/* ==================================== */
int32_t mctopo3d_bordext26(uint8_t *F, index_t x, index_t rs, index_t ps, index_t N)
/* ==================================== */
/* teste si x a un 26-voisin a 0 */
{
  index_t k, y;
  for (k = 0; k < 26; k += 1) /* parcourt les voisins en 26-connexite */
  {
    y = voisin26(x, k, rs, ps, N);
    if ((y != -1) && (F[y] == 0)) return 1;
  } /* for k */      
  return 0;
} /* mctopo3d_bordext26() */

/* ==================================== */
int32_t mctopo3d_curve6( /* point de courbe en 6-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  if (img[p] == 0) return 0;
  preparecubes(img, p, rs, ps, N);
  if ((mctopo3d_T6(cube_topo3d) == 2) && (mctopo3d_nbvoiso6(img, p, rs, ps, N) == 2)) return 1;
  return 0;
} /* mctopo3d_curve6() */

/* ==================================== */
int32_t mctopo3d_curve18( /* point de courbe en 18-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  if (img[p] == 0) return 0;
  preparecubes(img, p, rs, ps, N);
  if ((mctopo3d_T18(cube_topo3d) == 2) && (mctopo3d_nbvoiso18(img, p, rs, ps, N) == 2)) return 1;
  return 0;
} /* mctopo3d_curve18() */

/* ==================================== */
int32_t mctopo3d_curve26( /* point de courbe en 26-connexite */
  uint8_t *img,          /* pointeur base image */
  index_t p,                       /* index du point */
  index_t rs,                      /* taille rangee */
  index_t ps,                      /* taille plan */
  index_t N)                       /* taille image */
/* ==================================== */
{
  assert(!is_on_frame(p, rs, ps, N));
  if (img[p] == 0) return 0;
  preparecubes(img, p, rs, ps, N);
  if ((mctopo3d_T26(cube_topo3d) == 2) && (mctopo3d_nbvoiso26(img, p, rs, ps, N) == 2)) return 1;
  return 0;
} /* mctopo3d_curve26() */
