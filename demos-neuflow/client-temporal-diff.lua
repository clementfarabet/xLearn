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
neuFlow = NeuFlow{prog_name           = 'debug-conv',
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
input = torch.Tensor(inputsize,inputsize)

-- subtracter module
sub = nn.Sequential()
sub:add(nn.CCSub())
--sub:add(nn.Tanh())
--sub:add(nn.AbsModule())


-- copy first image
input_dev_a = neuFlow:copyFromHost(input)
input_dev_b = neuFlow:copyFromHost(input)

-- global loop
neuFlow:beginLoop('main') do

   -- copy input image to dev
   neuFlow:copy(input_dev_a, input_dev_b)
   neuFlow:copyFromHost(input, input_dev_a)

   -- do the diff
   output_dev = neuFlow:compile(sub, {input_dev_a[1], input_dev_b[1]})

   -- sleep
   neuFlow.core:sleep(0.1)

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
-- camera and converter
source = nn.ImageSource(options.source or 'camera')
rgb2y = nn.ImageTransform('rgb2y')


soft_inputs = {}
soft_in1 = torch.Tensor(inputsize,inputsize)
soft_in2 = torch.Tensor(inputsize,inputsize)

soft_in1_raw = source:forward()
soft_in1Y = rgb2y:forward(soft_in1_raw):select(3,1)
image.scale(soft_in1Y, soft_in1, 'bilinear')

soft_in2_raw = source:forward()
soft_in2Y = rgb2y:forward(soft_in2_raw):select(3,1)
if options.source == 'lena' then soft_in2Y = soft_in2Y:t() end
image.scale(soft_in2Y, soft_in2, 'bilinear')

soft_inputs[1] = soft_in1
soft_inputs[2] = soft_in2

soft_out = sub:forward(soft_inputs)

-- window
painter = qtwidget.newwindow(1200,700)

-- displayers
disp_input = Displayer()
disp_output = Displayer()

-- send dummy input
neuFlow:copyToDev(input)
neuFlow:copyToDev(input)

error = torch.Tensor()

-- process loop
count = 0
while true do
   neuFlow.profiler:start('whole-loop','fps')
   count = count + 1

   camFrame = source:forward()
   camFrameY = rgb2y:forward(camFrame):select(3,1)
   if ((count%2) == 0) and (options.source == 'lena') then camFrameY = camFrameY:t() end
   image.scale(camFrameY, input, 'bilinear')

   -- execute code on neuFlow:
   neuFlow:copyToDev(input)
   neuFlow:copyFromDev(output)
   
   
   if options.source == 'lena' then
      neuFlow.profiler:start('error')
      error:resizeAs(output):copy(soft_out):mul(-1):add(output):abs()
      print('max error = '..error:max())
      neuFlow.profiler:lap('error')
   end


   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_input:show{tensor=input, painter=painter, offset_x=1, offset_y=10,
                   min=0, max=1,
                   legend='original', globalzoom=1}
   disp_output:show{tensor=output:select(3,1), painter=painter,
                    offset_x=inputsize+2, offset_y=10,
                    legend='from neuFlow', globalzoom=1,
                    min=0, max=1,}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=540, zoom=0.5}
   painter:gend()
end
