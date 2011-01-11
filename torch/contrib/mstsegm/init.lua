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
-- Note: this bit of code is a simple wrapper around the graph-based (MST)
-- image segmentation algorithm provided/published by Pedro Felzenszwalb:
--
-- Efficient Graph-Based Image Segmentation
-- Pedro F. Felzenszwalb and Daniel P. Huttenlocher
-- International Journal of Computer Vision, 59(2) September 2004.
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libmstsegm'

local infer_help_desc =
[[Computes a segmentation map given an input image.

The map is computed using MST (minimum spanning tree) ideas,
as presented in "Efficient Graph-Based Image Segmentation", by
Pedro F. Felzenszwalb and Daniel P. Huttenlocher.

The input image must be a WxHxN tensor, where N is the number
of channels (colors).]]

if not mstsegmLoaded then
   require 'XLearn'
   mstsegm = {}

   -- register functions
   mstsegm.infer = function(args)
                      args = args or {}
                      local img = args.image
                      local sigma = args.sigma or 0.8
                      local k = args.k or 500
                      local min = args.min or 20
                      if not img or img:nDimension() ~= 3 or img:size(3) ~= 3 then
                         error( toolBox.usage('mstsegm.infer',
                               infer_help_desc,
                               nil,
                               {arg='image', type='torch.Tensor', help='input image [WxHxN tensor]'},
                               {arg='sigma', type='number', help='gaussian for preprocessing [default=0.8]'},
                               {arg='k', type='number', help='thresholding parameter, a large k enforces larger segmentation areas [default=500]'},
                               {arg='min', type='number', help='min parameter [default=20]'})
                            )
                     end
                     return libmstsegm.infer(img, sigma, k, min)
                  end

   mstsegm.testme = function()
                       local img = image.lena(400,400)
                       local res = mstsegm.infer{image = img,
                                                 sigma = 0.8,
                                                 k = 500,
                                                 min = 20}
                       image.displayList{images={img, res}, 
                                         legend='Segmentation [Min Spanning Tree]',
                                         legends={'input', 'result'}}
                   end

   mstsegmLoaded = true
end

return mstsegm