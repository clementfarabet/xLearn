/*
Note: This is a copy of segment-image.h, modified to handle volumes
      incrementally, e.g. frame by frame
      (B.Corda & C.Farabet)
*/

#ifndef SEGMENT_INCREMENTAL
#define SEGMENT_INCREMENTAL

#include <cstdlib>
#include <image.h>
#include <misc.h>
#include <filter.h>
#include "segment-graph.h"
#include "segment-volume.h"

// global history variables
static universe *universe_prev = NULL;
static image<float> **img_prev = NULL;
static rgb *colors = NULL;
static int global_id = 0;

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
image<rgb> *segment_incremental(image<float> **img,
                                int nfeats, int reset,
                                float sigma, float c, int min_size,
                                int dt, int method, int *num_ccs) {
  // get dims
  image<float> *im = img[0];
  int width = im->width();
  int height = im->height();

  // reset
  if (reset && universe_prev != NULL) {
    delete universe_prev;
    for (int i=0; i<nfeats; i++) delete img_prev[i];
    delete [] img_prev;
    universe_prev = NULL;
    img_prev = NULL;
  }

  // smooth feature maps
  image<float> ** smoothed = new image<float> *[nfeats];
  for (int k=0; k<nfeats; k++) {
    // all channels:
    smoothed[k] = smooth(img[k], sigma);
  }

  // reset ?
  image<float> ***volume;
  int z;
  if (reset || img_prev == NULL) {
    // first slice = 1-deep volume
    volume = new image<float> **[1];
    volume[0] = smoothed; // img at t
    z = 0;
  } else {
    // artificial 2-deep volume
    volume = new image<float> **[2];
    volume[0] = img_prev; // img at t-1
    volume[1] = smoothed; // img at t
    z = 1;
  }

  // graph data;
  edge *edges=NULL; int num = 0;

  // 2 methods (0 = 4 edges per vertex, 1 = 8 edges per vertex)
  if (method == 4) {

    // build graph with 4-connex
    edges = new edge[width*height*3];
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = (z * height + y) * width + (x+1);
          edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y, z, dt);
          num++;
        }
        
        if (y < height-1) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = (z * height + (y+1)) * width + x;
          edges[num].w = vdiff(volume, nfeats, x, y, z, x, y+1, z, dt);
          num++;
        }
      
        if (z == 1) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = ((z-1) * height + y) * width + x;
          edges[num].w = vdiff(volume, nfeats, x, y, z, x, y, z-1, dt);
          num++;
        }
      }
    }

  } else if (method == 8) {

    // build graph with 8-connex
    edges = new edge[width*height*5];
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = (z * height + y) * width + (x+1);
          edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y, z, dt);
          num++;
        }
        
        if (y < height-1) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = (z * height + (y+1)) * width + x;
          edges[num].w = vdiff(volume, nfeats, x, y, z, x, y+1, z, dt);
          num++;
        }
      
        if ((x < width-1) && (y < height-1)) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = (z * height + (y+1)) * width + (x+1);
          edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y+1, z, dt);
          num++;
        }
      
        if ((x < width-1) && (y > 0)) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = (z * height + (y-1)) * width + (x+1);
          edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y-1, z, dt);
          num++;
        }

        if (z == 1) {
          edges[num].a = (z * height + y) * width + x;
          edges[num].b = ((z-1) * height + y) * width + x;
          edges[num].w = vdiff(volume, nfeats, x, y, z, x, y, z-1, dt);
          num++;
        }
      }
    }

  } else {
    THError("<libmstsegm.infer> unsupported connectivity (only 8 or 4 are valid)"); 
  }

  // cleanup or alloc
  if (img_prev == NULL)
    img_prev = new image<float> *[nfeats];
  else
    for (int i=0; i<nfeats; i++) delete img_prev[i];

  // keep current image in prev
  for (int k=0; k<nfeats; k++) {
    img_prev[k] = smoothed[k];
  }
  delete [] smoothed;

  // segment
  universe *u;

  // first time ? single slice : increment over previous volume
  if (reset || universe_prev == NULL)
    u = segment_graph(width*height, num, edges, c);
  else
    u = incremental_segment_graph(universe_prev, width*height, num, edges, c);

  // delete previous univ
  if (universe_prev != NULL) delete universe_prev;

  // cleanup universe if z>0
  if (reset) {
    global_id = 0;
  }

  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->joinincr(a, b);
  }
  delete [] edges;
  
  if (z>0) {
    int *uids = new int[width*height*2];
    int *parents = new int[width*height*2];
    for (int i=0; i<width*height*2; i++) {
      uids[i] = -1;
      parents[i] = -1;
    }

/*     for (int y = 0; y < height; y++) { */
/*       for (int x = 0; x < width; x++) { */
/*         int comp = u->find((z * height + y) * width + x); */
/*         if ( u->uid(comp) == -1){ */
/*           printf("mofo there is root %d at -1\n",comp); */
/*           u->set_uid(comp,10); */
/*         } */
/*       }} */
/*     printf("no root at -1 z= %d\n",z); */

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int old_index = (z * height + y) * width + x;
        int comp = u->find(old_index);
        if (uids[comp] == -1) { // first time seen the root
          if (u->uid(comp) == -1) { // still has no ID
            uids[comp] = global_id++;
            u->set_uid(comp, uids[comp]);
          } else {
            uids[comp] = u->uid(comp);
          }
        }
        if (comp > width*height) // root is in new slice, remap
          parents[old_index] = comp - height*width;
        else { // root is in old slice set the current guy the root
          parents[old_index] = old_index - height*width;
          uids[old_index] = u->uid(comp);
          // set current as root          
          u->set_p(old_index, old_index);
          u->set_size(old_index, u->size(comp));
          u->set_uid(old_index, u->uid(comp));
          // set old parent the child of current
          u->set_p(comp, old_index);
        }


        if (x==50 && y==50) {
          printf("1> uid=%0d, rank=%0d, size=%0d\n", u->uid(comp), u->element(comp).rank,
                 u->element(comp).size);
        }
      }
    }

    // create new universe to shift current results (from slice 1 to 0)
    universe_prev = new universe(width*height,c);
    universe_prev->set_num_sets(u->num_sets());
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int comp = u->find((z * height + y) * width + x);
        universe_prev->set_p(y*width + x, parents[comp]);
        universe_prev->set_p(parents[comp], parents[comp]);
        universe_prev->set_uid(parents[comp], u->uid(comp));
        universe_prev->set_size(parents[comp], u->size(comp));
        universe_prev->set_rank(parents[comp], 2);
        float *dest = universe_prev->thresholds();
        float *src = u->thresholds();
        dest[parents[comp]] = src[comp];

/*         int root = u->find((z * height + y) * width + x); */
/*         int old_index = (z * height + y) * width + x; */
/*         int new_index = y*width + x; */
/*         // if root is an old parent we swap root */
/*         if (root < (width*height)) { */
/*           u->set_p(root,(z * height + y) * width + x); */
/*           // set the new universe accordingly */
/*           universe_prev->set_p(new_index, new_index); */
/*           universe_prev->set_uid(new_index, u->uid(root)); */
/*           universe_prev->set_size(new_index, u->size(root)); */
/*           universe_prev->set_rank(new_index, 1); */
/*           float *dest = universe_prev->thresholds(); */
/*           float *src = u->thresholds(); */
/*           dest[new_index] = src[root]; */
/*         } else if (old_index == root){ // the current index is a root */
/*           universe_prev->set_p(new_index, new_index); */
/*           if (u->uid(root) == -1) // still has no ID */
/*             universe_prev->set_uid(new_index, global_id++); */
/*           else */
/*             universe_prev->set_uid(new_index, u->uid(root)); */
/*           universe_prev->set_size(new_index, u->size(root)); */
/*           universe_prev->set_rank(new_index, 1); */
/*         } else { // the current index as a new vertex for root */
/*           universe_prev->set_p(new_index, root - (width*height)); */
/*           universe_prev->set_size(new_index, u->size(old_index)); */
/*           universe_prev->set_rank(new_index, 1); */
/*         } */
      }
    }
    delete u;
    delete [] uids;
    delete [] parents;
  } else {
    int *uids = new int[width*height];
    for (int i=0; i<(width*height); i++) {
      uids[i] = -1;
    }
    
    // set uids of all components' root
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int comp = u->find(y * width + x);
        if (uids[comp] == -1) { // first time seen the root
          if (u->uid(comp) == -1) { // still has no ID
            uids[comp] = global_id;
            global_id += 1;
            u->set_uid(comp, uids[comp]);
          } else {
            uids[comp] = u->uid(comp);
          }
        }
        if (x==50 && y==50) {
          printf("0> uid=%0d, rank=%0d, size=%0d\n", u->uid(comp), u->element(comp).rank,
                 u->element(comp).size);
        }
      }
    }
    delete [] uids;
    // for the first time, just remember the current universe
    universe_prev = u;
  }

  // global_id needs to wrap around, to avoid exceeding color index space
  if ((global_id+width*height) >= (1<<24)) global_id = 0;

  *num_ccs = universe_prev->num_sets();
  printf("got %0d sets\n", *num_ccs);
  // print all components
  if (*num_ccs < 100) {
    for (int i =0; i < height*width; i++)
      if (i ==  universe_prev->find(i))
        printf("component id= %d, root= %d, size= %d\n",  universe_prev->uid(i), i,  universe_prev->size(i));
  }

  // produce RGB output
  image<rgb> *output = new image<rgb>(width, height);

  // pick random colors for each component
  // TODO : replace this hard-coded depth by a sliding pool of colors
  // (in anycase 2^24 cannot represent more than 16e6 colors
  if (reset && colors != NULL) {
    delete [] colors;
    colors = NULL;
  }
  if (colors == NULL) {
    const int RGBSPACE = 16*1024*1024;
    colors = new rgb[RGBSPACE];
    for (int i = 0; i < RGBSPACE; i++)
      colors[i] = random_rgb();
  }
  printf("set colors\n");
  // replace IDs by colors
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = universe_prev->find(y*width + x);
      imRef(output, x, y) = colors[universe_prev->uid(comp)];
    }
  }

  return output;
}

#include "segment-features.h"

image<rgb> *segment_incrementalben(image<float> **img,
                                   int nfeats, int reset,
                                   float sigma, float c, int min_size,
                                   int dt, int method, int *num_ccs) {
  // get dims
  image<float> *im = img[0];
  int width = im->width();
  int height = im->height();

  printf("incr Ben\n");
  // reset
  if (reset && universe_prev != NULL) {
    delete universe_prev;
    for (int i=0; i<nfeats; i++) delete img_prev[i];
    delete [] img_prev;
    universe_prev = NULL;
    img_prev = NULL;
  }

  // smooth feature maps
  image<float> ** smoothed = new image<float> *[nfeats];
  for (int k=0; k<nfeats; k++) {
    // all channels:
    smoothed[k] = smooth(img[k], sigma);
  }



  // graph data;
  edge *edges=NULL; int num = 0;

  // 2 methods (0 = 4 edges per vertex, 1 = 8 edges per vertex)
  if (method == 4) {

    // build graph with 4-connex
    edges = new edge[width*height*3];
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
    edges = new edge[width*height*5];
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


  // segment
  universe *u;
  // compute the frame based
  u = segment_graph(width*height, num, edges, c);

  // cleanup universe if z>0
  if (reset) {
    global_id = 0;
  }

  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
    // set the id if none
    a = u->find(edges[i].a);
    // if new then attribute new uids
    if (universe_prev == NULL)
      if (u->uid(a) == -1) u->set_uid(a,global_id++);
  }
  delete [] edges;
  
  // now compute volume segm for merging
  if (img_prev != NULL) {
    image<float> ***volume;
    volume = new image<float> **[2];
    volume[0] = img_prev; // img at t-1
    volume[1] = smoothed; // img at t
    int nslices = 2;
    num = 0;
     // 2 methods (0 = 4 edges per vertex, 1 = 8 edges per vertex)
    if (method == 4) {

      // build graph with 4-connex
      edges = new edge[width*height*nslices*2 + width*height*(nslices-1)];
      for (int z = 0; z < nslices; z++) {
        for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
            if (x < width-1) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = (z * height + y) * width + (x+1);
              edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y, z, dt);
              num++;
          }
            
            if (y < height-1) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = (z * height + (y+1)) * width + x;
              edges[num].w = vdiff(volume, nfeats, x, y, z, x, y+1, z, dt);
              num++;
            }
            
            if (z < (nslices-1)) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = ((z+1) * height + y) * width + x;
              edges[num].w = vdiff(volume, nfeats, x, y, z, x, y, z+1, dt);
              num++;
            }
          }
        }
      }
    } else if (method == 8) {

    // build graph with 8-connex
      edges = new edge[width*height*nslices*4 + width*height*(nslices-1)];
      for (int z = 0; z < nslices; z++) {
        for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
            if (x < width-1) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = (z * height + y) * width + (x+1);
              edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y, z, dt);
              num++;
            }
            
            if (y < height-1) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = (z * height + (y+1)) * width + x;
              edges[num].w = vdiff(volume, nfeats, x, y, z, x, y+1, z, dt);
              num++;
            }
            
            if ((x < width-1) && (y < height-1)) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = (z * height + (y+1)) * width + (x+1);
              edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y+1, z, dt);
              num++;
            }
            
            if ((x < width-1) && (y > 0)) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = (z * height + (y-1)) * width + (x+1);
              edges[num].w = vdiff(volume, nfeats, x, y, z, x+1, y-1, z, dt);
              num++;
            }
            
            if (z < (nslices-1)) {
              edges[num].a = (z * height + y) * width + x;
              edges[num].b = ((z+1) * height + y) * width + x;
              edges[num].w = vdiff(volume, nfeats, x, y, z, x, y, z+1, dt);
              num++;
            }
          }
        }
      }
    }

    // segment
    universe *u_vol = segment_graph(width*height*nslices, num, edges, c);
    
    // post process small components
    for (int i = 0; i < num; i++) {
      int a = u_vol->find(edges[i].a);
      int b = u_vol->find(edges[i].b);
      if ((a != b) && ((u_vol->size(a) < min_size) || (u_vol->size(b) < min_size)))
        u_vol->join(a, b);
    }
    delete [] edges;


    // decide how to propagate uids
    for (int i = 0; i < width*height; i++) {
      // if this vertex is connected to its dual old then propagate uid
      if (u_vol->find(i) == u_vol->find(i+width*height*(nslices-1))) {
        int old_root = universe_prev->find(i);
        int new_root = u->find(i);
        u->set_uid(i, universe_prev->uid(old_root));
      }
    }
    for (int i = 0; i < width*height; i++) {
      // else get random
      int root = u->find(i);
      if (u->uid(root) == -1) u->set_uid(root, global_id++);
    }
    delete u_vol;

    printf("propag volume done\n");
  }

  // cleanup or alloc
  if (img_prev == NULL)
    img_prev = new image<float> *[nfeats];
  else
    for (int i=0; i<nfeats; i++) delete img_prev[i];

  // keep current image in prev
  for (int k=0; k<nfeats; k++) {
    img_prev[k] = smoothed[k];
  }
  delete [] smoothed;
  
  printf("img_prev transf done\n");
  // delete previous univ
  if (universe_prev != NULL) delete universe_prev;
  // save the current universe
  universe_prev = u;

  // global_id needs to wrap around, to avoid exceeding color index space
  if ((global_id+width*height) >= (1<<24)) global_id = 0;

  *num_ccs = u->num_sets();
  printf("got %0d sets\n", *num_ccs);

  // produce RGB output
  image<rgb> *output = new image<rgb>(width, height);

  // pick random colors for each component
  // TODO : replace this hard-coded depth by a sliding pool of colors
  // (in anycase 2^24 cannot represent more than 16e6 colors
  if (reset && colors != NULL) {
    delete [] colors;
    colors = NULL;
  }
  if (colors == NULL) {
    const int RGBSPACE = 16*1024*1024;
    colors = new rgb[RGBSPACE];
    for (int i = 0; i < RGBSPACE; i++)
      colors[i] = random_rgb();
  }
  printf("set colors\n");
  // replace IDs by colors
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y*width + x);
      imRef(output, x, y) = colors[u->uid(comp)];
    }
  }

  return output;
}

#endif
