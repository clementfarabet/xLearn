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
#ifndef _mcliste_h
typedef struct {
  int32_t Max;          /* taille max de la Liste */
  int32_t Sp;           /* index de pile (pointe la 1ere case libre) */
  int32_t Pts[1];
} Liste;

/* prototypes     */
extern Liste * CreeListeVide(int32_t taillemax);
extern void ListeFlush(Liste * L);
extern int32_t ListeTaille(Liste * L);
extern int32_t ListeVide(Liste * L);
extern int32_t ListeElt(Liste * L, uint32_t n);
extern int32_t ListePop(Liste * L);
extern int32_t ListePush(Liste * L, int32_t V);
extern int32_t ListeIn(Liste * L, int32_t e);
extern void ListePrint(Liste * L);
extern void ListePrintLine(Liste * L);
extern void ListeTermine(Liste * L);
#endif
#define _mcliste_h
#ifdef __cplusplus
}
#endif
