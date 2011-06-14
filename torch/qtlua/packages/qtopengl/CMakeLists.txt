# -*- cmake -*-
FIND_PACKAGE(Qt COMPONENTS QtOpenGL)
MESSAGE(STATUS "QTLua: libs :" ${QT_QTOPENGL_LIBRARIES})

IF(QT_QTOPENGL_FOUND)
  MESSAGE(STATUS "QTLua: building against opengl")
  SET(QT_USE_QTOPENGL 1)
  INCLUDE(${QT_USE_FILE})		     
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
  
  SET(qtopengl_SRC hellogl.h hellogl.cpp qtopengl.h qtopengl.cpp)

  MACRO_INSTALL_QTLUA_FILES(qtopengl init.lua)

  IF (APPLE)
     SET(QT_LIBRARIES ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/OpenGL.framework ${QT_LIBRARIES})
  ENDIF (APPLE)
  
  MACRO_QT4_AUTOGEN(qtopengl_GEN ${qtopengl_SRC})
  MACRO_ADD_QTLUA_MODULE(libqtopengl ${qtopengl_SRC} ${qtopengl_GEN})
  
  ADD_TORCH_HELP(qtopengl "QT Interface")
ENDIF(QT_QTOPENGL_FOUND)
