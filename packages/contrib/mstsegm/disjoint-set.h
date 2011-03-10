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

#ifndef DISJOINT_SET
#define DISJOINT_SET

// threshold function
#define THRESHOLD(size, c) (c/size)

// disjoint-set forests using union-by-rank and path compression (sort of).
typedef struct {
  int rank;
  int p;
  int size;
  int uid;
} uni_elt;

class universe {
 public:
  universe(int elements);
  universe(int elements, float c);
  universe(int elements, float c, universe *old_u);
  ~universe();
  int find(int x);  
  void join(int x, int y);
  void joinincr(int x, int y);
  // get methods:
  int size(int x) const { return elts[x].size; }
  int uid(int x) const { return elts[x].uid; }
  int num_sets() const { return num; }
  float *thresholds() const { return threshold; }
  uni_elt element(int idx) const { return elts[idx]; };
  // set methods:
  void set_uid(int x, int id) const { elts[x].uid = id; }
  void set_p(int x, int p) const { elts[x].p = p; }
  void set_size(int x, int s) const { elts[x].size = s; }
  void set_rank(int x, int r) const { elts[x].rank = r; }
  void set_num_sets(int n) { num = n; }

  int num_vertex;
 private:
  float *threshold;
  uni_elt *elts;
  int num;
};

universe::universe(int elements) {
  elts = new uni_elt[elements];
  num = elements;
  threshold = NULL;
  for (int i = 0; i < elements; i++) {
    elts[i].rank = 0;
    elts[i].size = 1;
    elts[i].p = i;
    elts[i].uid = -1;
  }
}

universe::universe(int elements, float c) {
  elts = new uni_elt[elements];
  num = elements;
  for (int i = 0; i < elements; i++) {
    elts[i].rank = 0;
    elts[i].size = 1;
    elts[i].p = i;
    elts[i].uid = -1;
  }
  // keep the threshold of all components
  threshold = new float[elements];
  // new thresh
  for (int i = 0; i < elements; i++)
    threshold[i] = THRESHOLD(1,c);
}

universe::universe(int elements, float c, universe *old_u) {
  elts = new uni_elt[elements*2];
  num = elements + old_u->num;

  num_vertex = elements;
  // old elmts
  for (int i = 0; i < elements; i++) {
    elts[i].rank = old_u->elts[i].rank;
    elts[i].size = old_u->elts[i].size;
    elts[i].p = old_u->elts[i].p;
    elts[i].uid = old_u->elts[i].uid;
  }

  // new elemts
  for (int i = elements; i < elements*2; i++) {
    elts[i].rank = 0;
    elts[i].size = 1;
    elts[i].p = i;
    elts[i].uid = -1;
  }

  // keep the threshold of all components
  threshold = new float[elements*2];
  // old thresh
  for (int i = 0; i < elements; i++)
    threshold[i] = old_u->threshold[i];

  // new thresh
  for (int i = elements; i < elements*2; i++)
    threshold[i] = THRESHOLD(1,c);
}
  
universe::~universe() {
  delete [] elts;
  if (threshold != NULL) delete [] threshold;
}

int universe::find(int x) {
  int y = x;
  while (y != elts[y].p)
    y = elts[y].p;
  elts[x].p = y;
  return y;
}

void universe::join(int x, int y) {
  if (elts[x].rank > elts[y].rank) {
    elts[y].p = x;
    elts[x].size += elts[y].size;
  } else {
    elts[x].p = y;
    elts[y].size += elts[x].size;
    if (elts[x].rank == elts[y].rank)
      elts[y].rank++;
  }
  num--;
}

void universe::joinincr(int x, int y) {
  if (elts[x].rank > elts[y].rank) {
    elts[y].p = x;
    if ((elts[y].uid != -1) && (elts[x].uid == -1 || elts[x].size < elts[y].size) ){
      printf("joining %d with %d\n",x ,y);
      elts[x].uid = elts[y].uid;
    }
    elts[x].size += elts[y].size;    
  } else {
    elts[x].p = y;
    if (elts[x].rank == elts[y].rank)
      elts[y].rank++;
    if ((elts[x].uid != -1) && (elts[y].uid == -1 || elts[y].size < elts[x].size)){
      printf("joining %d with %d\n",y ,x);
      elts[y].uid = elts[x].uid;
    }
    elts[y].size += elts[x].size;
  }
  num--;
}

#endif
