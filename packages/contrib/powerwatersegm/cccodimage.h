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

#include<stdbool.h>
#include <lMSF.h>

extern int neighbor_node_edge( int indice, int num, int rs, int cs, int ds);
extern  int neighbor( int indice, int num, int rs, int cs, int ds);
extern int neighbor_edge(int indice, int num, int rs, int cs, int ds);
extern int neighbor_edge_3D(int node1, int node2,  int i, int k,  int rs,  int cs, int ds);

extern uint32_t * color_standard_weights_PW( struct xvimage * image_r, struct xvimage * image_v, struct xvimage * image_b, 
                                             uint32_t* weights, int ** edges, int * seeds,  int size_seeds, uint32_t * max_weight , bool quicksort);
extern int color_standard_weights( struct xvimage * image_r, struct xvimage * image_v, struct xvimage * image_b, 
                                   uint32_t * weights, int ** edges, int * seeds, int size_seeds,   bool geod, bool quicksort );
extern uint32_t * grey_weights_PW( struct xvimage * image , int ** edges, int * seeds, int size_seeds,  uint32_t * weights , bool quicksort);
extern void grey_weights( struct xvimage * image ,  uint32_t * weights, int ** edges, int * seeds,   int size_seeds,  bool geod, bool quicksort);

void compute_edges(int ** edges,  int rs,  int cs,  int ds);
void print_gradient(int ** edges,  uint32_t * weights,  int rs,  int cs,  int ds);
extern struct xvimage * mult_image_value(struct xvimage * image, double n);
extern struct xvimage * boundary(struct xvimage * image);




