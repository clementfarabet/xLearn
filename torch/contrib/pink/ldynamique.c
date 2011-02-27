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
  Calcul de la dynamique ordonnée (nouvelle version)

  Ref: Ber05
  G. Bertrand, "A new definition of the dynamics", Proceedings of the 7th International Symposium on Mathematical Morphology,
  Springer, Computational Imaging and Vision series, Vol. 30, pp. 197-206, 2005.

  Michel Couprie - septembre 2005
  
Updates: 

Algo: 
  Input: K(F) // nodes of the component tree
         par  // "parent" relation defining the tree, with par(root) = nil
	 mu   // map from [1..m] to the leaves of K(F)
  Output: D   // map from [1..m] to Z

  for all c in K(F)
    alpha(c) = 0
  for i from m downto 1 do
    c = mu(i)
    while (c != nil) and (alpha(c) == 0) do
      alpha(c) = i
      c = par(c)
    D(i) = h(c)

*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcfahsalembier.h>
#include <mccomptree.h>
#include <mcutil.h>
#include <mcindic.h>
#include <ldynamique.h>
#include <assert.h>

#define EN_FAHS     0 
#define WATERSHED  1
#define MASSIF     2
#define MODIFIE    4
#define LCA1         0x08

#define VERBOSE
//#define DEBUG
#define PARANO

/* =============================================================== */
static int32_t i_Partitionner(int32_t *A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements q tq T[A[q]] <= T[A[p]] et les autres.
*/
{
  int32_t t;
  int32_t x = T[A[p]];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (T[A[j]] > x);
    do i++; while (T[A[i]] < x);
    if (i < j) { t = A[i]; A[i] = A[j]; A[j] = t; }
    else return j;
  } /* while (1) */   
} /* i_Partitionner() */

/* =============================================================== */
static int32_t i_PartitionStochastique(int32_t *A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements k tels que T[A[k]] <= T[A[q]] et les autres, 
  avec q tire au hasard dans [p,r].
*/
{
  int32_t t, q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  return i_Partitionner(A, T, p, r);
} /* i_PartitionStochastique() */

/* =============================================================== */
static void i_TriRapideStochastique (int32_t * A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/* 
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris) 
  par ordre croissant 
*/
{
  int32_t q; 
  if (p < r)
  {
    q = i_PartitionStochastique(A, T, p, r);
    i_TriRapideStochastique (A, T, p, q) ;
    i_TriRapideStochastique (A, T, q+1, r) ;
  }
} /* i_TriRapideStochastique() */

/* ==================================== */
int32_t lordermaxima(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t nblabels)
/* ==================================== */
/*! \fn int32_t lordermaxima(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t nblabels)
    \param image (entrée) : une image
    \param order (entrée/sortie) : labels définissant les maxima (de 1 à nbmaxima, 0 pour les non-maxima)
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D) 
    \param nblabels (entrée) : nombre de maxima plus 1
    \return code erreur : 0 si échec, 1 sinon
    \brief ordonne les labels sur les maxima par altitude croissante
*/
#undef F_NAME
#define F_NAME "lordermaxima"
{
  int32_t i;
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *O = SLONGDATA(order);      /* l'image de labels */
  int32_t *A; // table de correspondance pour le tri
  int32_t *T; // table avec l'altitude de chaque maximum
  
  if ((rowsize(order) != rs) || (colsize(order) != cs) || (depth(order) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  A = (int32_t *)malloc(nblabels * sizeof(int32_t));
  T = (int32_t *)malloc(nblabels * sizeof(int32_t));
  if ((A == NULL) || (T == NULL))
  {   fprintf(stderr, "%s() : malloc failed\n", F_NAME);
      return(0);
  }
  for (i = 0; i < nblabels; i++) A[i] = i;

  for (i = 0; i < N; i++) 
    if (O[i])
      T[O[i]] = (int32_t)F[i];

  i_TriRapideStochastique (A, T, 1, nblabels-1);

  for (i = 1; i < nblabels; i++)
    T[A[i]] = i;
  for (i = 0; i < N; i++) 
    if (O[i])
      O[i] = T[O[i]];

  free(A);
  free(T);
  return 1;
} // lordermaxima()

/* ==================================== */
int32_t lordermaximasurf(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t nblabels)
/* ==================================== */
/*! \fn int32_t lordermaxima(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t nblabels)
    \param image (entrée) : une image
    \param order (sortie) : labels définissant les maxima (de 1 à nbmaxima, 0 pour les non-maxima)
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D) 
    \param nblabels (entrée) : nombre de maxima plus 1
    \return code erreur : 0 si échec, 1 sinon
    \brief ordonne les labels sur les maxima par surface croissante
*/
#undef F_NAME
#define F_NAME "lordermaximasurf"
{
  int32_t c, i, j, k, x;
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *O = SLONGDATA(order);      /* l'image de labels */
  int32_t *A; // table de correspondance pour le tri
  int32_t *T; // table avec l'altitude de chaque maximum
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t *CM;                      /* etat d'un pixel */
  ctree * CT;                   /* resultat : l'arbre des composantes */
  
  if ((rowsize(order) != rs) || (colsize(order) != cs) || (depth(order) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  FAHS = CreeFahsVide(N);
  if (FAHS == NULL)
  {   fprintf(stderr, "%s() : CreeFahsVide failed\n", F_NAME);
      return 0;
  }
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(F, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else if ((connex == 6) || (connex == 18) || (connex == 26))
  {
    if (!ComponentTree3d(F, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  { fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
    return 0;
  }

  A = (int32_t *)malloc(nblabels * sizeof(int32_t));
  T = (int32_t *)calloc(nblabels, sizeof(int32_t));
  if ((A == NULL) || (T == NULL))
  {   fprintf(stderr, "%s() : malloc failed\n", F_NAME);
      return(0);
  }
  for (i = 0; i < nblabels; i++) A[i] = i;

  // CALCUL DES SURFACES DES MAXIMA

  for (x = 0; x < N; x++)
  {
    i = O[x];
    c = CM[x];
    if ((i != 0) && (T[i] == 0))
    {
      // remonte jusqu'à trouver une bifurcation (ou la racine)
      j = c;
      k = CT->tabnodes[j].father; 
      while ((k != -1) && (CT->tabnodes[k].nbsons < 2)) 
      {
	j = k; 
	k = CT->tabnodes[k].father; 
      }
      // on prend la surface de l'avant-dernière composante
      T[i] = CT->tabnodes[j].area; 
#ifdef DEBUG
      printf("max %d : val %d\n", i, T[i]);
#endif
    }
  } // for (x = 0; x < N; x++)

  // CLASSEMENT DES MAXIMA

  i_TriRapideStochastique (A, T, 1, nblabels-1);

  for (i = 1; i < nblabels; i++)
    T[A[i]] = i;
  for (i = 0; i < N; i++) 
    if (O[i])
      O[i] = T[O[i]];

  FahsTermine(FAHS);
  ComponentTreeFree(CT);
  free(CM);
  free(A);
  free(T);
  return 1;
} // lordermaximasurf()

/* ==================================== */
int32_t lordermaximavol(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t nblabels)
/* ==================================== */
/*! \fn int32_t lordermaxima(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t nblabels)
    \param image (entrée) : une image
    \param order (sortie) : labels définissant les maxima (de 1 à nbmaxima, 0 pour les non-maxima)
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D) 
    \param nblabels (entrée) : nombre de maxima plus 1
    \return code erreur : 0 si échec, 1 sinon
    \brief ordonne les labels sur les maxima par volume croissant
*/
#undef F_NAME
#define F_NAME "lordermaximavol"
{
  int32_t c, i, j, k, x;
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *O = SLONGDATA(order);      /* l'image de labels */
  int32_t *A; // table de correspondance pour le tri
  int32_t *T; // table avec l'altitude de chaque maximum
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t *CM;                      /* etat d'un pixel */
  ctree * CT;                   /* resultat : l'arbre des composantes */
  
  if ((rowsize(order) != rs) || (colsize(order) != cs) || (depth(order) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  FAHS = CreeFahsVide(N);
  if (FAHS == NULL)
  {   fprintf(stderr, "%s() : CreeFahsVide failed\n", F_NAME);
      return 0;
  }
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(F, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else if ((connex == 6) || (connex == 18) || (connex == 26))
  {
    if (!ComponentTree3d(F, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  { fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
    return 0;
  }

  A = (int32_t *)malloc(nblabels * sizeof(int32_t));
  T = (int32_t *)calloc(nblabels, sizeof(int32_t));
  if ((A == NULL) || (T == NULL))
  {   fprintf(stderr, "%s() : malloc failed\n", F_NAME);
      return(0);
  }
  for (i = 0; i < nblabels; i++) A[i] = i;

  // CALCUL DES VOLUMES DES MAXIMA

  for (x = 0; x < N; x++)
  {
    i = O[x];
    c = CM[x];
    if ((i != 0) && (T[i] == 0))
    {
      // remonte jusqu'à trouver une bifurcation (ou la racine)
      j = c;
      k = CT->tabnodes[j].father; 
      while ((k != -1) && (CT->tabnodes[k].nbsons < 2)) 
      {
	j = k; 
	k = CT->tabnodes[k].father; 
      }
      // on prend la volace de l'avant-dernière composante
      T[i] = CT->tabnodes[j].vol; 
#ifdef DEBUG
      printf("max %d : val %d\n", i, T[i]);
#endif
    }
  } // for (x = 0; x < N; x++)

  // CLASSEMENT DES MAXIMA

  i_TriRapideStochastique (A, T, 1, nblabels-1);

  for (i = 1; i < nblabels; i++)
    T[A[i]] = i;
  for (i = 0; i < N; i++) 
    if (O[i])
      O[i] = T[O[i]];

  FahsTermine(FAHS);
  ComponentTreeFree(CT);
  free(CM);
  free(A);
  free(T);
  return 1;
} // lordermaximavol()

/* ==================================== */
int32_t ldynamique_ldynamique(struct xvimage *image, struct xvimage *order, int32_t connex)
/* ==================================== */
/*! \fn int32_t ldynamique_ldynamique(struct xvimage *image, struct xvimage *order, int32_t connex)
    \param image (entrée/sortie) : une image
    \param order (entrée) : labels définissant un ordre sur les maxima (de 1 à nbmaxima)
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D) 
    \return code erreur : 0 si échec, 1 sinon
    \brief calcule la dynamique ordonnée des maxima
*/
#undef F_NAME
#define F_NAME "ldynamique_ldynamique"
{
  register int32_t i, j, k, x;      /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *O = SLONGDATA(order);      /* l'image de labels */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t *CM;                      /* etat d'un pixel */
  ctree * CT;                   /* resultat : l'arbre des composantes */
  int32_t *mu;                      /* pour représenter l'ordre */
  int32_t *alpha;                   /* pour certains calculs intermédiaires */
  int32_t *dyn;                     /* pour représenter la dynamique */
  
  if ((rowsize(order) != rs) || (colsize(order) != cs) || (depth(order) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  FAHS = CreeFahsVide(N);
  if (FAHS == NULL)
  {   fprintf(stderr, "%s() : CreeFahsVide failed\n", F_NAME);
      return 0;
  }
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(F, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else if ((connex == 6) || (connex == 18) || (connex == 26))
  {
    if (!ComponentTree3d(F, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  { fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
    return 0;
  }

#ifdef DEBUG
  ComponentTreePrint(CT);
  writeimage(order, "_o");
#endif

  mu = (int32_t *)calloc(CT->nbleafs+1, sizeof(int32_t)); // 0 non utilise
  dyn = (int32_t *)calloc(CT->nbleafs+1, sizeof(int32_t)); // 0 non utilise
  alpha = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  if ((mu == NULL) || (dyn == NULL) || (alpha == NULL)) 
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return 0;
  }

  for (x = 0; x < N; x++)
  {
    k = CM[x];
    i = O[x]; // label donnant l'ordre
    if (CT->tabnodes[k].nbsons == 0) // feuille
      mu[i] = k; 
  } // for (x = 0; x < N; x++)

  for (i = CT->nbleafs; i >= 1; i--)
  {
    j = k = mu[i];
#ifdef DEBUG
    printf("node %d : %d ", i, k);
#endif
    while ((k != -1) && (alpha[k] == 0))
    {
      alpha[k] = i;
      k = CT->tabnodes[k].father;
    } 
    if (k == -1)
      dyn[i] = 255;
    else
      dyn[i] = CT->tabnodes[j].data - CT->tabnodes[k].data;
#ifdef DEBUG
    printf("  ancestor %d, level %d\n", k, dyn[i]);
#endif
  } // for (i = CT->nbleafs; i >= 1; i--)

  for (x = 0; x < N; x++)
    if (O[x]) F[x] = dyn[O[x]]; else F[x] = 0;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  ComponentTreeFree(CT);
  free(CM);
  free(mu);
  free(dyn);
  free(alpha);
  return(1);
} /* ldynamique_ldynamique() */

/* ==================================== */
int32_t lfiltredynamique(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t seuil)
/* ==================================== */
/*! \fn int32_t lfiltredynamique(struct xvimage *image, struct xvimage *order, int32_t connex, int32_t seuil)
    \param image (entrée/sortie) : une image
    \param order (entrée) : labels définissant un ordre sur les maxima (de 1 à nbmaxima)
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D) 
    \param seuil (entrée) : pour le filtrage
    \return code erreur : 0 si échec, 1 sinon
    \brief filtre les composantes selon la dynamique ordonnée des maxima
*/
#undef F_NAME
#define F_NAME "lfiltredynamique"
{
  register int32_t i, j, k, x;      /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *O = SLONGDATA(order);      /* l'image de labels */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t *CM;                      /* etat d'un pixel */
  ctree * CT;                   /* resultat : l'arbre des composantes */
  int32_t *mu;                      /* pour représenter l'ordre */
  int32_t *alpha;                   /* pour certains calculs intermédiaires */
  int32_t *dyn;                     /* pour représenter la dynamique */
  int32_t noleaf;

  if ((rowsize(order) != rs) || (colsize(order) != cs) || (depth(order) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  FAHS = CreeFahsVide(N);
  if (FAHS == NULL)
  {   fprintf(stderr, "%s() : CreeFahsVide failed\n", F_NAME);
      return 0;
  }
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(F, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else if ((connex == 6) || (connex == 18) || (connex == 26))
  {
    if (!ComponentTree3d(F, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  { fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
    return 0;
  }

#ifdef DEBUG
  ComponentTreePrint(CT);
#endif

  mu = (int32_t *)calloc(CT->nbleafs+1, sizeof(int32_t)); // 0 non utilise
  dyn = (int32_t *)calloc(CT->nbleafs+1, sizeof(int32_t)); // 0 non utilise
  alpha = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  if ((mu == NULL) || (dyn == NULL) || (alpha == NULL)) 
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return 0;
  }

  for (x = 0; x < N; x++)
  {
    k = CM[x];
    i = O[x]; // label donnant l'ordre
    if (CT->tabnodes[k].nbsons == 0) // feuille
      mu[i] = k; 
  } // for (x = 0; x < N; x++)

  for (i = CT->nbleafs; i >= 1; i--)
  {
    j = k = mu[i];
#ifdef DEBUG
    printf("node %d : %d ", i, k);
#endif
    while ((k != -1) && (alpha[k] == 0))
    {
      alpha[k] = i;
      k = CT->tabnodes[k].father;
    } 
    if (k == -1)
      dyn[i] = 255;
    else
      dyn[i] = CT->tabnodes[j].data - CT->tabnodes[k].data;
#ifdef DEBUG
    printf("  ancestor %d, level %d\n", k, dyn[i]);
#endif
  } // for (i = CT->nbleafs; i >= 1; i--)

  noleaf = CT->nbleafs + 1; // marqueur 
  memset(alpha, 0, CT->nbnodes * sizeof(int32_t));
  for (i = CT->nbleafs; i >= 1; i--)
  {
    if (dyn[i] >= seuil)
    {
      k = mu[i];
      while ((k != -1) && (alpha[k] == 0))
      {
	alpha[k] = i;
	k = CT->tabnodes[k].father;
      }
      while (k != -1) 
      {
	alpha[k] = noleaf;
	k = CT->tabnodes[k].father;
      }
    }
  } // for (i = CT->nbleafs; i >= 1; i--)
  
  for (x = 0; x < N; x++)
  {
    k = CM[x];
    while ((k != -1) && (alpha[k] == 0))
      k = CT->tabnodes[k].father;
    if (k == -1) break;
    i = alpha[k];
    if (i == noleaf) F[x] = 0; else F[x] = dyn[i]; 
  }
  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  ComponentTreeFree(CT);
  free(CM);
  free(mu);
  free(dyn);
  free(alpha);
  return(1);
} /* lfiltredynamique() */
