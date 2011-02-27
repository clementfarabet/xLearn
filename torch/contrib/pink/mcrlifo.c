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
   Librairie mcrlifo :

   fonctions pour la gestion d'une liste lifo
   avec réallocation en cas de répassement

   Michel Couprie 2009
*/

/* #define TESTRlifo */
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcrlifo.h>

/* ==================================== */
Rlifo * CreeRlifoVide(index_t taillemax)
/* ==================================== */
#undef F_NAME
#define F_NAME "CreeRlifoVide" 
{
  Rlifo * L = (Rlifo *)calloc(1,sizeof(Rlifo) + sizeof(index_t) * (taillemax-1));
  if (L == NULL)
  {   
#ifdef MC_64_BITS
    fprintf(stderr, "%s: malloc failed : %lld bytes\n", F_NAME, sizeof(Rlifo) + sizeof(index_t) * (taillemax-1));
#else
    fprintf(stderr, "%s: malloc failed : %d bytes\n", F_NAME, sizeof(Rlifo) + sizeof(index_t) * (taillemax-1));
#endif
    return NULL;
  }
  L->Max = taillemax;
  L->Sp = 0;
  return L;
}

/* ==================================== */
void RlifoFlush(Rlifo * L)
/* ==================================== */
{
  L->Sp = 0;
}

/* ==================================== */
int32_t RlifoVide(Rlifo * L)
/* ==================================== */
{
  return (L->Sp == 0);
}

/* ==================================== */
index_t RlifoPop(Rlifo * L)
/* ==================================== */
#undef F_NAME
#define F_NAME "RlifoPop" 
{
  if (L->Sp == 0)
  {
    fprintf(stderr, "%s: empty stack\n", F_NAME);
    exit(1);
  }
  L->Sp -= 1;
  return L->Pts[L->Sp];
}

/* ==================================== */
index_t RlifoHead(Rlifo * L)
/* ==================================== */
#undef F_NAME
#define F_NAME "RlifoHead" 
{
  if (L->Sp == 0)
  {
    fprintf(stderr, "%s: empty stack\n", F_NAME);
    exit(1);
  }
  return L->Pts[L->Sp-1];
}

/* ==================================== */
void RlifoPush(Rlifo ** L, index_t V)
/* ==================================== */
#undef F_NAME
#define F_NAME "RlifoPush" 
{
  if ((*L)->Sp > (*L)->Max - 1)
  {
    index_t newsize = (*L)->Max + (*L)->Max;
    (*L)->Max = newsize;
    (*L) = (Rlifo *)realloc((*L), sizeof(Rlifo) + sizeof(index_t) * (newsize - 1));
    if ((*L) == NULL)
    {
#ifdef MC_64_BITS
      fprintf(stderr, "%s: realloc failed : %lld bytes\n", F_NAME, sizeof(Rlifo) + sizeof(index_t) * (newsize-1));
#else
      fprintf(stderr, "%s: realloc failed : %d bytes\n", F_NAME, sizeof(Rlifo) + sizeof(index_t) * (newsize-1));
#endif
      exit(1);
    }
  }
  (*L)->Pts[(*L)->Sp] = V;
  (*L)->Sp += 1;
}

/* ==================================== */
void RlifoPrint(Rlifo * L)
/* ==================================== */
{
  index_t i;
  if (RlifoVide(L)) {printf("[]"); return;}
  printf("[ ");
  for (i = 0; i < L->Sp; i++)
#ifdef MC_64_BITS
    printf("%lld ", L->Pts[i]);
#else
    printf("%d ", L->Pts[i]);
#endif
  printf("]");
}

/* ==================================== */
void RlifoPrintLine(Rlifo * L)
/* ==================================== */
{
  index_t i;
  if (RlifoVide(L)) {printf("[]\n"); return;}
  printf("[ ");
  for (i = 0; i < L->Sp; i++)
#ifdef MC_64_BITS
    printf("%lld ", L->Pts[i]);
#else
    printf("%d ", L->Pts[i]);
#endif
  printf("]\n");
}

/* ==================================== */
void RlifoTermine(Rlifo * L)
/* ==================================== */
{
  free(L);
}

#ifdef TESTRlifo
void main()
{
  Rlifo * L = CreeRlifoVide(3);
  RlifoPrint(L);
  if (RlifoVide(L)) printf("RlifoVide OUI\n");
  RlifoPush(L,1);
  RlifoPrint(L);
  if (!RlifoVide(L)) printf("RlifoVide NON\n");
  RlifoPush(L,2);
  RlifoPrint(L);
  RlifoPush(L,3);
  RlifoPrint(L);
  printf("RlifoPop %d attendu 3\n", RlifoPop(L));
  RlifoPrint(L);
  RlifoPush(L,4);
  RlifoPrint(L);
  printf("RlifoPop %d attendu 4\n", RlifoPop(L));
  RlifoPrint(L);
  printf("RlifoPop %d attendu 2\n", RlifoPop(L));
  RlifoPrint(L);
  printf("RlifoPop %d attendu 1\n", RlifoPop(L));
  RlifoPrint(L);
  if (RlifoVide(L)) printf("RlifoVide OUI\n");
  printf("maintenant sortie attendue sur rlifo pleine :\n");
  RlifoPush(L,3);
  RlifoPush(L,3);
  RlifoPush(L,3);
  RlifoPush(L,3);  
}
#endif

