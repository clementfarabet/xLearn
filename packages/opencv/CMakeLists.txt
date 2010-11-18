SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/opencv
                      ${CMAKE_MODULE_PATH})

SET(src opencv.c) 
SET(luasrc init.lua) 

FIND_PACKAGE(OpenCV)
IF(OpenCV_FOUND)
   #MESSAGE(STATUS "Found OpenCV: ${OpenCV_LIBRARIES} and ${OpenCV_INCLUDE_DIRS}")
   INCLUDE_DIRECTORIES (${OpenCV_INCLUDE_DIRS})
   LINK_DIRECTORIES (${OpenCV_LIBRARY_DIRS})
   ADD_TORCH_PACKAGE(opencv "${src}" "${luasrc}")
   TARGET_LINK_LIBRARIES(opencv luaT TH ${OpenCV_LIBRARIES})
ENDIF(OpenCV_FOUND)
