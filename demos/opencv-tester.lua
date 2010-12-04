#!/usr/bin/env qlua
----------------------------------
-- an opencv tester
----------------------------------

require 'XLearn'
require 'opencv'

-- need QT
toolBox.useQT()

-- resource file
local ui = paths.thisfile('opencv-tester.ui')

-- demo proper
function demo()

   -- Camera frame
   local camera = nn.ImageSource('camera')
   local vframesat = torch.Tensor(640,480,3)
   local cameraframe = torch.CharTensor(640,480,3)

   -- and results
   local frame = torch.CharTensor(640,480,1)
   local canny = torch.CharTensor(640,480,1)
   local sobel = torch.ShortTensor(640,480,1)
   local previous = torch.CharTensor(640,480,1)
   local motion_x = torch.FloatTensor((640-7)/20,(480-7)/20,1)
   local motion_y = torch.FloatTensor((640-7)/20,(480-7)/20,1)
   local faces = {}

   -- Window
   local widget = qtuiloader.load(ui)
   local painter = qt.QtLuaPainter(widget.frame)

   -- Displayers
   local disp_input = Displayer()
   local disp_sobel = Displayer()
   local disp_canny = Displayer()
   local disp_oflow_x = Displayer()
   local disp_oflow_y = Displayer()

   -- Sliders
   local vslide = widget.verticalSlider.value
   local hslide = widget.horizontalSlider.value
   widget.horizontalSlider_2.value = 80
   widget.horizontalSlider_3.value = 20

   local function process()
      -- zoom 
      local zoom = 1/(hslide*2)

      -- initiate drawing
      painter:gbegin()
      painter:showpage()
      painter:setcolor("black")
      painter:setfont(qt.QFont{serif=false,italic=true,size=12})

      -- Get camera frame
      local vframe = camera:forward()
      image.scaleForDisplay{tensor=vframe,tensorOut=vframesat}

      -- disp
      disp_input:show{ tensor=vframesat, painter=painter, 
                       globalzoom=zoom,
                       min=0, max=1,
                       offset_x=0, offset_y=40, 
                       legend='Face Detect'}

      -- copy
      vframesat:mul(255)
      cameraframe:copy(vframesat)

      -- get green channel
      frame = cameraframe:narrow(3,2,1)

      -- TEST 1 : Canny
      if widget.radioButton_3.checked then
         libopencv.canny(frame, canny, 
                         widget.horizontalSlider_2.value, 
                         widget.horizontalSlider_3.value, 3)
         disp_canny:show{ tensor=canny, painter=painter, 
                          globalzoom=zoom,
                          min=-1, max=1,
                          offset_x=800, offset_y=40,
                          legend='Canny filter'}
      end
      
      -- TEST 2 : Sobel
      if widget.radioButton_0.checked then
         libopencv.sobel(frame, sobel)
         disp_sobel:show{ tensor=sobel, painter=painter, 
                          min=-32, max=64, 
                          globalzoom=zoom,
                          min=-32, max=32,
                          offset_x=0, offset_y=560,
                          legend='Sobel filter'}
      end

      -- TEST 3 : Optic Flow
      if widget.radioButton_2.checked then
         libopencv.calcOpticalFlow(frame, previous, motion_x, motion_y)
         previous:copy(frame)
         disp_oflow_x:show{ tensor=motion_x, painter=painter, 
                            globalzoom=zoom,
                            min=-100, max=100,
                            zoom=8,
                            offset_x=800, offset_y=570,
                            legend='Optic Flow - X' }
         disp_oflow_y:show{ tensor=motion_y, painter=painter, 
                            globalzoom=zoom,
                            min=-100, max=100,
                            zoom=8,
                            offset_x=800, offset_y=780,
                            legend='Optic Flow - Y' }
      end

      -- TEST 4 : Face Detect
      if widget.radioButton_1.checked then
         faces = libopencv.haarDetectObjects(cameraframe, 'haarcascade_frontalface_alt.xml')

         -- draw a box on each face ;-)
         if faces then
            faces[1] = 100
            faces[2] = 200
            faces[3] = 80
            faces[4] = 80
            for i=1,#faces,4 do
               image.qtdrawbox{ painter=painter,
                                x=faces[i], y=faces[i+1],
                                w=faces[i+2], h=faces[i+3],
                                globalzoom=zoom, 
                                legend='face' }
            end
         end
      end

      painter:grestore()
      painter:gend()
   end

   -- Loop Process
   local timer = qt.QTimer()
   timer.interval = 1
   timer.singleShot = true
   qt.connect(timer,'timeout()',
              function() 
                 vslide = widget.verticalSlider.value
                 hslide = widget.horizontalSlider.value
                 process()
                 timer:start()
              end )
   
   -- Close Process
   local listener = qt.QtLuaListener(widget)
   qt.connect(listener,'sigClose()',
              function()
                 print('exiting...')
                 camera:stop()
                 print('1')
                 timer:stop()
                 print('2')
                 timer:deleteLater()
                 print('3')
                 widget:deleteLater()
                 print('4')
              end )
   qt.connect(listener,'sigShow(bool)',
              function(b) 
                 if b then timer:start() end 
              end )
   
   widget.windowTitle = "Live Processing"
   widget:show()

end

demo()

