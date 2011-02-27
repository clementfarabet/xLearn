-- 
-- Note: this bit of code is a simple wrapper around the vlfeat
-- library
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libvlfeat'

------------------------------------------------------------
-- helps
local help = {
---
sift = [[Scale-Invariant Feature Transform.

Computes the SIFT frames [1] (keypoints) F of the
image I. I is a gray-scale image in single precision. Each column
of F is a feature frame and has the format [X;Y;S;TH], where X,Y
is the (fractional) center of the frame, S is the scale and TH is
the orientation (in radians).

If computeDescriptors is set, it also computes the SIFT descriptors
[1] as well. Each column is the descriptor of the corresponding frame. 
A descriptor is a 128-dimensional vector.

[1] D. G. Lowe, Distinctive image features from scale-invariant
    keypoints. IJCV, vol. 2, no. 60, pp. 91-110, 2004.]],

---
sift_match = [[Match SIFT features.

Matches the two sets of SIFT descriptors provided.
Returns the matches and also the squared Euclidean distance between 
the matches.

The function uses the algorithm suggested by D. Lowe [1] to reject
matches that are too ambiguous.

If a threshold is provided, a descriptor D1 is matched to a 
descriptor D2 only if the distance d(D1,D2) multiplied by threshold 
is not greater than the distance of D1 to all other descriptors. 

[1] D. G. Lowe, Distinctive image features from scale-invariant
    keypoints. IJCV, vol. 2, no. 60, pp. 91-110, 2004.]]
}

------------------------------------------------------------
-- load
if not vlfeatLoaded then
   require 'XLearn'
   vlfeat = {}

   -- sift
   vlfeat.sift
      = function(...)
	   local args, img, octaves, levels, firstOctave, peakThresh, edgeThresh,
           normThresh, magnif, windowSize, frames, orientations, computeDescriptors, 
           verbose = toolBox.unpack(
	      {...},
	      'vlfeat.sift',
	      help.sift,
	      {arg='image', type='torch.Tensor', help='input image (WxH tensor)', req=true},
	      {arg='octaves', type='number', help='number of octaves of the DoG scale space [default=MAX]'},
	      {arg='levels', type='number', help='number of levels per octave of the DoG scale space', default=3},
	      {arg='firstOctave', type='number', help='index of the first octave of the DoG scale space', default=0},
              {arg='peakThresh', type='number', help='peak selection threshold', default=0},
              {arg='edgeThresh', type='number', help='non-edge selection threshold', default=10},
              {arg='normThresh', type='number', help='min l2-norm of descriptors before normalization [default=-INF]'},
              {arg='magnif', type='number', help='descriptor magnification factor', default=3},
              {arg='windowSize', type='number', help='variance of the Gaussian window that determines the descriptor support', default=2},
              {arg='frames', type='torch.Tensor', help='frames to use: bypass the detector'},
              {arg='orientations', type='boolean', help='compute the orietantions of the frames', default=false},
              {arg='descriptors', type='boolean', help='compute and return SIFT descriptors', default=true},
              {arg='verbose', type='boolean', help='be verbose', default=false}
	   )
           if img:nDimension() ~= 2 then
              xerror('image should be a WxH tensor',nil,args.usage)
           end
           return libvlfeat.sift(img,verbose,octaves,levels,firstOctave,edgeThresh,peakThresh,
                                 normThresh,magnif,windowSize,orientations,computeDescriptors,
                                 frames)
        end

   -- sift matcher
   vlfeat.sift_match
      = function(...)
	   local args, descs1, descs2, threshold = toolBox.unpack(
	      {...},
	      'vlfeat.sift_match',
	      help.sift_match,
	      {arg='descs1', type='torch.Tensor', help='first descriptors', req=true},
	      {arg='descs2', type='torch.Tensor', help='second descriptors', req=true},
              {arg='threshold', type='number', help='used to limit matches', default=1.5}
           )
           if not descs1 or not descs2 then
              return false, false
           end
           return libvlfeat.siftmatch(descs1,descs2,threshold)
        end

   -- sift displayer
   vlfeat.sift_display
      = function(...)
	   local args, img, frames, descs, max, win, tensor = toolBox.unpack(
	      {...},
	      'vlfeat.sift_display',
	      'provide original image and frames, as computed by: frames = vlfeat.sift(image)',
	      {arg='image', type='torch.Tensor', help='original image (WxH tensor)', req=true},
	      {arg='frames', type='torch.Tensor', help='sift frames, as computed by vlfeat.sift', req=true},
	      {arg='descriptors', type='torch.Tensor', help='if provided, used to generate unique colors for each frame'},
	      {arg='max', type='number', help='max nb of points to display [default=ALL]'},
	      {arg='win', type='gfx.Window', help='if not provided, it is created and returned'},
              {arg='totensor', type='torch.Tensor', help='used to render results'}
           )
           -- tensor ?
           if tensor then
              win = win or gfx.ImageSurface(img:size(1), img:size(2))
              tensor:resize(img:size(1), img:size(2), 3)
           end
           -- clear hist
           if win then win:cls() end
           -- disp image
           win = image.display{image=img, legend='SIFT Key Points', win=win}
           -- draw one circle per frame
           win:batchBegin()
           if frames then
              -- max defaults to all
              max = max or (frames:size(2)-1)
              for i = frames:size(2),frames:size(2)-max,-1 do
                 if i == 0 then break end
                 local x = frames[1][i]
                 local y = frames[2][i]
                 local scale = frames[3][i]
                 local angle = frames[4][i]
                 local x1 = x + scale*math.sin(angle)
                 local y1 = y + scale*math.cos(angle)
                 local circ = win:circle(x,y)
                 local color = {}
                 if not descs then
                    for i=1,3 do color[i] = random.uniform() end
                 else
                    -- making a hash out of the descriptors:
                    local desc = descs:select(2,i)
                    local red = desc:narrow(1,1,50):norm()
                    local green = desc:narrow(1,51,50):norm()
                    local blue = desc:narrow(1,101,28):norm()
                    red = math.min(red,green)/math.max(red,green)
                    green = math.min(blue,green)/math.max(blue,green)
                    blue = math.min(blue,red)/math.max(blue,red)
                    -- using the hash to make a unique/repeatable color:
                    color = {red,green,blue}
                 end
                 circ:set('markerSize',scale/2)
                 circ:set('penColor',color)
                 win:line(x,y,x1,y1):set('penColor',color)
              end
           end
           win:batchEnd()
           -- render into tensor 
           if tensor then
              win:redraw()
              win.lcairo_object:toTensor(tensor)
           end
           return win, tensor
        end

   -- sift match displayer
   vlfeat.sift_match_display
      = function(...)
	   local args, imgs, matches, frames, color, max, win, tensor = toolBox.unpack(
	      {...},
	      'vlfeat.sift_match_display',
	      'provide original image, descriptors and frames, as computed by:\n'
                 ..' frames1,descs1 = vlfeat.sift(image1);\n'
                 ..' frames2,descs2 = vlfeat.sift(image2);\n'
                 ..' matches = vlfeat.sift_match(descs1,descs2)',
	      {arg='images', type='table', help='pair of original images', req=true},
	      {arg='matches', type='torch.Tensor', help='matches (2xN tensor)', req=true},
	      {arg='frames', type='table', help='pair of sift frames', req=true},
	      {arg='color', type='table', help='color (RGB table)'},
	      {arg='max', type='number', help='max nb of frames to display [default=ALL]'},
	      {arg='win', type='gfx.Window', help='if not provided, it is created and returned'},
              {arg='totensor', type='torch.Tensor', help='used to render results'}
           )
           -- tensor ?
           if tensor then
              local img = imgs[1]
              win = win or gfx.ImageSurface(img:size(1)*2, img:size(2))
              tensor:resize(img:size(1)*2, img:size(2), 3)
           end
           -- clear hist
           if win then win:cls() end
           -- disp images
           local ofx = imgs[1]:size(1)
           win = image.displayList{images={imgs[1],imgs[2]}, window=win,
                                   win_w=ofx*2, legend='SIFT Matches'}
           win:batchBegin()
           if matches and matches:nDimension() == 2 then
              -- max defaults to all
              max = max or matches:size(2)
              for i = 1,math.min(matches:size(2),max) do
                 local idx1 = matches[1][i]
                 local idx2 = matches[2][i]
                 local desc1_x = frames[1][1][idx1]
                 local desc1_y = frames[1][2][idx1]
                 local desc1_sc = frames[1][3][idx1]
                 local desc2_x = frames[2][1][idx2]
                 local desc2_y = frames[2][2][idx2]
                 local desc2_sc = frames[2][3][idx2]
                 local col = color
                 if not col then
                    col = {}
                    for i=1,3 do col[i] = random.uniform() end
                 end
                 local circ1 = win:circle(desc1_x,desc1_y)
                 circ1:set('penColor',col)
                 circ1:set('markerSize',desc1_sc/2)
                 local circ2 = win:circle(desc2_x+ofx,desc2_y)
                 circ2:set('penColor',col)
                 circ2:set('markerSize',desc2_sc/2)
                 win:line(desc1_x,desc1_y, desc2_x+ofx,desc2_y):set('penColor', col)
              end
           end
           win:batchEnd()
           -- render into tensor 
           if tensor then
              win:redraw()
              win.lcairo_object:toTensor(tensor)
           end
           return win, tensor
        end

   -- test function
   vlfeat.sift_testme
      = function()
           print 'computing SIFT descriptors on lena'
           local img = image.lena()
           local frames, descs = vlfeat.sift{image=img:select(3,2)}
           vlfeat.sift_display(img, frames, descs)
           print('found ' .. frames:size(2) .. ' key points')

           print 'computing SIFT descriptors on gradient pattern'
           local img = image.load(paths.concat(paths.install_lua_path, 'vlfeat/pattern-1.jpg'))
           local frames, descs = vlfeat.sift{image=img:select(3,1)}
           vlfeat.sift_display(img, frames, descs)
           print('found ' .. frames:size(2) .. ' key points')
        end

   -- test function
   vlfeat.sift_match_testme
      = function()
           print 'computing SIFT descriptors on image 1'
           local img1 = image.lena()
           local frames1, descs1 = vlfeat.sift{image=img1:select(3,2)}

           print 'computing SIFT descriptors on image 2'
           local img2 = torch.Tensor():resizeAs(img1)
           image.rotate(img1,img2,0.2)
           local frames2, descs2 = vlfeat.sift{image=img2:select(3,2)}

           print 'finding matches between SIFT descriptors'
           local matches, dists = vlfeat.sift_match(descs1, descs2, 100)
           print (matches:size(2) .. ' matches!')
           vlfeat.sift_match_display({img1,img2}, matches, {frames1, frames2})
        end

   -- loaded
   vlfeatLoaded = true
end

return vlfeat
