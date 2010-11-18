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
neuFlow = NeuFlow{prog_name           = 'debug-convacc',
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
kernel = image.gaussian{width=7,normalize=true}
output = torch.Tensor(inputsize-6,inputsize-6,3)

-- global loop
neuFlow:beginLoop('main') do

   -- copy input image to dev (in simul, the data is embedded)
   -- and alloc output, and kernel
   input_dev = neuFlow:copyFromHost(input)
   kernel_dev = neuFlow:allocDataPacked(kernel)
   output_dev = neuFlow:allocHeap(output)

   -- this data is gonna be accumulated over, so it's not allocated
   -- separately (shitty hack)
   input_acc = {x=input_dev[2].x, y=input_dev[2].y,
                w=input_dev[2].w-6, h=input_dev[2].h-6}

   -- coefs for mapper: tanh()
   coefs=math.approx{mapping=math.tanh, min=-5, max=5, odd=true,
                  nbSegments=grid.mapper_segs, Q=num.frac_, name = 'Tanh',
                     verbose=true}

   -- convolve all chanels by kernel
   for i = 1,output:size(3) do
      neuFlow.core:convolveAndAccAndMap(input_dev[i], kernel_dev[1], input_acc, output_dev[i], coefs)
   end

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

-- groundtruth
ground_output = torch.Tensor():resizeAs(output)
for i = 1,output:size(3) do
   ground_output:select(3,i):copy(input:select(3,2):narrow(1,1,inputsize-6):narrow(2,1,inputsize-6))
   image.convolveAndAcc(input:select(3,i), kernel, ground_output:select(3,i))
end
ground_output:tanh()
error = torch.Tensor():resizeAs(ground_output)

-- window
painter = qtwidget.newwindow(1200,700)

-- displayers
disp_input = Displayer()
disp_output = Displayer()
disp_error = Displayer()

-- process loop
while true do
   neuFlow.profiler:start('whole-loop','fps')

   -- execute code on neuFlow:
   neuFlow:copyToDev(input)
   neuFlow:copyFromDev(output)
   
   -- compute error
   error:copy(ground_output):mul(-1):add(output):abs()
   print('max error is '.. error:max())

   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_input:show{tensor=input:select(3,2), painter=painter, offset_x=1, offset_y=10,
                   min=0, max=1,
                   legend='original', globalzoom=1}
   disp_output:show{tensor=output:select(3,2), painter=painter,
                    offset_x=inputsize+2, offset_y=10,
                    legend='from neuFlow', globalzoom=1,
                    min=0, max=1,}
   disp_error:show{tensor=error, painter=painter,
                   offset_x=2*inputsize+4, offset_y=10,
                   legend='error', globalzoom=1,
                   min=0, max=1}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}
   painter:gend()
end
