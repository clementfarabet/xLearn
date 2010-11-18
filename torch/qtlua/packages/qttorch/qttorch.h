// -*- C++ -*-

#ifndef QTTORCH_H
#define QTTORCH_H

#ifdef LUAJIT
#include "lua.hpp"
#else
#include "lua.h"
#include "lauxlib.h"
#endif
#include "qtluaengine.h"
#include "qtluautils.h"


#ifdef LUA_BUILD_AS_DLL
# ifdef libqttorch_EXPORTS
#  define QTTORCH_API __declspec(dllexport)
# else
#  define QTTORCH_API __declspec(dllimport)
# endif
#else
# define QTTORCH_API /**/
#endif

#ifndef LUA_EXTERNC
#define LUA_EXTERNC extern "C"
#endif

LUA_EXTERNC QTTORCH_API int luaopen_libqttorch(lua_State *L);


#endif


/* -------------------------------------------------------------
   Local Variables:
   c++-font-lock-extra-types: ("\\sw+_t" "\\(lua_\\)?[A-Z]\\sw*[a-z]\\sw*")
   End:
   ------------------------------------------------------------- */


