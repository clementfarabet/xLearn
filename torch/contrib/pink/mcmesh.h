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
#define MCM_MAXADJFACES 25
#define MCM_MAXADJEDGES 25
#define MCM_MAXADJFACESEDGE 10

typedef struct {
  double x, y, z;    /* coordonnees */
  double xp, yp, zp; /* coordonnees bis (utilisees aussi pour stocker la normale) */
  double xo, yo, zo; /* coordonnees ter (pour memoriser la position originale) */
                     // utilisees seulement par RegulMeshHC
  int32_t nfaces;
  int32_t face[MCM_MAXADJFACES]; /* indices des faces adjacentes (pas plus de MCM_MAXADJFACES) */
  int32_t nedges;
  int32_t edge[MCM_MAXADJEDGES]; /* indices des cotes adjacents (pas plus de MCM_MAXADJEDGES) */

  float curv1, curv2; // Pour les courbures
  uint8_t red, green, blue; // pour la couleur
  int32_t aux;
} meshvertex;

typedef struct {
  int32_t vert[3];   /* indices des sommets adjacents */
  double xn, yn, zn; /* normale a la face */
  int32_t aux;
} meshface;

typedef struct {
  int32_t v1, v2;        /* indices des sommets adjacents */
  int32_t f1, f2;        /* indices des faces adjacentes */
  double curv;       /* pour stocker la courbure (angle entre -pi et pi) */
} meshedge2;

typedef struct {
  int32_t v1, v2;        /* indices des sommets adjacents */
  int32_t nfaces;
  int32_t face[MCM_MAXADJFACESEDGE]; /* indices des faces adjacentes (pas plus de MCM_MAXADJFACESEDGE) */
} meshedge;

typedef struct {
  int32_t max;        /* taille max du tableau de sommets */
  int32_t cur;        /* taille courante du tableau de sommets */
  uint8_t *lab;       /* tableau de labels associes aux sommets */
  uint8_t *tmp;       /* tableau de valeurs associes aux sommets */
  meshvertex v[1];    /* tableau des elements physiques */
} meshtabvertices;

typedef struct {
  int32_t max;         /* taille max du tableau de faces */
  int32_t cur;         /* taille courante du tableau de faces */
  meshface f[1];       /* tableau des elements physiques */
} meshtabfaces;

typedef struct {
  int32_t max;         /* taille max du tableau de cotes */
  int32_t cur;         /* taille courante du tableau de cotes */
  meshedge2 e[1];      /* tableau des elements physiques */
} meshtabedges2;

typedef struct {
  int32_t max;         /* taille max du tableau de cotes */
  int32_t cur;         /* taille courante du tableau de cotes */
  meshedge e[1];       /* tableau des elements physiques */
} meshtabedges;

typedef struct {
  int32_t nvert;       /* nombre de sommets */
  int32_t nedge;       /* nombre de cotes */
  int32_t *lastneigh;  /* tableau des index des derniers successeurs */
  int32_t *neigh;      /* tableau des successeurs */
} meshtablinks;

/* boite englobante */
typedef struct {
  double bxmin;
  double bxmax;
  double bymin;
  double bymax;
  double bzmin;
  double bzmax;
} meshbox;

/* structure principale pour un mesh */
typedef struct {
  meshtabvertices *Vertices;
  meshtabfaces *Faces;
  meshtabedges *Edges;
  meshtablinks *Links;
  Rbtp * RBTP;
} MCM;

extern meshtabvertices *Vertices;
extern meshtabfaces *Faces;
extern meshtabedges2 *Edges;
extern meshtablinks *Links;

/* ==================================== */
/* prototypes */
/* ==================================== */

extern meshtabvertices * MCM_AllocVertices(int32_t taillemax);
extern meshtabfaces * MCM_AllocFaces(int32_t taillemax);
extern meshtabedges * MCM_AllocEdges(int32_t taillemax);
extern meshtablinks * MCM_AllocLinks(int32_t nvert, int32_t nedge);
extern void MCM_ReAllocVertices(meshtabvertices **A);
extern void MCM_ReAllocFaces(meshtabfaces **A);
extern void MCM_ReAllocEdges(meshtabedges **A);
extern MCM * MCM_Init(int32_t taillemax);
extern void MCM_Termine(MCM *Mesh);
extern int32_t MCM_AddVertexStraight(
  MCM *M, double x, double y, double z, int32_t indface
);
extern int32_t MCM_AddVertexStraight2(
  MCM *M, double x, double y, double z
);
extern int32_t MCM_AddVertex(MCM *M, double x, double y, double z, int32_t indface);
extern void MCM_VertexAddFace(MCM *M, int32_t indvert, int32_t indface);
extern void MCM_VertexAddEdge(MCM *M, int32_t indvert, int32_t indedge);
extern void MCM_VertexRemoveFace(MCM *M, int32_t indvert, int32_t indface);
extern int32_t MCM_AddFace(MCM *M, 
  double x1, double y1, double z1, 
  double x2, double y2, double z2, 
  double x3, double y3, double z3
);
extern int32_t MCM_AddFaceWithExistingVertices(
  MCM *M, int32_t iv1, int32_t iv2, int32_t iv3
);
extern int32_t MCM_AddFace2(MCM *M, 
  double x1, double y1, double z1, int32_t t1, 
  double x2, double y2, double z2, int32_t t2,
  double x3, double y3, double z3, int32_t t3
);
extern int32_t MCM_AddEdge(MCM *M, int32_t v1, int32_t v2);
extern void MCM_ComputeEdges(MCM *M);
extern void MCM_VertexMerge2Faces(MCM *M, int32_t indvert);
extern int32_t MCM_CheckComplex(MCM *M);
extern int32_t MCM_HealMesh(MCM *M);
extern int32_t MCM_RemoveDegenerateFaces(MCM *M);
extern void MCM_Print(MCM *M);

extern void InitMesh(int32_t taillemax);
extern void TermineMesh();
extern meshtabedges2 * AllocEdges(int32_t taillemax);
extern int32_t AddFace(double x1, double y1, double z1, 
             double x2, double y2, double z2, 
             double x3, double y3, double z3
	     );
extern int32_t AddFaceFixe(double x1, double y1, double z1, 
                 double x2, double y2, double z2, 
                 double x3, double y3, double z3,
                 int32_t fix1, int32_t fix2, int32_t fix3
	         );
extern void SaveCoords();
extern void RestoreCoords();
extern void ComputeEdges();
extern void ComputeLinks();
extern void AddNoiseMesh(double alpha);
extern void RegulMeshLaplacian(int32_t nsteps);
extern void RegulMeshLaplacian2D(int32_t niters);
extern void RegulMeshHamam(double theta);
extern void RegulMeshHamam1(double theta);
extern void RegulMeshHamam2(int32_t nitermax);
extern void RegulMeshHamam3(double theta);
extern void RegulMeshHC(double alpha, double beta);
extern void RegulMeshTaubin(double lambda, double mu, int nitermax);
extern void CalculNormales();
extern void CalculNormalesFaces();
extern void BoundingBoxMesh(meshbox *B);
extern void IsobarMesh(double *X, double *Y, double *Z);
extern double MeanDistCenter();
extern void TranslateMesh(double x, double y, double z);
extern void ZoomMesh(double k);
extern void ZoomMeshX(double k);
extern void ZoomMeshY(double k);
extern void ZoomMeshZ(double k);
extern double VolMesh();
extern void PrintMesh();
extern double AngleFaces(int32_t f1, int32_t f2);
extern double MaxAngleFaces();
extern void MeanAngleFaces(double *mean, double *standev);
extern double MaxLengthEdges();
extern void ComputeCurvatures();

#ifdef __cplusplus
}
#endif
