#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_SparseCriterion_id = NULL;

static int nn_SparseCriterion_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  double sum;

  sum = 0;
  TH_TENSOR_APPLY(double, input, sum += fabs(*input_p);)

  if(sizeAverage)
    sum /= THTensor_nElement(input);

  lua_pushnumber(L, sum);
  lua_setfield(L, 1, "output");

  lua_pushnumber(L, sum);
  return 1;
}

static int nn_SparseCriterion_backward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);
  double norm = (sizeAverage ? 1./((double)THTensor_nElement(input)) : 1.);

  THTensor_resizeAs(gradInput, input);
  TH_TENSOR_APPLY2(double, gradInput, double, input,
                   *gradInput_p = ( *input_p >= 0 ? norm : -norm);)

  return 1;
}

static const struct luaL_Reg nn_SparseCriterion__ [] = {
  {"forward", nn_SparseCriterion_forward},
  {"backward", nn_SparseCriterion_backward},
  {NULL, NULL}
};

void nn_SparseCriterion_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_SparseCriterion_id = luaT_newmetatable(L, "nn.SparseCriterion", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_SparseCriterion__);
  lua_pop(L, 1);
}
