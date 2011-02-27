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

typedef struct {
  int32_t setsize;      /* taille du tableau de bits (a multiplier par 32) */
  uint32_t tab[1];
} Set;

typedef struct {
  int32_t n;            /* nombre d'ensembles dans la liste */
  int32_t nmax;         /* nombre maximal d'ensembles dans la liste */
  Set * tab[1];     /* table des ensembles */
} SetList;

/* ================================================================== */
/* PROTOTYPES DES FONCTIONS POUR LES ENSEMBLES (Set) */
/* ================================================================== */

extern Set *   CreateEmptySet(int32_t setsize);
extern void    FreeSet(Set *s);
extern void    PrintSet(Set *s);
extern void    PrintSetLine(Set *s);
extern int32_t IsEmpty(Set *s);
extern void    SetEmpty(Set *s);
extern void    SetElement(Set *s, int32_t e);
extern void    ResetElement(Set *s, int32_t e);
extern int32_t InSet(Set *s, int32_t e);
extern void    Complement(Set *s);
extern void    Union(Set *s, Set *sa);
extern void    Inter(Set *s, Set *sa);
extern int32_t EqualSets(Set *s1, Set *s2);
extern void    CopySet(Set *sd, Set *ss);
extern int32_t Included(Set *s1, Set *s2);
extern int32_t StrictlyIncluded(Set *s1, Set *s2);
extern int32_t     CardSet(Set *s);

/* ================================================================== */
/* PROTOTYPES DES FONCTIONS POUR LES LISTES D'ENSEMBLES (SetList) */
/* ================================================================== */

extern void      PrintSetList(SetList * l);
extern void      PrintGraph(SetList * graph);
extern SetList * InitSetList(int32_t n, int32_t setsize);
extern void      TermineSetList(SetList * l);
extern void      FlushSetList(SetList * l);
extern void      PushSetList(SetList * l, Set *s);
extern int32_t   InSetList(SetList * l, Set *s);
extern int32_t   InclusSetList(SetList * l, Set *s);
#ifdef __cplusplus
}
#endif
