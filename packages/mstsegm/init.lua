--
-- Copyright (C) 2006 Pedro Felzenszwalb, 2010 Clement Farabet
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
-- It also contains lots of additional functions to segment volumes, images
-- of features; and additional tools to manipulate, compute min spanning
-- trees.
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libmstsegm'
require 'XLearn'

local infer_help_desc =
[[Computes a segmentation map given an input image.

The map is computed using MST (minimum spanning tree) ideas,
as presented in "Efficient Graph-Based Image Segmentation", by
Pedro F. Felzenszwalb and Daniel P. Huttenlocher.

The input image must be a WxHxN tensor, where N is the number
of channels (colors or any kind of features); or a
WxHxNxZ tensor, where Z is the number of slices in some 3D data
(e.g. a series of RGB images, or a series of N-dim feature maps).]]

if not mstsegm then
   ------------------------------------------------------------
   -- Pedro Felzenszwalb's segmentation algorithm.
   --
   -- Also contains additional tools to manipulate min spanning
   -- trees.
   --
   -- P.F.'s code was modified/extended to support multi-dimensional
   -- images, volumes, and incremental (shifting) volume segmentations,
   -- as well as optional usage of match points (e.g. SIFT)
   --
   -- To load: require 'mstsegm'
   --
   -- @release 2010 Clement Farabet
   ------------------------------------------------------------
   module('mstsegm', package.seeall)

   ------------------------------------------------------------
   -- dependencies
   ------------------------------------------------------------
   torch.include('mstsegm', 'Graph.lua')

   ------------------------------------------------------------
   -- Computes a segmentation map given an input image.
   --
   -- The map is computed using MST (minimum spanning tree) ideas,
   -- as presented in "Efficient Graph-Based Image Segmentation", by
   -- Pedro F. Felzenszwalb and Daniel P. Huttenlocher.
   --
   -- The input image must be a WxHxN tensor, where N is the number
   -- of channels (colors or any kind of features); or a
   -- WxHxNxZ tensor, where Z is the number of slices in some 3D data
   -- (e.g. a series of RGB images, or a series of N-dim feature maps).
   --
   -- @usage mstsegm.infer() -- prints online help
   --
   -- @param image  input image (WxHxN or WxHxNxZ tensor) [required] [type = torch.Tensor]
   -- @param sigma  gaussian for preprocessing [default = 0.8] [type = number]
   -- @param k  thresholding parameter, a large k enforces larger segmentation areas [default = 500] [type = number]
   -- @param min  min parameter [default = 20] [type = number]
   -- @param disttype  distance metric to use [default = euclid] [type = string]
   -- @param connex  connectivity (4 or 8, edges per vertex) [default = 8] [type = number]
   -- @param incremental  incremental mode [type = boolean]
   -- @param incremental_cheap  cheap incremental mode [type = boolean]
   -- @param reset  incremental only: reset universe [type = boolean]
   -- @param edges  returns edges rather than colors [type = boolean]
   -- @param matches  array of matches in 2 between two last frames (2x2xN tensor) [type = torch.Tensor]
   ------------------------------------------------------------
   infer = function(...)
           local args, img, sigma, k, min, dt, connex, incr, 
           benincr, incr_rst, edges, matches = toolBox.unpack(
              {...},
              'mstsegm.infer', infer_help_desc,
              {arg='image', type='torch.Tensor', help='input image (WxHxN or WxHxNxZ tensor)', req=true},
              {arg='sigma', type='number', help='gaussian for preprocessing', default=0.8},
              {arg='k', type='number', help='thresholding parameter, a large k enforces larger segmentation areas', default=500},
              {arg='min', type='number', help='min parameter', default=20},
              {arg='disttype', type='string', help='distance metric to use: euclid | angle', default='euclid'},
              {arg='connex', type='number', help='connectivity (4 or 8, edges per vertex)', default=4},
              {arg='incremental', type='boolean', help='incremental mode', default=false},
              {arg='incremental_cheap', type='boolean', help='cheap incremental mode', default=false},
              {arg='reset', type='boolean', help='incremental only: reset universe', default=false},
              {arg='edges', type='boolean', help='returns edges rather than colors', default=false},
              {arg='matches', type='torch.Tensor', help='array of matches in 2 between two last frames (2x2xN tensor)'}
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
           return libmstsegm.segment(img, sigma, k, min, dtn, connex, 
                                     incr, incr_rst, benincr, edges, 
                                     matches)
        end

   ------------------------------------------------------------
   -- Computes the minimum spanning tree of a graph.
   --
   -- If an image is provided, then an edge-weighted graph is
   -- first computed, using the given connexity (4 or 8). 
   --
   -- Then it computes the MST of that graph.
   --
   -- @usage mstsegm.mst() -- prints online help
   --
   -- @param image  input image (WxHxN or WxH tensor) [type = torch.Tensor]
   -- @param graph  input graph (WxHxK tensor) [type = torch.Tensor]
   -- @param connex  connectivity (4 or 8, edges per vertex) [default = 8] [type = number]
   -- @param sigma  gaussian for preprocessing [default = 0.8] [type = number]
   -- @param disttype  distance metric to use: euclid | angle [default = euclid] [type = string]
   --
   -- @return the minimum spanning tree [type = mstsegm.Tree]
   ------------------------------------------------------------
   mst = function(...)
      local args, img, connex, sigm, dist, gr = toolBox.unpack(
         {...},
         'mstsegm.mst', 'Computes the minimum spanning tree of a given image',
         {arg='image', type='torch.Tensor', help='input image (WxHxN or WxH tensor)'},
         {arg='connex', type='number', help='connectivity (4 or 8, edges per vertex)', default=4},
         {arg='sigma', type='number', help='gaussian for preprocessing', default=0.8},
         {arg='disttype', type='string', help='distance metric to use: euclid | angle', 
          default='euclid'},
         {arg='graph', type='mstsegm.Graph', help='input graph (WxHxK tensor)'}
      )
      if img then
         if img:nDimension() ~= 3 and img:nDimension() ~= 2 then
            error(args.usage)
         end
         if img:nDimension() == 2 then
            local t2d = torch.Tensor()
            t2d:set(img:storage(), 1, img:nElement())
            t2d:resize(img:size(1), img:size(2), 1)
            img = t2d
         end
         gr = Graph{image=img,connex=connex}
      end
      if dist == 'euclid' then dist = 0
      elseif dist == 'angle' then dist = 1
      else xerror('dist should be one of: angle | euclid',nil,args.usage) 
      end
      if not gr then xerror('please provide an image or a graph',nil,args.usage) end
      return libmstsegm.mst(gr.edges,sigm,dist,connex)
   end

   ------------------------------------------------------------
   -- Computes the minimax disparity between two vertices in a graph
   -- 
   -- @usage mstsegm.mst_minimaxDisparity() -- prints online help
   -- 
   -- @param mst  a minimum spanning tree [required] [type = mstsegm.Tree]
   -- @param vertex1  a vertex in the graph [required] [type = number]
   -- @param vertex2  a second vertex in the graph [required] [type = number]
   --
   -- @return the minimax disparity between the two verteces provided [type = number]
   --         or the full disparity matrix if no vertex is provided [type = NxN torch.Tensor]
   ------------------------------------------------------------
   mst_minimaxDisparity = function(...)
      local args, mst, v1, v2 = toolBox.unpack(
         {...},
         'mstsegm.mst_minimaxDisparity',
         'Computes the minimax disparity between two vertices in a graph'
            .. 'if no vertex is provided, then the full matrix is computed',
         {arg='mst', type='mstsegm.Tree', help='a minimum spanning tree', req=true},
         {arg='vertex1', type='number', help='a vertex in the graph'},
         {arg='vertex2', type='number', help='a second vertex in the graph'}
      )
      return libmstsegm.mst_minimaxdisparity(mst,v1,v2)
   end

   ------------------------------------------------------------
   -- Computes the Rand Index between two segmentations
   ------------------------------------------------------------
   rand = function(...)
      local args, segm1, segm2 = toolBox.unpack(
         {...},
         'mstsegm.rand',
         'Computes the Rand index between two segmentations',
         {arg='segm1', type='torch.Tensor', help='first segmentation (WxH tensor)', req=true},
         {arg='segm2', type='torch.Tensor', help='second segmentation (WxH tensor)', req=true}
      )
      return libmstsegm.rand(segm1,segm2)
   end

   ------------------------------------------------------------
   -- Computes the binary edges (contours) of a segmentation
   --
   -- The input image must be a WxHxN tensor, where N is the number
   -- of channels (colors or any kind of features); or a
   -- WxHxNxZ tensor, where Z is the number of slices in some 3D data
   -- (e.g. a series of RGB images, or a series of N-dim feature maps).
   --
   -- @usage mstsegm.segm2edges() -- prints online help
   --
   -- @param image  input image (WxHxN or WxHxNxZ tensor) [required] [type = torch.Tensor]
   --
   -- @return edges binary countours
   ------------------------------------------------------------
   segm2edges = function(...)
           local args, img = toolBox.unpack(
              {...},
              'mstsegm.segm2edges', 'Computes the binary edges (contours) of a segmentation',
              {arg='image', type='torch.Tensor', help='input image (WxHxN or WxHxNxZ tensor)', req=true}
           )
           if img:nDimension() ~= 2 and img:nDimension() ~= 3 and img:nDimension() ~= 4 then
              error(args.usage)
           end
           local res = torch.Tensor():resizeAs(img):copy(img)
           libmstsegm.segm2edges(res)
           return res
        end

   ------------------------------------------------------------
   -- Computes a segmentation map on the famous Lena image
   --
   -- @see                  mstsegm.infer
   ------------------------------------------------------------
   testme = function()
           local img = image.lena(400,400)
           local res = mstsegm.infer{image = img,
                                     sigma = 0.8,
                                     k = 500,
                                     min = 20}
           image.displayList{images={img, res}, 
                             legend='Segmentation [Min Spanning Tree]',
                             legends={'input', 'result'}}
        end
end

return mstsegm
