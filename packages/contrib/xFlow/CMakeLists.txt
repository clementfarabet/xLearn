
SET(src) 
SET(luasrc init.lua 
           engine_0.2.lua 
           engine_0.1.lua 
           lib-math.lua 
           lib-neural.lua)

ADD_TORCH_PACKAGE(xFlow "${src}" "${luasrc}")
