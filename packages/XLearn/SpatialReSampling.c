#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_SpatialReSampling_id = NULL;

#ifndef MAX
#define MAX(a,b) ( ((a)>(b)) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )
#endif

static int nn_SpatialReSampling_forward(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  int owidth = luaT_getfieldcheckint(L, 1, "owidth");
  int oheight = luaT_getfieldcheckint(L, 1, "oheight");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

  // check dims
  luaL_argcheck(L, input->nDimension == 3, 2, "3D tensor expected");

  // dims
  int iwidth = input->size[0];
  int iheight = input->size[1];
  int ochannels = input->size[2];

  // resize output
  THTensor_resize3d(output, owidth, oheight, ochannels);

  // select planes
  THTensor *outputPlane = THTensor_new();
  THTensor *inputPlane = THTensor_new();

  // mapping ratios
  float wratio = (float)(iwidth-1) / (owidth-1);
  float hratio = (float)(iheight-1) / (oheight-1);

  // resample each plane
  int k;
  for (k=0; k<ochannels; k++) {
    // get planes
    THTensor_select(inputPlane, input, 2, k);
    THTensor_select(outputPlane, output, 2, k);

    // for each plane, resample
    int x,y;
    for (y=0; y<oheight; y++) {
      for (x=0; x<owidth; x++) {
        // subpixel position:
        float ix = wratio*x;
        float iy = hratio*y;

        // 4 nearest neighbors:
        float ix_nw = floor(ix);
        float iy_nw = floor(iy);
        float ix_ne = ix_nw + 1;
        float iy_ne = iy_nw;
        float ix_sw = ix_nw;
        float iy_sw = iy_nw + 1;
        float ix_se = ix_nw + 1;
        float iy_se = iy_nw + 1;

        // get surfaces to each neighbor:
        float se = (ix-ix_nw)*(iy-iy_nw);
        float sw = (ix_ne-ix)*(iy-iy_ne);
        float ne = (ix-ix_sw)*(iy_sw-iy);
        float nw = (ix_se-ix)*(iy_se-iy);

        // weighted sum of neighbors:
        double sum = THTensor_get2d(inputPlane, ix_nw, iy_nw) * nw
          + THTensor_get2d(inputPlane, MIN(ix_ne,iwidth-1), iy_ne) * ne
          + THTensor_get2d(inputPlane, ix_sw, MIN(iy_sw,iheight-1)) * sw
          + THTensor_get2d(inputPlane, MIN(ix_se,iwidth-1), MIN(iy_se,iheight-1)) * se;

        // set output
        THTensor_set2d(outputPlane, x, y, sum);
      }
    }
  }

  // cleanup
  THTensor_free(inputPlane);
  THTensor_free(outputPlane);
  return 1;
}

static int nn_SpatialReSampling_backward(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);

  // dims
  int iwidth = input->size[0];
  int iheight = input->size[1];
  int ichannels = input->size[2];
  int owidth = gradOutput->size[0];
  int oheight = gradOutput->size[1];
  int ochannels = gradOutput->size[2];

  // resize gradInput
  THTensor_resize3d(gradInput, iwidth, iheight, ichannels);
  THTensor_zero(gradInput);

  // select planes
  THTensor *gradOutputPlane = THTensor_new();
  THTensor *gradInputPlane = THTensor_new();

  // mapping ratios
  float wratio = (float)(iwidth-1) / (owidth-1);
  float hratio = (float)(iheight-1) / (oheight-1);

  // compute gradients for each plane
  int k;
  for (k=0; k<ochannels; k++) {
    // get planes
    THTensor_select(gradInputPlane, gradInput, 2, k);
    THTensor_select(gradOutputPlane, gradOutput, 2, k);

    // for each plane, resample
    int x,y;
    for (y=0; y<oheight; y++) {
      for (x=0; x<owidth; x++) {
        // subpixel position:
        float ix = wratio*x;
        float iy = hratio*y;

        // 4 nearest neighbors:
        float ix_nw = floor(ix);
        float iy_nw = floor(iy);
        float ix_ne = ix_nw + 1;
        float iy_ne = iy_nw;
        float ix_sw = ix_nw;
        float iy_sw = iy_nw + 1;
        float ix_se = ix_nw + 1;
        float iy_se = iy_nw + 1;

        // get surfaces to each neighbor:
        float se = (ix-ix_nw)*(iy-iy_nw);
        float sw = (ix_ne-ix)*(iy-iy_ne);
        float ne = (ix-ix_sw)*(iy_sw-iy);
        float nw = (ix_se-ix)*(iy_se-iy);

        // output gradient
        double ograd = THTensor_get2d(gradOutputPlane, x, y);

        // accumulate gradient
        THTensor_set2d(gradInputPlane, ix_nw, iy_nw, 
                       THTensor_get2d(gradInputPlane, ix_nw, iy_nw) + nw * ograd);
        THTensor_set2d(gradInputPlane, MIN(ix_ne,iwidth-1), iy_ne, 
                       THTensor_get2d(gradInputPlane, MIN(ix_ne,iwidth-1), iy_ne) + ne * ograd);
        THTensor_set2d(gradInputPlane, ix_sw, MIN(iy_sw,iheight-1), 
                       THTensor_get2d(gradInputPlane, ix_sw, MIN(iy_sw,iheight-1)) + sw * ograd);
        THTensor_set2d(gradInputPlane, MIN(ix_se,iwidth-1), MIN(iy_se,iheight-1), 
                       THTensor_get2d(gradInputPlane, MIN(ix_se,iwidth-1), MIN(iy_se,iheight-1)) + se * ograd);
      }
    }
  }

  // cleanup
  THTensor_free(gradInputPlane);
  THTensor_free(gradOutputPlane);
  return 1;
}

static const struct luaL_Reg nn_SpatialReSampling__ [] = {
  {"forward", nn_SpatialReSampling_forward},
  {"backward", nn_SpatialReSampling_backward},
  {NULL, NULL}
};

void nn_SpatialReSampling_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_SpatialReSampling_id = luaT_newmetatable(L, "nn.SpatialReSampling", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_SpatialReSampling__);
  lua_pop(L, 1);
}
