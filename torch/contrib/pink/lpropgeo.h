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
/* prototype for lpropgeo.h */
/* ============== */

#undef MIN
#undef MAX
#define MIN  0
#define MAX  1
#define MOY  2
#define MIN1 3
#define MAX1 4
#define MOY1 5
#define MINB 6
#define MAXB 7
#define MOYB 8
#define RANDB 9

extern int32_t lpropgeo(
        struct xvimage *img1,
        struct xvimage *mask, 
        int32_t connex, 
        int32_t function
);

extern int32_t lpropgeolong(
        struct xvimage *img1,
        struct xvimage *mask, 
        int32_t connex, 
        int32_t function
	);
#ifdef __cplusplus
}
#endif
