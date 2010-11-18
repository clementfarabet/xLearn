#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- LAGR / Obstacle Classification
--
require 'NeuFlow'

-- debug
bypass_neuflow = false

----------------------------------------------------------------------
-- INIT: initialize the neuFlow context
-- a mem manager, the dataflow core, and the compiler
--
neuFlow = NeuFlow{prog_name           = 'norb',
                  offset_code         = bootloader.entry_point_b,
                  offset_data_1D      = bootloader.entry_point_b + 1*MB,
                  offset_data_2D      = bootloader.entry_point_b + 1.5*MB,
                  offset_heap         = bootloader.entry_point_b + 2*MB,
                  nb_convs            = 4,
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

-- Load LAGR convnet 
convnet = nn.Sequential()
file = torch.DiskFile('../trained-nets/network-lagr-1.0')
convnet:read(file)

-- Separate convnet from classifier
convnet_noclassifier = nn.Sequential()
for i=1,12 do
   convnet_noclassifier:add(convnet.modules[i])
end
classifier = nn.Sequential()
for i=13,16 do
   classifier:add(convnet.modules[i])
end

-- load ground truth data
groundtruthpath = '../../ext-xlearn/datasets/lagr_groundtruth/'
grndinputs = toolBox.loadIDX(groundtruthpath..'pyramids-input-5x3x156x505.mat'):narrow(2,1,96)
grndoutputs = toolBox.loadIDX(groundtruthpath..'pyramids-output-5x100x145x121.mat')
input_height = grndinputs:size(2)
input_width = grndinputs:size(1)
input = grndinputs:select(4,1)

-- loop over the main code
neuFlow:beginLoop('main') do
   
   -- copy input image to dev (in simul, the data is embedded)
   input_dev = neuFlow:copyFromHost(input)

   -- compile network
   output_dev = neuFlow:compile(convnet_noclassifier, input_dev)

   -- ship results back to the host
   output = neuFlow:copyToHost(output_dev)

end neuFlow:endLoop('main')


----------------------------------------------------------------------
-- LOAD: load the bytecode on the device, and execute it
--
neuFlow:loadBytecode()


----------------------------------------------------------------------
-- EXEC: this part compiles the code and executes it
--

-- displayers
disp_input = Displayer()
disp_maps = Displayer()

-- receiving 34x34 images
toreceive = torch.Tensor()
error = torch.Tensor()

-- camera and converter
source = nn.ImageSource('camera','opencv')
rgb2yuv = nn.ImageTransform('rgb2y')

-- window
painter = qtwidget.newwindow(1200,700)

-- dummy fprop for sizes
result_ground = {}
for i = 1,grndinputs:size(4) do
   local temp = convnet_noclassifier:forward(grndinputs:select(4,i))
   result_ground[i] = torch.Tensor():resizeAs(temp):copy(temp)
end
print(output:size())
-- loop
inputn = 1
while true do
   neuFlow.profiler:start('GLOBAL','fps')

   -- execute code on neuFlow:
   neuFlow:copyToDev(grndinputs:select(4,inputn))
   neuFlow:copyFromDev(output)

   -- compute error
   neuFlow.profiler:start('error')
   error:resizeAs(output):copy(result_ground[inputn]):add(-1, output):abs()
   print('max error is ' .. error:max())
   neuFlow.profiler:lap('error')

   neuFlow.profiler:start('display')
   painter:gbegin()
   painter:showpage()

   -- display input image
   for i = 1,3 do
      disp_input:show{tensor=grndinputs:select(4,inputn):select(3,i),
                      painter=painter, offset_x=0, offset_y=grndinputs:size(2)*(i-1),
                      min=0, max=1,
                      inplace=true,
                      legend='original', globalzoom=0.5}
   end

   -- maps
   local j=1
   for k = 1,4 do
      for i = 1,4 do
         disp_maps:show{tensor=output:select(3,j), painter=painter, 
                        offset_x=input_width+(k-1)*(output:size(1)+1), 
                        offset_y=(i-1)*(output:size(2)+1),
                        min=0, max=1,
                        inplace=true,
                        globalzoom=0.5, min=-1, max=1}
         disp_maps:show{tensor=result_ground[inputn]:select(3,j), painter=painter, 
                        offset_x=input_width*2+(k-1)*(output:size(1)+1), 
                        offset_y=(i-1)*(output:size(2)+1),
                        min=0, max=1,
                        inplace=true,
                        globalzoom=0.5, min=-1, max=1}
         disp_maps:show{tensor=error:select(3,j), painter=painter, 
                        offset_x=input_width*3+(k-1)*(output:size(1)+1), 
                        offset_y=(i-1)*(output:size(2)+1),
                        min=0, max=1,
                        inplace=true,
                        globalzoom=0.5, min=0, max=1}
         j=j+1
      end
   end

   neuFlow.profiler:lap('display')
   neuFlow.profiler:lap('GLOBAL')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=500, zoom=0.5}
   painter:gend()

   if inputn == grndinputs:size(4) then inputn = 1
   --else inputn = inputn + 1 
   end
end

-- cleanup
neuFlow:cleanup()
