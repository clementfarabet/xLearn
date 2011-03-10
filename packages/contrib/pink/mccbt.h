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
#define NIL -1

typedef struct
{
  uint8_t data;    /* pour le niveau de la coupe */
  uint8_t label;   /* pour l'algo lca */
  int32_t father;
  int32_t cbtaux;          
} cbtcell;

#define ancestor cbtaux     /* utile pour la construction seulement */
#define firstson cbtaux     /* utile pour l'arbre inverse seulement */

#define Data(tab,c) (tab[c].data)
#define Father(tab,c) (tab[c].father)
#define Label(tab,c) (tab[c].label)
#define FirstSon(tab,c) (tab[c].cbtaux)
#define Aux(tab,c) (tab[c].cbtaux)

/* ============== */
/* prototypes     */
/* ============== */

extern void InitCbt(
  cbtcell * arbre,
  int32_t nbcell);

extern int32_t CreateCell(
  cbtcell * arbre,
  int32_t * nbcell,
  int32_t nbmaxcell
);

extern void SetFather(
  cbtcell * arbre,
  int32_t cell,
  int32_t f
);

extern void SetData(
  cbtcell * arbre,
  int32_t cell,
  int32_t d
);

extern void PrintCbt(
  cbtcell * arbre,
  int32_t nbcell
);

extern int32_t Anc(
  cbtcell * arbre,
  int32_t cell,
  int32_t d
);

extern int32_t DiffAnc(
  cbtcell * arbre,
  int32_t argc,
  int32_t *argv
);

extern int32_t LowComAnc(
  cbtcell * arbre,
  int32_t argc,
  int32_t *argv,
  uint8_t d
);

extern int32_t IsAnc(
  cbtcell * arbre,
  int32_t a,
  int32_t x
);

extern int32_t Euthanasie(
  cbtcell * arbre,
  int32_t argc,
  int32_t *argv
);

extern int32_t * Regularise(
  cbtcell * arbre,
  int32_t nbleaf,
  int32_t nbcell
);

extern int32_t * InverseCBT(
  cbtcell * arbre,
  int32_t nbleaf,
  int32_t nbcell
);
#ifdef __cplusplus
}
#endif
