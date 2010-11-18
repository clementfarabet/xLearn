#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;
static const void* nn_SpatialMaxPooling_id = NULL;

static int nn_SpatialMaxPooling_forward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
  int kW = luaT_getfieldcheckint(L, 1, "kW");
  int kH = luaT_getfieldcheckint(L, 1, "kH");
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");
  THTensor *indices = luaT_getfieldcheckudata(L, 1, "indices", torch_Tensor_id);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

  luaL_argcheck(L, input->nDimension == 3, 2, "3D tensor expected");
  luaL_argcheck(L, input->size[0] >= kW && input->size[1] >= kH, 2, "input image smaller than kernel size");

  THTensor *outputPlane, *inputPlane, *unfoldedInputPlane, *localInput;
  int k,i,j;

  THTensor_resize3d(output,
                    (input->size[0] - kW) / dW + 1, 
                    (input->size[1] - kH) / dH + 1,
                    input->size[2]);

  inputPlane = THTensor_new();
  outputPlane = THTensor_new();
  localInput = THTensor_new();
  unfoldedInputPlane = THTensor_new();


  /* indices will contain i,j locatyions for each output point */
  THTensor_resize4d(indices, output->size[0],output->size[1],output->size[2],2);

  for (k = 0; k < input->size[2]; k++)
  {
    /* get input and output plane */
    THTensor_select(outputPlane, output, 2, k);
    THTensor_select(inputPlane, input, 2, k);

    /* Unfold input to get each local window */
    THTensor_unfold(unfoldedInputPlane, inputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedInputPlane, NULL,       1, kH, dH);

    /* Calculate max points */
    for(i = 0; i < outputPlane->size[0]; i++)
    {
      for(j = 0; j < outputPlane->size[1]; j++)
      {
	THTensor_select(localInput, unfoldedInputPlane,0,i);
	THTensor_select(localInput, NULL,              0,j);
	long maxindex = -1;
	double maxval = -THInf;
	long tcntr = 0;
	TH_TENSOR_APPLY(double, localInput,
			if (*localInput_p > maxval)
			  {
			    maxval = *localInput_p;
			    maxindex = tcntr;
			  };
			tcntr++;
			);
	THTensor_set4d(indices,i,j,k,0, (maxindex % localInput->size[0]) +1);
	THTensor_set4d(indices,i,j,k,1, (int)(maxindex / localInput->size[0])+1);
	THTensor_set2d(outputPlane,i,j,maxval);
      }
    }
  }
  THTensor_free(inputPlane);
  THTensor_free(outputPlane);
  THTensor_free(unfoldedInputPlane);
  THTensor_free(localInput);

  return 1;
}

static int nn_SpatialMaxPooling_backward(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);  
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);  
  int kW = luaT_getfieldcheckint(L, 1, "kW");
  int kH = luaT_getfieldcheckint(L, 1, "kH");
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");

  THTensor *indices = luaT_getfieldcheckudata(L, 1, "indices", torch_Tensor_id);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);

  THTensor *gradOutputPlane, *gradInputPlane, *unfoldedGradInputPlane, *gradLocalInput;
  int k,i,j;

  THTensor_resizeAs(gradInput, input);
  THTensor_zero(gradInput);

  gradInputPlane = THTensor_new();
  gradOutputPlane = THTensor_new();
  gradLocalInput = THTensor_new();
  unfoldedGradInputPlane = THTensor_new();

  for (k = 0; k < input->size[2]; k++)
  {
    /* get input and output plane */
    THTensor_select(gradOutputPlane, gradOutput, 2, k);
    THTensor_select(gradInputPlane, gradInput, 2, k);

    /* Unfold input to get each local window */
    THTensor_unfold(unfoldedGradInputPlane, gradInputPlane, 0, kW, dW);
    THTensor_unfold(unfoldedGradInputPlane, NULL,           1, kH, dH);

    /* Calculate max points */
    for(i = 0; i < gradOutputPlane->size[0]; i++)
    {
      for(j = 0; j < gradOutputPlane->size[1]; j++)
      {
	THTensor_select(gradLocalInput, unfoldedGradInputPlane,0,i);
	THTensor_select(gradLocalInput, NULL,                  0,j);
	long maxi = THTensor_get4d(indices,i,j,k,0)-1;
	long maxj = THTensor_get4d(indices,i,j,k,1)-1;
	double gi = THTensor_get2d(gradLocalInput,maxi,maxj)+
	  THTensor_get2d(gradOutputPlane,i,j);
	THTensor_set2d(gradLocalInput,maxi,maxj,gi);
      }
    }
  }

  THTensor_free(gradInputPlane);
  THTensor_free(gradOutputPlane);
  THTensor_free(unfoldedGradInputPlane);
  THTensor_free(gradLocalInput);

  return 1;
}

static const struct luaL_Reg nn_SpatialMaxPooling__ [] = {
  {"forward", nn_SpatialMaxPooling_forward},
  {"backward", nn_SpatialMaxPooling_backward},
  {NULL, NULL}
};

void nn_SpatialMaxPooling_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  nn_SpatialMaxPooling_id = luaT_newmetatable(L, "nn.SpatialMaxPooling", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, nn_SpatialMaxPooling__);
  lua_pop(L, 1);
}
