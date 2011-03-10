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
Squelette 2D ou 3D binaire guide par une fonction distance 
et value.

Soit X l'objet initial.
Soit DX une fonction distance au complementaire de X (selon la distance d).
Les points ayant les plus basses valeurs dans DX sont traites en priorite.
Les points ayant une valeur specifiee VAL_INHIBIT ne seront
pas selectionnes.

On definit l'operateur SquelVal(X,DX,DT) : 
// 1ere phase
DT(X) = INFINI; DT(~X) = 0
k = 0;
repeter jusqu'a stabilite
  choisir un point x de X, simple pour X, tel que DX[x] != VAL_INHIBIT 
      et de valeur de DX minimale
  DT[x] = k
  X = X \ {x}
  k++
fin repeter 
// 2eme phase
pour tout x tel que DT[x] != 0, par ordre de valeurs DT croissantes
  faire
    si x simple pour DT a la coupe de niveau DT[x] et a la coupe de niveau DT[x]-1 alors 
      abaisse x au niveau min(alpha8-(DT,x)+1,DT[x]-1)
      // pour avoir la prop. 6 on rajoute l'instruction:
      // si il existe y dans gamma(x) telque alpha+(DT,y) == DT[y] alors DT[x]++
  tant que x a pu etre abaisse
fin pour
resultat: X, DT
*/
/* Michel Couprie - mai 2002 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mctopo.h>
#include <mcindic.h>
#include <mcrbt.h>
#include <mcliste.h>
#include <mctopo3d.h>
#include <mcutil.h>
#include <lbresen.h>
#include <lsquelval.h>

#define INFINI        2000000000
#define EN_RBT        0
#define PARANO
#define INCR_PRIO     1

/*
#define DEBUG
*/
#define VERBOSE

/* ==================================== */
int32_t abaisse8(int32_t x, int32_t *DT, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t y, k, abaisse;
  int32_t t4mm, t4m, t8p, t8pp;
  uint32_t d, old;

  old = DT[x];

  do
  {
    abaisse = 0;
    nbtopoh_l(DT, x, DT[x], rs, N, &t4mm, &t4m, &t8p, &t8pp);
    if ((t8p == 1) && (t4mm == 1))
    {
      nbtopoh_l(DT, x, DT[x]-1, rs, N, &t4mm, &t4m, &t8p, &t8pp);
      if ((t8p == 1) && (t4mm == 1))
      {
        d = alpha8m_l(DT, x, rs, N);
        d = mcmin((DT[x]-1),(d+1));
        DT[x] = d;
        abaisse = 1;
#ifdef DEBUG
        printf("pre-abaisse a d = %ld\n", d);
#endif
#ifndef OLD_VERSION
        for (k = 0; k < 8; k += 1)         /* parcourt les voisins en 8-connexite  */
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (DT[y] <= DT[x]) && (DT[y]==alpha8p_l(DT, y, rs, N)))
          {
            DT[x] += 1; abaisse = 0; break;
          } /* if y */
        } /* for k */      
#endif
      }
    } // if ((t8p == 1) && (t4mm == 1))
  } while (abaisse);
  if (DT[x] < old) return 1; else return 0;
} // abaisse8()

/* ==================================== */
int32_t abaisse4(int32_t x, int32_t *DT, int32_t rs, int32_t N)
/* ==================================== */
{
  int32_t y, k, abaisse;
  int32_t t8mm, t8m, t4p, t4pp;
  uint32_t d, old;

  old = DT[x];
  do
  {
    abaisse = 0;
    nbtopoh2_l(DT, x, DT[x], rs, N, &t8mm, &t8m, &t4p, &t4pp);
    if ((t4p == 1) && (t8mm == 1))
    {
      nbtopoh2_l(DT, x, DT[x]-1, rs, N, &t8mm, &t8m, &t4p, &t4pp);
      if ((t4p == 1) && (t8mm == 1))
      {
        d = alpha8m_l(DT, x, rs, N);
        d = mcmin((DT[x]-1),(d+1));
        DT[x] = d;
        abaisse = 1;
#ifdef DEBUG
        printf("pre-abaisse a d = %ld\n", d);
#endif
#ifndef OLD_VERSION
        for (k = 0; k < 8; k += 2)         /* parcourt les voisins en 4-connexite  */
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (DT[y] <= DT[x]) && (DT[y]==alpha4p_l(DT, y, rs, N)))
          {
            DT[x] += 1; break;
          } /* if y */
        } /* for k */      
#endif
      }  // if ((t4p == 1) && (t8mm == 1))
    } // if ((t4p == 1) && (t8mm == 1))
  } while (abaisse);
  if (DT[x] < old) return 1; else return 0;
} // abaisse4()

/* ==================================== */
int32_t lsquelval(struct xvimage *image, // entree/sortie: image originale / squelette
              struct xvimage *dx,    // entree/sortie: distance / distance topologique
              int32_t connex, 
              int32_t val_inhibit)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsquelval"
{ 
  int32_t k;
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  struct xvimage *dt;          /* pour le calcul de la "distance topologique" */
  uint8_t *IM = UCHARDATA(image);      /* l'image de depart */
  uint32_t *DX;           /* fonction distance au complementaire de IM */
  uint32_t *DT;                    /* fonction "distance topologique" */
  uint32_t d;
  Rbt * RBT;
  int32_t taillemaxrbt;

  IndicsInit(N);
  if ((rowsize(dx) != rs) || (colsize(dx) != cs) || (depth(dx) != 1))
  {
    fprintf(stderr, "%s() : bad size for dx\n", F_NAME);
    return(0);
  }
  if (datatype(dx) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s() : datatype(dx) must be uint32_t\n", F_NAME);
    return(0);
  }
  DX = ULONGDATA(dx); 
  dt = copyimage(dx);
  DT = ULONGDATA(dt); 
  taillemaxrbt = 2 * rs + 2 * cs ;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s() : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               PREMIERE PHASE                     */
  /* ================================================ */

#ifdef VERBOSE
    printf("1ere etape\n");
#endif

  // INITIALISATION DT
  for (x = 0; x < N; x++) if (IM[x]) DT[x] = INFINI; else DT[x] = 0;

  // INITIALISATION DU RBT
  for (x = 0; x < N; x++)
    if (IM[x] && (DX[x] != val_inhibit) && bordext8(IM, x, rs, N))
    {
      mcrbt_RbtInsert(&RBT, DX[x], x);
      Set(x, EN_RBT);
    } // if, for

  d = 1;
  if (connex == 4)
  {
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      UnSet(x, EN_RBT);
#ifdef DEBUG
      printf("pop x = %d,%d, im = %d, dx = %ld\n", x%rs, x/rs, IM[x], DX[x]);
#endif
      if (simple4(IM, x, rs, N))
      {
        DT[x] = d;
        IM[x] = 0;
        d++;
        for (k = 0; k < 8; k += 1)         /* parcourt les voisins en 8-connexite  */
        {                                              /* pour empiler les voisins */
          y = voisin(x, k, rs, N);                             /* non deja empiles */
          if ((y != -1) && (IM[y]) && (DX[y] != val_inhibit) && (! IsSet(y, EN_RBT)))
          {
            mcrbt_RbtInsert(&RBT, DX[y], y);
            Set(y, EN_RBT);
          } /* if y */
        } /* for k */      
      } // if (simple4(IM, x, rs, N))
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 4) */
  else
  if (connex == 8)
  {
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      UnSet(x, EN_RBT);
#ifdef DEBUG
      printf("pop x = %d,%d, im = %d, dx = %ld\n", x%rs, x/rs, IM[x], DX[x]);
#endif
      if (simple8(IM, x, rs, N))
      {
        DT[x] = d;
        IM[x] = 0;
        d++;
        for (k = 0; k < 8; k += 1)         /* parcourt les voisins en 8-connexite  */
        {                                              /* pour empiler les voisins */
          y = voisin(x, k, rs, N);                             /* non deja empiles */
          if ((y != -1) && (IM[y]) && (DX[y] != val_inhibit) && (! IsSet(y, EN_RBT)))
          {
            mcrbt_RbtInsert(&RBT, DX[y], y);
            Set(y, EN_RBT);
          } /* if y */
        } /* for k */      
      } // if (simple8(IM, x, rs, N))
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 8) */

#ifdef DEBUG
  writeimage(dt, "_dt");
#endif

  /* ================================================ */
  /*               SECONDE PHASE                      */
  /* ================================================ */

#ifdef SECONDE_PHASE

  stabilite = 0;
  while (!stabilite)
  {
#ifdef VERBOSE
    printf("2eme etape\n");
#endif
    stabilite = 1;

    // INITIALISATION DU RBT
    for (j = 1; j < cs-1; j++) 
      for (i = 1; i < rs-1; i++) 
      {
        x = j * rs + i;
        if (DT[x] && (DT[x] != INFINI)) mcrbt_RbtInsert(&RBT, DT[x], x);
      }

    if (connex == 4)
    {
      while (!mcrbt_RbtVide(RBT))
      {
        x = RbtPopMin(RBT);
#ifdef DEBUG
        printf("pop x = %d,%d, dt = %ld\n", x%rs, x/rs, DT[x]);
#endif
        if (abaisse4(x, DT, rs, N)) 
        {
          stabilite = 0;
#ifdef DEBUG
          printf("abaisse a %ld\n", DT[x]);
#endif
	} // if (abaisse4(x, DT, rs, N)) 
      } // while (!mcrbt_RbtVide(RBT))
    } // if (connex == 4)
    else
    if (connex == 8)
    {
      int32_t abaisse;
      while (!mcrbt_RbtVide(RBT))
      {
        x = RbtPopMin(RBT);
#ifdef DEBUG
        printf("pop x = %d,%d, dt = %d\n", x%rs, x/rs, DT[x]);
#endif
        if (abaisse8(x, DT, rs, N)) 
        {
          stabilite = 0;
#ifdef DEBUG
          printf("abaisse a %ld\n", DT[x]);
#endif
	} // if (abaisse8(x, DT, rs, N)) 
      } // while (!mcrbt_RbtVide(RBT))
    } // if (connex == 8)
  } // while (!stabilite)

#endif

  // RECUPERATION DU RESULTAT
  d = 0; // valeur pour l'infini: plus grande valeur finie + 1
  for (x = 0; x < N; x++) if ((DT[x] > d) && (DT[x] < INFINI)) d = DT[x];
  d += 1;
  for (x = 0; x < N; x++) if (DT[x] == INFINI) DX[x] = d; else DX[x] = DT[x];
  
  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  freeimage(dt);
  return(1);
} /* lsquelval() */

/* ==================================== */
int32_t lsquelval3d(struct xvimage *image, // entree/sortie: image originale / squelette
                struct xvimage *dx,    // entree/sortie: distance / distance topologique
                int32_t connex, 
                int32_t val_inhibit)
/* ==================================== */
{ 
  int32_t i, j, k;
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t ds = depth(image);
  int32_t N = ds * ps;             /* taille image */
  struct xvimage *dt;          /* pour le calcul de la "distance topologique" */
  uint8_t *IM = UCHARDATA(image);      /* l'image de depart */
  uint32_t *DX;           /* fonction distance au complementaire de IM */
  uint32_t *DT;           /* fonction "distance topologique" */
  uint32_t d;
  Rbt * RBT;
  int32_t taillemaxrbt;
  int32_t mctopo3d_t6mm, mctopo3d_t26mm, t6p, mctopo3d_t26p;

  IndicsInit(N);
  mctopo3d_init_topo3d();
  if ((rowsize(dx) != rs) || (colsize(dx) != cs) || (depth(dx) != ds))
  {
    fprintf(stderr, "%s() : bad size for dx\n", F_NAME);
    return(0);
  }
  if (datatype(dx) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s() : datatype(dx) must be uint32_t\n", F_NAME);
    return(0);
  }
  DX = ULONGDATA(dx); 
  dt = copyimage(dx);
  DT = ULONGDATA(dt); 
  taillemaxrbt = 2 * rs * cs +  2 * rs * ds +  2 * ds * cs;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s() : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  /* ================================================ */
  /*               PREMIERE PHASE                     */
  /* ================================================ */

#ifdef VERBOSE
    printf("1ere etape\n");
#endif

  // INITIALISATION DT
  for (x = 0; x < N; x++) if (IM[x]) DT[x] = -1; else DT[x] = 0;

  // INITIALISATION DU RBT
  for (x = 0; x < N; x++)
    if (IM[x] && (DX[x] != val_inhibit) && mctopo3d_bordext26(IM, x, rs, ps, N))
    {
      mcrbt_RbtInsert(&RBT, DX[x], x);
      Set(x, EN_RBT);
    } // if, for

  d = 1;
  if (connex == 6)
  {
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      UnSet(x, EN_RBT);
      if (mctopo3d_simple6(IM, x, rs, ps, N))
      {
        DT[x] = d;
        IM[x] = 0;
        d++;
        for (k = 0; k < 26; k += 1)       /* parcourt les voisins en 26-connexite  */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (IM[y]) && (DX[y] != val_inhibit) && (! IsSet(y, EN_RBT)))
          {
            mcrbt_RbtInsert(&RBT, DX[y], y);
            Set(y, EN_RBT);
          } /* if y */
        } /* for k */      
      } // if (mctopo3d_simple6(IM, x, rs, N))
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 6) */
  else
  if (connex == 26)
  {
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      UnSet(x, EN_RBT);
      if (mctopo3d_simple26(IM, x, rs, ps, N))
      {
        DT[x] = d;
        IM[x] = 0;
        d++;
        for (k = 0; k < 26; k += 1)       /* parcourt les voisins en 26-connexite  */
        {                                              /* pour empiler les voisins */
          y = voisin26(x, k, rs, ps, N);                       /* non deja empiles */
          if ((y != -1) && (IM[y]) && (DX[y] != val_inhibit) && (! IsSet(y, EN_RBT)))
          {
            mcrbt_RbtInsert(&RBT, DX[y], y);
            Set(y, EN_RBT);
          } /* if y */
        } /* for k */      
      } // if (mctopo3d_simple26(IM, x, rs, N))
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 26) */

  /* ================================================ */
  /*               SECONDE PHASE                      */
  /* ================================================ */

#ifdef VERBOSE
    printf("2eme etape\n");
#endif

  // INITIALISATION DU RBT
  for (k = 1; k < ds-1; k++) 
    for (j = 1; j < cs-1; j++) 
      for (i = 1; i < rs-1; i++) 
      {
        x = k * ps + j * rs + i;
        if (DT[x]) mcrbt_RbtInsert(&RBT, DT[x], x);
      }

  if (connex == 6)
  {
    int32_t abaisse;
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      do
      {
        abaisse = 0;
        mctopo3d_nbtopoh3d26_l((int32_t *)DT, x, DT[x], rs, ps, N, &t6p, &mctopo3d_t26mm);
        if ((t6p == 1) && (mctopo3d_t26mm == 1))
	{
          mctopo3d_nbtopoh3d26_l((int32_t *)DT, x, DT[x], rs, ps, N, &t6p, &mctopo3d_t26mm);
          if ((t6p == 1) && (mctopo3d_t26mm == 1))
	  {
            d = mctopo3d_alpha26m_l((int32_t *)DT, x, rs, ps, N);
            d = mcmin((DT[x]-1),(d+1));
            DT[x] = d;
            abaisse = 1;
	  }
        }
      } while (abaisse);
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 6) */
  else
  if (connex == 26)
  {
    int32_t abaisse;
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      do
      {
        abaisse = 0;
        mctopo3d_nbtopoh3d6_l((int32_t *)DT, x, DT[x], rs, ps, N, &mctopo3d_t26p, &mctopo3d_t6mm);
        if ((mctopo3d_t26p == 1) && (mctopo3d_t6mm == 1))
	{
          mctopo3d_nbtopoh3d26_l((int32_t *)DT, x, DT[x], rs, ps, N, &mctopo3d_t26p, &mctopo3d_t6mm);
          if ((mctopo3d_t26p == 1) && (mctopo3d_t6mm == 1))
	  {
            d = mctopo3d_alpha26m_l((int32_t *)DT, x, rs, ps, N);
            d = mcmin((DT[x]-1),(d+1));
            DT[x] = d;
            abaisse = 1;
	  }
        }
      } while (abaisse);
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 26) */


  // RECUPERATION DU RESULTAT
  d = 0;
  for (x = 0; x < N; x++) if ((DT[x] > d) && (DT[x] < INFINI)) d = DT[x];
  d += 1;
  for (x = 0; x < N; x++) if (DT[x] == INFINI) DX[x] = d; else DX[x] = DT[x];

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  mctopo3d_termine_topo3d();
  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  freeimage(dt);
  return(1);
} /* lsquelval3d() */

/* ==================================== */
int32_t smooth(struct xvimage *image, int32_t x, int32_t r, Liste *cx, Liste *cy)
/* ==================================== */
/*
  soit X l'ensemble des points a 0 dans image
  renvoie 1 si X U {x} est ouvert par un cercle de rayon r, 0 sinon 
*/
{ 
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  uint8_t *IM = UCHARDATA(image);      /* l'image de depart */
  int32_t y, z, i, j, k, l, open, joker;

  ListeFlush(cx);
  lellipseliste2(cx, rs, cs, r, 0, 0, r, x%rs, x/rs);
  while (!ListeVide(cx))
  {
    y = ListePop(cx);
    if (IM[y] == 0)
    {
      i = y % rs; j = y / rs; 
      open = 1;
      joker = 1;
      for (k = i-r; k <= i+r; k++)
      for (l = j-r; l <= j+r; l++)
      if ((k-i)*(k-i)+(l-j)*(l-j) <= r*r)
      {
        z = l * rs + k;
        if (((k < 0) && (k >= rs) && (l < 0) && (l >= cs)) || 
            ((z != x) && (IM[z] != 0)))
        {
          if (joker) joker--;
          else { open = 0; break; }
        }
      }
      if (open) return 1; 
    }
  } // while (!ListeVide(cx))
  return 0;
} // smooth()

/* ==================================== */
int32_t lsquelsmoothval(struct xvimage *image, // entree/sortie: image originale / squelette
              struct xvimage *dx,    // entree/sortie: distance / distance topologique
              struct xvimage *ni,    // entree/sortie: niveaux - image 1D 
              struct xvimage *gr,    // entree: gradient
              int32_t connex, 
              int32_t val_inhibit, 
              int32_t rayon)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsquelsmoothval"
{ 
  int32_t k;
  int32_t x;                       /* index muet de pixel */
  int32_t y;                       /* index muet (generalement un voisin de x) */
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  struct xvimage *dt;          /* pour le calcul de la "distance topologique" */
  uint32_t *DT;           /* fonction "distance topologique" */
  uint8_t *IM = UCHARDATA(image); /* l'image de depart */
  uint32_t *DX = ULONGDATA(dx);    /* fonction distance au complementaire de IM */
  uint8_t *NI = UCHARDATA(ni);    /* fonction niveau (1D) - taille N * 1 */
  uint8_t *GR = UCHARDATA(gr);    /* fonction gradient */
  uint32_t d;
  Rbt * RBT;
  int32_t taillemaxrbt;
  Liste * cx;                  // pour le cercle de centre x
  Liste * cy;                  // pour le cercle de centre y
  uint32_t dmax;

  IndicsInit(N);
  if ((rowsize(dx) != rs) || (colsize(dx) != cs) || (depth(dx) != 1))
  {
    fprintf(stderr, "%s() : bad size for dx\n", F_NAME);
    return(0);
  }
  if (datatype(dx) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s() : datatype(dx) must be uint32_t\n", F_NAME);
    return(0);
  }
  if ((rowsize(ni) != N) || (colsize(ni) != 1) || (depth(ni) != 1))
  {
    fprintf(stderr, "%s() : bad size for ni\n", F_NAME);
    return(0);
  }
  dt = copyimage(dx);
  DT = ULONGDATA(dt); 
  taillemaxrbt = 2 * rs + 2 * cs ;
  /* cette taille est indicative, le RBT est realloue en cas de depassement */
  RBT = mcrbt_CreeRbtVide(taillemaxrbt);
  if (RBT == NULL)
  {
    fprintf(stderr, "%s() : mcrbt_CreeRbtVide failed\n", F_NAME);
    return(0);
  }

  for (dmax = 0, x = 0; x < N; x++) if (DX[x] > dmax) dmax = DX[x];

  // INITIALISATION DT
  for (x = 0; x < N; x++) if (IM[x]) DT[x] = INFINI; else DT[x] = 0;

  // INITIALISATION DU RBT
  for (x = 0; x < N; x++)
    if (IM[x] && (DX[x] != val_inhibit) && bordext8(IM, x, rs, N))
    {
      mcrbt_RbtInsert(&RBT, DX[x], x);
      Set(x, EN_RBT);
    } // if, for

  cx = CreeListeVide(2*rs + 2*cs);
  cy = CreeListeVide(2*rs + 2*cs);

  d = 1;
  if (connex == 4)
  {
    while (!mcrbt_RbtVide(RBT))
    {
      x = RbtPopMin(RBT);
      UnSet(x, EN_RBT);
#ifdef DEBUG
      printf("pop x = %d,%d, im = %d, dx = %ld\n", x%rs, x/rs, IM[x], DX[x]);
#endif
      if (simple4(IM, x, rs, N))
      {
        if (smooth(image, x, rayon, cx, cy))
	{
          NI[d] = GR[x];
          DT[x] = d;
          IM[x] = 0;
          d++;
          for (k = 0; k < 8; k += 1)         /* parcourt les voisins en 8-connexite  */
          {                                              /* pour empiler les voisins */
            y = voisin(x, k, rs, N);                             /* non deja empiles */
            if ((y != -1) && (IM[y]) && (DX[y] != val_inhibit) && (! IsSet(y, EN_RBT)))
            {
              mcrbt_RbtInsert(&RBT, DX[y], y);
              Set(y, EN_RBT);
            } /* if y */
          } /* for k */      
	}
        else
	{
          DX[x] += INCR_PRIO;
          if (DX[x] > dmax) break;
          mcrbt_RbtInsert(&RBT, DX[x], x);
          Set(x, EN_RBT);
	}
      } // if (simple4(IM, x, rs, N))
    } /* while (!mcrbt_RbtVide(RBT)) */
  } /* if (connex == 4) */
  else
  if (connex == 8)
  {
    printf("connex 8 NYI\n");
  } /* if (connex == 8) */

  // RECUPERATION DU RESULTAT
  d = 0; // valeur pour l'infini: plus grande valeur finie + 1
  for (x = 0; x < N; x++) if ((DT[x] > d) && (DT[x] < INFINI)) d = DT[x];
  d += 1;
  for (x = 0; x < N; x++) if (DT[x] == INFINI) DX[x] = d; else DX[x] = DT[x];
  
  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  mcrbt_RbtTermine(RBT);
  freeimage(dt);
  ListeTermine(cx);  
  ListeTermine(cy);  
  return(1);
} /* lsquelsmoothval() */
