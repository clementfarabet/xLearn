#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- Face Detection
--
require 'NeuFlow'

-- debug
bypass_neuflow = false

------------------------------------------------------------
-- parse args
--
op = OptionParser('%prog [options]')
op:add_option{'-s', '--source', action='store', dest='source', 
              help='image source, can be one of: camera | lena'}
op:add_option{'-d', '--debug', action='store', dest='tty', 
              help='tty device [to dump messages from neuFlow]'}
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
neuFlow = NeuFlow{prog_name           = 'face-detect',
                  offset_code         = bootloader.entry_point_b,
                  offset_data_1D      = bootloader.entry_point_b + 5*MB,
                  offset_data_2D      = bootloader.entry_point_b + 10*MB,
                  offset_heap         = bootloader.entry_point_b + 15*MB,
                  serial_device       = options.tty,
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

-- load Face Detector ConvNet and drop its last layer (classifier is done in software)
convnet = nn.Sequential()
convnet:read(torch.DiskFile('../trained-nets/network-face-detect', 'r'))
normer = convnet.modules[ 1 ]
classifier = convnet.modules[ #convnet.modules ]
newconvnet = nn.Sequential()
for i = 2,#convnet.modules-1 do
   newconvnet:add(convnet.modules[i])
end
convnet = newconvnet

-- camera and converter
source = nn.ImageSource(options.source or 'camera')
rgb2y = nn.ImageTransform('rgb2y')

-- multiscale packer
scales = {1/2.8, 1/4, 1/5, 1/7, 1/10, 1/12}
packer = nn.PyramidPacker(scales, convnet)
unpacker = nn.PyramidUnPacker(convnet)

-- dummy fprop to get sizes
camFrame = source:forward()
camFrameY = rgb2y:forward(camFrame)
pyramid = packer:forward(camFrameY)
normed = normer:forward(pyramid)

convnet:forward(normed)


-- resize input
input = torch.Tensor(pyramid:size(1),pyramid:size(2),pyramid:size(3))

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
--neuFlow:loadBytecode()
bytecode_loaded = false

if target == 'neuflow' then
   neuFlow:loadBytecode()
   bytecode_loaded = true
end


----------------------------------------------------------------------
-- EXEC: this part executes the host code, and interacts with the dev
--

-- setup GUI (external UI file)
toolBox.useQT()
widget = qtuiloader.load('client-facedetect_button.ui')
painter = qt.QtLuaPainter(widget.frame)
--painter = qtwidget.newwindow(1200,800)

-- displayers
disp_input = Displayer()
disp_maps = Displayer()
disp_error = Displayer()
disp_result = Displayer()

-- error map
error = torch.Tensor()
--rescaler = nn.ImageRescale(300,200,1)
camSmallFrame = torch.Tensor(300,300,1)
camSmallFrame1 = torch.Tensor(300,300,1)
-- loop
loop = 1
function process()
   neuFlow.profiler:start('GLOBAL','fps')
   neuFlow.profiler:start('get-image')
   camFrame = source:forward()
   camFrameY = rgb2y:forward(camFrame)
   neuFlow.profiler:lap('get-image')

   neuFlow.profiler:start('pyramid-pack')
   pyramid, coordinates = packer:forward(camFrameY)
   neuFlow.profiler:lap('pyramid-pack')
   
   neuFlow.profiler:start('normalize')
   input = normer:forward(pyramid)
   neuFlow.profiler:lap('normalize')
   
  -- if not bypass_neuflow then 
   if (target == 'neuflow') then
      -- execute code on neuFlow:
      neuFlow:copyToDev(input)
      neuFlow:copyFromDev(outputs)
      
      neuFlow.profiler:start('classify')
      packed_result = classifier:forward(outputs)
      neuFlow.profiler:lap('classify')
   else
      profiler_cpu = neuFlow.profiler:start('compute-cpu')
      neuFlow.profiler:setColor('compute-cpu', 'blue')
      packed_result = convnet:forward(input)
      outputs = convnet.output
      packed_result = classifier:forward(outputs)
      neuFlow.profiler:lap('compute-cpu')
   end
   
   if options.source == 'lena' then
      neuFlow.profiler:start('error')
      error:resizeAs(outputs):copy(convnet.output):mul(-1):add(outputs):abs()
      print('max error = '..error:max())
      neuFlow.profiler:lap('error')
   end

   neuFlow.profiler:start('pyramid-unpack')
   result_hw = unpacker:forward(packed_result, coordinates)
   neuFlow.profiler:lap('pyramid-unpack')
   
   neuFlow.profiler:start('find-blobs')
   listOfFaces = nil
   for i = 1,#result_hw do
      listOfFaces = image.findBlobs{tensor=result_hw[i], threshold=-0.7, 
                                    maxsize=10, discardClass=2, scale=scales[i],
                                    labels={"face"},
                                    listOfBlobs=listOfFaces}
   end
   neuFlow.profiler:lap('find-blobs')
   
   neuFlow.profiler:start('order-blobs')
   listOfFaces = image.reorderBlobs(listOfFaces)
   listOfFaces = image.remapBlobs(listOfFaces)
   listOfFaces = image.mergeBlobs(listOfFaces, 50)
   neuFlow.profiler:lap('order-blobs')
   
   neuFlow.profiler:start('display')
   painter:gbegin()
   painter:showpage()

   --camSmallFrame = rescaler:forward(camFrameY)
   --image.scale(camFrameY, camSmallFrame, 'bilinear')
   --camSmallFrame1:resizeAs(camFrameY):copy(camFrameY)
   --image.scale(camSmallFrame1, camSmallFrame, 'bilinear')
   -- display input image
   disp_input:show{tensor=camFrameY, painter=painter, offset_x=0, offset_y=0,
                   min=0, max=1,
                   inplace=true,
                   legend='original', globalzoom=0.6}
   -- and pyramid
   if widget.checkBox1.checked then
      disp_input:show{tensor=input, painter=painter, 
		      min=-1, max=1,
		      inplace=true,
		      legend='original', globalzoom=0.5}
   end
   -- print boxes
   local i = 1
   local done = 0
   if listOfFaces then
      while true do
         if listOfFaces[i] then
            image.qtdrawbox{ painter=painter,
                             x=listOfFaces[i].x * 4,
                             y=listOfFaces[i].y * 4,
                             w=32/listOfFaces[i].scale,
                             h=32/listOfFaces[i].scale,
                             globalzoom=0.6,
                             legend=listOfFaces[i].tag}
            done = done + 1
         end
         i = i + 1
         if (done == listOfFaces.nbOfBlobs) then
            break
         end
      end
   end
   
   -- maps
   if widget.checkBox1.checked then
      local j=1
      for k = 1,4 do
	 for i = 1,outputs:size(3)/4 do
	    disp_maps:show{tensor=outputs:select(3,j), painter=painter, 
			   offset_x=680+(k-1)*(outputs:size(1)+1), 
			   offset_y=(i-1)*(outputs:size(2)+1),
			   min=0, max=1,
			   inplace=true,
			   globalzoom=1, min=-1, max=1}
	    j=j+1
	 end
      end
   end
   
   if options.source == 'lena' then
      -- error maps
      local j=1
      for k = 1,4 do
         for i = 1,outputs:size(3)/4 do
            disp_error:show{tensor=error:select(3,j), painter=painter, 
                            offset_x=680+(k-1)*(outputs:size(1)+1), 
                            offset_y=400+(i-1)*(outputs:size(2)+1),
                            min=0, max=1,
                            inplace=true,
                            globalzoom=1, min=0, max=1}
            j=j+1
         end
      end
   end
   
   -- and display result
   if widget.checkBox1.checked then
      local step = 40
      for i=1,#result_hw do
	 disp_result:show{ tensor=result_hw[i]:select(3,1), painter=painter, 
			   globalzoom=zoom,
			   zoom = 6,
			   inplace=true,
			   min=-1, max=1,
			   offset_x=700+4*(outputs:size(1)+1), 
			   offset_y=step,
			   legend='Scaled Outputs #'..i}
	 step = result_hw[i]:size(2)*8 + step + 40
      end
   end
   neuFlow.profiler:lap('display')
   neuFlow.profiler:lap('GLOBAL')
   neuFlow.profiler:displayAll_only_sec{painter=painter, x=10, y=350, zoom=0.7}
   
   local x = 320
   local y = 350
   local zoom = 0.7
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

      x = 480
      y = 500
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

widget.windowTitle = "NeuFlow Demo: Face Detect"
widget:show()

-- cleanup
--neuFlow:cleanup()
