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
lhma.c

Functions for the Higher-resolution Medial Axis (HMA)

Andre Vital Saude - jan 2006
*/

//#define CHRONO
#define VERBOSE

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include <mcimage.h>
#include <mcchrono.h>
#include <mccodimage.h>
#include <avsimage.h>
#include <llut.h>
#include <mcgeo.h>
#include <ldist.h>
#include <lhma.h>

//private Functions
void applyConstrainedSnSyms2d(AVS_PointSet *set, AVS_Point vec, AVS_Point orig, int32_t rs, int32_t cs);
void applyConstrainedSnSyms3d(AVS_PointSet *set, AVS_Point vec, AVS_Point orig, int32_t rs, int32_t cs, int32_t ds);
int32_t EnRmin2d(AVS_Point x, int32_t R, AVS_Point v, int32_t Rv, SQDLut SQDn);
int32_t EnRmin3d(AVS_Point x, int32_t R, AVS_Point v, int32_t Rv, SQDLut SQDn);
int32_t IsHMAEnhanced2d(AVS_Point x, /*int32_t Rmax,*/ uint32_t *D2Xh,
		    MLut mlut, RTLutCol Lut, SQDLut SQDn, int32_t rs, int32_t cs);
int32_t IsHMAEnhanced3d(AVS_Point x, /*int32_t Rmax,*/ uint32_t *D2Xh, 
		    MLut mlut, RTLutCol Lut, SQDLut SQDn, int32_t rs, int32_t cs, int32_t ds);



/* ==================================== */
struct xvimage *lhma(struct xvimage *Xh)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhma"
/*
  Algorithm to compute the Higher-resolution Medial Axis

  Written by: Andre Vital Saude
  created: 19/01/2006 from older version
  last modified: 19/01/2006

  Input : Xh, binary image Xh = H(X), where X is the object in original resolution
          
  Output : HMA, the weighted higher-resolution medial axis
*/
{
  //Images
  struct xvimage *imD2Xh, *imHMA;
  uint32_t *D2Xh, *HMA;
  int32_t rs, cs, ds, ps, N;

  //aux
  AVS_Point p;
  int32_t rmax;

  //tables
  SQDLut SQDn;
  MLut mlut;
  RTLutCol Lut;

#ifdef CHRONO
  chrono Chrono2;
#endif

#ifdef CHRONO
  start_chrono(&Chrono2);
#endif

  //test parameters
  if (!Xh) {
    fprintf(stderr, "%s: null image!\n", F_NAME);
    return(NULL);
  }

  //init
  rs = rowsize(Xh);        /* taille ligne Xh */
  cs = colsize(Xh);        /* taille colonne Xh */
  ds = depth(Xh);
  ps = rs * cs;
  N = ps * ds;            /* taille image Xh */

  //alloc images hma and edt
  imD2Xh = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  imHMA = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if ((!imD2Xh) || (!imHMA)) {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(NULL);
  }
  if (! lsedt_meijster(Xh, imD2Xh)) // calcule la fonction distance
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return(NULL);
  }

#ifdef CHRONO
  printf("calc dist = %d\n", read_chrono(&Chrono2));
  start_chrono(&Chrono2);
#endif

  D2Xh = ULONGDATA(imD2Xh);
  HMA = ULONGDATA(imHMA);
  memset(HMA, 0, N*sizeof(int32_t));

  rmax = findMaxLong(D2Xh, N);
#ifdef VERBOSE
  printf("rmax=%d\n", rmax);
  fflush(stdout);
#endif

  //Read tables
  if (! lread_SQD_RT_Luts(&SQDn, &mlut, &Lut, rmax, ds)) {
    fprintf(stderr, "%s: lread_SQD_RT_Luts failed. This is fatal error.\n", F_NAME);
    return(NULL);
  }

  lfree_MLut(&mlut);
  if (! lread_MhLut(&mlut, rmax, ds)) {
    fprintf(stderr, "%s: lread_MhLut failed. This is fatal error.\n", F_NAME);
    return(NULL);
  }

#ifdef CHRONO
  printf("load tables = %d\n", read_chrono(&Chrono2));
  start_chrono(&Chrono2);
#endif

  // do it!
  if (ds == 1) {
    for(p.y = 0; p.y < cs; p.y++) 
      for(p.x = 0; p.x < rs; p.x++) {
	if(AVS_point2d(D2Xh, p, rs) !=0) 
	  AVS_point2d(HMA, p, rs) = IsHMAEnhanced2d(p, D2Xh, mlut, Lut, SQDn, rs, cs);
      }
  }
  else
  {
    for (p.z = 0; p.z < ds; p.z++) 
      for(p.y = 0; p.y < cs; p.y++) 
	for(p.x = 0; p.x < rs; p.x++) 
	  if(AVS_point3d(D2Xh, p, ps, rs) != 0) {
	    AVS_point3d(HMA, p, ps, rs) = IsHMAEnhanced3d(p, D2Xh, mlut, Lut, SQDn, rs, cs, ds);
	    if (AVS_point3d(HMA, p, ps, rs) == 1) printf("p=(%d,%d,%d)\n", p.x, p.y, p.z);
	  }

  }

#ifdef CHRONO
  printf("calc hma = %d\n", read_chrono(&Chrono2));
#endif

  //Free
  lfree_MLut(&mlut);
  lfree_SQDLut(&SQDn);
  free(Lut);
  freeimage(imD2Xh);

  return imHMA;
} // lhma()


/* ==================================== */
struct xvimage *lhma_givenTables(struct xvimage *Xh, MLut mlut, RTLutCol Lut, SQDLut SQDn, int32_t *rmax)
/* ==================================== */
#undef F_NAME
#define F_NAME "lhma"
/*
  Algorithm to compute the Higher-resolution Medial Axis with the means of the given
  tables. Useful for building the MhLut. See src/tables/mhlut.c for details.

  Written by: Andre Vital Saude
  created: 19/01/2006 from older version
  last modified: 19/01/2006

  Input : Xh, binary image Xh = H(X), where X is the object in original resolution
          
  Output : HMA, the weighted higher-resolution medial axis
*/
{
  //Images
  struct xvimage *imD2Xh, *imHMA;
  uint32_t *D2Xh, *HMA;
  int32_t rs, cs, ds, ps, N;

  //aux
  AVS_Point p;

  //test parameters
  if (!Xh) {
    fprintf(stderr, "%s: null image!\n", F_NAME);
    return(NULL);
  }

  //init
  rs = rowsize(Xh);        /* taille ligne Xh */
  cs = colsize(Xh);        /* taille colonne Xh */
  ds = depth(Xh);
  ps = rs * cs;
  N = ps * ds;            /* taille image Xh */

  //alloc images hma and edt
  imD2Xh = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  imHMA = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if ((!imD2Xh) || (!imHMA)) {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(NULL);
  }
  if (! lsedt_meijster(Xh, imD2Xh)) // calcule la fonction distance
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return(NULL);
  }

  D2Xh = ULONGDATA(imD2Xh);
  HMA = ULONGDATA(imHMA);
  memset(HMA, 0, N*sizeof(int32_t));

  *rmax = findMaxLong(D2Xh, N);

  // do it!
  if (ds == 1) {
    for(p.y = 0; p.y < cs; p.y++) 
      for(p.x = 0; p.x < rs; p.x++) {
	if(AVS_point2d(D2Xh, p, rs) !=0) 
	  AVS_point2d(HMA, p, rs) = IsHMAEnhanced2d(p, D2Xh, mlut, Lut, SQDn, rs, cs);
      }
  }
  else
  {
    for (p.z = 0; p.z < ds; p.z++) 
      for(p.y = 0; p.y < cs; p.y++) 
	for(p.x = 0; p.x < rs; p.x++) 
	  if(AVS_point3d(D2Xh, p, ps, rs) != 0)
	    AVS_point3d(HMA, p, ps, rs) = IsHMAEnhanced3d(p, D2Xh, mlut, Lut, SQDn, rs, cs, ds);
  }

  //Free
  freeimage(imD2Xh);

  return imHMA;
} // lhma_build()

/* ==================================== */
int32_t EnRmin2d(AVS_Point x, int32_t R, AVS_Point v,
	     int32_t Rv, SQDLut SQDn)
/* ==================================== */
{
  int32_t i;
  int32_t Rvless;
  AVS_Point rg, xplusv, S;
  AVS_Point r1, r2;

  S.x = (v.x < 0) ? -1 : 1; S.y = (v.y < 0) ? -1 : 1;
  xplusv.x = x.x + v.x;  xplusv.y = x.y + v.y;

  //Rv <- Lut[vg, R]: within parameters
  while (Rv > 0) {
    //foreach rg in SQDn(Rv-)
    posIntLess(SQDn, Rv, Rvless);
    for (i=0; i < SQDn_size(SQDn, Rvless); i++) {
      rg = SQDn_get(SQDn, Rvless)[i];

      //is rg useful? can x+v-r be in E3?
      if(En2d(rg.x, rg.y) != En2d(xplusv.x, xplusv.y)) continue;

      //foreach P' in Sn2
      //r <- SP'rg
      r1.x = S.x*rg.x; r1.y = S.y*rg.y; //first P'
      r2.x = S.x*rg.y; r2.y = S.y*rg.x; //second P'

      //if (((x+v-r) in En) and ((v-r)^2 < R) then return Rv
      if( ( (2 == En2d(xplusv.x - r1.x, xplusv.y - r1.y)) &&
	    ((v.x - r1.x) * (v.x - r1.x) + (v.y - r1.y) * (v.y - r1.y) < R) ) || //first P'
	  ( (2 == En2d(xplusv.x - r2.x, xplusv.y - r2.y)) &&
	    ((v.x - r2.x) * (v.x - r2.x) + (v.y - r2.y) * (v.y - r2.y) < R) ) ) //second P'
	{
	  return Rv;
	}
    }
    //Rv <- Rv-
    Rv = Rvless;
  }
  return 0;
}

/* ==================================== */
int32_t EnRmin3d(AVS_Point x, int32_t R, AVS_Point v,
	     int32_t Rv, SQDLut SQDn)
/* ==================================== */
{
  int32_t i, j;
  int32_t Rvless;
  AVS_Point rg, xplusv, S;
  AVS_Point psyms[6];

  S.x = (v.x < 0) ? -1 : 1; S.y = (v.y < 0) ? -1 : 1; S.z = (v.z < 0) ? -1 : 1;
  xplusv.x = x.x + v.x;  xplusv.y = x.y + v.y;  xplusv.z = x.z + v.z;

  //Rv <- Lut[vg, R]: within parameters
  while (Rv > 0) {
    //foreach rg in SQDn(Rv-)
    posIntLess(SQDn, Rv, Rvless);
    for (i=0; i < SQDn_size(SQDn, Rvless); i++) {
      rg = SQDn_get(SQDn, Rvless)[i];

      //is rg useful? can x+v-r be in E3?
      if(En3d(rg.x, rg.y, rg.z) != En3d(xplusv.x, xplusv.y, xplusv.z)) continue;

      //foreach P' in Sn2
      psyms[0] = rg; 
      psyms[1].x = rg.x; psyms[1].y = rg.z; psyms[1].z = rg.y;
      psyms[2].x = rg.y; psyms[2].y = rg.x; psyms[2].z = rg.z;
      psyms[3].x = rg.y; psyms[3].y = rg.z; psyms[3].z = rg.x;
      psyms[4].x = rg.z; psyms[4].y = rg.x; psyms[4].z = rg.y;
      psyms[5].x = rg.z; psyms[5].y = rg.y; psyms[5].z = rg.x;
      for(j = 0; j < 6; j++) {
	//r <- SP'rg
	AVS_Point r = psyms[j];
	r.x *= S.x; r.y*=S.y; r.z *=S.z;

	//if (((x+v-r) in En) and ((v-r)^2 < R) then return Rv
	if( (3 == En3d(xplusv.x - r.x, xplusv.y - r.y, xplusv.z - r.z)) &&
	    (((v.x - r.x) * (v.x - r.x) + 
	      (v.y - r.y) * (v.y - r.y) + 
	      (v.z - r.z) * (v.z - r.z)) < R))
	  {
	    return Rv;
	  }
      } 
    }
    //Rv <- Rv-
    Rv = Rvless;
  }
  return 0;
}

/* ==================================== */
int32_t EnRminAuto2d(AVS_Point x, int32_t R,
		 SQDLut SQDn)
/* ==================================== */
{
  int32_t i, Rless;
  int32_t Enx = En2d(x.x, x.y);
  AVS_Point rg;

  //while (R>0)
  while(R > 0) {
    //foreach rg in SQDn(R-)
    posIntLess(SQDn, R, Rless);
    for (i=0; i < SQDn_size(SQDn, Rless); i++) {
      rg = SQDn_get(SQDn, Rless)[i];
      if( Enx == En2d(rg.x, rg.y)) return R;
    }
    R = Rless;
  }

  return 0;
} //EnRminAuto2d


/* ==================================== */
int32_t EnRminAuto3d(AVS_Point x, int32_t R,
		 SQDLut SQDn)
/* ==================================== */
{
  int32_t i, Rless;
  int32_t Enx = En3d(x.x, x.y, x.z);
  AVS_Point rg;

  /*
  if ((R == 1)&&( Enx == En3d(0, 0, 0)) ) {
    printf("x=(%d,%d,%d)\n", x.x, x.y, x.z);
    printf("R=1 and Rless=%d, SQDn_size(SQDn, Rless)=%d", Rless, SQDn_size(SQDn, Rless));
  }
  */

  //while (R>0)
  while(R > 0) {
    //foreach rg in SQDn(R-)
    posIntLess(SQDn, R, Rless);
    for (i=0; i < SQDn_size(SQDn, Rless); i++) {
      rg = SQDn_get(SQDn, Rless)[i];
      if( Enx == En3d(rg.x, rg.y, rg.z)) return R;
    }
    R = Rless;
  }

  return 0;
} //EnRminAuto3d


/* ==================================== */
int32_t IsHMAEnhanced2d(AVS_Point x, /*int32_t Rmax,*/ uint32_t *D2Xh,
		    MLut mlut, RTLutCol Lut, SQDLut SQDn, int32_t rs, int32_t cs)
/* ==================================== */
/*
  Is HMA
  returns D2Xh(x) if true
*/
#undef F_NAME
#define F_NAME IsHMAEnhanced2d
{
  int32_t Rx, Rv;
  int32_t i,j;
  AVS_PointSet Svg;
  AVS_Point v;

  //Rx = EnRmin(x, D2Xh(x), 0)
  Rx = EnRminAuto2d(x, AVS_point2d(D2Xh, x, rs), SQDn);
  if (Rx == 0) return 0; //En(B<(x, D2Xh(x))) is empty

  //avoid zero values in RTLut
  posIntOver(SQDn, Rx, Rx);

  //foreach vg in MLut do
  for (i = 0; i < mlut.numd; i++) {
    //foreach S in Sn do
    //v = Svg
    applyConstrainedSnSyms2d(&Svg, mlut.vec[i], x, rs, cs); //vg = mlut.vec[i];
    for (j = 0; j < Svg.size; j++) {
      v = Svg.points[j];
      Rv = EnRmin2d(x, Rx, v, Lut[mlut.indmap[i] + Rx], SQDn);
      if (AVS_pixel(D2Xh, x.x + v.x, x.y + v.y, rs) >= Rv) return 0;
    }
  }

  return AVS_point2d(D2Xh, x, rs);
} // IsHMAEnhanced2d()



/* ==================================== */
int32_t IsHMAEnhanced3d(AVS_Point x, /*int32_t Rmax,*/ uint32_t *D2Xh,
		    MLut mlut, RTLutCol Lut, SQDLut SQDn, int32_t rs, int32_t cs, int32_t ds)
/* ==================================== */
/*
  Is HMA
  returns D2Xh(x) if true
*/
#undef F_NAME
#define F_NAME IsHMAEnhanced3d
{
  int32_t Rx, Rv;
  int32_t i,j;
  int32_t ps = rs*cs;
  AVS_PointSet Svg;
  AVS_Point v;

  //Rx = EnRmin(x, D2Xh(x), 0)
  Rx = EnRminAuto3d(x, AVS_point3d(D2Xh, x, ps, rs), SQDn);
  if (Rx == 0) return 0; //En(B<(x, D2Xh(x))) is empty

  //avoid zero values in RTLut
  posIntOver(SQDn, Rx, Rx);

  //foreach vg in MLut do
  for (i = 0; i < mlut.numd; i++) {
    //foreach S in Sn do
    //v = Svg
    applyConstrainedSnSyms3d(&Svg, mlut.vec[i], x, rs, cs, ds); //vg = mlut.vec[i];
    for (j = 0; j < Svg.size; j++) {
      v = Svg.points[j];
      Rv = EnRmin3d(x, Rx, v, Lut[mlut.indmap[i] + Rx], SQDn);
      if (AVS_voxel(D2Xh, x.x + v.x, x.y + v.y, x.z + v.z, ps, rs) >= Rv) return 0;
    }
  }

  if (AVS_point3d(D2Xh, x, ps, rs) == 1)
    printf("x=(%d,%d,%d)\n", x.x, x.y, x.z);

  return AVS_point3d(D2Xh, x, ps, rs);
} // IsHMAEnhanced3d()



//----------------------------------
void applyConstrainedSnSyms2d(AVS_PointSet *set, AVS_Point vec, AVS_Point orig, int32_t rs, int32_t cs)
//----------------------------------
/*
  Apply Sn symmetries to vec with the constraint that orig+vec=(x,y) is
  such that 0<=x<rs, 0<=y<cs

  returns 'set', with the 'set->size' vectors found
*/
{
  int32_t vectx[8], vecty[8], i;
  int32_t xx, yy;
  int32_t count=0;

  vectx[0] = vec.x;   vecty[0] = vec.y;
  vectx[1] = vec.y;   vecty[1] = vec.x;
  vectx[2] = vec.y;   vecty[2] = -vec.x;
  vectx[3] = vec.x;   vecty[3] = -vec.y;
  vectx[4] = -vec.x;  vecty[4] = -vec.y;
  vectx[5] = -vec.y;  vecty[5] = -vec.x;
  vectx[6] = -vec.y;  vecty[6] = vec.x;
  vectx[7] = -vec.x;  vecty[7] = vec.y;

  for (i = 0; i <= 7; i++)
  {
    xx = orig.x + vectx[i];
    yy = orig.y + vecty[i];
    if ((xx >= 0) && (xx < rs) && (yy >= 0) && (yy < cs))
    {
      set->points[count].x = vectx[i];
      set->points[count].y = vecty[i];
      count++;
    }
  }
  set->size = count;
} // applyConstrainedSnSyms2d()


//----------------------------------
void applyConstrainedSnSyms3d(AVS_PointSet *set, AVS_Point vec, AVS_Point orig, int32_t rs, int32_t cs, int32_t ds)
//----------------------------------
/*
  Apply Sn symmetries to vec with the constraint that orig+vec=(x,y,z) is
  such that 0<=x<rs, 0<=y<cs, 0<=z<ds

  returns 'set', with the 'set->size' vectors found
*/
{
  int32_t vectx[48], vecty[48], vectz[48], i;
  int32_t xx, yy, zz;
  int32_t x = vec.x, y = vec.y, z = vec.z;
  int32_t count = 0;

  // S=(1,1,1)
  vectx[ 0] =  x;  vecty[ 0] =  y;   vectz[ 0] =  z;
  vectx[ 1] =  x;  vecty[ 1] =  z;   vectz[ 1] =  y;
  vectx[ 2] =  y;  vecty[ 2] =  x;   vectz[ 2] =  z;
  vectx[ 3] =  y;  vecty[ 3] =  z;   vectz[ 3] =  x;
  vectx[ 4] =  z;  vecty[ 4] =  x;   vectz[ 4] =  y;
  vectx[ 5] =  z;  vecty[ 5] =  y;   vectz[ 5] =  x;

  // S=(-1,1,1)
  vectx[ 6] = -x;  vecty[ 6] =  y;   vectz[ 6] =  z;
  vectx[ 7] = -x;  vecty[ 7] =  z;   vectz[ 7] =  y;
  vectx[ 8] = -y;  vecty[ 8] =  x;   vectz[ 8] =  z;
  vectx[ 9] = -y;  vecty[ 9] =  z;   vectz[ 9] =  x;
  vectx[10] = -z;  vecty[10] =  x;   vectz[10] =  y;
  vectx[11] = -z;  vecty[11] =  y;   vectz[11] =  x;

  // S=(1,-1,1)
  vectx[12] =  x;  vecty[12] = -y;   vectz[12] =  z;
  vectx[13] =  x;  vecty[13] = -z;   vectz[13] =  y;
  vectx[14] =  y;  vecty[14] = -x;   vectz[14] =  z;
  vectx[15] =  y;  vecty[15] = -z;   vectz[15] =  x;
  vectx[16] =  z;  vecty[16] = -x;   vectz[16] =  y;
  vectx[17] =  z;  vecty[17] = -y;   vectz[17] =  x;

  // S=(1,1,-1)
  vectx[18] =  x;  vecty[18] =  y;   vectz[18] = -z;
  vectx[19] =  x;  vecty[19] =  z;   vectz[19] = -y;
  vectx[20] =  y;  vecty[20] =  x;   vectz[20] = -z;
  vectx[21] =  y;  vecty[21] =  z;   vectz[21] = -x;
  vectx[22] =  z;  vecty[22] =  x;   vectz[22] = -y;
  vectx[23] =  z;  vecty[23] =  y;   vectz[23] = -x;

  // S=(-1,-1,1)
  vectx[24] = -x;  vecty[24] = -y;   vectz[24] =  z;
  vectx[25] = -x;  vecty[25] = -z;   vectz[25] =  y;
  vectx[26] = -y;  vecty[26] = -x;   vectz[26] =  z;
  vectx[27] = -y;  vecty[27] = -z;   vectz[27] =  x;
  vectx[28] = -z;  vecty[28] = -x;   vectz[28] =  y;
  vectx[29] = -z;  vecty[29] = -y;   vectz[29] =  x;

  // S=(-1,1,-1)
  vectx[30] = -x;  vecty[30] =  y;   vectz[30] = -z;
  vectx[31] = -x;  vecty[31] =  z;   vectz[31] = -y;
  vectx[32] = -y;  vecty[32] =  x;   vectz[32] = -z;
  vectx[33] = -y;  vecty[33] =  z;   vectz[33] = -x;
  vectx[34] = -z;  vecty[34] =  x;   vectz[34] = -y;
  vectx[35] = -z;  vecty[35] =  y;   vectz[35] = -x;

  // S=(1,-1,-1)
  vectx[36] =  x;   vecty[36] = -y;  vectz[36] = -z;
  vectx[37] =  x;   vecty[37] = -z;  vectz[37] = -y;
  vectx[38] =  y;   vecty[38] = -x;  vectz[38] = -z;
  vectx[39] =  y;   vecty[39] = -z;  vectz[39] = -x;
  vectx[40] =  z;   vecty[40] = -x;  vectz[40] = -y;
  vectx[41] =  z;   vecty[41] = -y;  vectz[41] = -x;

  // S=(-1,-1,-1)
  vectx[42] = -x;  vecty[42] = -y;  vectz[42] = -z;
  vectx[43] = -x;  vecty[43] = -z;  vectz[43] = -y;
  vectx[44] = -y;  vecty[44] = -x;  vectz[44] = -z;
  vectx[45] = -y;  vecty[45] = -z;  vectz[45] = -x;
  vectx[46] = -z;  vecty[46] = -x;  vectz[46] = -y;
  vectx[47] = -z;  vecty[47] = -y;  vectz[47] = -x;

  for (i = 0; i < 48; i++) {
    xx = orig.x + vectx[i];

    if ((xx >= 0) && (xx < rs)) {
      yy = orig.y + vecty[i];

      if ((yy >= 0) && (yy < cs)) {
	zz = orig.z + vectz[i];

	if ((zz >= 0) && (zz < ds)) {
	  set->points[count].x = vectx[i];
	  set->points[count].y = vecty[i];
	  set->points[count].z = vectz[i];
	  count++;
	}
      }
    }
  }

  set->size = count;
} // GSym3d()
