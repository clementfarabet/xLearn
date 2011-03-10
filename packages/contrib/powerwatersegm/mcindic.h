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

/* $Id: mcindic.h,v 1.2 2009/01/07 12:46:33 mcouprie Exp $ */
#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t Indicstype;

extern Indicstype *Indics;       /* en global pour etre efficace */

#define Set(x,INDIC)   Indics[x]|=(1<<INDIC)
#define UnSet(x,INDIC) Indics[x]&=~(1<<INDIC)
#define UnSetAll(x)    Indics[x]=0
#define IsSet(x,INDIC) (Indics[x]&(1<<INDIC))
#define IsSetAny(x)    (Indics[x])

#define Set1(x)   Indics[x/8]|=(1<<(x%8))
#define UnSet1(x) Indics[x/8]&=~(1<<(x%8))
#define IsSet1(x) (Indics[x/8]&(1<<(x%8)))

/* ============== */
/* prototypes     */
/* ============== */

extern void IndicsInit(int32_t Size); 

extern void Indics1bitInit(int32_t Size); 
    
extern void Indics1bitReInit(int32_t Size);
    
extern void IndicsTermine();


#ifdef __cplusplus
}
#endif
