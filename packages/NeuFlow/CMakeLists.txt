
SET(src )

FILE(GLOB MyCSources segments/coef*)

SET(luasrc defines.lua
           defines_ibm_asic.lua
           defines_xilinx_ml605.lua
           defines_pico_m503.lua
           tools.lua
           rom.lua
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

ADD_TORCH_PACKAGE(NeuFlow "${src}" "${luasrc}" "Hardware") 
