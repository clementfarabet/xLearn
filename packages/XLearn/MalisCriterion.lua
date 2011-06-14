local MalisCriterion, parent = torch.class('nn.MalisCriterion', 'nn.EdgeCriterion')

function MalisCriterion:__init()
   parent.__init(self)
   self.maxSamplings = self.nbPairs*50
end

function MalisCriterion:forward(input, target)
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

function MalisCriterion:backward(input, target)
   -- derivate is 0 almost everywhere
   self.gradInput:resizeAs(input):zero()

   -- treat the input as edges of an edge-weighted graph
   self.input_graph = mstsegm.Graph{edges=input, connex=4}

   -- adjust target
   target = self:adjustTarget(input, target)

   -- get edges from target, and threshold it (values don't matter)
   self.target_graph = mstsegm.Graph{image=target, connex=4}
   self.target_graph:threshold(0.1)

   -- compute minimum spanning tree of target and input
   self.input_mst = mstsegm.mst{graph=self.input_graph, connex=4}
   self.target_mst = mstsegm.mst{graph=self.target_graph, connex=4}

   -- compute the gradient on N random pairs
   for i = 1,self.nbPairs do

      -- if flat target (e.g. no edges), then simply bypass
      if self.target_graph.edges:max() == 0 then
         break
      end

      -- sample random vertices, and compute their maximin affinity (minimax disparity)
      -- the while loop is used to force equal sampling (this can be painfully inefficient)
      local input_minimax, i_v1, i_v2
      local target_minimax
      local nbSamplings = 0
      while true do
         local vertex1 = math.random(1,target:nElement())
         local vertex2 = math.random(1,target:nElement())
         while vertex1 == vertex2 do
            vertex2 = math.random(1,target:nElement())
         end
         input_minimax, i_v1, i_v2 = mstsegm.mst_minimaxDisparity(self.input_mst, vertex1, vertex2)
         target_minimax = mstsegm.mst_minimaxDisparity(self.target_mst, vertex1, vertex2)
         if (self.onEdge and target_minimax == 0) or (not self.onEdge and target_minimax == 1) then
            break
         end
         nbSamplings = nbSamplings + 1
         if nbSamplings == self.maxSamplings then 
            break 
         end
      end

      -- edge or not ?
      if target_minimax == 1 then
         self.onEdge = true
         self.edgesSeen = self.edgesSeen + 1
      elseif target_minimax == 0 then
         self.onEdge = false
      else
         xerror('found target edge with weight ~= 1 | 0','MalisCriterion')
      end
      self.pairsSeen = self.pairsSeen + 1

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

      -- derivate input_minimax wrt its weight
      if math.max(i_v2,i_v1) == math.min(i_v2,i_v1)+1 then -- horizontal edge
         local off = math.min(i_v2,i_v1)
         self.gradInput:select(3,1):storage()[off] = dl_dx
      else -- vertical edge
         local off = math.min(i_v2,i_v1)
         self.gradInput:select(3,2):storage()[off] = dl_dx
      end
   end

   -- return gradients
   return self.gradInput
end
