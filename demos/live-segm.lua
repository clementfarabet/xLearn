#!/usr/bin/env qlua
----------------------------------------------------------------------
-- WHAT: segments a image source.
--       the image source must be provided on the command line.
--
require 'XLearn'
require 'os'
require 'mstsegm'


-- parse args
op = OptionParser('%prog -s SOURCE [options]')
op:add_option{'-s', '--source', action='store', dest='source', 
              help='image source, can be one of: camera | lena'}
options,args = op:parse_args()


-- setup QT gui

toolBox.useQT()
painter = qtwidget.newwindow(1000,500)

-- video source

source = nn.ImageSource('camera')
scaler = nn.ImageRescale(250,190,3)

-- displayers
displayer_source = Displayer()
displayer_segments = Displayer()

-- global zoom

zoom = 2

-- profiler

profiler = Profiler()

local function run()

   profiler:start('global', 'fps')
   profiler:start('get-frame')
   frame = source:forward()
   frame = scaler:forward(frame)
   profiler:lap('get-frame')

   profiler:start('get-segments')
   segments = mstsegm.infer{image = frame}
   profiler:lap('get-segments')
   
   painter:gbegin()
   painter:showpage()
   profiler:start('display')
   
   displayer_source:show{tensor = frame:add(segments), painter = painter, globalzoom=zoom, 
                         min=0,max=2, offset_x=0, offset_y=10, 
                         legend='camera image'}
   
   displayer_segments:show{tensor = segments, painter = painter, globalzoom=zoom, 
                           min = 0, max = 1, 
                           offset_x = frame:size(1), offset_y = 10, 
                           legend = 'segmented image'}
   
   profiler:lap('display')
   profiler:lap('global')

   --disp times
   profiler:displayAll{painter = painter, x = 10, y = 420, zoom=1/2}
   painter:gend()
end

-- Loop Process
local timer = qt.QTimer()
timer.interval = 1
timer.singleShot = true
timer:start()
qt.connect(timer, 'timeout()', function() run() timer:start() end)
