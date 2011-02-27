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
#ifdef __cplusplus
extern "C" {
#endif
/*
Manage look-up tables (Luts) in Pink

The folowing Luts are available:
  1. Square Decompositions Table or Bisector Table (SQDLut)
  2. Remy and Thiel Medial Axis Table (MgLut and RTLutCol)
  3. Higher-resolution Medial Axis (HMA) sufficient vectors table (MhLut)

Author: Andre Vital Saude, 2005

ADDITIONAL REMARK:

With llut.h and llut.c, call in lmedialaxis.c could be:
nval = lreadbisectorLut(&TabIndDec, &ListDec, ds);
if (distmax >= nval)
{
  fprintf(stderr, "%s: bisector table overflow: %d >= %d\n", F_NAME, distmax, nval);
  exit(0);
}
and distmax would not be a parameter


DETAILS:

1. Square Decompositions Table or Bisector Table (SQDLut)
---------------------------------------------------------

Tables for the bisector function

format : 

Table 1:
NVAL 
TabIndDec[NVAL+1] 

Table 2:
NPOINTS 
ListDecs[NPOINTS]

For each i between 0 and NVAL-1, 
TabNbDec[i] gives the index of the first decomposition (if any) 
of the integer i into a sum of two squares.

For each i between 0 and NVAL-1, 
TabIndDec[i+1] - TabIndDec[i] gives the number of different 
decompositions of the integer i into a sum of two squares.

For each p between 0 and NPOINTS-1, 
ListDecs[NPOINTS] is a couple of integers x, y which is 
a decomposition of an integer into a sum of two squares.

Michel Couprie - juillet 2004



2. Remy and Thiel Medial Axis Table (RTLut)
--------------------------------------------------------

The RTLut is composed by two separated Luts:
  - MgLut: the sufficient vectors Lut
  - RTLutCol: the necessary radii Lut

The MgLut is stocked in the MLut type, since it shares the same
structure as the MhLut explained bellow



3. Higher-resolution Medial Axis sufficient vectors table (MhLut)
-----------------------------------------------------------------

The sufficient vectors Lut associated to the HMA.

*/


//---------- FILENAMES ----------------------------
/*
  In the separate TabMgLut, as well as in TabMhLut, two lines are added to the 
  begining of the original MgLut from TabRemyThiel: i) the maximal known radius
  mlut.rknown and ii) the required maximal distance in SQDLut mlut.sqdnReqDist,
  in case it is to be used. The first one was already present in TabRemyThiel,
  but it is better placed in the begining.

  In TabMhLut, an additional line called "indmap" is added to the end. This is
  the map between the order of occurence of a vector in MhLut (index) and its
  occurence in MgLut (indmap[index]) 
*/
#define TAB_INDDEC(ds) ((ds == 1) ? "TabBisector_1.txt" : "TabBisector3d_1.txt")
#define TAB_LISTDEC(ds) ((ds == 1) ? "TabBisector_2.txt" : "TabBisector3d_2.txt")
#define TAB_MGLUT(ds) ((ds == 1) ? "TabMgLut2d.txt" : "TabMgLut3d.txt")
#define TAB_MHLUT(ds) ((ds == 1) ? "TabMhLut2d.txt" : "TabMhLut3d.txt")
#define TAB_RTLUT(ds) ((ds == 1) ? "TabRemyThiel.txt" : "TabRemyThiel3d.txt")


// --------- MACROS --------------------
//Positive Integer operations
#define posIntOver(SQDn,  d, dover)  dover = d;  while (! SQDn_size(SQDn, dover))  dover++
#define posIntPlus(SQDn,  d, dplus)  posIntOver(SQDn,d+1,dplus)
#define posIntUnder(SQDn, d, dunder) dunder = d; while (! SQDn_size(SQDn, dunder)) dunder--
#define posIntLess(SQDn,  d, dless)  posIntUnder(SQDn,d-1,dless)

//Square Decompositions
#define SQDn_get(SQDn, R) ((Coordinates *)(SQDn.ListDecs + SQDn.TabIndDec[R]))
#define SQDn_size(SQDn, R) (SQDn.TabIndDec[R + 1] - SQDn.TabIndDec[R])

//RTLut operations
#define RTLut_get(MLut, Lut, v, R) (Lut[MLut.indmap[v] + (int32_t)(R)]) //get R', such that B^<(x, R) \subseteq B^<(x+v, R')


// --------- TABLES --------------------
typedef struct SQDLut {
  int32_t maxdist;            //maximal distance value in TabIndDec
  int32_t npoints;            //number of points in ListDecs
  int32_t *TabIndDec;         //Indices for ListDecs
  Coordinates *ListDecs;      //Squares decompositions table
} SQDLut;

typedef struct MLut {
  int32_t numd;         // number of vectors
  int32_t rknown;       // known radius
  int32_t ds;           // third dimension of the image (1 if image is 2d)
  int32_t sqdnReqDist;  // required SQDLut.maxdist
  Coordinates *vec;     // the vectors
  int32_t *RR;          // the radii where vectors become necessary
  int32_t *indmap;      // maps the index of vector to its index in the Lut table
} MLut;

typedef int32_t * RTLutCol;



// --------- FUNCTIONS --------------------
/*
  maxdist <= 0: read the entire Lut
*/
extern int32_t lread_SQDLut(SQDLut *SQDn, int32_t maxdist, int32_t ds);
extern int32_t lread_MgLut(MLut *mglut, int32_t maxdist, int32_t ds);
extern int32_t lread_MhLut(MLut *mhlut, int32_t maxdist, int32_t ds);
extern int32_t lread_RTLut(MLut *mglut, RTLutCol *Lut, int32_t maxdist, int32_t ds);
extern int32_t lread_SQD_RT_Luts(SQDLut *SQDn, MLut *mlut, RTLutCol *Lut, int32_t maxdist, int32_t ds);
extern int32_t lread_MLut(MLut *mlut, int32_t maxdist, int32_t ds, FILE *fd);

/* copy */
extern int32_t lcopy_MLut(MLut *dest, const MLut *orig);

/* Free memory */
extern void lfree_MLut(MLut *mlut);
extern void lfree_SQDLut(SQDLut *SQDn);

/* stdout Printing */
extern void lprint_MLut(MLut mlut);

/* file Printing */
extern void lfprint_MLut(MLut mlut, char *filename);

#ifdef __cplusplus
}
#endif
