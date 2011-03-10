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
Librairie mcfusion :

structure et algorithmes pour la "fusion" optimale de 
   composantes connexes dans un graphe 

Michel Couprie 1996
*/

/* 
Probleme: 
  soit E un ensemble fini (prenons E = [1..N])
  soit Gamma inclus dans E x E

  donnee: le graphe non oriente G = (E, Gamma)

  resultat: une fonction f: E ----> E
                            s ----> sr
        tq: pour tous les s, s' de E, si il existe un chemin de s a s' dans G 
            alors il existe un seul sr dans E / f(s) = f(s') = f(sr) = sr

  On veut pouvoir construire f de facon incrementale en introduisant un a un
  les elements de Gamma (couples (s, s')). 
  On appellera "fusion" une telle etape.

  La representation de f se fera sous forme de tableau, de sorte que 
  l'evaluation de f(x) pour un x particulier soit en O(1).

  L'algorithme de fusion presente est en O(N) au pire, mais
  N-1 etapes de fusion ont une complexite globale au pire majoree par O(N log N).
  
  Il existe un algorithme lineaire pour resoudre le probleme global en une 
  seule fois, mais il ne fournit pas les fonctions intermediaires
  correspondant aux introductions successives de couples (s, s').

  Taille memoire requise : ((N * 4) + 4) * 4 octets
*/

/* M. Couprie 1996 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mcfusion.h>

/*
#define TESTFusion
#define TRACEFusion
*/

/* ==================================== */
Fus * CreeFus(
  int32_t taillemax)
/* ==================================== */
{
  int32_t i;
  Fus * L = (Fus *)calloc(1,sizeof(Fus));
  if (L == NULL)
  {   fprintf(stderr, "CreeFus() : malloc failed for L\n");
      return(NULL);
  }

#ifdef TRACEFusion
printf("CreeFus: taille %d\n", taillemax);
#endif

  L->Max = taillemax;
  L->Tabf = (int32_t *)calloc(1,taillemax * sizeof(int32_t));
  if (L->Tabf == NULL)
  {   fprintf(stderr, "CreeFus() : malloc failed for Tabf\n");
      return(NULL);
  }
  for (i = 0; i < taillemax; i++)
    (L->Tabf)[i] = i;
  L->Tabelts = (LisElt *)calloc(1,taillemax * sizeof(LisElt));
  if (L->Tabelts == NULL)
  {   fprintf(stderr, "CreeFus() : malloc failed for Tabelts\n");
      return(NULL);
  }
  L->Tablis = (LisElt **)calloc(1,taillemax * sizeof(LisElt *));
  if (L->Tablis == NULL)
  {   fprintf(stderr, "CreeFus() : malloc failed for Tablis\n");
      return(NULL);
  }
  for (i = 0; i < taillemax; i++)
  {
    (L->Tablis)[i] = &((L->Tabelts)[i]);
    (L->Tabelts)[i].Next = NULL;
    (L->Tabelts)[i].Data = 1;
  }
  return L;
}

/* ==================================== */
void FusReinit(
  Fus * L)
/* ==================================== */
{
  int32_t i, taillemax;
#ifdef TRACEFusion
printf("FusReinit\n");
#endif

  taillemax = L->Max;
  for (i = 0; i < taillemax; i++)
    (L->Tabf)[i] = i;
  for (i = 0; i < taillemax; i++)
  {
    (L->Tablis)[i] = &((L->Tabelts)[i]);
    (L->Tabelts)[i].Next = NULL;
    (L->Tabelts)[i].Data = 1;
  }
}

/* ==================================== */
void FusTermine(
  Fus * L)
/* ==================================== */
{

#ifdef TRACEFusion
printf("FusTermine\n");
#endif
  free(L->Tabf);
  free(L->Tabelts);
  free(L->Tablis);
  free(L);
}

/* ==================================== */
void FusPrint(
  Fus * L)
/* ==================================== */
{
  int32_t i;
  LisElt * LE;
  for (i = 0; i < L->Max; i++)
  {
    printf("f(%d)=%d ; f-1 = [ ", i, (L->Tabf)[i]);
    if (((L->Tablis)[i]) != NULL)
    {
      printf("%d ", i);
      for (LE = (L->Tablis)[i]->Next; LE != NULL; LE = LE->Next)
        printf("%d ", LE->Data);
    }
    printf(" ]\n");
  }
}

/* ==================================== */
int32_t FusF(
  Fus * L,
  int32_t i)
/* ==================================== */
{
  return (L->Tabf)[i];
}

/* ==================================== */
int32_t Fusion(
  Fus * L,
  int32_t A, 
  int32_t B)
/* retourne le representant de la fusion ou -1 si la fusion est inutile */
/* ==================================== */
{
  int32_t RA;   /* representant de A */
  int32_t RB;   /* representant de B */  
  int32_t R;    /* representant choisi pour la fusion */
  int32_t D;    /* celui qui doit disparaitre en tant que representant */
  LisElt * LE, *Last;

#ifdef TRACEFusion
printf("Fusion: %d , %d\n", A, B);
#endif

  RA = (L->Tabf)[A]; 
  RB = (L->Tabf)[B]; 

  if (RA == RB) return -1;    /* rien a faire */

  /* choix du representant : */
  /* on prend celui qui represente deja le plus d'elements */
  if ((L->Tablis)[RB]->Data > (L->Tablis)[RA]->Data) 
    { R = RB; D = RA; }
  else 
    { R = RA; D = RB; }
  
  /* propagation du nouveau representant */
  /* et memorisation du dernier de la liste */
  (L->Tabf)[D] = R;
  Last = (L->Tablis)[D];
  for (LE = (L->Tablis)[D]->Next; LE != NULL; LE = LE->Next)
  {
    (L->Tabf)[LE->Data] = R;
    if (LE->Next == NULL) Last = LE;
  }

  /* fusion des listes */
  (L->Tablis)[R]->Data += (L->Tablis)[D]->Data;
  (L->Tablis)[D]->Data = D;
  Last->Next = (L->Tablis)[R]->Next;
  (L->Tablis)[R]->Next = (L->Tablis)[D];

  /* elimination de l'ancien representant */
  (L->Tablis)[D] = NULL;
  return R;
}

/* ==================================== */
void Fusion1rep(
  Fus * L,
  int32_t A,
  int32_t B)
/* variante de la fusion ou A est le representant "force'" */
/* ==================================== */
{
  int32_t RB;   /* representant de B */  
  LisElt * LE, *Last;

#ifdef TRACEFusion
printf("Fusion1rep: %d , %d\n", A, B);
#endif

  RB = (L->Tabf)[B]; 
  if ((L->Tabf)[A] != A)
  {   
    printf("Fusion1rep() : ATTENTION : mauvais representant\n");
  }

  if (A == RB) return;    /* rien a faire */

  /* propagation du nouveau representant */
  /* et memorisation du dernier de la liste */
  (L->Tabf)[RB] = A;
  Last = (L->Tablis)[RB];
  for (LE = (L->Tablis)[RB]->Next; LE != NULL; LE = LE->Next)
  {
    (L->Tabf)[LE->Data] = A;
    if (LE->Next == NULL) Last = LE;
  }

  /* fusion des listes */
  (L->Tablis)[A]->Data += (L->Tablis)[RB]->Data;
  (L->Tablis)[RB]->Data = RB;
  Last->Next = (L->Tablis)[A]->Next;
  (L->Tablis)[A]->Next = (L->Tablis)[RB];

  /* elimination de l'ancien representant */
  (L->Tablis)[RB] = NULL;
}

/* ==================================== */
int32_t FusNormalise(
  Fus * L)
/* normalise et retourne le nombre de representants */
/* normalisation: le renumerotage des representants se fait a partir de 1 */
/* ==================================== */
{
  int32_t i;
  int32_t NbRep = 0;
  LisElt * LE;
  for (i = 0; i < L->Max; i++)
  {
    if (((L->Tablis)[i]) != NULL)   /* c'est un representant */
    {
      NbRep = NbRep + 1;
      (L->Tabf)[i] = NbRep;
      for (LE = (L->Tablis)[i]->Next; LE != NULL; LE = LE->Next)
        (L->Tabf)[LE->Data] = NbRep;
    }
  }
  return NbRep;
}

#ifdef TESTFusion
void main()
{
  Fus * L = CreeFus(5);
  char r[80];
  int32_t A, B;

  do
  {
    printf("commande (qUIT, fUSION, nORMALISATION, pRINT) > ");
    scanf("%s", r);
    switch (r[0])
    {
      case 'p': FusPrint(L); break;
      case 'n': (void)FusNormalise(L); break;
      case 'f': scanf("%d", &A);
                scanf("%d", &B);
                Fusion(L, A, B);
                break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  FusTermine(L);
}
#endif
