
require 'XLearn'
torch.include('debugger', 'debugger.lua')

-- setting this var is useful for conditional debugging
_DEBUG_ = true

print("INFO: running Lua in debug mode (global variable _DEBUG_ set to true)")
print("> to insert a breakpoint, use the snippet:")
print(toolBox.COLORS.blue .. "if _DEBUG_ then pause() end" .. toolBox.COLORS.none)
