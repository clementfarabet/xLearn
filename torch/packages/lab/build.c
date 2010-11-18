#include "TH.h"
#include "utils.h"

static const void* torch_Tensor_id = NULL;

static int lab_zeros(lua_State *L)
{
  THLongStorage *dimension = lab_checklongargs(L, 1);
  THTensor *t = THTensor_newWithSize(dimension->size, dimension->data, NULL);
  THTensor_zero(t);
  luaT_pushudata(L, t, torch_Tensor_id);
  THLongStorage_free(dimension);
  return 1;
}

static int lab_ones(lua_State *L)
{
  THLongStorage *dimension = lab_checklongargs(L, 1);
  THTensor *t = THTensor_newWithSize(dimension->size, dimension->data, NULL);
  THTensor_fill(t, 1);
  luaT_pushudata(L, t, torch_Tensor_id);
  THLongStorage_free(dimension);
  return 1;
}

static int lab_diag(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int k = luaL_optint(L, 2, 0);
  luaL_argcheck(L, t->nDimension == 1 || t->nDimension == 2, 1, "not a matrix or a vector");

  if(t->nDimension == 1)
  {
    long sz = t->size[0] + (k >= 0 ? k : -k);
    THTensor *diag = THTensor_newWithSize2d(sz, sz);    
    THTensor *sub;

    luaT_pushudata(L, diag, torch_Tensor_id);
    THTensor_zero(diag);
    if(k >= 0)
      sub = THTensor_newWithStorage1d(diag->storage, k*diag->stride[1], t->size[0], diag->stride[1]+1);
    else
      sub = THTensor_newWithStorage1d(diag->storage, -k, t->size[0], diag->stride[1]+1);
    THTensor_copy(sub, t);
    THTensor_free(sub);
    return 1;
  }
  else
  {
    THTensor *diag, *sub;
    long sz = 0;

    if(k >= 0)
      sz = (t->size[0] < t->size[1]-k ? t->size[0] : t->size[1]-k);
    else
      sz = (t->size[0]+k < t->size[1] ? t->size[0]+k : t->size[1]);
    diag = THTensor_newWithSize1d(sz);
    luaT_pushudata(L, diag, torch_Tensor_id);
    if(k >= 0)
      sub = THTensor_newWithStorage1d(t->storage, k*t->stride[1], sz, t->stride[1]+1);
    else
      sub = THTensor_newWithStorage1d(t->storage, -k, sz, t->stride[1]+1);
    THTensor_copy(diag, sub);
    THTensor_free(sub);
    return 1;
  }

  return 0;
}

static int lab_eye(lua_State *L)
{
  long n = luaL_checklong(L, 1);
  long m = luaL_optlong(L, 2, 0);
  THTensor *eye;
  double *eye_p;
  long i;

  luaL_argcheck(L, n > 0, 1, "invalid argument");

  if( m <= 0 )
    m = n;
  
  eye = THTensor_newWithSize2d(n, m);
  THTensor_zero(eye);
  luaT_pushudata(L, eye, torch_Tensor_id);

  i = 0;
  eye_p = THTensor_dataPtr(eye);
  while( (i < eye->size[0]) && (i < eye->size[1]) )
  {
    *(eye_p+i*eye->stride[0]+i*eye->stride[1]) = 1;
    i++;
  }
  
  return 1;
}

static int lab_linspace(lua_State *L)
{
  double a = luaL_checknumber(L, 1);
  double b = luaL_checknumber(L, 2);
  long n = luaL_optlong(L, 3, 100);
  THTensor *t;
  long i;

  luaL_argcheck(L, n > 0, 3, "invalid number of points");
  luaL_argcheck(L, a <= b, 2, "end range should be greater than start range");
  
  t = THTensor_newWithSize1d(n);
  luaT_pushudata(L, t, torch_Tensor_id);
  for(i = 0; i < n; i++)
    t->storage->data[i] = a + ((double)i)*(b-a)/((double)(n-1));
  return 1;
}

static int lab_logspace(lua_State *L)
{
  double a = luaL_checknumber(L, 1);
  double b = luaL_checknumber(L, 2);
  long n = luaL_optlong(L, 3, 100);
  THTensor *t;
  long i;

  luaL_argcheck(L, n > 0, 3, "invalid number of points");
  luaL_argcheck(L, a <= b, 2, "end range should be greater than start range");
  
  t = THTensor_newWithSize1d(n);
  luaT_pushudata(L, t, torch_Tensor_id);
  for(i = 0; i < n; i++)
    t->storage->data[i] = pow(10.0, a + ((double)i)*(b-a)/((double)(n-1)));
  return 1;
}

static int lab_rand(lua_State *L)
{
  THLongStorage *dimension = lab_checklongargs(L, 1);
  THTensor *t = THTensor_newWithSize(dimension->size, dimension->data, NULL);
  long i;
  for(i = 0; i < t->storage->size; i++)
    t->storage->data[i] = THRandom_uniform(0,1);
  luaT_pushudata(L, t, torch_Tensor_id);
  THLongStorage_free(dimension);
  return 1;
}

static int lab_randn(lua_State *L)
{
  THLongStorage *dimension = lab_checklongargs(L, 1);
  THTensor *t = THTensor_newWithSize(dimension->size, dimension->data, NULL);
  long i;
  for(i = 0; i < t->storage->size; i++)
    t->storage->data[i] = THRandom_normal(0,1);
  luaT_pushudata(L, t, torch_Tensor_id);
  THLongStorage_free(dimension);
  return 1;
}

static int lab_range(lua_State *L)
{
  double xmin = luaL_checknumber(L, 1);
  double xmax = luaL_checknumber(L, 2);
  double step = luaL_optnumber(L, 3, 1);
  long size, i;
  THTensor *x;
  double *x_p;

  luaL_argcheck(L, step > 0, 3, "step must be a positive number");
  luaL_argcheck(L, xmax > xmin, 2, "upper bound must be larger than lower bound");

  size = (long)((xmax-xmin)/step+1);
  
  x = THTensor_newWithSize1d(size);
  x_p = THTensor_dataPtr(x);
  
  for(i = 0; i < size; i++)
    x_p[i] = xmin + ((double)i)*step;
  
  luaT_pushudata(L, x, torch_Tensor_id);
  return 1;
}

static int lab_randperm(lua_State *L)
{
  long n = luaL_checklong(L, 1);
  THTensor *t;
  double *t_p;
  long i;

  luaL_argcheck(L, n > 0, 1, "must be strictly positive");

  t = THTensor_newWithSize1d(n);
  t_p = THTensor_dataPtr(t);

  for(i = 0; i < n; i++)
      t_p[i] = (double)(i+1);

  for(i = 0; i < n-1; i++)
  {
    long z = THRandom_random() % (n-i);
    double sav = t_p[i];
    t_p[i] = t_p[z+i];
    t_p[z+i] = sav;
  }

  luaT_pushudata(L, t, torch_Tensor_id);
  return 1;
}

static const struct luaL_Reg lab_build__ [] = {
  {"ones", lab_ones},
  {"zeros", lab_zeros},
  {"diag", lab_diag},
  {"eye", lab_eye},
  {"linspace", lab_linspace},
  {"logspace", lab_logspace},
  {"rand", lab_rand},
  {"randn", lab_randn},
  {"range", lab_range},
  {"randperm", lab_randperm},
  {NULL, NULL}
};

void lab_build_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  luaL_register(L, NULL, lab_build__);
}
