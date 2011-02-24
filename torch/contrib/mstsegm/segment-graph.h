/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef SEGMENT_GRAPH
#define SEGMENT_GRAPH

#include <algorithm>
#include <cmath>
#include "disjoint-set.h"

typedef struct {
  float w;
  int a, b;
} edge;

bool operator<(const edge &a, const edge &b) {
  return a.w < b.w;
}

/*
 * Segment a graph
 *
 * Returns a disjoint-set forest representing the segmentation.
 *
 * num_vertices: number of vertices in graph.
 * num_edges: number of edges in graph
 * edges: array of edges.
 * c: constant for treshold function.
 */
static universe *segment_graph(int num_vertices, int num_edges, edge *edges, 
                               float c) { 
  // sort edges by weight
  std::sort(edges, edges + num_edges);

  // make a disjoint-set forest
  universe *u = new universe(num_vertices, c);

  // init thresholds
  float *threshold = u->thresholds();

  // for each edge, in non-decreasing weight order...
  for (int i = 0; i < num_edges; i++) {
    edge *pedge = &edges[i];
    
    // components conected by this edge
    int a = u->find(pedge->a);
    int b = u->find(pedge->b);
    if (a != b) {
      if ((pedge->w <= threshold[a]) &&
	  (pedge->w <= threshold[b])) {
	u->join(a, b);
	a = u->find(a);
	threshold[a] = pedge->w + THRESHOLD(u->size(a), c);
      }
    }
  }

  // free up
  return u;
}

/*
 * Segment a graph, incrementally (e.g. by 2D slice)
 *
 * Returns a disjoint-set forest representing the segmentation for the current slice
 *
 * old_u: previous universe, representing the history
 * num_vertices: number of vertices in graph
 * num_edges: number of edges in graph
 * edges: array of edges.
 * c: constant for treshold function.
 */
static universe *incremental_segment_graph(universe *old_u,
                                           int num_vertices, int num_edges, edge *edges, 
                                           float c) { 
  // sort edges by weight
  std::sort(edges, edges + num_edges);

  // check that old universe exists
  if (old_u == NULL)
    THError("<libmstsegm.infer [incremental_segment_graph]> old_u == NULL");

  // make a disjoint-set forest
  universe *u = new universe(num_vertices, c, old_u);

  // init thresholds
  float *threshold = u->thresholds();

  // for each edge, in non-decreasing weight order...
  for (int i = 0; i < num_edges; i++) {
    edge *pedge = &edges[i];

    // components conected by this edge
    int a = u->find(pedge->a);
    int b = u->find(pedge->b);
    if (a != b) {
      if ((pedge->w <= threshold[a]) &&
	  (pedge->w <= threshold[b])) {
	u->joinincr(a, b);
	a = u->find(a);
	threshold[a] = pedge->w + THRESHOLD(u->size(a), c);
      }
    }
  }

  // free up
  return u;
}

/*
 * Segment a graph
 *
 * Returns a disjoint-set forest representing the segmentation.
 *
 * num_vertices: number of vertices in graph.
 * num_edges: number of edges in graph
 * edges: array of edges.
 * c: constant for treshold function.
 */
static universe *segment_graph_wo_size(int num_vertices, int num_edges, edge *edges, float c) { 
  // sort edges by weight
  std::sort(edges, edges + num_edges);

  // make a disjoint-set forest
  universe *u = new universe(num_vertices, c);

  // init thresholds
  float *threshold = u->thresholds();

  // for each edge, in non-decreasing weight order...
  for (int i = 0; i < num_edges; i++) {
    edge *pedge = &edges[i];
    
    // components conected by this edge
    int a = u->find(pedge->a);
    int b = u->find(pedge->b);
    if (a != b) {
      if ((pedge->w <= threshold[a]) &&
	  (pedge->w <= threshold[b])) {
	u->join(a, b);
	a = u->find(a);
	threshold[a] = pedge->w + THRESHOLD(sqrt(u->size(a)), c);
      }
    }
  }

  // free up
  return u;
}

#endif
