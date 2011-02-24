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
of channels (colors or any kind of features); or a
WxHxNxZ tensor, where Z is the number of slices in some 3D data
(e.g. a series of RGB images, or a series of N-dim feature maps).]]

if not mstsegmLoaded then
   require 'XLearn'
   mstsegm = {}

   -- register functions
   mstsegm.infer 
      = function(...)
           local args, img, sigma, k, min, dt, connex, incr, 
           benincr, incr_rst, edges = toolBox.unpack(
              {...},
              'mstsegm.infer', infer_help_desc,
              {arg='image', type='torch.Tensor', help='input image (WxHxN or WxHxNxZ tensor)', req=true},
              {arg='sigma', type='number', help='gaussian for preprocessing', default=0.8},
              {arg='k', type='number', help='thresholding parameter, a large k enforces larger segmentation areas', default=500},
              {arg='min', type='number', help='min parameter', default=20},
              {arg='disttype', type='string', help='distance metric to use', default='euclid'},
              {arg='connex', type='number', help='connectivity (4 or 8, edges per vertex)', default=8},
              {arg='incremental', type='boolean', help='incremental mode', default=false},
              {arg='benincremental', type='boolean', help='incremental mode', default=false},
              {arg='reset', type='boolean', help='incremental only: reset universe', default=false},
              {arg='edges', type='boolean', help='returns edges rather than colors', default=false}
           )
           if img:nDimension() ~= 3 and img:nDimension() ~= 4 then
              error(args.usage)
           end
           local dtn = 0
           if dt ~= 'euclid' then
              dtn = 1
           end
           if connex ~= 4 and connex ~= 8 and type(connex) ~= 'string' 
              and not connex:find('rad-') then
              error(args.usage)
           end
           if type(connex) == 'string' then
              connex = 100 + connex:sub(5,10)
           end
           return libmstsegm.infer(img, sigma, k, min, dtn, connex, incr, incr_rst, benincr, edges)
        end

   mstsegm.testme
      = function()
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