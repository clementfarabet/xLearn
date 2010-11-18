#include "luaT.h"
#include "TH.h"
#include "svqp2.h"

static const void* QPSolver_id = NULL;
static const void* torch_Tensor_id = NULL;
static const void* torch_IntStorage_id = NULL;

static int QPSolver_new(lua_State *L)
{
  int n = (int)luaL_checknumber(L, 1);
  SVQP2 *qp = new SVQP2(n);
  luaT_pushudata(L, qp, QPSolver_id);
  return 1;
}

static int QPSolver_free(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  delete qp;
  return 0;
}

static int QPSolver_n(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  lua_pushnumber(L, qp->n);
  return 1;
}

/* humm... confusing with permutation */
static int QPSolver_Aperm(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  if(lua_isnoneornil(L, 2))
  {
    THIntStorage *Aperm = THIntStorage_newWithSize(qp->n);
    for(int i = 0; i < qp->n; i++)
      Aperm->data[i] = qp->Aperm[i];
    luaT_pushudata(L, Aperm, torch_IntStorage_id); \
    return 1;
  }
  else
  {
    THIntStorage *Aperm = (THIntStorage*)luaT_checkudata(L, 2, torch_IntStorage_id);
    luaL_argcheck(L, Aperm->size == qp->n, 2, "invalid size");
    for(int i = 0; i < qp->n; i++)
      qp->Aperm[i] = Aperm->data[i];
    return 0;
  }
}


#define QPSOLVER_INPUT_OUTPUT_TENSOR(NAME) \
static int QPSolver_##NAME(lua_State *L) \
{ \
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id); \
  if(lua_isnoneornil(L, 2)) \
  { \
    THTensor *var = THTensor_newWithSize1d(qp->n); \
    int i = 0; \
    TH_TENSOR_APPLY(double, var, *var_p = qp->NAME[i++];); \
    luaT_pushudata(L, var, torch_Tensor_id); \
    return 1; \
  } \
  else \
  { \
    THTensor *var = (THTensor*)luaT_checkudata(L, 2, torch_Tensor_id); \
    luaL_argcheck(L, var->nDimension == 1, 2, "vector expected"); \
    luaL_argcheck(L, var->size[0] == qp->n, 2, "invalid size"); \
    int i = 0; \
    TH_TENSOR_APPLY(double, var, qp->NAME[i++] = *var_p;); \
    return 0; \
  } \
}

QPSOLVER_INPUT_OUTPUT_TENSOR(b)
QPSOLVER_INPUT_OUTPUT_TENSOR(cmin)
QPSOLVER_INPUT_OUTPUT_TENSOR(cmax)
QPSOLVER_INPUT_OUTPUT_TENSOR(x)

static int QPSolver_g(lua_State *L)
{ 
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  THTensor *var = THTensor_newWithSize1d(qp->n);
  const double *g = qp->gradient();
  int i = 0;
  TH_TENSOR_APPLY(double, var, *var_p = g[i++];);
  luaT_pushudata(L, var, torch_Tensor_id);
  return 1;
}

static int QPSolver_sumflag(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  qp->sumflag = luaT_checkboolean(L, 2);
  return 0;
}

static int QPSolver_verbosity(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  int verbosity = (int)luaL_checknumber(L, 2);
  luaL_argcheck(L, verbosity >= 0 && verbosity <= 3, 2, "integer between 0 and 3 expected");
  qp->verbosity = verbosity;
  return 0;
}

#define QPSOLVER_INPUT_DOUBLE(NAME) \
static int QPSolver_##NAME(lua_State *L) \
{ \
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id); \
  qp->NAME = luaL_checknumber(L, 2); \
  return 0; \
}

QPSOLVER_INPUT_DOUBLE(epskt)
QPSOLVER_INPUT_DOUBLE(epsgr)
QPSOLVER_INPUT_DOUBLE(maxst)

#define QPSOLVER_INPUT_LONG(NAME) \
static int QPSolver_##NAME(lua_State *L) \
{ \
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id); \
  qp->NAME = (long)luaL_checknumber(L, 2); \
  return 0; \
}

QPSOLVER_INPUT_LONG(maxcachesize)

static double QPSolver_luaClosure(int i, int j, void *L_)
{
  lua_State *L = (lua_State*)L_;
  double res;
  lua_pushvalue(L, 2);
  lua_pushnumber(L, i+1);
  lua_pushnumber(L, j+1);
  lua_call(L, 2, 1);
  if(!lua_isnumber(L, -1))
    luaL_error(L, "the given closure did not return a number");
  res = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return res;
}

struct QPSolver_cClosureParams
{
    double (*func)(THTensor *, THTensor*, void *);
    void *params;
    lua_State *L;
    THTensor *data;
    THTensor *x1;
    THTensor *x2;
};

static double QPSolver_cClosure(int i, int j, void *stuff_)
{
  QPSolver_cClosureParams *stuff = (QPSolver_cClosureParams*)(stuff_);
  THTensor_select(stuff->x1, stuff->data, 1, i);
  THTensor_select(stuff->x2, stuff->data, 1, j);
  return stuff->func(stuff->x1, stuff->x2, stuff->params);
}

static double QPSolver_cLuaClosure(int i, int j, void *stuff_)
{
  QPSolver_cClosureParams *stuff = (QPSolver_cClosureParams*)(stuff_);
  lua_State *L = stuff->L;
  THTensor_select(stuff->x1, stuff->data, 1, i);
  THTensor_select(stuff->x2, stuff->data, 1, j);
  lua_pushvalue(L, -4); /* kernel eval function */
  lua_pushvalue(L, -4); /* kernel self */
  lua_pushvalue(L, -4); /* x1 */
  lua_pushvalue(L, -4); /* x2 */
  lua_call(L, 3, 1);
  if(!lua_isnumber(L, -1))
    luaL_error(L, "the given closure did not return a number");
  double res = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return res;
}

static int QPSolver_run(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  luaL_argcheck(L, lua_isfunction(L, 2) || luaT_isudata(L, 2, torch_Tensor_id), 2, "function or Tensor expected");

  if(lua_isfunction(L, 2))
  {
    bool initialize = luaT_optboolean(L, 3, true);
    bool finalize = luaT_optboolean(L, 4, true);
    qp->Aclosure = L;
    qp->Afunction = QPSolver_luaClosure;
    int result = qp->run(initialize, finalize);
    if(result == -1)
      luaL_error(L, qp->errmsg);
    return 0;
  }
  else
  {
    bool initialize = luaT_optboolean(L, 4, true);
    bool finalize = luaT_optboolean(L, 5, true);
    
    THTensor *data = (THTensor*)luaT_checkudata(L, 2, torch_Tensor_id);
    luaL_argcheck(L, data->nDimension == 2, 2, "2D Tensor expected");
    luaL_argcheck(L, data->size[1] == qp->n, 2, "invalid size");
    
    struct QPSolver_cClosureParams stuff;
    
    lua_getfield(L, 3, "__eval");
    bool isCKernel = lua_islightuserdata(L, -1);
    lua_pop(L, 1);
    if(isCKernel)
    {
      double (*func)(THTensor*, THTensor*, void *) = ( double (*)(THTensor*, THTensor*, void *))luaT_getfieldchecklightudata(L, 3, "__eval");
      void *params = luaT_getfieldchecklightudata(L, 3, "__params");
      stuff.func = func;
      stuff.params = params;
    }
    else
    {
      lua_getfield(L, 3, "eval");
      lua_pushvalue(L, 3);
      stuff.func = NULL;
      stuff.params = NULL;
    }
    
    stuff.L = L;
    stuff.data = data;
    stuff.x1 = THTensor_new();
    stuff.x2 = THTensor_new();
    luaT_pushudata(L, stuff.x1, torch_Tensor_id); /* auto dealloc */
    luaT_pushudata(L, stuff.x2, torch_Tensor_id); /* auto dealloc */
    
    qp->Aclosure = &stuff;
    qp->Afunction = (stuff.func ? QPSolver_cClosure : QPSolver_cLuaClosure);
    int result = qp->run(initialize, finalize);
    if(result == -1)
      luaL_error(L, qp->errmsg);
    return 0;
  }
}

#define QPSOLVER_OUTPUT_DOUBLE(NAME) \
static int QPSolver_##NAME(lua_State *L) \
{ \
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id); \
  lua_pushnumber(L, qp->NAME); \
  return 1; \
}

QPSOLVER_OUTPUT_DOUBLE(gmin)
QPSOLVER_OUTPUT_DOUBLE(gmax)
QPSOLVER_OUTPUT_DOUBLE(w)

static int QPSolver_permutation(lua_State *L)
{
  SVQP2 *qp = (SVQP2*)luaT_checkudata(L, 1, QPSolver_id);
  THIntStorage *table = THIntStorage_newWithSize(qp->n);
  qp->permutation(table->data);
  for (int i=0; i<qp->n; i++)
    table->data[i] += 1; // first index must be 1
  luaT_pushudata(L, table, torch_IntStorage_id);
  return 1;
}

static const struct luaL_Reg QPSolver__ [] = {
  {"n", QPSolver_n},
  {"Aperm", QPSolver_Aperm},
  {"b", QPSolver_b},
  {"cmin", QPSolver_cmin},
  {"cmax", QPSolver_cmax},
  {"x", QPSolver_x},
  {"g", QPSolver_g},
  {"sumflag", QPSolver_sumflag},
  {"verbosity", QPSolver_verbosity},
  {"epskt", QPSolver_epskt},
  {"epsgr", QPSolver_epsgr},
  {"maxst", QPSolver_maxst},
  {"maxcachesize", QPSolver_maxcachesize},
  {"run", QPSolver_run},
  {"gmin", QPSolver_gmin},
  {"gmax", QPSolver_gmax},
  {"w", QPSolver_w},
  {"permutation", QPSolver_permutation},
  {NULL, NULL}
};

extern "C" void svm_QPSolver_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  torch_IntStorage_id = luaT_checktypename2id(L, "torch.IntStorage");

  QPSolver_id = luaT_newmetatable(L, "svm.QPSolver", NULL,
                                  QPSolver_new, QPSolver_free, NULL);
  
  luaL_register(L, NULL, QPSolver__);
  lua_pop(L, 1);
}
