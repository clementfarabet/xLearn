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

if not opticalFlow then
   require 'XLearn'

   ------------------------------------------------------------
   -- Liu's optical flow algorithm.
   --
   -- C. Liu. Beyond Pixels: Exploring New Representations and Applications
   -- for Motion Analysis. Doctoral Thesis. Massachusetts Institute of 
   -- Technology. May 2009.
   --
   -- To load: require 'opticalFlow'
   --
   -- @release 2010 Clement Farabet
   ------------------------------------------------------------
   module('opticalFlow', package.seeall)
   
   ------------------------------------------------------------
   -- Computes the optical flow of a pair of images, and returns
   -- the norm and the direction fields, plus a warped version of the second
   -- image, according to the flow field.
   --
   -- The flow field is computed using CG, as described in
   -- "Exploring New Representations and Applications for Motion Analysis",
   -- by C. Liu (Doctoral Thesis).
   -- More at http://people.csail.mit.edu/celiu/OpticalFlow/
   --
   -- The input images must be a WxHxN tensor, where N is the number
   -- of channels (colors).
   --
   -- @usage opticalFlow.infer() -- prints online help
   --
   -- @param pair  a pair of images (2 WxHxN tensor) [type = table]
   -- @param image1  the first image (WxHxN tensor) [type = torch.Tensor]
   -- @param image2  the second image (WxHxN tensor) [type = torch.Tensor]
   -- @param alpha  regularization weight [default = 0.01] [type = number]
   -- @param ratio  downsample ratio [default = 0.75] [type = number]
   -- @param minWidth  width of the coarsest level [default = 30] [type = number]
   -- @param nOuterFPIterations  number of outer fixed-point iterations [default = 15] [type = number]
   -- @param nInnerFPIterations  number of inner fixed-point iterations [default = 1] [type = number]
   -- @param nCGIterations  number of CG iterations [default = 20] [type = number]
   ------------------------------------------------------------
   infer = function(...)
           -- check args
           local args, pair, img1, img2, alpha, ratio, minWidth, 
           nOuterFPIterations, nInnerFPIterations, nCGIterations = toolBox.unpack(
              {...},
              'opticalFlow.infer',
              infer_help_desc,
              {arg='pair', type='table', help='a pair of images (2 WxHxN tensor)'},
              {arg='image1', type='torch.Tensor', help='the first image (WxHxN tensor)'},
              {arg='image2', type='torch.Tensor', help='the second image (WxHxN tensor)'},
              {arg='alpha', type='number', help='regularization weight', default=0.01},
              {arg='ratio', type='number', help='downsample ratio', default=0.75},
              {arg='minWidth', type='number', help='width of the coarsest level', default=30},
              {arg='nOuterFPIterations', type='number', help='number of outer fixed-point iterations', default=15},
              {arg='nInnerFPIterations', type='number', help='number of inner fixed-point iterations', default=1},
              {arg='nCGIterations', type='number', help='number of CG iterations', default=20}
           )

           -- pair ?
           if pair then 
              img1 = pair[1]
              img2 = pair[2]
           end

           -- check dims
           if img1:nDimension() ~= 3 then
              xerror('image should be a WxHxN tensor',nil,args.usage)
           end

           -- compute flow
           local flow_x, flow_y, warp =  libopticalFlow.infer(img1, img2, 
                                                              alpha, ratio, minWidth, 
                                                              nOuterFPIterations, nInnerFPIterations,
                                                              nCGIterations)

	   local flow_norm  = computeNorm(flow_x,flow_y)
	   local flow_angle = computeAngle(flow_x,flow_y)

           -- return results
           return flow_norm, flow_angle, warp, flow_x, flow_y
        end

   -- warper
   warp = function(...)
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

   ------------------------------------------------------------
   -- Computes the optical flow on some example images
   --
   -- @see                  opticalFlow.infer
   ------------------------------------------------------------
   testme = function()
           local img1 = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img1.jpg'))
           local img2 = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img2.jpg'))
           local img1s = torch.Tensor(img1:size(1)/2,img1:size(2)/2,3)
           local img2s = torch.Tensor(img1:size(1)/2,img1:size(2)/2,3)
           image.scale(img1,img1s,'bilinear')
           image.scale(img2,img2s,'bilinear')

           local resn,resa,warp,resx,resy = infer{pair={img1s,img2s},
                                                              alpha=0.005,ratio=0.6,
                                                              minWidth=50,nOuterFPIterations=6,
                                                              nInnerFPIterations=1,
                                                              nCGIterations=40}

           local resn_q = torch.Tensor():resizeAs(resn):copy(resn):div(resn:max()):mul(6):floor():div(8)
           local resa_q = torch.Tensor():resizeAs(resa):copy(resa):div(360/16):floor():mul(360/16)

           image.displayList{images={img1s, field2rgb(resn,resa), 
                                     img1s, (img2s-img1s):abs(),
                                     img2s, field2rgb(resn_q,resa_q), 
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

   ------------------------------------------------------------
   -- computes norm (size) of flow field from flow_x and flow_y,
   --
   -- @usage opticalFlow.computeNorm() -- prints online help
   --
   -- @param flow_x  flow field (x), (WxH) [required] [type = torch.Tensor]
   -- @param flow_y  flow field (y), (WxH) [required] [type = torch.Tensor]
   ------------------------------------------------------------
   computeNorm = function(...)
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

   ------------------------------------------------------------
   -- computes angle (direction) of flow field from flow_x and flow_y,
   --
   -- @usage opticalFlow.computeAngle() -- prints online help
   --
   -- @param flow_x  flow field (x), (WxH) [required] [type = torch.Tensor]
   -- @param flow_y  flow field (y), (WxH) [required] [type = torch.Tensor]
   ------------------------------------------------------------
   computeAngle = function(...)
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

   ------------------------------------------------------------
   -- merges Norm and Angle flow fields into a single RGB image,
   -- where saturation=intensity, and hue=direction
   --
   -- @usage opticalFlow.field2rgb() -- prints online help
   --
   -- @param norm  flow field (norm), (WxH) [required] [type = torch.Tensor]
   -- @param angle  flow field (angle), (WxH) [required] [type = torch.Tensor]
   -- @param max  if not provided, norm:max() is used [type = number]
   -- @param legend  prints a legend on the image [type = boolean]
   ------------------------------------------------------------
   field2rgb = function (...)
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
              _legend_ = _legend_
                 or image.load(paths.concat(paths.install_lua_path, 'opticalFlow/legend.png'),3)
              legend = torch.Tensor(hsl:size(2)/8, hsl:size(2)/8, 3)
              image.scale(_legend_, legend, 'bilinear')
              rgb:narrow(1,1,legend:size(2)):narrow(2,hsl:size(2)-legend:size(2)+1,legend:size(2)):copy(legend)
           end

           -- done
           return rgb
        end

   ------------------------------------------------------------
   -- Simplifies display of flow field in HSV colorspace when the
   -- available field is in x,y displacement
   --
   -- @usage opticalFlow.xy2rgb() -- prints online help
   --
   -- @param x  flow field (x), (WxH) [required] [type = torch.Tensor]
   -- @param y  flow field (y), (WxH) [required] [type = torch.Tensor]
   ------------------------------------------------------------
   xy2rgb = function (...)
	       -- check args
	       local args, x, y, max = toolBox.unpack(
		  {...},
		  'opticalFlow.xy2rgb',
		  'merges x and y flow fields into a single RGB image,\n'
		     .. 'where saturation=intensity, and hue=direction',
		  {arg='x', type='torch.Tensor', help='flow field (norm), (WxH)', req=true},
		  {arg='y', type='torch.Tensor', help='flow field (angle), (WxH)', req=true},
		  {arg='max', type='number', help='if not provided, norm:max() is used'}
	       )
	       
	       local norm = computeNorm(x,y)
	       local angle = computeAngle(x,y)
	       return field2rgb(norm,angle,max)
	    end


   
   ------------------------------------------------------------
   -- Function to read .flo files from middlebury
   ------------------------------------------------------------
   readflo = function (...)
	     local args, filename, setoutofband = toolBox.unpack(
		{...},
		'opticalFlow.readflo',
		'read a .flo file from middlebury',
		{arg='filename',type='string', help='filename to be read',
		 req=true},
		{arg='setoutofband',type='boolean',
		 help='reset out of band values to 0',default=false}
	     )
	     if not paths.filep(filename) then
		xerror(filename..' does not exist','opticalFlow.readflo()')
	     end
	     TAG_FLOAT = 202021.25
	     local ff = torch.DiskFile(filename)
	     ff:binary()
	     local tag = ff:readFloat()
	     if tag ~= TAG_FLOAT then
		xerror('unable to read '..filename..
		       ' perhaps bigendian error','opticalFlow.readflo()')
	     end

	     local w = ff:readInt()
	     local h = ff:readInt()
	     local nbands = 2
	     local tf = torch.FloatTensor(nbands,w,h)
	     ff:readFloat(tf:storage())
	     local td = torch.Tensor(w,h,nbands)
	     -- make compatible with XLearn flows
	     td:select(3,1):copy(tf:select(1,1))
	     td:select(3,2):copy(tf:select(1,2))
	     -- the middlebury flow files set out of range to 1.6666e-9
	     if setoutofband then 
		local ts = td:storage()
		local maxflow = math.max(w,h)
		for i = 1,td:nElement() do
		   if ts[i] > maxflow then 
		      ts[i] = 0
		   end
		end
	     end
	     ff:close()
	     return td
	  end

   ------------------------------------------------------------
   -- Function to write .flo files from middlebury
   ------------------------------------------------------------

   writeflo = function (...)
	     local args, flow, filename = toolBox.unpack(
		{...},
		'opticalFlow.writeflo',
		'write an x,y flow to the .flo file format from middlebury',
		{arg='flow',type='torch.Tensor',
		 help='flow tensor to be written [w][h][2]',req=true},
		{arg='filename',type='string', 
		 help='filename to be written',req=true}
	     )
	     if paths.filep(filename) then
		xerror(filename..' exists','opticalFlow.writeflo()')
	     end
	     if not filename:match('.flo$') then
		filename = filename..'.flo'
	     end
	     TAG_FLOAT = 202021.25
	     local ff = torch.DiskFile(filename,'w')
	     ff:binary()
	     ff:writeFloat(TAG_FLOAT)

	     local w = flow:size(1)
	     local h = flow:size(2)
	     local nbands = flow:size(3)
	     if nbands > 2 then
		print('only writing first 2 slices in 3 dim of flow')
		nbands = 2
	     elseif nbands < 2 then
		xerror('flow tensor must have x and y values format [w][h][2] where the last dim is x and y','flowFile.write')
	     end
	     ff:writeInt(w)
	     ff:writeInt(h)
	     local tt = torch.FloatTensor(nbands,w,h)
	     -- make compatible with XLearn flows
	     tt:select(1,1):copy(flow:select(3,1))
	     tt:select(1,2):copy(flow:select(3,2)) 
	     ff:writeFloat(tt:storage())
	     ff:close()
	     return 1
	  end

   ------------------------------------------------------------
   -- Function to test .flo IO code for files from middlebury
   ------------------------------------------------------------
   testfloIO = function (...) 
	     local args, filename, tmpfilename = toolBox.unpack(
		{...},
		'opticalFlow.testfloIO',
		'read a .flo file format, write a copy and compare',
		{arg='filename',type='string', 
		 help='filename to be read',
		 default='other-gt-flow/Grove2/flow10.flo'},
		{arg='tmpfilename',type='string', 
		 help='tmp filename to be written and removed',
		 default='/tmp/test.flo'}
	     )
	     print('Reading: '..filename)
	     local d10 = readflo(filename,true)
	     print('Writing: '..tmpfilename)
	     writeflo(d10,tmpfilename)
	     local d10c = readflo(tmpfilename)
	     local diff = (d10-d10c):abs():sum()
	     image.displayList{
		images={
		   opticalFlow.xy2rgb(d10:select(3,1),d10:select(3,2)),
		   opticalFlow.xy2rgb(d10c:select(3,1),d10c:select(3,2))
		},
		legends={'original','copy'},
		legend=filename..' diff: '..diff,
		gui=false,
		nhtiles=2
	     }
	     print('Diff: '..diff)
	     print('Removing: '..tmpfilename)
	     os.execute('rm '..tmpfilename)
	     return 1
	  end

end

return opticalFlow
