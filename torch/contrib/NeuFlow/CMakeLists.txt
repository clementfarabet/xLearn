
SET(src )


FILE(GLOB MyCSources segments/*)

SET(luasrc defines.lua
           tools.lua
           Log.lua
           Memory.lua
           Compiler.lua
           Interface.lua
           Core.lua
           CoreUser.lua
           Linker.lua
           NeuFlow.lua
	   Serial.lua
	   init.lua
	   ${MyCSources})



ADD_TORCH_PACKAGE(NeuFlow "${src}" "${luasrc}") 


TARGET_LINK_LIBRARIES(NeuFlow)
