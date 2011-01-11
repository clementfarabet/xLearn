#!/usr/bin/env qlua

------------------------------------------------------------
-- computes the optical flow-field of a pair of images
--

require 'opticalFlow'
opticalFlow.testme()

print([[

To know more about these algos, start a Lua shell, and try:
> require 'opticalFlow'
> img1 = image.load('/path/to/img1.jpg')
> img2 = image.load('/path/to/img2.jpg')
> opticalFlow.infer()
... prints some online help on the function ...
> flow_x, flow_y = opticalFlow.infer{pair={img1,img2}}
> image.displayList{images={flow_x,flow_y}}
... the flow can be displayed in HSL space ...
> hsl = opticalFlow.field2rgb(flow_x, flow_y)
> image.display(hsl)
]])
