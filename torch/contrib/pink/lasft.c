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
   lasft.c
   lhpclosing.c
   lhpopening.c
   lcondhpclosing.c
   lcondhpopening.c
   lasft3d.c
   lhpclosing3d.c
   lhpopening3d.c
   lcondhpclosing3d.c
   lcondhpopening3d.c
*/
/* M. Couprie -- mai 2002 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>
#include <mcgeo.h>
#include <ldist.h>
#include <lhtkern.h>
#include <lhtkern3d.h>
#include <lasft.h>
#include <lmedialaxis.h>
#include <ldilateros.h>
#include <ldilateros3d.h>

#define VERBOSE
#define FERMETURE_EN_1

/* =============================================================== */
/* =============================================================== */
//                              2D
/* =============================================================== */
/* =============================================================== */

/* =============================================================== */
void hpclosing(struct xvimage * image, int32_t nptb, int32_t * tab_es_x, int32_t * tab_es_y, 
                int32_t xc, int32_t yc, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpclosing"
{
  struct xvimage * image_sav;
  struct xvimage * image_cpy;
  uint8_t *I, *S, *C;
  int32_t rs, cs, N, i;

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_cpy = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  C = UCHARDATA(image_cpy);
  if (! ldilat2(image_cpy, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function ldilat2 failed\n", F_NAME);
    exit(0);
  }
  if (!lhthickdelta(image, image_cpy, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }
  memcpy(C, I, N); // copie image dans image_cpy
  if (! leros2(image_cpy, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function leros2 failed\n", F_NAME);
    exit(0);
  }
  // max image_cpy image_sav image_cpy
  for (i = 0; i < N; i++) C[i] = mcmax(C[i],S[i]);
  if (!lhthindelta(image, image_cpy, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }
  freeimage(image_cpy);
  freeimage(image_sav);
} // hpclosing()

/* =============================================================== */
void hpclosingdisc(struct xvimage * image, struct xvimage * dist, int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpclosingdisc" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T;
  int32_t rs, cs, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }
  freeimage(image_tmp);
  freeimage(image_sav);
} // hpclosingdisc()

/* =============================================================== */
void hpopening(struct xvimage * image, int32_t nptb, int32_t * tab_es_x, int32_t * tab_es_y, 
                int32_t xc, int32_t yc, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpopening" 
{
  struct xvimage * image_sav;
  struct xvimage * image_cpy;
  uint8_t *I, *S, *C;
  int32_t rs, cs, N, i;

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_cpy = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  C = UCHARDATA(image_cpy);
  if (! leros2(image_cpy, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function leros2 failed\n", "hpopening");
    exit(0);
  }
  if (!lhthindelta(image, image_cpy, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", "hpopening");
    exit(0);
  }
  memcpy(C, I, N); // copie image dans image_cpy
  if (! ldilat2(image_cpy, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function ldilat2 failed\n", "hpopening");
    exit(0);
  }
  // min image_cpy image_sav image_cpy
  for (i = 0; i < N; i++) C[i] = mcmin(C[i],S[i]);
  if (!lhthickdelta(image, image_cpy, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", "hpopening");
    exit(0);
  }
  freeimage(image_cpy);
  freeimage(image_sav);
} // hpopening()

/* =============================================================== */
void hpopeningdisc(struct xvimage * image, struct xvimage * dist, int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpopeningdisc" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T;
  int32_t rs, cs, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }
  memcpy(T, I, N); // copie image dans image_tmp

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }
  freeimage(image_tmp);
  freeimage(image_sav);
} // hpopeningdisc()

/* =============================================================== */
void hpopeningdisc_bad(struct xvimage * image, struct xvimage * dist, int32_t r, int32_t connex)
/* =============================================================== */
/* juste pour montrer que ce n'est pas la bonne definition */
{
  struct xvimage * image_tmp;
  uint8_t *I, *T;
  int32_t rs, cs, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  T = UCHARDATA(image_tmp);

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // controle topologique pour l'ouverture
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
} // hpopeningdisc_bad()

/* =============================================================== */
void condhpclosing(
  struct xvimage * image, struct xvimage * cond, 
  int32_t nptb, int32_t * tab_es_x, int32_t * tab_es_y, 
  int32_t xc, int32_t yc, int32_t connex)
/* =============================================================== */
// ATTENTION: LA CONTRAINTE DOIT ETRE UN SUR-ENSEMBLE DE L'OBJET 
#undef F_NAME
#define F_NAME "condhpclosing" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, N, i;

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  if (! ldilat2(image_tmp, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function ldilat2 failed\n", F_NAME);
    exit(0);
  }

  // min image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],C[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  if (! leros2(image_tmp, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function leros2 failed\n", F_NAME);
    exit(0);
  }

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpclosing()

/* =============================================================== */
void condhpclosingdisc(
  struct xvimage * image, struct xvimage * cond, struct xvimage * dist, 
  int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "condhpclosingdisc" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // sub image_tmp cond image_tmp
  for (i = 0; i < N; i++) if (C[i]) T[i] = 0;

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpclosingdisc()

/* =============================================================== */
void condhpclosingdisc2(
  struct xvimage * image, struct xvimage * cond, struct xvimage * dist, 
  int32_t r2, int32_t connex)
/* =============================================================== */
// le parametre r2 contient le carre du rayon du disque
#undef F_NAME
#define F_NAME "condhpclosingdisc2" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, N, i;
  uint32_t *D = ULONGDATA(dist);

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // sub image_tmp cond image_tmp
  for (i = 0; i < N; i++) if (C[i]) T[i] = 0;

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpclosingdisc2()

/* =============================================================== */
void condhpopening(
  struct xvimage * image, struct xvimage * cond, 
  int32_t nptb, int32_t * tab_es_x, int32_t * tab_es_y, 
  int32_t xc, int32_t yc, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "condhpopening" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, N, i;

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  if (! leros2(image_tmp, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function leros2 failed\n", F_NAME);
    exit(0);
  }

  // max image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],C[i]);    

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  if (! ldilat2(image_tmp, nptb, tab_es_x, tab_es_y, xc, yc))
  {
    fprintf(stderr, "%s: function ldilat2 failed\n", F_NAME);
    exit(0);
  }

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpopening()

/* =============================================================== */
void condhpopeningdisc(
  struct xvimage * image, struct xvimage * cond, struct xvimage * dist, 
  int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "condhpopeningdisc" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],C[i]);    

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpopeningdisc()

/* =============================================================== */
void condhpopeningdisc2(
  struct xvimage * image, struct xvimage * cond, struct xvimage * dist, 
  int32_t r2, int32_t connex)
/* =============================================================== */
// le parametre r2 contient le carre du rayon du disque
#undef F_NAME
#define F_NAME "condhpopeningdisc2" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, N, i;
  uint32_t *D = ULONGDATA(dist);

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  N = rs * cs;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (connex == 4) connex = 8; else connex = 4; 
  // (pour hthi* la connexite est celle des minima)

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],C[i]);    

  // controle topologique pour l'erosion
  if (!lhthindelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // dilatation
  if (!ldistquad(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpopeningdisc2()


/* =============================================================== */
int32_t lasft_2D3D(struct xvimage * image, struct xvimage * imagec, struct xvimage * imagecc, 
                   int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasft_2D3D" 
{
  if (depth(image) == 1)
  {
    if (! lasft(image, imagec, imagecc, connex, rayonmax))
    {
      fprintf(stderr, "%s: lasft failed\n", F_NAME);
      exit(1);
    }
  }
  else
  {
    if (! lasft3d(image, imagec, imagecc, connex, rayonmax))
    {
      fprintf(stderr, "%s: lasft3d failed\n", F_NAME);
      exit(1);
    }
  }

  return 1;
} /* lasft_2D3D */

/* =============================================================== */
int32_t lasft_2D3D_null(struct xvimage * image,  
                   int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasft_2D3D_null" 
{
  if (!lasft_2D3D(image, NULL, NULL, connex, rayonmax))
  {
    fprintf(stderr, "%s: lasft_2D3D failed\n", F_NAME);
    exit(1);
  }
  
  return 1;
} /* lasft_2D3D */


/* =============================================================== */
int32_t lasft(struct xvimage * image, struct xvimage * imagec, struct xvimage * imagecc, 
          int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasft" 
{
#define RAYONMAXMAX 1000 
  int32_t rayon;
  int32_t rs = rowsize(image);         /* taille ligne */
  int32_t cs = colsize(image);         /* taille colonne */
  struct xvimage *dist;

  dist = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "lasft: allocimage failed\n");
    return 0;
  }

  if (rayonmax != -1)
  {
    for (rayon = 1; rayon <= rayonmax; rayon++)
    {
#ifdef VERBOSE
      fprintf(stderr, "asft: rayon = %d\n", rayon);      
#endif
      if ((imagec != NULL) && (imagecc != NULL))
      {
#ifdef FERMETURE_EN_1
        condhpclosingdisc(image, imagecc, dist, rayon, connex);
        condhpopeningdisc(image, imagec, dist, rayon, connex);
#else
        condhpopeningdisc(image, imagec, dist, rayon, connex);
        condhpclosingdisc(image, imagecc, dist, rayon, connex);
#endif
      }
      else
      {
#ifdef FERMETURE_EN_1
        hpclosingdisc(image, dist, rayon, connex);
        hpopeningdisc(image, dist, rayon, connex);
#else
        hpopeningdisc(image, dist, rayon, connex);
        hpclosingdisc(image, dist, rayon, connex);
#endif
      }
    } /* for (rayon = 1; rayon <= rayonmax; rayon++) */
  }
  else
  {
    struct xvimage *imagepred = copyimage(image);
    rayon = 1;
    do
    {
      copy2image(imagepred, image);
#ifdef VERBOSE
      fprintf(stderr, "asft: rayon = %d\n", rayon);      
#endif  
      if ((imagec != NULL) && (imagecc != NULL))
      {
        condhpclosingdisc(image, imagecc, dist, rayon, connex);
        condhpopeningdisc(image, imagec, dist, rayon, connex);
      }
      else
      {
        hpclosingdisc(image, dist, rayon, connex);
        hpopeningdisc(image, dist, rayon, connex);
      }
      rayon++;
      if (rayon > RAYONMAXMAX)
      {
        fprintf(stderr,"lasft : stability not reached\n");
        break;
      }
    } while (!equalimages(image, imagepred));
  } /* else */

  freeimage(dist);    
  return 1;
} // lasft()

/* =============================================================== */
int32_t lasft_ndg(struct xvimage * image, struct xvimage * imagec, struct xvimage * imagecc, 
          int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ndg" 
{
  int32_t rayon;
  int32_t d, xc, yc, x, y, nptb;
  int32_t *tab_es_x, *tab_es_y;

  d = 2 * rayonmax + 1;
  tab_es_x = (int32_t *)calloc(1,d * d * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,d * d * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }

  if (rayonmax > -1)
  {
    for (rayon = 1; rayon <= rayonmax; rayon++)
    {
      /* genere l'element structurant circulaire sous forme de liste des coord. des points */
      yc = xc = rayon; 
      d = 2 * rayon + 1;
      nptb = 0;
      for (x = 0; x < d; x++)
        for (y = 0; y < d; y++)
          if (((x - xc) * (x - xc) + (y - yc) * (y - yc)) <= (rayon * rayon))
 	  {
            tab_es_x[nptb] = x;
            tab_es_y[nptb] = y;
            nptb++;
 	  }
#ifdef VERBOSE
      fprintf(stderr, "asft_ndg: rayon = %d\n", rayon);      
#endif
      if ((imagec != NULL) && (imagecc != NULL))
      {
#ifdef FERMETURE_EN_1
        condhpclosing(image, imagecc, nptb, tab_es_x, tab_es_y, xc, yc, connex);
        condhpopening(image, imagec, nptb, tab_es_x, tab_es_y, xc, yc, connex);
#else
        condhpopening(image, imagec, nptb, tab_es_x, tab_es_y, xc, yc, connex);
        condhpclosing(image, imagecc, nptb, tab_es_x, tab_es_y, xc, yc, connex);
#endif
      }
      else
      {
#ifdef FERMETURE_EN_1
        hpclosing(image, nptb, tab_es_x, tab_es_y, xc, yc, connex);
        hpopening(image, nptb, tab_es_x, tab_es_y, xc, yc, connex);
#else
        hpopening(image, nptb, tab_es_x, tab_es_y, xc, yc, connex);
        hpclosing(image, nptb, tab_es_x, tab_es_y, xc, yc, connex);
#endif
      }
    } /* for (rayon = 1; rayon <= rayonmax; rayon++) */
  }
  else
  {
    fprintf(stderr, "%s: rayonmax must be positive\n", F_NAME);
    return(0);
  } /* else */

  free(tab_es_x);
  free(tab_es_y);
  return 1;
} // lasft_ndg()

/* =============================================================== */
int32_t lhpclosingdisc(struct xvimage * image, int32_t connex, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lhpclosingdisc" 
{
  int32_t rs = rowsize(image);         /* taille ligne */
  int32_t cs = colsize(image);         /* taille colonne */
  struct xvimage *dist;

  dist = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "lhpclosingdisc: allocimage failed\n");
    return 0;
  }

  hpclosingdisc(image, dist, rayon, connex);

  freeimage(dist);    
  return 1;
} // lhpclosingdisc()

/* =============================================================== */
int32_t lhpopeningdisc(struct xvimage * image, int32_t connex, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lhpopeningdisc" 
{
  int32_t rs = rowsize(image);         /* taille ligne */
  int32_t cs = colsize(image);         /* taille colonne */
  struct xvimage *dist;

  dist = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "lhpopeningdisc: allocimage failed\n");
    return 0;
  }

  hpopeningdisc(image, dist, rayon, connex);

  freeimage(dist);    
  return 1;
} // lhpopeningdisc()


/* =============================================================== */
int32_t lasftmed_2D3D(struct xvimage * image, int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasftmed_2D3D" 
{

  if (depth(image) == 1)
  {
    if (! lasftmed(image, connex, rayonmax))
    {
      fprintf(stderr, "lasftmed failed\n");
      exit(1);
    }
  }
  else
  {
    if (! lasftmed3d(image, connex, rayonmax))
    {
      fprintf(stderr, "lasftmed3d failed\n");
      exit(1);
    }
  }
  return 1;

} /* asftmed_2D3D */

/* =============================================================== */
int32_t lasftmed(struct xvimage * image, int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasftmed" 
{
  int32_t x;
  struct xvimage * medaxis;   // axe median de image
  struct xvimage * medaxis_i; // axe median de l'inverse de image
  uint8_t *M;
  int32_t N = rowsize(image) * colsize(image);

  medaxis = copyimage(image);
  if (! lmedialaxis_lmedialaxisbin(medaxis, 8))
  {
    fprintf(stderr, "lasft: lmedialaxis_lmedialaxisbin failed\n");
    exit(0);
  }
  medaxis_i = copyimage(image);
  M = UCHARDATA(medaxis_i);
  for (x = 0; x < N; x++) M[x] = NDG_MAX - M[x]; // inverse M
  if (! lmedialaxis_lmedialaxisbin(medaxis_i, 8))
  {
    fprintf(stderr, "lasft: lmedialaxis_lmedialaxisbin failed\n");
    exit(0);
  }

  lasft(image, medaxis, medaxis_i, connex, rayonmax);

  freeimage(medaxis);
  freeimage(medaxis_i);

  return 1;
} // lasftmed()

/* =============================================================== */
/* =============================================================== */
//                              3D
/* =============================================================== */
/* =============================================================== */

/* =============================================================== */
void hpclosing3d(struct xvimage *image, int32_t nptb, int32_t *tab_es_x, int32_t *tab_es_y, int32_t *tab_es_z, 
                  int32_t xc, int32_t yc, int32_t zc, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpclosing3d" 
{
  struct xvimage * image_sav;
  struct xvimage * image_cpy;
  uint8_t *I, *S, *C;
  int32_t rs, cs, ds, ps, N, i;

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_cpy = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  C = UCHARDATA(image_cpy);
  if (! ldilatbin3d2(image_cpy, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilatbin3d2 failed\n", F_NAME);
    exit(0);
  }
  if (!lhthickdelta3d(image, image_cpy, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }
  memcpy(C, I, N); // copie image dans image_cpy
  if (! ldilateros3d_lerosbin3d2(image_cpy, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilateros3d_lerosbin3d2 failed\n", F_NAME);
    exit(0);
  }
  // max image_cpy image_sav image_cpy
  for (i = 0; i < N; i++) C[i] = mcmax(C[i],S[i]);
  if (!lhthindelta3d(image, image_cpy, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }
  freeimage(image_cpy);
  freeimage(image_sav);
} // hpclosing3d()

/* =============================================================== */
void hpclosing3dball(struct xvimage *image, struct xvimage * dist, int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpclosing3dball" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T;
  int32_t rs, cs, ds, ps, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);
  // dilatation
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // controle topologique pour la dilatation
  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  // controle topologique pour l'erosion
  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }
  freeimage(image_tmp);
  freeimage(image_sav);
} // hpclosing3dball()

/* =============================================================== */
int32_t lhpclosing3dball(struct xvimage * image, int32_t connex, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lhpclosing3dball" 
{
  int32_t rs = rowsize(image);         /* taille ligne */
  int32_t cs = colsize(image);         /* taille colonne */
  int32_t ds = depth(image);           /* nb plans */
  struct xvimage *dist;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }

  hpclosing3dball(image, dist, rayon, connex);

  freeimage(dist);    
  return 1;
} // lhpclosing3dball()

/* =============================================================== */
void hpopening3d(struct xvimage *image, int32_t nptb, int32_t *tab_es_x, int32_t *tab_es_y, int32_t *tab_es_z, 
                  int32_t xc, int32_t yc, int32_t zc, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpopening3d" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T;
  int32_t rs, cs, ds, ps, N, i;

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);
  if (! ldilateros3d_lerosbin3d2(image_tmp, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilateros3d_lerosbin3d2 failed\n", F_NAME);
    exit(0);
  }
  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }
  memcpy(T, I, N); // copie image dans image_tmp
  if (! ldilatbin3d2(image_tmp, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilatbin3d2 failed\n", F_NAME);
    exit(0);
  }
  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);
  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }
  freeimage(image_tmp);
  freeimage(image_sav);
} // hpopening3d()

/* =============================================================== */
void hpopening3dball(struct xvimage *image, struct xvimage * dist, int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "hpopening3dball" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T;
  int32_t rs, cs, ds, ps, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // controle topologique pour l'erosion
  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }
  memcpy(T, I, N); // copie image dans image_tmp

  // dilatation
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // hpopening3dball()

/* =============================================================== */
int32_t lhpopening3dball(struct xvimage * image, int32_t connex, int32_t rayon)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lhpopening3dball" 
{
  int32_t rs = rowsize(image);         /* taille ligne */
  int32_t cs = colsize(image);         /* taille colonne */
  int32_t ds = depth(image);           /* nb plans */
  struct xvimage *dist;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }

  hpopening3dball(image, dist, rayon, connex);

  freeimage(dist);    
  return 1;
} // lhpopening3dball()

/* =============================================================== */
void condhpclosing3d(
  struct xvimage * image, struct xvimage * cond, 
  int32_t nptb, int32_t * tab_es_x, int32_t * tab_es_y, int32_t * tab_es_z, 
  int32_t xc, int32_t yc, int32_t zc, int32_t connex)
/* =============================================================== */
// ATTENTION: LA CONTRAINTE DOIT ETRE UN SUR-ENSEMBLE DE L'OBJET 
#undef F_NAME
#define F_NAME "condhpclosing3d" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, ds, ps, N, i;

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (! ldilatbin3d2(image_tmp, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilatbin3d2 failed\n", F_NAME);
    exit(0);
  }

  // min image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],C[i]);

  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  if (! ldilateros3d_lerosbin3d2(image_tmp, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilateros3d_lerosbin3d2 failed\n", F_NAME);
    exit(0);
  }

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpclosing3d()

/* =============================================================== */
void condhpclosing3dball(
  struct xvimage * image, struct xvimage * cond, struct xvimage * dist, 
  int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "condhpclosing3dball" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, ds, ps, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  // dilatation
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // sub image_tmp cond image_tmp
  for (i = 0; i < N; i++) if (C[i]) T[i] = 0;

  // controle topologique pour la dilatation
  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],S[i]);

  // controle topologique pour l'erosion
  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpclosing3dball()

/* =============================================================== */
void condhpopening3d(
  struct xvimage * image, struct xvimage * cond, 
  int32_t nptb, int32_t * tab_es_x, int32_t * tab_es_y, int32_t * tab_es_z, 
  int32_t xc, int32_t yc, int32_t zc, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "condhpopening3d" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, ds, ps, N, i;

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  if (! ldilateros3d_lerosbin3d2(image_tmp, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilateros3d_lerosbin3d2 failed\n", F_NAME);
    exit(0);
  }

  // max image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],C[i]);    

  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  if (! ldilatbin3d2(image_tmp, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc))
  {
    fprintf(stderr, "%s: function ldilatbin3d2 failed\n", F_NAME);
    exit(0);
  }

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpopening3d()

/* =============================================================== */
void condhpopening3dball(
  struct xvimage * image, struct xvimage * cond, struct xvimage * dist, 
  int32_t r, int32_t connex)
/* =============================================================== */
#undef F_NAME
#define F_NAME "condhpopening3dball" 
{
  struct xvimage * image_sav;
  struct xvimage * image_tmp;
  uint8_t *I, *S, *T, *C;
  int32_t rs, cs, ds, ps, N, i, r2 = r * r;
  uint32_t *D = ULONGDATA(dist);

  if (connex == 6) connex = 26; else connex = 6; 
  // (pour hthi* la connexite est celle des minima)

  image_sav = copyimage(image);
  image_tmp = copyimage(image);
  rs = rowsize(image);         /* taille ligne */
  cs = colsize(image);         /* taille colonne */
  ds = depth(image);
  ps = rs * cs;                /* taille plan */
  N = ps * ds;                 /* taille image */
  I = UCHARDATA(image);
  C = UCHARDATA(cond);
  S = UCHARDATA(image_sav);
  T = UCHARDATA(image_tmp);

  // erosion
  for(i=0; i<N; i++) if (T[i]) T[i] = NDG_MIN; else T[i] = NDG_MAX;
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MAX; else T[i] = NDG_MIN;

  // max image_tmp cond image_tmp
  for (i = 0; i < N; i++) T[i] = mcmax(T[i],C[i]);    

  // controle topologique pour l'erosion
  if (!lhthindelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthindelta3d failed\n", F_NAME);
    exit(0);
  }

  memcpy(T, I, N); // copie image dans image_tmp

  // dilatation
  if (!ldistquad3d(image_tmp, dist))
  {
    fprintf(stderr, "%s: function ldistquad3d failed\n", F_NAME);
    exit(0);
  }
  for(i=0; i<N; i++) if (D[i] > r2) T[i] = NDG_MIN; else T[i] = NDG_MAX;

  // min image_tmp image_sav image_tmp
  for (i = 0; i < N; i++) T[i] = mcmin(T[i],S[i]);

  // controle topologique pour la dilatation
  if (!lhthickdelta3d(image, image_tmp, -1, connex))
  {
    fprintf(stderr, "%s: function lhthickdelta3d failed\n", F_NAME);
    exit(0);
  }

  freeimage(image_tmp);
  freeimage(image_sav);
} // condhpopening3dball()

/* =============================================================== */
int32_t lasft3d(struct xvimage * image, struct xvimage * imagec, struct xvimage * imagecc, 
            int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasft3d" 
{
#define RAYONMAXMAX3D 300 
  int32_t rayon;
  int32_t rs = rowsize(image);         /* taille ligne */
  int32_t cs = colsize(image);         /* taille colonne */
  int32_t ds = depth(image);
  struct xvimage *dist;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "lasft: allocimage failed\n");
    return 0;
  }

  if (rayonmax != -1)
  {
    for (rayon = 1; rayon <= rayonmax; rayon++)
    {
#ifdef VERBOSE
      fprintf(stderr, "asft3d: rayon = %d\n", rayon);      
#endif
      if ((imagec != NULL) && (imagecc != NULL))
      {
#ifdef FERMETURE_EN_1
        condhpclosing3dball(image, imagecc, dist, rayon, connex);
        condhpopening3dball(image, imagec, dist, rayon, connex);
#else
        condhpopening3dball(image, imagec, dist, rayon, connex);
        condhpclosing3dball(image, imagecc, dist, rayon, connex);
#endif
      }
      else
      {
#ifdef FERMETURE_EN_1
        hpclosing3dball(image, dist, rayon, connex);
        hpopening3dball(image, dist, rayon, connex);
#else
        hpopening3dball(image, dist, rayon, connex);
        hpclosing3dball(image, dist, rayon, connex);
#endif
      }
    } /* for (rayon = 1; rayon <= rayonmax; rayon++) */
  }
  else
  {
    struct xvimage *imagepred = copyimage(image);
    rayon = 1;
    do
    {
      copy2image(imagepred, image);
#ifdef VERBOSE
      fprintf(stderr, "asft3d: rayon = %d\n", rayon);      
#endif  
      if ((imagec != NULL) && (imagecc != NULL))
      {
        condhpclosing3dball(image, imagecc, dist, rayon, connex);
        condhpopening3dball(image, imagec, dist, rayon, connex);
      }
      else
      {
        hpclosing3dball(image, dist, rayon, connex);
        hpopening3dball(image, dist, rayon, connex);
      }
      rayon++;
      if (rayon > RAYONMAXMAX3D)
      {
        fprintf(stderr,"lasft : stability not reached\n");
        break;
      }
    } while (!equalimages(image, imagepred));
  } /* else */

  freeimage(dist);    
  return 1;
} // lasft3d()

/* =============================================================== */
int32_t lasft_ndg3d(struct xvimage * image, struct xvimage * imagec, struct xvimage * imagecc, 
          int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "ndg3d" 
{
  int32_t rayon;
  int32_t d, xc, yc, zc, x, y, z, nptb;
  int32_t *tab_es_x, *tab_es_y, *tab_es_z;

  d = 2 * rayonmax + 1;
  tab_es_x = (int32_t *)calloc(1,d * d * d * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,d * d * d * sizeof(int32_t));
  tab_es_z = (int32_t *)calloc(1,d * d * d * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL) || (tab_es_z == NULL))
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }

  if (rayonmax > -1)
  {
    for (rayon = 1; rayon <= rayonmax; rayon++)
    {
      /* genere l'element structurant circulaire sous forme de liste des coord. des points */
      zc = yc = xc = rayon; 
      d = 2 * rayon + 1;
      nptb = 0;
      for (x = 0; x < d; x++)
        for (y = 0; y < d; y++)
          for (z = 0; z < d; z++)
            if (((x-xc)*(x-xc)+(y-yc)*(y-yc)+(z-zc)*(z-zc)) <= (rayon*rayon))
 	    {
              tab_es_x[nptb] = x;
              tab_es_y[nptb] = y;
              tab_es_z[nptb] = z;
              nptb++;
 	    }
#ifdef VERBOSE
      fprintf(stderr, "lasft_ndg3d: rayon = %d\n", rayon);      
#endif
      if ((imagec != NULL) && (imagecc != NULL))
      {
#ifdef FERMETURE_EN_1
        condhpclosing3d(image, imagecc, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
        condhpopening3d(image, imagec, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
#else
        condhpopening3d(image, imagec, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
        condhpclosing3d(image, imagecc, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
#endif
      }
      else
      {
#ifdef FERMETURE_EN_1
        hpclosing3d(image, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
        hpopening3d(image, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
#else
        hpopening3d(image, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
        hpclosing3d(image, nptb, tab_es_x, tab_es_y, tab_es_z, xc, yc, zc, connex);
#endif
      }
    } /* for (rayon = 1; rayon <= rayonmax; rayon++) */
  }
  else
  {
    fprintf(stderr, "%s: rayonmax must be positive\n", F_NAME);
    return(0);
  } /* else */

  free(tab_es_x);
  free(tab_es_y);
  free(tab_es_z);
  return 1;
} // lasft_ndg3d()

/* =============================================================== */
int32_t lasftmed3d(struct xvimage * image, int32_t connex, int32_t rayonmax)
/* =============================================================== */
#undef F_NAME
#define F_NAME "lasftmed3d" 
{
  struct xvimage * medaxis;   // axe median de image
  struct xvimage * medaxis_i; // axe median de l'inverse de image
  uint8_t *M;
  int32_t x, N = rowsize(image) * colsize(image) * depth(image);

  medaxis = copyimage(image);
  if (! lmedialaxis_lmedialaxisbin(medaxis, 26))
  {
    fprintf(stderr, "%s: lmedialaxis_lmedialaxisbin failed\n", F_NAME);
    exit(0);
  }
  medaxis_i = copyimage(image);
  M = UCHARDATA(medaxis_i);
  for (x = 0; x < N; x++) M[x] = NDG_MAX - M[x]; // inverse M
  if (! lmedialaxis_lmedialaxisbin(medaxis_i, 26))
  {
    fprintf(stderr, "%s: lmedialaxis_lmedialaxisbin failed\n", F_NAME);
    exit(0);
  }

#ifdef VERBOSE
      fprintf(stderr, "%s: medial axes done\n", F_NAME);
#endif

  lasft3d(image, medaxis, medaxis_i, connex, rayonmax);

  freeimage(medaxis);
  freeimage(medaxis_i);

  return 1;
} // lasftmed3d()

