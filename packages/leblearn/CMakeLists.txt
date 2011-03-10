
SET(src leblearn.cpp)
SET(luasrc init.lua) 

SET(EBLearn_LIBRARY_DIRS "/usr/lib/")
SET(EBLearn_INCLUDE_DIRS "/usr/include/eblearn")

MESSAGE(STATUS "Found EBLearn: ${EBLearn_LIBS}")
INCLUDE_DIRECTORIES (${EBLearn_INCLUDE_DIRS})
LINK_DIRECTORIES (${EBLearn_LIBRARY_DIRS})

ADD_TORCH_PACKAGE(leblearn "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(leblearn luaT TH eblearn idx eblearntools)
