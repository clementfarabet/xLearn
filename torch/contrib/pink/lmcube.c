// proposition

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcrbtp.h>
#include <mcmesh.h>
#include <mciomesh.h>
#include <lmcube.h>


/*
#define DEBUG
#define VERBOSE
*/

//#define PHONG

#define VAL_FOND 0
#define BOR_FOND 1
#define BOR_OBJ  125 /* offset a rajouter au label objet pour etiqueter un pixel de bord */
#define TEST_BOR (BOR_FOND + BOR_OBJ) /* les points > TEST_BOR sont des bords d'objets */

/* la Look-Up Table (LUT) est une table de 256 entrees */
/* chaque entree se compose de 19 entiers: */
/* - le nombre de facettes ( <= 6) */
/* - les coordonnees (codees) des sommets des facettes */
static int32_t LUT[256][19];
/* table de decodage des sommets de facettes */
static int32_t DSF[26][2] = {
 {0, 0}, /* lignes paires non utilisees */
 {0, 1}, {0, 0}, /* voxels adjacents au sommet 1, 2, etc */
 {0, 2}, {0, 0}, /* sommet 3 */
 {1, 3}, {0, 0}, /* sommet 5 */
 {2, 3}, {0, 0}, /* sommet 7 */
 {0, 4}, {0, 0}, /* sommet 9 */
 {1, 5}, {0, 0}, /* sommet 11 */
 {0, 0}, {0, 0}, /* sommet 13 (absent) */
 {2, 6}, {0, 0}, /* sommet 15 */
 {3, 7}, {0, 0}, /* sommet 17 */
 {4, 5}, {0, 0}, /* sommet 19 */
 {4, 6}, {0, 0}, /* sommet 21 */
 {5, 7}, {0, 0}, /* sommet 23 */
 {6, 7}          /* sommet 25 */
};
/* table des offsets des sommets du cube */
static int32_t SC[8][3] = {
  {0, 0, 0}, /* sommet 0 */
  {1, 0, 0}, /* sommet 1 */
  {0, 1, 0}, /* sommet 2 */
  {1, 1, 0}, /* sommet 3 */
  {0, 0, 1}, /* sommet 4 */
  {1, 0, 1}, /* sommet 5 */
  {0, 1, 1}, /* sommet 6 */
  {1, 1, 1}  /* sommet 7 */
};

void InitLUT();

#ifdef DEBUG
int32_t LUTold[256][19];
void InitLUTold();
void compareLUTs()
{
  int32_t i, j, k, s;

  for (i = 0; i < 256; i++)
  {
    if (LUT[i][0] != LUTold[i][0]) printf("config %d: nb sommets differents %d,%d\n", 
        i, LUT[i][0], LUTold[i][0]);
    for (j = 1; j <= LUT[i][0]; j++)
    {
      s = LUT[i][j];
      for (k = 1; k <= LUTold[i][0]; k++)
        if (LUTold[i][k] == s) break;
      if (k > LUTold[i][0]) 
      {
        printf("config %x: faces differentes\n", i);
        printf("new: ");
        for (k = 1; k <= LUT[i][0]; k++) printf("%d ", LUT[i][k]);
        printf("\n");
        printf("old: ");
        for (k = 1; k <= LUTold[i][0]; k++) printf("%d ", LUTold[i][k]);
        printf("\n");
      }
    }
  }
}
#endif

/* =============================================================== */
void lmcube_point(int32_t x, int32_t y, int32_t z, uint8_t *cube, int32_t point, 
		  double x_offset, double y_offset, double z_offset,
		  double *xp, double *yp, double *zp)
/* =============================================================== */
/* pour la version "proportionnelle" des marching cubes.
   non utilisee pour le moment.
*/
{
  int32_t p0, p1; uint8_t c0, c1;
  p0 = DSF[point][0]; c0 = cube[p0];
  p1 = DSF[point][1]; c1 = cube[p1];
  switch(point)
  {
    case 1: case 7: case 19: case 25:
      *xp = x_offset + x + ((double)SC[p0][0]*c0 + SC[p1][0]*c1)/(c0+c1);
      *yp = y_offset + y + (double)SC[p0][1];
      *zp = z_offset + z + (double)SC[p0][2];
      break;
    case 3: case 5: case 21: case 23:
      *xp = x_offset + x + (double)SC[p0][0];
      *yp = y_offset + y + ((double)SC[p0][1]*c0 + SC[p1][1]*c1)/(c0+c1);
      *zp = z_offset + z + (double)SC[p0][2];
      break;
    case 9: case 11: case 15: case 17:
      *xp = x_offset + x + (double)SC[p0][0];
      *yp = y_offset + y + (double)SC[p0][1];
      *zp = z_offset + z + ((double)SC[p0][2]*c0 + SC[p1][2]*c1)/(c0+c1);
      break;
  } /* switch(point) */
} /* point() */

/* =============================================================== */
void pointcst(int32_t x, int32_t y, int32_t z, uint8_t *cube, int32_t point, 
              double x_offset, double y_offset, double z_offset,
              double x_factor, double y_factor, double z_factor,
              double *xp, double *yp, double *zp)
/* =============================================================== */
{
  switch(point)
  {
    case 1: case 7: case 19: case 25:
      *xp = x_offset + x_factor * (x + ((double)(SC[DSF[point][1]][0] + SC[DSF[point][0]][0]))/2);
      *yp = y_offset + y_factor * (y + (double)SC[DSF[point][0]][1]);
      *zp = z_offset + z_factor * (z + (double)SC[DSF[point][0]][2]);
      break;
    case 3: case 5: case 21: case 23:
      *xp = x_offset + x_factor * (x + (double)SC[DSF[point][0]][0]);
      *yp = y_offset + y_factor * (y + ((double)(SC[DSF[point][1]][1] + SC[DSF[point][0]][1]))/2);
      *zp = z_offset + z_factor * (z + (double)SC[DSF[point][0]][2]);
      break;
    case 9: case 11: case 15: case 17:
      *xp = x_offset + x_factor * (x + (double)SC[DSF[point][0]][0]);
      *yp = y_offset + y_factor * (y + (double)SC[DSF[point][0]][1]);
      *zp = z_offset + z_factor * (z + ((double)(SC[DSF[point][1]][2] + SC[DSF[point][0]][2]))/2);
      break;
  } /* switch(point) */
} /* pointcst() */

/* =============================================================== */
int32_t estfixe(uint8_t *cube, int32_t point)
/* =============================================================== */
/* 
  Un point est fixe si :
  - il est entre un point BOR_FOND et un point > TEST_BOR
  - il est entre deux points > BOR_OBJ
*/
{
  if ((cube[DSF[point][0]]==BOR_FOND) && (cube[DSF[point][1]]>TEST_BOR)) return 1;
  if ((cube[DSF[point][1]]==BOR_FOND) && (cube[DSF[point][0]]>TEST_BOR)) return 1;
  if ((cube[DSF[point][0]]>BOR_OBJ) && (cube[DSF[point][1]]>BOR_OBJ))  return 1;
  return 0;
} /* estfixe() */

/* =============================================================== */
int32_t lmarchingcubes(struct xvimage * f, uint8_t v, 
                   int32_t nregul, int32_t obj_id, FILE *fileout, 
                   int32_t format)
/* =============================================================== */
{
  int32_t i, j, k, x, y, z, nbfac, fac;
  int32_t rs, cs, ps, ds, N;
  double xoff, yoff, zoff;
  double xdim, ydim, zdim;
  uint32_t c;
  uint8_t * F;
  uint8_t cube[8];
  uint8_t s;
  double x1, y1, z1, x2, y2, z2, x3, y3, z3 ;
  meshbox MB0; 

  /* v est la valeur de seuil. si v == 0 alors il s'agit d'une image binaire */
  if (v == 0) s = 1; else s = v;

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  ps = rs * cs;
  N = ps * ds;
  F = UCHARDATA(f);

#ifdef VERBOSE
  printf("lmarchingcubes: xdim=%g, ydim=%g, zdim=%g\n", f->xdim,f->ydim,f->zdim);
#endif

  xdim = f->xdim;
  ydim = f->zdim; // attention:
  zdim = f->ydim; // inversion necessaire

  xoff = 0.5-rs/2.0;
  yoff = 0.5-ds/2.0;
  zoff = 0.5-cs/2.0;

  for (z = 0; z < cs-1; z++) /* coordonnees image sortie */
  for (y = 0; y < ds-1; y++)
  for (x = 0; x < rs-1; x++)
  {
    /* calcule les coordonnees dans l'image 
       du point de base du cube 2x2x2 correspondant */
    i = x; j = cs - 1 - z; k = y;
    /* encode le cube 2x2x2 */
    c = 0;
    cube[0] = F[k*ps     + j*rs     + i];   if (cube[0]>=s) c |= 0x01; 
    cube[1] = F[k*ps     + j*rs     + i+1]; if (cube[1]>=s) c |= 0x02; 
    cube[2] = F[(k+1)*ps + j*rs     + i];   if (cube[2]>=s) c |= 0x04; 
    cube[3] = F[(k+1)*ps + j*rs     + i+1]; if (cube[3]>=s) c |= 0x08; 
    cube[4] = F[k*ps     + (j-1)*rs + i];   if (cube[4]>=s) c |= 0x10; 
    cube[5] = F[k*ps     + (j-1)*rs + i+1]; if (cube[5]>=s) c |= 0x20; 
    cube[6] = F[(k+1)*ps + (j-1)*rs + i];   if (cube[6]>=s) c |= 0x40; 
    cube[7] = F[(k+1)*ps + (j-1)*rs + i+1]; if (cube[7]>=s) c |= 0x80; 

    /* trouve le nombre de facettes dans la LUT */
    nbfac = LUT[c][0];
#ifdef DEBUG
    printf("synth %d %d %d voxel %d %d %d code %x nbfac %d\n", 
           x, y, z, i, j, k, c, nbfac);
#endif

    /* genere les facettes */
    for (fac = 1; fac <= nbfac; fac+=3)
    {
      pointcst(x,y,z,cube,LUT[c][fac],xoff,yoff,zoff, xdim,ydim,zdim, &x1, &y1, &z1);
      pointcst(x,y,z,cube,LUT[c][fac+1],xoff,yoff,zoff, xdim,ydim,zdim, &x2, &y2, &z2);
      pointcst(x,y,z,cube,LUT[c][fac+2],xoff,yoff,zoff, xdim,ydim,zdim, &x3, &y3, &z3);
      (void)AddFace(x1, y1, z1, x2, y2, z2, x3, y3, z3);
    }
  } /* for z, y, x */

  RegulMeshLaplacian(nregul); 
  //if (nregul) RegulMeshHamam(1.0);

  switch (format)
  {
    case T_POV:
      CalculNormales();
      BoundingBoxMesh(&MB0);
      genheaderPOV(fileout, obj_id, MB0);
#ifdef PHONG
      SaveMeshSPOV(fileout);
#else
      SaveMeshPOV(fileout);
#endif
      genfooterPOV(fileout);
      break;
    case T_POVB:
      CalculNormales();
#ifdef PHONG
      SaveMeshSPOV(fileout);
#else
      SaveMeshPOV(fileout);
#endif
      break;
    case T_COL:
      genheaderCOL(fileout, Faces->cur);
      CalculNormales();
      SaveMeshCOL(fileout, obj_id);
      break;
    case T_MCM:
      CalculNormales();
      CalculNormalesFaces();
      Edges = AllocEdges(1000);
      ComputeEdges();
      SaveMeshMCM(fileout);
      break;
    case T_AC:
      BoundingBoxMesh(&MB0);
      genheaderAC(fileout, MB0, 1.0, 1.0, 1.0, (char *)"");
      SaveMeshAC(fileout);
      genfooterAC(fileout);
      break;
    case T_GL:
      CalculNormales();
      SaveMeshGL(fileout, obj_id);
      break;
    case T_VTK:
      genheaderVTK(fileout, (char *)"mcube output");
      SaveMeshVTK(fileout);
      break;
    case T_VTK_PYTHON:
      genheaderVTK_PYTHON(fileout, (char *)"mcube output to python");
      SaveMeshVTK_PYTHON(fileout);
      break;
    case T_RAW:
      CalculNormales();
      CalculNormalesFaces();
      fprintf(fileout, "%d %d %d %d\n", Vertices->cur, Faces->cur, 
	      Vertices->cur, Faces->cur);
      // SOMMETS
      for (i = 0; i < Vertices->cur; i++)
      {
	//      if (Vertices->lab[i]) nfixes++;
	fprintf(fileout, "%g %g %g", Vertices->v[i].x, Vertices->v[i].y,
		Vertices->v[i].z);
	fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
      //FACES
      for (i = 0; i < Faces->cur; i++)
	fprintf(fileout, "%d %d %d\n", Faces->f[i].vert[0],
		Faces->f[i].vert[1], Faces->f[i].vert[2]);
      fprintf(fileout, "\n");

      // NORMALES AUX SOMMETS
      for (i = 0; i < Vertices->cur; i++)
      {
	fprintf(fileout, "%g %g %g", Vertices->v[i].xp, Vertices->v[i].yp,
		Vertices->v[i].zp);
	fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
      // NORMALES AUX FACES
      for (i = 0; i < Faces->cur; i++)
      {
	fprintf(fileout, "%g %g %g", Faces->f[i].xn, Faces->f[i].yn,
		Faces->f[i].zn);
	fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
   
  } /* switch (format) */

  return 1;

} /* lmarchingcubes() */

/* =============================================================== */
int32_t lmarchingcubes2( struct xvimage * f,
			 int32_t nregul, int32_t obj_id, FILE *fileout, 
			 int32_t format)
/* =============================================================== */
// version avec preservation de points fixes
/* 
   f est une image en 2n+2 niveaux: (n = nombre d'objets)
     VAL_FOND   = 0 pour le fond
     BOR_FOND   = 1 pour le fond "pres" d'un objet
     2              pour l'interieur du 1er objet
     2+BOR_OBJ      pour le bord du premier objet
     .
     .
     .
     k              pour l'interieur du kieme objet
     k+BOR_OBJ      pour le bord du kieme objet

   avant l'appel a RegulMesh, le label de tout sommet de facette situe entre
   deux points de valeurs differentes et tous deux > 0 est mis a 1
*/
{
  int32_t i, j, k, x, y, z, nbfac, fac;
  int32_t rs, cs, ps, ds, N;
  double xoff, yoff, zoff;
  double xdim, ydim, zdim;
  uint32_t c;
  uint8_t * F;
  uint8_t cube[8];
  double x1, y1, z1, x2, y2, z2, x3, y3, z3 ;
  meshbox MB0; 
  int32_t fix1, fix2, fix3;

  rs = rowsize(f);
  cs = colsize(f);
  ds = depth(f);
  ps = rs * cs;
  N = ps * ds;
  F = UCHARDATA(f);

#ifdef VERBOSE
  printf("lmarchingcubes: xdim=%g, ydim=%g, zdim=%g\n", f->xdim,f->ydim,f->zdim);
#endif

  xdim = f->xdim;
  ydim = f->zdim; // attention:
  zdim = f->ydim; // inversion necessaire

  xoff = 0.5-rs/2.0;
  yoff = 0.5-ds/2.0;
  zoff = 0.5-cs/2.0;

  for (z = 0; z < cs-1; z++) /* coordonnees image sortie */
  for (y = 0; y < ds-1; y++)
  for (x = 0; x < rs-1; x++)
  {
    /* calcule les coordonnees dans l'image 
       du point de base du cube 2x2x2 correspondant */
    i = x; j = cs - 1 - z; k = y;
    /* encode le cube 2x2x2 */
    c = 0;
    cube[0] = F[k*ps     + j*rs     + i];   if ((cube[0]%BOR_OBJ)==obj_id) c |= 0x01; 
    cube[1] = F[k*ps     + j*rs     + i+1]; if ((cube[1]%BOR_OBJ)==obj_id) c |= 0x02; 
    cube[2] = F[(k+1)*ps + j*rs     + i];   if ((cube[2]%BOR_OBJ)==obj_id) c |= 0x04; 
    cube[3] = F[(k+1)*ps + j*rs     + i+1]; if ((cube[3]%BOR_OBJ)==obj_id) c |= 0x08; 
    cube[4] = F[k*ps     + (j-1)*rs + i];   if ((cube[4]%BOR_OBJ)==obj_id) c |= 0x10; 
    cube[5] = F[k*ps     + (j-1)*rs + i+1]; if ((cube[5]%BOR_OBJ)==obj_id) c |= 0x20; 
    cube[6] = F[(k+1)*ps + (j-1)*rs + i];   if ((cube[6]%BOR_OBJ)==obj_id) c |= 0x40; 
    cube[7] = F[(k+1)*ps + (j-1)*rs + i+1]; if ((cube[7]%BOR_OBJ)==obj_id) c |= 0x80; 

    /* trouve le nombre de facettes dans la LUT */
    nbfac = LUT[c][0];
#ifdef DEBUG
    printf("synth %d %d %d voxel %d %d %d code %x nbfac %d\n", 
           x, y, z, i, j, k, c, nbfac);
#endif

    /* genere les facettes */
    for (fac = 1; fac <= nbfac; fac+=3)
    {
      pointcst(x,y,z,cube,LUT[c][fac],xoff,yoff,zoff, xdim,ydim,zdim, &x1, &y1, &z1);
      pointcst(x,y,z,cube,LUT[c][fac+1],xoff,yoff,zoff, xdim,ydim,zdim, &x2, &y2, &z2);
      pointcst(x,y,z,cube,LUT[c][fac+2],xoff,yoff,zoff, xdim,ydim,zdim, &x3, &y3, &z3);
      fix1 = estfixe(cube,LUT[c][fac]);      
      fix2 = estfixe(cube,LUT[c][fac+1]);      
      fix3 = estfixe(cube,LUT[c][fac+2]);      
      (void)AddFaceFixe(x1, y1, z1, x2, y2, z2, x3, y3, z3, fix1, fix2, fix3);
    }
  } /* for z, y, x */

  Edges = AllocEdges(1000);
  ComputeEdges();
  ComputeCurvatures();
  //PrintMesh();

  RegulMeshLaplacian(nregul); 

  switch (format)
  {
    case T_POV:
      BoundingBoxMesh(&MB0);
      genheaderPOV(fileout, obj_id, MB0);
      SaveMeshSPOV(fileout);
      genfooterPOV(fileout);
      break;
    case T_POVB:
      SaveMeshSPOV(fileout);
      break;
    case T_COL:
      genheaderCOL(fileout, Faces->cur);
      CalculNormales();
      SaveMeshCOL(fileout, obj_id);
      break;
    case T_MCM:
      CalculNormales();
      CalculNormalesFaces();
      SaveMeshMCM(fileout);
      break;
    case T_AC:
      BoundingBoxMesh(&MB0);
      genheaderAC(fileout, MB0, 1.0, 1.0, 1.0, (char *)"");
      SaveMeshAC(fileout);
      genfooterAC(fileout);
      break;
    case T_GL:
      CalculNormales();
      SaveMeshGL(fileout, obj_id);
      break;
    case T_VTK:
      genheaderVTK(fileout, (char *)"mcube output");
      SaveMeshVTK(fileout);
      break;
    case T_VTK_PYTHON:
      genheaderVTK_PYTHON(fileout, (char *)"mcube output to python");
      SaveMeshVTK_PYTHON(fileout);
      break;
    case T_RAW:
      CalculNormales();
      CalculNormalesFaces();
      fprintf(fileout, "%d %d %d %d\n", Vertices->cur, Faces->cur, 
	      Vertices->cur, Faces->cur);
      // SOMMETS
      for (i = 0; i < Vertices->cur; i++)
      {
	//      if (Vertices->lab[i]) nfixes++;
	fprintf(fileout, "%g %g %g", Vertices->v[i].x, Vertices->v[i].y,
		Vertices->v[i].z);
	fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
      //FACES
      for (i = 0; i < Faces->cur; i++)
	fprintf(fileout, "%d %d %d\n", Faces->f[i].vert[0],
		Faces->f[i].vert[1], Faces->f[i].vert[2]);
      fprintf(fileout, "\n");

      // NORMALES AUX SOMMETS
      for (i = 0; i < Vertices->cur; i++)
      {
	fprintf(fileout, "%g %g %g", Vertices->v[i].xp, Vertices->v[i].yp,
		Vertices->v[i].zp);
	fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
      // NORMALES AUX FACES
      for (i = 0; i < Faces->cur; i++)
      {
	fprintf(fileout, "%g %g %g", Faces->f[i].xn, Faces->f[i].yn,
		Faces->f[i].zn);
	fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
  } /* switch (format) */

  return 1;

} /* lmarchingcubes2() */


void InitLUT()
{
LUT[0][0]=0;
LUT[1][0]=3;LUT[1][1]=1;LUT[1][2]=9;LUT[1][3]=3;
LUT[2][0]=3;LUT[2][1]=1;LUT[2][2]=5;LUT[2][3]=11;
LUT[3][0]=6;LUT[3][1]=3;LUT[3][2]=5;LUT[3][3]=11;LUT[3][4]=3;LUT[3][5]=11;LUT[3][6]=9;
LUT[4][0]=3;LUT[4][1]=3;LUT[4][2]=15;LUT[4][3]=7;
LUT[5][0]=6;LUT[5][1]=1;LUT[5][2]=9;LUT[5][3]=7;LUT[5][4]=7;LUT[5][5]=9;LUT[5][6]=15;
LUT[6][0]=12;LUT[6][1]=1;LUT[6][2]=3;LUT[6][3]=15;LUT[6][4]=11;LUT[6][5]=1;LUT[6][6]=15;LUT[6][7]=7;LUT[6][8]=5;LUT[6][9]=11;LUT[6][10]=7;LUT[6][11]=11;LUT[6][12]=15;
LUT[7][0]=9;LUT[7][1]=7;LUT[7][2]=5;LUT[7][3]=11;LUT[7][4]=7;LUT[7][5]=11;LUT[7][6]=15;LUT[7][7]=9;LUT[7][8]=15;LUT[7][9]=11;
LUT[8][0]=3;LUT[8][1]=5;LUT[8][2]=7;LUT[8][3]=17;
LUT[9][0]=12;LUT[9][1]=3;LUT[9][2]=7;LUT[9][3]=17;LUT[9][4]=3;LUT[9][5]=17;LUT[9][6]=9;LUT[9][7]=1;LUT[9][8]=9;LUT[9][9]=5;LUT[9][10]=5;LUT[9][11]=9;LUT[9][12]=17;
LUT[10][0]=6;LUT[10][1]=1;LUT[10][2]=7;LUT[10][3]=17;LUT[10][4]=1;LUT[10][5]=17;LUT[10][6]=11;
LUT[11][0]=9;LUT[11][1]=3;LUT[11][2]=7;LUT[11][3]=17;LUT[11][4]=3;LUT[11][5]=17;LUT[11][6]=9;LUT[11][7]=9;LUT[11][8]=17;LUT[11][9]=11;
LUT[12][0]=6;LUT[12][1]=5;LUT[12][2]=15;LUT[12][3]=17;LUT[12][4]=3;LUT[12][5]=15;LUT[12][6]=5;
LUT[13][0]=9;LUT[13][1]=1;LUT[13][2]=9;LUT[13][3]=5;LUT[13][4]=5;LUT[13][5]=9;LUT[13][6]=17;LUT[13][7]=9;LUT[13][8]=15;LUT[13][9]=17;
LUT[14][0]=9;LUT[14][1]=1;LUT[14][2]=3;LUT[14][3]=15;LUT[14][4]=1;LUT[14][5]=15;LUT[14][6]=11;LUT[14][7]=11;LUT[14][8]=15;LUT[14][9]=17;
LUT[15][0]=6;LUT[15][1]=9;LUT[15][2]=15;LUT[15][3]=11;LUT[15][4]=11;LUT[15][5]=15;LUT[15][6]=17;
LUT[16][0]=3;LUT[16][1]=9;LUT[16][2]=19;LUT[16][3]=21;
LUT[17][0]=6;LUT[17][1]=1;LUT[17][2]=19;LUT[17][3]=21;LUT[17][4]=1;LUT[17][5]=21;LUT[17][6]=3;
LUT[18][0]=12;LUT[18][1]=1;LUT[18][2]=5;LUT[18][3]=9;LUT[18][4]=5;LUT[18][5]=21;LUT[18][6]=9;LUT[18][7]=11;LUT[18][8]=19;LUT[18][9]=21;LUT[18][10]=5;LUT[18][11]=11;LUT[18][12]=21;
LUT[19][0]=9;LUT[19][1]=11;LUT[19][2]=19;LUT[19][3]=21;LUT[19][4]=5;LUT[19][5]=11;LUT[19][6]=21;LUT[19][7]=3;LUT[19][8]=5;LUT[19][9]=21;
LUT[20][0]=12;LUT[20][1]=3;LUT[20][2]=9;LUT[20][3]=19;LUT[20][4]=3;LUT[20][5]=19;LUT[20][6]=7;LUT[20][7]=7;LUT[20][8]=21;LUT[20][9]=15;LUT[20][10]=7;LUT[20][11]=19;LUT[20][12]=21;
LUT[21][0]=9;LUT[21][1]=7;LUT[21][2]=21;LUT[21][3]=15;LUT[21][4]=7;LUT[21][5]=19;LUT[21][6]=21;LUT[21][7]=1;LUT[21][8]=19;LUT[21][9]=7;
LUT[22][0]=15;LUT[22][1]=1;LUT[22][2]=3;LUT[22][3]=9;LUT[22][4]=7;LUT[22][5]=5;LUT[22][6]=11;LUT[22][7]=7;LUT[22][8]=11;LUT[22][9]=15;LUT[22][10]=15;LUT[22][11]=11;LUT[22][12]=19;LUT[22][13]=15;LUT[22][14]=19;LUT[22][15]=21;
LUT[23][0]=12;LUT[23][1]=7;LUT[23][2]=5;LUT[23][3]=11;LUT[23][4]=7;LUT[23][5]=11;LUT[23][6]=15;LUT[23][7]=15;LUT[23][8]=11;LUT[23][9]=19;LUT[23][10]=15;LUT[23][11]=19;LUT[23][12]=21;
LUT[24][0]=18;LUT[24][1]=5;LUT[24][2]=7;LUT[24][3]=9;LUT[24][4]=7;LUT[24][5]=21;LUT[24][6]=9;LUT[24][7]=7;LUT[24][8]=17;LUT[24][9]=21;LUT[24][10]=17;LUT[24][11]=19;LUT[24][12]=21;LUT[24][13]=5;LUT[24][14]=9;LUT[24][15]=19;LUT[24][16]=5;LUT[24][17]=19;LUT[24][18]=17;
LUT[25][0]=15;LUT[25][1]=1;LUT[25][2]=19;LUT[25][3]=5;LUT[25][4]=5;LUT[25][5]=19;LUT[25][6]=17;LUT[25][7]=17;LUT[25][8]=19;LUT[25][9]=21;LUT[25][10]=3;LUT[25][11]=7;LUT[25][12]=21;LUT[25][13]=7;LUT[25][14]=17;LUT[25][15]=21;
LUT[26][0]=15;LUT[26][1]=1;LUT[26][2]=7;LUT[26][3]=9;LUT[26][4]=7;LUT[26][5]=21;LUT[26][6]=9;LUT[26][7]=7;LUT[26][8]=17;LUT[26][9]=21;LUT[26][10]=11;LUT[26][11]=19;LUT[26][12]=17;LUT[26][13]=17;LUT[26][14]=19;LUT[26][15]=21;
LUT[27][0]=12;LUT[27][1]=3;LUT[27][2]=7;LUT[27][3]=11;LUT[27][4]=7;LUT[27][5]=17;LUT[27][6]=11;LUT[27][7]=3;LUT[27][8]=11;LUT[27][9]=19;LUT[27][10]=3;LUT[27][11]=19;LUT[27][12]=21;
LUT[28][0]=15;LUT[28][1]=15;LUT[28][2]=17;LUT[28][3]=21;LUT[28][4]=17;LUT[28][5]=19;LUT[28][6]=21;LUT[28][7]=5;LUT[28][8]=19;LUT[28][9]=17;LUT[28][10]=3;LUT[28][11]=9;LUT[28][12]=5;LUT[28][13]=5;LUT[28][14]=9;LUT[28][15]=19;
LUT[29][0]=12;LUT[29][1]=5;LUT[29][2]=15;LUT[29][3]=17;LUT[29][4]=15;LUT[29][5]=5;LUT[29][6]=1;LUT[29][7]=15;LUT[29][8]=1;LUT[29][9]=21;LUT[29][10]=21;LUT[29][11]=1;LUT[29][12]=19;
LUT[30][0]=12;LUT[30][1]=1;LUT[30][2]=3;LUT[30][3]=9;LUT[30][4]=11;LUT[30][5]=15;LUT[30][6]=17;LUT[30][7]=15;LUT[30][8]=11;LUT[30][9]=19;LUT[30][10]=15;LUT[30][11]=19;LUT[30][12]=21;
LUT[31][0]=9;LUT[31][1]=11;LUT[31][2]=15;LUT[31][3]=17;LUT[31][4]=15;LUT[31][5]=11;LUT[31][6]=19;LUT[31][7]=15;LUT[31][8]=19;LUT[31][9]=21;
LUT[32][0]=3;LUT[32][1]=11;LUT[32][2]=23;LUT[32][3]=19;
LUT[33][0]=12;LUT[33][1]=3;LUT[33][2]=19;LUT[33][3]=9;LUT[33][4]=3;LUT[33][5]=23;LUT[33][6]=19;LUT[33][7]=1;LUT[33][8]=11;LUT[33][9]=23;LUT[33][10]=1;LUT[33][11]=23;LUT[33][12]=3;
LUT[34][0]=6;LUT[34][1]=1;LUT[34][2]=5;LUT[34][3]=19;LUT[34][4]=5;LUT[34][5]=23;LUT[34][6]=19;
LUT[35][0]=9;LUT[35][1]=3;LUT[35][2]=19;LUT[35][3]=9;LUT[35][4]=3;LUT[35][5]=23;LUT[35][6]=19;LUT[35][7]=3;LUT[35][8]=5;LUT[35][9]=23;
LUT[36][0]=18;LUT[36][1]=15;LUT[36][2]=23;LUT[36][3]=19;LUT[36][4]=3;LUT[36][5]=15;LUT[36][6]=19;LUT[36][7]=3;LUT[36][8]=19;LUT[36][9]=11;LUT[36][10]=3;LUT[36][11]=11;LUT[36][12]=7;LUT[36][13]=7;LUT[36][14]=23;LUT[36][15]=15;LUT[36][16]=7;LUT[36][17]=11;LUT[36][18]=23;
LUT[37][0]=15;LUT[37][1]=9;LUT[37][2]=15;LUT[37][3]=19;LUT[37][4]=15;LUT[37][5]=23;LUT[37][6]=19;LUT[37][7]=7;LUT[37][8]=23;LUT[37][9]=15;LUT[37][10]=1;LUT[37][11]=11;LUT[37][12]=7;LUT[37][13]=7;LUT[37][14]=11;LUT[37][15]=23;
LUT[38][0]=15;LUT[38][1]=5;LUT[38][2]=23;LUT[38][3]=7;LUT[38][4]=7;LUT[38][5]=23;LUT[38][6]=15;LUT[38][7]=15;LUT[38][8]=23;LUT[38][9]=19;LUT[38][10]=1;LUT[38][11]=3;LUT[38][12]=19;LUT[38][13]=3;LUT[38][14]=15;LUT[38][15]=19;
LUT[39][0]=12;LUT[39][1]=5;LUT[39][2]=23;LUT[39][3]=19;LUT[39][4]=5;LUT[39][5]=19;LUT[39][6]=9;LUT[39][7]=5;LUT[39][8]=9;LUT[39][9]=7;LUT[39][10]=7;LUT[39][11]=9;LUT[39][12]=15;
LUT[40][0]=12;LUT[40][1]=5;LUT[40][2]=7;LUT[40][3]=11;LUT[40][4]=7;LUT[40][5]=19;LUT[40][6]=11;LUT[40][7]=17;LUT[40][8]=23;LUT[40][9]=19;LUT[40][10]=7;LUT[40][11]=17;LUT[40][12]=19;
LUT[41][0]=15;LUT[41][1]=1;LUT[41][2]=11;LUT[41][3]=5;LUT[41][4]=3;LUT[41][5]=19;LUT[41][6]=9;LUT[41][7]=3;LUT[41][8]=23;LUT[41][9]=19;LUT[41][10]=3;LUT[41][11]=7;LUT[41][12]=23;LUT[41][13]=7;LUT[41][14]=17;LUT[41][15]=23;
LUT[42][0]=9;LUT[42][1]=17;LUT[42][2]=23;LUT[42][3]=19;LUT[42][4]=7;LUT[42][5]=17;LUT[42][6]=19;LUT[42][7]=1;LUT[42][8]=7;LUT[42][9]=19;
LUT[43][0]=12;LUT[43][1]=3;LUT[43][2]=19;LUT[43][3]=9;LUT[43][4]=3;LUT[43][5]=23;LUT[43][6]=19;LUT[43][7]=3;LUT[43][8]=7;LUT[43][9]=23;LUT[43][10]=7;LUT[43][11]=17;LUT[43][12]=23;
LUT[44][0]=15;LUT[44][1]=3;LUT[44][2]=11;LUT[44][3]=5;LUT[44][4]=3;LUT[44][5]=19;LUT[44][6]=11;LUT[44][7]=3;LUT[44][8]=15;LUT[44][9]=19;LUT[44][10]=15;LUT[44][11]=17;LUT[44][12]=23;LUT[44][13]=15;LUT[44][14]=23;LUT[44][15]=19;
LUT[45][0]=12;LUT[45][1]=1;LUT[45][2]=11;LUT[45][3]=5;LUT[45][4]=9;LUT[45][5]=15;LUT[45][6]=17;LUT[45][7]=9;LUT[45][8]=17;LUT[45][9]=23;LUT[45][10]=9;LUT[45][11]=23;LUT[45][12]=19;
LUT[46][0]=12;LUT[46][1]=1;LUT[46][2]=3;LUT[46][3]=17;LUT[46][4]=3;LUT[46][5]=15;LUT[46][6]=17;LUT[46][7]=1;LUT[46][8]=17;LUT[46][9]=23;LUT[46][10]=1;LUT[46][11]=23;LUT[46][12]=19;
LUT[47][0]=9;LUT[47][1]=9;LUT[47][2]=15;LUT[47][3]=17;LUT[47][4]=9;LUT[47][5]=17;LUT[47][6]=23;LUT[47][7]=9;LUT[47][8]=23;LUT[47][9]=19;
LUT[48][0]=6;LUT[48][1]=9;LUT[48][2]=11;LUT[48][3]=23;LUT[48][4]=9;LUT[48][5]=23;LUT[48][6]=21;
LUT[49][0]=9;LUT[49][1]=1;LUT[49][2]=11;LUT[49][3]=23;LUT[49][4]=1;LUT[49][5]=23;LUT[49][6]=3;LUT[49][7]=3;LUT[49][8]=23;LUT[49][9]=21;
LUT[50][0]=9;LUT[50][1]=1;LUT[50][2]=5;LUT[50][3]=9;LUT[50][4]=5;LUT[50][5]=21;LUT[50][6]=9;LUT[50][7]=5;LUT[50][8]=23;LUT[50][9]=21;
LUT[51][0]=6;LUT[51][1]=3;LUT[51][2]=23;LUT[51][3]=21;LUT[51][4]=3;LUT[51][5]=5;LUT[51][6]=23;
LUT[52][0]=15;LUT[52][1]=3;LUT[52][2]=9;LUT[52][3]=11;LUT[52][4]=3;LUT[52][5]=11;LUT[52][6]=7;LUT[52][7]=7;LUT[52][8]=11;LUT[52][9]=23;LUT[52][10]=15;LUT[52][11]=23;LUT[52][12]=21;LUT[52][13]=7;LUT[52][14]=23;LUT[52][15]=15;
LUT[53][0]=12;LUT[53][1]=1;LUT[53][2]=11;LUT[53][3]=21;LUT[53][4]=11;LUT[53][5]=23;LUT[53][6]=21;LUT[53][7]=1;LUT[53][8]=21;LUT[53][9]=15;LUT[53][10]=1;LUT[53][11]=15;LUT[53][12]=7;
LUT[54][0]=12;LUT[54][1]=1;LUT[54][2]=3;LUT[54][3]=9;LUT[54][4]=5;LUT[54][5]=23;LUT[54][6]=21;LUT[54][7]=5;LUT[54][8]=21;LUT[54][9]=15;LUT[54][10]=5;LUT[54][11]=15;LUT[54][12]=7;
LUT[55][0]=9;LUT[55][1]=5;LUT[55][2]=23;LUT[55][3]=21;LUT[55][4]=5;LUT[55][5]=21;LUT[55][6]=15;LUT[55][7]=5;LUT[55][8]=15;LUT[55][9]=7;
LUT[56][0]=15;LUT[56][1]=17;LUT[56][2]=23;LUT[56][3]=21;LUT[56][4]=7;LUT[56][5]=17;LUT[56][6]=21;LUT[56][7]=7;LUT[56][8]=21;LUT[56][9]=9;LUT[56][10]=5;LUT[56][11]=9;LUT[56][12]=11;LUT[56][13]=5;LUT[56][14]=7;LUT[56][15]=9;
LUT[57][0]=12;LUT[57][1]=1;LUT[57][2]=11;LUT[57][3]=5;LUT[57][4]=3;LUT[57][5]=23;LUT[57][6]=21;LUT[57][7]=3;LUT[57][8]=7;LUT[57][9]=23;LUT[57][10]=7;LUT[57][11]=17;LUT[57][12]=23;
LUT[58][0]=12;LUT[58][1]=9;LUT[58][2]=23;LUT[58][3]=21;LUT[58][4]=1;LUT[58][5]=23;LUT[58][6]=9;LUT[58][7]=1;LUT[58][8]=17;LUT[58][9]=23;LUT[58][10]=1;LUT[58][11]=7;LUT[58][12]=17;
LUT[59][0]=9;LUT[59][1]=3;LUT[59][2]=23;LUT[59][3]=21;LUT[59][4]=3;LUT[59][5]=7;LUT[59][6]=23;LUT[59][7]=7;LUT[59][8]=17;LUT[59][9]=23;
LUT[60][0]=12;LUT[60][1]=3;LUT[60][2]=11;LUT[60][3]=5;LUT[60][4]=3;LUT[60][5]=9;LUT[60][6]=11;LUT[60][7]=15;LUT[60][8]=17;LUT[60][9]=23;LUT[60][10]=15;LUT[60][11]=23;LUT[60][12]=21;
LUT[61][0]=9;LUT[61][1]=1;LUT[61][2]=11;LUT[61][3]=5;LUT[61][4]=15;LUT[61][5]=17;LUT[61][6]=23;LUT[61][7]=15;LUT[61][8]=23;LUT[61][9]=21;
LUT[62][0]=9;LUT[62][1]=1;LUT[62][2]=3;LUT[62][3]=9;LUT[62][4]=15;LUT[62][5]=23;LUT[62][6]=21;LUT[62][7]=15;LUT[62][8]=17;LUT[62][9]=23;
LUT[63][0]=6;LUT[63][1]=15;LUT[63][2]=17;LUT[63][3]=23;LUT[63][4]=15;LUT[63][5]=23;LUT[63][6]=21;
LUT[64][0]=3;LUT[64][1]=15;LUT[64][2]=21;LUT[64][3]=25;
LUT[65][0]=12;LUT[65][1]=9;LUT[65][2]=21;LUT[65][3]=25;LUT[65][4]=1;LUT[65][5]=9;LUT[65][6]=25;LUT[65][7]=1;LUT[65][8]=15;LUT[65][9]=3;LUT[65][10]=1;LUT[65][11]=25;LUT[65][12]=15;
LUT[66][0]=18;LUT[66][1]=1;LUT[66][2]=21;LUT[66][3]=11;LUT[66][4]=1;LUT[66][5]=15;LUT[66][6]=21;LUT[66][7]=1;LUT[66][8]=5;LUT[66][9]=15;LUT[66][10]=5;LUT[66][11]=25;LUT[66][12]=15;LUT[66][13]=11;LUT[66][14]=21;LUT[66][15]=25;LUT[66][16]=5;LUT[66][17]=11;LUT[66][18]=25;
LUT[67][0]=15;LUT[67][1]=3;LUT[67][2]=5;LUT[67][3]=15;LUT[67][4]=5;LUT[67][5]=25;LUT[67][6]=15;LUT[67][7]=5;LUT[67][8]=11;LUT[67][9]=25;LUT[67][10]=9;LUT[67][11]=21;LUT[67][12]=11;LUT[67][13]=11;LUT[67][14]=21;LUT[67][15]=25;
LUT[68][0]=6;LUT[68][1]=3;LUT[68][2]=25;LUT[68][3]=7;LUT[68][4]=3;LUT[68][5]=21;LUT[68][6]=25;
LUT[69][0]=9;LUT[69][1]=9;LUT[69][2]=21;LUT[69][3]=25;LUT[69][4]=1;LUT[69][5]=9;LUT[69][6]=25;LUT[69][7]=1;LUT[69][8]=25;LUT[69][9]=7;
LUT[70][0]=15;LUT[70][1]=1;LUT[70][2]=3;LUT[70][3]=21;LUT[70][4]=1;LUT[70][5]=21;LUT[70][6]=11;LUT[70][7]=11;LUT[70][8]=21;LUT[70][9]=25;LUT[70][10]=5;LUT[70][11]=25;LUT[70][12]=7;LUT[70][13]=5;LUT[70][14]=11;LUT[70][15]=25;
LUT[71][0]=12;LUT[71][1]=5;LUT[71][2]=9;LUT[71][3]=7;LUT[71][4]=5;LUT[71][5]=11;LUT[71][6]=9;LUT[71][7]=7;LUT[71][8]=9;LUT[71][9]=21;LUT[71][10]=7;LUT[71][11]=21;LUT[71][12]=25;
LUT[72][0]=12;LUT[72][1]=7;LUT[72][2]=15;LUT[72][3]=21;LUT[72][4]=5;LUT[72][5]=7;LUT[72][6]=21;LUT[72][7]=5;LUT[72][8]=25;LUT[72][9]=17;LUT[72][10]=5;LUT[72][11]=21;LUT[72][12]=25;
LUT[73][0]=15;LUT[73][1]=3;LUT[73][2]=7;LUT[73][3]=15;LUT[73][4]=5;LUT[73][5]=25;LUT[73][6]=17;LUT[73][7]=5;LUT[73][8]=21;LUT[73][9]=25;LUT[73][10]=1;LUT[73][11]=21;LUT[73][12]=5;LUT[73][13]=1;LUT[73][14]=9;LUT[73][15]=21;
LUT[74][0]=15;LUT[74][1]=11;LUT[74][2]=25;LUT[74][3]=17;LUT[74][4]=11;LUT[74][5]=21;LUT[74][6]=25;LUT[74][7]=1;LUT[74][8]=21;LUT[74][9]=11;LUT[74][10]=1;LUT[74][11]=7;LUT[74][12]=15;LUT[74][13]=1;LUT[74][14]=15;LUT[74][15]=21;
LUT[75][0]=12;LUT[75][1]=3;LUT[75][2]=7;LUT[75][3]=15;LUT[75][4]=9;LUT[75][5]=17;LUT[75][6]=11;LUT[75][7]=9;LUT[75][8]=21;LUT[75][9]=17;LUT[75][10]=17;LUT[75][11]=21;LUT[75][12]=25;
LUT[76][0]=9;LUT[76][1]=5;LUT[76][2]=25;LUT[76][3]=17;LUT[76][4]=5;LUT[76][5]=21;LUT[76][6]=25;LUT[76][7]=3;LUT[76][8]=21;LUT[76][9]=5;
LUT[77][0]=12;LUT[77][1]=5;LUT[77][2]=25;LUT[77][3]=17;LUT[77][4]=5;LUT[77][5]=21;LUT[77][6]=25;LUT[77][7]=1;LUT[77][8]=21;LUT[77][9]=5;LUT[77][10]=1;LUT[77][11]=9;LUT[77][12]=21;
LUT[78][0]=12;LUT[78][1]=1;LUT[78][2]=17;LUT[78][3]=11;LUT[78][4]=1;LUT[78][5]=3;LUT[78][6]=17;LUT[78][7]=3;LUT[78][8]=25;LUT[78][9]=17;LUT[78][10]=3;LUT[78][11]=21;LUT[78][12]=25;
LUT[79][0]=9;LUT[79][1]=9;LUT[79][2]=17;LUT[79][3]=11;LUT[79][4]=9;LUT[79][5]=21;LUT[79][6]=17;LUT[79][7]=17;LUT[79][8]=21;LUT[79][9]=25;
LUT[80][0]=6;LUT[80][1]=15;LUT[80][2]=19;LUT[80][3]=25;LUT[80][4]=9;LUT[80][5]=19;LUT[80][6]=15;
LUT[81][0]=9;LUT[81][1]=1;LUT[81][2]=15;LUT[81][3]=3;LUT[81][4]=1;LUT[81][5]=25;LUT[81][6]=15;LUT[81][7]=1;LUT[81][8]=19;LUT[81][9]=25;
LUT[82][0]=15;LUT[82][1]=11;LUT[82][2]=19;LUT[82][3]=25;LUT[82][4]=5;LUT[82][5]=11;LUT[82][6]=25;LUT[82][7]=5;LUT[82][8]=25;LUT[82][9]=15;LUT[82][10]=1;LUT[82][11]=15;LUT[82][12]=9;LUT[82][13]=1;LUT[82][14]=5;LUT[82][15]=15;
LUT[83][0]=12;LUT[83][1]=3;LUT[83][2]=5;LUT[83][3]=11;LUT[83][4]=3;LUT[83][5]=11;LUT[83][6]=19;LUT[83][7]=3;LUT[83][8]=19;LUT[83][9]=15;LUT[83][10]=15;LUT[83][11]=19;LUT[83][12]=25;
LUT[84][0]=9;LUT[84][1]=3;LUT[84][2]=9;LUT[84][3]=19;LUT[84][4]=3;LUT[84][5]=19;LUT[84][6]=7;LUT[84][7]=7;LUT[84][8]=19;LUT[84][9]=25;
LUT[85][0]=6;LUT[85][1]=1;LUT[85][2]=19;LUT[85][3]=7;LUT[85][4]=7;LUT[85][5]=19;LUT[85][6]=25;
LUT[86][0]=12;LUT[86][1]=1;LUT[86][2]=3;LUT[86][3]=9;LUT[86][4]=7;LUT[86][5]=19;LUT[86][6]=25;LUT[86][7]=5;LUT[86][8]=19;LUT[86][9]=7;LUT[86][10]=5;LUT[86][11]=11;LUT[86][12]=19;
LUT[87][0]=9;LUT[87][1]=7;LUT[87][2]=19;LUT[87][3]=25;LUT[87][4]=5;LUT[87][5]=19;LUT[87][6]=7;LUT[87][7]=5;LUT[87][8]=11;LUT[87][9]=19;
LUT[88][0]=15;LUT[88][1]=7;LUT[88][2]=15;LUT[88][3]=9;LUT[88][4]=5;LUT[88][5]=7;LUT[88][6]=9;LUT[88][7]=5;LUT[88][8]=9;LUT[88][9]=19;LUT[88][10]=17;LUT[88][11]=19;LUT[88][12]=25;LUT[88][13]=5;LUT[88][14]=19;LUT[88][15]=17;
LUT[89][0]=12;LUT[89][1]=3;LUT[89][2]=7;LUT[89][3]=15;LUT[89][4]=1;LUT[89][5]=19;LUT[89][6]=25;LUT[89][7]=1;LUT[89][8]=25;LUT[89][9]=17;LUT[89][10]=1;LUT[89][11]=17;LUT[89][12]=5;
LUT[90][0]=12;LUT[90][1]=1;LUT[90][2]=7;LUT[90][3]=9;LUT[90][4]=7;LUT[90][5]=15;LUT[90][6]=9;LUT[90][7]=11;LUT[90][8]=19;LUT[90][9]=17;LUT[90][10]=17;LUT[90][11]=19;LUT[90][12]=25;
LUT[91][0]=9;LUT[91][1]=3;LUT[91][2]=7;LUT[91][3]=15;LUT[91][4]=17;LUT[91][5]=19;LUT[91][6]=25;LUT[91][7]=11;LUT[91][8]=19;LUT[91][9]=17;
LUT[92][0]=12;LUT[92][1]=3;LUT[92][2]=25;LUT[92][3]=17;LUT[92][4]=3;LUT[92][5]=17;LUT[92][6]=5;LUT[92][7]=3;LUT[92][8]=9;LUT[92][9]=25;LUT[92][10]=9;LUT[92][11]=19;LUT[92][12]=25;
LUT[93][0]=9;LUT[93][1]=1;LUT[93][2]=19;LUT[93][3]=25;LUT[93][4]=1;LUT[93][5]=25;LUT[93][6]=17;LUT[93][7]=1;LUT[93][8]=17;LUT[93][9]=5;
LUT[94][0]=9;LUT[94][1]=1;LUT[94][2]=3;LUT[94][3]=9;LUT[94][4]=11;LUT[94][5]=19;LUT[94][6]=17;LUT[94][7]=17;LUT[94][8]=19;LUT[94][9]=25;
LUT[95][0]=6;LUT[95][1]=11;LUT[95][2]=19;LUT[95][3]=17;LUT[95][4]=17;LUT[95][5]=19;LUT[95][6]=25;
LUT[96][0]=12;LUT[96][1]=11;LUT[96][2]=21;LUT[96][3]=19;LUT[96][4]=11;LUT[96][5]=15;LUT[96][6]=21;LUT[96][7]=15;LUT[96][8]=23;LUT[96][9]=25;LUT[96][10]=11;LUT[96][11]=23;LUT[96][12]=15;
LUT[97][0]=15;LUT[97][1]=9;LUT[97][2]=21;LUT[97][3]=19;LUT[97][4]=1;LUT[97][5]=11;LUT[97][6]=23;LUT[97][7]=1;LUT[97][8]=23;LUT[97][9]=3;LUT[97][10]=3;LUT[97][11]=23;LUT[97][12]=25;LUT[97][13]=3;LUT[97][14]=25;LUT[97][15]=15;
LUT[98][0]=15;LUT[98][1]=1;LUT[98][2]=21;LUT[98][3]=19;LUT[98][4]=1;LUT[98][5]=15;LUT[98][6]=21;LUT[98][7]=1;LUT[98][8]=5;LUT[98][9]=15;LUT[98][10]=5;LUT[98][11]=23;LUT[98][12]=25;LUT[98][13]=5;LUT[98][14]=25;LUT[98][15]=15;
LUT[99][0]=12;LUT[99][1]=9;LUT[99][2]=21;LUT[99][3]=19;LUT[99][4]=3;LUT[99][5]=5;LUT[99][6]=23;LUT[99][7]=3;LUT[99][8]=23;LUT[99][9]=25;LUT[99][10]=3;LUT[99][11]=25;LUT[99][12]=15;
LUT[100][0]=15;LUT[100][1]=7;LUT[100][2]=23;LUT[100][3]=25;LUT[100][4]=7;LUT[100][5]=11;LUT[100][6]=23;LUT[100][7]=3;LUT[100][8]=11;LUT[100][9]=7;LUT[100][10]=3;LUT[100][11]=21;LUT[100][12]=19;LUT[100][13]=3;LUT[100][14]=19;LUT[100][15]=11;
LUT[101][0]=12;LUT[101][1]=9;LUT[101][2]=21;LUT[101][3]=19;LUT[101][4]=1;LUT[101][5]=25;LUT[101][6]=7;LUT[101][7]=1;LUT[101][8]=11;LUT[101][9]=25;LUT[101][10]=11;LUT[101][11]=23;LUT[101][12]=25;
LUT[102][0]=12;LUT[102][1]=1;LUT[102][2]=21;LUT[102][3]=19;LUT[102][4]=1;LUT[102][5]=3;LUT[102][6]=21;LUT[102][7]=5;LUT[102][8]=23;LUT[102][9]=25;LUT[102][10]=5;LUT[102][11]=25;LUT[102][12]=7;
LUT[103][0]=9;LUT[103][1]=9;LUT[103][2]=21;LUT[103][3]=19;LUT[103][4]=5;LUT[103][5]=23;LUT[103][6]=25;LUT[103][7]=5;LUT[103][8]=25;LUT[103][9]=7;
LUT[104][0]=15;LUT[104][1]=17;LUT[104][2]=23;LUT[104][3]=25;LUT[104][4]=7;LUT[104][5]=15;LUT[104][6]=21;LUT[104][7]=5;LUT[104][8]=7;LUT[104][9]=21;LUT[104][10]=5;LUT[104][11]=21;LUT[104][12]=19;LUT[104][13]=5;LUT[104][14]=19;LUT[104][15]=11;
LUT[105][0]=12;LUT[105][1]=1;LUT[105][2]=11;LUT[105][3]=5;LUT[105][4]=3;LUT[105][5]=7;LUT[105][6]=15;LUT[105][7]=9;LUT[105][8]=21;LUT[105][9]=19;LUT[105][10]=17;LUT[105][11]=23;LUT[105][12]=25;
LUT[106][0]=12;LUT[106][1]=17;LUT[106][2]=23;LUT[106][3]=25;LUT[106][4]=1;LUT[106][5]=7;LUT[106][6]=19;LUT[106][7]=7;LUT[106][8]=15;LUT[106][9]=19;LUT[106][10]=15;LUT[106][11]=21;LUT[106][12]=19;
LUT[107][0]=9;LUT[107][1]=3;LUT[107][2]=7;LUT[107][3]=15;LUT[107][4]=9;LUT[107][5]=21;LUT[107][6]=19;LUT[107][7]=17;LUT[107][8]=23;LUT[107][9]=25;
LUT[108][0]=12;LUT[108][1]=17;LUT[108][2]=23;LUT[108][3]=25;LUT[108][4]=3;LUT[108][5]=21;LUT[108][6]=5;LUT[108][7]=5;LUT[108][8]=21;LUT[108][9]=19;LUT[108][10]=5;LUT[108][11]=19;LUT[108][12]=11;
LUT[109][0]=9;LUT[109][1]=1;LUT[109][2]=11;LUT[109][3]=5;LUT[109][4]=17;LUT[109][5]=23;LUT[109][6]=25;LUT[109][7]=9;LUT[109][8]=21;LUT[109][9]=19;
LUT[110][0]=9;LUT[110][1]=17;LUT[110][2]=23;LUT[110][3]=25;LUT[110][4]=3;LUT[110][5]=21;LUT[110][6]=19;LUT[110][7]=1;LUT[110][8]=3;LUT[110][9]=19;
LUT[111][0]=6;LUT[111][1]=9;LUT[111][2]=21;LUT[111][3]=19;LUT[111][4]=17;LUT[111][5]=23;LUT[111][6]=25;
LUT[112][0]=9;LUT[112][1]=15;LUT[112][2]=23;LUT[112][3]=25;LUT[112][4]=11;LUT[112][5]=23;LUT[112][6]=15;LUT[112][7]=9;LUT[112][8]=11;LUT[112][9]=15;
LUT[113][0]=12;LUT[113][1]=1;LUT[113][2]=11;LUT[113][3]=23;LUT[113][4]=1;LUT[113][5]=23;LUT[113][6]=3;LUT[113][7]=3;LUT[113][8]=23;LUT[113][9]=25;LUT[113][10]=3;LUT[113][11]=25;LUT[113][12]=15;
LUT[114][0]=12;LUT[114][1]=1;LUT[114][2]=23;LUT[114][3]=9;LUT[114][4]=1;LUT[114][5]=5;LUT[114][6]=23;LUT[114][7]=9;LUT[114][8]=23;LUT[114][9]=25;LUT[114][10]=9;LUT[114][11]=25;LUT[114][12]=15;
LUT[115][0]=9;LUT[115][1]=3;LUT[115][2]=5;LUT[115][3]=23;LUT[115][4]=3;LUT[115][5]=23;LUT[115][6]=25;LUT[115][7]=3;LUT[115][8]=25;LUT[115][9]=15;
LUT[116][0]=12;LUT[116][1]=9;LUT[116][2]=11;LUT[116][3]=23;LUT[116][4]=9;LUT[116][5]=23;LUT[116][6]=25;LUT[116][7]=3;LUT[116][8]=9;LUT[116][9]=25;LUT[116][10]=3;LUT[116][11]=25;LUT[116][12]=7;
LUT[117][0]=9;LUT[117][1]=1;LUT[117][2]=25;LUT[117][3]=7;LUT[117][4]=1;LUT[117][5]=11;LUT[117][6]=25;LUT[117][7]=11;LUT[117][8]=23;LUT[117][9]=25;
LUT[118][0]=9;LUT[118][1]=1;LUT[118][2]=3;LUT[118][3]=9;LUT[118][4]=5;LUT[118][5]=25;LUT[118][6]=7;LUT[118][7]=5;LUT[118][8]=23;LUT[118][9]=25;
LUT[119][0]=6;LUT[119][1]=5;LUT[119][2]=23;LUT[119][3]=25;LUT[119][4]=5;LUT[119][5]=25;LUT[119][6]=7;
LUT[120][0]=12;LUT[120][1]=17;LUT[120][2]=23;LUT[120][3]=25;LUT[120][4]=9;LUT[120][5]=11;LUT[120][6]=15;LUT[120][7]=5;LUT[120][8]=15;LUT[120][9]=11;LUT[120][10]=5;LUT[120][11]=7;LUT[120][12]=15;
LUT[121][0]=9;LUT[121][1]=17;LUT[121][2]=23;LUT[121][3]=25;LUT[121][4]=1;LUT[121][5]=11;LUT[121][6]=5;LUT[121][7]=3;LUT[121][8]=7;LUT[121][9]=15;
LUT[122][0]=9;LUT[122][1]=17;LUT[122][2]=23;LUT[122][3]=25;LUT[122][4]=1;LUT[122][5]=7;LUT[122][6]=15;LUT[122][7]=1;LUT[122][8]=15;LUT[122][9]=9;
LUT[123][0]=6;LUT[123][1]=3;LUT[123][2]=7;LUT[123][3]=15;LUT[123][4]=17;LUT[123][5]=23;LUT[123][6]=25;
LUT[124][0]=9;LUT[124][1]=17;LUT[124][2]=23;LUT[124][3]=25;LUT[124][4]=5;LUT[124][5]=9;LUT[124][6]=11;LUT[124][7]=3;LUT[124][8]=9;LUT[124][9]=5;
LUT[125][0]=6;LUT[125][1]=1;LUT[125][2]=11;LUT[125][3]=5;LUT[125][4]=17;LUT[125][5]=23;LUT[125][6]=25;
LUT[126][0]=6;LUT[126][1]=1;LUT[126][2]=3;LUT[126][3]=9;LUT[126][4]=17;LUT[126][5]=23;LUT[126][6]=25;
LUT[127][0]=3;LUT[127][1]=17;LUT[127][2]=23;LUT[127][3]=25;
LUT[128][0]=3;LUT[128][1]=17;LUT[128][2]=25;LUT[128][3]=23;
LUT[129][0]=18;LUT[129][1]=3;LUT[129][2]=17;LUT[129][3]=25;LUT[129][4]=3;LUT[129][5]=25;LUT[129][6]=9;LUT[129][7]=9;LUT[129][8]=25;LUT[129][9]=23;LUT[129][10]=1;LUT[129][11]=9;LUT[129][12]=23;LUT[129][13]=1;LUT[129][14]=17;LUT[129][15]=3;LUT[129][16]=1;LUT[129][17]=23;LUT[129][18]=17;
LUT[130][0]=12;LUT[130][1]=1;LUT[130][2]=23;LUT[130][3]=11;LUT[130][4]=1;LUT[130][5]=25;LUT[130][6]=23;LUT[130][7]=5;LUT[130][8]=17;LUT[130][9]=25;LUT[130][10]=1;LUT[130][11]=5;LUT[130][12]=25;
LUT[131][0]=15;LUT[131][1]=9;LUT[131][2]=23;LUT[131][3]=11;LUT[131][4]=9;LUT[131][5]=25;LUT[131][6]=23;LUT[131][7]=3;LUT[131][8]=25;LUT[131][9]=9;LUT[131][10]=3;LUT[131][11]=5;LUT[131][12]=17;LUT[131][13]=3;LUT[131][14]=17;LUT[131][15]=25;
LUT[132][0]=12;LUT[132][1]=15;LUT[132][2]=25;LUT[132][3]=23;LUT[132][4]=3;LUT[132][5]=15;LUT[132][6]=23;LUT[132][7]=3;LUT[132][8]=17;LUT[132][9]=7;LUT[132][10]=3;LUT[132][11]=23;LUT[132][12]=17;
LUT[133][0]=15;LUT[133][1]=1;LUT[133][2]=17;LUT[133][3]=7;LUT[133][4]=1;LUT[133][5]=23;LUT[133][6]=17;LUT[133][7]=1;LUT[133][8]=9;LUT[133][9]=23;LUT[133][10]=9;LUT[133][11]=15;LUT[133][12]=25;LUT[133][13]=9;LUT[133][14]=25;LUT[133][15]=23;
LUT[134][0]=15;LUT[134][1]=5;LUT[134][2]=17;LUT[134][3]=7;LUT[134][4]=1;LUT[134][5]=3;LUT[134][6]=15;LUT[134][7]=1;LUT[134][8]=15;LUT[134][9]=11;LUT[134][10]=11;LUT[134][11]=15;LUT[134][12]=25;LUT[134][13]=11;LUT[134][14]=25;LUT[134][15]=23;
LUT[135][0]=12;LUT[135][1]=5;LUT[135][2]=17;LUT[135][3]=7;LUT[135][4]=9;LUT[135][5]=15;LUT[135][6]=11;LUT[135][7]=11;LUT[135][8]=15;LUT[135][9]=25;LUT[135][10]=11;LUT[135][11]=25;LUT[135][12]=23;
LUT[136][0]=6;LUT[136][1]=7;LUT[136][2]=25;LUT[136][3]=23;LUT[136][4]=5;LUT[136][5]=7;LUT[136][6]=23;
LUT[137][0]=15;LUT[137][1]=3;LUT[137][2]=7;LUT[137][3]=25;LUT[137][4]=3;LUT[137][5]=25;LUT[137][6]=9;LUT[137][7]=9;LUT[137][8]=25;LUT[137][9]=23;LUT[137][10]=1;LUT[137][11]=23;LUT[137][12]=5;LUT[137][13]=1;LUT[137][14]=9;LUT[137][15]=23;
LUT[138][0]=9;LUT[138][1]=1;LUT[138][2]=23;LUT[138][3]=11;LUT[138][4]=1;LUT[138][5]=25;LUT[138][6]=23;LUT[138][7]=1;LUT[138][8]=7;LUT[138][9]=25;
LUT[139][0]=12;LUT[139][1]=3;LUT[139][2]=11;LUT[139][3]=9;LUT[139][4]=3;LUT[139][5]=7;LUT[139][6]=11;LUT[139][7]=7;LUT[139][8]=23;LUT[139][9]=11;LUT[139][10]=7;LUT[139][11]=25;LUT[139][12]=23;
LUT[140][0]=9;LUT[140][1]=15;LUT[140][2]=25;LUT[140][3]=23;LUT[140][4]=3;LUT[140][5]=15;LUT[140][6]=23;LUT[140][7]=3;LUT[140][8]=23;LUT[140][9]=5;
LUT[141][0]=12;LUT[141][1]=5;LUT[141][2]=15;LUT[141][3]=25;LUT[141][4]=5;LUT[141][5]=25;LUT[141][6]=23;LUT[141][7]=1;LUT[141][8]=15;LUT[141][9]=5;LUT[141][10]=1;LUT[141][11]=9;LUT[141][12]=15;
LUT[142][0]=12;LUT[142][1]=1;LUT[142][2]=3;LUT[142][3]=15;LUT[142][4]=1;LUT[142][5]=15;LUT[142][6]=11;LUT[142][7]=11;LUT[142][8]=15;LUT[142][9]=25;LUT[142][10]=11;LUT[142][11]=25;LUT[142][12]=23;
LUT[143][0]=9;LUT[143][1]=9;LUT[143][2]=15;LUT[143][3]=11;LUT[143][4]=11;LUT[143][5]=15;LUT[143][6]=25;LUT[143][7]=11;LUT[143][8]=25;LUT[143][9]=23;
LUT[144][0]=12;LUT[144][1]=9;LUT[144][2]=25;LUT[144][3]=21;LUT[144][4]=9;LUT[144][5]=17;LUT[144][6]=25;LUT[144][7]=17;LUT[144][8]=19;LUT[144][9]=23;LUT[144][10]=9;LUT[144][11]=19;LUT[144][12]=17;
LUT[145][0]=15;LUT[145][1]=3;LUT[145][2]=25;LUT[145][3]=21;LUT[145][4]=3;LUT[145][5]=17;LUT[145][6]=25;LUT[145][7]=1;LUT[145][8]=17;LUT[145][9]=3;LUT[145][10]=1;LUT[145][11]=19;LUT[145][12]=23;LUT[145][13]=1;LUT[145][14]=23;LUT[145][15]=17;
LUT[146][0]=15;LUT[146][1]=11;LUT[146][2]=19;LUT[146][3]=23;LUT[146][4]=9;LUT[146][5]=25;LUT[146][6]=21;LUT[146][7]=9;LUT[146][8]=17;LUT[146][9]=25;LUT[146][10]=1;LUT[146][11]=17;LUT[146][12]=9;LUT[146][13]=1;LUT[146][14]=5;LUT[146][15]=17;
LUT[147][0]=12;LUT[147][1]=11;LUT[147][2]=19;LUT[147][3]=23;LUT[147][4]=3;LUT[147][5]=5;LUT[147][6]=21;LUT[147][7]=5;LUT[147][8]=17;LUT[147][9]=21;LUT[147][10]=17;LUT[147][11]=25;LUT[147][12]=21;
LUT[148][0]=15;LUT[148][1]=15;LUT[148][2]=25;LUT[148][3]=21;LUT[148][4]=17;LUT[148][5]=19;LUT[148][6]=23;LUT[148][7]=9;LUT[148][8]=19;LUT[148][9]=17;LUT[148][10]=7;LUT[148][11]=9;LUT[148][12]=17;LUT[148][13]=3;LUT[148][14]=9;LUT[148][15]=7;
LUT[149][0]=12;LUT[149][1]=15;LUT[149][2]=25;LUT[149][3]=21;LUT[149][4]=1;LUT[149][5]=19;LUT[149][6]=7;LUT[149][7]=7;LUT[149][8]=19;LUT[149][9]=23;LUT[149][10]=7;LUT[149][11]=23;LUT[149][12]=17;
LUT[150][0]=12;LUT[150][1]=11;LUT[150][2]=19;LUT[150][3]=23;LUT[150][4]=1;LUT[150][5]=3;LUT[150][6]=9;LUT[150][7]=15;LUT[150][8]=25;LUT[150][9]=21;LUT[150][10]=5;LUT[150][11]=17;LUT[150][12]=7;
LUT[151][0]=9;LUT[151][1]=11;LUT[151][2]=19;LUT[151][3]=23;LUT[151][4]=5;LUT[151][5]=17;LUT[151][6]=7;LUT[151][7]=15;LUT[151][8]=25;LUT[151][9]=21;
LUT[152][0]=15;LUT[152][1]=5;LUT[152][2]=19;LUT[152][3]=23;LUT[152][4]=5;LUT[152][5]=9;LUT[152][6]=19;LUT[152][7]=5;LUT[152][8]=7;LUT[152][9]=9;LUT[152][10]=7;LUT[152][11]=25;LUT[152][12]=21;LUT[152][13]=7;LUT[152][14]=21;LUT[152][15]=9;
LUT[153][0]=12;LUT[153][1]=3;LUT[153][2]=7;LUT[153][3]=25;LUT[153][4]=3;LUT[153][5]=25;LUT[153][6]=21;LUT[153][7]=1;LUT[153][8]=23;LUT[153][9]=5;LUT[153][10]=1;LUT[153][11]=19;LUT[153][12]=23;
LUT[154][0]=12;LUT[154][1]=11;LUT[154][2]=19;LUT[154][3]=23;LUT[154][4]=1;LUT[154][5]=7;LUT[154][6]=25;LUT[154][7]=1;LUT[154][8]=25;LUT[154][9]=21;LUT[154][10]=1;LUT[154][11]=21;LUT[154][12]=9;
LUT[155][0]=9;LUT[155][1]=11;LUT[155][2]=19;LUT[155][3]=23;LUT[155][4]=7;LUT[155][5]=25;LUT[155][6]=21;LUT[155][7]=3;LUT[155][8]=7;LUT[155][9]=21;
LUT[156][0]=12;LUT[156][1]=15;LUT[156][2]=25;LUT[156][3]=21;LUT[156][4]=3;LUT[156][5]=23;LUT[156][6]=5;LUT[156][7]=3;LUT[156][8]=9;LUT[156][9]=23;LUT[156][10]=9;LUT[156][11]=19;LUT[156][12]=23;
LUT[157][0]=9;LUT[157][1]=15;LUT[157][2]=25;LUT[157][3]=21;LUT[157][4]=1;LUT[157][5]=19;LUT[157][6]=23;LUT[157][7]=1;LUT[157][8]=23;LUT[157][9]=5;
LUT[158][0]=9;LUT[158][1]=15;LUT[158][2]=25;LUT[158][3]=21;LUT[158][4]=1;LUT[158][5]=3;LUT[158][6]=9;LUT[158][7]=11;LUT[158][8]=19;LUT[158][9]=23;
LUT[159][0]=6;LUT[159][1]=15;LUT[159][2]=25;LUT[159][3]=21;LUT[159][4]=11;LUT[159][5]=19;LUT[159][6]=23;
LUT[160][0]=6;LUT[160][1]=11;LUT[160][2]=25;LUT[160][3]=19;LUT[160][4]=11;LUT[160][5]=17;LUT[160][6]=25;
LUT[161][0]=15;LUT[161][1]=1;LUT[161][2]=11;LUT[161][3]=17;LUT[161][4]=1;LUT[161][5]=17;LUT[161][6]=3;LUT[161][7]=3;LUT[161][8]=17;LUT[161][9]=25;LUT[161][10]=9;LUT[161][11]=25;LUT[161][12]=19;LUT[161][13]=3;LUT[161][14]=25;LUT[161][15]=9;
LUT[162][0]=9;LUT[162][1]=5;LUT[162][2]=17;LUT[162][3]=25;LUT[162][4]=1;LUT[162][5]=5;LUT[162][6]=25;LUT[162][7]=1;LUT[162][8]=25;LUT[162][9]=19;
LUT[163][0]=12;LUT[163][1]=5;LUT[163][2]=19;LUT[163][3]=9;LUT[163][4]=3;LUT[163][5]=5;LUT[163][6]=9;LUT[163][7]=5;LUT[163][8]=17;LUT[163][9]=19;LUT[163][10]=17;LUT[163][11]=25;LUT[163][12]=19;
LUT[164][0]=15;LUT[164][1]=15;LUT[164][2]=25;LUT[164][3]=19;LUT[164][4]=3;LUT[164][5]=15;LUT[164][6]=19;LUT[164][7]=3;LUT[164][8]=19;LUT[164][9]=11;LUT[164][10]=7;LUT[164][11]=11;LUT[164][12]=17;LUT[164][13]=3;LUT[164][14]=11;LUT[164][15]=7;
LUT[165][0]=12;LUT[165][1]=15;LUT[165][2]=25;LUT[165][3]=19;LUT[165][4]=9;LUT[165][5]=15;LUT[165][6]=19;LUT[165][7]=7;LUT[165][8]=11;LUT[165][9]=17;LUT[165][10]=1;LUT[165][11]=11;LUT[165][12]=7;
LUT[166][0]=12;LUT[166][1]=5;LUT[166][2]=17;LUT[166][3]=7;LUT[166][4]=1;LUT[166][5]=25;LUT[166][6]=19;LUT[166][7]=1;LUT[166][8]=3;LUT[166][9]=25;LUT[166][10]=3;LUT[166][11]=15;LUT[166][12]=25;
LUT[167][0]=9;LUT[167][1]=5;LUT[167][2]=17;LUT[167][3]=7;LUT[167][4]=9;LUT[167][5]=15;LUT[167][6]=25;LUT[167][7]=9;LUT[167][8]=25;LUT[167][9]=19;
LUT[168][0]=9;LUT[168][1]=5;LUT[168][2]=7;LUT[168][3]=11;LUT[168][4]=7;LUT[168][5]=19;LUT[168][6]=11;LUT[168][7]=7;LUT[168][8]=25;LUT[168][9]=19;
LUT[169][0]=12;LUT[169][1]=1;LUT[169][2]=11;LUT[169][3]=5;LUT[169][4]=7;LUT[169][5]=25;LUT[169][6]=19;LUT[169][7]=7;LUT[169][8]=19;LUT[169][9]=9;LUT[169][10]=3;LUT[169][11]=7;LUT[169][12]=9;
LUT[170][0]=6;LUT[170][1]=1;LUT[170][2]=25;LUT[170][3]=19;LUT[170][4]=1;LUT[170][5]=7;LUT[170][6]=25;
LUT[171][0]=9;LUT[171][1]=7;LUT[171][2]=25;LUT[171][3]=19;LUT[171][4]=7;LUT[171][5]=19;LUT[171][6]=9;LUT[171][7]=3;LUT[171][8]=7;LUT[171][9]=9;
LUT[172][0]=12;LUT[172][1]=3;LUT[172][2]=15;LUT[172][3]=5;LUT[172][4]=5;LUT[172][5]=15;LUT[172][6]=25;LUT[172][7]=5;LUT[172][8]=25;LUT[172][9]=11;LUT[172][10]=11;LUT[172][11]=25;LUT[172][12]=19;
LUT[173][0]=9;LUT[173][1]=1;LUT[173][2]=11;LUT[173][3]=5;LUT[173][4]=9;LUT[173][5]=25;LUT[173][6]=19;LUT[173][7]=9;LUT[173][8]=15;LUT[173][9]=25;
LUT[174][0]=9;LUT[174][1]=1;LUT[174][2]=25;LUT[174][3]=19;LUT[174][4]=1;LUT[174][5]=3;LUT[174][6]=25;LUT[174][7]=3;LUT[174][8]=15;LUT[174][9]=25;
LUT[175][0]=6;LUT[175][1]=9;LUT[175][2]=15;LUT[175][3]=25;LUT[175][4]=9;LUT[175][5]=25;LUT[175][6]=19;
LUT[176][0]=9;LUT[176][1]=9;LUT[176][2]=25;LUT[176][3]=21;LUT[176][4]=9;LUT[176][5]=17;LUT[176][6]=25;LUT[176][7]=9;LUT[176][8]=11;LUT[176][9]=17;
LUT[177][0]=12;LUT[177][1]=11;LUT[177][2]=17;LUT[177][3]=25;LUT[177][4]=11;LUT[177][5]=25;LUT[177][6]=21;LUT[177][7]=1;LUT[177][8]=11;LUT[177][9]=21;LUT[177][10]=1;LUT[177][11]=21;LUT[177][12]=3;
LUT[178][0]=12;LUT[178][1]=9;LUT[178][2]=25;LUT[178][3]=21;LUT[178][4]=9;LUT[178][5]=17;LUT[178][6]=25;LUT[178][7]=1;LUT[178][8]=17;LUT[178][9]=9;LUT[178][10]=1;LUT[178][11]=5;LUT[178][12]=17;
LUT[179][0]=9;LUT[179][1]=3;LUT[179][2]=5;LUT[179][3]=21;LUT[179][4]=5;LUT[179][5]=17;LUT[179][6]=21;LUT[179][7]=17;LUT[179][8]=25;LUT[179][9]=21;
LUT[180][0]=12;LUT[180][1]=15;LUT[180][2]=25;LUT[180][3]=21;LUT[180][4]=9;LUT[180][5]=11;LUT[180][6]=17;LUT[180][7]=7;LUT[180][8]=9;LUT[180][9]=17;LUT[180][10]=3;LUT[180][11]=9;LUT[180][12]=7;
LUT[181][0]=9;LUT[181][1]=15;LUT[181][2]=25;LUT[181][3]=21;LUT[181][4]=7;LUT[181][5]=11;LUT[181][6]=17;LUT[181][7]=1;LUT[181][8]=11;LUT[181][9]=7;
LUT[182][0]=9;LUT[182][1]=1;LUT[182][2]=3;LUT[182][3]=9;LUT[182][4]=15;LUT[182][5]=25;LUT[182][6]=21;LUT[182][7]=5;LUT[182][8]=17;LUT[182][9]=7;
LUT[183][0]=6;LUT[183][1]=15;LUT[183][2]=25;LUT[183][3]=21;LUT[183][4]=5;LUT[183][5]=17;LUT[183][6]=7;
LUT[184][0]=12;LUT[184][1]=11;LUT[184][2]=25;LUT[184][3]=21;LUT[184][4]=9;LUT[184][5]=11;LUT[184][6]=21;LUT[184][7]=5;LUT[184][8]=25;LUT[184][9]=11;LUT[184][10]=5;LUT[184][11]=7;LUT[184][12]=25;
LUT[185][0]=9;LUT[185][1]=1;LUT[185][2]=11;LUT[185][3]=5;LUT[185][4]=3;LUT[185][5]=7;LUT[185][6]=21;LUT[185][7]=7;LUT[185][8]=25;LUT[185][9]=21;
LUT[186][0]=9;LUT[186][1]=1;LUT[186][2]=7;LUT[186][3]=25;LUT[186][4]=1;LUT[186][5]=25;LUT[186][6]=21;LUT[186][7]=1;LUT[186][8]=21;LUT[186][9]=9;
LUT[187][0]=6;LUT[187][1]=3;LUT[187][2]=7;LUT[187][3]=21;LUT[187][4]=7;LUT[187][5]=25;LUT[187][6]=21;
LUT[188][0]=9;LUT[188][1]=15;LUT[188][2]=25;LUT[188][3]=21;LUT[188][4]=3;LUT[188][5]=9;LUT[188][6]=5;LUT[188][7]=5;LUT[188][8]=9;LUT[188][9]=11;
LUT[189][0]=6;LUT[189][1]=15;LUT[189][2]=25;LUT[189][3]=21;LUT[189][4]=1;LUT[189][5]=11;LUT[189][6]=5;
LUT[190][0]=6;LUT[190][1]=1;LUT[190][2]=3;LUT[190][3]=9;LUT[190][4]=15;LUT[190][5]=25;LUT[190][6]=21;
LUT[191][0]=3;LUT[191][1]=15;LUT[191][2]=25;LUT[191][3]=21;
LUT[192][0]=6;LUT[192][1]=17;LUT[192][2]=21;LUT[192][3]=23;LUT[192][4]=15;LUT[192][5]=21;LUT[192][6]=17;
LUT[193][0]=15;LUT[193][1]=9;LUT[193][2]=21;LUT[193][3]=23;LUT[193][4]=1;LUT[193][5]=9;LUT[193][6]=23;LUT[193][7]=1;LUT[193][8]=23;LUT[193][9]=17;LUT[193][10]=3;LUT[193][11]=17;LUT[193][12]=15;LUT[193][13]=1;LUT[193][14]=17;LUT[193][15]=3;
LUT[194][0]=15;LUT[194][1]=5;LUT[194][2]=17;LUT[194][3]=15;LUT[194][4]=1;LUT[194][5]=5;LUT[194][6]=15;LUT[194][7]=1;LUT[194][8]=15;LUT[194][9]=21;LUT[194][10]=11;LUT[194][11]=21;LUT[194][12]=23;LUT[194][13]=1;LUT[194][14]=21;LUT[194][15]=11;
LUT[195][0]=12;LUT[195][1]=9;LUT[195][2]=23;LUT[195][3]=11;LUT[195][4]=9;LUT[195][5]=21;LUT[195][6]=23;LUT[195][7]=3;LUT[195][8]=5;LUT[195][9]=17;LUT[195][10]=3;LUT[195][11]=17;LUT[195][12]=15;
LUT[196][0]=9;LUT[196][1]=3;LUT[196][2]=17;LUT[196][3]=7;LUT[196][4]=3;LUT[196][5]=23;LUT[196][6]=17;LUT[196][7]=3;LUT[196][8]=21;LUT[196][9]=23;
LUT[197][0]=12;LUT[197][1]=17;LUT[197][2]=21;LUT[197][3]=23;LUT[197][4]=7;LUT[197][5]=21;LUT[197][6]=17;LUT[197][7]=7;LUT[197][8]=9;LUT[197][9]=21;LUT[197][10]=1;LUT[197][11]=9;LUT[197][12]=7;
LUT[198][0]=12;LUT[198][1]=5;LUT[198][2]=17;LUT[198][3]=7;LUT[198][4]=3;LUT[198][5]=21;LUT[198][6]=23;LUT[198][7]=3;LUT[198][8]=23;LUT[198][9]=11;LUT[198][10]=1;LUT[198][11]=3;LUT[198][12]=11;
LUT[199][0]=9;LUT[199][1]=5;LUT[199][2]=17;LUT[199][3]=7;LUT[199][4]=11;LUT[199][5]=21;LUT[199][6]=23;LUT[199][7]=9;LUT[199][8]=21;LUT[199][9]=11;
LUT[200][0]=9;LUT[200][1]=7;LUT[200][2]=15;LUT[200][3]=21;LUT[200][4]=5;LUT[200][5]=7;LUT[200][6]=21;LUT[200][7]=5;LUT[200][8]=21;LUT[200][9]=23;
LUT[201][0]=12;LUT[201][1]=3;LUT[201][2]=7;LUT[201][3]=15;LUT[201][4]=5;LUT[201][5]=21;LUT[201][6]=23;LUT[201][7]=1;LUT[201][8]=21;LUT[201][9]=5;LUT[201][10]=1;LUT[201][11]=9;LUT[201][12]=21;
LUT[202][0]=12;LUT[202][1]=7;LUT[202][2]=15;LUT[202][3]=23;LUT[202][4]=15;LUT[202][5]=21;LUT[202][6]=23;LUT[202][7]=7;LUT[202][8]=23;LUT[202][9]=11;LUT[202][10]=1;LUT[202][11]=7;LUT[202][12]=11;
LUT[203][0]=9;LUT[203][1]=3;LUT[203][2]=7;LUT[203][3]=15;LUT[203][4]=9;LUT[203][5]=21;LUT[203][6]=11;LUT[203][7]=11;LUT[203][8]=21;LUT[203][9]=23;
LUT[204][0]=6;LUT[204][1]=3;LUT[204][2]=21;LUT[204][3]=5;LUT[204][4]=5;LUT[204][5]=21;LUT[204][6]=23;
LUT[205][0]=9;LUT[205][1]=5;LUT[205][2]=21;LUT[205][3]=23;LUT[205][4]=1;LUT[205][5]=21;LUT[205][6]=5;LUT[205][7]=1;LUT[205][8]=9;LUT[205][9]=21;
LUT[206][0]=9;LUT[206][1]=3;LUT[206][2]=21;LUT[206][3]=23;LUT[206][4]=3;LUT[206][5]=23;LUT[206][6]=11;LUT[206][7]=1;LUT[206][8]=3;LUT[206][9]=11;
LUT[207][0]=6;LUT[207][1]=11;LUT[207][2]=21;LUT[207][3]=23;LUT[207][4]=9;LUT[207][5]=21;LUT[207][6]=11;
LUT[208][0]=9;LUT[208][1]=17;LUT[208][2]=19;LUT[208][3]=23;LUT[208][4]=9;LUT[208][5]=19;LUT[208][6]=17;LUT[208][7]=9;LUT[208][8]=17;LUT[208][9]=15;
LUT[209][0]=12;LUT[209][1]=15;LUT[209][2]=19;LUT[209][3]=23;LUT[209][4]=15;LUT[209][5]=23;LUT[209][6]=17;LUT[209][7]=3;LUT[209][8]=19;LUT[209][9]=15;LUT[209][10]=1;LUT[209][11]=19;LUT[209][12]=3;
LUT[210][0]=12;LUT[210][1]=11;LUT[210][2]=19;LUT[210][3]=23;LUT[210][4]=9;LUT[210][5]=17;LUT[210][6]=15;LUT[210][7]=1;LUT[210][8]=17;LUT[210][9]=9;LUT[210][10]=1;LUT[210][11]=5;LUT[210][12]=17;
LUT[211][0]=9;LUT[211][1]=11;LUT[211][2]=19;LUT[211][3]=23;LUT[211][4]=3;LUT[211][5]=5;LUT[211][6]=17;LUT[211][7]=3;LUT[211][8]=17;LUT[211][9]=15;
LUT[212][0]=12;LUT[212][1]=17;LUT[212][2]=19;LUT[212][3]=23;LUT[212][4]=9;LUT[212][5]=19;LUT[212][6]=17;LUT[212][7]=7;LUT[212][8]=9;LUT[212][9]=17;LUT[212][10]=3;LUT[212][11]=9;LUT[212][12]=7;
LUT[213][0]=9;LUT[213][1]=1;LUT[213][2]=19;LUT[213][3]=7;LUT[213][4]=7;LUT[213][5]=19;LUT[213][6]=23;LUT[213][7]=7;LUT[213][8]=23;LUT[213][9]=17;
LUT[214][0]=9;LUT[214][1]=5;LUT[214][2]=17;LUT[214][3]=7;LUT[214][4]=11;LUT[214][5]=19;LUT[214][6]=23;LUT[214][7]=1;LUT[214][8]=3;LUT[214][9]=9;
LUT[215][0]=6;LUT[215][1]=11;LUT[215][2]=19;LUT[215][3]=23;LUT[215][4]=5;LUT[215][5]=17;LUT[215][6]=7;
LUT[216][0]=12;LUT[216][1]=5;LUT[216][2]=7;LUT[216][3]=23;LUT[216][4]=7;LUT[216][5]=15;LUT[216][6]=23;LUT[216][7]=15;LUT[216][8]=19;LUT[216][9]=23;LUT[216][10]=9;LUT[216][11]=19;LUT[216][12]=15;
LUT[217][0]=9;LUT[217][1]=3;LUT[217][2]=7;LUT[217][3]=15;LUT[217][4]=1;LUT[217][5]=23;LUT[217][6]=5;LUT[217][7]=1;LUT[217][8]=19;LUT[217][9]=23;
LUT[218][0]=9;LUT[218][1]=11;LUT[218][2]=19;LUT[218][3]=23;LUT[218][4]=1;LUT[218][5]=15;LUT[218][6]=9;LUT[218][7]=1;LUT[218][8]=7;LUT[218][9]=15;
LUT[219][0]=6;LUT[219][1]=3;LUT[219][2]=7;LUT[219][3]=15;LUT[219][4]=11;LUT[219][5]=19;LUT[219][6]=23;
LUT[220][0]=9;LUT[220][1]=3;LUT[220][2]=23;LUT[220][3]=5;LUT[220][4]=3;LUT[220][5]=9;LUT[220][6]=23;LUT[220][7]=9;LUT[220][8]=19;LUT[220][9]=23;
LUT[221][0]=6;LUT[221][1]=1;LUT[221][2]=23;LUT[221][3]=5;LUT[221][4]=1;LUT[221][5]=19;LUT[221][6]=23;
LUT[222][0]=6;LUT[222][1]=1;LUT[222][2]=3;LUT[222][3]=9;LUT[222][4]=11;LUT[222][5]=19;LUT[222][6]=23;
LUT[223][0]=3;LUT[223][1]=11;LUT[223][2]=19;LUT[223][3]=23;
LUT[224][0]=9;LUT[224][1]=11;LUT[224][2]=21;LUT[224][3]=19;LUT[224][4]=11;LUT[224][5]=15;LUT[224][6]=21;LUT[224][7]=11;LUT[224][8]=17;LUT[224][9]=15;
LUT[225][0]=12;LUT[225][1]=9;LUT[225][2]=21;LUT[225][3]=19;LUT[225][4]=11;LUT[225][5]=17;LUT[225][6]=15;LUT[225][7]=3;LUT[225][8]=11;LUT[225][9]=15;LUT[225][10]=1;LUT[225][11]=11;LUT[225][12]=3;
LUT[226][0]=12;LUT[226][1]=15;LUT[226][2]=21;LUT[226][3]=17;LUT[226][4]=17;LUT[226][5]=21;LUT[226][6]=19;LUT[226][7]=5;LUT[226][8]=17;LUT[226][9]=19;LUT[226][10]=1;LUT[226][11]=5;LUT[226][12]=19;
LUT[227][0]=9;LUT[227][1]=9;LUT[227][2]=21;LUT[227][3]=19;LUT[227][4]=3;LUT[227][5]=17;LUT[227][6]=15;LUT[227][7]=3;LUT[227][8]=5;LUT[227][9]=17;
LUT[228][0]=12;LUT[228][1]=17;LUT[228][2]=21;LUT[228][3]=19;LUT[228][4]=11;LUT[228][5]=17;LUT[228][6]=19;LUT[228][7]=7;LUT[228][8]=21;LUT[228][9]=17;LUT[228][10]=3;LUT[228][11]=21;LUT[228][12]=7;
LUT[229][0]=9;LUT[229][1]=9;LUT[229][2]=21;LUT[229][3]=19;LUT[229][4]=1;LUT[229][5]=11;LUT[229][6]=7;LUT[229][7]=7;LUT[229][8]=11;LUT[229][9]=17;
LUT[230][0]=9;LUT[230][1]=5;LUT[230][2]=17;LUT[230][3]=7;LUT[230][4]=1;LUT[230][5]=3;LUT[230][6]=19;LUT[230][7]=3;LUT[230][8]=21;LUT[230][9]=19;
LUT[231][0]=6;LUT[231][1]=9;LUT[231][2]=21;LUT[231][3]=19;LUT[231][4]=5;LUT[231][5]=17;LUT[231][6]=7;
LUT[232][0]=12;LUT[232][1]=7;LUT[232][2]=15;LUT[232][3]=21;LUT[232][4]=5;LUT[232][5]=7;LUT[232][6]=21;LUT[232][7]=5;LUT[232][8]=21;LUT[232][9]=19;LUT[232][10]=5;LUT[232][11]=19;LUT[232][12]=11;
LUT[233][0]=9;LUT[233][1]=9;LUT[233][2]=21;LUT[233][3]=19;LUT[233][4]=3;LUT[233][5]=7;LUT[233][6]=15;LUT[233][7]=1;LUT[233][8]=11;LUT[233][9]=5;
LUT[234][0]=9;LUT[234][1]=1;LUT[234][2]=7;LUT[234][3]=19;LUT[234][4]=7;LUT[234][5]=15;LUT[234][6]=19;LUT[234][7]=15;LUT[234][8]=21;LUT[234][9]=19;
LUT[235][0]=6;LUT[235][1]=9;LUT[235][2]=21;LUT[235][3]=19;LUT[235][4]=3;LUT[235][5]=7;LUT[235][6]=15;
LUT[236][0]=9;LUT[236][1]=3;LUT[236][2]=21;LUT[236][3]=5;LUT[236][4]=5;LUT[236][5]=21;LUT[236][6]=19;LUT[236][7]=5;LUT[236][8]=19;LUT[236][9]=11;
LUT[237][0]=6;LUT[237][1]=9;LUT[237][2]=21;LUT[237][3]=19;LUT[237][4]=1;LUT[237][5]=11;LUT[237][6]=5;
LUT[238][0]=6;LUT[238][1]=3;LUT[238][2]=21;LUT[238][3]=19;LUT[238][4]=1;LUT[238][5]=3;LUT[238][6]=19;
LUT[239][0]=3;LUT[239][1]=9;LUT[239][2]=21;LUT[239][3]=19;
LUT[240][0]=6;LUT[240][1]=9;LUT[240][2]=17;LUT[240][3]=15;LUT[240][4]=9;LUT[240][5]=11;LUT[240][6]=17;
LUT[241][0]=9;LUT[241][1]=11;LUT[241][2]=17;LUT[241][3]=15;LUT[241][4]=3;LUT[241][5]=11;LUT[241][6]=15;LUT[241][7]=1;LUT[241][8]=11;LUT[241][9]=3;
LUT[242][0]=9;LUT[242][1]=9;LUT[242][2]=17;LUT[242][3]=15;LUT[242][4]=1;LUT[242][5]=17;LUT[242][6]=9;LUT[242][7]=1;LUT[242][8]=5;LUT[242][9]=17;
LUT[243][0]=6;LUT[243][1]=3;LUT[243][2]=5;LUT[243][3]=17;LUT[243][4]=3;LUT[243][5]=17;LUT[243][6]=15;
LUT[244][0]=9;LUT[244][1]=9;LUT[244][2]=11;LUT[244][3]=17;LUT[244][4]=7;LUT[244][5]=9;LUT[244][6]=17;LUT[244][7]=3;LUT[244][8]=9;LUT[244][9]=7;
LUT[245][0]=6;LUT[245][1]=7;LUT[245][2]=11;LUT[245][3]=17;LUT[245][4]=1;LUT[245][5]=11;LUT[245][6]=7;
LUT[246][0]=6;LUT[246][1]=1;LUT[246][2]=3;LUT[246][3]=9;LUT[246][4]=5;LUT[246][5]=17;LUT[246][6]=7;
LUT[247][0]=3;LUT[247][1]=5;LUT[247][2]=17;LUT[247][3]=7;
LUT[248][0]=9;LUT[248][1]=9;LUT[248][2]=11;LUT[248][3]=15;LUT[248][4]=5;LUT[248][5]=15;LUT[248][6]=11;LUT[248][7]=5;LUT[248][8]=7;LUT[248][9]=15;
LUT[249][0]=6;LUT[249][1]=3;LUT[249][2]=7;LUT[249][3]=15;LUT[249][4]=1;LUT[249][5]=11;LUT[249][6]=5;
LUT[250][0]=6;LUT[250][1]=1;LUT[250][2]=15;LUT[250][3]=9;LUT[250][4]=1;LUT[250][5]=7;LUT[250][6]=15;
LUT[251][0]=3;LUT[251][1]=3;LUT[251][2]=7;LUT[251][3]=15;
LUT[252][0]=6;LUT[252][1]=5;LUT[252][2]=9;LUT[252][3]=11;LUT[252][4]=3;LUT[252][5]=9;LUT[252][6]=5;
LUT[253][0]=3;LUT[253][1]=1;LUT[253][2]=11;LUT[253][3]=5;
LUT[254][0]=3;LUT[254][1]=1;LUT[254][2]=3;LUT[254][3]=9;
LUT[255][0]=0;
} /* InitLUT*/



























// LuM end of file
