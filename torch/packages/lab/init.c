#include "luaT.h"

extern void lab_math_init(lua_State *L);
extern void lab_stat_init(lua_State *L);
extern void lab_op_init(lua_State *L);
extern void lab_manip_init(lua_State *L);
extern void lab_build_init(lua_State *L);
extern void lab_utils_init(lua_State *L);

DLL_EXPORT int luaopen_liblab(lua_State *L)
{
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, "lab");

  lab_math_init(L);
  lab_stat_init(L);
  lab_op_init(L);
  lab_manip_init(L);
  lab_build_init(L);
  lab_utils_init(L);

  return 1;
}
