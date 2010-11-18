#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- Face Detection
--
require 'NeuFlow'

----------------------------------------------------------------------
-- EXEC: this part compiles the code and executes it
--

-- Load Face Detector ConvNet
convnet = nn.Sequential()
file = torch.DiskFile('../trained-nets/network-face-detect', 'r')
convnet:read(file)
file:close()

-- receiving 34x34 images
inputsize = 100
smallinput = torch.Tensor(inputsize,inputsize,1)
toreceive = torch.Tensor()
error = torch.Tensor()

-- camera
source = nn.ImageSource('camera')
rgb2yuv = nn.ImageTransform('rgb2y')

-- window
painter = qtwidget.newwindow(800,600)

-- just one dummy fprop to get sizes
convnet:forward(smallinput)

while true do
   -- get image and compute groundtruth
   camFrame = source:forward()
   camFrameY = rgb2yuv:forward(camFrame)
   image.scale(camFrameY, smallinput, 'bilinear')

   -- run this feature vector through the classifier
   result_hw = convnet:forward(smallinput)

   -- find faces
   listOfFaces = image.findBlobs{tensor=result_hw, threshold=-0.5,
                                 maxsize=10, discardClass=2, scale=1,
                                 preprocess=image.gaussian{width=3},
                                 labels={"face"}, listOfBlobs=nil}
   listOfFaces = image.reorderBlobs(listOfFaces)
   listOfFaces = image.remapBlobs(listOfFaces)
   listOfFaces = image.mergeBlobs(listOfFaces, 50)

   -- PAINT
   painter:gbegin()
   painter:showpage()
   -- display input image
   image.qtdisplay{tensor=smallinput, painter=painter, offset_x=0, offset_y=0,
                   min=0, max=1,
                   legend='original', globalzoom=2}

   -- print boxes
   local i = 1
   local done = 0
   while true do
      if (listOfFaces[i] ~= nil) then
         image.qtdrawbox{ painter=painter,
                          x=listOfFaces[i].x * 4,
                          y=listOfFaces[i].y * 4,
                          w=32/listOfFaces[i].scale,
                          h=32/listOfFaces[i].scale,
                          globalzoom=2,
                          legend=listOfFaces[i].tag}
         done = done + 1
      end
      i = i + 1
      if (done == listOfFaces.nbOfBlobs) then
         break
      end
   end

   -- and display result
   image.qtdisplay{tensor=result_hw:select(3,1), painter=painter, 
                   offset_x=40+inputsize, offset_y=0,
                   legend='from neuFlow', globalzoom=2, min=-1, max=1}

   -- maps
   toreceive = convnet.modules[8].output
   for k = 1,2 do
      for i = 1,toreceive:size(3)/2 do
         image.qtdisplay{tensor=toreceive:select(3,(k-1)*toreceive:size(3)/2+i), painter=painter, 
                         offset_x=inputsize*2+k*25, offset_y=i*25,
                         min=0, max=1,
                         globalzoom=2, min=-1, max=1}
      end
   end
   painter:gend()
end
