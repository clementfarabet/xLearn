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
/* operateur fondamental de segmentation par seuillage pour image 3d numerique*/
/* utilise une File d'Attente Hierarchique */
/* utilise un arbre des bassins versants (captation basin tree, CBT) */
/* Michel Couprie - javvier 1997 */
/* version 0.4 du 25/8/97 */
/*
  1/2/98 : retire   UnSetAll(x);   apres        ncc = 0; x = FahPop(FAH);
           remplace
              n = FirstSon(CBT, y + 1) - FirstSon(CBT, y);
           par
              if (y < nbcell - 1)
                n = FirstSon(CBT, y + 1) - FirstSon(CBT, y);
              else
                n = nbcell - 2 - FirstSon(CBT, y);
   20/4/00 : corrige bug 
*/

#define PARANO                 /* even paranoid people have ennemies */
#define VERBOSE

/*
#define MAXIMISE
*/

/*
#define TRACEIMAGE
*/
#define TRACEPOP
#define TRACEPUSH
#define TRACEBAISSE
#define TRACEETIQ
#define SEGMENT
#define TRACECBT


/*
   Methode : 

===============================================
1ere etape : CONSTRUCTION DU CBT
===============================================
  entree : une image F
  sortie : le CBT, l'application M: Dom(F) -> CBT

  M <- etiquettes des minima de F
  nminima <- nombre de minima de F
  init(CBT, nminima)
  pour tout point x de Dom(F)
    si x appartient a un minimum
      SetData(CBT, M[x], SOURCE[x] + 1);
      pour tout y dans gamma(x)
        si y n'appartient pas a un minimum et pas deja dans FAH
          FahPush(FAH, y, SOURCE[y]);
        finsi
      finpour
    finsi
  finpour

  tant que FAH non vide
    x = FahPop(FAH);
    etiqcc = liste des M[y], y dans gamma(x)
    diffanc = liste des ancetres differents des etiquettes de etiqcc dans le CBT
    si |diffanc| == 1
      M[x] = first(diffanc)
    sinon
      si il existe une ou des cellules de niveau F[x]+1 dans gamma(x)
        new = fusion de ces cellules
      sinon
        new = CreateCell(CBT)
      M[x] = new
      SetData(CBT, new, F[x] + 1)
      pour tout a dans diffanc 
        SetFather(CBT, a, new)
      finpour
    fin si
    pour tout y dans gamma(x) pas deja dans FAH
      si M[y] == 0
        FahPush(FAH, y, SOURCE[y]);
      finsi
    finpour
  fintantque

===============================================
2eme etape: 
===============================================

  en parallele avec la construction du CBT, on a recueilli une "mesure" (MU)
  des bassins versants.

  ici cette mesure est la surface de la plus grande coupe d'un BV avant sa fusion
  avec un autre.

  l'algorithme de segmentation est alors:

  1ERE ETAPE : SELECTIONNE LE NB MAX DE CC PERTINENTES

  init:  ranger au niveau 0 de la FAH les feuilles du CBT

  tant que FAH non vide faire
    k = FAHNIVEAU()
    x = FAHPOP()
    si MU[c] >= seuil alors
      marque les ancetres de x "INVALIDE"
      marque x "PERTINENT"
    sinon
      si pere(x) existe et non INVALIDE alors FAHPUSH(pere(x), k+1) finsi
    finsi
  fintantque

  2EME ETAPE : MAXIMISE AU SENS DE L'INCLUSION

  Re-init:  ranger a leur niveau data dans la FAH les cellules PERTINENTes

  I = arbre inverse

  tant que FAH non vide faire
    x = FAHPOP()
    y = pere(x)
    si y existe alors 
      k = FirstSon(y)
      n = FirstSon(y + 1) - FirstSon(y)
      nbfilspert = 0
      pour i de 0 a n-1 faire
        si Label(I[k+i]) == PERTINENT
          nbfilspert++ finsi finpour
      si nbfilspert == 1 alors
        Label(x) = NONMARQUE
        Label(y) = PERTINENT
        FahPush(y, Data(y))
      finsi
    finsi
  fintantque

*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfah.h>
#include <mcindic.h>
#include <mccbt.h>
#include <llabelextrema.h>
#include <lsegment3d.h>

#define EN_FAH 0
#define MINIMUM 1
#define CC_EN_FAH 2

#define NONMARQUE 0
#define INVALIDE 1
#define PERTINENT 2

/* ==================================== */
int32_t lsegment3d(
        struct xvimage *image,
        int32_t connex,
        int32_t mesure,
        int32_t seuil,
        int32_t maximise,
        int32_t trace)
/* ==================================== */
{
  register int32_t i;                       /* index muet */
  register int32_t x;                       /* index muet de pixel */
  register int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t d = depth(image);        /* nb plans */
  int32_t n = rs * cs;             /* taille plan */
  int32_t N = n * d;               /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *lab;
  uint32_t *M;            /* l'image d'etiquettes de composantes connexes */
  int32_t *MU;                     /* pour le stockage de la mesure mu */
  int32_t *T;                      /* table de correspondance pour regularisation */
  int32_t *I;                      /* pour l'arbre inverse */
  int32_t nminima;                 /* nombre de minima differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  cbtcell * CBT;               /* arbre des bassins versants (Catchment Basin Tree) */
  int32_t nbcell;
  int32_t nbmaxcell;
  int32_t etiqcc[26];
  int32_t ncc;
  int32_t new;
  int32_t prof;
  int32_t *SURF;                   /* pour le stockage de la "surface" */
  int32_t *PROF;                   /* pour le stockage de la derniere profondeur */
  int32_t tracedate = 0;
  
  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "lsegment3d() : CreeFah failed\n");
      return(0);
  }

  lab = allocimage(NULL, rs, cs, d, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "lsegment3d: allocimage failed\n");
    return 0;
  }
  M = SLONGDATA(lab);

  if (!llabelextrema(image, connex, LABMIN, lab, &nminima))
  {   
    fprintf(stderr, "lsegment3d: llabelextrema failed\n");
    return 0;
  }
  nminima -= 1;  /* le label 0 ne represente pas un minimum */

  nbcell = nminima+1;  /* +1 : pour le decalage entre no minima et no neuds - 20/4/00 */
  nbmaxcell = nbcell * 2; /* 21/4/00: nbcell au lieu de nminima */
  CBT = (cbtcell *)calloc(1,nbmaxcell * sizeof(cbtcell));
  if (CBT == NULL)
  {   fprintf(stderr, "lsegment3d() : malloc failed for CBT\n");
      return(0);
  }

  MU = (int32_t *)calloc(nbmaxcell, sizeof(int32_t));   /* init a 0 */
  if (MU == NULL)
  {   fprintf(stderr, "lsegment3d() : malloc failed for MU\n");
      return(0);
  }

  if (mesure == SURFACE)
  {
    /* calcul des tailles des minima (histogramme des labels) */
    for (x = 0; x < N; x++) MU[M[x]] += 1;
  }
  if (mesure == VOLUME)
  {
    SURF = (int32_t *)calloc(nbmaxcell, sizeof(int32_t));   /* init a 0 */
    PROF = (int32_t *)calloc(nbmaxcell, sizeof(int32_t));   /* init a 0 */
    if ((SURF == NULL) || (PROF == NULL))
    {   fprintf(stderr, "lsegment3d() : malloc failed for SURF or PROF\n");
        return(0);
    }
    /* calcul des tailles des minima (histogramme des labels) */
    /* et memorisation des altitudes */
    for (x = 0; x < N; x++) 
    {
      if (M[x] != 0)                   /* le pixel appartient a un minimum */
      {
        SURF[M[x]] += 1;
        PROF[M[x]] = SOURCE[x] + 1;
      }
    }
  } /* if (mesure == VOLUME) */

  /* ================================================ */
  /*                CONSTRUCTION DU CBT               */
  /* ================================================ */

  InitCbt(CBT, nbcell);
  
  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
                                     /* NECESSAIRE pour eviter la creation prematuree */
                                     /* de la file d'urgence */ 
  for (x = 0; x < N; x++)
  {
    if (M[x] != 0)                   /* le pixel appartient a un minimum */
    {
      Set(x, MINIMUM);
      SetData(CBT, M[x], SOURCE[x]);     /* conceptuellement : SOURCE[x] + 1 */
      switch (connex)
      {
      case 6:
        for (k = 0; k <= 10; k += 2)       /* parcourt les 6 voisins */
        {                                  /* si un voisin n'est pas deja dans la FAH */
          y = voisin6(x, k, rs, n, N);     /* et n'est pas dans un minimum, on le met en FAH */
          if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
	  {
            FahPush(FAH, y, SOURCE[y]);
            Set(y, EN_FAH);
#ifdef TRACEPUSH
            if (trace) printf("%d: empile point %d (%d %d,%d) au niveau %d\n", 
                               tracedate++, y, y/n, (y%n)%rs, (y%n)/rs, SOURCE[y]);
#endif
          } /* if y */
        } /* for k */
        break;

      case 18:
        for (k = 0; k < 18; k++)           /* parcourt les 18 voisins */
        {                                  /* si un voisin n'est pas deja dans la FAH */
          y = voisin18(x, k, rs, n, N);    /* et n'est pas dans un minimum, on le met en FAH */
          if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
	  {
            FahPush(FAH, y, SOURCE[y]);
            Set(y, EN_FAH);
#ifdef TRACEPUSH
            if (trace) printf("%d: empile point %d (%d %d,%d) au niveau %d\n", 
                               tracedate++, y, y/n, (y%n)%rs, (y%n)/rs, SOURCE[y]);
#endif
          } /* if y */
        } /* for k */
        break;

      case 26:
        for (k = 0; k < 26; k++)           /* parcourt les 26 voisins */
        {                                  /* si un voisin n'est pas deja dans la FAH */
          y = voisin26(x, k, rs, n, N);    /* et n'est pas dans un minimum, on le met en FAH */
          if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
	  {
            FahPush(FAH, y, SOURCE[y]);
            Set(y, EN_FAH);
#ifdef TRACEPUSH
            if (trace) printf("%d: empile point %d (%d %d,%d) au niveau %d\n", 
                               tracedate++, y, y/n, (y%n)%rs, (y%n)/rs, SOURCE[y]);
#endif
          } /* if y */
        } /* for k */
        break;

      } /* switch (connex) */
    } /* if (M[x] != 0) */
  } /* for x */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     printf("segment3d() : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n");
     return(0);
  }
#endif

  /* INONDATION */
  while (! FahVide(FAH))
  {
    ncc = 0;
    x = FahPop(FAH);

#ifdef TRACEPOP
    if (trace) 
    {
      printf("%d: POP point %d (%d %d,%d) ; niveau %d\n", 
              tracedate++, x, x/n, (x%n)%rs, (x%n)/rs, SOURCE[x]);
    }
#endif

    switch (connex)
    {
    case 6:
      for (k = 0; k <= 10; k += 2)   /* parcourt les 6 voisins */
      {
        y = voisin6(x, k, rs, n, N);
        if ((y != -1) && (M[y] != 0))
        {
          etiqcc[ncc] = M[y];
          ncc += 1;
        } /* if y */
      } /* for k */
      break;

    case 18:
      for (k = 0; k < 18; k++)      /* parcourt les 18 voisins */
      {
        y = voisin18(x, k, rs, n, N);
        if ((y != -1) && (M[y] != 0))
        {
          etiqcc[ncc] = M[y];
          ncc += 1;
        } /* if y */
      } /* for k */
      break;

    case 26:
      for (k = 0; k < 26; k++)      /* parcourt les 26 voisins */
      {
        y = voisin26(x, k, rs, n, N);
        if ((y != -1) && (M[y] != 0))
        {
          etiqcc[ncc] = M[y];
          ncc += 1;
        } /* if y */
      } /* for k */
      break;

    } /* switch (connex) */

    ncc = DiffAnc(CBT, ncc, etiqcc);

#ifdef PARANO
    if (ncc == 0)
    {
      printf("segment3d() : construction cbt : PAS DE cc DANS LE VOISINAGE !!!\n"); 
      
      return(0);  
    } else
#endif
    if (ncc == 1)
    {
      M[x] = etiqcc[0];
#ifdef TRACEETIQ
    if (trace) 
    {
      printf("%d: ETIQ point %d ancienne etiquette %d\n", 
              tracedate++, x, etiqcc[0]);
    }
#endif
      if (mesure == SURFACE) MU[M[x]] += 1;
      if (mesure == VOLUME)
      {
        if (SOURCE[x] + 1 > PROF[M[x]])
        {
          MU[M[x]] += SURF[M[x]] * (SOURCE[x] + 1 - PROF[M[x]]);
          PROF[M[x]] = SOURCE[x] + 1;
        }
        SURF[M[x]] += 1;
      } /* if (mesure == VOLUME) */
    } /* if (ncc == 1) */
    else
    {
      new = CreateCell(CBT, &nbcell, nbmaxcell);
      M[x] = new;
#ifdef TRACEETIQ
    if (trace) 
    {
      printf("%d: ETIQ point %d nouvellee etiquette %d\n", 
              tracedate++, x, new);
    }
#endif
      if (mesure == SURFACE) MU[M[x]] = 1;
      if (mesure == PROFONDEUR) prof = 0;
      if (mesure == VOLUME) SURF[M[x]] = 1;
      SetData(CBT, new, SOURCE[x]);     /* conceptuellement : SOURCE[x] + 1 */
      for (i = 0; i < ncc; i++)
      {
        SetFather(CBT, etiqcc[i], new);
        if (mesure == SURFACE) MU[M[x]] += MU[etiqcc[i]];
        if (mesure == PROFONDEUR)
        {
          k = MU[etiqcc[i]]+SOURCE[x]-Data(CBT,etiqcc[i]); /* conceptuellement : SOURCE[x] + 1 */
          if (k > prof) prof = k;
        }
        if (mesure == VOLUME)
        {
          if (SOURCE[x] + 1 > PROF[etiqcc[i]])
          {
            MU[etiqcc[i]] += SURF[etiqcc[i]] * (SOURCE[x] + 1 - PROF[etiqcc[i]]);
            PROF[etiqcc[i]] = SOURCE[x] + 1;
          }
          MU[M[x]] += MU[etiqcc[i]];
          SURF[M[x]] += SURF[etiqcc[i]];
        }
      } /* for i */
      if (mesure == PROFONDEUR) MU[M[x]] = prof;
      if (mesure == VOLUME) PROF[M[x]] = SOURCE[x] + 1;
    } /* else if (ncc == 1) */

    switch (connex)
    {
    case 6:
      for (k = 0; k <= 10; k += 2)       /* parcourt les 6 voisins */
      {                                  /* si un voisin n'est pas deja dans la FAH */
        y = voisin6(x, k, rs, n, N);     /* et n'est pas dans un minimum, on le met en FAH */
        if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
        {
          FahPush(FAH, y, SOURCE[y]);
          Set(y, EN_FAH);
#ifdef TRACEPUSH
          if (trace) printf("%d: empile point %d (%d %d,%d) au niveau %d\n", 
                             tracedate++, y, y/n, (y%n)%rs, (y%n)/rs, SOURCE[y]);
#endif
        } /* if y */
      } /* for k */
      break;

    case 18:
      for (k = 0; k < 18; k++)           /* parcourt les 18 voisins */
      {                                  /* si un voisin n'est pas deja dans la FAH */
        y = voisin18(x, k, rs, n, N);    /* et n'est pas dans un minimum, on le met en FAH */
        if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
        {
          FahPush(FAH, y, SOURCE[y]);
          Set(y, EN_FAH);
#ifdef TRACEPUSH
          if (trace) printf("%d: empile point %d (%d %d,%d) au niveau %d\n", 
                             tracedate++, y, y/n, (y%n)%rs, (y%n)/rs, SOURCE[y]);
#endif
        } /* if y */
      } /* for k */
      break;

    case 26:
      for (k = 0; k < 26; k++)           /* parcourt les 26 voisins */
      {                                  /* si un voisin n'est pas deja dans la FAH */
        y = voisin26(x, k, rs, n, N);    /* et n'est pas dans un minimum, on le met en FAH */
        if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
        {
          FahPush(FAH, y, SOURCE[y]);
          Set(y, EN_FAH);
#ifdef TRACEPUSH
          if (trace) printf("%d: empile point %d (%d %d,%d) au niveau %d\n", 
                             tracedate++, y, y/n, (y%n)%rs, (y%n)/rs, SOURCE[y]);
#endif
        } /* if y */
      } /* for k */
      break;

    } /* switch (connex) */

  } /* while (! FahVide(FAH)) */
  /* FIN INONDATION */

#ifdef TRACEIMAGE
  if (trace) 
  {
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) printf("\n");
      printf("%3d ", SOURCE[i]);
    }
    printf("\n");
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) printf("\n");
      printf("%3d ", M[i]);
    }
    printf("\n");
  }
#endif

#ifdef VERBOSE
  printf("FIN DE LA CONSTRUCTION DU CBT\n");
#endif

  T = Regularise(CBT, nminima, nbcell);

  for (x = 0; x < N; x++)
    if ((M[x] >= nminima) && (T[M[x] - nminima]))
      M[x] = T[M[x] - nminima]; /* nouvelle etiquette pour le point x */

  free(T);

#ifdef TRACECBT
  if (trace) 
  {
    PrintCbt(CBT, nbcell);
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) printf("\n");
      printf("%3d ", M[i]);
    }
    printf("\n");
    printf("MU = ");
    for (i = 1; i < nbcell; i++)
      printf("%3d ", MU[i]);
    printf("\n");
  }
#endif

#ifdef VERBOSE
  printf("FIN DE LA REGULARISATION DU CBT\n");
#endif

#ifdef PARANO
  for (i = 0; i < nbcell; i++)
    if (Label(CBT,i)) printf("ERROR : label = %d\n", Label(CBT, i));
#endif

  /* ================================================ */
  /* SEGMENTATION                               */
  /* ================================================ */

  /* pour la mesure de la profondeur, il faut rajouter la difference de niveau avec le pere */
  /* on remplace donc les MU dans l'arbre */
  for (x = 1; x < nbcell; x++) UnSetAll(x);
  if (mesure == PROFONDEUR)
  {
    FahFlush(FAH);
    for (i = 1; i < nminima+1; i++) FahPush(FAH, i, 0);
    while (! FahVide(FAH))
    {
      k = FahNiveau(FAH);
      x = FahPop(FAH);
      y = Father(CBT,x);
      if (y != NIL)
      { 
        MU[x] += ((Data(CBT,y) - 1) - Data(CBT,x));
        if (!IsSet(y, EN_FAH))
        { 
          Set(y, EN_FAH);
          FahPush(FAH, y, k + 1); 
        }
      }
      else MU[x] = NDG_MAX - NDG_MIN; /* prof. maximum pour la racine */
    } 
  }

/*  1ERE ETAPE : SELECTIONNE LE NB MAX DE CC PERTINENTES */

  /* init:  ranger au niveau 0 de la FAH les feuilles du CBT */
  FahFlush(FAH);
  for (i = 1; i < nminima+1; i++) FahPush(FAH, i, 0);

  while (! FahVide(FAH))
  {
    k = FahNiveau(FAH);
    x = FahPop(FAH);

    if (Label(CBT,x) == NONMARQUE)
    {
      if (MU[x] >= seuil)
      {
        Label(CBT,x) = PERTINENT;
#ifdef SEGMENT
        if (trace) printf("PERTINENT : %d\n", x);
#endif
        y = Father(CBT,x);
        while ((y != NIL) && (Label(CBT,y) != INVALIDE))
        {
#ifdef SEGMENT
          if (trace) printf("INVALIDE : %d\n", y);
#endif
          Label(CBT,y) = INVALIDE;
          y = Father(CBT,y);
        } /* while ((y != NIL) && (Label(CBT,y) != INVALIDE)) */
      } /* if (MU[x] >= seuil) */
      else
      {
        y = Father(CBT,x);
        if ((y != NIL) && (Label(CBT,y) != INVALIDE))
          FahPush(FAH, y, k + 1);
      } /* else if (MU[x] >= seuil) */
    } /* if (Label(CBT,x) != INVALIDE) */
  } /* while (! FahVide(FAH)) */
#ifdef VERBOSE
  printf("FIN DE LA SEGMENTATION (1ere ETAPE)\n");
#endif

  if (maximise)
  {
/*  2EME ETAPE : MAXIMISE AU SENS DE L'INCLUSION */

    I = InverseCBT(CBT, nminima, nbcell);
    FahFlush(FAH);
    for (x = 1; x < nbcell - 1; x++)   /* pas la racine */
      if (Label(CBT, x) == PERTINENT)
        FahPush(FAH, x, Data(CBT, x));

    while (! FahVide(FAH))
    {
      int32_t n, nbfilspert;

      x = FahPop(FAH);
      y = Father(CBT, x);
      k = FirstSon(CBT, y);
      if (y < nbcell - 1)              /* pas la racine */
        n = FirstSon(CBT, y + 1) - FirstSon(CBT, y);
      else
        n = nbcell - 2 - FirstSon(CBT, y);
      nbfilspert = 0;
      for (i = 0; i < n; i++)
        if ((Label(CBT, I[k+i]) == PERTINENT) || (Label(CBT, I[k+i]) == INVALIDE))
          nbfilspert++;
      if (nbfilspert == 1)
      {
        Label(CBT, x) = NONMARQUE;
        Label(CBT, y) = PERTINENT;
        FahPush(FAH, y, Data(CBT, y));
      } /* if (nbfilspert == 1) */
    } /* while (! FahVide(FAH)) */
#ifdef VERBOSE
    printf("FIN DE LA SEGMENTATION (2eme etape : maximisation)\n");
#endif
  } /* if (maximise) */

/* RECUPERATION DE LA SEGMENTATION */

  for (i = 0; i < N; i++)
  {
    x = M[i];

    while ((x != NIL) && (Label(CBT,x) == NONMARQUE)) 
      x = Father(CBT,x);
    if (Label(CBT,x) == PERTINENT)
      SOURCE[i] = 255;
    else
      SOURCE[i] = 0;
  } /* for x */  

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  if (maximise) free(I);
  free(CBT);
  free(MU);
  if (mesure == VOLUME)
  {
    free(SURF);
    free(PROF);
  }
  IndicsTermine();
  FahTermine(FAH);
  freeimage(lab);
  return(1);
}

