
SET(src)

SET(luasrc init.lua 
           Array.lua  
           Node.lua)

ADD_TORCH_PACKAGE(luaFlow "${src}" "${luasrc}")
