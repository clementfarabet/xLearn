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
/* ============== */
/* types publics  */
/* ============== */

typedef struct LISELT {
  int32_t Data;            /* Ce champ peut contenir 2 informations de natures differentes.*/
                       /* Le 1er element de liste indique le nombre d'elements (outre  */
                       /* le representant lui-meme), et les suivants contiennent les   */
                       /* autres elements representes.                                 */
  struct LISELT * Next;
} LisElt;

typedef struct {
  int32_t Max;             /* taille max de E (en nombre d'elements) */
  int32_t * Tabf;          /* table representant la fonction f */
  LisElt ** Tablis;    /* tableau des pointeurs de liste */
  LisElt * Tabelts;    /* tableau des elements de liste */
} Fus;

/* ============== */
/* prototypes     */
/* ============== */

extern Fus * CreeFus(
  int32_t taillemax
);

extern void FusTermine(
  Fus * L
);

extern void FusReinit(
  Fus * L
);

extern void FusPrint(
  Fus * L
);

extern int32_t FusF(
  Fus * L,
  int32_t i
);

extern int32_t Fusion(
  Fus * L,
  int32_t A, 
  int32_t B
);

extern void Fusion1rep(
  Fus * L,
  int32_t A, 
  int32_t B
);

extern int32_t FusNormalise(
  Fus * L
);

#ifdef __cplusplus
}
#endif
