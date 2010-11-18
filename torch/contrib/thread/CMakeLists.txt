SET(src ) 
SET(luasrc init.lua queue.lua) 

ADD_TORCH_PACKAGE(thread "${src}" "${luasrc}")
