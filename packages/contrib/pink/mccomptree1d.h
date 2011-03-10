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
#ifdef __cplusplus
extern "C" {
#endif
/* ============================================================================== */
/* 
  Structure de donnees pour la construction de l'arbre des composantes 1d.

  Les sommets de cet arbre representent les composantes des coupes de F,  
  a l'exception de celles qui sont egales a une composante d'un niveau inferieur.
  Il y a donc moins de N sommets (N = nombre de pixels) et de N-1 arcs.

  Une composante (sommet) est representee par une structure ctree1dnode.
*/
/* ============================================================================== */

typedef struct
{
  int32_t level;   // node's level
  int32_t father;  // value -1 indicates the root
  int32_t begin;
  int32_t end;
  int32_t area;
  int32_t height;
  int32_t volume;
} ctree1dnode;

typedef struct
{
  int32_t nbnodes;
  int32_t nbmaxnodes;
  int32_t root;
  ctree1dnode * tabnodes;
} ctree1d;

/* ==================================== */
/* PROTOTYPES */
/* ==================================== */
extern ctree1d * ComponentTree1dAlloc(int32_t N);
extern void ComponentTree1dFree(ctree1d * CT);
extern void ComponentTree1d( uint8_t *F, int32_t N, // inputs
                           ctree1d **CompTree, // output
                           int32_t **CompMap       // output
			 );
#ifdef __cplusplus
}
#endif
