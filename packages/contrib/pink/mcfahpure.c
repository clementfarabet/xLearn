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
/* structure de file d'attente hierarchique de points d'image */
/* les valeurs les plus basses correspondent a la plus grande priorite */
/* Michel Couprie */

/* un point d'une image F est reperee par son index */
/* dans le tableau mono-dimension ou les valeurs sont rangees */
/* par ligne, puis par colonne */

/* d'apres F. Meyer: "Un Algorithme Optimal de Ligne de Partage des Eaux" */
/* version "pure" (un point insere sous le niveau courant recree une file a ce niveau) */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcfahpure.h>

/*
#define TESTFahp
#define VERBOSE
*/

/* ==================================== */
Fahp * CreeFahpVide(
  index_t taillemax)
/* ==================================== */
{
  index_t i;
  Fahp * L = (Fahp *)calloc(1,4 * sizeof(index_t) + 
                          (FAHP_NPRIO+4)*sizeof(FahpElt *) + 
                          taillemax*sizeof(FahpElt));
  if (L == NULL)
  {
    fprintf(stderr, "erreur allocation Fahp\n");
    exit(1);
  }
  L->Max = taillemax;
  L->Util = 0;
  L->Maxutil = 0;
  L->Niv = 0;
  for (i = 0; i < taillemax - 1; i++)
    L->Elts[i].Next = &(L->Elts[i+1]);
  L->Elts[taillemax - 1].Next = NULL;
  L->Libre = &(L->Elts[0]);
  for (i = 0; i < FAHP_NPRIO; i++)
    L->Tete[i]= NULL;
  L->Queue = NULL;
  return L;
} /* CreeFahpVide() */

/* ==================================== */
void FahpFlush(
  Fahp * L)
/* ==================================== */
{
  index_t i;
  L->Niv = 0;
  L->Util = 0;
  for (i = 0; i < L->Max - 1; i++)
    L->Elts[i].Next = &(L->Elts[i+1]);
  L->Elts[L->Max - 1].Next = NULL;
  L->Libre = &(L->Elts[0]);
  for (i = 0; i < FAHP_NPRIO; i++)
    L->Tete[i]= NULL;
  L->Queue = NULL;
} /* FahpFlush() */

/* ==================================== */
int32_t FahpVide(
  Fahp * L)
/* ==================================== */
{
  return (L->Queue == NULL);
} /* FahpVide() */

/* ==================================== */
int32_t FahpVideNiveau(
  Fahp * L,
  int32_t niv)
/* ==================================== */
{
  if (L->Queue == NULL) return 1;
  if (L->Niv > niv) return 1;
  return 0;
} /* FahpVideNiveau() */

/* ==================================== */
int32_t FahpNiveau(
  Fahp * L)
/* ==================================== */
{
  if (L->Queue == NULL)
  {
    fprintf(stderr, "erreur Fahp vide\n");
    exit(1);
  }
  return L->Niv;
} /* FahpNiveau() */

/* ==================================== */
index_t FahpPop(
  Fahp * L)
/* ==================================== */
{
  index_t V;
  FahpElt * FE;
  if (L->Queue == NULL)
  {
    fprintf(stderr, "erreur Fahp vide\n");
    exit(1);
  }

  L->Util--;

  if (L->Queue == L->Tete[L->Niv]) /* on se prepare a effacer le dernier */
  {                                /* element du niveau courant: il faut */
    L->Tete[L->Niv] = NULL;        /* annuler le pointeur de tete */
    do (L->Niv)++;                 /* et incrementer le niveau */
    while ((L->Niv < FAHP_NPRIO)          
            && (L->Tete[L->Niv] == NULL));
  }
  V = L->Queue->Point;
  FE = L->Queue->Next;
  L->Queue->Next = L->Libre;
  L->Libre = L->Queue;
  L->Queue = FE;
  return V;
} /* FahpPop() */
  
/* ==================================== */
index_t FahpFirst(
  Fahp * L)
/* ==================================== */
{
  if (L->Queue == NULL)
  {
    fprintf(stderr, "erreur Fahp vide\n");
    exit(1);
  }
  return L->Queue->Point;
} /* FahpFirst() */
  
/* ==================================== */
void FahpPush(
  Fahp * L,
  index_t Po,
  int32_t Ni)
/* ==================================== */
{
  if (L->Libre == NULL)
  {
    fprintf(stderr, "erreur Fahp pleine\n");
    exit(1);
  }
  if (Ni >= FAHP_NPRIO)
  {
    fprintf(stderr, "erreur niveau = %d; max autorise = %d\n", Ni, FAHP_NPRIO-1);
    exit(1);
  }

  L->Util++;
  if (L->Util > L->Maxutil) L->Maxutil = L->Util;
  if (L->Queue == NULL)    /* insertion dans une Fahp vide */
  {
    L->Queue = L->Libre;
    L->Libre = L->Libre->Next;
    L->Queue->Point = Po;
    L->Niv = Ni;
    L->Queue->Next = NULL;
    L->Tete[Ni] = L->Queue;
  } /* if Fahp Vide */
  else if (L->Tete[Ni] != NULL)   /* insertion dans la liste de niveau Ni non vide */
  {
    FahpElt * FE = L->Tete[Ni]->Next;
    L->Tete[Ni]->Next = L->Libre;
    L->Libre = L->Libre->Next;
    L->Tete[Ni] = L->Tete[Ni]->Next;
    L->Tete[Ni]->Next = FE;
    L->Tete[Ni]->Point = Po;      
  }
  else /* (L->Tete[Ni] == NULL) */
  {
    FahpElt * FE;
    int32_t NiPrec = Ni;
    while ((NiPrec >= 0) && (L->Tete[NiPrec] == NULL)) NiPrec--; 
    if (NiPrec < 0)              /* creation d'un niveau inferieur au niveau courant */
    {
      FE = L->Queue;
      L->Queue = L->Libre;
      L->Libre = L->Libre->Next;
      L->Queue->Point = Po;
      L->Niv = Ni;
      L->Queue->Next = FE;
      L->Tete[Ni] = L->Queue;
    }
    else
    {
      L->Tete[Ni] = L->Libre;
      L->Libre = L->Libre->Next;
      L->Tete[Ni]->Point = Po;
      FE = L->Tete[NiPrec]->Next;
      L->Tete[NiPrec]->Next = L->Tete[Ni];
      L->Tete[Ni]->Next = FE;
    }
  }
} /* FahpPush() */

/* ==================================== */
void FahpTermine(
  Fahp * L)
/* ==================================== */
{
#ifdef VERBOSE
  printf("Fahp: taux d'utilisation: %g\n", (double)L->Maxutil / (double)L->Max);
#endif
  free(L);
} /* FahpTermine() */

/* ==================================== */
void FahpPrint(
  Fahp * L)
/* ==================================== */
{
  int32_t i;
  FahpElt * FE;
  if (FahpVide(L)) {printf("[]\n"); return;}
  FE = L->Queue;
  for (i = L->Niv; i < FAHP_NPRIO; i++)
    if (L->Tete[i] != NULL) 
    {
      printf("%d [ ", i);
      for (;FE != L->Tete[i]; FE = FE->Next)
#ifdef MC_64_BITS
        printf("%lld ", FE->Point);
#else
        printf("%d ", FE->Point);
#endif
#ifdef MC_64_BITS
      printf("%lld ]\n", FE->Point);
#else
      printf("%d ]\n", FE->Point);
#endif
      FE = FE->Next;
    }
} /* FahpPrint() */

#ifdef TESTFahp
void main()
{
  Fahp * L = CreeFahpVide(5);
  char r[80];
  int32_t p, n;

  do
  {
    printf("commande (qUIT, PuSH, PoP, pRINT, TESTvIDE) > ");
    scanf("%s", r);
    switch (r[0])
    {
      case 'u':
        printf("niveau > ");
        scanf("%d", &n);
        printf("valeur > ");
        scanf("%d", &p);
        FahpPush(L, p, n);
        break;
      case 'o': 
        printf("pop: %d\n", FahpPop(L));
        break;
      case 'p': FahpPrint(L); break;
      case 'v': 
        printf("vide: %d\n", FahpVide(L));
        break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  FahpTermine(L);
}
#endif
