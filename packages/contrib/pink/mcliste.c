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
/* structure de liste d'entiers */

/* #define TESTListe */
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcliste.h>

/* ==================================== */
Liste * CreeListeVide(int32_t taillemax)
/* ==================================== */
#undef F_NAME
#define F_NAME "CreeListeVide"
{
  Liste * L = (Liste *)calloc(1,sizeof(Liste) + sizeof(int32_t) * (taillemax-1));
  if (L == NULL)
  {
    fprintf(stderr, "%s: erreur calloc\n", F_NAME);
    exit(1);
  }
  L->Max = taillemax;
  L->Sp = 0;
  return L;
}

/* ==================================== */
void ListeFlush(Liste * L)
/* ==================================== */
{
  L->Sp = 0;
}

/* ==================================== */
int32_t ListeTaille(Liste * L)
/* ==================================== */
{
  return (L->Sp);
}

/* ==================================== */
int32_t ListeVide(Liste * L)
/* ==================================== */
{
  return (L->Sp == 0);
}

/* ==================================== */
int32_t ListeElt(Liste * L, uint32_t n)
/* ==================================== */
#undef F_NAME
#define F_NAME "ListeElt"
{
  if (n >= L->Sp)
  {
    fprintf(stderr, "%s: erreur hors limite\n", F_NAME);
    exit(1);
  }
  return L->Pts[n];
}

/* ==================================== */
int32_t ListePop(Liste * L)
/* ==================================== */
#undef F_NAME
#define F_NAME "ListePop"
{
  if (L->Sp == 0)
  {
    fprintf(stderr, "%s: erreur Liste vide\n", F_NAME);
    exit(1);
  }
  L->Sp -= 1;
  return L->Pts[L->Sp];
}
  
/* ==================================== */
int32_t ListePush(Liste * L, int32_t V)
/* ==================================== */
#undef F_NAME
#define F_NAME "ListePush"
{
  if (L->Sp > L->Max - 1)
  {
    fprintf(stderr, "%s: erreur Liste pleine\n", F_NAME);
    exit(1);
  }
  L->Pts[L->Sp] = V;
  L->Sp += 1;
  return L->Sp - 1; 
}

/* ==================================== */
int32_t ListeIn(Liste * L, int32_t e)
/* ==================================== */
{
  int32_t i;
  for (i = 0; i < L->Sp; i++)
    if (L->Pts[i] == e) return 1;
  return 0;
}

/* ==================================== */
void ListePrint(Liste * L)
/* ==================================== */
{
  int32_t i;
  if (ListeVide(L)) {printf("[]"); return;}
  printf("[ ");
  for (i = 0; i < L->Sp; i++)
    printf("%d ", L->Pts[i]);
  printf("]");
}

/* ==================================== */
void ListePrintLine(Liste * L)
/* ==================================== */
{
  int32_t i;
  if (ListeVide(L)) {printf("[]\n"); return;}
/*
  printf("Max = %d ; Sp = %d \n", L->Max, L->Sp);
*/
  printf("[ ");
  for (i = 0; i < L->Sp; i++)
    printf("%d ", L->Pts[i]);
  printf("]\n");
}

/* ==================================== */
void ListeTermine(Liste * L)
/* ==================================== */
{
  free(L);
}

#ifdef TESTListe
void main()
{
  Liste * L = CreeListeVide(3);
  ListePrint(L);
  if (ListeVide(L)) printf("ListeVide OUI\n");
  ListePush(L,1);
  ListePrint(L);
  if (!ListeVide(L)) printf("ListeVide NON\n");
  ListePush(L,2);
  ListePrint(L);
  ListePush(L,3);
  ListePrint(L);
  printf("ListePop %d attendu 3\n", ListePop(L));
  ListePrint(L);
  ListePush(L,4);
  ListePrint(L);
  printf("ListePop %d attendu 4\n", ListePop(L));
  ListePrint(L);
  printf("ListePop %d attendu 2\n", ListePop(L));
  ListePrint(L);
  printf("ListePop %d attendu 1\n", ListePop(L));
  ListePrint(L);
  if (ListeVide(L)) printf("ListeVide OUI\n");
  printf("maintenant sortie attendue sur liste pleine :\n");
  ListePush(L,3);
  ListePush(L,3);
  ListePush(L,3);
  ListePush(L,3);  
}
#endif

