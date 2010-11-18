#include "luaT.h"
#include "TH.h"
#include <math.h>

static const void* torch_Tensor_id = NULL;
static const void* nn_LcDecoder_id = NULL;

static int nn_LcDecoder_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  int winX = luaT_getfieldcheckint(L, 1, "winX");
  int winY = luaT_getfieldcheckint(L, 1, "winY");
  int woutX = luaT_getfieldcheckint(L, 1, "woutX");
  int woutY = luaT_getfieldcheckint(L, 1, "woutY");
  double xStep = luaT_getfieldchecknumber(L, 1, "xStep");
  double yStep = luaT_getfieldchecknumber(L, 1, "yStep");

  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

  luaL_argcheck(L, input->nDimension == 3, 2, "3D tensor expected");
  luaL_argcheck(L, input->size[2] == 1, 2, "invalid input 3rd dim size has to be 1");

  // get output size from input
  THTensor_resize3d(output,
                    input->size[0]*xStep + winX-1,
                    input->size[1]*yStep + winY-1,
                    1); 
  THTensor_fill(output, 0);

  THTensor *outputPlane, *outputNarrowedX, *outputNarrowedYX;
  outputPlane = THTensor_new();
  outputNarrowedX = THTensor_new();
  outputNarrowedYX = THTensor_new();
  THTensor_select(outputPlane, output, 2, 0);

  THTensor *weightSelectedX, *weightSelectedYX;
  weightSelectedX = THTensor_new();
  weightSelectedYX = THTensor_new();

  int y,x,iy,ix,wy,wx;
  for (y = 0; y<input->size[1]; y++)
    {
      iy = (int)floor(y*yStep);
      wy = y%woutY;
      for (x = 0; x<input->size[0]; x++)
        {
          ix = (int)floor(x*xStep);
          wx = x%woutX;
          double inputVal = THTensor_get3d(input,x,y,0);
          THTensor_narrow(outputNarrowedX, outputPlane, 0, ix, winX);
          THTensor_narrow(outputNarrowedYX, outputNarrowedX, 1, iy, winY);
          THTensor_select(weightSelectedX, weight, 3, wy);
          THTensor_select(weightSelectedYX, weightSelectedX, 2, wx);
          THTensor_addTensor(outputNarrowedYX, inputVal, weightSelectedYX);
        }
    }

  THTensor_free(outputPlane);
  THTensor_free(outputNarrowedX);
  THTensor_free(outputNarrowedYX);
  THTensor_free(weightSelectedX);
  THTensor_free(weightSelectedYX);

  return 1;
}

static int nn_LcDecoder_backward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);  
  int winX = luaT_getfieldcheckint(L, 1, "winX");
  int winY = luaT_getfieldcheckint(L, 1, "winY");
  int woutX = luaT_getfieldcheckint(L, 1, "woutX");
  int woutY = luaT_getfieldcheckint(L, 1, "woutY");
  double xStep = luaT_getfieldchecknumber(L, 1, "xStep");
  double yStep = luaT_getfieldchecknumber(L, 1, "yStep");

  luaL_argcheck(L, input->nDimension == 3, 2, "input 3D tensor expected");
  luaL_argcheck(L, input->size[2] == 1, 2, "invalid input 3rd dim size has to be 1");
  luaL_argcheck(L, gradOutput->nDimension == 3, 3, "gradOutput 3D tensor expected");
  luaL_argcheck(L, gradOutput->size[2] == 1, 3, "invalid gradOutput 3rd dim size has to be 1");

  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);
  THTensor *gradWeight = luaT_getfieldcheckudata(L, 1, "gradWeight", torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);


  /* ----------------------- gradWeight ----------------------- */
  THTensor_fill(gradWeight, 0);

  THTensor *gradOutputPlane, *gradOutputNarrowedX, *gradOutputNarrowedYX;
  gradOutputPlane = THTensor_new();
  gradOutputNarrowedX = THTensor_new();
  gradOutputNarrowedYX = THTensor_new();
  THTensor_select(gradOutputPlane, gradOutput, 2, 0);

  THTensor *gradWeightSelectedX, *gradWeightSelectedYX;
  gradWeightSelectedX = THTensor_new();
  gradWeightSelectedYX = THTensor_new();

  /* ----------------------- gradInput ------------------------ */
  THTensor_resizeAs(gradInput, input);

  THTensor *weightSelectedX, *weightSelectedYX;
  weightSelectedX = THTensor_new();
  weightSelectedYX = THTensor_new();


  int y,x,iy,ix,wy,wx;
  for (y = 0; y<input->size[1]; y++)
    {
      iy = (int)floor(y*yStep);
      wy = y%woutY;
      for (x = 0; x<input->size[0]; x++)
        {
          ix = (int)floor(x*xStep);
          wx = x%woutX;

          /* ----------------------- gradWeight ----------------------- */
          double inputVal = THTensor_get3d(input,x,y,0);
          THTensor_narrow(gradOutputNarrowedX, gradOutputPlane, 0, ix, winX);
          THTensor_narrow(gradOutputNarrowedYX, gradOutputNarrowedX, 1, iy, winY);
          THTensor_select(gradWeightSelectedX, gradWeight, 3, wy);
          THTensor_select(gradWeightSelectedYX, gradWeightSelectedX, 2, wx);
          THTensor_addTensor(gradWeightSelectedYX, inputVal, gradOutputNarrowedYX);

          /* ----------------------- gradInput ------------------------ */
          THTensor_select(weightSelectedX, weight, 3, wy);
          THTensor_select(weightSelectedYX, weightSelectedX, 2, wx);
          double dot = THTensor_dot(gradOutputNarrowedYX, weightSelectedYX);
          THTensor_set3d(gradInput,x,y,0,dot);
        }
    }

  /* free gradWeight  */
  THTensor_free(gradOutputPlane);
  THTensor_free(gradOutputNarrowedX);
  THTensor_free(gradOutputNarrowedYX);
  THTensor_free(gradWeightSelectedX);
  THTensor_free(gradWeightSelectedYX);
  /* free gradInput  */
  THTensor_free(weightSelectedX);
  THTensor_free(weightSelectedYX);

  return 1;
}

static const struct luaL_Reg nn_LcDecoder__ [] = {
  {"forward", nn_LcDecoder_forward},
  {"backward", nn_LcDecoder_backward},
  {NULL, NULL}
};

void nn_LcDecoder_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_LcDecoder_id = luaT_newmetatable(L, "nn.LcDecoder", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_LcDecoder__);
  lua_pop(L, 1);
}
