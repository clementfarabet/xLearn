#ifndef LUAT_UTILS_INC
#define LUAT_UTILS_INC

#include <lua.h>
#include <lauxlib.h>

#ifndef LUA_EXTERNC
# ifdef __cplusplus
#  define LUA_EXTERNC extern "C"
# else
#  define LUA_EXTERNC extern
# endif
#endif


#ifdef _MSC_VER
# define DLL_EXPORT __declspec(dllexport)
# define DLL_IMPORT __declspec(dllimport)
# ifdef luaT_EXPORTS
#  define LUAT_API LUA_EXTERNC DLL_EXPORT
# else
#  define LUAT_API LUA_EXTERNC DLL_IMPORT
# endif
#else
# define DLL_EXPORT
# define DLL_IMPORT
# define LUAT_API LUA_EXTERNC
#endif


/* C functions */

LUAT_API void* luaT_alloc(lua_State *L, long size);
LUAT_API void* luaT_realloc(lua_State *L, void *ptr, long size);
LUAT_API void luaT_free(lua_State *L, void *ptr);
LUAT_API void luaT_stackdump(lua_State *L);

LUAT_API const void* luaT_newmetatable(lua_State *L, const char *tname, const char *parenttname,
                                       lua_CFunction constructor, lua_CFunction destructor, lua_CFunction factory);

LUAT_API void luaT_pushmetatable(lua_State *L, const void *id);
LUAT_API void luaT_pushmetaclass(lua_State *L, const void *id);
LUAT_API int luaT_getmetaclass(lua_State *L, int index);

LUAT_API const char* luaT_id2typename(lua_State *L, const void *id);
LUAT_API const void* luaT_typename2id(lua_State *L, const char*);
LUAT_API const void* luaT_checktypename2id(lua_State *L, const char *tname);

LUAT_API const void* luaT_id(lua_State *L, int ud);
LUAT_API const char* luaT_typename(lua_State *L, int ud);

LUAT_API void luaT_pushudata(lua_State *L, void *udata, const void *id);

LUAT_API void *luaT_toudata (lua_State *L, int ud, const void *id);
LUAT_API int luaT_isudata (lua_State *L, int ud, const void *id);
LUAT_API void *luaT_checkudata (lua_State *L, int ud, const void *id);

LUAT_API void *luaT_getfieldcheckudata (lua_State *L, int ud, const char *field, const void *id);
LUAT_API void *luaT_getfieldchecklightudata (lua_State *L, int ud, const char *field);
LUAT_API double luaT_getfieldchecknumber (lua_State *L, int ud, const char *field);
LUAT_API int luaT_getfieldcheckint (lua_State *L, int ud, const char *field);
LUAT_API const char* luaT_getfieldcheckstring (lua_State *L, int ud, const char *field);
LUAT_API int luaT_getfieldcheckboolean (lua_State *L, int ud, const char *field);
LUAT_API void luaT_getfieldchecktable (lua_State *L, int ud, const char *field);

LUAT_API int luaT_typerror(lua_State *L, int ud, const char *tname);
LUAT_API int luaT_checkboolean(lua_State *L, int narg);
LUAT_API int luaT_optboolean(lua_State *L, int narg, int def);

LUAT_API const char *luaT_classrootname(const char *tname);
LUAT_API const char *luaT_classmodulename(const char *tname);
LUAT_API void luaT_stackdump(lua_State *L);

/* Lua functions */

LUAT_API int luaT_lua_newmetatable(lua_State *L);
LUAT_API int luaT_lua_factory(lua_State *L);
LUAT_API int luaT_lua_typename(lua_State *L);
LUAT_API int luaT_lua_isequal(lua_State *L);
LUAT_API int luaT_lua_pointer(lua_State *L);
LUAT_API int luaT_lua_setenv(lua_State *L);
LUAT_API int luaT_lua_getenv(lua_State *L);
LUAT_API int luaT_lua_getmetatable(lua_State *L);
LUAT_API int luaT_lua_version(lua_State *L);
LUAT_API int luaT_lua_setmetatable(lua_State *L);

#endif
