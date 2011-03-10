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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mctopo.h>
#include <mcimage.h>
#include <mccodimage.h>

/****************************************************************
*
* Routine Name: llevialdi - library call for levialdi
*
* Purpose: algorithme parallele d'amincissement de Levialdi
*
* Input:
* Output:
* Written By: Michel Couprie - juillet 1999
*
****************************************************************/

#define VERBOSE

/*
#define DEBUG
*/
#define WRITEINTER


/* ==================================== */
int32_t llevialdi(
        struct xvimage *img, /* image de depart */
        int32_t connex,          /* 4, 8 */
        int32_t nbiter)          /* nombre d'iterations */
/* ==================================== */
{
  int32_t x;
  uint8_t *F = UCHARDATA(img);
  uint8_t *imgt;
  uint8_t *FT;
  uint8_t *FTT;
  int32_t rs = rowsize(img);
  int32_t cs = colsize(img);
  int32_t N = rs * cs;          /* taille image */
  int32_t stabilite;
  int32_t p;
  int32_t mn, ms, me, mo, mne, mno, mse, mso;
  int32_t pn, ps, pe, po, pne, pno, pse, pso;

  if (depth(img) != 1)
  {
    fprintf(stderr, "llevialdi: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  imgt = (uint8_t *)calloc(1,N * sizeof(char));
  if (imgt == NULL)
  {
    fprintf(stderr, "llevialdi: malloc failed\n");
    return 0;
  }
  memcpy((void *)imgt, (void *)F, N * sizeof(char));
  FT = imgt;
  
  stabilite = 0;

  while ((nbiter > 0) && (!stabilite))
  {
#ifdef VERBOSE
printf("reste %d iteration(s)\n", nbiter);
#endif

#ifdef DEBUG1
printf("img = %d ; img->name = %d,%s\n", (int32_t)img, (int32_t)(img->name), img->name);
printf("F = %d ; FT = %d ; rs = %d ; cs = %d ; N = %d\n", (int32_t)F, (int32_t)FT, rs, cs, N);
#endif

    stabilite = 1;

    for (x = 0; x < N; x++)
    {
      if (!bord(x, rs, N))
      {
#ifdef BINAIRE
        p  = (int32_t)FT[x];
        e  = (int32_t)FT[x+1];
        ne = (int32_t)FT[x+1-rs];
        n  = (int32_t)FT[x-rs];
        no = (int32_t)FT[x-rs-1];
        o  = (int32_t)FT[x-1];
        so = (int32_t)FT[x-1+rs];
        s  = (int32_t)FT[x+rs];
        se = (int32_t)FT[x+rs+1];
        if (p && (!e && !s)) 
          { F[x] = (uint8_t)0; stabilite = 0; }
        else if (!p && (e && s && se)) 
          { F[x] = (uint8_t)255; stabilite = 0; }
        else F[x] = FT[x];
#endif
        p  = (int32_t)FT[x];
        me  = (int32_t)FT[x+1] < p;
        ms  = (int32_t)FT[x+rs] < p;
        pe  = (int32_t)FT[x+1] > p;
        ps  = (int32_t)FT[x+rs] > p;
        pse = (int32_t)FT[x+rs+1] > p;
        if (me && ms)
          { F[x] = alpha8m(FT, x, rs, N); stabilite = 0; }
        else if (pe && ps && pse)
          { F[x] = alpha8p(FT, x, rs, N); stabilite = 0; }
        else F[x] = FT[x];
      } /* if (!bord(x, rs, N)) */
    } /* for (x = 0; x < N; x++) */

    FTT = FT; FT = F; F = FTT;
    nbiter--;
  } /* while ((nbiter > 0) && (!stabilite)) */

#ifdef DEBUG
printf("fin boucle\n");
#endif

  FTT = UCHARDATA(img);
  for (x = 0; x < N; x++) FTT[x] = FT[x];
  free(imgt);
  return(1);
}

/* ==================================== */
int32_t lrao(
        struct xvimage *img, /* image de depart */
        int32_t connex,          /* 4, 8 */
        int32_t nbiter)          /* nombre d'iterations */
/* ==================================== */
{
  int32_t x;
  uint8_t *F = UCHARDATA(img);
  uint8_t *imgt;
  uint8_t *FT;
  uint8_t *FTT;
  int32_t rs = rowsize(img);
  int32_t cs = colsize(img);
  int32_t N = rs * cs;          /* taille image */
  int32_t stabilite;
  int32_t p;
  int32_t n, s, e, o, ne, no, se, so;
  int32_t a1, a2, a3, a4, b1, b2, b3, b4;
  int32_t A1, A2, A3, A4, B1, B2, B3, B4;
  char buf[1024];

  if (depth(img) != 1)
  {
    fprintf(stderr, "lrao: cette version ne traite pas les images volumiques\n");
    return 0;
  }

  imgt = (uint8_t *)calloc(1,N * sizeof(char));
  if (imgt == NULL)
  {
    fprintf(stderr, "lrao: malloc failed\n");
    return 0;
  }
  memcpy((void *)imgt, (void *)F, N * sizeof(char));
  FT = imgt;
  
  stabilite = 0;

  while ((nbiter > 0) && (!stabilite))
  {
#ifdef VERBOSE
printf("reste %d iteration(s)\n", nbiter);
#endif

#ifdef DEBUG1
printf("img = %d ; img->name = %d,%s\n", (int32_t)img, (int32_t)(img->name), img->name);
printf("F = %d ; FT = %d ; rs = %d ; cs = %d ; N = %d\n", (int32_t)F, (int32_t)FT, rs, cs, N);
#endif

    stabilite = 1;

    /* 1ere demi-iteration (sud-ouest) */

      for (x = 0; x < N; x++)
      {
        if (!bord(x, rs, N))
        {
#ifdef BINAIRE
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1];
          ne = (int32_t)FT[x+1-rs];
          n  = (int32_t)FT[x-rs];
          no = (int32_t)FT[x-rs-1];
          o  = (int32_t)FT[x-1];
          so = (int32_t)FT[x-1+rs];
          s  = (int32_t)FT[x+rs];
          se = (int32_t)FT[x+rs+1];
          A1 = !p && !so && n && e;
          A2 = !p && !no && s && e;
          B1 = !p && o && s && e;
          B2 = !p && n && o && s;
          B3 = !p && n && e && s;
          B4 = !p && o && n && e;
          if (A1 || A2 || B1 || B2 || B3 || B4) 
          { 
            F[x] = (uint8_t)255; 
            stabilite = 0; 
          }
          else F[x] = FT[x];
#endif
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1] > p;
          ne = (int32_t)FT[x+1-rs] > p;
          n  = (int32_t)FT[x-rs] > p;
          no = (int32_t)FT[x-rs-1] > p;
          o  = (int32_t)FT[x-1] > p;
          so = (int32_t)FT[x-1+rs] > p;
          s  = (int32_t)FT[x+rs] > p;
          se = (int32_t)FT[x+rs+1] > p;
          A1 = !so && n && e;
          A2 = !no && s && e;
          B1 = o && s && e;
          B2 = n && o && s;
          B3 = n && e && s;
          B4 = o && n && e;
          if (A1 || A2 || B1 || B2 || B3 || B4) 
          { 
            F[x] = alpha8p(FT, x, rs, N); 
            stabilite = 0; 
          }
          else F[x] = FT[x];
        } /* if (!bord(x, rs, N)) */
      } /* for (x = 0; x < N; x++) */
      FTT = FT; FT = F; F = FTT;
      for (x = 0; x < N; x++)
      {
        if (!bord(x, rs, N))
        {
#ifdef BINAIRE
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1];
          ne = (int32_t)FT[x+1-rs];
          n  = (int32_t)FT[x-rs];
          no = (int32_t)FT[x-rs-1];
          o  = (int32_t)FT[x-1];
          so = (int32_t)FT[x-1+rs];
          s  = (int32_t)FT[x+rs];
          se = (int32_t)FT[x+rs+1];
          a1 = !no && !n && !ne && p && s;
          a2 = !ne && !e && !se && p && o;
          a3 = !so && !o && !no && !n && !ne && p && se;
          a4 = !no && !n && !ne && !e && !se && p && so;
          if (a1 || a2 || a3 || a4)
          { 
            F[x] = (uint8_t)0; 
            stabilite = 0; 
          }
          else F[x] = FT[x];
#endif
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1] >= p;
          ne = (int32_t)FT[x+1-rs] >= p;
          n  = (int32_t)FT[x-rs] >= p;
          no = (int32_t)FT[x-rs-1] >= p;
          o  = (int32_t)FT[x-1] >= p;
          so = (int32_t)FT[x-1+rs] >= p;
          s  = (int32_t)FT[x+rs] >= p;
          se = (int32_t)FT[x+rs+1] >= p;
          a1 = !no && !n && !ne && s;
          a2 = !ne && !e && !se && o;
          a3 = !so && !o && !no && !n && !ne && se;
          a4 = !no && !n && !ne && !e && !se && so;
          if (a1 || a2 || a3 || a4)
          { 
            F[x] = alpha8m(FT, x, rs, N); 
            stabilite = 0; 
          }
          else F[x] = FT[x];
        } /* if (!bord(x, rs, N)) */
      } /* for (x = 0; x < N; x++) */
      FTT = FT; FT = F; F = FTT;

    /* 2eme demi-iteration (nord-est) */

      for (x = 0; x < N; x++)
      {
        if (!bord(x, rs, N))
        {
#ifdef BINAIRE
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1];
          ne = (int32_t)FT[x+1-rs];
          n  = (int32_t)FT[x-rs];
          no = (int32_t)FT[x-rs-1];
          o  = (int32_t)FT[x-1];
          so = (int32_t)FT[x-1+rs];
          s  = (int32_t)FT[x+rs];
          se = (int32_t)FT[x+rs+1];
          A3 = !p && !se && n && o;
          A4 = !p && !ne && s && o;
          B1 = !p && o && s && e;
          B2 = !p && n && o && s;
          B3 = !p && n && e && s;
          B4 = !p && o && n && e;
          if (A3 || A4 || B1 || B2 || B3 || B4) 
          { 
            F[x] = (uint8_t)255; 
            stabilite = 0; 
          }
          else F[x] = FT[x];
#endif
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1] > p;
          ne = (int32_t)FT[x+1-rs] > p;
          n  = (int32_t)FT[x-rs] > p;
          no = (int32_t)FT[x-rs-1] > p;
          o  = (int32_t)FT[x-1] > p;
          so = (int32_t)FT[x-1+rs] > p;
          s  = (int32_t)FT[x+rs] > p;
          se = (int32_t)FT[x+rs+1] > p;
          A3 = !se && n && o;
          A4 = !ne && s && o;
          B1 = o && s && e;
          B2 = n && o && s;
          B3 = n && e && s;
          B4 = o && n && e;
          if (A3 || A4 || B1 || B2 || B3 || B4) 
          { 
            F[x] = alpha8p(FT, x, rs, N); 
            stabilite = 0; 
          }
          else F[x] = FT[x];
        } /* if (!bord(x, rs, N)) */
      } /* for (x = 0; x < N; x++) */
      FTT = FT; FT = F; F = FTT;
      for (x = 0; x < N; x++)
      {
        if (!bord(x, rs, N))
        {
#ifdef BINAIRE
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1];
          ne = (int32_t)FT[x+1-rs];
          n  = (int32_t)FT[x-rs];
          no = (int32_t)FT[x-rs-1];
          o  = (int32_t)FT[x-1];
          so = (int32_t)FT[x-1+rs];
          s  = (int32_t)FT[x+rs];
          se = (int32_t)FT[x+rs+1];
          b1 = !so && !s && !se && p && n;
          b2 = !no && !o && !so && p && e;
          b3 = !so && !s && !se && !e && !ne && p && no;
          b4 = !no && !o && !so && !s && !se && p && ne;
          if (b1 || b2 || b3 || b4)
          { 
            F[x] = (uint8_t)0; 
            stabilite = 0; 
          }
          else F[x] = FT[x];
#endif
          p  = (int32_t)FT[x];
          e  = (int32_t)FT[x+1] >= p;
          ne = (int32_t)FT[x+1-rs] >= p;
          n  = (int32_t)FT[x-rs] >= p;
          no = (int32_t)FT[x-rs-1] >= p;
          o  = (int32_t)FT[x-1] >= p;
          so = (int32_t)FT[x-1+rs] >= p;
          s  = (int32_t)FT[x+rs] >= p;
          se = (int32_t)FT[x+rs+1] >= p;
          b1 = !so && !s && !se && n;
          b2 = !no && !o && !so && e;
          b3 = !so && !s && !se && !e && !ne && no;
          b4 = !no && !o && !so && !s && !se && ne;
          if (b1 || b2 || b3 || b4)
          { 
            F[x] = alpha8m(FT, x, rs, N); 
            stabilite = 0; 
          }
          else F[x] = FT[x];
        } /* if (!bord(x, rs, N)) */
      } /* for (x = 0; x < N; x++) */
#ifdef WRITEINTER
sprintf(buf,"_%4d",nbiter);
{ int32_t i; for (i = 1; i <= 4; i++) if (buf[i] == ' ') buf[i] = '0'; }
writeimage(img,buf);
#endif
      FTT = FT; FT = F; F = FTT;

    /* detection et abaissement des pics 8-connexes (donc, minima 4-connexes) */
    for (x = 0; x < N; x++) if (peak4(FT, x, rs, N)) FT[x] = alpha8m(FT, x, rs, N);

    nbiter--;
  } /* while ((nbiter > 0) && (!stabilite)) */

#ifdef DEBUG
printf("fin boucle\n");
#endif

  FTT = UCHARDATA(img);
  for (x = 0; x < N; x++) FTT[x] = FT[x];
  free(imgt);
  return(1);
}
