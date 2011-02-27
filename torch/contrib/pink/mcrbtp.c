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
   Librairie mcrbtp :

   Fonctions pour la gestion d'un arbre rouge et noir

   Data de type point (triplet de double)

   D'apres "Introduction a l'algorithmique", 
     T. Cormen, C. Leiserson, R. Rivest, pp. 258, Dunod Ed. 

   Michel Couprie - aout 2000

   Modif avril 2001: reallocation si depassement de capacite
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcrbtp.h>

/* #define TESTRBT */
/* #define VERBOSE */

/* ==================================== */
Rbtp * CreeRbtpVide(
  int32_t taillemax)
/* ==================================== */
{
  int32_t i;
  Rbtp * T = (Rbtp *)calloc(1,sizeof(Rbtp) + taillemax*sizeof(RbtpElt));
  /* le tableau Elts du Rbtp peut stocker taillemax+1 elements, dont 1 pour nil */
  /* l'element 0 du tableau est reserve pour representer nil */
  if (T == NULL)
  {   fprintf(stderr, "CreeRbtpVide() : malloc failed\n");
      return NULL;
  }
  T->max = taillemax;
  T->util = 0;
  T->maxutil = 0;
  T->nil = &(T->elts[0]);
  T->nil->left = T->nil->right = T->nil->parent = NULL;
  T->root = T->nil;

  /* chaine les elements libres a l'aide du pointeur right */
  for (i = 1; i < taillemax; i++) T->elts[i].right = &(T->elts[i+1]);
  T->elts[taillemax].right = NULL;
  T->libre = &(T->elts[1]);

  return T;
} /* CreeRbtpVide() */

/* ==================================== */
void RbtpTransRec(
  Rbtp **T, Rbtp * A, RbtpElt * x)
/* ==================================== */
{
  if (x == A->nil) return;
  RbtpInsert(T, x->key, x->auxdata);
  RbtpTransRec(T, A, x->left);
  RbtpTransRec(T, A, x->right);
} /* RbtpTransRec() */

/* ==================================== */
void RbtpReAlloc(Rbtp **A)
/* ==================================== */
{
  int32_t taillemax;
  Rbtp * T, *Tmp;

#ifdef VERBOSE
  printf("RbtpReAlloc: ancienne taille %d nouvelle taille %d\n", (*A)->max, 2 * (*A)->max);
#endif

  taillemax = 2 * (*A)->max;  /* alloue le double de l'ancienne taille */ 
  T = CreeRbtpVide(taillemax);
  RbtpTransRec(&T, *A, (*A)->root);
  Tmp = *A;
  *A = T;
  free(Tmp);
} /* RbtpReAlloc() */

/* ==================================== */
void RbtpFlush(
  Rbtp * T)
/* ==================================== */
{
  int32_t i;
  T->util = 0;
  for (i = 0; i < T->max - 1; i++) T->elts[i].right = &(T->elts[i+1]);
  T->elts[T->max - 1].right = NULL;
  T->root = T->nil;
} /* RbtpFlush() */

/* ==================================== */
int32_t RbtpVide(
  Rbtp * T)
/* ==================================== */
{
  return (T->util == 0);
} /* RbtpVide() */

/* ==================================== */
void RbtpTermine(
  Rbtp * T)
/* ==================================== */
{
#ifdef VERBOSE
  printf("Rbtp: taux d'utilisation: %g\n", (double)T->maxutil / (double)T->max);
#endif
  free(T);
} /* RbtpTermine() */

/* ==================================== */
void RbtpPrintRec(
  Rbtp * T, RbtpElt * x, int32_t niv)
/* ==================================== */
{
  int32_t i;
  if (x == T->nil) return;
  RbtpPrintRec(T, x->left, niv+1);
  for (i = 0; i < niv; i++) printf("    ");
  PRINTKEY(x->key); printf("(");
  if (x->color == RBTP_Red) printf("r"); else  printf("b");
  printf(")\n");
  RbtpPrintRec(T, x->right, niv+1);
} /* RbtpPrintRec() */

/* ==================================== */
void RbtpPrint(
  Rbtp * T)
/* ==================================== */
{
  RbtpPrintRec(T, T->root, 0);
} /* RbtpPrint() */

/* ==================================== */
RbtpElt * RbtpSearch(
  Rbtp * T, TypRbtpKey k)
/* ==================================== */
{
  RbtpElt * x = T->root;
  while ((x != T->nil) && !EQUALKEY(k,x->key))
    if (LESSKEY(k,x->key)) x = x->left; else x = x->right;
  return x;
} /* RbtpSearch() */

/* ==================================== */
RbtpElt * RbtpMinimum(
  Rbtp * T, RbtpElt * x)
/* ==================================== */
{
  while (x->left != T->nil) x = x->left;
  return x;
} /* RbtpMinimum() */

/* ==================================== */
RbtpElt * RbtpMaximum(
  Rbtp * T, RbtpElt * x)
/* ==================================== */
{
  while (x->right != T->nil) x = x->right;
  return x;
} /* RbtpMaximum() */

/* ==================================== */
RbtpElt * RbtpSuccessor(
  Rbtp * T, RbtpElt * x)
/* ==================================== */
{
  RbtpElt * y;
  if (x->right != T->nil) return RbtpMinimum(T, x->right);
  y = x->parent;
  while ((y != T->nil) && (x == y->right))
  {
    x = y;
    y = y->parent;
  }
  return y;
} /* RbtpSuccessor() */

/* ==================================== */
void RbtpInsertSimple(
  Rbtp * T, RbtpElt * z)
/* ==================================== */
{
  RbtpElt * x;
  RbtpElt * y;

  y = T->nil;
  x = T->root;
  while (x != T->nil)
  {
    y = x;
    if (LESSKEY(z->key,x->key)) x = x->left; else x = x->right;
  }
  z->parent = y;
  if (y == T->nil)
    T->root = z;
  else
    if (LESSKEY(z->key,y->key)) y->left = z; else y->right = z;
} /* RbtpInsertSimple() */

/* ==================================== */
RbtpElt * RbtpInsertAux(  /* allocation et insertion simple */
  Rbtp ** T, TypRbtpKey k, TypRbtpAuxData d)
/* ==================================== */
{
  RbtpElt * z;

  if ((*T)->libre == NULL) RbtpReAlloc(T);
  (*T)->util++;
  if ((*T)->util > (*T)->maxutil) (*T)->maxutil = (*T)->util;
  z = (*T)->libre;
  (*T)->libre = (*T)->libre->right;
  COPYKEY(z->key,k);
  z->auxdata = d;
  z->left = (*T)->nil;
  z->right = (*T)->nil;
  RbtpInsertSimple((*T), z);
  return z;
} /* RbtpInsertAux() */

/* ==================================== */
static void LeftRotate(
  Rbtp * T, RbtpElt * x)
/* ==================================== */
{
  RbtpElt * y;

  y = x->right;                    /* assume right(x) != NIL */
  x->right = y->left;              /* move y's child over */
  if (y->left != T->nil)
    y->left->parent = x;
  y->parent = x->parent;           /* move y up to x's position */
  if (x->parent == T->nil)
    T->root = y;
  else 
  {
    if (x == x->parent->left)
      x->parent->left = y;
    else x->parent->right = y;
  }
  y->left = x;                     /* move x down */
  x->parent = y;
} /* LeftRotate() */

/* ==================================== */
static void RightRotate(
  Rbtp * T, RbtpElt * x)
/* ==================================== */
{
  RbtpElt * y;

  y = x->left;              /* assume left(x) != NIL */
  x->left = y->right;
  if (y->right != T->nil)
    y->right->parent = x;
  y->parent = x->parent;
  if (x->parent == T->nil)
    T->root = y;
  else 
  {
    if (x == x->parent->right)
       x->parent->right = y;
    else x->parent->left = y;
  }
  y->right = x;
  x->parent = y;
} /* RightRotate() */

/* ==================================== */
RbtpElt * RbtpInsert(
  Rbtp ** T, TypRbtpKey k, TypRbtpAuxData d)
/* ==================================== */
{
  RbtpElt * x;
  RbtpElt * xc;            /* pour retourner le pointeur sur l'element alloue */
  RbtpElt * uncle;

  xc = x = RbtpInsertAux(T, k, d);          /* allocation et insertion simple */
  x->color = RBTP_Red;

  /* re-equilibrage de l'arbre */
  while ((x != (*T)->root) && (x->parent->color == RBTP_Red))
  {
    if (x->parent == x->parent->parent->left)
    {
      uncle = x->parent->parent->right;
      if (uncle->color == RBTP_Red)
      {
        x->parent->color = RBTP_Black;                    /* Case I */
        uncle->color = RBTP_Black;
        x->parent->parent->color = RBTP_Red;
        x = x->parent->parent;
      }
      else 
      {
        if (x == x->parent->right)
        {
          x = x->parent;                             /* Case II */
          LeftRotate((*T),x);
        }
        x->parent->color = RBTP_Black;                    /* Case III */
        x->parent->parent->color = RBTP_Red;
        RightRotate((*T), x->parent->parent);
      }
    }
    else /* same as "then" with "right" and "left" swapped */
    {
      uncle = x->parent->parent->left;
      if (uncle->color == RBTP_Red)
      {
        x->parent->color = RBTP_Black;                     /* Case I */
        uncle->color = RBTP_Black;
        x->parent->parent->color = RBTP_Red;
        x = x->parent->parent;
      }
      else 
      {
        if (x == x->parent->left)
        {
          x = x->parent;                             /* Case II */
          RightRotate((*T),x);
        }
        x->parent->color = RBTP_Black;                    /* Case III */
        x->parent->parent->color = RBTP_Red;
        LeftRotate((*T), x->parent->parent);
      }
    }
  } /* while */
  (*T)->root->color = RBTP_Black;
  return xc;                      /* modif mc: retourne xc plutot que x */
} /* RbtpInsert() */

/* ==================================== */
void RbtpDeleteFixup(
  Rbtp * T, RbtpElt * x)
/* ==================================== */
{
  RbtpElt * s;

  while ((x != T->root) && (x->color == RBTP_Black))
  {
    if (x == x->parent->left)
    {
      s = x->parent->right;               /* Get x's sibling */
      if (s->color == RBTP_Red)
      {
        s->color = RBTP_Black;              /* Case I */
        x->parent->color = RBTP_Red;
        LeftRotate(T, x->parent);
        s = x->parent->right;
      }
      if ((s->left->color == RBTP_Black) && (s->right->color == RBTP_Black))
      {
        s->color = RBTP_Red;                /* Case II */
        x = x->parent;
      }              
      else 
      {
        if (s->right->color == RBTP_Black)
	{
          s->left->color = RBTP_Black;      /* Case III */
          s->color = RBTP_Red;                        
          RightRotate(T,s);
          s = x->parent->right;
        }
        s->color = x->parent->color;   /* Case IV */
        x->parent->color = RBTP_Black;
        s->right->color = RBTP_Black;
        LeftRotate(T, x->parent);                   
        x = T->root;
      }
    }
    else
    {            /* Same as "then" with right and left swapped */
      s = x->parent->left;               /* Get x's sibling */
      if (s->color == RBTP_Red)
      {
        s->color = RBTP_Black;              /* Case I */
        x->parent->color = RBTP_Red;
        RightRotate(T, x->parent);
        s = x->parent->left;
      }
      if ((s->right->color == RBTP_Black) && (s->left->color == RBTP_Black))
      {
        s->color = RBTP_Red;                /* Case II */
        x = x->parent;
      }              
      else 
      {
        if (s->left->color == RBTP_Black)
	{
          s->right->color = RBTP_Black;     /* Case III */
          s->color = RBTP_Red;                        
          LeftRotate(T,s);
          s = x->parent->left;
        }
        s->color = x->parent->color;   /* Case IV */
        x->parent->color = RBTP_Black;
        s->left->color = RBTP_Black;
        RightRotate(T, x->parent);                   
        x = T->root;
      }
    }
  } /* while */
  x->color = RBTP_Black;
} /* RbtpDeleteFixup() */

/* ==================================== */
RbtpElt * RbtpDeleteAux(         /* return deleted node */
  Rbtp * T, RbtpElt * z)
/* ==================================== */
{
  RbtpElt * c;
  RbtpElt * d;

  if ((z->left == T->nil) || (z->right == T->nil))
    d = z;
  else 
    d = RbtpSuccessor(T, z);
  if (d->left != T->nil)
    c = d->left;
  else 
    c = d->right;
  c->parent = d->parent;      /* no test for NIL with sentinel */
  if (d->parent == T->nil)
    T->root = c;
  else 
  {
    if (d == d->parent->left)
      d->parent->left = c;
    else 
      d->parent->right = c;
  }
  if (d != z)
  {
    COPYKEY(z->key,d->key);
    z->auxdata = d->auxdata;
  }
  if (d->color == RBTP_Black)
    RbtpDeleteFixup(T, c);     /* c is now "Double-Black" */
  return d;
} /* RbtpDeleteAux() */

/* ==================================== */
void RbtpDelete(
  Rbtp * T, RbtpElt * z)
/* ==================================== */
{
  z = RbtpDeleteAux(T, z);
  z->right = T->libre;
  T->libre = z;
  T->util -= 1;
} /* RbtpDelete() */

/* ==================================== */
TypRbtpAuxData RbtpPopMin(
  Rbtp * T)
/* ==================================== */
/* 
  Retire de l'arbre l'element de cle min.
  ATTENTION: pas de test arbre vide.
*/
{
  RbtpElt * z = T->root;
  while (z->left != T->nil) z = z->left; /* recherche le min */
  z = RbtpDeleteAux(T, z);                /* efface de l'arbre */
  z->right = T->libre;
  T->libre = z;
  T->util -= 1;
  return z->auxdata;
} /* RbtpPopMin() */

/* ==================================== */
TypRbtpAuxData RbtpPopMax(
  Rbtp * T)
/* ==================================== */
/* 
  Retire de l'arbre l'element de cle max.
  ATTENTION: pas de test arbre vide.
*/
{
  RbtpElt * z = T->root;
  while (z->left != T->nil) z = z->right; /* recherche le max */
  z = RbtpDeleteAux(T, z);                 /* efface de l'arbre */
  z->right = T->libre;
  T->libre = z;
  T->util -= 1;
  return z->auxdata;
} /* RbtpPopMax() */

/* ==================================== */
TypRbtpKey RbtpMinLevel(
  Rbtp * T)
/* ==================================== */
{
  RbtpElt * x = T->root;
  while (x->left != T->nil) x = x->left;
  return x->key;
} /* RbtpMinLevel() */

#ifdef TESTRBTP
int32_t main()
{
  Rbtp * T = CreeRbtpVide(1);
  char r[80];
  double p;
  RbtpElt * x;

  do
  {
    printf("commande (qUIT, PuSH, PoP, pRINT, TESTvIDE\n");
    printf("          sEARCH MiNIMUM MaXIMUM SUcCESSOR dELETE) > \n");
    scanf("%s", r);
    switch (r[0])
    {
      case 'u':
        printf("valeur > ");
        scanf("%lf", &p);
        (void)RbtpInsert(&T, p, 0);
        break;
      case 'd':
        printf("valeur > ");
        scanf("%lf", &p);
        x = RbtpSearch(T, p);
        if (x != T->nil) RbtpDelete(T, x);
        else printf("pas trouve !\n");
        break;
      case 's':
        printf("valeur > ");
        scanf("%lf", &p);
        x = RbtpSearch(T, p);
        printf("trouve: %d\n", x != T->nil);
        break;
      case 'i':
        x = RbtpMinimum(T, T->root);
        printf("minimum: %g\n", x->key);
        break;
      case 'a':
        x = RbtpMaximum(T, T->root);
        printf("maximum: %g\n", x->key);
        break;
      case 'c':
        printf("valeur > ");
        scanf("%lf", &p);
        x = RbtpSearch(T, p);
        printf("trouve: %d\n", x != T->nil);
        if (x != T->nil)
	{
          x = RbtpSuccessor(T, x);
          if (x != T->nil) printf("succ: %g\n", x->key);
	}
        break;
      case 'o': 
        if (RbtpVide(T)) 
          printf("vide\n");
        else
          (void)RbtpPopMin(T); 
        break;
      case 'p': RbtpPrint(T); break;
      case 'v': printf("vide: %d\n", RbtpVide(T)); break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  RbtpTermine(T);
}
#endif
