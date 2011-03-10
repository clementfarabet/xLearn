/*
Note: This is a copy of segment-image.h, modified to handle features rather
      than plain RGB image.
      (C.Farabet)
*/

#ifndef SEGMENT_VOLUME
#define SEGMENT_VOLUME

#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "segment-graph.h"
#include "segment-image.h"

// dissimilarity measure between pixels, in some feature space
static inline float vdiff(image<float> ***feats,
                          int nfeats,
			  int x1, int y1, int z1, int x2, int y2, int z2,
			  int dt) {
  float dist = 0;
  for (int i=0; i<nfeats; i++) {
    dist += square(imRef(feats[z1][i], x1, y1)-imRef(feats[z2][i], x2, y2));
  }
  return sqrt(dist);
}

/*
 * Segment a feature map
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * nslices: nb of slices in volume
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
static image<rgb> **segment_volume(image<float> ***feats, 
                                   int nslices,
                                   int nfeats,
                                   float sigma, float c, int min_size,
                                   int dt,
                                   int method,
                                   int *num_ccs) {
  // get dims
  image<float> *im = feats[0][0];
  int width = im->width();
  int height = im->height();

  // smooth feature maps
  image<float> *** smoothed = new image<float> **[nslices];
  for (int i=0; i<nslices; i++) {
    // new slice:
    smoothed[i] = new image<float> *[nfeats];
    for (int k=0; k<nfeats; k++) {
      // all channels:
      smoothed[i][k] = smooth(feats[i][k], sigma);
    }
  }

  // graph data;
  edge *edges=NULL; int num = 0;

  // 2 methods (0 = 4 edges per vertex, 1 = 8 edges per vertex)
  if (method == 4 || method == 6) {

    // build graph with 4-connex
    edges = new edge[width*height*nslices*2 + width*height*(nslices-1)];
    for (int z = 0; z < nslices; z++) {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          if (x < width-1) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + y) * width + (x+1);
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x+1, y, z, dt);
            num++;
          }

          if (y < height-1) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + (y+1)) * width + x;
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y+1, z, dt);
            num++;
          }

          if (z < (nslices-1)) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = ((z+1) * height + y) * width + x;
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y, z+1, dt);
            num++;
          }
        }
      }
    }

  } else if (method == 8 || method == 10) {

    // build graph with 8-connex
    edges = new edge[width*height*nslices*4 + width*height*(nslices-1)];
    for (int z = 0; z < nslices; z++) {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          if (x < width-1) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + y) * width + (x+1);
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x+1, y, z, dt);
            num++;
          }

          if (y < height-1) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + (y+1)) * width + x;
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y+1, z, dt);
            num++;
          }


          if ((x < width-1) && (y < height-1)) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + (y+1)) * width + (x+1);
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x+1, y+1, z, dt);
            num++;
          }

          if ((x < width-1) && (y > 0)) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + (y-1)) * width + (x+1);
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x+1, y-1, z, dt);
            num++;
          }

          if (z < (nslices-1)) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = ((z+1) * height + y) * width + x;
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y, z+1, dt);
            num++;
          }
        }
      }
    }

  } else if (method > 100) {

    // build graph with 4-connex on spatial plane, and the rest in temporal
    // dim
    int dist = method - 100;
    edges = new edge[width*height*nslices*(3+dist*2)];
    for (int z = 0; z < nslices; z++) {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          if (x < width-1) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + y) * width + (x+1);
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x+1, y, z, dt);
            num++;
          }

          if (y < height-1) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + (y+1)) * width + x;
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y+1, z, dt);
            num++;
          }

          if (z < (nslices-1)) {
            edges[num].a = (z * height + y) * width + x;
            edges[num].b = ((z+1) * height + y) * width + x;
            edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y, z+1, dt);
            num++;

            for (int k = 0; k < dist; k++) {
              int step = 2 << k;

              if (y < height-step) {
                edges[num].a = (z * height + y) * width + x;
                edges[num].b = ((z+1) * height + (y+step)) * width + x;
                edges[num].w = vdiff(smoothed, nfeats, x, y, z, x, y+step, z+1, dt) + k;
                num++;
              }

              if (x < width-step) {
                edges[num].a = (z * height + y) * width + x;
                edges[num].b = ((z+1) * height + y) * width + (x+step);
                edges[num].w = vdiff(smoothed, nfeats, x, y, z, x+step, y, z+1, dt) + k;
                num++;
              }
            }
          }
        }
      }
    }

  } else {
    THError("<libmstsegm.infer> unsupported connectivity (only 8, 4 or rad-N are valid)");
  }

  // cleanup
  for (int i=0; i<nslices; i++) {
    for (int k=0; k<nfeats; k++) delete smoothed[i][k];
    delete smoothed[i];
  }
  delete [] smoothed;

  // segment
  universe *u = segment_graph(width*height*nslices, num, edges, c);

  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  delete [] edges;
  *num_ccs = u->num_sets();

  image<rgb> **output = new image<rgb> *[nslices];
  for (int i=0; i<nslices; i++)
    output[i] = new image<rgb>(width, height);

  // pick random colors for each component
  rgb *colors = new rgb[width*height*nslices];
  for (int i = 0; i < width*height*nslices; i++)
    colors[i] = random_rgb();

  for (int z = 0; z < nslices; z++) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int comp = u->find((z * height + y) * width + x);
        imRef(output[z], x, y) = colors[comp];
      }
    }
  }

  delete [] colors;  
  delete u;

  return output;
}

#endif
