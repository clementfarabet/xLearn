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
Librairie mccbt :

calcul de l'arbre des coupes 

Michel Couprie --- novembre 1996 
*/
/* derniere modification le 3/2/97 : regularisation de l'arbre lineaire */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcfah.h>
#include <mcindic.h>
#include <mccbt.h>
#include <mcutil.h>

/*
#define TRACEETIQ
#define TRACEPOP
#define TRACEPUSH
#define VERBOSE
#define PRINTCBT
#define PRINTIMAGE
#define TRACECBT
*/

#define PARANO

/* ============================================= */
/* ============================================= */
/* FONCTIONS PRIVEES */
/* ============================================= */
/* ============================================= */

/* ==================================== */
int32_t mccbt_NotIn(
  int32_t e,
  int32_t *list,                   
  int32_t n)                       
/* ==================================== */
{
/* renvoie 1 si e n'est pas dans list, 0 sinon */
/* e : l'element a rechercher */
/* list : la liste (tableau d'entiers) */
/* n : le nombre d'elements dans la liste */
  while (n > 0)
    if (list[--n] == e) return 0;
  return 1;
} /* mccbt_NotIn() */

/* ==================================== */
int32_t RgMinimum(
  cbtcell * arbre,
  int32_t *list,             
  int32_t n)                       
/* renvoie le rang d'une cellule de valeur data min de list */
/* suppose que la liste a au moins 1 element */
/* list : la liste (tableau d'entiers) */
/* n : le nombre d'elements dans la liste */
/* ==================================== */
{                              
  int32_t m = --n;
  while (n > 0)
    if (arbre[list[--n]].data < arbre[list[m]].data) m = n;
  return m;
} /* RgMinimum() */

/* ==================================== */
void TriCroissant(
  cbtcell * arbre,
  int32_t *list,             
  int32_t n)                       
/* trie la liste list par valeurs croissantes des valeurs data */
/* methode : tri par extraction (utilise la recherche du minimum) */
/* list : la liste (tableau d'entiers) */
/* n : le nombre d'elements dans la liste */
/* ==================================== */
{
  int32_t i, m, tmp;
  for (i = 0; i < n-1; i++)
  {
    m = i + RgMinimum(arbre, list+i, n - i);
    if (m != i)
    {
      tmp = list[m];
      list[m] = list[i];
      list[i] = tmp;
    }
  }
} /* TriCroissant() */

/* ============================================= */
/* ============================================= */
/* FONCTIONS PUBLIQUES */
/* ============================================= */
/* ============================================= */


/* ==================================== */
void InitCbt(
  cbtcell * arbre,
  int32_t nbcell)
/* ==================================== */
{
  int32_t i;
  for (i = 0; i <nbcell ; i++)
  {
    arbre[i].label = 0;
    arbre[i].data = 0;
    arbre[i].father = NIL;
    arbre[i].ancestor = NIL;
  } /* for i */
} /* InitCbt() */


/* ==================================== */
int32_t CreateCell(
  cbtcell * arbre,
  int32_t * nbcell,
  int32_t nbmaxcell)
/* ==================================== */
{
  int32_t n = *nbcell;
  if (n >= nbmaxcell)
  {
    fprintf(stderr, "CreateCell: too many cells\n");
    exit(0);
  }

  arbre[n].label = 0;
  arbre[n].data = 0;
  arbre[n].father = NIL;
  arbre[n].ancestor = NIL;
  *nbcell += 1;
#ifdef TRACECBT
printf("CreateCell -> %d\n", n);
#endif
  return n;
} /* CreateCell() */

/* ==================================== */
void SetFather(
  cbtcell * arbre,
  int32_t cell,
  int32_t f)
/* ==================================== */
{
  arbre[cell].father = f;
#ifdef TRACECBT
printf("SetFather: father(%d) <- %d\n", cell, f);
#endif
} /* SetFather() */

/* ==================================== */
void SetData(
  cbtcell * arbre,
  int32_t cell,
  int32_t d)
/* ==================================== */
{
  arbre[cell].data = d;
#ifdef TRACECBT
printf("SetData: data(%d) <- %d\n", cell, d);
#endif
} /* SetData() */

/* ==================================== */
void PrintCbt(
  cbtcell * arbre,
  int32_t nbcell)
/* ==================================== */
{
  int32_t i;


  printf("function [g]=cbt()\n");
  printf("node_label = []\n");
  printf("n = %d\n", nbcell - 1);

  printf("ta   = [");
  for (i = 1; i < nbcell - 1 ; i++)
    printf("%3d ", arbre[i].father);
  printf("]\n");

  printf("he   = [");
  for (i = 1; i < nbcell - 1 ; i++)
    printf("%3d ", i);
  printf("]\n");

  printf("data = [");
  for (i = 1; i < nbcell ; i++)
    printf("%3d ", arbre[i].data);
  printf("]\n");

  printf("aux  = [");
  for (i = 1; i < nbcell ; i++)
    printf("%3d ", arbre[i].cbtaux);
  printf("]\n");

  printf("for i = 1:n\n");
  printf("  node_label(i) = '[' + string(i) + ',' + string(data(i)) + ']'\n");
  printf("end\n");
  printf("g = MakeGraph('cbt', 0, n, ta, he)\n");
  printf("g(33) = node_label'\n");

} /* PrintCbt() */

/* ==================================== */
int32_t Anc(
  cbtcell * arbre,
  int32_t cell,
  int32_t d)
/* ==================================== */
{
  int32_t a = cell;
  while ((arbre[a].father != NIL) && (arbre[arbre[a].father].data <= d)) 
    a = arbre[a].father;
  /* (arbre[a].father == NIL) || (arbre[arbre[a].father].data > d) */
 
#ifdef TRACECBT
printf("Anc(%d, %d) -> %d\n", cell, d, a);
#endif
  return a;
} /* Anc() */

/* ==================================== */
int32_t DiffAnc(
  cbtcell * arbre,
  int32_t argc,
  int32_t *argv)
/* ==================================== */
/*
   Donnees : arbre, un arbre des coupes
             argc, le nombre de labels a tester
             argv, la liste des labels a tester
   Resultats :
             argc, le nombre d'ancetres differents des labels de la liste argv originale
             argv, la liste des ancetres differents
*/ 
{
  int32_t nv = 0;
  int32_t a;
  int32_t i;

#ifdef TRACECBT
printf("DiffAnc(");
for (i = 0; i < argc; i++) printf(" %d ", argv[i]);
printf(") -> ");
#endif

  for (i = 0; i < argc; i++)
  {
    a = argv[i];
    while ((arbre[a].father != NIL) || (arbre[a].ancestor != NIL))
    {
      if (arbre[a].ancestor != NIL) 
        a = arbre[a].ancestor;
      else
        a = arbre[a].father;
    }
    if (a != argv[i]) arbre[argv[i]].ancestor = a;
    if (mccbt_NotIn(a, argv, nv))
    {
      argv[nv] = a;
      nv += 1;
    }
  }
#ifdef TRACECBT
for (i = 0; i < nv; i++) printf(" %d ", argv[i]);
printf("\n");
#endif
  return nv;
} /* DiffAnc() */

/* ==================================== */
int32_t LowComAnc(
  cbtcell * arbre,
  int32_t argc,
  int32_t *argv,
  uint8_t d)
/* retourne le plus proche commun ancetre des cellules de la liste (argc, argv)
   dont le champ data est <= d, ou NIL s'il n'existe pas. 
*/
/* ==================================== */
{
  int32_t x, i, lca, NoComAnc;
  if (argc <= 0) return -1;
  if (argc == 1) return argv[0];

  x = argv[0];      /* index de la premiere cellule */
  while ((x != NIL) && (arbre[x].data <= d)) 
  {                            /* marque 1 tous les ancetres de x "sous" d */
    arbre[x].label = 1;
    x = arbre[x].father;
  }  /* while ((x != NIL) && (arbre[x].data <= d)) */
  
  for (i = 1; i < argc; i++)   /* boucle pour les autres cellules */
  {
    x = argv[i];
    while ((x != NIL) && (arbre[x].data <= d)) 
    {                          /* remonte les ancetres de x sans depasser d */
      if (arbre[x].label > 0)  /* on a un ancetre commun */
      {
      	arbre[x].label = 2;    /* on le marque 2 */
      	break;                 /* on arrete la remontee (sort du while) */
      }
      else
        x = arbre[x].father;   /* on continue la remontee */
    }  /* while ((x != NIL) && (arbre[x].data <= d)) */
    NoComAnc = ((x == NIL) || (arbre[x].data > d));   /* pas d'ancetre commun */
    if (NoComAnc) break;                   /* on sort aussi du for */
  } /* for (i = 1; i < argc; i++) */

  x = argv[0];        /* index de la premiere cellule */
  while ((x != NIL) && (arbre[x].data <= d)) 
  {                   /* derniere remontee: demarque et repere le lca */
    if (arbre[x].label == 2) lca = x;
    arbre[x].label = 0;
    x = arbre[x].father;
  }  /* while ((x != NIL) && (arbre[x].data <= d)) */
  
#ifdef TRACECBT
printf("LowComAnc(");
for (i = 0; i < argc; i++) printf(" %d ", argv[i]);
printf(") -> ");
if (NoComAnc) printf("NIL\n"); else printf("%d\n", lca);
#endif

  if (NoComAnc)                /* pas d'ancetre commun */
    return NIL;
  else
    return lca;
} /* LowComAnc() */

/* ==================================== */
int32_t IsAnc(
  cbtcell * arbre,
  int32_t a,
  int32_t x)
/* tests if a ancestor of x */
/* ==================================== */
{
  if (a == x) return 1;
  while (arbre[x].father != NIL)
  {
    x = arbre[x].father;
    if (a == x) return 1;
  }
  return 0;
} /* IsAnc() */

/* ==================================== */
int32_t Euthanasie(
  cbtcell * arbre,
  int32_t argc,
  int32_t *argv)
/* elimine les elements de argv qui sont ancetres d'autres elements de argv */
/* ==================================== */
{
  int32_t i, j, x;

#ifdef TRACECBT
printf("Euthanasie(");
for (i = 0; i < argc; i++) printf(" %d ", argv[i]);
printf(") -> ");
#endif

  /* met le label 3 a toutes les cellules de argv */
  for (i = 0; i < argc; i++) arbre[argv[i]].label = 3;

  for (i = 0; i < argc; i++)
  {
    if (argv[i] != NIL)
    {
      x = arbre[argv[i]].father;
      while (x != NIL)
      {
        if (arbre[x].label == 3) 
        {
          arbre[x].label = 0; 
          for (j = 0; j < argc; j++) 
            if (argv[j] == x) argv[j] = NIL;
        }
        x = arbre[x].father;
      }  /* while (x != NIL) */
    } /* if (argv[i] != NIL) */
  } /* for i */

  /* remet le label 0 a toutes les cellules de argv */
  for (i = 0; i < argc; i++) arbre[argv[i]].label = 0;

  j = i = 0;           /* compacte le tableau en eliminant les NIL */
  while (i < argc)
  {
    if (argv[i] != NIL) { argv[j] = argv[i]; j++; }
    i++;
  }

#ifdef TRACECBT
for (i = 0; i < j; i++) printf(" %d ", argv[i]);
printf("\n");
#endif

  return j;            /* nouveau nombre d'elements */
} /* Euthanasie() */

/* ==================================== */
int32_t * Regularise(
  cbtcell * arbre,
  int32_t nbleaf,
  int32_t nbcell)
/* ==================================== */
{
/* 
   remplace les ensembles connectes de cellules ayant une meme valeur de data
   par une cellule (nouvelle) possedant pour filles toutes les cellules
   filles (survivantes) des cellules remplacees et pour data la valeur
   data des cellules remplacees

   renvoie une table de correspondance ancienne cellule -> nouvelle cellule

   methode:
     1/ on reserve la place pour la table de correspondance T
        (nb de cellules non feuilles) et on l'initialise a 0
     2/ on parcourt les cellules non feuilles sauf la racine.
        si une cellule a pour pere une cellule de meme niveau, 
          on etablit la correspondance T[fils] <- pere
     3/ on parcourt T en RETROGRADE et on fait : 
          si T[x] != 0 et T[T[x]] != 0 alors 
            T[x] = T[T[x]]
     4/ on parcourt toutes les cellules c sauf la derniere (pas de pere)
        si T[father(c)] != 0 alors father(c) <- T[father(c)]
     5/ on renvoie T
*/
  int32_t *T;
  int32_t a, c;
#ifdef TRACECBT
printf("Regularise()");
#endif

 T = (int32_t *)calloc(nbcell - nbleaf, sizeof(int32_t));
  if (T == NULL)
  {
    fprintf(stderr, "Regularise: calloc failed\n");
    exit(0);
  }

  for (c = nbleaf; c < nbcell - 1; c++)   /* pour toutes les non-feuilles sauf la racine */
  {
    a = arbre[c].father;
#ifdef PARANO
    if (a == NIL) 
    {
      fprintf(stderr, "FATALE ERREUR : la cellule %d n'a pas de pere\n", c);
      exit(0);
    }
#endif
    if (arbre[a].data == arbre[c].data)   /* qui ont un pere au meme niveau */
      T[c - nbleaf] = a;                  /* enregistre le 'nouveau pere' */
  } /* for c */

  for (c = nbcell - 2; c >= nbleaf; c--)
    if ((T[c - nbleaf] != 0) && (T[T[c - nbleaf] - nbleaf] != 0))
      T[c - nbleaf] = T[T[c - nbleaf] - nbleaf];

  for (c = 1; c < nbcell - 1; c++)
    if (T[arbre[c].father - nbleaf] != 0) 
    {
      a = T[arbre[c].father - nbleaf];
      arbre[c].father = a;
    }

  return T;
} /* Regularise() */

/* ==================================== */
int32_t * InverseCBT(
  cbtcell * arbre,
  int32_t nbleaf,
  int32_t nbcell)
/* ==================================== */
{
/* 
   Construit une structure de pointeurs inverses pour le CBT arbre.
   Les feuilles de l'arbre n'ont pas de pointeur inverse, elles sont
     reconnues grace au test : c < nbleaf.
   La premiere cellule non feuille a pour indice nbleaf.
   L'arbre inverse est represente par un tableau : 

                                    ----------------------
    arbre[nbleaf].firstson     ->   | 1er fils de nbleaf | 
                                    ----------------------
                                    | 2nd fils de nbleaf | 
                                    ----------------------
                                    |       ...          | 
                                    ----------------------
                                    | der.fils de nbleaf | 
                                    ----------------------
    arbre[nbleaf+1].firstson   ->   | 1er f. de nbleaf+1 | 
                                    ----------------------
                                    | 2nd f. de nbleaf+1 | 
                                    ----------------------
                                    |       ...          | 
                                    ----------------------

   Pour chaque cellule, le champ firstson donne le pointeur sur son 1er fils dans le 
   tableau. Pour les feuilles, ce pointeur est NIL.

   Le nombre de fils d'une cellule c non feuille ET NON RACINE (la racine est la derniere)
   est obtenu par firstson(c+1) - firstson(c)

   Dans ce tableau il y a nbcell-1 elements (la racine n'est fils de personne) indices de 0 
   a nbcell - 2. 
   Le nombre de fils de la racine r est donc obtenu par nbcell - 2 - firstson(r)

   Algorithme: 

     soit I un tableau de taille nbcell

       1ere ETAPE : COMPTAGE DES FILS

     soit T un tableau de taille nbcell - nbleaf, initialisee a 0

     pour tout c de 1 a nbcell - 2 (la derniere n'a pas de pere) faire
       a = pere(c)
       T[a - nbleaf] += 1
     finpour

       2eme ETAPE : ATTRIBUTION DES POINTEURS firstson

     pour tout c de 1 a nbleaf - 1 faire arbre[c].firstson = NIL finpour
     i = 0
     pour tout c de nbleaf a nbcell - 1 faire 
       arbre[c].firstson = i
       i += T[c - nbleaf]
     finpour

       3eme ETAPE : REMPLISSAGE DU TABLEAU I

     pour tout c de 1 a nbcell - 2 (la derniere n'a pas de pere) faire
       a = pere(c)
       T[a - nbleaf] -= 1
       fa = arbre[a].firstson + T[a - nbleaf]
       I[fa] = c
     finpour     

     libere T
     retourne I
*/
  int32_t *I;
  int32_t *T;
  int32_t a, c, i;
#ifdef TRACECBT
printf("InverseCBT()");
#endif

 I = (int32_t *)calloc(1,nbcell * sizeof(int32_t));
 T = (int32_t *)calloc(nbcell - nbleaf, sizeof(int32_t));
  if ((I == NULL) || (T == NULL))
  {
    fprintf(stderr, "InverseCBT: allocation failed\n");
    exit(0);
  }

  /*     1ere ETAPE : COMPTAGE DES FILS      */

  for (c = 1; c < nbcell - 1; c++)   /* pour toutes les cellules sauf la derniere */
  {
    a = arbre[c].father;
    T[a - nbleaf] += 1;
  }
  
  /*     2eme ETAPE : ATTRIBUTION DES POINTEURS firstson    */

  for (c = 1; c < nbleaf; c++)   /* pour toutes les feuilles */
    arbre[c].firstson = NIL;
  i = 0;
  for (c = nbleaf; c < nbcell; c++)   /* pour toutes les non - feuilles */
  {
    arbre[c].firstson = i;
    i += T[c - nbleaf];
  }

  /*       3eme ETAPE : REMPLISSAGE DU TABLEAU I        */

  for (c = 1; c < nbcell - 1; c++)   /* pour toutes les cellules sauf la derniere */
  {
    a = arbre[c].father;
    T[a - nbleaf] -= 1;
    i = arbre[a].firstson + T[a - nbleaf];
    I[i] = c;
  }    

  free(T);
  return I;

} /* InverseCBT() */


#ifdef TESTCBT
#define MAXNBCELL 10
static cbtcell arbre[MAXNBCELL];
static int32_t nbleaf = 5;
static int32_t nbcell = 5;

/* ============================================= */
void main()
/* ============================================= */
{
  char r[80];
  int32_t A, B;
  int32_t C[4];
  int32_t *I;

  InitCbt(arbre, nbleaf);
  do
  {
    printf("commande (qUIT, cREATECELL, SETfATHER, SETdATA, INvERSE\n");
    printf("           pRINTCBT, aNC, DiFFANC, lOWCOMANC, tRICROISSANT) > ");
    scanf("%s", r);
    switch (r[0])
    {
      case 'a': printf("cell ? ");
                scanf("%d", &A);
                printf("data ? ");
                scanf("%d", &B);
                A = Anc(arbre, A, B);
                printf("resultat: %d \n", A);
                break;
      case 'i': printf("cells ? ");
                for (B = 0; B < 4; B++)
                {
                  scanf("%d", &A);
                  if (A<0) break;
                  C[B] = A;
                }
                printf("data ? ");
                scanf("%d", &A);
                B = DiffAnc(arbre, B, C, A);
                printf("resultat : ");
                for (A = 0; A < B; A++)
                  printf(" %d ", C[A]);
                printf("\n");
                break;
      case 't': printf("cells ? ");
                for (B = 0; B < 4; B++)
                {
                  scanf("%d", &A);
                  if (A<0) break;
                  C[B] = A;
                }
                TriCroissant(arbre, C, B);
                printf("resultat : ");
                for (A = 0; A < B; A++)
                  printf(" %d ", C[A]);
                printf("\n");
                break;
      case 'l': printf("cells ? ");
                for (B = 0; B < 4; B++)
                {
                  scanf("%d", &A);
                  if (A<0) break;
                  C[B] = A;
                }
                B = LowComAnc(arbre, B, C);
                printf("resultat : %d\n", B);
                break;
      case 'v': I = InverseCBT(arbre, nbleaf, nbcell);
                for (B = 0; B < nbcell; B++)
                {
                  printf("%d : %d\n", B, I[B]);
                }
                break;
      case 'c': A = CreateCell(arbre, &nbcell, MAXNBCELL);
                printf("cell created : %d\n", A);
                break;
      case 'f': scanf("%d", &A);
                scanf("%d", &B);
                SetFather(arbre, A, B);
                break;
      case 'd': scanf("%d", &A);
                scanf("%d", &B);
                SetData(arbre, A, B);
                break;
      case 'p': PrintCbt(arbre, nbcell);
                break;
      case 'q': break;
    }
  } while (r[0] != 'q');
}
#endif

