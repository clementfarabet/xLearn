
// -*- C++ -*-

#include "qtopengl.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lauxlib.h"
#include "lualib.h"

#include <QAction>
#include <QApplication>
#include <QGLWidget>
#include <QWidget>
#include "hellogl.h"

// ========================================
// QGLWIDGET


static int 
qglwidget_new(lua_State *L)
{
  QWidget *parent = luaQ_optqobject<QWidget>(L, 1);
  QGLWidget *w = new QGLWidget(parent);
  luaQ_pushqt(L, w, !parent);
  return 1;
}

static struct luaL_Reg qglwidget_lib[] = {
  {"new", qglwidget_new},
  {0,0}
};

static int qglwidget_hook(lua_State *L) 
{
  lua_getfield(L, -1, "__metatable");
  luaQ_register(L, qglwidget_lib, QCoreApplication::instance());
  return 0;
}

static int 
hellogl_new(lua_State *L)
{
  QWidget *parent = luaQ_optqobject<QWidget>(L, 1);
  HelloGL *w = new HelloGL(parent);
  luaQ_pushqt(L, w, !parent);
  return 1;
}

static struct luaL_Reg hellogl_lib[] = {
  {"new", hellogl_new},
  {0,0}
};

static int hellogl_hook(lua_State *L) 
{
  lua_getfield(L, -1, "__metatable");
  luaQ_register(L, hellogl_lib, QCoreApplication::instance());
  return 0;
}


// ====================================


#ifndef LUAJIT
LUA_EXTERNC
#endif
QTOPENGL_API int 
luaopen_libqtopengl(lua_State *L)
{
  // load module 'qt'
  if (luaL_dostring(L, "require 'qt'"))
    lua_error(L);
  if (QApplication::type() == QApplication::Tty)
    luaL_error(L, "Graphics have been disabled (running with -nographics)");

  // register object types
  QtLuaEngine::registerMetaObject(&QGLWidget::staticMetaObject);
  QtLuaEngine::registerMetaObject(&HelloGL::staticMetaObject);

  // call hook for qobjects
#define HOOK_QOBJECT(T, t) \
     lua_pushcfunction(L, t ## _hook);\
     luaQ_pushmeta(L, &T::staticMetaObject);\
     lua_call(L, 1, 0)
  
  HOOK_QOBJECT(QGLWidget, qglwidget);  
  HOOK_QOBJECT(HelloGL, hellogl);

  return 0;
}



/* -------------------------------------------------------------
   Local Variables:
   c++-font-lock-extra-types: ("\\sw+_t" "\\(lua_\\)?[A-Z]\\sw*[a-z]\\sw*")
   End:
   ------------------------------------------------------------- */



