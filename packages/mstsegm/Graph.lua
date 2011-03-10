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
   self.nbVertices = 0
   self.nbEdges = 0
   self.connexity = 0
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
