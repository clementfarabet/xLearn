# add find script for libusb and libfreenect
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/kinect/scripts
                      ${CMAKE_MODULE_PATH})
find_package(Freenect REQUIRED)

IF (FREENECT_FOUND)
   SET(src libfreenect_sync.c kinect.c) 
   SET(luasrc init.lua Kinect.lua) 

   ADD_TORCH_PACKAGE(kinect "${src}" "${luasrc}")

   INCLUDE_DIRECTORIES(${FREENECT_INCLUDE_DIR})

   TARGET_LINK_LIBRARIES(kinect luaT TH ${FREENECT_LIBRARIES})
ELSE (FREENECT_FOUND)
   MESSAGE("WARNING: Could not find libfreenect, Kinect wrapper will not be installed")
ENDIF (FREENECT_FOUND)
