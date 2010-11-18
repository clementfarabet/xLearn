#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;

static int lab_numel(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  lua_pushnumber(L, THTensor_nElement(t));
  return 1;
}

static int lab_max(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *values, *indices;
  long *dim;
  int i;

  luaL_argcheck(L, dimension >= 0 && dimension < t->nDimension, 2, "dimension out of range");

  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  values = THTensor_newWithSize(t->nDimension, dim, NULL);
  indices = THTensor_newWithSize(t->nDimension, dim, NULL);
  THFree(dim);
  luaT_pushudata(L, values, torch_Tensor_id);
  luaT_pushudata(L, indices, torch_Tensor_id);

  TH_TENSOR_DIM_APPLY3(double, t, double, values, double, indices, dimension,
                       int theIndex = 0;
                       double theMax = t_p[0];
                       for(i = 1; i < t_size; i++)
                       {
                         if(t_p[i*t_stride] > theMax)
                         {
                           theIndex = i;
                           theMax = t_p[i*t_stride];
                         }
                       }
                       *indices_p = theIndex+1;
                       *values_p = theMax;)

  return 2;
}

static int lab_min(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *values, *indices;
  long *dim;
  int i;

  luaL_argcheck(L, dimension >= 0 && dimension < t->nDimension, 2, "dimension out of range");

  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  values = THTensor_newWithSize(t->nDimension, dim, NULL);
  indices = THTensor_newWithSize(t->nDimension, dim, NULL);
  THFree(dim);
  luaT_pushudata(L, values, torch_Tensor_id);
  luaT_pushudata(L, indices, torch_Tensor_id);

  TH_TENSOR_DIM_APPLY3(double, t, double, values, double, indices, dimension,
                       int theIndex = 0;
                       double theMin = t_p[0];
                       for(i = 1; i < t_size; i++)
                       {
                         if(t_p[i*t_stride] < theMin)
                         {
                           theIndex = i;
                           theMin = t_p[i*t_stride];
                         }
                       }
                       *indices_p = theIndex+1;
                       *values_p = theMin;)

  return 2;
}

static int lab_sum(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L,dimension >= 0 && dimension < t->nDimension, 2, "dimension out of range");
  
  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double sum = 0;
                       long i;
                       for(i = 0; i < t_size; i++)
                         sum += t_p[i*t_stride];
                       *r_p = sum;)

  luaT_pushudata(L, r, torch_Tensor_id);
  return 1;
}

static int lab_prod(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L,dimension >= 0 && dimension < t->nDimension, 2, "dimension out of range");
  
  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double prod = 1;
                       long i;
                       for(i = 0; i < t_size; i++)
                         prod *= t_p[i*t_stride];
                       *r_p = prod;)

  luaT_pushudata(L, r, torch_Tensor_id);
  return 1;
}

static int lab_cumsum(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L,dimension >= 0 && dimension < t->nDimension, 2, "dimension out of range");
  
  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double cumsum = 0;
                       long i;
                       for(i = 0; i < t_size; i++)
                       {
                         cumsum += t_p[i*t_stride];
                         r_p[i*r_stride] = cumsum;
                       })

  luaT_pushudata(L, r, torch_Tensor_id);
  return 1;
}

static int lab_cumprod(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L,dimension >= 0 && dimension < t->nDimension, 2, "dimension out of range");
  
  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double cumprod = 1;
                       long i;
                       for(i = 0; i < t_size; i++)
                         {
                           cumprod *= t_p[i*t_stride];
                           r_p[i*r_stride] = cumprod;
                         })
    
  luaT_pushudata(L, r, torch_Tensor_id);
  return 1;
}

static int lab_mean(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L, dimension >= 0 && dimension < t->nDimension, 2, "invalid dimension");

  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  luaT_pushudata(L, r, torch_Tensor_id);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double sum = 0;
                       long i;
                       for(i = 0; i < t_size; i++)
                         sum += t_p[i*t_stride];
                       *r_p = sum/t_size;)

  return 1;
}

static int lab_std(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int flag = luaT_optboolean(L, 2, 0);
  int dimension = luaL_optint(L, 3, 1)-1;  
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L, dimension >= 0 && dimension < t->nDimension, 3, "invalid dimension");

  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  luaT_pushudata(L, r, torch_Tensor_id);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double sum = 0;
                       double sum2 = 0;
                       long i;
                       for(i = 0; i < t_size; i++)
                       {
                         double z = t_p[i*t_stride];
                         sum += z;
                         sum2 += z*z;
                       }

                       if(flag)
                       {
                         sum /= t_size;
                         sum2 /= t_size;
                         sum2 -= sum*sum;
                         sum2 = (sum2 < 0 ? 0 : sum2);
                         *r_p = sqrt(sum2);
                       }
                       else
                       {
                         sum /= t_size;
                         sum2 /= t_size-1;
                         sum2 -= ((double)t_size)/((double)(t_size-1))*sum*sum;
                         sum2 = (sum2 < 0 ? 0 : sum2);
                         *r_p = sqrt(sum2);
                       })

  return 1;
}

static int lab_var(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int flag = luaT_optboolean(L, 2, 0);
  int dimension = luaL_optint(L, 3, 1)-1;  
  THTensor *r = NULL;
  long *dim;
  int i;

  luaL_argcheck(L, dimension >= 0 && dimension < t->nDimension, 3, "invalid dimension");

  dim = THAlloc(sizeof(long)*t->nDimension);
  for(i = 0; i < t->nDimension; i++)
    dim[i] = t->size[i];
  dim[dimension] = 1;
  r = THTensor_newWithSize(t->nDimension, dim, NULL);
  luaT_pushudata(L, r, torch_Tensor_id);
  THFree(dim);

  TH_TENSOR_DIM_APPLY2(double, t, double, r, dimension,
                       double sum = 0;
                       double sum2 = 0;
                       long i;
                       for(i = 0; i < t_size; i++)
                       {
                         double z = t_p[i*t_stride];
                         sum += z;
                         sum2 += z*z;
                       }

                       if(flag)
                       {
                         sum /= t_size;
                         sum2 /= t_size;
                         sum2 -= sum*sum;
                         sum2 = (sum2 < 0 ? 0 : sum2);
                         *r_p = sum2;
                       }
                       else
                       {
                         sum /= t_size;
                         sum2 /= t_size-1;
                         sum2 -= ((double)t_size)/((double)(t_size-1))*sum*sum;
                         sum2 = (sum2 < 0 ? 0 : sum2);
                         *r_p = sum2;
                       })

  return 1;
}

static int lab_norm(lua_State *L)
{ 
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  double value = luaL_optnumber(L, 2, 2);
  double sum = 0; 

  TH_TENSOR_APPLY(double, t, sum += pow(fabs(*t_p), value);)

  lua_pushnumber(L, pow(sum,1.0/value));
  return 1;
}

static int lab_dist(lua_State *L)
{ 
  THTensor *a = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *b = luaT_checkudata(L, 2, torch_Tensor_id);
  double value = luaL_optnumber(L, 3, 2);
  double sum = 0; 
  TH_TENSOR_APPLY2(double, a, double, b, sum += pow(fabs(*a_p - *b_p), value);)
  lua_pushnumber(L, pow(sum,1.0/value));
  return 1;
}

static int lab_trace(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  double *t_p = THTensor_dataPtr(t);
  double sum = 0;
  long i = 0;

  luaL_argcheck(L, t->nDimension == 2, 1, "not a matrix");

  while( (i < t->size[0]) && (i < t->size[1]) )
  {
    sum += *(t_p+i*t->stride[0]+i*t->stride[1]);
    i++;
  }

  lua_pushnumber(L, sum);
  return 1;
}

static const struct luaL_Reg lab_stat__ [] = {
  {"numel", lab_numel},
  {"max", lab_max},
  {"min", lab_min},
  {"sum", lab_sum},
  {"prod", lab_prod},
  {"cumsum", lab_cumsum},
  {"cumprod", lab_cumprod},
  {"mean", lab_mean},
  {"std", lab_std},
  {"var", lab_var},
  {"norm", lab_norm},
  {"dist", lab_dist},
  {"trace", lab_trace},
  {NULL, NULL}
};

void lab_stat_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  luaL_register(L, NULL, lab_stat__);
}
