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

extern int32_t lskelubp(
        struct xvimage *image,
        struct xvimage *prio,
        int32_t connex,
        int32_t inhibit
);
extern int32_t lskelubp2(
        struct xvimage *image,
        struct xvimage *prio,
        int32_t connex,
        struct xvimage *imageinhib
);
extern int32_t lskelubp3d(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        int32_t connex, 
        int32_t val_inhibit
);
extern int32_t lskelubp3d2(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        int32_t connex, 
        struct xvimage *imageinhib
);
extern int32_t lskelcurv(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        struct xvimage *inhibit, 
        int32_t connex
);
extern int32_t lskelcurv3d(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        struct xvimage *inhibit, 
        int32_t connex
);
extern int32_t lskelsurf3d(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        struct xvimage *inhibit, 
	int32_t connex
);
extern int32_t ltoposhrink(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        int32_t connex, 
        int32_t tmin, 
        int32_t tmax, 
        int32_t tbmin, 
        int32_t tbmax, 
        struct xvimage *imageinhibit
);
extern int32_t ltoposhrink3d(
        struct xvimage *image, 
        struct xvimage *imageprio, 
        int32_t connex, 
        int32_t tmin, 
        int32_t tmax, 
        int32_t tbmin, 
        int32_t tbmax, 
        struct xvimage *imageinhibit
);

extern int32_t lskeleucl(
	struct xvimage *image, 
        int32_t connex, 
        struct xvimage *imageinhib
);

extern int32_t lskelend3d(
        struct xvimage *image, 
	int32_t connex, 
	uint8_t *endpoint,
	int32_t niseuil
);
extern int32_t lskelendcurvlab3d(
	struct xvimage *image, 
	int32_t connex, 
	int32_t niseuil
);
#ifdef __cplusplus
}
#endif
