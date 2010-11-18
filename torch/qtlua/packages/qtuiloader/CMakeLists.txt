# -*- cmake -*-

SET(QT_USE_QTXML 1)
SET(QT_USE_QTUITOOLS 1)
INCLUDE(${QT_USE_FILE})		     
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

IF (USE_LUAJIT)
   SET(CMAKE_CXX_FLAGS "-DLUAJIT")
   MESSAGE(STATUS "QTLua: building against LuaJIT")
ENDIF (USE_LUAJIT)

SET(qtuiloader_SRC uiloader.h uiloader.cpp)

MACRO_INSTALL_QTLUA_FILES(qtuiloader init.lua test.lua test.ui)

MACRO_ADD_QTLUA_MODULE(libqtuiloader ${qtuiloader_SRC})

ADD_TORCH_HELP(qtuiloader "QT Interface")
