------------------------------------------------------------
-- A class to handle edge-weighted Graphs.
-- Restricted to local connexity graphs, typically built
-- out of images, image volumes (videos)
------------------------------------------------------------
local Graph = torch.class('mstsegm.Graph')

------------------------------------------------------------
-- Creates a Graph object
------------------------------------------------------------
function Graph:__init(...)
   local args, img, connex, sigma, dist, edges, vertices = toolBox.unpack(
      {...},
      'Graph.createFromImage', 
      'Creates a graph from an image.',
      {arg='image', type='torch.Tensor', help='input image (WxHxN or WxH tensor)'},
      {arg='connex', type='number', help='connexity (4 or 8, edges per vertex)', default=4},
      {arg='sigma', type='number', help='gaussian for preprocessing', default=0.8},
      {arg='dist', type='string', help='distance metric to use: euclid | angle', default='euclid'},
      {arg='edges', type='torch.Tensor', help='existing edges (WxHxK tensor, K=connex/2)'},
      {arg='vertices', type='torch.Tensor', help='existing vertices'}
   )
   self.edges = torch.Tensor()
   self.vertices = torch.Tensor()
   self.mst = nil
   self.nbVertices = 0
   self.nbEdges = 0
   self.connex = 0
   if img then
      self:createFromImage(...)
   elseif edges then
      self:createFromEdges(...)
   end
end

------------------------------------------------------------
-- Builds graph from an image
------------------------------------------------------------
function Graph:createFromImage(...)
   local args, img, connex, sigma, dist = toolBox.unpack(
      {...},
      'Graph.createFromImage', 
      'Creates a graph from an image.',
      {arg='image', type='torch.Tensor', help='input image (WxHxN or WxH tensor)', req=true},
      {arg='connex', type='number', help='connexity (4 or 8, edges per vertex)', default=4},
      {arg='sigma', type='number', help='gaussian for preprocessing', default=0.8},
      {arg='dist', type='string', help='distance metric to use: euclid | angle', default='euclid'}
   )
   if img:nDimension() == 2 then
      self.vertices:resize(img:size(1), img:size(2), 1)
      self.vertices:copy(img)
   elseif img:nDimension() == 3 then
      self.vertices:resizeAs(img):copy(img)
   else
      xerror('image should be a WxH or WxHxN tensor',nil,args.usage)  
   end
   if dist == 'euclid' then dist = 0
   elseif dist == 'angle' then dist = 1
   else xerror('dist should be one of: angle | euclid',nil,args.usage) end
   self.nbVertices = self.vertices:select(3,1):nElement()
   self.connex = connex
   self.edges, self.nbEdges = libmstsegm.image2graph(self.vertices, connex, dist)
   return self
end

------------------------------------------------------------
-- Builds graph from existing edges
------------------------------------------------------------
function Graph:createFromEdges(...)
   local args, edges, vertices = toolBox.unpack(
      {...},
      'Graph.createFromImage', 
      'Creates a graph from an image.',
      {arg='edges', type='torch.Tensor', help='existing edges (WxHxK tensor, K=connex/2)', req=true},
      {arg='vertices', type='torch.Tensor', help='existing vertices'}
   )
   if edges:nDimension() ~= 2 and edges:nDimension() ~= 3 then
      xerror('edges should be a WxHxK tensor',nil,args.usage)
   end
   self.connex = edges:size(3)*2
   self.edges:resizeAs(edges):copy(edges)
   self.nbEdges = self.edges:nElement()
   if vertices then
      self.vertices:resizeAs(vertices):copy(vertices)
      self.nbVertices = self.vertices:select(3,1):nElement()
   else
      self.vertices:resize(0)
      self.nbVertices = 0
   end
   return self
end

------------------------------------------------------------
-- Exports graph to image (connected components)
------------------------------------------------------------
function Graph:toImage(...)
   local args, threshold, color = toolBox.unpack(
      {...},
      'Graph.toImage', 
      'Exports an image, generated from the current edge-weighted graph',
      {arg='threshold', type='number', help='threshold', default=0.5},
      {arg='colorize', type='boolean', help='colorize IDs (if false, a simple ID map is returned)', default=false}
   )
   self.idmap = libmstsegm.graph2image(self.edges, threshold)
   local map = self.idmap
   if color then
      xrequire 'pink'
      self.colorMap = pink.colorMap()
      map = image.maskToRGB(map, pink.colorMap())
   end
   return map
end

------------------------------------------------------------
-- Thresholds graph
------------------------------------------------------------
function Graph:threshold(...)
   local args, threshold, color = toolBox.unpack(
      {...},
      'Graph.threshold', 
      'Thresholds the edge-weighted graph',
      {arg='threshold', type='number', help='threshold', default=0.5}
   )
   image.threshold(self.edges, threshold, 0, 0, 1)
   return self.edges
end

------------------------------------------------------------
-- Compute minimax on graph
------------------------------------------------------------
function Graph:minimax(...)
   local args, v1, v2, mst, show = toolBox.unpack(
      {...},
      'mstsegm.mst_minimaxDisparity',
      'Computes the minimax disparity between two vertices in the graph'
         .. 'if no vertex is provided, then the full matrix is computed',
      {arg='vertex1', type='number', help='a vertex in the graph'},
      {arg='vertex2', type='number', help='a second vertex in the graph'},
      {arg='mst', type='boolean', help='if true, recompute mst', default=true},
      {arg='show', type='boolean', help='show (display) minimax point', default=false}
   )
   if not self.mst or mst then
      self.mst = libmstsegm.mst(self.edges,0,0,self.connex)
   end
   if v1 and v2 and type(v1) == 'table' then
      v1 = (v1[2]-1)*self.edges:size(1) + v1[1]
      v2 = (v2[2]-1)*self.edges:size(1) + v2[1]
   end
   local w,e1,e2 = libmstsegm.mst_minimaxdisparity(self.mst,v1,v2)
   if show and v1 and self.connex == 4 then -- display only supported for connex 4
      -- render graph edges
      local surface = gfx.ImageSurface(self.edges:size(1)*2, self.edges:size(2))
      image.displayList{images=self.edges, window=surface, legends={'Horz Edges','Vert Edges'}}
      surface:batchBegin()
      -- horizontal edge:
      if math.max(e1,e2) == math.min(e1,e2)+1 then
         local off = math.min(e1,e2)-1
         local offy = math.floor(off/self.edges:size(1))
         local offx = off-offy*self.edges:size(1)
         offx = offx+1
         offy = offy+1
         -- minimax:
         local circ1 = surface:circle(offx,offy)
         circ1:set('penColor',{1,0,0})
         circ1:set('markerSize',1)
         -- vertices:
         v1 = v1-1
         offy = math.floor(v1/self.edges:size(1))
         offx = v1-offy*self.edges:size(1)
         offx = offx+1
         offy = offy+1
         local circ2 = surface:circle(offx,offy)
         circ2:set('penColor',{0,1,0})
         circ2:set('markerSize',2)
         v2 = v2-1
         offy = math.floor(v2/self.edges:size(1))
         offx = v2-offy*self.edges:size(1)
         offx = offx+1
         offy = offy+1
         local circ3 = surface:circle(offx,offy)
         circ3:set('penColor',{0,1,0})
         circ3:set('markerSize',2)
      -- vertical edge:
      else
         local off = math.min(e1,e2)-1
         local offy = math.floor(off/self.edges:size(1))
         local offx = off-offy*self.edges:size(1)
         offx = offx+1
         offy = offy+1
         -- minimax:
         local circ1 = surface:circle(offx+self.edges:size(1),offy)
         circ1:set('penColor',{1,0,0})
         circ1:set('markerSize',1)
         -- vertices:
         v1 = v1-1
         offy = math.floor(v1/self.edges:size(1))
         offx = v1-offy*self.edges:size(1)
         offx = offx+1
         offy = offy+1
         local circ2 = surface:circle(offx+self.edges:size(1),offy)
         circ2:set('penColor',{0,1,0})
         circ2:set('markerSize',2)
         v2 = v2-1
         offy = math.floor(v2/self.edges:size(1))
         offx = v2-offy*self.edges:size(1)
         offx = offx+1
         offy = offy+1
         local circ3 = surface:circle(offx+self.edges:size(1),offy)
         circ3:set('penColor',{0,1,0})
         circ3:set('markerSize',2)
      end
      surface:batchEnd()
      surface:redraw()
      image.display{image=surface.lcairo_object:toTensor(), legend='Minimax Path'}
   end
   return w,e1,e2
end

------------------------------------------------------------
-- Display method
------------------------------------------------------------
function Graph:__show()
   local zoom = 1
   if self.edges:size(1) > 400 then
      zoom = 400 / self.edges:size(1)
   end
   image.displayList{images=self.edges, 
                     zoom=zoom,
                     win_w=self.edges:size(1)*2*zoom,
                     win_h=self.edges:size(2)*zoom,
                     font=12,
                     legend='mstsegm.Graph [connex = ' .. self.connex .. ']',
                     legends={'horizontal','vertical','diag left','diag right'}}
end
