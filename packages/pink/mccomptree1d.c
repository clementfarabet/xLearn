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
/* $id $ */
/* 
  Arbre des composantes 1d - algorithme linéaire

  Ref: C????

  Michel Couprie - mai 2004
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mclifo.h>
#include <mccomptree1d.h>

#define PARANO
#define VERBOSE

//#define DEBUG

/* ==================================== */
ctree1d * ComponentTree1dAlloc(int32_t N)
/* ==================================== */
#undef F_NAME
#define F_NAME "ComponentTree1dAlloc"
{
  ctree1d *CT;
  CT = (ctree1d *)calloc(1,sizeof(ctree1d));
  CT->tabnodes = (ctree1dnode *)calloc(1,N * sizeof(ctree1dnode));
  if ((CT == NULL) || (CT->tabnodes == NULL))
  { 
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return NULL;
  }
  CT->nbmaxnodes = N;
  CT->nbnodes = 0;
  return CT;
} // ComponentTree1dAlloc()

/* ==================================== */
void ComponentTree1dFree(ctree1d * CT)
/* ==================================== */
{
  free(CT->tabnodes);
  free(CT);
} // ComponentTree1dFree()

/* ==================================== */
int32_t Create1dNode(ctree1d * CT, int32_t level, int32_t father)
/* ==================================== */
#undef F_NAME
#define F_NAME "Create1dNode"
{
  int32_t node;
  if (CT->nbnodes >= CT->nbmaxnodes)
  { 
    fprintf(stderr, "%s : two many nodes\n", F_NAME);
    exit(0);
  }
  node = CT->nbnodes;
  CT->nbnodes += 1;
  CT->tabnodes[node].level = level; 
  CT->tabnodes[node].father = father; 
  return node;
} // Create1dNode()

/* ==================================== */
void ComponentTree1dPrint(ctree1d * CT)
/* ==================================== */
{
  int32_t i;
  printf("root = %d ;  nbnodes: %d\n", CT->root, CT->nbnodes);
  for (i = 0; i < CT->nbnodes; i++) 
  {
    printf("node: %d ; level: %d ; father: %d ; begin: %d ; end: %d ; area: %d ; height: %d ; volume: %d\n", 
	   i, CT->tabnodes[i].level, CT->tabnodes[i].father,
	   CT->tabnodes[i].begin, CT->tabnodes[i].end,
	   CT->tabnodes[i].area, CT->tabnodes[i].height, CT->tabnodes[i].volume);
  }
} // ComponentTree1dPrint()

/* ==================================== */
void ComponentTree1d( uint8_t *F, int32_t N, // inputs
                    ctree1d **CompTree, // output
                    int32_t **CompMap       // output
                  )
/* ==================================== */
#undef F_NAME
#define F_NAME "ComponentTree1d"
{
  ctree1d *CT; 
  ctree1d *CTfinal; 
  int32_t *CM; 
  ctree1dnode *TN;
  int32_t i, node, curnode, nodemem, curlev;
  Lifo * LIFO;

  //=========================================
  //=========================================
  // INIT
  //=========================================
  //=========================================

  CT = ComponentTree1dAlloc(N+1);
  if (CT == NULL)
  {   fprintf(stderr, "%s() : ComponentTree1dAlloc failed\n", F_NAME);
      exit(0);
  }
  CM = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (CM == NULL)
  {   fprintf(stderr, "%s() : malloc failed for CM\n", F_NAME);
      exit(0);
  }
  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      exit(0);
  }

  curnode = Create1dNode(CT, -1, -1); // create a dummy node with level -1 (temp. root)
  LifoPush(LIFO, curnode);

  //=========================================
  //=========================================
  // MAIN LOOP
  //=========================================
  //=========================================

  for (i = 0; i < N; i++) 
  {
#ifdef DEBUG
printf("New sample : F[%d] = %d\n", i, F[i]);
#endif
    nodemem = -1;
    curnode = LifoHead(LIFO);
    curlev = CT->tabnodes[curnode].level;
    while (F[i] < curlev)
    {
      nodemem = curnode;
      (void)LifoPop(LIFO);
      curnode = LifoHead(LIFO);
      curlev = CT->tabnodes[curnode].level;
#ifdef DEBUG
printf("Pop : curnode = %d curlev = %d\n", curnode, curlev);
#endif
    } // while
    if (F[i] == curlev) CM[i] = curnode;
    else // we know that F[i] > curlev
    {
      node = Create1dNode(CT, F[i], curnode);
      LifoPush(LIFO, node);
      curnode = node;
      curlev = F[i];
#ifdef DEBUG
printf("Push : curnode = %d curlev = %d\n", curnode, curlev);
#endif
      CM[i] = curnode;
      if (nodemem != -1) 
      {
	CT->tabnodes[nodemem].father = curnode;
#ifdef DEBUG
printf("Link : nodemem = %d curnode = %d\n", nodemem, curnode);
#endif
      }
    } // else
  } // for

  //=========================================
  //=========================================
  // POST-PROCESSING
  //=========================================
  //=========================================

  LifoTermine(LIFO);

  if (CT->nbnodes < 1)
  {   fprintf(stderr, "%s() : Empty component tree\n", F_NAME);
      exit(0);
  }

  node = 1; // search for the actual root
  while (CT->tabnodes[node].father != 0) node = CT->tabnodes[node].father;

  CTfinal = ComponentTree1dAlloc(CT->nbnodes - 1);
  if (CTfinal == NULL)
  {   fprintf(stderr, "%s() : ComponentTree1dAlloc failed\n", F_NAME);
      exit(0);
  }
  CTfinal->nbnodes = CT->nbnodes - 1;
  CTfinal->root = node - 1;
  
  for (i = 1; i < CT->nbnodes; i++)
  {
    CTfinal->tabnodes[i-1].level = CT->tabnodes[i].level;
    CTfinal->tabnodes[i-1].father = CT->tabnodes[i].father - 1;
  }
  for (i = 0; i < N; i++) CM[i] -= 1;

  ComponentTree1dFree(CT);
  *CompTree = CTfinal;
  *CompMap = CM;
} // ComponentTree1d()

/* ==================================== */
void ComponentTreeAttrib1d( 
                    uint8_t *F, int32_t N, // inputs
                    ctree1d **CompTree, // output
                    int32_t **CompMap       // output
                  )
/* ==================================== */
#undef F_NAME
#define F_NAME "ComponentTreeAttrib1d"
{
  ctree1d *CT; 
  ctree1d *CTfinal; 
  int32_t *CM; 
  ctree1dnode *TN;
  int32_t i, node, curnode, nodemem, curlev;
  Lifo * LIFO;

  //=========================================
  //=========================================
  // INIT
  //=========================================
  //=========================================

  CT = ComponentTree1dAlloc(N+1);
  if (CT == NULL)
  {   fprintf(stderr, "%s() : ComponentTree1dAlloc failed\n", F_NAME);
      exit(0);
  }
  CM = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (CM == NULL)
  {   fprintf(stderr, "%s() : malloc failed for CM\n", F_NAME);
      exit(0);
  }
  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      exit(0);
  }

  curnode = Create1dNode(CT, -1, -1); // create a dummy node with level -1 (temp. root)
  LifoPush(LIFO, curnode);

  //=========================================
  //=========================================
  // MAIN LOOP
  //=========================================
  //=========================================

  for (i = 0; i < N; i++) 
  {
#ifdef DEBUG
printf("New sample : F[%d] = %d\n", i, F[i]);
#endif
    nodemem = -1;
    curnode = LifoHead(LIFO);
    curlev = CT->tabnodes[curnode].level;
    while (F[i] < curlev)
    {
      nodemem = curnode;
      node = LifoPop(LIFO);
      curnode = LifoHead(LIFO);
      curlev = CT->tabnodes[curnode].level;
#ifdef DEBUG
printf("Pop : curnode = %d curlev = %d\n", curnode, curlev);
#endif
      CT->tabnodes[node].end = i; 
      CT->tabnodes[curnode].height = 
        mcmax(CT->tabnodes[curnode].height, CT->tabnodes[node].height);
    } // while
    if (F[i] == curlev) 
    {
      CM[i] = curnode;
    }
    else // we know that F[i] > curlev
    {
      node = Create1dNode(CT, F[i], curnode);
      LifoPush(LIFO, node);
      curnode = node;
      curlev = F[i];
#ifdef DEBUG
printf("Push : curnode = %d curlev = %d\n", curnode, curlev);
#endif
      CT->tabnodes[curnode].begin = i;
      CT->tabnodes[curnode].height = F[i];
      CM[i] = curnode;
      if (nodemem != -1) 
      {
	CT->tabnodes[nodemem].father = curnode;
#ifdef DEBUG
printf("Link : nodemem = %d curnode = %d\n", nodemem, curnode);
#endif
        CT->tabnodes[curnode].height = CT->tabnodes[nodemem].height;
        CT->tabnodes[curnode].begin = CT->tabnodes[nodemem].begin;
      }
    } // else
  } // for
  CT->tabnodes[curnode].end = i; 

  //=========================================
  //=========================================
  // POST-PROCESSING
  //=========================================
  //=========================================

  LifoTermine(LIFO);

  if (CT->nbnodes < 1)
  {   fprintf(stderr, "%s() : Empty component tree\n", F_NAME);
      exit(0);
  }

  node = 1; // search for the actual root
  while (CT->tabnodes[node].father != 0) node = CT->tabnodes[node].father;

  CTfinal = ComponentTree1dAlloc(CT->nbnodes - 1);
  if (CTfinal == NULL)
  {   fprintf(stderr, "%s() : ComponentTree1dAlloc failed\n", F_NAME);
      exit(0);
  }
  CTfinal->nbnodes = CT->nbnodes - 1;
  CTfinal->root = node - 1;
  
  for (i = 1; i < CT->nbnodes; i++)
  {
    CTfinal->tabnodes[i-1].level = CT->tabnodes[i].level;
    CTfinal->tabnodes[i-1].father = CT->tabnodes[i].father - 1;
    CTfinal->tabnodes[i-1].begin = CT->tabnodes[i].begin;
    CTfinal->tabnodes[i-1].end = CT->tabnodes[i].end;
    CTfinal->tabnodes[i-1].area = CT->tabnodes[i].end - CT->tabnodes[i].begin;
    CTfinal->tabnodes[i-1].height = CT->tabnodes[i].height - CT->tabnodes[i].level;
    CTfinal->tabnodes[i-1].volume = CT->tabnodes[i].volume;
  }
  for (i = 0; i < N; i++) CM[i] -= 1;

  ComponentTree1dFree(CT);
  *CompTree = CTfinal;
  *CompMap = CM;
} // ComponentTreeAttrib1d()

#ifdef TEST
int32_t main() {
uint8_t F[15] = {
    110,  90, 100,  10,  40,  50,  50,  50,  10,  20,  80,  60,  70,  10,  30
 //   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14
 };
  int32_t i, N=15;
  ctree1d *CT;
  int32_t *CM;

  ComponentTreeAttrib1d(F, N, &CT, &CM);
  printf("component tree1d:\n");
  ComponentTree1dPrint(CT);
  printf("component mapping:\n");
  for (i = 0; i < N; i++)
  {
    printf("%3d ", CM[i]);
  } /* for i */
  printf("\n");

  ComponentTree1dFree(CT);
  free(CM);
}
#endif
