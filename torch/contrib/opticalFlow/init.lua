-- 
-- Note: this bit of code is a simple wrapper around the optical-flow
-- algorithm developped/published by C.Liu:
--
-- C. Liu. Beyond Pixels: Exploring New Representations and Applications
-- for Motion Analysis. Doctoral Thesis. Massachusetts Institute of 
-- Technology. May 2009.
--
-- More at: http://people.csail.mit.edu/celiu/OpticalFlow/
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libopticalFlow'

local infer_help_desc =
[[Computes the optical flow of a pair of images, and returns
the norm and the direction fields, plus a warped version of the second
image, according to the flow field.

The flow field is computed using CG, as described in
"Exploring New Representations and Applications for Motion Analysis",
by C. Liu (Doctoral Thesis).
More at http://people.csail.mit.edu/celiu/OpticalFlow/

The input images must be a WxHxN tensor, where N is the number
of channels (colors).]]

if not opticalFlowLoaded then
   require 'XLearn'
   opticalFlow = {}

   
   -- register functions
   opticalFlow.infer 
      = function(args)
           -- get args
           args = args or {}
           local pair = args.pair
           local img1 = args.image1
           local img2 = args.image2
           if pair then 
              img1 = pair[1]
              img2 = pair[2]
           end
           local alpha = args.alpha or 0.01
           local ratio = args.ratio or 0.75
           local minWidth = args.minWidth or 30
           local nOuterFPIterations = args.nOuterFPIterations or 15
           local nInnerFPIterations = args.nInnerFPIterations or 1
           local nCGIterations = args.nCGIterations or 20

           -- check args
           if not img1 or not img2 
              or img1:nDimension() ~= 3 then
              error( 
                 toolBox.usage(
                    'opticalFlow.infer',
                    infer_help_desc,
                    nil,
                    {arg='pair', type='table', help='a pair of images [2 WxHxN tensor]'},
                    {arg='image1', type='torch.Tensor', help='the first image [WxHxN tensor]'},
                    {arg='image2', type='torch.Tensor', help='the second image [WxHxN tensor]'},
                    {arg='alpha', type='number', help='regularization weight [default = 0.01]'},
                    {arg='ratio', type='number', help='downsample ratio [default = 0.75]'},
                    {arg='minWidth', type='number', help='width of the coarsest level [default = 30]'},
                    {arg='nOuterFPIterations', type='number', help='number of outer fixed-point iterations [default = 15]'},
                    {arg='nInnerFPIterations', type='number', help='number of inner fixed-point iterations[default = 1]'},
                    {arg='nCGIterations', type='number', help='number of CG iterations [default = 20]'})
              )
           end

           -- compute flow
           local flow_x, flow_y, warp =  libopticalFlow.infer(img1, img2, 
                                                              alpha, ratio, minWidth, 
                                                              nOuterFPIterations, nInnerFPIterations,
                                                              nCGIterations)

	   local flow_norm  = opticalFlow.computeNorm(flow_x,flow_y)
	   local flow_angle = opticalFlow.computeAngle(flow_x,flow_y)

           -- return results
           return flow_norm, flow_angle, warp, flow_x, flow_y
        end

   -- warper
   opticalFlow.warp
      = function(...)
           local args, inp, vx, vy = toolBox.unpack(
              {...},
              'opticalFlow.warp', 
              'warps an image according to a flow field:\n'
                 ..'if flow was computed from img1->img2, then warp(img2,vx,vy) will compute\n'
                 ..'a reconstruction of img1',
              {arg='image', type='torch.Tensor', help='input image (WxHxN tensor)', req=true},
              {arg='flow_x', type='torch.Tensor', help='x component of flow field', req=true},
              {arg='flow_y', type='torch.Tensor', help='y component of flow field', req=true}
           )
           if inp:nDimension() ~= 3 then
              xerror('image should be a WxHxN tensor',nil,args.usage)
           end
           return libopticalFlow.warp(inp, vx, vy)
        end

   -- test me
   opticalFlow.testme 
      = function()
           local img1 = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img1.jpg'))
           local img2 = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img2.jpg'))
           local img1s = torch.Tensor(img1:size(1)/2,img1:size(2)/2,3)
           local img2s = torch.Tensor(img1:size(1)/2,img1:size(2)/2,3)
           image.scale(img1,img1s,'bilinear')
           image.scale(img2,img2s,'bilinear')

           local resn,resa,warp,resx,resy = opticalFlow.infer{pair={img1s,img2s},
                                                              alpha=0.005,ratio=0.6,
                                                              minWidth=50,nOuterFPIterations=6,
                                                              nInnerFPIterations=1,
                                                              nCGIterations=40}

           local resn_q = torch.Tensor():resizeAs(resn):copy(resn):div(resn:max()):mul(6):floor():div(8)
           local resa_q = torch.Tensor():resizeAs(resa):copy(resa):div(360/16):floor():mul(360/16)

           image.displayList{images={img1s, opticalFlow.field2rgb(resn,resa), 
                                     img1s, (img2s-img1s):abs(),
                                     img2s, opticalFlow.field2rgb(resn_q,resa_q), 
                                     warp, (warp-img1s):abs()}, 
                             zoom=1,
                             min=0, max=1,
                             win_w=1300,
                             win_h=500,
                             legends={'input 1', 'flow field', 'input 1', 
                                      'input 1 - input 2',
                                      'input 2', 'quantized flow field', 
                                      'warped(input 2)', 
                                      'input 1 - warped(input 2)'},
                             legend="optical flow, method = C.Liu"}

           return resn, resa, warp
        end

   -- compute norm from x,y:
   opticalFlow.computeNorm
      = function(...)
           -- check args
           local args, flow_x, flow_y = toolBox.unpack(
              {...},
              'opticalFlow.computeNorm',
              'computes norm (size) of flow field from flow_x and flow_y,\n',
              {arg='flow_x', type='torch.Tensor', help='flow field (x), (WxH)', req=true},
              {arg='flow_y', type='torch.Tensor', help='flow field (y), (WxH)', req=true}
           )
           local flow_norm = torch.Tensor()
           local x_squared = torch.Tensor():resizeAs(flow_x):copy(flow_x):cmul(flow_x)
           flow_norm:resizeAs(flow_y):copy(flow_y):cmul(flow_y):add(x_squared):sqrt()
	   return flow_norm
	end

   -- compute angle from x,y:
   opticalFlow.computeAngle
      = function(...)
           -- check args
           local args, flow_x, flow_y = toolBox.unpack(
              {...},
              'opticalFlow.computeAngle',
              'computes angle (direction) of flow field from flow_x and flow_y,\n',
              {arg='flow_x', type='torch.Tensor', help='flow field (x), (WxH)', req=true},
              {arg='flow_y', type='torch.Tensor', help='flow field (y), (WxH)', req=true}
           )
	   local flow_angle = torch.Tensor()
           flow_angle:resizeAs(flow_y):copy(flow_y):cdiv(flow_x):abs():atan():mul(180/math.pi)
           flow_angle:map2(flow_x, flow_y, function(h,x,y)
                                              if x == 0 and y >= 0 then
                                                 return 90
                                              elseif x == 0 and y <= 0 then
                                                 return 270
                                              elseif x >= 0 and y >= 0 then
                                                 -- all good
                                              elseif x >= 0 and y < 0 then
                                                 return 360 - h
                                              elseif x < 0 and y >= 0 then
                                                 return 180 - h
                                              elseif x < 0 and y < 0 then
                                                 return 180 + h
                                              end
                                           end)
	   return flow_angle
	end


   -- generate colored version of flow fields
   opticalFlow.field2rgb
      = function (...)
           -- check args
           local args, norm, angle, max, legend = toolBox.unpack(
              {...},
              'opticalFlow.field2rgb',
              'merges Norm and Angle flow fields into a single RGB image,\n'
                 .. 'where saturation=intensity, and hue=direction',
              {arg='norm', type='torch.Tensor', help='flow field (norm), (WxH)', req=true},
              {arg='angle', type='torch.Tensor', help='flow field (angle), (WxH)', req=true},
              {arg='max', type='number', help='if not provided, norm:max() is used'},
              {arg='legend', type='boolean', help='prints a legend on the image', default=false}
           )

           -- max
           local saturate = false
           if max then saturate = true end
           max = math.max(max or norm:max(), 1e-2)

           -- merge them into an HSL image
           local hsl = torch.Tensor(norm:size(1), norm:size(2), 3)
           -- hue = angle:
           hsl:select(3,1):copy(angle):div(360)
           -- saturation = normalized intensity:
           hsl:select(3,2):copy(norm):div(max)
           if saturate then hsl:select(3,2):tanh() end
           -- light varies inversely from saturation (null flow = white):
           hsl:select(3,3):copy(hsl:select(3,2)):mul(-0.5):add(1)

           -- convert HSL to RGB
           local rgb = image.hsl2rgb(hsl)

           -- legend
           if legend then
              opticalFlow.legend = opticalFlow.legend
                 or image.load(paths.concat(paths.install_lua_path, 'opticalFlow/legend.png'),3)
              legend = torch.Tensor(hsl:size(2)/8, hsl:size(2)/8, 3)
              image.scale(opticalFlow.legend, legend, 'bilinear')
              rgb:narrow(1,1,legend:size(2)):narrow(2,hsl:size(2)-legend:size(2)+1,legend:size(2)):copy(legend)
           end

           -- done
           return rgb
        end

   opticalFlowLoaded = true
end

return opticalFlow