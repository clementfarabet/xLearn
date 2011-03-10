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
   Algorithmes 3D "fully parallel" de squelettisation

   Algo. de Palagyi
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>
#include <lskelpar3d_others.h>




#define VERBOSE

/* global variables */
  long int           long_mask[26];
  unsigned char      char_mask[8];
  unsigned long int  neighbours;
  unsigned long int  neighbours_UD;
  unsigned long int  neighbours_NS;
  unsigned long int  neighbours_EW;
  unsigned long int  direction;
  unsigned char     *lut_deletable;
  unsigned long int  size_x;
  unsigned long int  size_y;
  unsigned long int  size_z;
  unsigned long int  size_xy;
  unsigned long int  size_xyz;
  unsigned char     *image;
  List               SurfaceVoxels;
  List               BorderVoxels;
  unsigned long int  act_addr;
  struct xvimage *pgmimage;
  Bordercell *borderlist;
  int                DEL6NEI;
  unsigned long int  endflag;
  unsigned char      *lut_simple;
  unsigned long int  z_size_xy, zm_size_xy, zp_size_xy;
  unsigned long int  y_size_x, ym_size_x, yp_size_x;





//#define DEBUG
//#ifdef DEBUG
//int32_t trace = 1;
//#endif
//
//typedef uint32_t mat[3][3];
//typedef uint32_t vec[3];
//
//// matrices pour les 8 symétries de type 1
//// et translations associées
//
//mat R0 = {
//  {  1,  0,  0},
//  {  0,  1,  0},
//  {  0,  0,  1}
//};
//vec T0 = {0,0,0};
//
//mat R1 = {
//  { -1,  0,  0},
//  {  0,  1,  0},
//  {  0,  0,  1}
//};
//vec T1 = {-4,0,0};
//
//mat R2 = {
//  {  1,  0,  0},
//  {  0, -1,  0},
//  {  0,  0,  1}
//};
//vec T2 = {0,-4,0};
//
//mat R3 = {
//  { -1,  0,  0},
//  {  0, -1,  0},
//  {  0,  0,  1}
//};
//vec T3 = {-4,-4,0};
//
//mat R4 = {
//  {  1,  0,  0},
//  {  0,  1,  0},
//  {  0,  0, -1}
//};
//vec T4 = {0,0,-4};
//
//mat R5 = {
//  { -1,  0,  0},
//  {  0,  1,  0},
//  {  0,  0, -1}
//};
//vec T5 = {-4,0,-4};
//
//mat R6 = {
//  {  1,  0,  0},
//  {  0, -1,  0},
//  {  0,  0, -1}
//};
//vec T6 = {0,-4,-4};
//
//mat R7 = {
//  { -1,  0,  0},
//  {  0, -1,  0},
//  {  0,  0, -1}
//};
//vec T7 = {-4,-4,-4};
//
//// matrices pour les 6 symétries de type 2
//
//mat S0 = {
//  {  1,  0,  0},
//  {  0,  1,  0},
//  {  0,  0,  1}
//};
//
//mat S1 = {
//  {  1,  0,  0},
//  {  0,  0,  1},
//  {  0,  1,  0}
//};
//
//mat S2 = {
//  {  0,  1,  0},
//  {  1,  0,  0},
//  {  0,  0,  1}
//};
//
//mat S3 = {
//  {  0,  1,  0},
//  {  0,  0,  1},
//  {  1,  0,  0}
//};
//
//mat S4 = {
//  {  0,  0,  1},
//  {  1,  0,  0},
//  {  0,  1,  0}
//};
//
//mat S5 = {
//  {  0,  0,  1},
//  {  0,  1,  0},
//  {  1,  0,  0}
//};

///* ==================================== */
//static void multmatvec(
//  mat m,
//  vec v,
//  vec sol)
///* ==================================== */
//{
//  int32_t i, j, t;
//  for (i = 0; i < 3; i++)
//  {
//    t = 0;
//    for (j = 0; j < 3; j++)
//      t += m[i][j] * v[j];
//    sol[i] = t;
//  }
//} // multmatvec()
//
///* ==================================== */
//static int32_t isometry_coord(mat s1, vec t1, mat s2, vec x, vec tmp)
///* ==================================== */
//{
//  multmatvec(s1,x,tmp);
//  tmp[0] -= t1[0];
//  tmp[1] -= t1[1];
//  tmp[2] -= t1[2];
//  multmatvec(s2,tmp,x);
//  return x[2]*25 + x[1]*5 + x[0];
//} // isometry_coord()
//
///* ==================================== */
//static void isometry(uint8_t *dest, uint8_t *sour, mat s1, vec t1, mat s2)
///* ==================================== */
//{
//  vec x, tmp = {0,0,0};
//  int32_t i, j, k, p;
//  for (k = 0; k < 5; k++)
//    for (j = 0; j < 5; j++)
//      for (i = 0; i < 5; i++)
//      {
//	x[0] = i; x[1] = j; x[2] = k;
//	p = isometry_coord(s1, t1, s2, x, tmp);
//	dest[p] = sour[k*25 + j*5 + i];
//      }
//} // isometry()
//
///* ==================================== */
//static void extract_vois(
//  uint8_t *img,                    /* pointeur base image */
//  int32_t p,                       /* index du point */
//  int32_t rs,                      /* taille rangee */
//  int32_t ps,                      /* taille plan */
//  int32_t N,                       /* taille image */
//  uint8_t *vois)
///*
//  retourne dans "vois" les valeurs des 125 voisins de p, dans l'ordre suivant:
//
//  0   1   2   3   4
//  5   6   7   8   9
// 10  11  12  13  14
// 15  16  17  18  19
// 20  21  22  23  24
//
// 25  26  27  28  29
// 30  31  32  33  34
// 35  36  37  38  39
// 40  41  42  43  44
// 45  46  47  48  49
//
// 50  51  52  53  54
// 55  56  57  58  59
// 60  61  62  63  64
// 65  66  67  68  69
// 70  71  72  73  74
//
// 75  76  77  78  79
// 80  81  82  83  84
// 85  86  87  88  89
// 90  91  92  93 ...
//
// le tableau vois doit avoir été alloué
//
//*/
///* ==================================== */
//{
//#undef F_NAME
//#define F_NAME "extract_vois"
//  uint32_t xx, yy, zz, i, j, k;
//  if ((p%rs>=rs-2) || (p%ps<rs-1) || (p%rs<=1) || (p%ps>=ps-rs-rs) ||
//      (p < ps-1) || (p >= N-ps-ps)) /* point de bord */
//  {
//    printf("%s: ERREUR: point de bord\n", F_NAME);
//    exit(0);
//  }
//
//  zz = p / ps;
//  yy = (p % ps) / rs;
//  xx = p % rs;
//  for (k = 0; k < 5; k++)
//    for (j = 0; j < 5; j++)
//      for (i = 0; i < 5; i++)
//      {
//	vois[(k * 25) + (j * 5) + i] =
//	  img[((zz-2+k) * ps) + ((yy-2+j) * rs) + xx-2+i];
//      }
//
//} /* extract_vois() */
//
///* ==================================== */
//int32_t pala_match1(uint8_t *v)
///* ==================================== */
//{
//  if (v[12] && v[37] && v[32] && v[36] && v[38] && v[42] &&
//      !v[81] && !v[82] && !v[83] && !v[86] && !v[87] && !v[88] &&
//      !v[91] && !v[92] && !v[93])
//    return 1;
//  else
//    return 0;
//} // palagyi_match1()
//
///* ==================================== */
//int32_t pala_match2(uint8_t *v)
///* ==================================== */
//{
//  if (v[12] && v[37] && v[32] && v[36] && v[38] && v[42] &&
//      v[67] && (!v[61] || !v[63]) && !v[86] && !v[87] && !v[88] &&
//      !v[81] && !v[82] && !v[83])
//    return 1;
//  else
//    return 0;
//} // palagyi_match2()
//
///* ==================================== */
//int32_t pala_match3(uint8_t *v)
///* ==================================== */
//{
//  if (v[12] && v[37] && v[32] && v[36] && v[38] && v[42] &&
//      v[67] && v[61] && !v[57] && !v[63] && !v[87] && !v[88] &&
//      !v[82] && !v[83])
//    return 1;
//  else
//    return 0;
//} // palagyi_match3()
//
///* ==================================== */
//int32_t pala_match4(uint8_t *v)
///* ==================================== */
//{
//  if (v[12] && v[37] && v[32] && v[36] && v[38] && v[42] &&
//      v[61] && v[63] && v[67] &&
//      !v[81] && !v[82] && !v[83] && !v[86] && !v[87] && !v[88] && !v[92])
//    return 1;
//  else
//    return 0;
//} // palagyi_match4()
//
///* ==================================== */
//int32_t pala_match5(uint8_t *v)
///* ==================================== */
//{
//  if (v[12] && v[37] && v[32] && v[36] && v[38] && v[42] &&
//      v[61] && v[63] && v[66] && v[67] && v[68] &&
//      !v[81] && !v[82] && !v[83] && !v[86] && !v[87] && !v[88])
//    return 1;
//  else
//    return 0;
//} // palagyi_match5()
//
///* ==================================== */
//int32_t pala_match6(uint8_t *v)
///* ==================================== */
//{
//  if (v[12] && v[37] && v[32] && v[36] && v[38] && v[42] && v[57] && v[67] &&
//      !v[61] && !v[63] && !v[86] && !v[87] && !v[88])
//    return 1;
//  else
//    return 0;
//} // palagyi_match6()
//
///* ==================================== */
//int32_t palagyi_match1(uint8_t *v, uint8_t *vv)
///* ==================================== */
//{
//  isometry(vv, v, R0, T0, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R1, T1, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R2, T2, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R3, T3, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R4, T4, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R5, T5, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R6, T6, S0); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R7, T7, S0); if (pala_match1(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R1, T1, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R2, T2, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R3, T3, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R4, T4, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R5, T5, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R6, T6, S1); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R7, T7, S1); if (pala_match1(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R1, T1, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R2, T2, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R3, T3, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R4, T4, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R5, T5, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R6, T6, S2); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R7, T7, S2); if (pala_match1(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R1, T1, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R2, T2, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R3, T3, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R4, T4, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R5, T5, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R6, T6, S3); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R7, T7, S3); if (pala_match1(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R1, T1, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R2, T2, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R3, T3, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R4, T4, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R5, T5, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R6, T6, S4); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R7, T7, S4); if (pala_match1(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R1, T1, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R2, T2, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R3, T3, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R4, T4, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R5, T5, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R6, T6, S5); if (pala_match1(vv)) return 1;
//  isometry(vv, v, R7, T7, S5); if (pala_match1(vv)) return 1;
//
//  return 0;
//} // palagyi_match1()
//
///* ==================================== */
//int32_t palagyi_match2(uint8_t *v, uint8_t *vv)
///* ==================================== */
//{
//  isometry(vv, v, R0, T0, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R1, T1, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R2, T2, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R3, T3, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R4, T4, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R5, T5, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R6, T6, S0); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R7, T7, S0); if (pala_match2(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R1, T1, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R2, T2, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R3, T3, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R4, T4, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R5, T5, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R6, T6, S1); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R7, T7, S1); if (pala_match2(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R1, T1, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R2, T2, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R3, T3, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R4, T4, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R5, T5, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R6, T6, S2); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R7, T7, S2); if (pala_match2(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R1, T1, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R2, T2, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R3, T3, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R4, T4, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R5, T5, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R6, T6, S3); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R7, T7, S3); if (pala_match2(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R1, T1, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R2, T2, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R3, T3, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R4, T4, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R5, T5, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R6, T6, S4); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R7, T7, S4); if (pala_match2(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R1, T1, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R2, T2, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R3, T3, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R4, T4, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R5, T5, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R6, T6, S5); if (pala_match2(vv)) return 1;
//  isometry(vv, v, R7, T7, S5); if (pala_match2(vv)) return 1;
//
//  return 0;
//} // palagyi_match2()
//
///* ==================================== */
//int32_t palagyi_match3(uint8_t *v, uint8_t *vv)
///* ==================================== */
//{
//  isometry(vv, v, R0, T0, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R1, T1, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R2, T2, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R3, T3, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R4, T4, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R5, T5, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R6, T6, S0); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R7, T7, S0); if (pala_match3(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R1, T1, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R2, T2, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R3, T3, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R4, T4, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R5, T5, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R6, T6, S1); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R7, T7, S1); if (pala_match3(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R1, T1, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R2, T2, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R3, T3, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R4, T4, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R5, T5, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R6, T6, S2); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R7, T7, S2); if (pala_match3(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R1, T1, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R2, T2, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R3, T3, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R4, T4, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R5, T5, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R6, T6, S3); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R7, T7, S3); if (pala_match3(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R1, T1, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R2, T2, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R3, T3, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R4, T4, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R5, T5, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R6, T6, S4); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R7, T7, S4); if (pala_match3(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R1, T1, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R2, T2, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R3, T3, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R4, T4, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R5, T5, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R6, T6, S5); if (pala_match3(vv)) return 1;
//  isometry(vv, v, R7, T7, S5); if (pala_match3(vv)) return 1;
//
//  return 0;
//} // palagyi_match3()
//
///* ==================================== */
//int32_t palagyi_match4(uint8_t *v, uint8_t *vv)
///* ==================================== */
//{
//  isometry(vv, v, R0, T0, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R1, T1, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R2, T2, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R3, T3, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R4, T4, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R5, T5, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R6, T6, S0); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R7, T7, S0); if (pala_match4(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R1, T1, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R2, T2, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R3, T3, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R4, T4, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R5, T5, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R6, T6, S1); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R7, T7, S1); if (pala_match4(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R1, T1, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R2, T2, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R3, T3, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R4, T4, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R5, T5, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R6, T6, S2); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R7, T7, S2); if (pala_match4(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R1, T1, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R2, T2, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R3, T3, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R4, T4, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R5, T5, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R6, T6, S3); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R7, T7, S3); if (pala_match4(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R1, T1, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R2, T2, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R3, T3, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R4, T4, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R5, T5, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R6, T6, S4); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R7, T7, S4); if (pala_match4(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R1, T1, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R2, T2, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R3, T3, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R4, T4, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R5, T5, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R6, T6, S5); if (pala_match4(vv)) return 1;
//  isometry(vv, v, R7, T7, S5); if (pala_match4(vv)) return 1;
//
//  return 0;
//} // palagyi_match4()
//
///* ==================================== */
//int32_t palagyi_match5(uint8_t *v, uint8_t *vv)
///* ==================================== */
//{
//  isometry(vv, v, R0, T0, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R1, T1, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R2, T2, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R3, T3, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R4, T4, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R5, T5, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R6, T6, S0); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R7, T7, S0); if (pala_match5(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R1, T1, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R2, T2, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R3, T3, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R4, T4, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R5, T5, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R6, T6, S1); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R7, T7, S1); if (pala_match5(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R1, T1, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R2, T2, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R3, T3, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R4, T4, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R5, T5, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R6, T6, S2); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R7, T7, S2); if (pala_match5(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R1, T1, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R2, T2, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R3, T3, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R4, T4, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R5, T5, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R6, T6, S3); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R7, T7, S3); if (pala_match5(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R1, T1, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R2, T2, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R3, T3, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R4, T4, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R5, T5, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R6, T6, S4); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R7, T7, S4); if (pala_match5(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R1, T1, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R2, T2, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R3, T3, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R4, T4, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R5, T5, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R6, T6, S5); if (pala_match5(vv)) return 1;
//  isometry(vv, v, R7, T7, S5); if (pala_match5(vv)) return 1;
//
//  return 0;
//} // palagyi_match5()
//
///* ==================================== */
//int32_t palagyi_match6(uint8_t *v, uint8_t *vv)
///* ==================================== */
//{
//  isometry(vv, v, R0, T0, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R1, T1, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R2, T2, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R3, T3, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R4, T4, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R5, T5, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R6, T6, S0); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R7, T7, S0); if (pala_match6(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R1, T1, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R2, T2, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R3, T3, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R4, T4, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R5, T5, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R6, T6, S1); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R7, T7, S1); if (pala_match6(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R1, T1, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R2, T2, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R3, T3, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R4, T4, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R5, T5, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R6, T6, S2); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R7, T7, S2); if (pala_match6(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R1, T1, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R2, T2, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R3, T3, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R4, T4, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R5, T5, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R6, T6, S3); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R7, T7, S3); if (pala_match6(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R1, T1, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R2, T2, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R3, T3, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R4, T4, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R5, T5, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R6, T6, S4); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R7, T7, S4); if (pala_match6(vv)) return 1;
//
//  isometry(vv, v, R0, T0, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R1, T1, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R2, T2, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R3, T3, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R4, T4, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R5, T5, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R6, T6, S5); if (pala_match6(vv)) return 1;
//  isometry(vv, v, R7, T7, S5); if (pala_match6(vv)) return 1;
//
//  return 0;
//} // palagyi_match6()
//
///* ==================================== */
//int32_t lpalagyi(struct xvimage *image,
//		 int32_t nsteps)
///* ==================================== */
//#undef F_NAME
//#define F_NAME "lpalagyi"
//{
//  int32_t i, j, k, x;
//  int32_t rs = rowsize(image);     /* taille ligne */
//  int32_t cs = colsize(image);     /* taille colonne */
//  int32_t ds = depth(image);       /* nb plans */
//  int32_t ps = rs * cs;            /* taille plan */
//  int32_t N = ps * ds;             /* taille image */
//  uint8_t *S = UCHARDATA(image);      /* l'image de depart */
//  struct xvimage *t = copyimage(image);
//  uint8_t *T = UCHARDATA(t);
//  int32_t step, nonstab;
//  uint8_t v[125];
//  uint8_t vv[125];
//
//#ifdef VERBOSE
//    printf("Warning: this is a very slow implementation. I might take time...\n");
//#endif
//
//  if (nsteps == -1) nsteps = 1000000000;
//
//  /* ================================================ */
//  /*               DEBUT ALGO                         */
//  /* ================================================ */
//
//  step = 0;
//  nonstab = 1;
//  while (nonstab && (step < nsteps))
//  {
//    nonstab = 0;
//    step++;
//#ifdef VERBOSE
//    printf("step %d\n", step);
//#endif
//    for (k = 2; k < ds-2; k++)
//      for (j = 2; j < cs-2; j++)
//	for (i = 2; i < rs-2; i++)
//	{
//	  x = k*ps + j*rs + i;
//	  if (T[x])
//	  {
//	    extract_vois(T, x, rs, ps, N, v);
//	    if (palagyi_match1(v, vv) || palagyi_match2(v, vv) ||
//		palagyi_match3(v, vv) || palagyi_match4(v, vv) ||
//		palagyi_match5(v, vv) || palagyi_match6(v, vv))
//	    {
//	      S[x] = 0;
//	      nonstab = 1;
//	    }
//	  }
//	} // for k,j,i
//    memcpy(T, S, N);
//  } // while (nonstab && (step < nsteps))
//
//#ifdef VERBOSE1
//    printf("number of steps: %d\n", step);
//#endif
//
//  for (i = 0; i < N; i++) if (S[i]) S[i] = 255; // normalize values
//
//  freeimage(t);
//  return(1);
//} /* lpalagyi() */
//
//#ifdef PROBE
//#include <mckhalimsky3d.h>
//#include <l3dkhalimsky.h>
//
///* =============================================================== */
//int32_t main()
///* =============================================================== */
//{
//  struct xvimage * image;
//  int32_t i, j, k;
//  int32_t rs, cs, ds, ps, N, valmax;
//  uint8_t *Im;
//  struct xvimage * kh;
//  int32_t nbcc1, nbcav1, nbtun1, euler1;
//  int32_t nbcc2, nbcav2, nbtun2, euler2;
//
//  rs = 9;
//  cs = 9;
//  ds = 9;
//  ps = rs * cs;
//  N = ps * ds;
//  valmax = 5;
//  srand(time(NULL));
//  image = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
//  Im = UCHARDATA(image);
//  memset(Im, 0, N);
//
//  while(1)
//  {
//    for (k = 2; k < 7; k++)
//    for (j = 2; j < 7; j++)
//    for (i = 2; i < 7; i++)
//    {
//      Im[k*ps + j*rs + i] = (int8_t)((valmax+1.0)*rand() / (RAND_MAX + 1.0));
//    }
//    l3dkhalimskize(image, &kh, 0);
//    l3dinvariants(kh, &nbcc1, &nbcav1, &nbtun1, &euler1);
//    freeimage(kh);
//    lpalagyi(image, -1);
//    l3dkhalimskize(image, &kh, 0);
//    l3dinvariants(kh, &nbcc2, &nbcav2, &nbtun2, &euler2);
//    freeimage(kh);
//    if ((nbcc1 != nbcc2) || (nbcav1 != nbcav2) || (nbtun1 != nbtun2))
//    {
//      printf("BINGO !\n");
//      writeimage(image,"contre_exemple.pgm");
//      return 1;
//    }
//    printf("%d composantes connexes, ", nbcc1);
//    printf("%d cavites, ", nbcav1);
//    printf("%d tunnels\n", nbtun1);
//  }
//  return 0;
//} /* main */
//#endif




/*******************************************************************************
*
*       Author:              K. Palagyi (adapted by J. Chaussard for PINK)
*
*******************************************************************************/



/*========= function read_image =========*/
/* -------------
  considered global variables:
      NONE
  changed global variables:
      pgmimage
      size_x
      size_y
      size_z
      size_xy
      size_xyz
      image
----------------*/
void read_image()
{
	unsigned long int  i;

	image=(unsigned char*)pgmimage->image_data;
	size_x=rowsize(pgmimage);
	size_y=colsize(pgmimage);
	size_z=depth(pgmimage);
	size_xy=size_x*size_y;
	size_xyz=size_xy*size_z;

	if ( size_z <= 1 )
      	{
        	printf("ERROR: The image data is not a 3D one. Number of slices is %lu \n", size_z);
        	exit(-1);
      	}

	for(i=0; i<size_xyz; i++)
		if(UCHARDATA(pgmimage)[i]!=0) UCHARDATA(pgmimage)[i]=1; //Threshold
}
/*========= end of function read_image =========*/



/*========= function write_image =========*/
/* -------------
  considered global variables:
      pgmimage
      size_xyz
  changed global variables:
      image (data)
----------------*/
void write_image()
{
	unsigned long int  i;

	for(i=0; i<size_xyz; i++)
		if(UCHARDATA(pgmimage)[i]!=0) UCHARDATA(pgmimage)[i]=255; //Threshold
}

/*========= end of function write_image =========*/


/*========= function set_long_mask =========*/
/* -------------
  considered global variables:
      NONE
  changed global variables:
      long_mask
----------------*/
void set_long_mask( void )
  {
    long_mask[ 0] = 0x00000001;
    long_mask[ 1] = 0x00000002;
    long_mask[ 2] = 0x00000004;
    long_mask[ 3] = 0x00000008;
    long_mask[ 4] = 0x00000010;
    long_mask[ 5] = 0x00000020;
    long_mask[ 6] = 0x00000040;
    long_mask[ 7] = 0x00000080;
    long_mask[ 8] = 0x00000100;
    long_mask[ 9] = 0x00000200;
    long_mask[10] = 0x00000400;
    long_mask[11] = 0x00000800;
    long_mask[12] = 0x00001000;
    long_mask[13] = 0x00002000;
    long_mask[14] = 0x00004000;
    long_mask[15] = 0x00008000;
    long_mask[16] = 0x00010000;
    long_mask[17] = 0x00020000;
    long_mask[18] = 0x00040000;
    long_mask[19] = 0x00080000;
    long_mask[20] = 0x00100000;
    long_mask[21] = 0x00200000;
    long_mask[22] = 0x00400000;
    long_mask[23] = 0x00800000;
    long_mask[24] = 0x01000000;
    long_mask[25] = 0x02000000;
  }
/*========= end of function set_char_mask =========*/


/*========= function set_char_mask =========*/
/* -------------
  considered global variables:
      NONE
  changed global variables:
      char_mask
----------------*/
void set_char_mask( void )
  {
    char_mask[0] = 0x01;
    char_mask[1] = 0x02;
    char_mask[2] = 0x04;
    char_mask[3] = 0x08;
    char_mask[4] = 0x10;
    char_mask[5] = 0x20;
    char_mask[6] = 0x40;
    char_mask[7] = 0x80;
  }
/*========= end of function set_long_mask =========*/


/*============= function init_lut =============*/
/* -------------
  considered global variables:
      NONE
  changed global variables:
      lut_deletable
      long_mask
      char_mask
----------------*/
void init_lut( char* mask_name )
{
  char  lutname [100];
  FILE  *lutfile;

  /* alloc lut_deletable */
    lut_deletable = (unsigned char *)malloc(0x00400000);    /* 4 Mbyte */
    if ( lut_deletable == NULL)
      {
        printf("\n Alloc error!!!\n");
        exit(1);
      }  /* end if */

  /* open lutfile */
    strcpy( lutname, mask_name);
    lutfile = fopen( lutname, "rb");
    if ( lutfile == NULL)
      {
        printf("\n\n file %s is not found!!!\n", mask_name);
        exit(1);
      }  /* end if */

  /* reading lutfile */
    if ( fread( lut_deletable, 1, 0x00400000, lutfile) != 0x00400000 )      /* 4 Mbyte */
      {
         printf("ERROR: Couldn't read LUT\n");
         exit(1);
      }

    fclose(lutfile);

  /* setting masks */
    set_long_mask();
    set_char_mask();

}
/*=========== end of function init_lut ===========*/



/*==================================*/
/*========= list functions =========*/
/*==================================*/

void NewSurfaceVoxel( void ) {
ListElement * LE;
	LE=(ListElement *)malloc(sizeof(ListElement));
	(*LE).addr=act_addr;
	(*LE).next=NULL;
	(*LE).prev=SurfaceVoxels.last;
	if (SurfaceVoxels.last!=NULL) (*((ListElement*)(SurfaceVoxels.last))).next=LE;
	SurfaceVoxels.last=LE;
	if (SurfaceVoxels.first==NULL) SurfaceVoxels.first=LE;
}

void RemoveSurfaceVoxel(ListElement * LE) {
ListElement * LE2;
	if (SurfaceVoxels.first==LE) SurfaceVoxels.first=(*LE).next;
	if (SurfaceVoxels.last==LE) SurfaceVoxels.last=(*LE).prev;
	if ((*LE).next!=NULL) {
		LE2=(ListElement*)((*LE).next);
		(*LE2).prev=(*LE).prev;
	}
	if ((*LE).prev!=NULL) {
		LE2=(ListElement*)((*LE).prev);
		(*LE2).next=(*LE).next;
	}
	free(LE);
}

void CreatePointList(PointList *s) {
	s->Head=NULL;
	s->Tail=NULL;
	s->Length=0;
}

void AddToList(PointList *s, ListElement * ptr) {
Cell * newcell;
	newcell=(Cell *)malloc(sizeof(Cell));
	newcell->addr=act_addr;
	newcell->ptr=ptr;
	newcell->next=NULL;
	if (s->Head==NULL) {
		s->Head=newcell;
		s->Tail=newcell;
		s->Length=1;
	}
	else {
		s->Tail->next=newcell;
		s->Tail=newcell;
		s->Length++;
	}
}

unsigned long int GetFromList(PointList *s, ListElement **ptr) {
unsigned long int curr_addr;
Cell *tmp;
	(*ptr)=NULL;
	if(s->Length==0) return 0;
	else {
	        curr_addr = s->Head->addr;
		(*ptr)=s->Head->ptr;
		tmp=(Cell *)s->Head->next;
		free(s->Head);
		s->Head=tmp;
		s->Length--;
		if(s->Length==0) {
			s->Head=NULL;
			s->Tail=NULL;
		}
                return curr_addr;
	}
}

void DestroyPointList(PointList *s) {
ListElement * ptr;
	while(s->Length>0) GetFromList(s, &ptr);
}

void CollectSurfaceVoxels(void) {
unsigned long int x,y,z, num;
unsigned long int z_size_xy, y_size_x;

  SurfaceVoxels.first = NULL;
  SurfaceVoxels.last  = NULL;
  num = 0;

  for( z=1, z_size_xy=size_xy;
       z<size_z-1;
       z++, z_size_xy+=size_xy )
    {
      for( y=1, y_size_x=size_x;
           y<size_y-1;
           y++, y_size_x+=size_x )
        {
          for(x=1; x<size_x-1; x++)
	    {
	      act_addr = x + y_size_x + z_size_xy;
              if ( *(image + act_addr ) )
                {
                  if (  ( *(image + act_addr     - size_x           ) ==0 ) ||
                        ( *(image + act_addr     + size_x           ) ==0 ) ||
                        ( *(image + act_addr              - size_xy ) ==0 ) ||
                        ( *(image + act_addr              + size_xy ) ==0 ) ||
                        ( *(image + act_addr + 1                    ) ==0 ) ||
                        ( *(image + act_addr - 1                    ) ==0 )    )
                     {
                        *(image + act_addr ) = 2;    /* surface point */
                        NewSurfaceVoxel();
		        num ++;
                     } /* endif */
                } /* endif */
            } /* endfor x */
        } /* endfor y */
    } /* endfor z */
#ifdef VERBOSE
  printf("\n Number of surface/bourder points in the original image: %lu\n", num);
#endif
}

/*===============================================================*/
/*========= functions concerning topological properties =========*/
/*===============================================================*/


/*========= function collect_26_neighbours =========*/
void collect_26_neighbours( void )
  {

    neighbours = 0x00000000;

    if ( direction == U )
      {
        /*  U
          indices in "neighbours":
           0  1  2     8  9 10     16  17  18   y-1
           3  .  4    11  . 12     19  20  21   y
           5  6  7    13 14 15     22  23  24   y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1
        */
        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr     + size_x           ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[15];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[24];
      } /* endif U */
   else
      { /* not-U */
       if ( direction == D )
      {
        /*  D
          indices in "neighbours":
           16  17  18     8  9 10     0  1  2   y-1
           19  20  21    11  . 12     3  .  4   y
           22  23  24    13 14 15     5  6  7   y+1
          x-1 x x+1     x-1 x x+1   x-1  x  x+1
              z-1          z            z+1
        */
        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr              - size_xy ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[24];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr     + size_x           ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[15];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 7];
      } /* endif D */
    else
      { /* not-D */
    if ( direction == N )
      {
        /*  N
          indices in "neighbours":
           5  6  7     3  .  4      0   1   2   y-1
          13 14 15    11  . 12      8   9  10   y
          22 23 24    19 20 21     16  17  18   y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1
        */
        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr              - size_xy ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[15];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[24];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr     + size_x           ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[21];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[18];
      } /* endif N */
    else
      { /* not-N */
     if ( direction == S )
      {
        /*  S
          indices in "neighbours":
          22 23 24    19 20 21     16  17  18   y-1
          13 14 15    11  . 12      8   9  10   y
           5  6  7     3  .  4      0   1   2   y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1
        */
        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[24];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr              - size_xy ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[15];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[ 4];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 2];
      } /* endif S */
    else
      { /* not-S */
     if ( direction == E )
      {
        /*  E
          indices in "neighbours":
           2 10 18     1  9 17      0   8  16   y-1
           4 12 21     .  . 20      3  11  19   y
           7 15 24     6 14 23      5  13  22   y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1
        */
        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr              - size_xy ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[15];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[24];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr     + size_x           ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[23];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[22];
      } /* endif E */
    else
      { /* not-E */
     if ( direction == W )
      {
        /*  W
          indices in "neighbours":
          18 10  2    17  9  1     16   8  0   y-1
          21 12  4    20  .  .     19  11  3   y
          24 15  7    23 14  6     22  13  5   y+1
          x-1 x x+1   x-1 x x+1    x-1  x x+1
             z-1          z           z+1
        */
        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr              - size_xy ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[24];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[15];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr     + size_x           ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[ 6];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 5];
      } /* endif W */
      } /* endelse not-E */
      } /* endelse not-S */
      } /* endelse not-N */
      } /* endelse not-D */
      } /* endelse not-U */


  }
/*========= end of function collect_26_neighbours =========*/


/*========= function deletable =========*/
int deletable( void )
{
  collect_26_neighbours();
  return ( ( *(lut_deletable + (neighbours>>3) ) ) & char_mask[neighbours%8]);
}
/*========= end of function deletable =========*/


/*=========== function DetectDeletablePoints ===========*/
void DetectDeletablePoints(PointList *s) {
ListElement * LE3;
unsigned char maybe;

  LE3=(ListElement *)SurfaceVoxels.first;
  while (LE3!=NULL)
    {

      act_addr = (*LE3).addr;

      maybe = 0;

      if ( direction == U )
        {
          if ( *(image + act_addr - size_xy ) == 0 )
            maybe = 1;
	} /* endif U */
      else
        { /* not-U */
      if ( direction == D )
        {
          if ( *(image + act_addr + size_xy ) == 0 )
            maybe = 1;
	}  /* endif D */
      else
        { /* not-D */
      if ( direction == N )
        {
          if ( *(image + act_addr - size_x  ) == 0 )
            maybe = 1;
	} /* endif N */
      else
        { /* not-N */
      if ( direction == S )
        {
          if ( *(image + act_addr + size_x  ) == 0 )
            maybe = 1;
	} /* endif S */
      else
        { /* not-S */
      if ( direction == E )
        {
          if ( *(image + act_addr - 1       ) == 0 )
            maybe = 1;
	} /* endif E */
      else
        { /* not_E */
      if ( direction == W )
        {
          if ( *(image + act_addr + 1       ) == 0 )
            maybe = 1;
	} /* endif W */
	} /* endelse not-E */
	} /* endelse not-S */
	} /* endelse not-N */
	} /* endelse not-D */
	} /* endelse not-U */

      if ( maybe )
        {
          if ( deletable() )
            {
	       AddToList(s,LE3);
            } /* endif */
        } /* endif */

      LE3=(ListElement *)(*LE3).next;
    } /* endwhile */

}
/*=========== end of function DetectDeletablePoints ===========*/


/*========= function thinning_iteration_step =========*/
unsigned int thinning_iteration_step(void)
{
  unsigned long int i, changed;
  ListElement * ptr;
  PointList s;
  unsigned long int  curr_addr;

  changed = 0;
  for ( i=0; i<6; i++ )                          /* 6-subiteration alg. */
    {
      direction = i;
      CreatePointList(&s);
      DetectDeletablePoints(&s);
      while ( s.Length > 0 )
        {
           curr_addr = GetFromList( &s, &ptr );
           *(image + curr_addr ) = 0;            /* deleted */
	  changed ++;
           /* investigating v's 6-neighbours */
	      act_addr = curr_addr - 1;          /* (x-1,y,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr + 1;          /* (x+1,y,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr - size_x;     /* (x,y-1,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr + size_x;     /* (x,y+1,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr - size_xy;    /* (x,y,z-1) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr + size_xy;    /* (x,y,z+1) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
            RemoveSurfaceVoxel(ptr);
        } /* endwhile */
      DestroyPointList(&s);
    } /* endfor */

  return changed;
}
/*========= end of function thinning_iteration_step =========*/

/*========= function thinning =========*/
void thinning(void)
{
  unsigned int iter, changed;

  CollectSurfaceVoxels();

  iter = 0;
  changed = 1;
  while ( changed )
    {
      changed = thinning_iteration_step();
      iter++;
#ifdef VERBOSE
      printf("\n  iteration step: %3d.    (deleted point(s): %6d)",
             iter, changed );
#endif
    }

}
/*========= end of function thinning =========*/


int palagyi_skelpar_curv_98(struct xvimage *input)
{
	char tablefilename[512];
	pgmimage=input;
  /********************/
  /* READ INPUT IMAGE */
  /********************/
    read_image();

  /***************/
  /* READING LUT */
  /***************/
	sprintf(tablefilename, "%s/src/tables/palagyi_pardir_curv_98.dat", getenv("PINK"));
    init_lut(tablefilename);

  /************/
  /* THINNING */
  /************/
#ifdef VERBOSE
    printf("\n 6-subiteration directional curve-thinning ...");
#endif
    thinning();

  /********************/
  /* WRITE OUPUT IMAGE */
  /********************/
    write_image( );

  /********/
  /* FREE */
  /********/
    free(lut_deletable);


    return(0);
}
/*===========================================================================
    end of function    m a i n
  ===========================================================================*/


/*============= function init_lut_simple =============*/
/* -------------
  considered global variables:
      NONE
  changed global variables:
      lut_simple
      long_mask
      char_mask
----------------*/
void init_lut_simple( char* mask_name )
{
  char  lutname [100];
  FILE  *lutfile;

  /* alloc lut_simple */
    lut_simple = (unsigned char *)malloc(0x00800000);
    if ( lut_simple == NULL)
      {
        printf("\n Alloc error!!!\n");
        exit(1);
      }  /* end if */

  /* open lutfile */
    strcpy( lutname, mask_name);
    lutfile = fopen( lutname, "rb");
    if ( lutfile == NULL)
      {
        printf("\n\n file %s is not found!!!\n", mask_name);
        exit(1);
      }  /* end if */

  /* reading lutfile */
    fread( lut_simple, 1, 0x00800000, lutfile);
    fclose(lutfile);

  /* setting masks */
    set_long_mask();
    set_char_mask();

}
/*=========== end of function init_lut_simple ===========*/


void NewSurfaceVoxel2(unsigned long int x,
                     unsigned long int y,
		     unsigned long int z) {
ListElement * LE;
	LE=(ListElement *)malloc(sizeof(ListElement));
	(*LE).x=x;
	(*LE).y=y;
	(*LE).z=z;
	(*LE).next=NULL;
	(*LE).prev=SurfaceVoxels.last;
	if (SurfaceVoxels.last!=NULL) (*((ListElement*)(SurfaceVoxels.last))).next=LE;
	SurfaceVoxels.last=LE;
	if (SurfaceVoxels.first==NULL) SurfaceVoxels.first=LE;
}


void AddToList2(PointList *s,Voxel e, ListElement * ptr) {
Cell * newcell;
	newcell=(Cell *)malloc(sizeof(Cell));
	newcell->v=e;
	newcell->ptr=ptr;
	newcell->next=NULL;
	if (s->Head==NULL) {
		s->Head=newcell;
		s->Tail=newcell;
		s->Length=1;
	}
	else {
		s->Tail->next=newcell;
		s->Tail=newcell;
		s->Length++;
	}
}

Voxel GetFromList2(PointList *s, ListElement **ptr) {
Voxel R;
Cell *tmp;
        R.i = -1;
        R.j = -1;
        R.k = -1;
        R.oldnei = 0;    /*** +field ***/
	(*ptr)=NULL;
	if(s->Length==0) return R;
	else {
		R=s->Head->v;
		(*ptr)=s->Head->ptr;
		tmp=(Cell *)s->Head->next;
		free(s->Head);
		s->Head=tmp;
		s->Length--;
		if(s->Length==0) {
			s->Head=NULL;
			s->Tail=NULL;
		}
		return R;
	}
}

void CollectSurfaceVoxels2(void) {
unsigned long int x,y,z;

  SurfaceVoxels.first = NULL;
  SurfaceVoxels.last  = NULL;

  for( z=1, z_size_xy=size_xy;
       z<size_z-1;
       z++, z_size_xy+=size_xy )
    {
      zm_size_xy = z_size_xy - size_xy;
      zp_size_xy = z_size_xy + size_xy;
      for( y=1, y_size_x=size_x;
           y<size_y-1;
           y++, y_size_x+=size_x )
        {
          ym_size_x  = y_size_x - size_x;
          yp_size_x  = y_size_x + size_x;
          for(x=1; x<size_x-1; x++)
            if ( *(image + x + y_size_x + z_size_xy ) )
              {
                if (  ( *(image +   x + ym_size_x +  z_size_xy ) ==0 ) ||
                      ( *(image +   x + yp_size_x +  z_size_xy ) ==0 ) ||
                      ( *(image +   x +  y_size_x + zm_size_xy ) ==0 ) ||
                      ( *(image +   x +  y_size_x + zp_size_xy ) ==0 ) ||
                      ( *(image + x+1 +  y_size_x +  z_size_xy ) ==0 ) ||
                      ( *(image + x-1 +  y_size_x +  z_size_xy ) ==0 )    )
                   {
                      *(image + x + y_size_x + z_size_xy ) = 2;
                      NewSurfaceVoxel2(x,y,z);
                   } /* endif */
              } /* endif */
        } /* endfor y */
    } /* endfor z */

}

void DestroyPointList2(PointList *s) {
ListElement * ptr;
	while(s->Length>0) GetFromList2(s, &ptr);
}

/*===============================================================*/
/*========= functions concerning topological properties =========*/
/*===============================================================*/

/*========= function collect_26_neighbours =========*/
void collect_26_neighbours2(unsigned long int x,
                           unsigned long int y,
                           unsigned long int z )
  {
    /*
      indices in "neighbours":
      0  1  2     9 10 11     17 18 19    y-1
      3  4  5    12    13     20 21 22     y
      6  7  8    14 15 16     23 24 25    y+1
     x-1 x x+1   x-1 x x+1    x-1 x x+1
        z-1          z           z+1
    */

    z_size_xy  = z*size_xy;
    zm_size_xy = z_size_xy - size_xy;
    zp_size_xy = z_size_xy + size_xy;
    y_size_x   = y*size_x;
    ym_size_x  = y_size_x  - size_x;
    yp_size_x  = y_size_x  + size_x;

    neighbours = 0x00000000;
    endflag = 0;

    if ( *(image + x-1 + ym_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 0];
        endflag++;
      }
    if ( *(image +   x + ym_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 1];
        endflag++;
      }
    if ( *(image + x+1 + ym_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 2];
        endflag++;
      }
    if ( *(image + x-1 +  y_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 3];
        endflag++;
      }
    if ( *(image +   x +  y_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 4];
        endflag++;
      }
    if ( *(image + x+1 +  y_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 5];
        endflag++;
      }
    if ( *(image + x-1 + yp_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 6];
        endflag++;
      }
    if ( *(image +   x + yp_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 7];
        endflag++;
      }
    if ( *(image + x+1 + yp_size_x + zm_size_xy ) )
      {
        neighbours |= long_mask[ 8];
        endflag++;
      }

    if ( *(image + x-1 + ym_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[ 9];
        endflag++;
      }
    if ( *(image +   x + ym_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[10];
        endflag++;
      }
    if ( *(image + x+1 + ym_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[11];
        endflag++;
      }
    if ( *(image + x-1 +  y_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[12];
        endflag++;
      }
    if ( *(image + x+1 +  y_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[13];
        endflag++;
      }
    if ( *(image + x-1 + yp_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[14];
        endflag++;
      }
    if ( *(image +   x + yp_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[15];
        endflag++;
      }
    if ( *(image + x+1 + yp_size_x +  z_size_xy ) )
      {
        neighbours |= long_mask[16];
        endflag++;
      }

    if ( *(image + x-1 + ym_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[17];
        endflag++;
      }
    if ( *(image +   x + ym_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[18];
        endflag++;
      }
    if ( *(image + x+1 + ym_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[19];
        endflag++;
      }
    if ( *(image + x-1 +  y_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[20];
        endflag++;
      }
    if ( *(image +   x +  y_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[21];
        endflag++;
      }
    if ( *(image + x+1 +  y_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[22];
        endflag++;
      }
    if ( *(image + x-1 + yp_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[23];
        endflag++;
      }
    if ( *(image +   x + yp_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[24];
        endflag++;
      }
    if ( *(image + x+1 + yp_size_x + zp_size_xy ) )
      {
        neighbours |= long_mask[25];
        endflag++;
      }

  }
/*========= end of function collect_26_neighbours =========*/


/*========= function simple_26_6 =========*/
int simple_26_62(unsigned long int x,
                unsigned long int y,
                unsigned long int z )
{
  collect_26_neighbours2(x,y,z);
  return ( ( *(lut_simple + (neighbours>>3) ) ) & char_mask[neighbours%8]);
}
/*========= end of function simple_26_6 =========*/


/*=========== function DetectSimpleBorderPoints ===========*/
void DetectSimpleBorderPoints2(PointList *s) {
unsigned char value;
Voxel v;
ListElement * LE3;
unsigned long int x, y, z;

  LE3=(ListElement *)SurfaceVoxels.first;
  while (LE3!=NULL)
    {
      x = (*LE3).x;
      y = (*LE3).y;
      z = (*LE3).z;
      y_size_x   = y*size_x;
      ym_size_x  = y_size_x - size_x;
      yp_size_x  = y_size_x + size_x;
      z_size_xy  = z*size_xy;
      zm_size_xy = z_size_xy - size_xy;
      zp_size_xy = z_size_xy + size_xy;
      switch( direction )
        {
          case U: value = *(image + x   + ym_size_x + z_size_xy  );
                  break;
          case D: value = *(image + x   + yp_size_x + z_size_xy  );
                  break;
          case N: value = *(image + x   + y_size_x  + zm_size_xy );
                  break;
          case S: value = *(image + x   + y_size_x  + zp_size_xy );
                  break;
          case E: value = *(image + x+1 + y_size_x  + z_size_xy  );
                  break;
          case W: value = *(image + x-1 + y_size_x  + z_size_xy  );
                  break;
        } /* endswitch */
      if( value == 0 )
        {
          if ( simple_26_62(x, y, z) )
            {
              v.i = x;
              v.j = y;
              v.k = z;
              v.oldnei = neighbours;     /*** storing oldnei ***/
              AddToList2(s,v,LE3);
            } /* endif */
        } /* endif */
      LE3=(ListElement *)(*LE3).next;
    } /* endwhile */

}
/*=========== end of function DetectSimpleBorderPoints ===========*/


/*========= function thinning_iteration_step =========*/
unsigned int thinning_iteration_step2(void)
{
  unsigned long int i, changed, deletable, diffnei, del6nei;
  ListElement * ptr;
  PointList s;
  Voxel v;

  changed = 0;
  for ( i=0; i<6; i++ )
    {
      direction = i;
      CreatePointList(&s);
      DetectSimpleBorderPoints2(&s);
      while ( s.Length > 0 )
        {
           v = GetFromList2( &s, &ptr );
           deletable = 0;
           if ( simple_26_62( v.i, v.j, v.k ) )
             {
               if ( endflag > 1 )
                 {
                   deletable = 1;    /*** simple and not endpoint ***/
                 }
               else                  /*** endpoint ***/
                 {
                   diffnei = v.oldnei ^ neighbours;
                   del6nei = 0;
                   if ( diffnei & long_mask[ 4] )
                     del6nei++;
                   if ( diffnei & long_mask[10] )
                     del6nei++;
                   if ( diffnei & long_mask[12] )
                     del6nei++;
                   if ( diffnei & long_mask[13] )
                     del6nei++;
                   if ( diffnei & long_mask[15] )
                     del6nei++;
                   if ( diffnei & long_mask[21] )
                     del6nei++;
                   if ( del6nei >= DEL6NEI )
                     deletable = 1;          /*** deletable endpoint found ***/
                 }
               if ( deletable )
                 {
                    z_size_xy = (v.k)*size_xy;
                    y_size_x  = (v.j)*size_x;
                    *(image + v.i + y_size_x + z_size_xy ) = 0; /* simple point is deleted */
                    changed ++;
                    /* investigating v's 6-neighbours */
                      if (*(image + v.i-1 + y_size_x + z_size_xy                )==1)
                        {
                          NewSurfaceVoxel2( v.i-1, v.j  , v.k   );
                          *(image + v.i-1 + y_size_x + z_size_xy                ) = 2;
                        }
                      if (*(image + v.i+1 + y_size_x        + z_size_xy         )==1)
                        {
                          NewSurfaceVoxel2( v.i+1, v.j  , v.k   );
                          *(image + v.i+1 + y_size_x        + z_size_xy         ) = 2;
                        }
                      if (*(image + v.i   + y_size_x-size_x + z_size_xy         )==1)
                        {
                          NewSurfaceVoxel2( v.i  , v.j-1, v.k   );
                          *(image + v.i   + y_size_x-size_x + z_size_xy         ) = 2;
                        }
                      if (*(image + v.i   + y_size_x+size_x + z_size_xy         )==1)
                        {
                          NewSurfaceVoxel2( v.i  , v.j+1, v.k   );
                          *(image + v.i   + y_size_x+size_x + z_size_xy         ) = 2;
                        }
                      if (*(image + v.i   + y_size_x        + z_size_xy-size_xy )==1)
                        {
                          NewSurfaceVoxel2( v.i  , v.j  , v.k-1 );
                          *(image + v.i   + y_size_x        + z_size_xy-size_xy ) = 2;
                        }
                      if (*(image + v.i   + y_size_x        + z_size_xy+size_xy )==1)
                        {
                          NewSurfaceVoxel2( v.i  , v.j  , v.k+1 );
                          *(image + v.i   + y_size_x        + z_size_xy+size_xy ) = 2;
                        }
                    RemoveSurfaceVoxel(ptr);
                 } /* endif */
             } /* endif */
        } /* endwhile */
      DestroyPointList2(&s);
    } /* endfor */

  return changed;
}
/*========= end of function thinning_iteration_step =========*/

/*========= function sequential_thinning =========*/
void sequential_thinning2(void)
{
  unsigned int iter, changed;

  CollectSurfaceVoxels2();

  iter = 0;
  changed = 1;
  while ( changed )
    {
      changed = thinning_iteration_step2();
      iter++;
      printf("\n  iteration step: %3d.    (deleted point(s): %6d)",
             iter, changed );
    }

}
/*========= end of function sequential_thinning =========*/


int palagyi_skelpar_curv_06(struct xvimage *input)
{
	char tablefilename[512];
	pgmimage=input;
	DEL6NEI = 1;
  /********************/
  /* READ INPUT IMAGE */
  /********************/
    read_image();

  /***************/
  /* READING LUT */
  /***************/
	sprintf(tablefilename, "%s/src/tables/palagyi_pardir_curv_06.dat", getenv("PINK"));
    init_lut_simple(tablefilename);

  /************/
  /* THINNING */
  /************/
#ifdef VERBOSE
    printf("\n Centerline extraction by thinning ...");
#endif
    sequential_thinning2();

  /********************/
  /* WRITE OUPUT IMAGE */
  /********************/
    write_image( );

  /********/
  /* FREE */
  /********/
    free(lut_simple);


    return(0);
}



/*========= function collect_26_neighbours =========*/
void collect_26_neighbours3( void )
  {

    neighbours = 0x00000000;

    if ( direction == UD )
      {
        /*  UD
          indices in "neighbours":
           0  1  2     9 10 11     17  18  19   y-1
           3  4  5    12    13     20      21   y
           6  7  8    14 15 16     22  23  24   y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1
        */

        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr              - size_xy ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 8];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr     + size_x           ) )
          neighbours |= long_mask[15];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[16];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[24];

      } /* endif UD */

    else
      {

    if ( direction == NS )
      {
        /*  NS
          indices in "neighbours":
          6   7   8      3   4  5      0   1   2    y-1
         14  15  16     12     13      9  10  11     y
         22  23  24     20     21     17  18  19    y+1
         x-1  x x+1     x-1 x  x+1     x-1  x x+1
            z-1              z              z+1
        */

        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[ 7];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr              - size_xy ) )
	  neighbours |= long_mask[15];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[22];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[23];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[24];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr     - size_x           ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr + 1                    ) )
          neighbours |= long_mask[13];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[21];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[10];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[19];

      } /* endif NS */

    else
      {

    if ( direction == EW )
      {
        /*  EW
          indices in "neighbours":
          2  11  19     1  10  18      0   9  17    y-1
          5  13  21     4              3  12  20     y
          8  16  24     7  15  23      6  14  22    y+1
         x-1  x x+1    x-1  x x+1     x-1  x  x+1
            z-1             z              z+1
        */

        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
          neighbours |= long_mask[ 2];
        if ( *(image + act_addr     - size_x - size_xy ) )
          neighbours |= long_mask[11];
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
          neighbours |= long_mask[19];
        if ( *(image + act_addr - 1          - size_xy ) )
          neighbours |= long_mask[ 5];
        if ( *(image + act_addr              - size_xy ) )
	  neighbours |= long_mask[13];
        if ( *(image + act_addr + 1          - size_xy ) )
          neighbours |= long_mask[21];
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
          neighbours |= long_mask[ 8];
        if ( *(image + act_addr     + size_x - size_xy ) )
          neighbours |= long_mask[16];
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
          neighbours |= long_mask[24];
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
          neighbours |= long_mask[ 1];
        if ( *(image + act_addr     - size_x           ) )
	  neighbours |= long_mask[10];
        if ( *(image + act_addr + 1 - size_x           ) )
          neighbours |= long_mask[18];
        if ( *(image + act_addr - 1                    ) )
          neighbours |= long_mask[ 4];
        if ( *(image + act_addr - 1 + size_x           ) )
          neighbours |= long_mask[ 7];
        if ( *(image + act_addr     + size_x           ) )
	  neighbours |= long_mask[15];
        if ( *(image + act_addr + 1 + size_x           ) )
          neighbours |= long_mask[23];
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
          neighbours |= long_mask[ 0];
        if ( *(image + act_addr     - size_x + size_xy ) )
          neighbours |= long_mask[ 9];
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
          neighbours |= long_mask[17];
        if ( *(image + act_addr - 1          + size_xy ) )
          neighbours |= long_mask[ 3];
        if ( *(image + act_addr              + size_xy ) )
          neighbours |= long_mask[12];
        if ( *(image + act_addr + 1          + size_xy ) )
          neighbours |= long_mask[20];
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
          neighbours |= long_mask[ 6];
        if ( *(image + act_addr     + size_x + size_xy ) )
          neighbours |= long_mask[14];
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
          neighbours |= long_mask[22];

      } /* endif EW */
      } /* endelse */
      } /* endelse */

  }
/*========= end of function collect_26_neighbours =========*/


/*========= function deletable =========*/
int deletable3( void )
{
  collect_26_neighbours3();
  return ( ( *(lut_deletable + (neighbours>>3) ) ) & char_mask[neighbours%8]);
}
/*========= end of function deletable =========*/


/*=========== function UpdateSurfaceList ===========*/
void UpdateSurfaceList(PointList *r) {
ListElement *LE3;
ListElement *ptr;
unsigned char surface_end;

  LE3=(ListElement *)SurfaceVoxels.first;
  while (LE3!=NULL)
    {

      act_addr = (*LE3).addr;
      surface_end = 0;

      if (  ( *(image + act_addr - 1 ) == 0 ) &&
            ( *(image + act_addr + 1 ) == 0 )     )
        {
           surface_end = 1;
	}
      else
        {  /* 1 */
          if (  ( *(image + act_addr - size_x ) == 0 ) &&
                ( *(image + act_addr + size_x ) == 0 )     )
            {
               surface_end = 1;
	    }
          else
            {  /* 2*/
              if (  ( *(image + act_addr - size_xy ) == 0 ) &&
                    ( *(image + act_addr + size_xy ) == 0 )     )
                {
                   surface_end = 1;
	        }
	    }  	/* 2 */
	}  /* 1 */

      if ( surface_end  )
        AddToList(r,LE3);

      LE3=(ListElement *)(*LE3).next;
    } /* endwhile */

  while ( r->Length > 0 )
    {
       GetFromList( r, &ptr );
       RemoveSurfaceVoxel(ptr);
    } /* endwhile */

}
/*=========== end of function UpdateSurfaceList ===========*/


/*=========== function DetectDeletablePoints ===========*/
void DetectDeletablePoints3(PointList *s) {
ListElement * LE3;
unsigned char maybe;
PointList r;

  /* update suurface list */
    CreatePointList(&r);
    UpdateSurfaceList(&r);
    DestroyPointList(&r);


  LE3=(ListElement *)SurfaceVoxels.first;
  while (LE3!=NULL)
    {

      act_addr = (*LE3).addr;

      maybe = 0;
      if ( direction == UD )
        {
           if ( ( ( *(image + act_addr - size_xy           ) == 0 ) &&
	          ( *(image + act_addr + size_xy           )      ) &&
		  ( *(image + act_addr + size_xy + size_xy )      )     )
		                                                                  ||
		( ( *(image + act_addr + size_xy           ) == 0 ) &&
	          ( *(image + act_addr - size_xy           )      )     )    )
          maybe = 1;
	}
      else
        {
      if ( direction == NS )
        {
	   if ( ( ( *(image + act_addr - size_x            ) == 0 ) &&
	          ( *(image + act_addr + size_x            )      ) &&
		  ( *(image + act_addr + size_x + size_x   )      )     )
		                                                                  ||
		( ( *(image + act_addr + size_x            ) == 0 ) &&
	          ( *(image + act_addr - size_x            )      )     )    )
          maybe = 1;
	}
      else
        {
      if ( direction == EW )
        {
           if ( ( ( *(image + act_addr - 1                 ) == 0 ) &&
	          ( *(image + act_addr + 1                 )      ) &&
		  ( *(image + act_addr + 2                 )      )     )
		                                                                  ||
		( ( *(image + act_addr + 1                 ) == 0 ) &&
	          ( *(image + act_addr - 1                 )      )     )    )
          maybe = 1;
	}
	}  /* endelse */
	}  /* endelse */

      if ( maybe )
        {
          if ( deletable3() )
            {
               AddToList(s,LE3);
            } /* endif */
        } /* endif */

      LE3=(ListElement *)(*LE3).next;
    } /* endwhile */

}
/*=========== end of function DetectDeletablePoints ===========*/


/*========= function thinning_iteration_step =========*/
unsigned int thinning_iteration_step3(void)
{
  unsigned long int i, changed;
  ListElement * ptr;
  PointList s;
  unsigned long int  curr_addr;

  changed = 0;
  for ( i=0; i<3; i++ )   /* 3-subiteration alg. */
    {
      direction = i;
      CreatePointList(&s);
      DetectDeletablePoints3(&s);
      while ( s.Length > 0 )
        {
           curr_addr = GetFromList( &s, &ptr );
           *(image + curr_addr ) = 0;            /* deleted */
           changed ++;
           /* investigating v's 6-neighbours */
	      act_addr = curr_addr - 1;          /* (x-1,y,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr + 1;          /* (x+1,y,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr - size_x;     /* (x,y-1,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr + size_x;     /* (x,y+1,z) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr - size_xy;    /* (x,y,z-1) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
	      act_addr = curr_addr + size_xy;    /* (x,y,z+1) */
              if (*(image + act_addr )==1)
                {
                  NewSurfaceVoxel();
                  *(image + act_addr ) = 2;
                }
            RemoveSurfaceVoxel(ptr);
        } /* endwhile */
      DestroyPointList(&s);
    } /* endfor */

  return changed;
}
/*========= end of function thinning_iteration_step =========*/

/*========= function thinning =========*/
void thinning3(void)
{
  unsigned int iter, changed;

  CollectSurfaceVoxels();

  iter = 0;
  changed = 1;
  while ( changed )
    {
      changed = thinning_iteration_step3();
      iter++;
      printf("\n  iteration step: %3d.    (deleted point(s): %6d)",
             iter, changed );
    }

}
/*========= end of function thinning =========*/



int palagyi_skelpar_surf_02(struct xvimage *input)
{
	char tablefilename[512];
	pgmimage=input;
  /********************/
  /* READ INPUT IMAGE */
  /********************/
    read_image();

  /***************/
  /* READING LUT */
  /***************/
	sprintf(tablefilename, "%s/src/tables/palagyi_pardir_surf_02.dat", getenv("PINK"));
    init_lut(tablefilename);

  /************/
  /* THINNING */
  /************/
#ifdef VERBOSE
    printf("\n 3-subiteration directional surface-thinning ...");
#endif
    thinning3();

  /********************/
  /* WRITE OUPUT IMAGE */
  /********************/
    write_image( );

  /********/
  /* FREE */
  /********/
    free(lut_deletable);


    return(0);
}
/*===========================================================================
    end of function    m a i n
  ===========================================================================*/


void NewBorderVoxel( void ) {
ListElement * LE;
	LE=(ListElement *)malloc(sizeof(ListElement));
	(*LE).addr=act_addr;
	(*LE).next=NULL;
	(*LE).prev=BorderVoxels.last;
	if (BorderVoxels.last!=NULL) (*((ListElement*)(BorderVoxels.last))).next=LE;
	BorderVoxels.last=LE;
	if (BorderVoxels.first==NULL) BorderVoxels.first=LE;
}

void RemoveBorderVoxel(ListElement * LE) {
ListElement * LE2;
	if (BorderVoxels.first==LE) BorderVoxels.first=(*LE).next;
	if (BorderVoxels.last==LE) BorderVoxels.last=(*LE).prev;
	if ((*LE).next!=NULL) {
		LE2=(ListElement*)((*LE).next);
		(*LE2).prev=(*LE).prev;
	}
	if ((*LE).prev!=NULL) {
		LE2=(ListElement*)((*LE).prev);
		(*LE2).next=(*LE).next;
	}
	free(LE);
}

void CollectBorderVoxels(void) {
unsigned long int x,y,z, num;
unsigned long int z_size_xy, y_size_x;

  BorderVoxels.first = NULL;
  BorderVoxels.last  = NULL;
  num = 0;

  for( z=1, z_size_xy=size_xy;
       z<size_z-1;
       z++, z_size_xy+=size_xy )
    {
      for( y=1, y_size_x=size_x;
           y<size_y-1;
           y++, y_size_x+=size_x )
        {
          for(x=1; x<size_x-1; x++)
	    {
	      act_addr = x + y_size_x + z_size_xy;
              if ( *(image + act_addr ) )
                {
                  if (  ( *(image + act_addr     - size_x           ) ==0 ) ||
                        ( *(image + act_addr     + size_x           ) ==0 ) ||
                        ( *(image + act_addr              - size_xy ) ==0 ) ||
                        ( *(image + act_addr              + size_xy ) ==0 ) ||
                        ( *(image + act_addr + 1                    ) ==0 ) ||
                        ( *(image + act_addr - 1                    ) ==0 )    )
                     {
                        *(image + act_addr ) = 2;    /* Border point */
                        NewBorderVoxel();
		        num ++;
                     } /* endif */
                } /* endif */
            } /* endfor x */
        } /* endfor y */
    } /* endfor z */

  printf("\n Number of border points in the original image: %lu\n", num);
}


/*========= function collect_26_neighbours =========*/
void collect_26_neighbours4( void )
  {

    neighbours_UD = 0x00000000;
    neighbours_NS = 0x00000000;
    neighbours_EW = 0x00000000;

        /*

          indices in "neighbours_UD":
           0  1  2     9 10 11     17  18  19   y-1
           3  4  5    12    13     20   *  21   y
           6  7  8    14 15 16     22  23  24   y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1

          indices in "neighbours_NS":
           6  7  8     3  4  5      0   1   2    y-1
          14 15 16    12    13      9  10  11    y
          22 23 24    20  * 21     17  18  19    y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1

          indices in "neighbours_EW":
          17  9  0    18 10  1     19  11   2    y-1
          20 12  3     *     4     21  13   5    y
          22 14  6    23 15  7     24  10   8    y+1
          x-1 x x+1   x-1 x x+1    x-1  x  x+1
             z-1          z            z+1

        */

        /* face z-1 */
        if ( *(image + act_addr - 1 - size_x - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 0];
	     neighbours_NS |= long_mask[ 6];
	     neighbours_EW |= long_mask[17];
	  }
        if ( *(image + act_addr     - size_x - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 1];
	     neighbours_NS |= long_mask[ 7];
	     neighbours_EW |= long_mask[ 9];
	  }
        if ( *(image + act_addr + 1 - size_x - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 2];
	     neighbours_NS |= long_mask[ 8];
	     neighbours_EW |= long_mask[ 0];
	  }
        if ( *(image + act_addr - 1          - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 3];
	     neighbours_NS |= long_mask[14];
	     neighbours_EW |= long_mask[20];
	  }
        if ( *(image + act_addr              - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 4];
	     neighbours_NS |= long_mask[15];
	     neighbours_EW |= long_mask[12];
	  }
        if ( *(image + act_addr + 1          - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 5];
	     neighbours_NS |= long_mask[16];
	     neighbours_EW |= long_mask[ 3];
	  }
        if ( *(image + act_addr - 1 + size_x - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 6];
	     neighbours_NS |= long_mask[22];
	     neighbours_EW |= long_mask[22];
	  }
        if ( *(image + act_addr     + size_x - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 7];
	     neighbours_NS |= long_mask[23];
	     neighbours_EW |= long_mask[14];
	  }
        if ( *(image + act_addr + 1 + size_x - size_xy ) )
	  {
             neighbours_UD |= long_mask[ 8];
	     neighbours_NS |= long_mask[24];
	     neighbours_EW |= long_mask[ 6];
	  }
        /* face z */
        if ( *(image + act_addr - 1 - size_x           ) )
	  {
             neighbours_UD |= long_mask[ 9];
	     neighbours_NS |= long_mask[ 3];
	     neighbours_EW |= long_mask[18];
	  }
        if ( *(image + act_addr     - size_x           ) )
	  {
             neighbours_UD |= long_mask[10];
	     neighbours_NS |= long_mask[ 4];
	     neighbours_EW |= long_mask[10];
	  }
        if ( *(image + act_addr + 1 - size_x           ) )
	  {
             neighbours_UD |= long_mask[11];
	     neighbours_NS |= long_mask[ 5];
	     neighbours_EW |= long_mask[ 1];
	  }
        if ( *(image + act_addr - 1                    ) )
	  {
             neighbours_UD |= long_mask[12];
	     neighbours_NS |= long_mask[12];
	   /*neighbours_EW  not considered*/
	  }
        if ( *(image + act_addr + 1                    ) )
	  {
             neighbours_UD |= long_mask[13];
	     neighbours_NS |= long_mask[13];
	     neighbours_EW |= long_mask[ 4];
	  }
        if ( *(image + act_addr - 1 + size_x           ) )
	  {
             neighbours_UD |= long_mask[14];
	     neighbours_NS |= long_mask[20];
	     neighbours_EW |= long_mask[23];
	  }
        if ( *(image + act_addr     + size_x           ) )
	  {
             neighbours_UD |= long_mask[15];
	   /*neighbours_NS not considered*/
	     neighbours_EW |= long_mask[15];
	  }
        if ( *(image + act_addr + 1 + size_x           ) )
	  {
             neighbours_UD |= long_mask[16];
	     neighbours_NS |= long_mask[21];
	     neighbours_EW |= long_mask[ 7];
	  }
        /* face z+1 */
        if ( *(image + act_addr - 1 - size_x + size_xy ) )
	  {
             neighbours_UD |= long_mask[17];
	     neighbours_NS |= long_mask[ 0];
	     neighbours_EW |= long_mask[19];
	  }
        if ( *(image + act_addr     - size_x + size_xy ) )
	  {
             neighbours_UD |= long_mask[18];
	     neighbours_NS |= long_mask[ 1];
	     neighbours_EW |= long_mask[11];
	  }
        if ( *(image + act_addr + 1 - size_x + size_xy ) )
	  {
             neighbours_UD |= long_mask[19];
	     neighbours_NS |= long_mask[ 2];
	     neighbours_EW |= long_mask[ 2];
	  }
        if ( *(image + act_addr - 1          + size_xy ) )
	  {
             neighbours_UD |= long_mask[20];
	     neighbours_NS |= long_mask[ 9];
	     neighbours_EW |= long_mask[21];
	  }
        if ( *(image + act_addr              + size_xy ) )
	  {
           /*neighbours_UD not considered*/
	     neighbours_NS |= long_mask[10];
	     neighbours_EW |= long_mask[13];
	  }
        if ( *(image + act_addr + 1          + size_xy ) )
	  {
             neighbours_UD |= long_mask[21];
	     neighbours_NS |= long_mask[11];
	     neighbours_EW |= long_mask[ 5];
	  }
        if ( *(image + act_addr - 1 + size_x + size_xy ) )
	  {
             neighbours_UD |= long_mask[22];
	     neighbours_NS |= long_mask[17];
	     neighbours_EW |= long_mask[24];
	  }
        if ( *(image + act_addr     + size_x + size_xy ) )
	  {
             neighbours_UD |= long_mask[23];
	     neighbours_NS |= long_mask[18];
	     neighbours_EW |= long_mask[16];
	  }
        if ( *(image + act_addr + 1 + size_x + size_xy ) )
	  {
             neighbours_UD |= long_mask[24];
	     neighbours_NS |= long_mask[19];
	     neighbours_EW |= long_mask[ 8];
	  }

  }
/*========= end of function collect_26_neighbours =========*/


/*========= function deletable =========*/
int deletable4( void )
{
  collect_26_neighbours4();


  /* UD test */
    if  (  ( (
               ( *(image + act_addr              - size_xy )==0 )
               &&
               ( *(image + act_addr              + size_xy )==1 )
	     )
	     ||
	     (
	       ( *(image + act_addr              - size_xy )==1 )
	       &&
               ( *(image + act_addr              + size_xy )==0 )
	     )
	   )
	   &&
           (
	     ( *(lut_deletable + (neighbours_UD>>3) ) ) & char_mask[neighbours_UD%8]
	   )
	 )
      return ( 1 );
  /* NS test */
    if  (  ( (
               ( *(image + act_addr     - size_x           )==0 )
               &&
               ( *(image + act_addr     + size_x           )==1 )
	     )
	     ||
	     (
	       ( *(image + act_addr     - size_x           )==1 )
	       &&
               ( *(image + act_addr     + size_x           )==0 )
	     )
	   )
	   &&
           (
	     ( *(lut_deletable + (neighbours_NS>>3) ) ) & char_mask[neighbours_NS%8]
	   )
	 )
      return ( 1 );
  /* EW test */
    if  (  ( (
               ( *(image + act_addr - 1                    )==0 )
               &&
               ( *(image + act_addr + 1                    )==1 )
	     )
	     ||
	     (
	       ( *(image + act_addr - 1                    )==1 )
	       &&
               ( *(image + act_addr + 1                    )==0 )
	     )
	   )
	   &&
           (
	     ( *(lut_deletable + (neighbours_EW>>3) ) ) & char_mask[neighbours_EW%8]
	   )
	 )
      return ( 1 );

   return ( 0 );
}

/*=========== function UpdateSurfaceList ===========*/
void UpdateSurfaceList4(PointList *r) {
ListElement *LE3;
ListElement *ptr;
unsigned char int_nei_exists;

  LE3=(ListElement *)BorderVoxels.first;
  while (LE3!=NULL)
    {

      act_addr = (*LE3).addr;
      int_nei_exists = 0;

      if ( *(image + act_addr - 1 ) == 1 )
        {
           int_nei_exists = 1;
	}
      else
        {  /* 1 */
          if ( *(image + act_addr + 1 ) == 1 )
            {
              int_nei_exists = 1;
	    }
          else
            {  /* 2*/
              if ( *(image + act_addr - size_x ) == 1 )
                {
                  int_nei_exists = 1;
	        }
              else
                {  /* 3 */
                  if ( *(image + act_addr + size_x ) == 1 )
                    {
                      int_nei_exists = 1;
   	            }
                  else
                    {  /* 4 */
                      if ( *(image + act_addr - size_xy ) == 1 )
                        {
                          int_nei_exists = 1;
   	                }
                      else
                        {  /* 5 */
                          if ( *(image + act_addr + size_xy ) == 1 )
                            {
                              int_nei_exists = 1;
   	                    }
	                }  /* 5 */
	            }   /* 4 */
		}   /* 3 */
	    }  	/* 2 */
	}  /* 1 */

      if ( int_nei_exists == 0 )
        AddToList(r,LE3);

      LE3=(ListElement *)(*LE3).next;
    } /* endwhile */

  while ( r->Length > 0 )
    {
       GetFromList( r, &ptr );
       RemoveBorderVoxel(ptr);
    } /* endwhile */

}
/*=========== end of function UpdateSurfaceList ===========*/


/*=========== function DetectDeletablePoints ===========*/
void DetectDeletablePoints4(PointList *s) {
ListElement * LE3;
PointList r;

  /* update suurface list */
    CreatePointList(&r);
    UpdateSurfaceList4(&r);
    DestroyPointList(&r);

  LE3=(ListElement *)BorderVoxels.first;

  while (LE3!=NULL)
    {
      act_addr = (*LE3).addr;

      if ( deletable4() )
        AddToList(s,LE3);

      LE3=(ListElement *)(*LE3).next;

    } /* endwhile */

}
/*=========== end of function DetectDeletablePoints ===========*/


/*========= function thinning_iteration_step =========*/
unsigned int thinning_iteration_step4(void)
{
  unsigned long int changed;
  ListElement * ptr;
  PointList s;
  unsigned long int  curr_addr;

  changed = 0;
  CreatePointList(&s);
  DetectDeletablePoints4(&s);

  while ( s.Length > 0 )
    {
       curr_addr = GetFromList( &s, &ptr );
       *(image + curr_addr ) = 0;            /* deleted */
       changed ++;
       /* investigating v's 6-neighbours */
	  act_addr = curr_addr - 1;          /* (x-1,y,z) */
          if (*(image + act_addr )==1)
            {
              NewBorderVoxel();
              *(image + act_addr ) = 2;
            }
	  act_addr = curr_addr + 1;          /* (x+1,y,z) */
          if (*(image + act_addr )==1)
            {
              NewBorderVoxel();
              *(image + act_addr ) = 2;
            }
	  act_addr = curr_addr - size_x;     /* (x,y-1,z) */
          if (*(image + act_addr )==1)
            {
              NewBorderVoxel();
              *(image + act_addr ) = 2;
            }
          act_addr = curr_addr + size_x;     /* (x,y+1,z) */
          if (*(image + act_addr )==1)
            {
              NewBorderVoxel();
              *(image + act_addr ) = 2;
            }
          act_addr = curr_addr - size_xy;    /* (x,y,z-1) */
          if (*(image + act_addr )==1)
            {
              NewBorderVoxel();
              *(image + act_addr ) = 2;
            }
	  act_addr = curr_addr + size_xy;    /* (x,y,z+1) */
          if (*(image + act_addr )==1)
            {
              NewBorderVoxel();
              *(image + act_addr ) = 2;
            }
        RemoveBorderVoxel(ptr);
    } /* endwhile */

  DestroyPointList(&s);

  return changed;
}
/*========= end of function thinning_iteration_step =========*/

/*========= function thinning =========*/
void thinning4(void)
{
  unsigned int iter, changed;

  CollectBorderVoxels();

  iter = 0;
  changed = 1;
  while ( changed )
    {
      changed = thinning_iteration_step4();
      iter++;
      printf("\n  iteration step: %3d.    (deleted point(s): %6d)",
             iter, changed );
    }

}
/*========= end of function thinning =========*/


int palagyi_skelpar_surf_08(struct xvimage *input)
{
	char tablefilename[512];
	pgmimage=input;
  /********************/
  /* READ INPUT IMAGE */
  /********************/
    read_image();

  /***************/
  /* READING LUT */
  /***************/
	sprintf(tablefilename, "%s/src/tables/palagyi_pardir_surf_08.dat", getenv("PINK"));
    init_lut(tablefilename);

  /************/
  /* THINNING */
  /************/
#ifdef VERBOSE
    printf("\n fully parallel surface-thinning ...");
#endif
    thinning4();

  /********************/
  /* WRITE OUPUT IMAGE */
  /********************/
    write_image( );

  /********/
  /* FREE */
  /********/
    free(lut_deletable);


    return(0);
}
