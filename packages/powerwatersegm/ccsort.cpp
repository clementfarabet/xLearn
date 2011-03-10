/*Copyright ESIEE (2009) 

Author :
Camille Couprie (c.couprie@esiee.fr)

Contributors : 
Hugues Talbot (h.talbot@esiee.fr)
Leo Grady (leo.grady@siemens.com)
Laurent Najman (l.najman@esiee.fr)

This software contains some image processing algorithms whose purpose is to be
used primarily for research.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
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

#include<stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <mclifo.h>
#include <ccsort.h>

/* =============================================================== */
void BucketSort (uint32_t * A, /* array to sort */
		 int *I,       /* array such as I[i]=i*/
		 int32_t r,    /* number of element to sort */
		 int N)        /* N : nb of buckets */
/* =============================================================== */
/*  Sort the r values of array A by descending order */
{
  int i;
  int *H=(int *)calloc(N, sizeof(int));
  for (i=0;i<r;i++) 
    H[A[i]]++;
    
  Lifo ** Bucket =(Lifo **)malloc(N*sizeof(Lifo*)) ;
  
  for (i=0;i<N;i++)
    Bucket[i] = CreeLifoVide(H[i]);
  
  for (i=0;i<r;i++) 
    LifoPush(Bucket[A[i]], i);
  
  int j=0; 
  for (i=N-1;i>=0;i--) 
    {
      while(!LifoVide(Bucket[i]))
	{
	  I[j] = LifoPop(Bucket[i]);
	  A[j]= i; 
	  j++; 
	}
    }
  free(H);
  for (i=0;i<N;i++) LifoTermine(Bucket[i]);
  free(Bucket);
} 


/* =============================================================== */
void BucketSortCroiss (uint32_t * A, /* array to sort */
		       int *I,       /* array such as I[i]=i*/
		       int32_t r,    /* number of element to sort */
		       int N)        /* N : nb of buckets */
/* =============================================================== */
/* Sort the r values of array A by ascending order */
{
  int i;
  int *H=(int *)calloc(N, sizeof(int));
  for (i=0;i<r;i++) 
    H[A[i]]++;
    
  Lifo ** Bucket =(Lifo **)malloc(N*sizeof(Lifo*)) ;
  for (i=0;i<N;i++)
    Bucket[i] = CreeLifoVide(H[i]);
    
  for (i=0;i<r;i++) 
    LifoPush(Bucket[A[i]], i);
    
  int j=0; 
  for (i=0;i<N;i++) 
    {
      while(!LifoVide(Bucket[i]))
	{
	  I[j] = LifoPop(Bucket[i]);
	  A[j]= i;  
	  j++; 
	}
    }
  free(H);
  for (i=0;i<N;i++) LifoTermine(Bucket[i]);
  free(Bucket);
} 



/* =============================================================== */
int32_t Partitionner(int *A, int * I, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[p] et les autres.
*/
{
  int  t;
  int t1;
  int x = A[p];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (A[j] > x);
    do i++; while (A[i] < x);
    if (i < j) 
      { 
	t = A[i];
	A[i] = A[j];
	A[j] = t; 
	t1 = I[i];
	I[i] = I[j];
	I[j] = t1; 
      }
    else return j;
  } /* while (1) */   
} /* Partitionner() */

/* =============================================================== */
int32_t PartitionStochastique (int *A, int * I, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[q] et les autres, avec q tire au hasard dans [p,r].
*/
{
  int t;
  int t1;
  int32_t q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  
  t1 = I[p];         /* echange I[p] et I[q] */
  I[p] = I[q]; 
  I[q] = t1;

  return Partitionner(A, I, p, r);
} /* PartitionStochastique() */



/* =============================================================== */
void TriRapideStochastique (int * A, int *I, int32_t p, int32_t r)
/* =============================================================== */
/* 
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris) 
  par ordre croissant 
*/
{
  int32_t q; 
  if (p < r)
  {
    q = PartitionStochastique(A, I, p, r);
    TriRapideStochastique (A, I, p, q) ;
    TriRapideStochastique (A, I, q+1, r) ;
  }
} /* TriRapideStochastique() */




/* =============================================================== */
int32_t Partitionner_dec(uint32_t *A, int * I, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[p] et les autres.
*/
{
  uint32_t  t;
  int t1;
  uint32_t x = A[p];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (A[j] < x);
    do i++; while (A[i] > x);
    if (i < j) 
      { 
	t = A[i];
	A[i] = A[j];
	A[j] = t; 
	t1 = I[i];
	I[i] = I[j];
	I[j] = t1; 
      }
    else return j;
  } /* while (1) */   
} /* Partitionner() */

/* =============================================================== */
int32_t PartitionStochastique_dec (uint32_t *A, int * I, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[q] et les autres, avec q tire au hasard dans [p,r].
*/
{
  int16_t t;
  int t1;
  int32_t q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  
  t1 = I[p];         /* echange I[p] et I[q] */
  I[p] = I[q]; 
  I[q] = t1;

  return Partitionner_dec(A, I, p, r);
} /* PartitionStochastique() */


/* =============================================================== */
void TriRapideStochastique_dec (uint32_t * A, int *I, int32_t p, int32_t r)
/* =============================================================== */
/* 
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris) 
  par ordre decroissant 
*/
{
  int32_t q; 
  if (p < r)
  {
    q = PartitionStochastique_dec(A, I, p, r);
    TriRapideStochastique_dec (A, I, p, q) ;
    TriRapideStochastique_dec (A, I, q+1, r) ;
  }
} /* TriRapideStochastique() */





/* =============================================================== */
int32_t Partitionner_inc(uint32_t *A, int * I, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[p] et les autres.
*/
{
  uint32_t  t;
  int t1;
  uint32_t x = A[p];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (A[j] > x);
    do i++; while (A[i] < x);
    if (i < j) 
      { 
	t = A[i];
	A[i] = A[j];
	A[j] = t; 
	t1 = I[i];
	I[i] = I[j];
	I[j] = t1; 
      }
    else return j;
  } /* while (1) */   
} 

/* =============================================================== */
int32_t PartitionStochastique_inc (uint32_t *A, int * I, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[q] et les autres, avec q tire au hasard dans [p,r].
*/
{
  uint32_t t;
  int t1;
  int32_t q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  
  t1 = I[p];         /* echange I[p] et I[q] */
  I[p] = I[q]; 
  I[q] = t1;

  return Partitionner_inc(A, I, p, r);
} 



/* =============================================================== */
void TriRapideStochastique_inc (uint32_t * A, int *I, int32_t p, int32_t r)
/* =============================================================== */
/* 
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris) 
  par ordre croissant 
*/
{
  int32_t q; 
  if (p < r)
  {
    q = PartitionStochastique_inc(A, I, p, r);
    TriRapideStochastique_inc (A, I, p, q) ;
    TriRapideStochastique_inc (A, I, q+1, r) ;
  }
} /* TriRapideStochastique() */


