/*Copyright ESIEE (2009) 

Author :
Camille Couprie (c.couprie@esiee.fr)

Contributors : 
Hugues Talbot (h.talbot@esiee.fr)
Leo Grady (leo.grady@siemens.com)
Laurent Najman (l.najman@esiee.fr)

This software contains some image processing algorithms whose purpose is to be
used primarily for research.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
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


#include "powerwatersegm.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void gageodilate_union_find( uint32_t  *F, /* f : image seeds */ 
				    uint32_t  *G, /* g : image weights */
				    uint32_t * O,        /* O : result of the reconstruction by dilation of g under f */
				    int ** edges,      /* list of couple of vertices forming edges*/
				    int32_t rs,        /* rs : row size */
				    int32_t cs,        /* cs : col size */
				    int32_t ds,        /* ds : depth size */
				    int max_weight, 
				    bool quicksort);
  

  
#ifdef __cplusplus
}
#endif
