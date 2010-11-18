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
inputsize = 30
input = torch.Tensor(inputsize,inputsize)
divider = torch.Tensor(inputsize,inputsize)
output = torch.Tensor(inputsize,inputsize)


-- global loop
neuFlow:beginLoop('main') do

   -- copy input image to dev (in simul, the data is embedded)
   -- and alloc output
   input_dev = neuFlow:copyFromHost(input)
   divider_dev = neuFlow:copyFromHost(divider)
   output_dev = neuFlow:allocHeap(output)

   -- convolve all chanels by kernel
   neuFlow.core:divide(input_dev[1], divider_dev[1], output_dev[1])
   
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
-- camFrame = image.lena()
-- camFrameY = image.rgb2y(camFrame):select(3,1)
-- image.scale(camFrameY, input, 'bilinear')
M = torch.Tensor(inputsize,inputsize)
R = torch.Tensor(inputsize,inputsize)
input:fill(8)--(-10)--(0.10)--
input:narrow(1,10, 10):narrow(2, 10,10):fill(0)
divider:fill(-4)--(-5)--(0.05)--

print('INPUT:')
print(input:narrow(1,7,14):narrow(2,7,14))
print('DIVIDER:')
print(divider:narrow(1,7,14):narrow(2,7,14))

-- window
painter = qtwidget.newwindow(1200,700)

-- displayers
disp_input = Displayer()
disp_output = Displayer()
disp_groundtruth = Displayer()

-- offline version
groundtruth =  torch.Tensor(inputsize,inputsize, 1)

for i = 1, groundtruth:size(1) do
   for j = 1, groundtruth:size(2) do
      groundtruth[i][j][1] = input[i][j] / divider[i][j]
   end
end


-- process loop
while true do
   neuFlow.profiler:start('whole-loop','fps')
      
   print('INPUT:')
   print(input:narrow(1,7,14):narrow(2,7,14))
   print('DIVIDER:')
   print(divider:narrow(1,7,14):narrow(2,7,14))
   print('OUTPUT SOFTWARE:')
   print(groundtruth:select(3,1):narrow(1,7,14):narrow(2,7,14))

   -- execute code on neuFlow:
   neuFlow:copyToDev(input)
   neuFlow:copyToDev(divider)
   neuFlow:copyFromDev(output)

   print('OUTPUT HARDWARE:')
   print(output:select(3,1):narrow(1,7,14):narrow(2,7,14))
   
   
   --print('max error: '..(groundtruth-output):abs():max())
   --print('std '..normer.inStdDev:mean())
 
   for i=1,output:size(1) do
      for j=1,output:size(2) do
	 M[i][j] = math.max(math.abs(output[i][j][1]), math.abs(groundtruth[i][j][1]))
      end
   end
   for i=1,output:size(1) do
      for j=1,output:size(2) do
	 if(M[i][j] < 10^-6) then
	 --if(groundtruth[i][j][1] < 10^-6) then
	    R[i][j] = 0
	 else R[i][j] = math.abs((output[i][j][1] - groundtruth[i][j][1])/M[i][j])
	 end
      end
   end
   print('ERROR:')
   print(R:narrow(1,7,14):narrow(2,7,14))


   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_input:show{tensor=input, painter=painter, offset_x=1, offset_y=10,
                   min=0, max=1,
                   legend='original', globalzoom=10}
   disp_output:show{tensor=output, painter=painter,
                    offset_x=inputsize+2, offset_y=10,
                    legend='from neuFlow', globalzoom=10,
                    min=-1, max=1}
   disp_groundtruth:show{tensor=groundtruth, 
                         painter=painter,
                         offset_x=inputsize*2+4, offset_y=10,
                         legend='from torch', globalzoom=10,
                         min=-1, max=1}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}
   painter:gend()
end
