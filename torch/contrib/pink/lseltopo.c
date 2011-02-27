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
/* selection de points selon des criteres topologiques locaux */
/* Michel Couprie - octobre 1997 */

// update janvier 2008 add lsimplepair
// update juillet 2010 modif lptmultiple
// update juillet 2010 add lptjunction

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <mckhalimsky3d.h>
#include <mcskelcurv.h>
#include <lseltopo.h>

#define PARANO

/* ==================================== */
int32_t lptisolated(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptisolated"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if ((nonbord(x, rs, N)) && (typetopobin(SOURCE, x, rs, N) == ISOLE))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if ((nonbord(x, rs, N)) && (typetopobin8(SOURCE, x, rs, N) == ISOLE))
        RES[x] = NDG_MAX;
  }
  else if (connex == 6)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && (mctopo3d_nbvoiso6(SOURCE, x, rs, ps, N) == 0))
        RES[x] = NDG_MAX;
  }
  else if (connex == 18)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && (mctopo3d_nbvoiso18(SOURCE, x, rs, ps, N) == 0))
        RES[x] = NDG_MAX;
  }
  else if (connex == 26)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && (mctopo3d_nbvoiso26(SOURCE, x, rs, ps, N) == 0))
        RES[x] = NDG_MAX;
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptisolated() */

/* ==================================== */
int32_t lptinterior(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptinterior"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if ((nonbord(x, rs, N)) && (typetopobin(SOURCE, x, rs, N) == INTERIEUR))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if ((nonbord(x, rs, N)) && (typetopobin8(SOURCE, x, rs, N) == INTERIEUR))
        RES[x] = NDG_MAX;
  }
  else if (connex == 6)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && (mctopo3d_nbvoiso6(SOURCE, x, rs, ps, N) == 6))
        RES[x] = NDG_MAX;
  }
  else if (connex == 18)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && (mctopo3d_nbvoiso18(SOURCE, x, rs, ps, N) == 18))
        RES[x] = NDG_MAX;
  }
  else if (connex == 26)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && (mctopo3d_nbvoiso26(SOURCE, x, rs, ps, N) == 26))
        RES[x] = NDG_MAX;
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptinterior() */

/* ==================================== */
int32_t lptmultiple(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptmultiple"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */
  int32_t t, tb;

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if ((nonbord(x, rs, N)) && (typetopobin(SOURCE, x, rs, N) == MULTIPLE))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if ((nonbord(x, rs, N)) && (typetopobin8(SOURCE, x, rs, N) == MULTIPLE))
        RES[x] = NDG_MAX;
/*
  ancienne version : 
      if ((nonbord(x, rs, N)) && SOURCE[x] && (nbtrans8(SOURCE, x, rs, N) > 2))
        RES[x] = NDG_MAX;
*/
  }
  else if (connex == 6)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top6(SOURCE, x, rs, ps, N, &t, &tb);
        if (t > 2) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 26)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top26(SOURCE, x, rs, ps, N, &t, &tb);
        if (t > 2) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 18)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top18(SOURCE, x, rs, ps, N, &t, &tb);
        if (t > 2) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptmultiple() */

/* ==================================== */
int32_t lptend(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptend"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord(x, rs, N)) && (nbvois4(SOURCE, x, rs, N) == 1))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord(x, rs, N)) && (nbvois8(SOURCE, x, rs, N) == 1))
        RES[x] = NDG_MAX;
  }
  else if (connex == 6)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord3d(x, rs, ps, N)) && (mctopo3d_nbvoiso6(SOURCE, x, rs, ps, N) == 1))
        RES[x] = NDG_MAX;
  }
  else if (connex == 18)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord3d(x, rs, ps, N)) && (mctopo3d_nbvoiso18(SOURCE, x, rs, ps, N) == 1))
        RES[x] = NDG_MAX;
  }
  else if (connex == 26)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord3d(x, rs, ps, N)) && (mctopo3d_nbvoiso26(SOURCE, x, rs, ps, N) == 1))
        RES[x] = NDG_MAX;
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }

  for (x = 0; x < N; x++)
    SOURCE[x] = RES[x];

  free(RES);
  return 1;
} /* lptend() */

/* ==================================== */
int32_t lptcurve(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptcurve"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N) && curve4(SOURCE, x, rs, N))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N) && curve8(SOURCE, x, rs, N))
        RES[x] = NDG_MAX;
  }
  else if (connex == 6)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && mctopo3d_curve6(SOURCE, x, rs, ps, N))
        RES[x] = NDG_MAX;
    mctopo3d_termine_topo3d();
  }
  else if (connex == 18)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && mctopo3d_curve18(SOURCE, x, rs, ps, N))
        RES[x] = NDG_MAX;
    mctopo3d_termine_topo3d();
  }
  else if (connex == 26)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N) && mctopo3d_curve26(SOURCE, x, rs, ps, N))
        RES[x] = NDG_MAX;
    mctopo3d_termine_topo3d();
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptcurve() */

/* ==================================== */
int32_t lptjunction(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptjunction"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord(x, rs, N)) && (nbvois4(SOURCE, x, rs, N) >= 3))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord(x, rs, N)) && (nbvois8(SOURCE, x, rs, N) >= 3))
        RES[x] = NDG_MAX;
  }
  else if (connex == 6)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord3d(x, rs, ps, N)) && (mctopo3d_nbvoiso6(SOURCE, x, rs, ps, N) >= 3))
        RES[x] = NDG_MAX;
  }
  else if (connex == 18)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord3d(x, rs, ps, N)) && (mctopo3d_nbvoiso18(SOURCE, x, rs, ps, N) >= 3))
        RES[x] = NDG_MAX;
  }
  else if (connex == 26)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && (nonbord3d(x, rs, ps, N)) && (mctopo3d_nbvoiso26(SOURCE, x, rs, ps, N) >= 3))
        RES[x] = NDG_MAX;
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }

  for (x = 0; x < N; x++)
    SOURCE[x] = RES[x];

  free(RES);
  return 1;
} /* lptjunction() */

/* ==================================== */
int32_t lptseparatinggray(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptseparatinggray"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N) && separant4(SOURCE, x, rs, N))
        RES[x] = NDG_MAX;
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N) && separant8(SOURCE, x, rs, N))
        RES[x] = NDG_MAX;
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptseparatinggray() */

/* ==================================== */
int32_t lpthseparatinggray(struct xvimage * image, int32_t connex, int32_t h)
/* ==================================== */
/*
An h-separating point for an image F is a point p, 
such that there exists c verifying F(p)-h < c <= F(p) and
#CC(X inter N(p)) > 1, with X = {x | F(x) < c}.
 */
#undef F_NAME
#define F_NAME "lpthseparatinggray"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 6)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N) && mctopo3d_hfseparant6(SOURCE, x, SOURCE[x]-h, rs, ps, N))
        RES[x] = NDG_MAX;
    mctopo3d_termine_topo3d();
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lpthseparatinggray() */

/* ==================================== */
int32_t lptseparating(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptseparating"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */
  int32_t t, tb;

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N))
      {
        top4(SOURCE, x, rs, N, &t, &tb);
        if (tb >= 2) RES[x] = NDG_MAX;
      }
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N))
      {
        top8(SOURCE, x, rs, N, &t, &tb);
        if (tb >= 2) RES[x] = NDG_MAX;
      }
  }
  else if (connex == 6)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top6(SOURCE, x, rs, ps, N, &t, &tb);
        if (tb >= 2) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 18)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top18(SOURCE, x, rs, ps, N, &t, &tb);
        if (tb >= 2) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 26)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top26(SOURCE, x, rs, ps, N, &t, &tb);
        if (tb >= 2) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptseparating() */

/* ==================================== */
int32_t lptsimple(struct xvimage * image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lptsimple"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */
  int32_t t, tb;

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N))
      {
        top4(SOURCE, x, rs, N, &t, &tb);
        if ((t == 1) && (tb == 1)) RES[x] = NDG_MAX;
      }
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N))
      {
        top8(SOURCE, x, rs, N, &t, &tb);
        if ((t == 1) && (tb == 1)) RES[x] = NDG_MAX;
      }
  }
  else if (connex == 6)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        if (mctopo3d_simple6(SOURCE, x, rs, ps, N)) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 18)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        if (mctopo3d_simple18(SOURCE, x, rs, ps, N)) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 26)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        if (mctopo3d_simple26(SOURCE, x, rs, ps, N)) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lptsimple() */

/* ==================================== */
int32_t lseltopo(struct xvimage * image, int32_t connex, int32_t tm, int32_t tp, int32_t tbm, int32_t tbp)
/* ==================================== */
#undef F_NAME
#define F_NAME "lseltopo"
{
  int32_t x;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */
  int32_t t, tb;

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (connex == 4)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N))
      {
        top4(SOURCE, x, rs, N, &t, &tb);
        if ((t >= tm) && (t <= tp) && (tb >= tbm) && (tb <= tbp)) RES[x] = NDG_MAX;
      }
  }
  else if (connex == 8)
  {
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord(x, rs, N))
      {
        top8(SOURCE, x, rs, N, &t, &tb);
        if ((t >= tm) && (t <= tp) && (tb >= tbm) && (tb <= tbp)) RES[x] = NDG_MAX;
      }
  }
  else if (connex == 6)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top6(SOURCE, x, rs, ps, N, &t, &tb);
        if ((t >= tm) && (t <= tp) && (tb >= tbm) && (tb <= tbp)) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 18)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top18(SOURCE, x, rs, ps, N, &t, &tb);
        if ((t >= tm) && (t <= tp) && (tb >= tbm) && (tb <= tbp)) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else if (connex == 26)
  {
    mctopo3d_init_topo3d();
    for (x = 0; x < N; x++)
      if (SOURCE[x] && nonbord3d(x, rs, ps, N))
      {
        mctopo3d_top26(SOURCE, x, rs, ps, N, &t, &tb);
        if ((t >= tm) && (t <= tp) && (tb >= tbm) && (tb <= tbp)) RES[x] = NDG_MAX;
      }
    mctopo3d_termine_topo3d();
  }
  else
  {   fprintf(stderr,"%s: bad connexity: %d\n", F_NAME, connex);
      return 0;
  }
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lseltopo() */

/* ==================================== */
int32_t lsimplepair(struct xvimage * image, uint32_t onepair)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsimplepair"
{
  int32_t x, y, z, p, p1;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  for (z = 1; z < ds-1; z++)
    for (y = 1; y < cs-1; y++)
      for (x = 1; x < rs-2; x++)
      {
	p = z*ps + y*rs + x; p1 = z*ps + y*rs + x+1;
	if (SOURCE[p] && SOURCE[p1])
	{
	  if (pairesimple_26_x(SOURCE, p, rs, ps, N)) 
	  {
	    RES[p] = RES[p1] = NDG_MAX;
	    if (onepair) goto fin;
	  }
	}
      }

  for (z = 1; z < ds-1; z++)
    for (y = 1; y < cs-2; y++)
      for (x = 1; x < rs-1; x++)
      {
	p = z*ps + y*rs + x; p1 = z*ps + (y+1)*rs + x;
	if (SOURCE[p] && SOURCE[p1])
	{
	  if (pairesimple_26_y(SOURCE, p, rs, ps, N))
	  {
	    RES[p] = RES[p1] = NDG_MAX;
	    if (onepair) goto fin;
	  }
	}
      }

  for (z = 1; z < ds-2; z++)
    for (y = 1; y < cs-1; y++)
      for (x = 1; x < rs-1; x++)
      {
	p = z*ps + y*rs + x; p1 = (z+1)*ps + y*rs + x;
	if (SOURCE[p] && SOURCE[p1])
	{
	  if (pairesimple_26_z(SOURCE, p, rs, ps, N))
	  {
	    RES[p] = RES[p1] = NDG_MAX;
	    if (onepair) goto fin;
	  }
	}
      }

 fin:
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lsimplepair() */

/* ==================================== */
int32_t lminimalsimplepair(struct xvimage * image, uint32_t onepair)
/* ==================================== */
#undef F_NAME
#define F_NAME "lminimalsimplepair"
{
  int32_t x, y, z, p, p1;
  uint8_t *SOURCE = UCHARDATA(image);
  uint8_t *RES;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;          /* taille plan */
  int32_t N = ps * ds;           /* taille image */

  RES = (uint8_t *)calloc(N, sizeof(char));
  if (RES == NULL)
  {   fprintf(stderr,"%s: malloc failed for RES\n", F_NAME);
      return 0;
  }

  mctopo3d_init_topo3d();

  for (z = 1; z < ds-1; z++)
    for (y = 1; y < cs-1; y++)
      for (x = 1; x < rs-2; x++)
      {
	p = z*ps + y*rs + x; p1 = z*ps + y*rs + x+1;
	if (SOURCE[p] && SOURCE[p1] && 
	    !mctopo3d_simple26(SOURCE, p, rs, ps, N) && !mctopo3d_simple26(SOURCE, p1, rs, ps, N))
	{
	  if (pairesimple_26_x(SOURCE, p, rs, ps, N)) 
	  {
	    RES[p] = RES[p1] = NDG_MAX;
	    if (onepair) goto fin;
	  }
	}
      }

  for (z = 1; z < ds-1; z++)
    for (y = 1; y < cs-2; y++)
      for (x = 1; x < rs-1; x++)
      {
	p = z*ps + y*rs + x; p1 = z*ps + (y+1)*rs + x;
	if (SOURCE[p] && SOURCE[p1] && 
	    !mctopo3d_simple26(SOURCE, p, rs, ps, N) && !mctopo3d_simple26(SOURCE, p1, rs, ps, N))
	{
	  if (pairesimple_26_y(SOURCE, p, rs, ps, N))
	  {
	    RES[p] = RES[p1] = NDG_MAX;
	    if (onepair) goto fin;
	  }
	}
      }

  for (z = 1; z < ds-2; z++)
    for (y = 1; y < cs-1; y++)
      for (x = 1; x < rs-1; x++)
      {
	p = z*ps + y*rs + x; p1 = (z+1)*ps + y*rs + x;
	if (SOURCE[p] && SOURCE[p1] && 
	    !mctopo3d_simple26(SOURCE, p, rs, ps, N) && !mctopo3d_simple26(SOURCE, p1, rs, ps, N))
	{
	  if (pairesimple_26_z(SOURCE, p, rs, ps, N))
	  {
	    RES[p] = RES[p1] = NDG_MAX;
	    if (onepair) goto fin;
	  }
	}
      }

 fin:
  mctopo3d_termine_topo3d();
  for (x = 0; x < N; x++) SOURCE[x] = RES[x];
  free(RES);
  return 1;
} /* lminimalsimplepair() */
