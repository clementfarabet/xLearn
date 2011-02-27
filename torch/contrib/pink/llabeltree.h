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
extern int32_t llabeltree(
  struct xvimage *imgx, 
  int32_t connex,
  struct xvimage *res
);

extern int32_t llabeltree2d(struct xvimage *imgx,   /* donnee: image binaire */       
                 int32_t connex,
                 struct xvimage *res     /* resultat: distances (doit être allouée) */
		     );

extern int32_t llabeltree3d(struct xvimage *imgx,   /* donnee: image binaire */       
                 int32_t connex,
                 struct xvimage *res     /* resultat: distances (doit être allouée) */
		     );

extern int32_t labelbranch2d4(uint8_t *X,  /* donnee: pointeur sur l'image x */
                   int32_t rs,            /* donnee: taille ligne */
		   int32_t N,             /* donnee: taille image */
                   int32_t current_pixel, /* donnee: numéro pixel courant */  
                   int32_t intersection,  /* donnee: flag d'intersection */  
                   uint32_t *D        /* resultat: pointeur sur distances */
); /* Etiquette les branches avec la 4-connexité */

extern int32_t labelbranch2d8(uint8_t *X,  /* donnee: pointeur sur l'image x */
                   int32_t rs,            /* donnee: taille ligne */
		   int32_t N,             /* donnee: taille image */
                   int32_t current_pixel, /* donnee: numéro pixel courant */  
                   int32_t intersection,  /* donnee: flag d'intersection */  
                   uint32_t *D        /* resultat: pointeur sur distances */
); /* Etiquette les branches avec la 8-connexité */

extern int32_t labelbranch3d6(uint8_t *X,  /* donnee: pointeur sur l'image x */
                   int32_t rs,            /* donnee: taille ligne */
                   int32_t ps,            /* donnee: taille plan */
		   int32_t N,             /* donnee: taille image */
                   int32_t current_pixel, /* donnee: numéro pixel courant */  
                   int32_t intersection,  /* donnee: flag d'intersection */  
                   uint32_t *D        /* resultat: pointeur sur distances */
); /* Etiquette les branches avec la 6-connexité */

extern int32_t labelbranch3d18(uint8_t *X,  /* donnee: pointeur sur l'image x */
                    int32_t rs,            /* donnee: taille ligne */
                    int32_t ps,            /* donnee: taille plan */
		    int32_t N,             /* donnee: taille image */
                    int32_t current_pixel, /* donnee: numéro pixel courant */  
                    int32_t intersection,  /* donnee: flag d'intersection */  
                    uint32_t *D        /* resultat: pointeur sur distances */
); /* Etiquette les branches avec la 18-connexité */

extern int32_t labelbranch3d26(uint8_t *X,  /* donnee: pointeur sur l'image x */
                    int32_t rs,            /* donnee: taille ligne */
                    int32_t ps,            /* donnee: taille plan */
		    int32_t N,             /* donnee: taille image */
                    int32_t current_pixel, /* donnee: numéro pixel courant */  
                    int32_t intersection,  /* donnee: flag d'intersection */  
                    uint32_t *D        /* resultat: pointeur sur distances */
); /* Etiquette les branches avec la 26-connexité */
#ifdef __cplusplus
}
#endif
