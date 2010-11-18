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
neuFlow = NeuFlow{prog_name           = 'debug-conv',
                  offset_code         = bootloader.entry_point_b,
                  offset_data_1D      = bootloader.entry_point_b + 320*kB,
                  offset_data_2D      = bootloader.entry_point_b + 360*kB,
                  offset_heap         = bootloader.entry_point_b + 550*kB,
                  nb_convs            = 3,
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
kersize = 10
inputsize = 501
input = torch.Tensor(inputsize,inputsize,3)
kernel1 = image.gaussian{width=kersize,normalize=true}
kernel2 = image.laplacian{width=kersize,normalize=true}:mul(-1)
bias = torch.Tensor(1):fill(-1)
output = torch.Tensor((inputsize-kersize)+1,(inputsize-kersize)+1,3)

-- to compute GOP/s
ops = 0

-- global loop
neuFlow:beginLoop('main') do

   -- copy input image to dev (in simul, the data is embedded)
   -- and alloc output, and kernel
   input_dev = neuFlow:copyFromHost(input)
   kernel1_dev = neuFlow:allocDataPacked(kernel1, bias)
   kernel2_dev = neuFlow:allocDataPacked(kernel2)
   kernel_dev = {kernel1_dev[1], kernel2_dev[1]}
   output_dev = neuFlow:allocHeap(output)

   -- convolve all chanels by kernel
   neuFlow.core:convolBank({input_dev[1], input_dev[2], input_dev[2], input_dev[2],
                            input_dev[2], input_dev[2], input_dev[2], input_dev[2],
                            input_dev[2], input_dev[2], input_dev[2], input_dev[2]},
                           {kernel_dev[1], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[2], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[2], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[1], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[2], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[2], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[1], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[2], kernel_dev[2], kernel_dev[2], kernel_dev[2],
                            kernel_dev[2], kernel_dev[2], kernel_dev[2], kernel_dev[2]},
                           {output_dev[1],output_dev[2],output_dev[3]},
                           math.approx{name='Tanh'})
   -- ops
   ops = ops + output:size(1)*output:size(2)*kernel1:size(1)*kernel1:size(2)*2*12*3

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
   ground_output:select(3,i):copy( image.convolve(input:select(3,1), kernel1) ):add( bias[1] )
   for j = 1,12-1 do
      image.convolveAndAcc(input:select(3,2), kernel2, ground_output:select(3,i))
   end
end
ground_output:tanh()
error = torch.Tensor():resizeAs(ground_output)

-- window
painter = qtwidget.newwindow(1440,800)

-- displayers
disp = table.generate(4, Displayer)

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
   disp[1]:show{tensor=input, painter=painter, offset_x=1, offset_y=10,
                min=0, max=1,
                legend='original', globalzoom=1}
   disp[2]:show{tensor=ground_output, painter=painter,
                offset_x=inputsize+4, offset_y=10,
                legend='groundtruth', globalzoom=1,
                min=0, max=1}
   disp[3]:show{tensor=output, painter=painter,
                offset_x=2*(inputsize+4), offset_y=10,
                legend='from neuFlow', globalzoom=1,
                min=0, max=1}
   disp[4]:show{tensor=error, painter=painter,
                offset_x=3*(inputsize+4), offset_y=10,
                legend='error', globalzoom=1,
                min=0, max=1}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}

   -- GOP/s
   painter:setcolor('red')
   modelTime = neuFlow.profiler.events['on-board-processing'].reald
   gops = (ops / modelTime) / 1e9
   painter:setfont(qt.QFont{serif=false,italic=false,bold=true,size=14})
   painter:moveto(700,540) painter:show(tostring(gops) .. ' GOP/s')
   painter:gend()
end
