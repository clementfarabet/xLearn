#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_LogSoftMax_id = NULL;

static int nn_LogSoftMax_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);
	double sum = THLogZero;
  
  THTensor_resizeAs(output, input);

  TH_TENSOR_APPLY2(double, output, double, input, \
                   double z = *input_p; \
                   *output_p = z; \
                   sum = THLogAdd(sum, z);)

  THTensor_add(output, -sum);

  return 1;
}

static int nn_LogSoftMax_backward(lua_State *L)
{
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);
  double sum = THTensor_sum(gradOutput);

  THTensor_resizeAs(gradInput, output);
  TH_TENSOR_APPLY3(double, gradInput, double, gradOutput, double, output, \
                     *gradInput_p = *gradOutput_p - exp(*output_p)*sum;);
  return 1;
}

static const struct luaL_Reg nn_LogSoftMax__ [] = {
  {"forward", nn_LogSoftMax_forward},
  {"backward", nn_LogSoftMax_backward},
  {NULL, NULL}
};

void nn_LogSoftMax_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_LogSoftMax_id = luaT_newmetatable(L, "nn.LogSoftMax", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_LogSoftMax__);
  lua_pop(L, 1);
}
