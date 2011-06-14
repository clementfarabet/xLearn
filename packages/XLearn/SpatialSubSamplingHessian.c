#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_SpatialSubSamplingHessian_id = NULL;

static int nn_SpatialSubSamplingHessian_backwardHessian(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  THTensor *hessianOutput = luaT_checkudata(L, 3, torch_Tensor_id);  
  int kW = luaT_getfieldcheckint(L, 1, "kW");
  int kH = luaT_getfieldcheckint(L, 1, "kH");
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");
  int nInputPlane = luaT_getfieldcheckint(L, 1, "nInputPlane");

  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);

  THTensor *squaredWeight = luaT_getfieldcheckudata(L, 1, "squaredWeight", torch_Tensor_id);
  THTensor *squaredInput = luaT_getfieldcheckudata(L, 1, "squaredInput", torch_Tensor_id);

  THTensor *hessianWeight = luaT_getfieldcheckudata(L, 1, "hessianWeight", torch_Tensor_id);
  THTensor *hessianBias = luaT_getfieldcheckudata(L, 1, "hessianBias", torch_Tensor_id);
  THTensor *hessianInput = luaT_getfieldcheckudata(L, 1, "hessianInput", torch_Tensor_id);

  THTensor *hessianInputPlane, *unfoldedInputPlane, *unfoldedHessianInputPlane, *inputPlane;
  THTensor *hessianOutputPlane;
  THTensor *weightPlane, *hessianWeightPlane;
  int k;

  hessianInputPlane = THTensor_new();
  unfoldedInputPlane = THTensor_new();
  unfoldedHessianInputPlane = THTensor_new();
  inputPlane = THTensor_new();
  hessianOutputPlane = THTensor_new();
  weightPlane = THTensor_new();
  hessianWeightPlane = THTensor_new();
  
  /* Not necessary with partial backprop: */
  THTensor_resizeAs(hessianInput, input);
  THTensor_zero(hessianInput);

  /* Hold temporary squared input vector */
  THTensor_resizeAs(squaredInput, input);
  THTensor_copy(squaredInput, input);
  THTensor_cmul(squaredInput, squaredInput);

  /* Hold temporary squared weight vector */
  THTensor_resizeAs(squaredWeight, weight);
  THTensor_copy(squaredWeight, weight);
  THTensor_cmul(squaredWeight, squaredWeight);

  for(k = 0; k < nInputPlane; k++)
  {
    THTensor_select(hessianOutputPlane, hessianOutput, 2, k);
    THTensor_set1d(hessianBias, k, THTensor_get1d(hessianBias, k) + THTensor_sum(hessianOutputPlane));
      
    /* ------------------------- hessianWeight ------------------------------------- */
    
    /* Get the input image */
    THTensor_select(inputPlane, squaredInput, 2, k);
    THTensor_unfold(unfoldedInputPlane, inputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedInputPlane, NULL,       1, kH, dH);
    THTensor_transpose(unfoldedInputPlane,NULL,0,2);
    THTensor_transpose(unfoldedInputPlane,NULL,1,3);

    THTensor_setStorage2d(hessianWeightPlane, hessianWeight->storage, hessianWeight->storageOffset+hessianWeight->stride[0]*k, kW, 0, kH, 0);
    THTensor_addT4dotT2(hessianWeightPlane, 1, unfoldedInputPlane, hessianOutputPlane);

    /* -------------------------- hessianInput ------------------------------------- */
    
    /* Not necessary with partial backprop: */
    
    /* Get the hessianInput image */
    THTensor_select(hessianInputPlane, hessianInput, 2, k);
    THTensor_unfold(unfoldedHessianInputPlane, hessianInputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedHessianInputPlane, NULL,           1, kH, dH);
    
    THTensor_setStorage2d(weightPlane, squaredWeight->storage, squaredWeight->storageOffset+squaredWeight->stride[0]*k, kW, 0, kH, 0);
    THTensor_addT2outT2(unfoldedHessianInputPlane, 1, hessianOutputPlane, weightPlane);
  }

  THTensor_free(hessianInputPlane);
  THTensor_free(unfoldedInputPlane);
  THTensor_free(unfoldedHessianInputPlane);
  THTensor_free(inputPlane);
  THTensor_free(hessianOutputPlane);
  THTensor_free(weightPlane);
  THTensor_free(hessianWeightPlane);

  return 1;
}

static const struct luaL_Reg nn_SpatialSubSamplingHessian__ [] = {
  {"backwardHessian", nn_SpatialSubSamplingHessian_backwardHessian},
  {NULL, NULL}
};

void nn_SpatialSubSamplingHessian_init(lua_State *L) {
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_SpatialSubSamplingHessian_id = luaT_newmetatable(L, "nn.SpatialSubSamplingHessian", 
                                                           NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_SpatialSubSamplingHessian__);
  lua_pop(L, 1);
}
