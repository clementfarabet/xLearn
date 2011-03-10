SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/mpeg2
                      ${CMAKE_MODULE_PATH})


FIND_PACKAGE(MPEG2)
IF(MPEG2_FOUND)
  SET(src mpeg2decoder.c init.c )
  SET(luasrc init.lua )

  ADD_TORCH_PACKAGE(mpeg2 "${src}" "${luasrc}" "MPEG2 package")

  INCLUDE_DIRECTORIES(${MPEG2_INCLUDE_DIR})
  LINK_DIRECTORIES(${MPEG2_LIBRARY_DIRS}) 
  TARGET_LINK_LIBRARIES(mpeg2 luaT TH ${MPEG2_LIBRARIES})
ELSE(MPEG2_FOUND)
   MESSAGE("WARNING: Could not find MPEG2 libraries, MPEG2 wrapper will not be installed")
ENDIF(MPEG2_FOUND)

