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
/* arbre des composantes "a la Ronald Jones" */
/* variante mc : utilise un seul tableau auxiliaire de la taille de l'image */
/* Michel Couprie - avril 1998 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcfah.h>
#include <mcindic.h>
#include <mctree.h>
#include <mcutil.h>
#include <ljones.h>

#define LOCMAX 0
#define TRAITE 1
#define ENFAH  2

#define PARANO
#define VERBOSE
/*
#define DEBUG
*/
/* ==================================== */
int32_t ljones(struct xvimage *image, int32_t connex)
/* ==================================== */
{
  register int32_t i, k, l;        /* index muet */
  register int32_t w, x, y, z;     /* index muet de pixel */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  TreeCell ** M;               /* l'image de pointeurs sur les cellules de l'arbre */
  TreeCell * cell;             /* une cellule */
  TreeCell * ocell;            /* une cellule */
  TreeCell * fcell;            /* une cellule */
  int32_t nmaxima;                 /* nombre de maxima differents */
  TreeCell ** tabmax;          /* le tableau des maxima tries par ordre decroissant */
  int32_t histomax[256];           /* histogramme des maxima (pour le tri par denombrement) */
  Lifo * LIFO;
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t incr_vois;
  int32_t maxi;
  int32_t level;
  char buf[1024];
  
  if (depth(image) != 1) 
  {
    fprintf(stderr, "ljones: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "ljones: mauvaise connexite: %d\n", connex);
      return 0;
  } /* switch (connex) */

  IndicsInit(N);
  TreeInit();
  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "ljones() : CreeLifoVide failed\n");
      return(0);
  }

  /* =================================================== */
  /* CALCUL MAXIMA ET INIT FEUILLES */
  /* =================================================== */

  M = (TreeCell **)calloc(1,N * sizeof(TreeCell *));
  if (M == NULL)
  {   fprintf(stderr, "ljones() : malloc failed for M\n");
      return(0);
  }

  /* M initialement est mis a NULL */
  for (x = 0; x < N; x++) M[x] = NULL;

  nmaxima = 0;
  cell = AllocTreeCell();

  for (x = 0; x < N; x++)
  {
    if (!IsSet(x, TRAITE))       /* on trouve un point x non traite */
    {
      nmaxima += 1;
      M[x] = cell;
      LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
      maxi = 1;
      ocell = cell;
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
        Set(w, TRAITE);
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if (y != -1)
          {
            if (maxi && (F[y] > F[w]))
            { /* w non dans un maximum */
              ocell = NULL;
              maxi = 0;
              nmaxima -= 1;
              M[w] = ocell;
              LifoPush(LIFO, w);
            } 
            else
            if (F[y] == F[w])
            {
              if ((maxi && (M[y] == NULL)) ||
                  ((!maxi) && (M[y] != NULL)))
              {
                M[y] = ocell;
                LifoPush(LIFO, y);
              } /* if .. */
            } /* if F ... */
          } /* if (y != -1) */
        } /* for k ... */
      } /* while (! LifoVide(LIFO)) */
      if (maxi) 
      {
        Set(x, LOCMAX);
        cell->key = x;             
        cell->father = NULL;
        cell = AllocTreeCell();  /* la cellule a ete utilisee */
      }
    } /* if (M[x] == -1) */
  } /* for (x = 0; x < N; x++) */
  LifoTermine(LIFO);
#ifdef VERBOSE
      fprintf(stderr, "ETIQUETAGE MAXIMA TERMINE - %d maxima\n", nmaxima);
#endif

  /* =================================================== */
  /* TRI DES MAXIMA (tri par denombrement) */
  /* =================================================== */

  tabmax = (TreeCell **)calloc(1,nmaxima * sizeof(TreeCell *));
  if (tabmax == NULL)
  {   fprintf(stderr, "ljones() : malloc failed for tabmax\n");
      return(0);
  }

  for (x = 0; x < 256; x++) histomax[x] = 0;
  for (x = 0; x < N; x++)
    if (IsSet(x, LOCMAX))
      histomax[F[x]]++;     /* histogramme calcule */
  maxi = 0;  
  for (x = 255; x >= 0; x--)
  {
    histomax[x] += maxi;
    maxi = histomax[x];
  }                         /* histogramme cumule inverse calcule */
  for (x = 0; x < N; x++)
    if (IsSet(x, LOCMAX))
    {
      i = histomax[F[x]];
      histomax[F[x]]--;
      tabmax[i-1] = M[x];
    }                       /* tri par denombrement calcule */

#ifdef VERBOSE
printf("TRI DES MAXIMA TERMINE\n");
#endif
#ifdef DEBUG
for (x = 0; x < nmaxima; x++) printf("%d\n", tabmax[x]->key);
#endif
  
  /* =================================================== */
  /* BOUCLE SUR LES MAXIMA  */
  /* =================================================== */

  FAH = CreeFahVide(N);
  if (FAH == NULL)
  {   fprintf(stderr, "ljones() : CreeFah failed\n");
      return(0);
  }

  x = tabmax[0]->key;    /* le premier maximum est traite de facon particuliere */
#ifdef DEBUG
printf("premier maximum : %d\n", x);
#endif
  cell = M[x];
  level = F[x];
  FahPush(FAH, x, NDG_MAX - level);
  Set(x, ENFAH);
  while (!FahVide(FAH))
  {
    w = FahPop(FAH);
#ifdef DEBUG
printf("pop %d(%d) ; level = %d\n", w, F[w], level);
#endif
    if (F[w] >= level)
    {
      M[w] = cell;
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(w, k, rs, N);
        if ((y != -1) && (!IsSet(y, ENFAH)))
        {
          Set(y, ENFAH);
          FahPush(FAH, y, NDG_MAX - F[y]);
	}
      } /* for k ... */
    } /* if ((F[w] == level) || ((F[w] > level) && (M[w] == NULL))) */
    else /* F[w] < level */
    {
      level = F[w];
      fcell = cell;
      cell = AllocTreeCell();
      cell->key = w;
      cell->father = NULL;
      fcell->father = cell;
      M[w] = cell;
#ifdef DEBUG
printf("branche arbre : %d(%d) -> %d(%d)\n", fcell->key, F[fcell->key], cell->key, F[cell->key]);
#endif
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(w, k, rs, N);
        if ((y != -1) && (!IsSet(y, ENFAH)))
        {
          Set(y, ENFAH);
          FahPush(FAH, y, NDG_MAX - F[y]);
	}
      } /* for k ... */
    } /* else */
  } /* while (!FahVide(FAH)) */
  for (i = 0; i < N; i++) UnSetAll(i);

#ifdef VERBOSE
printf("PREMIER MAXIMUM TERMINE\n");
#endif

#ifdef DEBUG
    for (l = 0; l < N; l++)
    {
      if (l % rs == 0) printf("\n");
      printf("%3d ", M[l]->key);
    }
    printf("\n");
#endif

  for (i = 1; i < nmaxima; i++)
  {
    cell = tabmax[i];
    x = cell->key;
    level = F[x];
#ifdef DEBUG
printf("maximum : %d(%d)\n", x, level);
#endif
    FahPush(FAH, x, NDG_MAX - level);
    Set(x, ENFAH);
    while (!FahVide(FAH))
    {
      w = FahPop(FAH);
      UnSet(w, ENFAH);
      ocell = M[w];
#ifdef DEBUG
printf("pop %d(%d) ; ocell = %d(%d) ; level = %d\n", w, F[w], ocell->key, F[ocell->key], level);
#endif
      if ((F[w] == level) || ((F[w] > level) && (F[ocell->key] < level)))
      {
        M[w] = cell;
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) && (!IsSet(y, ENFAH)) && (M[y] != cell))
          {
            Set(y, ENFAH);
            FahPush(FAH, y, NDG_MAX - F[y]);
	  }
        } /* for k ... */        
      } /* if ((F[w] == level) || ((F[w] > level) && (F[ocell->key] < level))) */
      else if (F[w] < level)
      {
        level = F[w];
        fcell = cell;
        if (F[ocell->key] < level)
	{
          cell = AllocTreeCell();
          cell->key = w;
          cell->father = NULL;
          fcell->father = cell;
          M[w] = cell;
#ifdef DEBUG
printf("branche arbre : %d(%d) -> %d(%d)\n", fcell->key, F[fcell->key], cell->key, F[cell->key]);
#endif
          Set(w, ENFAH);
          FahPush(FAH, w, NDG_MAX - F[w]);
        } /* if (F[ocell->key] < level) */
        else
	{
          cell->father = ocell;            
#ifdef DEBUG
printf("branche arbre (jonction) : %d(%d) -> %d(%d)\n", cell->key, F[cell->key], ocell->key, F[ocell->key]);
#endif
          while (!FahVide(FAH)) { w = FahPop(FAH); UnSet(w, ENFAH); } 
          FahFlush(FAH);
	}
      } /* else */
    } /* while (!FahVide(FAH)) */
#ifdef DEBUG
    for (l = 0; l < N; l++)
    {
      if (l % rs == 0) printf("\n");
      printf("%3d ", M[l]->key);
    }
    printf("\n");
#endif
#ifdef VERBOSE
if (i%1000 == 0) printf("%d MAXIMA TRAITES\n", i);
#endif
  } /* for (i = 1; i < nmaxima; i++) */

#ifdef VERBOSE
      fprintf(stderr, "CONSTRUCTION ARBRE TERMINEE\n");
#endif

  /* ================================================ */
  /* CALCUL DE L'ATTRIBUT DE PROFONDEUR               */
  /* ================================================ */

  for (i = 0; i < nmaxima; i++) 
    for (cell = tabmax[i]; cell != NULL; cell = cell->father)
      cell->prof = 0;

  for (i = 0; i < nmaxima; i++) 
  {
    maxi = F[tabmax[i]->key];
    for (cell = tabmax[i]; cell != NULL; cell = cell->father)
      cell->prof = mcmax(cell->prof,maxi-F[cell->key]);
  } /* for (i = 1; i < nmaxima; i++) */

#ifdef DEBUG
  for (i = 0; i < nmaxima; i++) 
  {
    for (cell = tabmax[i]; cell != NULL; cell = cell->father)
      printf("%d(%d,%d) ", cell->key, F[cell->key], cell->prof);
    printf("\n");
  } /* for (i = 1; i < nmaxima; i++) */
#endif
  do
  {
    int32_t X, Y;
    printf("entrer un point (x,y)\n");
    scanf("%d", &X);
    scanf("%d", &Y);
    for (cell = M[Y*rs + X]; cell != NULL; cell = cell->father)
      printf("%d %d\n", NDG_MAX - F[cell->key], cell->prof);
    printf("continue ? (o/n)\n");
    scanf("%s", buf);
  } while(buf[0] != 'n');
  
  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  TreeTermine();
  free(M);
  return(1);
}
