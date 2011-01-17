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
op:add_option{'-c', '--camera', action='store', dest='camidx', 
              help='if source=camera, you can specify the camera index: /dev/videoIDX [default=0]'}
op:add_option{'-d', '--debug', action='store', dest='tty', 
              help='tty device [to dump messages from neuFlow]'}
options,args = op:parse_args()


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
source = nn.ImageSource(options.source or 'camera', nil, options.camidx)
rgb2y = nn.ImageTransform('rgb2y')
rgb2hsl = nn.ImageTransform('rgb2hsl')

-- multiscale packer
scales = {1/2.8, 1/4, 1/5, 1/7, 1/10, 1/12}
packer = nn.PyramidPacker(scales, convnet)
unpacker = nn.PyramidUnPacker(convnet)

-- dummy fprop to get sizes
camFrame = source:forward()
camFrameY = rgb2y:forward(camFrame)
pyramid = packer:forward(camFrameY)
normed = normer:forward(pyramid)
neuFlow.profiler:start('offline-convnet')
convnet:forward(normed)
neuFlow.profiler:lap('offline-convnet')

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
neuFlow:loadBytecode()


----------------------------------------------------------------------
-- EXEC: this part executes the host code, and interacts with the dev
--

-- window
painter = qtwidget.newwindow(1200,800)

-- displayers
disp_input = Displayer()
disp_maps = Displayer()
disp_error = Displayer()
disp_result = Displayer()

-- error map
error = torch.Tensor()

-- loop
loop = 1
while true do
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

   if not bypass_neuflow then      
      -- execute code on neuFlow:
      neuFlow:copyToDev(input)
      neuFlow:copyFromDev(outputs)
      
      neuFlow.profiler:start('classify')
      packed_result = classifier:forward(outputs)
      neuFlow.profiler:lap('classify')
   else
      neuFlow.profiler:start('forward-prop')
      packed_result = convnet:forward(input)
      outputs = convnet.output
      packed_result = classifier:forward(outputs)
      neuFlow.profiler:lap('forward-prop')
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
   listOfFaces = image.mergeBlobs(listOfFaces, 5)
   neuFlow.profiler:lap('order-blobs')

   neuFlow.profiler:start('display')
   painter:gbegin()
   painter:showpage()
   -- display input image
   disp_input:show{tensor=camFrame, painter=painter, offset_x=0, offset_y=0,
                   min=0, max=1,
                   inplace=true,
                   legend='original', globalzoom=1}
   -- and pyramid
   disp_input:show{tensor=input, painter=painter, 
                   min=-1, max=1,
                   inplace=true,
                   legend='original', globalzoom=0.5}

   -- print boxes
   local i = 1
   local done = 0
   for i,result in ipairs(listOfFaces) do
      local x = result.x
      local y = result.y
      local scale = result.scale

      -- new: extract a patch around the detection, in HSL space,
      --      then compute the histogram of the Hue
      --      for skin tones, the Hue is always < 0.05
      local patch = camFrame:narrow(1,x*4,32):narrow(2,y*4,32)
      local patchH = rgb2hsl:forward(patch):select(3,1)
      local hist = lab.hist(patchH,100)

      -- only display skin-tone detections
      if hist.max.val < 0.2 or hist.max.val > 0.9 then
         image.qtdrawbox{ painter=painter,
                          x=listOfFaces[i].x * 4,
                          y=listOfFaces[i].y * 4,
                          w=32/listOfFaces[i].scale,
                          h=32/listOfFaces[i].scale,
                          globalzoom=1,
                          legend=listOfFaces[i].tag}
      end
   end

   -- maps
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
   neuFlow.profiler:lap('display')
   neuFlow.profiler:lap('GLOBAL')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=500, zoom=0.5}
   painter:gend()
end

-- cleanup
neuFlow:cleanup()
