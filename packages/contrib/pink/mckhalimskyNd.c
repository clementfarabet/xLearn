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
   Librairie mckhalimskyNd

   Michel Couprie  --  novembre 1998

   Les cliques de la grille de khamisnski sont codees par les points
   dont toutes les coordonnees sont impaires, les atomes (singletons)
   par les points dont toutes les coordonnees sont paires.

   On considere un objet N-dimensionel S contenu dans un hyper-pave
   de cotes c0 x c1 x ... x cN-1 .

   La dimension N et les cotes du pave (vecteur C a N elements)
   sont en global pour etre efficace et concis.

   On stocke egalement en global le vecteur D qui contient les offsets:
     d0 = c0
     dn = cn * dn-1 pour n de 1 a N-1
   Notons que dN-1 est la taille totale de l'hyperpave. 
   Par exemple, pour N = 3:
     c0 = rowsize ; c1 = colsize ; c2 = depth
     d0 = rowsize ; d1 = rowsize * colsize = planesize ; d2 = planesize * depth

   Un point x de S peut etre designe de deux facons:
     - par vecteur X de coordonnees x0, x1, ... xN-1 (note X)
     - par l'entier x0 + x1 * d0 + x2 * d1 + ... + xN-1 * dN-2 (note x)

                                                             N
   La taille maxi. du voisinage d'un point de la grille est 3

                                                 32
   Limitation de l'implementation : N <= log  ( 2   ) 
                                            3

   On utilise un tableau de mots de 8 bits pour coder l'objet et differentes
   etiquettes.
   Le bit 0 (poids faible) du mot represente l'appartenance a l'objet.

   Notations et conventions :      
     preceder = etre inclus dans
     H = grille de khalimsky
     Hm = ensemble des m-cubes de H
     soit x un element du poset X appartenant a Hm
     alphamoins(x) = { y de Hm-1 / y predecesseur de x }
     alphaplus(x) = { y de Hm+1 / y successeur de x }
     betamoins(x) = { y de H / y predecesseur (strict) de x }
     betaplus(x) = { y de H / y successeur (strict) de x }
     gammamoins(x) = { y de S / y predecesseur maximal de x }
     gammaplus(x) = { y de S / y successeur minimal de x }

*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <mcutil.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mckhalimskyNd.h>

/*
static int32_t debug=0;
#define DEBUGCS
#define DEBUGSS
#define DEBUGCONNEXE
#define DEBUGNS
#define DEBUGMGP
#define DEBUGC8
#define DEBUGELL

#define DEBUGBETAMOINSBIT
#define DEBUGBETAPLUSBIT
#define DEBUGBP
#define DEBUGMBM
#define DEBUGA
#define DEBUGF
#define DEBUGP0CA
#define DEBUGNP
#define DEBUGECS
#define DEBUGES
#define DEBUGCD
#define DEBUGCA
*/

#define VAL_NULLE    0

#define BIT_OBJET    0
#define BIT_GAMMA    1
#define BIT_NONGAMMA 2
#define BIT_CONNEX   3
#define BIT_MARQUE   4

#define Set(x,BIT)   x|=(1<<BIT)
#define UnSet(x,BIT) x&=~(1<<BIT)
#define UnSetAll(x)    x=0
#define IsSet(x,BIT) (x&(1<<BIT))
#define IsSetAny(x)    (x)

#define DIGIT3(x,i) (((i)==0)?((x)%3):(((x)%D3[i])/D3[i-1]))
                            /* retourne le ieme digit (en base 3) de x */

static int32_t *tL; /* liste temporaire en global pour utilisation dans des fonctions "terminales" : */
/* void InitNd(int32_t N, int32_t *userC)
   void TermineNd()              
   int32_t NbPredNd(Khalimsky *K, int32_t x)
   int32_t NbSuccNd(Khalimsky *K, int32_t x)
   int32_t EstCliqueNd(Khalimsky *K, int32_t x)
   int32_t EstAtomeNd(Khalimsky *K, int32_t x) 
   void CopieAncetresNd(Khalimsky *Kc, Khalimsky *K, int32_t x)
   void Connex4ObjNd(Khalimsky *K)    
   int32_t CourbeSimpleNd(Khalimsky *K)   
   int32_t SurfaceSimpleNd(Khalimsky *K, Khalimsky *Kp)   
*/

static int32_t *D3;           /* liste des puissances de 3 a partir de 3 */

/* ========================================================================== */
/* FONCTIONS DIVERSES */
/* ========================================================================== */

/* ========================================== */
void erreur(char * mess)
/* ========================================== */
{
  fprintf(stderr, "%s\n", mess);
  exit(0);
} /* erreur() */

/* ========================================== */
void erreur1(char * mess, uint32_t i)
/* ========================================== */
{
  fprintf(stderr, "%s%x\n", mess, i);
  exit(0);
} /* erreur1() */

/* ========================================================================== */
/* FONCTIONS POUR LA PILE DE POINTS */
/* ========================================================================== */

static int32_t **PileNd;        /* pile de points (forme vecteur) temporaires */
static int32_t IndexPileNd;     /* index pour la pile PileNd */
static int32_t TaillePileNd;    /* capacite de la pile PileNd */

/* ========================================== */
void InitPileNd(int32_t taillepile, int32_t N)
/* ========================================== */
{
  int32_t i;
  PileNd = (int32_t **)calloc(1,taillepile * sizeof(int32_t *));
  if (PileNd == NULL) erreur((char *)"InitPileNd: malloc failed ");
  for (i = 0; i < taillepile; i++)
  {
    PileNd[i] = (int32_t *)calloc(1,N * sizeof(int32_t));
    if (PileNd[i] == NULL) erreur((char *)"InitPileNd: malloc failed ");
  }
  IndexPileNd = 0;
  TaillePileNd = taillepile;
} /* InitPileNd() */

/* ========================================== */
void TerminePileNd()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < TaillePileNd; i++) free(PileNd[i]);
  free(PileNd);
} /* TerminePileNd() */

/* ========================================== */
int32_t * AllocNd()
/* ========================================== */
{
  int32_t * X;
  if (IndexPileNd >= TaillePileNd) erreur((char *)"AllocNd: pile pleine ");
  X = PileNd[IndexPileNd];
  IndexPileNd++;
#ifdef DEBUGA
printf("AllocNd: %x\n", (int32_t)X);
#endif
  return X;
} /* AllocNd() */

/* ========================================== */
void FreeNd(int32_t *X)
/* ========================================== */
{
  if (IndexPileNd <= 0) erreur((char *)"FreeNd: pile vide");
#ifdef DEBUGF
printf("FreeNd: haut pile = %x, arg = %x\n", (int32_t)PileNd[IndexPileNd], (int32_t)X);
#endif
  if (PileNd[IndexPileNd-1] != X) erreur((char *)"FreeNd: mauvais ordonnancement");
  IndexPileNd --;
} /* FreeNd() */

/* ========================================================================== */
/* FONCTIONS POUR LA PILE DE LISTES DE VOISINAGE */
/* ========================================================================== */

static int32_t **PileVoisListNd;        /* pile de listes de voisinage temporaires */
static int32_t IndexPileVoisListNd;     /* index pour la pile PileVoisListNd */
static int32_t TaillePileVoisListNd;    /* capacite de la pile PileVoisListNd */

/* ========================================== */
void InitPileVoisListNd(int32_t taillepileVoisList, int32_t tailleList)
/* ========================================== */
{
  int32_t i;
  PileVoisListNd = (int32_t **)calloc(1,taillepileVoisList * sizeof(int32_t *));
  if (PileVoisListNd == NULL) erreur((char *)"InitPileVoisListNd: malloc failed ");
  for (i = 0; i < taillepileVoisList; i++)
  {
    PileVoisListNd[i] = (int32_t *)calloc(1,tailleList * sizeof(int32_t));
    if (PileVoisListNd[i] == NULL) erreur((char *)"InitPileVoisListNd: malloc failed ");
  }
  IndexPileVoisListNd = 0;
  TaillePileVoisListNd = taillepileVoisList;
} /* InitPileVoisListNd() */

/* ========================================== */
void TerminePileVoisListNd()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < TaillePileVoisListNd; i++) free(PileVoisListNd[i]);
  free(PileVoisListNd);
} /* TerminePileVoisListNd() */

/* ========================================== */
int32_t * AllocVoisListNd()
/* ========================================== */
{
  int32_t * X;
  if (IndexPileVoisListNd >= TaillePileVoisListNd) erreur((char *)"AllocVoisListNd: pileVoisList pleine ");
  X = PileVoisListNd[IndexPileVoisListNd];
  IndexPileVoisListNd++;
#ifdef DEBUGA
printf("AllocVoisListNd: %x\n", (int32_t)X);
#endif
  return X;
} /* AllocVoisListNd() */

/* ========================================== */
void FreeVoisListNd(int32_t *X)
/* ========================================== */
{
  if (IndexPileVoisListNd <= 0) erreur((char *)"FreeVoisListNd: pileVoisList vide");
#ifdef DEBUGF
printf("FreeVoisListNd: haut pileVoisList = %x, arg = %x\n", 
        (int32_t)PileVoisListNd[IndexPileVoisListNd], (int32_t)X);
#endif
  if (X && (PileVoisListNd[IndexPileVoisListNd-1] != X)) 
    erreur((char *)"FreeVoisListNd: mauvais ordonnancement");
  IndexPileVoisListNd --;
} /* FreeVoisListNd() */

/* ========================================================================== */
/* FONCTIONS POUR LA PILE DE GRILLES */
/* ========================================================================== */

static Khalimsky ** PileGrillesNd;
static int32_t IndexPileGrillesNd;
static int32_t TaillePileGrillesNd;

/* ========================================== */
Khalimsky * AllocKhalimskyNd(int32_t dim, int32_t *userC)
/* ========================================== */
{
  int32_t i;
  Khalimsky *K;

  K = (Khalimsky *)calloc(1,sizeof(Khalimsky));
  if (K == NULL) erreur((char *)"AllocKhalimsky: malloc failed");
  K->N = dim;
  K->C = (int32_t *)calloc(1,dim * sizeof(int32_t));
  K->D = (int32_t *)calloc(1,dim * sizeof(int32_t));
  if ((K->C == NULL) || (K->D == NULL)) erreur((char *)"AllocKhalimsky: malloc failed");
  for (i = 0; i < dim; i++) K->C[i] = userC[i];
  K->D[0] = K->C[0];
  for (i = 1; i < dim; i++) K->D[i]  = K->C[i] * K->D[i-1];
  K->S = (uint8_t *)calloc(1,K->D[dim - 1] * sizeof(char));
  return K;
} /* AllocKhalimskyNd() */

/* ========================================== */
void FreeKhalimskyNd(Khalimsky * K)
/* ========================================== */
{
  free(K->C); free(K->D); free(K->S); free(K);
} /* FreeKhalimskyNd() */

/* ========================================== */
void InitPileGrillesNd(int32_t taillepile, int32_t dim, int32_t *userC)
/* ========================================== */
{
  int32_t i;
  PileGrillesNd = (Khalimsky **)calloc(1,taillepile * sizeof(Khalimsky *));
  if (PileGrillesNd == NULL) erreur((char *)"InitPileGrillesNd: malloc failed ");
  for (i = 0; i < taillepile; i++)
    PileGrillesNd[i] = AllocKhalimskyNd(dim, userC);
  IndexPileGrillesNd = 0;
  TaillePileGrillesNd = taillepile;
} /* InitPileGrillesNd() */

/* ========================================== */
void TerminePileGrillesNd()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < TaillePileGrillesNd; i++) FreeKhalimskyNd(PileGrillesNd[i]);
  free(PileGrillesNd);
} /* TerminePileGrillesNd() */

/* ========================================== */
Khalimsky * AllocGrilleNd()
/* ========================================== */
{
  Khalimsky * G;
  if (IndexPileGrillesNd >= TaillePileGrillesNd) erreur((char *)"AllocGrilleNd: pile pleine ");
  G = PileGrillesNd[IndexPileGrillesNd];
  IndexPileGrillesNd++;
  return G;
} /* AllocGrilleNd() */

/* ========================================== */
void FreeGrilleNd(Khalimsky *G)
/* ========================================== */
{
  if (IndexPileGrillesNd <= 0) erreur((char *)"FreeNd: pile vide");
  if (PileGrillesNd[IndexPileGrillesNd-1] != G) erreur((char *)"FreeGrilleNd: mauvais ordonnancement");
  IndexPileGrillesNd --;
} /* FreeGrilleNd() */

/* ========================================================================== */
/* FONCTIONS POUR LE ND */
/* ========================================================================== */

/* ==================================== */
void InitNd(int32_t N, int32_t *userC)                 /* ! : utilise tL */
/* ==================================== */
{
  int32_t i;
  D3 = (int32_t *)calloc(1,N * sizeof(int32_t));
  D3[0] = 3;
  for (i = 1; i < N; i++) D3[i] = D3[i-1] * 3;
  tL = (int32_t *)calloc(1,D3[N-1] * sizeof(int32_t));
  if (tL == NULL) erreur((char *)"InitNd: malloc failed");
  InitPileNd(4*N, N);
  InitPileVoisListNd(32*N, D3[N-1]);
  InitPileGrillesNd(2*N, N, userC);
} /* initNd() */

/* ==================================== */
void TermineNd()                 /* ! : utilise tL */
/* ==================================== */
{
  TerminePileGrillesNd();
  TerminePileVoisListNd();
  TerminePileNd();
  free(tL);
} /* termineNd() */

/* ==================================== */
int32_t encodeNd(Khalimsky *K, int32_t *X)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D;
  int32_t i, x = X[0];
  for (i = 1; i < N; i++) x += X[i] * D[i-1];
  return x;
} /* encodeNd() */

/* ==================================== */
void decodeNd(Khalimsky *K, int32_t x, int32_t *X)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D;
  int32_t i;
  X[0] = x % D[0];
  for (i = 1; i < N; i++) X[i] = (x % D[i]) / D[i-1]; 
} /* encodeNd() */

/* ==================================== */
void alphamoinsNd(Khalimsky *K, int32_t x, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere) , predecesseurs de x dans la grille
*/
{
  int32_t N = K->N; int32_t *C = K->C;
  int32_t i, j, n = 0;
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
  decodeNd(K, x, tX);
  for (i = 0; i < N; i++) 
    if (mcodd(tX[i]))
    {
      for (j = 0; j < i; j++) tY[j] = tX[j];
      for (j = i+1; j < N; j++) tY[j] = tX[j];
      tY[i] = tX[i] - 1; if ((tY[i]>= 0) && (tY[i]<C[i])) L[n++] = encodeNd(K, tY);
      tY[i] = tX[i] + 1; if ((tY[i]>= 0) && (tY[i]<C[i])) L[n++] = encodeNd(K, tY);
    }
  *nL = n;
  FreeNd(tY);
  FreeNd(tX);
} /* alphamoinsNd() */

/* ==================================== */
void alphaplusNd(Khalimsky *K, int32_t x, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere) , succecesseurs de x dans la grille
*/
{
  int32_t N = K->N; int32_t *C = K->C;
  int32_t i, j, n = 0;
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
  decodeNd(K, x, tX);
  for (i = 0; i < N; i++) 
    if (mceven(tX[i]))
    {
      for (j = 0; j < i; j++) tY[j] = tX[j];
      for (j = i+1; j < N; j++) tY[j] = tX[j];
      tY[i] = tX[i] - 1; if ((tY[i]>= 0) && (tY[i]<C[i])) L[n++] = encodeNd(K, tY);
      tY[i] = tX[i] + 1; if ((tY[i]>= 0) && (tY[i]<C[i])) L[n++] = encodeNd(K, tY);
    }
  *nL = n;
  FreeNd(tY);
  FreeNd(tX);
} /* alphaplusNd() */

/* ==================================== */
void betamoinsbitNd(Khalimsky *K, int32_t x, int32_t bit, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere), 
          ancetres (stricts) de x dans la grille
          et dont le bit "bit" est positionne
*/
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t i, j, n, u, d, y;
  int32_t ci;                /* compte le nombre de coord. impaires */
  int32_t v;                 /* nombre de points dans betamoins(x) U {x} */
  uint32_t b, mask; /* pour le masque binaire des coordonnees impaires */
  uint32_t moins1;  /* masque des coord. a diminuer de 1 */
  uint32_t plus1;   /* masque des coord. a augmenter de 1 */
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
#ifdef DEBUGBETAMOINSBIT
printf("BetaMoinsBit\n");
#endif
  decodeNd(K, x, tX);
  n = 0; 
  mask = 0; ci = 0;
  for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    if (mcodd(tX[i])) { mask |= b; ci++; }
  if (ci > 0) v = D3[ci-1]; else v = 0;
  for (u = 1; u < v; u++) /* u = 0 correspond a x lui-meme */
  {
    moins1 = 0; plus1 = 0;
    for (i = 0, j = 0, b = 1;     i < N;     i++, b = b << 1)
      if (mask & b)        /* i correspond a la jeme coord. impaire */
      {
        d = DIGIT3(u,j);
        j++;
        if (d == 1) moins1 |= b; else
        if (d == 2) plus1  |= b;
      }
    for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    {
      if (moins1 & b) tY[i] = tX[i] - 1; else
      if (plus1  & b) tY[i] = tX[i] + 1; else
                      tY[i] = tX[i];
      if ((tY[i]< 0) || (tY[i]>=C[i])) goto skip;   
    }
    y = encodeNd(K, tY);
    if (IsSet(S[y],bit)) L[n++] = y;
skip:;
  }
  *nL = n;
  FreeNd(tY);
  FreeNd(tX);
} /* betamoinsbitNd() */

/* ==================================== */
void betamoinsNd(Khalimsky *K, int32_t x, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere), 
          ancetres (stricts) de x dans la grille
*/
{
  int32_t N = K->N; int32_t *C = K->C;
  int32_t i, j, n, u, d, y;
  int32_t ci;                /* compte le nombre de coord. impaires */
  int32_t v;                 /* nombre de points dans betamoins(x) U {x} */
  uint32_t b, mask; /* pour le masque binaire des coordonnees impaires */
  uint32_t moins1;  /* masque des coord. a diminuer de 1 */
  uint32_t plus1;   /* masque des coord. a augmenter de 1 */
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
  decodeNd(K, x, tX);
  n = 0; 
  mask = 0; ci = 0;
  for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    if (mcodd(tX[i])) { mask |= b; ci++; }
  if (ci > 0) v = D3[ci-1]; else v = 0;
  for (u = 1; u < v; u++) /* u = 0 correspond a x lui-meme */
  {
    moins1 = 0; plus1 = 0;
    for (i = 0, j = 0, b = 1;     i < N;     i++, b = b << 1)
      if (mask & b)        /* i correspond a la jeme coord. impaire */
      {
        d = DIGIT3(u,j);
        j++;
        if (d == 1) moins1 |= b; else
        if (d == 2) plus1  |= b;
      }
    for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    {
      if (moins1 & b) tY[i] = tX[i] - 1; else
      if (plus1  & b) tY[i] = tX[i] + 1; else
                      tY[i] = tX[i];
      if ((tY[i]< 0) || (tY[i]>=C[i])) goto skip;   
    }
    y = encodeNd(K, tY);
    L[n++] = y;
skip:;
  }
  *nL = n;
  FreeNd(tY);
  FreeNd(tX);
} /* betamoinsNd() */

/* ==================================== */
void marquebetamoinsNd(Khalimsky *K, int32_t x, int32_t bit, int32_t val)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  marque dans S, par la valeur "val" au bit "bit", les points de betamoins(x)
*/
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t i, j, n, u, d;
  int32_t ci;                /* compte le nombre de coord. impaires */
  int32_t v;                 /* nombre de points dans betamoins(x) U {x} */
  uint32_t b, mask; /* pour le masque binaire des coordonnees impaires */
  uint32_t moins1;  /* masque des coord. a diminuer de 1 */
  uint32_t plus1;   /* masque des coord. a augmenter de 1 */
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
#ifdef DEBUGMBM
printf("marquebetamoinsNd: x=%d\n", x);
#endif
  decodeNd(K, x, tX);
  n = 0;
  mask = 0; ci = 0;
  for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    if (mcodd(tX[i])) { mask |= b; ci++; }
  if (ci > 0) v = D3[ci-1]; else v = 0;
#ifdef DEBUGMBM
printf("mask = %x ; ci = %d ; v = %d\n", mask, ci, v);
#endif
  for (u = 1; u < v; u++) /* u = 0 correspond a x lui-meme */
  {
    moins1 = 0; plus1 = 0;
    for (i = 0, j = 0, b = 1;     i < N;     i++, b = b << 1)
      if (mask & b)        /* i correspond a la jeme coord. impaire */
      {
        d = DIGIT3(u,j);
        j++;
        if (d == 1) moins1 |= b; else
        if (d == 2) plus1  |= b;
      }
    for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    {
      if (moins1 & b) tY[i] = tX[i] - 1; else
      if (plus1  & b) tY[i] = tX[i] + 1; else
                      tY[i] = tX[i];
      if ((tY[i]< 0) || (tY[i]>=C[i])) goto skip;   
    }
    if (val) Set(S[encodeNd(K, tY)],bit); else UnSet(S[encodeNd(K, tY)],bit);
skip:;
  }
  FreeNd(tY);
  FreeNd(tX);
} /* marquebetamoinsNd() */

/* ==================================== */
void betaplusbitNd(Khalimsky *K, int32_t x, int32_t bit, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere) , 
          descendants (stricts) de x dans la grille
          et dont le bit "bit" est positionne
*/
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t i, j, n, u, d, y;
  int32_t ci;                /* compte le nombre de coord. paires */
  int32_t v;                 /* nombre de points dans betaplus(x) U {x} */
  uint32_t b, mask; /* pour le masque binaire des coordonnees paires */
  uint32_t moins1;  /* masque des coord. a diminuer de 1 */
  uint32_t plus1;   /* masque des coord. a augmenter de 1 */
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
#ifdef DEBUGBETAPLUSBIT
printf("BetaPlusBit\n");
#endif
  decodeNd(K, x, tX);
  n = 0; 
  mask = 0; ci = 0;
  for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    if (mceven(tX[i])) { mask |= b; ci++; }
  if (ci > 0) v = D3[ci-1]; else v = 0;
  for (u = 1; u < v; u++) /* u = 0 correspond a x lui-meme */
  {
    moins1 = 0; plus1 = 0;
    for (i = 0, j = 0, b = 1;     i < N;     i++, b = b << 1)
      if (mask & b)        /* i correspond a la jeme coord. paire */
      {
        d = DIGIT3(u,j);
        j++;
        if (d == 1) moins1 |= b; else
        if (d == 2) plus1  |= b;
      }
    for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    {
      if (moins1 & b) tY[i] = tX[i] - 1; else
      if (plus1  & b) tY[i] = tX[i] + 1; else
                      tY[i] = tX[i];
      if ((tY[i]< 0) || (tY[i]>=C[i])) goto skip;   
    }
    y = encodeNd(K, tY);
    if (IsSet(S[y],bit)) 
    {
      L[n++] = y;
    }
skip:;
  } 
  *nL = n;
  FreeNd(tY);
  FreeNd(tX);
} /* betaplusbitNd() */

/* ==================================== */
void betaplusNd(Khalimsky *K, int32_t x, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere) , 
          descendants (stricts) de x dans la grille
*/
{
  int32_t N = K->N; int32_t *C = K->C;
  int32_t i, j, n, u, d, y;
  int32_t ci;                /* compte le nombre de coord. paires */
  int32_t v;                 /* nombre de points dans betaplus(x) U {x} */
  uint32_t b, mask; /* pour le masque binaire des coordonnees paires */
  uint32_t moins1;  /* masque des coord. a diminuer de 1 */
  uint32_t plus1;   /* masque des coord. a augmenter de 1 */
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();

  decodeNd(K, x, tX);
  n = 0; 
  mask = 0; ci = 0;
  for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    if (mceven(tX[i])) { mask |= b; ci++; }
  if (ci > 0) v = D3[ci-1]; else v = 0;
  for (u = 1; u < v; u++) /* u = 0 correspond a x lui-meme */
  {
    moins1 = 0; plus1 = 0;
    for (i = 0, j = 0, b = 1;     i < N;     i++, b = b << 1)
      if (mask & b)        /* i correspond a la jeme coord. paire */
      {
        d = DIGIT3(u,j);
        j++;
        if (d == 1) moins1 |= b; else
        if (d == 2) plus1  |= b;
      }
    for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    {
      if (moins1 & b) tY[i] = tX[i] - 1; else
      if (plus1  & b) tY[i] = tX[i] + 1; else
                      tY[i] = tX[i];
      if ((tY[i]< 0) || (tY[i]>=C[i])) goto skip;   
    }
    y = encodeNd(K, tY);
    L[n++] = y;
skip:;
  } 
  *nL = n;
  FreeNd(tY);
  FreeNd(tX);
} /* betaplusNd() */

/* ==================================== */
void marquebetaplusNd(Khalimsky *K, int32_t x, int32_t bit, int32_t val)
/* ==================================== */
/*
  entree: un point x de la grille sous forme entiere
  marque dans S, par la valeur "val" au bit "bit", les points de betaplus(x)
*/
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t i, j, n, u, d;
  int32_t ci;                /* compte le nombre de coord. paires */
  int32_t v;                 /* nombre de points dans betaplus(x) U {x} */
  uint32_t b, mask; /* pour le masque binaire des coordonnees impaires */
  uint32_t moins1;  /* masque des coord. a diminuer de 1 */
  uint32_t plus1;   /* masque des coord. a augmenter de 1 */
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
  int32_t y;

  decodeNd(K, x, tX);
  n = 0; 
  mask = 0; ci = 0;
  for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    if (mceven(tX[i])) { mask |= b; ci++; }
  if (ci > 0) v = D3[ci-1]; else v = 0;
  for (u = 1; u < v; u++) /* u = 0 correspond a x lui-meme */
  {
    moins1 = 0; plus1 = 0;
    for (i = 0, j = 0, b = 1;     i < N;     i++, b = b << 1)
      if (mask & b)        /* i correspond a la jeme coord. paire */
      {
        d = DIGIT3(u,j);
        j++;
        if (d == 1) moins1 |= b; else
        if (d == 2) plus1  |= b;
      }
    for (i = 0, b = 1;    i < N;    i++, b = b << 1) 
    {
      if (moins1 & b) tY[i] = tX[i] - 1; else
      if (plus1  & b) tY[i] = tX[i] + 1; else
                      tY[i] = tX[i];
      if ((tY[i]< 0) || (tY[i]>=C[i])) goto skip;   
    }
    y = encodeNd(K, tY);
    if (val) 
    {
      Set(S[y],bit); 
    }
    else 
    {
      UnSet(S[y],bit);
    }
skip:;
  }
  FreeNd(tY);
  FreeNd(tX);
} /* marquebetaplusNd() */

/* ==================================== */
void marquegammamoinsNd(Khalimsky *K, int32_t x)
/* ==================================== */
/* methode : parcours par niveaux */
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t i, j, y, u;
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
  int32_t *L1 = AllocVoisListNd(); /* pour le niveau courant */
  int32_t *L2 = AllocVoisListNd(); /* pour le niveau suivant */
  int32_t *Ltmp;                   /* pour l'echange des listes */
  int32_t n1, n2;                  /* tailles des listes L1, L2 */

  decodeNd(K, x, tX);
  n1 = n2 = 0;
  for (i = 0; i < N; i++)      /* init premier niveau: alphamoins(x) */
    if (mcodd(tX[i]))           /* coord. impaire du point x */
    {
      for (j = 0; j < i; j++) tY[j] = tX[j];
      for (j = i+1; j < N; j++) tY[j] = tX[j];
      tY[i] = tX[i] - 1;       
      if ((tY[i]>= 0) && (tY[i]<C[i])) L1[n1++] = encodeNd(K, tY); /* y est dans alphamoins(x) */
      tY[i] = tX[i] + 1; 
      if ((tY[i]>= 0) && (tY[i]<C[i])) L1[n1++] = encodeNd(K, tY); /* y est dans alphamoins(x) */
    } /* if (mcodd(tX[i])) */

  while (n1 > 0)      /* boucle sur les niveaux */
  {
    for (u = 0; u < n1; u++)
    {
      x = L1[u]; 
      decodeNd(K, x, tX); 
      UnSet(S[x],BIT_MARQUE);
      if (IsSet(S[x],BIT_OBJET) && !IsSet(S[x],BIT_NONGAMMA)) Set(S[x],BIT_GAMMA);
      for (i = 0; i < N; i++)
        if (mcodd(tX[i]))
        {
          for (j = 0; j < i; j++) tY[j] = tX[j];
          for (j = i+1; j < N; j++) tY[j] = tX[j];
          tY[i] = tX[i] - 1;       
          if ((tY[i]>= 0) && (tY[i]<C[i])) 
	  {
            y = encodeNd(K, tY);      /* y est dans alphamoins(x) */
            if (IsSet(S[x],BIT_GAMMA) || IsSet(S[x],BIT_NONGAMMA)) Set(S[y],BIT_NONGAMMA);
            if (!IsSet(S[y],BIT_MARQUE)) { L2[n2++] = encodeNd(K, tY); Set(S[y],BIT_MARQUE); }
	  }
          tY[i] = tX[i] + 1; 
          if ((tY[i]>= 0) && (tY[i]<C[i])) 
	  {
            y = encodeNd(K, tY);      /* y est dans alphamoins(x) */
            if (IsSet(S[x],BIT_GAMMA) || IsSet(S[x],BIT_NONGAMMA)) Set(S[y],BIT_NONGAMMA);
            if (!IsSet(S[y],BIT_MARQUE)) { L2[n2++] = encodeNd(K, tY); Set(S[y],BIT_MARQUE); }
	  }
        } /* if (mcodd(tX[i])) */

    } /* for (u = 0; u < n1; u++) */

    Ltmp = L1; L1 = L2; L2 = Ltmp; n1 = n2; n2 = 0; /* echange les listes */
  } /* while n1 */

  FreeNd(tY);
  FreeNd(tX);
  FreeVoisListNd(NULL);
  FreeVoisListNd(NULL);
} /* marquegammamoinsNd() */

/* ==================================== */
void marquegammaplusNd(Khalimsky *K, int32_t x)
/* ==================================== */
/* methode : parcours par niveaux */
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t i, j, y, u;
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
  int32_t *L1 = AllocVoisListNd(); /* pour le niveau courant */
  int32_t *L2 = AllocVoisListNd(); /* pour le niveau suivant */
  int32_t *Ltmp;                   /* pour l'echange des listes */
  int32_t n1, n2;                  /* tailles des listes L1, L2 */

#ifdef DEBUGMGP
printf("marquegammaplus x = %d\n", x);
#endif
  decodeNd(K, x, tX);
  n1 = n2 = 0;
  for (i = 0; i < N; i++)      /* init premier niveau: alphaplus(x) */
    if (mceven(tX[i]))           /* coord. paire du point x */
    {
      for (j = 0; j < i; j++) tY[j] = tX[j];
      for (j = i+1; j < N; j++) tY[j] = tX[j];
      tY[i] = tX[i] - 1;       
      if ((tY[i]>= 0) && (tY[i]<C[i])) L1[n1++] = encodeNd(K, tY); /* y est dans alphaplus(x) */
      tY[i] = tX[i] + 1; 
      if ((tY[i]>= 0) && (tY[i]<C[i])) L1[n1++] = encodeNd(K, tY); /* y est dans alphaplus(x) */
    } /* if (mceven(tX[i])) */

  while (n1 > 0)      /* boucle sur les niveaux */
  {
    for (u = 0; u < n1; u++)
    {
      x = L1[u]; 
      decodeNd(K, x, tX); 
      UnSet(S[x],BIT_MARQUE);
      if (IsSet(S[x],BIT_OBJET) && !IsSet(S[x],BIT_NONGAMMA)) Set(S[x],BIT_GAMMA);
      for (i = 0; i < N; i++)
        if (mceven(tX[i]))
        {
          for (j = 0; j < i; j++) tY[j] = tX[j];
          for (j = i+1; j < N; j++) tY[j] = tX[j];
          tY[i] = tX[i] - 1;       
          if ((tY[i]>= 0) && (tY[i]<C[i])) 
	  {
            y = encodeNd(K, tY);      /* y est dans alphaplus(x) */
            if (IsSet(S[x],BIT_GAMMA) || IsSet(S[x],BIT_NONGAMMA)) Set(S[y],BIT_NONGAMMA);
            if (!IsSet(S[y],BIT_MARQUE)) { L2[n2++] = encodeNd(K, tY); Set(S[y],BIT_MARQUE); }
	  }
          tY[i] = tX[i] + 1; 
          if ((tY[i]>= 0) && (tY[i]<C[i])) 
	  {
            y = encodeNd(K, tY);      /* y est dans alphaplus(x) */
            if (IsSet(S[x],BIT_GAMMA) || IsSet(S[x],BIT_NONGAMMA)) Set(S[y],BIT_NONGAMMA);
            if (!IsSet(S[y],BIT_MARQUE)) { L2[n2++] = encodeNd(K, tY); Set(S[y],BIT_MARQUE); }
	  }
        } /* for i ... if (mceven(tX[i])) */

    } /* for (u = 0; u < n1; u++) */

    Ltmp = L1; L1 = L2; L2 = Ltmp; n1 = n2; n2 = 0; /* echange les listes */
  } /* while n1 */

  FreeNd(tY);
  FreeNd(tX);
  FreeVoisListNd(NULL);
  FreeVoisListNd(NULL);
} /* marquegammaplusNd() */

/* ==================================== */
void gammamoinsNd(Khalimsky *K, int32_t x, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un objet K
  entree: un point x de K sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere) , predecesseurs maximaux de x dans K
*/
{
  if (!IsSet(K->S[x],BIT_OBJET)) erreur1((char *)"gammamoinsNd: point non objet: ", x);
  marquegammamoinsNd(K, x);
  betamoinsbitNd(K, x, BIT_GAMMA, L, nL);
  marquebetamoinsNd(K, x, BIT_GAMMA, 0);
  marquebetamoinsNd(K, x, BIT_NONGAMMA, 0);
} /* gammamoinsNd() */

/* ==================================== */
void gammaplusNd(Khalimsky *K, int32_t x, int32_t *L, int32_t *nL)
/* ==================================== */
/*
  entree: un objet K
  entree: un point x de K sous forme entiere
  sortie: la liste (L, nL) de points (forme entiere) , successeurs minimaux de x dans K
*/
{
  if (!IsSet(K->S[x],BIT_OBJET)) erreur1((char *)"gammaplusNd: point non objet: ", x);
  marquegammaplusNd(K, x);
  betaplusbitNd(K, x, BIT_GAMMA, L, nL);
  marquebetaplusNd(K, x, BIT_GAMMA, 0);
  marquebetaplusNd(K, x, BIT_NONGAMMA, 0);
} /* gammaplusNd() */

/* ==================================== */
int32_t NbPredNd(Khalimsky *K, int32_t x)                 /* ! : utilise tL */
/* ==================================== */
/*
  entree: un objet K
  entree: un point x de K sous forme entiere
  sortie: le nombre de predesseurs maximaux de x dans K
*/
{
  int32_t n;
  if (!IsSet(K->S[x],BIT_OBJET)) erreur1((char *)"NbPredNd: point non objet: ", x);
  marquegammamoinsNd(K, x);
  betamoinsbitNd(K, x, BIT_GAMMA, tL, &n);
  marquebetamoinsNd(K, x, BIT_GAMMA, 0);
  marquebetamoinsNd(K, x, BIT_NONGAMMA, 0);
  return n;
} /* NbPredNd() */

/* ==================================== */
int32_t NbSuccNd(Khalimsky *K, int32_t x)                 /* ! : utilise tL */
/* ==================================== */
/*
  entree: un objet K
  entree: un point x de K sous forme entiere
  sortie: le nombre de successeurs minimaux de x dans K
*/
{
  int32_t n;
#ifdef DEBUGNS
printf("NbSuccNd : x = %d\n", x);
#endif
  if (!IsSet(K->S[x],BIT_OBJET)) erreur1((char *)"NbSuccNd: point non objet: ", x);
  marquegammaplusNd(K, x);
  betaplusbitNd(K, x, BIT_GAMMA, tL, &n);
  marquebetaplusNd(K, x, BIT_GAMMA, 0);
  marquebetaplusNd(K, x, BIT_NONGAMMA, 0);
#ifdef DEBUGNS
printf("NbSuccNd : x = %d, retourne %d\n", x, n);
if (x == 7) debug = 0;
#endif
  return n;
} /* NbSuccNd() */

/* ==================================== */
int32_t EstCliqueNd(Khalimsky *K, int32_t x)                 /* ! : utilise tL */
/* ==================================== */
{
  int32_t n;
  betaplusbitNd(K, x, BIT_OBJET, tL, &n);
  return (n == 0);
} /* EstCliqueNd() */

/* ==================================== */
int32_t EstAtomeNd(Khalimsky *K, int32_t x)                 /* ! : utilise tL */
/* ==================================== */
{
  int32_t n;
  betamoinsbitNd(K, x, BIT_OBJET, tL, &n);
  return (n == 0);
} /* EstAtomeNd() */

/* ==================================== */
int32_t ExactementUneCliqueNd(Khalimsky *K)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  int32_t x, n = 0;
  for (x = 0; x < D[N-1]; x++)
    if (IsSet(S[x],BIT_OBJET) && EstCliqueNd(K, x))
      if (++n > 1) return 0;
  if (n < 1) return 0; else return 1;
} /* ExactementUneCliqueNd() */

/* ==================================== */
void EffaceLiensLibresNd(Khalimsky *K)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  int32_t x, n;
#ifdef DEBUGELL
printf("EffaceLiensLibresNd\n");
#endif
  do 
  {
    n = 0;
    for (x = 0; x < D[N-1]; x++)
      if (IsSet(S[x],BIT_OBJET) && (NbSuccNd(K, x) == 1))
      {
        UnSet(S[x],BIT_OBJET);
        n++;
#ifdef DEBUGELL
printf("EffaceLiensLibresNd : efface %d ; n = %d\n", x, n);
#endif
      }
  } while (n > 0);
} /* EffaceLiensLibresNd() */

/* ==================================== */
int32_t TypeNd(Khalimsky *K, int32_t x)
/* ==================================== */
/* 
  Le type d'un point x de la grille de Khalimsky est son nombre 
  de coordonnees paires. Ex: une clique de la grille a pour type 0. 
*/
{
  int32_t N = K->N;
  int32_t i, n = 0;
  int32_t *tX = AllocNd();
  decodeNd(K, x, tX);
  for (i = 0; i < N; i++) if (mceven(tX[i])) n++;
  FreeNd(tX);
  return n;
} /* TypeNd() */

/* ==================================== */
int32_t BordNd(Khalimsky *K, int32_t x)
/* ==================================== */
/* 
  retourne vrai si x est sur un bord
*/
{
  int32_t N = K->N; int32_t *C = K->C;
  int32_t i, ret;
  int32_t *tX = AllocNd();
  decodeNd(K, x, tX);
  ret = 0;
  for (i = 0; i < N; i++) if ((tX[i]==0)||(tX[i]==C[i]-1)) { ret = 1; break; }
  FreeNd(tX);
  return ret;
} /* BordNd() */

/* ==================================== */
void CopieAncetresNd(Khalimsky *Kc, Khalimsky *K, int32_t x)                 /* ! : utilise tL */
/* ==================================== */
/*
    L'image de x est un point xp de la grille G (3x3x...x3) du meme type que x,
    et dont chaque coordonnee est dans {0, 1}. On applique la translation de 
    vecteur y = (xp - x) a chaque point du voisinage de x.
*/
{
  int32_t Nc = Kc->N; int32_t *Dc = Kc->D; uint8_t *Sc = Kc->S; 
  int32_t i, u, n;
  int32_t *tX = AllocNd();
  int32_t *tY = AllocNd();
#ifdef DEBUGCA
printf("CopieAncetresNd\n");
#endif
  decodeNd(K, x, tX);
  for (i = 0; i < Nc; i++) tY[i] = (tX[i] % 2) - tX[i];
  memset(Sc, 0, Dc[Nc-1]);
  betamoinsbitNd(K, x, BIT_OBJET, tL, &n);
  for (u = 0; u < n; u += 1)
  {
    decodeNd(K, tL[u], tX);
    for (i = 0; i < Nc; i += 1) tX[i] += tY[i];
    Set(Sc[encodeNd(Kc, tX)],BIT_OBJET);
  }  
  FreeNd(tY);
  FreeNd(tX);
} /* CopieAncetresNd() */

/* ==================================== */
int32_t EffaceCliquesSimplesNd(Khalimsky *K)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  int32_t x, n = 0;
  Khalimsky *G = AllocGrilleNd();
#ifdef DEBUGECS
printf("EffaceCliquesSimplesNd : niveau %d\n", IndexPileGrillesNd);
#endif
  for (x = 0; x < D[N-1]; x += 1)
    if (IsSet(S[x],BIT_OBJET) && EstCliqueNd(K, x))
        {
          CopieAncetresNd(G, K, x);
          EffaceLiensLibresNd(G);
          if (EnsembleSimpleNd(G)) 
          { 
            UnSet(S[x],BIT_OBJET); n++; 
          }
        }
  FreeGrilleNd(G);
  return n;
} /* EffaceCliquesSimplesNd() */

/* ==================================== */
int32_t EnsembleSimpleNd(Khalimsky *K)
/* ==================================== */
{
#ifdef DEBUGES
printf("EnsembleSimpleNd\n");
#endif
  if (ExactementUneCliqueNd(K)) return 1;
  if (EffaceCliquesSimplesNd(K) == 0) return 0;
  EffaceLiensLibresNd(K);
  return EnsembleSimpleNd(K);
} /* EnsembleSimpleNd() */

/* ==================================== */
int32_t CliqueSimpleNd(Khalimsky *K, int32_t x)
/* ==================================== */
{
  uint8_t *S = K->S; 
  int32_t ret;
  Khalimsky *G;

#ifdef DEBUGCS
printf("CliqueSimpleNd\n");
#endif
  if (!IsSet(S[x],BIT_OBJET)) return 0;
  if (!EstCliqueNd(K, x)) return 0;
  G = AllocGrilleNd();
  CopieAncetresNd(G, K, x);
  EffaceLiensLibresNd(G);
  ret = EnsembleSimpleNd(G);
  FreeGrilleNd(G);
  return ret;
} /* CliqueSimpleNd() */

/* ==================================== */
Khalimsky * KhalimskizeNd(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale
   resultat - chaque pixel de o est devenu une clique de k
*/
{
  int32_t rs = rowsize(o);
  int32_t cs = colsize(o);
  int32_t ds = depth(o);
  int32_t ps = rs * cs;
  uint8_t *O = UCHARDATA(o);
  int32_t N;
  int32_t userC[3];
  Khalimsky *K;
  uint8_t *S;
  int32_t i, j, k;

  if (ds == 1) N = 2; else N = 3;
  userC[0] = 2*rs + 1;
  userC[1] = 2*cs + 1;
  userC[2] = 2*ds + 1;
  
  K = AllocKhalimskyNd(N, userC);
  S = K->S;
  InitNd(N, userC);
  memset(S, 0, K->D[N-1]); /* init a 0 */

  if (N == 2)
  {
    for (j = 0; j < cs; j++)
      for (i = 0; i < rs; i++)
        if (O[j * rs + i])
          Set(S[(2*j+1) * K->D[0] + (2*i+1)],BIT_OBJET);
  }
  else
  {
    for (k = 0; k < ds; j++)
      for (j = 0; j < cs; j++)
        for (i = 0; i < rs; i++)
          if (O[k * ps + j * rs + i])
            Set(S[(2*k+1) * K->D[1] + (2*j+1) * K->D[0] + (2*i+1)],BIT_OBJET);
  }
  return K;
} /* KhalimskizeNd() */

#define NDG_0 255
#define NDG_1 200
#define NDG_2 128
#define NDG_3  60

/* ==================================== */
struct xvimage *DeKhalimskizeNd(Khalimsky * K)
/* ==================================== */
{
  int32_t N = K->N; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t rs, cs, ds, is;
  int32_t x, t;
  struct xvimage *ima;
  uint8_t *I;

  if (N == 2)
  {
    rs = C[0]; cs = C[1]; is = rs * cs;
    ima = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
    if (ima == NULL) erreur((char *)"DeKhalimskizeNd: allocimage failed");
    I = UCHARDATA(ima);
    for (x = 0; x < is; x++) 
      if (IsSet(S[x],BIT_OBJET))
      {
        t = TypeNd(K, x);
        if (t == 0) I[x] = NDG_0;
        if (t == 1) I[x] = NDG_1;
        if (t == 2) I[x] = NDG_2;
      }
  }
  else if (N == 3)
  {
    rs = C[0]; cs = C[1]; ds = C[2]; is = rs * cs * ds;
    ima = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
    if (ima == NULL) erreur((char *)"DeKhalimskizeNd: allocimage failed");
    I = UCHARDATA(ima);
    for (x = 0; x < is; x++) 
      if (IsSet(S[x],BIT_OBJET))
      {
        t = TypeNd(K, x);
        if (t == 0) I[x] = NDG_0;
        if (t == 1) I[x] = NDG_1;
        if (t == 2) I[x] = NDG_2;
        if (t > 2) I[x] = NDG_3;
      }
  }
  return ima;
} /* DeKhalimskizeNd() */

/* ==================================== */
void PrintKhalimsky(Khalimsky * K)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; int32_t *C = K->C; uint8_t *S = K->S; 
  int32_t rs, cs, d, ps, is, i;

  if (N == 3)
  {
    rs = C[0]; cs = C[1]; d = C[2]; ps = D[1]; is = D[2];
    for (i = 0; i < is; i++)
    {
      if (i % rs == 0) printf("\n");
      if (i % ps == 0) printf("\n");
      switch (TypeNd(K, i))
      {
        case 0: if (S[i]) printf("OOO"); else printf(" o "); break;
        case 1: if (S[i]) printf("XXX"); else printf(" x "); break;
        case 2: if (S[i]) printf("==="); else printf(" - "); break;
        case 3: if (S[i]) printf("***"); else printf(" . "); break;
        default: printf("unknown type");
      } /* switch (TypeNd(K, i)) */
    } /* for i */
    printf("\n");
  }
  else if (N == 2)
  {
    rs = C[0]; cs = C[1]; is = D[1];
    for (i = 0; i < is; i++)
    {
      if (i % rs == 0) printf("\n");
      printf("%3d ", (int32_t)(S[i]));
    } /* for i */
    printf("\n");
  }

} /* PrintKhalimsky() */

/* ==================================== */
void Connex8ObjNd(Khalimsky *K)
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  int32_t x;
  for (x = 0; x < D[N-1]; x++) 
    if (IsSet(S[x],BIT_OBJET)) 
      marquebetamoinsNd(K, x, BIT_OBJET, 1);
#ifdef DEBUGC8
printf("Connex8ObjNd : avant EffaceLiensLibres\n");
PrintKhalimsky(K);
#endif
  EffaceLiensLibresNd(K);
} /* Connex8ObjNd() */

/* ==================================== */
void Connex4ObjNd(Khalimsky *K)                 /* ! : utilise tL */
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  int32_t x, n, u;

  for (x = 0; x < D[N-1]; x++) 
  {
    if (!BordNd(K, x) && (TypeNd(K, x) != 0))     /* x non bord et non carre */
    {
      betaplusNd(K, x, tL, &n);            /* ses descendants dans la grille */
      for (u = 0; u < n; u++)
        if ((TypeNd(K, tL[u]) == 0) && !IsSet(S[tL[u]],BIT_OBJET)) goto skip;
      Set(S[x],BIT_OBJET);
skip: ;
    }  
  }
} /* Connex4ObjNd() */

/* ==================================== */
void InitKhalimsky3Nd(uint32_t c, Khalimsky * K)
/* ==================================== */
/*            
   c : config 3x3x3 codee sur 27 bits
   K: resultat - chaque pixel de o est devenu une clique de K
*/
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  uint8_t O[27];
  int32_t i, j, k, n;

  for (n = 0; n < 27; n++)
  {
    O[n] = (uint8_t)(c&1);
    c = c >> 1;
  } /* for n */

  memset(S, 0, K->D[N-1]); /* init a 0 */

  for (k = 0; k < 3; k++)
    for (j = 0; j < 3; j++)
      for (i = 0; i < 3; i++)
        if (O[k * 9 + j * 3 + i])
          Set(S[(2*k+1) * D[1] + (2*j+1) * D[0] + (2*i+1)],BIT_OBJET);

} /* InitKhalimsky3Nd() */

/* ==================================== */
void InitCubeKhalimsky3Nd(uint32_t c, Khalimsky * K)
/* ==================================== */
/*            
   c : config 2x2x2 codee sur 8 bits
   K: resultat - les points a 1 de la config sont codes par des atomes
*/
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  uint8_t O[8];
  int32_t i, j, k, n, x;

  for (n = 0; n < 8; n++)
  {
    O[n] = (uint8_t)(c&1);
    c = c >> 1;
  } /* for n */

  memset(S, 0, K->D[N-1]); /* init a 0 */

  for (k = 0; k < 2; k++)
    for (j = 0; j < 2; j++)
      for (i = 0; i < 2; i++)
        if (O[k * 4 + j * 2 + i])
          Set(S[(2*k) * D[1] + (2*j) * D[0] + (2*i)],BIT_OBJET);

  /* rajoute les elements superieurs */
  for (x = 0; x < D[N-1]; x++) 
    if (IsSet(S[x],BIT_OBJET)) 
      marquebetaplusNd(K, x, BIT_OBJET, 1);

  /* efface les liens libres */
  do 
  {
    n = 0;
    for (x = 0; x < D[N-1]; x++)
      if (IsSet(S[x],BIT_OBJET) && (NbPredNd(K, x) == 1))
      {
        UnSet(S[x],BIT_OBJET);
        n++;
      }
  } while (n > 0);

  /* retire l'element maximal */
  for (x = 0; x < D[N-1]; x++) 
    if (IsSet(S[x],BIT_OBJET) && EstCliqueNd(K, x))
    {
      UnSet(S[x],BIT_OBJET);
      break;
    }

} /* InitCubeKhalimsky3Nd() */

/* ==================================== */
void InitCubeKhalimsky4Nd(uint32_t c, Khalimsky * K)
/* ==================================== */
/*            
   c : config 2x2x2x2 codee sur 16 bits
   K: resultat - les points a 1 de la config sont codes par des atomes
*/
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  uint8_t O[16];
  int32_t i, j, k, l, n, x;

  for (n = 0; n < 16; n++)
  {
    O[n] = (uint8_t)(c&1);
    c = c >> 1;
  } /* for n */

  memset(S, 0, K->D[N-1]); /* init a 0 */

  for (l = 0; l < 2; l++)
    for (k = 0; k < 2; k++)
      for (j = 0; j < 2; j++)
        for (i = 0; i < 2; i++)
          if (O[l * 8 + k * 4 + j * 2 + i])
            Set(S[(2*l) * D[2] + (2*k) * D[1] + (2*j) * D[0] + (2*i)],BIT_OBJET);

  /* rajoute les elements superieurs */
  for (x = 0; x < D[N-1]; x++) 
    if (IsSet(S[x],BIT_OBJET)) 
      marquebetaplusNd(K, x, BIT_OBJET, 1);

  /* efface les liens libres */
  do 
  {
    n = 0;
    for (x = 0; x < D[N-1]; x++)
      if (IsSet(S[x],BIT_OBJET) && (NbPredNd(K, x) == 1))
      {
        UnSet(S[x],BIT_OBJET);
        n++;
      }
  } while (n > 0);

  /* retire l'element maximal */
  for (x = 0; x < D[N-1]; x++) 
    if (IsSet(S[x],BIT_OBJET) && EstCliqueNd(K, x))
    {
      UnSet(S[x],BIT_OBJET);
      break;
    }

} /* InitCubeKhalimsky4Nd() */

/* ==================================== */
void PropageRecNd(Khalimsky *K, int32_t x, int32_t bit)
/* ==================================== */
/* propage recursivement le bit "bit" dans le theta du point x */
{
  uint8_t *S = K->S; 
  int32_t u, n = 0;
  int32_t *L = AllocVoisListNd();
  Set(S[x],bit);
  betamoinsbitNd(K, x, BIT_OBJET, L, &n);
  for (u = 0; u < n; u++) 
    if (!IsSet(S[L[u]],bit)) PropageRecNd(K, L[u], bit);  
  betaplusbitNd(K, x, BIT_OBJET, L, &n);
  for (u = 0; u < n; u++) 
    if (!IsSet(S[L[u]],bit)) PropageRecNd(K, L[u], bit);  
  FreeVoisListNd(L);
}

/* ==================================== */
int32_t ConnexeNd(Khalimsky *K)
/* ==================================== */
/*
  Suppose que tous les points objet ont leur bit BIT_OBJET a 1.
  On repere un point objet (si non existe, retour 1: l'ensemble vide est connexe)
  et on lance la propagation recursive du marqueur BIT_CONNEX dans la comp. connexe
  du point.
  Enfin, on verifie que tous les points ont leur bit BIT_CONNEX positionne.

  Attention: cette fonction laisse le bit BIT_CONNEX des points a la valeur 1.
*/

{
  int32_t N = K->N; uint8_t *S = K->S; 
  int32_t x;

  for (x = 0; x < N; x++)
    if (IsSet(S[x],BIT_OBJET))
    {
      PropageRecNd(K, x, BIT_CONNEX);
      break;
    }
  if (x == N) return 1; /* ens. vide connexe */

  for (x = 0; x < N; x++) 
    if (IsSet(S[x],BIT_OBJET) && !IsSet(S[x],BIT_CONNEX)) 
    {
#ifdef DEBUGCONNEXE
printf("Connexe : connexite NON ok : x = %d\n", x);
#endif
      return 0;
    }
#ifdef DEBUGCONNEXE
printf("Connexe : connexite ok\n");
#endif
  return 1;
} /* ConnexeNd() */

/* ==================================== */
int32_t CourbeSimpleNd(Khalimsky *K)                 /* ! : utilise tL */
/* ==================================== */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; 
  int32_t x, n, m, nbliens;

#ifdef DEBUGCS
printf("CourbeSimpleNd\n");
PrintKhalimsky(K);
#endif

  if (!ConnexeNd(K)) 
  {
#ifdef DEBUGCS
printf("CourbeSimpleNd : connexite NON ok\n");
#endif
    return 0;
  }
  
  nbliens = 0;
  for (x = 0; x < D[N-1]; x += 1)
    if (IsSet(S[x],BIT_OBJET))
    { /* verifie que le theta* de x se compose d'exactement deux liens */
      nbliens++;
      betamoinsbitNd(K, x, BIT_OBJET, tL, &n);
      betaplusbitNd(K, x, BIT_OBJET, tL, &m);
      if ((n+m) != 2) 
      {
#ifdef DEBUGCS
printf("CourbeSimpleNd : condition de courbe violee: point %d\n", x);
#endif
        return 0;
      }
    }
  if (nbliens == 0) return 0;
  return 1;
}  /* CourbeSimpleNd() */

/* ==================================== */
int32_t SurfaceSimpleNd(Khalimsky *K, Khalimsky *Kp)     /* ! : utilise tL */
/* ==================================== */
     /* NON TESTE */
{
  int32_t N = K->N; int32_t *D = K->D; uint8_t *S = K->S; uint8_t *Sp = Kp->S; 
  int32_t x, n, u, npoints = 0;

  if (!ConnexeNd(K)) return 0;
  
  for (x = 0; x < D[N-1]; x += 1)
    if (IsSet(S[x],BIT_OBJET))
    { /* verifie que le theta* de x est une courbe simple */
      npoints++;
      memset(Sp, VAL_NULLE, D[N-1]); /* init a VAL_NULLE */
      betaplusbitNd(K, x, BIT_OBJET, tL, &n);
      for (u = 0; u < n; u++) Set(Sp[tL[u]],BIT_OBJET);
#ifdef DEBUGSS
for (u = 0; u < n; u++) printf("%d ", tL[u]); printf("\n");
#endif
      betamoinsbitNd(K, x, BIT_OBJET, tL, &n);
      for (u = 0; u < n; u++) Set(Sp[tL[u]],BIT_OBJET);
#ifdef DEBUGSS
for (u = 0; u < n; u++) printf("%d ", tL[u]); printf("\n");
#endif
      if (!CourbeSimpleNd(Kp)) 
      {
#ifdef DEBUGSS
printf("SurfaceSimpleNd : theta* non courbe simple : %d\n", x);
#endif
        return 0;            
      }
    }
  if (npoints == 0) return 0;
  return 1;
}  /* SurfaceSimpleNd() */
