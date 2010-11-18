#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;

#define IMPLEMENT_FUNC(NAME, CFUNC) \
static int lab_##NAME(lua_State *L) \
{ \
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id); \
  THTensor *r = THTensor_new(); \
  THTensor_resizeAs(r, t); \
  luaT_pushudata(L, r, torch_Tensor_id); \
  TH_TENSOR_APPLY2(double, t, double, r, *r_p = CFUNC(*t_p);) \
  return 1; \
}

#define IMPLEMENT_FUNC_VALUE(NAME, CFUNC) \
static int lab_##NAME(lua_State *L) \
{ \
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id); \
  double value = luaL_checknumber(L, 2); \
  THTensor *r = THTensor_new(); \
  THTensor_resizeAs(r, t); \
  luaT_pushudata(L, r, torch_Tensor_id); \
  TH_TENSOR_APPLY2(double, t, double, r, *r_p = CFUNC(*t_p, value);) \
  return 1; \
}

IMPLEMENT_FUNC(log,log)
IMPLEMENT_FUNC(log1p,log1p)
IMPLEMENT_FUNC(exp,exp)
IMPLEMENT_FUNC(cos,cos)
IMPLEMENT_FUNC(acos,acos)
IMPLEMENT_FUNC(cosh,cosh)
IMPLEMENT_FUNC(sin,sin)
IMPLEMENT_FUNC(asin,asin)
IMPLEMENT_FUNC(sinh,sinh)
IMPLEMENT_FUNC(tan,tan)
IMPLEMENT_FUNC(atan,atan)
IMPLEMENT_FUNC(tanh,tanh)
IMPLEMENT_FUNC_VALUE(pow,pow)
IMPLEMENT_FUNC(sqrt,sqrt)
IMPLEMENT_FUNC(ceil,ceil)
IMPLEMENT_FUNC(floor,floor)
IMPLEMENT_FUNC(abs,fabs)

static const struct luaL_Reg lab_math__ [] = {
  {"log", lab_log},
  {"log1p", lab_log1p},
  {"exp", lab_exp},
  {"cos", lab_cos},
  {"acos", lab_acos},
  {"cosh", lab_cosh},
  {"sin", lab_sin},
  {"asin", lab_asin},
  {"sinh", lab_sinh},
  {"tan", lab_tan},
  {"atan", lab_atan},
  {"tanh", lab_tanh},
  {"pow", lab_pow},
  {"sqrt", lab_sqrt},
  {"ceil", lab_ceil},
  {"floor", lab_floor},
  {"abs", lab_abs},
  {NULL, NULL}
};

void lab_math_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  luaL_register(L, NULL, lab_math__);
}
