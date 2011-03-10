#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_AbsModule_id = NULL;

static int nn_AbsModule_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

  THTensor_resizeAs(output, input);

  TH_TENSOR_APPLY2(double, output, double, input, \
                   *output_p = fabs(*input_p);)

  return 1;
}

static int nn_AbsModule_backward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);

  THTensor_resizeAs(gradInput, input);
  TH_TENSOR_APPLY3(double, gradInput, double, gradOutput, double, input, \
                   double z = *input_p; \
                   *gradInput_p = *gradOutput_p * (z >= 0 ? 1 : -1);)
  return 1;
}

static const struct luaL_Reg nn_AbsModule__ [] = {
  {"forward", nn_AbsModule_forward},
  {"backward", nn_AbsModule_backward},
  {NULL, NULL}
};

void nn_AbsModule_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_AbsModule_id = luaT_newmetatable(L, "nn.AbsModule", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_AbsModule__);
  lua_pop(L, 1);
}
