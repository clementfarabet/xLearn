--
-- Copyright ESIEE (2009,2010) 
-- 
-- Michel Couprie (m.couprie@esiee.fr)
-- Camille Couprie (c.couprie@esiee.fr)
-- Hugues Talbot (h.talbot@esiee.fr)
-- Leo Grady (leo.grady@siemens.com)
-- Laurent Najman (l.najman@esiee.fr)
-- 
-- This software contains some image processing algorithms whose purpose is to be
-- used primarily for research.
-- 
-- This software is governed by the CeCILL license under French law and
-- abiding by the rules of distribution of free software.  You can  use, 
-- modify and/ or redistribute the software under the terms of the CeCILL
-- license as circulated by CEA, CNRS and INRIA at the following URL
-- "http://www.cecill.info". 
-- 
-- As a counterpart to the access to the source code and  rights to copy,
-- modify and redistribute granted by the license, users are provided only
-- with a limited warranty  and the software's author,  the holder of the
-- economic rights,  and the successive licensors  have only  limited
-- liability. 
-- 
-- In this respect, the user's attention is drawn to the risks associated
-- with loading,  using,  modifying and/or developing or reproducing the
-- software by the user in light of its specific status of free software,
-- that may mean  that it is complicated to manipulate,  and  that  also
-- therefore means  that it is reserved for developers  and  experienced
-- professionals having in-depth computer knowledge. Users are therefore
-- encouraged to load and test the software's suitability as regards their
-- requirements in conditions enabling the security of their systems and/or 
-- data to be ensured and,  more generally, to use and operate it in the 
-- same conditions as regards security. 
-- 
-- The fact that you are presently reading this means that you have had
-- knowledge of the CeCILL license and that you accept its terms.
-- 

-- 
-- Note: this bit of code is a simple wrapper around the Pink library,
-- devloped and maintained at ESIEE, Paris.
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libpink'

------------------------------------------------------------
local help_desc_watershed = [[
Topological binary watershed (Couprie-Bertrand algorithm),
as defined in [CB97].

A marker image is supplied in <B>mark.pgm</B>. The result 
is a binary image.

If invert==true then the dual operator is applied.

[CB97] M. Couprie and G. Bertrand:
``Topological Grayscale Watershed Transformation'',
SPIE Vision Geometry V Proceedings, 3168 (136--146), 1997.]]

------------------------------------------------------------
local help_desc_gradient = [[
Computes an approximation of the gradient on a 
grayscale image (deriche method).

Mode can be set to:
'mod' = gradient modulus
'dir' = gradient direction
'laplacian' = normalized laplacian]]

------------------------------------------------------------
local help_desc_minima = [[
Computes the minima of an image.]]

------------------------------------------------------------
-- load
if not pinkLoaded then
   require 'XLearn'
   pink = {}

   -- wrappers
   pink.watershed 
      = function(...)
           local args, img, markers, connex, invert = toolBox.unpack(
              {...},
              'pink.watershed', 
              help_desc_watershed,
              {arg='image', type='torch.Tensor', help='input image (WxHxN tensor)', req=true},
              {arg='markers', type='torch.Tensor', help='markers (WxHx1 or WxH tensor)', req=true},
              {arg='connex', type='number', help='connectivity (4 or 8)', default=4},
              {arg='invert', type='boolean', help='if true, the dual operator is applied', default=false}
           )
           if img:nDimension() ~= 2 then
              xerror('image should be a WxHx1 or WxH tensor',nil,args.usage)
           end
           if markers:nDimension() ~= 2 then
              error(args.usage)
           end
           return libpink.watershed(img,markers,connex,invert)
        end

   pink.gradient 
      = function(...)
           local args, img, mode, alpha = toolBox.unpack(
              {...},
              'pink.gradient',
              help_desc_gradient,
              {arg='image', type='torch.Tensor', help='input image (WxH tensor)', req=true},
              {arg='mode', type='string', help='one of: mod | dir | laplacian', default='mod'},
              {arg='alpha', type='number', help='filter size (alpha ~ 1/size)', default=1}
           )
           if img:nDimension() ~= 2 then
              xerror('image should be a WxH tensor',nil,args.usage)
           end
           if mode == 'mod' then mode = 0
           elseif mode == 'dir' then mode = 1
           elseif mode == 'laplacian' then mode = 2
           else error(args.usage) end
           return libpink.gradient(img,mode,alpha)
        end

   pink.minima 
      = function(...)
           local args, img, connex = toolBox.unpack(
              {...},
              'pink.minima',
              help_desc_minima,
              {arg='image', type='torch.Tensor', 
               help='input image (WxH or WxHxN tensor)', req=true},
              {arg='connex', type='string', 
               help='connectivity (0, 4, 8, b0, b1 or 14)', default='0'}
           )
           if img:nDimension() ~= 2 and img:nDimension() ~= 3 then
              xerror('image should be a WxH or WxHxN tensor',nil,args.usage)
           end
           return libpink.minima(img,connex)
        end

   pink.image2graph 
      = function(...)
           local args, img, mode, alpha = toolBox.unpack(
              {...},
              'pink.image2graph',
              help_desc_image2graph,
              {arg='image', type='torch.Tensor', help='input image (WxH tensor)', req=true},
              {arg='mode', type='string', help='one of: diff | max | deriche', default='diff'},
              {arg='alpha', type='number', help='filter size (alpha ~ 1/size)', default=1}
           )
           if img:nDimension() ~= 2 then
              xerror('image should be a WxH tensor',nil,args.usage)
           end
           if mode == 'diff' then mode = 0
           elseif mode == 'max' then mode = 1
           elseif mode == 'deriche' then mode = 2
           else error(args.usage) end
           return libpink.image2graph(img,mode,alpha)
        end

   pink.graph2image 
      = function(...)
           local args, graph, half, bar = toolBox.unpack(
              {...},
              'pink.graph2image',
              help_desc_graph2image,
              {arg='graph', type='torch.Tensor', help='input graph (WxH tensor)', req=true},
              {arg='half', type='boolean', help='returns half-size image', default=false},
              {arg='invert', type='number', help='if true, uses the complement', default=false}
           )
           if graph:nDimension() ~= 2 then
              xerror('graph should be a WxH tensor',nil,args.usage)
           end
           local result = libpink.graph2image(graph,bar)
           if half then
              local maxer = nn.SpatialMaxPooling(2,2)
              result = maxer:forward(result:resize(result:size(1), result:size(2), 1))
              result = result:select(3,1)
           end
           return result
        end

   pink.saliency 
      = function(...)
           local args, graph, mode, img = toolBox.unpack(
              {...},
              'pink.saliency',
              help_desc_saliency,
              {arg='graph', type='torch.Tensor', help='input graph (WxH tensor)', req=true},
              {arg='mode', type='string', help='can be one of: surface | dynamic | volume | omega',
               default='surface'},
              {arg='image', type='torch.Tensor', help='aux image (WxH tensor)'}
           )
           if graph:nDimension() ~= 2 then
              xerror('graph should be a WxH tensor',nil,args.usage)
           end
           if mode == 'surface' then mode = 0
           elseif mode == 'dynamic' then mode = 1
           elseif mode == 'volume' then mode = 2
           elseif mode == 'omega' then mode = 3
           else error(args.usage) end
           return libpink.saliency(graph, mode, img)
        end

   pink.testme 
      = function()
           local lena = image.rgb2y(image.lena()):select(3,1)
           local lenas = image.convolve(lena, image.gaussian(3,0.4), 'same')
           local gradient = pink.gradient(lenas)
           local mins = pink.minima(gradient)
           local watersheds = pink.watershed(gradient, mins)
           image.displayList{images={lenas,gradient,mins,watersheds},
                             legend='basic Pink operators',
                             legends={'input','gradient','mins','watershed'},
                             font=11,
                             win_w=1024*0.8,
                             win_h=1024*0.8,
                             zoom=0.8}
        end

   pink.edges2colors
      = function(...)
           local args, img = toolBox.unpack(
              {...},
              'pink.edges2colors',
              help_desc_edges2colors,
              {arg='image', type='torch.Tensor', help='input image (WxH or WxHx1 tensor)', req=true}
           )
           if img:nDimension() ~= 2 and img:nDimension() ~= 3 then
              xerror('image should be a WxH or WxHx1 tensor',nil,args.usage)
           end
           -- generate colors
           local colors = pink.uniqueColors
           if not colors then
              local nb_cols = 2^24
              colors = libpink.colors(nb_cols):t()
              pink.uniqueColors = torch.Tensor(nb_cols,4):copy(colors)
              colors = pink.uniqueColors
           end
           -- dims ?
           if img:nDimension() == 3 then img = img:selec(3,1) end
           -- edge 2 colors
           local ids = libpink.edges2ids(img, colors):add(1)
           -- convert ids into colors
           return image.maskToRGB(ids,colors)
        end

   pinkLoaded = true
end

return pink
