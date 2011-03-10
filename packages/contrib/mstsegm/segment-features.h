/*
Note: This is a copy of segment-image.h, modified to handle features rather
      than plain RGB image.
      (C.Farabet)
*/

#ifndef SEGMENT_FEATURES
#define SEGMENT_FEATURES

#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "segment-graph.h"
#include "segment-image.h"

enum { euclid, angle };

// dissimilarity measure between pixels, in some feature space
static inline float ndiff(image<float> **feats,
			  int nfeats,
			  int x1, int y1, int x2, int y2,
			  int dt) {
  float dist  = 0;
  float dot   = 0;
  float normx = 0;
  float normy = 0;
  float res = 0;
  for (int i=0; i<nfeats; i++) {
    if (dt == euclid) {
      dist  += square(imRef(feats[i], x1, y1)-imRef(feats[i], x2, y2));
    } else if (dt == angle) {
      dot   += imRef(feats[i], x1, y1) * imRef(feats[i], x2, y2);
      normx += square(imRef(feats[i], x1, y1));
      normy += square(imRef(feats[i], x2, y2));
    }
  }
  if (dt == euclid) {
    res = sqrt(dist);
    // printf("result: %f\n", res);
  } else if (dt == angle) {
    // FIXME need to handle different cases of acos
    // acos(x) : x must be between -1,1 result is between 0 and pi
    res = acos(dot/(sqrt(normx)*sqrt(normy)));
    //printf("result %f dot: %f nx: %d ny:%d\n", res, dot, normx, normy);
  }
  return res;
}

/*
 * Segment a feature map
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * nfeats: nb of features
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
image<rgb> *segment_features(image<float> **feats, 
                             int nfeats,
                             float sigma, float c, int min_size,
			     int dt, int method,
                             int *num_ccs) {
  // get first feature map
  image<float> *im = feats[0];

  // get dims
  int width = im->width();
  int height = im->height();

  // smooth feature maps
  image<float> ** smoothed = new image<float> *[nfeats];
  for (int i=0; i<nfeats; i++)
    smoothed[i] = smooth(feats[i],sigma);

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
    THError("<libmstsegm.infer> unsupported connectivity (only 8 or 4 are valid)");
  }
  
  // cleanup
  for (int i=0; i<nfeats; i++) delete smoothed[i];
  delete [] smoothed;
#if DEBUG
      if (dt == euclid){
	printf("euclid: ");
      } else {
	printf("angle: ");
      }
      printf("c = %f\n", c); 
#endif
  // segment
  universe *u = segment_graph(width*height, num, edges, c);
  
  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  delete [] edges;
  *num_ccs = u->num_sets();

  image<rgb> *output = new image<rgb>(width, height);

  // pick random colors for each component
  rgb *colors = new rgb[width*height];
  for (int i = 0; i < width*height; i++)
    colors[i] = random_rgb();
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
      imRef(output, x, y) = colors[comp];
    }
  }  

  delete [] colors;  
  delete u;

  return output;
}

#endif
