#include "luaT.h"
#include "mpeg2decoder.h"

static const void *torch_Tensor_id = NULL;
static const void *mpeg2_Decoder_id   = NULL;

static int mpeg2_Decoder_new(lua_State *L){
  tDecoder *p=NULL;
  const char *fname=luaL_checkstring(L,1);
  const char *type=luaL_optstring(L,2,NULL);
  int optarg = luaL_optnumber(L, 3, -1);
  if (type!=NULL && optarg>-1)
    p=decoder_new2(fname,type[0],optarg);
  else
    p=decoder_new(fname);
  luaT_pushudata(L, p, mpeg2_Decoder_id);
  return 1;
}

static int mpeg2_Decoder_delete(lua_State *L){
  tDecoder *p=(tDecoder*)luaT_checkudata(L,1,mpeg2_Decoder_id);
  decoder_delete(p);
  return 0;
}
static int mpeg2_Decoder_newFrame(lua_State *L){
  tDecoder *p=(tDecoder*)luaT_checkudata(L,1,mpeg2_Decoder_id);
  THTensor *x=(THTensor *)luaT_checkudata(L,2,torch_Tensor_id);
  int ret=decoder_newFrame(p,x);
  lua_pushnumber(L,ret);
  return 1;
}

static const struct luaL_Reg mpeg2_Decoder__ [] = {
  {"newFrame", mpeg2_Decoder_newFrame},
  {NULL, NULL}
};


DLL_EXPORT int luaopen_libmpeg2(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, "mpeg2");

  mpeg2_Decoder_id =luaT_newmetatable(L, "mpeg2.Decoder",  NULL, mpeg2_Decoder_new, mpeg2_Decoder_delete, NULL);
  luaL_register(L,NULL,mpeg2_Decoder__);


  return 1;
}


