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
/* prototype for lhisto.c */
/* ============== */

extern int32_t lhisto(
        struct xvimage *image,
        struct xvimage *mask,
        index_t *histo
);

extern int32_t lhisto1(
        struct xvimage *image,
        index_t *histo
);

extern int32_t lhisto2(
        struct xvimage *image1, 
        struct xvimage *image2, 
        struct xvimage *mask, 
        struct xvimage *histo
);

extern int32_t lhistolong(
        struct xvimage *image, 
        struct xvimage *mask,
        index_t **histo, 
        int32_t *size
);

extern int32_t lhistofloat(struct xvimage *image, struct xvimage *mask, index_t **histo, int32_t *size, 
		       float *Sincr, float *Smin, float *Smax);

extern void lhistcompact(index_t *histo, int32_t n);

extern int32_t lhistsum(index_t *histo);

#define NONMIN 1
#define NONMAX 2
#undef MAX
#define MAX 1
#undef MIN
#define MIN 2
#define NONEXTR 3

extern void labelextr1d(int32_t *F, int32_t n, uint8_t *E);

extern int32_t lhisto2image(index_t *histo, int32_t size, struct xvimage **image);

extern int32_t lhistoazimuth(struct xvimage * field, int32_t nbins, index_t **histo);
extern int32_t lhistoelevation(struct xvimage * field, int32_t nbins, index_t **histo);
extern int32_t lhisto_distance_modulo_raw (index_t * A, index_t * B, int32_t n);
extern int32_t lhisto_distance_ordinal_raw (index_t * A, index_t * B, int32_t n);
extern double lhisto_distance_modulo (index_t * A, index_t * B, int32_t n);
extern double lhisto_distance_ordinal (index_t * A, index_t * B, int32_t n);
extern int32_t lseuilhisto (struct xvimage *image, struct xvimage *masque, double p);
extern int32_t lcountvalues(struct xvimage *image, struct xvimage *mask);
#ifdef __cplusplus
}
#endif
