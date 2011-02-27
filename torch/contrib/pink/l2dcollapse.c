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
   l2dcollapse: collapse guidÅÈ et contraint (OBSOLETE)
     
   Michel Couprie - avril 2007

   l2dpardircollapse: collapse parallÅËle par sous-ÅÈtapes directionnelles
   l2dpardircollapse_l: collapse guidÅÈ et contraint - prioritÅÈ ULONG
   l2dpardircollapse_f: collapse guidÅÈ et contraint - prioritÅÈ FLOAT
   l2dtopoflow_l: topological flow - prioritÅÈ ULONG
   l2dtopoflow_f: topological flow - prioritÅÈ FLOAT

   Michel Couprie - juin 2009

*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcrlifo.h>
#include <mcrbt.h>
#include <mcindic.h>
#include <mcutil.h>
#include <mcgraphe.h>
#include <mckhalimsky2d.h>
#include <l2dkhalimsky.h>
#include <l2dcollapse.h>

#define EN_RBT        0
#define EN_RLIFO       1
#define BORDER        2

//#define VERBOSE

//#define TRACECOL
//#define TRACECONSGRAPH
//#define DESSINECOLSEQ

/* =============================================================== */
int32_t l2dcollapse(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit)
/* =============================================================== */
/* 
  collapse sÅÈquentiel, guidÅÈ et contraint
  OBSOLETE - utiliser l2dpardircollapse_l
*/
#undef F_NAME
#define F_NAME "l2dcollapse"
{
  int32_t u, v, n, x, y, xv, yv;
  index_t i, rs, cs, N;
  uint8_t * K;
  int32_t * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  index_t taillemaxrbt;
  index_t tab[GRS2D*GCS2D];

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(0);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != 1))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(0);
  }
  if (datatype(prio) == VFF_TYP_4_BYTE) 
    P = SLONGDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be int32_t\n", F_NAME);
    return(0);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != 1))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemaxrbt = 2 * (rs + cs);
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    i = y*rs + x;
    if (K[i] && ((I == NULL) || (!I[i])) && FaceLibre2d(k, x, y))
    {
      mcrbt_RbtInsert(&RBT, P[i], i);
      Set(i, EN_RBT);
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    i = RbtPopMin(RBT);
    UnSet(i, EN_RBT);
    x = i % rs; y = i / rs;
    u = Collapse2d(k, x, y);
    if (u != -1)
    {
      x = u % rs; y = u / rs;
      Alphacarre2d(rs, cs, x, y, tab, &n);
      for (u = 0; u < n; u += 1)
      {
        v = tab[u];
	xv = v % rs; yv = v / rs;
	if (K[v] && !IsSet(v, EN_RBT) && ((I == NULL) || (!I[v])) && FaceLibre2d(k, xv, yv))
	{
	  mcrbt_RbtInsert(&RBT, P[v], v);
	  Set(v, EN_RBT);
	}
      }
    }
  } /* while (!mcrbt_RbtVide(RBT)) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  return 1;

} /* l2dcollapse() */

/* =============================================================== */
int32_t l2dpardircollapse_l(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, int32_t priomax)
/* =============================================================== */
/* 
  collapse parallÅËle directionnel
  fonction de prioritÅÈ en entiers longs
  les ÅÈlÅÈments Å‡ prÅÈserver sont ceux de l'image "inhibit" ou, si celle-ci est "NULL", ceux supÅÈrieurs Å‡ "priomax" 
*/
#undef F_NAME
#define F_NAME "l2dpardircollapse_l"
{
  int32_t g, f, u, n, xf, yf, xg, yg;
  index_t i, rs, cs, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  int32_t * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  index_t taillemaxrbt;
  index_t tab[GRS2D*GCS2D];
  TypRbtKey p, pp;

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(0);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != 1))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(0);
  }
  if (datatype(prio) == VFF_TYP_4_BYTE) 
    P = SLONGDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be long\n", F_NAME);
    return(0);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != 1))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemaxrbt = (rs * cs)/8;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  RLIFO = CreeRlifoVide(taillemaxrbt);
  RLIFOb = CreeRlifoVide(taillemaxrbt);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  printf("%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = yg*rs + xg;
    if (K[g])
    {
      f = PaireLibre2d(k, xg, yg);
      if ((f != -1) && 
	  (((I != NULL) && (!I[g] && !I[f])) || 
	   ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
      {
	pp = (TypRbtKey)(mcmax(P[g],P[f]));
	mcrbt_RbtInsert(&RBT, pp, g);
	Set(g, EN_RBT);
      }
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    // construit la liste de toutes les paires libres ayant la prioritÅÈ courante
    p = RbtMinLevel(RBT); 
    while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))
    {
      g = RbtPopMin(RBT);
      UnSet(g, EN_RBT);
      xg = g % rs; yg = g / rs;
      f = PaireLibre2d(k, xg, yg);
      if (f != -1)
      {
	RlifoPush(&RLIFO, f);
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
      }
    } // while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))

    for (dir = 0; dir <= 1; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 2; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = f / rs;
	    if (K[f] && K[g] &&
		(((I != NULL) && (!I[g] && !I[f])) || 
		 ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
	    {
	      xg = g % rs; yg = g / rs;
	      if (xf - xg) { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else         { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      if ((DIM2D(xf,yf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE
#ifdef TRACECOL
		printf("Collapse : %d,%d -> %d,%d\n", xg, yg, xf, yf);
#endif
		Alphacarre2d(rs, cs, xf, yf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = g / rs;
		  if (K[g] && 
		      (((I != NULL) && (!I[g])) || ((I == NULL) && (P[g] < priomax))) )
		  {
		    f = PaireLibre2d(k, xg, yg);
		    if ((f != -1) &&
			(((I != NULL) && (!I[f])) || ((I == NULL) && (P[f] < priomax))) )
		    {
		      if ((P[g] <= p) && (P[f] <= p) && !IsSet(g, EN_RLIFO))
		      { // PrÅÈparation sous-ÅÈtapes suivantes
			RlifoPush(&RLIFOb, f);
			RlifoPush(&RLIFOb, g);
		      }
		      if (!IsSet(g, EN_RBT))
		      { // PrÅÈparation ÅÈtape suivante
			pp = (TypRbtKey)(mcmax(P[g],P[f]));
			mcrbt_RbtInsert(&RBT, pp, g);
			Set(g, EN_RBT);
		      }
		    } // if (f != -1)
		  } // if K[g] && ...
		} // for u
	      } // if ((DIM2D(xf,yf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 2; dim >= 1; dim--)
    } // for for

    RlifoFlush(RLIFO);
  } /* while (!mcrbt_RbtVide(RBT)) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l2dpardircollapse_l() */

/* =============================================================== */
int32_t l2dpardircollapse_f(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, float priomax)
/* =============================================================== */
/* 
  collapse parallÅËle directionnel
  fonction de prioritÅÈ en flottants
  les ÅÈlÅÈments Å‡ prÅÈserver sont ceux de l'image "inhibit" ou, si celle-ci est "NULL", ceux supÅÈrieurs Å‡ "priomax" 
*/
#undef F_NAME
#define F_NAME "l2dpardircollapse_f"
{
  int32_t g, f, u, n, xf, yf, xg, yg;
  index_t i, rs, cs, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  float * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  index_t taillemaxrbt;
  index_t tab[GRS2D*GCS2D];
  TypRbtKey p, pp;

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(0);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != 1))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(0);
  }
  if (datatype(prio) == VFF_TYP_FLOAT) 
    P = FLOATDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be float\n", F_NAME);
    return(0);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != 1))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemaxrbt = (rs * cs)/8;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  RLIFO = CreeRlifoVide(taillemaxrbt);
  RLIFOb = CreeRlifoVide(taillemaxrbt);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  printf("%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = yg*rs + xg;
    if (K[g])
    {
      f = PaireLibre2d(k, xg, yg);
      if ((f != -1) && 
	  (((I != NULL) && (!I[g] && !I[f])) || 
	   ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
      {
	pp = (TypRbtKey)(mcmax(P[g],P[f]));
	mcrbt_RbtInsert(&RBT, pp, g);
	Set(g, EN_RBT);
      }
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    // construit la liste de toutes les paires libres ayant la prioritÅÈ courante
    p = RbtMinLevel(RBT); 
    while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))
    {
      g = RbtPopMin(RBT);
      UnSet(g, EN_RBT);
      xg = g % rs; yg = g / rs;
      f = PaireLibre2d(k, xg, yg);
      if (f != -1)
      {
	RlifoPush(&RLIFO, f);
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
      }
    } // while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))

    for (dir = 0; dir <= 1; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 2; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = f / rs;
	    if (K[f] && K[g] &&
		(((I != NULL) && (!I[g] && !I[f])) || 
		 ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
	    {
	      xg = g % rs; yg = g / rs;
	      if (xf - xg) { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else         { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      if ((DIM2D(xf,yf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE
#ifdef TRACECOL
		printf("Collapse : %d,%d -> %d,%d\n", xg, yg, xf, yf);
#endif
		Alphacarre2d(rs, cs, xf, yf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = g / rs;
		  if (K[g] && 
		      (((I != NULL) && (!I[g])) || ((I == NULL) && (P[g] < priomax))) )
		  {
		    f = PaireLibre2d(k, xg, yg);
		    if ((f != -1) &&
			(((I != NULL) && (!I[f])) || ((I == NULL) && (P[f] < priomax))) )
		    {
		      if ((P[g] <= p) && (P[f] <= p) && !IsSet(g, EN_RLIFO))
		      { // PrÅÈparation sous-ÅÈtapes suivantes
			RlifoPush(&RLIFOb, f);
			RlifoPush(&RLIFOb, g);
		      }
		      if (!IsSet(g, EN_RBT))
		      { // PrÅÈparation ÅÈtape suivante
			pp = (TypRbtKey)(mcmax(P[g],P[f]));
			mcrbt_RbtInsert(&RBT, pp, g);
			Set(g, EN_RBT);
		      }
		    } // if (f != -1)
		  } // if K[g] && ...
		} // for u
	      } // if ((DIM2D(xf,yf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 2; dim >= 1; dim--)
    } // for for

    RlifoFlush(RLIFO);
  } /* while (!mcrbt_RbtVide(RBT)) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l2dpardircollapse_f() */

/* =============================================================== */
int32_t l2dpardircollapse_short(struct xvimage * k, int32_t nsteps)
/* =============================================================== */
{
#undef F_NAME
#define F_NAME "l2dpardircollapse_short"

  int result;
  result = l2dpardircollapse(k, nsteps, NULL);
  return result;
} /* l2dpardircollapse_short */

/* =============================================================== */
int32_t l2dpardircollapse(struct xvimage * k, int32_t nsteps, struct xvimage * inhibit)
/* =============================================================== */
/* 
  collapse parallÅËle directionnel
  sans fonction de prioritÅÈ
*/
#undef F_NAME
#define F_NAME "l2dpardircollapse"
{
  int32_t g, f, u, n, xf, yf, xg, yg;
  index_t i, rs, cs, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  uint8_t * I = NULL;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  Rlifo * RLIFOt;
  index_t taillemax;
  index_t tab[GRS2D*GCS2D];

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != 1))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(0);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(0);
    }
  }

  taillemax = (rs * cs)/4;
  RLIFO = CreeRlifoVide(taillemax);
  RLIFOb = CreeRlifoVide(taillemax);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(0);
  }

  if (nsteps == -1) nsteps = 1000000000;

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /* INITIALISATION DE LA RLIFO ET DE LA "BORDER" */
  /* ========================================================= */

  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = yg*rs + xg;
    if (K[g] && ((I == NULL) || (!I[g])))
    {
      f = PaireLibre2d(k, xg, yg);
      if (f != -1) 
      { 
	RlifoPush(&RLIFO, f); 
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
	xf = f % rs; yf = f / rs;
	Alphacarre2d(rs, cs, xf, yf, tab, &n);
	for (u = 0; u < n; u += 1)
	{
	  g = tab[u];
	  Set(g, BORDER);
	} // for u
      }
    }
  }

  /* ================================================ */
  /*              DEBUT BOUCLE PRINCIPALE             */
  /* ================================================ */

  while (!RlifoVide(RLIFO) && nsteps > 0)
  {
    nsteps --;

    for (dir = 0; dir <= 1; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 2; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = f / rs;
	    if (K[f] && K[g] && ((I == NULL) || (!I[g] && !I[f])))
	    {
	      xg = g % rs; yg = g / rs;
	      if (xf - xg) { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else         { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      if ((DIM2D(xf,yf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE
		// PrÅÈparation sous-ÅÈtapes suivantes
		Alphacarre2d(rs, cs, xf, yf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  g = tab[u];
		  xg = g % rs; yg = g / rs;
		  if (K[g] && ((I == NULL) || (!I[g])))
		  {
		    f = PaireLibre2d(k, xg, yg);
		    if ((f != -1) && IsSet(f, BORDER) && !IsSet(g, EN_RLIFO))
		    { 
		      RlifoPush(&RLIFOb, f); 
		      RlifoPush(&RLIFOb, g);
		    }
		  }
		} // for u
	      } // if ((DIM2D(xf,yf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 2; dim >= 1; dim--)
    } // for for

    // PREPARATION ETAPE SUIVANTE
    for (i = 0; i < RLIFO->Sp; i++) UnSet(RLIFO->Pts[i], EN_RLIFO);
    for (i = 0; i < RLIFO->Sp; i += 2)
    {
      f = RLIFO->Pts[i];
      UnSet(f, BORDER);
      xf = f % rs; yf = f / rs;
      Alphacarre2d(rs, cs, xf, yf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	UnSet(g, BORDER);
	if (K[g] && !IsSet(g, EN_RLIFO) && ((I == NULL) || (!I[g])))
	{
	  xg = g % rs; yg = g / rs;
	  f = PaireLibre2d(k, xg, yg);
	  if (f != -1) 
	  { 
	    RlifoPush(&RLIFOb, f); 
	    RlifoPush(&RLIFOb, g);
	    Set(g, EN_RLIFO);
	  }
	}
      } // for u
    }

    for (i = 0; i < RLIFOb->Sp; i += 2)
    {
      f = RLIFOb->Pts[i];
      xf = f % rs; yf = f / rs;
      Alphacarre2d(rs, cs, xf, yf, tab, &n);
      for (u = 0; u < n; u += 1)
      {
	g = tab[u];
	Set(g, BORDER);
      } // for u
    }

    RlifoFlush(RLIFO);

    RLIFOt = RLIFOb;
    RLIFOb = RLIFO;
    RLIFO = RLIFOt;

  } // while (!RlifoVide(RLIFO) && nsteps > 0)

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return 1;

} /* l2dpardircollapse() */

// 888888888888888888888888888888888888888888888888888888888888888888
// 888888888888888888888888888888888888888888888888888888888888888888
//                        TOPOLOGICAL FLOW
// 888888888888888888888888888888888888888888888888888888888888888888
// 888888888888888888888888888888888888888888888888888888888888888888
 
/* =============================================================== */
graphe * l2dtopoflow_l(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, float priomax)
/* =============================================================== */
/* 
  construction du flot topologique associÅÈ Å‡ un collapse parallÅËle directionnel
  (voir l2dpardircollapse_f)
  fonction de prioritÅÈ en entiers longs
*/
#undef F_NAME
#define F_NAME "l2dtopoflow_l"
{
  fprintf(stderr, "%s: Not Yet Implemented\n", F_NAME);
  return(NULL);
} /* l2dtopoflow_l() */
 
/* =============================================================== */
graphe * l2dtopoflow_f(struct xvimage * k, struct xvimage * prio, struct xvimage * inhibit, float priomax)
/* =============================================================== */
/* 
  construction du flot topologique associÅÈ Å‡ un collapse parallÅËle directionnel
  (voir l2dpardircollapse_f)
  fonction de prioritÅÈ en flottants
  le rÅÈsultat est un graphe dont les sommets (faces du complexe k) 
  sont valuÅÈs par :
    TF_NOT_IN_I: la face n'appartient pas au complexe initial (avant collapse)
    TF_HEAD: la face est une tete de paire libre (face libre)
    TF_TAIL: la face est une queue de paire libre
    TF_PERMANENT: la face n'appartient pas au complexe final (aprÅËs collapse)
*/
#undef F_NAME
#define F_NAME "l2dtopoflow_f"
{
  int32_t g, gg, f, ff, u, n, xf, yf, xg, yg;
  index_t i, rs, cs, N;
  int32_t dim, ori, dir, direc, orien;
  uint8_t * K;
  float * P;
  uint8_t * I = NULL;
  Rbt * RBT;
  Rlifo * RLIFO;
  Rlifo * RLIFOb;
  index_t taillemaxrbt;
  index_t tab[GRS2D*GCS2D];
  TypRbtKey p, pp;
  graphe * flow = NULL;
  index_t narcs, ncoll = 0;

  rs = rowsize(k);
  cs = colsize(k);
  N = rs * cs;
  K = UCHARDATA(k);

  IndicsInit(N);

  if (prio == NULL)
  {
    fprintf(stderr, "%s : prio is needed\n", F_NAME);
    return(NULL);
  }

  if ((rowsize(prio) != rs) || (colsize(prio) != cs) || (depth(prio) != 1))
  {
    fprintf(stderr, "%s : bad size for prio\n", F_NAME);
    return(NULL);
  }
  if (datatype(prio) == VFF_TYP_FLOAT) 
    P = FLOATDATA(prio); 
  else 
  {
    fprintf(stderr, "%s : datatype(prio) must be float\n", F_NAME);
    return(NULL);
  }

  if (inhibit != NULL)
  {
    if ((rowsize(inhibit) != rs) || (colsize(inhibit) != cs) || (depth(inhibit) != 1))
    {
      fprintf(stderr, "%s : bad size for inhibit\n", F_NAME);
      return(NULL);
    }
    if (datatype(inhibit) == VFF_TYP_1_BYTE) 
      I = UCHARDATA(inhibit); 
    else 
    {
      fprintf(stderr, "%s : datatype(inhibit) must be uint8_t\n", F_NAME);
      return(NULL);
    }
  }

  taillemaxrbt = N/8;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(NULL);
  }

  RLIFO = CreeRlifoVide(taillemaxrbt);
  RLIFOb = CreeRlifoVide(taillemaxrbt);
  if ((RLIFO == NULL) || (RLIFOb == NULL))
  {
    fprintf(stderr, "%s : CreeRlifoVide failed\n", F_NAME);
    return(NULL);
  }

  /* ================================================ */
  /*               INITIALISATION GRAPHE              */
  /* ================================================ */

  narcs = 0; // evalue la taille max du graphe
  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    if (K[yg*rs + xg]) 
    { 
      if (CARRE(xg,yg)) narcs += 8;
      else if (INTER(xg,yg)) narcs += 2;
    }
  }
  flow = InitGraphe(N, narcs); // toutes les faces (dans K ou non) sont des sommets
  if (flow == NULL)
  {
    fprintf(stderr, "%s : InitGraphe failed\n", F_NAME);
    return(NULL);
  }
  for (yg = 0; yg < cs; yg++) // coordonnÅÈes des sommets
  for (xg = 0; xg < rs; xg++)
  {
    g = yg*rs + xg;
    flow->x[g] = xg; // coordonnÅÈes des sommets
    flow->y[g] = yg;
    if (!K[g]) flow->v_sommets[g] = TF_NOT_IN_I;
  }

  /* ================================================ */
  /*               DEBUT ALGO                         */
  /* ================================================ */

#ifdef VERBOSE
  fprintf(stderr, "%s: Debut traitement\n", F_NAME);
#endif

  /* ========================================================= */
  /*   INITIALISATION DU RBT */
  /* ========================================================= */

  for (yg = 0; yg < cs; yg++)
  for (xg = 0; xg < rs; xg++)
  {
    g = yg*rs + xg;
    if (K[g])
    {
      f = PaireLibre2d(k, xg, yg);
      if ((f != -1) && 
	  (((I != NULL) && (!I[g] && !I[f])) || 
	   ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
      {
	pp = (TypRbtKey)(mcmax(P[g],P[f]));
	mcrbt_RbtInsert(&RBT, pp, g);
	Set(g, EN_RBT);
      }
    }
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  while (!mcrbt_RbtVide(RBT))
  {
    // construit la liste de toutes les paires libres ayant la prioritÅÈ courante
    p = RbtMinLevel(RBT); 
    while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))
    {
      g = RbtPopMin(RBT);
      UnSet(g, EN_RBT);
      xg = g % rs; yg = g / rs;
      f = PaireLibre2d(k, xg, yg);
      if (f != -1)
      {
	RlifoPush(&RLIFO, f);
	RlifoPush(&RLIFO, g);
	Set(g, EN_RLIFO);
      }
    } // while (!mcrbt_RbtVide(RBT) && (RbtMinLevel(RBT) == p))

    for (dir = 0; dir <= 1; dir++) // For all face directions
      for (ori = 0; ori <= 1; ori++) // For both orientations
      {
	for (dim = 2; dim >= 1; dim--) // For dimensions in decreasing order
	{
	  for (i = 0; i < RLIFO->Sp; i += 2) // Scan the free faces list
	  {
	    f = RLIFO->Pts[i];
	    g = RLIFO->Pts[i+1];
	    xf = f % rs; yf = f / rs;
	    if (K[f] && K[g] && 
		(((I != NULL) && (!I[g] && !I[f])) || 
		 ((I == NULL) && (P[g] < priomax) && (P[f] < priomax)) ) )
	    {
	      xg = g % rs; yg = g / rs;
	      if (xf - xg) { direc = 0; if (xf > xg) orien = 0; else orien = 1; }
	      else         { direc = 1; if (yf > yg) orien = 0; else orien = 1; }
	      if ((DIM2D(xf,yf) == dim) && (direc == dir) && (orien == ori))
	      {
		K[g] = K[f] = VAL_NULLE; // COLLAPSE
		ncoll ++;
#ifdef TRACECOL
		printf("Collapse : %d,%d -> %d,%d\n", xg, yg, xf, yf);
#endif
		flow->v_sommets[g] = TF_HEAD;
#ifdef DESSINECOLSEQ
		flow->v_sommets[f] = -ncoll;//pour dessiner une col. seq.
#else
      		flow->v_sommets[f] = TF_TAIL;
#endif
		AjouteArcValue(flow, g, f, (TYP_VARC)1);
#ifdef TRACECONSGRAPH
		printf("Ajoute arc : %d [%d,%d] -> %d [%d,%d] (%g)\n", g, xg, yg, f, xf, yf, (TYP_VARC)1);
#endif
		Alphacarre2d(rs, cs, xf, yf, tab, &n);
		for (u = 0; u < n; u += 1)
		{
		  gg = tab[u];
		  xg = gg % rs; yg = gg / rs;
		  if (K[gg]) 
		  {
#ifdef TRACECONSGRAPH
		    printf("Ajoute arc : %d [%d,%d] -> %d [%d,%d] (%g)\n", f, xf, yf, gg, xg, yg, (TYP_VARC)1);
#endif
		    AjouteArcValue(flow, f, gg, (TYP_VARC)1);
		  }
		  if (K[gg] && 
		      (((I != NULL) && (!I[gg])) || ((I == NULL) && (P[gg] < priomax))) )
		  {
		    ff = PaireLibre2d(k, xg, yg);
		    if ((ff != -1) &&
			(((I != NULL) && (!I[ff])) || ((I == NULL) && (P[ff] < priomax))) )
		    {
		      if ((P[gg] <= p) && (P[ff] <= p) && !IsSet(gg, EN_RLIFO))
		      { // PrÅÈparation sous-ÅÈtapes suivantes
			RlifoPush(&RLIFOb, ff);
			RlifoPush(&RLIFOb, gg);
		      }
		      if (!IsSet(gg, EN_RBT))
		      { // PrÅÈparation ÅÈtape suivante
			pp = (TypRbtKey)(mcmax(P[gg],P[ff]));
			mcrbt_RbtInsert(&RBT, pp, gg);
			Set(gg, EN_RBT);
		      }
		    } // if (ff != -1)
		  } // if K[gg] && ...
		} // for u
	      } // if ((DIM2D(xf,yf) == dim) &&...
	    } // if (K[f] && K[g])
	  } // for (i = 0; i < RLIFO->Sp; i += 2)
	  while (!RlifoVide(RLIFOb))
	  { 
	    g = RlifoPop(RLIFOb);
	    f = RlifoPop(RLIFOb);
	    RlifoPush(&RLIFO, f); 
	    RlifoPush(&RLIFO, g); 
	  }
      } // for (dim = 2; dim >= 1; dim--)
    } // for for

    RlifoFlush(RLIFO);

  } /* while (!mcrbt_RbtVide(RBT)) */

  for (g = 0; g < N; g++)
    if (K[g]) flow->v_sommets[g] = TF_PERMANENT;

#ifdef VERBOSE
  fprintf(stderr, "%s: Fin traitement\n", F_NAME);
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  RlifoTermine(RLIFO);
  RlifoTermine(RLIFOb);
  return(flow);

} /* l2dtopoflow_f() */
