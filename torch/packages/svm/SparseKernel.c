#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id;


static double THTensor_sparsedot(THTensor *x1, THTensor *x2)
{
  long i1, i2, sz1, sz2, st1, st2;
  double *ptr1, *ptr2;
  double sum;

  if( (x1->nDimension != 1) || (x2->nDimension != 1) )
    THError("sparsedot: requires sparse tensor");

  sz1 = x1->size[0]/2;
  sz2 = x2->size[0]/2;
  st1 = x1->stride[0];
  st2 = x2->stride[0];
  ptr1 = THTensor_dataPtr(x1);
  ptr2 = THTensor_dataPtr(x2);

  i1 = 0;
  i2 = 0;
  sum = 0;
  while( (i1 < sz1) && (i2 < sz2) )
  {
    if(ptr1[i1*st1*2] == ptr2[i2*st2*2])
    {
      sum += ptr1[i1*st1*2+1] * ptr2[i2*st2*2+1];
      i1++;
      i2++;
    }
    else
    {
      if(ptr1[i1*st1*2] > ptr2[i2*st2*2])
        i2++;
      else
        i1++;
    }
  }
  return sum;
}

/*
static double THTensor_sparsedot(THTensor *x1, THTensor *x2)
{
  long i1, i2, sz1, sz2, st1, st2;
  double *ptridx1, *ptridx2, *ptrval1, *ptrval2;
  double sum;

  if( (x1->nDimension != 2) || (x2->nDimension != 2) || (x1->size[1] != 2) || (x2->size[1] != 2) )
    THError("sparsedot: requires sparse tensor");

  sz1 = x1->size[0];
  sz2 = x2->size[0];
  st1 = x1->stride[0];
  st2 = x2->stride[0];
  ptridx1 = THTensor_dataPtr2d(x1, 0, 0);
  ptridx2 = THTensor_dataPtr2d(x2, 0, 0);
  ptrval1 = THTensor_dataPtr2d(x1, 0, 1);
  ptrval2 = THTensor_dataPtr2d(x2, 0, 1);

  i1 = 0;
  i2 = 0;
  sum = 0;
  while( (i1 < sz1) && (i2 < sz2) )
  {
    if(ptridx1[i1*st1] == ptridx2[i2*st2])
    {
      sum += ptrval1[i1*st1] * ptrval2[i2*st2];
      i1++;
      i2++;
    }
    else
    {
      if(ptridx1[i1*st1] > ptridx2[i2*st2])
        i2++;
      else
        i1++;
    }
  }
  return sum;
}
*/


/* Linear SparseKernel ****************************************************/
double LinearSparseKernel___eval(THTensor *x1, THTensor *x2, double *gamma_p)
{
  return (*gamma_p) * THTensor_sparsedot(x1, x2);
}


static int LinearSparseKernel___cinit(lua_State *L)
{
  double gamma = luaL_checknumber(L, 2);
  double *gamma_p = luaT_alloc(L, sizeof(double)); /* DEBUG: right now the memory is lost */
  *gamma_p = gamma;

  lua_pushlightuserdata(L, LinearSparseKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, gamma_p);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg LinearSparseKernel__ [] = {
  {"__cinit", LinearSparseKernel___cinit},
  {NULL, NULL}
};

/* RBF SparseKernel ****************************************************/
double RBFSparseKernel___eval(THTensor *x1, THTensor *x2, double *gamma_p)
{
  double sum = THTensor_sparsedot(x1, x1) - 2.*THTensor_sparsedot(x1, x2) + THTensor_sparsedot(x2, x2);
  return exp(-(*gamma_p)*sum);
}


static int RBFSparseKernel___cinit(lua_State *L)
{
  double gamma = luaL_checknumber(L, 2);
  double *gamma_p = luaT_alloc(L, sizeof(double)); /* DEBUG: right now the memory is lost */
  *gamma_p = gamma;

  lua_pushlightuserdata(L, RBFSparseKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, gamma_p);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg RBFSparseKernel__ [] = {
  {"__cinit", RBFSparseKernel___cinit},
  {NULL, NULL}
};

/* Polynomial SparseKernel ****************************************************/
struct PolynomialSparseKernelParams
{
    double degree;
    double gamma;
    double bias;
};

double PolynomialSparseKernel___eval(THTensor *x1, THTensor *x2, struct PolynomialSparseKernelParams *params)
{
  return pow(params->gamma* THTensor_sparsedot(x1, x2) + params->bias, params->degree);
}


static int PolynomialSparseKernel___cinit(lua_State *L)
{
  double degree = luaL_checknumber(L, 2);
  double gamma = luaL_checknumber(L, 3);
  double bias = luaL_checknumber(L, 4);

  struct PolynomialSparseKernelParams *params = luaT_alloc(L, sizeof(struct PolynomialSparseKernelParams)); /* DEBUG: right now the memory is lost */
  params->degree = degree;
  params->gamma = gamma;
  params->bias = bias;

  lua_pushlightuserdata(L, PolynomialSparseKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, params);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg PolynomialSparseKernel__ [] = {
  {"__cinit", PolynomialSparseKernel___cinit},
  {NULL, NULL}
};

/* Tanh SparseKernel ****************************************************/
struct TanhSparseKernelParams
{
    double gamma;
    double bias;
};

double TanhSparseKernel___eval(THTensor *x1, THTensor *x2, struct TanhSparseKernelParams *params)
{
  return tanh(params->gamma* THTensor_sparsedot(x1, x2) + params->bias);
}


static int TanhSparseKernel___cinit(lua_State *L)
{
  double gamma = luaL_checknumber(L, 2);
  double bias = luaL_checknumber(L, 3);

  struct TanhSparseKernelParams *params = luaT_alloc(L, sizeof(struct TanhSparseKernelParams)); /* DEBUG: right now the memory is lost */
  params->gamma = gamma;
  params->bias = bias;

  lua_pushlightuserdata(L, TanhSparseKernel___eval);
  lua_setfield(L, 1, "__eval");

  lua_pushlightuserdata(L, params);
  lua_setfield(L, 1, "__params");
  return 0;
}

static const struct luaL_Reg TanhSparseKernel__ [] = {
  {"__cinit", TanhSparseKernel___cinit},
  {NULL, NULL}
};

/* Initialize the stuff *********************************************/
void svm_SparseKernel_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");

  luaT_newmetatable(L, "svm.LinearSparseKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, LinearSparseKernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.RBFSparseKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, RBFSparseKernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.PolynomialSparseKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, PolynomialSparseKernel__);
  lua_pop(L, 1);

  luaT_newmetatable(L, "svm.TanhSparseKernel", NULL, NULL, NULL, NULL);
  luaL_register(L, NULL, TanhSparseKernel__);
  lua_pop(L, 1);
}
