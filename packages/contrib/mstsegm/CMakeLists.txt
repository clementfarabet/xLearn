
SET(src mstsegm.cpp segment-image.h segment-graph.h disjoint-set.h image.h misc.h filter.h imconv.h convolve.h imutil.h)
SET(luasrc init.lua)

IF (USE_LUAJIT)
   SET(CMAKE_CXX_FLAGS "-DLUAJIT")
   MESSAGE(STATUS "QTLua: building against LuaJIT")
ENDIF (USE_LUAJIT)

ADD_TORCH_PACKAGE(mstsegm "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(mstsegm luaT TH)
