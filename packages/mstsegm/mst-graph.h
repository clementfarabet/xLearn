/*
  Computes the min spanning tree of a given graph.
  (C.Farabet)
*/

#ifndef MST_GRAPH
#define MST_GRAPH

#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "tree.h"
#include "graph.h"

/*
 * Computes the MST of a given graph.
 *
 * Returns the tree.
 *
 * im: image to segment.
 * nfeats: nb of features
 * sigma: to smooth the image.
 * dt: distance type
 * method: connexity (4 or 8)
 */
tree *mst_graph(THTensor *graph) {

  // graph data:
  int width = graph->size[0];
  int height = graph->size[1];
  int connex = graph->size[2]*2;
  edge *edges = NULL; 
  int num = 0;

  // connex can be one of: 4 | 8
  if (connex == 4) {

    // build graph with 4-connex
    edges = new edge[width*height*connex/2];
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          edges[num].a = y * width + x;
          edges[num].b = y * width + (x+1);
          edges[num].w = THTensor_get3d(graph, x, y, 0);
          num++;
        }

        if (y < height-1) {
          edges[num].a = y * width + x;
          edges[num].b = (y+1) * width + x;
          edges[num].w = THTensor_get3d(graph, x, y, 1);
          num++;
        }
      }
    }

  } else if (connex == 8) {

    // build graph with 8-connex
    edges = new edge[width*height*connex/2];
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          edges[num].a = y * width + x;
          edges[num].b = y * width + (x+1);
          edges[num].w = THTensor_get3d(graph, x, y, 0);
          num++;
        }

        if (y < height-1) {
          edges[num].a = y * width + x;
          edges[num].b = (y+1) * width + x;
          edges[num].w = THTensor_get3d(graph, x, y, 1);
          num++;
        }

        if ((x < width-1) && (y < height-1)) {
          edges[num].a = y * width + x;
          edges[num].b = (y+1) * width + (x+1);
          edges[num].w = THTensor_get3d(graph, x, y, 2);
          num++;
        }

        if ((x < width-1) && (y > 0)) {
          edges[num].a = y * width + x;
          edges[num].b = (y-1) * width + (x+1);
          edges[num].w = THTensor_get3d(graph, x, y, 3);
          num++;
        }
      }
    }

  } else {
    THError("<libmstsegm.mst> unsupported connectivity (only 8 or 4 are valid)");
  }

  // compute MST
  tree *t = newtree(edges, num, width*height, connex);
  compute_mst(t);

  // done
  return t;
}

#endif
