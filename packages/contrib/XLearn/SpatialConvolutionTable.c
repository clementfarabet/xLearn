#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_SpatialConvolutionTable_id = NULL;

static int nn_SpatialConvolutionTable_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  int kW = luaT_getfieldcheckint(L, 1, "kW");
  int kH = luaT_getfieldcheckint(L, 1, "kH");
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");
  int nInputPlane = luaT_getfieldcheckint(L, 1, "nInputPlane");
  int nOutputPlane = luaT_getfieldcheckint(L, 1, "nOutputPlane");

  THTensor *connTable = luaT_getfieldcheckudata(L, 1, "connTable", torch_Tensor_id);
  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);
  THTensor *bias = luaT_getfieldcheckudata(L, 1, "bias", torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);
  
  THTensor *outputPlane, *inputPlane, *weightPlane, *unfoldedInputPlane;
  int k;

  luaL_argcheck(L, input->nDimension == 3, 2, "3D tensor expected");
  luaL_argcheck(L, input->size[2] == nInputPlane, 2, "invalid number of input planes");
  luaL_argcheck(L, input->size[0] >= kW && input->size[1] >= kH, 2, "input image smaller than kernel size");


  THTensor_resize3d(output,
                    (input->size[0] - kW) / dW + 1, 
                    (input->size[1] - kH) / dH + 1,
                    nOutputPlane);

  inputPlane = THTensor_new();
  weightPlane = THTensor_new();
  outputPlane = THTensor_new();
  unfoldedInputPlane = THTensor_new();

  /* Initialize each plane to the bias */
  for (k = 0; k < nOutputPlane; k++)
  {
    THTensor_select(outputPlane,output,2,k);
    THTensor_fill(outputPlane, THTensor_get1d(bias, k));
  }

  int nkernel = connTable->size[0];
  for (k = 0; k < nkernel; k++)
  {
    int outplaneid = (int)THTensor_get2d(connTable,k,1)-1;
    int inplaneid = (int)THTensor_get2d(connTable,k,0)-1;

    /* Get input, output and kernel*/
    THTensor_select(outputPlane, output, 2, outplaneid);
    THTensor_select(inputPlane, input, 2, inplaneid);
    THTensor_select(weightPlane, weight, 2, k);
    
    /* Prepare input by unfolding */
    THTensor_unfold(unfoldedInputPlane, inputPlane,  0, kW, dW);
    THTensor_unfold(unfoldedInputPlane, NULL,        1, kH, dH);

    /* Convolve */
    THTensor_addT4dotT2(outputPlane, 1, unfoldedInputPlane, weightPlane);
  }

  THTensor_free(inputPlane);
  THTensor_free(weightPlane);
  THTensor_free(outputPlane);
  THTensor_free(unfoldedInputPlane);

  return 1;
}

static int nn_SpatialConvolutionTable_backward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);  
  int kW = luaT_getfieldcheckint(L, 1, "kW");
  int kH = luaT_getfieldcheckint(L, 1, "kH");
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");
  int nInputPlane = luaT_getfieldcheckint(L, 1, "nInputPlane");
  int nOutputPlane = luaT_getfieldcheckint(L, 1, "nOutputPlane");

  THTensor *connTable = luaT_getfieldcheckudata(L, 1, "connTable", torch_Tensor_id);
  THTensor *weight = luaT_getfieldcheckudata(L, 1, "weight", torch_Tensor_id);
  THTensor *gradWeight = luaT_getfieldcheckudata(L, 1, "gradWeight", torch_Tensor_id);
  THTensor *gradBias = luaT_getfieldcheckudata(L, 1, "gradBias", torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);

  THTensor *gradInputPlane, *unfoldedInputPlane, *unfoldedGradInputPlane, *inputPlane;
  THTensor *gradOutputPlane;
  THTensor *weightPlane, *gradWeightPlane;
  int i, k;

  gradInputPlane = THTensor_new();
  unfoldedInputPlane = THTensor_new();
  unfoldedGradInputPlane = THTensor_new();
  inputPlane = THTensor_new();
  gradOutputPlane = THTensor_new();
  weightPlane = THTensor_new();
  gradWeightPlane = THTensor_new();
  
  /* Not necessary with partial backprop: */
  THTensor_resizeAs(gradInput, input);
  THTensor_zero(gradInput);

  /* gradients wrt bias */
  for(k = 0; k < nOutputPlane; k++)
  {
    THTensor_select(gradOutputPlane, gradOutput, 2, k);
    THTensor_set1d(gradBias, k, THTensor_get1d(gradBias, k) + THTensor_sum(gradOutputPlane));
  }
  int nkernel = connTable->size[0];    
  for(k = 0; k < nkernel; k++)
  {
    int outplaneid = (int)THTensor_get2d(connTable,k,1)-1;
    int inplaneid = (int)THTensor_get2d(connTable,k,0)-1;
    /* ------------------------- gradWeight ------------------------------------- */
    
    /* Get input, output and kernel*/
    THTensor_select(inputPlane, input, 2, inplaneid);
    THTensor_select(gradOutputPlane, gradOutput, 2, outplaneid);
    THTensor_select(gradWeightPlane, gradWeight, 2, k);

    /* prepare input by unfolding */
    THTensor_unfold(unfoldedInputPlane, inputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedInputPlane, NULL,       1, kH, dH);
    THTensor_transpose(unfoldedInputPlane,NULL,0,2);
    THTensor_transpose(unfoldedInputPlane,NULL,1,3);
        
    THTensor_addT4dotT2(gradWeightPlane, 1, unfoldedInputPlane, gradOutputPlane);
    
    /* -------------------------- gradInput ------------------------------------- */
    
    /* Not necessary with partial backprop: */
    
    /* Get the gradInput image */
    THTensor_select(gradInputPlane, gradInput, 2, inplaneid);
    THTensor_unfold(unfoldedGradInputPlane, gradInputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedGradInputPlane, NULL          , 1, kH, dH);
    THTensor_select(weightPlane, weight, 2, k);
    
    THTensor_addT2outT2(unfoldedGradInputPlane, 1, gradOutputPlane, weightPlane);
  }


  THTensor_free(gradInputPlane);
  THTensor_free(unfoldedInputPlane);
  THTensor_free(unfoldedGradInputPlane);
  THTensor_free(inputPlane);
  THTensor_free(gradOutputPlane);
  THTensor_free(weightPlane);
  THTensor_free(gradWeightPlane);

  return 1;
}

static const struct luaL_Reg nn_SpatialConvolutionTable__ [] = {
  {"forward", nn_SpatialConvolutionTable_forward},
  {"backward", nn_SpatialConvolutionTable_backward},
  {NULL, NULL}
};

void nn_SpatialConvolutionTable_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_SpatialConvolutionTable_id = luaT_newmetatable(L, "nn.SpatialConvolutionTable", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_SpatialConvolutionTable__);
  lua_pop(L, 1);
}
