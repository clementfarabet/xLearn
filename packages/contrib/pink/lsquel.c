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
/* operateur de calcul du squelette 2.5D */
/* methode: saturation avec politique 4-directionnelle */
/*          utilise 4 fifos */
/*          test des points extremites de 8-courbes */
/*          atteints au bout de niseuil iterations */
/* Michel Couprie -  octobre 1997 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfifo.h>
#include <mctopo.h>
#include <mcindic.h>
#ifdef PERF
#include <mcchrono.h>
#endif
#include <llabelextrema.h>
#include <lsquel.h>

#define EN_FIFO       0
#define MINI          1
#define COURBE        2
#define PARANO
/*
#define DEBUG
*/
#define REGULARISE

/* ==================================== */
int32_t testabaisse(uint8_t *SOURCE, 
                int32_t x, int32_t rs, int32_t N, int32_t seuil, int32_t niter, int32_t niseuil)
/* ==================================== */
{
  int32_t t4mm, t4m, t8p, t8pp;
  int32_t modifie = 0;

#ifdef DEBUG
printf("testabaisse : point %d (%d %d), val = %d\n", x, x%rs, x/rs, SOURCE[x]);
#endif

  nbtopo(SOURCE, x, rs, N, &t4mm, &t4m, &t8p, &t8pp);

  if (t4mm == 2)
    Set(x, COURBE);

#ifdef REGULARISE
  if (IsSet(x, COURBE))
  {
    int32_t valmaxmin = -1;
    int32_t k, y;
    /* 
       si c'est un point de courbe "inconsistant", on l'abaisse
       def. x inconsistant : 
         l'ecart entre F(x) et la valeur du minimum voisin le plus proche est < seuil
    */
    
    for (k = 0; k < 8; k += 2)               /* parcourt les voisins en 4-connexite */
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (IsSet(y, MINI)) &&  ((int32_t)(SOURCE[y]) > valmaxmin))
        valmaxmin = (int32_t)(SOURCE[y]); 
    } /* for k */
    if ((valmaxmin > -1) && ((SOURCE[x] - (uint8_t)valmaxmin) < seuil))
    {
      modifie = 1;
      SOURCE[x] = (uint8_t)valmaxmin;
      UnSet(x, COURBE);
    }
  }
#endif

/*  if (!IsSet(x, COURBE) && */
  if ((niter <= niseuil) || !extremite8(SOURCE, x, rs, N))
  {
    while ((t4mm == 1) && (t8p == 1))
    {
      modifie = 1;
      SOURCE[x] = alpha8m(SOURCE, x, rs, N);
      nbtopo(SOURCE, x, rs, N, &t4mm, &t4m, &t8p, &t8pp);
    }
  } /* if (!extremite8(SOURCE, x, rs, N)) */
#ifdef DEBUG
if (modifie) printf("========> ABAISSE : %d\n", SOURCE[x]);
#endif

  return modifie;
} /* testabaisse() */

/* ==================================== */
void testmini(uint8_t *SOURCE, int32_t x, int32_t rs, int32_t N, Fifo *FIFO)
/* ==================================== */
{
  int32_t k, y, kk, yy, w;
  uint8_t valmin;

#ifdef REGULARISE
  for (k = 0; k < 8; k += 2)               /* parcourt les voisins en 4-connexite */
  {                                        /* pour voir s'il existe un minimum voisin */
    y = voisin(x, k, rs, N);               /* a un niveau > a celui atteint par x */
    if ((y != -1) && (IsSet(y, MINI)) && (SOURCE[y] > SOURCE[x])) 
    {                                      /* ce n'est plus un minimum */
      FifoPush(FIFO, y);
      UnSet(y, MINI);
      valmin = SOURCE[y];
      while (! FifoVide(FIFO)) /* parcours pour demarquer l'ex minimum */
      {
        w = FifoPop(FIFO);
        for (kk = 0; kk < 8; kk += 2)
        {
          yy = voisin(w, kk, rs, N);
          if ((yy != -1) && (SOURCE[yy] == valmin) && (IsSet(yy, MINI)))
          { 
            FifoPush(FIFO, yy);
	    UnSet(yy, MINI);
          }
        } /* for kk ... */
      } /* while (! FifoVide(FIFO)) */
    } /* if ((y != -1) && (IsSet(y, MINI)) && (SOURCE[y] > SOURCE[x])) */
  } /* for k */
#endif

  for (k = 0; k < 8; k += 2)  /* parcourt les voisins en 4-connexite */
  {                           /* pour voir s'il existe un minimum */
    y = voisin(x, k, rs, N);  /* au niveau atteint par x */
    if ((y != -1) && (IsSet(y, MINI)) && (SOURCE[y] == SOURCE[x])) 
    { 
      Set(x, MINI); 
      break; 
    }
  } /* for k */
} /* testmini() */

/* ==================================== */
void empilevoisins(int32_t x, int32_t rs, int32_t N, Fifo *FIFOna, Fifo *FIFOea, Fifo *FIFOsa, Fifo *FIFOoa)
/* ==================================== */
{
  int32_t y, z, k;

  /* x est un point qui vient de passer a 0 */
  for (k = 0; k < 8; k++)
  {
    y = voisin(x, k, rs, N);
    if ((y!=-1) && !IsSet(y,MINI) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
    { 
      z = voisin(y, NORD, rs, N);
      if ((z != 0) && IsSet(z, MINI))
      { FifoPush(FIFOna, y); Set(y, EN_FIFO); goto nextk; }
      z = voisin(y, SUD, rs, N);
      if ((z != 0) && IsSet(z, MINI))
      { FifoPush(FIFOsa, y); Set(y, EN_FIFO); goto nextk; }
      z = voisin(y, EST, rs, N);
      if ((z != 0) && IsSet(z, MINI))
      { FifoPush(FIFOea, y); Set(y, EN_FIFO); goto nextk; }
      z = voisin(y, OUEST, rs, N);
      if ((z != 0) && IsSet(z, MINI))
      { FifoPush(FIFOoa, y); Set(y, EN_FIFO); goto nextk; }
    }
nextk: ;
  }
} /* empilevoisins() */

/* ==================================== */
int32_t lsquel(struct xvimage *image, int32_t seuil, int32_t niseuil)
/* ==================================== */
{ 
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *lab;
  int32_t *M;            /* l'image d'etiquettes de composantes connexes */
  int32_t nminima;                 /* nombre de minima differents */
  Fifo * FIFOn;
  Fifo * FIFOs;
  Fifo * FIFOe;
  Fifo * FIFOo;
  Fifo * FIFOna;
  Fifo * FIFOsa;
  Fifo * FIFOea;
  Fifo * FIFOoa;
  Fifo * FIFOtmp;
  Fifo * FIFO;
  int32_t niter;
#ifdef PERF
  chrono chrono1;
#endif

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lsquel: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

#ifdef PERF  
  start_chrono(&chrono1);  /* pour l'analyse de performances */
#endif

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "lhtkern: allocimage failed\n");
    return 0;
  }
  M = SLONGDATA(lab);

  if (!llabelextrema(image, 4, LABMIN, lab, &nminima))
  {   
    fprintf(stderr, "lhtkern: llabelextrema failed\n");
    return 0;
  }

  IndicsInit(N);

  FIFO = CreeFifoVide(N);
  FIFOn = CreeFifoVide(N/2);
  FIFOs = CreeFifoVide(N/2);
  FIFOe = CreeFifoVide(N/2);
  FIFOo = CreeFifoVide(N/2);
  if ((FIFO == NULL) && (FIFOn == NULL) && (FIFOs == NULL) && (FIFOe == NULL) && (FIFOo == NULL))
  {   fprintf(stderr, "lsquel() : CreeFifoVide failed\n");
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT ALGO                      */
  /* ================================================ */

  /* ========================================================= */
  /*   INITIALISATION DES FIFOs: empile les voisins des minima */
  /* ========================================================= */

  for (x = 0; x < N; x++)
  {
    if (M[x] != 0)                  /* le pixel appartient a un minimum */
    {
        Set(x, MINI);
        y = voisin(x, NORD, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOn, y); Set(y, EN_FIFO); }
#ifdef DIAG
        y = voisin(x, NORD+1, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOn, y); Set(y, EN_FIFO); }
#endif
        y = voisin(x, EST, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOe, y); Set(y, EN_FIFO); }
#ifdef DIAG
        y = voisin(x, EST+1, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOe, y); Set(y, EN_FIFO); }
#endif
        y = voisin(x, SUD, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOs, y); Set(y, EN_FIFO); }
#ifdef DIAG
        y = voisin(x, SUD+1, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOs, y); Set(y, EN_FIFO); }
#endif
        y = voisin(x, OUEST, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOo, y); Set(y, EN_FIFO); }
#ifdef DIAG
        y = voisin(x, OUEST+1, rs, N);
        if ((y!=-1) && (M[y]==0) && !IsSet(y,EN_FIFO) && nonbord(y,rs,N))
          { FifoPush(FIFOo, y); Set(y, EN_FIFO); }
#endif
    } /* if (M[x] != 0) */
  } /* for x */

  freeimage(lab);

  FIFOna = CreeFifoVide(N/4);
  FIFOsa = CreeFifoVide(N/4);
  FIFOea = CreeFifoVide(N/4);
  FIFOoa = CreeFifoVide(N/4);
  if ((FIFOna == NULL) && (FIFOsa == NULL) && (FIFOea == NULL) && (FIFOoa == NULL))
  {   fprintf(stderr, "lsquel() : CreeFifoVide failed\n");
      return(0);
  }

  /* ================================================ */
  /*                  DEBUT SATURATION                */
  /* ================================================ */

  niter = 0;
  while (! (FifoVide(FIFOn) && FifoVide(FIFOe) && FifoVide(FIFOs) && FifoVide(FIFOo)))
  {
    niter++;
    while (! FifoVide(FIFOn))
    {
      x = FifoPop(FIFOn);
      UnSet(x, EN_FIFO);
      if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil))
      {                                    /* modifie l'image le cas echeant */
        testmini(SOURCE, x, rs, N, FIFO);        /* reactualise l'image MINI */
        empilevoisins(x, rs, N, FIFOna, FIFOea, FIFOsa, FIFOoa);
      } /* if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil)) */
    } /* while (! FifoVide(FIFOn)) */

    while (! FifoVide(FIFOs))
    {
      x = FifoPop(FIFOs);
      UnSet(x, EN_FIFO);
      if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil))
      {                                    /* modifie l'image le cas echeant */
        testmini(SOURCE, x, rs, N, FIFO);        /* reactualise l'image MINI */
        empilevoisins(x, rs, N, FIFOna, FIFOea, FIFOsa, FIFOoa);
      } /* if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil)) */
    } /* while (! FifoVide(FIFOs)) */

    while (! FifoVide(FIFOe))
    {
      x = FifoPop(FIFOe);
      UnSet(x, EN_FIFO);
      if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil))
      {                                    /* modifie l'image le cas echeant */
        testmini(SOURCE, x, rs, N, FIFO);        /* reactualise l'image MINI */
        empilevoisins(x, rs, N, FIFOna, FIFOea, FIFOsa, FIFOoa);
      } /* if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil)) */
    } /* while (! FifoVide(FIFOe)) */

    while (! FifoVide(FIFOo))
    {
      x = FifoPop(FIFOo);
      UnSet(x, EN_FIFO);
      if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil))
      {                                    /* modifie l'image le cas echeant */
        testmini(SOURCE, x, rs, N, FIFO);        /* reactualise l'image MINI */
        empilevoisins(x, rs, N, FIFOna, FIFOea, FIFOsa, FIFOoa);
      } /* if (testabaisse(SOURCE, x, rs, N, seuil, niter, niseuil)) */
    } /* while (! FifoVide(FIFOo)) */

    FIFOtmp = FIFOn; FIFOn = FIFOna; FIFOna = FIFOtmp;
    FIFOtmp = FIFOe; FIFOe = FIFOea; FIFOea = FIFOtmp;
    FIFOtmp = FIFOs; FIFOs = FIFOsa; FIFOsa = FIFOtmp;
    FIFOtmp = FIFOo; FIFOo = FIFOoa; FIFOoa = FIFOtmp;

  } /* while (! (FifoVide(FIFOn) && FifoVide(FIFOe) && FifoVide(FIFOs) && FifoVide(FIFOo))) */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FifoTermine(FIFO);
  FifoTermine(FIFOn);
  FifoTermine(FIFOe);
  FifoTermine(FIFOs);
  FifoTermine(FIFOo);
  FifoTermine(FIFOna);
  FifoTermine(FIFOea);
  FifoTermine(FIFOsa);
  FifoTermine(FIFOoa);

#ifdef PERF
  save_time(N, read_chrono(&chrono1), "lsquel", image->name);    
#endif
  return(1);
}
/* -library_code_end */
