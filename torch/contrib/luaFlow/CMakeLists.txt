
SET(src)

SET(luasrc init.lua 
           luaflow.lua  
           neuflow.lua
           cudaflow.lua)

ADD_TORCH_PACKAGE(luaFlow "${src}" "${luasrc}")
