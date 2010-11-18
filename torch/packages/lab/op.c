#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;

static int lab_cross(lua_State *L)
{
  THTensor *a = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *b = luaT_checkudata(L, 2, torch_Tensor_id);
  int dimension = luaL_optint(L, 3, 0)-1;
  THTensor *r;
  int i;

  if(a->nDimension != b->nDimension)
    luaL_error(L, "inconsitent tensor sizes");
  
  for(i = 0; i < a->nDimension; i++)
  {
    if(a->size[i] != b->size[i])
      luaL_error(L, "inconsitent tensor sizes");
  }
  
  if(dimension < 0)
  {
    for(i = 0; i < a->nDimension; i++)
    {
      if(a->size[i] == 3)
      {
        dimension = i;
        break;
      }
    }
    if(dimension < 0)
      luaL_error(L, "no dimension of size 3");
  }

  luaL_argcheck(L, dimension >= 0 && dimension < a->nDimension, 3, "dimension out of range");
  luaL_argcheck(L, a->size[dimension] == 3, 3, "dimension size is not 3");

  r = THTensor_new();
  THTensor_resizeAs(r, a);
  luaT_pushudata(L, r, torch_Tensor_id);

  TH_TENSOR_DIM_APPLY3(double, a, double, b, double, r, dimension,
                       r_p[0*r_stride] = a_p[1*a_stride]*b_p[2*b_stride] - a_p[2*a_stride]*b_p[1*b_stride];
                       r_p[1*r_stride] = a_p[2*a_stride]*b_p[0*b_stride] - a_p[0*a_stride]*b_p[2*b_stride];
                       r_p[2*r_stride] = a_p[0*a_stride]*b_p[1*b_stride] - a_p[1*a_stride]*b_p[0*b_stride];)
    
  return 1;
}

static const struct luaL_Reg lab_op__ [] = {
  {"cross", lab_cross},
  {NULL, NULL}
};

void lab_op_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  luaL_register(L, NULL, lab_op__);
}
