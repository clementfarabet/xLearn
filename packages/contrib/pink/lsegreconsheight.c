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
/* 
   Operateurs utilisant l'arbre des composantes.
   =============================================
   (algorithme de P. Salembier)

   Operateur : 
      lsegreconsheight (avec reconstruction basee sur la signature de l'attribut contrast)

*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcfahsalembier.h>
#include <mcutil.h>
#include <mcindic.h>
#include <lsegreconsheight.h>

#define PARANO
/*
#define VERBOSE
#define DEBUG
#define DEBUGRECONS
#define DEBUGFLOOD
*/

#define ATTR_HEIGHT
#define ATTR_PERIM
#define ATTR_HBORD
#define ATTR_CONTRAST

#include "lattrib.h"

/* ======================================================================== */
/* ======================================================================== */
/* OPERATEURS BASES SUR L'ARBRE DES COMPOSANTES */
/* ======================================================================== */
/* ======================================================================== */

/* ==================================== */
int32_t lsegreconsheight(struct xvimage *image, int32_t connex, int32_t param)
/* ==================================== */
{
  register int32_t i, k;         /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t N = rs * cs;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * CTREE;          /* resultat : l'arbre des composantes compacte' */

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lsegreconsheight: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "lsegreconsheight: mauvaise connexite: %d\n", connex);
      return 0;
  } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)calloc(1,N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "lsegreconsheight() : malloc failed for STATUS\n");
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "lsegreconsheight() : calloc failed for number_nodes\n");
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "lsegreconsheight() : calloc failed for node_at_level\n");
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "lsegreconsheight() : InitCompTree failed\n");
      return(0);
  }

  /* ================================================ */
  /* INITIALISATIONS                                  */
  /* ================================================ */

  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 1; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);

#ifdef VERBOSE
  fprintf(stderr, "init terminee\n");
#endif

  /* ================================================ */
  /* APPEL FONCTION RECURSIVE flood                   */
  /* ================================================ */

  (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 


#ifdef VERBOSE
  fprintf(stderr, "flood terminee\n");
#endif
#ifdef DEBUG
  AfficheCompTree(TREE);
#endif

  CTREE = CompTree2CompactTree(TREE, number_nodes);

#ifdef VERBOSE
  fprintf(stderr, "CompTree2CompactTree terminee\n");
#endif
#ifdef DEBUG
  AfficheCompactTree(CTREE);
#endif

  CalculeAttributs(CTREE);

#ifdef VERBOSE
  fprintf(stderr, "CalculeAttributs terminee\n");
#endif

  (void)FiltreHeightRec(CTREE, 0, param);

#ifdef VERBOSE
  fprintf(stderr, "FiltreHeightRec terminee\n");
#endif

  (void)MaximiseSegmentation(CTREE, 0);

#ifdef VERBOSE
  fprintf(stderr, "MaximiseSegmentation terminee\n");
#endif

  Reconstruction(CTREE, 0);

#ifdef VERBOSE
  fprintf(stderr, "Reconstruction terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(CTREE);
  WriteCompactTree(CTREE, "ctree.graph");
#endif

  RecupereSegmentation(CTREE, STATUS, rs, N, F);

#ifdef VERBOSE
  fprintf(stderr, "RecupereSegmentation terminee\n");
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(CTREE);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  return(1);
} /* lsegreconsheight() */
