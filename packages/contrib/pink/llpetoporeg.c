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
/* operateur de calcul de la ligne de partage des eaux topologique */
/* utilise une File d'Attente Hierarchique */
/* utilise un arbre des bassins versants (captation basin tree, CBT) */
/* methode : construit le CBT puis detecte les points CB-simples 
   (dans un ordre de ndg croissant et avec propagation autour des pts modifies) 
   et les abaisse. la propagation des modifications se fait 
   grace a la FAH.
*/
/* Michel Couprie - novembre 1996 */
/* version 5.0 du 27/12/96 */
/* changements depuis la derniere version: 
   introduction d'une etape de regularisation basee sur un seuil.
*/

#define PARANO                 /* even paranoid people have ennemies */
#define VERBOSE

/*
#define TRACEPOP
#define TRACEPUSH
#define TRACEBAISSE
*/
#define TRACEETIQ
#define TRACECBT
#define TRACEIMAGE
#define TRACEREGUL

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
      pour tout y dans gamma4(x)
        si y n'appartient pas a un minimum et pas deja dans FAH
          FahPush(FAH, y, SOURCE[y]);
        finsi
      finpour
    finsi
  finpour

  tant que FAH non vide
    x = FahPop(FAH);
    etiqcc = liste des M[y], y dans gamma4(x)
    diffanc = liste des ancetres differents des etiquettes de etiqcc dans le CBT
    si |diffanc| == 1
      M[x] = first(diffanc)
    sinon
      newcell = CreateCell(CBT)
      M[x] = newcell
      SetData(CBT, newcell, F[x] + 1)
      pour tout a dans diffanc 
        SetFather(CBT, a, newcell)
      finpour
    fin si
    pour tout y dans gamma4(x) pas deja dans FAH
      si M[y] == 0
        FahPush(FAH, y, SOURCE[y]);
      finsi
    finpour
  fintantque

  regularise(CBT)  ;; ce n'est pas LA regularisation au sens GB, mais seulement
                   ;; la regularisation du CBT.

===============================================
2eme etape: ELIMITION DES MINIMA NON PERTINENTS (REGULARISATION)
===============================================

  utilise deux labels : INVALIDE et DISPARU
  utilise une table de correspondance T : vieille etiquette -> nouvelle etiquette

  InvCBT = inverse(CBT)

  pour tout x noeud du CBT T[x] = 0 finpour

  // marque INVALIDE les feuilles invalides et les empile.
  pour tout x feuille de CBT 
    si Data(Father(x)) - Data(x) < seuil alors 
      SetLabel(x, INVALIDE)
      FahPush(x, 0)
    finsi
  finpour

  // remonte dans l'arbre : un noeud dont tous les fils sont INVALIDE
  // est marque INVALIDE s'il ne satisfait pas le critere
  // et ses fils sont supprimes (qu'il satisfasse ou non le critere)
  // un noeud ayant au moins un frere valide est supprime.
  // l'etiquette correspondant a un noeud supprime est remplacee
  // par celle correspondant au pere du noeud supprime.
  tant que FAH non vide
    i = FahNiveau(FAH)
    x = FahPop(FAH)
    si Label(x) == INVALIDE alors
      k = Father(x)
      toutes_invalides = 1
      pour tout y fils de k 
        si (Label(y) != INVALIDE et Label(y) != DISPARU) alors 
          toutes_invalides = 0 
        finsi
      finpour
      si toutes_invalides alors
        min = Data(x)
        pour tout y fils de k 
          si Data(y) < min alors min = Data(y) finsi
          SetLabel(y, DISPARU)
          T[y] = k
        finpour
        SetData(k, min)
        si Father(k) existe et Data(Father(k)) - Data(k) < seuil alors 
          SetLabel(k, INVALIDE) 
          FahPush(k, i+1)
        finsi
      sinon
        SetLabel(x, DISPARU)
        T[x] = Father(x)
      finsi
    finsi
  fintantque  
  
  pour tout x noeud du CBT    ;; "regularise" la table de correspondance
    k = x
    tantque T[k] != 0 faire
      k = T[k]
    fintantque
    T[x] = k
  finpour

  pour tout point x de Dom(F)
    si T[M[x]] != 0 alors M[x] = T[M[x]] finsi
  finpour

===============================================
3eme etape: NOYAU HOMOTOPIQUE
===============================================

  pour tout point x de Dom(F)
    FahPush(FAH, x, SOURCE[x])
  finpour

  tant que FAH non vide
    x = FahPop(FAH);
    etiqcc = liste des M[y], y dans gamma4(x), Data(CBT,M(y)) <= F(x)
    newcell = LowComAnc(CBT, etiqcc, SOURCE[x])
    si newcell != NIL et Data(CBT, newcell) - 1 < F[x]  // point abaissable
      F[x] = Data(CBT, newcell) - 1
      M[x] = newcell
      pour tout y dans gamma4(x) pas deja dans FAH
        FahPush(FAH, y, SOURCE[y])
      finpour
      FahPush(FAH, y, SOURCE[y])
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
#include <llpetoporeg.h>

#define EN_FAH   0
#define MINIMUM  1

#define INVALIDE 1
#define DISPARU  2

/* ==================================== */
int32_t llpetoporeg(
        struct xvimage *image,
        int32_t seuil,
        int32_t trace)
/* ==================================== */
{
  register int32_t i;                       /* index muet */
  register int32_t x;                       /* index muet de pixel */
  register int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  int32_t j, n;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *SOURCE = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *lab;
  int32_t *M;            /* l'image d'etiquettes de composantes connexes */
  int32_t *T;                      /* table de correspondance pour regularisation */
  int32_t *I;                      /* pour l'inversion du CBT */
  int32_t nminima;                 /* nombre de minima differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  cbtcell * CBT;               /* arbre des bassins versants (Catchment Basin Tree) */
  int32_t nbcell;
  int32_t nbmaxcell;
  int32_t nombre_abaissements = 0;
  int32_t nombre_examens = 0;
  int32_t etiqcc[4];
  int32_t ncc;
  int32_t newcell;
  int32_t tracedate = 0;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "llpetoporeg: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  
  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "llpetoporeg() : CreeFah failed\n");
      return(0);
  }

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "llpetoporeg: allocimage failed\n");
    return 0;
  }
  M = SLONGDATA(lab);

  if (!llabelextrema(image, 4, LABMIN, lab, &nminima))
  {   
    fprintf(stderr, "llpetoporeg: llabelextrema failed\n");
    return 0;
  }

  nbcell = nminima;
  nbmaxcell = nminima * 2;
  CBT = (cbtcell *)calloc(1,nbmaxcell * sizeof(cbtcell));
  if (CBT == NULL)
  {   fprintf(stderr, "llpetoporeg() : malloc failed for CBT\n");
      return(0);
  }

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
      SetData(CBT, M[x], SOURCE[x]);  /* conceptuellement : SOURCE[x] + 1 */
      for (k = 0; k < 8; k += 2)     /* parcourt les voisins en 4-connexite */
      {                              /* si un voisin n'est pas deja dans la FAH */
        y = voisin(x, k, rs, N);     /* et n'est pas dans un minimum, on le met en FAH */
        if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
	{
          FahPush(FAH, y, SOURCE[y]);
          Set(y, EN_FAH);
#ifdef TRACEPUSH
          if (trace) fprintf(stderr,"%d: empile point %d (%d,%d) au niveau %d\n", 
                             tracedate++, y, y%rs, y/rs, SOURCE[y]);
#endif
        } /* if y */
      } /* for k */
    } /* if (M[x] != 0) */
  } /* for x */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"lpetoporeg() : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n");
     return(0);
  }
#endif

  /* INONDATION */
  while (! FahVide(FAH))
  {
    ncc = 0;
    x = FahPop(FAH);
    UnSetAll(x);

#ifdef TRACEPOP
    if (trace) 
    {
      fprintf(stderr,"%d: POP point %d (%d,%d) ; niveau %d\n", 
              tracedate++, x, x%rs, x/rs, SOURCE[x]);
    }
#endif

    for (k = 0; k < 8; k += 2)     /* parcourt les voisins en 4-connexite */
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (M[y] != 0))
      {
        etiqcc[ncc] = M[y];
        ncc += 1;
      } /* if y */
    } /* for k */
    ncc = DiffAnc(CBT, ncc, etiqcc);

#ifdef PARANO
    if (ncc == 0)
    {
      fprintf(stderr,"lpetoporeg() : construction cbt : PAS DE cc DANS LE VOISINAGE !!!\n"); 
      
      return(0);  
    } else
#endif
    if (ncc == 1)
      M[x] = etiqcc[0];
    else
    {
      newcell = CreateCell(CBT, &nbcell, nbmaxcell);
      M[x] = newcell;
      SetData(CBT, newcell, SOURCE[x]);    /* conceptuellement : SOURCE[x] + 1 */
      for (i = 0; i < ncc; i++)
        SetFather(CBT, etiqcc[i], newcell);
    }

    for (k = 0; k < 8; k += 2)     /* parcourt les voisins en 4-connexite */
    {                              /* si un voisin n'est pas deja dans la FAH */
      y = voisin(x, k, rs, N);     /* et n'est pas dans un minimum, on le met en FAH */
      if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
      {
        FahPush(FAH, y, SOURCE[y]);
        Set(y, EN_FAH);
#ifdef TRACEPUSH
        if (trace) fprintf(stderr,"%d: empile point %d (%d,%d) au niveau %d\n", 
                           tracedate++, y, y%rs, y/rs, SOURCE[y]);
#endif
      } /* if y */
    } /* for k */

  } /* while (! FahVide(FAH)) */
  /* FIN INONDATION */

#ifdef TRACEIMAGE
  if (trace) 
  {
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(stderr,"\n");
      fprintf(stderr,"%3d ", SOURCE[i]);
    }
    fprintf(stderr,"\n");
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(stderr,"\n");
      fprintf(stderr,"%3d ", M[i]);
    }
    fprintf(stderr,"\n");
  }
#endif

#ifdef VERBOSE
  fprintf(stderr,"FIN DE LA CONSTRUCTION DU CBT\n");
#endif

  T = Regularise(CBT, nminima, nbcell);
  if (T == NULL)
  {   fprintf(stderr, "llpetoporeg() : Regularise failed\n");
      return(0);
  }

  for (x = 0; x < N; x++)
  {
    if ((M[x] >= nminima) && (T[M[x] - nminima]))
      M[x] = T[M[x] - nminima];
  } /* for x */

  free(T);

#ifdef TRACECBT
  if (trace) 
  {
    PrintCbt(CBT, nbcell);
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(stderr,"\n");
      fprintf(stderr,"%3d ", M[i]);
    }
    fprintf(stderr,"\n");
  }
#endif

#ifdef VERBOSE
  fprintf(stderr,"FIN DE LA REGULARISATION DU CBT\n");
#endif

/* 
===============================================
2eme etape: ELIMITION DES MINIMA NON PERTINENTS (REGULARISATION)
=============================================== 
*/

  T = (int32_t *) calloc(nbcell, sizeof(int32_t));
  if (T == NULL)
  {   fprintf(stderr, "llpetoporeg() : calloc failed for T\n");
      return(0);
  }

  I = InverseCBT(CBT, nminima, nbcell);
  if (I == NULL)
  {   fprintf(stderr, "llpetoporeg() : InverseCBT failed\n");
      return(0);
  }

  /* on va etre gene par les cellules "mortes" issues de la "regularisation" du CBT. */
  /* il faut les nettoyer.  */
  for (x = nminima; x < nbcell - 1; x++) /* pas les feuilles, pas la racine */
  {
    if ((FirstSon(CBT, x + 1) - FirstSon(CBT, x)) == 0)
      Label(CBT, x) = DISPARU;
  }

  FahFlush(FAH);

  for (x = 1; x < nminima; x++)
  {
    if (Data(CBT, Father(CBT, x)) - Data(CBT, x) < seuil)
    { 
#ifdef TRACEREGUL
          if (trace) fprintf(stderr,"%d: empile etiquette invalide %d au niveau 0\n", 
                             tracedate++, x);
#endif
      Label(CBT, x) = INVALIDE;
      FahPush(FAH, x, 0);
    }
  }

  while (! FahVide(FAH))
  {
    int32_t toutes_invalides;
    uint8_t min;

    i = FahNiveau(FAH);
    x = FahPop(FAH);
#ifdef TRACEREGUL
    if (trace) 
    {
      fprintf(stderr,"%d: POP etiquette %d ; niveau %d\n", 
              tracedate++, x, i);
    }
#endif
    if (Label(CBT, x) == INVALIDE)
    {
      k = Father(CBT, x);
      y = FirstSon(CBT, k);
      n = FirstSon(CBT, k + 1) - FirstSon(CBT, k);
      toutes_invalides = 1;
      for (j = 0; j < n; j++)
      {
        if ((Label(CBT, I[y+j]) != INVALIDE) && (Label(CBT, I[y+j]) != DISPARU))
          toutes_invalides = 0;
      } /* for (j... */
      if (toutes_invalides)
      {
        min = Data(CBT, x);
        for (j = 0; j < n; j++)
        {
          if (Data(CBT, I[y+j]) < min) min = Data(CBT, I[y+j]);
          Label(CBT, I[y+j]) = DISPARU;
          T[I[y+j]] = k;
#ifdef TRACEREGUL
          if (trace) fprintf(stderr,"%d: etiquette disparait (1) %d, T[%d] = %d\n", 
                             tracedate++, I[y+j], I[y+j], k);
#endif
        } /* for (j = 0; j < n; j++) */
        SetData(CBT, k, min);
        if ((Father(CBT, k) != NIL) && (Data(CBT, Father(CBT, k)) - Data(CBT, k) < seuil))
        {
#ifdef TRACEREGUL
          if (trace) fprintf(stderr,"%d: empile etiquette invalide %d au niveau %d\n", 
                             tracedate++, k, i+1);
#endif
          Label(CBT, k) = INVALIDE; 
          FahPush(FAH, k, i+1);
        } /* if ((Father(CBT, k) != NIL) ... */
      } /* if (toutes_invalides) */
      else
      {
        Label(CBT, x) = DISPARU;
        T[x] = Father(CBT, x);
#ifdef TRACEREGUL
          if (trace) fprintf(stderr,"%d: etiquette disparait (2) %d, T[%d] = %d\n", 
                             tracedate++, x, x, Father(CBT, x));
#endif
      } /* else if (toutes_invalides) */
    } /* if (Label(CBT, x) == INVALIDE) */
  } /* while (! FahVide(FAH)) */
  
  for (x = 1; x < nbcell; x++) /* "regularise" la table de correspondance */
  {
    if (T[x] != 0)
    {
      k = x;
      while (T[k] != 0) k = T[k];
      T[x] = k;
    }
  }

  for (x = 0; x < N; x++)      /* actualise les etiquettes image */
    if (T[M[x]] != 0) M[x] = T[M[x]];


#ifdef TRACEIMAGE
  if (trace) 
  {
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(stderr,"\n");
      fprintf(stderr,"%3d ", M[i]);
    }
    fprintf(stderr,"\n");
  }
#endif

#ifdef TRACECBT
  if (trace) 
  {
    PrintCbt(CBT, nbcell);
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(stderr,"\n");
      fprintf(stderr,"%3d ", M[i]);
    }
    fprintf(stderr,"\n");
  }
#endif

#ifdef VERBOSE
  fprintf(stderr,"FIN DE L'ELIMINATION DES MINIMA NON PERTINENTS\n");
#endif

  /* ================================================ */
  /* NOYAU HOMOTOPIQUE                                */
  /* ================================================ */

  FahFlush(FAH);

  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
                                     /* NECESSAIRE pour eviter la creation prematuree */
                                     /* de la file d'urgence */ 
  for (x = 0; x < N; x++)
  {
    SOURCE[x] = Data(CBT, M[x]);     /* remet les pixels au niveau de leur feuille du CBT */
    FahPush(FAH, x, SOURCE[x]);
    Set(x, EN_FAH);
#ifdef TRACEPUSH
    if (trace) fprintf(stderr,"%d: empile point %d (%d,%d) au niveau %d\n", 
                       tracedate++, x, x%rs, x/rs, SOURCE[x]);
#endif
  } /* for x */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"lpetoporeg() : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n");
     return(0);
  }
#endif


  /* PROPAGATION AVEC HEURISTIQUE D'INONDATION */
  while (! FahVide(FAH))
  {
    ncc = 0;
    x = FahPop(FAH);
    UnSetAll(x);
    nombre_examens += 1;

#ifdef TRACEPOP
    if (trace) 
    {
      fprintf(stderr,"%d: POP point %d (%d,%d) ; niveau %d\n", 
              tracedate++, x, x%rs, x/rs, SOURCE[x]);
    }
#endif

    for (k = 0; k < 8; k += 2)     /* parcourt les voisins en 4-connexite */
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (Data(CBT, M[y]) < SOURCE[x]))   /* conceptuellement : <= */
      {
        etiqcc[ncc] = M[y];
        ncc += 1;
      } /* if y */
    } /* for k */

    newcell = LowComAnc(CBT, ncc, etiqcc, SOURCE[x]);

    if ((newcell != NIL) && (Data(CBT, newcell) < SOURCE[x])) /* le point est CB-simple */
    {      /* conceptuellement : Data(CBT, newcell) - 1 */
#ifdef TRACEBAISSE
      if (trace) fprintf(stderr,"%d: point %d (%d,%d) ; niveau =  %d, etiq = %d\n", 
                        tracedate++, x, x%rs, x/rs, SOURCE[x], M[x]);
#endif
      nombre_abaissements += 1;
      SOURCE[x] = Data(CBT, newcell);   /* conceptuellement : Data(CBT, newcell) - 1 */
      M[x] = newcell;
#ifdef TRACEBAISSE
      if (trace) fprintf(stderr,"baisse au niveau  %d, etiq <- %d\n", 
                         SOURCE[x], M[x]);
#endif

      /* propagation aux voisins */

      for (k = 0; k < 8; k += 2)     /* parcourt les voisins en 4-connexite */
      {                              /* si un voisin n'est pas deja dans la FAH */
        y = voisin(x, k, rs, N);     
        if (y != -1)
        {
          if (! IsSet(y, EN_FAH))
          {
            FahPush(FAH, y, SOURCE[y]);
            Set(y, EN_FAH);
#ifdef TRACEPUSH
            if (trace) fprintf(stderr,"%d: empile point %d (%d,%d) au niveau %d\n", 
                               tracedate++, y, y%rs, y/rs, SOURCE[y]);
#endif
          } /* if (! IsSet(y, EN_FAH)) */
        } /* if (y != -1) */
      } /* for k */

      FahPush(FAH, x, SOURCE[x]);
      Set(x, EN_FAH);
#ifdef TRACEPUSH
      if (trace) fprintf(stderr,"%d: empile point %d (%d,%d) au niveau %d\n", 
                        tracedate++, x, x%rs, x/rs, SOURCE[x]);
#endif

    } /* if ((newcell != NIL) && (Data(CBT, newcell) < SOURCE[x]))  le point est CB-simple */
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

#ifdef VERBOSE
  fprintf(stderr,"NOMBRE D'EXAMENS: %d\n", nombre_examens);
  fprintf(stderr,"NOMBRE D'ABAISSEMENTS: %d\n", nombre_abaissements);
#endif
  
  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  free(CBT);
  free(T);
  free(I);
  IndicsTermine();
  FahTermine(FAH);
  freeimage(lab);
  return(1);
}

