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
/* diZenzo gradient based on Deriche filter: Laurent Najman juin 2004 */
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HP
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <math.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lderiche.h>


/* Result is in ImageR */
int32_t ldiZenzoGradient(struct xvimage *imageR, struct xvimage *imageV, struct xvimage *imageB,
	      double alpha)
{
  int32_t i;
  uint8_t *imaR = UCHARDATA(imageR);
  uint8_t *imaV = UCHARDATA(imageV);
  uint8_t *imaB = UCHARDATA(imageB);
  int32_t rs = imageR->row_size;
  int32_t cs = imageR->col_size;
  int32_t N = rs * cs;
  double *ImRx;    /* image intermediaire gradient x */
  double *ImRy;    /* image intermediaire gradient y */
  double *ImR;    /* image intermediaire */
  double *ImVx;    /* image intermediaire gradient x */
  double *ImVy;    /* image intermediaire gradient y */
  double *ImV;    /* image intermediaire */
  double *ImBx;    /* image intermediaire gradient x */
  double *ImBy;    /* image intermediaire gradient y */
  double *ImB;    /* image intermediaire */
  double *buf1;   /* buffer ligne ou colonne */
  double *buf2;   /* buffer ligne ou colonne */
  double k;       /* constante de normalisation pour le lisseur */
  double kp;      /* constante de normalisation pour le derivateur */
  double kpp;     /* constante de normalisation pour le laplacien */
  double e_a;     /* stocke exp(-alpha) */
  double e_2a;    /* stocke exp(-2alpha) */
  double a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4;
  double p,q,t, g;

  if ((depth(imageR) != 1) || (depth(imageB) != 1) || (depth(imageB) != 1))
  {
    fprintf(stderr, "ldiZenzo: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  if ((rs != imageB->row_size) || (rs != imageV->row_size)
      || (cs != imageB->col_size) || (cs != imageV->col_size))
    {
      fprintf(stderr, "ldiZenzo: Image not of same size\n");
      exit(0);
    }

  ImRx = (double *)calloc(1,N * sizeof(double));
  ImRy = (double *)calloc(1,N * sizeof(double));
  ImR = (double *)calloc(1,N * sizeof(double));
  ImVx = (double *)calloc(1,N * sizeof(double));
  ImVy = (double *)calloc(1,N * sizeof(double));
  ImV = (double *)calloc(1,N * sizeof(double));
  ImBx = (double *)calloc(1,N * sizeof(double));
  ImBy = (double *)calloc(1,N * sizeof(double));
  ImB = (double *)calloc(1,N * sizeof(double));

  buf1 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));
  buf2 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));

  if ((ImRx==NULL) || (ImRx==NULL) || (ImR==NULL) 
      ||(ImVx==NULL) || (ImVx==NULL) || (ImB==NULL) 
      ||(ImBx==NULL) || (ImBx==NULL) || (ImB==NULL) 
      || (buf1==NULL) || (buf2==NULL))
  {   fprintf(stderr,"lderiche() : malloc failed\n");
      return(0);
  }

  for (i = 0; i < N; i++) {
    ImR[i] = (double)imaR[i];
    ImV[i] = (double)imaV[i];
    ImB[i] = (double)imaB[i];
  }

  e_a = exp(- alpha);
  e_2a = exp(- 2.0 * alpha);
  k = 1.0 - e_a;
  k = k * k / (1.0 + (2 * alpha * e_a) - e_2a);
  kp = 1.0 - e_a;
  kp = - kp * kp / e_a;
  kpp = (1.0 - e_2a) / (2 * alpha * e_a);

#ifdef DEBUG
printf("alpha = %g , e_a = %g , e_2a = %g , k = %g\n", alpha, e_a, e_2a, k);
#endif

      /* valeurs de parametres pour filtre lisseur-derivateur */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      a5 = 0.0;
      a6 = kp * e_a;
      a7 = - kp * e_a;
      a8 = 0.0;

      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;

      derichegen(ImR, rs, cs, buf1, buf2, ImRx,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(ImR, rs, cs, buf1, buf2, ImRy,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      derichegen(ImV, rs, cs, buf1, buf2, ImVx,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(ImV, rs, cs, buf1, buf2, ImVy,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      derichegen(ImB, rs, cs, buf1, buf2, ImBx,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(ImB, rs, cs, buf1, buf2, ImBy,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      for (i = 0; i < N; i++)
      {
        p = ImRx[i]*ImRx[i] + ImVx[i]*ImVx[i] + ImBx[i]*ImBx[i];
        q = ImRy[i]*ImRy[i] + ImVy[i]*ImVy[i] + ImBy[i]*ImBy[i];
	t = ImRx[i]*ImRy[i] + ImVx[i]*ImVy[i] + ImBx[i]*ImBy[i];
	
	g = sqrt(0.5*(p+q+sqrt((p+q)*(p+q)-4.0*(p*q-t*t))));
	if (g<256.0)
	  imaR[i] = (uint8_t)g;
	else {
	  fprintf(stderr, "ldiZenzo: Gradient greater than 255 (= %lg)\n", g);
	  imaR[i] = g;
	}
      }

  free(ImRx);
  free(ImRy);
  free(ImR);
  free(ImVx);
  free(ImVy);
  free(ImV);
  free(ImBx);
  free(ImBy);
  free(ImB);
  free(buf1);
  free(buf2);
  return 1;
}

int32_t ldiZenzoDirection(struct xvimage *imageR, struct xvimage *imageV, struct xvimage *imageB,
		      struct xvimage *result,
		      double alpha)
#undef F_NAME
#define F_NAME "ldiZenzoDirection"
{
  int32_t i;
  uint8_t *imaR = UCHARDATA(imageR);
  uint8_t *imaV = UCHARDATA(imageV);
  uint8_t *imaB = UCHARDATA(imageB);
  int32_t rs = imageR->row_size;
  int32_t cs = imageR->col_size;
  int32_t N = rs * cs;
  double *ImRx;    /* image intermediaire gradient x */
  double *ImRy;    /* image intermediaire gradient y */
  double *ImR;    /* image intermediaire */
  double *ImVx;    /* image intermediaire gradient x */
  double *ImVy;    /* image intermediaire gradient y */
  double *ImV;    /* image intermediaire */
  double *ImBx;    /* image intermediaire gradient x */
  double *ImBy;    /* image intermediaire gradient y */
  double *ImB;    /* image intermediaire */
  double *buf1;   /* buffer ligne ou colonne */
  double *buf2;   /* buffer ligne ou colonne */
  double k;       /* constante de normalisation pour le lisseur */
  double kp;      /* constante de normalisation pour le derivateur */
  double kpp;     /* constante de normalisation pour le laplacien */
  double e_a;     /* stocke exp(-alpha) */
  double e_2a;    /* stocke exp(-2alpha) */
  double a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4;
  double p,q,t;
  int32_t *  dir = SLONGDATA(result);


  if ((depth(imageR) != 1) || (depth(imageB) != 1) || (depth(imageB) != 1))
  {
    fprintf(stderr, "ldiZenzo: cette version ne traite pas les images volumiques\n");
    exit(0);
  }
  if ((rs != imageB->row_size) || (rs != imageV->row_size)
      || (cs != imageB->col_size) || (cs != imageV->col_size))
    {
      fprintf(stderr, "%s: Image not of same size\n", F_NAME);
      exit(0);
    }
  if (datatype(result) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  ImRx = (double *)calloc(1,N * sizeof(double));
  ImRy = (double *)calloc(1,N * sizeof(double));
  ImR = (double *)calloc(1,N * sizeof(double));
  ImVx = (double *)calloc(1,N * sizeof(double));
  ImVy = (double *)calloc(1,N * sizeof(double));
  ImV = (double *)calloc(1,N * sizeof(double));
  ImBx = (double *)calloc(1,N * sizeof(double));
  ImBy = (double *)calloc(1,N * sizeof(double));
  ImB = (double *)calloc(1,N * sizeof(double));

  buf1 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));
  buf2 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));

  if ((ImRx==NULL) || (ImRx==NULL) || (ImR==NULL) 
      ||(ImVx==NULL) || (ImVx==NULL) || (ImB==NULL) 
      ||(ImBx==NULL) || (ImBx==NULL) || (ImB==NULL) 
      || (buf1==NULL) || (buf2==NULL))
  {   fprintf(stderr,"lderiche() : malloc failed\n");
      return(0);
  }

  for (i = 0; i < N; i++) {
    ImR[i] = (double)imaR[i];
    ImV[i] = (double)imaV[i];
    ImB[i] = (double)imaB[i];
  }

  e_a = exp(- alpha);
  e_2a = exp(- 2.0 * alpha);
  k = 1.0 - e_a;
  k = k * k / (1.0 + (2 * alpha * e_a) - e_2a);
  kp = 1.0 - e_a;
  kp = - kp * kp / e_a;
  kpp = (1.0 - e_2a) / (2 * alpha * e_a);

#ifdef DEBUG
printf("alpha = %g , e_a = %g , e_2a = %g , k = %g\n", alpha, e_a, e_2a, k);
#endif

      /* valeurs de parametres pour filtre lisseur-derivateur */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      a5 = 0.0;
      a6 = kp * e_a;
      a7 = - kp * e_a;
      a8 = 0.0;

      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;

      derichegen(ImR, rs, cs, buf1, buf2, ImRx,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(ImR, rs, cs, buf1, buf2, ImRy,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      derichegen(ImV, rs, cs, buf1, buf2, ImVx,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(ImV, rs, cs, buf1, buf2, ImVy,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      derichegen(ImB, rs, cs, buf1, buf2, ImBx,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(ImB, rs, cs, buf1, buf2, ImBy,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      for (i = 0; i < N; i++)
      {
        p = ImRx[i]*ImRx[i] + ImVx[i]*ImVx[i] + ImBx[i]*ImBx[i];
        q = ImRy[i]*ImRy[i] + ImVy[i]*ImVy[i] + ImBy[i]*ImBy[i];
	t = ImRx[i]*ImRy[i] + ImVx[i]*ImVy[i] + ImBx[i]*ImBy[i];
	
	if (p!=q) {
	  dir[i] = (int32_t) (180.0/3.1415927*(.5*atan2(2.0*t, (p-q)))+180.);
	  //printf("%d\n", dir[i]);
	}
	else dir[i] = 361;
      }

  free(ImRx);
  free(ImRy);
  free(ImR);
  free(ImVx);
  free(ImVy);
  free(ImV);
  free(ImBx);
  free(ImBy);
  free(ImB);
  free(buf1);
  free(buf2);
  return 1;
}
