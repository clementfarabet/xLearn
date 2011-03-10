
SET(src mstsegm.cpp)
SET(luasrc init.lua Graph.lua)

IF (USE_LUAJIT)
   SET(CMAKE_CXX_FLAGS "-DLUAJIT")
   MESSAGE(STATUS "QTLua: building against LuaJIT")
ENDIF (USE_LUAJIT)

ADD_TORCH_PACKAGE(mstsegm "${src}" "${luasrc}" "Graphs")
TARGET_LINK_LIBRARIES(mstsegm luaT TH)
