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

   Operateurs : 
     ldynamique_grimaud_ldynamique (d'apres Grimaud et L. Najman)
     lwshedtopo (d'apres MC, GB)
     lwshedval (d'apres LN, MC)

   Michel Couprie - 2002 - 2003
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcfahsalembier.h>
#include <mcutil.h>
#include <mcindic.h>
#include <ldynamique_grimaud.h>

#define PARANO
#define VERBOSE
//#define ANIMATE

//#define DEBUG
//#define DEBUGRECONS
//#define DEBUGFLOOD


#define ATTR_DYN

#include "lattrib.h"

/* ======================================================================== */
/* ======================================================================== */
/* OPERATEURS BASES SUR L'ARBRE DES COMPOSANTES */
/* ======================================================================== */
/* ======================================================================== */

/* ==================================== */
static void CalculeDynamiqueMaxima(CompactTree * cpct)
/* ==================================== */
/* 
Definition Grimaud : obligation d'atteindre un maximum de niveau strictement superieur
*/
{
  uint32_t nbcomp = cpct->nbcomp;
  int32_t i, h, ncompnivh, f, p;
  uint8_t *tmp;

  tmp = (uint8_t *)calloc(nbcomp, sizeof(char));
  if (tmp == NULL) 
  {   fprintf(stderr, "CalculeDynamiqueMaxima() : calloc failed for tmp\n");
      exit(0);
  }

  /* met le champ dyn a 0 */
  for (i = 0; i < nbcomp; i++)  cpct->dyn[i] = 0;

  /* balaye les feuilles par ordre decroissant */
  for (h = 255; h > 0; h--)
  {
    ncompnivh = cpct->hc[h] - cpct->hc[h-1]; /* nb composantes dans le niveau h */
    if (ncompnivh > 0)
    {
      for (i = cpct->hc[h] - 1; i > cpct->hc[h] - 1 - ncompnivh; i--)
      {
        if (NBFILS(i) == 0) /* feuille */
        {
          f = i;
          p = cpct->pere[f];
          /* 
            on remonte la branche a partir de la feuille i
            en marquant les noeuds (grace a tmp) par le niveau de i,
            jusqu'a trouver un noeud p marque (tmp) par une valeur plus forte
            ou la racine
            alors : dyn[i] = niv[i] - niv[p]
	  */
          while ((p != f) && (tmp[p] <= h))
	  {
            tmp[p] = h;
            f = p;
            p = cpct->pere[f];
	  } /* while ((p != f) && (tmp[p] <= h)) */
          cpct->dyn[i] = h - DECODENIV(cpct->comp[p]);
	} /* if (NBFILS(i) == 0) */
      } /* for i */
    } /* if (ncompnivh > 0) */
  } /* for h */

#ifdef VERBOSE
  printf("FIN DU CALCUL DE LA DYNAMIQUE DES MAXIMA\n");
#endif

  free(tmp);
} /* CalculeDynamiqueMaxima() */

#ifdef NOT_USED
/* ==================================== */
static void CalculeDynamiqueContours(CompactTree * cpct)
/* ==================================== */
/* 
Variante definition Grimaud : obligation d'atteindre un maximum de niveau superieur ou egal
*/
#define CONDITION_VIOLATION 0x01
{
  uint32_t nbcomp = cpct->nbcomp;
  int32_t i, h, ncompnivh, f, p, v;
  uint8_t *tmp;

  tmp = (uint8_t *)calloc(nbcomp, sizeof(char));
  if (tmp == NULL) 
  {   fprintf(stderr, "CalculeDynamiqueContours() : calloc failed for tmp\n");
      exit(0);
  }

  /* met le champ dyn a 0 */
  for (i = 0; i < nbcomp; i++)  cpct->dyn[i] = 0;

  /* balaye les feuilles par ordre decroissant */
  for (h = 255; h > 0; h--)
  {
    ncompnivh = cpct->hc[h] - cpct->hc[h-1]; /* nb composantes dans le niveau h */
    if (ncompnivh > 0)
    {
      for (i = cpct->hc[h] - 1; i > cpct->hc[h] - 1 - ncompnivh; i--)
      {
        if (NBFILS(i) == 0) /* feuille */
        {
          f = i;
          p = cpct->pere[f];
          /* 
            on remonte la branche a partir de la feuille i
            en marquant les noeuds (grace a tmp) par le niveau de i,
            jusqu'a trouver un noeud p marque (tmp) par une valeur plus forte ou egale
            ou la racine
            alors : dyn[i] = niv[i] - niv[p]
            et (si non racine) : dyn[p] = mcmax(dyn[p],dyn[i])
               (si racine) : -1 la premiere fois, idem ensuite
	  */
          while ((p != f) && (tmp[p] < h))
	  {
            tmp[p] = h;
            f = p;
            p = cpct->pere[f];
	  } /* while ((p != f) && (tmp[p] < h)) */
          cpct->dyn[i] = h - DECODENIV(cpct->comp[p]);
          if (p != f) cpct->dyn[p] = mcmax(cpct->dyn[p],cpct->dyn[i]);
          else
	  {
            if (cpct->dyn[p] == 0) cpct->dyn[p] = -1;
            else cpct->dyn[p] = mcmax(cpct->dyn[p],cpct->dyn[i]);
	  }
	} /* if (NBFILS(i) == 0) */
      } /* for i */
    } /* if (ncompnivh > 0) */
  } /* for h */

  // detection des noeuds i violant la condition : 
  //   val(i) >= val(p) pour tout p descendant de i

  // balaye les noeuds non feuilles par ordre de niveau decroissant.
  // a partir d'un noeud i (de valuation v)
  // on remonte vers la racine: 
  //   soit p le noeud courant
  //   si val(p) < v alors marque p (flag CONDITION_VIOLATION)
  //   sinon v = val(p) pour continuer la verification sur les ancetres de i
  for (h = 255; h > 0; h--)
  {
    ncompnivh = cpct->hc[h] - cpct->hc[h-1]; /* nb composantes dans le niveau h */
    if (ncompnivh > 0)
    {
      for (i = cpct->hc[h] - 1; i > cpct->hc[h] - 1 - ncompnivh; i--)
      {
        if (NBFILS(i) > 0) /* non feuille */
        {
          v = cpct->dyn[i];
          f = i;
          p = cpct->pere[f];
          while (p != f)
	  {
            if (cpct->dyn[p] < v) cpct->flags[p] |= CONDITION_VIOLATION;
            else v = cpct->dyn[p];
            f = p;
            p = cpct->pere[f];
	  } /* while (p != f) */
	} /* if (NBFILS(i) > 0) */
      } /* for i */
    } /* if (ncompnivh > 0) */
  } /* for h */

#ifdef VERBOSE
  printf("FIN DU CALCUL DE LA DYNAMIQUE DES CONTOURS\n");
#endif

  free(tmp);
} /* CalculeDynamiqueContours() */
#endif

/* ==================================== */
static void RecupereDynamique(CompactTree * cpct,           
       uint32_t *STATUS,
       int32_t rs, int32_t N, 
       uint8_t *ORI            /* informations sur l'image originale */
)
/* ==================================== */
{
  int32_t i, h;
  uint32_t c, comp;
  for (i = 0; i < N; i++) 
  {
    h = ORI[i];
    c = STATUS[i];
    comp = INDEXCOMP(h,c);
    ORI[i] = cpct->dyn[comp];
  }  
} /* RecupereDynamique() */

/* ==================================== */
int32_t ldynamique_grimaud_ldynamique(struct xvimage *image, int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldynamique_grimaud_ldynamique"
{
  register int32_t i, k;         /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
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

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)calloc(1,N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "%s : malloc failed for STATUS\n", F_NAME);
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "%s : calloc failed for number_nodes\n", F_NAME);
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "%s : calloc failed for node_at_level\n", F_NAME);
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "%s : InitCompTree failed\n", F_NAME);
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

  if ((connex == 4) || (connex == 8))
    (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
  else
    (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);

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

  CalculeDynamiqueMaxima(CTREE);

#ifdef VERBOSE
  fprintf(stderr, "CalculeDynamique terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(CTREE);
  AfficheImaComp(CTREE, STATUS, rs, N, F);
#endif

  RecupereDynamique(CTREE, STATUS, rs, N, F);

#ifdef VERBOSE
  fprintf(stderr, "RecupereDynamique terminee\n");
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
} /* ldynamique_grimaud_ldynamique() */

/* ==================================== */
static void SimplifyComp(CompactTree *cpct, int32_t *ncomp, int32_t *tabcomp) 
/* ==================================== */
// supprime dans tabcomp les composantes qui sont ancetres d'une autre
{
  int32_t k, j, i; 
  for (i = 0; i < *ncomp; i++)
  if (tabcomp[i] != -1)
  {
    for (j = i + 1; j < *ncomp; j++)
    if (tabcomp[j] != -1)
    {
      if (Ancestor(cpct, tabcomp[j], tabcomp[i])) 
      {
        tabcomp[j] = -1;
      }
      else if (Ancestor(cpct, tabcomp[i], tabcomp[j])) 
      {
        tabcomp[i] = -1;
        break;
      }
    } // for j
  } // for i
  k = j = 0;
  for (i = 0; i < *ncomp; i++)
  {
    tabcomp[j] = tabcomp[i];
    if (tabcomp[i] != -1)
      j++;
    else
      k++; // nb elements supprimes
  }
  *ncomp -= k;
} /* SimplifyComp() */

/* ================================================ */
static void BuildTree(uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex, int32_t incr_vois,
	       Fahs * FAHS, uint32_t *STATUS, 
               uint32_t *number_nodes, uint8_t *node_at_level,
               CompTree * TREE, CompactTree ** cpct
              )
/* ================================================ */
{
  int32_t i, k;

  // INITIALISATIONS
  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 0; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);
  // APPEL FONCTION RECURSIVE flood
  if ((connex == 4) || (connex == 8))
    (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
  else
    (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);
  *cpct = CompTree2CompactTree(TREE, number_nodes);
} // BuildTree()

/* ==================================== */
static int32_t TrouveComposantes(int32_t x, uint8_t *F, int32_t rs, int32_t N, int32_t incr_vois, 
                  uint32_t *STATUS, CompactTree * cpct, int32_t *tabcomp)
/* ==================================== */
{
    int32_t k, y, ncomp = 0;
    for (k = 0; k < 8; k += incr_vois) // parcourt les c-voisins y de x d'un niveau > F[x]
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (F[y] > F[x]))
      {
        tabcomp[ncomp] = INDEXCOMP(F[y],STATUS[y]);
        ncomp++;
      }
    } /* for (k = 0; k < 8; k += incr_vois) */
#ifdef DEBUG
    printf("    Comp. du voisinage: %d [%d %d %d %d]\n", ncomp, tabcomp[0], tabcomp[1], tabcomp[2], tabcomp[3]);
#endif
    SimplifyComp(cpct, &ncomp, tabcomp); // supprime dans tabcomp les composantes qui sont ancetres d'une autre

#ifdef DEBUG
    printf("    Comp. du voisinage (simpl.): %d [%d %d %d %d]\n", ncomp, tabcomp[0], tabcomp[1], tabcomp[2], tabcomp[3]);
#endif
    return ncomp;
} // TrouveComposantes() 

/* ==================================== */
static int32_t TrouveComposantes2(int32_t x, uint8_t *F, int32_t rs, int32_t N, int32_t incr_vois, 
                  uint32_t *STATUS, CompactTree * cpct, int32_t *tabcomp)
/* ==================================== */
// variante sans simplification 
// place la plus haute composante (ou l'une des plus hautes) en premier
{
  int32_t k, y, n = 1, maxval = F[x], first = 1;
  for (k = 0; k < 8; k += incr_vois) // parcourt les c-voisins y de x d'un niveau > F[x]
  {
    y = voisin(x, k, rs, N);
    if ((y != -1) && (F[y] > maxval)) maxval = F[y];
  } /* for (k = 0; k < 8; k += incr_vois) */

  if (maxval == F[x]) 
  {
#ifdef DEBUG
    printf("    Pas de Comp. dans le voisinage\n");
#endif
    return 0;
  }

  for (k = 0; k < 8; k += incr_vois) // parcourt les c-voisins y de x d'un niveau > F[x]
  {
    y = voisin(x, k, rs, N);
    if ((y != -1) && (F[y] > F[x]))
    {
      if (first && (F[y] == maxval)) { tabcomp[0] = INDEXCOMP(F[y],STATUS[y]); first = 0; }
      else                           { tabcomp[n] = INDEXCOMP(F[y],STATUS[y]); n++; }
    }
  } /* for (k = 0; k < 8; k += incr_vois) */
#ifdef DEBUG
    printf("    Comp. du voisinage: %d [%d %d %d %d]\n", n, tabcomp[0], tabcomp[1], tabcomp[2], tabcomp[3]);
#endif

    return n;
} // TrouveComposantes2() 

#define EN_FAHS     0 
#define WATERSHED  1
#define MASSIF     2
#define WATERSHED2 3
#define MODIFIE    4

/* ==================================== */
void Watershed(
  struct xvimage *image,
  int32_t incr_vois, 
  Fahs * FAHS,
  uint32_t *STATUS, 
  CompactTree * cpct)
/* ==================================== */
// nouvel algo en deux temps: 
// 1. propagation des minima en mode "inondation"
// 2. saturation
#undef F_NAME
#define F_NAME "Watershed"
{
  uint8_t *F = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t N = rs * colsize(image);
  int32_t i, j, k, x, y;
  int32_t c;                        /* une composante */
  int32_t tabcomp[8];               /* liste de composantes */
  int32_t ncomp;                    /* nombre de composantes dans tabcomp */
  int32_t nbelev;                   /* nombre d'elevations effectuees */
  Lifo * LIFO;                  /* liste d'attente pour les points de watershed */
  LIFO = CreeLifoVide(N);

  // INITIALISATIONS
  FahsFlush(FAHS); // Re-initialise la FAHS

  // etiquetage des c-maxima (doit pouvoir se faire au vol lors de la construction de l'arbre)
  for (i = 0; i < N; i++)
  {
    c = INDEXCOMP(F[i],STATUS[i]);
    if (NBFILS(c) == 0) Set(i,MASSIF);
  } // for (i = 0; i < N; i++)

  // empile les c-voisins des c-maxima
  for (i = 0; i < N; i++)
  {
    if (IsSet(i,MASSIF))
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && (!IsSet(j,MASSIF)) && (!IsSet(j,EN_FAHS)))
	{
          Set(j,EN_FAHS);
          FahsPush(FAHS, j, NDG_MAX - F[j]);
	}
      } /* for (k = 0; k < 8; k += incr_vois) */
    } // if (IsSet(i,MASSIF))
  } // for (i = 0; i < N; i++)

  // ******************************* BOUCLE 1
  nbelev = 0;
  while (!FahsVide(FAHS))
  {
    x = FahsPop(FAHS);
    UnSet(x,EN_FAHS);
#ifdef DEBUG
    printf("Pop Point %d,%d Niveau %d\n", x % rs, x / rs, F[x]);
#endif

    ncomp = TrouveComposantes(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);

    if (ncomp == 1)
      c = tabcomp[0];
    else 
      c = LowestCommonAncestor(cpct, ncomp, tabcomp, F[x]);

#ifdef DEBUG
    printf("    LCA: %d\n", c);
#endif

    if ((c == -1) && (ncomp >= 2))
    {
      Set(x,WATERSHED);
#ifdef DEBUG
      printf("    Pas d'elevation ; LPE\n");
#endif
    }

    if (c != -1)
    {
      if (NBFILS(c) == 0) // feuille
      {
        nbelev++;
        F[x] = DECODENIV(cpct->comp[c]);      // eleve le niveau du point x
        STATUS[x] = DECODENUM(cpct->comp[c]); // maj pointeur image -> composantes 
        Set(x,MODIFIE);
        Set(x,MASSIF);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; MASSIF\n", F[x]);
#endif
        // empile les c-voisins de x non marques MASSIF ni WATERSHED ni EN_FAHS
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,WATERSHED)) && (!IsSet(y,EN_FAHS)))
          {
            Set(y,EN_FAHS);
            FahsPush(FAHS, y, NDG_MAX - F[y]);
#ifdef DEBUG
            printf("        Push Point %d,%d Niveau %d\n", y % rs, y / rs, F[y]);
#endif
          }
        } // for (k = 0; k < 8; k += incr_vois)
      } // if feuille
      else 
      {
        Set(x,EN_FAHS);
        LifoPush(LIFO, x);
      }
    } // if (c != -1)
  } // while (!FahsVide(FAHS))

#ifdef VERBOSE
  printf("Nombre d'elevations en premiere passe %d\n", nbelev);
#endif

  // ******************************* BOUCLE 2
  while (!LifoVide(LIFO))
  {
    x = LifoPop(LIFO);
    UnSet(x,EN_FAHS);
#ifdef DEBUG
    printf("Pop Point %d,%d Niveau %d\n", x % rs, x / rs, F[x]);
#endif

    ncomp = TrouveComposantes(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);

    if (ncomp == 1)
      c = tabcomp[0];
    else 
      c = LowestCommonAncestor(cpct, ncomp, tabcomp, F[x]);

#ifdef DEBUG
    printf("    LCA: %d\n", c);
#endif

    if ((c == -1) && (ncomp >= 2))
    {
      Set(x,WATERSHED);
#ifdef DEBUG
      printf("    Pas d'elevation ; LPE\n");
#endif
    }

    if (c != -1)
    {
      nbelev++; k = F[x];
      F[x] = DECODENIV(cpct->comp[c]);      // eleve le niveau du point x
      STATUS[x] = DECODENUM(cpct->comp[c]); // maj pointeur image -> composantes 
#ifdef DEBUG
      printf("    Eleve au niveau: %d\n", F[x]);
#endif
#ifdef PARANO
      if (IsSet(x,MODIFIE))
      {
        printf("ERREUR: POINT MODIFIE 2 FOIS EN PASSE 2!!!!\n");
        printf("avant: %d ; apres: %d\n", k, F[x]);
      }
#endif
      Set(x,MODIFIE);

#ifdef PARANO
      if (NBFILS(c) == 0) // feuille
        printf("ERREUR: POINT MASSIF TROUVE EN PASSE 2!!!!\n");
#endif

      // empile les c-voisins de x non marques MASSIF ni WATERSHED ni EN_FAHS
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,WATERSHED)) && (!IsSet(y,EN_FAHS)))
        {
          Set(y,EN_FAHS);
          LifoPush(LIFO, y);
#ifdef DEBUG
          printf("        Push Point %d,%d Niveau %d\n", y % rs, y / rs, F[y]);
#endif
        }
      } // for (k = 0; k < 8; k += incr_vois)
    } // if (c != -1)
  } // while (!LifoVide(Lifo))

#ifdef VERBOSE
  printf("Nombre d'elevations total %d\n", nbelev);
#endif
  LifoTermine(LIFO);
} // Watershed()

#ifdef NOT_USED
/* ==================================== */
static void Watershed1(struct xvimage *image, int32_t incr_vois,
	      Fahs * FAHS, uint32_t *STATUS, CompactTree * cpct)
/* ==================================== */
// propage a partir d'un point constructible
#undef F_NAME
#define F_NAME "Watershed1"
{
  uint8_t *F = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t N = rs * colsize(image);
  int32_t k, x, y;
  int32_t c;                        /* une composante */
  int32_t tabcomp[8];               /* liste de composantes */
  int32_t ncomp;                    /* nombre de composantes dans tabcomp */
  int32_t nbelev;                   /* nombre d'elevations effectuees */

  // INITIALISATIONS
  FahsFlush(FAHS); // Re-initialise la FAHS

  // cherche un point constructible
  for (x = 0; x < N; x++)
  {
    ncomp = TrouveComposantes(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);
    if (ncomp == 1)
      c = tabcomp[0];
    else 
      c = LowestCommonAncestor(cpct, ncomp, tabcomp, F[x]);
    if (c != -1)
    {
      Set(x,EN_FAHS);
      FahsPush(FAHS, x, NDG_MAX - F[x]);
      break;
    }
  } // for (x = 0; x < N; x++)

  // BOUCLE PRINCIPALE
  nbelev = 0;
  while (!FahsVide(FAHS))
  {
    x = FahsPop(FAHS);
    UnSet(x,EN_FAHS);
#ifdef DEBUG
    printf("Pop Point %d,%d Niveau %d\n", x % rs, x / rs, F[x]);
#endif

    ncomp = TrouveComposantes(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);

    if (ncomp == 1)
      c = tabcomp[0];
    else 
      c = LowestCommonAncestor(cpct, ncomp, tabcomp, F[x]);

#ifdef DEBUG
    printf("    LCA: %d\n", c);
#endif

    if ((c == -1) && (ncomp >= 2))
    {
      Set(x,WATERSHED);
#ifdef DEBUG
      printf("    Pas d'elevation ; LPE\n");
#endif
    }

    if (c != -1)
    {
      nbelev++;
      F[x] = DECODENIV(cpct->comp[c]);      // eleve le niveau du point x
      STATUS[x] = DECODENUM(cpct->comp[c]); // maj pointeur image -> composantes 
      Set(x,MODIFIE);
      if (NBFILS(c) == 0) // feuille
      {
        Set(x,MASSIF);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; MASSIF\n", F[x]);
#endif
      } // if feuille
      else
      if (NBFILS(c) > 1) // noeud
      {
        Set(x,WATERSHED);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; LPE\n", F[x]);
#endif
      }
#ifdef PARANO
      else
        printf("%s : ERREUR COMPOSANTE BRANCHE!!!\n", F_NAME);
#endif

      // empile les c-voisins de x non marques MASSIF ni EN_FAHS
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
        {
          Set(y,EN_FAHS);
          FahsPush(FAHS, y, NDG_MAX - F[y]);
#ifdef DEBUG
          printf("        Push Point %d,%d Niveau %d\n", y % rs, y / rs, F[y]);
#endif
        }
      } // for (k = 0; k < 8; k += incr_vois)
    } // if (c != -1)
  } // while (!FahsVide(FAHS))

#ifdef VERBOSE
    printf("Nombre d'elevations %d\n", nbelev);
#endif
} // Watershed1()
#endif

/* ==================================== */
static void Watershed2(struct xvimage *image, int32_t incr_vois,
	      Fahs * FAHS, uint32_t *STATUS, CompactTree * cpct)
/* ==================================== */
// inondation a partir des voisins des maxima, suivant les ndg decroissants
#undef F_NAME
#define F_NAME "Watershed2"
{
  uint8_t *F = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t N = rs * colsize(image);
  int32_t i, j, k, x, y;
  int32_t c;                        /* une composante */
  int32_t tabcomp[8];               /* liste de composantes */
  int32_t ncomp;                    /* nombre de composantes dans tabcomp */
  int32_t nbelev;                   /* nombre d'elevations effectuees */
#ifdef ANIMATE
  int32_t curlev = NDG_MAX+1, nimage = 0; 
  char imname[128];
#endif

  // INITIALISATIONS
  FahsFlush(FAHS); // Re-initialise la FAHS

  // etiquetage des c-maxima (doit pouvoir se faire au vol lors de la construction de l'arbre)
  for (i = 0; i < N; i++)
  {
    c = INDEXCOMP(F[i],STATUS[i]);
    if (NBFILS(c) == 0) Set(i,MASSIF);
  } // for (i = 0; i < N; i++)

  // empile les c-voisins des c-maxima
  for (i = 0; i < N; i++)
  {
    if (IsSet(i,MASSIF))
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && (!IsSet(j,MASSIF)) && (!IsSet(j,EN_FAHS)))
	{
          Set(j,EN_FAHS);
          FahsPush(FAHS, j, NDG_MAX - F[j]);
	}
      } /* for (k = 0; k < 8; k += incr_vois) */
    } // if (IsSet(i,MASSIF))
  } // for (i = 0; i < N; i++)

  // BOUCLE PRINCIPALE
  nbelev = 0;
  while (!FahsVide(FAHS))
  {
    x = FahsPop(FAHS);
    UnSet(x,EN_FAHS);
#ifdef DEBUG
    printf("Pop Point %d,%d Niveau %d\n", x % rs, x / rs, F[x]);
#endif
#ifdef ANIMATE
    if (F[x] < curlev)
    {
      printf("Niveau %d\n", F[x]);
      sprintf(imname, "anim%03d.pgm", nimage); nimage++;
      writeimage(image, imname);
      curlev = F[x];
    }
#endif
    ncomp = TrouveComposantes(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);

    if (ncomp == 1)
      c = tabcomp[0];
    else 
      c = LowestCommonAncestor(cpct, ncomp, tabcomp, F[x]);

#ifdef DEBUG
    printf("    LCA: %d\n", c);
#endif

    if ((c == -1) && (ncomp >= 2))
    {
      Set(x,WATERSHED);
#ifdef DEBUG
      printf("    Pas d'elevation ; LPE\n");
#endif
    }

    if (c != -1)
    {
      nbelev++;
      F[x] = DECODENIV(cpct->comp[c]);      // eleve le niveau du point x
      STATUS[x] = DECODENUM(cpct->comp[c]); // maj pointeur image -> composantes 
      Set(x,MODIFIE);
      if (NBFILS(c) == 0) // feuille
      {
        Set(x,MASSIF);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; MASSIF\n", F[x]);
#endif
      } // if feuille
      else
      if (NBFILS(c) > 1) // noeud
      {
        Set(x,WATERSHED);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; LPE\n", F[x]);
#endif
      }
#ifdef PARANO
      else
        printf("%s : ERREUR COMPOSANTE BRANCHE!!!\n", F_NAME);
#endif

      // empile les c-voisins de x non marques MASSIF ni EN_FAHS
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
        {
          Set(y,EN_FAHS);
          FahsPush(FAHS, y, NDG_MAX - F[y]);
#ifdef DEBUG
          printf("        Push Point %d,%d Niveau %d\n", y % rs, y / rs, F[y]);
#endif
        }
      } // for (k = 0; k < 8; k += incr_vois)
    } // if (c != -1)
  } // while (!FahsVide(FAHS))

#ifdef VERBOSE
    printf("Nombre d'elevations %d\n", nbelev);
#endif
} // Watershed2()

#ifdef NOT_USED
/* ==================================== */
static void Watershed3(struct xvimage *image, int32_t incr_vois,
	      Fahs * FAHS, uint32_t *STATUS, CompactTree * cpct)
/* ==================================== */
// propagation en largeur a partir des voisins des maxima
#undef F_NAME
#define F_NAME "Watershed3"
{
  uint8_t *F = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t N = rs * colsize(image);
  int32_t i, j, k, x, y;
  int32_t c;                        /* une composante */
  int32_t tabcomp[8];               /* liste de composantes */
  int32_t ncomp;                    /* nombre de composantes dans tabcomp */
  int32_t nbelev;                   /* nombre d'elevations effectuees */
#ifdef ANIMATE
  int32_t curlev = NDG_MAX+1, nimage = 0; 
  char imname[128];
#endif

  // INITIALISATIONS
  FahsFlush(FAHS); // Re-initialise la FAHS

  // etiquetage des c-maxima (doit pouvoir se faire au vol lors de la construction de l'arbre)
  for (i = 0; i < N; i++)
  {
    c = INDEXCOMP(F[i],STATUS[i]);
    if (NBFILS(c) == 0) Set(i,MASSIF);
  } // for (i = 0; i < N; i++)

  // empile les c-voisins des c-maxima
  for (i = 0; i < N; i++)
  {
    if (IsSet(i,MASSIF))
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && (!IsSet(j,MASSIF)) && (!IsSet(j,EN_FAHS)))
	{
          Set(j,EN_FAHS);
          FahsPush(FAHS, j, 0);
	}
      } /* for (k = 0; k < 8; k += incr_vois) */
    } // if (IsSet(i,MASSIF))
  } // for (i = 0; i < N; i++)

  // BOUCLE PRINCIPALE
  nbelev = 0;
  while (!FahsVide(FAHS))
  {
    x = FahsPop(FAHS);
    UnSet(x,EN_FAHS);
#ifdef DEBUG
    printf("Pop Point %d,%d Niveau %d\n", x % rs, x / rs, F[x]);
#endif
#ifdef ANIMATE
    if (F[x] < curlev)
    {
      printf("Niveau %d\n", F[x]);
      sprintf(imname, "anim%03d.pgm", nimage); nimage++;
      writeimage(image, imname);
      curlev = F[x];
    }
#endif
    ncomp = TrouveComposantes(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);

    if (ncomp == 1)
      c = tabcomp[0];
    else 
      c = LowestCommonAncestor(cpct, ncomp, tabcomp, F[x]);

#ifdef DEBUG
    printf("    LCA: %d\n", c);
#endif

    if ((c == -1) && (ncomp >= 2))
    {
      Set(x,WATERSHED);
#ifdef DEBUG
      printf("    Pas d'elevation ; LPE\n");
#endif
    }

    if (c != -1)
    {
      nbelev++;
      F[x] = DECODENIV(cpct->comp[c]);      // eleve le niveau du point x
      STATUS[x] = DECODENUM(cpct->comp[c]); // maj pointeur image -> composantes 
      Set(x,MODIFIE);
      if (NBFILS(c) == 0) // feuille
      {
        Set(x,MASSIF);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; MASSIF\n", F[x]);
#endif
      } // if feuille
      else
      if (NBFILS(c) > 1) // noeud
      {
        Set(x,WATERSHED);
#ifdef DEBUG
        printf("    Eleve au niveau: %d ; LPE\n", F[x]);
#endif
      }
#ifdef DEBUG
      else
        printf("    Eleve au niveau: %d ; MASSIF\n", F[x]);
#endif

      // empile les c-voisins de x non marques MASSIF ni EN_FAHS
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
        {
          Set(y,EN_FAHS);
          FahsPush(FAHS, y, 0);
#ifdef DEBUG
          printf("        Push Point %d,%d Niveau %d\n", y % rs, y / rs, F[y]);
#endif
        }
      } // for (k = 0; k < 8; k += incr_vois)
    } // if (c != -1)
  } // while (!FahsVide(FAHS))

#ifdef VERBOSE
    printf("Nombre d'elevations %d\n", nbelev);
#endif
} // Watershed3()
#endif

/* ==================================== */
static void Watershed4(struct xvimage *image, int32_t incr_vois,
	      Fahs * FAHS, uint32_t *STATUS, CompactTree * cpct)
/* ==================================== */
// inondation a partir des voisins des maxima, suivant les ndg decroissants
// nouvelle (08/03) caracterisation des points destructibles
#undef F_NAME
#define F_NAME "Watershed4"
{
  uint8_t *F = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t N = rs * colsize(image);
  int32_t i, j, k, x, y;
  int32_t c;                        /* une composante */
  int32_t tabcomp[8];               /* liste de composantes */
  int32_t ncomp;                    /* nombre de composantes dans tabcomp */
  int32_t nbelev;                   /* nombre d'elevations effectuees */
  int32_t lcalevel;                 /* niveau du lca */

  // INITIALISATIONS
  FahsFlush(FAHS); // Re-initialise la FAHS

  // etiquetage des c-maxima (doit pouvoir se faire au vol lors de la construction de l'arbre)
  for (i = 0; i < N; i++)
  {
    c = INDEXCOMP(F[i],STATUS[i]);
    if (NBFILS(c) == 0) Set(i,MASSIF);
  } // for (i = 0; i < N; i++)

  // empile les c-voisins des c-maxima
  for (i = 0; i < N; i++)
  {
    if (IsSet(i,MASSIF))
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && (!IsSet(j,MASSIF)) && (!IsSet(j,EN_FAHS)))
	{
          Set(j,EN_FAHS);
          FahsPush(FAHS, j, NDG_MAX - F[j]);
	}
      } /* for (k = 0; k < 8; k += incr_vois) */
    } // if (IsSet(i,MASSIF))
  } // for (i = 0; i < N; i++)

  // BOUCLE PRINCIPALE
  nbelev = 0;
  while (!FahsVide(FAHS))
  {
    x = FahsPop(FAHS);
    UnSet(x,EN_FAHS);
#ifdef DEBUG
    printf("Pop Point %d,%d Niveau %d\n", x % rs, x / rs, F[x]);
#endif
    ncomp = TrouveComposantes2(x, F, rs, N, incr_vois, STATUS, cpct, tabcomp);

    if (ncomp > 0)
    {
      if (ncomp == 1)
        c = tabcomp[0];
      else 
      {
        int32_t c1;
        c = tabcomp[0];
        for (k = 1; k < ncomp; k++)
        {
          c1 = LowComAnc(cpct, c, tabcomp[k]);
          if (c1 != tabcomp[k]) c = c1;
        }
      }

      lcalevel = DECODENIV(cpct->comp[c]);

#ifdef DEBUG
printf("    LCA: %d ; level: %d\n", c, lcalevel);
#endif
      if (lcalevel > F[x])
      {
        nbelev++;
        F[x] = lcalevel;      // eleve le niveau du point x
        STATUS[x] = DECODENUM(cpct->comp[c]); // maj pointeur image -> composantes 
        Set(x,MODIFIE);
        if (NBFILS(c) == 0) // feuille
        {
          Set(x,MASSIF);
#ifdef DEBUG
printf("    Eleve au niveau: %d ; MASSIF\n", F[x]);
#endif
        } // if feuille
        else
        if (NBFILS(c) > 1) // noeud
        {
#ifdef DEBUG
printf("    Eleve au niveau: %d ; LPE\n", F[x]);
#endif
        }
#ifdef PARANO
        else
          printf("%s : ERREUR COMPOSANTE BRANCHE!!!\n", F_NAME);
#endif

        // empile les c-voisins de x non marques MASSIF ni EN_FAHS
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
          {
            Set(y,EN_FAHS);
            FahsPush(FAHS, y, NDG_MAX - F[y]);
#ifdef DEBUG
            printf("        Push Point %d,%d Niveau %d\n", y % rs, y / rs, F[y]);
#endif
          }
        } // for (k = 0; k < 8; k += incr_vois)
      } // if (lcalevel <= F[x])
    } // if (ncomp > 0)
  } // while (!FahsVide(FAHS))
#ifdef VERBOSE
  printf("Nombre d'elevations %d\n", nbelev);
#endif
} // Watershed4()

#ifdef NOT_USED
/* ==================================== */
static int32_t trouvefeuillerec(CompactTree * cpct, int32_t p, int32_t v)
/* ==================================== */
/*
  retourne l'index d'une feuille descendant de p tq dyn(f) = v, ou -1 en cas d'echec
*/ 
{
  int32_t i, n, j, f;
  n = NBFILS(p);
  if (n == 0) 
  {
    if (cpct->dyn[p] == v) return p;
    return -1;
  }
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(p, i);
    j = cpct->fils[j];
    f = trouvefeuillerec(cpct, j, v);
    if (f != -1) return f;
  }
  return -1;
} /* trouvefeuillerec() */

/* ==================================== */
static void ElimineDupliques(int32_t *ncomp, int32_t *tabcomp) 
/* ==================================== */
{
  int32_t k, j, i; 
  for (i = 0; i < *ncomp; i++)
    if (tabcomp[i] != -1)
      for (j = i + 1; j < *ncomp; j++)
        if (tabcomp[j] == tabcomp[i])
          tabcomp[j] = -1;
  k = j = 0;
  for (i = 0; i < *ncomp; i++)
  {
    tabcomp[j] = tabcomp[i];
    if (tabcomp[i] != -1)
      j++;
    else
      k++; // nb elements supprimes
  }
  *ncomp -= k;
} /* ElimineDupliques() */

/* ==================================== */
static int32_t InList(int32_t e, int32_t *list, int32_t n)                       
/* ==================================== */
{
/* renvoie 1 si e est dans list, 0 sinon */
/* e : l'element a rechercher */
/* list : la liste (tableau d'entiers) */
/* n : le nombre d'elements dans la liste */
  while (n > 0)
    if (list[--n] == e) return 1;
  return 0;
} /* InList() */
#endif

/* ==================================== */
int32_t lwshedval(struct xvimage *image, int32_t connex)
/* ==================================== */
/*! \fn int32_t lwshedval(struct xvimage *image, int32_t connex)
    \param image (entrée/sortie) : une image 2D ndg
    \param connex (entrée) : 4 ou 8 
    \return code erreur : 0 si échec, 1 sinon
    \brief ligne de partage des eaux valuée (algo MC, GB, LN)
*/
#undef F_NAME
#define F_NAME "lwshedval"
/*
ndg := noyau de l'image originale
calculer arbre et dynamiques des feuilles
repeter
  f = feuille de dynamique minimale
  df = dyn(f)
  hcf = |ndg(f) - dyn(f)| 
  ptcol = (select x in border(f), ndg(x) == hcf)
  vf = (select f' feuille, ptcol in border(f'))
  ptsarc = {x in border(f) inter border(vf)}
  pour tout x dans ptsarc faire V(x) := dyn(f)
  supprimer f
  raccourcir branches
jusqu'a size(tree) ==  1
*/
{
  register int32_t i, k;      /* index muet */
  register int32_t x, y;      /* index muet de pixel */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *V;             /* l'image resultat (valuations) */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * cpct;           /* resultat : l'arbre des composantes compacte' */

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)calloc(1,N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "%s : malloc failed for STATUS\n", F_NAME);
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "%s : calloc failed for number_nodes\n", F_NAME);
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "%s : calloc failed for node_at_level\n", F_NAME);
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "%s : InitCompTree failed\n", F_NAME);
      return(0);
  }

  V = (uint8_t *)calloc(N, sizeof(char));
  if (V == NULL)
  {   fprintf(stderr, "%s : calloc failed for V\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  /* ======================================================================= */
  /* 1ERE ETAPE : CALCUL DE L'ARBRE DES COMPOSANTES ET DE LA DYNAMIQUE */
  /* ======================================================================= */

  BuildTree(F, rs, ps, N, connex, incr_vois,
	    FAHS, STATUS, number_nodes, node_at_level, TREE, &cpct);

  CalculeDynamiqueMaxima(cpct);

#ifdef DEBUG
  AfficheCompactTree(cpct);
#endif

  /* ======================================================================= */
  /* 2EME ETAPE : CALCUL DE LA LIGNE DE PARTAGE DES EAUX */
  /* ======================================================================= */

  Watershed2(image, incr_vois, FAHS, STATUS, cpct);

{ int32_t nbcomp = cpct->nbcomp;
  int32_t h, c, comp;  
  int32_t f, df, hcf;
  int32_t vf, nvf, svf, ptcol, voisinf, voisinvf;

  i = 0;
  while (nbcomp > 1)
  {
    // trouve la feuille f de dyn. minimale
    f = -1;
    df = NDG_MAX+1;
    for (k = 0; k < cpct->nbcomp; k++) 
      if (!(cpct->flags[k] & FILTERED_OUT) && 
           (NbFilsNonFiltres(cpct,k) == 0) && (cpct->dyn[k] < df))
      {
        f = k;
        df = cpct->dyn[f];
      }

    //  hcf = |ndg(f) - dyn(f)| 
    hcf = DECODENIV(cpct->comp[f]) - df;

#ifdef DEBUGWV
printf("f = %d ; dyn = %d ; ndg = %d ; hcf = %d\n", f, cpct->dyn[f], hcf+df, hcf);
#endif

    //  ptcol = (select x in border(f), ndg(x) == hcf)
    // attention il faut remonter les "branches mortes" 
    for (x = 0; x < N; x++)
    {
      h = F[x];
      c = STATUS[x];
      comp = INDEXCOMP(h,c);
      while ((comp != cpct->pere[comp]) && (cpct->flags[cpct->pere[comp]] & FILTERED_OUT)) 
        comp = cpct->pere[comp]; // remonte branche morte
      if ((comp != f) && (h == hcf))
      {
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(x, k, rs, N);
          if (y != -1)
	  {
            h = F[y];
            c = STATUS[y];
            comp = INDEXCOMP(h,c);
            while ((comp != cpct->pere[comp]) && (cpct->flags[cpct->pere[comp]] & FILTERED_OUT)) 
              comp = cpct->pere[comp]; // remonte branche morte
            if (comp == f)
            {
              ptcol = x;
              goto ptcolfound;
	    }
	  } // if (y != -1)
        } // for (k = 0; k < 8; k += incr_vois)
      } // if (comp == f) 
    } // for (x = 0; x < N; x++)
ptcolfound:
#ifdef DEBUGWV
printf("ptcol = %d,%d\n", ptcol%rs, ptcol/rs);
#endif

    //  vf = (select f' feuille, ptcol in border(f'))
    x = ptcol;
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if (y != -1)
      {
        h = F[y];
        c = STATUS[y];
        comp = INDEXCOMP(h,c);
        // while ((comp != cpct->pere[comp]) && (cpct->flags[cpct->pere[comp]] & FILTERED_OUT)) 
        //  comp = cpct->pere[comp]; // remonte branche morte
        if ((comp != f) && !(cpct->flags[comp] & FILTERED_OUT) && (NbFilsNonFiltres(cpct,comp) == 0))
        {
          vf = comp;
          nvf = h;
          svf = c;
          break;
        }
      } // if (y != -1)
    } // for (k = 0; k < 8; k += incr_vois)
#ifdef DEBUGWV
printf("vf = %d, nvf = %d\n", vf, nvf);
#endif

    //  ptsarc = {x in border(f) inter border(vf)}
    //  pour tout x dans ptsarc faire V(x) := dyn(f)
    for (x = 0; x < N; x++)
    {
      voisinf = voisinvf  = 0;
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if (y != -1)
        {
          h = F[y];
          c = STATUS[y];
          comp = INDEXCOMP(h,c);
	  //          while ((comp != cpct->pere[comp]) && (cpct->flags[cpct->pere[comp]] & FILTERED_OUT)) 
          //  comp = cpct->pere[comp]; // remonte branche morte
          if (comp == f) { voisinf = 1; printf("x = %d,%d ; y = %d,%d ; voisin de f =  %d\n", x % rs, x / rs, y % rs, y / rs, f); }
          if (comp == vf) { voisinvf = 1; printf("x = %d,%d ; y = %d,%d ; voisin de vf =  %d\n", x % rs, x / rs, y % rs, y / rs, vf); }
	} // if (y != -1)
      } // for (k = 0; k < 8; k += incr_vois)
      if (voisinf && voisinvf) 
      {
        V[x] = df;
#ifdef DEBUGWV
printf("x = %d,%d ; V[x] = %d\n", x % rs, x / rs, V[x]);
#endif
      }
    } // for (x = 0; x < N; x++)

    //  supprimer f
    cpct->flags[f] |= FILTERED_OUT;
    nbcomp--;
#ifdef DEBUGWV
printf("supprime %d ; nbcomp = %d\n", f, nbcomp);
#endif
    // fusionne f et vf
    for (x = 0; x < N; x++)
    {
      h = F[x];
      c = STATUS[x];
      comp = INDEXCOMP(h,c);
      if (comp == f)
      {
        F[x] = nvf;
        STATUS[x] = svf;
#ifdef DEBUGWV
printf("x = %d,%d ; nouveau F[x] = %d\n", x % rs, x / rs, F[x]);
#endif
      }
    }

    //  raccourcir branches
    //  i.e. supprimer peres des noeuds dont le pere n'a qu'un fils
    for (comp = 0; comp < cpct->nbcomp; comp++)
      if (!(cpct->flags[comp] & FILTERED_OUT) && (NbFilsNonFiltres(cpct,cpct->pere[comp]) == 1))
        {
          if (!(cpct->flags[cpct->pere[comp]] & FILTERED_OUT))
          {          
            cpct->flags[cpct->pere[comp]] |= FILTERED_OUT;
            nbcomp--;
#ifdef DEBUGWV
printf("supprime %d ; nbcomp = %d\n", cpct->pere[comp], nbcomp);
#endif
	  }
	}

#ifdef DEBUG
AfficheCompactTree(cpct);
#endif

  } // while (nbcomp > 1)
}

  memcpy(F, V, N); // recopie de V dans F

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(cpct);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  free(V);
  return(1);
} /* lwshedval() */

/* ==================================== */
int32_t ldynamique_grimaud_ldynamique_grimaud_lwshedtopo(struct xvimage *image, int32_t connex)
/* ==================================== */
/*! \fn int32_t ldynamique_grimaud_ldynamique_grimaud_lwshedtopo(struct xvimage *image, int32_t connex)
    \param image (entrée/sortie) : une image 2D ndg
    \param connex (entrée) : 4 ou 8 
    \return code erreur : 0 si échec, 1 sinon
    \brief ligne de partage des eaux "topologique" (algo MC, GB)
*/
#undef F_NAME
#define F_NAME "ldynamique_grimaud_ldynamique_grimaud_lwshedtopo"
{
  register int32_t i, k;      /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * cpct;          /* resultat : l'arbre des composantes compacte' */

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)calloc(1,N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "%s : malloc failed for STATUS\n", F_NAME);
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "%s : calloc failed for number_nodes\n", F_NAME);
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "%s : calloc failed for node_at_level\n", F_NAME);
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "%s : InitCompTree failed\n", F_NAME);
      return(0);
  }

  /* ======================================================================= */
  /* 1ERE ETAPE : CALCUL DE L'ARBRE DES COMPOSANTES */
  /* ======================================================================= */

  /* ================================================ */
  /* INITIALISATIONS                                  */
  /* ================================================ */

  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 0; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);

#ifdef VERBOSE
  fprintf(stderr, "init terminee\n");
#endif

  /* ================================================ */
  /* APPEL FONCTION RECURSIVE flood                   */
  /* ================================================ */

  if ((connex == 4) || (connex == 8))
    (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
  else
    (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);

#ifdef VERBOSE
  fprintf(stderr, "flood terminee\n");
#endif
#ifdef DEBUG
  AfficheCompTree(TREE);
#endif

  cpct = CompTree2CompactTree(TREE, number_nodes);

#ifdef VERBOSE
  fprintf(stderr, "CompTree2CompactTree terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(cpct);
  AfficheImaComp(cpct, STATUS, rs, N, F);
#endif

  /* ======================================================================= */
  /* 2EME ETAPE : CALCUL DE LA LIGNE DE PARTAGE DES EAUX */
  /* ======================================================================= */

  IndicsInit(N);
  Watershed4(image, incr_vois, FAHS, STATUS, cpct);

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(cpct);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  return(1);
} /* ldynamique_grimaud_ldynamique_grimaud_lwshedtopo() */
