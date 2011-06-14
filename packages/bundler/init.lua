-- 
-- Note: this bit of code is a wrapper around the parts we wish to use
-- from the bunder code.  Intially we are interested in the structure
-- from motion parts of bundler
--
-- Wrapper: Marco Scoffier
-- 

-- load C lib
require 'libbundler'

------------------------------------------------------------
-- helps
local help = {
---
sfm = [[ Sparse Structure from Motion 

[1] http://phototour.cs.washington.edu/bundler/ ]],

}

------------------------------------------------------------
-- load
if not bundlerLoaded then
   require 'XLearn'
   bundler = {}

   -- sift
   bundler.sfm_pair
      = function(...)
	   local args, frames, matches,
           verbose = toolBox.unpack(
	      {...},
	      'bundler.sfm',
	      help.sfm,
	      {arg='frames', type='table', help='one frame per image, position of matched points.',req=true},
	      {arg='matches', type='torch.Tensor', help='matches are the indexes of matches between image pairs', req=true}
	   )
	   --	      {arg='match_ids', type='torch.Tensor', help='indicates which frame_ids are matched for in each match pair', req=true} 

	   -- FIXME for debugging I am now only doing the 2 image
	   -- scenario where each image has a projection for all the
	   -- points

	   local ncam = #frames
	   local npts = matches:size(2)
	   print ('ncam: ' .. ncam .. ' npts: ' .. npts .. ' Unknowns: ' .. ncam * 6 + npts *3)
	   -- FIXME build a list of views for each point
	   local nprj = ncam * npts
	   
	   vmask = torch.CharTensor(npts,ncam):fill(0)

	   -- -- x,y point in image of 3D point projected into an image.
	   projections = torch.Tensor(2,nprj)
	   -- each image
	   for i = 1,ncam do
	      -- FIXME number of points in image 
	      for j = 1,npts do
	   	 vmask[j][i] = 1
		 local frame_id = matches[i][j]
	   	 projections[1][(i-1)*npts+j] = frames[i][1][frame_id] -- x
	   	 projections[2][(i-1)*npts+j] = frames[i][2][frame_id] -- y
	      end
	   end
	   libbundler.sfm(ncam,npts,nprj,vmask,projections)
           return 0 
        end

   -- test function
   bundler.sfm_testme
      = function()
	   require 'vlfeat'
           print 'computing SIFT descriptors on image 1'
           local img1 = image.lena()
           frames1, descs1 = vlfeat.sift{image=img1:select(3,2)}

           print 'computing SIFT descriptors on image 2'
           local img2 = torch.Tensor():resizeAs(img1)
           image.rotate(img1,img2,0.2)
           frames2, descs2 = vlfeat.sift{image=img2:select(3,2)}

           print 'finding matches between SIFT descriptors'
           matches, dists = vlfeat.sift_match(descs1, descs2, 100)
           print (matches:size(2) .. ' matches!')
           -- vlfeat.sift_match_display({img1,img2}, matches, {frames1, frames2})
	   frames = { frames1, frames2 }
	   bundler.sfm_pair(frames, matches)
        end

   -- loaded
   bundlerLoaded = true
end

return bundler
