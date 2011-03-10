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
/* operateur fondamental de segmentation numerique par seuillage */
/* utilise une File d'Attente Hierarchique */
/* utilise un arbre des bassins versants (captation basin tree, CBT) */
/* Michel Couprie - decembre 1996 */
/* version 1.1 du 25/08/97 */
/* (anc. version 1.0 du 15/12/96) */
/*
Principe: l'idee est d'appliquer l'operateur fondamental de segmentation binaire
  sur un "gradient" de l'image originale. 

Pour eviter tous les problemes lies aux gradients discrets habituels, on considerera
  le gradient comme non nul seulement aux frontieres de pixels de l'image originale
  (ou aux points de la maille derivee, cf. []) :

  image O:                                         image G:

  ------------------------------------             ------------------------------------
  |      |      |      |      |      |             |      |      |      |      |      |
  |  1   |  1   |  12  |  8   |  16  |             |  0   0  0  11  0   4  0   8  0   |
  |      |      |      |      |      |             |      |      |      |      |      |
  ------------------------------------             ---1------0------0------1------12---
  |      |      |      |      |      |             |      |      |      |      |      |
  |  0   |  1   |  12  |  7   |  4   |             |  0   1  0  11  0   5  0   3  0   |
  |      |      |      |      |      |             |      |      |      |      |      |
  ------------------------------------             ---0------3------8------3------3----
  |      |      |      |      |      |             |      |      |      |      |      |
  |  0   |  4   |  4   |  4   |  7   |             |  0   4  0   0  0   0  0   3  0   |
  |      |      |      |      |      |             |      |      |      |      |      |
  ------------------------------------             ------------------------------------

Les minima du gradient sont lies aux plateaux de l'image originale. 

Contrairement a la situation de la segmentation binaire, les plateaux sont "en contact"
  les uns avec les autres, separes seulement par la ligne de gradient d'epaisseur nulle.

Il faut etiqueter les plateaux de O et construire le CBT du gradient:

  image E:                                     CBT:

  ------------------------------------    1   2   3   4   5   6   7   8   9        niveau
  |      |      |      |      |      |    10              10                          1
  |  1   |  1   |  2   |  3   |  4   |            11          11                      1
  |      |      |      |      |      |    12                  12  12  12  12          3
  ------------------------------------        13              13                      5       
  |      |      |      |      |      |        14      14                              8
  |  5   |  1   |  2   |  6   |  7   |
  |      |      |      |      |      |
  ------------------------------------
  |      |      |      |      |      |
  |  5   |  8   |  8   |  8   |  9   |
  |      |      |      |      |      |
  ------------------------------------

On selectionnera ensuite les "coupes consistantes" (representant les plateaux devant
  "fusionner") par la methode desormais classique.

Il ne restera plus qu'a decider du sort des plateaux fusionnant: quelle valeur leur
  donner ? idees de variantes: le min, le max, la valeur du plateau le plus etendu,
  la moyenne ponderee par la surface.
*/

#define PARANO                 /* even paranoid people have ennemies */
#define VERBOSE

/*
#define TRACEPOP
#define TRACEPUSH
#define TRACEBAISSE
#define TRACEETIQ
#define TRACEIMAGE
*/
#define SEGMENT
#define TRACECBT


/*
   Methode : 

===============================================
1ere etape : CONSTRUCTION DU CBT DU GRADIENT
===============================================
  entree : une image F, N = |Dom(F)|
  sortie : le CBT, l'application M: Dom(F) -> CBT

  M <- etiquettes des minima du gradient de F (ie - des plateaux de F)
  nminima <- nombre de minima du gradient de F
  init(CBT, nminima)

  pour tout point x de Dom(F)
    y = VoisinEst(x)
    si y existe ET M[x] != M[y] alors
      g = mcabs(F[y] - F[x])
      FahPush(FAH, x, g)
    finsi
    y = VoisinSud(x)
    si y existe ET M[x] != M[y] alors
      g = mcabs(F[y] - F[x])
      FahPush(FAH, x + N, g)   ;; il faudra retirer N pour recuperer la coord. du pixel 
    finsi
  finpour


  tant que FAH non vide
    g = FahNiveau(FAH)
    x = FahPop(FAH)
    etiqcc[0] = M[x]
    si x < N et VoisinEst(x) existe alors etiqcc[1] = M[VoisinEst(x)] finsi
    si x >= N et VoisinSud(x) existe alors etiqcc[1] = M[VoisinSud(x)] finsi

    diffanc = liste des ancetres differents des etiquettes de etiqcc dans le CBT
    si |diffanc| > 1
      new = CreateCell(CBT)
      SetData(CBT, new, g)
      pour tout a dans diffanc 
        SetFather(CBT, a, new)
      finpour
    fin si
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
      marque les ancetres de x "PERTINENT"
      marque x "PERTINENT"
    sinon
      si pere(x) existe et non PERTINENT alors FAHPUSH(pere(x), k+1) finsi
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
#include <mcutil.h>
#include <llabelplateaux.h>
#include <lsegmentnum.h>

#define EN_FAH 0
#define MINIMUM 1
#define CC_EN_FAH 2

#define NONMARQUE 0
#define INVALIDE  1
#define PERTINENT 2

/* ==================================== */
int32_t lsegmentnum(
        struct xvimage *image,
        int32_t mesure,
        int32_t seuil,
        int32_t trace,
        struct xvimage *result)
/* ==================================== */
{
  register int32_t i;                       /* index muet */
  register int32_t x;                       /* index muet de pixel */
  register int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  int32_t *M = SLONGDATA(result);
  int32_t *T;                      /* table de correspondance pour regularisation */
  int32_t nminima;                 /* nombre de minima differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  cbtcell * CBT;               /* arbre des bassins versants (Catchment Basin Tree) */
  int32_t nbcell;
  int32_t nbmaxcell;
  int32_t *MU;                     /* pour la mesure des regions */
  int32_t etiqcc[4];
  int32_t ncc;
  int32_t newcell;
  int32_t nbcomp = 0;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lsegmentnum: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  
  FAH = CreeFahVide(2*N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "lsegmentnum() : CreeFah failed\n");
      return(0);
  }

  if (!llabelplateaux(image, 4, result, &nminima))
  {   
    fprintf(stderr, "lsegmentnum: llabelplateaux failed\n");
    return 0;
  }

  nbcell = nminima + 1;
  nbmaxcell = nbcell * 2;
  CBT = (cbtcell *)calloc(1,nbmaxcell * sizeof(cbtcell));
  if (CBT == NULL)
  {   fprintf(stderr, "lsegmentnum() : malloc failed for CBT\n");
      return(0);
  }

  MU = (int32_t *)calloc(nbmaxcell, sizeof(int32_t));   /* init a 0 */
  if (MU == NULL)
  {   fprintf(stderr, "lsegmentnum() : malloc failed for MU\n");
      return(0);
  }
  /* calcul des tailles des minima (histogramme des labels) */
  for (x = 0; x < N; x++) MU[M[x]] += 1;

  /* ================================================ */
  /*                CONSTRUCTION DU CBT               */
  /* ================================================ */

  InitCbt(CBT, nbcell);
  
  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
                                     /* NECESSAIRE pour eviter la creation prematuree */
                                     /* de la file d'urgence */ 
  for (x = 0; x < N; x++)
  {
    y = voisin(x, EST, rs, N);
    if ((y != -1) && (M[x] != M[y]))
    {
      FahPush(FAH, x, mcabs((int32_t)SOURCE[y] - (int32_t)SOURCE[x]));
#ifdef TRACEPUSH
          if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
                             tracedate++, x, x%rs, x/rs, mcabs((int32_t)SOURCE[y] - (int32_t)SOURCE[x]));
#endif
    } /* if y */
    y = voisin(x, SUD, rs, N);
    if ((y != -1) && (M[x] != M[y]))
    {
      FahPush(FAH, N + x, mcabs((int32_t)SOURCE[y] - (int32_t)SOURCE[x]));
#ifdef TRACEPUSH
          if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
                             tracedate++, x, x%rs, x/rs, mcabs((int32_t)SOURCE[y] - (int32_t)SOURCE[x]));
#endif
    } /* if y */
  } /* for x */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     printf("segmentnum() : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n");
     return(0);
  }
#endif

  /* INONDATION */
  while (! FahVide(FAH))
  {
    ncc = 0;
    k = FahNiveau(FAH);
    x = FahPop(FAH);

#ifdef TRACEPOP
    if (trace) 
    {
      printf("%d: POP point %d (%d,%d) ; niveau %d\n", 
              tracedate++, x, (x%N)%rs, (x%N)/rs, k);
    }
#endif

    etiqcc[ncc] = M[x%N];
    ncc += 1;
    
    if (x < N)
    {
      y = voisin(x, EST, rs, N);
      if (y != 1) 
      {
        etiqcc[ncc] = M[y];
        ncc += 1;
      }
    }
    else /* x >= N */
    {
      y = voisin(x - N, SUD, rs, N);
      if (y != 1) 
      {
        etiqcc[ncc] = M[y];
        ncc += 1;
      }
    } /* else if (x < N) */

    ncc = DiffAnc(CBT, ncc, etiqcc);

    if (ncc > 1)
    {
      newcell = CreateCell(CBT, &nbcell, nbmaxcell);
      SetData(CBT, newcell, k);                  /* conceptuellement : k + 1 */
      for (i = 0; i < ncc; i++)
      {
        MU[newcell] += MU[etiqcc[i]];
        SetFather(CBT, etiqcc[i], newcell);
      } /* for i */
    } /* if (ncc > 1) */

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

  T = Regularise(CBT, nminima+1, nbcell);
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
      switch (mesure)
      {
        case SURFACE:    newcell = MU[x]; break;
        case PROFONDEUR: newcell = (int32_t)Data(CBT, x); break;
        case VOLUME:     newcell = MU[x] * (int32_t)Data(CBT, x); break;
      } /* switch (mesure) */
      if (newcell >= seuil)
      {
        nbcomp++;
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
  printf("FIN DE LA SEGMENTATION - 1ERE ETAPE\n");
#endif
 
/* RECUPERATION DE LA SEGMENTATION */

  for (x = 1; x < nbcell; x++) Aux(CBT,x) = 0; /* pour la renumerotation */
  nbcell = 1;                                  /* pour la renumerotation */

  for (i = 0; i < N; i++)
  {
    x = M[i];
    
    while ((x != NIL) && (Label(CBT,x) == NONMARQUE)) 
      x = Father(CBT,x);
    if (Label(CBT,x) == PERTINENT)
    {
#ifdef SEGMENT
      if (trace) printf("RECUP PERTINENT : %d\n", x);
#endif
      if (Aux(CBT, x) == 0) Aux(CBT, x) = nbcell++;
      M[i] = Aux(CBT, x);
    }
    else
      M[i] = 0;

  } /* for i */  

#ifdef VERBOSE
  printf("FIN DE LA SEGMENTATION - RECUPERATION\n");
#endif
 
  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  free(CBT);
  free(MU);
  FahTermine(FAH);
  return(1);
}
