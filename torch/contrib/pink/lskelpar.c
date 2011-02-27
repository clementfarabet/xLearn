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
   Algorithmes 2D "fully parallel" de squelettisation :

   - Pavlidis
   - Eckhardt
   - Couprie ultime
   - Couprie curviligne
   - Couprie curviligne avec reconstruction
   - Rutovitz
   - Zhang & Wang
   - Han & La & Rhee (merdique et peu int32_téressant)
   - Guo & Hall
   - Chin & Wan & Stover & Iverson
   - Jang & Chin
   - Jang & Chin (mc correction)
   - MNS_preservation
   - Jang & Chin rec
   - Choy & Choy & Siu
   - Manzanera and Bernard
   - Manzanera and Bernard (variante gb)
   - Hall
   - Wu and Tsai
   - Manzanera and Bernard (variant by GB)
   - Couprie and Bertrand, ultimate with medial axis constraint (NK2)
   - Bertrand curvilinear with reconstruction, asymmetrical
   - Bertrand curvilinear with reconstruction, symmetrical
   - Rosenfeld directionnel
   - Nemeth et Palagyi, 2009 (1)
   - Nemeth et Palagyi, 2009 (2)
   - Nemeth et Palagyi, 2009 (3)
   - Couprie and Bertrand, ultimate, asymmetrical (NK2b)

   Couprie ultime:
     input/output F
     repeat until stability
       for all x do if simple8(x) then F[x] = 2
       for all x do if (m1(x) v m2(x) v m3(x) v m4(x) v m5(x) v m6(x)) then T[x] = 1
       for all x do
         if (F[x]=2) then
           if not T[x] then F[x] = 0 else F[x] = 1

     m1 : match   A 2 B   avec origine = (1,0) et [au moins un des A et au moins un des B non nuls]
                  A 2 B   ou [A et B tous nuls] (+ rotation 90)

     m2 : match   x 0 0 x   avec origine = (1,2) et x = don't care
                  0 2 2 0   (+ rotations 90, 180, 270)
		  0 2 2 0
		  x 0 0 x

     m3: match   x 0 0 x   avec origine = (1,2) et x = don't care
                 0 2 2 x   (+ rotations 90, 180, 270)
	         0 2 0 x
		 x x x x

     m4: match   x x x x   avec origine = (1,2) et x = don't care
                 x 0 2 0   (+ rotations 90, 180, 270)
		 x 2 2 0
		 x 0 0 x

     m5: match   x x x x   avec origine = (1,2) et x = don't care
                 0 2 0 x   (+ rotations 90, 180, 270)
		 0 2 2 x
		 x 0 0 x

     m6 : match   2 0      avec origine = (1,1)
                  0 2      (+ rotations 90, 180, 270)

   Couprie curviligne:
     input/output F
     repeat until stability
       for all x do if simple8(x) and not end(x) then F[x] = 2
       for all x do if (m1(x) v m2(x) v m3(x) v m4(x) v m5(x) v m6(x)) then T[x] = 1
       for all x do
         if (F[x]=2) then
           if not T[x] then F[x] = 0 else F[x] = 1

     end : match   0 0 x   ou    0 x 1    avec origine = (1,1) et x = don't care
                   0 1 1         0 1 x
                   0 0 x         0 0 0

     VARIANTE:

     end2: match   0 0 p   ou    0 x 1    avec origine = (1,1) et x = don't care
                   0 1 1         0 1 x    et not (p = 1 and q = 1)
                   0 0 q         0 0 0

     end2: match   0 0 0   ou    0 x 1   ou   0 1 x   avec origine = (1,1) et x = don't care
                   0 1 1         0 1 x        0 1 1
                   0 0 0         0 0 0        0 0 0

   Couprie curviligne avec reconstruction:
     input/output X
     Y = emptyset
     repeat until stability
       I = Interior(X \ Y)
       D = Dilat(I) Inter [X \ Y]
       E = Y Union [X \ D]
       for all x do if simple8(x) and x not in E then X[x] = 2
       for all x do if (m1(x) v m2(x) v m3(x) v m4(x) v m5(x) v m6(x)) then T[x] = 1
       for all x do
         if (X[x]=2) then
           if not T[x] then X[x] = 0 else X[x] = 1
       Y = X \ I

   MNS_preservation:
     input/output F
     repeat until stability
       for all x do if simple8(x) then F[x] = 2
       for all x do if (m1(x) v m2(x) v m3(x) v m4(x) v m5(x) v m6(x)) then T[x] = 1
       for all x do
         if (F[x]=2) then
           if not T[x] then F[x] = 0 else F[x] = 1

     m1 : match   A 2 B   avec origine = (1,0) et [au moins un des A et au moins un des B non nuls]
                  A 2 B   ou [A et B tous nuls] (+ rotation 90)

		  (preuve équivalence par programme : Squel2D/ronse.c)

     m2 : match   0 0 0 0   avec origine = (1,2) et x = don't care
                  0 2 2 0   (+ rotations 90, 180, 270)
		  0 2 2 0
		  0 0 0 0

     m3 : match   0 0 0 0   avec origine = (1,2) et x = don't care
                  0 2 2 0   (+ rotations 90, 180, 270)
		  0 2 0 0
		  0 0 0 x

     m4 : match   x 0 0 0   avec origine = (1,2) et x = don't care
                  0 0 2 0   (+ rotations 90, 180, 270)
		  0 2 2 0
		  0 0 0 0

     m5 : match   0 0 0 x   avec origine = (1,2) et x = don't care
                  0 2 0 0   (+ rotations 90, 180, 270)
		  0 2 2 0
		  0 0 0 0

		  (preuve suffisant papier de Ronse)

     m6 : match   x 0 0 0   avec origine = (1,2) et x = don't care
                  0 0 2 0   (+ rotations 90, 180, 270)
                  0 2 0 0
		  0 0 0 x

		  (preuve suffisant par programme : Squel2D/ronse.c)

   Michel Couprie - juillet 2001
   Benjamin Raynal 2010 - algos Nemeth & Palagyi
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mcutil.h>
#include <lskelpar.h>

//#define DEBUG_BERTRAND
//#define DEBUG_PAV
//#define DEBUG_MC
//#define VERBOSE1
//#define ETUDE
//#define ANIMATE

static int32_t jangrec_match23(uint8_t *F, int32_t x, int32_t rs, int32_t N);
static int32_t jang_match27b(uint8_t *F, int32_t x, int32_t rs, int32_t N);
static int32_t jang_match28b(uint8_t *F, int32_t x, int32_t rs, int32_t N);


/* ==================================== */
void extract_vois(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N,                       /* taille image */
  uint8_t *vois)
/*
  retourne dans "vois" les valeurs des 8 voisins de p, dans l'ordre suivant:

		3	2	1
		4	p	0
		5	6	7
  le point p ne doit pas être un point de bord de l'image
*/
/* ==================================== */
{
#undef F_NAME
#define F_NAME "extract_vois"
  register uint8_t * ptr = img+p;
  if ((p%rs==rs-1) || (p<rs) || (p%rs==0) || (p>=N-rs)) /* point de bord */
  {
    printf("%s: ERREUR: point de bord\n", F_NAME);
    exit(0);
  }
  vois[0] = *(ptr+1);
  vois[1] = *(ptr+1-rs);
  vois[2] = *(ptr-rs);
  vois[3] = *(ptr-rs-1);
  vois[4] = *(ptr-1);
  vois[5] = *(ptr-1+rs);
  vois[6] = *(ptr+rs);
  vois[7] = *(ptr+rs+1);
} /* extract_vois() */

/* ==================================== */
void print_vois(uint8_t *vois)
/*
   affiche vois (debug)
*/
/* ==================================== */
{
  printf("vois = %d %d %d %d %d %d %d %d\n",
	 vois[0], vois[1], vois[2], vois[3], vois[4], vois[5], vois[6], vois[7]);
} /* print_vois() */

/* ==================================== */
void rotate45_vois(uint8_t *vois)
/*
   effectue une rotation du voisinage "vois" de 45 degres dans le sens
   trigonométrique
*/
/* ==================================== */
{
  uint8_t tmp = vois[0];
  vois[0] = vois[7]; vois[7] = vois[6]; vois[6] = vois[5]; vois[5] = vois[4];
  vois[4] = vois[3]; vois[3] = vois[2]; vois[2] = vois[1]; vois[1] = tmp;
} /* rotate45_vois() */

/* ==================================== */
void rotate90_vois(uint8_t *vois)
/*
   effectue une rotation du voisinage "vois" de 90 degres dans le sens
   trigonométrique
*/
/* ==================================== */
{
  rotate45_vois(vois); rotate45_vois(vois);
} /* rotate90_vois() */

/* ==================================== */
int32_t extract_vois2(
  uint8_t *img,          /* pointeur base image */
  int32_t p,                       /* index du point */
  int32_t rs,                      /* taille rangee */
  int32_t N,                       /* taille image */
  uint8_t *vois)
/*
  retourne dans "vois" les valeurs des 16 voisins d'ordre 2 de p, dans l'ordre suivant:

		6   5   4   3   2
		7   X   X   X   1
		8   X   p   X   0
		9   X   X   X   15
		10  11  12  13  14
  le point p doit être à une distance minimale de 2 du bord de l'image
*/
/* ==================================== */
{
#undef F_NAME
#define F_NAME "extract_vois2"
  register uint8_t * ptr = img+p;
  register int32_t rs2 = rs + rs;
  if ((p%rs>=rs-2) || (p<rs2) || (p%rs<2) || (p>=N-rs2)) /* point de bord */
  {
#ifdef PARANO
    printf("%s: ERREUR: point de bord\n", F_NAME);
    exit(0);
#else
    return 0;
#endif
  }
  vois[0] = *(ptr+2);
  vois[1] = *(ptr+2-rs);
  vois[2] = *(ptr+2-rs2);
  vois[3] = *(ptr+1-rs2);
  vois[4] = *(ptr-rs2);
  vois[5] = *(ptr-1-rs2);
  vois[6] = *(ptr-2-rs2);
  vois[7] = *(ptr-2-rs);
  vois[8] = *(ptr-2);
  vois[9] = *(ptr-2+rs);
  vois[10] = *(ptr-2+rs2);
  vois[11] = *(ptr-1+rs2);
  vois[12] = *(ptr+rs2);
  vois[13] = *(ptr+1+rs2);
  vois[14] = *(ptr+2+rs2);
  vois[15] = *(ptr+2+rs);
  return 1;
} /* extract_vois2() */

/* ==================================== */
void rotate90_vois2(uint8_t *vois)
/*
   effectue une rotation du voisinage d'ordre 2 "vois" de 90 degres dans le sens
   trigonométrique
*/
/* ==================================== */
{
  uint8_t tmp;
  tmp = vois[0]; vois[0] = vois[12]; vois[12] = vois[8]; vois[8] = vois[4]; vois[4] = tmp;
  tmp = vois[1]; vois[1] = vois[13]; vois[13] = vois[9]; vois[9] = vois[5]; vois[5] = tmp;
  tmp = vois[2]; vois[2] = vois[14]; vois[14] = vois[10]; vois[10] = vois[6]; vois[6] = tmp;
  tmp = vois[3]; vois[3] = vois[15]; vois[15] = vois[11]; vois[11] = vois[7]; vois[7] = tmp;
} /* rotate90_vois2() */

/* ==================================== */
static int32_t pav_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  A A A   avec origine = (1,1) et au moins un des A et au moins un des B non nuls
  0 P 0
  B B B
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 0) goto fail;
    if ((v[1] == 0) && (v[2] == 0) && (v[3] == 0)) goto fail;
    if (v[4] != 0) goto fail;
    if ((v[5] == 0) && (v[6] == 0) && (v[7] == 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* pav_match1() */

/* ==================================== */
static int32_t pav_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  D D D     avec origine = (1,1) et valeurs des pixels D quelconques
  D P 0
  D 0 2
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 0) goto fail;
    //    if ((v[1] == 0) && (v[2] == 0) && (v[3] == 0) && (v[4] == 0) && (v[5] == 0)) goto fail;
    if (v[6] != 0) goto fail;
    if (v[7] != 2) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* pav_match2() */

/* ==================================== */
static int32_t pav_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
// condition for "tentatively multiple" points
/*
  A A C   avec origine = (1,1) et au moins un des A et au moins un des B
  0 2 2   et au moins un des C non nuls - de plus si les deux C sont non
  B B C   nuls, alors les pixels A et B peuvent être quelconques
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if ((v[0] != 1) && (v[1] != 1) && (v[2] != 1) && (v[3] != 1) &&
      (v[4] != 1) && (v[5] != 1) && (v[6] != 1) && (v[7] != 1)) return 1;
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if (v[4] != 0) goto fail;
    if ((v[1] == 0) && (v[7] == 0)) goto fail;
    if ((v[1] != 0) && (v[7] != 0)) return 1;
    if ((v[2] == 0) && (v[3] == 0)) goto fail;
    if ((v[5] == 0) && (v[6] == 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* pav_match3() */

#ifdef NOT_USED
/* ==================================== */
static int32_t pav_match3b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
// condition for "tentatively multiple" points
/*
  A A C   avec origine = (1,1) et au moins un des A OU au moins un des B OU au moins un des C non nuls
  0 2 2
  B B C
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if ((v[0] != 1) && (v[1] != 1) && (v[2] != 1) && (v[3] != 1) &&
      (v[4] != 1) && (v[5] != 1) && (v[6] != 1) && (v[7] != 1)) return 1;
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if (v[4] != 0) goto fail;
    if ((v[1] != 0) || (v[7] != 0)) return 1;
    if ((v[2] != 0) || (v[3] != 0)) return 1;
    if ((v[5] != 0) || (v[6] != 0)) return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* pav_match3b() */
#endif

/* ==================================== */
static int32_t pav_match4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
// condition for "corner" points
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if ((v[1] != 0) || (v[2] != 0) || (v[3] != 0) ||
	(v[4] != 0) || (v[5] != 0)) goto fail;
    if (v[6] != 2) goto fail;
    if (v[7] == 0) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* pav_match4() */

/* ==================================== */
static int32_t pav_multiple(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t m1 = pav_match1(F, x, rs, N);
  int32_t m2 = pav_match2(F, x, rs, N);
  int32_t m3 = pav_match3(F, x, rs, N);
  int32_t m4 = pav_match4(F, x, rs, N);
#ifdef DEBUG_PAV
  if (m1 || m2 || m3 || m4)
    printf("point %d,%d : m1 = %d; m2 = %d; m3 = %d; m4 = %d\n",
	   x % rs, x / rs, m1, m2, m3, m4);
#endif
  return m1 || m2 || m3 || m4;
} /* pav_multiple() */

/* ==================================== */
int32_t lskelpavlidis(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
// T. Pavlidis : "A flexible parallel thinning algorithm"
// see also T. Pavlidis : "An asynchronous thinning algorithm - CGIP 1982"
#undef F_NAME
#define F_NAME "lskelpavlidis"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  while (step < nsteps)
  {
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++) if (F[i] && bordext4(F, i, rs, N)) F[i] = 2;
    for (i = 0; i < N; i++) if ((F[i] == 2) && pav_multiple(F, i, rs, N)) T[i] = 1;
#ifdef DEBUG_PAV
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif
    for (i = 0; i < N; i++) if ((F[i] == 2) && !T[i]) break;
    if (i == N) break;
    for (i = 0; i < N; i++) if ((F[i] == 1) || T[i]) F[i] = 1; else F[i] = 0;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelpavlidis() */

/* ==================================== */
int32_t lskelpavlidis1(struct xvimage *image,
		  int32_t pixel)
/* ==================================== */
// T. Pavlidis : "A flexible parallel thinning algorithm"
// see also T. Pavlidis : "An asynchronous thinning algorithm - CGIP 1982"
// Une étape - un seul point testé
#undef F_NAME
#define F_NAME "lskelpavlidis1"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  memset(T, 0, N);
  for (i = 0; i < N; i++) if (F[i] && bordext4(F, i, rs, N)) F[i] = 2;
  for (i = 0; i < N; i++) if ((F[i] == 2) && pav_multiple(F, i, rs, N)) T[i] = 1;
  if ((F[pixel] == 2) && !T[pixel]) F[pixel] = 0;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelpavlidis1() */

/* ==================================== */
static int32_t eck_perfect(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i = x % rs;
  int32_t j = x / rs;
  if ((F[j*rs + i-1] == 2) && (F[j*rs + i+1] == 0)) return 1;
  if ((F[j*rs + i+1] == 2) && (F[j*rs + i-1] == 0)) return 1;
  if ((F[(j-1)*rs + i] == 2) && (F[(j+1)*rs + i] == 0)) return 1;
  if ((F[(j+1)*rs + i] == 2) && (F[(j-1)*rs + i] == 0)) return 1;
  return 0;
} /* eck_perfect() */

/* ==================================== */
int32_t lskeleckhardt(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
// Eckhardt and Maderlechner : "Invariant thinning"
#undef F_NAME
#define F_NAME "lskeleckhardt"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++) if (F[i] && nbvois4(F, i, rs, N) == 4) F[i] = 2;
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && eck_perfect(F, i, rs, N))
      {
	nonstab = 1;
	T[i] = 1;
      }
#ifdef DEBUG_ECK
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif
    for (i = 0; i < N; i++)
      if (T[i] == 1) F[i] = 0;
      else if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskeleckhardt() */

/* ==================================== */
static int32_t crossing_nb(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
// retourne le nombre de transitions 1 -> 0 dans la liste ordonnée et cyclique des voisins
{
  int32_t i, n = 0;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 8; i++) if (v[i] && !v[(i+1)%8]) n++;
  return n;
} /* crossing_nb() */

/* ==================================== */
static int32_t rutovitz_match(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "rutovitz_match"
  int32_t n;
  uint8_t v[8];
  if (!F[x]) return 0;
  n = nbvois8(F, x, rs, N);
  //  if ((n < 2) || (n > 6)) return 0; // cond. donnée par Zhang et Wang
  if (n < 2) return 0;
  if (crossing_nb(F, x, rs, N) != 1) return 0;
  extract_vois(F, x, rs, N, v);
  if ((x < rs) || (x%rs == rs-1))
  {
    fprintf(stderr, "%s: object must not hit the frame\n", F_NAME);
    exit(0);
  }
  n = crossing_nb(F, x-rs, rs, N);
  if ((n == 1) && v[2] && v[0] && v[4]) return 0;
  n = crossing_nb(F, x+1, rs, N);
  if ((n == 1) && v[2] && v[0] && v[6]) return 0;
  return 1;
} /* rutovitz_match() */

/* ==================================== */
int32_t lskelrutovitz(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
// described in Zhang & Wang : "A modified parallel thinning algorithm"
#undef F_NAME
#define F_NAME "lskelrutovitz"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (rutovitz_match(F, i, rs, N))
	//if (rutovitz_match(F, i, rs, N) && !jangrec_match23(F, i, rs, N)) // correction mc
      {
	nonstab = 1;
	T[i] = 1;
      }
    for (i = 0; i < N; i++)
      if (T[i] == 1) F[i] = 0;
      else if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelrutovitz() */

/* ==================================== */
static int32_t zhangwang_match(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "zhangwang_match"
  int32_t n;
  uint8_t v[8];
  if (!F[x]) return 0;
  n = nbvois8(F, x, rs, N);
  if ((n < 2) || (n > 6)) return 0;
  if (crossing_nb(F, x, rs, N) != 1) return 0;
  extract_vois(F, x, rs, N, v);
  if ((x < rs+rs) || (x%rs >= rs-2))
  {
    fprintf(stderr, "%s: object must not hit the 2 pixel-thick frame\n", F_NAME);
    exit(0);
  }
  if ((F[x-(rs+rs)] == 0) && v[2] && v[0] && v[4]) return 0;
  if ((F[x+2]       == 0) && v[2] && v[0] && v[6]) return 0;
  return 1;
} /* zhangwang_match() */

/* ==================================== */
int32_t lskelzhangwang(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
// described in Zhang & Wang : "A modified parallel thinning algorithm"
#undef F_NAME
#define F_NAME "lskelzhangwang"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (zhangwang_match(F, i, rs, N))
	//if (zhangwang_match(F, i, rs, N) && !jangrec_match23(F, i, rs, N)) // correction mc
      {
	nonstab = 1;
	T[i] = 1;
      }
    for (i = 0; i < N; i++)
      if (T[i] == 1) F[i] = 0;
      else if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelzhangwang() */

/* ==================================== */
static int32_t hanlarhee_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);

  // at least one neighbor must be greater than or equal to 3
  for (i = 0; i < 8; i++) if (v[i] >= 3) break;
  if (i == 8) return 0;

  for (i = 0; i < 4; i++)
  {
    if ((v[0])&&(v[1])&&(!v[2])&&(!v[3])&&(!v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  for (i = 0; i < 4; i++)
  {
    if ((!v[0])&&(v[1])&&(v[2])&&(!v[3])&&(!v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  for (i = 0; i < 4; i++)
  {
    if ((v[0])&&(!v[1])&&(v[2])&&(!v[3])&&(!v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  return 0;
} /* hanlarhee_match2() */

/* ==================================== */
static int32_t hanlarhee_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);

  // at least one neighbor must be greater than or equal to 7
  for (i = 0; i < 8; i++) if (v[i] >= 7) break;
  if (i == 8) return 0;

  for (i = 0; i < 4; i++)
  {
    if ((v[0])&&(v[1])&&(v[2])&&(!v[3])&&(!v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  for (i = 0; i < 4; i++)
  {
    if ((!v[0])&&(v[1])&&(v[2])&&(v[3])&&(!v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  if ((v[0])&&(!v[1])&&(!v[2])&&(!v[3])&&(!v[4])&&(v[5])&&(v[6])&&(!v[7])) return 1;
  if ((!v[0])&&(!v[1])&&(!v[2])&&(v[3])&&(v[4])&&(!v[5])&&(v[6])&&(!v[7])) return 1;
  if ((v[0])&&(v[1])&&(!v[2])&&(!v[3])&&(!v[4])&&(!v[5])&&(v[6])&&(!v[7])) return 1;
  if ((!v[0])&&(!v[1])&&(!v[2])&&(!v[3])&&(v[4])&&(!v[5])&&(v[6])&&(v[7])) return 1;

  return 0;
} /* hanlarhee_match3() */

/* ==================================== */
static int32_t hanlarhee_match4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);

  for (i = 0; i < 4; i++)
  {
    if ((v[0])&&(v[1])&&(v[2])&&(v[3])&&(!v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  for (i = 0; i < 4; i++)
  {
    if ((!v[0])&&(v[1])&&(v[2])&&(v[3])&&(v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  if ((v[0])&&(v[1])&&(!v[2])&&(!v[3])&&(!v[4])&&(v[5])&&(v[6])&&(!v[7])) return 1;
  if ((!v[0])&&(!v[1])&&(!v[2])&&(v[3])&&(v[4])&&(!v[5])&&(v[6])&&(v[7])) return 1;

  return 0;
} /* hanlarhee_match4() */

/* ==================================== */
static int32_t hanlarhee_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);

  // at least one neighbor must be equal to 8
  for (i = 0; i < 8; i++) if (v[i] == 8) break;
  if (i == 8) return 0;

  for (i = 0; i < 4; i++)
  {
    if ((v[0])&&(v[1])&&(v[2])&&(v[3])&&(v[4])&&(!v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  for (i = 0; i < 4; i++)
  {
    if ((!v[0])&&(v[1])&&(v[2])&&(v[3])&&(v[4])&&(v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  return 0;
} /* hanlarhee_match5() */

/* ==================================== */
static int32_t hanlarhee_match6(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);

  // at least one neighbor must be equal to 8
  for (i = 0; i < 8; i++) if (v[i] == 8) break;
  if (i == 8) return 0;

  for (i = 0; i < 4; i++)
  {
    if ((v[0])&&(v[1])&&(v[2])&&(v[3])&&(v[4])&&(v[5])&&(!v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  for (i = 0; i < 4; i++)
  {
    if ((!v[0])&&(v[1])&&(v[2])&&(v[3])&&(v[4])&&(v[5])&&(v[6])&&(!v[7])) return 1;
    rotate90_vois(v);
  }

  return 0;
} /* hanlarhee_match6() */

/* ==================================== */
static int32_t hanlarhee_match7(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i, n;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);

  // at least two neighbors must be equal to 8
  n = 0;
  for (i = 0; i < 8; i++) if (v[i] == 8) n++;
  if (n < 2) return 0;

  for (i = 0; i < 4; i++)
  {
    if ((!v[0])&&(v[1])&&(v[2])&&(v[3])&&(v[4])&&(v[5])&&(v[6])&&(v[7])) return 1;
    rotate90_vois(v);
  }

  return 0;
} /* hanlarhee_match7() */

/* ==================================== */
static int32_t hanlarhee_match(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  switch (F[x])
  {
  case 0: return 0;
  case 1:
    extract_vois(F, x, rs, N, v);
    for (i = 0; i < 8; i++)
      if (v[i] >= 3) return 1;
    return 0;
  case 2: return hanlarhee_match2(F, x, rs, N);
  case 3: return hanlarhee_match3(F, x, rs, N);
  case 4: return hanlarhee_match4(F, x, rs, N);
  case 5: return hanlarhee_match5(F, x, rs, N);
  case 6: return hanlarhee_match6(F, x, rs, N);
  case 7: return hanlarhee_match7(F, x, rs, N);
  case 8: return 0;
  }
  assert(1); exit(1);
} /* hanlarhee_match() */

/* ==================================== */
int32_t lskelhanlarhee(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
// described in Han, La & Rhee : "An efficient fully parallel algorithm"
#undef F_NAME
#define F_NAME "lskelhanlarhee"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  struct xvimage *nbn = copyimage(image);
  uint8_t *B = UCHARDATA(nbn);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++) if (F[i]) B[i] = nbvois8(F, i, rs, N);
    for (i = 0; i < N; i++)
      if (F[i] && hanlarhee_match(B, i, rs, N))
      {
	nonstab = 1;
	T[i] = 1;
      }
    for (i = 0; i < N; i++)
      if (T[i] == 1) F[i] = 0;
      else if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  freeimage(nbn);
  return(1);
} /* lskelhanlarhee() */

#undef F_NAME
#define F_NAME "lskelguohall"

/* ==================================== */
static int32_t guohall_L(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  int32_t q1, q2;
  if ((x+rs+rs >= N) || (x%rs < 2))
  {
    fprintf(stderr, "%s: object must not hit the 2 pixel-thick frame\n", F_NAME);
    exit(0);
  }
  q1 = F[x-2]; q2 = F[x+rs+rs];
  extract_vois(F, x, rs, N, v);
  return (((!v[2] && v[6] && !q2) &&
	   (v[1] || v[0] || v[7]) &&
	   (v[3] || v[4] || v[5])) ||
	  (!v[0] && v[4] && !q1));
} /* guohall_L() */

/* ==================================== */
static int32_t guohall_d1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  int32_t q1, q2;
  if ((x+rs+rs >= N) || (x%rs < 2))
  {
    fprintf(stderr, "%s: object must not hit the 2 pixel-thick frame\n", F_NAME);
    exit(0);
  }
  q1 = F[x-2]; q2 = F[x+rs+rs];
  extract_vois(F, x, rs, N, v);
  return (!v[2] && v[4] && v[5] && v[6] && v[7] && v[0] && !q2);
} /* guohall_d1() */

/* ==================================== */
static int32_t guohall_d2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  int32_t q1, q2;
  if ((x+rs+rs >= N) || (x%rs < 2))
  {
    fprintf(stderr, "%s: object must not hit the 2 pixel-thick frame\n", F_NAME);
    exit(0);
  }
  q1 = F[x-2]; q2 = F[x+rs+rs];
  extract_vois(F, x, rs, N, v);
  return (!v[0] && v[2] && v[3] && v[4] && v[5] && v[6] && !q1);
} /* guohall_d2() */

/* ==================================== */
static int32_t guohall_d3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  int32_t q1, q2;
  if ((x+rs+rs >= N) || (x%rs < 2))
  {
    fprintf(stderr, "%s: object must not hit the 2 pixel-thick frame\n", F_NAME);
    exit(0);
  }
  q1 = F[x-2]; q2 = F[x+rs+rs];
  extract_vois(F, x, rs, N, v);
  return (!v[0] && !v[1] && !v[2] && !v[3] && v[4] && v[5] && v[6] && !v[7] && !q1 && !q2);
} /* guohall_d3() */

/* ==================================== */
static int32_t guohall_b1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (!v[0] && !v[1] && !v[2] && v[4] && !v[5] && v[6]);
} /* guohall_b1() */

/* ==================================== */
static int32_t guohall_b2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (!v[2] && !v[3] && !v[4] && v[6] && !v[7] && v[0]);
} /* guohall_b2() */

/* ==================================== */
int32_t lskelguohall(struct xvimage *image,
		 int32_t nsteps,
		 struct xvimage *inhibit,
		 int32_t variante)
/* ==================================== */
// described in Guo & Hall : "Fast fully parallel thinning algorithms"
#undef F_NAME
#define F_NAME "lskelguohall"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);

    switch (variante)
    {
    case 1:
      for (i = 0; i < N; i++)
	if (F[i] && bordext4(F, i, rs, N) && (nbvois8(F, i, rs, N) > 2)
	    && (crossing_nb(F, i, rs, N) == 1) && !guohall_L(F, i, rs, N))
	{
	  nonstab = 1;
	  T[i] = 1;
	}
      for (i = 0; i < N; i++)
	if (T[i] == 1) F[i] = 0;
	else if (F[i]) F[i] = 1;
      break;
    case 2:
      for (i = 0; i < N; i++)
	if (F[i] && bordext4(F, i, rs, N) && (nbvois8(F, i, rs, N) > 2)
	    && (crossing_nb(F, i, rs, N) == 1) && !guohall_d1(F, i, rs, N)
	    && !guohall_d2(F, i, rs, N) && !guohall_d3(F, i, rs, N))
	{
	  nonstab = 1;
	  T[i] = 1;
	}
      for (i = 0; i < N; i++)
	if (T[i] == 1) F[i] = 0;
	else if (F[i]) F[i] = 1;
      break;
    case 3:
      for (i = 0; i < N; i++)
	if (F[i] &&
	    ((bordext4(F, i, rs, N) && (nbvois8(F, i, rs, N) > 2)
	     && (crossing_nb(F, i, rs, N) == 1) && !guohall_d1(F, i, rs, N)
	     && !guohall_d2(F, i, rs, N) && !guohall_d3(F, i, rs, N)) ||
	     guohall_b1(F, i, rs, N) || guohall_b2(F, i, rs, N)))
	{
	  nonstab = 1;
	  T[i] = 1;
	}
      for (i = 0; i < N; i++)
	if (T[i] == 1) F[i] = 0;
	else if (F[i]) F[i] = 1;
      break;
    default:
      fprintf(stderr, "%s: variant not implemented\n", F_NAME);
      return 0;
    } // switch (variante)

  } // while (nonstab && (step < nsteps))

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelguohall() */

/* ==================================== */
static int32_t chinwan_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0
  1 P 1
  X 1 X
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || !v[4] || !v[6]) goto fail;
    if (v[1] || v[2] || v[3]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* chinwan_match1() */

/* ==================================== */
static int32_t chinwan_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  X 0 0
  1 P 0
  X 1 X
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[4] || !v[6]) goto fail;
    if (v[0] || v[1] || v[2]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* chinwan_match2() */

/* ==================================== */
static int32_t chinwan_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  int32_t q1, q2;
  if ((x+rs+rs >= N) || (x%rs >= rs - 2))
  {
    fprintf(stderr, "%s: object must not hit the 2 pixel-thick frame\n", F_NAME);
    exit(0);
  }
  q1 = F[x+2]; q2 = F[x+rs+rs];
  extract_vois(F, x, rs, N, v);
  return ((v[0] && !v[4] && !q1) || (!v[2] && v[6] && !q2));
} /* chinwan_match3() */

/* ==================================== */
int32_t lskelchinwan(struct xvimage *image,
		 int32_t nsteps,
		 struct xvimage *inhibit)
/* ==================================== */
// described in Chin, Wan, Stover & Iverson : "A one-pass thinning algorithms and its parallel implementation"
#undef F_NAME
#define F_NAME "lskelchinwan"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);

    for (i = 0; i < N; i++)
      if (F[i] && (chinwan_match1(F, i, rs, N) || chinwan_match2(F, i, rs, N)) &&
	  !chinwan_match3(F, i, rs, N))
      {
	nonstab = 1;
	T[i] = 1;
      }
    for (i = 0; i < N; i++)
      if (T[i] == 1) F[i] = 0;
      else if (F[i]) F[i] = 1;

  } // while (nonstab && (step < nsteps))

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelchinwan() */

/* ==================================== */
static int32_t jang_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 x   avec origine = (1,1) et x = don't care
  0 1 1
  x 1 x
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || v[3] || v[4] || !v[6]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jang_match1() */

/* ==================================== */
static int32_t jang_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 0 x   avec origine = (1,1) et x = don't care
  1 1 1
  1 1 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || !v[4] || !v[5] || !v[6] || !v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jang_match5() */

/* ==================================== */
static int32_t jang_match9(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0   avec origine = (1,1) et (p ou q)
  0 1 0
  p 1 q
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[5] && !v[7]) goto fail;
    if (v[0] || v[1] || v[2] || v[3] || v[4] || !v[6]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jang_match9() */

/* ==================================== */
static int32_t jang_match13(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (v[0] || v[1] || v[2] || v[3] || v[4] || v[5] || v[6] || !v[7]) goto fail;
    if (!v2[15] || !v2[14] || !v2[13]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* jang_match13() */

/* ==================================== */
static int32_t jang_match17(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || !v[1] || !v[2] || v[3] || !v[4] || !v[5] || !v[6] || !v[7]) goto fail;
    if (!v2[0] || !v2[1] || !v2[3] || !v2[4] || !v2[8] || !v2[9] ||
	!v2[11] || !v2[12] || !v2[13] || !v2[14] || !v2[15]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* jang_match17() */

/* ==================================== */
static int32_t jang_match21(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || !v[1] || !v[2] || v[3] || !v[4] || v[6] || v[7]) return 0;
  if (v2[4] || v2[5]) return 0;
  return 1;
} /* jang_match21() */

/* ==================================== */
static int32_t jang_match22(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || v[1] || v[2] || !v[4] || v[5] || !v[6] || !v[7]) return 0;
  if (v2[11] || v2[12]) return 0;
  return 1;
} /* jang_match22() */

/* ==================================== */
static int32_t jang_match23(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || v[1] || v[2] || !v[3] || !v[4] || v[5] || !v[6]) return 0;
  if (v2[9] || v2[8]) return 0;
  return 1;
} /* jang_match23() */

/* ==================================== */
static int32_t jang_match24(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || !v[2] || v[3] || !v[4] || !v[5] || v[6] || v[7]) return 0;
  if (v2[7] || v2[8]) return 0;
  return 1;
} /* jang_match24() */

/* ==================================== */
static int32_t jang_match25(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (!v[0] || v[2] || !v[4] || !v[5] || !v[6] || !v[7]) return 0;
  if (v2[12]) return 0;
  return 1;
} /* jang_match25() */

/* ==================================== */
static int32_t jang_match26(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (!v[0] || !v[1] || !v[2] || v[4] || !v[6] || !v[7]) return 0;
  if (v2[0]) return 0;
  return 1;
} /* jang_match26() */

/* ==================================== */
static int32_t jang_match27(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[1] || v[2] || v[3] || v[4] || v[5] || !v[6]) goto fail;
    if (v2[0] || v2[1] || v2[11] || v2[12] || v2[13] || v2[14] || v2[15]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* jang_match27() */

/* ==================================== */
static int32_t jang_match27b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[1] || v[2] || v[3] || v[4] || v[5] || !v[6] || v[7]) goto fail;
    if (v2[0] || v2[1] || v2[11] || v2[12] || v2[13] || v2[15]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* jang_match27b() */

/* ==================================== */
static int32_t jang_match28b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[1] || v[2] || v[3] || v[4] || v[5] || !v[6] || !v[7]) goto fail;
    if (v2[0] || v2[1] || v2[11] || v2[12] || v2[13] || v2[14] || v2[15]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* jang_match28b() */

/* ==================================== */
int32_t lskeljang(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
// described in Jang and Chin : "One-pass parallel thinning: analysis, properties and quantitative evaluation" 1992
#undef F_NAME
#define F_NAME "lskeljang"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (F[i])
      {
	m1 = jang_match1(F, i, rs, N);
	m2 = jang_match5(F, i, rs, N);
	m3 = jang_match9(F, i, rs, N);
	m4 = jang_match13(F, i, rs, N);
	m5 = jang_match17(F, i, rs, N);
	m6 = jang_match21(F, i, rs, N);
	m7 = jang_match22(F, i, rs, N);
	m8 = jang_match23(F, i, rs, N);
	m9 = jang_match24(F, i, rs, N);
	m10 = jang_match25(F, i, rs, N);
	m11 = jang_match26(F, i, rs, N);
	m12 = jang_match27(F, i, rs, N);
	if ((m1 || m2 || m3 || m4 || m5) && !m6 && !m7 && !m8 && !m9 && !m10 && !m11 && !m12)
	{
//#define DEBUG_JANG
#ifdef DEBUG_JANG
	  printf("i = %d,%d : %d %d %d    %d %d %d    %d %d %d    %d %d %d\n",
		  i%rs, i/rs, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
#endif
	  T[i] = 1; // delete point

	}
      }

    for (i = 0; i < N; i++)
      if (T[i]) { F[i] = 0; nonstab = 1; }
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskeljang() */

/* ==================================== */
int32_t lskeljangcor(struct xvimage *image,
		 int32_t nsteps,
		 struct xvimage *inhibit)
/* ==================================== */
// described in Jang and Chin : "One-pass parallel thinning: analysis, properties and quantitative evaluation"
// correction by Michel Couprie
#undef F_NAME
#define F_NAME "lskeljang"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (F[i])
      {
	m1 = jang_match1(F, i, rs, N);
	m2 = jang_match5(F, i, rs, N);
	m3 = jang_match9(F, i, rs, N);
	m4 = jang_match13(F, i, rs, N);
	m5 = jang_match17(F, i, rs, N);
	m6 = jang_match21(F, i, rs, N);
	m7 = jang_match22(F, i, rs, N);
	m8 = jang_match23(F, i, rs, N);
	m9 = jang_match24(F, i, rs, N);
	m10 = jang_match25(F, i, rs, N);
	m11 = jang_match26(F, i, rs, N);
	m12 = jang_match27b(F, i, rs, N);
	m13 = jang_match28b(F, i, rs, N);
	if ((m1 || m2 || m3 || m4 || m5) && !m6 && !m7 && !m8 && !m9 && !m10 && !m11 && !m12 && !m13)
	{
#ifdef DEBUG_JANG
	  printf("i = %d,%d : %d %d %d    %d %d %d    %d %d %d    %d %d %d %d\n",
		  i%rs, i/rs, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13);
#endif
	  T[i] = 1; // delete point

	}
      }

    for (i = 0; i < N; i++)
      if (T[i]) { F[i] = 0; nonstab = 1; }
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskeljangcor() */

/* ==================================== */
int32_t mns_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if ((v[1] == 0) && (v[2] == 0) && (v[6] == 0) && (v[7] == 0)) return 1;
    if ((v[1] == 0) && (v[2] == 0)) goto fail;
    if ((v[6] == 0) && (v[7] == 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mns_match1() */

/* ==================================== */
int32_t mns_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 2) || (v[2] != 2)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[2] != 0) ||
	(v2[3] != 0) || (v2[4] != 0) || (v2[5] != 0) || (v2[15] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mns_match2() */

/* ==================================== */
int32_t mns_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 0) || (v[1] != 2) || (v[2] != 2)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[2] != 0) ||
	(v2[3] != 0) || (v2[4] != 0) || (v2[5] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mns_match3() */

/* ==================================== */
int32_t mns_match4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 2) || (v[2] != 0)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[2] != 0) ||
	(v2[3] != 0) || (v2[4] != 0) || (v2[15] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mns_match4() */

/* ==================================== */
int32_t mns_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 0) || (v[2] != 2)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[3] != 0) ||
	(v2[4] != 0) || (v2[5] != 0) || (v2[15] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mns_match5() */

/* ==================================== */
int32_t mns_match6(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 0) || (v[1] != 2) || (v[2] != 0)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[2] != 0) || (v2[3] != 0) || (v2[4] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mns_match6() */

/* ==================================== */
int32_t lskelmns(struct xvimage *image,
	     int32_t nsteps,
	     struct xvimage *inhibit)
/* ==================================== */
#undef F_NAME
#define F_NAME "lskelmns"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N))
	F[i] = 2;
    for (i = 0; i < N; i++)
      if (F[i] == 2)
      {
	m1 = mns_match1(F, i, rs, N);
	m2 = mns_match2(F, i, rs, N);
	m3 = mns_match3(F, i, rs, N);
	m4 = mns_match4(F, i, rs, N);
	m5 = mns_match5(F, i, rs, N);
	m6 = mns_match6(F, i, rs, N);
	if (m1 || m2 || m3 || m4 || m5 || m6)
	{
#ifdef DEBUG_MNS
	  printf("point %d,%d : m1 = %d; m2 = %d; m3 = %d; m4 = %d; m5 = %d; m6 = %d\n",
		 i % rs, i / rs, m1, m2, m3, m4, m5, m6);
#endif
	  T[i] = 1; // preserve point
	}
      }

#ifdef DEBUG_MNS
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    for (i = 0; i < N; i++)
      if ((F[i] == 2) && !T[i]) { F[i] = 0; nonstab = 1; }
    for (i = 0; i < N; i++) if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelmns() */

/* ==================================== */
static int32_t jangrec_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 q   avec origine = (1,1) et (p ou q)
  0 1 1
  p 1 0
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[1] && !v[5]) goto fail;
    if (!v[0] || v[2] || v[3] || v[4] || !v[6]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jangrec_match1() */

/* ==================================== */
static int32_t jangrec_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 x   avec origine = (1,1) et x = don't care
  0 1 1
  x 1 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || v[3] || v[4] || !v[6] || !v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jangrec_match5() */

/* ==================================== */
static int32_t jangrec_match9(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 0 x   avec origine = (1,1) et x = don't care
  1 1 1
  1 1 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || !v[4] || !v[5] || !v[6] || !v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jangrec_match9() */

/* ==================================== */
static int32_t jangrec_match13(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0   avec origine = (1,1)
  0 1 0
  1 1 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] || v[1] || v[2] || v[3] || v[4] || !v[5] || !v[6] || !v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* jangrec_match13() */

/* ==================================== */
static int32_t jangrec_match17(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (!v[0] || v[2] || !v[4] || !v[5] || !v[6] || !v[7]) return 0;
  if (v2[12]) return 0;
  return 1;
} /* jangrec_match17() */

/* ==================================== */
static int32_t jangrec_match18(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (!v[0] || !v[1] || !v[2] || v[4] || !v[6] || !v[7]) return 0;
  if (v2[0]) return 0;
  return 1;
} /* jangrec_match18() */

/* ==================================== */
static int32_t jangrec_match19(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || !v[1] || !v[2] || v[3] || !v[4] || v[6] || v[7]) return 0;
  if (v2[4] || v2[5]) return 0;
  return 1;
} /* jangrec_match19() */

/* ==================================== */
static int32_t jangrec_match20(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || v[1] || v[2] || !v[4] || v[5] || !v[6] || !v[7]) return 0;
  if (v2[11] || v2[12]) return 0;
  return 1;
} /* jangrec_match20() */

/* ==================================== */
static int32_t jangrec_match21(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || v[1] || v[2] || !v[3] || !v[4] || v[5] || !v[6]) return 0;
  if (v2[9] || v2[8]) return 0;
  return 1;
} /* jangrec_match21() */

/* ==================================== */
static int32_t jangrec_match22(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[0] || !v[2] || v[3] || !v[4] || !v[5] || v[6] || v[7]) return 0;
  if (v2[7] || v2[8]) return 0;
  return 1;
} /* jangrec_match22() */

/* ==================================== */
static int32_t jangrec_match23(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
   0 0 0 0   avec origine = (1,1)
   0 1 1 0
   0 1 1 0
   0 0 0 0
*/
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (!v[0] || v[1] || v[2] || v[3] || v[4] || v[5] || !v[6] || !v[7]) return 0;
  if (v2[0] || v2[1] || v2[11] || v2[12] || v2[13] || v2[14] || v2[15]) return 0;
  return 1;
} /* jangrec_match23() */

/* ==================================== */
int32_t lskeljangrec(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
// described in Jang and Chin : "Reconstructable  parallel thinning"
#undef F_NAME
#define F_NAME "lskeljangrec"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *I;            /* l'image d'inhibition (axe médian) */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11;

  if (inhibit == NULL)
  {
    fprintf(stderr, "%s: inhibit image (medial axis) must be present\n", F_NAME);
    return 0;
  }
  I = UCHARDATA(inhibit);

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (F[i])
      {
	m1 = jangrec_match1(F, i, rs, N);
	m2 = jangrec_match5(F, i, rs, N);
	m3 = jangrec_match9(F, i, rs, N);
	m4 = jangrec_match13(F, i, rs, N);
	m5 = jangrec_match17(F, i, rs, N);
	m6 = jangrec_match18(F, i, rs, N);
	m7 = jangrec_match19(F, i, rs, N);
	m8 = jangrec_match20(F, i, rs, N);
	m9 = jangrec_match21(F, i, rs, N);
	m10 = jangrec_match22(F, i, rs, N);
	m11 = jangrec_match23(F, i, rs, N);
	if ((m1 || m2 || m3 || m4) && !m5 && !m6 && !m7 && !m8 && !m9 && !m10 && !m11)
	{
//#define DEBUG_JANG
#ifdef DEBUG_JANG
	  printf("i = %d,%d : %d %d %d    %d %d %d    %d %d %d    %d %d\n",
		  i%rs, i/rs, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
#endif
	  T[i] = 1; // delete point

	}
      }

    for (i = 0; i < N; i++)
      if (T[i] && !I[i]) { F[i] = 0; nonstab = 1; }
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskeljangrec() */

/* ==================================== */
static int32_t choy_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 x   avec origine = (1,1) et x = don't care
  0 1 1
  x 1 x
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || v[3] || v[4] || !v[6]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* choy_match1() */

/* ==================================== */
static int32_t choy_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 0 x   avec origine = (1,1) et x = don't care
  1 1 1
  1 1 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || !v[4] || !v[5] || !v[6] || !v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* choy_match5() */

/* ==================================== */
static int32_t choy_match9(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0   avec origine = (1,1) et (p ou q)
  0 1 0
  p 1 q
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[5] && !v[7]) goto fail;
    if (v[0] || v[1] || v[2] || v[3] || v[4] || !v[6]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* choy_match9() */

/* ==================================== */
static int32_t choy_matchb1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 0 x
  1 1 1
  1 1 1
    0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && !v[2] && v[4] && v[5] && v[6] && v[7] && !v2[12];
} /* choy_matchb1() */

/* ==================================== */
static int32_t choy_matchb2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 0 0
  1 1 1
  0 1 1
    0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && !v[1] && !v[2] && v[4] && !v[5] && v[6] && v[7] && !v2[12];
} /* choy_matchb2() */

/* ==================================== */
static int32_t choy_matchb3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 x
  0 1 1
  1 1 0
    0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && !v[2] && !v[3] && !v[4] && v[5] && v[6] && !v[7] && !v2[12];
} /* choy_matchb3() */

/* ==================================== */
static int32_t choy_matchb7(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 1 1
  0 1 1 0
  x 1 1
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && v[1] && v[2] && !v[4] && v[6] && v[7] && !v2[0];
} /* choy_matchb7() */

/* ==================================== */
static int32_t choy_matchb8(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 1
  0 1 1 0
  x 1 0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && v[1] && !v[2] && !v[3] && !v[4] && v[6] && !v[7] && !v2[0];
} /* choy_matchb8() */

/* ==================================== */
static int32_t choy_matchb11(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
    1 0 0
  0 1 1 0
    0 1 x
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return !v[0] && !v[1] && !v[2] && v[3] && v[4] && !v[5] && v[6] && !v2[8];
} /* choy_match11() */

/* ==================================== */
static int32_t choy_matchb21(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0
  0 1 1 0
  0 1 1
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && !v[1] && !v[2] && !v[3] && !v[4] && !v[5] && v[6] && v[7] && !v2[0];
} /* choy_match21() */

/* ==================================== */
static int32_t choy_matchb22(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0
  0 1 1
  0 1 1
    0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return v[0] && !v[1] && !v[2] && !v[3] && !v[4] && !v[5] && v[6] && v[7] && !v2[12];
} /* choy_match22() */

/* ==================================== */
static int32_t choy_matchb23(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0
  1 1 0
  1 1 0
    0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return !v[0] && !v[1] && !v[2] && !v[3] && v[4] && v[5] && v[6] && !v[7] && !v2[12];
} /* choy_match23() */

/* ==================================== */
static int32_t choy_matchb24(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
    0 0 0
  0 1 1 0
    1 1 0
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  return !v[0] && !v[1] && !v[2] && !v[3] && v[4] && v[5] && v[6] && !v[7] && !v2[8];
} /* choy_match24() */

/* ==================================== */
int32_t lskelchoy(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
// described in Choy and Choy and Sui : ""
#undef F_NAME
#define F_NAME "lskelchoy"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (F[i])
      {
	m1 = choy_match1(F, i, rs, N);
	m2 = choy_match5(F, i, rs, N);
	m3 = choy_match9(F, i, rs, N);
	m4 = choy_matchb1(F, i, rs, N);
	m5 = choy_matchb2(F, i, rs, N);
	m6 = choy_matchb3(F, i, rs, N);
	m7 = choy_matchb7(F, i, rs, N);
	m8 = choy_matchb8(F, i, rs, N);
	m9 = choy_matchb11(F, i, rs, N);
	m10 = choy_matchb21(F, i, rs, N);
	m11 = choy_matchb22(F, i, rs, N);
	m12 = choy_matchb23(F, i, rs, N);
	m13 = choy_matchb24(F, i, rs, N);
	if ((m1 || m2 || m3) && !m4 && !m5 && !m6 && !m7 && !m8 && !m9 && !m10 && !m11 && !m12 && !m13)
	  //if ((m1 || m2 || m3) && !m4 && !m5 && !m6 && !m7 && !m8 && !m9 && !m10 && !m11 && !m12 && !m13 &&
	  //!jang_match27b(F, i, rs, N) && !jang_match27b(F, i, rs, N)) // correction mc
	{
//#define DEBUG_CHOY
#ifdef DEBUG_CHOY
	  printf("i = %d,%d : %d %d %d    %d %d %d    %d %d %d    %d %d %d    %d\n",
		  i%rs, i/rs, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13);
#endif
	  T[i] = 1; // delete point

	}
      }

    for (i = 0; i < N; i++)
      if (T[i]) { F[i] = 0; nonstab = 1; }
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelchoy() */

/* ==================================== */
int32_t mb_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x x 1 x
  0 P 1 1
  x x 1 x
*/
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || !v[1] || !v[7] || v[4]) goto fail;
    if (!v2[0]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mb_match1() */

/* ==================================== */
int32_t mb_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 x x
  0 P 1 x
  x 1 1 1
  x x 1 x
*/
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if (!v[0] || v[2] || v[3] || v[4] || !v[6] || !v[7]) goto fail;
    if (!v2[13] || !v2[15]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mb_match2() */

/* ==================================== */
int32_t mb_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x x x
  x P 0
  x 0 1
*/
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (!v[0] && !v[6] && v[7])  return 1;
    rotate90_vois(v);
  }
  return 0;
} /* mb_match3() */

/* ==================================== */
int32_t lskelmanz(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit,
	      int32_t variante)
/* ==================================== */
// described in Manzanera and Bernard : "Improved low complexity fully parallel thinning algorithm"
// variante =
//  1 : algo original
//  2 : variante GB (seulement masques 1 et 3)
#undef F_NAME
#define F_NAME "lskelmanz"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if ((variante < 1) || (variante > 2))
  {
    fprintf(stderr, "%s: variante: must be 1 or 2\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (F[i])
      {
	m1 = mb_match1(F, i, rs, N);
	if (variante == 1)
	  m2 = mb_match2(F, i, rs, N);
	else
	  m2 = 0;
	m3 = mb_match3(F, i, rs, N);
	if ((m1 || m2) && !m3)
	{
//#define DEBUG_MANZ
#ifdef DEBUG_MANZ
	  printf("i = %d,%d : %d %d %d\n",
		  i%rs, i/rs, m1, m2, m3);
#endif
	  T[i] = 1; // delete point

	}
      }

    for (i = 0; i < N; i++)
      if (T[i]) { F[i] = 0; nonstab = 1; }
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelmanz() */

/* ==================================== */
static int32_t hall_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return ((v[0] == 2) && v[2] && v[6]);
} /* hall_match1() */

/* ==================================== */
static int32_t hall_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return ((v[6] == 2) && v[0] && v[4]);
} /* hall_match2() */

/* ==================================== */
static int32_t hall_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return ((v[6] == 2) && (v[7] == 2) && (v[0] == 2));
} /* hall_match3() */

/* ==================================== */
int32_t lskelhall(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit,
	      int32_t variante)
/* ==================================== */
// described in Hall : "Fast parallel thinning algorithms: parallel speed and connectivity preservation"
// variante 1 : Holt et al.
// variante 2 : variante Hall pour "mieux" préserver les diagonales (?)
#undef F_NAME
#define F_NAME "lskelhall"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

#ifdef VERBOSE
  printf("nstep=%d, variante=%d\n", nsteps, variante);
#endif

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);

    switch (variante)
    {
    case 1:
      for (i = 0; i < N; i++)
	if (F[i] && (nbvois8(F, i, rs, N) > 1)  && (nbvois8(F, i, rs, N) < 7)
	    && (crossing_nb(F, i, rs, N) == 1))
	  F[i] = 2;
      for (i = 0; i < N; i++)
	if ((F[i] == 2) && !hall_match1(F, i, rs, N) && !hall_match2(F, i, rs, N) && !hall_match3(F, i, rs, N))
	{
	  nonstab = 1;
	  T[i] = 1;
	}
      for (i = 0; i < N; i++)
	if (T[i] == 1) F[i] = 0;
	else if (F[i]) F[i] = 1;
      break;
    case 2:
      for (i = 0; i < N; i++)
	if (F[i] && (nbvois8(F, i, rs, N) > 2)  && (nbvois8(F, i, rs, N) < 7)
	    && (crossing_nb(F, i, rs, N) == 1))
	  F[i] = 2;
      for (i = 0; i < N; i++)
	if ((F[i] == 2) && !hall_match1(F, i, rs, N) && !hall_match2(F, i, rs, N) && !hall_match3(F, i, rs, N))
	{
	  nonstab = 1;
	  T[i] = 1;
	}

//#define DEBUG_HALL2
#ifdef DEBUG_HALL2
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

      for (i = 0; i < N; i++)
	if (T[i] == 1) F[i] = 0;
	else if (F[i]) F[i] = 1;
      break;
    default:
      fprintf(stderr, "%s: variant not implemented\n", F_NAME);
      return 0;
    } // switch (variante)

  } // while (nonstab && (step < nsteps))

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelhall() */

/* ==================================== */
static int32_t wutsai_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  1 1 p   avec origine = (1,1) et (!p ou !q)
  1 1 0
  1 1 q
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[1] && v[7]) return 0;
  if (v[0] || !v[2] || !v[3] || !v[4] || !v[5] || !v[6]) return 0;
  return 1;
} /* wutsai_match1() */

/* ==================================== */
static int32_t wutsai_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  1 1 1   avec origine = (1,1) et (!p ou !q)
  1 1 1
  p 0 q
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[5] && v[7]) return 0;
  if (v[6] || !v[0] || !v[1] || !v[2] || !v[3] || !v[4]) return 0;
  return 1;
} /* wutsai_match2() */

/* ==================================== */
static int32_t wutsai_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  p 1 1     avec origine = (1,1) et (!p ou !q)
  0 1 1 1
  q 1 1
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[3] && v[5]) return 0;
  if (v[4] || !v[0] || !v[1] || !v[2] || !v[6] || !v[7]) return 0;
  if (!v2[0]) return 0;
  return 1;
} /* wutsai_match3() */

/* ==================================== */
static int32_t wutsai_match4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  p 0 q     avec origine = (1,1) et (!p ou !q)
  1 1 1
  1 1 1
    1
 */
{
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  if (v[1] && v[3]) return 0;
  if (v[2] || !v[0] || !v[4] || !v[5] || !v[6] || !v[7]) return 0;
  if (!v2[12]) return 0;
  return 1;
} /* wutsai_match4() */

/* ==================================== */
static int32_t wutsai_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 0 0   avec origine = (1,1) et x = don't care
  1 1 0
  x 1 x
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (!v[0] && !v[1] && !v[2] && v[4] && v[6]);
} /* wutsai_match5() */

/* ==================================== */
static int32_t wutsai_match6(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 1 1   avec origine = (1,1) et x = don't care
  0 1 1
  0 0 x
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (v[0] && v[1] && v[2] && !v[4] && !v[5] && !v[6]);
} /* wutsai_match6() */

/* ==================================== */
static int32_t wutsai_match7(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 1 0   avec origine = (1,1)
  0 1 1
  0 0 0
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (v[0] && !v[1] && v[2] && !v[3] && !v[4] && !v[5] && !v[6] && !v[7]);
} /* wutsai_match7() */

/* ==================================== */
static int32_t wutsai_match8(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  x 1 x   avec origine = (1,1) et x = don't care
  1 1 0
  x 0 0
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (!v[0] && v[2] && v[4] && !v[6] && !v[7]);
} /* wutsai_match8() */

/* ==================================== */
static int32_t wutsai_match9(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 x   avec origine = (1,1) et x = don't care
  0 1 1
  x 1 1
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (v[0] && !v[2] && !v[3] && !v[4] && v[6] && v[7]);
} /* wutsai_match9() */

/* ==================================== */
static int32_t wutsai_match10(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0   avec origine = (1,1)
  0 1 1
  0 1 0
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  return (v[0] && !v[1] && !v[2] && !v[3] && !v[4] && !v[5] && v[6] && !v[7]);
} /* wutsai_match10() */

/* ==================================== */
static int32_t wutsai_match11(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  0 0 0   avec origine = (1,1)
  0 1 0
  1 1 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] || v[1] || v[2] || v[3] || v[4] || !v[5] || !v[6] || !v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* wutsai_match11() */

/* ==================================== */
int32_t lskelwutsai(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
// described in Wutsai and Wutsai and Sui : "A new one-pass parallel thinning algorithm for binary images"
#undef F_NAME
#define F_NAME "lskelwutsai"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (F[i])
      {
	m1 = wutsai_match1(F, i, rs, N);
	m2 = wutsai_match2(F, i, rs, N);
	m3 = wutsai_match3(F, i, rs, N);
	m4 = wutsai_match4(F, i, rs, N);
	m5 = wutsai_match5(F, i, rs, N);
	m6 = wutsai_match6(F, i, rs, N);
	m7 = wutsai_match7(F, i, rs, N);
	m8 = wutsai_match8(F, i, rs, N);
	m9 = wutsai_match9(F, i, rs, N);
	m10 = wutsai_match10(F, i, rs, N);
	m11 = wutsai_match11(F, i, rs, N);
	if (m1 || m2 || m3 || m4 || m5 || m6 || m7 || m8 || m9 || m10 || m11)
	  //if ((m1 || m2 || m3 || m4 || m5 || m6 || m7 || m8 || m9 || m10 || m11) && !jangrec_match23(F, i, rs, N)) // cor. MC
	{
	  //#define DEBUG_WUTSAI
#ifdef DEBUG_WUTSAI
	  printf("i = %d,%d : %d %d %d    %d %d %d    %d %d %d    %d %d\n",
		  i%rs, i/rs, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
#endif
	  T[i] = 1; // delete point
	}
      }

    for (i = 0; i < N; i++)
      if (T[i]) { F[i] = 0; nonstab = 1; }
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelwutsai() */

/* ==================================== */
int32_t mc_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
//      m1 : match   A 2 B   avec origine = (1,0) et [au moins un des A et au moins un des B non nuls]
//                   A 2 B   ou [A et B tous nuls] (+ rotation 90)
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if ((v[1] == 0) && (v[2] == 0) && (v[6] == 0) && (v[7] == 0)) return 1;
    if ((v[1] == 0) && (v[2] == 0)) goto fail;
    if ((v[6] == 0) && (v[7] == 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mc_match1() */

/* ==================================== */
int32_t mc_match1b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
//     m1b : match   0 2 0   avec origine = (1,0) (+ rotation 90)
//                   0 2 0
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if ((v[1] == 0) && (v[2] == 0) && (v[6] == 0) && (v[7] == 0)) return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mc_match1b() */

/* ==================================== */
int32_t mc_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 2) || (v[2] != 2)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[3] != 0) || (v2[4] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mc_match2() */

/* ==================================== */
int32_t mc_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 0) || (v[1] != 2) || (v[2] != 2)) goto fail;
    if ((v2[0] != 0) || (v2[3] != 0) || (v2[4] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mc_match3() */

/* ==================================== */
int32_t mc_match3b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 0) || (v[1] != 2) || (v[2] != 2)) goto fail;
    if ((v2[3] != 0) || (v2[4] != 0) ||
	(v[3] != 0) || (v[4] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mc_match3b() */

/* ==================================== */
int32_t mc_match4(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 2) || (v[2] != 0)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) || (v2[4] != 0) ||
	(v[3] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mc_match4() */

/* ==================================== */
int32_t mc_match5(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 0) || (v[2] != 2)) goto fail;
    if ((v2[1] != 0) || (v2[3] != 0) ||
	(v[3] != 0) || (v[4] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mc_match5() */

/* ==================================== */
int32_t mc_match4b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8], v2[16];
  extract_vois(F, x, rs, N, v);
  if (!extract_vois2(F, x, rs, N, v2)) return 0;
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 2) || (v[2] != 0)) goto fail;
    if ((v2[0] != 0) || (v2[1] != 0) ||
	(v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
    rotate90_vois2(v2);
  }
  return 0;
} /* mc_match4b() */

/* ==================================== */
int32_t mc_match5b(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if ((v[0] != 2) || (v[1] != 0) || (v[2] != 2)) goto fail;
    if ((v[3] != 0) || (v[4] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
//printf("mc_match5b : %d %d fail\n", x % rs, x / rs);
//print_vois(v);
  return 0;
} /* mc_match5b() */

/* ==================================== */
int32_t mc_match6(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if ((v[3] != 2) || (v[2] != 0) || (v[4] != 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mc_match6() */

/* ==================================== */
int32_t mc_end(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 8; i++)
  {
    if (v[0] == 0) goto fail;
    if ((v[2] != 0) || (v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0)) goto fail;
    return 1;
  fail:
    rotate45_vois(v);
  }
  return 0;
} /* mc_end() */

/* ==================================== */
int32_t mc_maskend1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] == 0) goto fail;
    if ((v[1] != 0) || (v[2] != 0) || (v[3] != 0) || (v[4] != 0) || (v[5] != 0) ||
	(v[6] != 0) || (v[7] != 0)) goto fail;
//printf("mc_maskend1 : %d %d end\n", x % rs, x / rs);
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mc_maskend1() */

/* ==================================== */
int32_t mc_maskend2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[1] == 0) goto fail;
    if ((v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
//printf("mc_maskend2 : i = %d ; %d %d end\n", i, x % rs, x / rs);
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mc_maskend2() */

/* ==================================== */
int32_t mc_maskend3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if ((v[0] == 0) || (v[2] == 0)) goto fail;
    if ((v[3] != 0) || (v[4] != 0) || (v[5] != 0) || (v[6] != 0) || (v[7] != 0)) goto fail;
//printf("mc_maskend3 : %d %d end\n", x % rs, x / rs);
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* mc_maskend3() */

/* ==================================== */
int32_t mc_end2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
{
  return mc_maskend1(F, x, rs, N) || mc_maskend2(F, x, rs, N) || mc_maskend3(F, x, rs, N);
} /* mc_end2() */

/* ==================================== */
int32_t lskelmcultime(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
#undef F_NAME
#define F_NAME "lskelmcultime"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N))
	F[i] = 2;
    for (i = 0; i < N; i++)
      if (F[i] == 2)
      {
	m1 = mc_match1(F, i, rs, N);
	m2 = mc_match2(F, i, rs, N);
	m3 = mc_match3b(F, i, rs, N);
	m4 = mc_match4b(F, i, rs, N);
	m5 = mc_match5b(F, i, rs, N);
	m6 = mc_match6(F, i, rs, N);
	if (m1 || m2 || m3 || m4 || m5 || m6)
	{
#ifdef DEBUG_MC
	  printf("point %d,%d : m1 = %d; m2 = %d; m3 = %d; m4 = %d; m5 = %d; m6 = %d\n",
		 i % rs, i / rs, m1, m2, m3, m4, m5, m6);
#endif
	  T[i] = 1; // preserve point
	}
      }

#ifdef DEBUG_MC
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    for (i = 0; i < N; i++)
      if ((F[i] == 2) && !T[i]) { F[i] = 0; nonstab = 1; }
    for (i = 0; i < N; i++) if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelmcultime() */

/* ==================================== */
int32_t lskelmccurv(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
#undef F_NAME
#define F_NAME "lskelmccurv"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N)  && !mc_end2(F, i, rs, N))
	F[i] = 2;
    for (i = 0; i < N; i++)
      if (F[i] == 2)
      {
	m1 = mc_match1(F, i, rs, N);
	m2 = mc_match2(F, i, rs, N);
	m3 = mc_match3b(F, i, rs, N);
	m4 = mc_match4b(F, i, rs, N);
	m5 = mc_match5b(F, i, rs, N);
	m6 = mc_match6(F, i, rs, N);
	if (m1 || m2 || m3 || m4 || m5 || m6)
	{
#ifdef DEBUG_MC1
	  printf("point %d,%d : m1 = %d; m2 = %d; m3 = %d; m4 = %d; m5 = %d; m6 = %d\n",
		 i % rs, i / rs, m1, m2, m3, m4, m5, m6);
#endif
	  T[i] = 1; // preserve point
	}
      }

#ifdef DEBUG_MC
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    for (i = 0; i < N; i++)
      if ((F[i] == 2) && !T[i]) { F[i] = 0; nonstab = 1; }
    for (i = 0; i < N; i++) if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelmccurv() */

/* ==================================== */
int32_t lskelmccurvrec(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
#undef F_NAME
#define F_NAME "lskelmccurvrec"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *X = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  uint8_t *M = (uint8_t *)malloc(N);
  uint8_t *E = (uint8_t *)malloc(N);
  uint8_t *R = (uint8_t *)malloc(N);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (X[i]) X[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  memset(M, 0, N);
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(E, 0, N);
    for (i = 0; i < N; i++)
      if (T[i] && (nbvois4(T, i, rs, N) == 4)) E[i] = 1; // calcule E = eros (T)
    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (nbvois4(E, i, rs, N) >= 1) R[i] = 1;           // calcule D = Dilat(E)
    for (i = 0; i < N; i++)
      if (T[i] && !R[i]) R[i] = 1; else R[i] = 0;        // calcule D = T \ Dilat(E)
    for (i = 0; i < N; i++) T[i] = E[i];                 // T = E
    for (i = 0; i < N; i++) if (R[i]) M[i] = 1;          // M  = M union D

//#define DEBUG_MC

#ifdef DEBUG_MC
    printf("D, T\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", R[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    for (i = 0; i < N; i++)
      if ((X[i] == 1) && simple8(X, i, rs, N) && !M[i])
	X[i] = 2;

#ifdef DEBUG_MC
    printf("M, X\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", M[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", X[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (X[i] == 2)
      {
	m1 = mc_match1(X, i, rs, N);
	m2 = mc_match2(X, i, rs, N);
	m3 = mc_match3b(X, i, rs, N);
	m4 = mc_match4b(X, i, rs, N);
	m5 = mc_match5b(X, i, rs, N);
	m6 = mc_match6(X, i, rs, N);
	if (m1 || m2 || m3 || m4 || m5 || m6)
	{
#ifdef DEBUG_MC
	  printf("point %d,%d : m1 = %d; m2 = %d; m3 = %d; m4 = %d; m5 = %d\n",
		 i % rs, i / rs, m1, m2, m3, m4, m5);
#endif
	  R[i] = 1; // preserve point
	}
      }

#ifdef DEBUG_MC
    printf("X, R\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", X[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", R[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    for (i = 0; i < N; i++)
      if ((X[i] == 2) && !R[i] && !M[i]) { X[i] = 0; nonstab = 1; }
    for (i = 0; i < N; i++) if (X[i]) X[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (X[i]) X[i] = 255; // normalize values

  freeimage(tmp);
  free(M);
  free(E);
  free(R);
  return(1);
} /* lskelmccurvrec() */

/* ==================================== */
int32_t lskelmccurvrecold(struct xvimage *image,
		  int32_t nsteps,
		  struct xvimage *inhibit)
/* ==================================== */
#undef F_NAME
#define F_NAME "lskelmccurvrec"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *X = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  uint8_t *Y = (uint8_t *)malloc(N);
  uint8_t *I = (uint8_t *)malloc(N);
  uint8_t *D = (uint8_t *)malloc(N);
  int32_t step, nonstab;
  int32_t m1, m2, m3, m4, m5, m6;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (X[i]) X[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  memset(Y, 0, N);
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);
    memset(I, 0, N);
    memset(D, 0, N);
    for (i = 0; i < N; i++) if (X[i] && !Y[i]) T[i] = 1; // calcule T = X \ Y
    for (i = 0; i < N; i++)
      if (T[i] && (nbvois4(T, i, rs, N) == 4))
	I[i] = 1; // calcule I = interior (X \ Y)
    for (i = 0; i < N; i++)
//      if (T[i] && (nbvois4(I, i, rs, N) >= 1)) // test T
      if (nbvois4(I, i, rs, N) >= 1)
	D[i] = 1; // calcule D = Dilat(I) Inter [X \ Y]

//#define DEBUG_MC

#ifdef DEBUG_MC
    printf("I, D\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", I[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", D[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (Y[i] || (X[i] && !D[i]))
	T[i] = 1; // calcule E = Y Union [X \ D]
    for (i = 0; i < N; i++)
      if ((X[i] == 1) && simple8(X, i, rs, N) && !T[i])
	X[i] = 2;

#ifdef DEBUG_MC
    printf("E, X\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", X[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if (X[i] == 2)
      {
	m1 = mc_match1(X, i, rs, N);
	m2 = mc_match2(X, i, rs, N);
	m3 = mc_match3b(X, i, rs, N);
	m4 = mc_match4b(X, i, rs, N);
	m5 = mc_match5b(X, i, rs, N);
	m6 = mc_match6(X, i, rs, N);
	if (m1 || m2 || m3 || m4 || m5 || m6)
	{
#ifdef DEBUG_MC
	  printf("point %d,%d : m1 = %d; m2 = %d; m3 = %d; m4 = %d; m5 = %d\n",
		 i % rs, i / rs, m1, m2, m3, m4, m5);
#endif
	  T[i] = 1; // preserve point
	}
      }

#ifdef DEBUG_MC
    printf("X, T\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", X[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif

    for (i = 0; i < N; i++)
      if ((X[i] == 2) && !T[i]) { X[i] = 0; nonstab = 1; }
    for (i = 0; i < N; i++) if (X[i]) X[i] = 1;
    for (i = 0; i < N; i++) if (X[i] && !I[i]) Y[i] = 1;

#ifdef DEBUG_MC
    printf("X, Y\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", X[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", Y[j*rs + i]);
      printf("\n");
    }
    printf("\n");
#endif
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (X[i]) X[i] = 255; // normalize values

  freeimage(tmp);
  free(Y);
  free(I);
  free(D);
  return(1);
} /* lskelmccurvrecold() */

/* ==================================== */
int32_t bertrand_match1(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
A     A
2*    2
B     B
*/
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[0] != 2) return 0;
  if ((v[1] == 0) && (v[2] == 0)) return 0;
  if ((v[6] == 0) && (v[7] == 0)) return 0;
  return 1;
} /* bertrand_match1() */

/* ==================================== */
int32_t bertrand_match2(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
A   2   B
A   2*  B
*/
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[2] != 2) return 0;
  if ((v[0] == 0) && (v[1] == 0)) return 0;
  if ((v[3] == 0) && (v[4] == 0)) return 0;
  return 1;
} /* bertrand_match1() */

/* ==================================== */
int32_t lskelNK2(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
// algo asymetrique - ultime - minimal - avec contrainte axe median
// ATTENTION : les résultats sont incorrects si la contrainte n'est pas l'AM
#undef F_NAME
#define F_NAME "lskelNK2"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);   /* l'image de depart */
  uint8_t *I = NULL;               /* l'image d'inhibition */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2;
#ifdef ETUDE
  struct xvimage *tmp2 = copyimage(image);
  uint8_t *T2 = UCHARDATA(tmp2);
  memset(T2, 0, N);
#endif
#ifdef ANIMATE
  int32_t nimage = 1;
  char imname[128];
#endif

  if (inhibit == NULL) 
  {
    fprintf(stderr, "%s: inhibit image (medial axis) must be given\n", F_NAME);
    return 0;
  }

  I = UCHARDATA(inhibit);

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);

    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && !I[i])
	F[i] = 2;
    for (i = 0; i < N; i++)
      if (F[i] == 2)
      {
	m1 = bertrand_match1(F, i, rs, N);
	m2 = bertrand_match2(F, i, rs, N);
	if (m1 || m2)
	{
#ifdef DEBUG_BERTRAND
	  printf("point %d,%d : m1 = %d; m2 = %d\n",
		 i % rs, i / rs, m1, m2);
#endif
	  T[i] = 1; // preserve point
	}
      }

#ifdef DEBUG_BERTRAND
    { int j;
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
    }
#endif

    for (i = 0; i < N; i++)
      if ((F[i] == 2) && !T[i])
      {
	F[i] = 0;
	nonstab = 1;
#ifdef ETUDE
        T2[i] = step;
#endif
      }
    for (i = 0; i < N; i++) if (F[i]) F[i] = 1;

#ifdef ANIMATE
  sprintf(imname, "anim%03d.pgm", nimage); nimage++;
  writeimage(image, imname);
#endif

  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

#ifdef ETUDE
  writeimage(tmp2,"_etude");
#endif

  freeimage(tmp);
  return(1);
} /* lskelNK2() */

/* ==================================== */
int32_t lskelNK2b(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
// algo asymetrique - ultime - minimal - avec contrainte quelconque
#undef F_NAME
#define F_NAME "lskelNK2b"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);   /* l'image de depart */
  uint8_t *I = NULL;               /* l'image d'inhibition */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;
  int32_t m1, m2, mc1, mc2, mc3, mc4, mc5, mc6;
#ifdef ETUDE
  struct xvimage *tmp2 = copyimage(image);
  uint8_t *T2 = UCHARDATA(tmp2);
  memset(T2, 0, N);
#endif
#ifdef ANIMATE
  int32_t nimage = 1;
  char imname[128];
#endif

  if (inhibit != NULL) I = UCHARDATA(inhibit);

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif
    memset(T, 0, N);

    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && ((I == NULL) || !I[i]))
	F[i] = 2;
    for (i = 0; i < N; i++)
      if (F[i] == 2)
      {
	m1 = bertrand_match1(F, i, rs, N);
	m2 = bertrand_match2(F, i, rs, N);
	mc1 = mc_match1b(F, i, rs, N);
	mc2 = mc_match2(F, i, rs, N);
	mc3 = mc_match3b(F, i, rs, N);
	mc4 = mc_match4b(F, i, rs, N);
	mc5 = mc_match5b(F, i, rs, N);
	mc6 = mc_match6(F, i, rs, N);
	if (m1 || m2)
	{
	  T[i] = 1; // preserve point
	}
	if (mc1 || mc2 || mc3 || mc4 || mc5 || mc6)
	{
	  F[i] = T[i] = 1; // preserve point and unmark as simple
	}
      }

#ifdef DEBUG_BERTRAND
    { int j;
    printf("\n");
    for (j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++) printf("%d ", F[j*rs + i]);
      printf("      ");
      for (i = 0; i < rs; i++) printf("%d ", T[j*rs + i]);
      printf("\n");
    }
    printf("\n");
    }
#endif

    for (i = 0; i < N; i++)
      if ((F[i] == 2) && !T[i])
      {
	F[i] = 0;
	nonstab = 1;
#ifdef ETUDE
        T2[i] = step;
#endif
      }
    for (i = 0; i < N; i++) if (F[i]) F[i] = 1;

#ifdef ANIMATE
  sprintf(imname, "anim%03d.pgm", nimage); nimage++;
  writeimage(image, imname);
#endif

  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

#ifdef ETUDE
  writeimage(tmp2,"_etude");
#endif

  freeimage(tmp);
  return(1);
} /* lskelNK2b() */

/* ==================================== */
int32_t bertrand_match3(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
// A 2 B   avec origine = (1,0) et au moins un des A et au moins un des B non nuls
// A 2 B   (+ rotation 90)
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 2) goto fail;
    if ((v[1] == 0) && (v[2] == 0)) goto fail;
    if ((v[6] == 0) && (v[7] == 0)) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* bertrand_match3() */

/* ==================================== */
int32_t lskelbertrand_sym(struct xvimage *image,
	      int32_t nsteps,
	      struct xvimage *inhibit)
/* ==================================== */
/*
Algo CPSR1 données: S
T := S
Répéter jusqu'à stabilité
     T := T \ominus \Gamma_4
     P := pixels simples pour S et 4-voisins d'un point de T
     R := pixels de P qui s'apparient avec C2
     S :=  [S  \  P]  \cup  R
*/
#undef F_NAME
#define F_NAME "lskelbertrand_sym"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *t = copyimage(image);
  uint8_t *T = UCHARDATA(t);
  struct xvimage *r = copyimage(image);
  uint8_t *R = UCHARDATA(r);
  int32_t step, nonstab;
  int32_t m1;
#ifdef ETUDE
  struct xvimage *tmp2 = copyimage(image);
  uint8_t *T2 = UCHARDATA(tmp2);
  memset(T2, 0, N);
#endif

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif

    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (T[i] && (nbvois4(T, i, rs, N) == 4)) R[i] = 1; // calcule R = eros (T)
    for (i = 0; i < N; i++) T[i] = R[i];                 // T = eros (T)
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N)  && (nbvois4(T, i, rs, N) > 0))
	F[i] = 2; //  pixels simples pour F et 4-voisins d'un point de T
    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (F[i] == 2)
      {
	m1 = bertrand_match3(F, i, rs, N);
	if (m1)
	{
	  R[i] = 1; // preserve point
	}
      }

    for (i = 0; i < N; i++)
      if ((F[i] == 2) && !R[i])
      {
	F[i] = 0;
	nonstab = 1;
#ifdef ETUDE
        T2[i] = step;
#endif
      }
    for (i = 0; i < N; i++) if (F[i]) F[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

#ifdef ETUDE
  writeimage(tmp2,"_etude");
#endif

  freeimage(t);
  freeimage(r);
  return(1);
} /* lskelbertrand_sym() */

/* ==================================== */
int32_t lskelbertrand_asym_s(struct xvimage *image,
			int32_t nsteps,
			struct xvimage *inhibit)
/* ==================================== */
// described in [G. Bertrand - personnal communication - 26/10/2005]
// variante algo NK^2 (comme BK^2)
#undef F_NAME
#define F_NAME "lskelbertrand_asym_s"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *X = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  uint8_t *M = (uint8_t *)malloc(N);
  uint8_t *E = (uint8_t *)malloc(N);
  uint8_t *R = (uint8_t *)malloc(N);
  int32_t step, nonstab;
  int32_t m1, m2;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (X[i]) X[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif

    memset(E, 0, N);   // T = eros (T)
    for (i = 0; i < N; i++)
      if (T[i] && (nbvois4(T, i, rs, N) == 4)) E[i] = 1;
    memcpy(T, E, N);

    memset(M, 0, N);
    for (i = 0; i < N; i++)
      if (nbvois4(T, i, rs, N) >= 1) M[i] = 1;           // calcule M = Dilat(T)
    for (i = 0; i < N; i++)
      if ((X[i] == 1) && simple8(X, i, rs, N) && M[i])
	X[i] = 2;

    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (X[i] == 2)
      {
	m1 = bertrand_match1(X, i, rs, N);
	m2 = bertrand_match2(X, i, rs, N);
	if (m1 || m2)
	{
	  R[i] = 1; // preserve point
	}
      }

    for (i = 0; i < N; i++)
      if ((X[i] == 2) && !R[i]) { X[i] = 0; nonstab = 1; }
    for (i = 0; i < N; i++) if (X[i]) X[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (X[i]) X[i] = 255; // normalize values

  freeimage(tmp);
  free(M);
  free(E);
  free(R);
  return(1);
} /* lskelbertrand_asym_s() */

/* ==================================== */
int32_t lskelMK2(struct xvimage *image,
	     int32_t nsteps,
	     struct xvimage *inhibit)
/* ==================================== */
/*
Algo MK2 données: S, I
Répéter jusqu'à stabilité
  P := pixels simples pour S
  R := pixels de P qui s'apparient avec le masque C (bertrand_match3)
  T :=  [S  \  P] \cup  R \cup  I
  S := T \cup [S \ (T \oplus \Gamma_8*)]
*/
#undef F_NAME
#define F_NAME "lskelMK2"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *S = UCHARDATA(image);   /* l'image de depart */
  uint8_t *I;                      /* l'image d'inhibition */
  struct xvimage *t = copyimage(image);
  uint8_t *T = UCHARDATA(t);
  struct xvimage *r = copyimage(image);
  uint8_t *R = UCHARDATA(r);
  int32_t step, nonstab;
  int32_t m1;

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs))
    {
      fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
      return 0;
    }
    if (datatype(inhibit) != VFF_TYP_1_BYTE)
    {
      fprintf(stderr, "%s: incompatible image types\n", F_NAME);
      return 0;
    }
    I = UCHARDATA(inhibit);
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (S[i]) S[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif

    for (i = 0; i < N; i++)
      if ((S[i] == 1) && simple8(S, i, rs, N))
	S[i] = 2; //  pixels simples pour S
    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (S[i] == 2)
      {
	m1 = bertrand_match3(S, i, rs, N);
	if (m1) R[i] = 1; // preserve point
      }
    memset(T, 0, N);
    if (inhibit != NULL)
    {
      for (i = 0; i < N; i++) // T := [S \ P] \cup  R \cup I
	if ((S[i] == 1) || R[i] || I[i])
	  T[i] = 1;
    }
    else
    {
      for (i = 0; i < N; i++) // T := [S \ P] \cup  R
	if ((S[i] == 1) || R[i])
	  T[i] = 1;
    }
    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (nbvois8(T, i, rs, N) >= 1) R[i] = 1; // calcule R = Dilat(T)
    for (i = 0; i < N; i++)
      if (T[i] || (S[i] && !R[i])) T[i] = 1; else T[i] = 0; // T := T \cup [S \ R]

    for (i = 0; i < N; i++)
      if (S[i] && !T[i])
      {
	S[i] = 0;
	nonstab = 1;
      }
    for (i = 0; i < N; i++) if (S[i]) S[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (S[i]) S[i] = 255; // normalize values

  freeimage(t);
  freeimage(r);
  return(1);
} /* lskelMK2() */

/* ==================================== */
int32_t lskelAK2(struct xvimage *image,
	     int32_t nsteps,
	     struct xvimage *inhibit)
/* ==================================== */
/*
Amincissement symétrique avec inclusion de l'axe médian
Algo AK2 données: S
K := \emptyset ; T := S
Répéter jusqu'à stabilité
  E := T \ominus \Gamma_4
  D := T \ [E \oplus \Gamma_4]
  T := E
  K := K \cup D
  P := pixels de S \ K simples pour S
  R := pixels de P qui s'apparient avec le masque C (bertrand_match3)
  S := [S  \  P]  \cup  R
*/
#undef F_NAME
#define F_NAME "lskelAK2"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *S = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *t = copyimage(image);
  uint8_t *T = UCHARDATA(t);
  struct xvimage *r = copyimage(image);
  uint8_t *R = UCHARDATA(r);
  struct xvimage *e = copyimage(image);
  uint8_t *E = UCHARDATA(e);
  struct xvimage *d = copyimage(image);
  uint8_t *D = UCHARDATA(d);
  struct xvimage *k = copyimage(image);
  uint8_t *K = UCHARDATA(k);

  int32_t step, nonstab;
  int32_t m1;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (S[i]) S[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  // K := \emptyset ; T := S
  memset(K, 0, N);
  memcpy(T, S, N);
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif

    //  E := T \ominus \Gamma_4
    memset(E, 0, N);
    for (i = 0; i < N; i++)
      if (T[i] && (nbvois4(T, i, rs, N) == 4)) E[i] = 1;

    //  D := E \oplus \Gamma_4
    memset(D, 0, N);
    for (i = 0; i < N; i++)
      if (E[i] || nbvois4(E, i, rs, N) >= 1) D[i] = 1;

    //  D := T \ D
    for (i = 0; i < N; i++)
      if (T[i] && !D[i]) D[i] = 1; else D[i] = 0;

    //  T := E
    memcpy(T, E, N);

    //  K := K \cup D
    for (i = 0; i < N; i++)
      if (D[i]) K[i] = 1;

    //  P := pixels de S \ K simples pour S (P est représenté par les points à 2 dans S)
    for (i = 0; i < N; i++)
      if (S[i] && !K[i] && simple8(S, i, rs, N))
	S[i] = 2;

    //  R := pixels de P qui s'apparient avec le masque C (bertrand_match3)
    memset(R, 0, N);
    for (i = 0; i < N; i++)
      if (S[i] == 2)
      {
	m1 = bertrand_match3(S, i, rs, N);
	if (m1) R[i] = 1; // preserve point
      }

    //  D := [S  \  P]  \cup  R
    for (i = 0; i < N; i++)
      if ((S[i] == 1) || R[i])
	D[i] = 1;

    for (i = 0; i < N; i++) // pour  tester la stabilité
      if (S[i] && !D[i])
      {
	S[i] = 0;
	nonstab = 1;
      }
    for (i = 0; i < N; i++) if (S[i]) S[i] = 1;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (S[i]) S[i] = 255; // normalize values

  freeimage(t);
  freeimage(r);
  freeimage(e);
  freeimage(d);
  freeimage(k);
  return(1);
} /* lskelAK2() */

/* ==================================== */
static int32_t ros_match(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  D D D     avec origine = (1,1) et valeurs des pixels D quelconques
  D P 0
  D 0 1
 */
{
  int32_t i;
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  for (i = 0; i < 4; i++)
  {
    if (v[0] != 0) goto fail;
    if (v[6] != 0) goto fail;
    if (!v[7]) goto fail;
    return 1;
  fail:
    rotate90_vois(v);
  }
  return 0;
} /* ros_match() */

/* ==================================== */
static int32_t ros_north(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  D 0 D     avec origine = (1,1) et valeurs des pixels D quelconques
  D P D
  D D D
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[2] != 0) return 0;
  return 1;

} /* ros_north() */

/* ==================================== */
static int32_t ros_south(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  D D D     avec origine = (1,1) et valeurs des pixels D quelconques
  D P D
  D 0 D
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[6] != 0) return 0;
  return 1;

} /* ros_south() */

/* ==================================== */
static int32_t ros_east(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  D D D     avec origine = (1,1) et valeurs des pixels D quelconques
  D P 0
  D D D
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[0] != 0) return 0;
  return 1;

} /* ros_east() */

/* ==================================== */
static int32_t ros_west(uint8_t *F, int32_t x, int32_t rs, int32_t N)
/* ==================================== */
/*
  D D D     avec origine = (1,1) et valeurs des pixels D quelconques
  0 P D
  D D D
 */
{
  uint8_t v[8];
  extract_vois(F, x, rs, N, v);
  if (v[4] != 0) return 0;
  return 1;

} /* ros_west() */

/* ==================================== */
int32_t lskelrosenfeld(struct xvimage *image,
		       int32_t nsteps,
		       struct xvimage *inhibit)
/* ==================================== */
// Rosenfeld : algo directionnel
#undef F_NAME
#define F_NAME "lskelrosenfeld"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  if (nsteps == -1) nsteps = 1000000000;

  for (i = 0; i < N; i++) if (F[i]) F[i] = 1; // normalize values

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab && (step < nsteps))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && ros_north(F, i, rs, N) && !ros_match(F, i, rs, N))
      { nonstab = 1; T[i] = 1; }
    for (i = 0; i < N; i++) if (T[i] == 1) F[i] = 0;

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && ros_south(F, i, rs, N) && !ros_match(F, i, rs, N))
      { nonstab = 1; T[i] = 1; }
    for (i = 0; i < N; i++) if (T[i] == 1) F[i] = 0;

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && ros_east(F, i, rs, N) && !ros_match(F, i, rs, N))
      { nonstab = 1; T[i] = 1; }
    for (i = 0; i < N; i++) if (T[i] == 1) F[i] = 0;

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 1) && simple8(F, i, rs, N) && ros_west(F, i, rs, N) && !ros_match(F, i, rs, N))
      { nonstab = 1; T[i] = 1; }
    for (i = 0; i < N; i++) if (T[i] == 1) F[i] = 0;

  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif

  for (i = 0; i < N; i++) if (F[i]) F[i] = 255; // normalize values

  freeimage(tmp);
  return(1);
} /* lskelrosenfeld() */


//Detects endpoints of type 1, 2, 3 as defined in "Parallel connectivity preserving thinning algorithms", by Hall
int32_t is_hall_2dendpoint(struct xvimage* img, uint32_t x, uint32_t rs, uint32_t N, uint32_t type)
{
	uint32_t v, i, v1, v2;

	if(type <= 0 || type > 3)
	{
		fprintf(stderr, "hall_2dendpoint(): given type is not valid");
		return(0);
	}

	if(UCHARDATA(img)[x]==0) return 0;

	v = nbvois8((uint8_t*)(img->image_data), x, rs, N);

	if(v == 1)
	{
		return(1);
	}

	else if(v == 2)
	{
		if(type==1)
			return 0;

		else
		{
			for(i=0; i<8; i++)
			{
				v1 = voisin(x, i, rs, N);
				if((v1!=-1) && (UCHARDATA(img)[v1]!=0))
				{
					break;
				}
			}

			for(i=0; i<8; i++)
			{
				v2 = voisin(x, 7-i, rs, N);
				if((v2!=-1) && (UCHARDATA(img)[v2]!=0))
				{
					break;
				}
			}

			if(v1==v2) fprintf(stderr, "Bouh\n");

			if(type==2)
			{
				if(voisins8(v1, v2, rs))
					return 1;
				else
					return 0;
			}

			else
			{
				if(voisins4(v1, v2, rs))
					return 1;
				else
					return 0;
			}
		}
	}

	return 0;
}



//Detects i-self-deletable points as defined in "Parallel thinning algorithms based on Ronse's sufficient conditions for topology preservation", by Nemeth and Palagyi
int32_t is_self_deletable(struct xvimage *img, uint32_t x, uint32_t rs, uint32_t N, uint32_t type)
{
	if(type <= 0 || type > 3)
	{
		fprintf(stderr, "is_self_deletable(): given type is not valid");
		return(0);
	}

	if(UCHARDATA(img)[x]==0) return 0;

	return (simple8((uint8_t*)(img->image_data), x, rs, N) && (!is_hall_2dendpoint(img, x, rs, N, type)));
}


//Detects i-double-deletable points as defined in "Parallel thinning algorithms based on Ronse's sufficient conditions for topology preservation", by Nemeth and Palagyi
int32_t is_double_deletable(struct xvimage *img, uint32_t x, uint32_t rs, uint32_t N, uint32_t type)
{
	uint32_t i, v, ok1, ok2;

	if(type <= 0 || type > 3)
	{
		fprintf(stderr, "is_double_deletable(): given type is not valid");
		return(0);
	}

	if(UCHARDATA(img)[x]==0) return 0;

	if(is_self_deletable(img, x, rs, N, type))
	{
		for(i=0; i<8; i=i+2)
		{
			v = voisin(x, i, rs, N);
			if(v!=-1)
			{
				if((UCHARDATA(img)[v] != 0) && is_self_deletable(img, v, rs, N, type))
				{
					UCHARDATA(img)[v]=0;
					ok1=simple8((uint8_t*)(img->image_data), x, rs, N);
					UCHARDATA(img)[v]=255;

					if(!ok1)
					{
						UCHARDATA(img)[x]=0;
						ok2=simple8((uint8_t*)(img->image_data), v, rs, N);
						UCHARDATA(img)[x]=255;

						if(!ok2) return 0;
					}
				}
			}
		}
		return 1;
	}

	return 0;
}



//Detects if point x is surrounded by a ring of black points of radius 2 (necessary for i-square deletable points)
// 0 0 0 0
// 0 x * 0
// 0 * * 0
// 0 0 0 0
int32_t is_surrounded_by_radius2_ring(struct xvimage *img, uint32_t x, uint32_t rs, uint32_t N)
{
	if(nonbord(x, rs, N) && nonbord(x+rs, rs, N) && nonbord(x+1, rs, N) && nonbord(x+rs+1, rs, N))
	{
		if( (UCHARDATA(img)[x-1-rs] == 0) &&
			(UCHARDATA(img)[x-rs] == 0) &&
			(UCHARDATA(img)[x+1-rs] == 0) &&
			(UCHARDATA(img)[x+2-rs] == 0) &&
			(UCHARDATA(img)[x-1] == 0) &&
			(UCHARDATA(img)[x+2] == 0) &&
			(UCHARDATA(img)[x-1+rs] == 0) &&
			(UCHARDATA(img)[x+2+rs] == 0) &&
			(UCHARDATA(img)[x-1+2*rs] == 0) &&
			(UCHARDATA(img)[x+2*rs] == 0) &&
			(UCHARDATA(img)[x+1+2*rs] == 0) &&
			(UCHARDATA(img)[x+2+2*rs] == 0) )
		{
			return 1;
		}
	}

	return (0);
}

//Detects i-square-deletable points as defined in "Parallel thinning algorithms based on Ronse's sufficient conditions for topology preservation", by Nemeth and Palagyi
int32_t is_square_deletable(struct xvimage *img, uint32_t x, uint32_t rs, uint32_t N, uint32_t type)
{
	if(type <= 0 || type > 3)
	{
		fprintf(stderr, "is_square_deletable(): given type is not valid");
		return(0);
	}

	if(UCHARDATA(img)[x]==0) return 0;

	if(nonbord(x, rs, N))
	{
		if( is_surrounded_by_radius2_ring(img, x, rs, N) )
		{
			//Test configuration e
			if( (UCHARDATA(img)[x+1]==255) && (UCHARDATA(img)[x+rs+1]==255) && (UCHARDATA(img)[x+rs]==255) )
			{
				return 0;
			}

			//Stop here if type is 2 or 3
			if(type!=1)
			{
				return 1;
			}

			//Test configuration a
			if( (UCHARDATA(img)[x+1]==0) && (UCHARDATA(img)[x+rs+1]==255) && (UCHARDATA(img)[x+rs]==255) )
			{
				return 0;
			}

			//Test configuration b
			if( (UCHARDATA(img)[x+1]==255) && (UCHARDATA(img)[x+rs+1]==255) && (UCHARDATA(img)[x+rs]==0) )
			{
				return 0;
			}

			//Test configuration c
			if( (UCHARDATA(img)[x+1]==255) && (UCHARDATA(img)[x+rs+1]==0) && (UCHARDATA(img)[x+rs]==255) )
			{
				return 0;
			}
		}

		if(type==1)
		{
			//test configuration d
			if( is_surrounded_by_radius2_ring(img, x-1, rs, N) &&
				(UCHARDATA(img)[x-1]==0) &&
				(UCHARDATA(img)[x+rs]==255) &&
				(UCHARDATA(img)[x-1+rs]==255) )
			{
				return 0;
			}
		}

		return 1;
	}

	return 0;
}

/* ==================================== */
int32_t lskelnemethpalagyi(struct xvimage *image,
	     int32_t nsteps,
	     struct xvimage *inhibit,
	     int32_t type)
/* ==================================== */
/*
Voir "Parallel thinning algorithms based on Ronse's sufficient conditions for topology preservation", by Nemeth and Palagyi
*/
#undef F_NAME
#define F_NAME "lskelnemethpalagyi"
{
  int32_t i;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *tmp = copyimage(image);
  uint8_t *T = UCHARDATA(tmp);
  int32_t step, nonstab;

  if (inhibit != NULL)
  {
    fprintf(stderr, "%s: inhibit image: not implemented\n", F_NAME);
    return 0;
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

  step = 0;
  nonstab = 1;
  while (nonstab &&  ((nsteps==-1) || (step < nsteps)))
  {
    nonstab = 0;
    step++;
#ifdef VERBOSE
    printf("step %d\n", step);
#endif

    memset(T, 0, N);
    for (i = 0; i < N; i++)
      if ((F[i] == 255) && (is_double_deletable(image, i, rs, N, type)==1) && (is_square_deletable(image, i, rs, N, type)==1))
      { nonstab = 1; T[i] = 1; }
    for (i = 0; i < N; i++) if (T[i] == 1) F[i] = 0;
  }

#ifdef VERBOSE1
    printf("number of steps: %d\n", step);
#endif


  freeimage(tmp);
  return(1);
} /* lskelnemethpalagyi() */
