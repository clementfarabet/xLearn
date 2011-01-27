
SET(src stereo.cpp image.h misc.h filter.h imconv.h convolve.h imutil.h) 
SET(luasrc init.lua left.jpg right.jpg) 

IF (USE_LUAJIT)
   SET(CMAKE_CXX_FLAGS "-DLUAJIT")
   MESSAGE(STATUS "QTLua: building against LuaJIT")
ENDIF (USE_LUAJIT)

ADD_TORCH_PACKAGE(stereo "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(stereo luaT TH)
