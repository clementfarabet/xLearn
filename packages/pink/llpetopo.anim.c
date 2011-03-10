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
/* version 6.0 du 9/6/97 */
/* changements depuis la derniere version: 
   selection des composantes pertinentes a partir d'une image de marqueurs ponctuels
*/

#define PARANO                 /* even paranoid people have ennemies */
#define VERBOSE

/*
#define TRACEPOP1
#define TRACEPUSH1
#define TRACEETIQ
#define TRACECBT
#define TRACEIMAGE
#define TRACEREGUL
#define TRACEPOP
#define TRACEPUSH
#define TRACEBAISSE
#define ANIMATE
*/

#ifdef ANIMATE
#define XMIN          0
#define XMAX          20
#define YMIN          290
#define YMAX          310
#define ATTR_CANDIDAT A_BOLD
#define ATTR_ABAISSE  A_REVERSE
#endif

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
      new = CreateCell(CBT)
      M[x] = new
      SetData(CBT, new, F[x] + 1)
      pour tout a dans diffanc 
        SetFather(CBT, a, new)
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

  entree: une image binaire B de marqueurs

  utilise deux labels : INVALIDE et VALIDE
  utilise une table de correspondance T : vieille etiquette -> nouvelle etiquette

  InvCBT = inverse(CBT)

  pour tout x noeud du CBT T[x] = 0 finpour

  // marque VALIDE les feuilles marquees
  pour tout pixel i
    si B[i] alors SetLabel(M[i], VALIDE)
  finpour

  // marque INVALIDE les feuilles non marquees et les empile.
  pour tout x feuille de CBT 
    si Label(x) != VALIDE alors 
      SetLabel(x, INVALIDE)
      FahPush(x, 0)
    finsi
  finpour

  // l'etiquette correspondant a un noeud INVALIDE est remplacee
  // par celle de son pere
  // remonte dans l'arbre : un noeud dont tous les fils sont INVALIDE
  // est marque INVALIDE
  tant que FAH non vide
    i = FahNiveau(FAH)
    x = FahPop(FAH)
    si x non feuille
      toutes_invalides = 1
      pour tout y fils de x 
        si (Label(y) != INVALIDE) alors 
          toutes_invalides = 0 
        finsi
      finpour
      si toutes_invalides alors
        SetLabel(x, INVALIDE)
      finsi
    finsi
    si Label(x) == INVALIDE alors
      k = Father(x)
      si k existe alors
        T[x] = k
        FahPush(k, i+1)
      sinon erreur 
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
    new = LowComAnc(CBT, etiqcc, SOURCE[x])
    si new != NIL et Data(CBT, new) - 1 < F[x]  // point abaissable
      F[x] = Data(CBT, new) - 1
      M[x] = new
      pour tout y dans gamma4(x) pas deja dans FAH
        FahPush(FAH, y, SOURCE[y])
      finpour
      FahPush(FAH, y, SOURCE[y])
    finsi
  fintantque
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
//#include <curses.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfah.h>
#include <mcindic.h>
#include <mccbt.h>
#include <llabelextrema.h>
#include <llpetopo.h>

#define EN_FAH   0
#define MINIMUM  1

#define INVALIDE 1
#define VALIDE   2
#define DISPARU  3

/* ==================================== */
int32_t llpetopo_anim_llpetopo(
        struct xvimage *image,
        struct xvimage *marqueurs,
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
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  struct xvimage *lab;
  uint32_t *M;            /* l'image d'etiquettes de composantes connexes */
  int32_t *T;                      /* table de correspondance pour regularisation */
  int32_t *I;                      /* pour l'inversion du CBT */
  int32_t nminima;                 /* nombre de minima differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  cbtcell * CBT;               /* arbre des bassins versants (Catchment Basin Tree) */
  int32_t nbcell;
  int32_t nbmaxcell;
  char *name1, *name2, *name;
  int32_t nombre_abaissements = 0;
  int32_t nombre_examens = 0;
  int32_t etiqcc[4];
  int32_t ncc;
  int32_t new;
  int32_t tracedate = 0;
#ifdef ANIMATE
  int32_t oldx = -1;
  uint8_t oldvalx;
  char buf[32];
  int32_t ret;
  WINDOW * win;
#endif  

  if (depth(image) != 1) 
  {
    fprintf(stderr, "llpetopo_anim_llpetopo: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  
  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "llpetopo_anim_llpetopo: incompatible image sizes\n");
    return 0;
  }

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "llpetopo_anim_llpetopo() : CreeFah failed\n");
      return(0);
  }

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "llpetopo_anim_llpetopo: allocimage failed\n");
    return 0;
  }
  M = SLONGDATA(lab);

  if (!llabelextrema(image, 4, LABMIN, lab, &nminima))
  {   
    fprintf(stderr, "llpetopo_anim_llpetopo: llabelextrema failed\n");
    return 0;
  }

  nbcell = nminima;
  nbmaxcell = nminima * 2;
  CBT = (cbtcell *)calloc(1,nbmaxcell * sizeof(cbtcell));
  if (CBT == NULL)
  {   fprintf(stderr, "llpetopo_anim_llpetopo() : malloc failed for CBT\n");
      return(0);
  }

#ifdef ANIMATE
  win = initscr();
  (void)keypad(win, 1);
  (void)cbreak();
  clear();
#endif

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
#ifdef TRACEPUSH1
          if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
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
     fprintf(stderr,"lpetopo() : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n");
     return(0);
  }
#endif

  /* INONDATION */
  while (! FahVide(FAH))
  {
    ncc = 0;
    x = FahPop(FAH);
    UnSetAll(x);

#ifdef TRACEPOP1
    if (trace) 
    {
      printf("%d: POP point %d (%d,%d) ; niveau %d\n", 
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
      fprintf(stderr,"lpetopo() : construction cbt : PAS DE cc DANS LE VOISINAGE !!!\n"); 
      
      return(0);  
    } else
#endif
    if (ncc == 1)
      M[x] = etiqcc[0];
    else
    {
      new = CreateCell(CBT, &nbcell, nbmaxcell);
      M[x] = new;
      SetData(CBT, new, SOURCE[x]);    /* conceptuellement : SOURCE[x] + 1 */
      for (i = 0; i < ncc; i++)
        SetFather(CBT, etiqcc[i], new);
    }

    for (k = 0; k < 8; k += 2)     /* parcourt les voisins en 4-connexite */
    {                              /* si un voisin n'est pas deja dans la FAH */
      y = voisin(x, k, rs, N);     /* et n'est pas dans un minimum, on le met en FAH */
      if ((y != -1) && (! IsSet(y, EN_FAH)) && (M[y] == 0))
      {
        FahPush(FAH, y, SOURCE[y]);
        Set(y, EN_FAH);
#ifdef TRACEPUSH1
        if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
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
  fprintf(stderr,"FIN DE LA CONSTRUCTION DU CBT\n");
#endif

  T = Regularise(CBT, nminima, nbcell);
  if (T == NULL)
  {   fprintf(stderr, "llpetopo_anim_llpetopo() : Regularise failed\n");
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
      if (i % rs == 0) printf("\n");
      printf("%3d ", M[i]);
    }
    printf("\n");
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
  {   fprintf(stderr, "llpetopo_anim_llpetopo() : calloc failed for T\n");
      return(0);
  }

  I = InverseCBT(CBT, nminima, nbcell);
  if (I == NULL)
  {   fprintf(stderr, "llpetopo_anim_llpetopo() : InverseCBT failed\n");
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

  /* marque VALIDE les composantes ayant des points marques dans leur zone d'influence */
  for (i = 0; i < N; i++)
    if (B[i])
      Label(CBT, M[i]) = VALIDE;

  /* marque INVALIDE les feuilles non marquees et les empile. */
  for (x = 1; x < nminima; x++)
  {
    if ((Label(CBT, x) != VALIDE) && (Label(CBT, x) != DISPARU))
    { 
#ifdef TRACEREGUL
          if (trace) printf("%d: empile etiquette invalide %d au niveau 0\n", 
                             tracedate++, x);
#endif
      Label(CBT, x) = INVALIDE;
      FahPush(FAH, x, 0);
    }
  }


  /* 
     l'etiquette correspondant a un noeud INVALIDE est remplacee
     par celle de son pere
     remonte dans l'arbre : un noeud dont tous les fils sont INVALIDE
     est marque INVALIDE
  */

  while (! FahVide(FAH))
  {
    int32_t toutes_invalides;

    i = FahNiveau(FAH);
    x = FahPop(FAH);
#ifdef TRACEREGUL
    if (trace) 
    {
      printf("%d: POP etiquette %d ; niveau %d\n", 
              tracedate++, x, i);
    }
#endif
    if (x >= nminima)
    {
      toutes_invalides = 1;
      y = FirstSon(CBT, x);
      n = FirstSon(CBT, x + 1) - FirstSon(CBT, x);
      for (j = 0; j < n; j++)
        if ((Label(CBT, I[y+j]) != INVALIDE) && (Label(CBT, I[y+j]) != DISPARU))
          toutes_invalides = 0;
      if (toutes_invalides)
      {
#ifdef TRACEREGUL
        if (trace) printf("%d: etiquette invalide %d niveau %d\n", 
                             tracedate++, x, i);
#endif
        Label(CBT, x) = INVALIDE;
      }
    }

    if (Label(CBT, x) == INVALIDE)
    {
      k = Father(CBT, x);
      if (k != NIL)
      {
        T[x] = k;
        FahPush(FAH, k, i+1);
      }
    } /* if (Label(CBT, x) == INVALIDE) */
  } /* while (! FahVide(FAH)) */
  
#ifdef TRACEREGUL
  if (trace) 
  {
    for (x = 1; x < nbcell; x++)
    {
      if (x % 40 == 0) printf("\n");
      printf("%d ", T[x]);
    }
    printf("\n");
  }
#endif

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
      if (i % rs == 0) printf("\n");
      printf("%3d ", M[i]);
    }
    printf("\n");
  }
#endif

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
  }
#endif

#ifdef VERBOSE
  fprintf(stderr,"FIN DE L'ELIMINATION DES MINIMA NON PERTINENTS\n");
#endif

  /* ================================================ */
  /* NOYAU HOMOTOPIQUE                                */
  /* ================================================ */

  /* remet les labels a 0 car ils sont utilises par LowComAnc */
  for (x = 1; x < nbcell; x++) Label(CBT, x) = 0;

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
    if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
                       tracedate++, x, x%rs, x/rs, SOURCE[x]);
#endif
  } /* for x */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"lpetopo() : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n");
     return(0);
  }
#endif
#ifdef ANIMATE
  for (x = 0; x < N; x++)
  {
    i = x % rs;
    j = x / rs;
    if ((i >= XMIN) && (i <= XMAX) && (j >= YMIN) && (j <= YMAX))
    {
      attroff(A_ATTRIBUTES);
      if (IsSet(x, EN_FAH)) attron(ATTR_CANDIDAT);
      sprintf(buf, "%d", SOURCE[x]);
      mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
    }
  }  
  (void)refresh();        
#endif

  /* PROPAGATION AVEC HEURISTIQUE D'INONDATION */
  while (! FahVide(FAH))
  {
    ncc = 0;
    x = FahPop(FAH);
    UnSet(x, EN_FAH);
    nombre_examens += 1;

#ifdef TRACEPOP
    if (trace) 
    {
      printf("%d: POP point %d (%d,%d) ; niveau %d\n", 
              tracedate++, x, x%rs, x/rs, SOURCE[x]);
    }
#endif
#ifdef ANIMATE
    oldvalx = SOURCE[x];      /* sauve l'ancien niveau de x */
    i = x % rs;
    j = x / rs;
    if ((i >= XMIN) && (i <= XMAX) && (j >= YMIN) && (j <= YMAX))
    {
      attroff(A_ATTRIBUTES);
      sprintf(buf, "    ");
      mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
      sprintf(buf, "%d", SOURCE[x]);
      mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
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

    new = LowComAnc(CBT, ncc, etiqcc, SOURCE[x]);

    if ((new != NIL) && (Data(CBT, new) < SOURCE[x])) /* le point est CB-simple */
    {      /* conceptuellement : Data(CBT, new) - 1 */
#ifdef TRACEBAISSE
      if (trace) printf("%d: point %d (%d,%d) ; niveau =  %d, etiq = %d\n", 
                        tracedate++, x, x%rs, x/rs, SOURCE[x], M[x]);
#endif
      nombre_abaissements += 1;
      SOURCE[x] = Data(CBT, new);   /* conceptuellement : Data(CBT, new) - 1 */
      M[x] = new;
#ifdef TRACEBAISSE
      if (trace) printf("baisse au niveau  %d, etiq <- %d\n", 
                         SOURCE[x], M[x]);
#endif
#ifdef ANIMATE
        i = x % rs;
        j = x / rs;
        if ((i >= XMIN) && (i <= XMAX) && (j >= YMIN) && (j <= YMAX))
        {
          attroff(A_ATTRIBUTES);
          sprintf(buf, "%d", oldvalx);
          mvaddstr((j - YMIN) * 2 - 1, (i - XMIN)*4, buf);
          sprintf(buf, "    ");
          mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
          attron(ATTR_ABAISSE);
          sprintf(buf, "%d", SOURCE[x]);
          mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
          i = oldx % rs;
          j = oldx / rs;
          if ((i >= XMIN) && (i <= XMAX) && (j >= YMIN) && (j <= YMAX))
          {
            attroff(A_ATTRIBUTES);
            sprintf(buf, "    ");
            mvaddstr((j - YMIN) * 2 - 1, (i - XMIN)*4, buf);
          }
          (void)refresh(); 
          (void)getch();
          oldx = x;
        }
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
            if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
                               tracedate++, y, y%rs, y/rs, SOURCE[y]);
#endif
#ifdef ANIMATE
            i = y % rs;
            j = y / rs;
            if ((i >= XMIN) && (i <= XMAX) && (j >= YMIN) && (j <= YMAX))
            {
              attroff(A_ATTRIBUTES);
              sprintf(buf, "    ");
              mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
              attron(ATTR_CANDIDAT);
              sprintf(buf, "%d", SOURCE[y]);
              mvaddstr((j - YMIN) * 2, (i - XMIN)*4, buf);
            }
#endif
          } /* if (! IsSet(y, EN_FAH)) */
        } /* if (y != -1) */
      } /* for k */

      FahPush(FAH, x, SOURCE[x]);
      Set(x, EN_FAH);
#ifdef TRACEPUSH
      if (trace) printf("%d: empile point %d (%d,%d) au niveau %d\n", 
                        tracedate++, x, x%rs, x/rs, SOURCE[x]);
#endif

    } /* if ((new != NIL) && (Data(CBT, new) < SOURCE[x]))  le point est CB-simple */
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

#ifdef ANIMATE
  (void)getch();
  endwin();
#endif

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

  /* ---------------------------------------------------------- */
  /* fabrique le nom de l'image resultat */
  /* ---------------------------------------------------------- */

  name1 = image->name; 
  if (name1 == NULL) name1 = "?";
  name2 = marqueurs->name;
  if (name2 == NULL) name2 = "?";
  name = (char *)calloc(1,strlen(name1) + strlen(name2) + strlen("lpetopo(,)") + 1);
  if (name == NULL)
  {   fprintf(stderr,"lpetopo() : malloc failed for name\n");
      return 0;
  }
  strcpy(name, "lpetopo(");
  strcat(name, name1);
  strcat(name, ",");
  strcat(name, name2);
  strcat(name, ")");
  image->name = name;

  return(1);
}

