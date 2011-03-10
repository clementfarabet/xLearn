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

typedef struct {
  int32_t area;
  double xcentre; /* coord. isobarycentre */
  double ycentre;
  double gaxe;  /* longueur demi-grand axe de l'ellipse */
  double excen; /* facteur excentricite */
  double orien; /* orientation / direction horizontale */
  double gdiam; /* longueur du plus grand demi-diametre */
} Indicateur;

extern int32_t limagemoments(
        struct xvimage *img,    /* image de depart */
        Indicateur * indic);    /* resultat */

extern int32_t lmoments(
        struct xvimage *img,    /* image de depart */
        int32_t connex,             /* 4, 8  */
        int32_t minimum,            /* booleen */
        Indicateur ** tabindic, /* tableau des resultats */
        int32_t *nlabels);          /* resultat: nombre d'extrema traites */

extern int32_t lmomentslab(
        struct xvimage *labels, /* image de depart (cc etiquetees) */
        int32_t connex,             /* 4, 8  */
        int32_t nlabels,            /* nombre de composantes */
        Indicateur ** tabindic);/* tableau des resultats */

extern void PrintIndicateur(Indicateur I);

extern int32_t llistemoments2d(
	double *X, double *Y, int32_t N,
        Indicateur * indic);     /* resultat */

extern int32_t ldirectionsprincipales2d(
	double *X, double *Y, int32_t N,
        double *xc, double *yc,      /* resultat */
        double *dx1, double *dy1,    /* resultat */
        double *dx2, double *dy2);   /* resultat */


extern int32_t ldirectionsprincipales3d(
	double *X, double *Y, double *Z, int32_t N,
        double *xc, double *yc, double *zc,      /* resultat */
        double *dx1, double *dy1, double *dz1,   /* resultat */
        double *dx2, double *dy2, double *dz2,   /* resultat */
        double *dx3, double *dy3, double *dz3);  /* resultat */

#ifdef __cplusplus
}
#endif
