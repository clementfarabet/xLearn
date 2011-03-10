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

/* ordering of deletion directions - DON'T MODIFY THEM */
  #define U     0
  #define D     1
  #define N     2
  #define S     3
  #define E     4
  #define W     5

  #define UD      0
  #define NS      1
  #define EW      2



  typedef struct {
		unsigned long int addr;
		unsigned long int x,y,z;
        void *next;
        void *prev;
  } ListElement;

  typedef struct {
        void *first;
        void *last;
  } List;

  typedef struct {
	long i, j, k;
    unsigned long int oldnei; /*** storing the previous neighbourhood ***/
  } Voxel;

  typedef struct {
  	Voxel v;
  	unsigned long int addr;
	ListElement * ptr;
    void *next;
  } Cell;

  typedef struct {
        Cell *Head;
        Cell *Tail;
        int Length;
  } PointList;

  typedef struct {
        ListElement *first;
        ListElement *last;
  } DoubleList;

  typedef struct {
        unsigned long int x, y, z;
        void             *next;
  } Bordercell;           /* element of the list */

#ifdef __cplusplus
extern "C" {
#endif
//extern int32_t lpalagyi(struct xvimage *image, int32_t nsteps);
extern int palagyi_skelpar_curv_98(struct xvimage *input);
extern int palagyi_skelpar_curv_06(struct xvimage *input);
extern int palagyi_skelpar_surf_02(struct xvimage *input);
extern int palagyi_skelpar_surf_08(struct xvimage *input);
#ifdef __cplusplus
}
#endif
