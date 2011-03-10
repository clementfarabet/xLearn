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
extern int32_t lskelMK3a(struct xvimage *image, 
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelEK3(struct xvimage *image, 
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelCK3a(struct xvimage *image, 
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelCK3b(struct xvimage *image, 
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelCK3(struct xvimage *image, 
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelAK3(struct xvimage *image, 
			int32_t nsteps,
			struct xvimage *inhibit,
			int32_t filter);
extern int32_t lskelMK3(struct xvimage *image, 
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t ldistaxetopo3(struct xvimage *image, 
			     struct xvimage *inhibit,
			     struct xvimage *res);
extern int32_t ldisttopo3(struct xvimage *image, 
			  struct xvimage *inhibit,
			  struct xvimage *res);
extern int32_t lskelAMK3(struct xvimage *image, 
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelACK3a(struct xvimage *image, 
			  int32_t nsteps, int32_t n_earlysteps,
			  struct xvimage *inhibit);
extern int32_t lskelACK3(struct xvimage *image, 
			 int32_t nsteps,
			 struct xvimage *inhibit);
extern int32_t lskelRK3(struct xvimage *image, 
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelSK3(struct xvimage *image, 
			int32_t nsteps,
			struct xvimage *inhibit);
extern int32_t lskelDK3(struct xvimage *image, 
			 int32_t n_steps,
			 struct xvimage *inhibit);
extern int32_t lskelDSK3(struct xvimage *image, 
			 int32_t n_steps,
			 struct xvimage *inhibit);
extern int32_t lskelDRK3(struct xvimage *image, 
			 int32_t n_steps,
			 struct xvimage *inhibit);
extern int32_t lskel1Disthmuspoints(struct xvimage *image);
#ifdef __cplusplus
}
#endif
