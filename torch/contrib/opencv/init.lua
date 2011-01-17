-- 
-- Note: this bit of code is a simple wrapper around the OpenCV library
--       http://opencv.willowgarage.com/
--
-- For now, it contains wrappers for:
--  + cvCalcOpticalFlowBM
--  + cvCalcOpticalFlowHS
--  + cvCalcOpticalFlowLK
--  + cvHaarDetectObjects
--  + cvCaptureFromCAM
--  + cvSobel
--  + cvCanny
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libopencv'

local help = {
calcOpticalFlow = [[
Computes the optical flow of a pair of images, and returns
4 maps: the flow field intensities, the flow field directions, and
the raw X and Y components

The flow field is computed using one of 3 methods: Block Matching (BM),
Lucas-Kanade (LK) or Horn-Schunck (HS).

The input images must be a pair of WxHx1 tensors.]]
}

if not opencvLoaded then
   require 'XLearn'
   opencv = {}

   -- OpticalFlow:
   opencv.calcOpticalFlow 
      = function(...)
           local args, pair, method = toolBox.unpack(
              {...},
              'opencv.calcOpticalFlow', 
              help.calcOpticalFlow,
              {arg='pair', type='table', help='a pair of images (2 WxHx1 tensor)', req=true},
              
              {arg='method', type='string', help='method used: BM | HS | LK', default='BM'},

              {arg='block_w', type='number', help='matching block width (BM+LK)', default=9},
              {arg='block_h', type='number', help='matching block height (BM+LK)', default=9},

              {arg='shift_x', type='number', help='shift step in x (BM only)', default=4},
              {arg='shift_y', type='number', help='shift step in y (BM only)', default=4},

              {arg='window_w', type='number', help='matching window width (BM only)', default=30},
              {arg='window_h', type='number', help='matching window height (BM only)', default=30},

              {arg='lagrangian', type='number', help='lagrangian multiplier (HS only)', default=1},
              {arg='iterations', type='number', help='nb of iterations (HS only)', default=5},

              {arg='autoscale', type='boolean', help='auto resize results', default=true},

              {arg='raw', type='boolean', help='if set, returns the raw X,Y fields', default=false},

              {arg='reuse', type='boolean', help='reuse last flow computed (HS+BM)', default=false},
              {arg='flowX', type='torch.Tensor', help='existing (previous) X-field (WxHx1 tensor)'},
              {arg='flowY', type='torch.Tensor', help='existing (previous) Y-field (WxHx1 tensor)'}
           )
           if pair[1]:nDimension() ~= 3 or pair[1]:size(3) ~= 1 then
              print('inconsistent input size')
              error(args.usage)
           end
           local flow_x = args.flowX or torch.Tensor()
           local flow_y = args.flowY or torch.Tensor()
           local reuse = 0
           if args.reuse then reuse = 1 end
           if method == 'BM' then
              libopencv.calcOpticalFlow(pair[2], pair[1], flow_x, flow_y, 1,
                                        args.block_w, args.block_h,
                                        args.shift_x, args.shift_y,
                                        args.window_w, args.window_h,
                                        reuse)
           elseif method == 'LK' then
              libopencv.calcOpticalFlow(pair[2], pair[1], flow_x, flow_y, 2,
                                        args.block_w, args.block_h)
           elseif method == 'HS' then
              libopencv.calcOpticalFlow(pair[2], pair[1], flow_x, flow_y, 3,
                                        args.lagrangian, args.iterations,
                                           -1,-1,-1,-1,
                                        reuse)
           else
              print('Unkown method')
              error(args.usage)
           end
           if args.raw then
              if args.autoscale then
                 local flow_x_s = torch.Tensor():resizeAs(pair[1])
                 local flow_y_s = torch.Tensor():resizeAs(pair[1])
                 image.scale(flow_x, flow_x_s, 'simple')
                 image.scale(flow_y, flow_y_s, 'simple')
                 return flow_x_s, flow_y_s
              else
                 return flow_x, flow_y
              end
           else
              local flow_norm = torch.Tensor()
              local flow_angle = torch.Tensor()
              -- compute norm:
              local x_squared = torch.Tensor():resizeAs(flow_x):copy(flow_x):cmul(flow_x)
              flow_norm:resizeAs(flow_y):copy(flow_y):cmul(flow_y):add(x_squared):sqrt()
              -- compute angle:
              flow_angle:resizeAs(flow_y):copy(flow_y):cdiv(flow_x):abs():atan():mul(180/math.pi)
              flow_angle:map2(flow_x, flow_y, function(h,x,y)
                                                 if x >= 0 and y >= 0 then
                                                    -- all good
                                                 elseif x >= 0 and y < 0 then
                                                    return 360 - h
                                                 elseif x < 0 and y >= 0 then
                                                    return 180 - h
                                                 elseif x < 0 and y < 0 then
                                                    return 180 + h
                                                 end
                                              end)
              if args.autoscale then
                 local flow_norm_s = torch.Tensor():resizeAs(pair[1])
                 local flow_angle_s = torch.Tensor():resizeAs(pair[1])
                 image.scale(flow_angle, flow_angle_s, 'simple')
                 image.scale(flow_norm, flow_norm_s, 'simple')
                 return flow_norm_s, flow_angle_s
              else
                 return flow_norm, flow_angle
              end
           end
        end

   -- testers:
   opencv.test_calcOpticalFlow
      = function()
           local methods = {'LK', 'HS', 'BM'}
           for i,method in ipairs(methods) do
              local as = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img1.jpg'))
              local bs = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img2.jpg'))
              local a = torch.Tensor(as:size(1)/2,bs:size(2)/2,3)
              local b = torch.Tensor(as:size(1)/2,bs:size(2)/2,3)
              image.scale(as,a,'bilinear')
              image.scale(bs,b,'bilinear')

              local norm, angle = opencv.calcOpticalFlow{pair={a:narrow(3,2,1),b:narrow(3,2,1)},
                                                         method=method}

              local hsl = torch.Tensor(a:size(1), a:size(2), 3)
              hsl:select(3,1):copy(angle):div(360)
              hsl:select(3,2):copy(norm):div(math.max(norm:max(),1e-2))
              hsl:select(3,3):fill(0.5)
              local rgb = image.hsl2rgb(hsl)

              image.displayList{images={a,norm,rgb,
                                        b,angle},
                                win_w=1000, win_h=600,
                                legend='cvOpticalFLow, method = ' .. method,
                                legends={'input 1', 'flow intensity', 'HSL-mapped flow',
                                         'input 2', 'flow angle'}}
           end                     
        end

   opencvLoaded = true
end

return opencv
