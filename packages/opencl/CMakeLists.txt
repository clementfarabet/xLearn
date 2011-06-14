# add find script for libusb and libfreenect
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/opencl/scripts
                      ${CMAKE_MODULE_PATH})
find_package(OpenCl REQUIRED)

IF(OPENCL_FOUND)   # build wrapper
   SET(CMAKE_C_FLAGS "-fpic -std=c99 -Werror -g -Wall -D_XOPEN_SOURCE=500")
   SET(CMAKE_CXX_FLAGS "-fpic -std=c99 -Werror -g -Wall -D_XOPEN_SOURCE=500")

   SET(src cllua-helper.c 
           opencl.c
           ) 
   SET(luasrc init.lua
              Tensor.lua 
              Context.lua 
              Kernel.lua
              CLSpatialConvolution.lua
              ) 

   ADD_TORCH_PACKAGE(opencl "${src}" "${luasrc}")
   INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})
   TARGET_LINK_LIBRARIES(opencl luaT TH ${OPENCL_LIBRARY})
ELSE (OPENCL_FOUND)
   MESSAGE("WARNING: Could not find OpenCl, wrapper will not be installed")
ENDIF (OPENCL_FOUND)


