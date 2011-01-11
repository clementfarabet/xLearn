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
two flow fields (x an y), and a warp version of the second
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
   opticalFlow.infer = function(args)
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
                             error( toolBox.usage('opticalFlow.infer',
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
                          return libopticalFlow.infer(img1, img2, 
                                                      alpha, ratio, minWidth, 
                                                      nOuterFPIterations, nInnerFPIterations,
                                                      nCGIterations)
                       end

   -- test me
   opticalFlow.testme = function()
                           local img1 = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img1.jpg'))
                           local img2 = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img2.jpg'))
                           local img1s = torch.Tensor(img1:size(1)/2,img1:size(2)/2,3)
                           local img2s = torch.Tensor(img1:size(1)/2,img1:size(2)/2,3)
                           image.scale(img1,img1s,'bilinear')
                           image.scale(img2,img2s,'bilinear')

                           local resx,resy,warp = opticalFlow.infer{pair={img1s,img2s},
                                                                    alpha=0.005,ratio=0.6,
                                                                    minWidth=50,nOuterFPIterations=6,
                                                                    nInnerFPIterations=1,
                                                                    nCGIterations=40}

                           local max = math.max(resx:max(), resy:max())
                           resx:div(max)
                           resy:div(max)
                           local resx_q = torch.Tensor():resizeAs(resx):copy(resx):mul(8):floor()
                           local resy_q = torch.Tensor():resizeAs(resx):copy(resy):mul(8):floor()

                           image.displayList{images={img1s, opticalFlow.field2rgb(resx,resy), 
                                                     img1s, (img2s-img1s):abs(),
                                                     img2s, opticalFlow.field2rgb(resx_q,resy_q), 
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
                                             legend="optical flow results"}

                           return resx, resy, warp
                        end

   -- generate colored version of flow fields
   opticalFlow.field2rgb = function (x,y)
                              -- check args
                              if not x or not y then
                                 error(
                                 toolBox.usage('opticalFlow.field2rgb',
                                               'merges X and Y flow fields into a single RGB image,\n'
                                                  .. 'where saturation=intensity, and hue=direction',
                                               nil,
                                      {type='torch.Tensor', help='flow field (X), (WxH)', req=true},
                                      {type='torch.Tensor', help='flow field (Y), (WxH)', req=true}))
                              end

                              -- merge them into an HSL image
                              local hsl = torch.Tensor(x:size(1), x:size(2), 3)
                              -- hue = angle:
                              hsl:select(3,1):copy( (y:cdiv(x+1e-6):abs()):atan():mul(180/math.pi) )
                              hsl:select(3,1):map2(x,y, function(h,x,y)
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
                              -- saturation = normalized intensity:
                              hsl:select(3,2):copy( (x:pow(2)+y:pow(2)):sqrt() )
                              hsl:select(3,2):div( hsl:select(3,2):max() + 1e-6 )
                              -- constant light:
                              hsl:select(3,3):fill(0.5)

                              -- convert HSL to RGB
                              return image.hsl2rgb(hsl)
                           end

   opticalFlowLoaded = true
end

return opticalFlow