#include "luaT.h"
#include "TH.h"

extern void image_transform_init(lua_State *L);

DLL_EXPORT int luaopen_libimage(lua_State *L)
{
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, "image");

  image_transform_init(L);

  return 1;
}
