/*
  Creates a edge-weighted graph from an image,
  Computes the min spanning tree of this graph.
  (C.Farabet)
*/

#ifndef MST_FEATURES
#define MST_FEATURES

#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "tree.h"
#include "segment-features.h"

/*
 * Computes the MST of a feature image (e.g. each pixel is an N-dim vector)
 *
 * Returns the tree.
 *
 * im: image to segment.
 * nfeats: nb of features
 * sigma: to smooth the image.
 * dt: distance type
 * method: connexity (4 or 8)
 */
tree *mst_features(image<float> **feats, 
                   int nfeats,
                   float sigma,
                   int dt, 
                   int method) {
  // get first feature map
  image<float> *im = feats[0];

  // get dims
  int width = im->width();
  int height = im->height();

  // smooth feature maps
  image<float> ** smoothed = new image<float> *[nfeats];
  if (sigma == 0) {
    for (int i=0; i<nfeats; i++)
      smoothed[i] = feats[i];
  } else {
    for (int i=0; i<nfeats; i++)
      smoothed[i] = smooth(feats[i],sigma);
  }

  // graph data;
  edge *edges=NULL; int num = 0;

  // 2 methods (0 = 4 edges per vertex, 1 = 8 edges per vertex)
  if (method == 4) {

    // build graph with 4-connex
    edges = new edge[width*height*2];
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          edges[num].a = y * width + x;
          edges[num].b = y * width + (x+1);
          edges[num].w = ndiff(smoothed, nfeats, x, y, x+1, y, dt);
          num++;
        }

        if (y < height-1) {
          edges[num].a = y * width + x;
          edges[num].b = (y+1) * width + x;
          edges[num].w = ndiff(smoothed, nfeats, x, y, x, y+1, dt);
          num++;
        }
      }
    }

  } else if (method == 8) {

    // build graph with 8-connex
    edges = new edge[width*height*4];
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          edges[num].a = y * width + x;
          edges[num].b = y * width + (x+1);
          edges[num].w = ndiff(smoothed, nfeats, x, y, x+1, y, dt);
          num++;
        }

        if (y < height-1) {
          edges[num].a = y * width + x;
          edges[num].b = (y+1) * width + x;
          edges[num].w = ndiff(smoothed, nfeats, x, y, x, y+1, dt);
          num++;
        }

        if ((x < width-1) && (y < height-1)) {
          edges[num].a = y * width + x;
          edges[num].b = (y+1) * width + (x+1);
          edges[num].w = ndiff(smoothed, nfeats, x, y, x+1, y+1, dt);
          num++;
        }

        if ((x < width-1) && (y > 0)) {
          edges[num].a = y * width + x;
          edges[num].b = (y-1) * width + (x+1);
          edges[num].w = ndiff(smoothed, nfeats, x, y, x+1, y-1, dt);
          num++;
        }
      }
    }

  } else {
    THError("<libmstsegm.mst> unsupported connectivity (only 8 or 4 are valid)");
  }
  
  // cleanup
  if (sigma != 0)
    for (int i=0; i<nfeats; i++) delete smoothed[i];
  delete [] smoothed;

  // compute MST
  tree *t = newtree(edges, num, width*height, method);
  compute_mst(t);

  // done
  return t;
}

#endif
