#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_TanhHessian_id = NULL;

static int nn_TanhHessian_backwardHessian(lua_State *L) {
  THTensor *hessianOutput = luaT_checkudata(L, 3, torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);
  THTensor *hessianInput = luaT_getfieldcheckudata(L, 1, "hessianInput", torch_Tensor_id);

  THTensor_resizeAs(hessianInput, output);
  TH_TENSOR_APPLY3(double, hessianInput, double, hessianOutput, double, output, \
                   double z = *output_p; \
                   double squaredDerivate = (1. - z*z)*(1. - z*z); \
                   *hessianInput_p = *hessianOutput_p * squaredDerivate;);

  return 1;
}

static const struct luaL_Reg nn_TanhHessian__ [] = {
  {"backwardHessian", nn_TanhHessian_backwardHessian},
  {NULL, NULL}
};

void nn_TanhHessian_init(lua_State *L) {
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_TanhHessian_id = luaT_newmetatable(L, "nn.TanhHessian", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_TanhHessian__);
  lua_pop(L, 1);
}
