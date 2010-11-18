#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id;

/* Kernel class *****************************************************/
static int Kernel_eval(lua_State *L)
{
  double (*func)(THTensor *, THTensor*, void*) = luaT_getfieldchecklightudata(L, 1, "__eval");
  void *params = luaT_getfieldchecklightudata(L, 1, "__params");
  THTensor *x1 = luaT_checkudata(L, 2, torch_Tensor_id);
  THTensor *x2 = luaT_checkudata(L, 3, torch_Tensor_id);
  lua_pushnumber(L, func(x1, x2, params));
  return 1;
}

static const struct luaL_Reg Kernel__ [] = {
  {"eval", Kernel_eval},
  {NULL, NULL}
};

/* Linear Kernel ****************************************************/
double LinearKernel___eval(THTensor *x1, THTensor *x2, double *gamma_p)
{
  return (*gamma_p) * THTensor_dot(x1, x2);
}


static int LinearKernel___cinit(lua_State *L)
{
  double gamma = luaL_checknumber(L, 2);
  double *gamma_p = luaT_alloc(L, sizeof(double)); /* DEBUG: right now the memory is lost */
  *gamma_p = gamma;

  lua_pushlightuserdata(L, LinearKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, gamma_p);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg LinearKernel__ [] = {
  {"__cinit", LinearKernel___cinit},
  {NULL, NULL}
};

/* RBF Kernel ****************************************************/
double RBFKernel___eval(THTensor *x1, THTensor *x2, double *gamma_p)
{
  double sum = 0;
  TH_TENSOR_APPLY2(double, x1, double, x2, 
	double z = *x1_p - *x2_p;
  sum += z*z;)
  return exp(-(*gamma_p)*sum);
}


static int RBFKernel___cinit(lua_State *L)
{
  double gamma = luaL_checknumber(L, 2);
  double *gamma_p = luaT_alloc(L, sizeof(double)); /* DEBUG: right now the memory is lost */
  *gamma_p = gamma;

  lua_pushlightuserdata(L, RBFKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, gamma_p);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg RBFKernel__ [] = {
  {"__cinit", RBFKernel___cinit},
  {NULL, NULL}
};

/* Polynomial Kernel ****************************************************/
struct PolynomialKernelParams
{
    double degree;
    double gamma;
    double bias;
};

double PolynomialKernel___eval(THTensor *x1, THTensor *x2, struct PolynomialKernelParams *params)
{
  return pow(params->gamma* THTensor_dot(x1, x2) + params->bias, params->degree);
}


static int PolynomialKernel___cinit(lua_State *L)
{
  double degree = luaL_checknumber(L, 2);
  double gamma = luaL_checknumber(L, 3);
  double bias = luaL_checknumber(L, 4);

  struct PolynomialKernelParams *params = luaT_alloc(L, sizeof(struct PolynomialKernelParams)); /* DEBUG: right now the memory is lost */
  params->degree = degree;
  params->gamma = gamma;
  params->bias = bias;

  lua_pushlightuserdata(L, PolynomialKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, params);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg PolynomialKernel__ [] = {
  {"__cinit", PolynomialKernel___cinit},
  {NULL, NULL}
};

/* Tanh Kernel ****************************************************/
struct TanhKernelParams
{
    double gamma;
    double bias;
};

double TanhKernel___eval(THTensor *x1, THTensor *x2, struct TanhKernelParams *params)
{
  return tanh(params->gamma* THTensor_dot(x1, x2) + params->bias);
}


static int TanhKernel___cinit(lua_State *L)
{
  double gamma = luaL_checknumber(L, 2);
  double bias = luaL_checknumber(L, 3);

  struct TanhKernelParams *params = luaT_alloc(L, sizeof(struct TanhKernelParams)); /* DEBUG: right now the memory is lost */
  params->gamma = gamma;
  params->bias = bias;

  lua_pushlightuserdata(L, TanhKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, params);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg TanhKernel__ [] = {
  {"__cinit", TanhKernel___cinit},
  {NULL, NULL}
};

/* Initialize the stuff *********************************************/
void svm_Kernel_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");

  luaT_newmetatable(L, "svm.Kernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, Kernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.LinearKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, LinearKernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.RBFKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, RBFKernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.PolynomialKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, PolynomialKernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.TanhKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, TanhKernel__);
  lua_pop(L, 1);
}
