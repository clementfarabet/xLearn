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
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MCIMAGE_H
#include <mcimage.h>
#endif

// nombre maximum de sommets par face (polygone)
#define MCP_MAXVERTFACE 50

// nombre maximum de sommets intermédiaires par edge (subdivision)
#define MCP_MAXVERTEDGE 25

// nombre maximum de faces adjacentes a un sommet
#define MCP_MAXADJFACES 15

typedef struct {
  double x, y, z;    /* coordonnees */
  int32_t nfaces;
  index_t face[MCP_MAXADJFACES]; /* indices des faces adjacentes (pas plus de MCP_MAXADJFACES) */
} mcpvertex;

typedef struct {
  int32_t n;         /* nombre de sommets adjacents */
  int32_t na;         /* nombre de sommets auxiliaires adjacents */
  index_t vert[MCP_MAXVERTFACE];   /* indices des sommets adjacents */
} mcpface;

typedef struct {
  index_t v1, v2;    /* indices des sommets adjacents */
  int32_t n;         /* nombre de sommets dans la subdivision */
  index_t vert[MCP_MAXVERTEDGE];   /* indices des sommets de la subdivision */
} mcpedge;

typedef struct {
  int32_t max;       /* taille max du tableau de sommets */
  int32_t cur;       /* taille courante du tableau de sommets */
  int32_t nsp;       /* nombre de sommets "principaux" (hors subdivision) */
  mcpvertex v[1];    /* tableau des elements physiques */
} mcptabvertices;

typedef struct {
  int32_t max;       /* taille max du tableau de faces */
  int32_t cur;       /* taille courante du tableau de faces */
  mcpface f[1];      /* tableau des elements physiques */
} mcptabfaces;

typedef struct {
  int32_t max;       /* taille max du tableau de cotes */
  int32_t cur;       /* taille courante du tableau de cotes */
  mcpedge e[1];      /* tableau des elements physiques */
} mcptabedges;

typedef struct {
  mcptabvertices *Vertices;
  mcptabfaces *Faces;
  mcptabedges *Edges;
  Rbt * RBT;
} MCP;

/* ==================================== */
/* prototypes */
/* ==================================== */

extern mcptabvertices * MCP_AllocVertices(int32_t taillemax);
extern mcptabfaces * MCP_AllocFaces(int32_t taillemax);
extern mcptabedges * MCP_AllocEdges(int32_t taillemax);
extern void MCP_ReAllocVertices(mcptabvertices **A);
extern void MCP_ReAllocFaces(mcptabfaces **A);
extern void MCP_ReAllocEdges(mcptabedges **A);
extern MCP * MCP_Init(int32_t taillemax);
extern void MCP_Termine(MCP *P);
extern int32_t MCP_AddVertex(MCP *P, double x, double y, double z);
extern int32_t MCP_AddAuxVertex(MCP *P, double x, double y, double z);
extern int32_t MCP_AddFace(MCP *P, Liste *Face);
extern void MCP_ComputeFaces(MCP *P);
extern void MCP_ComputeEdges(MCP *P);
extern void MCP_SubdivEdges(MCP *P, double param);
extern void MCP_GetPolyPoints(MCP *P, int32_t indface, int32_t *pi, double *px, double *py, double *pz, int32_t *n);
extern void MCP_Print(MCP *P);
extern void MCP_SaveVTK(MCP *P, FILE *fileout);
extern MCP * MCP_LoadVTK(FILE *fileout);
#ifdef __cplusplus
}
#endif
