#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id;

static int SVM_forward(lua_State *L)
{
  double (*func)(THTensor *, THTensor*, const void*) = NULL;
  const void *params = NULL;
  int i;
  double sum;
  THTensor *x;
  THTensor *alpha;

  lua_settop(L, 2);
  x = luaT_checkudata(L, 2, torch_Tensor_id);
  luaT_getfieldchecktable(L, 1, "svs"); /* we are sure it is at 3 */
  sum = luaT_getfieldchecknumber(L, 1, "bias");
  alpha = luaT_getfieldcheckudata(L, 1, "alpha", torch_Tensor_id);
  
  lua_getfield(L, 1, "kernel");
  if(!lua_istable(L, -1))
    luaL_error(L, "no kernel available in this SVM");
  lua_getfield(L, -1, "__eval");
  if(lua_islightuserdata(L, -1))
  {
    func = lua_topointer(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "__params");
    if(lua_islightuserdata(L, -1))
    {
      params = lua_topointer(L, -1);
      lua_pop(L, 1);
    }
    else
      lua_pop(L, 1); /* not a C kernel */
  }
  else
    lua_pop(L, 1);   /* not a C kernel */

  if(func && params)
  {
    for(i = 0; i < alpha->size[0]; i++)
    {
      THTensor *sv;
      lua_pushnumber(L, i+1);
      lua_rawget(L, 3);
      sv = luaT_checkudata(L, -1, torch_Tensor_id);
      sum += THTensor_get1d(alpha, i) * func(x, sv, params);
      lua_pop(L, 1);
    }
  }
  else
  {
    lua_getfield(L, -1, "eval");
    if(!lua_isfunction(L, -1))
      luaL_error(L, "kernel does not have a valid eval method");

    for(i = 0; i < alpha->size[0]; i++)
    {
      lua_pushvalue(L, -1); /* the method eval */
      lua_pushvalue(L, -3); /* the kernel */
      lua_pushvalue(L, 2);  /* the input tensor x */

      lua_pushnumber(L, i+1);
      lua_rawget(L, 3);     /* the i-th SV */
      
      lua_call(L, 3, 1);

      if(!lua_isnumber(L, -1))
        luaL_error(L, "kernel did not return a real value");

      sum += THTensor_get1d(alpha, i) * lua_tonumber(L, -1);
      lua_pop(L, 1);
    }    
  }

  lua_pushnumber(L, sum);
  return 1;
}

static const struct luaL_Reg SVM__ [] = {
  {"forward", SVM_forward},
  {NULL, NULL}
};

void svm_SVM_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");

  luaT_newmetatable(L, "svm.SVM", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, SVM__);
  lua_pop(L, 1);
}
