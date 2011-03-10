// This code uses the
//--------------------------------------------------------------------------------------
//    MAXFLOW - software for computing mincut/maxflow in a graph
//                        Version 3.0
//    Yuri Boykov (yuri@csd.uwo.ca)
//    Vladimir Kolmogorov (vnk@adastral.ucl.ac.uk)
//    2001-2006                                                    #
//--------------------------------------------------------------------------------------
// available from
//  http://www.cs.adastral.ucl.ac.uk/~vnk/software.html
//
//
// Lua Wrapper: Benoit Corda, corda@nyu.edu
////////////////////////////////////////////
#ifdef LUAJIT //__cplusplus
 extern "C" {
#include <luaT.h>
#include <TH.h>
 }
#else
#include <luaT.h>
#include <TH.h>
#endif

#include "graph.h"

#define DEBUG 0

int sq(int x) {return x * x;}

extern "C" void segment(lua_State *L,
                        THTensor *output, 
                        THTensor *depth, 
                        THTensor *input,
                        double thres)
{
  //init
  int ndims = input->nDimension;
  if (ndims != 3)
    luaL_error(L, "<libkinect.semgentation> unsupported nb of dimensions must be 3D tensor");
  int width = input->size[0], height = input->size[1];
  
  // debug
  if (DEBUG) {
    printf("segment nb dim:%d\n",ndims);
    printf("depth size :%ld %ld max: %f\n",depth->size[0],depth->size[1], THTensor_max(depth));
    printf("input size :%ld %ld %ld max: %f\n",input->size[0],input->size[1],input->size[2],
           THTensor_max(input));
  }

  // Compute maxflow
  typedef Graph<double,double,double> GraphType;
  //typedef Graph<int,int,int> GraphType;
  GraphType *g = new GraphType(/*estimated # of nodes*/ width*height, /*estimated # of edges*/ width*height);
  g->add_node(width * height);

  // now add neighbor edges
  double ratio = 1.2;
  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++)
      {
        int offset = j * width + i;
        double val = THTensor_get3d(input, i, j, 1);
        double depthval =  THTensor_get2d(depth, i, j);
        g->add_tweights(offset, depthval, 2*thres - depthval); 
        int y,x, weight;
        y = j-1;
        x = i;
        if (y > 0){
          weight = abs((THTensor_get3d(input, x, y, 1) - val))/ratio;
          g->add_edge(offset, ((y) * width + x), weight, weight);
        }
        y = j;
        x = i-1;
        if (x > 0){
          weight = abs((THTensor_get3d(input, x, y, 1) - val))/ratio;
          g->add_edge(offset, ((y) * width + x), weight, weight);
        }
        y = j;
        x = i+1;
        if (x < width){
          weight = abs((THTensor_get3d(input, x, y, 1) - val))/ratio;
          g->add_edge(offset, ((y) * width + x), weight, weight);
        }
        y = j+1;
        x = i;
        if (y < height){
          weight = abs((THTensor_get3d(input, x, y, 1) - val))/ratio;
          g->add_edge(offset, ((y) * width + x), weight, weight);
        }
      }
  
  int flow = g->maxflow();
  // label the output
  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++)
      {
        THTensor_set3d(output, i, j, 0, 0);
        THTensor_set3d(output, i, j, 1, 0);
        THTensor_set3d(output, i, j, 2, 0);
        if (g->what_segment(j * width + i) != GraphType::SOURCE)
          {
            THTensor_set3d(output, i, j, 0, 1);
            THTensor_set3d(output, i, j, 1, 1);
            THTensor_set3d(output, i, j, 2, 1);
          }
      }
  delete g;
}
