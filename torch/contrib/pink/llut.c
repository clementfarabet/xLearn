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

Manage look-up tables (Luts) in Pink

See llut.h for details

Author: Andre Vital Saude, 2005

*/

//#define VERBOSE

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mccodimage.h>
#include <avsimage.h>
#include <llut.h>

/* =============================================================== */
int32_t lread_SQDLut(SQDLut *SQDn, int32_t maxdist, int32_t ds) {
/* =============================================================== */
#undef F_NAME
#define F_NAME "lread_SQDLut"
  /* for distmax <= 0: read the whole table */

  char tableI[512];
  char tableL[512];
  FILE *fdi=NULL;
  FILE *fdl=NULL;
  
  int32_t i;

  /* Reading the bissector table from the files */
  sprintf(tableI, "%s/src/tables/%s", getenv("PINK"), TAB_INDDEC(ds));
  sprintf(tableL, "%s/src/tables/%s", getenv("PINK"), TAB_LISTDEC(ds));

  fdi = fopen(tableI, "r");
  if (fdi == NULL) 
  {   
    fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tableI);
    return(0);
  }

  fdl = fopen (tableL, "r");
  if (fdl == NULL) 
  {   
    fprintf(stderr, "%s: error while opening table %s\n", F_NAME, tableL);
    return(0);
  }


  /* Reading SQDn->TabIndDec */
  fscanf(fdi, "%d", &SQDn->maxdist);
  if (maxdist > SQDn->maxdist)
  {
    fprintf(stderr, "%s: bisector table overflow: %d >= %d\n",
	    F_NAME, maxdist, SQDn->maxdist);
    exit(0);
  } else if (maxdist > 0)
    SQDn->maxdist = maxdist;
    
  SQDn->TabIndDec = (int32_t *)malloc((SQDn->maxdist + 2) * sizeof(int32_t));
  if (SQDn->TabIndDec == NULL) 
  {   
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }
  for (i = 0; i <= SQDn->maxdist+1; i++)
    fscanf(fdi,"%d", &SQDn->TabIndDec[i]);
  
  
  
  /* Reading ListDecs */
  fscanf(fdl,"%d",&SQDn->npoints);
#ifdef PARANO
  if (SQDn->TabIndDec[SQDn->maxdist + 1] > SQDn->npoints)
  {
    fprintf(stderr, "%s: bisector table overflow for npoints: %d >= %d\n", 
	    F_NAME, SQDn->TabIndDec[SQDn->maxdist], SQDn->npoints);
    exit(0);
  }
#endif

  SQDn->npoints = SQDn->TabIndDec[SQDn->maxdist + 1];
  SQDn->ListDecs = (Coordinates *)malloc(SQDn->npoints * sizeof(Coordinates));
  if (SQDn->ListDecs == NULL) 
  {   
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }
  
  //read the square decompositions
  if (ds == 1) {
    for (i = 0; i < SQDn->npoints; i++) 
      fscanf(fdl,"%d%d", &SQDn->ListDecs[i].x, &SQDn->ListDecs[i].y);
  } else {
    for (i = 0; i < SQDn->npoints; i++) 
      fscanf(fdl,"%d%d%d", &SQDn->ListDecs[i].x, &SQDn->ListDecs[i].y, &SQDn->ListDecs[i].z);
  }
  
  //close
  fclose(fdi);
  fclose(fdl);

#ifdef VERBOSE
  printf("SQDn->maxdist = %d ; SQDn->npoints = %d\n", SQDn->maxdist, SQDn->npoints);
#endif

  return (1);
} //lread_SQDLut()

/* =============================================================== */
int32_t lread_MgLut(MLut *mglut, int32_t maxdist, int32_t ds)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lread_MgLut"
{
  char tablefilename[512];
  FILE *fd;

  sprintf(tablefilename, "%s/src/tables/%s", getenv("PINK"), TAB_MGLUT(ds));
  fd = fopen(tablefilename, "r");
  if (fd == NULL) 
  {   
    fprintf(stderr, "%s: cannot open file %s\n", F_NAME, tablefilename);
    fclose(fd);
    return(0);
  }

  //read file
  if (!lread_MLut(mglut, maxdist, ds, fd)) {
    fprintf(stderr, "%s: lread_MLut failed\n", F_NAME);
    fclose(fd);
    return(0);
  }

  fclose(fd);

  return 1;
} //lread_MgLut()

/* =============================================================== */
int32_t lread_MhLut(MLut *mhlut, int32_t maxdist, int32_t ds)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lread_MhLut"
{
  char tablefilename[512];
  FILE *fd;
  int32_t i;

  //open file
  sprintf(tablefilename, "%s/src/tables/%s", getenv("PINK"), TAB_MHLUT(ds));
  fd = fopen(tablefilename, "r");
  if (fd == NULL) 
  {   
    fprintf(stderr, "%s: cannot open file %s\n", F_NAME, tablefilename);
    return(0);
  }

  //read file
  if (!lread_MLut(mhlut, maxdist, ds, fd)) {
    fprintf(stderr, "%s: lread_MLut failed\n", F_NAME);
    fclose(fd);
    return(0);
  }

  //correct indmap
  for (i=0; i < mhlut->numd; i++) {
    int32_t map;
    fscanf(fd, "%d", &map);
    //    printf("map=%d, mhlut->rknown=%d\n", map, mhlut->rknown);
    mhlut->indmap[i] = map * mhlut->rknown;
    //    printf("mhlut->indmap[%d]=%d\n", i, mhlut->indmap[i]);
  }

  fclose(fd);

  return 1;
} //lread_MhLut()

/* =============================================================== */
int32_t lread_MLut(MLut *mlut, int32_t maxdist, int32_t ds, FILE *fd)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lread_MLut"
{
  int32_t i;

  //maximal radius
  fscanf(fd,"%d",&mlut->rknown);

  if (maxdist <= 0) {
    maxdist = mlut->rknown;
  } else {
    if (maxdist > mlut->rknown) {   
      fprintf(stderr, "%s: maximal known radius is %d, image needs %d\n", F_NAME, mlut->rknown, maxdist);
      return(0);
    }
    mlut->rknown = maxdist;
  }

  //required maximal distance in SQDLut
  fscanf(fd, "%d", &mlut->sqdnReqDist); 
  
  //number of different directions
  fscanf(fd, "%d", &mlut->numd);

  //The vectors and their radii
  mlut->vec = (Coordinates *)malloc( mlut->numd * sizeof(struct Coordinates));
  mlut->RR = (int32_t *)malloc( mlut->numd * sizeof(int32_t));
  mlut->indmap = (int32_t *)malloc( mlut->numd * sizeof(int32_t));
  if ((mlut->vec == NULL) || (mlut->RR == NULL) || (mlut->indmap == NULL)) {   
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }

  mlut->ds = ds;
  if (ds == 1) {
    int32_t x, y, RR, newnumd;

    for (i=0; i<mlut->numd; i++) {
      fscanf(fd, "%d %d %d", &x, &y, &RR);
      if (RR <= maxdist) {
	mlut->vec[i].x = x;
	mlut->vec[i].y = y;
	mlut->RR[i] = RR;
	newnumd = i;
      }
    }
    mlut->numd = newnumd+1;
  } else {
    int32_t x, y, z, RR, newnumd;

    for (i=0; i<mlut->numd; i++) {
      fscanf(fd, "%d %d %d %d", &x, &y, &z, &RR);
      if (RR <= maxdist) {
	mlut->vec[i].x = x;
	mlut->vec[i].y = y;
	mlut->vec[i].z = z;
	mlut->RR[i] = RR;
	newnumd = i;
      }
    }
    mlut->numd = newnumd+1;
  }

  //indmap
  for (i=0; i < mlut->numd; i++) {
    mlut->indmap[i] = i*mlut->rknown;
  }

  return 1;
} //lread_MLut()


/* =============================================================== */
int32_t lcopy_MLut(MLut *dest, const MLut *orig)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lcopy_MLut"
{
    if (orig == NULL) return 0;
    if (dest == NULL) {
      fprintf(stderr, "%s: dest argument must be allocated\n", F_NAME);
      return(0);
    }

    *dest = *orig;
    dest->vec = (Coordinates *)malloc( dest->numd * sizeof(Coordinates) );
    dest->RR = (int *)malloc( dest->numd * sizeof(int32_t) );
    dest->indmap = (int *)malloc( dest->numd * sizeof(int32_t));

    memcpy((char *)dest->vec, (char *)orig->vec, dest->numd * sizeof(int32_t));
    memcpy((char *)dest->RR, (char *)orig->RR, dest->numd * sizeof(int32_t));
    memcpy((char *)dest->indmap, (char *)orig->indmap, dest->numd * sizeof(int32_t));

    return 1;
}

/* =============================================================== */
void lfree_MLut(MLut *mlut)
/* =============================================================== */
{
  free(mlut->vec);
  free(mlut->RR);
  if (mlut->indmap != NULL) free(mlut->indmap);
}

/* =============================================================== */
void lfree_SQDLut(SQDLut *SQDn)
/* =============================================================== */
{
  free(SQDn->ListDecs);
  free(SQDn->TabIndDec);
}


void lprint_MLut(MLut mlut)
/* =============================================================== */
{
  int32_t i;

  printf("\n\n\nMaximal radius: \t\t%d\n", mlut.rknown);
  printf("Number of directions: \t\t%d\n", mlut.numd);
  printf("Required SQDn max distance: \t%d\n", mlut.sqdnReqDist);
  
  if (mlut.ds == 1) {
    printf("Image dimensions: \t\t2D\n");
    for (i=0; i<mlut.numd; i++) {
      printf("%d\t%d\t%d\n", mlut.vec[i].x, mlut.vec[i].y, mlut.RR[i]);
    }
  } else {
    printf("Image dimensions: \t\t3D\n");
    for (i=0; i<mlut.numd; i++) {
      printf("%d\t%d\t%d\t%d\n", mlut.vec[i].x, mlut.vec[i].y, mlut.vec[i].z, mlut.RR[i]);
    }
  }

  printf("Index Map:\n");
  if (mlut.indmap != NULL) {
    for (i=0; i<mlut.numd; i++) {
      printf("\t%d\t%d\n", mlut.indmap[i] / mlut.rknown, mlut.indmap[i]);
      fflush(stdout); 
      //      getchar;   RETIRE SUITE A UN WARNING "instruction sans effet" MC
    }
    printf("\n");
  } else
    printf("\tNULL\n");

} //lprint_MLut()

/* =============================================================== */
void lprint_RTLut(MLut mlut, int32_t *lcol)
/* =============================================================== */
{
  int32_t i,j;
  
  lprint_MLut(mlut);
  for(i=0; i<mlut.numd; i++)
    for(j=0; j<mlut.rknown; j++)
      printf("%d\n", lcol[i*mlut.rknown + j]);
    
}//lprint_RTLut()
    
/* =============================================================== */
void lfprint_MLut(MLut mlut, char *filename)
/* =============================================================== */
{
  int32_t i;
  FILE *fd;

  fd = fopen(filename, "w");

  fprintf(fd, "%d\n\n", mlut.rknown);
  fprintf(fd, "%d\n\n", mlut.sqdnReqDist); 
  fprintf(fd, "%d\n\n", mlut.numd); 

  if (mlut.ds == 1) {
    for (i=0; i<mlut.numd; i++) {
      fprintf(fd, "%d\t%d\t%d\n", mlut.vec[i].x, mlut.vec[i].y, mlut.RR[i]);
    }
  } else {
    for (i=0; i<mlut.numd; i++) {
      fprintf(fd, "%d\t%d\t%d\t%d\n", mlut.vec[i].x, mlut.vec[i].y, mlut.vec[i].z, mlut.RR[i]);
    }
  }

  fprintf(fd, "\n");
  if (mlut.indmap != NULL) {
    for (i=0; i<mlut.numd; i++) {
      fprintf(fd, "%d ", mlut.indmap[i] / mlut.rknown);
    }
    fprintf(fd, "\n");
  } else
    fprintf(fd, "\tNULL\n");

  fclose(fd);
} //lfprint_mlut()


/* ==================================== */
int32_t lread_rtlut(MLut *mlut, RTLutCol *Lut, int32_t maxdist, int32_t ds)
/* ==================================== */
#undef F_NAME
#define F_NAME "lreadRTLut"
{
#define lcol (*Lut)

  char tablefilename[512];
  int32_t i, j;
  FILE *fd = NULL;

  //open file
  sprintf(tablefilename, "%s/src/tables/%s", getenv("PINK"), TAB_RTLUT(ds));
  fd = fopen(tablefilename, "r");
  if (fd == NULL) 
  {   
    fprintf(stderr, "%s: error while opening table file \"%s\"\n", F_NAME, tablefilename);
    return(0);
  }

  // number of different directions
  fscanf(fd,"%d",&mlut->numd);
  
  //The vectors and their radii
  mlut->vec = (Coordinates *)malloc( mlut->numd * sizeof(struct Coordinates));
  mlut->RR = (int32_t *)malloc( mlut->numd * sizeof(int32_t));
  mlut->indmap = (int32_t *)malloc( mlut->numd * sizeof(int32_t));
  if ((mlut->vec == NULL) || (mlut->RR == NULL)) {   
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }

  mlut->ds = ds;
  if (ds == 1) {
    for (i=0; i<mlut->numd; i++) {
      fscanf(fd, "%d %d %d", &mlut->vec[i].x, &mlut->vec[i].y, &mlut->RR[i]);
      mlut->vec[i].z = 0;
    }
  } else {
    for (i=0; i<mlut->numd; i++) {
      fscanf(fd, "%d %d %d %d", &mlut->vec[i].x, &mlut->vec[i].y, &mlut->vec[i].z, &mlut->RR[i]);
    }
  }

  // correct mlut->numd
  for (i=0; (i<mlut->numd) && (mlut->RR[i] <= maxdist); i++);
  mlut->numd = i;

  //maximal radius
  fscanf(fd,"%d",&mlut->rknown);
  if (maxdist <= 0) {
    maxdist = mlut->rknown;
  } else {
    if (maxdist > mlut->rknown) {   
      fprintf(stderr, "%s: rmax = %d > %d (max permitted by current table)\n", F_NAME, maxdist, mlut->rknown);
      return(0);
    }
    //the following line cannot be done because we need mlut->rknown to read the RTLutCol
    //mlut->rknown = maxdist;
  }

  //RTLutCol
  lcol = (int32_t *)malloc(mlut->numd * maxdist * sizeof(int32_t));
  if (lcol == NULL) {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }

  //RTLutCol read
  mlut->sqdnReqDist = 0;
  for (i = 0; i < mlut->numd; i++) {
    for (j = 0; j < maxdist; j++) {
      fscanf(fd,"%d",&(lcol[i*maxdist + j]));
      if (mlut->sqdnReqDist < lcol[i*maxdist + j])
	mlut->sqdnReqDist = lcol[i*maxdist + j];
    }
    //ignore these
    for (j = maxdist; j < mlut->rknown; j++) {
      int32_t x;
      fscanf(fd,"%d",&x);
    }
  }

  //now we can set mlut->rknown
  mlut->rknown = maxdist;

  //indmap
  for (i=0; i < mlut->numd; i++) {
    mlut->indmap[i] = i*mlut->rknown;
  }

  fclose(fd);

  return 1;
} // lread_rtlut()

//----------------------------------
int32_t lread_SQD_RT_Luts(SQDLut *SQDn, MLut *mlut, RTLutCol *Lut, int32_t maxdist, int32_t ds)
//----------------------------------
#undef F_NAME
#define F_NAME "lhma_readTables"
{
  //Read tables
  if ( ! lread_rtlut(mlut, Lut, maxdist, ds) ) {
    fprintf(stderr, "%s: could not read RTLut\n", F_NAME);
    return 0;
  }

  SQDn->maxdist = mlut->sqdnReqDist;
  if (!lread_SQDLut(SQDn, SQDn->maxdist, ds)) {
    fprintf(stderr, "%s: Could not read SQDn\n", F_NAME);
    return 0;
  }

  return 1;
} //readTables
