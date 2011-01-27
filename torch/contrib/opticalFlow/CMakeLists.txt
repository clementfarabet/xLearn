
SET(src opticalFlow.cpp GaussianPyramid.cpp GaussianPyramid.h Image.h ImageIO.h ImageProcessing.h OpticalFlowCode.cpp OpticalFlow.h project.h)
SET(luasrc init.lua img1.jpg img2.jpg)

SET(QT_MIN_VERSION "4.3.0")
FIND_PACKAGE(Qt4)

IF (USE_LUAJIT)
   SET(CMAKE_CXX_FLAGS "-DLUAJIT")
   MESSAGE(STATUS "QTLua: building against LuaJIT")
ENDIF (USE_LUAJIT)

IF (QT4_FOUND)
  INCLUDE_DIRECTORIES (${QT_INCLUDES})
  INCLUDE (${QT_USE_FILE})
  LINK_DIRECTORIES (${QT_LIBRARY_DIR})
  ADD_TORCH_PACKAGE(opticalFlow "${src}" "${luasrc}")
  TARGET_LINK_LIBRARIES(opticalFlow luaT TH ${QT_LIBRARIES})
ENDIF (QT4_FOUND)
