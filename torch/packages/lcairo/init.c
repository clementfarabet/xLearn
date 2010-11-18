#include "luaT.h"
#include "window.h"

extern void lcairo_init(lua_State *L);

DLL_EXPORT int luaopen_liblcairo(lua_State *L)
{
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, "lcairo");

  lcairo_init(L);

  window_init();

  return 1;
}
