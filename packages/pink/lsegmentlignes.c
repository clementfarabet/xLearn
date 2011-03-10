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
/* operateur de segmentation de structures lineaires */
/* utilise une File d'Attente Hierarchique */
/* Michel Couprie - janvier 1998 */
/*
   4/2/98 : Seuillage sur le niveau de gris pour selectionner les "germes". 
            On arrete la croissance aux points multiples.

   7/2/98 : Elimination des points multiples : on les transforme en points doubles,
            avec la strategie suivante : 
            - tous les points de Mb sont mis en FAH.
            - on inonde, et chaque fois qu'on tombe sur un point multiple, 
                on detecte se deux voisins les plus "pertinents"
                on efface les autres de Mb, en remettant a jour les infos V dans le voisinage
            1ere idee : les voisins "pertinents" sont les plus bas.
            2eme idee : basee sur le contraste local
*/

#define IDEE2

#define PARANO                 /* even paranoid people have ennemies */
#define VERBOSE
/*
#define DEBUG
*/

/*
   Methode : 

   1/ etiquetage par M des maxima en Nb-connexite (typiquement N = 8, Nb = 4)

   2/ parcours des points non-maxima et stockage de leur degre dans V

   2b/ elimination des points multiples

   3/ selection des points de Mb (compl. de M) de niveau <= seuilbas
      - les voisins non-maxima des points retenus sont mis dans la FAH

   4/ inondation :
      tq !fahvide
        x = fahpop
        h = F[x]
        etiquette x
        empile les voisins non traites et non-maxima de x
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mcfah.h>
#include <mclifo.h>
#include <mcindic.h>
#include <mcutil.h>
#include <llabelextrema.h>
#include <lsegmentlignes.h>

#define EN_FAH 0
#define RETENU 1

void tricroissant( int32_t * v, int32_t * vv, int32_t n )
{
  /* methode : tri a bulles */
  int32_t I, TEMP, ECHANGES;

  ECHANGES = 1;
  while (ECHANGES) 
  {
    ECHANGES = 0;
    for (I = 1; I < n; I++) 
      if (vv[I] < vv[I - 1])
      {
        TEMP = v[I]; v[I] = v[I - 1]; v[I - 1] = TEMP;
        TEMP = vv[I]; vv[I] = vv[I - 1]; vv[I - 1] = TEMP;
        ECHANGES = 1;
      } /* for, if */
    n--;
  } /* while (ECHANGES) */
} /* tricroissant() */

/* ==================================== */
int32_t lsegmentlignes(
        struct xvimage *image,
        int32_t connex,
        int32_t seuilbas,
        int32_t seuilhaut)
/* ==================================== */
{
  register int32_t x;              /* index muet de pixel */
  register int32_t y;              /* index muet (generalement un voisin de x) */
  register int32_t w;              /* index muet de pixel */
  register int32_t i;              /* index muet */
  register int32_t k;              /* index muet */
  register int32_t l;              /* index muet */
  register int32_t h;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  struct xvimage *lab;         /* l'image d'etiquettes de composantes connexes */
  uint32_t *M;            /* l'image d'etiquettes de composantes connexes (data) */
  int32_t nmaxima;                 /* nombre de maxima differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t incr_vois;
  int32_t connexbar;
  uint8_t * V;
  int32_t v1, v2, v3;
  int32_t v[8];
  int32_t vv[8];

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lsegmentlignes: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; connexbar = 8; break;
    case 8: incr_vois = 1; connexbar = 4; break;
    default: 
      fprintf(stderr, "lsegmentlignes: mauvaise connexite: %d\n", connex);
      return 0;
  } /* switch (connex) */

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "lsegmentlignes() : CreeFah failed\n");
      return(0);
  }

  lab = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (lab == NULL)
  {   
    fprintf(stderr, "lsegmentlignes: allocimage failed\n");
    return 0;
  }
  M = SLONGDATA(lab);

  V = (uint8_t *)calloc(N, sizeof(char));   /* init a 0 */
  if (V == NULL)
  {   fprintf(stderr, "lsegmentlignes() : malloc failed for V\n");
      return(0);
  }

/*
  1/ etiquetage par M des maxima en Nb-connexite (typiquement N = 8, Nb = 4)
*/

#ifdef DEBUG
printf("DEBUT ETIQUETAGE MAXIMA\n");
#endif

  if (!llabelextrema(image, connexbar, LABMAX, lab, &nmaxima))
  {   
    fprintf(stderr, "lsegmentlignes: llabelextrema failed\n");
    return 0;
  }

/*
  2/ parcours des points non-maxima et stockage de leur degre dans V
*/

  for (x = 0; x < N; x++)
  {
    if (M[x] == 0)
    {
      i = 0;     /* pour compter le nb de voisins de x dans Mbar */
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (M[y] == 0)) i++;   
      } /* for (k = 0; k < 8; k += incr_vois) */
      V[x] = i;
    } /* if (M[x] == 0)*/
  } /* for (x = 0; x < N; x++) */

/*
   2b/ elimination des points multiples
*/

  /* detection des "3-cycles" (pour la 8-connexite) : */
  /* si un 2-point a pour voisins deux N-points (N>2) voisins entre eux, */
  /* alors on baisse de 1 le V des deux N-points et on retire de Mb le 2-point */
  /* les valeurs des deux N-points sont eventuellement baissees au niveau du 2-point */

  for (x = 0; x < N; x++)
  {
    if (V[x] == 2)
    {
      v3 = 0;
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y!=-1) && (V[y]>2)) { if (v3 == 0) v1 = y; else v2 = y; v3++; }
      } /* for (k = 0; k < 8; k += incr_vois) */
      if ((v3 == 2) && (voisins8(v1, v2, rs))) 
      { 
        V[v1]--; V[v2]--; M[x] = nmaxima+1;
        F[v1] = mcmin(F[v1], F[x]); F[v2] = mcmin(F[v2], F[x]);
#ifdef DEBUG
printf("REDUCTION 3-CYCLE %d,%d : V[%d,%d]=%d, V[%d,%d]=%d\n", 
        x%rs, x/rs, v1%rs, v1/rs, V[v1], v2%rs, v2/rs, V[v2]);
#endif
      }
    } /* if (V[x] == 2)*/
  } /* for (x = 0; x < N; x++) */

  /* cas des points multiples : on ne compte pas les voisins */
  /* qui sont des points extremites (ceux-ci sont retires de Mb, */
  /* et le point multiple prend la valeur du min de son ancienne valeur */
  /* et de la valeur du point extremite) */

  for (x = 0; x < N; x++)
  {
    if (V[x] > 2)
    {
      v1 = 0;
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y!=-1) && (V[y]==1)) 
        { 
          v1++; F[x] = mcmin(F[x], F[y]); M[y] = nmaxima+1; 
#ifdef DEBUG
printf("REDUCTION %d,%d : EXTREMITE %d,%d\n", x%rs, x/rs, y%rs, y/rs);
#endif
        }
      } /* for (k = 0; k < 8; k += incr_vois) */
      V[x] -= v1;
    } /* if (V[x] > 2)*/
  } /* for (x = 0; x < N; x++) */
  
  /* reduction des points multiples par detection des voisins non "pertinents" */

  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
  for (x = 0; x < N; x++)
    if (M[x] == 0)
      FahPush(FAH, x, F[x]);
  x = FahPop(FAH);

  while(!FahVide(FAH))
  {
    x = FahPop(FAH);
    if ((M[x] == 0) && (V[x] > 2))   /* point multiple */
    {
#ifdef DEBUG
printf("REDUIT POINT MULTIPLE %d,%d : \n", x%rs, x/rs);
#endif
      i = 0;                         /* parcours des voisins et stockage dans v */
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y!=-1) && (M[y] == 0)) 
	{
          v[i] = y;
#ifdef IDEE1
          vv[i] = F[y];
#endif
#ifdef IDEE2
          vv[i] = F[y] - beta4p(F, y, rs, N);
#endif
          i++;
	}
      } /* for (k = 0; k < 8; k += incr_vois) */
      tricroissant(v, vv, i);       /* tri des voisins par ordre de F croissant */
      for (k = 2; k < i; k ++)      /* pour tous les voisins v[k] non pertinents */
      {
        y = v[k];
        M[y] = nmaxima+1;           /* elimine le voisin */
#ifdef DEBUG
printf("ELIMINE %d,%d   ", y%rs, y/rs);
#endif
        for (l = 0; l < 8; l += incr_vois)
        {                           /* met a jour les V dans le voisinage du voisin */
          w = voisin(y, l, rs, N);
          if ((w!=-1) && (M[w] == 0)) 
	  {
            V[w]--;
#ifdef DEBUG
printf("V[%d,%d] = %d  ", w%rs, w/rs, V[w]);
#endif
	  }
        } /* for (l = 0; l < 8; l += incr_vois) */
#ifdef DEBUG
printf("\n");
#endif
      } /* for (k = 2; k < i; k ++) */
    } /* if ((M[x] == 0) && (V[x] > 2)) */
  } /* while(!FahVide(FAH)) */


  FahFlush(FAH);

/*
  3/ selection des points de Mb (compl. de M) de niveau <= seuilbas
     les voisins non-maxima des points retenus sont mis dans la FAH
*/

#ifdef DEBUG
fprintf(stderr, "DEBUT INITIALISATION\n");
#endif
  
  FahPush(FAH, -1, 0);               /* force la creation du niveau 0 dans la Fah. */
                                     /* NECESSAIRE pour eviter la creation prematuree */
                                     /* de la file d'urgence */ 
  for (x = 0; x < N; x++)
  {
    if (M[x] == 0)
    {
      if (F[x] <= seuilbas) /* pour selectionner les "germes" */
      {
        Set(x, RETENU);
#ifdef DEBUG
printf("INIT : RETENU : %d,%d\n ", x%rs, x/rs);
#endif
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1)&&(M[y]==0)&&(!IsSet(y, RETENU))&&(!IsSet(y, EN_FAH)))
          {
            Set(y, EN_FAH);
            FahPush(FAH, y, F[y]);
#ifdef DEBUG
printf("INIT : PUSH %d,%d\n", y%rs, y/rs);
#endif
          } /* if (y != -1) ... */
        } /* for (k = 0; k < 8; k += incr_vois) */
      } /* if (F[x] <= seuilbas) */
    } /* if (M[x] == 0)*/
  } /* for (x = 0; x < N; x++) */
  x = FahPop(FAH);

/*
   4/ inondation :
      tq !fahvide
        x = fahpop
        h = F[x]
        etiquette x
        empile les voisins non traites et non-maxima de x
*/

#ifdef DEBUG
printf("DEBUT INONDATION\n");
#endif

  while(!FahVide(FAH))
  {
    x = FahPop(FAH);
#ifdef DEBUG
printf("POP : %d,%d, V = %d \n ", x%rs, x/rs, V[x]);
#endif
    Set(x, RETENU);
#ifdef DEBUG
printf("RETENU : %d,%d\n ", x%rs, x/rs);
#endif
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1)&&(M[y]==0)&&(!IsSet(y, RETENU))&&(!IsSet(y, EN_FAH)))
      {
        Set(y, EN_FAH);
        FahPush(FAH, y, F[y]);
#ifdef DEBUG
printf("PUSH : %d,%d \n", y%rs, y/rs);
#endif
      } /* if (y != -1) ... */
    } /* for (k = 0; k < 8; k += incr_vois) */
  } /* while(!FahVide(FAH)) */

  for (x = 0; x < N; x++)
    if (IsSet(x, RETENU)) F[x] = NDG_MAX;
    else                  F[x] = NDG_MIN;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  freeimage(lab);
  free(V);
  return(1);
}

