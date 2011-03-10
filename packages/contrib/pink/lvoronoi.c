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
/* diagramme de Voronoi et triangulation de Delaunay */
/* Michel Couprie - decembre 1998 */
/* d'apres "Mathematique et informatique - problemes resolus", vol. 2, pp. 95-109 */
/* Jean Berstel, Jean-Eric Pin, Michel Pocchiola - Ed. McGraw Hill */
/* Algorithme en o(n^2) : il y mieux ! */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <mcgeo.h>
#include <lvoronoi.h>

//#define DEBUG

#define AGAUCHE 1
#define ADROITE -1

/* ==================================== */
int32_t LePlusProcheVoisin(mcgeo_point *S, int32_t n, int32_t i)
/* ==================================== */
{
  int32_t j, k;
  double d, dk;

  if (i == 0) j = 1; else j = 0; 
  d = carredistance(S[i], S[j]);
  for (k = 0; k < n; k++)
  {
    if (k != i)
    {
      dk = carredistance(S[i], S[k]);
      if (dk < d) { d = dk; j = k; }
    }
  } /* for k */
  return j;
} /* LePlusProcheVoisin() */

/* ==================================== */
int32_t EstVoisin(int32_t **voisins, int32_t *nbvois, int32_t i, int32_t k)
/* ==================================== */
/* test si k est dans la liste des voisins de i */
{
  int32_t j;
  for (j = 0; j < nbvois[i]; j++)
    if (voisins[i][j] == k) return 1;
  return 0;
} /* EstVoisin() */

/* ==================================== */
int32_t EstVoisinSym(int32_t **voisins, int32_t *nbvois, int32_t i, int32_t k)
/* ==================================== */
/* test si k est dans la liste des voisins de i ou si 
   i est dans la liste des voisins de k */
{
  return EstVoisin(voisins, nbvois, i, k) || EstVoisin(voisins, nbvois, k, i);
} /* EstVoisinSym() */

/* ==================================== */
int32_t VoisinSuivant(mcgeo_point *S, int32_t n, int32_t **v, int32_t *nv, int32_t i, int32_t j, int32_t cote)
/* ==================================== */
/*
  On determine un triangle de Delaunay en considerant une arete Si, Sj et 
  en recherchant le site Sk qui forme avec les deux sites un angle maximal.
  La fonction calcule l'indice k d'un tel site, situe a droite ou a gauche de
  SiSj (droite orientee), ou retourne -1 s'il n'y en a pas.
*/
{
  int32_t a, k;
  double cosmin, cosinus;
  
  a = -1;
  cosmin = 1.0;
  for (k = 0; k < n; k++) /* recherche parmi les sites du cote donne */
  {
    if ((k != i) && (k != j) && (estsitue(S[k], S[i], S[j]) == cote))
    {
      cosinus = cosangle(S[k], S[i], S[j]);
      if (cosinus < cosmin) { cosmin = cosinus; a = k; } 
      else if ((cosinus == cosmin) && 
               ( EstVoisin(v, nv, i, k) || EstVoisin(v, nv, j, k) ||
                 EstVoisin(v, nv, k, i) || EstVoisin(v, nv, k, j))) a = k;
    }
  }

  return a;
} /* VoisinSuivant() */

/* ==================================== */
int32_t VoisinDroit(mcgeo_point *S, int32_t n, int32_t **v, int32_t *nv, int32_t i, int32_t j)
/* ==================================== */
{
  return VoisinSuivant(S, n, v, nv, i, j, ADROITE);
} /* VoisinDroit() */

/* ==================================== */
int32_t VoisinGauche(mcgeo_point *S, int32_t n, int32_t **v, int32_t *nv, int32_t i, int32_t j)
/* ==================================== */
{
  return VoisinSuivant(S, n, v, nv, i, j, AGAUCHE);
} /* VoisinGauche() */

/* ==================================== */
void InsererADroite(int32_t **voisins, int32_t *nbvois, int32_t k, int32_t i)
/* ==================================== */
/* insertion de k a la fin de la liste de voisins de i */
{
  voisins[i][nbvois[i]] = k;
  nbvois[i]++;
} /* InsererADroite() */

/* ==================================== */
void InsererAGauche(int32_t **voisins, int32_t *nbvois, int32_t k, int32_t i)
/* ==================================== */
/* insertion de k au debut de la liste de voisins de i */
{
  int32_t j; 
  for (j = nbvois[i]; j > 0; j--) voisins[i][j] = voisins[i][j - 1];

  voisins[i][0] = k;
  nbvois[i]++;
} /* InsererAGauche() */

/* ==================================== */
void Retirer1(int32_t **voisins, int32_t *nbvois, int32_t k, int32_t i)
/* ==================================== */
/* retirer k de la liste de voisins de i */
{
  int32_t j, h, n = nbvois[i]; 

  for (j = 0; j < n; j++) if (voisins[i][j] == k) break;
  if (j == n) return;
  for (h = j; h < n-1; h++) voisins[i][h] = voisins[i][h+1];
  nbvois[i]--;
} /* Retirer1() */

/* ==================================== */
void Retirer2(int32_t **voisins, int32_t *nbvois, int32_t k, int32_t i)
/* ==================================== */
/* retirer k de la liste de voisins de i et i de la liste de voisins de k */
{
  Retirer1(voisins, nbvois, k, i);
  Retirer1(voisins, nbvois, i, k);
} /* Retirer2() */

/* ==================================== */
int32_t ldelaunay(mcgeo_point *S, int32_t n, int32_t ***v, int32_t **nv, int32_t **ec)
/* ==================================== */
/* entrees : S : le tableau des points
             n : le nombre de points
   sorties : v : la table des listes de voisins (reperes par leur indice dans S)
             nv : la table des nombres de voisins
             ec : table de booleens indiquant les points de l'enveloppe convexe
*/
{
#undef F_NAME
#define F_NAME "ldelaunay"
  int32_t i, j, k, m;
  int32_t **voisins, *nbvois, *envconv;

  nbvois = (int32_t *)calloc(n, sizeof(int32_t));
  envconv = (int32_t *)calloc(n, sizeof(int32_t));
  voisins = (int32_t **)calloc(n, sizeof(int32_t *));
  if ((nbvois == NULL) || (envconv == NULL) || (voisins == NULL))
  {
    fprintf(stderr, "%s: malloc failed(1)\n", F_NAME);
    return 0;
  }
  for (i = 0; i < n; i++)
  {
    voisins[i] = (int32_t *)calloc(n, sizeof(int32_t));  
    if (voisins[i] == NULL)
    {
      fprintf(stderr, "%s: malloc failed(i = %d)\n", F_NAME, i);
      return 0;
    }
  }

  for (i = 0; i < n; i++)
  {
#ifdef DEBUG
printf("point %d (%d,%d) ", i, (int32_t)S[i].x, (int32_t)S[i].y);
#endif

    m = LePlusProcheVoisin(S, n, i);
#ifdef DEBUG
printf("ppvois = %d ", m);
#endif

    voisins[i][0] = m;
    nbvois[i] = 1;
    j = m;
    k = VoisinDroit(S, n, voisins, nbvois, i, j);
    while ((k != -1) && (k != m))
    {
      InsererADroite(voisins, nbvois, k, i);
#ifdef DEBUG
printf("vd=%d ", k);
#endif      
      j = k; 
      k = VoisinDroit(S, n, voisins, nbvois, i, j);
    } /* while ((k != -1) && (k != m)) */
    if (k == m)
    {
      envconv[i] = 0;
    }
    else
    {
      envconv[i] = 1;
      j = m;
      k = VoisinGauche(S, n, voisins, nbvois, i, j);
      while (k != -1)
      {
        InsererAGauche(voisins, nbvois, k, i);
#ifdef DEBUG
printf("vg=%d ", k);
#endif      
        j = k; 
        k = VoisinGauche(S, n, voisins, nbvois, i, j);
      } /* while ((k != -1) && (k != m)) */
    }
#ifdef DEBUG
printf("\n");
#endif      
  } /* for i */

  *v = voisins;
  *nv = nbvois;
  *ec = envconv;
  return 1;
}

/* ==================================== */
int32_t ltriang_delaunay(mcgeo_point *S, int32_t n, int32_t **v, int32_t *nv, int32_t *ec)
/* ==================================== */
/* 
   Modifie le graphe de Delaunay donné en entrée pour assurer qu'il s'agisse
   d'une 2-surface (en fait, une triangulation).
   La propriété garantie en sortie est que le voisinage (au sens: liste ordonnée des
   faces adjacentes) de tout sommet forme une courbe simple (fermée ou ouverte).
   entrees : S : le tableau des points
             n : le nombre de points
             ec : table de booleens indiquant les points de l'enveloppe convexe
   entrees/sorties :
             v : la table des listes de voisins (reperes par leur indice dans S)
             nv : la table des nombres de voisins
*/
{
#undef F_NAME
#define F_NAME "ldelaunay"
  int32_t i, j, k, vprec, vcour, vsuiv;
  for (i = 0; i < n; i++)
  {
    k = nv[i];
#ifdef DEBUG
printf("\n i = %d ; k = %d ; voisins : ", i, k);
for (j = 0; j < nv[i]; j++) printf("%d ", v[i][j]);
printf("\n");
#endif
    j = 0;
    while (j < k) 
    {
      vprec = v[i][(j-1+k) % k];
      vcour = v[i][j];
      vsuiv = v[i][(j+1) % k];
#ifdef DEBUG
printf("j = %d ; k = %d\n", j, k);
printf("ivprec = %d ; ivcour = %d ; ivsuiv = %d\n", (j-1+k) % k, j, (j+1) % k);
printf("vprec = %d ; vcour = %d ; vsuiv = %d\n", vprec, vcour, vsuiv);
#endif

      if (EstVoisinSym(v, nv, vprec, vcour)
       && EstVoisinSym(v, nv, vcour, vsuiv)
       && EstVoisinSym(v, nv, vprec, vsuiv)
       && sontsecants(S[i], S[vcour], S[vprec], S[vsuiv])
	 )
      {
	// si le segment (i, vcour) intersecte le segment (vprec, vsuiv) alors
	// retirer le sommet vcour de la liste des voisins de i (et inversement)
	Retirer2(v, nv, i, vcour);
	k--;
      }
      else
	j++;
    } // while (j < k) 
  }

  return 1;
}
