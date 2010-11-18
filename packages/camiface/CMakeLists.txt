SET(src camiface.c) 
SET(luasrc camera.lua init.lua) 

ADD_TORCH_PACKAGE(camiface "${src}" "${luasrc}")

TARGET_LINK_LIBRARIES(camiface luaT TH)
