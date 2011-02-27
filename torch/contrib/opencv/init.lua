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

The input images must be a pair of WxHx1 tensors.]],

   CornerHarris = [[
Computes the Harris Corner features of an image
The input image must be a of WxHx1 tensor]],

   TrackPyrLK = [[
Runs pyramidal Lucas-Kanade, on two input images and a set of points
which are meant to be tracked ]],

   calcOpticalFlowPyrLK = [[
Computes the optical flow of a pair of images using the Pyramidal 
Lucas-Kanade algorithm on a set of interest points.
Returns a points tensor of the sub-pixel positions of the features 
and a copy of the input image with yellow circles around the 
interest points ]]
}

if not opencvLoaded then
   require 'XLearn'
   opencv = {}
   -- CornerHarris
   opencv.CornerHarris
      = function(...)
	   local args, img, blocksize, aperturesize, k = toolBox.unpack(
	      {...},
	      'opencv.CornerHarris',
	      help.CornerHarris,
	      {arg='img', type='torch.Tensor', help='image in which to detect Haar points'},
	      {arg='blocksize',type='number', help='neighborhood size', default=9},
	      {arg='aperturesize',type='number',help='Sobel aperture size', default=3},
	      {arg='k',type='number',help='the Harris detector free parameter',default=0.04}
	   )
	   local img = img
	   if img:size(3) > 1 then
	      img = img:narrow(3,2,1)
	   end
	   if aperturesize % 2 == 0 then
	      print('WARNING: aperturesize (Sobel kernel size) must be odd and not larger than 31')
	      aperturesize = aperturesize -1 
	   end
	   local harris = torch.Tensor():resizeAs(img)
	   libopencv.CornerHarris(img,harris,blocksize,aperturesize,k)
	   return harris
	end

   -- testers:
   opencv.test_CornerHarris
      = function()
	   local a = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img1.jpg'))
	   
	   local harris = opencv.CornerHarris{img=a,
					      blocksize=5,
					      aperturesize=3,
					      k=0.05}

	   image.displayList{images={a,harris},
			     legends={'original image','Harris Corners'},
			     legend='harris corner detection',
			     win_w=a:size(1)*2,win_h=a:size(2)}
	end                     

   -- GoodFeaturesToTrack
   opencv.GoodFeaturesToTrack
      = function(...)
	   local args, img, count, quality, min_distance, win_size  = 
	      toolBox.unpack(
	      {...},
	      'opencv.GoodFeaturesToTrack',
	      [[
		    Computes the GoodFeatures algorithm of opencv.
		    + input img in which to find features 
		    + returns a points tensor of the sub-pixel positions of the features 
		       and a copy of the input image with yellow circles around the interest points ]], 
	      {arg='img', type='torch.Tensor', help='image in which to detect Good Feature points',req=true},
	      {arg='count',type='number', help='number of points to return', default=500},
	      {arg='quality',type='number', help='quality', default=0.01},
	      {arg='min_distance',type='number', help='min spatial distance (in pixels) between returned feature points', default=10},
	      {arg='win_size',type='number', help='window size over which to run heuristics', default=10}
	   )
	   local img = img
	   local img_out = torch.Tensor():resizeAs(img):copy(img)
	   local points = torch.Tensor(count,2)
	   local image_out = libopencv.GoodFeaturesToTrack(img,
							   points,
							   img_out,
							   count,
							   quality,
							   min_distance,
							   win_size)
	   return points, img_out
	end

   -- testers:
   opencv.test_GoodFeaturesToTrack
      = function()
	   local a = image.load(paths.concat(paths.install_lua_path, 'opticalFlow/img1.jpg'))
	   
	   local pts, iout = opencv.GoodFeaturesToTrack{img=a,count=125}

	   image.displayList{images={a,iout},
			     legends={'original image','Good Features'},
			     legend='opencv: GoodFeaturesToTrack',
			     win_w=a:size(1)*2,win_h=a:size(2)}
	end                     

   -- Pyramidal Lucas-Kanade
   opencv.TrackPyrLK
      = function(...)
	   local args, pair, points_in, win_size  = toolBox.unpack(
	      {...},
	      'opencv.TrackPyrLK',
	      help.TrackPyrLK,
	      {arg='pair', type='table', help='a pair of images (2 WxHx1 tensor)', req=true},
	      {arg='points_in',type='torch.Tensor', help='points to track', req=true},
	      {arg='win_size',type='number',help='over how large of a window can the LK track', default= 25}
	   )
	   local points_out = torch.Tensor():resizeAs(points_in):zero()
	   local feature_found = torch.Tensor(points_in:size(1)):zero()
	   local feature_error = torch.Tensor(points_in:size(1)):zero()
	   libopencv.TrackPyrLK(pair[1], pair[2], points_in, points_out, win_size, feature_found, feature_error)
	   
	   return points_out, feature_found, feature_error
	end

   opencv.drawFlowlinesOnImage
      = function (...)
	   local args, pair, image = toolBox.unpack(
	      {...},
	      'opencv.drawFlowlinesOnImage',
	      [[ utility to visualize sparse flows ]],
	      {arg='pair', type='table', help='a pair of point tensors (2 nPointsx2 tensor)', req=true},
	      {arg='image', type='torch.Tensor',help='image on which to draw the flowlines', req=true},
	      {arg='color', type='torch.Tensor',help='color of flow line eg. R = [255,0,0]'},
	      {arg='mask', type='torch.Tensor',help='mask tensor 1D npoints 0 when not to draw point'}
	   )
	   if not color then
	      color = torch.Tensor(3):zero()
	      color[1] = 255 
	   end
	   libopencv.drawFlowlinesOnImage(pair[1],pair[2],image,color,mask)
	end

   opencv.test_TrackPyrLK
      = function()
	   local im1 = image.load(paths.concat(paths.install_lua_path, 
					       'opticalFlow/img1.jpg'))
	   local im2 = image.load(paths.concat(paths.install_lua_path, 
					       'opticalFlow/img2.jpg'))
	   
	   local ptsin = opencv.GoodFeaturesToTrack{img=im1,count=im1:nElement()}
	   
	   local ptsout = opencv.TrackPyrLK{pair={im1,im2},points_in=ptsin}
	   opencv.drawFlowlinesOnImage({ptsin,ptsout},im2)
	   image.displayList{images={im1,im2},
			     legends={'previous image',
				      'current image w/ flowlines',
				      'Optical Flow Pyramidal LK Tracking'},
			     legend='opencv: Optical Flow Pyramidal LK Tracking',
			     win_w=im1:size(1)*2,win_h=im1:size(2)}
	end

	-- Pyramidal Lucas-Kanade
	opencv.calcOpticalFlowPyrLK
	   = function(...)
		local args, pair, count  = toolBox.unpack(
		   {...},
		   'opencv.calcOpticalFlowPyrLK',
		   help.calcOpticalFlowPyrLK,
		   {arg='pair', type='table', help='a pair of images (2 WxHx1 tensor)', req=true},
		   {arg='count',type='number', help='number of points on which to calculate', default=500}
		)
		local img_out = torch.Tensor():resizeAs(pair[2]):copy(pair[2])
		local points = torch.Tensor(count,2)
		local quality = 0.01
		local min_distance = 5
		local win_size = 25
		local flow_x = 
		   torch.Tensor(pair[1]:size(1),pair[1]:size(2),1):zero()
		local flow_y = 
		   torch.Tensor(pair[1]:size(1),pair[1]:size(2),1):zero()
		local image_out = 
		   libopencv.calcOpticalFlowPyrLK(pair[1], pair[2],
						  flow_x:select(3,1), flow_y:select(3,1),
						  points, img_out,
						  count, quality,
						  min_distance, win_size)
		local flow_norm = torch.Tensor()
		local flow_angle = torch.Tensor()
		-- compute norm:
		local x_squared = torch.Tensor():resizeAs(flow_x):copy(flow_x):cmul(flow_x)
		flow_norm:resizeAs(flow_y):copy(flow_y):cmul(flow_y):add(x_squared):sqrt()
		-- compute angle:
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

		return flow_norm, flow_angle, flow_x, flow_y, points, img_out
	     end

	-- testers:
	opencv.test_calcOpticalFlowPyrLK
	   = function()
		local im1 = image.load(paths.concat(paths.install_lua_path, 
						    'opticalFlow/img1.jpg'))
		local im2 = image.load(paths.concat(paths.install_lua_path, 
						    'opticalFlow/img2.jpg'))
		
		local n,a,x,y,pts,iout = 
		   opencv.calcOpticalFlowPyrLK{pair={im1,im2},
					       count=im1:size(1)*im1:size(2)} 
		image.displayList{images={im1,iout},
				  legends={'original image',
					   'Optical Flow Pyramidal LK'},
				  legend='opencv: Optical Flow Pyramidal LK',
				  win_w=im1:size(1)*2,win_h=im1:size(2)}
	     end                     
	
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
		   if args.autoscale then
		      local flow_norm_s = torch.Tensor():resizeAs(pair[1])
		      local flow_angle_s = torch.Tensor():resizeAs(pair[1])
		      local flow_x_s = torch.Tensor():resizeAs(pair[1])
		      local flow_y_s = torch.Tensor():resizeAs(pair[1])
		      image.scale(flow_angle, flow_angle_s, 'simple')
		      image.scale(flow_norm, flow_norm_s, 'simple')
		      image.scale(flow_x, flow_x_s, 'simple')
		      image.scale(flow_y, flow_y_s, 'simple')
		      return flow_norm_s, flow_angle_s, flow_x_s, flow_y_s
		   else
		      return flow_norm, flow_angle, flow_x, flow_y
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
