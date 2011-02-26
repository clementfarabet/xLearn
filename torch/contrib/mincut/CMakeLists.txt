 SET(src graph.cpp  segmentation.cpp libmincut.c) 
 SET(luasrc init.lua) 

 ADD_TORCH_PACKAGE(mincut "${src}" "${luasrc}")
 TARGET_LINK_LIBRARIES(mincut luaT TH)
