
FIND_PACKAGE(JPEG)

IF(JPEG_FOUND)
   SET(src jpeg.c)
   SET(luasrc init.lua)

   ADD_TORCH_PACKAGE(jpeg "${src}" "${luasrc}")

   INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
   TARGET_LINK_LIBRARIES(jpeg luaT TH ${JPEG_LIBRARIES})
ELSE(JPEG_FOUND)
   MESSAGE("WARNING: Could not find JPEG libraries, jpeg wrapper will not be installed")
ENDIF(JPEG_FOUND)
