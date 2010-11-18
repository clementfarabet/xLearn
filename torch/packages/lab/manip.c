#include "luaT.h"
#include "TH.h"
#include "utils.h"

static const void* torch_Tensor_id = NULL;
static const void* torch_LongStorage_id = NULL;
static const void* torch_DoubleStorage_id = NULL;

/* I cut and pasted (slightly adapted) the quicksort code from
   http://www.alienryderflex.com/quicksort/
   This public-domain C implementation by Darel Rex Finley.
   Thanks man :)
*/
#define  MAX_LEVELS  300
static void quicksortascend(double *arr, double *idx, long elements, long stride)
{
  long beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
  double piv, pid;
  
  beg[0]=0; end[0]=elements;
  while (i>=0) {
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      piv=arr[L*stride];
      pid=idx[L*stride];
      while (L<R) {
        while (arr[R*stride]>=piv && L<R) R--; if (L<R) {idx[L*stride]=idx[R*stride]; arr[L*stride]=arr[R*stride]; L++;}
        while (arr[L*stride]<=piv && L<R) L++; if (L<R) {idx[R*stride]=idx[L*stride]; arr[R*stride]=arr[L*stride]; R--;} }
      idx[L*stride]=pid; arr[L*stride]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
      if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
        swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
        swap=end[i]; end[i]=end[i-1]; end[i-1]=swap; }}
    else {
      i--; }}}

static void quicksortdescend(double *arr, double *idx, long elements, long stride)
{
  long beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;
  double piv, pid;
  
  beg[0]=0; end[0]=elements;
  while (i>=0) {
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      piv=arr[L*stride];
      pid=idx[L*stride];
      while (L<R) {
        while (arr[R*stride]<=piv && L<R) R--; if (L<R) {idx[L*stride]=idx[R*stride]; arr[L*stride]=arr[R*stride]; L++;}
        while (arr[L*stride]>=piv && L<R) L++; if (L<R) {idx[R*stride]=idx[L*stride]; arr[R*stride]=arr[L*stride]; R--;} }
      idx[L*stride]=pid; arr[L*stride]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
      if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
        swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
        swap=end[i]; end[i]=end[i-1]; end[i-1]=swap; }}
    else {
      i--; }}}

static int lab_repmat(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  THLongStorage *replication;
  long i, j;
  THTensor *tmp;
  long *repSize, *repStride;
  THDoubleStorage *repStorage;
  long totalSize;
  THTensor *newRepTensor, *narrowNewRepTensor, *prevRepTensor;

  replication = lab_checklongargs(L, 2);

  for(i = 0; i < replication->size; i++)
  {
    if(replication->data[i] <= 0)
    {
      THLongStorage_free(replication);
      luaL_error(L, "invalid replication number");
    }
  }

  
  if(replication->size < t->nDimension)
  {
    int rsize = replication->size;
    THLongStorage_resize(replication, t->nDimension, 1);
    for(i = rsize; i < replication->size; i++)
      replication->data[i] = 1;
  }

  if(t->nDimension < replication->size)
  {
    long *size = THAlloc(sizeof(long)*replication->size);
    long *stride = THAlloc(sizeof(long)*replication->size);
    for(i = 0; i < t->nDimension; i++)
    {
      size[i] = t->size[i];
      stride[i] = t->stride[i];
    }
    for(i = t->nDimension; i < replication->size; i++)
    {
      size[i] = 1;
      stride[i] = 0;
    }
    tmp = THTensor_newWithStorage(t->storage, t->storageOffset, replication->size, size, stride);
    THFree(size);
    THFree(stride);
  }
  else
  {
    tmp = t;
    THTensor_retain(tmp);
  }

  repSize = THAlloc(sizeof(long)*tmp->nDimension);
  repStride= THAlloc(sizeof(long)*tmp->nDimension);
  totalSize = 1;
  for(i = 0; i < tmp->nDimension; i++)
  {
    repSize[i] = tmp->size[i]; /* we start with original tensor size, but allocate space for the new one */
    if(i == 0)
      repStride[i] = 1;
    else
      repStride[i] = (tmp->size[i-1]*replication->data[i-1])*repStride[i-1];
    totalSize += (tmp->size[i]*replication->data[i]-1)*repStride[i];
  }
  repStorage = THDoubleStorage_newWithSize(totalSize);
  
  prevRepTensor = THTensor_newWithStorage(repStorage, 0, tmp->nDimension, repSize, repStride);
  THTensor_copy(prevRepTensor, tmp);
  narrowNewRepTensor = THTensor_new();
  for(i = 0; i < replication->size; i++)
  {    
    repSize[i] *= replication->data[i];
    newRepTensor = THTensor_newWithStorage(repStorage, 0, tmp->nDimension, repSize, repStride);
    for(j = 1; j < replication->data[i]; j++)
    {
      THTensor_narrow(narrowNewRepTensor, newRepTensor, i, j*tmp->size[i], tmp->size[i]);
      THTensor_copy(narrowNewRepTensor, prevRepTensor);
    }
    THTensor_free(prevRepTensor);
    prevRepTensor = newRepTensor;
  }
  THTensor_free(narrowNewRepTensor);

  luaT_pushudata(L, prevRepTensor, torch_Tensor_id);
  THFree(repSize);
  THFree(repStride);
  THDoubleStorage_free(repStorage);
  THTensor_free(tmp);
  THLongStorage_free(replication);
  return 1;
}

static int lab_reshape(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  THLongStorage *size = lab_checklongargs(L, 2);
  long negIndex, nElement, nElement_;
  long d;
  THTensor *r;

  if(t->nDimension == 0)
  {
    if(size->size != 0)
    {
      THLongStorage_free(size);
      luaL_error(L, "invalid size");
    }
  }

  negIndex = -1;
  for(d = 0; d < size->size; d++)
  {
    if(size->data[d] <= 0)
    {
      if(negIndex >= 0)
      {
        THLongStorage_free(size);
        luaL_error(L, "invalid size for dimension %d", d);
      }
      negIndex = d;
      size->data[d] = 1; /* temporary */
    }
  }

  nElement = 1;
  for(d = 0; d < size->size; d++)
    nElement *= size->data[d];
    
  nElement_ = THTensor_nElement(t);
  if(negIndex >= 0)
  {
    if(nElement_ % nElement)
    {
      THLongStorage_free(size);
      luaL_error(L, "invalid size");
    }
    size->data[negIndex] = nElement_ / nElement;
    nElement *= size->data[negIndex];
  }

  if(nElement != nElement_)
  {
    THLongStorage_free(size);
    luaL_error(L, "inconsistent sizes");
  }
  
  r = THTensor_newWithSize(size->size, size->data, NULL);
  luaT_pushudata(L, r, torch_Tensor_id);
  THTensor_copy(r, t);
  THLongStorage_free(size);
  return 1;
}

/* sort stuff */
typedef struct intdouble_
{
    long index;
    double value;
} longdouble;
  
static int comparelongdoubleascending(const void *v1, const void *v2)
{
  if( (*((const longdouble **)v1))->value > (*((const longdouble **)v2))->value )
    return 1;
  else
    return -1;
}
  
static int comparelongdoubledescending(const void *v1, const void *v2)
{
  if( (*((const longdouble **)v1))->value < (*((const longdouble **)v2))->value )
    return 1;
  else
    return -1;
}

static int lab_sort(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  int dimension = luaL_optint(L, 2, 1)-1;
  int descendingOrder = luaT_optboolean(L, 3, 0);
  THTensor *rt;
  THTensor *ri;
  longdouble *tidata;
  longdouble **tidatap;
  long i;

  luaL_argcheck(L, dimension >= 0 && dimension < t->nDimension, 2, "invalid dimension");

  rt = THTensor_new();
  THTensor_resizeAs(rt, t);
  THTensor_copy(rt, t);
  ri = THTensor_new();
  THTensor_resizeAs(ri, t);

  if(descendingOrder)
  {
    TH_TENSOR_DIM_APPLY2(double, rt, double, ri, dimension, 
                         for(i = 0; i < rt_size; i++)
                           ri_p[i*ri_stride] = i+1;
                         quicksortdescend(rt_p, ri_p, rt_size, rt_stride);)
  }
  else
  {
    TH_TENSOR_DIM_APPLY2(double, rt, double, ri, dimension, 
                         for(i = 0; i < rt_size; i++)
                           ri_p[i*ri_stride] = i+1;
                         quicksortascend(rt_p, ri_p, rt_size, rt_stride);)
  }

  luaT_pushudata(L, rt, torch_Tensor_id);
  luaT_pushudata(L, ri, torch_Tensor_id);
  return 2;
}

static int lab_tril(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  long k = luaL_optlong(L, 2, 0);
  THTensor *rt;
  long strider, stridec, stridedst;
  double *src, *dst;
  long r, c;

  luaL_argcheck(L, t->nDimension == 2, 1, "not a matrix");

  rt = THTensor_new();
  THTensor_resizeAs(rt, t);
  luaT_pushudata(L, rt, torch_Tensor_id);

  strider = t->stride[0];
  stridec = t->stride[1];
  stridedst = t->stride[1];
  dst = THTensor_dataPtr(rt);
  src = THTensor_dataPtr(t);
  for(c = 0; c < t->size[1]; c++)
  {
    long sz_ = (c-k <= t->size[0] ? c-k : t->size[0]);
    for(r = 0; r < sz_; r++)
      dst[r] = 0;
    for(r = (c-k >= 0 ? c-k : 0); r < t->size[0]; r++)
      dst[r] = src[r*strider];
    src += stridec;
    dst += stridedst;
  }
  return 1;
}

static int lab_triu(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  long k = luaL_optlong(L, 2, 0);
  THTensor *rt;
  long strider, stridec, stridedst;
  double *src, *dst;
  long r, c;

  luaL_argcheck(L, t->nDimension == 2, 1, "not a matrix");

  rt = THTensor_new();
  THTensor_resizeAs(rt, t);
  luaT_pushudata(L, rt, torch_Tensor_id);

  strider = t->stride[0];
  stridec = t->stride[1];
  stridedst = t->stride[1];
  dst = THTensor_dataPtr(rt);
  src = THTensor_dataPtr(t);
  for(c = 0; c < t->size[1]; c++)
  {
    long sz_ = (c-k+1 <= t->size[0] ? c-k+1 : t->size[0]);
    for(r = 0; r < sz_; r++)
      dst[r] = src[r*strider];
    for(r = (c-k+1 >= 0 ? c-k+1 : 0); r < t->size[0]; r++)
      dst[r] = 0;
    src += stridec;
    dst += stridedst;
  }
  return 1;
}

static int lab_map(lua_State *L)
{
  THTensor *t = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *r;
  luaL_checktype(L, 2, LUA_TFUNCTION);

  r = THTensor_new();
  THTensor_resizeAs(r, t);
  luaT_pushudata(L, r, torch_Tensor_id);

  TH_TENSOR_APPLY2(double, r, double, t,
                  lua_pushvalue(L, 2);
                  lua_pushnumber(L, *t_p);
                  lua_call(L, 1, 1);
                  if(lua_isnumber(L, 4))
                  {
                    *r_p = lua_tonumber(L, 4);
                    lua_pop(L, 1);
                  }
                  else if(lua_isnil(L, 4))
                  {
                    *r_p = 0;
                    lua_pop(L, 1);
                  }
                  else
                    luaL_error(L, "given function should return a number or nil"););

  return 1;
}

static const struct luaL_Reg lab_manip__ [] = {
  {"repmat", lab_repmat},
  {"reshape", lab_reshape},
  {"sort", lab_sort},
  {"tril", lab_tril},
  {"triu", lab_triu},
  {"map", lab_map},
  {NULL, NULL}
};

void lab_manip_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  torch_LongStorage_id = luaT_checktypename2id(L, "torch.LongStorage");
  torch_DoubleStorage_id = luaT_checktypename2id(L, "torch.DoubleStorage");
  luaL_register(L, NULL, lab_manip__);
}
