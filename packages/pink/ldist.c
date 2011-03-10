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
* Purpose:     Calcul de la 4, 8-distance (2D), 6, 18 et 26-distance (3D) 
*              et de la distance euclidienne
*
* Input:       Image binaire
* Output:      Image en niveaux de gris (entiers longs)
* Written By:  Michel Couprie Avr  98 (ldist)
*              Michel Couprie Dec  99 (distances 3D)
*              Michel Couprie Mai  00 (distances 2D chamfrein)
*                a voir: reiteration peut-etre superflue - lire Borgefors
*              Michel Couprie Juin 00 (distances 3D chamfrein)
*              Xavier Daragon Mai 02 (distance euclidienne quadratique approx. 3D)
*              Michel Couprie Mai 02 (ldilatdisc, lerosdisc, ldistquad)
*              Rita Zrour Avril 04 (SEDT exacte Saito-Toriwaki)
*              Michel Couprie Juillet 04 (REDT exacte Coeurjolly)
*              Michel Couprie Août 04 (SEDT exacte lineaire Meijster et al.)
*              Jean Cousty janvier 2005 (REDT 3D)
*              Michel Couprie mai 2007 (Feature Transform Hesselink et al.) (NON TESTE)
*              Michel Couprie août 2009 (lopeningfunction)
*
****************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mclifo.h>
#include <mcutil.h>
#include <mcgeo.h>
#include <ldist.h>

//#define VERBOSE

#define VOI1(p)  ( *( (p)+1)         )        
#define VOI2(p)  ( *( (p)+1-rs) )
#define VOI3(p)  ( *( (p)-rs)   )
#define VOI4(p)  ( *( (p)-rs-1) )
#define VOI5(p)  ( *( (p)-1)         )
#define VOI6(p)  ( *( (p)-1+rs) )
#define VOI7(p)  ( *( (p)+rs)   )
#define VOI8(p)  ( *( (p)+rs+1) )

#define INFINI 1000000000

/* ==================================== */
void inverse(struct xvimage * image)
/* ==================================== */
{
  index_t i, N; uint8_t *pt;
  N = rowsize(image) * colsize(image) * depth(image);
  for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++) 
    if (*pt) *pt = 0; else *pt = NDG_MAX;
} /* inverse() */

/* ==================================== */
int32_t ldist(struct xvimage *img,   /* donnee: image binaire */       
          int32_t mode,
          struct xvimage *res    /* resultat: distances (image deja allouee) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldist"
{ 
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img);
  index_t ps = rs * cs;
  index_t N = ps * ds;       /* taille de l'image */
  uint8_t *F;                /* pointeur sur l'image */
  uint32_t *D;               /* pointeur sur les distances */
  index_t i, j, k, d;
  Lifo * LIFO1;
  Lifo * LIFO2;
  Lifo * LIFOtmp;

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }
  if (datatype(img) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: image type must be uint8_t\n", F_NAME);
    return(0);
  }
  if (datatype(res) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: result type must be uint32_t\n", F_NAME);
    return(0);
  }

  F = UCHARDATA(img);
  D = ULONGDATA(res);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {   fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  switch (mode)
  {
    case 4:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points marques */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = -1; 
          for (k = 0; k < 8; k += 2)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 8; k += 2)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 8: 
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points marques */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = -1; 
          for (k = 0; k < 8; k += 1)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 8; k += 1)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 6:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points marques */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = -1; 
          for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
          {
            j = voisin6(i, k, rs, ps, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
          {
            j = voisin6(i, k, rs, ps, N);
            if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 18:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points marques */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = -1; 
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            j = voisin18(i, k, rs, ps, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            j = voisin18(i, k, rs, ps, N);
            if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 26:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points marques */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = -1; 
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            j = voisin26(i, k, rs, ps, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            j = voisin26(i, k, rs, ps, N);
            if ((j != -1) && (D[j] == -1)) { D[j] = d + 1; LifoPush(LIFO2, j); }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    default: 
      fprintf(stderr, "%s(): bad mode: %d\n", F_NAME, mode);
      return 0;
  } /* switch (mode) */

  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} // ldist()

/* ==================================== */
int32_t ldistbyte(struct xvimage *img,   /* donnee: image binaire */       
          int32_t mode,
          struct xvimage *res    /* resultat: distances (image deja allouee) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "ldistbyte"
// le resultat est code sur 8 bits. 
// les distances doivent etre entre 0 et 254 (255 est reservee pour marquer)
// test de depassement effectue
#define MARK 255
{ 
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img);
  index_t ps = rs * cs;
  index_t N = ps * ds;           /* taille de l'image */
  uint8_t *F;          /* pointeur sur l'image */
  uint8_t *D;          /* pointeur sur les distances */
  index_t i, j, k, d;
  Lifo * LIFO1;
  Lifo * LIFO2;
  Lifo * LIFOtmp;

  if (datatype(img) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: image type must be uint8_t\n", F_NAME);
    return(0);
  }
  if (datatype(res) != VFF_TYP_1_BYTE)
  {
    fprintf(stderr, "%s: result type must be uint8_t\n", F_NAME);
    return(0);
  }

  F = UCHARDATA(img);
  D = UCHARDATA(res);

  LIFO1 = CreeLifoVide(N);
  LIFO2 = CreeLifoVide(N);
  if ((LIFO1 == NULL) || (LIFO2 == NULL))
  {
    fprintf(stderr, "%s() : CreeLifoVide failed\n", F_NAME);
    return(0);
  }

  switch (mode)
  {
    case 40:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points objet */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = MARK; 
          for (k = 0; k < 8; k += 2)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          
          for (k = 0; k < 8; k += 2)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (D[j] == MARK)) 
            {
              if (d == MARK-1)
	      {
                fprintf(stderr, "%s() : distance overflow - do not use byte mode\n", F_NAME);
                return(0);
	      } 
              D[j] = d + 1; 
              LifoPush(LIFO2, j); 
            }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 80: 
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points objet */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = MARK; 
          for (k = 0; k < 8; k += 1)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 8; k += 1)
          {
            j = voisin(i, k, rs, N);
            if ((j != -1) && (D[j] == MARK))
            {
              if (d == MARK-1)
	      {
                fprintf(stderr, "%s() : distance overflow - do not use byte mode\n", F_NAME);
                return(0);
	      } 
              D[j] = d + 1; 
              LifoPush(LIFO2, j); 
            }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 60:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points objet */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = MARK; 
          for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
          {
            j = voisin6(i, k, rs, ps, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
          {
            j = voisin6(i, k, rs, ps, N);
            if ((j != -1) && (D[j] == MARK))
            {
              if (d == MARK-1)
	      {
                fprintf(stderr, "%s() : distance overflow - do not use byte mode\n", F_NAME);
                return(0);
	      } 
              D[j] = d + 1; 
              LifoPush(LIFO2, j); 
            }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 180:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points objet */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = MARK; 
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            j = voisin18(i, k, rs, ps, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            j = voisin18(i, k, rs, ps, N);
            if ((j != -1) && (D[j] == MARK))
            {
              if (d == MARK-1)
	      {
                fprintf(stderr, "%s() : distance overflow - do not use byte mode\n", F_NAME);
                return(0);
	      } 
              D[j] = d + 1; 
              LifoPush(LIFO2, j); 
            }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    case 260:
      for (i = 0; i < N; i++) /* on met en pile les n-voisins des points objet */
      {
        if (F[i]) 
          D[i] = 0; 
        else 
        {
          D[i] = -1; 
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            j = voisin26(i, k, rs, ps, N);
            if ((j != -1) && (F[j])) { D[i] = 1; LifoPush(LIFO1, i); break; }
          }    
        }
      } /* for (i = 0; i < N; i++) */

      while (! LifoVide(LIFO1)) /* propagation en largeur */
      {
        while (! LifoVide(LIFO1))
        {
          i = LifoPop(LIFO1);
          d = D[i];
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            j = voisin26(i, k, rs, ps, N);
            if ((j != -1) && (D[j] == MARK))
            {
              if (d == MARK-1)
	      {
                fprintf(stderr, "%s() : distance overflow - do not use byte mode\n", F_NAME);
                return(0);
	      } 
              D[j] = d + 1; 
              LifoPush(LIFO2, j); 
            }
          }
        } /* while (! LifoVide(LIFO1)) */
        LIFOtmp = LIFO2;
        LIFO2 = LIFO1;
        LIFO1 = LIFOtmp;
      } /* while (! LifoVide(LIFO1)) */
      break;

    default: 
      fprintf(stderr, "%s(): bad mode: %d\n", F_NAME, mode);
      return 0;
  } /* switch (mode) */

  LifoTermine(LIFO1);
  LifoTermine(LIFO2);
  return(1);
} // ldistbyte()

/* ==================================== */
int32_t ldistquad(struct xvimage *img,   /* donnee: image binaire */       
              struct xvimage *res    /* resultat: distances */
)
/* ==================================== */
/* 
  Call the Danielsson 4SED algorithm (with mc-modification)
*/
#undef F_NAME
#define F_NAME "ldistquad"
{ 
  index_t rs = img->row_size;
  index_t cs = img->col_size;
  index_t N= rs * cs;        /* taille de l'image */
  uint8_t *F;                /* pointeur sur l'image */
  uint32_t *D;               /* pointeur sur les distances */
  vect2Dint *L;              /* tableau de vecteur associe a un point de l'image */
  index_t i;

  if (depth(img) != 1)
  {
    fprintf(stderr, "%s(): only for 2D images\n", F_NAME);
    return 0;
  }

  L = (vect2Dint *)calloc(1,N*sizeof(vect2Dint));
  F = UCHARDATA(img);

  ldistvect(F, L, rs, cs);

  D = ULONGDATA(res);
  for (i = 0; i < N; i++)
    D[i] = (int32_t)(L[i].x * L[i].x + L[i].y * L[i].y);

  free(L);
  return(1);
} // ldistquad()

int32_t ldistvect(uint8_t *F, vect2Dint *L, index_t rs, index_t cs) 
/* 
  Danielsson 4SED algorithm (with mc-modification)
  input:  - F is a pointer on the image
          - (rs, cs) is the image size
  output: - L is the vector image (should be allocated in the calling function)
*/
{
  uint32_t n1, n2;     /* normes des vecteurs (au carre) */
  index_t p, N= rs * cs;            /* taille de l'image */
  uint8_t *pt;
  vect2Dint v1, v2;
  int32_t i, j; // attention: index signés (parcours inverse, petite taille)

  pt = F;

  for (p = 0; p < N; p++, pt++)
	{
	/*  pt de l'image a 0, fond a INFINI */
	if (*pt)
	        {
		L[p].x = 0;
		L[p].y = 0;
		}
		else 
		{
		L[p].x = INFINI;
		L[p].y = INFINI;
		}

	/* cadre a INFINI */	
#ifdef CADRE
	if (bord(p,rs,N) != 0)
		{
		L[p].x = INFINI;
		L[p].y = INFINI;
		}
#endif
	}

  // 1er parcours de haut en bas
  for (j = 1; j < cs; j++)
  {
    for (i = 0; i < rs; i++)
    {/* parcours de la ligne de la gauche vers la droite */
      v1.x=L[i+((j-1)*rs)].x;
      v1.y=L[i+((j-1)*rs)].y-1; // Modif mc: Danielsson dit +1
      v2.x=L[i+(j*rs)].x;
      v2.y=L[i+(j*rs)].y;
      /* calcul des normes des vecteurs */
      n1=v1.x*v1.x+v1.y*v1.y;
      n2=v2.x*v2.x+v2.y*v2.y;
      /* on affecte au vecteur courant le vecteur ayant la plus petite norme */
      if (n1<n2) { L[i+(j*rs)].x=v1.x; L[i+(j*rs)].y=v1.y; }
    }
    for (i = 1; i < rs; i++)
    {/* parcours de la ligne de la gauche vers la droite */
      v1.x=L[i-1+(j*rs)].x-1; // Modif mc: Danielsson dit +1
      v1.y=L[i-1+(j*rs)].y;
      v2.x=L[i+(j*rs)].x;
      v2.y=L[i+(j*rs)].y;
      /* calcul des normes des vecteurs */
      n1=v1.x*v1.x+v1.y*v1.y;
      n2=v2.x*v2.x+v2.y*v2.y;
      /* on affecte au vecteur courant le vecteur ayant la plus petite norme */
      if (n1<n2) { L[i+(j*rs)].x=v1.x; L[i+(j*rs)].y=v1.y; }
    }
    for (i = rs-2; i >= 0; i--)
    {/* parcours de la ligne de la droite vers la gauche */
      v1.x=L[i+1+(j*rs)].x+1;
      v1.y=L[i+1+(j*rs)].y;
      v2.x=L[i+(j*rs)].x;
      v2.y=L[i+(j*rs)].y;
      /* calcul des normes des vecteurs */
      n1=v1.x*v1.x+v1.y*v1.y;
      n2=v2.x*v2.x+v2.y*v2.y;
      /* on affecte au vecteur courant le vecteur ayant la plus petite norme */
      if (n1<n2) { L[i+(j*rs)].x=v1.x; L[i+(j*rs)].y=v1.y; }
    }
  } // for (j = 1; j < cs; j++)

  // 2nd parcours de bas en haut
  for (j = cs-2; j >= 0; j--)
  {
    for (i = 0; i < rs; i++)
    {/* parcours de la ligne de la gauche vers la droite */
      v1.x=L[i+((j+1)*rs)].x;
      v1.y=L[i+((j+1)*rs)].y+1;
      v2.x=L[i+(j*rs)].x;
      v2.y=L[i+(j*rs)].y;
      /* calcul des normes des vecteurs */
      n1=v1.x*v1.x+v1.y*v1.y;
      n2=v2.x*v2.x+v2.y*v2.y;
      /* on affecte au vecteur courant le vecteur ayant la plus petite norme */
      if (n1<n2) { L[i+(j*rs)].x=v1.x; L[i+(j*rs)].y=v1.y; }
    }
    for (i = 1; i < rs; i++)
    {/* parcours de la ligne de la gauche vers la droite */
      v1.x=L[i-1+(j*rs)].x-1; // Modif mc: Danielsson dit +1
      v1.y=L[i-1+(j*rs)].y;
      v2.x=L[i+(j*rs)].x;
      v2.y=L[i+(j*rs)].y;
      /* calcul des normes des vecteurs */
      n1=v1.x*v1.x+v1.y*v1.y;
      n2=v2.x*v2.x+v2.y*v2.y;
      /* on affecte au vecteur courant le vecteur ayant la plus petite norme */
      if (n1<n2) { L[i+(j*rs)].x=v1.x; L[i+(j*rs)].y=v1.y; }
    }
    for (i = rs-2; i >= 0; i--)
    {/* parcours de la ligne de la droite vers la gauche */
      v1.x=L[i+1+(j*rs)].x+1;
      v1.y=L[i+1+(j*rs)].y;
      v2.x=L[i+(j*rs)].x;
      v2.y=L[i+(j*rs)].y;
      /* calcul des normes des vecteurs */
      n1=v1.x*v1.x+v1.y*v1.y;
      n2=v2.x*v2.x+v2.y*v2.y;
      /* on affecte au vecteur courant le vecteur ayant la plus petite norme */
      if (n1<n2) { L[i+(j*rs)].x=v1.x; L[i+(j*rs)].y=v1.y; }
    }
  } // for (j = cs-2; j >= 0; j--)
  return(1);
} //ldistvect

/* ======================================================== */
int32_t ldisteuc(struct xvimage* ob, struct xvimage* res)
/* ======================================================== */
#undef F_NAME
#define F_NAME "ldisteuc"
{
 index_t rs=rowsize(ob),cs=colsize(ob),N=rs*cs;
 uint32_t *R = ULONGDATA(res);
 index_t i;
 double d;

 if (!ldistquad(ob, res)) return 0;
 
 for (i=0; i<N; i++)
 {
   d = sqrt((double)(R[i]));
   R[i] = (uint32_t)arrondi(d);
 }
    
 return 1;
} // ldisteuc()

/* ======================================================== */
int32_t ldilatdisc(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// dilation by a disc of radius r
#undef F_NAME
#define F_NAME "ldilatdisc"
{
  index_t rs = rowsize(ob);
  index_t cs = colsize(ob);
  index_t ds = depth(ob); 
  index_t N = rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s(): allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  if (mode == 0) r2 = r * r; else r2 = r;
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 3: inverse(ob); if (!lsedt_meijster(ob, dist)) return 0; break;
    case 4:
    case 8: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }

  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MIN; else O[i] = NDG_MAX;

  freeimage(dist);    
  return 1;
} // ldilatdisc()

/* ======================================================== */
int32_t lerosdisc(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// erosion by a disc of radius r
#undef F_NAME
#define F_NAME "lerosdisc"
{
  index_t rs = rowsize(ob);
  index_t cs = colsize(ob);
  index_t ds = depth(ob); 
  index_t N = rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

  for(i=0; i<N; i++) if (O[i]) O[i] = NDG_MIN; else O[i] = NDG_MAX;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s(): allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);
 
  if (mode == 0) r2 = r * r; else r2 = r;
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 3: inverse(ob); if (!lsedt_meijster(ob, dist)) return 0; break;
    case 4:
    case 8: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }

  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  freeimage(dist);    
  return 1;
} // lerosdisc()

/* ======================================================== */
int32_t lopendisc(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// opening by a disc of radius r
#undef F_NAME
#define F_NAME "lopendisc"
{
  index_t rs = rowsize(ob);
  index_t cs = colsize(ob);
  index_t ds = depth(ob); 
  index_t N = rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s(): allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);
 
  if (mode == 0) r2 = r * r; else r2 = r;

  // inverse + dilate + inverse
  for(i=0; i<N; i++) if (O[i]) O[i] = NDG_MIN; else O[i] = NDG_MAX;
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 3: inverse(ob); if (!lsedt_meijster(ob, dist)) return 0; break;
    case 4:
    case 8: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  // dilate
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 3: inverse(ob); if (!lsedt_meijster(ob, dist)) return 0; break;
    case 4:
    case 8: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MIN; else O[i] = NDG_MAX;

  freeimage(dist);    
  return 1;
} // lopendisc()

/* ======================================================== */
int32_t lclosedisc(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// closing by a disc of radius r
#undef F_NAME
#define F_NAME "lclosedisc"
{
  index_t rs = rowsize(ob);
  index_t cs = colsize(ob);
  index_t ds = depth(ob); 
  index_t N = rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

  dist = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s(): allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);
 
  if (mode == 0) r2 = r * r; else r2 = r;

  // dilate + inverse
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 3: inverse(ob); if (!lsedt_meijster(ob, dist)) return 0; break;
    case 4:
    case 8: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  // dilate + inverse
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 3: inverse(ob); if (!lsedt_meijster(ob, dist)) return 0; break;
    case 4:
    case 8: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  freeimage(dist);    
  return 1;
} // lclosedisc()


#define CHAM_4 5
#define CHAM_8 7

#define CHAM_6  4
#define CHAM_18 5
#define CHAM_26 6

/* ==================================== */
int32_t lchamfrein(struct xvimage *img,   /* donnee: image binaire */       
               struct xvimage *res    /* resultat: distances (image deja allouee) */
)
/* ==================================== */
#undef F_NAME
#define F_NAME "lchamfrein"
{ 
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img);
  index_t ps = rs * cs;
  index_t N = ps * ds;    /* taille de l'image */
  uint8_t *F;             /* pointeur sur l'image */
  uint32_t *D;            /* pointeur sur les distances */
  index_t i;
  int32_t st, d;

  F = UCHARDATA(img);
  D = ULONGDATA(res);

  for (i = 0; i < N; i++)
    if (F[i]) D[i] = 0; else D[i] = INFINI;
    
  if (ds == 1)
  {
   st = 0;
   while (!st)
   {
    st = 1;
    /* parcours direct */
    for (i = 0; i < N; i++)
    if (!F[i])
    {
      d = D[i]; 
      if ((i%rs!=0)&&(D[i-1]+CHAM_4<d)) { st = 0; d = D[i-1]+CHAM_4; }
      if ((i>=rs)&&(i%rs!=0)&&(D[i-rs-1]+CHAM_8<d)) { st = 0; d = D[i-rs-1]+CHAM_8; }
      if ((i>=rs)&&(D[i-rs]+CHAM_4<d)) { st = 0; d = D[i-rs]+CHAM_4; }
      if ((i%rs!=rs-1)&&(i>=rs)&&(D[i+1-rs]+CHAM_8<d)) { st = 0; d = D[i+1-rs]+CHAM_8; }
      D[i] = d;
    }
    /* parcours retro */
    for (i = N-1; i >= 0; i--)
    if (!F[i])
    {
      d = D[i]; 
      if ((i%rs!=rs-1)&&(D[i+1]+CHAM_4<d)) { st = 0; d = D[i+1]+CHAM_4; }
      if ((i%rs!=0)&&(i<N-rs)&&(D[i-1+rs]+CHAM_8<d)) { st = 0; d = D[i-1+rs]+CHAM_8; }
      if ((i<N-rs)&&(D[i+rs]+CHAM_4<d)) { st = 0; d = D[i+rs]+CHAM_4; }
      if ((i<N-rs)&&(i%rs!=rs-1)&&(D[i+rs+1]+CHAM_8<d)) { st = 0; d = D[i+rs+1]+CHAM_8; }
      D[i] = d;
    }
   }
  }
  else /* image 3D */
  {
   st = 0;
   while (!st)
   {
    st = 1;
    /* parcours direct */
    for (i = 0; i < N; i++)
    if (!F[i])
    {
      d = D[i]; 
      /* plan "AVANT" (-ps) */
      if ((i>=ps)&&(i%rs!=rs-1)&&(D[-ps+i+1]+CHAM_18<d)) { st = 0; d = D[-ps+i+1]+CHAM_18; }
      if ((i>=ps)&&(i%rs!=rs-1)&&(i%ps>=rs)&&(D[-ps+i+1-rs]+CHAM_26<d)) { st = 0; d = D[-ps+i+1-rs]+CHAM_26; }
      if ((i>=ps)&&(i%ps>=rs)&&(D[-ps+i-rs]+CHAM_18<d)) { st = 0; d = D[-ps+i-rs]+CHAM_18; }
      if ((i>=ps)&&(i%ps>=rs)&&(i%rs!=0)&&(D[-ps+i-rs-1]+CHAM_26<d)) { st = 0; d = D[-ps+i-rs-1]+CHAM_26; }
      if ((i>=ps)&&(i%rs!=0)&&(D[-ps+i-1]+CHAM_18<d)) { st = 0; d = D[-ps+i-1]+CHAM_18; }
      if ((i>=ps)&&(i%rs!=0)&&(i%ps<ps-rs)&&(D[-ps+i-1+rs]+CHAM_26<d)) { st = 0; d = D[-ps+i-1+rs]+CHAM_26; }
      if ((i>=ps)&&(i%ps<ps-rs)&&(D[-ps+i+rs]+CHAM_18<d)) { st = 0; d = D[-ps+i+rs]+CHAM_18; }
      if ((i>=ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)&&(D[-ps+i+rs+1]+CHAM_26<d)) { st = 0; d = D[-ps+i+rs+1]+CHAM_26; }
      if ((i>=ps)&&(D[-ps+i]+CHAM_6<d)) { st = 0; d = D[-ps+i]+CHAM_6; }
      /* plan "COURANT" () */
      if ((i%rs!=rs-1)&&(i%ps>=rs)&&(D[i+1-rs]+CHAM_18<d)) { st = 0; d = D[i+1-rs]+CHAM_18; }
      if ((i%ps>=rs)&&(D[i-rs]+CHAM_6<d)) { st = 0; d = D[i-rs]+CHAM_6; }
      if ((i%ps>=rs)&&(i%rs!=0)&&(D[i-rs-1]+CHAM_18<d)) { st = 0; d = D[i-rs-1]+CHAM_18; }
      if ((i%rs!=0)&&(D[i-1]+CHAM_6<d)) { st = 0; d = D[i-1]+CHAM_6; }
      D[i] = d;
    }
    /* parcours retro */
    for (i = N-1; i >= 0; i--)
    if (!F[i])
    {
      d = D[i]; 
      /* plan "COURANT" () */
      if ((i%rs!=rs-1)&&(D[i+1]+CHAM_6<d)) { st = 0; d = D[i+1]+CHAM_6; }
      if ((i%rs!=0)&&(i%ps<ps-rs)&&(D[i-1+rs]+CHAM_18<d)) { st = 0; d = D[i-1+rs]+CHAM_18; }
      if ((i%ps<ps-rs)&&(D[i+rs]+CHAM_6<d)) { st = 0; d = D[i+rs]+CHAM_6; }
      if ((i%ps<ps-rs)&&(i%rs!=rs-1)&&(D[i+rs+1]+CHAM_18<d)) { st = 0; d = D[i+rs+1]+CHAM_18; }
      /* plan "ARRIERE" (+n) */
      if ((i<N-ps)&&(i%rs!=rs-1)&&(D[ps+i+1]+CHAM_18<d)) { st = 0; d = D[ps+i+1]+CHAM_18; }
      if ((i<N-ps)&&(i%rs!=rs-1)&&(i%ps>=rs)&&(D[ps+i+1-rs]+CHAM_26<d)) { st = 0; d = D[ps+i+1-rs]+CHAM_26; }
      if ((i<N-ps)&&(i%ps>=rs)&&(D[ps+i-rs]+CHAM_18<d)) { st = 0; d = D[ps+i-rs]+CHAM_18; }
      if ((i<N-ps)&&(i%ps>=rs)&&(i%rs!=0)&&(D[ps+i-rs-1]+CHAM_26<d)) { st = 0; d = D[ps+i-rs-1]+CHAM_26; }
      if ((i<N-ps)&&(i%rs!=0)&&(D[ps+i-1]+CHAM_18<d)) { st = 0; d = D[ps+i-1]+CHAM_18; }
      if ((i<N-ps)&&(i%rs!=0)&&(i%ps<ps-rs)&&(D[ps+i-1+rs]+CHAM_26<d)) { st = 0; d = D[ps+i-1+rs]+CHAM_26; }
      if ((i<N-ps)&&(i%ps<ps-rs)&&(D[ps+i+rs]+CHAM_18<d)) { st = 0; d = D[ps+i+rs]+CHAM_18; }
      if ((i<N-ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)&&(D[ps+i+rs+1]+CHAM_26<d)) { st = 0; d = D[ps+i+rs+1]+CHAM_26; }
      if ((i<N-ps)&&(D[ps+i]+CHAM_6<d)) { st = 0; d = D[ps+i]+CHAM_6; }
      D[i] = d;
    }
   } /* while (!st) */
  }
  return(1);
} // lchamfrein()

/****************************************/
/* distance euclidienne quadratique     */
/* code: Xiangh                         */
/*************************              */
/* date: 16/05/2002                     */
/*************************              */
/* comments:                            */
/*  attention, risques de pbs           */
/*  (manque de memoire)                 */
/*  dans cette version?                 */
/*  [risque faible; mais eviter les     */
/*  images d'arrete superieur a 256     */
/*  (en fait tout depend du centrage... */
/*  il serait bon néanmoins de faire    */
/*  varier DC_SZ selon la distance      */
/*  (pour une valeur elevee de distance */
/*  les Dcols devraient etre            */
/*  de plus en plus vides)]             */
/****************************************/

/********************************************************************/
/*                               MACROS                             */
/********************************************************************/

#define _Drun( cl, et, com)   { \
   Dcol* __col=cl; int32_t __it;Dcell* et; \
   while(__col!=NULL) \
   { for(__it=0;__it<__col->nb;__it++) \
   { et=(__col->tab)+__it; \
   com ; } __col=__col->next; } }
#define DC_SZ 1<<12
#define MAXD 1<<20
#define IsSet2(i,x,y,z) UCHARDATA(i)[((z)*(colsize(i))+(y))*(rowsize(i))+(x)]

/********************************************************************/
/*                               TYPES                              */
/********************************************************************/

typedef struct _Doc{
  int16_t x;
  int16_t y;
  int16_t z;
  int16_t dx;
  int16_t dy;
  int16_t dz;
} Dcell; //sert pour les cellules libres ET occupees!

typedef struct _Dcol{
  int32_t nb;//nombre de cellules occupees
  Dcell tab[DC_SZ];
  struct _Dcol* next;
} Dcol;

typedef struct _Dbase{
  int32_t max;
  Dcol** cols;
  Dcol* add;
  Dcol* cur;
} Drow; 


/********************************************************************/
/*                      ALLOUER/DESALLOUER                          */
/********************************************************************/

Dcol* Dsetcol()
{
  Dcol* nw=(Dcol*)calloc(1,sizeof(Dcol));
  nw->nb=0; 
  nw->next=NULL;
  return nw;
}

Dcol* Dfreecol(Dcol* c)
{
  if(c!=NULL)
    {
      Dfreecol(c->next);
      free(c);
    }
  return NULL;
}

Dcol* Dresetcol(Dcol* tg)
{
  if(tg!=NULL)
  {
    tg->nb=0; 
    if(tg->next!=NULL) tg->next=Dresetcol(tg->next);
  }
  else tg=Dsetcol();
  return tg;
}

/* alloue la table des buckets*/
Drow* Dsetground(int32_t _max)
{
  int32_t i,sz=_max+1;
  Drow* nw=(Drow*)calloc(1,sizeof(Drow));
  nw->max=_max;
  nw->cols=(Dcol**)calloc(1,sz*sizeof(Dcol*));
  for(i=0;i<sz;i++)
    nw->cols[i]=NULL;
  nw->cur=NULL;
  nw->add=Dsetcol();
  return nw;
}


/********************************************************************/
/*                     GESTION DES CELLULES                         */
/********************************************************************/

/*n'est en general pas appele directement*/
Dcol* Dadd2col(Dcol* c, int32_t x, int32_t y, int32_t z,int32_t dx, int32_t dy, int32_t dz )
{
  int32_t free=c->nb;
  if(free<DC_SZ)
    {
      c->tab[free].x=(int16_t)x;
      c->tab[free].y=(int16_t)y;
      c->tab[free].z=(int16_t)z;
      c->tab[free].dx=(int16_t)dx;
      c->tab[free].dy=(int16_t)dy;
      c->tab[free].dz=(int16_t)dz;
      c->nb++;
    }
  else
    {
      if(c->next==NULL) {c->next=Dsetcol();}
      Dadd2col(c->next,x,y,z,dx,dy,dz);
    }
  return c;
}

Dcol* Dadd(Drow* r,int32_t x, int32_t y, int32_t z, int32_t dx, int32_t dy, int32_t dz, int32_t val)
{
  if(val>r->max) return NULL;
  if(r->cols[val]==NULL) r->cols[val]=Dsetcol();
  return Dadd2col(r->cols[val],x,y,z,dx,dy,dz);
}

int32_t Dcount(Drow* r,int32_t val)
{
  int32_t nb=0;
  Dcol* cur;
  if((val>r->max)||(r->cols[val]==NULL)) return 0;
  cur=r->cols[val];
  do{
    nb+=cur->nb;
    cur=cur->next;
  }while(cur!=NULL);
  return nb;
}

int32_t Dhas(Drow* r,int32_t val)
{
  if((val>r->max)||(r->cols[val]==NULL)) return 0;
  return (r->cols[val]->nb)>0;
}

/********************************************************************/
/*                    CODE POUR LES DISTANCES                       */
/********************************************************************/

void testAndAdd(Drow* r, struct xvimage* o,Dcell* t, int32_t d)
{
  int32_t nd;
  int32_t rs=rowsize(o),cs=colsize(o),ds=depth(o);
  uint32_t* O=ULONGDATA(o);
  if(t->dx>0 && t->x<rs-1)
    {
      nd=d+2*t->dx+1;
      if(nd<O[(t->z*cs+t->y)*rs+t->x+1]) 
	{ 
	  O[(t->z*cs+t->y)*rs+t->x+1]=nd;
	  Dadd(r,t->x+1,t->y,t->z,t->dx+1,t->dy,t->dz,nd);
	}
    }
  else if(t->dx<0 && t->x>0)
    {
      nd=d-2*t->dx+1;
      if(nd<O[(t->z*cs+t->y)*rs+t->x-1]) 
	{ 
	  O[(t->z*cs+t->y)*rs+t->x-1]=nd;
	  Dadd(r,t->x-1,t->y,t->z,t->dx-1,t->dy,t->dz,nd);
	}
    }
  if(t->dy>0 && t->y<cs-1)
    {
      nd=d+2*t->dy+1;
      if(nd<O[(t->z*cs+(t->y+1))*rs+t->x]) 
	{ 
	  O[(t->z*cs+(t->y+1))*rs+t->x]=nd;
	  Dadd(r,t->x,t->y+1,t->z,t->dx,t->dy+1,t->dz,nd);
	}
    }
  else if(t->dy<0 && t->y>0)
    {
      nd=d-2*t->dy+1;
      if(nd<O[(t->z*cs+(t->y-1))*rs+t->x]) 
	{ 
	  O[(t->z*cs+(t->y-1))*rs+t->x]=nd;
	  Dadd(r,t->x,t->y-1,t->z,t->dx,t->dy-1,t->dz,nd);
	}
    }
  if(t->dz>0 && t->z<ds-1)
    {
      nd=d+2*t->dz+1;
      if(nd<O[((t->z+1)*cs+t->y)*rs+t->x]) 
	{ 
	  O[((t->z+1)*cs+t->y)*rs+t->x]=nd;
	  Dadd(r,t->x,t->y,t->z+1,t->dx,t->dy,t->dz+1,nd);
	}
    }
  else if(t->dz<0 && t->z>0)
    {
      nd=d-2*t->dz+1;
      if(nd<O[((t->z-1)*cs+t->y)*rs+t->x]) 
	{ 
	  O[((t->z-1)*cs+t->y)*rs+t->x]=nd;
	  Dadd(r,t->x,t->y,t->z-1,t->dx,t->dy,t->dz-1,nd);
	}
    }
}

/* ======================================================== */
int32_t ldistquad3d(struct xvimage* ob, struct xvimage* res)
/* ======================================================== */
{
 index_t rs=rowsize(ob),cs=colsize(ob),ds=depth(ob);
 index_t max=((rs-1)+(cs-1)+(ds-1))*((rs-1)+(cs-1)+(ds-1));
 uint32_t *O = ULONGDATA(res);
 index_t xi,yi,zi,xj,yj,zj,i,j,d;
 Drow* r=Dsetground(max);
 
 for(i=0;i<rs*cs*ds;i++) O[i]=MAXD;
  /*now process*/
  /*step one: image scan to locate object points and set borders*/
  for(zi=0;zi<ds;zi++)
    for(yi=0;yi<cs;yi++)
      for(xi=0;xi<rs;xi++)
	{
	  if(IsSet2(ob,xi,yi,zi))
	    {
	      O[(zi*cs+yi)*rs+xi]=0;
	       for(zj=((zi>0)?zi-1:zi);zj<=((zi<ds-1)?zi+1:zi);zj++)
		for(yj=((yi>0)?yi-1:yi);yj<=((yi<cs-1)?yi+1:yi);yj++)
		  for(xj=((xi>0)?xi-1:xi);xj<=((xi<rs-1)?xi+1:xi);xj++)
		    if(!IsSet2(ob,xj,yj,zj))
		      {
			j=(zj-zi)*(zj-zi)+(yj-yi)*(yj-yi)+(xj-xi)*(xj-xi);
			if(O[(zj*cs+yj)*rs+xj]>j) 
			  {
			    Dadd(r,xj,yj,zj,xj-xi,yj-yi,zj-zi,j);
			    O[(zj*cs+yj)*rs+xj]=j;
			  }
		      }
	    }
	    
	}

  /*step two: propagation throught the image*/
  d=0;
  while((++d)<=max)
    {
      _Drun(r->cols[d],ti,testAndAdd(r,res,ti,d));
      Dfreecol(r->cols[d]);
    }
    
 return 1;
} // ldistquad3d_xd()

/*

Copyright (c) 2002, Roberto de Alencar Lotufo, and
Sate University fo Campinas, Brazil.  All rights reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Universidade Estadual
de Campinas not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

THE STATE UNIVERSITY OF CAMPINAS DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL THE STATE UNIVERSITY OF CAMPINAS BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

     -------------------------------------------------
     FUNCTION: ldistquad3d_rl - Euclidean Distance transform
     -------------------------------------------------

 Purpose: giving a binary image determine its
   Euclidean distance transform. Works for 1D, 2D,
   and 3D cases
 In: input binary image: type UBYTE values 0 and 255
 Return: distance transform USHORT image

This is exact 3-D Euclidean Distance Transform implementation
based on the paper by
Lotufo, R. A., and Zampirolli, F. A.
"Fast Multidimensional Parallel Euclidean Distance Transform Based
on Mathematical Morphology.". In: XIV Brazilian Symposium on Computer
Graphics and Image Processing, 2001, Brazil. IEEE Computer Society,
2001, p.100-105.

pf    : pointer to input  raster image, unsigned byte, uint8
pg    : pointer to output raster image, integer, int32
width : n. of pixels in width
height: n. of pixels in height
depth : n. of pixels in depth
*/

/* ======================================================== */
int32_t ldistquad3d_rl(struct xvimage* imgin, struct xvimage* imgout)
/* ======================================================== */
#undef F_NAME
#define F_NAME "ldistquad3d"
{
  int32_t w,h,d, i, j, k, b, y, m, wh;  // attention: index signés (parcours inverse, petite taille)
  uint32_t inf=(0x7fffffff - 2048);
  uint8_t *pf,*pfj;
  uint32_t *pg,*pgj, *pgi, *paux;

  uint32_t *plist1_r, *plist1_l, *plist2_r, *plist2_l;
  uint32_t ilist1_r, ilist1_rmax, ilist1_l, ilist1_lmax;
  uint32_t ilist2_r, ilist2_l;

  pf=UCHARDATA(imgin);
  w=rowsize(imgin); h=colsize(imgin); d=depth(imgin);
  wh = w * h;

  pg = ULONGDATA(imgout);
  for(m=0;m<w*h*d;m++) pg[m] = inf;

  /* alocacao das filas para a segunda fase do algoritmo */
  plist1_r = (uint32_t *) calloc(1,w*sizeof(uint32_t));
  plist2_r = (uint32_t *) calloc(1,w*sizeof(uint32_t));
  plist1_l = (uint32_t *) calloc(1,w*sizeof(uint32_t));
  plist2_l = (uint32_t *) calloc(1,w*sizeof(uint32_t));
  if ((plist1_r==NULL)||(plist2_r==NULL)||(plist1_l==NULL)||(plist2_l==NULL))
  {
    fprintf(stderr, "%s(): cannot allocate buffer\n", F_NAME);
    return 0;
  }
  for(m=0;m<d;m++) { /* for each slice */
    for(j=0;j<w;j++) { /* para cada coluna j */
      pfj=&pf[j + m*wh];
      pgj=&pg[j + m*wh];
      i=0;
      while (pfj[i*w] == 0) {
	i++;
	if (i>=h) break;
      }
      if (i<=(h-1)) { /* se chegou pela primeira vez */
	k=i; pgj[k*w]=0;
	b=1; k--;
	while (k >= 0) { /* volta subindo ate inicio da linha */
	  pgj[k*w] = pgj[(k+1)*w] + b;
	  b=b+2;k--;
	}
	i++;
	while (i<=(h-1)) { /* Loop principal deste primeiro passo */
	  while (pfj[i*w] != 0) {
	    pgj[i*w]=0; i++;
	    if (i >= h) break;
	  }
	  if (i<=(h-1)) {
	    b=1;
	    while (pfj[i*w] == 0) {
	      pgj[i*w] = pgj[(i-1)*w] + b;
	      b = b + 2; i++;
	      if (i >= h) break;
	    }
	    if (i<=(h-1)) { 
	      k=i; pgj[k*w]=0; b=1;
	      while (pgj[(k-1)*w] > pgj[k*w]) { /* volta subindo */
		pgj[(k-1)*w] = pgj[k*w] + b;
		b = b + 2; k--;
	      }
	      i++;
	    }
	  }
	}
      }
    }
  }
  /* segunda parte, erosao por propagacao, linha por linha */

  for (m=0; m<d; m++) { /* for each slice */
    for (i=0; i<h; i++) { /* for each column */
      pgi = &pg[i*w + m*wh];
      for (j=0; j<(w-1); j++) {
	plist1_r[j] = w-j-2; /* propagacao right, varredura a esquerda */
	plist1_l[j] = j+1;   /* propagacao left, varredura a direita */
      }
      ilist1_r    = ilist1_l    = 0;
      ilist1_rmax = ilist1_lmax = w - 1; /* n. de pontos na fronteira */
      b = -1;
      while (ilist1_lmax + ilist1_rmax) { /* se alguma das listas for nao vazia */
	b = b + 2;
	ilist2_r = ilist2_l = 0;
	while (ilist1_r < ilist1_rmax) { /* propagacao right */
	  j = plist1_r[ ilist1_r]; ilist1_r++; /* tira da fila */
	  y = j + 1;
	  if (pgi[y] > pgi[j] + b) {
	    pgi[y] = pgi[j] + b;
	    if (y < (w-1)) {
	      plist2_r[ilist2_r] = y; ilist2_r++; /* poe na fila */
	    }
	  }
	}
	while (ilist1_l < ilist1_lmax) { /* propagacao left */
	  j = plist1_l[ ilist1_l]; ilist1_l++; /* tira da fila */
	  y = j - 1;
	  if (pgi[y] > pgi[j] + b) {
	    pgi[y] = pgi[j] + b;
	    if (y > 0) {
	      plist2_l[ilist2_l] = y; ilist2_l++; /* poe na fila */
	    }
	  }
	}    
	paux = plist1_r; plist1_r = plist2_r; plist2_r = paux;
	paux = plist1_l; plist1_l = plist2_l; plist2_l = paux;
	ilist1_r = ilist1_l = 0;
	ilist1_rmax = ilist2_r;
	ilist1_lmax = ilist2_l;
      }
    }
  }
  /* terceira parte, erosao por propagacao, depth by depth */

  if (d > 1) { /* only if image has more than one slice */
    for (j=0; j<w; j++) { /* for each line */
      for (i=0; i<h; i++) { /* for each column */
	pgi = &pg[j + i*w];
	for (m=0; m<(d-1); m++) {
	  plist1_r[m] = d-m-2; /* propagacao right, varredura a esquerda */
	  plist1_l[m] = m+1;   /* propagacao left, varredura a direita */
	}
	ilist1_r    = ilist1_l    = 0;
	ilist1_rmax = ilist1_lmax = d - 1; /* n. de pontos na fronteira */
	b = -1;
	while (ilist1_lmax + ilist1_rmax) { /* se alguma das listas for nao vazia */
	  b = b + 2;
	  ilist2_r = ilist2_l = 0;
	  while (ilist1_r < ilist1_rmax) { /* propagacao right */
	    m = plist1_r[ ilist1_r]; ilist1_r++; /* tira da fila */
	    y = m + 1;
	    if (pgi[y*wh] > pgi[m*wh] + b) {
	      pgi[y*wh] = pgi[m*wh] + b;
	      if (y < (d-1)) {
		plist2_r[ilist2_r] = y; ilist2_r++; /* poe na fila */
	      }
	    }
	  }
	  while (ilist1_l < ilist1_lmax) { /* propagacao left */
	    m = plist1_l[ ilist1_l]; ilist1_l++; /* tira da fila */
	    y = m - 1;
	    if (pgi[y*wh] > pgi[m*wh] + b) {
	      pgi[y*wh] = pgi[m*wh] + b;
	      if (y > 0) {
		plist2_l[ilist2_l] = y; ilist2_l++; /* poe na fila */
	      }
	    }
	  }    
	  paux = plist1_r; plist1_r = plist2_r; plist2_r = paux;
	  paux = plist1_l; plist1_l = plist2_l; plist2_l = paux;
	  ilist1_r = ilist1_l = 0;
	  ilist1_rmax = ilist2_r;
	  ilist1_lmax = ilist2_l;
	}
      }
    }
  }
  free(plist1_r);
  free(plist1_l);
  free(plist2_r);
  free(plist2_l);
  return 1;
} // ldistquad3d_rl

/* ======================================================== */
int32_t ldisteuc3d(struct xvimage* ob, struct xvimage* res)
/* ======================================================== */
{
 index_t rs=rowsize(ob),cs=colsize(ob),ds=depth(ob);
 uint32_t *O = ULONGDATA(res);
 index_t i;
 double d;

 if (!ldistquad3d(ob, res)) return 0;
 
 for(i=0;i<rs*cs*ds;i++)
 {
   d = sqrt((double)(O[i]));
   O[i] = (uint32_t)arrondi(d);
 }
    
 return 1;
} // ldisteuc3d()

/* ======================================================== */
int32_t ldilatball(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// dilation by a ball of radius r
#undef F_NAME
#define F_NAME "ldilatball"
{
  index_t rs=rowsize(ob), cs=colsize(ob), ds=depth(ob), N=rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);
 
  if (mode == 0) r2 = r * r; else r2 = r;
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad3d(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 4:
    case 6:
    case 8: 
    case 18: 
    case 26: 
      if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }

  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MIN; else O[i] = NDG_MAX;

  freeimage(dist);    
  return 1;
} // ldilatball()

/* ======================================================== */
int32_t lerosball(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// erosion by a ball of radius r
#undef F_NAME
#define F_NAME "lerosball"
{
  index_t rs=rowsize(ob), cs=colsize(ob), ds=depth(ob), N=rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

#ifdef VERBOSE
  printf("%s: mode = %d, r = %d\n", F_NAME, mode, r);
#endif

  for(i=0; i<N; i++) if (O[i]) O[i] = NDG_MIN; else O[i] = NDG_MAX;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  if (mode == 0) r2 = r * r; else r2 = r;
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad3d(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 4:
    case 8:
    case 6:
    case 18:
    case 26: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }

  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  freeimage(dist);    
  return 1;
} // lerosball()

/* ======================================================== */
int32_t lopenball(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// opening by a ball of radius r
#undef F_NAME
#define F_NAME "lopenball"
{
  index_t rs=rowsize(ob), cs=colsize(ob), ds=depth(ob), N=rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;


  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  if (mode == 0) r2 = r * r; else r2 = r;

  // inverse + dilate + inverse
  for(i=0; i<N; i++) if (O[i]) O[i] = NDG_MIN; else O[i] = NDG_MAX;
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad3d(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 4:
    case 8:
    case 6:
    case 18:
    case 26: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  //dilate
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad3d(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 4:
    case 8:
    case 6:
    case 18:
    case 26: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MIN; else O[i] = NDG_MAX;

  freeimage(dist);    
  return 1;
} // lopenball()

/* ======================================================== */
int32_t lcloseball(struct xvimage* ob, int32_t r, int32_t mode)
/* ======================================================== */
// closing by a ball of radius r
#undef F_NAME
#define F_NAME "lcloseball"
{
  index_t rs=rowsize(ob), cs=colsize(ob), ds=depth(ob), N=rs*cs*ds;
  struct xvimage *dist;
  uint32_t *D;
  uint8_t *O = UCHARDATA(ob);
  index_t i;
  int32_t r2;

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  D = ULONGDATA(dist);

  if (mode == 0) r2 = r * r; else r2 = r;

  // dilate + inverse
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad3d(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 4:
    case 8:
    case 6:
    case 18:
    case 26: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  // dilate + inverse
  switch (mode)
  {
    case 0:
    case 1: if (!ldistquad3d(ob, dist)) return 0; break;
    case 2: if (!lchamfrein(ob, dist)) return 0; break;
    case 4:
    case 8:
    case 6:
    case 18:
    case 26: if (!ldist(ob, mode, dist)) return 0; break;
    default: 
      fprintf(stderr, "%s: bad mode: %d\n", F_NAME, mode);
      return 0;
  }
  for(i=0; i<N; i++) if (D[i] > r2) O[i] = NDG_MAX; else O[i] = NDG_MIN;

  freeimage(dist);    
  return 1;
} // lcloseball()

/* ======================================================== */
struct xvimage* ldilatdiscloc(struct xvimage* f, int32_t mode)
/* ======================================================== */
// local dilation by a disc of radius f(x)
#undef F_NAME
#define F_NAME "ldilatdiscloc"
{
  struct xvimage *tmp1;
  struct xvimage *res;
  uint8_t *T1;
  uint8_t *R;
  index_t rs = rowsize(f), cs = colsize(f), ds = depth(f), N = rs * cs * ds, i;
  int32_t vmax, v;
  int32_t go;
  tmp1 = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  res = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if ((tmp1 == NULL) || (res == NULL))
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(NULL);
  }
  T1 = UCHARDATA(tmp1);
  R = UCHARDATA(res);
  memset(R, 0, N);
  if (datatype(f) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(f);
    vmax = F[0];
    for (i = 0; i < N; i++) if (F[i] > vmax) vmax = F[i];
    for (v = 1; v <= vmax; v++)
    {
      memset(T1, 0, N);
      go = 0;
      for (i = 0; i < N; i++) if (F[i] == v) { go = 1; T1[i] = NDG_MAX; }
      if (go) 
      { 
        ldilatdisc(tmp1, v-1, mode);
        for (i = 0; i < N; i++) if (T1[i]) R[i] = NDG_MAX;
      }
    } // for (v = 1; v <= vmax; v++)
  }
  else if (datatype(f) == VFF_TYP_4_BYTE)
  {
    uint32_t *F = ULONGDATA(f);
    vmax = F[0];
    for (i = 0; i < N; i++) if (F[i] > vmax) vmax = F[i];
    for (v = 1; v <= vmax; v++)
    {
      memset(T1, 0, N);
      go = 0;
      for (i = 0; i < N; i++) if (F[i] == v) { go = 1; T1[i] = NDG_MAX; }
      if (go) 
      { 
        ldilatdisc(tmp1, v-1, mode);
        for (i = 0; i < N; i++) if (T1[i]) R[i] = NDG_MAX;
      }
    } // for (v = 1; v <= vmax; v++)
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return(NULL);
  }

  freeimage(tmp1);
  return(res);
} // ldilatdiscloc()

/* ======================================================== */
struct xvimage* ldilatballloc(struct xvimage* f, int32_t mode)
/* ======================================================== */
// local dilation by a ball of radius f(x)
// OBSOLETE - see REDT (lredt2d)
#undef F_NAME
#define F_NAME "ldilatballloc"
{
  struct xvimage *tmp1;
  struct xvimage *res;
  uint8_t *T1;
  uint8_t *R;
  index_t rs = rowsize(f), cs = colsize(f), ds = depth(f), N = rs * cs * ds, i;
  int32_t vmax, v;
  int32_t go;
  tmp1 = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  res = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if ((tmp1 == NULL) || (res == NULL))
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(NULL);
  }
  T1 = UCHARDATA(tmp1);
  R = UCHARDATA(res);
  memset(R, 0, N);
  if (datatype(f) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(f);
    vmax = F[0];
    for (i = 0; i < N; i++) if (F[i] > vmax) vmax = F[i];
    for (v = 1; v <= vmax; v++)
    {
      memset(T1, 0, N);
      go = 0;
      for (i = 0; i < N; i++) if (F[i] == v) { go = 1; T1[i] = NDG_MAX; }
      if (go) 
      { 
        ldilatball(tmp1, v-1, mode);
        for (i = 0; i < N; i++) if (T1[i]) R[i] = NDG_MAX;
      }
    } // for (v = 1; v <= vmax; v++)
  }
  else if (datatype(f) == VFF_TYP_4_BYTE)
  {
    uint32_t *F = ULONGDATA(f);
    vmax = F[0];
    for (i = 0; i < N; i++) if (F[i] > vmax) vmax = F[i];
    for (v = 1; v <= vmax; v++)
    {
      memset(T1, 0, N);
      go = 0;
      for (i = 0; i < N; i++) if (F[i] == v) { go = 1; T1[i] = NDG_MAX; }
      if (go) 
      { 
        ldilatball(tmp1, v-1, mode);
        for (i = 0; i < N; i++) if (T1[i]) R[i] = NDG_MAX;
      }
    } // for (v = 1; v <= vmax; v++)
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return(NULL);
  }

  freeimage(tmp1);
  return(res);
} // ldilatballloc()

/* ==================================== */
int32_t ldistquadSaito(struct xvimage *img,   /* donnee: image binaire */       
              struct xvimage *res    /* resultat: distances */
)
/* ==================================== */
/* 
  Call the Saito-Toriwaki algorithm

  ATTENTION: calcule la distance au complémentaire
*/
#undef F_NAME
#define F_NAME "ldistquadSaito"
{ 
  index_t i,j,df,db,n;
  index_t w,d,rMax,rStart,rEnd;  
  index_t rs = img->row_size;
  index_t cs = img->col_size;
  index_t N= rs * cs;            /* taille de l'image */
  uint8_t *F;                    /* pointeur sur l'image */
  uint32_t *D;                   /* pointeur sur les distances */
  int32_t * buff = (int32_t *)calloc(1,cs * sizeof(int32_t));

  if ((datatype(img) != VFF_TYP_1_BYTE) || (datatype(res) != VFF_TYP_4_BYTE))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  D = ULONGDATA(res);
  F = UCHARDATA(img);  
  for (i=0;i<N;i++) D[i] = (uint32_t)F[i];

  ////Forward scan
  for (j=0;j<cs;j++)
  {
    df=rs;
    for (i=0;i<rs;i++)
    {
      if (D[j*rs+i]!=0)
        df=df+1;
      else 
        df=0;
      D[j*rs+i]=df*df;
    }
  }

  ////Backward scan
  for (j=0;j<cs;j++)
  {
    db=rs;
    for (i=rs-1;i>=0;i--)
    {
      if (D[j*rs+i]!=0)
	db=db+1;
      else 
	db=0;
      D[j*rs+i]=mcmin(D[j*rs+i],db*db);

    }
  }

  //%%%% step 2
  for (i=0;i<rs;i++)
  {
	for (j=0;j<cs;j++)
	{
	buff[j]=D[i+j*rs];
	}
	
	for (j=0;j<cs;j++)
	{
		d=buff[j];
		if(d!=0)
		{
			rMax=(int32_t)(sqrt(d))+1;
			rStart=mcmin(rMax,j);
			rEnd=mcmin(rMax,(cs-1-j));
			
		for (n=-rStart;n<=rEnd;n++)
		{
		w=buff[j+n]+n*n;
		if (w<d)
		d=w;
		}
		}

		D[i+j*rs]=d;
	
	}
  }
  free(buff);
  return(1);
} // ldistquadSaito()

/* ==================================== */
int32_t ldistSaito(struct xvimage *img,   /* donnee: image binaire */       
              struct xvimage *res    /* resultat: distances */
)
/* ==================================== */
/* 
  Call the Saito-Toriwaki algorithm

  ATTENTION: calcule la distance au complémentaire
*/
#undef F_NAME
#define F_NAME "ldistSaito"
{ 
  index_t i,j,df,db,n;
  index_t w,d,rMax,rStart,rEnd;  
  index_t rs = img->row_size;
  index_t cs = img->col_size;
  index_t N= rs * cs;            /* taille de l'image */
  uint8_t *F;                    /* pointeur sur l'image */
  uint32_t *D;                   /* pointeur sur les distances */
  int32_t * buff = (int32_t *)calloc(1,cs * sizeof(int32_t));
  double * R = DOUBLEDATA(res);
  F = UCHARDATA(img);

  if ((datatype(img) != VFF_TYP_1_BYTE) || (datatype(res) != VFF_TYP_DOUBLE))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  D = (uint32_t *)calloc(1,N * sizeof(uint32_t));
  if (D == NULL)
  {
    fprintf(stderr, "%s: malloc failed\n", F_NAME);
    return(0);
  }

  for (i = 0; i < N; i++) D[i] = (uint32_t)F[i];

  ////Forward scan
  for (j=0;j<cs;j++)
  {
    df=rs;
    for (i=0;i<rs;i++)
    {
      if (D[j*rs+i]!=0)
        df=df+1;
      else 
        df=0;
      D[j*rs+i]=df*df;
    }
  }

  ////Backward scan
  for (j=0;j<cs;j++)
  {
    db=rs;
    for (i=rs-1;i>=0;i--)
    {
      if (D[j*rs+i]!=0)
	db=db+1;
      else 
	db=0;
      D[j*rs+i]=mcmin(D[j*rs+i],db*db);

    }
  }

  //%%%% step 2
  for (i=0;i<rs;i++)
  {
	for (j=0;j<cs;j++)
	{
	buff[j]=D[i+j*rs];
	}
	
	for (j=0;j<cs;j++)
	{
		d=buff[j];
		if(d!=0)
		{
			rMax=(int32_t)(sqrt(d))+1;
			rStart=mcmin(rMax,j);
			rEnd=mcmin(rMax,(cs-1-j));
			
		for (n=-rStart;n<=rEnd;n++)
		{
		w=buff[j+n]+n*n;
		if (w<d)
		d=w;
		}
		}

		D[i+j*rs]=d;
	
	}
  }

  for (i = 0; i < N; i++) R[i] = sqrt(D[i]);

  free(buff);
  free(D);
  return(1);
} // ldistSaito()

/* ======================================================== */
/* ======================================================== */
// Functions for the Reverse Euclidean Distance Transform (Coeurjolly)
// Linear algorithm
/* ======================================================== */
/* ======================================================== */

#define Sep1(u,v,f,j) (((u*u)-(v*v)-f[u+rs*j]+f[v+rs*j])/(2*(u-v)))
#define Sep2(u,v,f,i) (((u*u)-(v*v)-f[u*rs+i]+f[v*rs+i])/(2*(u-v)))

#define F1(x,xp,f,j) (f[x+rs*j]-(xp-x)*(xp-x))
#define F2(y,yp,f,i) (f[y*rs+i]-(yp-y)*(yp-y))

#define Sep1_3d(u,v,f,j,k) (((u*u)-(v*v)-f[u+rs*j+ps*k]+f[v+rs*j+ps*k])/(2*(u-v)))
#define Sep2_3d(u,v,f,i,k) (((u*u)-(v*v)-f[i+rs*u+ps*k]+f[i+rs*v+ps*k])/(2*(u-v)))
#define Sep3_3d(u,v,f,i,j) (((u*u)-(v*v)-f[i+rs*j+ps*u]+f[i+rs*j+ps*v])/(2*(u-v)))

#define F1_3d(x,xp,f,j,k) (f[x+rs*j+ps*k]-(xp-x)*(xp-x))
#define F2_3d(y,yp,f,i,k) (f[i+rs*y+ps*k]-(yp-y)*(yp-y))
#define F3_3d(z,zp,f,i,j) (f[i+rs*j+ps*z]-(zp-z)*(zp-z))

/* ======================================================== */
static void REDT_line(int32_t *f, int32_t *g, index_t rs, index_t cs)
/* ======================================================== */
{
  int32_t j, u, q, w; // attention: index signés (parcours inverse, petite taille)
  int32_t *s, *t; //sommets des paraboles
  s = (int32_t *)calloc(1,rs * sizeof(int32_t));
  t = (int32_t *)calloc(1,rs * sizeof(int32_t));

  for (j = 0; j < cs; j++)
  {
    q = 0; s[0] = 0; t[0] = 0;
    for (u = 1; u < rs; u++)
    {
      while ( (q >= 0) && (F1(s[q],t[q],f,j) < F1(u,t[q],f,j)) ) q--;
      if (q < 0)
      {
        q = 0;
        s[0] = u;
      }
      else
      {
        w = 1 + Sep1(s[q],u,f,j);
        if (w < rs)
        {
          q++; s[q] = u; t[q] = w;
        }
      }
    } 
    for (u = rs-1; u >= 0; u--)
    {
      g[u + rs*j] = F1(s[q],u,f,j);
      if (u == t[q]) q--;
    }
  }
  free(s); free(t);
} //  REDT_line()

/* ======================================================== */
static void REDT_column(int32_t *f, int32_t *g, index_t rs, index_t cs)
/* ======================================================== */
{
  int32_t i, u, q, w; // attention: index signés (parcours inverse, petite taille)
  int32_t *s, *t; //sommets des paraboles
  s = (int32_t *)calloc(1,cs * sizeof(int32_t));
  t = (int32_t *)calloc(1,cs * sizeof(int32_t));

  for (i = 0; i < rs; i++)
  {
    q = 0; s[0] = 0; t[0] = 0;
    for (u = 1; u < cs; u++)
    {
      while ( (q >= 0) && (F2(s[q],t[q],f,i) < F2(u,t[q],f,i)) ) q--;
      if (q < 0)
      {
        q = 0;
        s[0] = u;
      }
      else
      {
        w = 1 + Sep2(s[q],u,f,i);
        if (w < cs)
        {
          q++; s[q] = u; t[q] = w;
        }
      }
    } 
    for (u = cs-1; u >= 0; u--)
    {
      g[rs*u + i] = F2(s[q],u,f,i);
      if (u == t[q]) q--;
    }
  }
  free(s); free(t);
} //  REDT_column()

/* ======================================================== */
struct xvimage* lredt2d(struct xvimage* f)
/* ======================================================== */
// reverse euclidean distance transform
// from: "d-Dimensional Reverse Euclidean Distance Transformation and Euclidean Medial Axis 
//        Extraction in Optimal Time", D. Coeurjolly, DGCI 2003, LNCS 2886, pp. 327-337, 2003.
// Caution: original data in image f will be lost.
#undef F_NAME
#define F_NAME "lredt2d"
{
  struct xvimage *tmp;
  struct xvimage *res;
  uint8_t *R;
  index_t i, rs = rowsize(f), cs = colsize(f), ds = depth(f), N = rs * cs * ds;

  if (ds != 1)
  {
    fprintf(stderr, "%s: only for 2D images\n", F_NAME);
    return(NULL);
  }

  res = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  tmp = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if ((res == NULL) || (tmp == NULL))
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(NULL);
  }
  R = UCHARDATA(res);

  if (datatype(f) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(f);
    int32_t *T = SLONGDATA(tmp);
    REDT_line(F, T, rs, cs);
    copy2image(f, tmp);
    REDT_column(F, T, rs, cs);
    for (i = 0; i < N; i++) if (T[i]) R[i] = NDG_MAX; else R[i] = 0;
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return(NULL);
  }

  freeimage(tmp);
  return(res);
} // lredt2d()

/* ======================================================== */
static void REDT_line_3d(int32_t *f, int32_t *g, index_t rs, index_t cs, index_t ds)
/* ======================================================== */
{
  int32_t j, u, q, k, ps, w; // attention: index signés (parcours inverse, petite taille)
  int32_t *s, *t; //sommets des paraboles
  s = (int32_t *)calloc(1,rs * sizeof(int32_t));
  t = (int32_t *)calloc(1,rs * sizeof(int32_t));
  ps = cs * rs; // taille d'un plan
  
  for(k = 0; k < ds; k++)
    {
      for (j = 0; j < cs; j++)
	{
	  q = 0; s[0] = 0; t[0] = 0;
	  for (u = 1; u < rs; u++)
	    {
	      while ( (q >= 0) && (F1_3d(s[q],t[q],f,j,k) < F1_3d(u,t[q],f,j,k)) ) q--;
	      if (q < 0)
		{
		  q = 0;
		  s[0] = u;
		}
	      else
		{
		  w = 1 + Sep1_3d(s[q],u,f,j,k);
		  if (w < rs)
		    {
		      q++; s[q] = u; t[q] = w;
		    }
		}
	    } 
	  for (u = rs-1; u >= 0; u--)
	    {
	      g[u + rs*j + ps*k] = F1_3d(s[q],u,f,j,k);
	      if (u == t[q]) q--;
	    }
	}
    }
  free(s); free(t);
} //  REDT_line_3d()

/* ======================================================== */
static void REDT_column_3d(int32_t *f, int32_t *g, index_t rs, index_t cs, index_t ds)
/* ======================================================== */
{
  int32_t i, u, q, k, ps, w; // attention: index signés (parcours inverse, petite taille)
  int32_t *s, *t; //sommets des paraboles
  s = (int32_t *)calloc(1,cs * sizeof(int32_t));
  t = (int32_t *)calloc(1,cs * sizeof(int32_t));
  ps = rs * cs; // taille d'un plan

  for(k = 0; k < ds; k++)
    {
      for (i = 0; i < rs; i++)
	{
	  q = 0; s[0] = 0; t[0] = 0;
	  for (u = 1; u < cs; u++)
	    {
	      while ( (q >= 0) && (F2_3d(s[q],t[q],f,i,k) < F2_3d(u,t[q],f,i,k)) ) q--;
	      if (q < 0)
		{
		  q = 0;
		  s[0] = u;
		}
	      else
		{
		  w = 1 + Sep2_3d(s[q],u,f,i,k);
		  if (w < cs)
		    {
		      q++; s[q] = u; t[q] = w;
		    }
		}
	    } 
	  for (u = cs-1; u >= 0; u--)
	    {
	      g[k*ps + rs*u + i] = F2_3d(s[q],u,f,i,k);
	      if (u == t[q]) q--;
	    }
	}
    }
  free(s); free(t);
} //  REDT_column_3d()


/* ======================================================== */
static void REDT_zaxis_3d(int32_t *f, int32_t *g, index_t rs, index_t cs, index_t ds)
/* ======================================================== */
{
  int32_t i, u, q, j, ps, w; // attention: index signés (parcours inverse, petite taille)
  int32_t *s, *t; //sommets des paraboles
  s = (int32_t *)calloc(1,ds * sizeof(int32_t));
  t = (int32_t *)calloc(1,ds * sizeof(int32_t));
  ps = rs * cs; // taille d'un plan

  for(j = 0; j < cs; j++)
    {
      for (i = 0; i < rs; i++)
	{
	  q = 0; s[0] = 0; t[0] = 0;
	  for (u = 1; u < ds; u++)
	    {
	      while ( (q >= 0) && (F3_3d(s[q],t[q],f,i,j) < F3_3d(u,t[q],f,i,j)) ) q--;
	      if (q < 0)
		{
		  q = 0;
		  s[0] = u;
		}
	      else
		{
		  w = 1 + Sep3_3d(s[q],u,f,i,j);
		  if (w < ds)
		    {
		      q++; s[q] = u; t[q] = w;
		    }
		}
	    } 
	  for (u = ds-1; u >= 0; u--)
	    {
	      g[ps*u +  rs*j + i] = F3_3d(s[q],u,f,i,j);
	      if (u == t[q]) q--;
	    }
	}
    }
  free(s); free(t);
} //  REDT_zaxis_3d()

/* ======================================================== */
struct xvimage* lredt3d(struct xvimage* f)
/* ======================================================== */
// reverse euclidean distance transform
// from: "d-Dimensional Reverse Euclidean Distance Transformation and Euclidean Medial Axis 
//        Extraction in Optimal Time", D. Coeurjolly, DGCI 2003, LNCS 2886, pp. 327-337, 2003.
// Caution: original data in image f will be lost.
#undef F_NAME
#define F_NAME "lredt3d"
{
  struct xvimage *tmp;
  struct xvimage *res;
  uint8_t *R;
  index_t i, rs = rowsize(f), cs = colsize(f), ds = depth(f), N = rs * cs * ds;

  if (ds <= 1)
    {
      fprintf(stderr, "%s: only for 3D images\n", F_NAME);
      return(NULL);
    }

  res = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  tmp = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if ((res == NULL) || (tmp == NULL))
    {
      fprintf(stderr, "%s: allocimage failed\n", F_NAME);
      return(NULL);
    }
  R = UCHARDATA(res);
  
  if (datatype(f) == VFF_TYP_4_BYTE)
    {
      int32_t *F = SLONGDATA(f);
      int32_t *T = SLONGDATA(tmp);
      REDT_line_3d(F, T, rs, cs, ds);
      copy2image(f, tmp);
      REDT_column_3d(F, T, rs, cs, ds);
      copy2image(f, tmp);
      REDT_zaxis_3d(F, T, rs, cs, ds);
      for (i = 0; i < N; i++) if (T[i]) R[i] = NDG_MAX; else R[i] = 0;
    }
  else
    {
      fprintf(stderr, "%s: bad image type\n", F_NAME);
      return(NULL);
    }

  freeimage(tmp);
  return(res);
} // lredt3d()

/* ======================================================== */
static void ST_line(uint32_t *f, uint32_t *g, uint8_t *r, index_t rs, index_t cs)
/* ======================================================== */
{
  int32_t j, u, q, w; // attention: index signés (parcours inverse, petite taille)
  uint32_t *s, *t; //sommets des paraboles
  s = (uint32_t *)calloc(1,rs * sizeof(uint32_t));
  t = (uint32_t *)calloc(1,rs * sizeof(uint32_t));

  memset(r, 0, rs*cs);
  for (j = 0; j < cs; j++)
  {
    q = 0; s[0] = 0; t[0] = 0;
    for (u = 1; u < rs; u++)
    {
      while ( (q >= 0) && (F1(s[q],t[q],f,j) < F1(u,t[q],f,j)) ) q--;
      if (q < 0)
      {
        q = 0;
        s[0] = u;
      }
      else
      {
        w = 1 + Sep1(s[q],u,f,j);
        if (w < rs)
        {
          q++; s[q] = u; t[q] = w;
        }
      }
    } 
    for (u = rs-1; u >= 0; u--)
    {
      r[s[q] + rs*j] = 1;
      g[u + rs*j] = F1(s[q],u,f,j);
      if (u == t[q]) q--;
    }
  }
  free(s); free(t);
} //  ST_line()

/* ======================================================== */
static void ST_column(uint32_t *f, uint8_t *r, index_t rs, index_t cs)
/* ======================================================== */
// input f: result of line scan
// input/output r: binary image - positions of the skeleton points
{
  int32_t i, u, q, w; // attention: index signés (parcours inverse, petite taille)
  uint32_t *s, *t; //sommets des paraboles
  s = (uint32_t *)calloc(1,cs * sizeof(uint32_t));
  t = (uint32_t *)calloc(1,cs * sizeof(uint32_t));

  for (i = 0; i < rs; i++)
  {
    q = 0; s[0] = 0; t[0] = 0;
    for (u = 1; u < cs; u++)
    {
      while ( (q >= 0) && (F2(s[q],t[q],f,i) < F2(u,t[q],f,i)) ) q--;
      if (q < 0)
      {
        q = 0;
        s[0] = u;
      }
      else
      {
        w = 1 + Sep2(s[q],u,f,i);
        if (w < cs)
        {
          q++; s[q] = u; t[q] = w;
        }
      }
    } 
    for (u = cs-1; u >= 0; u--)
    {
      r[rs*s[q] + i] = 1;
      if (u == t[q]) q--;
    }
  }
  free(s); free(t);
} //  ST_column()

/* ======================================================== */
int32_t lskeleton_ST(struct xvimage* f, struct xvimage* res)
/* ======================================================== */
// optimal algorithm for Saito-Toriwaki's skeleton
// from: "d-Dimensional Reverse Euclidean Distance Transformation and Euclidean Medial Axis 
//        Extraction in Optimal Time", D. Coeurjolly, DGCI 2003, LNCS 2886, pp. 327-337, 2003.
// Caution: original data in image f will be lost.
#undef F_NAME
#define F_NAME "lskeleton_ST"
{
  struct xvimage *tmp;
  index_t i, rs = rowsize(f), cs = colsize(f), ds = depth(f), N = rs * cs * ds;
  uint32_t *F = ULONGDATA(f);
  uint32_t *R = ULONGDATA(res);
  uint8_t *T;

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }    

  if ((datatype(f) != VFF_TYP_4_BYTE) || (datatype(res) != VFF_TYP_4_BYTE))
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

  if (ds != 1)
  {
    fprintf(stderr, "%s: only for 2D images\n", F_NAME);
    return 0;
  }

  tmp = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  if (tmp == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  T = UCHARDATA(tmp);

  ST_line(F, R, T, rs, cs);
  ST_column(R, T, rs, cs);
  for (i = 0; i < N; i++) if (T[i]) R[i] = F[i]; else R[i] = 0;

  freeimage(tmp);
  return 1;
} // lskeleton_ST()

/* ======================================================== */
/* ======================================================== */
// Functions for the exact Squared Euclidean Distance Transform (Meijster et al.)
// Linear algorithm
/* ======================================================== */
/* ======================================================== */

#define F_2_2d(y,yp,f,i) (f[y*rs+i]+(yp-y)*(yp-y))
#define Sep_2_2d(v,u,f,i) (((u*u)-(v*v)+f[u*rs+i]-f[v*rs+i])/(2*(u-v)))

#define F_2_3d(y,yp,f,i,k) (f[k*ps+y*rs+i]+(yp-y)*(yp-y))
#define F_3_3d(z,zp,f,i,j) (f[z*ps+j*rs+i]+(zp-z)*(zp-z))
#define Sep_2_3d(v,u,f,i,k) (((u*u)-(v*v)+f[k*ps+u*rs+i]-f[k*ps+v*rs+i])/(2*(u-v)))
#define Sep_3_3d(v,u,f,i,j) (((u*u)-(v*v)+f[u*ps+j*rs+i]-f[v*ps+j*rs+i])/(2*(u-v)))

/* ======================================================== */
void SEDT_line(uint8_t *f, uint32_t *g, index_t rs, index_t cs)
/* ======================================================== */
{
  int32_t i, j; // attention: index signés (parcours inverse, petite taille)
  for (j = 0; j < cs; j++)
  {
    if (f[0 + rs*j] == 0) g[0 + rs*j] = 0; else g[0 + rs*j] = rs*cs; // infinity
    for (i = 1; i < rs; i++)
    {
      if (f[i + rs*j] == 0) g[i + rs*j] = 0; 
      else                  g[i + rs*j] = 1 + g[i-1 + rs*j]; 
    }
    for (i = rs-2; i >= 0; i--)
      if (g[i+1 + rs*j] < g[i + rs*j]) g[i + rs*j] = 1 + g[i+1 + rs*j];
    for (i = 0; i < rs; i++) 
    {
      if (g[i + rs*j] < rs*cs) // NECESSAIRE pour éviter un overflow
	g[i + rs*j] = g[i + rs*j] * g[i + rs*j];
    }
  }
} //  SEDT_line()

/* ======================================================== */
void SEDT_column(uint32_t *f, uint32_t *g, index_t rs, index_t cs)
/* ======================================================== */
{
  int32_t i, u, q, w; // attention: index signés (parcours inverse, petite taille)
  uint32_t *s, *t;
  s = (uint32_t *)calloc(1,cs * sizeof(uint32_t));
  t = (uint32_t *)calloc(1,cs * sizeof(uint32_t));

  for (i = 0; i < rs; i++)
  {
    q = 0; s[0] = 0; t[0] = 0;
    for (u = 1; u < cs; u++)
    {
      while ( (q >= 0) && (F_2_2d(s[q],t[q],f,i) > F_2_2d(u,t[q],f,i)) ) q--;
      if (q < 0)
      {
        q = 0;
        s[0] = u;
      }
      else
      {
        w = 1 + Sep_2_2d(s[q],u,f,i);
        if (w < cs)
        {
          q++; s[q] = u; t[q] = w;
        }
      }
    } 
    for (u = cs-1; u >= 0; u--)
    {
      g[rs*u + i] = F_2_2d(s[q],u,f,i);
      if (u == t[q]) q--;
    }
  }
  free(s); free(t);
} //  SEDT_column()

/* ======================================================== */
void SEDT3d_line(uint8_t *f, uint32_t *g, index_t rs, index_t cs, index_t ds)
/* ======================================================== */
{
  int32_t i, j, k, ps = rs*cs; // attention: index signés (parcours inverse, petite taille)
  for (k = 0; k < ds; k++)
    for (j = 0; j < cs; j++)
    {
      if (f[0 + rs*j + ps*k] == 0)
	g[0 + rs*j + ps*k] = 0; 
      else 
	g[0 + rs*j + ps*k] = rs*cs*ds; // infinity
      for (i = 1; i < rs; i++)
	if (f[i + rs*j + ps*k] == 0) g[i + rs*j + ps*k] = 0; 
	else g[i + rs*j + ps*k] = 1 + g[i-1 + rs*j + ps*k]; 
      for (i = rs-2; i >= 0; i--)
	if (g[i+1 + rs*j + ps*k] < g[i + rs*j + ps*k])
	  g[i + rs*j + ps*k] = 1 + g[i+1 + rs*j + ps*k];
      for (i = 0; i < rs; i++)
	if (g[i + rs*j + ps*k] < rs*cs*ds) // NECESSAIRE pour éviter un overflow
	  g[i + rs*j + ps*k] = g[i + rs*j + ps*k] * g[i + rs*j + ps*k];
    }
} //  SEDT3d_line()

/* ======================================================== */
void SEDT3d_column(uint32_t *f, uint32_t *g, index_t rs, index_t cs, index_t ds)
/* ======================================================== */
{
  int32_t i, k, u, q, ps = rs*cs, w; // attention: index signés (parcours inverse, petite taille)
  uint32_t *s, *t;
  s = (uint32_t *)calloc(1,cs * sizeof(uint32_t));
  t = (uint32_t *)calloc(1,cs * sizeof(uint32_t));

  for (k = 0; k < ds; k++)
    for (i = 0; i < rs; i++)
    {
      q = 0; s[0] = 0; t[0] = 0;
      for (u = 1; u < cs; u++)
      {
	while ( (q >= 0) && (F_2_3d(s[q],t[q],f,i,k) > F_2_3d(u,t[q],f,i,k)) ) q--;
	if (q < 0)
        {
	  q = 0;
	  s[0] = u;
	}
	else
        {
	  w = 1 + Sep_2_3d(s[q],u,f,i,k);
	  if (w < cs)
	  {
	    q++; s[q] = u; t[q] = w;
	  }
	}
      } 
      for (u = cs-1; u >= 0; u--)
      {
	g[ps*k + rs*u + i] = F_2_3d(s[q],u,f,i,k);
	if (u == t[q]) q--;
      }
    }
  free(s); free(t);
} //  SEDT3d_column()

/* ======================================================== */
void SEDT3d_planes(uint32_t *f, uint32_t *g, index_t rs, index_t cs, index_t ds)
/* ======================================================== */
{
  int32_t i, j, u, q, ps = rs*cs, w; // attention: index signés (parcours inverse, petite taille)
  uint32_t *s, *t;
  s = (uint32_t *)calloc(1,ds * sizeof(uint32_t));
  t = (uint32_t *)calloc(1,ds * sizeof(uint32_t));

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
    {
      q = 0; s[0] = 0; t[0] = 0;
      for (u = 1; u < ds; u++)
      {
	while ( (q >= 0) && (F_3_3d(s[q],t[q],f,i,j) > F_3_3d(u,t[q],f,i,j)) ) q--;
	if (q < 0)
        {
	  q = 0;
	  s[0] = u;
	}
	else
        {
	  w = 1 + Sep_3_3d(s[q],u,f,i,j);
	  if (w < ds)
	  {
	    q++; s[q] = u; t[q] = w;
	  }
	}
      } 
      for (u = ds-1; u >= 0; u--)
      {
	g[ps*u + rs*j + i] = F_3_3d(s[q],u,f,i,j);
	if (u == t[q]) q--;
      }
    }
  free(s); free(t);
} //  SEDT3d_planes()

/* ==================================== */
int32_t lsedt_meijster(struct xvimage *img,   /* donnee: image binaire */       
		   struct xvimage *res    /* resultat: distances */
)
/* ==================================== */
/* 
  Call the SEDT linear algorithm (Meijster & al.)
  ATTENTION: calcule la distance au complémentaire
*/
#undef F_NAME
#define F_NAME "lsedt_meijster"
{ 
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img);
  uint8_t *F;                /* pointeur sur l'image */
  uint32_t *D;               /* pointeur sur les distances */
  struct xvimage *tmp;
  uint32_t *T;

#ifdef DEBUG_MEIJSTER
  printf("lsedt_meijster : begin\n");
#endif

  if ((datatype(img) != VFF_TYP_1_BYTE) || (datatype(res) != VFF_TYP_4_BYTE))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  if ((rowsize(res) != rs) || (colsize(res) != cs) || (depth(res) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }    

  tmp = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (tmp == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(0);
  }
  T = ULONGDATA(tmp);
  D = ULONGDATA(res);
  F = UCHARDATA(img);  

  if (ds == 1)
  {
    SEDT_line(F, T, rs, cs);
    SEDT_column(T, D, rs, cs);
  }
  else
  {
    SEDT3d_line(F, D, rs, cs, ds);
    SEDT3d_column(D, T, rs, cs, ds);
    SEDT3d_planes(T, D, rs, cs, ds);
  }

  freeimage(tmp);
  return(1);
} // lsedt_meijster()

/* ==================================== */
int32_t ldistMeijster(struct xvimage *img,   /* donnee: image binaire */       
		  struct xvimage *res    /* resultat: distances */
)
/* ==================================== */
/* 
  Call the Meijster algorithm

  ATTENTION: calcule la distance au complémentaire
*/
#undef F_NAME
#define F_NAME "ldistMeijster"
{ 
  index_t i, rs = rowsize(img), cs = colsize(img), ds = depth(img);
  index_t N = rs * cs * ds;
  double * R = DOUBLEDATA(res);
  struct xvimage *dist;
  uint32_t *D;

  if ((datatype(img) != VFF_TYP_1_BYTE) || (datatype(res) != VFF_TYP_DOUBLE))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  dist = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (dist == NULL)
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return(0);
  }
  D = ULONGDATA(dist);

  if (!lsedt_meijster(img, dist))
  {
    fprintf(stderr, "%s: lsedt_meijster failed\n", F_NAME);
    return(0);
  }

  for (i = 0; i < N; i++) R[i] = sqrt(D[i]);

  freeimage(dist);
  return(1);
} // ldistMeijster()

/* ==================================== */
struct xvimage * lopeningfunction(
  struct xvimage *img,   /* donnee: image binaire */
  int32_t mode
)
/* ==================================== */
/* 
This operator associates, to each point x of the input object X, 
the radius of the biggest ball included in X that includes x.
The distance used depends on the optional parameter \b dist (default is 0) :
\li 0: approximate euclidean distance
\li 2: chamfer distance
\li 4: 4-distance in 2d
\li 8: 8-distance in 2d
\li 6: 6-distance in 3d
\li 18: 18-distance in 3d
\li 26: 26-distance in 3d

\warning The input image \b in.pgm must be a binary image. No test is done.
*/
#undef F_NAME
#define F_NAME "lopeningfunction"
{ 
  index_t rs = rowsize(img);
  index_t cs = colsize(img);
  index_t ds = depth(img); 
  index_t i, N = rs*cs*ds;
  struct xvimage *res;
  struct xvimage *tmp;
  uint8_t *T;
  uint32_t *R, r, vide;

  res = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (res == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return NULL;
  }
  R = ULONGDATA(res);
  razimage(res);

  if (ds == 1)
  {
    r = 1;
    do
    {
#ifdef VERBOSE
      printf("%s: r = %d\n", F_NAME, r);
#endif
      vide = 1;
      tmp = copyimage(img); assert(tmp != NULL);
      T = UCHARDATA(tmp);
      lopendisc(tmp, r, mode);
      for (i = 0; i < N; i++)
	if (T[i]) { R[i] = r; vide = 0; }
      freeimage(tmp);
      r++;
    } while (!vide && r < 150);
  }
  else
  {
    r = 1;
    do
    {
#ifdef VERBOSE
      printf("%s: r = %d\n", F_NAME, r);
#endif
      vide = 1;
      tmp = copyimage(img); assert(tmp != NULL);
      T = UCHARDATA(tmp);
      lopenball(tmp, r, mode);
      for (i = 0; i < N; i++)
	if (T[i]) { R[i] = r; vide = 0; }
      freeimage(tmp);
      r++;
    } while (!vide);
  }

  return res;
} // lopeningfunction()

/* ==================================== */
float ldistsets(
  struct xvimage *img1,   /* donnee: image binaire */
  struct xvimage *img2,   /* donnee: image binaire */
  int32_t mode,
  float cut
)
/* ==================================== */
/* 
Computes the distance between the object X defined by the binary image
img1 and the object Y defined by the binary image img2.

The used pointwise distance is the exact Euclidean distance (float).

The definition of the set distance used depends on the parameter 'mode':
\li 0: Hausdorff
\li 1: Baddeley, order 1
\li 2: Baddeley, order 2
\li 3: Dubuisson-Jain

The parameter 'cut' is required only for Baddeley distances. 

\warning The input images img1 and img2 must be binary images. No test is done.
*/
#undef F_NAME
#define F_NAME "ldistsets"
{ 
  index_t rs = rowsize(img1);
  index_t cs = colsize(img1);
  index_t ds = depth(img1); 
  index_t N = rs * cs * ds;
  uint8_t *I1 = UCHARDATA(img1);
  uint8_t *I2 = UCHARDATA(img2);
  struct xvimage *dist1;
  struct xvimage *dist2;
  float *D1, *D2;
  float result;
  index_t i;

  assert(datatype(img1) == VFF_TYP_1_BYTE);
  assert(datatype(img2) == VFF_TYP_1_BYTE);
  assert(rowsize(img2) == rs);
  assert(colsize(img2) == cs);
  assert(depth(img2) == ds);

  dist1 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  dist2 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if ((dist1 == NULL) || (dist2 == NULL))
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return -1;
  }

  for (i = 0; i < N; i++) I1[i] = !I1[i];
  for (i = 0; i < N; i++) I2[i] = !I2[i];
  // Attention : Ik représente maintenant le complémentaire de imgk
  if (! lsedt_meijster(img1, dist1))
  {
    fprintf(stderr, "%s: lsedt_meijster img1 failed\n", F_NAME);
    return -1;
  }
  if (! lsedt_meijster(img2, dist2))
  {
    fprintf(stderr, "%s: lsedt_meijster img2 failed\n", F_NAME);
    return -1;
  }

  convertfloat(&dist1);
  convertfloat(&dist2);
  D1 = FLOATDATA(dist1);
  D2 = FLOATDATA(dist2);
  for (i = 0; i < N; i++) D1[i] = (float)sqrt(D1[i]);
  for (i = 0; i < N; i++) D2[i] = (float)sqrt(D2[i]);

  if (mode == 0) // Hausdorff
  {
    float max1 = 0.0, max2 = 0.0;
    for (i = 0; i < N; i++) if (!I1[i] && (D2[i] > max2)) max2 = D2[i];
    for (i = 0; i < N; i++) if (!I2[i] && (D1[i] > max1)) max1 = D1[i];
    result = mcmax(max1,max2);
  }
  else if (mode == 1) // Baddeley, order 1
  {
    float d, sum = 0.0;
    for (i = 0; i < N; i++)
    { 
      d = mcmin(D2[i],cut) - mcmin(D1[i],cut);
      sum = sum + mcabs(d);
    }
    result = sum / N;
  }
  else if (mode == 2) // Baddeley, order 2
  {
    float d, sum = 0.0;
    for (i = 0; i < N; i++)
    { 
      d = mcmin(D2[i],cut) - mcmin(D1[i],cut);
      sum = sum + (d * d);
    }
    result = (float)sqrt(sum / N);
  }
  else if (mode == 3) // Dubuisson-Jain
  {
    float av1 = 0.0, av2 = 0.0;
    int32_t n1 = 0, n2 = 0;
    for (i = 0; i < N; i++) if (!I1[i]) { av2 += D2[i]; n2++; }
    for (i = 0; i < N; i++) if (!I2[i]) { av1 += D1[i]; n1++; }
    av1 = av1 / n2;
    av2 = av2 / n2;
    result = mcmax(av1,av2);
  }
  else
  {
    fprintf(stderr, "%s: bad value for mode: %d\n", F_NAME, mode);
    return -1;
  }

  return result;
} // ldistsets()
