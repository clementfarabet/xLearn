SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/opencv
                      ${CMAKE_MODULE_PATH})

SET(src opencv.c) 
SET(luasrc init.lua) 

FIND_PACKAGE(OpenCV)

IF(OpenCV_FOUND)
   MESSAGE(STATUS "Found OpenCV: ${OpenCV_LIBS}")
   INCLUDE_DIRECTORIES (${OpenCV_INCLUDE_DIRS})
   LINK_DIRECTORIES (${OpenCV_LIBRARY_DIRS})
   ADD_TORCH_PACKAGE(opencv "${src}" "${luasrc}")
   TARGET_LINK_LIBRARIES(opencv luaT TH ${OpenCV_LIBS})
ELSE(OpenCV_FOUND)
   MESSAGE("WARNING: Could not find OpenCV, OpenCV wrapper will not be installed")
ENDIF(OpenCV_FOUND)
