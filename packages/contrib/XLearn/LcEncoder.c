#include "luaT.h"
#include "TH.h"
#include <math.h>

static const void* torch_Tensor_id = NULL;
static const void* nn_LcEncoder_id = NULL;

static int nn_LcEncoder_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  int winX = luaT_getfieldcheckint(L, 1, "winX");
  int winY = luaT_getfieldcheckint(L, 1, "winY");
  int woutX = luaT_getfieldcheckint(L, 1, "woutX");
  int woutY = luaT_getfieldcheckint(L, 1, "woutY");
  double xStep = luaT_getfieldchecknumber(L, 1, "xStep");
  double yStep = luaT_getfieldchecknumber(L, 1, "yStep");

  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);
  THTensor *bias = luaT_getfieldcheckudata(L, 1, "bias", torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

  luaL_argcheck(L, input->nDimension == 3, 2, "3D tensor expected");
  luaL_argcheck(L, input->size[2] == 1, 2, "invalid input 3rd dim size has to be 1");

  THTensor *inputPlane, *inputNarrowedX, *inputNarrowedYX;
  THTensor *weightSelectedX, *weightSelectedYX;
  
  inputPlane = THTensor_new();
  inputNarrowedX = THTensor_new();
  inputNarrowedYX = THTensor_new();
  weightSelectedX = THTensor_new();
  weightSelectedYX = THTensor_new();

  // get output size from input
  THTensor_resize3d(output,
                    (input->size[0] - winX+1) / xStep, 
                    (input->size[1] - winY+1) / yStep,
                    1);
  
  THTensor_select(inputPlane, input, 2, 0);


  int y,x,iy,ix,wy,wx;
  for (y = 0; y<output->size[1]; y++)
    {
      iy = (int)floor(y*yStep);
      wy = y%woutY;
      for (x = 0; x<output->size[0]; x++)
        {
          ix = (int)floor(x*xStep);
          wx = x%woutX;
          THTensor_narrow(inputNarrowedX, inputPlane, 0, ix, winX);
          THTensor_narrow(inputNarrowedYX, inputNarrowedX, 1, iy, winY);
          THTensor_select(weightSelectedX, weight, 3, wy);
          THTensor_select(weightSelectedYX, weightSelectedX, 2, wx);
          double dot = THTensor_dot(inputNarrowedYX, weightSelectedYX);
          double biasSelect = THTensor_get2d(bias,wx,wy);
          THTensor_set3d(output,x,y,0,dot+biasSelect);
        }
    }

  THTensor_free(inputPlane);
  THTensor_free(inputNarrowedX);
  THTensor_free(inputNarrowedYX);
  THTensor_free(weightSelectedX);
  THTensor_free(weightSelectedYX);
  return 1;
}

static int nn_LcEncoder_backward(lua_State *L)
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
  THTensor *bias = luaT_getfieldcheckudata(L, 1, "bias", torch_Tensor_id);
  THTensor *gradBias = luaT_getfieldcheckudata(L, 1, "gradBias", torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);


  /* ----------------------- gradWeight ----------------------- */
  THTensor_fill(gradWeight, 0);

  THTensor *inputPlane, *inputNarrowedX, *inputNarrowedYX;
  inputPlane = THTensor_new();
  inputNarrowedX = THTensor_new();
  inputNarrowedYX = THTensor_new();
  THTensor_select(inputPlane, input, 2, 0);

  THTensor *gradWeightSelectedX, *gradWeightSelectedYX;
  gradWeightSelectedX = THTensor_new();
  gradWeightSelectedYX = THTensor_new();

  /* ----------------------- gradInput ------------------------ */
  THTensor_resizeAs(gradInput, input);
  THTensor_fill(gradInput, 0);

  THTensor *gradInputPlane, *gradInputNarrowedX, *gradInputNarrowedYX;
  gradInputPlane = THTensor_new();
  gradInputNarrowedX = THTensor_new();
  gradInputNarrowedYX = THTensor_new();
  THTensor_select(gradInputPlane, gradInput, 2, 0);

  THTensor *weightSelectedX, *weightSelectedYX;
  weightSelectedX = THTensor_new();
  weightSelectedYX = THTensor_new();


  int y,x,iy,ix,wy,wx;
  for (y = 0; y<gradOutput->size[1]; y++)
    {
      iy = (int)floor(y*yStep);
      wy = y%woutY;
      for (x = 0; x<gradOutput->size[0]; x++)
        {
          ix = (int)floor(x*xStep);
          wx = x%woutX;
          double gradOutVal = THTensor_get3d(gradOutput,x,y,0);

          /* ----------------------- gradWeight ----------------------- */
          THTensor_narrow(inputNarrowedX, inputPlane, 0, ix, winX);
          THTensor_narrow(inputNarrowedYX, inputNarrowedX, 1, iy, winY);
          THTensor_select(gradWeightSelectedX, gradWeight, 3, wy);
          THTensor_select(gradWeightSelectedYX, gradWeightSelectedX, 2, wx);
          THTensor_addTensor(gradWeightSelectedYX, gradOutVal, inputNarrowedYX);
          /* ----------------------- gradBias ----------------------- */
          THTensor_set2d(gradBias,wx,wy, THTensor_get2d(gradBias,wx,wy) + gradOutVal);
          /* ----------------------- gradInput ------------------------ */
          THTensor_narrow(gradInputNarrowedX, gradInputPlane, 0, ix, winX);
          THTensor_narrow(gradInputNarrowedYX, gradInputNarrowedX, 1, iy, winY);
          THTensor_select(weightSelectedX, weight, 3, wy);
          THTensor_select(weightSelectedYX, weightSelectedX, 2, wx);
          THTensor_addTensor(gradInputNarrowedYX, gradOutVal, weightSelectedYX);
        }
    }

  /* free gradWeight  */
  THTensor_free(inputPlane);
  THTensor_free(inputNarrowedX);
  THTensor_free(inputNarrowedYX);
  THTensor_free(gradWeightSelectedX);
  THTensor_free(gradWeightSelectedYX);
  /* free gradInput  */
  THTensor_free(gradInputPlane);
  THTensor_free(gradInputNarrowedX);
  THTensor_free(gradInputNarrowedYX);
  THTensor_free(weightSelectedX);
  THTensor_free(weightSelectedYX);

  return 1;
}

static const struct luaL_Reg nn_LcEncoder__ [] = {
  {"forward", nn_LcEncoder_forward},
  {"backward", nn_LcEncoder_backward},
  {NULL, NULL}
};

void nn_LcEncoder_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_LcEncoder_id = luaT_newmetatable(L, "nn.LcEncoder", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_LcEncoder__);
  lua_pop(L, 1);
}
