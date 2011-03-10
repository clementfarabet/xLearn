#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_AbsModuleHessian_id = NULL;

static int nn_AbsModuleHessian_backwardHessian(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  THTensor *hessianOutput = luaT_checkudata(L, 3, torch_Tensor_id);
  THTensor *hessianInput = luaT_getfieldcheckudata(L, 1, "hessianInput", torch_Tensor_id);

  THTensor_resizeAs(hessianInput, input);
  TH_TENSOR_APPLY3(double, hessianInput, double, hessianOutput, double, input, \
                   double z = *input_p; \
                   double squaredDerivate = (z >= 0 ? 1 : -1)*(z >= 0 ? 1 : -1);
                   *hessianInput_p = *hessianOutput_p * squaredDerivate;)
  return 1;
}

static const struct luaL_Reg nn_AbsModuleHessian__ [] = {
  {"backwardHessian", nn_AbsModuleHessian_backwardHessian},
  {NULL, NULL}
};

void nn_AbsModuleHessian_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_AbsModuleHessian_id = luaT_newmetatable(L, "nn.AbsModuleHessian", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_AbsModuleHessian__);
  lua_pop(L, 1);
}
