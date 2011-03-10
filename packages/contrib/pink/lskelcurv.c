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
/* analyse de squelettes curvilignes */
/* 
   Michel Couprie - june 2004 

   Update 2010:
   - limage2skel: modifications
   - limage2skel2
   - skeleton filtering
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <mclifo.h>
#include <mcutil.h>
#include <mcskelcurv.h>
#include <llabelextrema.h>
#include <lseltopo.h>
#include <lmoments.h>

//#define VERBOSE
//#define DEBUG_lskelfilter3
//#define DEBUG_lskelfilter2b
//#define DEBUGCOMPVECT
//#define DEBUGCURV
//#define DEBUGADJ
//#define DEBUGPOINT
//#define DEBUGDRAW
//#define DEBUG

#ifdef DEBUGPOINT
static uint32_t nptsisol=0;
static uint32_t nptsend=0;
static uint32_t nptscurv=0;
static uint32_t nptsjunc=0;
#endif
#ifdef DEBUGADJ
static uint32_t nadjisol=0;
static uint32_t nadjend=0;
static uint32_t nadjcurv=0;
static uint32_t nadjjunc=0;
#endif

#define VAL_ISOL 1
#define VAL_END  2
#define VAL_CURV 3
#define VAL_JUNC 4

#define MAXANGLE (M_PI/4)

/* ==================================== */
static double dist3(double x1, double y1, double z1, double x2, double y2, double z2)
/* ==================================== */
{
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

/* ==================================== */
static double scalarprod(double x1, double y1, double z1, double x2, double y2, double z2)
/* ==================================== */
{
  return (x1 * x2) + (y1 * y2) + (z1 * z2);
}

/* ==================================== */
static double norm(double x, double y, double z)
/* ==================================== */
{
  return sqrt((x * x) + (y * y) + (z * z));
}

/* ====================================================================== */
static void processend(skel *S, uint8_t *T1, uint8_t *T2, uint8_t *T3, int32_t *M,
		uint32_t start, uint32_t pos, uint32_t i)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "processend"
  int32_t m = start + pos; 

#ifdef DEBUGPOINT
  printf("%s: i = %d,%d\n", F_NAME, i%S->rs, i/S->rs);
#endif

  M[i] = m;
  S->tskel[m].adj = NULL; // les relations d'adjacence seront 
                                        // traitées plus tard
#ifdef DEBUGPOINT
  nptsend++;
  printf("add point end %d nb %d\n", i, nptsend);
#endif
  addptslist(S, m, i);
  //  S->tskel[m].pts = skeladdptcell(S, i, NULL);
} // processend()

/* ====================================================================== */
static void trouve2voisins(int32_t i, int32_t rs, int32_t ps, int32_t N, int32_t connex, uint8_t *F, int32_t *v1, int32_t *v2)
/* ====================================================================== */
// retourne dans v1 et v2 les 2 voisins de i qui sont des points objet (F)
{
#undef F_NAME
#define F_NAME "trouve2voisins"
  int32_t j, k, n = 0;
  switch (connex)
  {
  case 4:
    for (k = 0; k < 8; k += 2)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 8:
    for (k = 0; k < 8; k += 1)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 6:
    for (k = 0; k <= 10; k += 2)
    {
      j = voisin6(i, k, rs, ps, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 18:
    for (k = 0; k < 18; k += 1)
    {
      j = voisin18(i, k, rs, ps, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  case 26:
    for (k = 0; k < 26; k += 1)
    {
      j = voisin26(i, k, rs, ps, N);
      if ((j != -1) && F[j])
      { if (n == 0) *v1 = j; else *v2 = j; n++; }
    } // for k
    break;
  default:
    fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
    exit(0);
  } // switch (connex)
  if (n != 2) printf("%s: error n = %d != 2; point %d %d %d\n", F_NAME, n, 
		     i % rs, (i % ps) / rs, i / ps);
  assert(n == 2);
} // trouve2voisins()

/* ====================================================================== */
static int32_t trouve1voisin(int32_t i, int32_t rs, int32_t ps, int32_t N, int32_t connex, uint8_t *F)
/* ====================================================================== */
// retourne un voisin de i qui est un point objet (F), ou -1 si non trouvé
{
#undef F_NAME
#define F_NAME "trouve1voisin"
  int32_t j, k = 0;
  switch (connex)
  {
  case 4:
    for (k = 0; k < 8; k += 2)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 8:
    for (k = 0; k < 8; k += 1)
    {
      j = voisin(i, k, rs, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 6:
    for (k = 0; k <= 10; k += 2)
    {
      j = voisin6(i, k, rs, ps, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 18:
    for (k = 0; k < 18; k += 1)
    {
      j = voisin18(i, k, rs, ps, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  case 26:
    for (k = 0; k < 26; k += 1)
    {
      j = voisin26(i, k, rs, ps, N);
      if ((j != -1) && F[j]) return j;
    } // for k
    break;
  default:
    fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
    exit(0);
  } // switch (connex)
  return -1;
} // trouve1voisin()

/* ====================================================================== */
static void scancurve(int32_t i, struct xvimage *image, int32_t connex, uint8_t *T1, uint8_t *T2, uint8_t *T3, int32_t *length, int32_t *nbend, int32_t *nbjunc)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "scancurve"
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t j, k, jj, kk, le=1, ne=0, nj=0;

#ifdef DEBUGPOINT
  printf("%s: i=%d,%d,%d\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif

  assert((T2[i] == NDG_MAX) && !T1[i]);
  T2[i] = 1;
  trouve2voisins(i, rs, ps, N, connex, F, &jj, &kk);
  // suit la branche jj
  i = jj;
  while(1)
  {
    if (T1[i]) { ne++; break; }
    if (T3[i]) { nj++; break; }
    assert((T2[i] == NDG_MAX) && !T1[i]);
    T2[i] = 1;
    le++;
    trouve2voisins(i, rs, ps, N, connex, F, &j, &k);
    if (T2[j] == 1) i = k; else i = j;
    if (T2[i] == 1) break; // courbe fermée
  } // while(1)

  // suit la branche kk
  i = kk;
  while(1)
  {
    if (T2[i] == 1) break; // courbe fermée
    if (T1[i]) { ne++; break; }
    if (T3[i]) { nj++; break; }
    assert((T2[i] == NDG_MAX) && !T1[i]);
    T2[i] = 1;
    le++;
    trouve2voisins(i, rs, ps, N, connex, F, &j, &k);
    if (T2[j] == 1) i = k; else i = j;
    assert(T2[i] != 1);
  } // while(1)

  *length = le;
  *nbend = ne;
  *nbjunc = nj;

#ifdef DEBUGCURV
  printf("%s: return\n", F_NAME);
#endif

} // scancurve()

/* ====================================================================== */
static void deletecurve(int32_t i, struct xvimage *image, int32_t connex, uint8_t *T1, uint8_t *T2, uint8_t *T3)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "deletecurve"
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t jj, kk;

#ifdef DEBUGPOINT
  printf("%s: i=%d,%d,%d\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif
  assert(T2[i] == 1);
  T2[i] = F[i] = 0;
  trouve2voisins(i, rs, ps, N, connex, F, &jj, &kk);
  // suit la branche jj
  i = jj;
  while(1)
  {
    if (T1[i]) { T1[i] = T2[i] = F[i] = 0; break; }
    if (T3[i]) break;
    assert(T2[i] == 1);
    T2[i] = F[i] = 0;
    i = trouve1voisin(i, rs, ps, N, connex, F);
    if (i == -1) break;
  } // while(1)

  // suit la branche kk
  i = kk;
  while(1)
  {
    if (T1[i]) { T1[i] = T2[i] = F[i] = 0; break; }
    if (T3[i]) break;
    assert(T2[i] == 1);
    T2[i] = F[i] = 0;
    i = trouve1voisin(i, rs, ps, N, connex, F);
    if (i == -1) break;
  } // while(1)
} // deletecurve()

/* ====================================================================== */
static void curve2junction(int32_t i, struct xvimage *image, int32_t connex, uint8_t *T2, uint8_t *T3)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "curve2junction"
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);
  int32_t N = ds * ps;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t j, k, jj, kk;

#ifdef DEBUGPOINT
  printf("%s: i=%d,%d,%d\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif
  assert(T2[i] == 1);
  T2[i] = 0; T3[i] = NDG_MAX;
  trouve2voisins(i, rs, ps, N, connex, F, &jj, &kk);
  // suit la branche jj
  i = jj;
  while(1)
  {
    if (T2[i] != 1) break;
    T2[i] = 0; T3[i] = NDG_MAX;
    trouve2voisins(i, rs, ps, N, connex, F, &j, &k);
    if (T2[j] != 1) i = k; else i = j;
  } // while(1)

  // suit la branche kk
  i = kk;
  while(1)
  {
    if (T2[i] != 1) break;
    T2[i] = 0; T3[i] = NDG_MAX;
    trouve2voisins(i, rs, ps, N, connex, F, &j, &k);
    if (T2[j] != 1) i = k; else i = j;
  } // while(1)
} // curve2junction()

/* ====================================================================== */
static void processcurv(skel *S, uint8_t *F, uint8_t *T1, uint8_t *T2, uint8_t *T3, int32_t *M,
		uint32_t start, uint32_t pos, uint32_t i)
/* ====================================================================== */
// ATTENTION : tous les autres types de points doivent avoir été traités avant (structure M)
// T2 contient les points de courbe proprement dit (2 voisins) et les points extrémités (1 voisin).
// après exécution, tous les points traités sont marqués '3' dans T2
{
#undef F_NAME
#define F_NAME "processcurv"
  int32_t j, k, oldi = -1;
  int32_t rs = S->rs, cs = S->cs, ds = S->ds, ps = rs * cs, N = ps * ds;
  int32_t trouve = 0, closed = 0, incr_vois, connex = S->connex;

#ifdef DEBUGCURV
  printf("%s: initial point i = %d,%d,%d\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif

  if ((connex == 4) || (connex == 8))
  {
    if (ds != 1)
    {   
      fprintf(stderr,"%s: connexity 4 or 8 not defined for 3D\n", F_NAME);
      exit(0);
    }
    if (connex == 4) incr_vois = 2; else incr_vois = 1;
  }

  // 1ere étape: cherche une extrémité si elle existe
  // l'extrémité est hors de la courbe : point "end" ou "junc"
  while (!trouve)
  {
    if (T1[i] || T3[i]) { trouve = 1; }
    else // "vrai" point de courbe (non extrémité)
    {
      trouve2voisins(i, rs, ps, N, connex, F, &j, &k);
      T2[i] = 2;
      if (T2[j] == 2) 
      {
	if (T2[k] == 2) trouve = closed = 1; else i = k;
      }
      else i = j;
    }    
  } // while (!trouve)

#ifdef DEBUGPOINT
  printf("%s: extremite i = %d,%d,%d\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif

  // 2eme étape: parcourt la courbe et enregistre les infos
  if (closed) 
  {
    int32_t m = start + pos;
    S->tskel[m].adj = NULL;
    S->tskel[m].pts = NULL;

    assert(T2[i]);
#ifdef DEBUGPOINT
    nptscurv++;
    printf("add 1 point curv %d,%d nb %d\n", m, i, nptscurv);
#endif
    addptslist(S, m, i);
    T2[i] = 3;
#ifdef DEBUGPOINT
  printf("%s: closed : point i = %d,%d,%d traité\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif
    trouve = 0;
    while (!trouve)
    {
      trouve2voisins(i, rs, ps, N, connex, F, &j, &k);
      if (T2[j] != 2) j = k; 
      if (T2[j] != 2) { trouve = 1; break; }
      T2[j] = 3;
#ifdef DEBUGPOINT
      printf("%s: closed : point j = %d,%d,%d traité\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
#endif
#ifdef DEBUGPOINT
      nptscurv++;
      printf("add 2 point curv %d,%d nb %d\n", m, j, nptscurv);
#endif
      addptslist(S, m, j);
      i = j;
    } // while (!trouve)
  }
  else // courbe ouverte
  {
    int32_t m = start + pos; 
    S->tskel[m].adj = NULL;
    S->tskel[m].pts = NULL;

    if (T3[i]) // extrémité "junc"
    {
#ifdef DEBUGADJ
      nadjcurv++;
      printf("add 4 adj curv %d,%d nb %d\n", m, M[i], nadjcurv);
#endif
      addadjlist(S, m, M[i]);
      T2[i] = 3; // marque le point extrémité même si c'est une jonction
      oldi = i;
#ifdef DEBUGADJ
      printf("%s: courbe %d adj junc %d\n", F_NAME, m, M[i]);
#endif
    }
    else  // extrémité "end"
    {
      assert(T1[i] && T2[i]); 

#ifdef DEBUGADJ
      nadjcurv++;
      printf("add 6 adj curv %d,%d nb %d\n", m, M[i], nadjcurv);
#endif
      addadjlist(S, m, M[i]);
      T2[i] = 3;
#ifdef DEBUGPOINT
      printf("%s: point i = %d,%d traité\n", F_NAME, i%S->rs, i/S->rs);
#endif
#ifdef DEBUGADJ
      printf("%s: courbe %d adj end %d\n", F_NAME, m, M[i]);
#endif
    }

    // trouve le premier "vrai point de courbe" adjacent à l'extrémité i
    if (T1[i]) // extrémité "end"
    {
      if ((connex == 4) || (connex == 8))
	for (k = 0; k < 8; k += incr_vois)
	  { j = voisin(i, k, rs, N); if ((j != -1) && F[j]) break; }
      else if (connex == 6)
	for (k = 0; k < 10; k += 2)
	  { j = voisin6(i, k, rs, ps, N); if ((j != -1) && F[j]) break; }
      else if (connex == 18)
	for (k = 0; k < 18; k += 1)
	  { j = voisin18(i, k, rs, ps, N); if ((j != -1) && F[j]) break; }
      else if (connex == 26)
	for (k = 0; k < 26; k += 1)
	  { j = voisin26(i, k, rs, ps, N); if ((j != -1) && F[j]) break; }
#ifdef DEBUGPOINT
      nptscurv++;
      printf("add 7a point curv %d,%d nb %d\n", m, i, nptscurv);
#endif
      addptslist(S, m, i);
    }
    else // T3[i], extrémité "junc"
    {
      if ((connex == 4) || (connex == 8))
	for (k = 0; k < 8; k += incr_vois)
	  { j = voisin(i, k, rs, N); if ((j != -1) && (T2[j]==2)) break; }
      else if (connex == 6)
	for (k = 0; k < 10; k += 2)
	  { j = voisin6(i, k, rs, ps, N); if ((j != -1) && (T2[j]==2)) break; }
      else if (connex == 18)
	for (k = 0; k < 18; k += 1)
	  { j = voisin18(i, k, rs, ps, N); if ((j != -1) && (T2[j]==2)) break; }
      else if (connex == 26)
	for (k = 0; k < 26; k += 1)
	  { j = voisin26(i, k, rs, ps, N); if ((j != -1) && (T2[j]==2)) break; }
#ifdef DEBUGCURV
      printf("extremite junc, point i=%d (%d,%d,%d)\n", i, i%rs, (i%ps)/rs, i/ps);
#endif
    }
    assert(F[j] != 0);

    i = j; // premier point de la courbe
    while ((!T1[i]) && (!T3[i]))
    {      
      T2[i] = 3;
#ifdef DEBUGPOINT
      printf("%s: point i = %d,%d,%d traité\n", F_NAME, i%rs, (i%ps)/rs, i/ps);
      nptscurv++;
      printf("add 7 point curv %d,%d nb %d\n", m, i, nptscurv);
#endif
      addptslist(S, m, i);
      trouve2voisins(i, rs, ps, N, connex, F, &j, &k);

#ifdef DEBUGPOINT
      printf("voisins: %d [%d,%d,%d] %d [%d,%d,%d] \n", 
	     j, T1[j], T2[j], T3[j], k, T1[k], T2[k], T3[k]);
#endif

      if ((T2[j] == 3) && ((T2[k] != 3) || T3[k] || T1[k])) i = k; 
      else if ((T2[k] == 3) && ((T2[j] != 3) || T3[j] || T1[j])) i = j; 
      else if (T2[k] != 3) i = k; 
      else if (T2[j] != 3) i = j; 
      else assert(0);
    } // while (!trouve)

    // traite dernier point
    if (T3[i]) 
    {
      //      addptslist(S, m, i);
#ifdef DEBUGADJ
      nadjcurv++;
      printf("add 8 adj curv %d,%d nb %d\n", m, M[i], nadjcurv);
#endif
      addadjlist(S, m, M[i]);
#ifdef DEBUGPOINT
      printf("%s: point i = %d,%d traité\n", F_NAME, i%S->rs, i/S->rs);
#endif
#ifdef DEBUGADJ
      printf("%s: courbe %d adj junc %d\n", F_NAME, m, M[i]);
#endif
    }
    else
    {
      T2[i] = 3;
#ifdef DEBUGPOINT
      nptscurv++;
      printf("add 9 point curv %d,%d nb %d\n", m, i, nptscurv);
#endif
      addptslist(S, m, i);
#ifdef DEBUGADJ
      nadjcurv++;
      printf("add 10 adj curv %d,%d nb %d\n", m, M[i], nadjcurv);
#endif
      addadjlist(S, m, M[i]);
#ifdef DEBUGPOINT
      printf("%s: point i = %d,%d traité\n", F_NAME, i%S->rs, i/S->rs);
#endif
#ifdef DEBUGADJ
      printf("%s: courbe %d adj end %d\n", F_NAME, m, M[i]);
#endif
    }
    if (oldi != -1) T2[oldi] = 0;
  } // courbe ouverte
} // processcurv()

/* ====================================================================== */
static void processjunc(skel *S, uint8_t *T1, uint8_t *T2, uint8_t *T3, int32_t *M,
		uint32_t start, uint32_t pos, uint32_t i)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "processjunc"
  int32_t j, k;
  int32_t rs = S->rs, cs = S->cs, ds = S->ds, ps = rs * cs, N = ps * ds;
  Lifo * LIFO;
  int32_t incr_vois, connex = S->connex;
  int32_t m = start + pos; 

#ifdef DEBUGPOINT
  printf("%s: i = %d,%d ; m = %d\n", F_NAME, i%S->rs, i/S->rs, m);
#endif

  if ((connex == 4) || (connex == 8))
  {
    if (ds != 1)
    {   
      fprintf(stderr,"%s: connexity 4 or 8 not defined for 3D\n", F_NAME);
      exit(0);
    }
    if (connex == 4) incr_vois = 2; else incr_vois = 1;
  }

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   
    fprintf(stderr,"%s: CreeLifoVide failed\n", F_NAME);
    exit(0);
  }

  LifoPush(LIFO, i); 
  T3[i] = 2;

  S->tskel[m].adj = NULL;
  S->tskel[m].pts = NULL;

  while (! LifoVide(LIFO))
  {
    i = LifoPop(LIFO);

    // traiter point
    M[i] = m;     // enregistre i
#ifdef DEBUGPOINT
    nptsjunc++;
    printf("add point junc %d,%d nb %d\n", m, i, nptsjunc);
#endif
    addptslist(S, m, i);

#ifdef DEBUGPOINT
  printf("%s: traite point i = %d,%d\n", F_NAME, i%S->rs, i/S->rs);
#endif

    if ((connex == 4) || (connex == 8))
      for (k = 0; k < 8; k += incr_vois)
      {
	j = voisin(i, k, rs, N);
	if ((j != -1) && (T3[j] == NDG_MAX)) { LifoPush(LIFO, j); T3[j] = 2; }
      } /* for k */
    else if (connex == 6)
      for (k = 0; k < 10; k += 2)
      {
	j = voisin6(i, k, rs, ps, N);
	if ((j != -1) && (T3[j] == NDG_MAX)) { LifoPush(LIFO, j); T3[j] = 2; }
      } /* for k */
    else if (connex == 18)
      for (k = 0; k < 18; k += 1)
      {
	j = voisin18(i, k, rs, ps, N);
	if ((j != -1) && (T3[j] == NDG_MAX)) { LifoPush(LIFO, j); T3[j] = 2; }
      } /* for k */
    else if (connex == 26)
      for (k = 0; k < 26; k += 1)
      {
	j = voisin26(i, k, rs, ps, N);
	if ((j != -1) && (T3[j] == NDG_MAX)) { LifoPush(LIFO, j); T3[j] = 2; }
      } /* for k */

  } /* while ! LifoVide */

  LifoTermine(LIFO);
} // processjunc()

/* ====================================================================== */
static int32_t is_end(int32_t x, uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex)
/* ====================================================================== */
{
  switch (connex)
  {
  case 4:
    if (nbvois4(F, x, rs, N) == 1) return 1; else return 0;
  case 8:
    if (nbvois8(F, x, rs, N) == 1) return 1; else return 0;
  case 6:
    if (mctopo3d_nbvoiso6(F, x, rs, ps, N) == 1) return 1; else return 0;
  case 18:
    if (mctopo3d_nbvoiso18(F, x, rs, ps, N) == 1) return 1; else return 0;
  case 26:
    if (mctopo3d_nbvoiso26(F, x, rs, ps, N) == 1) return 1; else return 0;
  default: assert(0);
  }
} // is_end()

/* ====================================================================== */
static int32_t is_simple(int32_t x, uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex)
/* ====================================================================== */
{
  int32_t t, tb;
  switch (connex)
  {
  case 4:
    top4(F, x, rs, N, &t, &tb);
    if ((t == 1) && (tb == 1)) return 1; else return 0;
  case 8:
    top8(F, x, rs, N, &t, &tb);
    if ((t == 1) && (tb == 1)) return 1; else return 0;
  case 6:
    if (mctopo3d_simple6(F, x, rs, ps, N)) return 1; else return 0;
  case 18:
    if (mctopo3d_simple18(F, x, rs, ps, N)) return 1; else return 0;
  case 26:
    if (mctopo3d_simple26(F, x, rs, ps, N)) return 1; else return 0;
  default: assert(0);
  }
} // is_simple()

/* ====================================================================== */
static int32_t tailleptliste(SKC_pt_pcell p)
/* ====================================================================== */
{
  int32_t n = 0;
  for (; p != NULL; p = p->next) n++;
  return n;
} /* tailleptliste() */

#ifdef NOTUSED
/* ====================================================================== */
static int32_t tailleadjliste(SKC_adj_pcell p)
/* ====================================================================== */
{
  int32_t n = 0;
  for (; p != NULL; p = p->next) n++;
  return n;
} /* tailleadjliste() */
#endif

/* ========================================== */
int32_t lskelcurv_extractcurve(
  uint8_t *B,        // entrée/sortie : pointeur base image
  int32_t i,         // entrée : index du point de départ
  int32_t rs,        // entrée : taille rangee
  int32_t N,         // entrée : taille image
  int32_t connex,    // entrée : 4 ou 8
  int32_t ** X,      // sortie : points
  int32_t ** Y,
  int32_t * npoints) // sortie : nombre de points
/* ========================================== */
// extrait de l'image B la courbe débutant au point extrémité i
{
#undef F_NAME
#define F_NAME "lskelcurv_extractcurve"
  int32_t n = 0;     // compte le nombre de points
  int32_t v1, v2, ii, jj;

  ii = i;
  assert(is_end(ii, B, rs, 1, N, connex)); n++;
  jj = trouve1voisin(ii, rs, 1, N, connex, B); n++;
  while (!is_end(jj, B, rs, 1, N, connex))
  {
    trouve2voisins(jj, rs, 1, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
  }
  *npoints = n;
  *X = (int32_t *)malloc(n * sizeof(int32_t)); assert(*X != NULL); 
  *Y = (int32_t *)malloc(n * sizeof(int32_t)); assert(*Y != NULL); 
  n = 0;
  ii = i;
  *X[n] = ii % rs;
  *Y[n] = ii / rs;
  jj = trouve1voisin(ii, rs, 1, N, connex, B); n++;
  *X[n] = jj % rs;
  *Y[n] = jj / rs;
  while (!is_end(jj, B, rs, 1, N, connex))
  {
    trouve2voisins(jj, rs, 1, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
    *X[n] = jj % rs;
    *Y[n] = jj / rs;
  }  
  return 1;
} // lskelcurv_extractcurve()

/* ========================================== */
int32_t lskelcurv_extractcurve3d(
  uint8_t *B,        // entrée/sortie : pointeur base image
  int32_t i,         // entrée : index du point de départ
  int32_t rs,        // entrée : taille rangee
  int32_t ps,        // entrée : taille plan
  int32_t N,         // entrée : taille image
  int32_t connex,    // entrée : 6, 18 ou 26
  int32_t ** X,      // sortie : points
  int32_t ** Y,
  int32_t ** Z,
  int32_t * npoints) // sortie : nombre de points
/* ========================================== */
// extrait de l'image B la courbe débutant au point extrémité i
{
#undef F_NAME
#define F_NAME "lskelcurv_extractcurve3d"
  int32_t n = 0;     // compte le nombre de points
  int32_t v1, v2, ii, jj;

  ii = i;
  assert(is_end(ii, B, rs, ps, N, connex)); n++;
  jj = trouve1voisin(ii, rs, ps, N, connex, B); n++;
  while (!is_end(jj, B, rs, ps, N, connex))
  {
    trouve2voisins(jj, rs, ps, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
  }
  *npoints = n;
  *X = (int32_t *)malloc(n * sizeof(int32_t)); assert(*X != NULL); 
  *Y = (int32_t *)malloc(n * sizeof(int32_t)); assert(*Y != NULL); 
  *Z = (int32_t *)malloc(n * sizeof(int32_t)); assert(*Z != NULL); 
  n = 0;
  ii = i;
  *X[n] = ii % rs;
  *Y[n] = (ii % ps) / rs;
  *Z[n] = ii / ps;
  jj = trouve1voisin(ii, rs, ps, N, connex, B); n++;
  *X[n] = jj % rs;
  *Y[n] = (jj % ps) / rs;
  *Z[n] = jj / ps;
  while (!is_end(jj, B, rs, ps, N, connex))
  {
    trouve2voisins(jj, rs, ps, N, connex, B, &v1, &v2);
    if (v1 == ii) { ii = jj; jj = v2; } else { ii = jj; jj = v1; } 
    n++;
    *X[n] = jj % rs;
    *Y[n] = (jj % ps) / rs;
    *Z[n] = jj / ps;
  }  
  return 1;
} // lskelcurv_extractcurve3d()

/* ====================================================================== */
skel * limage2skel(struct xvimage *image, int32_t connex, int32_t len)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "limage2skel"
  int32_t i, j, x, y, z;
  int32_t rs, cs, ds, ps, N;
  uint8_t *F; /* l'image de depart */
  int32_t *M; // carte des parties de squelette
  struct xvimage *temp0, *temp1, *temp2, *temp3;
  struct xvimage *lab2=NULL, *lab3=NULL;
  uint8_t *T0, *T1, *T2, *T3;
  int32_t nbisol, nbend, nbcurv, nbjunc, nbvertex, nbpoints;
  int32_t length, ne, nj, ret;
  skel * S;
  SKC_adj_pcell p;

  assert(image != NULL);
  assert(datatype(image) == VFF_TYP_1_BYTE);
  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);
  N = ds * ps;             /* taille image */
  F = UCHARDATA(image);

#ifdef DEBUG
  printf("%s: begin\n", F_NAME);
#endif

  // filtre les bords de l'image
  if (ds > 1)
  {
    for (x = 0; x < rs; x++)
    for (y = 0; y < cs; y++) 
      F[0 * ps + y * rs + x] = 0;          /* plan z = 0 */
    for (x = 0; x < rs; x++)
    for (y = 0; y < cs; y++) 
      F[(ds-1) * ps + y * rs + x] = 0;     /* plan z = ds-1 */

    for (x = 0; x < rs; x++)
    for (z = 0; z < ds; z++) 
      F[z * ps + 0 * rs + x] = 0;          /* plan y = 0 */
    for (x = 0; x < rs; x++)
    for (z = 0; z < ds; z++) 
      F[z * ps + (cs-1) * rs + x] = 0;     /* plan y = cs-1 */

    for (y = 0; y < cs; y++)
    for (z = 0; z < ds; z++) 
      F[z * ps + y * rs + 0] = 0;          /* plan x = 0 */
    for (y = 0; y < cs; y++)
    for (z = 0; z < ds; z++) 
      F[z * ps + y * rs + (rs-1)] = 0;     /* plan x = rs-1 */
  }
  else
  {
    for (x = 0; x < rs; x++) F[x] = 0;
    for (x = 0; x < rs; x++) F[(cs - 1) * rs + x] = 0;

    for (y = 1; y < cs - 1; y++) F[y * rs] = 0;
    for (y = 1; y < cs - 1; y++) F[y * rs + rs - 1] = 0;
  }

  // detection des differents types de points
  temp0 = copyimage(image); assert(temp0);
  temp1 = copyimage(image); assert(temp1);
  temp2 = copyimage(image); assert(temp2);
  temp3 = copyimage(image); assert(temp3);
  T0 = UCHARDATA(temp0);
  T1 = UCHARDATA(temp1);
  T2 = UCHARDATA(temp2);
  T3 = UCHARDATA(temp3);
  M = (int32_t *)calloc(N, sizeof(int32_t)); assert (M);
  for (i = 0; i < N; i++) M[i] = -1;
  ret = lptisolated(temp0, connex); assert(ret);
  ret = lptend(temp1, connex); assert(ret);
  ret = lptcurve(temp2, connex); assert(ret);
  ret = lptsimple(temp3, connex); assert(ret);

  for (i = 0; i < N; i++)
    if (T3[i] && !T1[i])
    {
      fprintf(stderr, "%s: input image is not a curvilinear skeleton %d\n", F_NAME, i);
      return NULL;
    }
      
  copy2image(temp3, image);

  // detection des points de jonction (T3) par complementation
  // réalise de plus l'union de T2 et de T1 (résulat dans T2)
  for (i = 0; i < N; i++) 
  {
    if (F[i])
    {
      if (T0[i]) { T3[i] = 0; } else
      if (T1[i]) { T3[i] = 0; T2[i] = NDG_MAX; } else
      if (T2[i]) T3[i] = 0; else
      if (T3[i]) T3[i] = NDG_MAX;
    }
  } // for (i = 0; i < N; i++) 

  if (len != INT32_MAX)
  {
#ifdef DEBUG
    printf("ELIMINATION PETITES COURBES\n");
#endif
    mctopo3d_init_topo3d();

    // élimination des courbes de longueur inférieure à len
    for (i = 0; i < N; i++) 
    {
      if (T0[i] && (len > 0)) F[i] = T0[i] = 0; // points isolés
      if (T1[i] && (len > 1)) // branches de longueur 1
      {
	j = trouve1voisin(i, rs, ps, N, connex, F);
	assert(j != -1);
	if (T3[j]) F[i] = T1[i] = T2[i] = 0;
      } 
      if ((T2[i] == NDG_MAX) && !T1[i])
      {
	scancurve(i, image, connex, T1, T2, T3, &length, &ne, &nj); // etiquette à 1
	if (length < len)
	{
	  if (((ne == 0) && (nj == 0)) || (ne > 0))
	    deletecurve(i, image, connex, T1, T2, T3);
	  else 
	    curve2junction(i, image, connex, T2, T3);
	}
      }
    } // for (i = 0; i < N; i++) 
    for (i = 0; i < N; i++) if (T2[i]) T2[i] = NDG_MAX;

    do { // retire itérativement les points simples non end
      nbpoints = 0;
      for (i = 0; i < N; i++)
	if (F[i] && !is_end(i, F, rs, ps, N, connex) && is_simple(i, F, rs, ps, N, connex))
	{
	  nbpoints++;
	  F[i] = 0;
	}
    } while (nbpoints);

#ifdef RECARAC
    // Re-caractérisation des points
    // Une jonction qui n'est adjacente qu'à deux courbes est recaractérisée courbe
    // Une jonction qui n'est adjacente qu'à une courbe est recaractérisée extrémité
    // Une jonction qui n'est adjacente à aucune courbe est recaractérisée isolé

    for (i = 0; i < N; i++) 
    {
      if (T3[i] == NDG_MAX)
      {
	nc = scanjunction(i, image, connex, T2, T3); // etiquette à 1
	if (nc == 2) junction2curve(i, image, connex, T2, T3);
	else if (nc == 1) junction2end(i, image, connex, T1, T2, T3);
	else if (nc == 0) junction2isol(i, image, connex, T0, T3);
      }
    } // for (i = 0; i < N; i++) 
    for (i = 0; i < N; i++) if (T3[i]) T3[i] = NDG_MAX;
#endif

    mctopo3d_termine_topo3d();
  } // if (len != INT32_MAX)

  // comptage des points isoles et extremites et ...
  nbisol = nbend = nbcurv = nbjunc = nbpoints = 0;
  for (i = 0; i < N; i++) 
  {
    if (F[i])
    {
      nbpoints++;
      if (T0[i]) nbisol++;
      if (T1[i]) nbend++;
      if (T2[i]) nbcurv++;
      if (T3[i]) nbjunc++;
    }
  } // for (i = 0; i < N; i++) 

  // etiquetage des courbes et des jonctions
  
  if (nbcurv > 0)
  {
    nbcurv = 0;
    lab2 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE); assert(lab2 != NULL);

    if (! llabelextrema(temp2, connex, 0, lab2, &nbcurv))
    {
      fprintf(stderr, "%s: function llabelextrema failed\n", F_NAME);
      return NULL;
    }
    nbcurv -= 1;
#ifdef DEBUG
    printf("%s: llabelextrema done\n", F_NAME);
    writeimage(lab2, "_labelcurv");
#endif
  }

  if (nbjunc > 0)
  {
    nbjunc = 0;
    lab3 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE); assert(lab3 != NULL);
    if (! llabelextrema(temp3, connex, 0, lab3, &nbjunc))
    {
      fprintf(stderr, "%s: function llabelextrema failed\n", F_NAME);
      return NULL;
    }
    nbjunc -= 1;
#ifdef DEBUG
    printf("%s: llabelextrema done\n", F_NAME);
    writeimage(lab3, "_labeljunc");
#endif
  }

#ifdef VERBOSE
  printf("nb isol : %d ; nb end : %d ; nb curves : %d ; nb junctions : %d\n", nbisol, nbend, nbcurv, nbjunc);
#endif

  // construction de la structure "squelette"
  nbvertex = nbisol + nbend + nbcurv + nbjunc; 
#ifdef DEBUG
  printf("initskel nbvertex %d, nbpoints %d, nbcurv %d, nbend %d, adjcells %d\n", 
	 nbvertex, nbpoints, nbcurv, nbend, 2*nbcurv + nbvertex * (nbvertex-1));
#endif
  S = initskel(rs, cs, ds, nbvertex, 
	       nbpoints + nbpoints, // end points are both in curv and end vertices
	       2*nbcurv + nbvertex * (nbvertex-1), 
	       connex);
  if (S == NULL)
  {
    fprintf(stderr, "%s: function initskel failed\n", F_NAME);
    return NULL;
  }

  S->e_isol = nbisol;
  S->e_end  = nbisol + nbend;
  S->e_curv = nbisol + nbend + nbcurv;
  S->e_junc = nbisol + nbend + nbcurv + nbjunc;

  nbisol = nbend = nbjunc = 0;
  for (i = 0; i < N; i++) 
  {
    if (T0[i])
    {
#ifdef DEBUGPOINT
      nptsisol++;
      printf("add point isol %d,%d nb %d\n", nbisol, i, nptsisol);
#endif
      addptslist(S, nbisol, i);
      M[i] = nbisol;
      nbisol++;
    }
    else if (T1[i])
    {
      processend(S, T1, T2, T3, M, S->e_isol, nbend, i);
      nbend++;
    }
    else if (T3[i] == NDG_MAX)
    {
      processjunc(S, T1, T2, T3, M, S->e_curv, nbjunc, i);
      nbjunc++;
    }
  } // for i

  nbcurv = 0;
  for (i = 0; i < N; i++) 
  {
    if (T2[i] == NDG_MAX)
    {
      processcurv(S, F, T1, T2, T3, M, S->e_end, nbcurv, i);
      nbcurv++;
    }
  } // for i

  // ajoute les relations d'adjacence réciproques
  for (i = S->e_end; i < S->e_curv; i++) 
    for (p = S->tskel[i].adj; p != NULL; p = p->next)
    {
#ifdef DEBUGADJ
      nadjcurv++;
      printf("add adj curv %d,%d nb %d\n", p->val, i, nadjcurv);
#endif
      addadjlist(S, p->val, i);
    }

  // ménage
  free(M);
  if (lab2) free(lab2);
  if (lab3) free(lab3);
  free(temp0);
  free(temp1);
  free(temp2);
  free(temp3);
  /*olena a ecrit */
  return S;
} /* limage2skel() */

/* ====================================================================== */
skel * limage2skel2(struct xvimage *image, struct xvimage *morejunctions, int32_t connex)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "limage2skel2"
  int32_t i, x, y, z;
  int32_t rs, cs, ds, ps, N;
  uint8_t *F; /* l'image de depart */
  uint8_t *J; /* contient des "points de jonctions" artificiels */
  int32_t *M; // carte des parties de squelette
  struct xvimage *temp0, *temp1, *temp2, *temp3;
  struct xvimage *lab2=NULL, *lab3=NULL;
  uint8_t *T0, *T1, *T2, *T3;
  int32_t nbisol, nbend, nbcurv, nbjunc, nbvertex, nbpoints;
  int32_t ret;
  skel * S;
  SKC_adj_pcell p;

  assert(image != NULL);
  assert(datatype(image) == VFF_TYP_1_BYTE);
  rs = rowsize(image);     /* taille ligne */
  cs = colsize(image);     /* taille colonne */
  ps = rs * cs;            /* taille plan */
  ds = depth(image);
  N = ds * ps;             /* taille image */
  assert(morejunctions != NULL);
  assert(datatype(morejunctions) == VFF_TYP_1_BYTE);
  assert(rowsize(morejunctions) == rs);
  assert(colsize(morejunctions) == cs);
  assert(depth(morejunctions) == ds);
  F = UCHARDATA(image);
  J = UCHARDATA(morejunctions);

#ifdef DEBUG
  printf("%s: begin\n", F_NAME);
#endif

  // filtre les bords de l'image
  if (ds > 1)
  {
    for (x = 0; x < rs; x++)
    for (y = 0; y < cs; y++) 
      F[0 * ps + y * rs + x] = 0;          /* plan z = 0 */
    for (x = 0; x < rs; x++)
    for (y = 0; y < cs; y++) 
      F[(ds-1) * ps + y * rs + x] = 0;     /* plan z = ds-1 */

    for (x = 0; x < rs; x++)
    for (z = 0; z < ds; z++) 
      F[z * ps + 0 * rs + x] = 0;          /* plan y = 0 */
    for (x = 0; x < rs; x++)
    for (z = 0; z < ds; z++) 
      F[z * ps + (cs-1) * rs + x] = 0;     /* plan y = cs-1 */

    for (y = 0; y < cs; y++)
    for (z = 0; z < ds; z++) 
      F[z * ps + y * rs + 0] = 0;          /* plan x = 0 */
    for (y = 0; y < cs; y++)
    for (z = 0; z < ds; z++) 
      F[z * ps + y * rs + (rs-1)] = 0;     /* plan x = rs-1 */
  }
  else
  {
    for (x = 0; x < rs; x++) F[x] = 0;
    for (x = 0; x < rs; x++) F[(cs - 1) * rs + x] = 0;

    for (y = 1; y < cs - 1; y++) F[y * rs] = 0;
    for (y = 1; y < cs - 1; y++) F[y * rs + rs - 1] = 0;
  }

  // detection des differents types de points
  temp0 = copyimage(image); assert(temp0);
  temp1 = copyimage(image); assert(temp1);
  temp2 = copyimage(image); assert(temp2);
  temp3 = copyimage(image); assert(temp3);
  T0 = UCHARDATA(temp0);
  T1 = UCHARDATA(temp1);
  T2 = UCHARDATA(temp2);
  T3 = UCHARDATA(temp3);
  M = (int32_t *)calloc(N, sizeof(int32_t)); assert (M);
  for (i = 0; i < N; i++) M[i] = -1;
  ret = lptisolated(temp0, connex); assert(ret);
  ret = lptend(temp1, connex); assert(ret);
  ret = lptcurve(temp2, connex); assert(ret);
  ret = lptsimple(temp3, connex); assert(ret);

  for (i = 0; i < N; i++)
    if (T3[i] && !T1[i])
    {
      fprintf(stderr, "%s: input image is not a curvilinear skeleton\n", F_NAME);
      return NULL;
    }
      
  copy2image(temp3, image);

  // detection des points de jonction (T3) par complementation
  // réalise de plus l'union de T2 et de T1 (résulat dans T2)
  for (i = 0; i < N; i++) 
  {
    if (F[i])
    {
      if (T0[i]) { T3[i] = 0; } else
      if (T1[i]) { T3[i] = 0; T2[i] = NDG_MAX; } else
      if (T2[i])
      { 
	if (J[i]) T2[i] = 0; else T3[i] = 0; 
      } else
      if (T3[i] || J[i]) T3[i] = NDG_MAX;
    }
  } // for (i = 0; i < N; i++) 

#ifdef DEBUG
  writeimage(temp0, "_temp0");
  writeimage(temp1, "_temp1");
  writeimage(temp2, "_temp2");
  writeimage(temp3, "_temp3");
#endif

  // comptage des points isoles et extremites et ...
  nbisol = nbend = nbcurv = nbjunc = nbpoints = 0;
  for (i = 0; i < N; i++) 
  {
    if (F[i])
    {
      nbpoints++;
      if (T0[i]) nbisol++;
      if (T1[i]) nbend++;
      if (T2[i]) nbcurv++;
      if (T3[i]) nbjunc++;
    }
  } // for (i = 0; i < N; i++) 

  // etiquetage des courbes et des jonctions
  
  if (nbcurv > 0)
  {
    nbcurv = 0;
    lab2 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE); assert(lab2 != NULL);

    if (! llabelextrema(temp2, connex, 0, lab2, &nbcurv))
    {
      fprintf(stderr, "%s: function llabelextrema failed\n", F_NAME);
      return NULL;
    }
    nbcurv -= 1;
#ifdef DEBUG
    printf("%s: llabelextrema done\n", F_NAME);
    writeimage(lab2, "_labelcurv");
#endif
  }

  if (nbjunc > 0)
  {
    nbjunc = 0;
    lab3 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE); assert(lab3 != NULL);
    if (! llabelextrema(temp3, connex, 0, lab3, &nbjunc))
    {
      fprintf(stderr, "%s: function llabelextrema failed\n", F_NAME);
      return NULL;
    }
    nbjunc -= 1;
#ifdef DEBUG
    printf("%s: llabelextrema done\n", F_NAME);
    writeimage(lab3, "_labeljunc");
#endif
  }

#ifdef VERBOSE
  printf("nb isol : %d ; nb end : %d ; nb curves : %d ; nb junctions : %d\n", nbisol, nbend, nbcurv, nbjunc);
#endif

  // construction de la structure "squelette"
  nbvertex = nbisol + nbend + nbcurv + nbjunc; 
#ifdef DEBUG
  printf("initskel nbvertex %d, nbpoints %d, nbcurv %d, nbend %d, adjcells %d\n", 
	 nbvertex, nbpoints, nbcurv, nbend, 2*nbcurv + nbvertex * (nbvertex-1));
#endif
  S = initskel(rs, cs, ds, nbvertex, 
	       nbpoints + nbpoints, // end points are both in curv and end vertices
	       2*nbcurv + nbvertex * (nbvertex-1), connex);
  if (S == NULL)
  {
    fprintf(stderr, "%s: function initskel failed\n", F_NAME);
    return NULL;
  }

  S->e_isol = nbisol;
  S->e_end  = nbisol + nbend;
  S->e_curv = nbisol + nbend + nbcurv;
  S->e_junc = nbisol + nbend + nbcurv + nbjunc;

  nbisol = nbend = nbjunc = 0;
  for (i = 0; i < N; i++) 
  {
    if (T0[i])
    {
#ifdef DEBUGPOINT
      nptsisol++;
      printf("add point isol %d,%d nb %d\n", nbisol, i, nptsisol);
#endif
      addptslist(S, nbisol, i);
      M[i] = nbisol;
      nbisol++;
    }
    else if (T1[i])
    {
      processend(S, T1, T2, T3, M, S->e_isol, nbend, i);
      nbend++;
    }
    else if (T3[i] == NDG_MAX)
    {
      processjunc(S, T1, T2, T3, M, S->e_curv, nbjunc, i);
      nbjunc++;
    }
  } // for i

  nbcurv = 0;
  for (i = 0; i < N; i++) 
  {
    if (T2[i] == NDG_MAX)
    {
      processcurv(S, F, T1, T2, T3, M, S->e_end, nbcurv, i);
      nbcurv++;
    }
  } // for i

  // ajoute les relations d'adjacence réciproques
  for (i = S->e_end; i < S->e_curv; i++) 
    for (p = S->tskel[i].adj; p != NULL; p = p->next)
    {
#ifdef DEBUGADJ
      nadjcurv++;
      printf("add adj curv %d,%d nb %d\n", p->val, i, nadjcurv);
#endif
      addadjlist(S, p->val, i);
    }

  // ménage
  free(M);
  if (lab2) free(lab2);
  if (lab3) free(lab3);
  free(temp0);
  free(temp1);
  free(temp2);
  free(temp3);

  return S;
} /* limage2skel2() */

/* ====================================================================== */
struct xvimage * lskel2image(skel *S, int32_t id)
/* ====================================================================== */
// if id == -1 then output all skeleton elements. 
// otherwise output element id.
{
#undef F_NAME
#define F_NAME "lskel2image"
  int32_t i;
  int32_t rs = S->rs;
  int32_t cs = S->cs;
  int32_t ds = S->ds;
  int32_t N = ds * cs * rs;
  uint8_t *F;      /* l'image de depart */
  SKC_pt_pcell p;
  struct xvimage * image;

  image = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if (image == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return NULL;
  }
  F = UCHARDATA(image);      /* l'image de depart */
  memset(F, 0, N);

  if (id == -1)
  {
    for (i = S->e_curv; i < S->e_junc; i++)
    {
      for (p = S->tskel[i].pts; p != NULL; p = p->next) 
	F[p->val] = VAL_JUNC;
    }
    for (i = S->e_end; i < S->e_curv; i++)
    {
      for (p = S->tskel[i].pts; p != NULL; p = p->next) 
      {
	if (S->tskel[i].tag)
	  F[p->val] = VAL_ISOL;
	else
	  F[p->val] = VAL_CURV;
      }
    }
    for (i = S->e_isol; i < S->e_end; i++)
    {
      for (p = S->tskel[i].pts; p != NULL; p = p->next) 
	F[p->val] = VAL_END;
    }
    for (i = 0; i < S->e_isol; i++)
    {
      for (p = S->tskel[i].pts; p != NULL; p = p->next) 
	F[p->val] = VAL_ISOL;
    }
  }
  else
    for (p = S->tskel[id].pts; p != NULL; p = p->next)
      F[p->val] = NDG_MAX;

  return image;
} /* lskel2image() */

/* ====================================================================== */
struct xvimage * lskelmarked2image(skel *S)
/* ====================================================================== */
// only the marked items are transfered
{
#undef F_NAME
#define F_NAME "lskelmarked2image"
  int32_t i;
  int32_t rs = S->rs;
  int32_t cs = S->cs;
  int32_t ds = S->ds;
  int32_t N = ds * cs * rs;
  uint8_t *F;      /* l'image de depart */
  SKC_pt_pcell p;
  struct xvimage * image;

  image = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if (image == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return NULL;
  }
  F = UCHARDATA(image);      /* l'image de depart */
  memset(F, 0, N);

  for (i = 0; i < S->e_junc; i++)
  {
    if (S->tskel[i].tag)
      for (p = S->tskel[i].pts; p != NULL; p = p->next) 
	F[p->val] = NDG_MAX;
  }

  return image;
} /* lskelmarked2image() */

/* ====================================================================== */
static void coordvertex(skel *S, int32_t V, double *Vx, double *Vy, double *Vz)
/* ====================================================================== */
// computes the coordinates (Vx, Vy, Vz) of vertex V in S 
{
  int32_t rs = S->rs;
  int32_t cs = S->cs;
  int32_t ps = rs*cs;
  int32_t n = 0;
  double x=0.0, y=0.0, z=0.0;
  SKC_pt_pcell p = S->tskel[V].pts;

  assert(p != NULL);
  for (; p != NULL; p = p->next)
  {
    x += (double)(p->val % rs);
    y += (double)((p->val % ps) / rs);
    z += (double)(p->val / ps);
    n += 1;
  }  
  *Vx = x/n; *Vy = y/n; *Vz = z/n; 
} // coordvertex()

/* ====================================================================== */
static double distancetoskel(skel *S, double x, double y, double z)
/* ====================================================================== */
//
{
  int32_t rs = S->rs;
  int32_t cs = S->cs;
  int32_t ps = rs*cs;
  int32_t i;
  double xx, yy, zz, d, min = (double)(ps * S->ds);
  SKC_pt_pcell p;

  for (i = 0; i < S->e_junc; i++)
  {
    for (p = S->tskel[i].pts; p != NULL; p = p->next)
    {
      xx = (double)(p->val % rs);
      yy = (double)((p->val % ps) / rs);
      zz = (double)(p->val / ps);
      d = dist3(xx, yy, zz, x, y, z);
      if (d < min) min = d;
    }
  }
  return min;
} // distancetoskel()

/* ====================================================================== */
int32_t lskelmarkvertex(skel *S, int32_t vertex_id)
/* ====================================================================== */
// Mark the specified vertex
{
#undef F_NAME
#define F_NAME "lskelmarkvertex"
  int32_t i;

  if (vertex_id < 0) return 0;
  if (vertex_id >= S->e_junc) return 0;

  for (i = 0; i < S->e_junc; i++)
    S->tskel[i].tag = 0; // not marked

  S->tskel[vertex_id].tag = 1; // marked
  return 1;
} /* lskelmarkvertex() */

/* ====================================================================== */
int32_t lskelfilter1_old(skel *S, double length, double angle)
/* ====================================================================== */
/*
The skeleton found in S is searched for "small" branches which 
satisfies the following criteria:
\li Extremities A, B are both junctions.
\li Branch length (AB) is less than \b length parameter.
\li Let AA be the symmetric of A wrt B, and BB be the symmetric of B wrt A.
    The distance from AA to the skeleton is more than AB sin( \b angle ), or
    the distance from BB to the skeleton is more than AB sin( \b angle ).

The matching branches are marked (field "tag" = 1)

Parameter \b length is given in pixels, parameter \b angle in radians.
*/
{
#undef F_NAME
#define F_NAME "lskelfilter1_old"
  int32_t i, A, B;
  SKC_adj_pcell p;
  double AB, Ax, Ay, Az, Bx, By, Bz, AAx, AAy, AAz, BBx, BBy, BBz;
  double dAA, dBB;

#ifdef DEBUG
  printf("lskelfilter1: length = %g, angle %g, l sin a %g\n", 
	 length, angle, length * sin(angle));
#endif	  

  for (i = S->e_end; i < S->e_curv; i++)
  {
    S->tskel[i].tag = 0; // not marked
    p = S->tskel[i].adj;
    A = p->val;
    assert(p->next != NULL);
    p = p->next;
    B = p->val;
    assert(p->next == NULL);
    if (IS_JUNC(A) && IS_JUNC(B))
    {
      coordvertex(S, A, &Ax, &Ay, &Az);
      coordvertex(S, B, &Bx, &By, &Bz);
      AB = dist3(Ax, Ay, Az, Bx, By, Bz);
#ifdef DEBUG
      printf("arc %d, (%g,%g,%g)-(%g,%g,%g), length %g\n", 
	     i, Ax, Ay, Az, Bx, By, Bz, AB);
#endif	  
      if (AB <= length)
      {
	// symétrique de A par rapport à B
	AAx = Bx + Bx - Ax; BBy = By + By - Ay; BBz = Bz + Bz - Az; 
	// symétrique de B par rapport à A
	BBx = Ax + Ax - Bx; AAy = Ay + Ay - By; AAz = Az + Az - Bz; 
	dAA = distancetoskel(S, AAx, AAy, AAz);
	dBB = distancetoskel(S, BBx, BBy, BBz);

#ifdef DEBUG
	printf("dAA = %g, dBB = %g, AB * sin(angle) = %g\n", 
	       dAA, dBB, AB * sin(angle));
#endif	  
	
	if ((dAA >= (AB * sin(angle))) || (dBB >= (AB * sin(angle))))
	{
	  S->tskel[i].tag = 1; // mark for deletion
#ifdef DEBUG
	  printf("mark %d\n", i);
#endif	  
	}
      } // if (AB >= length)
    } // if (IS_JUNC(A) && IS_JUNC(B))
  } // for (i = S->e_end; i < S->e_curv; i++)

  return 1;
} /* lskelfilter1_old() */

/* ====================================================================== */
static void points_at_head(skel *S, int32_t Ai, double delta, int32_t *e, int32_t *f)
/* ====================================================================== */
// détermine deux points e et f au "début" de l'arc de courbe Ai, 
// séparés d'une distance euclidienne d'au moins delta
{
  int32_t rs = S->rs;
  int32_t ps = rs * S->cs;
  SKC_pt_pcell p = S->tskel[Ai].pts;
  double x, y, z, xx, yy, zz;

  assert(p != NULL);
  *e = p->val;
  if (p->next == NULL)
  {
    *f = p->val;
#ifdef VERBOSE
    printf("Warning: arc %d length 1\n", Ai);
#endif    
    return;
  }
  x = (double)(*e % rs);
  y = (double)((*e % ps) / rs);
  z = (double)(*e / ps);
  for (p = p->next; p != NULL; p = p->next)
  {
    xx = (double)(p->val % rs);
    yy = (double)((p->val % ps) / rs);
    zz = (double)(p->val / ps);
    if (dist3(x, y, z, xx, yy, zz) >= delta)
    {
      *f = p->val;
      break;
    }
    if (p->next == NULL)
    {
      *f = p->val;
#ifdef VERBOSE
      printf("Warning: arc %d length %g\n", Ai, dist3(x, y, z, xx, yy, zz));
#endif    
      return;
    }
  }
} // points_at_head()

/* ====================================================================== */
static void points_at_tail(skel *S, int32_t Ai, double delta, int32_t *e, int32_t *f)
/* ====================================================================== */
// détermine deux points e et f en "fin" de l'arc de courbe Ai, 
// séparés d'une distance euclidienne d'au moins delta
{
  int32_t rs = S->rs;
  int32_t ps = rs * S->cs;
  SKC_pt_pcell p = S->tskel[Ai].pts, pp;
  double x, y, z, xx, yy, zz;

  assert(p != NULL);
  if (p->next == NULL)
  {
    *e = *f = p->val;
#ifdef VERBOSE
    printf("Warning: arc %d length 1\n", Ai);
#endif    
    return;
  }

  for (; p != NULL; p = p->next) *e = p->val;

  x = (double)(*e % rs);
  y = (double)((*e % ps) / rs);
  z = (double)(*e / ps);

  pp = S->tskel[Ai].pts;
  xx = (double)(pp->val % rs);
  yy = (double)((pp->val % ps) / rs);
  zz = (double)(pp->val / ps);
  if (dist3(x, y, z, xx, yy, zz) < delta)
  {
    *f = pp->val;
#ifdef VERBOSE
    printf("Warning: arc %d length %g\n", Ai, dist3(x, y, z, xx, yy, zz));
#endif    
    return;
  }

  for (p = pp->next; p != NULL; p = p->next)
  {
    xx = (double)(p->val % rs);
    yy = (double)((p->val % ps) / rs);
    zz = (double)(p->val / ps);
    if (dist3(x, y, z, xx, yy, zz) < delta)
      break;
    pp = p;
  }
  *f = pp->val;
} // points_at_tail()

/* ====================================================================== */
static int32_t adj_point_junc(skel *S, int32_t e, int32_t J)
/* ====================================================================== */
{
  SKC_pt_pcell p;
  int32_t connex = S->connex;
  int32_t rs = S->rs;
  int32_t ps = rs * S->cs;

  switch(connex)
  {
    case 4:
      for (p = S->tskel[J].pts; p != NULL; p = p->next)
	if (sont4voisins(p->val, e, rs)) return 1;
      break;
    case 8:
      for (p = S->tskel[J].pts; p != NULL; p = p->next)
	if (sont8voisins(p->val, e, rs)) return 1;
      break;
    case 6:
      for (p = S->tskel[J].pts; p != NULL; p = p->next)
	if (sont6voisins(p->val, e, rs, ps)) return 1;
      break;
    case 18:
      for (p = S->tskel[J].pts; p != NULL; p = p->next)
	if (sont18voisins(p->val, e, rs, ps)) return 1;
      break;
    case 26:
      for (p = S->tskel[J].pts; p != NULL; p = p->next)
	if (sont26voisins(p->val, e, rs, ps)) return 1;
      break;
  }
  return 0;
} // adj_point_junc()

/* ====================================================================== */
int32_t lskelfilter1a_old(skel *S, double delta, double theta)
/* ====================================================================== */
/*
  For each junction J
    For each arc Ai adjacent to J
      compute and store the vector Vi tangent to Ai starting from J
    For each couple (Vi,Vj) of vectors 
      compute the cosine similarity Cij between Vi and -Vj
        (see http://en.wikipedia.org/wiki/Cosine_similarity)
      if Cij <= theta then mark the arcs Ai and Aj as "aligned"
*/
{
#undef F_NAME
#define F_NAME "lskelfilter1a_old"
  int32_t rs = S->rs;
  int32_t ps = rs * S->cs;
  int32_t J, Ai, nadj, e, f, i, j, A[26];
  SKC_adj_pcell p;
  double Vx[26], Vy[26], Vz[26], Cij;

#ifdef DEBUG
  printf("lskelfilter1a: delta = %g, theta %g\n", delta, theta);
#endif	  

  for (Ai = S->e_end; Ai < S->e_curv; Ai++)
    S->tskel[Ai].tag = 1; // mark all arcs as "not aligned"

  for (J = S->e_curv; J < S->e_junc; J++)
  {
    for (p = S->tskel[J].adj,nadj = 0; p != NULL; p = p->next, nadj++)
    {
      Ai = p->val;
      A[nadj] = Ai;
      points_at_head(S, Ai, delta, &e, &f);
      if (adj_point_junc(S, e, J))
      {
	Vx[nadj] = (double)((e % rs) - (f % rs));
	Vy[nadj] = (double)(((e % ps) / rs) - ((f % ps) / rs));
	Vz[nadj] = (double)((e / ps) - (f / ps));
      }
      else
      {
	points_at_tail(S, Ai, delta, &e, &f);
	assert(adj_point_junc(S, e, J));
	Vx[nadj] = (double)((e % rs) - (f % rs));
	Vy[nadj] = (double)(((e % ps) / rs) - ((f % ps) / rs));
	Vz[nadj] = (double)((e / ps) - (f / ps));	
      }
    } // for (p = S->tskel[J].adj,nadj = 0; p != NULL; p = p->next, nadj++)
    for (i = 0; i < nadj-1; i++)
      for (j = i+1; j < nadj; j++)
      {
	Cij = acos(scalarprod(Vx[i], Vy[i], Vz[i], -Vx[j], -Vy[j], -Vz[j]) / 
		      (norm(Vx[i], Vy[i], Vz[i]) * norm(Vx[j], Vy[j], Vz[j])));
	if (Cij <= theta)
	{
	  S->tskel[A[i]].tag = 0;
	  S->tskel[A[j]].tag = 0;
#ifdef DEBUG
	  printf("mark %d and %d\n", A[i], A[j]);
#endif	  
	}
      }
  } // for (J = S->e_curv; J < S->e_junc; J++)

  return 1;
} /* lskelfilter1a_old() */

/* ====================================================================== */
static void list_points_at_head(skel *S, int32_t Ai, double delta, int32_t *listpoints, int32_t *npoints)
/* ====================================================================== */
// Renvoie dans listpoints la liste des points du "début" de l'arc de courbe Ai, 
// séparés d'une distance euclidienne de moins (<=) de delta du premier.
// Renvoie dans npoints le nombre de points trouvés.
// Le tableau listpoints doit avoir été alloué.
// En entrée, *npoints contient la taille du tableau listpoints.
{
  int32_t rs = S->rs;
  int32_t ps = rs * S->cs;
  SKC_pt_pcell p = S->tskel[Ai].pts;
  double x, y, z, xx, yy, zz;
  int32_t n, nmax;

  nmax = *npoints;
  assert(nmax > 0);

  assert(p != NULL);
  listpoints[0] = p->val;
  n = 1;

  x = (double)(p->val % rs);
  y = (double)((p->val % ps) / rs);
  z = (double)(p->val / ps);

  p = p->next;
  for (; p != NULL; p = p->next)
  {
    xx = (double)(p->val % rs);
    yy = (double)((p->val % ps) / rs);
    zz = (double)(p->val / ps);
    if (dist3(x, y, z, xx, yy, zz) > delta) break;
    assert(n < nmax);
    listpoints[n] = p->val;
    n++;
  }
  *npoints = n;
} // list_points_at_head()

/* ====================================================================== */
static void list_points_at_tail(skel *S, int32_t Ai, double delta, int32_t *listpoints, int32_t *npoints)
/* ====================================================================== */
// Renvoie dans listpoints la liste des points de la "fin" de l'arc de courbe Ai, 
// séparés d'une distance euclidienne de moins (<=) de delta du dernier.
// Le dernier point de l'arc est le premier point de listpoints.
// Renvoie dans npoints le nombre de points trouvés.
// Le tableau listpoints doit avoir été alloué.
// En entrée, *npoints contient la taille du tableau listpoints.
{
  int32_t rs = S->rs;
  int32_t ps = rs * S->cs;
  SKC_pt_pcell p = S->tskel[Ai].pts;
  double x, y, z, xx, yy, zz;
  int32_t i, n, nmax, e;

  nmax = *npoints;
  assert(nmax > 0);

  assert(p != NULL);
  for (; p != NULL; p = p->next) e = p->val; // atteint le dernier point
  x = (double)(e % rs);
  y = (double)((e % ps) / rs);
  z = (double)(e / ps); 

  for (n = 0, p = S->tskel[Ai].pts; p != NULL; p = p->next) // repart du début
  {
    xx = (double)(p->val % rs);
    yy = (double)((p->val % ps) / rs);
    zz = (double)(p->val / ps);
    if (dist3(x, y, z, xx, yy, zz) <= delta)
    {
      assert(n < nmax);
      listpoints[n] = p->val;
      n++;
    }
  }
  // retourne la liste des points mémorisés
  for (i = 0; i < n/2; i++)
  {
    e = listpoints[i];
    listpoints[i] = listpoints[n-1-i];
    listpoints[n-1-i] = e;
  }
  *npoints = n;
} // list_points_at_tail()

/* ====================================================================== */
static int32_t compute_vector(skel *S, int32_t Ai, int32_t J, double delta1, double delta2, int32_t *listpoints, int32_t nmaxpoints, double *X, double *Y, double *Z, double *VVx, double *VVy, double *VVz)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "compute_vector"
{
  int32_t rs = S->rs, ps = rs * S->cs;
  int32_t i, j, ret, npoints;
  double xc, yc, zc, dmy, vx, vy, vz;
#ifdef DELTA_BOULES
  double x, y, z, xx, yy, zz;
#endif

  npoints = nmaxpoints;
  list_points_at_head(S, Ai, delta2, listpoints, &npoints);
  assert(npoints > 0);
  if (adj_point_junc(S, listpoints[0], J))
  { // arc partant de la jonction 
#ifdef DELTA_BOULES
    x = (double)(listpoints[0] % rs);
    y = (double)((listpoints[0] % ps) / rs);
    z = (double)(listpoints[0] / ps);
    for (j = i = 0; i < npoints; i++)
    {
      xx = (double)(listpoints[i] % rs);
      yy = (double)((listpoints[i] % ps) / rs);
      zz = (double)(listpoints[i] / ps);
      if (dist3(x, y, z, xx, yy, zz) >= delta1)
      { 
	X[j] = xx; Y[j] = yy; Z[j] = zz;
	j++;
      }
    }
#else
    for (i = (int32_t)delta1, j = 0; i < mcmin(npoints,(int32_t)delta2); i++, j++)
    {
      X[j] = (double)(listpoints[i] % rs);
      Y[j] = (double)((listpoints[i] % ps) / rs);
      Z[j] = (double)(listpoints[i] / ps);
    }
#endif
    for (i = 1; i < j; i++)
    { // origine pour le calcul de la direction principale : 1er point trouvé 
      X[i] -= X[0]; Y[i] -= Y[0]; Z[i] -= Z[0];
    }
    X[0] = Y[0] = Z[0] = 0.0;
    if (j > 1)
    {
      ret = ldirectionsprincipales3d(X, Y, Z, j, 
				     &xc, &yc, &zc, 
				     &vx, &vy, &vz, 
				     &dmy, &dmy, &dmy, 
				     &dmy, &dmy, &dmy); assert(ret != 0);
      if (scalarprod(vx, vy, vz, xc, yc, zc) < 0)
      { vx = -vx; vy = -vy; vz = -vz; }
#ifdef DEBUGCOMPVECT
      printf("Arc Ai = %d ; V=(%g,%g,%g) ; c=(%g,%g,%g)\n", 
	     Ai, vx, vy, vz, xc, yc, zc);
#endif	  
    }
    else
    {
#ifdef VERBOSE
      printf("%s: warning: arc %d too short (length %d)\n", F_NAME, Ai, tailleptliste(S->tskel[Ai].pts));
#endif
      *VVx = *VVy = *VVz = 0.0;
      return 0;
    }
  } // if (adj_point_junc(S, listpoints[0], J))
  else
  { // arc arrivant à la jonction 
    npoints = nmaxpoints;
    list_points_at_tail(S, Ai, delta2, listpoints, &npoints);
    assert(npoints > 0);
    assert(adj_point_junc(S, listpoints[0], J));
#ifdef DELTA_BOULES
    x = (double)(listpoints[0] % rs);
    y = (double)((listpoints[0] % ps) / rs);
    z = (double)(listpoints[0] / ps);
    for (j = i = 0; i < npoints; i++)
    {
      xx = (double)(listpoints[i] % rs);
      yy = (double)((listpoints[i] % ps) / rs);
      zz = (double)(listpoints[i] / ps);
      if (dist3(x, y, z, xx, yy, zz) >= delta1)
      { 
	X[j] = xx; Y[j] = yy; Z[j] = zz;
	j++;
      }
    }
#else
    for (i = (int32_t)delta1, j = 0; i < mcmin(npoints,(int32_t)delta2); i++, j++)
    {
      X[j] = (double)(listpoints[i] % rs);
      Y[j] = (double)((listpoints[i] % ps) / rs);
      Z[j] = (double)(listpoints[i] / ps);
    }
#endif
    for (i = 1; i < j; i++)
    { // origine pour le calcul de la direction principale : 1er point trouvé 
      X[i] -= X[0]; Y[i] -= Y[0]; Z[i] -= Z[0];
    }
    X[0] = Y[0] = Z[0] = 0.0;
    if (j > 1)
    {
      ret = ldirectionsprincipales3d(X, Y, Z, j, 
				     &xc, &yc, &zc, 
				     &vx, &vy, &vz, 
				     &dmy, &dmy, &dmy, 
				     &dmy, &dmy, &dmy); assert(ret != 0);
      if (scalarprod(vx, vy, vz, xc, yc, zc) < 0)
      { vx = -vx; vy = -vy; vz = -vz; }
#ifdef DEBUGCOMPVECT
      printf("Arc Ai = %d ; V=(%g,%g,%g) ; c=(%g,%g,%g)\n", 
	     Ai, vx, vy, vz, xc, yc, zc);
#endif	  
    }
    else
    {
#ifdef VERBOSE
      printf("%s: warning: arc %d too short (length %d)\n", F_NAME, Ai, tailleptliste(S->tskel[Ai].pts));
#endif
      *VVx = *VVy = *VVz = 0.0;
      return 0;
    }
  } // else
  *VVx = vx; *VVy = vy; *VVz = vz;
  return 1;
} // compute_vector()

/* ====================================================================== */
int32_t lskelfilter1a(skel *S, double delta1, double delta2, double theta, int32_t length)
/* ====================================================================== */
/*
  For each junction J
    For each arc Ai adjacent to J
      compute and store the vector Vi tangent to Ai starting from J
    For each couple (Vi,Vj) of vectors 
      compute the cosine similarity Cij between Vi and -Vj
        (see http://en.wikipedia.org/wiki/Cosine_similarity)
      if Cij <= theta then mark the arcs Ai and Aj as "aligned"

  The matching branches are marked (field "tag" = 1)
  Any arc that is closed (cycle), isolated (two ends) or longer than 
  parameter length is also marked as "aligned"
  For the sake of connectivity, junctions are also marked.
*/
{
#undef F_NAME
#define F_NAME "lskelfilter1a"
  int32_t ret, J, Ai, nadj, i, j, A[26], nmaxpoints, *listpoints;
  SKC_adj_pcell p;
  double Vx[26], Vy[26], Vz[26], VVx, VVy, VVz, Cij;
  double *X, *Y, *Z;
#ifdef DEBUG
  printf("%s: delta1 = %g, delta2 = %g, theta %g\n", F_NAME, delta1, delta2, theta);
#endif	  

  nmaxpoints = delta2 * 4;
  listpoints = (int32_t *)malloc(nmaxpoints * sizeof(int32_t)); assert(listpoints != NULL);
  X = (double *)malloc(nmaxpoints * sizeof(double)); assert(X != NULL);
  Y = (double *)malloc(nmaxpoints * sizeof(double)); assert(Y != NULL);
  Z = (double *)malloc(nmaxpoints * sizeof(double)); assert(Z != NULL);

  for (J = S->e_curv; J < S->e_junc; J++) // mark all junctions
    S->tskel[J].tag = 1; 

  for (Ai = S->e_end; Ai < S->e_curv; Ai++) // scan all arcs
  {
    p = S->tskel[Ai].adj;
    if (p == NULL) // arc fermé (cycle)
    {
      S->tskel[Ai].tag = 1; // mark as "aligned"
      break;
    }
    assert(p->next != NULL); // soit 0, soit 2 adjacences
    if (((!IS_JUNC(p->val)) && (!IS_JUNC(p->next->val))) ||
	(tailleptliste(S->tskel[Ai].pts) >= length))
      S->tskel[Ai].tag = 1; // mark as "aligned"
    else
      S->tskel[Ai].tag = 0; // mark as "not aligned"
  }

  for (J = S->e_curv; J < S->e_junc; J++)
  { // scan all junctions
    for (p = S->tskel[J].adj,nadj = 0; p != NULL; p = p->next, nadj++)
    { // scan arcs Ai adjacent to junction J
      Ai = p->val;
      A[nadj] = Ai;

      ret = compute_vector(S, Ai, J, delta1, delta2, listpoints, nmaxpoints, 
		     X, Y, Z, &VVx, &VVy, &VVz);
      assert(ret != 0);
      Vx[nadj] = VVx; Vy[nadj] = VVy; Vz[nadj] = VVz;
    } // for (p = S->tskel[J].adj,nadj = 0; p != NULL; p = p->next, nadj++)

    for (i = 0; i < nadj-1; i++)
    {
#ifdef DEBUG
      printf("Arc i = %d V=(%g,%g,%g)\n", A[i], Vx[i], Vy[i], Vz[i]);
#endif	  
      for (j = i+1; j < nadj; j++)
      {
	Cij = acos(scalarprod(Vx[i], Vy[i], Vz[i], -Vx[j], -Vy[j], -Vz[j]) / 
		      (norm(Vx[i], Vy[i], Vz[i]) * norm(Vx[j], Vy[j], Vz[j])));
#ifdef DEBUG
	printf("  Arc j = %d V=(%g,%g,%g), Cij = %g(%g)\n", A[j], Vx[j], Vy[j], Vz[j], Cij, (Cij*180)/M_PI);
#endif	  
	if (Cij <= theta)
	{
	  S->tskel[A[i]].tag = 1;
	  S->tskel[A[j]].tag = 1;
#ifdef DEBUG
	  printf("mark %d and %d\n", A[i], A[j]);
#endif	  
	}
      } // for j
    } // for i
  } // for (J = S->e_curv; J < S->e_junc; J++)

  free(listpoints); free(X); free(Y); free(Z);
  return 1;
} /* lskelfilter1a() */

/* ====================================================================== */
void mark_sharp_angles(skel *S, int32_t len, double sharp, struct xvimage *image, int32_t *lp, int32_t nmax)
/* ====================================================================== */
/*
The curves of skeleton S are searched for "sharp" angles.
Let <C[0], ... C[n-1]> be the points of the curve C. 
Let j be an index between len and n-1-len, 
let i = j - len, let k = j + len.
If angle(C[j]-C[i], C[j]-C[k]) <= sharp then mark C[j] (by a voxel in "image").

Le tableau lp doit avoir été alloué.
En entrée, nmax contient la taille du tableau lp.
*/
{
#undef F_NAME
#define F_NAME "mark_sharp_angles"
  int32_t C, i, j, k, n, rs = S->rs, ps = rs * S->cs;
  SKC_pt_pcell p;
  double xi, yi, zi, xj, yj, zj, xk, yk, zk, angle;
  uint8_t *F;

#ifdef DEBUG
  printf("%s: length = %d, sharp %g\n", F_NAME, len, sharp);
#endif	   
  assert(image != NULL);
  assert(datatype(image) == VFF_TYP_1_BYTE);
  assert(rowsize(image)==rs);
  assert(colsize(image)==S->cs);
  assert(depth(image)==S->ds);
  razimage(image);
  F = UCHARDATA(image);

  for (C = S->e_end; C < S->e_curv; C++) // scan all curves C
  {
    n = 0;
    for (p = S->tskel[C].pts; p != NULL; p = p->next)
    {
      assert(n < nmax);
      lp[n] = p->val;
      n++;
    }
    for (j = len, i = 0, k = j+len; k < n; i++, j++, k++)
    {
      xi = (double)(lp[i] % rs);
      yi = (double)((lp[i] % ps) / rs);
      zi = (double)(lp[i] / ps);
      xj = (double)(lp[j] % rs);
      yj = (double)((lp[j] % ps) / rs);
      zj = (double)(lp[j] / ps);
      xk = (double)(lp[k] % rs);
      yk = (double)((lp[k] % ps) / rs);
      zk = (double)(lp[k] / ps);
      // If angle(C[j]-C[i], C[j]-C[k]) <= sharp then mark C[j]
      angle = acos(scalarprod(xj-xi, yj-yi, zj-zi, xj-xk, yj-yk, zj-zk) / 
		   (norm(xj-xi, yj-yi, zj-zi) * norm(xj-xk, yj-yk, zj-zk)));
      if (angle <= sharp) F[lp[j]] = 255;
    }
  } // for (i = S->e_end; i < S->e_curv; i++)
#ifdef DEBUG
  printf("%s: leaving\n", F_NAME);
#endif	   
} /* mark_sharp_angles() */

/* ====================================================================== */
struct xvimage * lskelfindelbows(skel *S, double length, double angle)
/* ====================================================================== */
/*
Find "elbows" (points making sharp angles) in the curves of skeleton S.
Matching points are written as voxels in the returned image.  
*/
{
#undef F_NAME
#define F_NAME "lskelfindelbows"
  int32_t nmaxpoints, *listpoints;
  struct xvimage *sharppoints;

  sharppoints = allocimage(NULL, S->rs, S->cs, S->ds, VFF_TYP_1_BYTE); 
  assert(sharppoints != NULL);
  nmaxpoints = S->rs + S->rs + S->cs + S->cs + S->ds + S->ds;
  listpoints = (int32_t *)malloc(nmaxpoints * sizeof(int32_t));
  assert(listpoints != NULL);

  mark_sharp_angles(S, length, angle, sharppoints, listpoints, nmaxpoints);

  free(listpoints);
#ifdef DEBUG
  printf("%s: leaving\n", F_NAME);
#endif	   
  return sharppoints;
} /* lskelfindelbows() */

/* ====================================================================== */
int32_t lskelfilter1(skel *S, double length, double delta1, double delta2)
/* ====================================================================== */
/*
The skeleton S is searched for arcs A which satisfy the following criteria:
\li The length of A is not greater than parameter \b length.
\li Both extremities of A are junctions.
\li For at least one extremity E of A, 
    the arc A is the worst-aligned arc adjacent to E.

The matching branches are marked (field "tag" = 1).
*/
{
#undef F_NAME
#define F_NAME "lskelfilter1"
  int32_t ret, i, j, len, e, m, Ai, EE[2], E;
  int32_t nadj, A[26], nmaxpoints, *listpoints;
  SKC_adj_pcell p;
  double Vx[26], Vy[26], Vz[26], VVx, VVy, VVz, Cij, angle, maxangle;
  double *X, *Y, *Z;

#ifdef DEBUG
  printf("%s: length = %g, delta1 = %g, delta2 = %g\n", F_NAME, length, delta1, delta2);
#endif	  

  for (i = 0; i < S->e_junc; i++) S->tskel[i].tag = 0; // unmark all

  if (S->e_curv == S->e_end) return 1; // no arc: exit 

  nmaxpoints = delta2 * 4;
  listpoints = (int32_t *)malloc(nmaxpoints * sizeof(int32_t)); assert(listpoints != NULL);
  X = (double *)malloc(nmaxpoints * sizeof(double)); assert(X != NULL);
  Y = (double *)malloc(nmaxpoints * sizeof(double)); assert(Y != NULL);
  Z = (double *)malloc(nmaxpoints * sizeof(double)); assert(Z != NULL);

  for (Ai = S->e_end; Ai < S->e_curv; Ai++) // scan all arcs
  {  
    //Let E1, E2 be the vertices adjacent to A0
    len = tailleptliste(S->tskel[Ai].pts);
    p = S->tskel[Ai].adj;
    if ((len >= (int32_t)delta2) && (len <= length) && (p != NULL)) 
    {// si arc non fermé et assez court (mais pas trop)
      assert(p->next != NULL); // soit 0, soit 2 adjacences
      EE[0] = p->val;
      EE[1] = p->next->val;
#ifdef DEBUG
      printf("arc Ai=%d; E1 = %d, E2 = %d\n", Ai, EE[0], EE[1]);
#endif	  
    
      for (e = 0; e < 2; e++)
      {
	E = EE[e];
	if (!IS_JUNC(E)) break; // les 2 doivent être des jonctions
#ifdef DEBUG
	printf("processing junction E=%d\n", E);
#endif	  
	A[0] = Ai; nadj = 1;
	//      Let A1, ... Ak be the arcs other than Ai adjacent to E
	for (p = S->tskel[E].adj; p != NULL; p = p->next)
	{ // scan arcs adjacent to junction E
	  if ((p->val != Ai) && (tailleptliste(S->tskel[p->val].pts) >= (int32_t)delta2)) 
	  { 
	    A[nadj] = p->val; nadj++; 
	  }
	}
#ifdef DEBUG
	printf("  adjacent arcs: ");
	for (j = 0; j < nadj; j++) printf("%d ", A[j]);
	printf("\n");
#endif	  
	if (nadj >= 3)
	{
//        Let V0, ... Vk be the corresponding tangent vectors
	  for (j = 0; j < nadj; j++)
	  {
	    ret = compute_vector(S, A[j], E, delta1, delta2, listpoints, nmaxpoints, 
				 X, Y, Z, &VVx, &VVy, &VVz);
	    assert(ret != 0);
	    Vx[j] = VVx; Vy[j] = VVy; Vz[j] = VVz;
	  }

//        Let m be such that the angle <V0,-Vm> is maximal
	  angle = 0.0;
	  m = -1;
	  for (j = 1; j < nadj; j++)
	  {
	    Cij = acos(scalarprod(Vx[0], Vy[0], Vz[0], -Vx[j], -Vy[j], -Vz[j]) / 
		       (norm(Vx[0], Vy[0], Vz[0]) * norm(Vx[j], Vy[j], Vz[j])));
#ifdef DEBUG
	    printf("    C(%d,%d)=%g(%g)\n", A[0], A[j], Cij, (Cij*180)/M_PI);
#endif	  
	    if (Cij > angle) { angle = Cij; m = j; }
	  }
	  assert(m != -1);
#ifdef DEBUG
	  printf("  angle=%g\n", angle);
#endif	  

//        Let maxangle be min{<Vi,-Vj> | 0 <= i < k and i < j <= k} 
	  maxangle = 0.0;
	  for (i = 0; i < nadj-1; i++)
	    for (j = i+1; j < nadj; j++)
	    {
	      Cij = acos(scalarprod(Vx[i], Vy[i], Vz[i], -Vx[j], -Vy[j], -Vz[j]) / 
			 (norm(Vx[i], Vy[i], Vz[i]) * norm(Vx[j], Vy[j], Vz[j])));
#ifdef DEBUG
	      printf("    C(%d,%d)=%g(%g)\n", A[i], A[j], Cij, (Cij*180)/M_PI);
#endif	  
	      if (Cij > maxangle) { maxangle = Cij; }
	    }
#ifdef DEBUG
	  printf("  maxangle=%g\n", maxangle);
#endif	  
	  // If <V0,-Vm> == maxangle then 
	  if (angle == maxangle)
	  {
	    // mark Ai
	    S->tskel[Ai].tag = 1;
#ifdef DEBUG
	    printf("mark arc: %d\n", Ai);
#endif	  
	  }
        } // if (nadj >= 3)
      } // for (e = 0; e < 2; e++)
    } // if ((len <= length) && (p != NULL))
  } // for (Ai = S->e_end; Ai < S->e_curv; Ai++)
  return 1;
} /* lskelfilter1() */

/* ====================================================================== */
int32_t lskelfilter1b(skel *S, double length, double delta1, double delta2)
/* ====================================================================== */
/*
The skeleton S is searched for arcs A which satisfy the following criteria:
\li The length of A is not greater than parameter \b length.
\li Both extremities of A are junctions.

The matching arcs are marked (field "tag" = 1).
*/
{
#undef F_NAME
#define F_NAME "lskelfilter1b"
  int32_t i, len, Ai;
  SKC_adj_pcell p;

#ifdef DEBUG
  printf("%s: length = %g, delta1 = %g, delta2 = %g\n", F_NAME, length, delta1, delta2);
#endif	  

  for (i = 0; i < S->e_junc; i++) S->tskel[i].tag = 0; // unmark all

  if (S->e_curv == S->e_end) return 1; // no arc: exit 

  for (Ai = S->e_end; Ai < S->e_curv; Ai++) // scan all arcs
  {  
    len = tailleptliste(S->tskel[Ai].pts);
    p = S->tskel[Ai].adj;
    if ((len <= length) && (p != NULL)) 
    { // si arc non fermé et assez court
      assert(p->next != NULL); // soit 0, soit 2 adjacences
      if (IS_JUNC(p->val) && IS_JUNC(p->next->val))
      {	    // mark Ai
	S->tskel[Ai].tag = 1;
#ifdef DEBUG
	printf("mark arc: %d\n", Ai);
#endif	  
      }
    } // if ((len <= length) && (p != NULL))
  } // for (Ai = S->e_end; Ai < S->e_curv; Ai++)
  return 1;
} /* lskelfilter1b() */

/* ====================================================================== */
int32_t lskelfilter2(skel *S, double delta1, double delta2)
/* ====================================================================== */
/*
  One iteration of the "Mikado game" algorithm
  Let A0 be the arc in S having the greatest length
  Mark A0 (MARK1)
  Let E0, E1 be the vertices adjacent to A0
  For E in {E0, E1} do
    While E is a not yet marked junction do
      Mark E as end junction (MARK1)
      Let A1, ... Ak be the arcs other than A0 adjacent to E
      Let V0, ... Vk be the corresponding tangent vectors
      Let m be such that the angle <V0,-Vm> is minimal
      Let minangle be min{<Vi,-Vj> | 0 <= i < k and i < j <= k} 
      If <V0,-Vm> == minangle then 
        E' = extremity of Am different from E
	Mark E as internal junction (MARK2)
        A0 = Am; E = E'
        Mark A0
      EndIf
    EndWhile
    If E is an end junction then Unmark E
  EndFor
*/
{
#undef F_NAME
#define F_NAME "lskelfilter2"
  int32_t ret, e, i, j, m, Ai, A0, EE[2], E, Ep;
  int32_t nadj, A[26], nmaxpoints, *listpoints, maxlen;
  SKC_adj_pcell p;
  double Vx[26], Vy[26], Vz[26], VVx, VVy, VVz, Cij, angle, minangle, normprod;
  double *X, *Y, *Z;

#ifdef DEBUGDRAW
  int32_t rs = S->rs, ps = rs * S->cs;
  double xb, yb, zb;
  struct xvimage *dbg = allocimage(NULL, S->rs, S->cs, S->ds, VFF_TYP_1_BYTE); assert(dbg != NULL);
  razimage(dbg);
#endif	  

#ifdef DEBUG
  printf("%s: delta1 = %g, delta2 = %g\n", F_NAME, delta1, delta2);
#endif	  

  for (i = S->e_end, j = 0; i < S->e_curv; i++)
    if (!SK_DELETED(i)) j++; // counts non-deleted arcs
  if (j == 0) return 1; // no arc: exit 

  nmaxpoints = delta2 * 4;
  listpoints = (int32_t *)malloc(nmaxpoints * sizeof(int32_t)); assert(listpoints != NULL);
  X = (double *)malloc(nmaxpoints * sizeof(double)); assert(X != NULL);
  Y = (double *)malloc(nmaxpoints * sizeof(double)); assert(Y != NULL);
  Z = (double *)malloc(nmaxpoints * sizeof(double)); assert(Z != NULL);

//Let A0 be the arc in S having the greatest length
  maxlen = 0;
  for (i = S->e_end; i < S->e_curv; i++) // scan all arcs
    if (!SK_DELETED(i))
    {
      ret = tailleptliste(S->tskel[i].pts);
      if (ret > maxlen) { maxlen = ret; A0 = i; }
    }
  assert(maxlen > 0);

//Mark A0 (MARK1)
  SK_MARK1(A0);
#ifdef DEBUG
  printf("mark initial arc: %d    ", A0);
#endif	  
  
//Let E1, E2 be the vertices adjacent to A0
  p = S->tskel[A0].adj;
  if (p != NULL) // si arc non fermé
  {
    assert(p->next != NULL); // soit 0, soit 2 adjacences
    EE[0] = p->val;
    EE[1] = p->next->val;
#ifdef DEBUG
    printf("E1 = %d, E2 = %d\n", EE[0], EE[1]);
#endif	  
    
//  For E in {E0, E1} do
    for (e = 0; e < 2; e++)
    {
      E = EE[e];
#ifdef DEBUGDRAW
      p = S->tskel[E].pts;
      xb = (double)(p->val % rs);
      yb = (double)((p->val % ps) / rs);
      zb = (double)(p->val / ps);
#endif	  
#ifdef DEBUG
      printf("tracking from E=%d\n", E);
#endif	  

//    While E is a not yet marked junction do
      while (IS_JUNC(E) && (!SK_MARKED1(E)) && (!SK_MARKED2(E)))
      {
//      Mark E as end junction
	SK_MARK1(E);
#ifdef DEBUG
	printf("  mark end junction E=%d ; A0=%d\n", E, A0);
#endif	  
	
//      Let A1, ... Ak be the arcs other than A0 adjacent to E
	for (p = S->tskel[E].adj,nadj = 0; p != NULL; p = p->next, nadj++)
	{ // scan arcs Ai adjacent to junction E
	  Ai = p->val;
	  A[nadj] = Ai;
	  if (Ai == A0) { Ai = A[0]; A[0] = A0; A[nadj] = Ai; }
	}
#ifdef DEBUG
	printf("  adjacent arcs: ");
	for (j = 0; j < nadj; j++) printf("%d ", A[j]);
	printf("\n");
#endif	  
	if (nadj <= 2) break; // end or elbow: stop tracking

//      Let V0, ... Vk be the corresponding tangent vectors
	for (j = 0; j < nadj; j++)
	{
	  ret = compute_vector(S, A[j], E, delta1, delta2, listpoints, nmaxpoints, 
			       X, Y, Z, &VVx, &VVy, &VVz);
	  Vx[j] = VVx; Vy[j] = VVy; Vz[j] = VVz;
#ifdef DEBUGDRAW
	  ldrawline3d(dbg, arrondi(xb), arrondi(yb), arrondi(zb), arrondi((xb+(10*Vx[j]))), arrondi((yb+(10*Vy[j]))), arrondi((zb+(10*Vz[j]))));
#endif
	}

//      Let m be such that the angle <V0,-Vm> is minimal
	angle = 4.0; // greater than any angle
	m = -1;
	for (j = 1; j < nadj; j++)
	{
	  normprod = norm(Vx[0], Vy[0], Vz[0]) * norm(Vx[j], Vy[j], Vz[j]);
	  if (normprod != 0.0) 
	  {
	    Cij = acos(scalarprod(Vx[0], Vy[0], Vz[0], -Vx[j], -Vy[j], -Vz[j]) / normprod);
#ifdef DEBUG
	    printf("    C(%d,%d)=%g(%g)\n", A[0], A[j], Cij, (Cij*180)/M_PI);
#endif	  
	    if (Cij < angle) { angle = Cij; m = j; }
	  }
	}
	if (m == -1) break; // only small arcs: stop tracking
#ifdef DEBUG
	printf("  angle=%g\n", angle);
#endif	  

//      Let minangle be min{<Vi,-Vj> | 0 <= i < k and i < j <= k} 
	minangle = 4.0; // greater than any angle
	for (i = 0; i < nadj-1; i++)
	for (j = i+1; j < nadj; j++)
	{
	  normprod = norm(Vx[i], Vy[i], Vz[i]) * norm(Vx[j], Vy[j], Vz[j]);
	  if (normprod != 0.0) 
	  {
	    Cij = acos(scalarprod(Vx[i], Vy[i], Vz[i], -Vx[j], -Vy[j], -Vz[j]) / normprod);
#ifdef DEBUG
	    printf("    C(%d,%d)=%g(%g)\n", A[i], A[j], Cij, (Cij*180)/M_PI);
#endif	  
	    if (Cij < minangle) { minangle = Cij; }
	  }
	}
#ifdef DEBUG
	printf("  minangle=%g\n", minangle);
#endif	  


//      If <V0,-Vm> <= minangle then 
	if ((angle <= minangle) && (angle <= MAXANGLE))
	{
//        E' = extremity of Am different from E
	  p = S->tskel[A[m]].adj;
	  assert(p != NULL); assert(p->next != NULL);
	  if (p->val == E) Ep = p->next->val; else Ep = p->val;

//        Mark E as internal junction
	  SK_UNMARK1(E);
	  SK_MARK2(E);
#ifdef DEBUG
	  printf("  mark internal junction E=%d\n", E);
#endif	  

//        A0 = Am; E = E'
	  A0 = A[m]; E = Ep;

//        Mark A0
	  SK_MARK1(A0);
#ifdef DEBUG
	  printf("  mark arc %d\n", A0);
#endif	  
	} // if (angle <= minangle)
      } // while (IS_JUNC(E) && (!SK_MARKED1(E)) && (!SK_MARKED2(E)))

//    If E is an end junction then Unmark E
      if (IS_JUNC(E) && (SK_MARKED1(E))) 
      {
	SK_UNMARK1(E);
#ifdef DEBUG
	printf("  unmark end junction %d\n", E);
#endif	  	
      }
    } // for (i = 0; i < 2; i++)
  } // if (p != NULL) 
#ifdef DEBUGDRAW
  writeimage(dbg, "_dbg");
#endif

  free(listpoints); free(X); free(Y); free(Z);
  return 1;
} /* lskelfilter2() */

/* ====================================================================== */
int32_t lskelfilter2b(skel *S, double delta1, double delta2)
/* ====================================================================== */
/*
  One iteration of the "Mikado game" algorithm
  Let A0 be the arc in S having the greatest length
  Mark A0 (MARK1)
  Let E0, E1 be the vertices adjacent to A0
  For E in {E0, E1} do
    While E is a not yet marked junction do
      Mark E as end junction (MARK1)
      Let A1, ... Ak be the arcs other than A0 adjacent to E
      Let V0, ... Vk be the corresponding tangent vectors
      Let m be such that the angle <V0,-Vm> is minimal
      Let minangle be min{<Vi,-Vj> | 0 <= i < k and i < j <= k} 
      If <V0,-Vm> == minangle then 
        E' = extremity of Am different from E
	Mark E as internal junction (MARK2)
        A0 = Am; E = E'
        Mark A0
      EndIf
    EndWhile
    If E is an end junction then Unmark E
  EndFor

  Version with pre-computed tangent vectors, stored in the skel structure
*/
{
#undef F_NAME
#define F_NAME "lskelfilter2b"
  int32_t ret, e, i, j, m, A0, A0sav, EE[2], E, Ep;
  int32_t nadj, A[26], maxlen;
  SKC_adj_pcell p;
  double Vx[26], Vy[26], Vz[26], Cij, angle, minangle, normprod;

#ifdef DEBUG_lskelfilter2b
  printf("%s: begin\n", F_NAME);
#endif	  

  for (i = S->e_end, j = 0; i < S->e_curv; i++)
    if (!SK_DELETED(i)) j++; // counts non-deleted arcs
#ifdef DEBUG_lskelfilter2b
  printf("nb arcs = %d    \n", j);
#endif	  
  if (j == 0) return 0; // no arc: exit 

//Let A0 be the arc in S having the greatest length
  maxlen = 0;
  for (i = S->e_end; i < S->e_curv; i++) // scan all arcs
    if (!SK_DELETED(i))
    {
      ret = tailleptliste(S->tskel[i].pts);
      if (ret > maxlen) { maxlen = ret; A0 = i; }
    }
  assert(maxlen > 0);

//Mark A0 (MARK1)
  SK_MARK1(A0);
#ifdef DEBUG_lskelfilter2b
  printf("mark initial arc: %d    \n", A0);
#endif	  
  
//Let E1, E2 be the vertices adjacent to A0
  p = S->tskel[A0].adj;
  if (p != NULL) // si arc non fermé
  {
    assert(p->next != NULL); // soit 0, soit 2 adjacences
    EE[0] = p->val;
    EE[1] = p->next->val;
#ifdef DEBUG_lskelfilter2b
    printf("E1 = %d, E2 = %d\n", EE[0], EE[1]);
#endif	  
    
//  For E in {E0, E1} do
    A0sav = A0;
    for (e = 0; e < 2; e++)
    {
      A0 = A0sav;
      E = EE[e];
#ifdef DEBUG_lskelfilter2b
      printf("tracking from E=%d\n", E);
#endif	  

//    While E is a not yet marked junction do
      while (IS_JUNC(E) && (!SK_MARKED1(E)) && (!SK_MARKED2(E)))
      {
//      Mark E as end junction
	SK_MARK1(E);
#ifdef DEBUG_lskelfilter2b
	printf("  mark end junction E=%d ; A0=%d\n", E, A0);
#endif	  
	
//      Let A1, ... Ak be the arcs other than A0 adjacent to E
	for (p = S->tskel[E].adj,nadj = 0; p != NULL; p = p->next, nadj++)
	{ // scan arcs Ai (p->val) adjacent to junction E
	  A[nadj] = p->val;
//        Let V0, ... Vk be the corresponding tangent vectors (stored)
	  Vx[nadj] = p->vx; Vy[nadj] = p->vy; Vz[nadj] = p->vz;
	  if (p->val == A0) 
	  { 
	    A[nadj] = A[0]; Vx[nadj] = Vx[0]; Vy[nadj] = Vy[0]; Vz[nadj] = Vz[0]; 
	    A[0] = p->val; Vx[0] = p->vx; Vy[0] = p->vy; Vz[0] = p->vz;
	  }
	}
#ifdef DEBUG_lskelfilter2b
	printf("  adjacent arcs: ");
	for (j = 0; j < nadj; j++) printf("%d ", A[j]);
	printf("\n");
#endif	  
	if (nadj <= 2) break; // end or elbow: stop tracking

//      Let m be such that the angle <V0,-Vm> is minimal
	angle = 4.0; // greater than any angle
	m = -1;
	for (j = 1; j < nadj; j++)
	{
	  normprod = norm(Vx[0], Vy[0], Vz[0]) * norm(Vx[j], Vy[j], Vz[j]);
	  if (normprod != 0.0) 
	  {
	    Cij = acos(scalarprod(Vx[0], Vy[0], Vz[0], -Vx[j], -Vy[j], -Vz[j]) / normprod);
#ifdef DEBUG_lskelfilter2b
	    printf("    C(%d,%d)=%g(%g)\n", A[0], A[j], Cij, (Cij*180)/M_PI);
#endif	  
	    if (Cij < angle) { angle = Cij; m = j; }
	  }
	}
	if (m == -1) break; // only small arcs: stop tracking
#ifdef DEBUG_lskelfilter2b
	printf("  angle=%g\n", angle);
#endif	  

//      Let minangle be min{<Vi,-Vj> | 0 <= i < k and i < j <= k} 
	minangle = 4.0; // greater than any angle
	for (i = 0; i < nadj-1; i++)
	for (j = i+1; j < nadj; j++)
	{
	  normprod = norm(Vx[i], Vy[i], Vz[i]) * norm(Vx[j], Vy[j], Vz[j]);
	  if (normprod != 0.0) 
	  {
	    Cij = acos(scalarprod(Vx[i], Vy[i], Vz[i], -Vx[j], -Vy[j], -Vz[j]) / normprod);
#ifdef DEBUG_lskelfilter2b
	    printf("    C(%d,%d)=%g(%g)\n", A[i], A[j], Cij, (Cij*180)/M_PI);
#endif	  
	    if (Cij < minangle) { minangle = Cij; }
	  }
	}
#ifdef DEBUG_lskelfilter2b
	printf("  minangle=%g\n", minangle);
#endif	  

//      If <V0,-Vm> <= minangle then 
	if ((angle <= minangle) && (angle <= MAXANGLE))
	{
//        E' = extremity of Am different from E
	  p = S->tskel[A[m]].adj;
	  assert(p != NULL); assert(p->next != NULL);
	  if (p->val == E) Ep = p->next->val; else Ep = p->val;

//        Mark E as internal junction
	  SK_UNMARK1(E);
	  SK_MARK2(E);
#ifdef DEBUG_lskelfilter2b
	  printf("  mark internal junction E=%d\n", E);
#endif	  

//        A0 = Am; E = E'
	  A0 = A[m]; E = Ep;

//        Mark A0
	  SK_MARK1(A0);
#ifdef DEBUG_lskelfilter2b
	  printf("  mark arc %d\n", A0);
#endif	  
	} // if (angle <= minangle)
      } // while (IS_JUNC(E) && (!SK_MARKED1(E)) && (!SK_MARKED2(E)))

//    If E is an end junction then Unmark E
      if (IS_JUNC(E) && (SK_MARKED1(E))) 
      {
	SK_UNMARK1(E);
#ifdef DEBUG_lskelfilter2b
	printf("  unmark end junction %d\n", E);
#endif	  	
      }
    } // for (i = 0; i < 2; i++)
  } // if (p != NULL) 

#ifdef DEBUG_lskelfilter2b
  printf("%s: end\n", F_NAME);
#endif	  

  return 1;
} /* lskelfilter2b() */

/* ====================================================================== */
int32_t is_elbow(skel *S, int32_t j, double maxelbowangle)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "is_elbow"
  SKC_adj_pcell p = S->tskel[j].adj;
  double angle;

  assert(p != NULL); assert(p->next != NULL); assert(p->next->next == NULL);
  angle = acos(scalarprod(p->vx, p->vy, p->vz, p->next->vx, p->next->vy, p->next->vz) / 
	       (norm(p->vx, p->vy, p->vz) * norm(p->next->vx, p->next->vy, p->next->vz)));
  if (angle <= maxelbowangle) 
    return 1;
  else
    return 0;
} // is_elbow()

/* ====================================================================== */
struct xvimage * lskelfilter3(skel *S, double delta1, double delta2, double maxbridgelength, double maxelbowangle)
/* ====================================================================== */
/*
  "Mikado game" algorithm

  compute all tangent vectors

  repeat forever
    1. select a fiber (call to lskelfilter2b)
    2. if no fiber is found, then stop
    3. output fiber (write voxels in output image) and delete arcs
    4. mark (MARK2) branches adjacent to F and not longer than maxbridgelength
    5. remove all marked branches and
       update skeleton (merge branches at 2-junctions that are not elbows)
*/
{
#undef F_NAME
#define F_NAME "lskelfilter3"
  int32_t ret, i, j, nmaxpoints, *listpoints, nfiber, na, nd, n;
  SKC_adj_pcell p;
  SKC_pt_pcell pp;
  double VVx, VVy, VVz, *X, *Y, *Z;
  struct xvimage * result;
  int32_t *R;

#ifdef DEBUG_lskelfilter3
  printf("%s: begin e_isol=%d e_end=%d e_curv=%d e_junc=%d \n", F_NAME, S->e_isol, S->e_end, S->e_curv, S->e_junc);
#endif
  
  result = allocimage(NULL, S->rs, S->cs, S->ds, VFF_TYP_4_BYTE); 
  assert(result != NULL);
  razimage(result);
  R = SLONGDATA(result);
  
  nmaxpoints = delta2 * 4;
  listpoints = (int32_t *)malloc(nmaxpoints * sizeof(int32_t)); assert(listpoints != NULL);
  X = (double *)malloc(nmaxpoints * sizeof(double)); assert(X != NULL);
  Y = (double *)malloc(nmaxpoints * sizeof(double)); assert(Y != NULL);
  Z = (double *)malloc(nmaxpoints * sizeof(double)); assert(Z != NULL);

  // mark all arcs undeleted
  for (i = S->e_end; i < S->e_curv; i++) SK_UNREMOVE(i);

  // compute all tangent vectors
  for (j = S->e_curv; j < S->e_junc; j++) // scan all junctions
    for (p = S->tskel[j].adj; p != NULL; p = p->next)
      if (IS_CURV(p->val))
      {
        ret = compute_vector(S, p->val, j, delta1, delta2, listpoints, 
			   nmaxpoints, X, Y, Z, &VVx, &VVy, &VVz);
        p->vx = VVx; p->vy = VVy; p->vz = VVz;
      }

  nfiber = 0;
  // repeat
  while (lskelfilter2b(S, delta1, delta2))
  { // select a fiber (call to lskelfilter2b)
    // if no fiber is found, then stop
    // otherwise the selected fiber F is marked in the skel structure
    // (MARK1 for arcs and end junction, MARK2 for internal junctions)
    nfiber++;

#ifdef DEBUG_lskelfilter3
    printf("%s: fiber detected\n", F_NAME);
#endif

    // output fiber (write labeled voxels in output image) and delete arcs
    for (i = S->e_end, n=0; i < S->e_junc; i++) // scan arcs and junctions
      if (!SK_DELETED(i) && (SK_MARKED1(i) || SK_MARKED2(i)))
      {
	for (pp = S->tskel[i].pts; pp != NULL; pp = pp->next)
        {  R[pp->val] = nfiber; n++; }       // output
	if (IS_CURV(i)) skeldelete(S, i); // delete arcs only
      }

#ifdef DEBUG_lskelfilter3
    printf("%s: fiber written (%d points)\n", F_NAME, n);
#endif

    // remove branches adjacent to F and not longer than maxbridgelength
    for (j = S->e_curv; j < S->e_junc; j++) // scan all junctions
      if (!SK_DELETED(j) && (SK_MARKED1(j) || SK_MARKED2(j)))
      {
	na = nd = 0; // for counting remaining adjacent arcs
	for (p = S->tskel[j].adj; p != NULL; p = p->next)
	  if (IS_CURV(p->val) && !SK_DELETED(p->val))
	  {
	    na++;
	    if (tailleptliste(S->tskel[p->val].pts) <= maxbridgelength) 
	    { 
	      skeldelete(S, p->val); nd++; 	     
#ifdef DEBUG_lskelfilter3
	      printf("%s: arc %d deleted\n", F_NAME, p->val);
#endif
	    }
	  }
	// and unmark 
	SK_UNMARK1(j); SK_UNMARK2(j);
      } // scan junctions

    // update skeleton (merge branches at 2-junctions that are not elbows)
    for (j = S->e_curv; j < S->e_junc; j++) // scan all junctions
      if (!SK_DELETED(j) && (nb_adjacent_elts(S, j) == 2) && (!is_elbow(S, j, maxelbowangle)))
	skeldelete(S, j);

  } // while (lskelfilter2b(S, delta1, delta2))

  return result;
} /* lskelfilter3() */
