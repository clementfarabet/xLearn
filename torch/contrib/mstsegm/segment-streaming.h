/*
Note: This is a copy of segment-image.h, modified to handle volumes
      in a streaming fashion, as prescribed by Grundmann et al.
      (C.Farabet)
*/

#ifndef SEGMENT_STREAMING
#define SEGMENT_STREAMING

#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "segment-graph.h"
#include "segment-volume.h"

/*
 * global history variables
 */
static universe *u_prev = NULL;
static image<float> ***volume_prev = NULL;
static rgb *cols = NULL;
static int id_seed = 0;

/*
 * Segment a volume, in a streaming fashion.
 * The first volume is segmented by itself, each next one is constrained
 * by the previous by weighting its edge graph.
 *
 * Returns a colored volume representing the current segmentation.
 *
 * im: image to segment.
 * nslices: nb of slices in volume
 * nfeats: nb of features per voxel
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
image<rgb> **segment_streaming(image<float> ***volume,
                               int nslices, int nfeats, int reset,
                               float sigma, float c, int min_size,
                               int dt, int method, int *num_ccs) {
  // get dims
  image<float> *im = volume[0][0];
  int width = im->width();
  int height = im->height();

  // reset
  if (reset && u_prev != NULL) {
    delete u_prev;
    u_prev = NULL;
    for (int i=0; i<nslices; i++) {
      for (int k=0; k<nfeats; k++) { 
        delete volume_prev[i][k];
      }
      delete [] volume_prev[i];
    }
    delete [] volume_prev;
    volume_prev = NULL;
  }

  // smooth feature maps
  image<float> *** smoothed = new image<float> **[nslices];
  for (int i=0; i<nslices; i++) {
    // new slice:
    smoothed[i] = new image<float> *[nfeats];
    for (int k=0; k<nfeats; k++) {
      // all channels:
      smoothed[i][k] = smooth(volume[i][k], sigma);
    }
  }

  // graph data;
  edge *edges=NULL; int num = 0;

  // 2 methods (0 = 4 edges per vertex, 1 = 8 edges per vertex)
  if (method == 4) {

    // build graph with 4-connex
    edges = new edge[width*height*(nslices*2 + nslices-1)];
    for (int z = 0; z < nslices; z++) {
      // this coef is a linear weight, going from 0 to 1
      float linear_coef = ((float)(z))/(nslices-2);  // [0..1]  (linear)
      linear_coef = (linear_coef*2-1);               // [-1..1] (linear)
      linear_coef *= linear_coef;                    // [1..1]  (square)

      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          
          // weighting coef (from prev universe)
          float coef = 1;
          int a1=0, b1=0, a2=0, b2=0, a3=0, b3=0;
          if ((u_prev != NULL) && (z < (nslices-1))) {
            coef = linear_coef;

            a1 = u_prev->find(((z+1) * height + y) * width + x);
            if (x < width-1) {
              b1 = u_prev->find(((z+1) * height + y) * width + x + 1);
            }

            a2 = a1;
            if (y < height-1) {
              b2 = u_prev->find(((z+1) * height + y + 1) * width + x);
            }

            if (z > 0) {
              a3 = u_prev->find(((z+1) * height + y) * width + x);
              b3 = u_prev->find(((z) * height + y) * width + x);
            }
          }

#define MAX_EDGE 100
#define MIN_EDGE 10

          if (x < width-1) {
            float coef1 = coef;
            if (a1 != b1) coef1 = (MAX_EDGE-MIN_EDGE)*(1-coef) + MIN_EDGE;

            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + y) * width + (x+1);
            edges[num].w = coef1 * vdiff(smoothed, nfeats, x, y, z, x+1, y, z, dt);
            num++;
          }
        
          if (y < height-1) {
            float coef2 = coef;
            if (a2 != b2) coef2 = (MAX_EDGE-MIN_EDGE)*(1-coef) + MIN_EDGE;

            edges[num].a = (z * height + y) * width + x;
            edges[num].b = (z * height + (y+1)) * width + x;
            edges[num].w = coef2 * vdiff(smoothed, nfeats, x, y, z, x, y+1, z, dt);
            num++;
          }
      
          if (z > 0) {
            float coef3 = coef;
            if (a3 != b3) coef3 = (MAX_EDGE-MIN_EDGE)*(1-coef) + MIN_EDGE;

            edges[num].a = (z * height + y) * width + x;
            edges[num].b = ((z-1) * height + y) * width + x;
            edges[num].w = coef3 * vdiff(smoothed, nfeats, x, y, z, x, y, z-1, dt);
            num++;
          }
        }
      }
    }

  } else {
    THError("<libmstsegm.infer> unsupported connectivity (only 4 is valid)"); 
  }

  // cleanup or alloc
  if (volume_prev == NULL) {
    volume_prev = new image<float> **[nslices];
    for (int i=0; i<nslices; i++) volume_prev[i] = new image<float> *[nfeats];
  } else {
    for (int i=0; i<nslices; i++)
      for (int k=0; k<nfeats; k++) delete volume_prev[i][k];
  }
  
  // keep current image in prev
  for (int i=0; i<nslices; i++)
    for (int k=0; k<nfeats; k++)
      volume_prev[i][k] = smoothed[i][k];

  // cleanup 
  for (int i=0; i<nslices; i++) delete [] smoothed[i];
  delete [] smoothed;

  // segment
  universe *u = segment_graph_wo_size(width*height*nslices, num, edges, c);

  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  *num_ccs = u->num_sets();

  // cleanup universe if z>0
  if (reset) {
    id_seed = 0;
  }

  // propagate IDs
  if (id_seed == 0) { // first time, no IDs ever assigned

    // store all IDs
    int *uids = new int[nslices*width*height];
    for (int i=0; i<(nslices*width*height); i++) {
      uids[i] = -1;
    }

    // set uids of all components' root
    for (int z = 0; z < nslices; z++) {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          // get parent:
          int comp = u->find((z * height + y) * width + x);
          if (uids[comp] == -1) { // first time seen the root
            uids[comp] = id_seed++;
            u->set_uid(comp, uids[comp]);
          }
        }
      }
    }

    // cleanup
    delete [] uids;

  } else { // IDs need to be propagated

    // store all IDs
    int *uids = new int[nslices*width*height];
    int *uids_prev = new int[nslices*width*height];
    for (int i=0; i<(nslices*width*height); i++) {
      uids[i] = -1;
      uids_prev[i] = -1;
    }

    // propagate IDs in overlapping region
    for (int i = 0; i < num; i++) {
      // get root
      int comp = u->find(edges[i].b);
      // old ID ?
      if (edges[i].b < (nslices-1)*width*height) {        
        if (uids[comp] == -1) { // first time seen the root
          // get old ID
          int comp_prev = u_prev->find(edges[i].b + width*height);
          if (uids_prev[comp_prev] == -1) {
            uids[comp] = u_prev->uid(comp_prev);
            uids_prev[comp_prev] = u_prev->uid(comp_prev);
            u->set_uid(comp, uids[comp]);
          } else {
            uids[comp] = id_seed++;
            u->set_uid(comp, uids[comp]);                
          }
        }
      }
    }

    // last layer: new IDs
    int z = nslices-1;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int comp = u->find((z*height+y)*width+x);
        if (uids[comp] == -1) { // first time seen the root
          uids[comp] = id_seed++;
          u->set_uid(comp, uids[comp]);
        }
      }
    }

    // cleanup
    delete [] uids;
    delete [] uids_prev;
  }
  // done propagating

  // remember universe
  if (u_prev != NULL) delete u_prev;
  u_prev = u;

  // clean edges
  delete [] edges;

  // produce RGB output
  image<rgb> **output = new image<rgb> *[nslices];
  for (int i=0; i<nslices; i++)
    output[i] = new image<rgb>(width, height);

  // pick random cols for each component
  // TODO : replace this hard-coded depth by a sliding pool of cols
  // (in anycase 2^24 cannot represent more than 16e6 cols
  if (reset && cols != NULL) {
    delete [] cols;
    cols = NULL;
  }
  if (cols == NULL) {
    const int RGBSPACE = 16*1024*1024;
    cols = new rgb[RGBSPACE];
    for (int i = 0; i < RGBSPACE; i++)
      cols[i] = random_rgb();
  }

  // replace IDs by cols
  for (int z = 0; z < nslices; z++) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int comp = u->find((z * height + y) * width + x);
        imRef(output[z], x, y) = cols[u->uid(comp)];
      }
    }
  }

  // return colored volume
  return output;
}

#endif
