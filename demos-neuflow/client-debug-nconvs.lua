#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- Face Detection
--
require 'NeuFlow'

------------------------------------------------------------
-- parse args
--
op = OptionParser('%prog [options]')
op:add_option{'-s', '--source', action='store', dest='source', 
              help='image source, can be one of: camera | lena'}
options,args = op:parse_args()


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
inputsize = 300
nins = 1
nouts = 16
input = torch.Tensor(inputsize,inputsize,nins)

-- 16 convolutions
convnet = nn.Sequential()
convnet:add(nn.GaborLayer(4,4,4).bank)

-- loop over the main code
neuFlow:beginLoop('main') do

   -- copy input image to dev (in simul, the data is embedded)
   input_dev = neuFlow:copyFromHost(input)

   -- compile network
   outputs_dev = neuFlow:compile(convnet, input_dev)

   -- ship results back to the host
   outputs = neuFlow:copyToHost(outputs_dev)

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
for i=1,input:size(3) do
   image.scale(camFrame:select(3,2), input:select(3,i), 'bilinear')
end

-- window
painter = qtwidget.newwindow(1200,700)

-- camera and converter
source = nn.ImageSource(options.source or 'camera')
rgb2y = nn.ImageTransform('rgb2y')

-- displayers
disp_input = Displayer()
disp_output = Displayer()

-- process loop
while true do
   neuFlow.profiler:start('whole-loop','fps')

   camFrame = source:forward()
   for i=1,input:size(3) do
      image.scale(camFrame:select(3,2), input:select(3,i), 'bilinear')
   end

   -- execute code on neuFlow:
   neuFlow:copyToDev(input)
   neuFlow:copyFromDev(outputs)
   
   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_input:show{tensor=input:select(3,1), painter=painter, offset_x=1, offset_y=10,
                   min=0, max=1,
                   legend='original', globalzoom=1}
   local k
   for i=0,3 do
      for j=0,3 do
         k = 1+i*4+j
         if k > outputs:size(3) then
            break
         end
         if k==1 then
            legend = 'from neuFlow'
         else
            legend = ''
         end
         disp_output:show{tensor=outputs:select(3,k), painter=painter,
                          offset_x=inputsize*1.5 + 0.5*outputs:size(1)*i, 
                          offset_y=10 + 0.5*outputs:size(2)*j,
                          legend=legend, 
                          globalzoom=1, zoom=0.5,
                          min=-0.5, max=0.5}
      end
      if k > outputs:size(3) then
         break
      end
   end
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}
   painter:gend()
end
