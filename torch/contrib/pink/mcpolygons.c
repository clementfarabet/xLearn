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
  Gestion d'un maillage polygonal
  Michel Couprie  -  décembre 2008
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mcrbt.h>
#include <mcliste.h>
#include <mcpolygons.h>
#include <assert.h>

/* ==================================== */
mcptabvertices * MCP_AllocVertices(int32_t taillemax)
/* ==================================== */
#undef F_NAME
#define F_NAME "MCP_AllocVertices"
{
  mcptabvertices * T = (mcptabvertices *)calloc(1,sizeof(mcptabvertices) + taillemax*sizeof(mcpvertex));
  if (T == NULL) 
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  T->max = taillemax;
  T->cur = 0;
  return T;
} /* MCP_AllocVertices() */

/* ==================================== */
void MCP_ReAllocVertices(mcptabvertices **A)
/* ==================================== */
{
  int32_t taillemax;
  mcptabvertices * T, *Tmp;

  //printf("ReAllocVertices: ancienne taille %d nouvelle taille %d\n", (*A)->max, 2 * (*A)->max);

  taillemax = 2 * (*A)->max;  /* alloue le double de l'ancienne taille */ 
  T = MCP_AllocVertices(taillemax);
  T->cur = (*A)->cur;
  T->nsp = (*A)->nsp;
  memcpy(T->v, (*A)->v, T->cur * sizeof(mcpvertex));
  Tmp = *A;
  *A = T;
  free(Tmp);
} /* MCP_ReAllocVertices() */

/* ==================================== */
mcptabfaces * MCP_AllocFaces(int32_t taillemax)
/* ==================================== */
#undef F_NAME
#define F_NAME "MCP_AllocFaces"
{
  mcptabfaces * T = (mcptabfaces *)calloc(1,sizeof(mcptabfaces) + taillemax*sizeof(mcpface));
  if (T == NULL) 
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  T->max = taillemax;
  T->cur = 0;
  return T;
} /* MCP_AllocFaces() */

/* ==================================== */
void MCP_ReAllocFaces(mcptabfaces **A)
/* ==================================== */
{
  int32_t taillemax;
  mcptabfaces * T, *Tmp;

  //printf("ReAllocFaces: ancienne taille %d nouvelle taille %d\n", (*A)->max, 2 * (*A)->max);

  taillemax = 2 * (*A)->max;  /* alloue le double de l'ancienne taille */ 
  T = MCP_AllocFaces(taillemax);
  T->cur = (*A)->cur;
  memcpy(T->f, (*A)->f, T->cur * sizeof(mcpface));
  Tmp = *A;
  *A = T;
  free(Tmp);
} /* MCP_ReAllocFaces() */

/* ==================================== */
mcptabedges * MCP_AllocEdges(int32_t taillemax)
/* ==================================== */
#undef F_NAME
#define F_NAME "MCP_AllocEdges"
{
  mcptabedges * T = (mcptabedges *)calloc(1,sizeof(mcptabedges) + taillemax*sizeof(mcpedge));
  if (T == NULL) 
  {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    exit(0);
  }
  T->max = taillemax;
  T->cur = 0;
  return T;
} /* MCP_AllocEdges() */

/* ==================================== */
void MCP_ReAllocEdges(mcptabedges **A)
/* ==================================== */
{
  int32_t taillemax;
  mcptabedges * T, *Tmp;

  //printf("ReAllocEdges: ancienne taille %d nouvelle taille %d\n", (*A)->max, 2 * (*A)->max);

  taillemax = 2 * (*A)->max;  /* alloue le double de l'ancienne taille */ 
  T = MCP_AllocEdges(taillemax);
  T->cur = (*A)->cur;
  memcpy(T->e, (*A)->e, T->cur * sizeof(mcpedge));
  Tmp = *A;
  *A = T;
  free(Tmp);
} /* MCP_ReAllocEdges() */

/* ==================================== */
MCP * MCP_Init(int32_t taillemax)
/* ==================================== */
{
  MCP *P;
  P = (MCP *)malloc(sizeof(MCP));
  P->Vertices = MCP_AllocVertices(taillemax);
  P->Faces = MCP_AllocFaces(taillemax);
  P->Edges = NULL;
  P->RBT = mcrbt_CreeRbtVide(taillemax);
  if ((P == NULL) || (P->Vertices == NULL) || (P->Faces == NULL) || (P->RBT == NULL))
    exit(0);
  return P;
} /* MCP_Init() */

/* ==================================== */
void MCP_Termine(MCP *P)
/* ==================================== */
{
  free(P->Vertices);
  free(P->Faces);
  if (P->Edges) free(P->Edges);
  mcrbt_RbtTermine(P->RBT);
  free(P);
} /* MCP_Termine() */

/* ==================================== */
int32_t MCP_AddVertex(MCP *P, double x, double y, double z)
/* ==================================== */
/* 
   modifie la var. globale MCP_Vertices 
*/
#undef F_NAME
#define F_NAME "MCP_AddVertex"
{
  index_t i;
  if (P->Vertices->cur >= P->Vertices->max) MCP_ReAllocVertices(&P->Vertices);
  i = P->Vertices->cur;
  P->Vertices->nsp += 1;
  P->Vertices->cur += 1;
  P->Vertices->v[i].x = x;
  P->Vertices->v[i].y = y;
  P->Vertices->v[i].z = z;
  return i;
} /* MCP_AddVertex() */

/* ==================================== */
int32_t MCP_AddAuxVertex(MCP *P, double x, double y, double z)
/* ==================================== */
/* 
   modifie la var. globale MCP_Vertices 
*/
#undef F_NAME
#define F_NAME "MCP_AddAuxVertex"
{
  index_t i;
  if (P->Vertices->cur >= P->Vertices->max) MCP_ReAllocVertices(&P->Vertices);
  i = P->Vertices->cur;
  P->Vertices->cur += 1;
  P->Vertices->v[i].x = x;
  P->Vertices->v[i].y = y;
  P->Vertices->v[i].z = z;
  return i;
} /* MCP_AddAuxVertex() */

/* ==================================== */
int32_t MCP_AddFace(MCP *P, Liste *Face)
/* ==================================== */
/* modifie la var. globale MCP_Faces */
{
  int32_t i, n;
  index_t indface;
  if (P->Faces->cur >= P->Faces->max) MCP_ReAllocFaces(&P->Faces);
  indface = P->Faces->cur;
  P->Faces->cur += 1;
  n = ListeTaille(Face);
  if (n > MCP_MAXVERTFACE)
  {
    fprintf(stderr, "%s : too many vertices in a face (max %d)\n", F_NAME, MCP_MAXVERTFACE);
    exit(0);
  }
  P->Faces->f[indface].n = n; 
  for (i = 0; i < n; i++)
    P->Faces->f[indface].vert[i] = ListeElt(Face, i);
  return indface;
} /* MCP_AddFace() */

/* ==================================== */
void MCP_ComputeFaces(MCP *P)
/* ==================================== */
/*
  Met à jour l'information "faces adjacentes" des vertices 
*/
#undef F_NAME
#define F_NAME "MCP_ComputeFaces"
{
  int32_t i, j, k;
  mcpface F;
  int32_t nvert = P->Vertices->cur;
  int32_t nface = P->Faces->cur;

  for (k = 0; k < nvert; k++)
    P->Vertices->v[k].nfaces = 0;

  for (i = 0; i < nface; i++)
  {
    F = P->Faces->f[i];
    assert(F.n > 2);
    for (j = 0; j < F.n; j++)
    {
      k = F.vert[j]; 
      assert(P->Vertices->v[k].nfaces < MCP_MAXADJFACES);
      P->Vertices->v[k].face[ P->Vertices->v[k].nfaces++ ] = i;
    } /* for j */
  } /* for i */
} /* MCP_ComputeFaces() */

/* ==================================== */
int32_t MCP_AddEdge(MCP *P, index_t v1, index_t v2)
/* ==================================== */
/* modifie la var. globale MCP_Edges */
{
  int32_t indedge;
  if (P->Edges->cur >= P->Edges->max) MCP_ReAllocEdges(&P->Edges);
  indedge = P->Edges->cur;
  P->Edges->cur += 1;
  P->Edges->e[indedge].v1 = v1;
  P->Edges->e[indedge].v2 = v2;
  return indedge;
} /* MCP_AddEdge() */

/* ==================================== */
void MCP_ComputeEdges(MCP *P)
/* ==================================== */
/*
  Construit le tableau des cotes (edges)
*/
#undef F_NAME
#define F_NAME "MCP_ComputeEdges"
{
  int32_t i, j, nfaces, a, b, e;
  mcpface F;
  int32_t N = P->Vertices->cur;

  if (P->Edges == NULL) 
  {
    fprintf(stderr, "%s : Edges array must be allocated\n", F_NAME);
    exit(0);
  }  
  nfaces = P->Faces->cur;
  for (i = 0; i < nfaces; i++)
  {
    F = P->Faces->f[i];
    assert(F.n > 2);
    for (j = 0; j < F.n; j++)
    {
      a = F.vert[j]; 
      b = F.vert[(j+1)%F.n];
      if (RbtSearch(P->RBT, (TypRbtKey)(a*N+b)) == P->RBT->nil)
      {
	e = MCP_AddEdge(P, a, b);
	(void)mcrbt_RbtInsert(&P->RBT, (TypRbtKey)(a*N+b), (TypRbtAuxData)e);
	(void)mcrbt_RbtInsert(&P->RBT, (TypRbtKey)(b*N+a), (TypRbtAuxData)e);
      }
    } /* for j */
  } /* for i */
} /* MCP_ComputeEdges() */

/* ==================================== */
void MCP_SubdivEdges(MCP *P, double param)
/* ==================================== */
/*
  Subdivise les cotes (edges)
*/
#undef F_NAME
#define F_NAME "MCP_SubdivEdges"
{
  int32_t i, j, a, b, ns, v;
  mcpvertex A, B;
  mcpface F;
  mcpedge E;
  RbtElt * re;
  double x, y, z, len;

  if (P->Vertices->cur > P->Vertices->nsp)
  {
    fprintf(stderr, "%s: warning already subdivided\n", F_NAME);
    return;
  }

  if (P->Edges == NULL) return;
  for (i = 0; i < P->Edges->cur; i++)
  {
    a = P->Edges->e[i].v1;
    b = P->Edges->e[i].v2;
    A = P->Vertices->v[a]; 
    B = P->Vertices->v[b]; 
    x = (double)A.x - B.x; y = (double)A.y - B.y; z = (double)A.z - B.z;
    len = sqrt(x*x + y*y + z*z);
    ns = floor(len / param);
    if (ns > 1)
    { 
      if ((ns - 1) > MCP_MAXVERTEDGE)
      {
	fprintf(stderr, "%s : too many vertices in edge (max %d)\n", F_NAME, MCP_MAXVERTEDGE);
	exit(0);
      }
      // ns est le nombre de segments de la subdivision
      // ns-1 est le nombre de points (hormis a et b)
      P->Edges->e[i].n = ns-1;
      for (j = 1; j < ns; j++)
      {
	x = (((ns - j) * A.x) + ((j) * B.x)) / (ns);
	y = (((ns - j) * A.y) + ((j) * B.y)) / (ns);
	z = (((ns - j) * A.z) + ((j) * B.z)) / (ns);
	v = MCP_AddAuxVertex(P, x, y, z);
	P->Edges->e[i].vert[j-1] = v;
      }
    } // if (n != 0)
  } // for (i = 0; i < P->Edges->cur; i++)

  // pour l'information sur les faces
  for (i = 0; i < P->Faces->cur; i++)
  {
    P->Faces->f[i].na = 0;
    F = P->Faces->f[i];
    for (j = 0; j < F.n; j++)
    {
      a = F.vert[j]; 
      b = F.vert[(j+1)%F.n];
      re = RbtSearch(P->RBT, (TypRbtKey)(a*P->Vertices->nsp+b));
      if (re != P->RBT->nil)
      {
	E = P->Edges->e[re->auxdata];
	P->Faces->f[i].na += E.n;
      }
    } /* for j */
  } /* for i */
} /* MCP_SubdivEdges() */

/* ==================================== */
void MCP_SaveVTK(MCP *P, FILE *fileout)
/* ==================================== */
/* fileout doit avoir ete ouvert en ecriture */
/* format: 
POINTS %d float   // Sommets - champ obligatoire
%g %g %g          // coord. vertex
   ...
POLYGONS %d %d    // Faces - champ obligatoire 
                  // arg1: nb polygones; arg2: nb valeurs (=4*arg1 pour des triangles)
%d %d %d %d       // face (nb vert suivi de indices vert)
   ...
*/
{
  int32_t i, j, k, N, nsp;
  index_t a, b;
  RbtElt * re;
  mcpface F;
  mcpedge E;

  nsp = P->Vertices->nsp;

  fprintf(fileout, "# vtk DataFile Version 3.0\n");
  fprintf(fileout, "MCP_SaveVTK output\n");
  fprintf(fileout, "ASCII\n");
  fprintf(fileout, "DATASET POLYDATA\n");

  // SOMMETS
  fprintf(fileout, "POINTS %d float\n", P->Vertices->cur);
  for (i = 0; i < P->Vertices->cur; i++)
  {
    fprintf(fileout, "%g %g %g", P->Vertices->v[i].x, P->Vertices->v[i].y, P->Vertices->v[i].z);
    fprintf(fileout, "\n");
  }
  fprintf(fileout, "\n");


  // FACES
  N=0;
  j=0;
  for (i = 0; i < P->Faces->cur; i++)
  {
    if (P->Faces->f[i].n > 0)
    {
      F = P->Faces->f[i];
      j++;
      N += F.n + F.na;
    }
  }
  fprintf(fileout, "POLYGONS %d %d\n", j, j + N);

  for (i = 0; i < P->Faces->cur; i++)
  {
    F = P->Faces->f[i];
    if (F.n > 0)
    {
      fprintf(fileout, "%d ", F.n + F.na);
      for (j = 0; j < F.n; j++)
      {
	a = F.vert[j]; 
#ifdef MC_64_BITS
	fprintf(fileout, "%lld ", a);
#else
	fprintf(fileout, "%d ", a);
#endif
	b = F.vert[(j+1)%F.n];
	re = RbtSearch(P->RBT, (TypRbtKey)(a*nsp+b));
	if (re != P->RBT->nil)
	{
	  E = P->Edges->e[re->auxdata];
	  if (a == E.v1)
	  {
	    for (k = 0; k < E.n; k++)
#ifdef MC_64_BITS
	      fprintf(fileout, "%lld ", E.vert[k]);
#else
	      fprintf(fileout, "%d ", E.vert[k]);
#endif
	  } 
	  else
	  {
	    for (k = E.n - 1; k >= 0; k--)
#ifdef MC_64_BITS
	      fprintf(fileout, "%lld ", E.vert[k]);
#else
	      fprintf(fileout, "%d ", E.vert[k]);
#endif
	  } /* for k */ 
	} /* if (re != P->RBT->nil) */
      } /* for j */ 
      fprintf(fileout, "\n");
    } /* if (F.n > 0) */
  } /* for i */ 
} /* MCP_SaveVTK() */

/* ==================================== */
void MCP_GetPolyPoints(MCP *P, int32_t indface, int32_t *pi, double *px, double *py, double *pz, int32_t *n)
/* ==================================== */
/*
Extrait la liste des points d'un polygone
entrées : 
  P : un maillage polygonal
  indface : l'index du polygone considéré
  *n : la taille allouee pour les tableaux pi,px,py,pz

sorties : 
  pi : tableau des indices (dans P) des points trouvés
  px,py,pz : tableau des coordonnées des points trouvés
  *n : le nombre de points transmis  
*/
{
  int32_t i, j, k, a, b, nsp, taille = *n;
  RbtElt * re;
  mcpface F;
  mcpedge E;

  nsp = P->Vertices->nsp;
  i = 0;
  F = P->Faces->f[indface];
  for (j = 0; j < F.n; j++)
  {
    a = F.vert[j]; 
    assert(i < taille);
    pi[i] = a;     
    px[i] = P->Vertices->v[a].x; 
    py[i] = P->Vertices->v[a].y; 
    pz[i] = P->Vertices->v[a].z; 
    i++;
    b = F.vert[(j+1)%F.n];
    re = RbtSearch(P->RBT, (TypRbtKey)(a*nsp+b));
    if (re != P->RBT->nil)
    {
      E = P->Edges->e[re->auxdata];
      if (a == E.v1)
	for (k = 0; k < E.n; k++)
        { 
	  assert(i < taille);
	  pi[i] = E.vert[k];
	  px[i] = P->Vertices->v[E.vert[k]].x; 
	  py[i] = P->Vertices->v[E.vert[k]].y; 
	  pz[i] = P->Vertices->v[E.vert[k]].z; 
	  i++; 
	}
      else
	for (k = E.n - 1; k >= 0; k--)
        { 
	  assert(i < taille);
	  pi[i] = E.vert[k];
	  px[i] = P->Vertices->v[E.vert[k]].x; 
	  py[i] = P->Vertices->v[E.vert[k]].y; 
	  pz[i] = P->Vertices->v[E.vert[k]].z; 
	  i++; 
	}
    } /* if (re != P->RBT->nil) */
  } /* for j */
  *n = i;
} /* MCP_GetPolyPoints() */

/* ==================================== */
MCP * MCP_LoadVTK(FILE *filein)
/* ==================================== */
/* fileout doit avoir ete ouvert en lecture */
/* L'appel à MCP_Init est inclus */
/* format: 
POINTS %d float   // Sommets - champ obligatoire
%g %g %g          // coord. vertex
   ...
POLYGONS %d %d    // Faces - champ obligatoire 
                  // arg1: nb polygones; arg2: nb valeurs (=4*arg1 pour des triangles)
%d %d %d %d       // face (nb vert suivi de indices vert)
   ...
*/
#undef F_NAME
#define F_NAME "MCP_LoadVTK"
{
  MCP *P;
  int32_t i, j, nvert=-1, nfaces=-1, nf, indx;
  double x, y, z;
#define sbuf 1024
  char buf[sbuf];
  char *ret;
  Liste * Face = NULL;
  while (1)
  {
    do { ret=fgets(buf, sbuf, filein); }
    while (ret && strncmp(buf,"POINTS",6) && strncmp(buf,"POLYGONS",8));
    if (ret == NULL) goto end;
    if (strncmp(buf,"POINTS",6) == 0)
    {
      sscanf(buf+7, "%d", &nvert);
#ifdef VERBOSE
      printf("%s: loading %d points\n", F_NAME, nvert);
#endif
      P = MCP_Init(nvert);
      Face = CreeListeVide(nvert);
      for (i = 0; i < nvert; i++)
      {
        fscanf(filein, "%lf%lf%lf", &x, &y, &z);
        MCP_AddVertex(P, x, y, z);
      } // for i
    }
    else if (strncmp(buf,"POLYGONS",8) == 0)
    {
      assert(Face != NULL);
      sscanf(buf+9, "%d", &nfaces);
#ifdef VERBOSE
      printf("%s: loading %d faces\n", F_NAME, nfaces);
#endif
      for (i = 0; i < nfaces; i++)
      {
        fscanf(filein, "%d", &nf);
	ListeFlush(Face);
	for (j = 0; j < nf; j++)
	{
	  fscanf(filein, "%d", &indx);
	  ListePush(Face, indx);
	}
        MCP_AddFace(P, Face);
      }
    }
  } // while (1)

 end:
  if ((nvert == -1) || (nfaces == -1))
  {
    fprintf(stderr, "%s: bad file format\n", F_NAME);
    exit(0);
  }
  ListeTermine(Face);
  return P;
} /* MCP_LoadVTK() */

/* ==================================== */
void MCP_Print(MCP *P)
/* ==================================== */
{
  int32_t i, j;
  printf(" ========== VERTICES ===========\n");
  for (i = 0; i < P->Vertices->cur; i++)
  {
    printf("v[%d]: x=%g, y=%g, z=%g", i, 
           P->Vertices->v[i].x, P->Vertices->v[i].y, P->Vertices->v[i].z);
    printf("  faces "); 
    for (j = 0; j < P->Vertices->v[i].nfaces; j++)
#ifdef MC_64_BITS
      printf("%lld  ", P->Vertices->v[i].face[j]);
#else
      printf("%d  ", P->Vertices->v[i].face[j]);
#endif
    printf("\n");
  }
  printf(" ============ FACES ===========\n");
  for (i = 0; i < P->Faces->cur; i++)
  {
    printf("f[%d]: [ ", i);
    for (j = 0; j < P->Faces->f[i].n; j++)
#ifdef MC_64_BITS
      printf("%lld ", P->Faces->f[i].vert[j]);
#else
      printf("%d ", P->Faces->f[i].vert[j]);
#endif
    printf("]\n");
  }
  if (P->Edges)
  {
    mcpedge E;
    printf(" ============ EDGES ===========\n");
    for (i = 0; i < P->Edges->cur; i++)
    {
      E = P->Edges->e[i];
#ifdef MC_64_BITS
      printf("e[%d]: %lld, %lld (subd: %d)\n", i, E.v1, E.v2, E.n);
#else
      printf("e[%d]: %d, %d (subd: %d)\n", i, E.v1, E.v2, E.n);
#endif
      if (E.n)
      {
	printf("  subdiv: ");
	for (j = 0; j < E.n; j++)	
#ifdef MC_64_BITS
	  printf("%lld ", E.vert[j]);
#else
	  printf("%d ", E.vert[j]);
#endif
	printf("\n");
      }
    }
  }
} /* MCP_Print() */

#ifdef DEBUG
int32_t main()
{
  char r[80];
  Liste *L = CreeListeVide(100);
  int32_t a, b, c, d, e, f, g;
  FILE *fd;
  MCP *P = MCP_Init(1);

  a = MCP_AddVertex(P, 0, 0, 0); ListePush(L, a);
  b = MCP_AddVertex(P, 0, 1, 0); ListePush(L, b);
  c = MCP_AddVertex(P, 1, 1, 0); ListePush(L, c);
  d = MCP_AddVertex(P, 1, 0, 0); ListePush(L, d);
  (void)MCP_AddFace(P, L);

  ListeFlush(L);
  ListePush(L, c); ListePush(L, d);
  e = MCP_AddVertex(P, 2, 0, 0); ListePush(L, e);
  f = MCP_AddVertex(P, 2, 1, 0); ListePush(L, f);
  (void)MCP_AddFace(P, L);

  P->Edges = MCP_AllocEdges(1);
  MCP_ComputeEdges(P);
  MCP_ComputeFaces(P);
  do
  {
    printf("commande (qUIT, sUBDIVIDE, SaVE pRINT)\n");
    scanf("%s", r);
    switch (r[0])
    {
      case 'a':
	fd = fopen("toto.vtk","w");
	MCP_SaveVTK(P, fd);
	fclose(fd);
        break;
      case 's':
	MCP_SubdivEdges(P, 0.5);
	MCP_ComputeFaces(P);
        break;
      case 'p': MCP_Print(P); break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  MCP_Termine(P);
}
#endif
