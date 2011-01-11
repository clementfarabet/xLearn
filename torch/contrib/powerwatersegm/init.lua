--
-- Copyright ESIEE (2009) 
-- 
-- Author :
-- Camille Couprie (c.couprie@esiee.fr)
-- 
-- Contributors : 
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
-- Note: this bit of code is a simple wrapper around the graph-based
-- image segmentation algorithms provided/published by Couprie et al.:
--
-- Camille Couprie, Leo Grady, Laurent Najman, Hugues Talbot,
-- "Power Watershed: A Unifying Graph-Based Optimization Framework",
-- PAMI, 2010
--
-- The code has been modified in several aspects, to accomodoate Torch.
--
-- Wrapper: Clement Farabet.
-- 

-- load C lib
require 'libpowerwatersegm'

local infer_help_desc =
[[Computes a segmentation map given an input image, and a list
of seeds.

The map is computed using power watersheds, as described in
"Power Watershed: A Unifying Graph-Based Optimization Framework", by
Camille Couprie, Leo Grady, Laurent Najman, Hugues Talbot.

The input image must be a WxHxN tensor, where N is the number
of channels (colors). N must be 3 or 1.

The list of seeds is a simple Lua table, of the following form:
seeds = {{x1,y1,C1}, {x2,y2,C2}, ...}

Each entry in the table is a tripled specifying the the 
(x,y) location, followed by the class (~= 0). ]]

local usage = 
toolBox.usage('powerwatersegm.infer',
              infer_help_desc,
              nil,
              {arg='image', type='torch.Tensor', help='input image [WxHxN tensor]'},
              {arg='seeds', type='table', help='list of seeds {{x1,y1,C1}, ...}'},
              {arg='algo', type='string', help='type of algo: Kruskal, PWq2 or Prim [default PWq2]'},
              {arg='geodesics', type='boolean', help='use geodesics [default=false]'})

local col = toolBox.COLORS

if not powerwatersegmLoaded then
   require 'XLearn'
   powerwatersegm = {}

   -- register functions
   powerwatersegm.infer = 
      function(args)
         -- get args
         args = args or {}
         local img = args.image
         local seeds = args.seeds
         local algo = args.algo or 2
         local geodesics = args.geodesics or false

         -- check args
         if not img or img:nDimension() ~= 3
                or (img:size(3) ~= 3 and img:size(3) ~= 1)
                or not seeds then
                error(usage)
         end

         -- transform args
         local tseeds = torch.Tensor(#seeds,3)
         for i,seed in ipairs(seeds) do 
            tseeds[i][1] = seed[1]
            tseeds[i][2] = seed[2]
            tseeds[i][3] = seed[3]
            if seed[1] > img:size(1) or seed[1] < 1 or seed[2] > img:size(2) or seed[2] < 1 then
               print(col.Red .. 'seeds are incorrect, please check range' .. col.none)
               error(usage)
            end
         end
         if algo == 'kruskal' or algo == 'Kruskal' or algo == 1 then algo = 1
         elseif algo == 'PWq2' or algo == 2 then algo = 2
         elseif algo == 'prim' or algo == 'Prim' or algo == 3 then algo = 3
         else error(usage) end

         -- compute
         return libpowerwatersegm.infer(img, tseeds, algo, geodesics)
      end

   powerwatersegm.testme = 
      function()
         local img = image.lena(350,350)
         local seeds = { {200,200,1},
                         {200,350,2},
                         {200,100,3},
                         {80,300,4},
                         {50,50,5},
                         {140,200,6},
                         {50,350,7},
                         {345,345,8},
                         {170,345,9},
                      }
         local res = {}
         local qtz = {}
         local colors = image.createColorMap(256)

         for i = 1,3 do
            res[i] = powerwatersegm.infer{image = img,
                                          seeds = seeds,
                                          algo = 1}
            temp = res[i]:mul(255):add(1):floor()
            qtz[i] = torch.Tensor(temp:size(1), temp:size(2), 3)
            image.maskToRGB(temp,colors,qtz[i])
         end
         
         image.displayList{images={img, img, img,
                                   qtz[1], qtz[2], qtz[3]},
                           win_w=1050, win_h=700,
                           legend='Segmentation [Power Watershed]',
                           legends={'input', 'input', 'input',
                                    'Kruskal', 'PW q=2', 'Prim'}}
      end

   powerwatersegmLoaded = true
end

return powerwatersegm
