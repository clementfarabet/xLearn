
SET(src bit.c) 
SET(luasrc init.lua)

ADD_TORCH_PACKAGE(bit "${src}" "${luasrc}")

TARGET_LINK_LIBRARIES(bit luaT TH)
