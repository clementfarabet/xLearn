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
/*! \file mcgraphe.c
    \brief algorithmes fondamentaux
*/
/*
      Michel Couprie, septembre 1999

      janvier 2005 : DepthTree, MaxDiameterTree, Lee
      février 2005 : LCA (non efficace)

      juillet-août 2009 : CircuitsNiveaux, BellmanSC, Forêts...
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcfifo.h>
#include <mcutil.h>
#include <mcdrawps.h>
#include <mcgraphe.h>

#define PARANO

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS SUR LES LISTES CHAINEES DE SOMMETS */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn pcell AlloueCell(pcell * plibre)
    \param plibre (entrée) : pointeur sur une liste chaînee de cellules libres.
    \return pointeur sur une cellule.
    \brief retire la premiere cellule de la liste pointée par plibre et retourne un pointeur sur cette cellule.
*/
static pcell AlloueCell(pcell * plibre)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "AlloueCell"
{
  pcell p;
  if (*plibre == NULL) 
  {
    fprintf(stderr, "%s : plus de cellules libres\n", F_NAME);
    exit(0);
  }
  p = *plibre;
  *plibre = (*plibre)->next;
  return p;
} /* AlloueCell() */

/* ====================================================================== */
/*! \fn void LibereCell(pcell * plibre, pcell p)
    \param plibre (entrée) : pointeur sur une liste chaînee de cellules libres.
    \param p (entrée) : pointeur sur une cellule.
    \brief insère la cellule p au début de la liste pointée par 'plibre'. 
*/
static void LibereCell(pcell * plibre, pcell p)
/* ====================================================================== */
{
  p->next = *plibre;
  *plibre = p;
} /* LibereCell() */

/* ====================================================================== */
/*! \fn void RetireTete(pcell * plibre, pcell * pliste)
    \param plibre (entrée) : pointeur sur une liste chaînee de cellules libres.
    \param pliste (entrée) : pointeur sur une liste.
    \brief retire la première cellule de la liste 'pliste'. La cellule est chaînee à la liste 'plibre'. 
*/
static void RetireTete(pcell * plibre, pcell * pliste)
/* ====================================================================== */
{
  pcell p;
  p = (*pliste)->next;
  LibereCell(plibre, *pliste);
  *pliste = p;
} /* RetireTete() */

/* ====================================================================== */
/*! \fn void AjouteTete(pcell * plibre, pcell * pliste, int32_t a, TYP_VARC v)
    \param plibre (entrée) : pointeur sur une liste chaînee de cellules libres.
    \param pliste (entrée) : pointeur sur une liste.
    \param a (entrée) : un sommet.
    \param v (entrée) : une valeur.
    \brief ajoute une cellule contenant le sommet 'a' et la valeur 'v' en tête de la liste 'pliste'. La cellule est prise dans la liste 'plibre'. 
*/
static void AjouteTete(pcell * plibre, pcell * pliste, int32_t a, TYP_VARC v)
/* ====================================================================== */
{
  pcell p;
  p = AlloueCell(plibre);
  p->next = *pliste;
  p->som = a;
  p->v_arc = v;
  *pliste = p;
} /* AjouteTete() */

/* ====================================================================== */
/*! \fn int32_t EstDansListe(pcell p, int32_t a) 
    \param p (entrée) : une liste chaînee de successeurs.
    \param a (entrée) : un sommet.
    \return booléen.
    \brief retourne 1 si le sommet 'a' se trouve dans la liste 'p', 0 sinon. 
*/
static int32_t EstDansListe(pcell p, int32_t a) 
/* ====================================================================== */
{
  for (; p != NULL; p = p->next)
    if (p->som == a) return 1;
  return 0;
} /* EstDansListe() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS D'ALLOCATION / LIBERATION POUR UN GRAPHE */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn graphe * InitGraphe(int32_t nsom, int32_t nmaxarc)
    \param   nsom (entrée) : nombre de sommets.
    \param nmaxarc (entrée) : nombre maximum d'arcs.
    \return un graphe.
    \brief alloue la memoire nécessaire pour représenter un graphe a 'nsom' sommets,
              possédant au maximum 'nmaxarc' arcs. 
              Retourne un pointeur sur la structure allouée. 
*/
graphe * InitGraphe(int32_t nsom, int32_t nmaxarc)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "InitGraphe"
{
  graphe * g;
  int32_t i;
  
  g = (graphe *)malloc(sizeof(graphe));
  if (g == NULL)
  {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
      exit(0);
  }

  g->reserve = (cell *)malloc(nmaxarc * sizeof(cell));
  if (g->reserve == NULL)
  {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
      exit(0);
  }

  g->gamma = (pcell *)calloc(nsom, sizeof(pcell)); /* calloc met a 0 la memoire allouee */
  if (g->gamma == NULL)
  {   fprintf(stderr, "%s : calloc failed\n", F_NAME);
      exit(0);
  }

  g->tete = (int32_t *)malloc(nmaxarc * sizeof(int32_t));
  g->queue = (int32_t *)malloc(nmaxarc * sizeof(int32_t));
  if ((g->tete == NULL) || (g->tete == NULL))
  {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
      exit(0);
  }

  g->v_arcs = (TYP_VARC *)malloc(nmaxarc * sizeof(TYP_VARC));
  if (g->v_arcs == NULL)
  {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
      exit(0);
  }

  g->v_sommets = (TYP_VSOM *)malloc(nsom * sizeof(TYP_VSOM));
  if (g->v_sommets == NULL)
  {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
      exit(0);
  }

  g->x = (double *)malloc(nsom * sizeof(double));
  g->y = (double *)malloc(nsom * sizeof(double));
  g->z = (double *)malloc(nsom * sizeof(double));
  if ((g->x == NULL) || (g->y == NULL) || (g->z == NULL))
  {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
      exit(0);
  }

  g->nsom = nsom;
  g->nmaxarc = nmaxarc;
  g->narc = 0;

  /* construit la liste initiale de cellules libres */
  for (i = 0; i < nmaxarc - 1; i++)
    (g->reserve+i)->next = g->reserve+i+1;
  (g->reserve+i)->next = NULL;
  g->libre = g->reserve;  

  g->nomsommet = NULL;
  
  return g;
} /* InitGraphe() */

/* ====================================================================== */
/*! \fn void TermineGraphe(graphe * g)
    \param g (entrée) : un graphe.
    \brief libère la memoire occupée par le graphe g. 
*/
void TermineGraphe(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "TermineGraphe"
{
  int32_t i, n = g->nsom;
  free(g->reserve);
  if (g->gamma) free(g->gamma);
  if (g->tete) { free(g->tete); free(g->queue); }
  if (g->v_arcs) free(g->v_arcs);
  if (g->v_sommets) free(g->v_sommets);
  if (g->nomsommet)
  {
    for (i = 0; i < n; i++) free(g->nomsommet[i]);
    free(g->nomsommet);
  }
  if (g->x) free(g->x);
  if (g->y) free(g->y);
  if (g->z) free(g->z);
  free(g);
} /* TermineGraphe() */

/* ====================================================================== */
/*! \fn void CopyXY(graphe * g1, graphe * g2)
    \param g1 (entrée) : un graphe
    \param g2 (entrée/sortie) : un graphe
    \brief copie les coordonnées x, y des sommets du graphe 1 dans le graphe 2
*/
void CopyXY(graphe * g1, graphe * g2)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "CopyXY"
{
  int32_t i, n = g1->nsom;
  if (g2->nsom != n)
  {   fprintf(stderr, "%s : graphs must have the same number of vertices\n", F_NAME);
      exit(0);
  }
  for (i = 0; i < n; i++) { g2->x[i] = g1->x[i]; g2->y[i] = g1->y[i]; }
} /* CopyXY() */

/* ====================================================================== */
/*! \fn void CopyVsom(graphe * g1, graphe * g2)
    \param g1 (entrée) : un graphe
    \param g2 (entrée/sortie) : un graphe
    \brief copie les valeurs associées aux sommets du graphe 1 dans le graphe 2
*/
void CopyVsom(graphe * g1, graphe * g2)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "CopyVsom"
{
  int32_t i, n = g1->nsom;
  if (g2->nsom != n)
  {   fprintf(stderr, "%s : graphs must have the same number of vertices\n", F_NAME);
      exit(0);
  }
  for (i = 0; i < n; i++) g2->v_sommets[i] = g1->v_sommets[i];
} /* CopyVsom() */

/* ====================================================================== */
/*! \fn graphe * CopyGraphe(graphe * g1)
    \param g1 (entrée) : un graphe
    \return un graphe
    \brief retourne un clone du graphe g1
*/
graphe * CopyGraphe(graphe * g1)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "CopyGraphe"
{
  graphe * g;
  int32_t i, j;
  TYP_VARC v;
  int32_t nsom = g1->nsom;
  int32_t nmaxarc = g1->nmaxarc;
  pcell p;

  g = InitGraphe(nsom, nmaxarc);

  if (g1->v_sommets)
    for (i = 0; i < nsom; i++) 
      g->v_sommets[i] = g1->v_sommets[i];

  if (g1->x)
    for (i = 0; i < nsom; i++) 
    {
      g->x[i] = g1->x[i];
      g->y[i] = g1->y[i];
      g->z[i] = g1->z[i];
    }

  for (i = 0; i < nsom; i++) 
    for (p = g1->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      v = p->v_arc;
      AjouteArcValue(g, i, j, (TYP_VARC)v);
    }
  
  return g;
} /* CopyGraphe() */

/* ====================================================================== */
/*! \fn void UnionGraphes(graphe * g1, graphe * g2)
    \param g1 (entrée) : un graphe
    \param g2 (entrée) : un graphe
    \brief calcule l'union des graphes g1 et g2, le résultat est stocké dans g1.
    Le champ v_sommet de l'union est le max des champs v_sommets des deux graphes.
*/
void UnionGraphes(graphe * g1, graphe * g2)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "UnionGraphes"
{
  int32_t i, j, n = g1->nsom, m1 = g1->narc, m2 = g2->narc;
  TYP_VARC v;
  pcell p;

  if (g2->nsom != n)
  {   fprintf(stderr, "%s: graphs must have the same number of vertices\n", F_NAME);
      exit(0);
  }
  if (m1 + m2 > g1->nmaxarc)
  {   fprintf(stderr, "%s: two many arcs in union\n", F_NAME);
      exit(0);
  }

  for (i = 0; i < n; i++) 
    for (p = g2->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      v = p->v_arc;
      AjouteArcValue(g1, i, j, v);
    }

  for (i = 0; i < n; i++) 
    if (g2->v_sommets[i] > g1->v_sommets[i])
      g1->v_sommets[i] = g2->v_sommets[i];
  
} /* UnionGraphes() */

/* ====================================================================== */
/*! \fn void UnitLength(graphe * g) 
    \param g (entrée) : un graphe.
    \brief met à 1 les longueurs de tous les arcs (représentation gamma seulement).
*/
void UnitLength(graphe * g) 
/* ====================================================================== */
#undef F_NAME
#define F_NAME "UnitLength"
{
  int32_t i, n = g->nsom;
  pcell p;
  for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
      p->v_arc = (TYP_VARC)1;
} /* UnitLength() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS D'ENTREE / SORTIE FICHIER POUR UN GRAPHE */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn graphe * ReadGraphe(char * filename)
    \param   filename (entrée) : nom du fichier graphe.
    \return un graphe.
    \brief Lit les données d'un graphe dans le fichier filename, retourne un pointeur sur la structure graphe construite. 
*/
graphe * ReadGraphe(char * filename)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "ReadGraphe"
{
#define TAILLEBUF 4096
  graphe * g;
  int32_t i, n, m, t, q;
  char buf[TAILLEBUF];
  char *ret;

  FILE *fd = NULL;

  fd = fopen(filename,"r");
  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  fscanf(fd, "%d %d\n", &n, &m);
  g = InitGraphe(n, m);
  do
  {
    ret = fgets(buf, TAILLEBUF, fd);
    if ((ret != NULL) && (strncmp(buf, "noms sommets", 12) == 0))
    {
      g->nomsommet = (char **)malloc(n * sizeof(char *));
      if (g->nomsommet == NULL)
      {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
          exit(0);
      }
      for (i = 0; i < n; i++)
      {
        fscanf(fd, "%d\n", &t);
        fgets(buf, TAILLEBUF, fd);
        g->nomsommet[t] = (char *)malloc((strlen(buf)+1) * sizeof(char));
        if (g->nomsommet[t] == NULL)
        {   fprintf(stderr, "%s : malloc failed\n", F_NAME);
            exit(0);
        }
        strcpy(g->nomsommet[t], buf);
      } /* for (i = 0; i < n; i++) */
    } /* if ((ret != NULL) && (strcmp(buf, "noms sommets") == 0)) */
    else if ((ret != NULL) && (strncmp(buf, "val sommets", 11) == 0))
    {
      double v;
      for (i = 0; i < n; i++)  
      {
        fscanf(fd, "%d %lf\n", &t, &v);
        g->v_sommets[t] = (TYP_VSOM)v; 
      }
    } /*  if ((ret != NULL) && (strncmp(buf, "val sommets", 11) == 0)) */
    else if ((ret != NULL) && (strncmp(buf, "coord sommets", 13) == 0))
    {
      double x, y;
      for (i = 0; i < n; i++)  
      {
        fscanf(fd, "%d %lf %lf\n", &t, &x, &y);
        g->x[t] = x; 
        g->y[t] = y; 
      }
    } /*  if ((ret != NULL) && (strncmp(buf, "coord sommets", 13) == 0)) */
    else if ((ret != NULL) && (strncmp(buf, "arcs values", 11) == 0))
    {
      double v;
      for (i = 0; i < m; i++)  
      {
        fscanf(fd, "%d %d %lf\n", &t, &q, &v);
        AjouteArcValue(g, t, q, (TYP_VARC)v);
      }
    } /*  if ((ret != NULL) && (strncmp(buf, "arcs values", 11) == 0)) */
    else if ((ret != NULL) && (strncmp(buf, "arcs", 4) == 0))
    {
      for (i = 0; i < m; i++)  
      {
        fscanf(fd, "%d %d\n", &t, &q);
        AjouteArc(g, t, q);
      }
    } /*  if ((ret != NULL) && (strncmp(buf, "arcs", 4) == 0)) */
  } while (ret != NULL);

  return g;
} /* ReadGraphe() */

/* ====================================================================== */
/*! \fn void SaveGraphe(graphe * g, char *filename) 
    \param g (entrée) : un graphe.
    \param filename (entrée) : nom du fichier à générer.
    \brief sauve le graphe g dans le fichier filename. 
*/
void SaveGraphe(graphe * g, char *filename) 
/* ====================================================================== */
#undef F_NAME
#define F_NAME "SaveGraphe"
{
  int32_t i, j, n = g->nsom, m = g->narc;
  pcell p;
  FILE * fd = NULL;
  TYP_VARC v;

  fd = fopen(filename,"w");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return;
  }
  
  fprintf(fd, "%d %d\n", n, m);

  if (g->v_sommets)
  {
    fprintf(fd, "val sommets\n");
    for (i = 0; i < n; i++) 
      fprintf(fd, "%d %g\n", i, (double)(g->v_sommets[i]));
  }

  if (g->x)
  {
    fprintf(fd, "coord sommets\n");
    for (i = 0; i < n; i++) 
      fprintf(fd, "%d %g %g\n", i, g->x[i], g->y[i]);
  }

  fprintf(fd, "arcs values\n");
  for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      v = p->v_arc;
      fprintf(fd, "%d %d %g\n", i, j, (double)v);
    }
  
  fclose(fd);
} /* SaveGraphe() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS DE CONVERSION */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn graphe * Image2Graphe(struct xvimage *image, int32_t mode, int32_t connex)
    \param image (entrée) : structure image
    \param mode (entrée) : mode de valuation des arcs
    \param connex (entrée) : relation d'adjacence (4 ou 8 en 2D, 6 ou 26 en 3D)
    \return un graphe.
    \brief Lit les données d'une image dans la structure image, 
    retourne un pointeur sur la structure graphe construite. 
*/
graphe * Image2Graphe(struct xvimage *image, int32_t mode, int32_t connex)
/* ====================================================================== */
/*
A weighted graph G = (V,E) is built with V = set of pixels of \b image
and E = {(P,Q) in VxV ; P and Q are adjacent}. 
Let F(P) be the value of pixel P in \b image. 
A weight W(P,Q) is assigned to each edge, according to the value of \b mode:

\li max : W(P,Q) = max{F(P),F(Q)} 
\li min : W(P,Q) = min{F(P),F(Q)} 
\li avg : W(P,Q) = (F(P) + F(Q)) / 2 
*/
#undef F_NAME
#define F_NAME "Image2Graphe"
{
  int32_t rs, cs, ds, N, M;
  uint8_t *F;
  graphe * g;
  int32_t i, j, p;
  TYP_VARC v;

  if (datatype(image) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: bad data type (only byte is available)\n", F_NAME);
    return NULL;
  }
  
  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  F = UCHARDATA(image);

  if ((ds == 1) && (connex == 4))
  {
    N = rs * cs;
    M = 2 * ((rs-1) * cs + (cs-1) * rs); 
    g = InitGraphe(N, M);
    for (j = 0; j < cs; j++)  
      for (i = 0; i < rs; i++)  
      {
	p = j*rs + i;
        g->x[p] = (double)i; 
        g->y[p] = (double)j; 	// coord sommet
	if (i < (rs-1)) 
	{
	  switch(mode)
	  {
	  case SP_MIN: v = mcmin((TYP_VARC)F[j*rs+i],(TYP_VARC)F[j*rs+i+1]); break;
	  case SP_MAX: v = mcmax((TYP_VARC)F[j*rs+i],(TYP_VARC)F[j*rs+i+1]); break;
	  case SP_AVG: v = ((TYP_VARC)F[j*rs+i]+(TYP_VARC)F[j*rs+i+1]) / 2; break;
	  }
	  AjouteArcValue(g, p, p+1, v*v);
	  AjouteArcValue(g, p+1, p, v*v);
	}
	if (j < (cs-1))
	{
	  switch(mode)
	  {
	  case SP_MIN: v = mcmin((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j+1)*rs+i]); break;
	  case SP_MAX: v = mcmax((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j+1)*rs+i]); break;
	  case SP_AVG: v = ((TYP_VARC)F[j*rs+i]+(TYP_VARC)F[(j+1)*rs+i]) / 2; break;
	  }
	  AjouteArcValue(g, p, p+rs, v*v);
	  AjouteArcValue(g, p+rs, p, v*v);
	}
      }
  }
  else if ((ds == 1) && (connex == 8))
  {
    N = rs * cs;
    M = 4 * ((rs-1) * cs + (cs-1) * rs); 
    g = InitGraphe(N, M);
    for (j = 0; j < cs; j++)  
      for (i = 0; i < rs; i++)  
      {
	p = j*rs + i;
        g->x[p] = (double)i; 
        g->y[p] = (double)j; 	// coord sommet
	if (i < (rs-1)) 
	{
	  switch(mode)
	  {
	  case SP_MIN: v = mcmin((TYP_VARC)F[j*rs+i],(TYP_VARC)F[j*rs+i+1]); break;
	  case SP_MAX: v = mcmax((TYP_VARC)F[j*rs+i],(TYP_VARC)F[j*rs+i+1]); break;
	  case SP_AVG: v = ((TYP_VARC)F[j*rs+i]+(TYP_VARC)F[j*rs+i+1]) / 2; break;
	  }
	  AjouteArcValue(g, p, p+1, v);
	  AjouteArcValue(g, p+1, p, v);
	}
	if (j < (cs-1))
	{
	  switch(mode)
	  {
	  case SP_MIN: v = mcmin((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j+1)*rs+i]); break;
	  case SP_MAX: v = mcmax((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j+1)*rs+i]); break;
	  case SP_AVG: v = ((TYP_VARC)F[j*rs+i]+(TYP_VARC)F[(j+1)*rs+i]) / 2; break;
	  }
	  AjouteArcValue(g, p, p+rs, v);
	  AjouteArcValue(g, p+rs, p, v);
	}
	if ((i < (rs-1)) && (j < (cs-1)))
	{
	  switch(mode)
	  {
	  case SP_MIN: v = mcmin((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j+1)*rs+i+1]); break;
	  case SP_MAX: v = mcmax((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j+1)*rs+i+1]); break;
	  case SP_AVG: v = ((TYP_VARC)F[j*rs+i]+(TYP_VARC)F[(j+1)*rs+i+1]) / 2; break;
	  }
	  AjouteArcValue(g, p, p+rs+1, v);
	  AjouteArcValue(g, p+rs+1, p, v);
	}
	if ((i < (rs-1)) && (j > 0))
	{
	  switch(mode)
	  {
	  case SP_MIN: v = mcmin((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j-1)*rs+i+1]); break;
	  case SP_MAX: v = mcmax((TYP_VARC)F[j*rs+i],(TYP_VARC)F[(j-1)*rs+i+1]); break;
	  case SP_AVG: v = ((TYP_VARC)F[j*rs+i]+(TYP_VARC)F[(j-1)*rs+i+1]) / 2; break;
	  }
	  AjouteArcValue(g, p, p-rs+1, v);
	  AjouteArcValue(g, p-rs+1, p, v);
	}
      }
  }
  else
  {
    fprintf(stderr, "%s: 3D not yet implemented\n", F_NAME);
    return NULL;
  }

  return g;
} /* Image2Graphe() */

/* ====================================================================== */
/*! \fn graphe * BinaryImage2Graphe(struct xvimage *image, int32_t connex)
    \param image (entrée) : structure image
    \param connex (entrée) : relation d'adjacence (4 ou 8 en 2D, 6 ou 18 ou 26 en 3D)
    \return un graphe.
    \brief Lit les données d'une image dans la structure image, 
    retourne un pointeur sur la structure graphe construite. 
*/
graphe * BinaryImage2Graphe(struct xvimage *image, int32_t connex)
/* ====================================================================== */
/*
A simple graph G = (V,E) is built with V = set of object pixels of \b image
and E = {(P,Q) in VxV ; P and Q are adjacent}. 
*/
#undef F_NAME
#define F_NAME "BinaryImage2Graphe"
{
  int32_t rs, cs, ds, N, M;
  uint8_t *F;
  graphe * g;
  int32_t i, j, p;

  if (datatype(image) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: bad data type (only byte is available)\n", F_NAME);
    return NULL;
  }
  
  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  F = UCHARDATA(image);

  if ((ds == 1) && (connex == 4))
  {
    N = rs * cs;
    M = 2 * ((rs-1) * cs + (cs-1) * rs); // max nb of arcs
    g = InitGraphe(N, M);
    for (j = 0; j < cs; j++)  
      for (i = 0; i < rs; i++)  
      {
	p = j*rs + i;
	g->x[p] = (double)i; 
	g->y[p] = (double)j; 	// coord sommet
	g->v_sommets[p] = 0;
	if (F[p])
	{
	  g->v_sommets[p] = 1;
	  if ((i < (rs-1)) && F[j*rs+i+1])
	  {
	    AjouteArc(g, p, p+1);
	    AjouteArc(g, p+1, p);
	  }
	  if ((j < (cs-1)) && F[(j+1)*rs+i])
	  {
	    AjouteArc(g, p, p+rs);
	    AjouteArc(g, p+rs, p);
	  }
	}
      }
  }
  else if ((ds == 1) && (connex == 8))
  {
    N = rs * cs;
    M = 4 * ((rs-1) * cs + (cs-1) * rs); 
    g = InitGraphe(N, M);
    for (j = 0; j < cs; j++)  
      for (i = 0; i < rs; i++)  
      {
	p = j*rs + i;
	g->x[p] = (double)i; 
	g->y[p] = (double)j; 	// coord sommet
	g->v_sommets[p] = 0;
	if (F[p])
	{
	  g->v_sommets[p] = 1;
	  if ((i < (rs-1)) && F[j*rs+i+1])
	  {
	    AjouteArc(g, p, p+1);
	    AjouteArc(g, p+1, p);
	  }
	  if ((j < (cs-1)) && F[(j+1)*rs+i])
	  {
	    AjouteArc(g, p, p+rs);
	    AjouteArc(g, p+rs, p);
	  }
	  if ((i < (rs-1)) && (j < (cs-1)) && F[(j+1)*rs+i+1])
	  {
	    AjouteArc(g, p, p+rs+1);
	    AjouteArc(g, p+rs+1, p);
	  }
	  if ((i < (rs-1)) && (j > 0) && F[(j-1)*rs+i+1])
	  {
	    AjouteArc(g, p, p-rs+1);
	    AjouteArc(g, p-rs+1, p);
	  }
	}
      }
  }
  else if (ds == 1)
  {
    fprintf(stderr, "%s: bad connex %d\n", F_NAME, connex);
    return NULL;
  }
  else
  {
    fprintf(stderr, "%s: 3D not yet implemented\n", F_NAME);
    return NULL;
  }

  return g;
} /* BinaryImage2Graphe() */

/* ====================================================================== */
/*! \fn struct xvimage *Graphe2Image(graphe * g, int32_t rs)
    \param g (entrée) : un graphe
    \return une structure image
    \brief génère une image à partir du graphe g 
    (d'après les valeurs associées aux sommets) 
    et retourne un pointeur sur la structure image construite. 
*/
struct xvimage *Graphe2Image(graphe * g, int32_t rs)
/* ====================================================================== */
/*

*/
#undef F_NAME
#define F_NAME "Graphe2Image"
{
  int32_t N;
  struct xvimage *image;
  uint8_t *F;
  int32_t i;

  N = g->nsom;
  if (N % rs != 0)
  {
    fprintf(stderr, "%s: rs and g->nsom must be congruent\n", F_NAME);
    return NULL;
  }

  image = allocimage(NULL, rs, N / rs, 1, VFF_TYP_1_BYTE);
  if (image == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }
  F = UCHARDATA(image);
  memset(F, 0, N);
  
  for (i = 0; i < N; i++) F[i] = (uint8_t)g->v_sommets[i]; 

  return image;
} /* Graphe2Image() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS DE MANIPULATION DES ARCS (APPLICATION GAMMA UNIQUEMENT) */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn void AjouteArc(graphe * g, int32_t i, int32_t s)
    \param g (entrée/sortie) : un graphe.
    \param i (entrée) : extrémité initiale de l'arc.
    \param s (entrée) : extrémité finale de l'arc.
    \brief ajoute l'arc (i,s) au graphe g (application gamma seulement). 
*/
void AjouteArc(graphe * g, int32_t i, int32_t s)
/* ====================================================================== */
{
  AjouteTete(&(g->libre), &(g->gamma[i]), s, 0);
  g->narc++;
} /* AjouteArc() */

/* ====================================================================== */
/*! \fn void AjouteArcValue(graphe * g, int32_t i, int32_t s, TYP_VARC v)
    \param g (entrée/sortie) : un graphe.
    \param i (entrée) : extrémité initiale de l'arc.
    \param s (entrée) : extrémité finale de l'arc.
    \param v (entrée) : une valeur pour l'arc.
    \brief ajoute l'arc (i,s) au graphe g (application gamma seulement). 
*/
void AjouteArcValue(graphe * g, int32_t i, int32_t s, TYP_VARC v)
/* ====================================================================== */
{
  AjouteTete(&(g->libre), &(g->gamma[i]), s, v);
  g->narc++;
} /* AjouteArcValue() */

/* ====================================================================== */
/*! \fn void RetireArc(graphe * g, int32_t i, int32_t s)
    \param g (entrée/sortie) : un graphe.
    \param i (entrée) : un sommet de g.
    \param s (entrée) : un sommet de g.
    \brief retire l'arc (i,s) du graphe g (application gamma seulement),
              si celui-ci etait présent. Sinon, pas d'action. 
*/
void RetireArc(graphe * g, int32_t i, int32_t s)
/* ====================================================================== */
{
  pcell * pliste;
  pliste = g->gamma + i;
  while ((*pliste != NULL) && (((*pliste)->som) != s))
    pliste = &((*pliste)->next);
  if (*pliste != NULL) RetireTete(&(g->libre), pliste);
  g->narc--;
} /* RetireArc() */

/* ====================================================================== */
/*! \fn int32_t PopSuccesseur(graphe * g, int32_t i)
    \param g (entrée/sortie) : un graphe.
    \param i (entrée) : un sommet de g.
    \brief retire un arc (i,s) du graphe g (application gamma seulement),
           et retourne le sommet s
    \warning le sommet i doit avoir au moins un successeur (pas de vérification)
*/
int32_t PopSuccesseur(graphe * g, int32_t i)
/* ====================================================================== */
{
  int32_t s = g->gamma[i]->som;
  RetireTete(&(g->libre), &(g->gamma[i]));
  return s;
} /* PopSuccesseur() */

/* ====================================================================== */
/*! \fn int32_t EstSuccesseur(graphe *g, int32_t i, int32_t s) 
    \param g (entrée) : un graphe.
    \param i (entrée) : un sommet de g.
    \param s (entrée) : un sommet de g.
    \return booléen.
    \brief retourne 1 si le sommet 's' est un successeur du sommet 'i', 0 sinon.
*/
int32_t EstSuccesseur(graphe *g, int32_t i, int32_t s) 
/* ====================================================================== */
{
  return EstDansListe(g->gamma[i], s);
} /* EstSuccesseur() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS DE MANIPULATION DES ARCS (LISTE D'ARCS) */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn void Gamma2ListArcs(graphe *g) 
    \param g (entrée) : un graphe.
    \brief génère la représentation "liste d'arcs" du graphe g.
*/
void Gamma2ListArcs(graphe *g) 
/* ====================================================================== */
{
  int32_t i, a, n = g->nsom;
  pcell p;

  a = 0;
  for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      g->tete[a] = i;
      g->queue[a] = p->som;
      g->v_arcs[a] = p->v_arc;
      a++;
    }
} /* Gamma2ListArcs() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS DE GENERATION DE GRAPHES */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn graphe * GrapheAleatoire(int32_t nsom, int32_t narc)
    \param nsom (entrée) : nombre de sommets.
    \param narc (entrée) : nombre d'arcs.
    \return un graphe.
    \brief retourne un graphe aléatoire à 'nsom' sommets et 'narc' arcs. 
              Le graphe est antisymétrique et sans boucle.
              Le nombre d'arcs doit être <= nsom (nsom - 1) / 2.
              Les arcs sont pondérés (valeur aléatoire entre 0.0 et 1.0).
    \warning la méthode employée ici est naïve, tres inefficace du point de vue
              temps de calcul et ne garantit aucune propriété statistique. 
*/
graphe * GrapheAleatoire(int32_t nsom, int32_t narc)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "GrapheAleatoire"
{
  graphe * g;
  int32_t i, j, m;
  double mmax = ((double)nsom * ((double)nsom - 1)) / 2;

  if (narc > mmax)
  {
    fprintf(stderr, "%s : pas plus de %g arcs pour %d sommets\n", 
	    F_NAME, mmax, nsom);
    exit(0);
  }

  if ((mmax >= (1<<31)) || (narc <= ((int32_t)mmax) / 2))
  {
    g = InitGraphe(nsom, narc);
    while (narc > 0)
    {
      narc--;
      do
      {
        i = rand() % nsom;
        j = rand() % nsom;
      } while ((i == j) || EstSuccesseur(g, i, j) || EstSuccesseur(g, j, i));
      AjouteArc(g, i, j);
      g->tete[narc] = i;
      g->queue[narc] = j;
      if (g->v_arcs) g->v_arcs[narc] = (TYP_VARC)(rand()*100.0);
    }
  }
  else /* on part du graphe complet et on retire des arcs au hasard */
  {
    /* construit un graphe complet antisymetrique sans boucle */
    g = InitGraphe(nsom, (int32_t)mmax);
    for (i = 0; i < nsom ; i++)
      for (j = i+1; j < nsom; j++)
        AjouteArc(g, i, j);         

    /* retire mmax - narc arcs */
    m = (int32_t)mmax;
    while (m > narc)
    {
      m--;
      do
      {
        i = rand() % nsom;
        j = rand() % nsom;
      } while ((i == j) || !EstSuccesseur(g, mcmin(i,j), mcmax(i,j)));
      RetireArc(g, mcmin(i,j), mcmax(i,j));
    }

    /* rajoute la liste des arcs et les poids */
    m = 0;
    for (i = 0; i < nsom ; i++)
      for (j = i+1; j < nsom; j++)
        if (EstSuccesseur(g, i, j))
	{
          g->tete[m] = i;
          g->queue[m] = j;
          if (g->v_arcs) g->v_arcs[narc] = (TYP_VARC)(rand()*100.0);
          m++;
	}
  }

  return g;
} /* GrapheAleatoire() */

/* ====================================================================== */
/* ====================================================================== */
/* OPERATEURS DE BASE SUR LES GRAPHES */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn boolean * EnsembleVide(int32_t n)
    \param n (entrée) : taille du tableau booléen.
    \return un tableau de \b n booléens, tous égaux à FALSE.
    \brief alloue et initialise un tableau de \b n booléens, représentant l'ensemble vide.
*/
static boolean * EnsembleVide(int32_t n)
/* ====================================================================== */
{
  boolean *Ca = (boolean *)calloc(n, sizeof(boolean));
  if (Ca == NULL)
  {   fprintf(stderr, "EnsembleVide : calloc failed\n");
      exit(0);
  }
  return Ca;
} /* EnsembleVide() */

/* ====================================================================== */
/*! \fn int32_t * ListeVide(int32_t n)
    \param n (entrée) : taille max de la liste.
    \return un tableau de \b n entiers
    \brief alloue et initialise un tableau de \b n entiers
*/
static int32_t * ListeVide(int32_t n)
/* ====================================================================== */
{
  int32_t *Ca = (int32_t *)calloc(n, sizeof(int32_t));
  if (Ca == NULL)
  {   fprintf(stderr, "ListeVide : calloc failed\n");
      exit(0);
  }
  return Ca;
} /* ListeVide() */

/* ====================================================================== */
/*! \fn int32_t Degre(graphe * g, int32_t s)
    \param g (entrée) : un graphe
    \param s (entrée) : un sommet
    \return un entier
    \brief retourne le degré du sommet s dans le graphe graphe g (nombre de successeurs)
    \warning seule la représentation 'gamma' est utilisée
*/
int32_t Degre(graphe * g, int32_t s)
/* ====================================================================== */
{
  int32_t d = 0;
  pcell p;
  for (p = g->gamma[s]; p != NULL; p = p->next) d++;
  return d;
} /* Degre() */

/* ====================================================================== */
/*! \fn graphe * Symetrique(graphe * g)
    \param g (entrée) : un graphe
    \return un graphe
    \brief construit et retourne le graphe g_1 symétrique du graphe g (algorithme linéaire)
    \warning seule la représentation 'gamma' est utilisée
*/
graphe * Symetrique(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Symetrique"
{
  graphe *g_1;
  int32_t nsom, narc, i, j;
  pcell p;

  nsom = g->nsom;
  narc = g->narc;
  g_1 = InitGraphe(nsom, narc);
  for (i = 0; i < nsom; i++) /* pour tout i sommet de g */
  {
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout j successeur de i */
      j = p->som;
      AjouteArcValue(g_1, j, i, p->v_arc);
    } // for p
  } // for i

  return g_1;
} /* Symetrique() */

/* ====================================================================== */
/*! \fn graphe * FermetureSymetrique(graphe * g)
    \param g (entrée) : un graphe
    \return un graphe
    \brief construit et retourne la fermeture symétrique du graphe g (algorithme linéaire)
    \warning seule la représentation 'gamma' est utilisée
*/
graphe * FermetureSymetrique(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "FermetureSymetrique"
{
  graphe *gs;
  int32_t nsom, narc, i, j;
  pcell p;

  nsom = g->nsom;
  narc = g->narc;
  gs = InitGraphe(nsom, 2 * narc);
  for (i = 0; i < nsom; i++) /* pour tout i sommet de g */
  {
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout j successeur de i */
      j = p->som;
      AjouteArcValue(gs, i, j, p->v_arc);
      AjouteArcValue(gs, j, i, p->v_arc);
    } // for p
  } // for i

  return gs;
} /* FermetureSymetrique() */

/* ====================================================================== */
/*! \fn boolean *Descendants(graphe *g, int32_t a)
    \param g (entrée) : le graphe 
    \param a (entrée) : un sommet du graphe g
    \brief retourne l'ensemble des sommets descendants de a dans le graphe g
    (le sommet a est inclus)
*/
boolean * Descendants(graphe * g, int32_t a)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Descendants"
{
  boolean * D;   /* pour les "descendants" (successeurs a N niveaux) */
  Lifo * T;   /* liste temporaire geree en pile (Last In, First Out) */
  int32_t i, s, n;
  pcell p;
  n = g->nsom;
  T = CreeLifoVide(n);
  D = EnsembleVide(n);
  D[a] = TRUE;
  LifoPush(T, a);
  while (!LifoVide(T))
  {
    i = LifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next) 
    { /* pour tout s prédécesseur de i */
      s = p->som;
      if (!D[s]) 
      {
        D[s] = TRUE;
        LifoPush(T, s);
      }
    } // for p
  } // while (!LifoVide(T))
  LifoTermine(T);
  return D;
} // Descendants()

/* ====================================================================== */
/*! \fn void CompFortConnexe(graphe * g, graphe *g_1, int32_t a, boolean * Ca)
    \param g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique de g
    \param a (entrée) : un sommet du graphe g
    \param Ca (sortie) : un sous-ensemble des sommets de g (tableau de booléens)
    \brief retourne dans Ca la composante fortement connexe de g contenant a 
          (sous la forme d'un tableau de booléens)
    \warning Ca doit avoir été alloué correctement (pas de vérification)
*/
void CompFortConnexe(graphe * g, graphe *g_1, int32_t a, boolean * Ca)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "CompFortConnexe"
{
  boolean * D;   /* pour les "descendants" (successeurs a N niveaux) */
  boolean * A;   /* pour les "ascendants" (predecesseurs a N niveaux) */
  int32_t i, n;
  n = g->nsom;
  D = Descendants(g, a);
  A = Descendants(g_1, a);
  for (i = 0; i < n; i++) Ca[i] = (D[i] && A[i]);
  free(A);
  free(D);
} /* CompFortConnexe() */

/* ====================================================================== */
/*! \fn boolean ExisteCircuit(graphe * g, int32_t a)
    \param g (entrée) : un graphe
    \param a (entrée) : un sommet du graphe g
    \return booléen
    \brief teste l'existence d'un circuit dans g contenant a 
*/
boolean ExisteCircuit(graphe * g, int32_t a)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "ExisteCircuit"
{
  boolean * D;   /* pour les "descendants" (successeurs a N niveaux) */
  Lifo * T;   /* liste temporaire geree en pile (Last In, First Out) */
  int32_t i, n, s;
  pcell p;

  n = g->nsom;
  T = CreeLifoVide(n);
  D = EnsembleVide(n);

  /* calcule les descendants de a */
  LifoPush(T, a);  
  while (!LifoVide(T))
  {
    i = LifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout sommet s successeur de i */
      s = p->som;
      if (s == a) { free(D); LifoTermine(T); return TRUE; }
      if (!D[s]) 
      {
        D[s] = TRUE;
        LifoPush(T, s);
      }
    } // for p
  } // while (!LifoVide(T))

  free(D);
  LifoTermine(T);
  return FALSE;
} /* ExisteCircuit() */

/* ====================================================================== */
/*! \fn void CompConnexe(graphe * g, graphe *g_1, int32_t a, boolean * Ca)
    \param   g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique de g
    \param a (entrée) : un sommet du graphe g
    \return un sous-ensemble de sommets de g (tableau de booléens)
    \brief retourne la composante connexe de g contenant a
          (sous la forme d'un tableau de booléens)
*/
void CompConnexe(graphe * g, graphe *g_1, int32_t a, boolean * Ca)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "CompConnexe"
{
  Lifo * T;   /* liste temporaire geree en pile (Last In, First Out) */
  int32_t i, n, s;
  pcell p;

  n = g->nsom;
  T = CreeLifoVide(n);
  memset(Ca, 0, n); // Ca := vide

  Ca[a] = TRUE;
  LifoPush(T, a);  
  while (!LifoVide(T))
  {
    i = LifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s successeur de i */
      s = p->som;
      if (!Ca[s]) 
      {
        Ca[s] = TRUE;
        LifoPush(T, s);
      }
    } // for p
    for (p = g_1->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s successeur de i dans g_1 */
      s = p->som;
      if (!Ca[s]) 
      {
        Ca[s] = TRUE;
        LifoPush(T, s);
      }
    } // for p
  } // while (!LifoVide(T))

  LifoTermine(T);
} /* CompConnexe() */

/* ====================================================================== */
/*! \fn int32_t Connexe(graphe * g, graphe *g_1)
    \param g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique de g
    \return booléen
    \brief retourne 1 si le graphe est connexe, 0 sinon
*/
boolean Connexe(graphe * g, graphe *g_1)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Connexe"
{
  int32_t i;
  boolean ret = TRUE;
  int32_t n = g->nsom;
  boolean * C = EnsembleVide(n);
  CompConnexe(g, g_1, 0, C);
  for (i = 0; i < n; i++) if (!C[i]) { ret = FALSE; break; }
  free(C);
  return ret;
} /* Connexe() */

/* ====================================================================== */
/*! \fn boolean CircuitNiveaux(graphe * g)
    \param g (entrée) : un graphe
    \return booléen
    \brief si le graphe possède au moins un circuit, retourne TRUE ;
           sinon, calcule les niveaux des sommets du graphes (dans v_sommets) 
           et retourne FALSE
*/
boolean CircuitNiveaux(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "CircuitNiveaux"
{
  int32_t * D;   // pour les degrés intérieurs
  int32_t i, n, x, y, N;
  pcell p;
  Lifo * T, *U, *V;

  n = g->nsom;
  T = CreeLifoVide(n);
  U = CreeLifoVide(n);
  if ((T == NULL) || (U == NULL))
  {
    fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
    exit(0);
  }
  D = (int32_t *)calloc(n, sizeof(int32_t));
  if (D == NULL)
  {
    fprintf(stderr, "%s : calloc failed\n", F_NAME);
    exit(0);
  }
  i = N = 0;
  
  for (x = 0; x < n; x++)
    for (p = g->gamma[x]; p != NULL; p = p->next)
    { 
      y = p->som;
      D[y] += 1;
    }

  for (x = 0; x < n; x++)
    g->v_sommets[x] = -1;
  
  for (x = 0; x < n; x++)
    if (D[x] == 0)
    {
      g->v_sommets[x] = i;
      N++;
      LifoPush(T, x);
    }

  while ((N < n) && !LifoVide(T))
  {
    i++;
    while (!LifoVide(T))
    {
      x = LifoPop(T);    
      for (p = g->gamma[x]; p != NULL; p = p->next)
      { 
	y = p->som;
	D[y] -= 1;
	if (D[y] == 0)
	{
	  g->v_sommets[y] = i;
	  LifoPush(U, y);
	  N++;
	}
      }
    } // while (!LifoVide(T))
    V = T; T = U; U = V;
  } // while ((N < n) && !LifoVide(T))

  free(D);
  LifoTermine(T);
  LifoTermine(U);
  if (N < n) return TRUE; else return FALSE;
} /* CircuitNiveaux() */

/* ====================================================================== */
/* ====================================================================== */
/* ARBRES */
/* ====================================================================== */
/* ====================================================================== */

/*************************************************
  Tri rapide et selection
  D'apres "Introduction a l'algorithmique", 
    T. Cormen, C. Leiserson, R. Rivest, pp. 152, Dunod Ed. 

  Michel Couprie  -  Decembre 1997

  Version avec index et cle - Septembre 1999
  Les donnees cle sont contenues dans un tableau T.
  Le tri s'effectue sur un tableau A contenant les index
    des elements de T.
**************************************************/

#define TypeCle TYP_VARC

/* =============================================================== */
static int32_t mcgraphe_Partitionner(int32_t *A, TypeCle *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements q tq T[A[q]] <= T[A[p]] et les autres.
*/
{
  int32_t t;
  TypeCle x = T[A[p]];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (T[A[j]] > x);
    do i++; while (T[A[i]] < x);
    if (i < j) { t = A[i]; A[i] = A[j]; A[j] = t; }
    else return j;
  } /* while (1) */   
} /* mcgraphe_Partitionner() */

/* =============================================================== */
static int32_t PartitionStochastique(int32_t *A, TypeCle *T, int32_t p, int32_t r)
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
  return mcgraphe_Partitionner(A, T, p, r);
} /* PartitionStochastique() */

/* =============================================================== */
/*! \fn void TriRapideStochastique (int32_t * A, TypeCle *T, int32_t p, int32_t r)
    \param A (entrée/sortie) : un tableau d'entiers
    \param T (entrée) : un tableau de valeurs de type TypeCle.
    \param p (entrée) : indice du début de la zone à trier.
    \param r (entrée) : indice de fin de la zone à trier.
    \brief tri par ordre croissant des valeurs du tableau \b T.
           Le tri s'effectue sur un tableau \b A contenant les index
           des elements de \b T, l'indice \b p (compris) à l'indice \b r (compris).
*/
static void TriRapideStochastique (int32_t * A, TypeCle *T, int32_t p, int32_t r)
/* =============================================================== */
{
  int32_t q; 
  if (p < r)
  {
    q = PartitionStochastique(A, T, p, r);
    TriRapideStochastique (A, T, p, q) ;
    TriRapideStochastique (A, T, q+1, r) ;
  }
} /* TriRapideStochastique() */

/* ====================================================================== */
/*! \fn graphe * Kruskal1(graphe * g, graphe *g_1)
    \param g (entrée) : un graphe pondéré connexe antisymétrique antiréflexif
    \param g_1 (entrée) : le graphe symétrique de g
    \return un arbre
    \brief retourne un arbre de poids maximal pour g
*/
graphe * Kruskal1(graphe * g, graphe *g_1)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Kruskal1"
{
  int32_t n = g->nsom;
  int32_t m = g->narc;
  graphe * apm;    /* pour le resultat */
  graphe * apm_1;  /* pour la detection de cycles */
  int32_t *A;          /* tableau pour ranger les index des arcs */
  int32_t i, j, t, q;
  boolean *Ct = EnsembleVide(n);

  /* tri des arcs par poids croissant */
  A = (int32_t *)malloc(m * sizeof(int32_t)); /* allocation index */
  if (A == NULL)
  {   fprintf(stderr, "%s: malloc failed\n", F_NAME);
      exit(0);
  }  
  for (i = 0; i < m; i++) A[i] = i; /* indexation initiale */
  TriRapideStochastique(A, g->v_arcs, 0, m-1);

  /* construit le graphe resultat, initialement vide */
  apm = InitGraphe(n, n-1);
  apm_1 = InitGraphe(n, n-1);

  /* Boucle sur les arcs pris par ordre decroissant:
     on ajoute un arc dans apm, si cela ne cree pas un cycle dans apm.
     Arret lorsque nb arcs ajoutes = n-1. */

  j = 0; 
  i = m - 1;
  while (j < n-1)
  {
    t = g->tete[A[i]]; q = g->queue[A[i]]; 
    CompConnexe(apm, apm_1, t, Ct);
    if (!Ct[q]) 
    {
      AjouteArcValue(apm, t, q, g->v_arcs[A[i]]);
      AjouteArc(apm_1, q, t);
      j++; 
    }
    i--;
    if ((i < 0) && (j < (n-1)))
    {   fprintf(stderr, "%s: graphe d'origine non connexe\n", F_NAME);
        exit(0);
    }  
  } // while (j < n-1)

  if (g->x != NULL) // recopie les coordonnees des sommets pour l'affichage
    for (i = 0; i < n; i++) { apm->x[i] = g->x[i];  apm->y[i] = g->y[i]; }

  free(A);
  free(Ct);
  TermineGraphe(apm_1);
  return apm;
} /* Kruskal1() */

/* ====================================================================== */
/*! \fn graphe * Kruskal2(graphe * g, graphe *g_1)
    \param g (entrée) : un graphe pondéré connexe antisymétrique antiréflexif
    \param g_1 (entrée) : le graphe symétrique de g
    \return un arbre
    \brief retourne un arbre de poids maximal pour g
    \warning les données du graphe d'origine seront détruites
*/
graphe * Kruskal2(graphe * g, graphe *g_1)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Kruskal2"
{
  int32_t n = g->nsom;
  int32_t msav, m = g->narc;
  graphe * apm;    /* pour le resultat */
  int32_t *A;          /* tableau pour ranger les index des arcs */
  int32_t i, j, t, q;
  msav = m;

  if (!Connexe(g, g_1)) 
  {   fprintf(stderr, "%s: graphe d'origine non connexe\n", F_NAME);
      exit(0);
  }  

  /* tri des arcs par poids croissant */
  A = (int32_t *)malloc(m * sizeof(int32_t)); /* allocation index */
  if (A == NULL)
  {   fprintf(stderr, "%s: malloc failed\n", F_NAME);
      exit(0);
  }  
  for (i = 0; i < m; i++) A[i] = i; /* indexation initiale */
  TriRapideStochastique(A, g->v_arcs, 0, m-1);

  /* Boucle sur les arcs pris par ordre croissant:
       on enleve un arc, si cela deconnecte le graphe on le remet.
     Arret lorsque m = n-1. */
  i = 0;
  while (m > n-1)
  {
    t = g->tete[A[i]]; q = g->queue[A[i]]; 
    RetireArc(g, t, q);
    RetireArc(g_1, q, t);
    if (Connexe(g, g_1)) 
      m--; 
    else 
    {
      AjouteArc(g, t, q);
      AjouteArc(g_1, q, t);
    }
    i++;
  } // while (m > n-1)

  /* construit le graphe resultat */
  apm = InitGraphe(n, m);
  for (i = 0, j = 0; i < msav; i++)
  {
    t = g->tete[i]; q = g->queue[i]; 
    if (EstSuccesseur(g, t, q)) 
    {
      AjouteArc(apm, t, q);
      apm->tete[j] = t; 
      apm->queue[j] = q; 
      apm->v_arcs[j] = g->v_arcs[i]; 
      j++;
    }
  } // for i

  if (g->x)
    for (i = 0; i < n; i++) { apm->x[i] = g->x[i];  apm->y[i] = g->y[i]; }

  free(A);
  return apm;
} /* Kruskal2() */

/* ====================================================================== */
/*! \fn void DepthTree(graphe * g, graphe *g_1, int32_t a, TYP_VARC *depth, int32_t *farthest)
    \param g (entrée) : un graphe (arbre), symétrique ou non
    \param a (entrée) : un sommet du graphe g
    \param depth (sortie) : la profondeur de l'arbre de racine a
    \param farthest (sortie) : un sommet le plus éloigné de a 
    \brief calcule la profondeur de l'arbre de racine a
*/
void DepthTree(graphe * g, int32_t a, TYP_VARC *depth, int32_t *farthest)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "DepthTree"
{
  Lifo * T;   /* liste temporaire geree en pile (Last In, First Out) */
  int32_t i, f, n, s;
  pcell p;
  TYP_VARC* dist;
  TYP_VARC d;

  n = g->nsom;
  dist = (TYP_VARC *)malloc(n * sizeof(TYP_VARC));
  T = CreeLifoVide(n);
  for (i = 0; i < n; i++) dist[i] = -1;

  d = dist[a] = 0;
  LifoPush(T, a);  
  while (!LifoVide(T))
  {
    i = LifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s successeur de i */
      s = p->som;
      if (dist[s] == -1) 
      {
        dist[s] = dist[i] + p->v_arc;
	if (dist[s] > d) { d = dist[s]; f = s; }
        LifoPush(T, s);
      }
    } // for p
  } // while (!LifoVide(T))

  free(dist);
  LifoTermine(T);
  *depth = d;
  *farthest = f;
} /* DepthTree() */

/* ====================================================================== */
/*! \fn void MaxDiameterTree(graphe * g)
    \param g (entrée) : un graphe (arbre) symétrique
    \brief calcule un diametre maximal
*/
graphe * MaxDiameterTree(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "MaxDiameterTree"
{
  int32_t n = g->nsom;
  int32_t i, imd;
  TYP_VARC * diam, d, maxdiam = 0;
  int32_t *opp_som, f;
  graphe *pcc;

  diam = (TYP_VARC *)malloc(n * sizeof(TYP_VARC *));
  opp_som = (int32_t *)malloc(n * sizeof(int32_t *));
  for (i = 0; i < n; i++) diam[i] = -1;
  
  for (i = 0; i < n; i++)
    if (Degre(g, i) == 1)
    {
      DepthTree(g, i, &d, &f);
      diam[i] = d;
      opp_som[i] = f;
      if (d > maxdiam) { maxdiam = d; imd = i; }
    }
  Lee(g, imd);
  pcc = PCCna(g, imd, opp_som[imd]);
  return pcc;
} /* MaxDiameterTree() */

/* ====================================================================== */
/*! \fn int32_t LCA(graphe * g, int32_t i, int32_t j)
    \param g (entrée) : un graphe (arbre orienté des feuilles vers la racine)
    \param i (entrée) : un sommet de \b g
    \param j (entrée) : un sommet de \b g
    \brief retourne le plus proche ancêtre commun de i et j (least common ancestor)
*/
int32_t LCA(graphe * g, int32_t i, int32_t j)
/* ====================================================================== */
/* algorithme linéaire, efficace pour quelques requêtes mais 
   pour un algo en temps constant avec pre-processing linéaire voir:
   author = 	 {M.A. Bender and M. Farach-Colton},
   title = 	 {The {LCA} problem revisited},
   booktitle = 	 {procs. Latin American Theoretical Informatics, LNCS, Springer Verlag},
   volume = {1776},
   pages = 	 {88--94},
   year = 	 {2000}   
 */
{
#undef F_NAME
#define F_NAME "LCA"  
#define CHECK_TREE
  int32_t k = -1, n = g->nsom;
  pcell p;
  boolean *M = EnsembleVide(n);

  M[i] = TRUE; // met i et les ancêtres de i dans l'ensemble M
  p = g->gamma[i]; 
  while (p != NULL)
  {
#ifdef CHECK_TREE
    if (p->next != NULL)
    {
      fprintf(stderr,"%s: non-tree structure\n", F_NAME);
      exit(0);
    }
#endif
    i = p->som;
    M[i] = TRUE;
    p = g->gamma[i]; 
  } 

  if (M[j]) k = j; // recherche un element de M parmi les ancêtres de j
  else
  {
    p = g->gamma[j]; 
    while (p != NULL) 
    {
#ifdef CHECK_TREE
      if (p->next != NULL)
      {
	fprintf(stderr,"%s: non-tree structure\n", F_NAME);
	exit(0);
      }
#endif
      j = p->som;
      if (M[j]) { k = j; break; }
      p = g->gamma[j];
    } 
  }

#ifdef CHECK_TREE
  if (k == -1)
  {
    fprintf(stderr,"%s: no LCA found\n", F_NAME);
    exit(0);
  }
#endif

  free(M);
  return k;
} /* LCA() */

/* ====================================================================== */
/*! \fn graphe * RootTree(graphe * g, graphe * g_1, int32_t i)
    \param g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique du graphe g
    \param i (entrée) : un sommet de \b g
    \brief retourne un arbre orienté (arboresence) de racine i
    dont les arcs correspondent aux arêtes du graphe non-orienté représenté par g et g_1.
    Les distances (nombre d'arcs) de la racine aux sommets sont de plus stockés dans les champs v_sommets.
*/
graphe * RootTree(graphe * g, graphe * g_1, int32_t i)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "RootTree"
{
  int32_t k, s, n;
  pcell p;
  Fifo * T;
  graphe * tree;

  if ((g == NULL) || (g_1 == NULL))
  {
    fprintf(stderr,"%s: input graph not available\n", F_NAME);
    exit(0);
  }
  n = g->nsom;

  T = CreeFifoVide(n);
  if (T == NULL)
  {
    fprintf(stderr,"%s: CreeFifoVide failed\n", F_NAME);
    exit(0);
  }

  tree = InitGraphe(n, n-1);
  if (tree == NULL)
  {
    fprintf(stderr,"%s: InitGraphe failed\n", F_NAME);
    exit(0);
  }

  for (k = 0; k < n; k++) tree->v_sommets[k] = -1;
  tree->v_sommets[i] = 0;
  FifoPush(T, i);

  while (!FifoVide(T))
  {
    i = FifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s successeur de i */
      s = p->som;
      if (tree->v_sommets[s] == -1) 
      {
	AjouteArcValue(tree, i, s, p->v_arc);
	tree->v_sommets[s] = tree->v_sommets[i] + 1;
	FifoPush(T, s);
      }
    } // for p
    for (p = g_1->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s prédécesseur de i */
      s = p->som;
      if (tree->v_sommets[s] == -1) 
      {
	AjouteArcValue(tree, i, s, p->v_arc);
	tree->v_sommets[s] = tree->v_sommets[i] + 1;
	FifoPush(T, s);
      }
    } // for p
  } // while (!FifoVide(T))

  FifoTermine(T);
  return tree;
} /* RootTree() */

/* ====================================================================== */
/*! \fn void DistRoot(graphe * g, int32_t r)
    \param g (entrée) : un graphe (arbre orienté ou arborescence)
    \param r (entrée) : racine de \b g
    \brief calcule, pour chaque sommet x de g, la longueur (somme des poids d'arcs) d'un plus court
           chemin de r vers x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
*/
void DistRoot(graphe * g, int32_t r)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "DistRoot"
{
  int32_t k, s, n = g->nsom;
  pcell p;
  Fifo * T = CreeFifoVide(n);

  for (k = 0; k < n; k++) g->v_sommets[k] = -1;
  g->v_sommets[r] = 0;
  FifoPush(T, r);

  while (!FifoVide(T))
  {
    r = FifoPop(T);
    for (p = g->gamma[r]; p != NULL; p = p->next)
    { /* pour tout s successeur de r */
      s = p->som;
      if (g->v_sommets[s] == -1) 
      {
        g->v_sommets[s] = g->v_sommets[r] + p->v_arc;
        FifoPush(T, s);
      }
    } // for p
  } // while (!FifoVide(T))

  FifoTermine(T);
} /* DistRoot() */

/* ====================================================================== */
/* ====================================================================== */
/* PLUS COURTS CHEMINS */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/* \fn void Dijkstra1(graphe * g, int32_t i)
    \param g (entrée) : un graphe pondéré. La pondération de chaque arc doit 
                        se trouver dans le champ \b v_arc de la structure \b cell
    \param i (entrée) : un sommet de \b g
    \brief calcule, pour chaque sommet x de g, la longueur d'un plus court
           chemin de i vers x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
*/
void Dijkstra1(graphe * g, int32_t i)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Dijkstra1"
{
  int32_t n = g->nsom;
  boolean * S = EnsembleVide(n);
  int32_t k, x, y;
  pcell p;
  TYP_VSOM vmin;

  S[i] = TRUE;
  for (k = 0; k < n; k++) g->v_sommets[k] = MAX_VSOM;
  g->v_sommets[i] = 0;
  k = 1;
  x = i;
  while ((k < n) && (g->v_sommets[x] < MAX_VSOM))
  {
    for (p = g->gamma[x]; p != NULL; p = p->next)
    { /* pour tout y successeur de x */
      y = p->som;
      if (!S[y])
      {
        g->v_sommets[y] = mcmin((g->v_sommets[y]),(g->v_sommets[x]+p->v_arc));
      } // if (!S[y])
    } // for p
    // extraire un sommet x hors de S de valeur g->v_sommets minimale
    vmin = MAX_VSOM;
    for (y = 0; y < n; y++)
      if (!S[y]) 
        if (g->v_sommets[y] <= vmin) { x = y; vmin = g->v_sommets[y]; }
    k++; 
    S[x] = TRUE;
  } // while ((k < n) && (g->v_sommets[x] < MAX_VSOM))

} /* Dijkstra1() */

/* ====================================================================== */
/*! \fn void Dijkstra(graphe * g, int32_t i)
    \param g (entrée) : un graphe pondéré. La pondération de chaque arc doit 
                        se trouver dans le champ \b v_arc de la structure \b cell
    \param i (entrée) : un sommet de \b g
    \brief calcule, pour chaque sommet x de g, la longueur d'un plus court
           chemin de i vers x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
*/
void Dijkstra(graphe * g, int32_t i)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Dijkstra"
{
  int32_t n = g->nsom;
  boolean * S = EnsembleVide(n);
  boolean * TT = EnsembleVide(n);
  int32_t k, x, y;
  pcell p;
  TYP_VSOM vmin;
  int32_t *T = ListeVide(n);
  int32_t tx, ty, tt = 0;

  if ((i >= n) || (i < 0))
  {
    fprintf(stderr, "%s: bad vertex index: %d\n", F_NAME, i);
    exit(0);
  }

  S[i] = TRUE;
  TT[i] = TRUE;
  for (k = 0; k < n; k++) g->v_sommets[k] = MAX_VSOM;
  g->v_sommets[i] = 0;
  x = i;
  do
  {
    for (p = g->gamma[x]; p != NULL; p = p->next)
    { /* pour tout y successeur de x */
      y = p->som;
      if (!S[y])
      {
        g->v_sommets[y] = mcmin((g->v_sommets[y]),(g->v_sommets[x]+p->v_arc));
        if (!TT[y]) 
	{  
	  TT[y] = TRUE;
	  T[tt] = y; // insere y dans T
	  tt++;
	}
      } // if (!S[y])
    } // for p

    // extraire un sommet x de T de valeur g->v_sommets minimale
    tx = 0; 
    x = T[0];
    vmin = g->v_sommets[x];
    for (ty = 1; ty < tt; ty++)
    {
      y = T[ty];
      if (g->v_sommets[y] <= vmin) { x = y; tx = ty; vmin = g->v_sommets[y]; }
    }
    S[x] = TRUE;
    for (; tx < tt-1; tx++) T[tx] = T[tx+1]; // retire x de T
    tt--;
  } while (tt > 0);

  free(T);
  free(S);
  free(TT);

} /* Dijkstra() */

/* ====================================================================== */
/*! \fn void Lee(graphe * g, int32_t i)
    \param g (entrée) : un graphe
    \param i (entrée) : un sommet de \b g
    \brief calcule, pour chaque sommet x de g, la longueur (nombre d'arcs) d'un plus court
           chemin de i vers x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
*/
void Lee(graphe * g, int32_t i)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "Lee"
{
  int32_t k, s, n = g->nsom;
  pcell p;
  Fifo * T = CreeFifoVide(n);

  for (k = 0; k < n; k++) g->v_sommets[k] = -1;
  g->v_sommets[i] = 0;
  FifoPush(T, i);

  while (!FifoVide(T))
  {
    i = FifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s successeur de i */
      s = p->som;
      if (g->v_sommets[s] == -1) 
      {
        g->v_sommets[s] = g->v_sommets[i] + 1;
        FifoPush(T, s);
      }
    } // for p
  } // while (!FifoVide(T))

  FifoTermine(T);
} /* Lee() */

/* ====================================================================== */
/*! \fn void LeeNO(graphe * g, graphe * g_1, int32_t i)
    \param g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique du graphe g
    \param i (entrée) : un sommet de \b g
    \brief calcule, pour chaque sommet x de g, la longueur (nombre d'arêtes) 
    d'un plus court chemin de i vers x dans le graphe non orienté représenté 
    par (g, g_1). Cette longueur est stockée dans le champ \b v_sommets de \b g
*/
void LeeNO(graphe * g, graphe * g_1, int32_t i)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "LeeNO"
{
  int32_t k, s, n = g->nsom;
  pcell p;
  Fifo * T = CreeFifoVide(n);

  for (k = 0; k < n; k++) g->v_sommets[k] = -1;
  g->v_sommets[i] = 0;
  FifoPush(T, i);

  while (!FifoVide(T))
  {
    i = FifoPop(T);
    for (p = g->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s successeur de i */
      s = p->som;
      if (g->v_sommets[s] == -1) 
      {
        g->v_sommets[s] = g->v_sommets[i] + 1;
        FifoPush(T, s);
      }
    } // for p
    for (p = g_1->gamma[i]; p != NULL; p = p->next)
    { /* pour tout s prédécesseur de i */
      s = p->som;
      if (g->v_sommets[s] == -1) // sic: g
      {
        g->v_sommets[s] = g->v_sommets[i] + 1;  // sic: g
        FifoPush(T, s);
      }
    } // for p
  } // while (!FifoVide(T))

  FifoTermine(T);
} /* LeeNO() */

/* ====================================================================== */
/*! \fn graphe * PCC(graphe * g, int32_t d, int32_t a)
    \param g (entrée) : un graphe pondéré, représenté par son application successeurs,
            et dont les sommets ont été valués par la distance au sommet \b d
    \param d (entrée) : un sommet (départ)
    \param a (entrée) : un sommet (arrivée)
    \return un plus court chemin de \b d vers \b a dans \b g , représenté par un graphe
    \brief retourne un plus court chemin de \b d vers \b a dans \b g
*/
graphe * PCC(graphe * g, int32_t d, int32_t a)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "PCC"
{
  int32_t n = g->nsom;
  graphe * pcc = InitGraphe(n, n-1); /* pour le resultat */
  graphe * g_1 = Symetrique(g);
  int32_t i, y, x = a;
  pcell p;

  while (x != d)
  {
    for (p = g_1->gamma[x]; p != NULL; p = p->next)
    { /* pour tout y predecesseur de x */
      y = p->som;
      if ((g->v_sommets[x]-g->v_sommets[y]) == p->v_arc) 
      {
        AjouteArcValue(pcc, y, x, p->v_arc);
        x = y;
        break;
      }
    } // for p
    if (p == NULL)
    {
      printf("WARNING: pas de pcc trouve de %d vers %d\n", d, a);
      TermineGraphe(g_1);
      return pcc;
    }
  }
  TermineGraphe(g_1);
  for (i = 0; i < n; i++) { pcc->x[i] = g->x[i]; pcc->y[i] = g->y[i]; }
  return pcc;
} /* PCC() */

/* ====================================================================== */
/*! \fn graphe * PCCna(graphe * g, int32_t d, int32_t a)
    \param g (entrée) : un graphe, représenté par son application successeurs,
            et dont les sommets ont été valués par la distance (nombre d'arcs) au sommet \b d
    \param d (entrée) : un sommet (départ)
    \param a (entrée) : un sommet (arrivée)
    \return un plus court chemin (au sens du nombre d'arcs) de \b d vers \b a dans \b g , 
    représenté par un graphe
    \brief retourne un plus court chemin de \b d vers \b a dans \b g
*/
graphe * PCCna(graphe * g, int32_t d, int32_t a)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "PCCna"
{
  int32_t n = g->nsom;
  graphe * pcc = InitGraphe(n, n-1); /* pour le resultat */
  graphe * g_1 = Symetrique(g);
  int32_t i, y, x = a;
  pcell p;

  while (x != d)
  {
    for (p = g_1->gamma[x]; p != NULL; p = p->next)
    { /* pour tout y predecesseur de x */
      y = p->som;
      if ((g->v_sommets[x]-g->v_sommets[y]) == 1) 
      {
        AjouteArcValue(pcc, y, x, p->v_arc);
        x = y;
        break;
      }
    } // for p
    if (p == NULL)
    {
      printf("WARNING: pas de pcc trouve \n");
      TermineGraphe(g_1);
      return pcc;
    }
  }
  TermineGraphe(g_1);
  for (i = 0; i < n; i++) { pcc->x[i] = g->x[i]; pcc->y[i] = g->y[i]; }
  return pcc;
} /* PCCna() */

/* ====================================================================== */
/* ====================================================================== */
/* GRAPHES SANS CIRCUIT (GSC) */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn void BellmanSC(graphe * g)
    \param g (entrée) : un graphe pondéré. La pondération de chaque arc doit 
                        se trouver dans le champ \b v_arc de la structure \b cell
    \brief calcule, pour chaque sommet x de g, la longueur d'un plus court
           chemin arrivant en x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
    \warning ne s'applique qu'aux graphes sans circuit
*/
void BellmanSC(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "BellmanSC"
{
  int32_t x, y, i, r, rmax, cumul, n = g->nsom;
  int32_t * T;   // pour les sommets triés
  int32_t * H;   // histogramme des rangs
  graphe * g_1 = Symetrique(g);
  TYP_VARC tmp, minv;
  pcell p;

  if (CircuitNiveaux(g))
  {
    fprintf(stderr, "%s: the graph is not acyclic\n", F_NAME);
    exit(0);
  }
  rmax = 0;
  for (x = 0; x < n; x++)
  {
    r = (int32_t)g->v_sommets[x];
    if (r > rmax) rmax = r;
  }
  H = (int32_t *)calloc(rmax + 1, sizeof(int32_t));
  if (H == NULL)
  {
    fprintf(stderr, "%s : calloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++) H[(int32_t)g->v_sommets[x]]++; // calcule l'histo
  cumul = H[0];
  H[0] = 0;
  for (i = 1; i <= rmax; i++) // calcule l'histo cumulé
  {
    x = H[i];
    H[i] = cumul;
    cumul += x;
  }
  T = (int32_t *)malloc(n * sizeof(int32_t));
  if (T == NULL)
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++)  // tri des sommets par rang croissant
  {
    r = (int32_t)g->v_sommets[x];
    T[H[r]] = x;
    H[r] += 1;
  }
  free(H);

  for (x = 0; x < n; x++)
    g->v_sommets[x] = MAX_VSOM;

  for (i = 0; i < n; i++)
  {
    x = T[i];
    p = g_1->gamma[x];
    if (p == NULL) g->v_sommets[x] = 0;
    else
    {
      minv = MAX_VARC; 
      for (; p != NULL; p = p->next)
      { /* pour tout y prédécesseur de x */
	y = p->som;
	tmp = p->v_arc + g->v_sommets[y];
	if (tmp < minv) minv = tmp;
      } // for p
      g->v_sommets[x] = minv;
    }
  }

  TermineGraphe(g_1);  
  free(T);
} /* BellmanSC() */

/* ====================================================================== */
/*! \fn void BellmanSCmax(graphe * g)
    \param g (entrée) : un graphe pondéré. La pondération de chaque arc doit 
                        se trouver dans le champ \b v_arc de la structure \b cell
    \brief calcule, pour chaque sommet x de g, la longueur d'un plus long
           chemin arrivant en x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
    \warning ne s'applique qu'aux graphes sans circuit
*/
void BellmanSCmax(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "BellmanSCmax"
{
  int32_t x, y, i, r, rmax, cumul, n = g->nsom;
  int32_t * T;   // pour les sommets triés
  int32_t * H;   // histogramme des rangs
  graphe * g_1 = Symetrique(g);
  TYP_VARC tmp, maxv;
  pcell p;

  if (CircuitNiveaux(g))
  {
    fprintf(stderr, "%s: the graph is not acyclic\n", F_NAME);
    exit(0);
  }
  rmax = 0;
  for (x = 0; x < n; x++)
  {
    r = (int32_t)g->v_sommets[x];
    if (r > rmax) rmax = r;
  }
  H = (int32_t *)calloc(rmax + 1, sizeof(int32_t));
  if (H == NULL)
  {
    fprintf(stderr, "%s : calloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++) H[(int32_t)g->v_sommets[x]]++; // calcule l'histo
  cumul = H[0];
  H[0] = 0;
  for (i = 1; i <= rmax; i++) // calcule l'histo cumulé
  {
    x = H[i];
    H[i] = cumul;
    cumul += x;
  }
  T = (int32_t *)malloc(n * sizeof(int32_t));
  if (T == NULL)
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++)  // tri des sommets par rang croissant
  {
    r = (int32_t)g->v_sommets[x];
    T[H[r]] = x;
    H[r] += 1;
  }
  free(H);

  for (x = 0; x < n; x++)
    g->v_sommets[x] = MAX_VSOM;

  for (i = 0; i < n; i++)
  {
    x = T[i];
    p = g_1->gamma[x];
    if (p == NULL) g->v_sommets[x] = 0;
    else
    {
      maxv = MIN_VARC; 
      for (; p != NULL; p = p->next)
      { /* pour tout y prédécesseur de x */
	y = p->som;
	tmp = p->v_arc + g->v_sommets[y];
	if (tmp > maxv) maxv = tmp;
      } // for p
      g->v_sommets[x] = maxv;
    }
  }

  TermineGraphe(g_1);  
  free(T);
} /* BellmanSCmax() */

/* ====================================================================== */
/*! \fn void BellmanSC1(graphe * g, int32_t dep)
    \param g (entrée) : un graphe pondéré. La pondération de chaque arc doit 
                        se trouver dans le champ \b v_arc de la structure \b cell
    \param dep (entrée) : un sommet de \b g
    \brief calcule, pour chaque sommet x de g, la longueur d'un plus court
           chemin de dep vers x. Cette longueur est stockée dans le champ
           \b v_sommets de \b g
    \warning ne s'applique qu'aux graphes sans circuit
*/
/*
  ATTENTION : PAS TESTE
*/
void BellmanSC1(graphe * g, int32_t dep)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "BellmanSC1"
{
  int32_t x, y, i, r, rmax, cumul, n = g->nsom;
  int32_t * T;   // pour les sommets triés
  int32_t * H;   // histogramme des rangs
  graphe * g_1 = Symetrique(g);
  TYP_VARC tmp, minv;
  pcell p;

  if (CircuitNiveaux(g))
  {
    fprintf(stderr, "%s: the graph is not acyclic\n", F_NAME);
    exit(0);
  }
  rmax = 0;
  for (x = 0; x < n; x++)
  {
    r = (int32_t)g->v_sommets[x];
    if (r > rmax) rmax = r;
  }
  H = (int32_t *)calloc(rmax + 1, sizeof(int32_t));
  if (H == NULL)
  {
    fprintf(stderr, "%s : calloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++) H[(int32_t)g->v_sommets[x]]++; // calcule l'histo
  cumul = H[0];
  H[0] = 0;
  for (i = 1; i <= rmax; i++) // calcule l'histo cumulé
  {
    x = H[i];
    H[i] = cumul;
    cumul += x;
  }
  T = (int32_t *)malloc(n * sizeof(int32_t));
  if (T == NULL)
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++)  // tri des sommets par rang croissant
  {
    r = (int32_t)g->v_sommets[x];
    T[H[r]] = x;
    H[r] += 1;
  }
  free(H);

  for (x = 0; x < n; x++)
    g->v_sommets[x] = MAX_VSOM;

  for (i = 0; i < n; i++) if (T[i] == dep) break;
  g->v_sommets[dep] = 0;
  for (; i < n; i++)
  {
    x = T[i];
    minv = MAX_VARC; 
    for (p = g_1->gamma[x]; p != NULL; p = p->next)
    { /* pour tout y prédécesseur de x */
      y = p->som;
      tmp = p->v_arc + g->v_sommets[y];
      if (tmp < minv) minv = tmp;
    } // for p
    g->v_sommets[x] = minv;
  }

  TermineGraphe(g_1);  
  free(T);
} /* BellmanSC1() */

static TYP_VSOM IntegreGSC_Aux(graphe * g, graphe * g_1, int32_t s)
{
  pcell p;
  int32_t t;
  TYP_VSOM sum;

  if (g_1->v_sommets[s] == (TYP_VSOM)1) return g->v_sommets[s];
  sum = g->v_sommets[s];
  for (p = g_1->gamma[s]; p != NULL; p = p->next)
  {
    t = p->som;
    sum += IntegreGSC_Aux(g, g_1, t) / g_1->x[t];
  }
  g->v_sommets[s] = sum;
  g_1->v_sommets[s] = (TYP_VSOM)1;
  return sum;
}

/* ====================================================================== */
/*! \fn void IntegreGSC(graphe * g)
    \param g (entrée) : un graphe sans circuit orienté et pondéré, 
    représenté par son application successeurs, et dont les sommets sont valués.
    \brief pour chaque sommet x, la nouvelle pondération est la somme 
    des pondérations des ancêtres de x dans la forêt valuée d'origine.
    \warning cette fonction ne fonctionne que si le graphe g est sans circuit.
    Aucune vérification n'est faite.
*/
void IntegreGSC(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "IntegreGSC"
{
  int32_t s, n = g->nsom;
  double d;
  graphe * g_1 = Symetrique(g);
  pcell p;

  // ce champ de g_1 sera utilisé comme indicateur: 0=non traité, 1=traité
  for (s = 0; s < n; s++) g_1->v_sommets[s] = (TYP_VSOM)0;
  // ce champ de g_1 sera utilisé pour stocker le degré sortant (pour g) de s 
  for (s = 0; s < n; s++)
  {  
    for (d = 0, p = g->gamma[s]; p != NULL; p = p->next) d += 1;
    g_1->x[s] = d;
  }
  // lance la fonction récursive pour tous les sommets
  for (s = 0; s < n; s++)
    if (g_1->v_sommets[s] == (TYP_VSOM)0)
      IntegreGSC_Aux(g, g_1, s);
  TermineGraphe(g_1);
} /* IntegreGSC() */

/* ====================================================================== */
/*! \fn boolean EstConfluent(graphe * g, graphe *g_1, int32_t a)
    \param g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique de g
    \param a (entrée) : un sommet du graphe g
    \brief retourne VRAI si le sommet a est confluent, ie., si 
      Succ(Anc(a)\{a}) est inclus dans Anc(a)
*/
boolean EstConfluent(graphe * g, graphe *g_1, int32_t a)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "EstConfluent"
{
  boolean * A;   /* pour les "ascendants" (predecesseurs a N niveaux) */
  Lifo * T;   /* liste temporaire geree en pile (Last In, First Out) */
  int32_t i, n, s;
  pcell p;
  boolean ret = TRUE;

  n = g->nsom;
  T = CreeLifoVide(n);
  A = EnsembleVide(n);

  /* calcule les ascendants (stricts) de a */
  LifoPush(T, a);
  while (!LifoVide(T))
  {
    i = LifoPop(T);
    for (p = g_1->gamma[i]; p != NULL; p = p->next) 
    { /* pour tout s prédécesseur de i */
      s = p->som;
      if (!A[s]) 
      {
        A[s] = TRUE;
        LifoPush(T, s);
      }
    } // for p
  } // while (!LifoVide(T))

  // vérification de la prop.
  for (i = 0; i < n; i++) 
    if (A[i])
      for (p = g->gamma[i]; p != NULL; p = p->next) 
      { /* pour tout s successeur de i */
	s = p->som;
	if (!A[s] && (s != a)) { ret = FALSE; goto fin; }
      }

 fin:    
  free(A);
  LifoTermine(T);
  return ret;
} // EstConfluent()

/* ====================================================================== */
/*! \fn void PointsConfluents(graphe * g, graphe *g_1)
    \param g (entrée) : un graphe
    \param g_1 (entrée) : le graphe symétrique de g
    \brief marque l'ensemble des sommets confluents de g (utilise v_sommets)
*/
void PointsConfluents(graphe * g, graphe *g_1)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "EstConfluent"
{
  int32_t i, n;
  n = g->nsom;
  for (i = 0; i < n; i++) 
    if (EstConfluent(g, g_1, i))
      g->v_sommets[i] = (TYP_VSOM)1;
    else
      g->v_sommets[i] = (TYP_VSOM)0;
} // PointsConfluents()

/* ====================================================================== */
/* ====================================================================== */
/* FORETS */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn graphe * ForetPCC(graphe * g)
    \param g (entrée) : un graphe, représenté par son application successeurs et dont les arcs sont valués
    \return une forêt de plus courts chemins relatifs aux sources de g
    \brief retourne une forêt de plus courts chemins relatifs aux sources de g. Les longueurs des plus courts chemins sont stockés dans le champ v_sommet du graphe d'origine et de la forêt générée.
    \warning ne s'applique qu'aux graphes sans circuit
*/
graphe * ForetPCC(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "ForetPCC"
{
  int32_t x, y, miny, i, r, rmax, cumul, n = g->nsom;
  int32_t * T;   // pour les sommets triés
  int32_t * H;   // histogramme des rangs
  graphe * g_1 = Symetrique(g);
  graphe * foret = InitGraphe(n, n-1); // pour le resultat
  TYP_VARC tmp, minv;
  pcell p;

  assert (g_1 != NULL);
  assert (foret != NULL);

  if (CircuitNiveaux(g) != 0)
  {
    fprintf(stderr, "%s: the graph is not acyclic\n", F_NAME);
    exit(0);
  }
  rmax = 0;
  for (x = 0; x < n; x++)
  {
    r = (int32_t)g->v_sommets[x];
    if (r > rmax) rmax = r;
  }
  H = (int32_t *)calloc(rmax + 1, sizeof(int32_t));
  if (H == NULL)
  {
    fprintf(stderr, "%s : calloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++) H[(int32_t)g->v_sommets[x]]++; // calcule l'histo
  cumul = H[0];
  H[0] = 0;
  for (i = 1; i <= rmax; i++) // calcule l'histo cumulé
  {
    x = H[i];
    H[i] = cumul;
    cumul += x;
  }
  T = (int32_t *)malloc(n * sizeof(int32_t));
  if (T == NULL)
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  for (x = 0; x < n; x++)  // tri des sommets par rang croissant
  {
    r = (int32_t)g->v_sommets[x];
    T[H[r]] = x;
    H[r] += 1;
  }
  free(H);

  for (x = 0; x < n; x++)
    g->v_sommets[x] = MAX_VSOM;

  for (i = 0; i < n; i++)
  {
    x = T[i];
    p = g_1->gamma[x];
    if (p == NULL) foret->v_sommets[x] = g->v_sommets[x] = 0; // cas des sources
    else
    {
      miny = p->som;
      minv = p->v_arc + g->v_sommets[miny];
      p = p->next; 
      for (; p != NULL; p = p->next)
      { /* pour tout y prédécesseur de x */
	y = p->som;
	tmp = p->v_arc + g->v_sommets[y];
	if (tmp < minv) { minv = tmp; miny = y; }
      } // for p
      foret->v_sommets[x] = g->v_sommets[x] = minv;
      AjouteArc(foret, miny, x);
    }
  }

  TermineGraphe(g_1);  
  free(T);
  return foret;
} /* ForetPCC() */

/* ====================================================================== */
/*! \fn graphe * ForetPCC_PoidsEntiers(graphe * g)
    \param g (entrée) : un graphe pondéré, représenté par son application successeurs,
            et dont les sommets ont été valués par la distance à une source
    \return une forêt de plus courts chemins relatifs aux sources de g
    \brief retourne une forêt de plus courts chemins relatifs aux sources de g
    \warning convient seulement aux pondérations entières 
*/
graphe * ForetPCC_PoidsEntiers(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "ForetPCC_PoidsEntiers"
{
  int32_t n = g->nsom;
  graphe * fpcc = InitGraphe(n, n-1); /* pour le resultat */
  graphe * g_1 = Symetrique(g);
  int32_t i, y, x;
  pcell p;

  for (x = 0; x < n; x++)
  {
    for (p = g_1->gamma[x]; p != NULL; p = p->next)
    { /* pour tout y predecesseur de x */
      y = p->som;
      if ((g->v_sommets[x]-g->v_sommets[y]) == p->v_arc) 
      {
        AjouteArcValue(fpcc, y, x, p->v_arc);
        break;
      }
    } // for p
  }
  TermineGraphe(g_1);
  for (i = 0; i < n; i++) 
  { 
    fpcc->x[i] = g->x[i]; 
    fpcc->y[i] = g->y[i]; 
    fpcc->v_sommets[i] = g->v_sommets[i];
  }
  return fpcc;
} /* ForetPCC_PoidsEntiers() */

static TYP_VSOM IntegreForet_Aux(graphe * g, graphe * g_1, int32_t s)
{
  pcell p;
  int32_t y;
  TYP_VSOM sum = g->v_sommets[s];
  for (p = g_1->gamma[s]; p != NULL; p = p->next)
  {
    y = p->som;
    sum += IntegreForet_Aux(g, g_1, y);
  }
  g->v_sommets[s] = sum;
  return sum;
}

/* ====================================================================== */
/*! \fn void IntegreForet(graphe * g)
    \param g (entrée) : un graphe (forêt) orienté et pondéré, représenté par son application successeurs, et dont les sommets sont valués
    \brief pour chaque sommet x, la nouvelle pondération est la somme des pondérations des ancêtres de x dans la forêt valuée d'origine
*/
void IntegreForet(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "IntegreForet"
{
  int32_t n = g->nsom;
  int32_t x;
  graphe * g_1 = Symetrique(g);

  for (x = 0; x < n; x++)
    if (g->gamma[x] == NULL)
      IntegreForet_Aux(g, g_1, x);
  TermineGraphe(g_1);
} /* IntegreForet() */

/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS D'AFFICHAGE */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn void AfficheEnsemble(boolean *s, int32_t n)
    \brief affiche à l'écran l'ensemble representé par le tableau de booléens s.
    \param s (entrée) : un tableau de valeurs booléennes.
    \param n (entrée) : la taille du tableau. 
*/
void AfficheEnsemble(boolean *s, int32_t n)
/* ====================================================================== */
{
  int32_t i;
  for (i = 0; i < n; i++) if (s[i]) printf("%d ", i);
  printf("\n");
} /* AfficheEnsemble() */

/* ====================================================================== */
/*! \fn void AfficheListe(pcell p) 
    \param p (entrée) : une liste chaînee de successeurs.
    \brief affiche le contenu de la liste p. */
void AfficheListe(pcell p) 
/* ====================================================================== */
{
  for (; p != NULL; p = p->next) printf("%d (%g); ", p->som, (double)(p->v_arc));
  printf("\n");
} /* AfficheListe() */

/* ====================================================================== */
/*! \fn void AfficheSuccesseurs(graphe * g) 
    \param   g (entrée) : un graphe.
    \brief affiche le graphe dans sa représentation "listes de successeurs". 
*/
void AfficheSuccesseurs(graphe * g) 
/* ====================================================================== */
#undef F_NAME
#define F_NAME "AfficheSuccesseurs"
{
  int32_t i;
  
  if (g->gamma)
  {
    for (i = 0; i < g->nsom; i++)
    {
      printf("[%d] ", i);
      AfficheListe(g->gamma[i]);
    }
    printf("\n");
  }
  else fprintf(stderr, "%s: representation successeurs absente\n", F_NAME);
} /* AfficheSuccesseurs() */

/* ====================================================================== */
/*! \fn void AfficheArcs(graphe * g)
    \param   g (entrée) : un graphe.
    \brief affiche le graphe dans sa représentation "listes d'arcs". 
*/
void AfficheArcs(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "AfficheArcs"
{
  int32_t i;

  if (g->tete && g->v_arcs)
  {
    for (i = 0; i < g->narc; i++)
      printf("[%d, %d] %g\n", g->tete[i], g->queue[i], (double)(g->v_arcs[i]));
    printf("\n");
  }
  else if (g->tete)
  {
    for (i = 0; i < g->narc; i++)
      printf("[%d, %d]\n", g->tete[i], g->queue[i]);
    printf("\n");
  }
  else fprintf(stderr, "%s: representation arcs absente\n", F_NAME);
} /* AfficheArcs() */

/* ====================================================================== */
/*! \fn void AfficheValeursSommets(graphe * g)
    \param   g (entrée) : un graphe.
    \brief affiche les valeurs associées aux sommets. 
*/
void AfficheValeursSommets(graphe * g)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "AfficheValeursSommets"
{
  int32_t i;

  if (g->v_sommets)
  {
    for (i = 0; i < g->nsom; i++)
      printf("sommet %d : valeur %g\n", i, (double)(g->v_sommets[i]));
    printf("\n");
  }
  else fprintf(stderr, "%s: valeurs sommets absentes\n", F_NAME);
} /* AfficheValeursSommets() */


/* ====================================================================== */
/* ====================================================================== */
/* GENERATION POSTSCRIPT */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn void PSGraphe(graphe * g, char *filename, double r, double t, double marge) 
    \param g (entrée) : un graphe.
    \param filename (entrée) : nom du fichier postscript à générer.
    \param r (entrée) : rayon des cercles qui représentent les sommets (0 pour ne pas les dessiner).
    \param t (entrée) : taille (demi-longueur) des flèches pour les arcs (0 pour ne pas les dessiner).
    \param marge (entrée) : marge en x et en y.
    \brief génère une figure PostScript d'après la représentation "successeurs" du graphe g. 
*/
void PSGraphe(graphe * g, char *filename, double r, double t, double marge) 
/* ====================================================================== */
#undef F_NAME
#define F_NAME "PSGraphe"
{
  int32_t i, j, n = g->nsom;
  double xmin, xmax, ymin, ymax;
  double x1, y1, x2, y2, x3, y3, x, y, a, b, d;
  pcell p;
  FILE * fd = NULL;
  
  if (g->gamma == NULL) 
  {  fprintf(stderr, "%s: representation successeurs absente\n", F_NAME);
     return;
  }
  
  if (g->x == NULL) 
  {  fprintf(stderr, "%s: coordonnees des sommets absentes\n", F_NAME);
     return;
  }

  fd = fopen(filename,"w");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return;
  }

  /* determine le rectangle englobant et genere le header */
  xmin = xmax = g->x[0];
  ymin = ymax = g->y[0];
  for (i = 1; i < n; i++) 
  {
    if (g->x[i] < xmin) xmin = g->x[i]; else if (g->x[i] > xmax) xmax = g->x[i];
    if (g->y[i] < ymin) ymin = g->y[i]; else if (g->y[i] > ymax) ymax = g->y[i];
  }
  EPSHeader(fd, xmax - xmin + 2.0 * marge, ymax - ymin + 2.0 * marge, 1.0, 14);
  
  /* dessine les sommets */
  for (i = 0; i < n; i++) 
    PSDrawcircle(fd, g->x[i]-xmin+marge, g->y[i]-ymin+marge, r);
  if (g->nomsommet)
    for (i = 0; i < n; i++) 
      PSString(fd, g->x[i]-xmin+marge+2*r, g->y[i]-ymin+marge+2*r, g->nomsommet[i]);

  /* dessine les arcs */
  if (r > 0.0)
  for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      PSLine(fd, g->x[i]-xmin+marge, g->y[i]-ymin+marge, g->x[j]-xmin+marge, g->y[j]-ymin+marge);
    }

  /* dessine les fleches sur les arcs */
  if (t > 0.0)
  for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      x1 = g->x[i]-xmin+marge;
      y1 = g->y[i]-ymin+marge;
      x2 = g->x[j]-xmin+marge;
      y2 = g->y[j]-ymin+marge;
      x = (x2 + x1) / 2;
      y = (y2 + y1) / 2;
      a = x2 - x1;
      b = y2 - y1;             /* (a,b) est un vecteur directeur de l'arc */
      d = sqrt(a * a + b * b); /* longueur de l'arc */
      if (d > 1) // sinon on ne dessine rien
      { 
        a /= d; b /= d;          /* norme le vecteur */
        x1 = x + 2 * t * a;
        y1 = y + 2 * t * b;      /* pointe de la fleche */
        x2 = x - 2 * t * a;
        y2 = y - 2 * t * b;      /* base de la fleche */
        x3 = x2 + t * -b;        /* (-b,a) est normal a (a,b) */
        y3 = y2 + t * a;
        x2 = x2 - t * -b;
        y2 = y2 - t * a;
        PSLine(fd, x1, y1, x2, y2);
        PSLine(fd, x2, y2, x3, y3);
        PSLine(fd, x3, y3, x1, y1);
      }
    }
  
  PSFooter(fd);
  fclose(fd);
} /* PSGraphe() */

/* ====================================================================== */
/*! \fn void EPSGraphe(graphe * g, char *filename, double s, double r, double t, double marge, int32_t noms_sommets, int32_t v_sommets, int32_t col_sommets, int32_t v_arcs ) 
    \param g (entrée) : un graphe.
    \param filename (entrée) : nom du fichier postscript à générer.
    \param s (entrée) : facteur d'échelle à appliquer aux coordonnées des sommets. 
    \param r (entrée) : rayon des cercles qui représentent les sommets (0 pour ne pas les dessiner).
    \param t (entrée) : taille (demi-longueur) des flèches pour les arcs (0 pour ne pas les dessiner).
    \param marge (entrée) : marge en x et en y.
    \param noms_sommets (entrée) : booléen indiquant s'il faut écrire les noms des sommets.
    \param v_sommets (entrée) : booléen indiquant s'il faut écrire les valeurs des sommets.
    \param col_sommets (entrée) : booléen indiquant s'il faut colorier les sommets.
    \param v_arcs (entrée) : booléen indiquant s'il faut écrire les valeurs des arcs.
    \brief génère une figure PostScript d'après la représentation "successeurs" du graphe g. 
*/
void EPSGraphe(graphe * g, char *filename, double s, double r, double t, double marge, int32_t noms_sommets, int32_t v_sommets, int32_t col_sommets, int32_t v_arcs) 
/* ====================================================================== */
#undef F_NAME
#define F_NAME "EPSGraphe"
{
  int32_t i, j, n = g->nsom;
  double xmin, xmax, ymin, ymax;
  double x1, y1, x2, y2, x3, y3, x, y, a, b, d;
  pcell p;
  FILE * fd = NULL;
  char buf[80];
  
  if (g->gamma == NULL) 
  {  fprintf(stderr, "%s: representation successeurs absente\n", F_NAME);
     return;
  }
  
  if (g->x == NULL) 
  {  fprintf(stderr, "%s: coordonnees des sommets absentes\n", F_NAME);
     return;
  }

  fd = fopen(filename,"w");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return;
  }

  /* determine le rectangle englobant et genere le header */
  xmin = xmax = s*g->x[0];
  ymin = ymax = s*g->y[0];
  for (i = 1; i < n; i++) 
  {
    if (s*g->x[i] < xmin) xmin = s*g->x[i]; else if (s*g->x[i] > xmax) xmax = s*g->x[i];
    if (s*g->y[i] < ymin) ymin = s*g->y[i]; else if (s*g->y[i] > ymax) ymax = s*g->y[i];
  }
  EPSHeader(fd, xmax - xmin + 2.0 * marge, ymax - ymin + 2.0 * marge, 1.0, 14);
  
  /* dessine le fond */
  PSSetColor (fd, 255);
  PSDrawRect (fd, 0, 0, xmax - xmin + 2.0 * marge, ymax - ymin + 2.0 * marge);
  PSSetColor (fd, 0);

  /* dessine les sommets */
  if (r > 0.0)
  for (i = 0; i < n; i++) 
    if (col_sommets && (g->v_sommets[i] != 0)) 
      PSDrawdisc(fd, s*g->x[i]-xmin+marge, s*g->y[i]-ymin+marge, r);
    else
      PSDrawcircle(fd, s*g->x[i]-xmin+marge, s*g->y[i]-ymin+marge, r);

  if (noms_sommets && g->nomsommet)
    for (i = 0; i < n; i++) 
      PSString(fd, s*g->x[i]-xmin+marge+2*r, s*g->y[i]-ymin+marge-2*r, g->nomsommet[i]);
  if (v_sommets)
    for (i = 0; i < n; i++) 
    {
      sprintf(buf, "%g", (double)(g->v_sommets[i]));      
      PSString(fd, s*g->x[i]-xmin+marge+2*r, s*g->y[i]-ymin+marge+2*r, buf);
    }

  /* dessine les arcs */
  for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      PSLine(fd, s*g->x[i]-xmin+marge, s*g->y[i]-ymin+marge, s*g->x[j]-xmin+marge, s*g->y[j]-ymin+marge);
    }

  /* dessine les fleches sur les arcs */
  if (t > 0.0)
  {
    for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      x1 = s*g->x[i]-xmin+marge;
      y1 = s*g->y[i]-ymin+marge;
      x2 = s*g->x[j]-xmin+marge;
      y2 = s*g->y[j]-ymin+marge;
      x = (x2 + x1) / 2; // milieu de l'arc
      y = (y2 + y1) / 2;
      if (v_arcs)
      {
        sprintf(buf, "%g", (double)(p->v_arc));      
        PSString(fd, x + r, y, buf);
      }
      a = x2 - x1;
      b = y2 - y1;             /* (a,b) est un vecteur directeur de l'arc */
      d = sqrt(a * a + b * b); /* longueur de l'arc */
      if (d > 1) // sinon on ne dessine pas la fleche
      { 
        a /= d; b /= d;          /* norme le vecteur */
        x1 = x + 2 * t * a;
        y1 = y + 2 * t * b;      /* pointe de la fleche */
        x2 = x - 2 * t * a;
        y2 = y - 2 * t * b;      /* base de la fleche */
        x3 = x2 + t * -b;        /* (-b,a) est normal a (a,b) */
        y3 = y2 + t * a;
        x2 = x2 - t * -b;
        y2 = y2 - t * a;
        PSLine(fd, x1, y1, x2, y2);
        PSLine(fd, x2, y2, x3, y3);
        PSLine(fd, x3, y3, x1, y1);
      }
    }
  }
  else if (v_arcs)
  {
    for (i = 0; i < n; i++) 
    for (p = g->gamma[i]; p != NULL; p = p->next)
    {
      j = p->som;
      x1 = s*g->x[i]-xmin+marge;
      y1 = s*g->y[i]-ymin+marge;
      x2 = s*g->x[j]-xmin+marge;
      y2 = s*g->y[j]-ymin+marge;
      x = (x2 + x1) / 2; // milieu de l'arc
      y = (y2 + y1) / 2;
      sprintf(buf, "%g", (double)(p->v_arc));      
      PSString(fd, x, y, buf);
    }
  }
  
  PSFooter(fd);
  fclose(fd);
} /* EPSGraphe() */


/* ====================================================================== */
/* ====================================================================== */
/* PROGRAMMES DE TEST */
/* ====================================================================== */
/* ====================================================================== */

#ifdef TESTKRUSKAL
int32_t main(int32_t argc, char **argv)
{
  graphe * g, *g_1, *a;
  int32_t s1, s2, na, ns;
  boolean *Cs;

  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <nombre sommets> <nombre arcs>\n", argv[0]);
    exit(0);
  }

  ns = atoi(argv[1]);
  na = atoi(argv[2]);
  
  g = GrapheAleatoire(ns, na);
  AfficheSuccesseurs(g);
  AfficheArcs(g);
  g_1 = Symetrique(g);
  AfficheSuccesseurs(g_1);
  AfficheArcs(g_1);

  if (Connexe(g, g_1))
  {
    printf("graphe connexe\n");
    a = Kruskal2(g, g_1);
    AfficheSuccesseurs(a);
    AfficheArcs(a);
    TermineGraphe(a);
  }
  else printf("graphe NON connexe\n");

  do
  {
    printf("entrer un sommet : ");
    scanf("%d", &s1);
    if (s1 >= 0) 
    {
      Cs = CompFortConnexe(g, g_1, s1);
      AfficheEnsemble(Cs, g->nsom);
      free(Cs);
    }
  } while (s1 >= 0);

  do
  {
    printf("entrer un sommet : ");
    scanf("%d", &s1);
    if (s1 >= 0) 
    {
      Cs = CompConnexe(g, g_1, s1);
      AfficheEnsemble(Cs, g->nsom);
      free(Cs);
    }
  } while (s1 >= 0);

  TermineGraphe(g);
  TermineGraphe(g_1);

} /* main() */
#endif

#ifdef TESTCIRCUITNIVEAUX
int32_t main(int32_t argc, char **argv)
{
  graphe * g;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <filename>\n\n", argv[0]);
    exit(0);
  }

  g = ReadGraphe(argv[1]);  /* lit le graphe a partir du fichier */
  AfficheSuccesseurs(g);    /* affiche les ensembles "successeurs" a l'ecran */

  if (CircuitNiveaux(g))
    printf("Le graphe contient un circuit\n");
  else
    AfficheValeursSommets(g);

  TermineGraphe(g);
  return 0;
} /* main() */
#endif

#ifdef TESTBELLMANSC
int32_t main(int32_t argc, char **argv)
{
  graphe * g;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <filename>\n\n", argv[0]);
    exit(0);
  }

  g = ReadGraphe(argv[1]);  /* lit le graphe a partir du fichier */
  AfficheSuccesseurs(g);    /* affiche les ensembles "successeurs" a l'ecran */

  BellmanSC(g);
  AfficheValeursSommets(g);

  TermineGraphe(g);
  return 0;
} /* main() */
#endif

#ifdef TEST
int32_t main(int32_t argc, char **argv)
{
  graphe * g;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <filename>\n\n", argv[0]);
    exit(0);
  }

  g = ReadGraphe(argv[1]);  /* lit le graphe a partir du fichier */
  AfficheSuccesseurs(g);    /* affiche les ensembles "successeurs" a l'ecran */
  AfficheValeursSommets(g);

  IntegreForet(g);
  AfficheValeursSommets(g);

  TermineGraphe(g);
  return 0;
} /* main() */
#endif

#ifdef TESTFORETPCC
int32_t main(int32_t argc, char **argv)
{
  graphe * g;
  graphe * f;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <filename>\n\n", argv[0]);
    exit(0);
  }

  g = ReadGraphe(argv[1]);  /* lit le graphe a partir du fichier */
  AfficheSuccesseurs(g);    /* affiche les ensembles "successeurs" a l'ecran */

  f = ForetPCC(g);
  AfficheSuccesseurs(f);
  AfficheValeursSommets(f);

  TermineGraphe(g);
  TermineGraphe(f);
  return 0;
} /* main() */
#endif
