#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- Loopback an image
--
require 'NeuFlow'

----------------------------------------------------------------------
-- INIT: initialize the neuFlow context
-- a mem manager, the dataflow core, and the compiler
--
neuFlow = NeuFlow{prog_name = 'loopback',
                  offset_code = bootloader.entry_point_b,
                  offset_data_1D = bootloader.entry_point_b + 320*kB,
                  offset_data_2D = bootloader.entry_point_b + 360*kB,
                  offset_heap = bootloader.entry_point_b + 550*kB,
                  global_msg_level = 'none',
                  compiler_msg_level = 'none',
                  core_msg_level = 'none',
                  interface_msg_level = 'none'}


----------------------------------------------------------------------
-- ELABORATION: describe the algorithm to be run on neuFlow, and 
-- how it should interact with the host (data exchange)
-- note: any copy**Host() inserted here needs to be matched by
-- a copy**Dev() in the EXEC section.
--

-- input data
inputsize = 500
input = torch.Tensor(inputsize,inputsize,3)
image.scale(image.lena(), input, 'bilinear')

-- loop over the main code
neuFlow:beginLoop('main') do

   -- send data to device
   input_dev = neuFlow:copyFromHost(input)

   -- get it back
   outputs = neuFlow:copyToHost(input_dev)

end neuFlow:endLoop('main')


----------------------------------------------------------------------
-- LOAD: load the bytecode on the device, and execute it
--
neuFlow:loadBytecode()


----------------------------------------------------------------------
-- EXEC: this part executes the host code, and interacts with the dev
--

-- window
painter = qtwidget.newwindow(1200,700)

-- displayers
disp_input = Displayer()
disp_error = Displayer()
disp_outputs = Displayer()

-- error
error = torch.Tensor()

-- process loop
while true do
   neuFlow.profiler:start('whole-loop','fps')

   -- this is the actual code running on the device...
   neuFlow:copyToDev(input)
   neuFlow:copyFromDev(outputs)

   neuFlow.profiler:start('compute-error')
   error:resizeAs(outputs):copy(outputs):add(-1,input):abs()
   neuFlow.profiler:lap('compute-error')
   
   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_input:show{tensor=input, painter=painter, offset_x=1, offset_y=10,
                   min=0, max=1,
                   legend='original', globalzoom=1}
   disp_outputs:show{tensor=outputs, painter=painter,
                     offset_x=inputsize+2, offset_y=10,
                     legend='from neuFlow', globalzoom=1,
                     min=0, max=1,}
   disp_error:show{tensor=error, painter=painter,
                   offset_x=inputsize*2+3, offset_y=10,
                   min=0, max=1,
                   legend='error map', globalzoom=1, min=-1, max=1}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}
   painter:gend()
end
