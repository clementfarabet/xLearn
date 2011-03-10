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
      Librairie mcskelcurv :

      Gestion de squelettes curvilignes

      Michel Couprie, juin 2004

      Update Michel Couprie march 2010: 
      - deletion of skeleton elements
        deleted elements are marked (field "tag"), not physically removed.
      - use different list cells for points and neighbours
*/
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcskelcurv.h>

//#define DEBUG
//#define DEBUG_skeldelete

#ifdef DEBUG
struct xvimage *DBGIM;
#endif

/* ================================================ */
/* ================================================ */
/* fonctions privees */
/* ================================================ */
/* ================================================ */

/* ====================================================================== */
static int32_t tailleptliste(SKC_pt_pcell p)
/* ====================================================================== */
{
  int32_t n = 0;
  for (; p != NULL; p = p->next) n++;
  return n;
} /* tailleptliste() */

/* ====================================================================== */
static int32_t tailleadjliste(SKC_adj_pcell p)
/* ====================================================================== */
{
  int32_t n = 0;
  for (; p != NULL; p = p->next) n++;
  return n;
} /* tailleadjliste() */

/* ====================================================================== */
static void printadjliste(SKC_adj_pcell p)
/* ====================================================================== */
{
  for (; p != NULL; p = p->next)
    printf("%d ", p->val);
} /* printadjliste() */

/* ====================================================================== */
static void printptsliste(SKC_pt_pcell p, int32_t rs)
/* ====================================================================== */
{
  for (; p != NULL; p = p->next)
    printf("%d,%d ; ", p->val%rs, p->val/rs);
} /* printptsliste() */

/* ====================================================================== */
void printskel(skel * S)
/* ====================================================================== */
{
  uint32_t i;
  
  printf("isol:\n");
  for (i = 0; i < S->e_isol; i++)
  {
    if (SK_DELETED(i)) printf("DELETED ");
    printf("[%d] ", i); printf("adj: "); printadjliste(S->tskel[i].adj);
    printf("pts: "); printptsliste(S->tskel[i].pts, S->rs); printf("\n");
  }
  printf("end:\n");
  for (i = S->e_isol; i < S->e_end; i++)
  {
    if (SK_DELETED(i)) printf("DELETED ");
    printf("[%d] ", i); printf("adj: "); printadjliste(S->tskel[i].adj);
    printf("pts: "); printptsliste(S->tskel[i].pts, S->rs); printf("\n");
  }
  printf("curv:\n");
  for (i = S->e_end; i < S->e_curv; i++)
  {
    if (SK_DELETED(i)) printf("DELETED ");
    printf("[%d] ", i); printf("adj: "); printadjliste(S->tskel[i].adj);
    printf("pts: "); printptsliste(S->tskel[i].pts, S->rs); printf("\n");
  }
  printf("junc:\n");
  for (i = S->e_curv; i < S->e_junc; i++)
  {
    if (SK_DELETED(i)) printf("DELETED ");
    printf("[%d] ", i); printf("adj: "); printadjliste(S->tskel[i].adj);
    printf("pts: "); printptsliste(S->tskel[i].pts, S->rs); printf("\n");
  }
} /* printskel() */

/* ================================================ */
/* ================================================ */
/* fonctions publiques */
/* ================================================ */
/* ================================================ */

/* ================================================ */
/* fonctions d'entrée-sortie */
/* ================================================ */

/*
FILE = 2Dskel | 3Dskel <CONNEX> <NVERTEX> <NCELL> <RS> <CS> <DS> <ISOL> <END> <CURV> <JUNC>
ISOL = "ISOL" <N> <VERTEX> ... <VERTEX>
END  = "END"  <N> <VERTEX> ... <VERTEX>
CURV = "CURV" <N> <VERTEX> ... <VERTEX>
JUNC = "JUNC" <N> <VERTEX> ... <VERTEX>
VERTEX = "vertex" <I> "adj" <N> <I> ... <I> "pts" <N> <POINT> ...  <POINT> 
CONNEX = 4 | 8 | 6 | 18 | 26
NVERTEX = int32_t
NCELL = int32_t
RS = int32_t
CS = int32_t
DS = int32_t
N = int32_t
I = int32_t
POINT = int32_t
*/

/* ====================================================================== */
void fprintadjliste(FILE *fd, SKC_adj_pcell p)
/* ====================================================================== */
{
  int32_t n = tailleadjliste(p);
  fprintf(fd, "%d ", n);
  for (; p != NULL; p = p->next) fprintf(fd, "%d ", p->val);
} /* fprintadjliste() */

/* ====================================================================== */
void fprintptliste(FILE *fd, SKC_pt_pcell p)
/* ====================================================================== */
{
  int32_t n = tailleptliste(p);
  fprintf(fd, "%d ", n);
  for (; p != NULL; p = p->next) fprintf(fd, "%d ", p->val);
} /* fprintptliste() */

/* ====================================================================== */
void fprintptvliste(FILE *fd, SKC_pt_pcell p, float *V)
/* ====================================================================== */
{
  int32_t n = tailleptliste(p);
  fprintf(fd, "%d ", n);
  for (; p != NULL; p = p->next) fprintf(fd, "%d %g ", p->val, V[p->val]);
} /* fprintptvliste() */

/* ====================================================================== */
SKC_pt_pcell skeladdptcell(skel *S, int32_t val, SKC_pt_pcell next)
/* ====================================================================== */
// allocate a new cell and updates fields val and next
{
#undef F_NAME
#define F_NAME "skeladdptcell"
  SKC_pt_pcell p;
#ifdef DEBUGADD
  printf("SKELADDPTSCELL: val %d, free %d\n", val, S->freeptcell);
#endif
#ifdef DEBUG
  ((uint8_t*)((DBGIM)->image_data))[val] += 1;
#endif
  if (S->freeptcell >= S->nbptcell)
  {
    fprintf(stderr, "%s: not enough cells (%d >= %d)\n", F_NAME, S->freeptcell, S->nbptcell);
#ifdef DEBUG
    writeimage(DBGIM, "_DBGIM");
#endif
    exit(0);
  }
  p = &(S->tptcell[S->freeptcell]);
  S->freeptcell++;
  p->val = val;
  p->next = next;
  return p;
} // skeladdptcell()

/* ====================================================================== */
SKC_adj_pcell skeladdadjcell(skel *S, int32_t val, SKC_adj_pcell next)
/* ====================================================================== */
// allocate a new cell and updates fields val and next
{
#undef F_NAME
#define F_NAME "skeladdadjcell"
  SKC_adj_pcell p;
  if (S->freeadjcell >= S->nbadjcell)
  {
    fprintf(stderr, "%s: not enough cells\n", F_NAME);
    exit(0);
  }
  p = &(S->tadjcell[S->freeadjcell]);
  S->freeadjcell++;
  p->val = val;
  p->next = next;
  return p;
} // skeladdadjcell()

/* ====================================================================== */
void addadjlist(skel * S, uint32_t k, uint32_t v)
/* ====================================================================== */
// add v to the adjacency list of k
{
  SKC_adj_pcell p = skeladdadjcell(S, v, S->tskel[k].adj);
  S->tskel[k].adj = p;
} /* addadjlist() */

/* ====================================================================== */
void removeadjlist(skel * S, uint32_t k, uint32_t v)
/* ====================================================================== */
// remove v from the adjacency list of k - the cell is lost
{
  SKC_adj_pcell p = S->tskel[k].adj;
  assert(p != NULL);
  if (p->val == v) 
  {
    S->tskel[k].adj = p->next;
    return;
  }
  while (p->next != NULL)
  {
    if (p->next->val == v)
    {
      p->next = p->next->next;
      return;
    }    
    p = p->next;
  }
  assert(0);
} /* removeadjlist() */

/* ====================================================================== */
void addptslist(skel * S, uint32_t k, uint32_t v)
/* ====================================================================== */
{
  SKC_pt_pcell p;
#ifdef DEBUGADD
  printf("ADDPTSLIST: elt %d, pt %d, free %d\n", k, v, S->freeptcell);
#endif
  p = skeladdptcell(S, v, S->tskel[k].pts);
  S->tskel[k].pts = p;
} /* addptslist() */

/* ====================================================================== */
void writeskel(skel * S, char *filename)
/* ====================================================================== */
// write all elements, even those that are "deleted" 
{
#undef F_NAME
#define F_NAME "writeskel"
  uint32_t i;
  FILE *fd = NULL;

  fd = fopen(filename,"w");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  if (S->ds == 1) fprintf(fd, "2Dskel "); else fprintf(fd, "3Dskel ");

  fprintf(fd, "%d %d %d %d %d %d %d\n", S->connex, S->e_junc, S->freeptcell, S->freeadjcell, S->rs, S->cs, S->ds);

  fprintf(fd, "ISOL %d\n", S->e_isol);
  for (i = 0; i < S->e_isol; i++)
  {
    fprintf(fd, "vertex %d\n", i); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptliste(fd, S->tskel[i].pts); fprintf(fd, "\n"); 
  }

  fprintf(fd, "END %d\n", S->e_end - S->e_isol);
  for (i = S->e_isol; i < S->e_end; i++)
  {
    fprintf(fd, "vertex %d\n", i); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptliste(fd, S->tskel[i].pts); fprintf(fd, "\n"); 
  }

  fprintf(fd, "CURV %d\n", S->e_curv - S->e_end);
  for (i = S->e_end; i < S->e_curv; i++)
  {
    fprintf(fd, "vertex %d\n", i); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptliste(fd, S->tskel[i].pts); fprintf(fd, "\n"); 
  }

  fprintf(fd, "JUNC %d\n", S->e_junc - S->e_curv);
  for (i = S->e_curv; i < S->e_junc; i++)
  {
    fprintf(fd, "vertex %d\n", i); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptliste(fd, S->tskel[i].pts); fprintf(fd, "\n"); 
  }
  fclose(fd);
} /* writeskel() */

/* ====================================================================== */
void writevskel(skel * S, char *filename, struct xvimage *val)
/* ====================================================================== */
// idem writeskel, mais de plus chaque point est suivi d'une valeur trouvée dans l'image val,
// et chaque vertex est suivi d'une valeur (nulle par défaut).
// write all elements, even those that are "deleted" 
{
#undef F_NAME
#define F_NAME "writevskel"
  uint32_t i;
  FILE *fd = NULL;
  float *V;

  if ((rowsize(val) != S->rs) || (colsize(val) != S->cs) || (depth(val) != S->ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  (void)convertfloat(&val);
  V = FLOATDATA(val);

  fd = fopen(filename,"w");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  if (S->ds == 1) fprintf(fd, "2Dvskel "); else fprintf(fd, "3Dvskel ");

  fprintf(fd, "%d %d %d %d %d %d %d\n", S->connex, S->e_junc, S->freeptcell, S->freeadjcell, S->rs, S->cs, S->ds);

  fprintf(fd, "ISOL %d\n", S->e_isol);
  for (i = 0; i < S->e_isol; i++)
  {
    fprintf(fd, "vertex %d %d\n", i, 0); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptvliste(fd, S->tskel[i].pts, V); fprintf(fd, "\n"); 
  }

  fprintf(fd, "END %d\n", S->e_end - S->e_isol);
  for (i = S->e_isol; i < S->e_end; i++)
  {
    fprintf(fd, "vertex %d %d\n", i, 0); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptvliste(fd, S->tskel[i].pts, V); fprintf(fd, "\n"); 
  }

  fprintf(fd, "CURV %d\n", S->e_curv - S->e_end);
  for (i = S->e_end; i < S->e_curv; i++)
  {
    fprintf(fd, "vertex %d %d\n", i, 0); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptvliste(fd, S->tskel[i].pts, V); fprintf(fd, "\n"); 
  }

  fprintf(fd, "JUNC %d\n", S->e_junc - S->e_curv);
  for (i = S->e_curv; i < S->e_junc; i++)
  {
    fprintf(fd, "vertex %d %d\n", i, 0); 
    fprintf(fd, "adj "); fprintadjliste(fd, S->tskel[i].adj); fprintf(fd, "\n"); 
    fprintf(fd, "pts "); fprintptvliste(fd, S->tskel[i].pts, V); fprintf(fd, "\n"); 
  }
  fclose(fd);
} /* writevskel() */

/* ====================================================================== */
skel * readskel(char *filename)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "readskel"
#define BUFFERSIZE 1024
  skel * S;
  uint32_t i, j, k, v, n;
  char buf[BUFFERSIZE];
  FILE *fd = NULL;
  int32_t dim, rs, cs, ds, ret;
  int32_t nbisol, nbend, nbcurv, nbjunc, nvertex, nptcell, nadjcell, connex;

  fd = fopen(filename,"r");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return NULL;
  }

  fscanf(fd, "%s", buf);
  if (strncmp(buf, "3Dskel", 6) == 0) dim = 3; else
  if (strncmp(buf, "2Dskel", 6) == 0) dim = 2; else
  {
    fprintf(stderr, "%s: bad file format : %s\n", F_NAME, buf);
    return NULL;
  }

  ret = fscanf(fd, "%d %d %d %d %d %d %d", &connex, &nvertex, &nptcell, &nadjcell, &rs, &cs, &ds);
  assert(ret == 7);

#ifdef DEBUG
  printf("connex : %d ; nvertex : %d ; nptcell : %d ; nadjcell : %d ; rs : %d ; cs : %d ; ds : %d\n", 
	 connex, nvertex, nptcell, nadjcell, rs, cs, ds);
#endif

  S = initskel(rs, cs, ds, nvertex, nptcell, nadjcell, connex); 
  if (S == NULL)
  {   fprintf(stderr, "%s: initskel failed\n", F_NAME);
      return NULL;
  }

  // POINTS ISOLES
  fscanf(fd, "%s", buf);
  if ((strncmp(buf, "isol", 4) != 0) && (strncmp(buf, "ISOL", 4) != 0))
  {
    fprintf(stderr, "%s: bad file format (0) : %s\n", F_NAME, buf);
    return NULL;
  }
  fscanf(fd, "%d", &nbisol);
  S->e_isol = nbisol;

#ifdef DEBUG
  printf("points isolés : %d\n", nbisol);
#endif

  for (i = 0; i < nbisol; i++)
  {
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "vertex", 6) != 0)
    {
      fprintf(stderr, "%s: bad file format (1) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &k);
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "adj", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (2) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    assert(n == 0); // isole : pas de voisin
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "pts", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (3) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addptslist(S, k, v);
    }
  }

  // POINTS EXTREMITES
  fscanf(fd, "%s", buf);
  if ((strncmp(buf, "end", 3) != 0) && (strncmp(buf, "END", 3) != 0))
  {
    fprintf(stderr, "%s: bad file format (4) : %s\n", F_NAME, buf);
    return NULL;
  }
  fscanf(fd, "%d", &nbend);
  S->e_end = nbisol+nbend;

#ifdef DEBUG
  printf("points extrémités : %d\n", nbend);
#endif

  for (i = 0; i < nbend; i++)
  {
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "vertex", 6) != 0)
    {
      fprintf(stderr, "%s: bad file format (5) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &k);
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "adj", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (6) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addadjlist(S, k, v);
    }
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "pts", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (7) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addptslist(S, k, v);
    }
  }

  // POINTS DE COURBE
  fscanf(fd, "%s", buf);
  if ((strncmp(buf, "curv", 4) != 0) && (strncmp(buf, "CURV", 4) != 0))
  {
    fprintf(stderr, "%s: bad file format (8) : %s\n", F_NAME, buf);
    return NULL;
  }
  fscanf(fd, "%d", &nbcurv);
  S->e_curv = nbisol+nbend+nbcurv;

#ifdef DEBUG
  printf("courbes : %d\n", nbcurv);
#endif

  for (i = 0; i < nbcurv; i++)
  {
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "vertex", 6) != 0)
    {
      fprintf(stderr, "%s: bad file format (9) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &k);
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "adj", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (10) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addadjlist(S, k, v);
    }
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "pts", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (11) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addptslist(S, k, v);
    }
  }

  // POINTS DE JONCTION
  fscanf(fd, "%s", buf);
  if ((strncmp(buf, "junc", 4) != 0) && (strncmp(buf, "JUNC", 4) != 0))
  {
    fprintf(stderr, "%s: bad file format (12) : %s\n", F_NAME, buf);
    return NULL;
  }
  fscanf(fd, "%d", &nbjunc);
  S->e_junc = nbisol+nbend+nbcurv+nbjunc;

#ifdef DEBUG
  printf("jonctions : %d\n", nbjunc);
#endif

  for (i = 0; i < nbjunc; i++)
  {
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "vertex", 6) != 0)
    {
      fprintf(stderr, "%s: bad file format (13) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &k);
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "adj", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (14) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addadjlist(S, k, v);
    }
    fscanf(fd, "%s", buf);
    if (strncmp(buf, "pts", 3) != 0)
    {
      fprintf(stderr, "%s: bad file format (15) : %s\n", F_NAME, buf);
      return NULL;
    }
    fscanf(fd, "%d", &n);
    for (j = 0; j < n; j++)
    {
      fscanf(fd, "%d", &v);
      addptslist(S, k, v);
    }
  }

  fclose(fd);
  return S;
} /* readskel() */

/* ====================================================================== */
skel * initskel(uint32_t rs, uint32_t cs, uint32_t ds, uint32_t nbvertex, uint32_t nbptcell, uint32_t nbadjcell, int32_t connex)
/* ====================================================================== */
{
#undef F_NAME
#define F_NAME "initskel"
  skel * S;
  uint32_t i;
  
#ifdef DEBUG
  DBGIM = allocimage(NULL, rs, cs, ds, VFF_TYP_1_BYTE);
  razimage(DBGIM);
#endif

  S = (skel *)calloc(1,sizeof(skel));
  if (S == NULL)
  {   fprintf(stderr, "%s: calloc failed for S\n", F_NAME);
      return(0);
  }
 
  S->tptcell = (SKC_pt_cell *)calloc(nbptcell, sizeof(SKC_pt_cell));
  if (S->tptcell == NULL)
  {   fprintf(stderr, "%s: calloc failed for S->tptcell\n", F_NAME);
      return(0);
  }
 
  S->tadjcell = (SKC_adj_cell *)calloc(nbadjcell, sizeof(SKC_adj_cell));
  if (S->tadjcell == NULL)
  {   fprintf(stderr, "%s: calloc failed for S->tadjcell\n", F_NAME);
      return(0);
  }

  S->connex = connex;
  S->rs = rs;
  S->cs = cs;
  S->ds = ds;
  S->freeptcell = 0;
  S->freeadjcell = 0;
  S->nbptcell = nbptcell;
  S->nbadjcell = nbadjcell;
 
  S->tskel = (skelpart *)calloc(nbvertex, sizeof(skelpart));
  if (S->tskel == NULL)
  {   fprintf(stderr, "%s: calloc failed for S->tskel\n", F_NAME);
      return(0);
  }

  for (i = 0; i < nbvertex; i++) 
  {
    S->tskel[i].pts = NULL;
    S->tskel[i].adj = NULL;
    S->tskel[i].tag = 0;
  }

  return S;
} /* initskel() */

/* ====================================================================== */
void termineskel(skel * S)
/* ====================================================================== */
{
  free(S->tptcell);
  free(S->tadjcell);
  free(S->tskel);
  free(S);
#ifdef DEBUG
  writeimage(DBGIM, "_DBGIM");
  printf("TERMINESKEL\n");
#endif
} /* termineskel() */

/* ====================================================================== */
uint32_t nb_adjacent_elts(skel * S, uint32_t i)
/* ====================================================================== */
// returns the number of adjacent elements
{
  SKC_adj_pcell p;
  uint32_t nadj = 0;
  for (p = S->tskel[i].adj; p != NULL; p = p->next) nadj++;
  return nadj;
} // nb_adjacent_elts()

/* ====================================================================== */
void skeldelete(skel * S, uint32_t i)
/* ====================================================================== */
// Remove element i from skeleton S.
// CAUTION: some properties of the initial skeleton structure 
// will not be guaranteed in the result, for example a junction 
// can be left with less than 3 adjacent elements.
{
#undef F_NAME
#define F_NAME "skeldelete"
  SKC_adj_pcell p;
  uint32_t j;

#ifdef DEBUG_skeldelete
  //  printf("%s: begin\n", F_NAME);
#endif

  p = S->tskel[i].adj;
  if (IS_ISOL(i)) 
  {
#ifdef DEBUG_skeldelete
    printf("%s: isolated i=%d\n", F_NAME, i);
#endif
    assert(p == NULL);
    SK_REMOVE(i);
  }
  else if (IS_END(i)) 
  {
    assert(p != NULL);
    assert(p->next == NULL); // un seul voisin
    j = p->val;
    assert(!IS_CURV(j) || SK_DELETED(j));
    removeadjlist(S, j, i); // remove i from the adjacency list of j
    SK_REMOVE(i);
#ifdef DEBUG_skeldelete
    printf("%s: end i=%d, j=%d\n", F_NAME, i, j);
#endif
  }
  else if (IS_CURV(i)) 
  {
    if (p == NULL) // si arc fermé
    {
#ifdef DEBUG_skeldelete
    printf("%s: curv (closed) i=%d\n", F_NAME, i);
#endif
      SK_REMOVE(i);
    }
    else
    {
      uint32_t e1, e2;
      assert(p->next != NULL); // soit 0, soit 2 adjacences
      e1 = p->val;
      e2 = p->next->val;
      removeadjlist(S, e1, i); // remove i from the adjacency list of e1
      removeadjlist(S, e2, i); // remove i from the adjacency list of e2
#ifdef DEBUG_skeldelete
      printf("%s: curv i=%d, e1=%d(t=%d), e2=%d(t=%d)\n", F_NAME, i, e1, S->tskel[e1].tag, e2, S->tskel[e2].tag);
#endif
      SK_REMOVE(i);
      if (IS_END(e1)) SK_REMOVE(e1);
      if (IS_END(e2)) SK_REMOVE(e2);
    }
  }
  else // IS_JUNC(i)
  {
    SKC_adj_pcell pp;
    uint32_t nadj = nb_adjacent_elts(S, i);;
    if (nadj == 0)
    {
#ifdef DEBUG_skeldelete
    printf("%s: junc (0 neibrs) i=%d\n", F_NAME, i);
#endif
      SK_REMOVE(i);
    }
    else
    {
      SKC_pt_pcell pt;
      uint32_t a1, a2, v2;
      assert(nadj == 2); // do not remove junction otherwise
      a1 = p->val;
      a2 = p->next->val;
#ifdef DEBUG_skeldelete
      printf("%s: junc (2 neibrs) i=%d, a1=%d, a2=%d\n", F_NAME, i, a1, a2);
#endif
      // transfer all points from a2 and i to a1
      pt = S->tskel[a1].pts;
      assert(pt != NULL);
      while (pt->next != NULL) pt = pt->next;
      pt->next = S->tskel[i].pts;
      while (pt->next != NULL) pt = pt->next;
      pt->next = S->tskel[a2].pts;

      // update adjacencies
      pp = S->tskel[a2].adj;
      assert(pp != NULL); assert(pp->next != NULL);
      assert((pp->val == i) || (pp->next->val == i));
      if (pp->val == i) v2 = pp->next->val; else v2 = pp->val;
      for (pp = S->tskel[v2].adj; pp != NULL; pp = pp->next)
	if (pp->val == a2) { pp->val = a1; break; }
      assert(pp != NULL);

      pp = S->tskel[a1].adj;
      assert(pp != NULL); assert(pp->next != NULL);
      assert((pp->val == i) || (pp->next->val == i));
      if (pp->val == i) pp->val = v2; else pp->next->val = v2;

      // remove a2 and i
      SK_REMOVE(a2);
      SK_REMOVE(i);
    }
  } // else // IS_JUNC(i)

} //skeldelete()

#ifdef TESTSKEL
int main()
{
  skel * S;
  // 0225336441 adj
  // eccjccjcce type pts
  // 0123456789 pts
  S = initskel(10, 1, 1, 7, 20, 20, 8);
  S->e_isol = 0;
  S->e_end = 2;
  S->e_curv = 5;
  S->e_junc = 7;

  addptslist(S, 0, 0);
  addptslist(S, 2, 1);
  addptslist(S, 2, 2);
  addptslist(S, 5, 3);
  addptslist(S, 3, 4);
  addptslist(S, 3, 5);
  addptslist(S, 6, 6);
  addptslist(S, 4, 7);
  addptslist(S, 4, 8);
  addptslist(S, 1, 9);

  addadjlist(S, 0, 2);
  addadjlist(S, 2, 0);
  addadjlist(S, 2, 5);
  addadjlist(S, 5, 2);
  addadjlist(S, 5, 3);
  addadjlist(S, 3, 5);
  addadjlist(S, 3, 6);
  addadjlist(S, 6, 3);
  addadjlist(S, 6, 4);
  addadjlist(S, 4, 6);
  addadjlist(S, 4, 1);
  addadjlist(S, 1, 4);

  printskel(S);

  skeldelete(S, 5);

  printf("----------\n");
  printskel(S);

  skeldelete(S, 3);

  printf("----------\n");
  printskel(S);

  termineskel(S);

  return 0;
} /* main() */
#endif
