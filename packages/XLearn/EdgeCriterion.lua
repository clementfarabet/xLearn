local EdgeCriterion, parent = torch.class('nn.EdgeCriterion', 'nn.Criterion')

function EdgeCriterion:__init()
   local mstsegm = xrequire 'mstsegm'
   if not mstsegm then 
      xerror('module <mstsegm> is required', 'EdgeCriterion')
   end
   parent.__init(self)
   -- public
   self.threshold = 0.5
   self.nbPairs = 1
   self.randCycles = 1
   self.inputRange = 1
   self.loss = 'square'      -- square | square-square
   self.loss_margin = 0.1
   self.resampleTarget = 1
   -- private
   self.output = 1
   self.randCtr = 1
   self.onEdge = false
   self.edgeIdx = 1
   self.pairsSeen = 0
   self.edgesSeen = 0
end

function EdgeCriterion:adjustTarget(input, target)
   -- preprocess target ?
   local s = self.resampleTarget
   if (target:size(1)*self.resampleTarget) ~= input:size(1) then
      local w = input:size(1)/s
      local x = math.floor((target:size(1) - (input:size(1)-1)*1/s)/2) + 1
      local h = input:size(2)/s
      local y = math.floor((target:size(2) - (input:size(1)-1)*1/s)/2) + 1
      target = target:narrow(1,x,w):narrow(2,y,h)
   end
   if s ~= 1 then
      local targets = torch.Tensor(input:size(1), input:size(2))
      image.scale(target, targets, 'simple')
      target = targets
   end
   return target
end

function EdgeCriterion:forward(input, target)
   -- compute full Rand every K iterations
   if self.randCtr == self.randCycles then
      -- treat the input as edges of an edge-weighted graph
      self.input_graph = mstsegm.Graph{edges=input, connex=4}

      -- compute connected components
      self.input_cc = self.input_graph:toImage{threshold=self.threshold}

      -- adjust target
      target = self:adjustTarget(input, target)

      -- compute Rand index
      local rand = mstsegm.rand(self.input_cc, target)

      -- the output (error) = 1 - rand  (e.g. if segm is perfect, then rand = 1, error = 0)
      self.output = 1 - rand
   end

   -- update counter
   self.randCtr = self.randCtr + 1
   if self.randCtr > self.randCycles then self.randCtr = 1 end

   -- just return error
   return self.output
end

function EdgeCriterion:backward(input, target)
   -- derivate is 0 almost everywhere
   self.gradInput:resizeAs(input):zero()

   -- treat the input as edges of an edge-weighted graph
   self.input_graph = mstsegm.Graph{edges=input, connex=4}

   -- adjust target
   target = self:adjustTarget(input, target)

   -- get edges from target, and threshold it (values don't matter)
   self.target_graph = mstsegm.Graph{image=target, connex=4}
   self.target_graph:threshold(0.1)

   -- dims
   local width = self.target_graph.edges:size(1)
   local height = self.target_graph.edges:size(2)

   -- compute the gradient on N random pairs
   for i = 1,self.nbPairs do

      -- here we try to uniformly sample edges and non-edges targets
      local vertex1_x, vertex1_y

      -- to do so, we add random noise to the target graph: therefore getting
      -- its max will return new edges each time, getting its min will return
      -- new backgrounds each time
      local noise_map = lab.rand(width,height,2)
      noise_map:mul(0.1)
      noise_map:select(3,1):narrow(1,width,1):fill(0.5) -- last column should never be sampled
      noise_map:select(3,2):narrow(2,height,1):fill(0.5) -- last row should never be sampled
      local target_edges = self.target_graph.edges + noise_map

      -- get max or min:
      -- getting the max of this map usually returns the position of an edge, 
      -- the min the position of a flat area.
      -- if the map is flat (e.g. no edge), then max also returns a flat area
      local maxes,maxes_x,maxes_y,target_minimax
      if not self.onEdge then
         -- alternate between vert and hor edges
         if self.edgeIdx == 1 then self.edgeIdx = 2
         else self.edgeIdx = 1 end
         -- search edge
         if self.edgeIdx == 1 then
            maxes, maxes_x = lab.max(target_edges:select(3,self.edgeIdx):narrow(1,1,width-1))
         else
            maxes, maxes_x = lab.max(target_edges:select(3,self.edgeIdx):narrow(2,1,height-1))
         end
         maxes, maxes_y = lab.max(maxes:t())
         vertex1_y = maxes_y[1][1]
         vertex1_x = maxes_x[1][vertex1_y]
         target_minimax = self.target_graph.edges[vertex1_x][vertex1_y][self.edgeIdx]
      else
         -- search non-edge
         if self.edgeIdx == 1 then
            mins, mins_x = lab.min(target_edges:select(3,self.edgeIdx):narrow(1,1,width-1))
         else
            mins, mins_x = lab.min(target_edges:select(3,self.edgeIdx):narrow(2,1,height-1))
         end
         mins, mins_y = lab.min(mins:t())
         vertex1_y = mins_y[1][1]
         vertex1_x = mins_x[1][vertex1_y]
         target_minimax = self.target_graph.edges[vertex1_x][vertex1_y][self.edgeIdx]
      end

      -- edge or not ?
      if target_minimax == 1 then
         self.onEdge = true
         self.edgesSeen = self.edgesSeen + 1
      elseif target_minimax == 0 then
         self.onEdge = false
      else
         xerror('found target edge with weight ~= 1 | 0','EdgeCriterion')
      end
      self.pairsSeen = self.pairsSeen + 1

      -- the minimax is always the weight between the two neighbors
      local input_minimax
      if self.edgeIdx == 1 then -- hor edge
         input_minimax = self.input_graph.edges[vertex1_x][vertex1_y][1]
      else -- vert edge
         input_minimax = self.input_graph.edges[vertex1_x][vertex1_y][2]
      end

      -- which loss ?
      local dl_dx
      if self.loss == 'square' then
         -- l(t,x) = 0.5(x-t)^2
         -- and dl/dx = (x-t)
         dl_dx = (input_minimax - target_minimax*self.inputRange)

      elseif self.loss == 'square-square' then
         -- l(t,x) = t*max(0,1-x-m)^2 + (1-t)*max(0,x-m)^2
         -- and dl/dx = 2(x-(1-m))  if t = 1, x <  1-m
         --           = 0           if t = 1, x >= 1-m
         --           = 2(x-m)      if t = 0, x >  m
         --           = 0           if t = 0, x <= m
         local t = target_minimax
         local x = input_minimax/self.inputRange
         local m = self.loss_margin
         if t == 1 then
            if x < (1 - m) then dl_dx = 2 * (x - (1 - m))
            else dl_dx = 0
            end
         else
            if x > m then dl_dx = 2 * (x - m)
            else dl_dx = 0
            end
         end
         dl_dx = dl_dx * self.inputRange
      end

      -- propagate gradient to the right place
      if self.edgeIdx == 1 then -- hor edge
         self.gradInput[vertex1_x][vertex1_y][1] = dl_dx
      else -- vertical edge
         self.gradInput[vertex1_x][vertex1_y][2] = dl_dx
      end

      -- if flat target (e.g. no edges), then don't sample multiple times
      if self.target_graph.edges:max() == 0 then
         break
      end
   end

   -- return gradients
   return self.gradInput
end

function EdgeCriterion:write(file)
   parent.write(self, file)
   file:writeDouble(self.threshold)
   file:writeInt(self.nbPairs)
   file:writeInt(self.randCycles)
   file:writeDouble(self.inputRange)
   file:writeInt(((self.loss == 'square') and 1) or ((self.loss == 'square-square') and 2))
   file:writeDouble(self.loss_margin)
   file:writeDouble(self.resampleTarget)
end

function EdgeCriterion:read(file)
   local mstsegm = xrequire 'mstsegm'
   if not mstsegm then 
      xerror('module <mstsegm> is required', 'EdgeCriterion')
   end
   parent.read(self, file)
   self.threshold = file:readDouble()
   self.nbPairs = file:readInt()
   self.randCycles = file:readInt()
   self.inputRange = file:readDouble()
   self.loss = file:readInt()
   self.loss = ((self.loss == 1) and 'square') or ((self.loss == 2) and 'square-square')
   self.loss_margin = file:readDouble()
   self.resampleTarget = file:readDouble()
   self.output = 1
   self.randCtr = 1
   self.onEdge = false
   self.edgeIdx = 1
   self.pairsSeen = 0
   self.edgesSeen = 0
end
