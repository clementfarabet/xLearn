#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- Face Detection
--
require 'NeuFlow'

----------------------------------------------------------------------
-- INIT: initialize the neuFlow context
-- a mem manager, the dataflow core, and the compiler
--
neuFlow = NeuFlow{prog_name           = 'debug-norm',
                  offset_code         = bootloader.entry_point_b,
                  offset_data_1D      = bootloader.entry_point_b + 320*kB,
                  offset_data_2D      = bootloader.entry_point_b + 360*kB,
                  offset_heap         = bootloader.entry_point_b + 550*kB,
                  global_msg_level    = 'none',
                  compiler_msg_level  = 'none',
                  core_msg_level      = 'none',
                  interface_msg_level = 'none'}


----------------------------------------------------------------------
-- ELABORATION: describe the algorithm to be run on neuFlow, and 
-- how it should interact with the host (data exchange)
-- note: any copy**Host() inserted here needs to be matched by
-- a copy**Dev() in the EXEC section.
--

-- input image
inputsize = 400
input = torch.Tensor(inputsize,inputsize,3)
kernel = torch.Tensor(7,7):fill(1)
output = torch.Tensor(inputsize,inputsize)
image_zero_mean = torch.Tensor(inputsize,inputsize)

normer = nn.Sequential()
normer:add(nn.LocalNorm(kernel,3))

-- global loop
neuFlow:beginLoop('main') do

   -- copy input image to dev
   input_dev = neuFlow:copyFromHost(input)

   -- convolve all chanels by kernel
   output_dev = neuFlow:compile(normer, input_dev)

   -- copy result to host
   output = neuFlow:copyToHost(output_dev)

end neuFlow:endLoop('main')


----------------------------------------------------------------------
-- LOAD: load the bytecode on the device, and execute it
--
neuFlow:loadBytecode()


----------------------------------------------------------------------
-- EXEC: this part executes the host code, and interacts with the dev
--

-- preprocess
camFrame = image.lena()
image.scale(camFrame, input, 'bilinear')

-- window
painter = qtwidget.newwindow(1200,700)

-- displayers
disp_input = Displayer()
disp_output = Displayer()
disp_groundtruth = Displayer()

-- offline version
normer = nn.ContrastNormalization(kernel,3)
groundtruth = normer:forward(input)

-- process loop
while true do
   neuFlow.profiler:start('whole-loop','fps')

   -- execute code on neuFlow:
   neuFlow:copyToDev(input)
   neuFlow:copyFromDev(output)

   print('max error: '..(groundtruth-output):abs():max())
   print('global std = threshold '..normer.thresmod.threshold)

   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_input:show{tensor=input, painter=painter, offset_x=1, offset_y=10,
                   min=0, max=1,
                   legend='original', globalzoom=1}
   disp_output:show{tensor=output, painter=painter,
                    offset_x=inputsize+2, offset_y=10,
                    legend='from neuFlow', globalzoom=1,
                    min=-1, max=1}
   disp_groundtruth:show{tensor=normer.output, 
                         painter=painter,
                         offset_x=inputsize*2+4, offset_y=10,
                         legend='from torch', globalzoom=1,
                         min=-1, max=1}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}
   painter:gend()
end
