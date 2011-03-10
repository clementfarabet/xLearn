
#ifndef GRAPH
#define GRAPH

#include <TH.h>

#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))

// dissimilarity measure between pixels, in some feature space
static inline float ndiff_2d(THTensor *img,
                             int x1, int y1, int x2, int y2,
                             int dt) {
  int nfeats = img->size[2];
  float dist  = 0;
  float dot   = 0;
  float normx = 0;
  float normy = 0;
  float res = 0;
  for (int i=0; i<nfeats; i++) {
    if (dt == euclid) {
      dist  += square(THTensor_get3d(img, x1, y1, i)-THTensor_get3d(img, x2, y2, i));
    } else if (dt == angle) {
      dot   += THTensor_get3d(img, x1, y1, i) * THTensor_get3d(img, x2, y2, i);
      normx += square(THTensor_get3d(img, x1, y1, i));
      normy += square(THTensor_get3d(img, x2, y2, i));
    }
  }
  if (dt == euclid) res = sqrt(dist);
  else if (dt == angle) res = acos(dot/(sqrt(normx)*sqrt(normy)));
  return res;
}

// generate graph from image
static THTensor *image2graph(THTensor *img, int connex, int dt, int *num) {
  // result
  THTensor *graph;
  int width = img->size[0];
  int height = img->size[1];

  // connex can be one of: 4 | 8
  if (connex == 4) {

    // create a WxHx2 tensor, to hold all the edges
    // plane 1 contains the horizontal edges, plane 2 the vertical edges
    graph = THTensor_newWithSize3d(width, height, connex/2);
    THTensor_fill(graph,-1);

    // build graph with 4-connex
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          THTensor_set3d(graph, x, y, 0, ndiff_2d(img, x, y, x+1, y, dt));
          (*num)++;
        }
        if (y < height-1) {
          THTensor_set3d(graph, x, y, 1, ndiff_2d(img, x, y, x, y+1, dt));
          (*num)++;
        }
      }
    }

  } else if (connex == 8) {

    // create a WxHx4 tensor, to hold all the edges
    // plane 1 contains the horizontal edges, plane 2 the vertical edges
    // plane 3 the diag right edges, plane 4 the diag left edges
    graph = THTensor_newWithSize3d(width, height, connex/2);
    THTensor_fill(graph,-1);

    // build graph with 4-connex
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (x < width-1) {
          THTensor_set3d(graph, x, y, 0, ndiff_2d(img, x, y, x+1, y, dt));
          (*num)++;
        }
        if (y < height-1) {
          THTensor_set3d(graph, x, y, 1, ndiff_2d(img, x, y, x, y+1, dt));
          (*num)++;
        }
        if ((x < width-1) && (y < height-1)) {
          THTensor_set3d(graph, x, y, 1, ndiff_2d(img, x, y, x+1, y+1, dt));
          (*num)++;
        }
        if ((x < width-1) && (y > 0)) {
          THTensor_set3d(graph, x, y, 1, ndiff_2d(img, x, y, x+1, y-1, dt));
          (*num)++;
        }
      }
    }

  } else {
    THError("<libmstsegm.image2graph> unsupported connectivity (only 8 or 4 are valid)");
  }

  // return result
  return graph;
}

// generate image from graph
static THTensor *graph2image(THTensor *graph, double threshold) {
  // new img
  int width = graph->size[0];
  int height = graph->size[1];
  int edges = graph->size[2];
  THTensor *img = THTensor_newWithSize2d(width, height);

  // check edges
  if (edges != 2)
    THError("<libmstsegm.graph2image> supported connexity: 4 only");

  // gen IDs
  int idg = 0;

  // for second pass, store all merges
  int *merges = new int[width*height];
  for (int i=0; i<width*height; i++) merges[i] = -1;

  // create image, in 2 passes
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (y == 0) {
        THTensor_set2d(img, x, y, idg);
        if (x<(width-1))
          if (THTensor_get3d(graph, x, y, 0) >= threshold) idg++;
      } else {
        if ((x > 0) && (THTensor_get3d(graph, x-1, y, 0) < threshold)) {
          if (THTensor_get3d(graph, x, y-1, 1) < threshold) {
            double mn = min(THTensor_get2d(img, x-1, y), THTensor_get2d(img, x, y-1));
            double mx = max(THTensor_get2d(img, x-1, y), THTensor_get2d(img, x, y-1));
            if (mn == mx) {
              THTensor_set2d(img, x, y, mn);
            } else {
              merges[(int)mx] = mn;
              THTensor_set2d(img, x, y, mn);
            }
          } else {
            THTensor_set2d(img, x, y, THTensor_get2d(img, x-1, y));
          }
        }
        else if (THTensor_get3d(graph, x, y-1, 1) < threshold)
          THTensor_set2d(img, x, y, THTensor_get2d(img, x, y-1));
        else 
          THTensor_set2d(img, x, y, ++idg);
      }
    }
  }

  // second pass cleanup
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int id = (int)THTensor_get2d(img, x, y);
      if (merges[id] != -1) THTensor_set2d(img, x, y, merges[id]);
    }
  }

  // cleanup
  delete [] merges;

  // return result
  return img;
}

#endif
