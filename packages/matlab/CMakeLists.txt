
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/matlab
                      ${CMAKE_MODULE_PATH})
FIND_PACKAGE(Matlab)

IF (MATLAB_FOUND)
   SET(src matlab.c)
   SET(luasrc init.lua)

   SET(CMAKE_C_FLAGS "-std=gnu99")

   INCLUDE_DIRECTORIES (${MATLAB_INCLUDE_DIR})
   LINK_DIRECTORIES (${CMAKE_LIBRARY_PATH})

   ADD_TORCH_PACKAGE(matlab "${src}" "${luasrc}" "Misc")
   TARGET_LINK_LIBRARIES(matlab luaT TH ${MATLAB_LIBRARIES})
ELSE (MATLAB_FOUND)
   MESSAGE("WARNING: Could not find Matlab headers/libs, wrapper not built")
ENDIF (MATLAB_FOUND)
