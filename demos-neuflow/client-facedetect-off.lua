#!/usr/bin/env qlua
----------------------------------------------------------------------
-- WHAT: executes a pretrained neural net on an image source.
--       the image source must be provided on the command line.
--
require 'XLearn'
require 'os'

-- parse args
op = OptionParser('%prog -s SOURCE [options]')
op:add_option{'-s', '--source', action='store', dest='source', 
              help='image source, can be one of: camera | lena'}
options,args = op:parse_args()

-- setup QT gui
toolBox.useQT()
widget = qtuiloader.load('client-facedetect.ui')
painter = qt.QtLuaPainter(widget.frame)

-- video source
if not options.source then options.source = 'camera' end
source = nn.ImageSource(options.source)

-- retrieve trained network
convnet = nn.Sequential()
file = torch.DiskFile('../trained-nets/network-face-detect', 'r')
convnet:read(file)
file:close()

-- various transformers:
rgb2y = nn.ImageTransform('rgb2y')
scales = {1/3, 1/5, 1/7.5, 1/11, 1/15}
packer = nn.PyramidPacker(scales, convnet)
unpacker = nn.PyramidUnPacker(convnet)

-- displayers to hold print buffers
displayer_source = Displayer()
displayer_features = Displayer()
displayer_preproc = Displayer()
displayer_result = Displayer()

-- profiler
profiler = Profiler()

----------------------------------------------------------------------
-- ROUTINES: we use two routines: process() and paint()
--
local function process()
   -- profiler
   profiler:start('full-loop','fps')

   -- get frame from camera, convert to YUV, and just keep Y
   profiler:start('get-frame')
   camFrame = source:forward()
   frameY = rgb2y:forward(camFrame)
   profiler:lap('get-frame')

   -- generate pyramid of scales
   profiler:start('pyramid-pack')
   pyramid, coordinates = packer:forward(frameY)
   profiler:lap('pyramid-pack')

   -- forward prop
   profiler:start('convnet')
   packed_result = convnet:forward(pyramid)
   profiler:lap('convnet')

   -- unpack
   profiler:start('pyramid-unpack')
   outputMaps = unpacker:forward(packed_result, coordinates)
   profiler:lap('pyramid-unpack')

   -- find blobs
   profiler:start('find-blobs')
   listOfFaces = nil
   for i = 1,#outputMaps do
      listOfFaces = image.findBlobs{tensor=outputMaps[i], threshold=0.02*vslide-1, 
                                    maxsize=10, discardClass=2, scale=scales[i],
                                    labels={"face"},
                                    listOfBlobs=listOfFaces}
   end
   profiler:lap('find-blobs')

   -- Extract Centroids of detections
   profiler:start('order-blobs')
   listOfFaces = image.reorderBlobs(listOfFaces)
   listOfFaces = image.remapBlobs(listOfFaces)
   listOfFaces = image.mergeBlobs(listOfFaces, 5)
   profiler:lap('order-blobs')
end

local function paint()
   painter:gbegin()
   painter:showpage()
   profiler:start('display')

   -- image from cam
   displayer_source:show{tensor=frameY, painter=painter, 
                         globalzoom=zoom, 
                         min=0, max=1,
                         offset_x=0, offset_y=0,
                         legend='Face detection'}

   if widget.checkBox1.checked then
      -- image pre-processed
      displayer_preproc:show{tensor=convnet.modules[1].output, painter=painter, 
                             globalzoom=zoom,
                             zoom=1,
                             min=-1, max=1,
                             offset_x=0, offset_y=frameY:size(2)+30,
                             legend='Local Normalized'}

      -- disp features
      local features = convnet.modules[#convnet.modules-1].output
      local legend = 'Features'
      local k = 0
      local j = 0
      for i=1,features:size(3) do
         displayer_features:show{tensor=features:select(3,i),
                                 painter=painter, 
                                 globalzoom=zoom,
                                 min=-1, max=1,
                                 offset_x=800+j, 
                                 offset_y=k*(features:size(2)+10) + 40,
                                 legend=legend}
         legend = nil
         k = k + 1
         if k == features:size(3)/2 then j = j + 100; k = 0 end
      end

      -- disp classifications
      local step = 40
      for i=1,#outputMaps do
         displayer_result:show{tensor=outputMaps[i]:select(3,1), painter=painter, 
                               globalzoom=zoom,
                               zoom = 6,
                               inplace=true,
                               min=-1, max=1,
                               offset_x=1100, 
                               offset_y=step,
                               legend='Scaled Output #'..i}
         step = outputMaps[i]:size(2)*8 + step + 40
      end
   end

   -- print objects
   local i = 1
   local done = 0
   while true do
      if (listOfFaces[i] ~= nil) then
         local x = listOfFaces[i].x
         local y = listOfFaces[i].y
         local scale = listOfFaces[i].scale
         image.qtdrawbox{ painter=painter,
                          x=x * 4,
                          y=y * 4,
                          w=32/scale,
                          h=32/scale,
                          globalzoom=zoom, 
                          legend=listOfFaces[i].tag}
         done = done + 1
      end
      i = i + 1
      if (done == listOfFaces.nbOfBlobs) then
         break
      end
   end
   profiler:lap('display')
   profiler:lap('full-loop')

   -- disp times
   profiler:displayAll{painter=painter, x=10, y=frameY:size(2)*1.8*zoom, zoom=zoom}
   painter:gend()
end


function demo()
   -- Loop Process
   local timer = qt.QTimer()
   timer.interval = 0
   timer.singleShot = false
   timer:start()
   qt.connect(timer,'timeout()',
              function() 
                 vslide = widget.verticalSlider.value
                 hslide = widget.horizontalSlider.value
                 zoom = 1/hslide
                 process()
                 if widget.checkBox3.checked then
                    paint()
                 end
                 timer:start()
              end )
   
   -- Close Process   
   widget.windowTitle = "Face Detection"
   widget:show()

end

demo()

