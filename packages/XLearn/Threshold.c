#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_Threshold_id = NULL;

static int nn_Threshold_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  double val = luaT_getfieldchecknumber(L, 1, "val");
  double threshold = luaT_getfieldchecknumber(L, 1, "threshold");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);
  
  THTensor_resizeAs(output, input);
  //THTensor_copy(output, input);
  TH_TENSOR_APPLY2(double, output, double, input, \
                  *output_p = (*input_p > threshold) ? *input_p : val;);

  return 1;
}

static const struct luaL_Reg nn_Threshold__ [] = {
  {"forward", nn_Threshold_forward},
  {NULL, NULL}
};

void nn_Threshold_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_Threshold_id = luaT_newmetatable(L, "nn.Threshold", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_Threshold__);
  lua_pop(L, 1);
}
