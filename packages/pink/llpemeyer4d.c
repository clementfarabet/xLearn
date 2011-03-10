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
   Operateur de calcul de la ligne de partage des eaux en 4D
   d'apres "Un algorithme optimal de ligne de partage des eaux"
           F. Meyer - actes du 8eme congres AFCET - Lyon-Villeurbanne
           1991 
   variante de la section VI (ligne d'epaisseur 1 pixel)

   Utilise une File d'Attente Hierarchique.

   Jean Cousty -  2005 
   d'après Michel Couprie 97
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mcimage.h>
#include <jcimage.h>
#include <mcfah.h>
#include <mcindic.h>
#include <llpemeyer4d.h>

#define EN_FAH   0

/* ==================================== */
int32_t llpemeyer4d_NotIn(int32_t e, int32_t *list, int32_t n)                       
/* ==================================== */
{
/* renvoie 1 si e n'est pas dans list, 0 sinon */
/* e : l'element a rechercher */
/* list : la liste (tableau d'entiers) */
/* n : le nombre d'elements dans la liste */
  while (n > 0)
    if (list[--n] == e) return 0;
  return 1;
} /* llpemeyer4d_NotIn() */


/* ==================================== */
int32_t llpemeyer4d(
        struct xvimage4D *image,
        struct xvimage4D *marqueurs,
        struct xvimage4D *marqueursfond,
        struct xvimage4D *masque,
        int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyer4d"
{
  register int32_t x;                       /* index muet de pixel */
  register int32_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  int32_t j;
  int32_t rs = rowsize(image->frame[0]);     /* taille ligne */
  int32_t cs = colsize(image->frame[0]);     /* taille colonne */
  int32_t ds = depth(image->frame[0]);        /* nb plans */
  int32_t ss = image->ss;
  int32_t n = rs * cs;             /* taille plan */
  int32_t N = n * ds;               /* taille frame */
  int32_t Nt = N * ss;
  uint8_t **F;       /* l'image de depart */
  uint8_t **B;       /* l'image de marqueurs */
  uint8_t **BF;                             /* l'image de marqueurs du fond */
  uint8_t **MA;                             /* l'image de masque */
  uint32_t *M;             /* l'image d'etiquettes */
  int32_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[8];
  int32_t ncc;  
  
  F = (uint8_t **)malloc(sizeof(char *) * image->ss);
  B = (uint8_t **)malloc(sizeof(char *) * image->ss);
  for(j = 0; j < ss; j++)
  {
    F[j] = UCHARDATA(image->frame[j]);
    B[j] =  UCHARDATA(marqueurs->frame[j]);
  }

  if ((rowsize(marqueurs->frame[0]) != rs) || (colsize(marqueurs->frame[0]) != cs) || (depth(marqueurs->frame[0]) != ds) || (marqueurs->ss != ss) )
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
     
  if (marqueursfond && ((rowsize(marqueursfond->frame[0]) != rs) || (colsize(marqueursfond->frame[0]) != cs) || (depth(marqueursfond->frame[0]) != ds) || (marqueursfond->ss != ss)) )
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (marqueursfond) 
  {
    BF = (uint8_t **)malloc(sizeof(char *) * image->ss);
    for(j = 0; j < ss; j++)
    {
      BF[j] = UCHARDATA(marqueursfond->frame[j]);
    }
  }
  if (masque && ((rowsize(masque->frame[0]) != rs) || (colsize(masque->frame[0]) != cs) || (depth(masque->frame[0]) != ds) || (masque->ss != ss)  ))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) 
  {
    MA = (uint8_t **)malloc(sizeof(char *) * image->ss);
    for(j = 0; j < ss; j++)
    {
      MA[j] = UCHARDATA(masque->frame[j]);
    }
  }

  IndicsInit(Nt);
  FAH = CreeFahVide(Nt+1);
  if (FAH == NULL)
  {   
    fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */

  M = (uint32_t *)calloc(Nt, sizeof(int32_t));
  if (M == NULL)
  {   fprintf(stderr, "%s() : calloc failed\n", F_NAME);
      return(0);
  }
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < Nt; x++)
    { 
      if (BF[x/N][x%N] && (M[x] == 0) && (!masque || MA[x/N][x%N]))
      {
	M[x] = nlabels; 
	FahPush(FAH, x, 0);
	while (! FahVide(FAH))
	{
	  w = FahPop(FAH);
	  switch (connex)
	  {
	  case 8:
	    for (k = 0; k < 8; k ++) /* parcours les 8 voisins */
	    {
	      y = voisin4D8(w, k, rs, n, N, Nt);
	      if ((y != -1) && BF[y/N][y%N] && (M[y] == 0) && (!masque || MA[y/N][y%N]))
	      { M[y] = nlabels; FahPush(FAH, y, 0); }
	    } /* for k ... */
	    break;
	  } /* switch (connex) */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */
  
  for (x = 0; x < Nt; x++)
  {
    if (B[x/N][x%N] && (M[x] == 0) && (!masque || MA[x/N][x%N]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
        switch (connex)
        {
	  case 8:
            for (k = 0; k < 8; k++)
            {
              y = voisin4D8(w, k, rs, n, N, Nt);
              if ((y!=-1) && (M[y]==0) && (B[y/N][y%N]==B[w/N][w%N]) && (!masque || MA[y/N][y%N]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;	 
	} /* switch (connex) */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < Nt; x++)
  {
    if (M[x] && (!masque || MA[x/N][x%N]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 8:
          for (k = 0; k < 8; k ++)
          {
            y = voisin4D8(x, k, rs, n, N, Nt);
            if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y/N][y%N]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;	 
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 8:
        for (k = 0; k < 8; k ++)
        {
          y = voisin4D8(x, k, rs, n, N, Nt);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer4d_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];
      switch (connex)
      {
        case 8:
          for (k = 0; k < 8; k ++)
          {
            y = voisin4D8(x, k, rs, n, N, Nt);     
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y/N][y%N]))
            {
              FahPush(FAH, y, F[y/N][y%N]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;

      } /* switch (connex) */
    } 
    else 
      if (ncc > 1)
      {
	M[x] = nlabels;
      }
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < Nt; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x/N][x%N] = 255; else F[x/N][x%N] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  free(M);
  free(F);
  if (marqueursfond)
    free(BF);
  if (masque)
    free(MA);
  return(1);
} /* llpemeyer4d() */

