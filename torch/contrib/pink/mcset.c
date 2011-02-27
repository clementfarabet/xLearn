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
   utilitaires pour la gestion d'ensembles et de listes d'ensembles
   Michel COUPRIE, aout 1997, mai 1998

   un ensemble est represente par un tableau de bits, plus precisement,
   par un pointeur sur le premier uint32_t de ce tableau.

   les 32 premiers elements sont dans le uint32_t d'indice 0.

   le premier element est represente par le bit 0 (poids faible).
*/

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include "mcset.h"

/* ================================================================== */
/* ================================================================== */
/* FONCTIONS POUR LES ENSEMBLES (Set) */
/* ================================================================== */
/* ================================================================== */

/* ========================================== */
Set *CreateEmptySet(int32_t setsize) 
/* ========================================== */
/* retourne un ensemble vide taille max. setsize * 32 */
{
  Set * s;
  s = (Set *)calloc(sizeof(Set) + (setsize - 1) * sizeof(uint32_t), 1);
  if (s == NULL)
  {   
    fprintf(stderr,"CreateEmptySet : calloc failed\n");
    return 0;
  }
  s->setsize = setsize;
  return s;
} /* CreateEmptySet() */

/* ========================================== */
void FreeSet(Set *s)
/* ========================================== */
{
  free(s);
} /* FreeSet() */

/* ========================================== */
void PrintSet(Set *s)
/* ========================================== */
{
  int32_t i, j;
  uint32_t t;

  for (i = 0; i < s->setsize; i++)
  {
    t = s->tab[i];
    for (j = 0; j < 32; j++)
    {
      if (t % 2) printf("%d ", j + (32 * i));
      t = t >> 1;
    }
  }
} /* PrintSet() */

/* ========================================== */
void PrintSetLine(Set *s)
/* ========================================== */
{
  PrintSet(s);
  printf("\n");
} /* PrintSetLine() */

/* ============================================ */
int32_t IsEmpty(Set *s)
/* ============================================ */
{
  int32_t i;
  for (i = 0; i < s->setsize; i++)
    if (s->tab[i]) 
      return 0;
  return 1;
} /* IsEmpty() */

/* ============================================ */
void SetEmpty(Set *s)
/* ============================================ */
{
  int32_t i;
  for (i = 0; i < s->setsize; i++)
    s->tab[i] = 0;
} /* SetEmpty() */

/* ============================================ */
void SetFull(Set *s)
/* ============================================ */
{
  int32_t i;
  for (i = 0; i < s->setsize; i++)
    s->tab[i] = 0xffffffff;
} /* SetFull() */

/* ============================================ */
void SetElement(Set *s, int32_t e)
/* ============================================ */
{
  int32_t b;
  int32_t i = e / 32;

  if (i >= s->setsize)
  {
    fprintf(stderr, "SetElement: element index out of range: %d", e);
    exit(0);
  }
  b = e % 32;
  s->tab[i] |= (uint32_t)1 << b; 
} /* SetElement() */

/* ============================================ */
void ResetElement(Set *s, int32_t e)
/* ============================================ */
{
  int32_t b;
  int32_t i = e / 32;

  if (i >= s->setsize)
  {
    fprintf(stderr, "ResetElement: element index out of range: %d", e);
    exit(0);
  }
  b = e % 32;
  s->tab[i] &= ~((uint32_t)1 << b);
} /* ResetElement() */

/* ============================================ */
int32_t InSet(Set *s, int32_t e)
/* ============================================ */
{
  int32_t b;
  int32_t i = e / 32;

  if (i >= s->setsize)
  {
    fprintf(stderr, "InSet: element index out of range: %d", e);
    exit(0);
  }
  b = e % 32;
  return (s->tab[i] & ((uint32_t)1 << b));
} /* InSet() */

/* ============================================ */
void Complement(Set *s)
/* ============================================ */
/*
  le complementaire de s est mis dans s
*/
{
  int32_t i;

  for (i = 0; i < s->setsize; i++)
    s->tab[i] = ~s->tab[i];
} /* Complement() */

/* ============================================ */
void Union(Set *s, Set *sa)
/* ============================================ */
/*
  l'union de s et de sa est mise dans s
  ATTENTION : pas de test de compatibilite de dimensions
*/
{
  int32_t i;

  for (i = 0; i < s->setsize; i++)
    s->tab[i] |= sa->tab[i];
} /* Union() */

/* ============================================ */
void Inter(Set *s, Set *sa)
/* ============================================ */
/*
  l'intersection de s et de sa est mise dans s
  ATTENTION : pas de test de compatibilite de dimensions
*/
{
  int32_t i;

  for (i = 0; i < s->setsize; i++)
    s->tab[i] &= sa->tab[i];
} /* Inter() */

/* ============================================ */
int32_t EqualSets(Set *s1, Set *s2)
/* ============================================ */
/*
  teste l'egalite de deux ensembles
  ATTENTION : pas de test de compatibilite de dimensions
*/
{
  int32_t i;
  for (i = 0; i < s1->setsize; i++)
    if (s1->tab[i] != s2->tab[i]) 
      return 0;
  return 1;
} /* EqualSets() */

/* ============================================ */
void CopySet(Set *sd, Set *ss)
/* ============================================ */
/*
  ss est copie dans sd 
  sd doit avoie ete alloue auparavant
  ATTENTION : pas de test de compatibilite de dimensions
*/
{
  int32_t i;

  for (i = 0; i < ss->setsize; i++)
    sd->tab[i] = ss->tab[i];
} /* CopySet() */

/* ============================================ */
int32_t CardSet(Set *s)
/* ============================================ */
/*
  retourne le cardinal de l'ensemble s
*/
{
  int32_t il, ib, n;
  uint32_t t;

  n = 0;
  for (il = 0; il < s->setsize; il++)
  {
    t = s->tab[il];
    for (ib = 0; ib < 32; ib++)
    {
      if (t % 2) n++;
      t = t >> 1;
    }
  }
  return n;
} /* CardSet() */

/* ============================================ */
int32_t Included(Set *s1, Set *s2)
/* ============================================ */
/*
  teste l'inclusion de s1 dans s2
  ATTENTION : pas de test de compatibilite de dimensions
*/
{
  int32_t ret;
  Set * t = CreateEmptySet(s1->setsize);
  CopySet(t, s1);
  Inter(t, s2);
  ret = EqualSets(s1, t);
  free(t);
  return ret;
} /* Included() */

/* ============================================ */
int32_t StrictlyIncluded(Set *s1, Set *s2)
/* ============================================ */
{
  return (int32_t)(Included(s1, s2) && (CardSet(s1) < CardSet(s2)));
} /* StrictlyIncluded() */

/* ================================================================== */
/* ================================================================== */
/* FONCTIONS POUR LES LISTES D'ENSEMBLES (SetList) */
/* ================================================================== */
/* ================================================================== */

/* ========================================== */
void CopySetList(SetList * d, SetList * s)
/* ========================================== */
/*
  copie la liste s dans la liste d
  ATTENTION : pas de test de compatibilite de dimensions
*/
{
  int32_t i;
  for (i = 0; i < s->n; i++) CopySet(d->tab[i], s->tab[i]);
  d->n = s->n;
} /* CopySetList() */

/* ========================================== */
void PrintSetList(SetList * l)
/* ========================================== */
{
  int32_t i;

  for (i = 0; i < l->n; i++)
  {
    printf("[%d] ", i);
    PrintSetLine(l->tab[i]);
  }
  printf("\n");
} /* PrintSetList() */

/* ========================================== */
void PrintGraph(SetList * graph)
/* ========================================== */
{
  int32_t u, v;

  for (u = 0; u < graph->n; u++)
  {
    printf("sommet %3d : successeurs ", u);
    for (v = 0; v < graph->n; v++)
      if (InSet(graph->tab[u], v))
      {
      	if (!InSet(graph->tab[v], u)) fprintf(stderr, "graphe non symetrique %d\n", (uint32_t)u);
        printf("%d ", v);
      }
      else
      if (InSet(graph->tab[v], u)) fprintf(stderr, "graphe non symetrique %d\n", (uint32_t)u);
    printf("\n");
  }
} /* PrintGraph() */

/* ========================================== */
SetList * InitSetList(int32_t n, int32_t setsize)
/* ========================================== */
{
  SetList * l;
  int32_t i;
  l = (SetList *)calloc(1,sizeof(SetList) + (n-1) * sizeof(Set *));
  if (l == NULL)
  { 
    fprintf(stderr, "InitSetList: malloc failed\n");
    exit(0);
  }
  for (i = 0; i < n; i++)
    l->tab[i] = CreateEmptySet(setsize);
  l->n = 0;
  l->nmax = n;
  return l;
} /* InitSetList() */

/* ========================================== */
void TermineSetList(SetList * l)
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < l->n; i++)
    FreeSet(l->tab[i]);
  free(l);
} /* TermineSetList() */

/* ========================================== */
void FlushSetList(SetList * l)
/* ========================================== */
{
  l->n = 0;
} /* FlushSetList() */

/* ========================================== */
void PushSetList(SetList * l, Set *s)
/* ========================================== */
{
  if (l->n >= l->nmax)
  { 
    fprintf(stderr, "PushSetList: liste pleine\n");
    exit(0);
  }
  CopySet(l->tab[l->n], s);
  l->n += 1;
} /* PushSetList() */

/* ========================================== */
int32_t InSetList(SetList * l, Set *s)
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < l->n; i++)
    if (EqualSets(s, l->tab[i]))
      return 1;
  return 0;
} /* InSetList() */

/* ========================================== */
int32_t InclusSetList(SetList * l, Set *s)
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < l->n; i++)
    if (Included(s, l->tab[i]))
      return 1;
  return 0;
} /* InclusSetList() */

/* ========================================== */
void CliqueMax(Set *C, Set *P, SetList *G, SetList *LC)
/* ========================================== */
/*
  Fonction RECURSIVE
  C est une clique du graphe G
  P set une liste de sommets de G dont tous les antecedents sont dans C 
  LC est la liste resultat contenant les cliques max
*/
{
  int32_t i;
  Set * savset;
  if (IsEmpty(P)) 
  {
    if (!InclusSetList(LC, C))
      PushSetList(LC, C);
  }
  else
  {
    savset = CreateEmptySet(P->setsize);
    for (i = 0; i < P->setsize * 32; i++)
      if (InSet(P, i))
      {
        CopySet(savset, P);
        ResetElement(P, i);
        Inter(P, G->tab[i]);
        SetElement(C, i);
        CliqueMax(C, P, G, LC);
        ResetElement(C, i);
        CopySet(P, savset);
      }
  } 
} /* CliqueMax() */

#ifdef DEBUGSET
void main()
{
  SetList *G;
  SetList *LC;
  Set *C = CreateEmptySet(1);
  Set *P = CreateEmptySet(1);

  SetElement(P, 0);
  SetElement(P, 1);
  SetElement(P, 2);
  SetElement(P, 3);
  SetElement(P, 4);
  PrintSetLine(P);

  G = InitSetList(5, 1);
  LC = InitSetList(20, 1);

  SetElement(G->tab[0], 1);
  SetElement(G->tab[1], 0);
  SetElement(G->tab[0], 2);
  SetElement(G->tab[2], 0);
  SetElement(G->tab[2], 1);
  SetElement(G->tab[1], 2);
  SetElement(G->tab[1], 4);
  SetElement(G->tab[4], 1);
  SetElement(G->tab[2], 4);
  SetElement(G->tab[4], 2);
  SetElement(G->tab[3], 4);
  SetElement(G->tab[4], 3);
  SetElement(G->tab[0], 4);
  SetElement(G->tab[4], 0);
  SetElement(G->tab[0], 3);
  SetElement(G->tab[3], 0);
  G->n = 5;
  PrintGraph(G);

  CliqueMax(C, P, G, LC);

  PrintSetList(LC);

  TermineSetList(G);
  TermineSetList(LC);


} /* main() */
#endif
