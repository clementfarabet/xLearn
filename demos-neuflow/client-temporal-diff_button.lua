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
op:add_option{'-c', '--camera', action='store', dest='camidx', 
              help='if source=camera, you can specify the camera index: /dev/videoIDX [default=0]'}
op:add_option{'-t', '--target', action='store', dest='target', 
              help='target to run the code on: neuflow | cpu | cuda (default=cpu)'}
options,args = op:parse_args()

target = options.target or 'cpu'
last_name = target
last_fps = -1
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
   --neuFlow.core:sleep(0.05)

   -- copy result to host
   output = neuFlow:copyToHost(output_dev)   

end neuFlow:endLoop('main')


----------------------------------------------------------------------
-- LOAD: load the bytecode on the device, and execute it
--
--neuFlow:loadBytecode()
bytecode_loaded = false

if target == 'neuflow' then
   neuFlow:loadBytecode()
   bytecode_loaded = true
   neuFlow:copyToDev(input)
   neuFlow:copyToDev(input)
end



function sleep(sec)
   os.execute("sleep 0.05")
end

toolBox.useQT()
widget = qtuiloader.load('client-temp-diff.ui')
painter = qt.QtLuaPainter(widget.frame)

----------------------------------------------------------------------
-- EXEC: this part executes the host code, and interacts with the dev
--

-- preprocess
-- camera and converter
source = nn.ImageSource(options.source or 'camera', nil, options.camidx)
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

-- displayers
disp_input = Displayer()
disp_output = Displayer()

-- if (target == 'neuflow') then
--    -- send dummy input
--    neuFlow:copyToDev(input)
--    neuFlow:copyToDev(input)
-- end
error = torch.Tensor()

-- process loop
count = 0
function process()
   neuFlow.profiler:start('whole-loop','fps')
   count = count + 1

   camFrame = source:forward()
   camFrameY = rgb2y:forward(camFrame):select(3,1)
   if ((count%2) == 0) and (options.source == 'lena') then camFrameY = camFrameY:t() end
   image.scale(camFrameY, input, 'bilinear')

   if (target == 'neuflow') then
      -- execute code on neuFlow:
      neuFlow:copyToDev(input)
      neuFlow:copyFromDev(output)
   else
      neuFlow.profiler:start('compute-cpu')
      neuFlow.profiler:setColor('compute-cpu', 'blue')
      soft_inputs[1]:copy(soft_inputs[2])
      soft_inputs[2]:copy(input)
      output = sub:forward(soft_inputs)
      output:resize(output:size(1), output:size(2), 1)
      --sleep(0.05)
      neuFlow.profiler:lap('compute-cpu')
   end
   
   
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
                   legend='input', globalzoom=1}
   disp_output:show{tensor=output:select(3,1), painter=painter,
                    offset_x=inputsize+2, offset_y=10,
                    legend='output', globalzoom=1,
                    min=0, max=1,}
   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll_only_sec{painter=painter, x=10, y=540, zoom=0.6}
   
   local x = 300
   local y = 540
   local zoom = 0.6
   painter:setfont(qt.QFont{serif=false,italic=false,size=24*zoom})
   
   painter:setcolor("red")
   local str
   str = string.format('compare to <%s> = %f fps',
		       last_name,
		       last_fps)
   
   -- disp line:
   painter:moveto(x,y);
   painter:show(str)

   -- if we have both cpu and neuflow timimgs saved
   if(last_fps ~= -1) then
      
      x = 400
      y = 600
      painter:setfont(qt.QFont{serif=false,italic=false,size=28*zoom})
      painter:setcolor("red")
      
      local speedup = neuFlow.profiler.events['compute-cpu'].reald/neuFlow.profiler.events['on-board-processing'].reald
      str = string.format('speedup = %f ',
			  speedup)
      
      -- disp line:
      painter:moveto(x,y);
      painter:show(str)
   end

   painter:gend()
end

-- Loop Process
local timer = qt.QTimer()
timer.interval = 2
timer.singleShot = true
timer:start()
qt.connect(timer,
           'timeout()', 
           function()
	      process()
              timer:start()
           end)


qt.connect(qt.QtLuaListener(widget.pushButton),
           'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)',
           function (...) 
	      -- only if we are changing the process source,
	      --change the fps
	      if (target ~= 'neuflow') then
		 last_fps = 1/neuFlow.profiler.list[1].reald
	      end
              target = 'neuflow'
	      if not bytecode_loaded then
		 neuFlow:loadBytecode()
		 bytecode_loaded = true
		 
		 -- send dummy input
		 neuFlow:copyToDev(input)
		 neuFlow:copyToDev(input)
	      
	      end

	      last_name = 'cpu'
	      
           end)
qt.connect(qt.QtLuaListener(widget.pushButton_2),
           'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)',
           function (...) 
	      -- only if we are changing the process source,
	      --change the fps
	      if (target ~= 'cpu') then
		 last_fps = 1/neuFlow.profiler.list[1].reald
	      end
              target = 'cpu'
	      last_name = 'neuflow'
	      
           end)

widget.windowTitle = "NeuFlow Demo: Temporal Difference"
widget:show()

