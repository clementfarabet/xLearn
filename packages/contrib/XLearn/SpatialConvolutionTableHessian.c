#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_SpatialConvolutionTableHessian_id = NULL;

static int nn_SpatialConvolutionTableHessian_backwardHessian(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  THTensor *hessianOutput = luaT_checkudata(L, 3, torch_Tensor_id);  

  int kW = luaT_getfieldcheckint(L, 1, "kW");
  int kH = luaT_getfieldcheckint(L, 1, "kH");
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");

  int nInputPlane = luaT_getfieldcheckint(L, 1, "nInputPlane");
  int nOutputPlane = luaT_getfieldcheckint(L, 1, "nOutputPlane");

  THTensor *connTable = luaT_getfieldcheckudata(L, 1, "connTable", torch_Tensor_id);
  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);

  THTensor *squaredWeight = luaT_getfieldcheckudata(L, 1, "squaredWeight", torch_Tensor_id);
  THTensor *squaredInput = luaT_getfieldcheckudata(L, 1, "squaredInput", torch_Tensor_id);

  THTensor *hessianWeight = luaT_getfieldcheckudata(L, 1, "hessianWeight", torch_Tensor_id);
  THTensor *hessianBias = luaT_getfieldcheckudata(L, 1, "hessianBias", torch_Tensor_id);
  THTensor *hessianInput = luaT_getfieldcheckudata(L, 1, "hessianInput", torch_Tensor_id);

  THTensor *hessianInputPlane, *unfoldedSquaredInputPlane;
  THTensor *unfoldedHessianInputPlane, *squaredInputPlane;
  THTensor *hessianOutputPlane;
  THTensor *squaredWeightPlane, *hessianWeightPlane;
  int i, k;

  hessianInputPlane = THTensor_new();
  unfoldedSquaredInputPlane = THTensor_new();
  unfoldedHessianInputPlane = THTensor_new();
  squaredInputPlane = THTensor_new();
  hessianOutputPlane = THTensor_new();
  squaredWeightPlane = THTensor_new();
  hessianWeightPlane = THTensor_new();

  /* Not necessary with partial backprop: */
  THTensor_resizeAs(hessianInput, input);
  THTensor_zero(hessianInput);

  /* Hold temporary squared input vector */
  THTensor_resizeAs(squaredInput, input);
  THTensor_copy(input, squaredInput);
  THTensor_cmul(squaredInput, squaredInput);

  /* Hold temporary squared weight vector */
  THTensor_resizeAs(squaredWeight, weight);
  THTensor_copy(weight, squaredWeight);
  THTensor_cmul(squaredWeight, squaredWeight);

  /* hessian wrt bias */
  for(k = 0; k < nOutputPlane; k++)
  {
    THTensor_select(hessianOutputPlane, hessianOutput, 2, k);
    THTensor_set1d(hessianBias, k, THTensor_get1d(hessianBias, k) + THTensor_sum(hessianOutputPlane));
  }
  int nkernel = connTable->size[0];    
  for(k = 0; k < nkernel; k++)
  {
    int outplaneid = (int)THTensor_get2d(connTable,k,1)-1;
    int inplaneid = (int)THTensor_get2d(connTable,k,0)-1;
    /* ------------------------- hessianWeight ------------------------------------- */
    
    /* Get input, output and kernel*/
    THTensor_select(squaredInputPlane, squaredInput, 2, inplaneid);
    THTensor_select(hessianOutputPlane, hessianOutput, 2, outplaneid);
    THTensor_select(hessianWeightPlane, hessianWeight, 2, k);

    /* prepare input by unfolding */
    THTensor_unfold(unfoldedSquaredInputPlane, squaredInputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedSquaredInputPlane, NULL, 1, kH, dH);
    THTensor_transpose(unfoldedSquaredInputPlane, NULL, 0, 2);
    THTensor_transpose(unfoldedSquaredInputPlane, NULL, 1, 3);
        
    THTensor_addT4dotT2(hessianWeightPlane, 1, unfoldedSquaredInputPlane, hessianOutputPlane);
    
    /* -------------------------- hessianInput ------------------------------------- */
    
    /* Not necessary with partial backprop: */
    
    /* Get the hessianInput image */
    THTensor_select(hessianInputPlane, hessianInput, 2, inplaneid);
    THTensor_unfold(unfoldedHessianInputPlane, hessianInputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedHessianInputPlane, NULL          , 1, kH, dH);
    THTensor_select(squaredWeightPlane, squaredWeight, 2, k);
    
    THTensor_addT2outT2(unfoldedHessianInputPlane, 1, hessianOutputPlane, squaredWeightPlane);
  }


  THTensor_free(hessianInputPlane);
  THTensor_free(unfoldedSquaredInputPlane);
  THTensor_free(unfoldedHessianInputPlane);
  THTensor_free(squaredInputPlane);
  THTensor_free(hessianOutputPlane);
  THTensor_free(squaredWeightPlane);
  THTensor_free(hessianWeightPlane);

  return 1;
}

static const struct luaL_Reg nn_SpatialConvolutionTableHessian__ [] = {
  {"backwardHessian", nn_SpatialConvolutionTableHessian_backwardHessian},
  {NULL, NULL}
};

void nn_SpatialConvolutionTableHessian_init(lua_State *L) {
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_SpatialConvolutionTableHessian_id = luaT_newmetatable(L, "nn.SpatialConvolutionTableHessian", 
                                                           NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_SpatialConvolutionTableHessian__);
  lua_pop(L, 1);
}
