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
/* structure d'arbre */

/* #define TESTTREE */
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mctree.h>

/* var. globales (privees) pour les pages de cellules */
static TreePage * FirstTreePage;
static TreePage * CurTreePage;

/* ==================================== */
void TreeInit()
/* ==================================== */
{
  FirstTreePage = (TreePage *)calloc(1,sizeof(TreePage));
  if (FirstTreePage == NULL)
  {
     fprintf(stderr, "InitTree : malloc failed\n");
     exit(0);
  }
  FirstTreePage->lastfreecell = 0;
  FirstTreePage->nextpage = NULL;
  CurTreePage = FirstTreePage;
} /* TreeInit() */

/* ==================================== */
void TreeTermine()
/* ==================================== */
{
  TreePage *t = FirstTreePage;
  TreePage *tp;

  while (t)
  {
    tp = t->nextpage;
    free(t);
    t = tp;
  }
} /* TreeTermine() */

/* ==================================== */
TreeCell * AllocTreeCell()
/* ==================================== */
{
  TreeCell *c;
  if (CurTreePage->lastfreecell >= TREEPAGECELLS)
  {
    TreePage *t = (TreePage *)calloc(1,sizeof(TreePage));
    if (t == NULL)
    {
      fprintf(stderr, "AllocTreeCell : malloc failed\n");
      exit(0);
    }
    t->lastfreecell = 0;
    t->nextpage = NULL;
    CurTreePage->nextpage = t;
    CurTreePage = t;
  }
  c = &(CurTreePage->tab[CurTreePage->lastfreecell]);
  (CurTreePage->lastfreecell)++;
  return c;
} /* AllocTreeCell() */
