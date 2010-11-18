/*======================================================================*\
* LuaThreads: multi-(platform|threading) support for the Lua language.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: luathread.h,v 1.12 2006/04/27 21:29:41 diego Exp $
\*======================================================================*/
#ifndef LUATHREAD_H
#define LUATHREAD_H

#include "pt.h"

#define LUATHREAD_VERSION    "LuaThread 1.1 (beta)"
#define LUATHREAD_COPYRIGHT  "Copyright (C) 2004-2006 Diego Nehab"
#define LUATHREAD_AUTHORS    "Diego Nehab"

#ifndef LUATHREAD_API
#define LUATHREAD_API
#endif

/*-----------------------------------------------------------------------*\
* Initializes the LuaThreads library, making available to Lua scripts the
* newthread, newmutex and newcond functions.
\*-----------------------------------------------------------------------*/
LUATHREAD_API int luaopen_thread_core(lua_State *L);

#endif
