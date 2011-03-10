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
/****************************************************************
*
* Routine Name: ldistXXX - library call for dist
*
* Purpose:     Récupération de l'arbre virtuel d'une image
               (n = 4,8,6,18,26)
*
* Input:       X: Image binaire
*Output:       Image en niveaux de gris (entiers longs)
*
* Author: Cédric Allène
*
****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mctopo3d.h>
#include <lgettree.h>


/* ==================================== */
noeudArbre * lgettree(struct xvimage *imgx,   /* donnee: image binaire */
                      int32_t connex,             /* donnee: connexité */
                      int32_t extension,          /* donnee: longueur de l'extension des intersections */
                      struct xvimage *res     /* resultat: distances (doit être allouée) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgettree"
{ 
  noeudArbre * racine;

  if ((connex == 4) || (connex == 8))
    racine = (noeudArbre *) lgettree2d(imgx, connex, extension, res);
  else
    racine = (noeudArbre *) lgettree3d(imgx, connex, extension, res);
  
  return (racine);
} // lgettree


/* ==================================== */
noeudArbre * lgettree2d(struct xvimage *imgx,   /* donnee: image binaire */       
                        int32_t connex,             /* donnee: connexité */
                        int32_t extension,          /* donnee: longueur de l'extension des intersections */
                        struct xvimage *res     /* resultat: distances (doit être allouée) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgettree2d"
{ 
  int32_t rs = imgx->row_size;
  int32_t cs = imgx->col_size;
  int32_t N = rs * cs;           /* taille de l'image */
  uint8_t *X;          /* pointeur sur l'image x */
  int32_t *D;               /* pointeur sur les distances */
  int32_t i, j, k, current_pixel, incr_vois;

  noeudArbre * racine;

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != 1))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (datatype(res) != VFF_TYP_4_BYTE)  /* vérification du type d'image obtenu */
  {
    fprintf(stderr, "%s: result image type must be int32_t\n", F_NAME);
    return 0;
  }

  X = UCHARDATA(imgx);
  D = SLONGDATA(res);

  k = 0;
  
  for (i = 0; i < rs; i++)   /* parcours des points de la ligne 0 */
  {
    if (!X[i])           /* si le point est nul... */
    {
      D[i] = 0;          /* ... mise à zéro du point correspondant pour la sortie */
    }
    else                 /* sinon... */
    { /* X[i] != 0 */
      D[i] = 1;          /* ... mise à 1 du point correspondant pour la sortie */
      k ++;              /* incrémentation du compteur de points non nuls du plan 0 */
      current_pixel = i; /* mise en mémoire du point non nul rencontré */
    }
  } /* for (i = 0; i < rs; i++) */
  
  if (k!=1)  /* il ne doit y avoir qu'un seul point non nul sur la ligne 0 */
  {          /* ce point est la racine de l'arbre */
    fprintf (stderr, "%s: uniquement le point de départ de la racine sur la première ligne (y=0)\n", F_NAME);
    return 0;
  } /* if (k!=1) */

  for (i = rs; i < N; i++)  /* parcours des points restants de l'image */
  {
    if (!X[i])           /* si le point est nul... */
    {
      D[i] = 0;          /* ... mise à zéro du point correspondant pour la sortie */
    }
    else                 /* sinon... */
    { /* X[i] != 0 */
      D[i] = -1;         /* ... mise à -1 du point correspondant pour la sortie */
                         /* -1 est la valeur des points non traités */
    }
  } /* for (i = rs; i < N; i++) */

  switch (connex)
  {
    case 4: racine = (noeudArbre *) getbranch2d4 (X, rs, N, current_pixel, extension, D, RACINE); break;
    case 8: racine = (noeudArbre *) getbranch2d8 (X, rs, N, current_pixel, extension, D, RACINE); break;
    default: 
      fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */
  racine->longBranche--;  /* il n'y a pas de voxel rajouté par l'intersection d'origine */

  return (racine);
} // lgettree2d()


/* ==================================== */
noeudArbre * lgettree3d(struct xvimage *imgx,   /* donnee: image binaire */       
                        int32_t connex,             /* donnee: connexité */
                        int32_t extension,          /* donnee: longueur de l'extension des intersections */
                        struct xvimage *res     /* resultat: distances (doit être allouée) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgettree3d"
{ 
  int32_t rs = imgx->row_size;
  int32_t cs = imgx->col_size;
  int32_t ds = depth(imgx);
  int32_t ps = rs * cs;          /* taille d'un plan */
  int32_t N = ps * ds;           /* taille de l'image */
  uint8_t *X;          /* pointeur sur l'image x */
  int32_t *D;               /* pointeur sur les distances */
  int32_t i, j, k, current_pixel, incr_vois;

  noeudArbre * racine;

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (datatype(res) != VFF_TYP_4_BYTE)  /* vérification du type d'image obtenu */
  {
    fprintf(stderr, "%s: result image type must be int32_t\n", F_NAME);
    return 0;
  }

  X = UCHARDATA(imgx);
  D = SLONGDATA(res);

  k = 0;
  
  for (i = 0; i < ps; i++)  /* parcours des points du plan 0 */
  {
    if (!X[i])           /* si le point est nul... */
    {
      D[i] = 0;          /* ... mise à zéro du point correspondant pour la sortie */
    }
    else                 /* sinon... */
    { /* X[i] != 0 */
      D[i] = 1;          /* ... mise à 1 du point correspondant pour la sortie */
      k ++;              /* incrémentation du compteur de points non nuls du plan 0 */
      current_pixel = i; /* mise en mémoire du point non nul rencontré */
    }
  } /* for (i = 0; i < rs*cs; i++) */
  
  if (k!=1)  /* il ne doit y avoir qu'un seul point non nul sur le plan 0 */
  {          /* ce point est la racine de l'arbre */
    fprintf (stderr, "%s: uniquement le point de départ de la racine sur le plan d'origine (z=0)\n", F_NAME);
    return 0;
  } /* if (k!=1) */

  for (i = rs*cs; i < N; i++) /* parcours des points restants de l'image */
  {
    if (!X[i])           /* si le point est nul... */
    {
      D[i] = 0;          /* ... mise à zéro du point correspondant pour la sortie */
    }
    else                 /* sinon... */
    { /* X[i] != 0 */
      D[i] = -1;         /* ... mise à -1 du point correspondant pour la sortie */
                         /* -1 est la valeur des points non traités */
    }
  } /* for (i = rs*cs; i < N; i++) */

  switch (connex)  /* traitement différent selon le type de connexité désiré */
  {
    case 6:  racine = (noeudArbre *) getbranch3d6  (X, rs, ps, N, current_pixel, extension, D, RACINE); break;
    case 18: racine = (noeudArbre *) getbranch3d18 (X, rs, ps, N, current_pixel, extension, D, RACINE); break;
    case 26: racine = (noeudArbre *) getbranch3d26 (X, rs, ps, N, current_pixel, extension, D, RACINE); break;
    default: 
      fprintf(stderr, "%s: bad connectivity: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */
  racine->longBranche--;  /* il n'y a pas de voxel rajouté par l'intersection d'origine */
  
  return (racine);
} // lgettree3d()


/* ==================================== */
noeudArbre * getbranch2d4(uint8_t *X,  /* donnee: pointeur sur l'image x */
                          int32_t rs,            /* donnee: taille ligne */
		          int32_t N,             /* donnee: taille image */
                          int32_t current_pixel, /* donnee: numéro pixel courant */  
                          int32_t extension,     /* donnee: longueur de l'extension des intersections */
                          int32_t *D,       /* donnee: pointeur sur distances */
			  int32_t racine         /* donnee: indicateur pour l'origine de l'arbre */
) /* Etiquette les branches avec la 4-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "getbranch2d4"
{ 
  printf ("Pas encore implémenté...\n");

  return(NULL);
} // lgettree2d4()

/* ==================================== */
noeudArbre * getbranch2d8(uint8_t *X,  /* donnee: pointeur sur l'image x */
                          int32_t rs,            /* donnee: taille ligne */
		          int32_t N,             /* donnee: taille image */
                          int32_t current_pixel, /* donnee: numéro pixel courant */  
                          int32_t extension,     /* donnee: longueur de l'extension des intersections */
                          int32_t *D,       /* donnee: pointeur sur distances */
			  int32_t racine         /* donnee: indicateur pour l'origine de l'arbre */
) /* Etiquette les branches avec la 8-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "getbranch2d8"
{ 
  printf ("Pas encore implémenté...\n");

  return(NULL);
} // lgettree2d8()

/* ==================================== */
noeudArbre * getbranch3d6(uint8_t *X,  /* donnee: pointeur sur l'image x */
                          int32_t rs,            /* donnee: taille ligne */
                          int32_t ps,            /* donnee: taille plan */
		          int32_t N,             /* donnee: taille image */
                          int32_t current_pixel, /* donnee: numéro pixel courant */  
                          int32_t extension,     /* donnee: longueur de l'extension des intersections */
                          int32_t *D,       /* donnee: pointeur sur distances */
			  int32_t racine         /* donnee: indicateur pour l'origine de l'arbre */
) /* Etiquette les branches avec la 6-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "getbranch3d6"
{ 
  printf ("Pas encore implémenté...\n");
  
  return(NULL);
} // lgettree3d6()

/* ==================================== */
noeudArbre * getbranch3d18(uint8_t *X,  /* donnee: pointeur sur l'image x */
                           int32_t rs,            /* donnee: taille ligne */
                           int32_t ps,            /* donnee: taille plan */
		           int32_t N,             /* donnee: taille image */
                           int32_t current_pixel, /* donnee: numéro pixel courant */  
                           int32_t extension,     /* donnee: longueur de l'extension des intersections */
                           int32_t *D,       /* donnee: pointeur sur distances */
			   int32_t racine         /* donnee: indicateur pour l'origine de l'arbre */
) /* Etiquette les branches avec la 18-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "getbranch3d18"
{ 
  printf ("Pas encore implémenté...\n");
  
  return(NULL);
} // lgettree3d18()

/* ==================================== */
noeudArbre * getbranch3d26(uint8_t *X,  /* donnee: pointeur sur l'image x */
                           int32_t rs,            /* donnee: taille ligne */
                           int32_t ps,            /* donnee: taille plan */
		           int32_t N,             /* donnee: taille image */
                           int32_t current_pixel, /* donnee: numéro pixel courant */  
                           int32_t extension,     /* donnee: longueur de l'extension des intersections */
                           int32_t *D,       /* donnee: pointeur sur distances */
			   int32_t racine         /* donnee: indicateur pour l'origine de l'arbre */
) /* Etiquette les branches avec la 26-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "getbranch3d26"
{ 
  noeudArbre * noeud = NULL;
  size_t tailleNoeud;
  static int32_t numNoeud = 0;  /* numéro de noeud */

  liste * listeFils = NULL;
  liste * ptrListe = NULL;

  int32_t nbVois, nbFils, d, k, j;
     
  d = D[current_pixel];                              /* récupère le niveau dans l'arbre du point courant */

  nbVois = mctopo3d_nbvoiso26 (X, current_pixel, rs, ps, N);  /* calcule le nombre de voisin du point courant */

  if (nbVois > 2)  /* s'il y a plus de 2 voisins, on se trouve dans une intersection */
  {
    listeFils = (liste *) fils3d26 (X, rs, ps, N, current_pixel, extension, D, extension); /* on récupère les fils du noeud */
                                                                                           /* (les points voisins de l'intersection) */
                /* les fils d'une intersection ne sont pas forcément voisins du premier point de l'intersection */
    
    nbFils = NbFils (listeFils);  /* on récupère le nombre de fils du noeud */
    
    if (nbFils > 1)
    {
      numNoeud++;                                                           /* incrémente le numéro de noeud */
      tailleNoeud = sizeof (noeudArbre) + (nbFils-1)*sizeof (noeudArbre *); /* calcule la taille nécessaire pour le noeud */
      noeud = (noeudArbre *) malloc (tailleNoeud);                          /* alloue la taille mémoire nécessaire au noeud */
      noeud->numNoeud = numNoeud;
      noeud->visible = VISIBLE;                                             /* indique si le noeud doit apparaître sur le schéma */
      noeud->coord_X = current_pixel % rs;
      noeud->coord_Y = (current_pixel % ps) / rs;
      noeud->coord_Z = current_pixel / ps;                                  /* copie des infos concernant le noeud */
      noeud->num_pixel = current_pixel;
      noeud->longBranche = extension + 1;  /* on prend ici en compte le(s) voxel(s) non décompté(s) en début de branche (après l'intersection) */
      noeud->nbFils = nbFils;
      noeud->level = d;        /* initialise le level au niveau courant */
      noeud->levelMax = 0;     /* initialise le levelMax à 0 */
      noeud->levelMin = -1;    /* initialise le levelMin à l'infini */
    
      k = 0;
      while (listeFils != NULL)
      {
        j = listeFils->valeur;  /* récupère la coordonnée du premier point de la branche fille */
        noeud->fils[k] = getbranch3d26 (X, rs, ps, N, j, extension, D, NON_RACINE);  /* ... récupère le sous-arbre correspondant */
       
        ptrListe = listeFils;
        listeFils = listeFils->suivant;  /* avance d'un élément dans la liste */
        free (ptrListe);                 /* libère la mémoire de l'élément de la liste */
        
        noeud->levelMax = MAX((noeud->levelMax), (noeud->fils[k]->levelMax));  /* récupère le niveau max des fils déjà traités */
        noeud->levelMin = MIN((noeud->levelMin), (noeud->fils[k]->levelMin));  /* récupère le niveau min des fils déjà traités */
        k++;
      }
      if (k != nbFils)  /* vérifie que le nombre de fils trouvé correspond au nombre d'élément de la liste renvoyée */
      {
        fprintf(stderr, "%s: ERREUR dans le décompte des fils de l'intersection...", F_NAME);
        exit (0);
      }
    }
    else if (nbFils == 1)
    {
      current_pixel = listeFils->valeur;  /* on récupère la coordonnée de la suite de la branche */
      free (listeFils);
      D[current_pixel] = d;  /* on attribue au point courant du noeud le niveau de la branche ascendante */
      
      BaisserBranche3d26 (rs, ps, N, current_pixel, D, d);  /* corrige les voxels mis à un niveau supérieur */
      
      noeud = getbranch3d26 (X, rs, ps, N, current_pixel, extension, D, NON_RACINE);
                     /* transmet l'adresse de la structure du sous-arbre inférieur au point courant */
      noeud->longBranche += (extension + 1);  /* incrémente la valeur de la longueur de la branche */
                                              /* pour le voxel courant et le point détecté comme une intersection fantôme */
    }
    else /* nbFils == 0 */  /* on est dans le cas où l'extrémité de la branche est un amas de points */
    {
        /* création d'un noeud extrémité */
      numNoeud++;                                   /* incrémente le numéro de noeud */
      tailleNoeud = sizeof (noeudArbre);            /* calcule la taille nécessaire pour le noeud */
      noeud = (noeudArbre *) malloc (tailleNoeud);  /* alloue la taille mémoire nécessaire au noeud */
      noeud->numNoeud = numNoeud;
      noeud->visible = VISIBLE;                     /* indique si le noeud doit apparaître sur le schéma */
      noeud->coord_X = current_pixel % rs;
      noeud->coord_Y = (current_pixel % ps) / rs;
      noeud->coord_Z = current_pixel / ps;          /* copie des infos concernant le noeud */
      noeud->longBranche = extension + 1;  /* on prend ici en compte le(s) voxel(s) non décompté(s) en début de branche (après l'intersection) */
      noeud->nbFils = 0;
      noeud->fils[0] = NULL;
      noeud->level = d;                             /* initialise le level au niveau courant */
      noeud->levelMax = d;                          /* initialise le levelMax au niveau courant */
      noeud->levelMin = d;                          /* initialise le levelMin au niveau courant */
    }
  }
  else if ((nbVois == 1) && (racine == NON_RACINE))  /* on est dans le cas d'un point feuille (extrémité d'une branche) */
  {               /* le cas (racine == RACINE) correspond au point racine, qui peut n'avoir qu'un seul voisin */
      /* création d'un noeud extrémité */
    numNoeud++;                                   /* incrémente le numéro de noeud */
    tailleNoeud = sizeof (noeudArbre);            /* calcule la taille nécessaire pour le noeud */
    noeud = (noeudArbre *) malloc (tailleNoeud);  /* alloue la taille mémoire nécessaire au noeud */
    noeud->numNoeud = numNoeud;
    noeud->visible = VISIBLE;                     /* indique si le noeud doit apparaître sur le schéma */
    noeud->coord_X = current_pixel % rs;
    noeud->coord_Y = (current_pixel % ps) / rs;
    noeud->coord_Z = current_pixel / ps;          /* copie des infos concernant le noeud */
    noeud->longBranche = extension + 1;  /* on prend ici en compte le(s) voxel(s) non décompté(s) en début de branche (après l'intersection) */
    noeud->nbFils = 0;
    noeud->fils[0] = NULL;
    noeud->level = d;                             /* initialise le level au niveau courant */
    noeud->levelMax = d;                          /* initialise le levelMax au niveau courant */
    noeud->levelMin = d;                          /* initialise le levelMin au niveau courant */
  }
  else  /* nbVois == 2 */  /* on est dans le cas d'un point branche (situé au milieu de la branche) */
  {     /* on ne doit donc rencontrer qu'un seul point non traité parmi les voisins du point courant */
    for (k = 0; k < 26; k += 1)                   /* regarde tous les voisins potentiels */
    {
      j = voisin26 (current_pixel, k, rs, ps, N); /* teste la présence du voisin */
      if ((j != -1) && (D[j] == -1))              /* vérifie qu'ils n'ont pas été déjà traités */
      {
        if (nonbord3d (j, rs, ps, N) == 0)        /* teste si le point est sur le bord de l'image */
        {
          fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d z=%d\n", F_NAME, j%rs,(j%ps)/rs, j/ps);
          exit (0);
        }
        D[j] = d;  /* on attribue au point courant du noeud le niveau de la branche ascendante */

        noeud = getbranch3d26 (X, rs, ps, N, j, extension, D, NON_RACINE);
                   /* transmet l'adresse de la structure du sous-arbre inférieur au point courant */
        noeud->longBranche++;  /* incrémente la valeur de la longueur de la branche */
      }
    }
  }
  
  return(noeud);
} // lgettree3d26()

/* ==================================== */
liste * fils3d26(uint8_t *X,  /* donnee: pointeur sur l'image x */
                 int32_t rs,            /* donnee: taille ligne */
                 int32_t ps,            /* donnee: taille plan */
		 int32_t N,             /* donnee: taille image */
                 int32_t current_pixel, /* donnee: numéro pixel courant */  
                 int32_t extension,     /* donnee: longueur de l'extension des intersections */
                 int32_t *D,       /* donnee: pointeur sur distances */
		 int32_t compteur       /* donnee: compteur pour l'extension d'intersection */
) /* Liste les fils d'une intersection avec la 26-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "fils3d26"
{ 
  liste * ptrListe = NULL;
  liste entete;
  int32_t d, k, j, i;

  ptrListe = &entete; 
  ptrListe->suivant = NULL;   

  i = 0;
  d = D[current_pixel];                             /* récupère le niveau dans l'arbre du point courant */

  if (mctopo3d_nbvoiso26 (X, current_pixel, rs, ps, N) > 2)  /* teste le nombre de voisins */
  {
    if (compteur != extension)  /* si il y a des voxels de liaison entre les noyaux de l'intersection */
    {
      d--;    /* décrémente le niveau de branche puisque l'intersection détectée fait partie du même noeud */
      BaisserBranche3d26 (rs, ps, N, current_pixel, D, d);  /* corrige les voxels mis à un niveau supérieur */
    }
    
    for (k = 0; k < 26; k += 1)                     /* regarde tous les voisins potentiels */
    {
      j = voisin26 (current_pixel, k, rs, ps, N);   /* teste la présence du voisin */
      if ((j != -1) && (D[j] == -1))                /* vérifie qu'ils n'ont pas été déjà traités */
      {
        if (nonbord3d (j, rs, ps, N) == 0)          /* teste si le point est sur le bord de l'image */
        {
          fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d z=%d\n", F_NAME, j%rs,(j%ps)/rs, j/ps);
          exit (0);
        }

	D[j] = d;   /* on attribue au point courant du noeud le niveau de la branche ascendante */
        ptrListe->suivant = fils3d26 (X, rs, ps, N, j, extension, D, extension);    /* ... rajoute l'élément à la liste */
	while (ptrListe->suivant != NULL) ptrListe = ptrListe->suivant;  /* se positionne en fin de liste */
      }
    }
  }
  else  /* 2 voisins ou moins */ /* on se trouve dans le cas d'un début de branche */
  {
    for (k = 0; k < 26; k += 1)                     /* regarde tous les voisins potentiels */
    {
      j = voisin26 (current_pixel, k, rs, ps, N);   /* teste la présence du voisin */
      if ((j != -1) && (D[j] == -1))                /* vérifie qu'ils n'ont pas été déjà traités */
      {
        if (nonbord3d (j, rs, ps, N) == 0)          /* teste si le point est sur le bord de l'image */
        {
          fprintf(stderr, "%s: traitement des bords impossible: x=%d y=%d z=%d\n", F_NAME, j%rs,(j%ps)/rs, j/ps);
          exit (0);
        }
        
	if (compteur == extension)
        {
          D[current_pixel] = d + 1; /* corrige la valeur du niveau de branche courant (point directement après le noyau de l'intersection) */
          D[j] = d +1;              /* attribut le niveau de branche au point suivant de la branche */
        }
	else
        {
	  D[j] = d;                 /* attribut le niveau de branche au point suivant de la branche */
        }
	compteur--;  /* décrémente le nombre de pixels à parcourir dans l'extension de l'intersection */
	i++;         /* incrémente le nombre de voisins traités */
	
        if (compteur > 0)
	{
	  ptrListe->suivant = fils3d26 (X, rs, ps, N, j, extension, D, compteur);  /* propagation de la zone d'intersection au voxel suivant */
	}
	else  /* compteur == 0 */
	{
          entete.suivant = malloc (sizeof (liste)); /* crée un élément supplémentaire de la liste */
          entete.suivant->valeur = j;               /* copie la coordonnée du point dans la liste des fils */
          entete.suivant->suivant = NULL;           /* pas d'élément suivant */
        }
      }
    }
    if ((i == 0) && (compteur != extension))  /* s'il n'y a pas de voisin traité (on se trouve à l'extrémité d'un embryon de branche avortée) */
    {                                         /* et que le point courant n'est pas un voisin du noyau de l'intersection */
      d--;    /* décrémente le niveau de branche puisque l'embryon de branche détecté fait partie du noeud */
      BaisserBranche3d26 (rs, ps, N, current_pixel, D, d);  /* corrige les voxels mis à un niveau supérieur */
    }
  }

  return (entete.suivant);
} // fils3d26()


/* ==================================== */
int32_t BaisserBranche3d26(int32_t rs,            /* donnee: taille ligne */
                       int32_t ps,            /* donnee: taille plan */
		       int32_t N,             /* donnee: taille image */
                       int32_t current_pixel, /* donnee: numéro pixel courant */  
                       int32_t *D,       /* donnee: pointeur sur distances */
		       int32_t level          /* donnee: valeur de remplacement */
) /* Liste les fils d'une intersection avec la 26-connexité */
/* ==================================== */
#undef F_NAME
#define F_NAME "BaisserBranche3d26"
{ 
  int32_t j, k;
  
  for (k = 0; k < 26; k += 1)                     /* regarde tous les voisins potentiels */
  {
    j = voisin26 (current_pixel, k, rs, ps, N);   /* teste la présence du voisin */
    if ((j != -1) && (D[j] > level))              /* vérifie que ce sont bien les points à traiter */
    {
      D[j] = level;               /* attribut le niveau de branche au point suivant de la branche */
      BaisserBranche3d26 (rs, ps, N, j, D, level);
    }
  }
  return (0);
} // BaisseBranche3d26


/* ==================================== */
int32_t MenuEnregistrement (char * chaine)
/* Affiche le menu d'enregistrement */
/* ==================================== */
{ 
  printf ("%sQue désirez-vous faire?\n", chaine);
  printf ("%s  1) Enregistrer la structure de l'arbre complète\n", chaine);
  printf ("%s  2) Enregistrer la structure de l'arbre réduit\n\n", chaine);

  return (0);
} //MenuEnregistrement


/* ==================================== */
int32_t Menu1 ()
/* Affiche le menu */
/* ==================================== */
{ 
  printf ("  3) Récupérer les marqueurs des poumons\n");
  printf ("  4) Récupérer les marqueurs des lobes\n");
  
  printf ("\n  0) Quitter\n");
  printf ("Faites votre choix puis \"ENTER\":  ");

  return (0);
} //Menu1


/* ==================================== */
int32_t Menu2 ()
/* Affiche le menu */
/* ==================================== */
{ 
  printf ("      3) Enregistrer l'image résultat\n");
  
  printf ("\n      0) Retour au menu principal\n");
  printf ("    Faites votre choix puis \"ENTER\":  ");

  return (0);
} //Menu2


/* ==================================== */
int32_t Choix1 (noeudArbre * arbre,          /* donnee: pointeur sur l'arbre */
	    struct xvimage * etiquette   /* donnee: image etiquettée */
) /* Gère le choix de l'utilisateur */
/* ==================================== */
{ 
  int32_t * choix = (int32_t *) malloc (sizeof (int32_t));
  char * chaine = (char *) malloc (256);
  * choix = -1;  /* évite la conservation du choix précédent au cas où l'utilisateur se trompe
                    et entre une chaîne de caractères */
  
  printf ("\n  MENU PRINCIPAL\n");
  MenuEnregistrement ("");
  Menu1 ();
  
  while (scanf ("%d", choix) || scanf ("%s", chaine))
  {
    switch (* choix)
    {
      case 1:     /* arbre complet */
      case 2:     /* arbre réduit */
        ChoixEnregistrement (arbre, *choix);
	break;
      case 3:     /* Extraction des poumons */
	Choix2 (arbre, POUMONS, etiquette);
	break;
      case 4:     /* Extraction des poumons */
	Choix2 (arbre, LOBES, etiquette);
	break;
      case 0:     /* fin du programme */
	freeArbre (arbre);
	free (chaine);
	printf ("\nFIN DU PROGRAMME...\n");
	return (0);
	break;
      default:      /* cas par défaut (saisie erronée) */
	printf ("\n    ERREUR DE SAISIE...\n");
	break;
    }
    * choix = -1;  /* évite la conservation du choix précédent au cas où l'utilisateur se trompe
                      et entre une chaîne de caractères */
    printf ("\n  MENU PRINCIPAL\n");
    MenuEnregistrement ("");
    Menu1 ();
  }
  
  return (0);
} //Choix


/* ==================================== */
int32_t Choix2 (noeudArbre * arbre,          /* donnee: pointeur sur l'arbre */
            int32_t choixSegment,            /* donnee: choix de la segmentation */  
	    struct xvimage * etiquette   /* donnee: image etiquettée */
) /* Gère le choix de l'utilisateur */
/* ==================================== */
{ 
  listeNoeud * origines = NULL;
  listeNoeud * ptrListe = NULL;
  int32_t nbElements = 0;
  char * chaine = (char *) malloc (256);
  char * nomFichier = (char *) malloc (256);
  int32_t * choix = (int32_t *) malloc (sizeof (int32_t));
  * choix = -1;  /* évite la conservation du choix précédent au cas où l'utilisateur se trompe
                    et entre une chaîne de caractères */
  
  if (choixSegment == POUMONS)
    printf ("\n      MENU EXTRACTION DES POUMONS\n");
  else
    printf ("\n      MENU EXTRACTION DES LOBES\n");

  origines = SegmentPoumons (arbre, choixSegment);

  if (origines == NULL)
    printf ("\n  ATTENTION!!! Pas de sous-arbre à ce niveau...\n");

  ptrListe = origines;
  while (ptrListe != NULL)  /* teste le nombre de composantes déjà présentes */
  {
    ptrListe = ptrListe->suivant;
    nbElements++;
  }
  printf ("    Nombre de marqueurs: %d\n", nbElements);

  
  MenuEnregistrement ("    ");
  Menu2 ();
  
  while (scanf ("%d", choix) || scanf ("%s", chaine))
  {
    switch (* choix)
    {
      case 1:     /* arbre complet centré */
      case 2:     /* arbre réduit centré */
        ChoixEnregistrement (arbre, *choix);
	break;
      case 3:     /* Enregistrement image */
        printf ("\nEnregistrement image:\n");
        printf ("    Nom du fichier d'enregistrement:  ");
        scanf ("%s", nomFichier);
        TronquerImage (origines, etiquette, nomFichier);
        printf ("    Arbre enregistré dans le fichier: %s\n", nomFichier);
	break;
      case 0:     /* fin du programme */
	ArbreVisible (arbre);
	free (chaine);
	free (nomFichier);
	freeListeNoeud (origines);
	printf ("\nRetour au menu principal...\n");
	return (0);
	break;
      default:      /* cas par défaut (saisie erronée) */
	printf ("\n    ERREUR DE SAISIE...\n");
	break;
    }
    * choix = -1;  /* évite la conservation du choix précédent au cas où l'utilisateur se trompe
                      et entre une chaîne de caractères */
    if (choixSegment == POUMONS)
      printf ("\n      MENU EXTRACTION DES POUMONS\n");
    else
      printf ("\n      MENU EXTRACTION DES LOBES\n");

    MenuEnregistrement ("    ");
    Menu2 ();
  }
  
  return (0);
} //Choix


/* ==================================== */
int32_t ChoixEnregistrement (noeudArbre * arbre,  /* donnee: pointeur sur l'arbre */
                         int32_t choix
) /* Gère le choix de l'enregistrement */
/* ==================================== */
{ 
  char * nomFichier = (char *) malloc (256);
  FILE * fichier;
  
  switch (choix)
  {
    case 1:     /* arbre complet */
      printf ("\nEnregistrement arbre complet:\n");
      printf ("    Nom du fichier d'enregistrement:  ");
      scanf ("%s", nomFichier);
      fichier = fopen (nomFichier, "w");
      printArbreComplet (arbre, fichier);
      fclose (fichier);
      printf ("    Arbre enregistré dans le fichier: %s\n", nomFichier);
      break;
    case 2:     /* arbre réduit */
      printf ("\nEnregistrement arbre réduit:\n");
      printf ("    Nom du fichier d'enregistrement:  ");
      scanf ("%s", nomFichier);
      fichier = fopen (nomFichier, "w");
      printArbreReduit (arbre, fichier);
      fclose (fichier);
      printf ("    Arbre enregistré dans le fichier: %s\n", nomFichier);
      break;
    default:      /* cas par défaut (saisie erronée) */
      printf ("\n    ERREUR DE SAISIE...\n");
      break;
  }
  
  free (nomFichier);
  
  return (0);
} //ChoixEnregistrement


/* ==================================== */
int32_t printArbreComplet (noeudArbre * arbre,  /* donnee: pointeur sur l'arbre */
                       FILE * fichier       /* donnee: pointeur sur la sortie (fichier ou console) */
) /* Affiche la structure d'arbre obtenue */
/* ==================================== */
#undef F_NAME
#define F_NAME "printArbreComplet"
{ 
  int32_t i;
  
  caseArbre * ligne = NULL;
  ligne = malloc (sizeof (caseArbre));
  
  ligne->noeud = arbre;
  ligne->level = 1;
  ligne->nbFeuilles = NbFeuilles (arbre, 1);
  ligne->numFils = 0;
  ligne->nbFreres = 0;
  ligne->suivant = NULL;
  ligne->visiblePere = INVISIBLE;
  ligne->colonne = ligne->nbFeuilles / 2;
  ligne->colonnePere = ligne->nbFeuilles / 2;
  ligne->debutPere = 0;
  
/* Type d'affichage à faire: */
/*
  fprintf (fichier, "                 |             "); fprintf (fichier, "\n");
  fprintf (fichier, "                 |             "); fprintf (fichier, "\n");
  fprintf (fichier, "    +------------+------------+"); fprintf (fichier, "\n");
  fprintf (fichier, "    | N° NOEUD: %13d |", ligne->noeud->numNoeud); fprintf (fichier, "\n");
  fprintf (fichier, "    +-------------------------+"); fprintf (fichier, "\n");
  fprintf (fichier, "    | X=%4d  Y=%4d  Z=%4d  |", ligne->noeud->coord_X, ligne->noeud->coord_Y, ligne->noeud->coord_Z); fprintf (fichier, "\n");
  fprintf (fichier, "    | Longueur branche: %5d |", ligne->noeud->longBranche); fprintf (fichier, "\n");
  fprintf (fichier, "    | Niveau courant: %7d |", ligne->noeud->level); fprintf (fichier, "\n");
  fprintf (fichier, "    +-------------------------+"); fprintf (fichier, "\n");
  fprintf (fichier, "    | Niveau branches dessous |"); fprintf (fichier, "\n");
  fprintf (fichier, "    | MAX=%6d   MIN=%6d |", ligne->noeud->levelMax, ligne->noeud->levelMin); fprintf (fichier, "\n");
  fprintf (fichier, "    | Nombre feuilles: %6d |", ligne->noeud->nbFeuilles); fprintf (fichier, "\n");
  fprintf (fichier, "    +------------+------------+"); fprintf (fichier, "\n");
  fprintf (fichier, "                 |             "); fprintf (fichier, "\n");
  fprintf (fichier, "                 |             "); fprintf (fichier, "\n");
  fprintf (fichier, "                 +-------------"); fprintf (fichier, "\n");

  fprintf (fichier, "-----------------+-------------"); fprintf (fichier, "\n");
  fprintf (fichier, "-----------------+             "); fprintf (fichier, "\n");
  fprintf (fichier, "                               "); fprintf (fichier, "\n");
*/

  while (ligne != NULL)  /* pour toutes les lignes d'éléments dans l'arbre... */
  {
    /* Traitement spécifique à faire pour chaque ligne... */
    printDataComplet (ligne, LIAISON, fichier);

    for (i = 0; i < HAUTEUR_LIEN; i++)
      printDataComplet (ligne, LIEN_HAUT, fichier);
      
    printDataComplet (ligne, CADRE_HAUT, fichier);
    printDataComplet (ligne, NUM_NOEUD, fichier);
    printDataComplet (ligne, LIGNE, fichier);
    printDataComplet (ligne, COORD, fichier);
    printDataComplet (ligne, LONGUEUR_BRANCHE, fichier);
    printDataComplet (ligne, LEVEL, fichier);
    printDataComplet (ligne, LIGNE, fichier);
    printDataComplet (ligne, BRANCHES, fichier);
    printDataComplet (ligne, MAX_MIN, fichier);
    printDataComplet (ligne, NB_FEUILLES, fichier);
    printDataComplet (ligne, CADRE_BAS, fichier);
    
    for (i = 0; i < (2 * HAUTEUR_LIEN); i++)
      printDataComplet (ligne, LIEN_BAS, fichier);
    
    ligne = NextLevel (ligne);  /* prépare la prochaine ligne */
  }
  return (0);
} //printArbreComplet


/* ==================================== */
int32_t printArbreReduit (noeudArbre * arbre,  /* donnee: pointeur sur l'arbre */
                      FILE * fichier       /* donnee: pointeur sur la sortie (fichier ou console) */
) /* Affiche la structure d'arbre obtenue simplifiée */
/* ==================================== */
#undef F_NAME
#define F_NAME "printArbreReduit"
{ 
  int32_t i;
  
  caseArbre * ligne = NULL;
  ligne = malloc (sizeof (caseArbre));
  
  ligne->noeud = arbre;
  ligne->level = 1;
  ligne->nbFeuilles = NbFeuilles (arbre, 1);
  ligne->numFils = 0;
  ligne->nbFreres = 0;
  ligne->suivant = NULL;
  ligne->visiblePere = INVISIBLE;
  ligne->colonne = ligne->nbFeuilles / 2;
  ligne->colonnePere = ligne->nbFeuilles / 2;
  ligne->debutPere = 0;
  
  while (ligne != NULL)  /* pour toutes les lignes d'éléments dans l'arbre... */
  {
    /* Traitement spécifique à faire pour chaque ligne... */
    printDataReduit (ligne, LIAISON, fichier);

    for (i = 0; i < HAUTEUR_LIEN; i++)
      printDataReduit (ligne, LIEN_HAUT, fichier);
      
    ligne = NextLevel (ligne);  /* prépare la prochaine ligne */
  }
  return (0);
} //printArbreReduit


/* ==================================== */
listeNoeud * SegmentPoumons (noeudArbre * arbre,  /* donnee: pointeur sur l'arbre */
                             int32_t choix            /* donnee: récupération des poumons ou des lobes */
) /* Divise l'arbre en deux parties (pour chaque poumon) et appelle la procédure de séparation en lobes
    (SupprimerNoeud) si demandé */
/* ==================================== */
#undef F_NAME
#define F_NAME "ExtraireLobes"
{ 
  listeNoeud * ptrListe = NULL;
  listeNoeud entete;
  int32_t i;
  int32_t nbComp[2] = {2, 3};                /* on a 2 lobes dans le poumon gauche et 3 dans le poumon droit */

  entete.suivant = NULL;
  ptrListe = &entete;
  
  ArbreVisible (arbre);                  /* réinitialise la visibilité int32_tégrale de l'arbre */
  
  ElaguerArbre (arbre, arbre->levelMin); /* met à niveau les extrémités de sbranches */
  arbre->visible = INVISIBLE;            /* efface la racine de l'arbre */

  for (i = 0; i < arbre->nbFils; i++)    /* pour chaque sous-arbre de la racine */
  {
    ptrListe->suivant = malloc (sizeof (liste));
    ptrListe->suivant->noeud = arbre->fils[i];
    ptrListe->suivant->valeur = arbre->levelMin;
    ptrListe->suivant->suivant = NULL;

    if (choix == LOBES)
      ptrListe->suivant = (listeNoeud *) SupprimerNoeud (ptrListe->suivant, nbComp[i]);
      /* supprime des noeuds jusqu'à avoir le nombre de composantes désirées */
    
    while (ptrListe->suivant != NULL) ptrListe = ptrListe->suivant;  /* se positionne en fin de liste */
  }
  
  return (entete.suivant);
} //SegmentPoumons


/* ==================================== */
listeNoeud * SupprimerNoeud (listeNoeud * ptrListe,  /* donnee: pointeur sur l'arbre */
	                     int32_t nbComponents        /* donnee: nombre de noeuds à supprimer */
) /* Supprime de l'affichage le noeud au nombre de feuilles le plus important dans l'arbre */
/* ==================================== */
#undef F_NAME
#define F_NAME "SupprimerNoeud"
{ 
  listeNoeud entete;
  listeNoeud prov;
  listeNoeud * elementPrecedent = NULL;
  listeNoeud * elementMaxFeuilles = NULL;
  listeNoeud * elementMaxPrecedent = NULL;
  noeudArbre * sousArbre = NULL;
  noeudArbre * arbreMax = NULL;
  int32_t maxFeuilles = 0;
  int32_t nbFeuilles = 0;
  int32_t nbElements = 0;
  int32_t i;
  
  entete.suivant = ptrListe;
  elementPrecedent = &entete;

  while (ptrListe != NULL)  /* teste le nombre de composantes déjà présentes */
  {
    ptrListe = ptrListe->suivant;
    nbElements++;
  }
  
  if (nbElements <= 0)
  {
    fprintf(stderr, "%s: liste vide\n", F_NAME);
    exit (0);
  }
  
  if (nbElements < nbComponents)
  {
    ptrListe = entete.suivant;
    
    while (ptrListe != NULL)  /* on parcourt les noeuds listés */
    {
      sousArbre = ptrListe->noeud;              /* récupère l'adresse du noeud correspondant */
      nbFeuilles = NbFeuilles (sousArbre, 1);   /* calcule le nombre de feuilles */
      if (nbFeuilles > maxFeuilles)             /* s'il a plus de feuilles que le max déjà trouvé... */
      {
        maxFeuilles = nbFeuilles;               /* ...stocke la nouvelle valeur max */
        arbreMax = sousArbre;                   /* ...stocke l'adresse du noeud */
        elementMaxFeuilles = ptrListe;          /* ...stocke l'adresse de l'élément */
        elementMaxPrecedent = elementPrecedent; /* ...stocke l'adresse de l'élément précédent */
      }
      else if (nbFeuilles == maxFeuilles)          /* s'il a le même nombre de feuilles que le max déjà trouvé... */
        if (sousArbre->longBranche > arbreMax->longBranche)  /* compare la longueur des branches menant au noeud... */
        {
          arbreMax = sousArbre;                              /* conserve l'adresse du noeud ayant la plus grande branche */
          elementMaxFeuilles = ptrListe;                     /* conserve l'adresse de l'élément correspondant */
        }

    elementPrecedent = ptrListe;            /* conserve l'adresse courante en mémoire */
    ptrListe = ptrListe->suivant;           /* avance dans la liste */
    }
  
    ElaguerArbre (arbreMax, arbreMax->levelMin);  /* met à niveau les extrémités de sbranches */
    arbreMax->visible = INVISIBLE;  /* efface le noeud correspondant au plus grand nombre de feuilles */

    prov.suivant = NULL;
    ptrListe = &prov;
    
    for (i = 0; i < arbreMax->nbFils; i++)  /* crée une liste provisoire contenant les éléments à rajouter */
    {
      ptrListe->suivant = malloc (sizeof (liste));
      ptrListe = ptrListe->suivant;

      ptrListe->noeud = arbreMax->fils[i];
      ptrListe->valeur = arbreMax->levelMin;
      ptrListe->suivant = NULL;

    }

    /* remplace l'élément de la liste correspondant au plus grand nombre de feuilles par la liste provisoire */
    ptrListe->suivant = elementMaxFeuilles->suivant;
    free (elementMaxFeuilles);
    elementMaxPrecedent->suivant = prov.suivant;

    entete.suivant = (listeNoeud *) SupprimerNoeud (entete.suivant, nbComponents);
  }
  
  return (entete.suivant);
} //SupprimerNoeud


/* ==================================== */
int32_t TronquerImage (listeNoeud * ptrListe,       /* donnee: pointeur sur la liste des noeuds de départ des marqueurs */
	           struct xvimage * etiquette,  /* donnee: image etiquettée */
                   char * nomFichier            /* donnee: nom du fichier d'enregistrement */
) /* Enregistre l'image des marqueurs */
/* ==================================== */
#undef F_NAME
#define F_NAME "TronquerImage"
{ 
  struct xvimage * result;
  int32_t * M;
  int32_t * D;
  noeudArbre * noeud;
  int32_t i;

  int32_t rs = etiquette->row_size;
  int32_t cs = etiquette->col_size;
  int32_t ds = depth(etiquette);
  int32_t ps = rs * cs;          /* taille d'un plan */
  int32_t N = ps * ds;           /* taille de l'image */
  D = SLONGDATA(etiquette);
  
  result = allocimage(NULL, rowsize(etiquette), colsize(etiquette), depth(etiquette), VFF_TYP_4_BYTE);
  if (result == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }

  M = SLONGDATA(result);
  
  for (i = 0; i < N; i++)   /* parcours des points de l'image */
    M[i] = 0;               /* copie l'image originale */

  while (ptrListe != NULL)  /* pour chaque noeud de départ d'une branche... */
  {
    noeud = ptrListe->noeud;
    ConstruireMarqueurs (D, M, noeud->num_pixel, rs, ps, N, ptrListe->valeur);
    ptrListe = ptrListe->suivant;
  }
  
  result->name = NULL;
  writeimage(result, nomFichier);
  freeimage(result);
  
  return (0);
} //TronquerImage


/* ==================================== */
int32_t ConstruireMarqueurs (int32_t * D,      /* donnee: pointeur sur distances */
                         int32_t * M,      /* donnee: pointeur sur image de marqueurs */
                         int32_t current_pixel, /* donnee: numéro pixel courant */
                         int32_t rs,            /* donnee: taille ligne */
                         int32_t ps,            /* donnee: taille plan */
		         int32_t N,             /* donnee: taille image */
		         int32_t brancheMax     /* donnee: valeur max des branches à afficher */
) /* Supprime de l'image les branches qui ne sont pas des marqueurs */
/* ==================================== */
#undef F_NAME
#define F_NAME "ConstruireMarqueurs"
{ 
  int32_t i, j, k;
 
  M[current_pixel] = D[current_pixel];           /* affiche le point de la branche marqueur */
  for (k = 0; k < 26; k += 1)                    /* regarde tous les voisins potentiels */
  {
    j = voisin26 (current_pixel, k, rs, ps, N);              /* teste la présence du voisin */
    if ((j != -1) && (D[j] >= D[current_pixel]) && (D[j] <= brancheMax) && (M[j] == 0))
      /* vérifie qu'ils font parties des branches marqueurs et qu'ils n'ont pas été déjà traités*/
      ConstruireMarqueurs (D, M, j, rs, ps, N, brancheMax);  /* ... poursuit la construction de la branche */
  }

  return (0);
} //ConstruireMarqueurs


/* ==================================== */
int32_t printDataComplet (caseArbre * ligne, /* donnee: pointeur sur la ligne */
                      int32_t data,          /* donnee: type de donnée à afficher */
                      FILE * fichier     /* donnee: pointeur sur le fichier d'écriture */
) /* Affiche la donnée désirée */
/* ==================================== */
#undef F_NAME
#define F_NAME "printDataComplet"
{ 
  int32_t k;

  k = 0;
  while (ligne != NULL)  /* pour tous les éléments dans la ligne... */
  {
    if (data != LIAISON)
    {
      while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
      {
        fprintf (fichier, "                               ");
        k++;
      }
      if ((ligne->noeud->visible == VISIBLE) || (ligne->noeud->visible == INVISIBLE_BAS))  /* si le noeud est à afficher */
      {
        switch (data)
        {
          case LIEN_HAUT:
            fprintf (fichier, "                 |             ");
            break;
          case LIEN_BAS:
            if (ligne->noeud->nbFils > 0)
              fprintf (fichier, "                 |             ");
            else
              fprintf (fichier, "                               ");
            break;
          case CADRE_HAUT:
            fprintf (fichier, "    +------------+------------+");
            break;
          case CADRE_BAS:
            if (ligne->noeud->nbFils > 0)
              fprintf (fichier, "    +------------+------------+");
            else
              fprintf (fichier, "    +-------------------------+");
            break;
          case NUM_NOEUD:
            fprintf (fichier, "    | N° NOEUD: %13d |", ligne->noeud->numNoeud);
            break;
          case LIGNE:
            fprintf (fichier, "    +-------------------------+");
            break;
          case COORD:
            fprintf (fichier, "    | X=%4d  Y=%4d  Z=%4d  |", ligne->noeud->coord_X, ligne->noeud->coord_Y, ligne->noeud->coord_Z);
            break;
          case LONGUEUR_BRANCHE:
            fprintf (fichier, "    | Longueur branche: %5d |", ligne->noeud->longBranche);
            break;
          case LEVEL:
            fprintf (fichier, "    | Niveau courant: %7d |", ligne->noeud->level);
            break;
          case BRANCHES:
            fprintf (fichier, "    | Niveau branches dessous |");
            break;
          case MAX_MIN:
            fprintf (fichier, "    | MAX=%6d   MIN=%6d |", ligne->noeud->levelMax, ligne->noeud->levelMin);
            break;
          case NB_FEUILLES:
            if (ligne->noeud->nbFils > 0)
              fprintf (fichier, "    | Nombre feuilles: %6d |", ligne->nbFeuilles);
            else
              fprintf (fichier, "    |         FEUILLE         |", ligne->nbFeuilles);
            break;
          default:
            fprintf(stderr, "%s: Erreur affichage arbre!n", F_NAME);
            exit (0);
            break;
	}
      }
      else                              /* si le noeud n'est pas à afficher */
        fprintf (fichier, "                               ");  /* ...lorsqu'on ne veut pas afficher le noeud */
    }
    else  /* data == LIAISON */
    {
      if (ligne->visiblePere == VISIBLE)/* si le père est affiché */
      {
        if (ligne->nbFreres > 0)        /* vérifie s'il y a des frères */
        {
          if (ligne->numFils == 0)      /* vérifie si c'est le premier fils */
          {
            while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
            {
              fprintf (fichier, "                               ");
              k++;
            }
            fprintf (fichier, "                 +-------------");
          }
          else                          /* si ce n'est pas le premier fils */
          {
            while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
            {
              if (k != ligne->colonnePere)
                fprintf (fichier, "-------------------------------");
              else
                fprintf (fichier, "-----------------+-------------");
              k++;
            }
        
            if (ligne->numFils == ligne->nbFreres)  /* si on est au dernier des fils... */
              fprintf (fichier, "-----------------+             ");
            else                                    /* sinon... */
              fprintf (fichier, "-----------------+-------------");
          }
        }
        else                          /* s'il n'y a pas de frères */
        {
          while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
          {
            fprintf (fichier, "                               ");
            k++;
          }
          fprintf (fichier, "                 |             ");
        }
      }
      else                            /* si le père n'est pas affiché */
      {
        while (k < ligne->colonne)    /* on atteint le numéro de colonne désiré */
        {
          fprintf (fichier, "                               ");
          k++;
        }
        fprintf (fichier, "                               ");
      }
    }
    
    k++;
    ligne = ligne->suivant;
  } 
  
  fprintf (fichier, "\n");
  return (0);
}


/* ==================================== */
int32_t printDataReduit (caseArbre * ligne, /* donnee: pointeur sur la ligne */
                     int32_t data,          /* donnee: type de donnée à afficher */
                     FILE * fichier     /* donnee: pointeur sur le fichier d'écriture */
) /* Affiche la donnée désirée */
/* ==================================== */
#undef F_NAME
#define F_NAME "printDataReduit"
{ 
  int32_t k;

  k = 0;
  while (ligne != NULL)  /* pour tous les éléments dans la ligne... */
  {
    if (data != LIAISON)
    {
      while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
      {
        fprintf (fichier, "   ");
        k++;
      }
      if ((ligne->noeud->visible == VISIBLE) || (ligne->noeud->visible == INVISIBLE_BAS))  /* si le noeud est à afficher */
      {
        switch (data)
        {
          case LIEN_HAUT:
            fprintf (fichier, " | ");
            break;
          case LIEN_BAS:
            if (ligne->noeud->nbFils > 0)
              fprintf (fichier, " | ");
            else
              fprintf (fichier, "   ");
            break;
          default:
            fprintf(stderr, "%s: Erreur affichage arbre!n", F_NAME);
            exit (0);
            break;
        }
      }
      else                         /* si le noeud n'est pas à afficher */
        fprintf (fichier, "   ");  /* ...lorsqu'on ne veut pas afficher le noeud */
    }
    else  /* data == LIAISON */
    {
      if (ligne->visiblePere == VISIBLE)/* si le père est affiché */
      {
	if (ligne->nbFreres > 0)        /* vérifie s'il y a des frères */
        {
	  if (ligne->numFils == 0)      /* vérifie si c'est le premier fils */
          {
            while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
            {
              fprintf (fichier, "   ");
              k++;
            }
            fprintf (fichier, " +-");
          }
          else                          /* si ce n'est pas le premier fils */
          {
            while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
            {
              if (k != ligne->colonnePere)
                fprintf (fichier, "---");
              else
                fprintf (fichier, "-+-");
              k++;
            }
          
            if (ligne->numFils == ligne->nbFreres)  /* si on est au dernier des fils... */
              fprintf (fichier, "-+ ");
            else                                    /* sinon... */
              fprintf (fichier, "-+-");
          }
        }
        else                          /* s'il n'y a pas de frères */
        {
          while (k < ligne->colonne)  /* on atteint le numéro de colonne désiré */
          {
            fprintf (fichier, "   ");
            k++;
          }
          fprintf (fichier, " + ");
        }
      }
      else                            /* si le père n'est pas affiché */
      {
        while (k < ligne->colonne)    /* on atteint le numéro de colonne désiré */
        {
          fprintf (fichier, "   ");
          k++;
        }
        fprintf (fichier, "   ");
      }
    }
    
    k++;
    ligne = ligne->suivant;
  } 
  
  fprintf (fichier, "\n");
  return (0);
}


/* ==================================== */
caseArbre * NextLevel (caseArbre * ligne)  /* donnee: pointeur sur un élément de l'arbre */
/* Liste les éléments d'un niveau de l'arbre */
/* ==================================== */
#undef F_NAME
#define F_NAME "NextLevel"
{ 
  int32_t k, l;
  caseArbre * parcoursLigne = NULL;
  caseArbre * elementLigne = NULL;
  caseArbre entete;
  
  entete.suivant = NULL;
  parcoursLigne = ligne;   /* parcours de la ligne courante */
  elementLigne = &entete;  /* prépare la nouvelle ligne */
  
  while (parcoursLigne != NULL)    /* pour tous les éléments de la ligne... */
  {
    l = 1;
    for (k = 0; k < parcoursLigne->noeud->nbFils; k++)  /* pour tous les fils du noeud de l'élément de la ligne courante... */
    {
      elementLigne->suivant = malloc (sizeof (caseArbre));              /* crée un nouvel élément de la ligne suivante */
      elementLigne = elementLigne->suivant;
      elementLigne->suivant = NULL;
      elementLigne->noeud = parcoursLigne->noeud->fils[k];    /* indique le noeud correspondant à l'élément de la ligne suivante */
      elementLigne->level = (parcoursLigne->level + 1);       /* indique le niveau du noeud de l'élément */
      if (elementLigne->level != elementLigne->noeud->level)  /* vérifie que le noeud appartient bien au bon niveau */
      {
        fprintf (stderr, "%s: Erreur noeud placé à un mauvais niveau!n", F_NAME);
        exit (0);
      }
      elementLigne->numFils = k;                              /* indique le numéro de fils du noeud de l'élément */
      elementLigne->nbFreres = (parcoursLigne->noeud->nbFils - 1);
                                               /* indique le nombre de frères du noeud de l'élément */
                                               /* (nombre de fils issus du même père, noeud de l'élément courant non compris) */
      elementLigne->nbFeuilles = NbFeuilles (elementLigne->noeud, 1);  /* indique le nombre de feuilles du noeud de l'élément */
      
      elementLigne->visiblePere = parcoursLigne->noeud->visible;
      elementLigne->colonnePere = parcoursLigne->colonne;
      elementLigne->debutPere = parcoursLigne->colonne - parcoursLigne->nbFeuilles / 2;
                                               /* calcule la première colonne occupée par une feuille du père */
      elementLigne->colonne = (elementLigne->nbFeuilles / 2 + (l - 1) + k + elementLigne->debutPere);
                                               /* calcule le numéro de colonne à occuper */
      
      l = (l - 1) + elementLigne->nbFeuilles;  /* garde le nombre de colonnes à laisser libres pour les descendants des éléments */
                                               /* avant de placer le prochain élément de même niveau */
    }
  parcoursLigne = parcoursLigne->suivant;  /* on passe à l'élément suivant de la ligne courante */
  }
  
  freeLigne (ligne);  /* on libère l'ancienne ligne */
  
  return (entete.suivant);
}


/* ==================================== */
int32_t NbFils (liste * liste)    /* donnee: pointeur sur liste chainee */
/* Renvoie le nombre d'éléments de la liste chainee */
/* ==================================== */
#undef F_NAME
#define F_NAME "NbFils"
{ 
  int32_t nbFils = 0;
  if (liste != NULL)             /* s'il y a d'autres éléments dans la liste... */
    nbFils = 1 + NbFils (liste->suivant);  /* ... en renvoie le nombre, auquel on ajoute 1 */
  return (nbFils);
}


/* ==================================== */
int32_t NbFeuilles (noeudArbre * arbre, /* donnee: pointeur sur l'arbre */
                int32_t raz             /* donnee: indicateur de remise à zéro */
) /* Renvoie le nombre de feuilles que possède l'arbre */
/* ==================================== */
#undef F_NAME
#define F_NAME "NbFeuilles"
{ 
  static int32_t nbFeuilles = 0;
  int32_t k;
  if (raz) nbFeuilles = 0;
  if (arbre->nbFils == 0)   /* si le noeud n'a pas de fils... */
    nbFeuilles++;           /* ... on incrémente le nombre de feuilles (le noeud est une feuille) */
  else                      /* sinon... */
    for (k = 0; k < arbre->nbFils; k++)  /* ... pour tous les fils du noeud courant... */
    {
      NbFeuilles (arbre->fils[k], 0);       /* ... on calcule leur nombre de feuilles */
    }
  return (nbFeuilles);
}


/* ==================================== */
int32_t ArbreVisible (noeudArbre * arbre)  /* donnee: pointeur sur l'arbre */
/* Réinitialise l'affichage de tout l'arbre */
/* ==================================== */
#undef F_NAME
#define F_NAME "ArbreVisible"
{ 
  int32_t i;
  
  arbre->visible = VISIBLE;
  
  for (i = 0; i < arbre->nbFils; i++)  /* pour tous les fils du noeud... */
    ArbreVisible (arbre->fils[i]);     /* ...rend visible la branche */
  
  return (0);
} //ArbreVisible


/* ==================================== */
int32_t ElaguerArbre (noeudArbre * arbre,  /* donnee: pointeur sur l'arbre */
                  int32_t levelMax
) /* Met en invisible les bouts de certaines branches afin de les mettre à niveau égal */
/* ==================================== */
#undef F_NAME
#define F_NAME "ElaguerArbre"
{ 
  int32_t i;

  for (i = 0; i < arbre->nbFils; i++)  /* pour tous les fils du noeud... */
  {
    if (arbre->fils[i]->level > levelMax )
    {
      if (arbre->level <= levelMax)
        arbre->fils[i]->visible = INVISIBLE_BAS;
      else
        arbre->fils[i]->visible = INVISIBLE;
    }
    else
      arbre->fils[i]->visible = VISIBLE;
    ElaguerArbre (arbre->fils[i], levelMax);     /* ...se propage aux sous-branches */
  }
  
  return (0);
} //ElaguerArbre


/* ==================================== */
int32_t freeArbre (noeudArbre * arbre)    /* donnee: pointeur sur l'arbre */
/* Libère la mémoire occupée par l'arbre */
/* ==================================== */
#undef F_NAME
#define F_NAME "freeArbre"
{ 
  int32_t k;
  for (k = 0; k < arbre->nbFils; k++)  /* pour tous les fils du noeud courant... */
  {
    freeArbre (arbre->fils[k]);        /* ... les libère */
  }
  free (arbre);                        /* libère le noeud courant */
  return (0);
}


/* ==================================== */
int32_t freeLigne (caseArbre * ligne)    /* donnee: pointeur sur la ligne */
/* Libère la mémoire occupée par la structure de la ligne d'affichage */
/* ==================================== */
#undef F_NAME
#define F_NAME "freeLigne"
{ 
  if (ligne != NULL)             /* s'il y a d'autres éléments dans la ligne... */
    freeLigne (ligne->suivant);  /* ... les libère */
  free (ligne);                  /* libère l'élément courant */
  return (0);
}


/* ==================================== */
int32_t freeListeNoeud (listeNoeud * liste)    /* donnee: pointeur sur la ligne */
/* Libère la mémoire occupée par la structure de la ligne d'affichage */
/* ==================================== */
#undef F_NAME
#define F_NAME "freeListe"
{ 
  if (liste != NULL)                  /* s'il y a d'autres éléments dans la liste... */
    freeListeNoeud (liste->suivant);  /* ... les libère */
  free (liste);                       /* libère l'élément courant */
  return (0);
}
