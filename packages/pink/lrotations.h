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
extern int32_t lquasishear(struct xvimage * image, double theta, double xc, double yc);
extern struct xvimage * lquasishear2(struct xvimage * image, double theta);
extern struct xvimage * lrotationRT(struct xvimage * image, double theta, double xc, double yc, uint8_t resize);
extern struct xvimage * lrotationRT3Dx(struct xvimage * image, double theta, double yc, double zc, double *newyc, double *newzc, uint8_t resize);
extern struct xvimage * lrotationRT3Dy(struct xvimage * image, double theta, double xc, double zc, double *newxc, double *newzc, uint8_t resize);
extern struct xvimage * lrotationRT3Dz(struct xvimage * image, double theta, double xc, double yc, double *newxc, double *newyc, uint8_t resize);
extern struct xvimage * lrotationInter(struct xvimage * image, double theta, double xc, double yc, uint8_t resize);
extern int32_t laffinetransformation(struct xvimage * image, double hx, double hy, double theta, double tx, double ty, struct xvimage * image2);
#ifdef __cplusplus
}
#endif
