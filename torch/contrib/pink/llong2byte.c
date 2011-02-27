/* $Id: long2byte.c,v 1.2 2009/01/06 13:18:06 mcouprie Exp $ */

/*! \file long2byte.c

\brief converts a "int32_t" image to a "byte" image

<B>Usage:</B> long2byte in.pgm [mode] [n] out.pgm

<B>Description:</B>

Depending on the value given for the (optional) parameter <B>mode</B>:
\li   <B>mode</B> = 0 (default) : for all x, out[x] = min(255, in[x]).
\li   <B>mode</B> = 1 : for all x, out[x] = in[x] modulo 256.
\li   <B>mode</B> = 2 : scales values in the range 0-255.
\li   <B>mode</B> = 3 : sorts the values by decreasing number of occurence in the image.
Replaces the values by their order.
Only the <B>n</B> (default 255) first values are kept.
Useful for label images.
\li   <B>mode</B> = 4 : truncation of the square root in the range 0-255.
\li   <B>mode</B> = 5 : truncation of the log in the range 0-255.

<B>Types supported:</B> int32_t 2d, int32_t 3d

<B>Category:</B> convert
\ingroup  convert

\author Michel Couprie
*/

/*
   Michel Couprie - mai 1998

   Modif : decembre 1999 - mode 3 (trunchisto)
 */

#include <pink.h>

// ERROR N is not supposed to be used as a define macro
#undef N


/* =============================================================== */
int32_t long2byte_Partitionner(int32_t *A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements q tq T[A[q]] <= T[A[p]] et les autres.
*/
{
  int32_t t;
  int32_t x = T[A[p]];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (T[A[j]] > x);
    do i++; while (T[A[i]] < x);
    if (i < j) { t = A[i]; A[i] = A[j]; A[j] = t; }
    else return j;
  } /* while (1) */
}; /* Partitionner() */

/* =============================================================== */
int32_t long2byte_PartitionStochastique(int32_t *A, int32_t *T, int32_t p, int32_t r)
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
  return long2byte_Partitionner(A, T, p, r);
}; /* PartitionStochastique() */

/* =============================================================== */
void long2byte_TriRapideStochastique (int32_t * A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris)
  par ordre croissant
*/
{
  int32_t q;
  if (p < r)
  {
    q = long2byte_PartitionStochastique(A, T, p, r);
    long2byte_TriRapideStochastique (A, T, p, q) ;
    long2byte_TriRapideStochastique (A, T, q+1, r) ;
  }
}; /* TriRapideStochastique() */

/* =============================================================== */
struct xvimage * long2byte( struct xvimage * imagelong, int mode, int nbnewval )
/* =============================================================== */
{
//  struct xvimage * imagelong;
  struct xvimage * imagebyte;
  uint32_t *L;
  uint8_t *B;
  uint32_t x, i;
//  int32_t mode = 0;
  uint32_t Max;
  uint32_t *histo;
  uint32_t *newvals;
  int32_t nbval;//, nbnewval;
  uint32_t *index;
  double t;

  int32_t rs, cs, d, N;

  rs = rowsize(imagelong);
  cs = colsize(imagelong);
  d = depth(imagelong);
  N = rs * cs * d;
  L = ULONGDATA(imagelong);
  
  imagebyte = allocimage(imagelong->name, rs, cs, d, VFF_TYP_1_BYTE);

  if (imagebyte == NULL)
  {
    fprintf(stderr, "allocimage failed\n");
    exit(1);
  }

  B = UCHARDATA(imagebyte);
  imagebyte->xdim = imagelong->xdim;
  imagebyte->ydim = imagelong->ydim;
  imagebyte->zdim = imagelong->zdim;

  switch(mode)
  {
    case 0:
      for (x = 0; x < N; x++)
        B[x] = (uint8_t)mcmin(L[x],255);
      break;
    case 1:
      for (x = 0; x < N; x++)
        B[x] = (uint8_t)(L[x] % 256);
      break;
    case 2:
      Max = L[0];
      for (x = 0; x < N; x++) if (L[x] > Max) Max = L[x];
      if (Max > 255)
        for (x = 0; x < N; x++)
          B[x] = (uint8_t)(L[x] * 255 / Max);
      else
        for (x = 0; x < N; x++)
          B[x] = (uint8_t)L[x];
      break;
    case 3:
      if (! lhistolong(imagelong, NULL, &histo, &nbval))
      {
        fprintf(stderr, "function lhistolong failed\n");
        exit(1);
      }
      index = (uint32_t *)calloc(1,nbval * sizeof(int32_t));
      if (index == NULL)
      {
        fprintf(stderr, "malloc failed\n");
        exit(1);
      }
      for (i = 0; i < nbval; i++) index[i] = i;
      long2byte_TriRapideStochastique ((int32_t *)index, (int32_t *)histo, 1, nbval-1);
      newvals = histo; /* reutilisation de la place memoire allouee pour histo */
      for (i = 0; i < nbval; i++) newvals[i] = 0;
      for (i = 1; i < mcmin(nbval,nbnewval); i++) newvals[index[mcmax((nbval-nbnewval),0)+i]] = i;
      for (x = 0; x < N; x++)
        B[x] = (uint8_t)(newvals[L[x]]);
      free(histo);
      free(index);
      break;
    case 4:
      for (x = 0; x < N; x++)
      {
        t = sqrt((double)(L[x]));
        if (t > 255) t = 255;
        B[x] = (uint8_t)arrondi(t);
      }
      break;
    case 5:
      for (x = 0; x < N; x++)
      {
        t = log((double)(L[x]));
        if (t > 255) t = 255;
        B[x] = (uint8_t)arrondi(t);
      }
      break;
    default:
      fprintf(stderr, "usage: in1.pgm [mode] [n] out.pgm \n");
      fprintf(stderr, "mode = 0 (trunc) | 1 (modulo 256) | 2 (scale) | 3 (trunchisto)\n");
      fprintf(stderr, "       4 (square root) | 5 (log)\n");
      exit(1);
  } /* switch(mode) */
  return imagebyte;
}; /* long2byte */

