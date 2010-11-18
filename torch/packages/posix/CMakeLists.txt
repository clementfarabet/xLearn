IF(UNIX)

  SET(src libposix.c)
  SET(luasrc init.lua)

  ADD_TORCH_PACKAGE(posix "${src}" "${luasrc}" "System")

  TARGET_LINK_LIBRARIES(posix ${LUA_LIBRARIES})

ENDIF(UNIX)
