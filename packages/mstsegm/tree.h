/*
  A simple structure to represent an MST.
  (C.Farabet)
*/

#ifndef TREE
#define TREE

#include "segment-graph.h"
#include "disjoint-set.h"

// basic structure to represent a tree:
// a list of edges and a universe
typedef struct {
  edge *edges;
  int *mst;
  float *mst_weights;
  int nedges;
  int nvertices;
  int connex;
} tree;

tree *newtree(edge *edges, int nedges, int nvertices, int connex) {
  tree *t = (tree *)malloc(sizeof(tree));
  t->edges = edges;
  t->nedges = nedges;
  t->nvertices = nvertices;
  t->connex = connex;
  t->mst = NULL;
  return t;
}

void deltree(tree *t) {
  delete [] t->edges;
  if (t->mst != NULL) {
    delete [] t->mst;
    delete [] t->mst_weights;
  }
}

void pushtree(lua_State *L, tree *t) {
  tree *nt = (tree *)lua_newuserdata(L, sizeof(tree));
  luaL_getmetatable(L, "mstsegm.Tree");
  lua_setmetatable(L, -2);
  memcpy(nt, t, sizeof(tree));
  free(t);
}

tree *checktree(lua_State *L, int pos) {
  // args
  tree *t = (tree *)luaL_checkudata(L, pos, "mstsegm.Tree");
  if (t == NULL) THError("<mstsegm.Tree> requires a valid mstsegm.Tree");
  return t;
}

const char *printtree(tree *t) {
  const char *cstr = new char[1000000];
  char *str = (char *)cstr;
  str += sprintf(str, "mstsegm.Tree: {\n");
  str += sprintf(str, " + number of vertices: %d\n", t->nvertices);
  str += sprintf(str, " + number of edges: %d\n", t->nedges);
  str += sprintf(str, " + connexity (max nb of edges per vertex): %d\n", t->connex);
  str += sprintf(str, " + sorted edges:\n");
  for (int i = 0; i < t->nedges; i++) {
    edge *pedge = &t->edges[i];
    str += sprintf(str, "   - edge %d: %d <-> %d (w=%f)\n", i, pedge->a, pedge->b, pedge->w);
    if (i == 40) {
      str += sprintf(str, "   ...\n");
      break;
    }
  }
  str += sprintf(str, " + mst:\n");
  for (int i = 0; i < t->nvertices; i++) {
    str += sprintf(str, "   - virtex %d:\n", i);
    int k = 0;
    while ((t->mst[t->connex*i+k] != -1) && k < t->connex) {
      str += sprintf(str, "     -> %d (w=%f)\n",
                     t->mst[t->connex*i+k], t->mst_weights[t->connex*i+k]);
      k++;
    }
    if (i == 40) {
      str += sprintf(str, "   ...\n");
      break;
    }
  }
  str += sprintf(str, "}");
  return cstr;
}

void compute_mst(tree *t) {
  // already computed ?
  if (t->mst != NULL) return;

  // init list of edges
  t->mst = new int[t->nvertices * t->connex];
  t->mst_weights = new float[t->nvertices * t->connex];
  for (int i=0; i<(t->nvertices * t->connex); i++) t->mst[i] = -1;

  // sort edges
  std::sort(t->edges, t->edges + t->nedges);

  // make a disjoint-set forest
  universe *u = new universe(t->nvertices);

  // process all edges
  for (int i = 0; i < t->nedges; i++) {
    edge *pedge = &t->edges[i];

    // components conected by this edge
    int a = u->find(pedge->a);
    int b = u->find(pedge->b);
    if (a != b) {
      // merge them
      u->join(a, b);
      a = u->find(a);

      // add double-linked edges to mst list
      int k = 0;
      while (t->mst[t->connex*pedge->a+k] != -1) k++; // find next free slot
      t->mst[t->connex*pedge->a+k] = pedge->b;
      t->mst_weights[t->connex*pedge->a+k] = pedge->w;
      k = 0;
      while (t->mst[t->connex*pedge->b+k] != -1) k++; // find next free slot
      t->mst[t->connex*pedge->b+k] = pedge->a;
      t->mst_weights[t->connex*pedge->b+k] = pedge->w;
    }
  }

  // cleanup
  delete u;
}

bool minimax(tree *t, int vertex1, int vertex2, int *marks, edge *mm) {
  int k = 0;
  int nvertex;
  while (((nvertex = t->mst[t->connex*vertex1+k]) != -1) && k < t->connex) {

    // mark vertex as already seen (to avoid infinite loops)
    int kk = 0;
    while (t->mst[t->connex*nvertex+kk] != vertex1) kk++;
    marks[t->connex*nvertex+kk] = 1;

    // is not seen before, process
    if (marks[t->connex*vertex1+k] == -1) {
      if (nvertex == vertex2) {
        if (mm->w <= t->mst_weights[t->connex*vertex1+k]) {
          mm->w = t->mst_weights[t->connex*vertex1+k];
          mm->a = vertex1;
          mm->b = nvertex;
        }
        return true;
      }
      if (minimax(t, nvertex, vertex2, marks, mm)) {
        if (mm->w < t->mst_weights[t->connex*vertex1+k]) {
          mm->w = t->mst_weights[t->connex*vertex1+k];
          mm->a = vertex1;
          mm->b = nvertex;
        } else if (mm->w == t->mst_weights[t->connex*vertex1+k]) {
          int chance = rand() % 10;
          if ((mm->w == -1) || (chance == 5)) {
            mm->w = t->mst_weights[t->connex*vertex1+k];
            mm->a = vertex1;
            mm->b = nvertex;
          }
        }
        return true;
      }
    }

    k++;
  }
  return false;
}

edge minimax_disparity(tree *t, int vertex1, int vertex2) {
  // already computed ?
  if (t->mst == NULL) THError("<mstsegm.Tree> min spanning tree not computed yet");

  // store minimax edge and its position
  edge mm;
  mm.w = -1;
  mm.a = -1;
  mm.b = -1;

  // keep track of visited vertices
  int *marks = new int[t->nvertices * t->connex];
  for (int i=0; i<(t->nvertices * t->connex); i++) marks[i] = -1;

  // check equality
  if (vertex1 == vertex2) {
    mm.w = 0;
    mm.a = vertex1;
    mm.b = vertex2;
  } else {
    // compute minimax disparity between two vertices, by walking through the tree
    minimax(t, vertex1, vertex2, marks, &mm);
  }

  // assert
  if ((mm.a == -1) || (mm.b == -1)) {
    const char * treestr = printtree(t);
    printf("%s\n",treestr);
    THError("<libmstsegm.minimax_disparity> error in algorithm: couldnt find minimax edge!!");
  }

  // cleanup
  delete [] marks;

  // acc contains the result
  return mm;
}

THTensor *minimax_disparities(tree *t) {
  // already computed ?
  if (t->mst == NULL) THError("<mstsegm.Tree> min spanning tree not computed yet");

  // create matrix to hold all disparities
  THTensor *matrix = THTensor_newWithSize3d(t->nvertices, t->nvertices, 3);

  // compute all disparities
  for (int n=0; n<t->nvertices; n++) {
    THTensor_set3d(matrix, n, n, 0, 0);
    THTensor_set3d(matrix, n, n, 1, n);
    THTensor_set3d(matrix, n, n, 2, n);
    for (int l=n+1; l<t->nvertices; l++) {
      edge mm = minimax_disparity(t, n, l);
      THTensor_set3d(matrix, n, l, 0, (double)mm.w);
      THTensor_set3d(matrix, l, n, 0, (double)mm.w);
      THTensor_set3d(matrix, n, l, 1, (double)mm.a);
      THTensor_set3d(matrix, l, n, 1, (double)mm.a);
      THTensor_set3d(matrix, n, l, 2, (double)mm.b);
      THTensor_set3d(matrix, l, n, 2, (double)mm.b);
    }
  }

  // done
  return matrix;
}

#endif
