--
-- Copyright (C) 2006 Pedro Felzenszwalb
-- 
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
-- 

-- 
-- Note: this bit of code is a simple wrapper around the belief-propagation
-- stereo algorithm provided/publised by Pedro Felzenszwalb:
--
-- Efficient Belief Propagation for Early Vision
-- Pedro F. Felzenszwalb and Daniel P. Huttenlocher
-- International Journal of Computer Vision, Vol. 70, No. 1, October 2006
-- http://people.cs.uchicago.edu/~pff/papers/bp-long.pdf
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libstereo'

local infer_help_desc =
[[Computes a disparity map from a pair of stereo images.

The map is computed using belief-propagation on a pyramid 
of scales, as presented in "Efficient Belief Propagation for Early Vision",
by Pedro F. Felzenszwalb and Daniel P. Huttenlocher.

The method is iterative, and therefore best results are
achieved after multiple iterations.

The pair of images can be passed as a single WxHx2 tensor,
or two separate WxH tensors.]]

if not stereoLoaded then
   require 'XLearn'
   stereo = {}

   -- register functions
   stereo.infer = function(args)
                     args = args or {}
                     local left = args.left
                     local right = args.right
                     if args.pair then
                        left = args.pair:select(3,1)
                        right = args.pair:select(3,2)
                     end
                     if not left or not right 
                        or left:nDimension() ~= 2 
                        or right:nDimension() ~= 2 then
                        error( toolBox.usage('stereo.infer',
                                             infer_help_desc,
                                             nil,
                                             {arg='pair', type='torch.Tensor', help='a pair of stereo images [WxHx2 tensor]'},
                                             {arg='left', type='torch.Tensor', help='the left image [WxH tensor]'},
                                             {arg='right', type='torch.Tensor', help='the right image [WxH tensor]'},
                                             {arg='iterations', type='number', help='nb of iterations [default=5]'},
                                             {arg='scales', type='number', help='nb of scales (going down by factors of 2) [default=5]'})
                            )
                     end
                     return libstereo.infer(left, right, args.iterations or 5, args.scales or 5)
                  end

   stereo.testme = function()
                      local left = image.load(paths.concat(paths.install_lua_path, 'stereo/left.jpg'),1):select(3,1)
                      local right = image.load(paths.concat(paths.install_lua_path, 'stereo/right.jpg'),1):select(3,1)
                      local res = stereo.infer{left = left,
                                               right = right,
                                               scales = 5,
                                               iterations = 5}
                      image.displayList{images={left, right, res},
                                        legend='stereo results',
                                        legends={'left','right','disparity map'}}
                   end

   stereoLoaded = true
end

return stereo