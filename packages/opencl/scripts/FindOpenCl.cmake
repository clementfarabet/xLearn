set(ENV_OPENCL_DIR $ENV{OPENCL_DIR})
if(ENV_OPENCL_DIR)
  find_path(
        OPENCL_INCLUDE_DIR
        NAMES CL/cl.h
        PATHS $ENV{OPENCL_DIR}/include
        NO_DEFAULT_PATH
        )

  if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
          set(
                OPENCL_LIB_SEARCH_PATH
                ${OPENCL_LIB_SEARCH_PATH}
                $ENV{OPENCL_DIR}/lib/x86_64
                )
        elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")
          set(
                OPENCL_LIB_SEARCH_PATH
                ${OPENCL_LIB_SEARCH_PATH}
                $ENV{OPENCL_DIR}/lib/x86
                )
        endif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  endif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  find_library(
        OPENCL_LIBRARY
        NAMES OpenCL
        PATHS ${OPENCL_LIB_SEARCH_PATH}
        NO_DEFAULT_PATH
        )
else(ENV_OPENCL_DIR)
  find_path(
        OPENCL_INCLUDE_DIR
        NAMES CL/cl.h
        )

  find_library(
        OPENCL_LIBRARY
        NAMES OpenCL
        )
endif(ENV_OPENCL_DIR)


IF (APPLE)
  FIND_LIBRARY(OPENCL_LIBRARY OpenCL DOC "OpenCL lib for OSX")
  FIND_PATH(OPENCL_INCLUDE_DIR OpenCL/cl.h DOC "Include for OpenCL on OSX")
ENDIF (APPLE)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OPENCL
  DEFAULT_MSG
  OPENCL_LIBRARY OPENCL_INCLUDE_DIR
  )

if(OPENCL_FOUND)
  set(OPENCL_LIBRARIES ${OPENCL_LIBRARY})
else(OPENCL_FOUND)
  set(OPENCL_LIBRARIES)
endif(OPENCL_FOUND)

mark_as_advanced(
  OPENCL_INCLUDE_DIR
  OPENCL_LIBRARY
  )

IF(OPENCL_FOUND)
  MESSAGE(STATUS "Found OpenCl library")
ELSE(OPENCL_FOUND)
  IF(OPENCL_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find OpenCl 
-- please give some paths to CMake or make sure libfreenect is installed in your system")
  ELSE(OPENCL_FIND_REQUIRED)
    MESSAGE(STATUS "Could not find OpenCl 
-- please give some paths to CMake or make sure libfreenect is installed in your system")
  ENDIF(OPENCL_FIND_REQUIRED)
ENDIF(OPENCL_FOUND)