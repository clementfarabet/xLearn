local MalisCriterion, parent = torch.class('nn.MalisCriterion', 'nn.Criterion')

function MalisCriterion:__init()
   local mstsegm = xrequire 'mstsegm'
   if not mstsegm then 
      xerror('module <mstsegm> is required to use Malis Criterion', 'MalisCriterion')
   end
   parent.__init(self)
   self.output = 1
   self.thresholdedInput = torch.Tensor()
   self.threshold = 0.5
   self.nbPairs = 1
   self.randCycles = 1
   self.randCtr = 1
end

function MalisCriterion:forward(input, target)
   -- compute full Rand every K iterations
   if self.randCtr == self.randCycles then
      -- treat the input as edges of an edge-weighted graph
      self.input_graph = mstsegm.Graph{edges=input, connex=4}

      -- compute connected components
      self.input_cc = self.input_graph:toImage{threshold=threshold}

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

   -- get edges from target, and threshold it (values don't matter)
   self.target_graph = mstsegm.Graph{image=target, connex=4}
   self.target_graph:threshold(0.1)

   -- compute minimum spanning tree of target and input
   self.input_mst = mstsegm.mst{graph=self.input_graph, connex=4}
   self.target_mst = mstsegm.mst{graph=self.target_graph, connex=4}

   -- compute the gradient on N random pairs
   for i = 1,self.nbPairs do
      -- sample random vertices, and compute their maximin affinity (minimax disparity)
      local vertex1 = math.floor(random.uniform(1.5, target:nElement() + 0.5))
      local vertex2 = math.floor(random.uniform(1.5, target:nElement() + 0.5))
      local input_minimax, i_v1, i_v2 = mstsegm.mst_minimaxDisparity(self.input_mst, vertex1, vertex2)
      local target_minimax = mstsegm.mst_minimaxDisparity(self.target_mst, vertex1, vertex2)

      -- using square loss l(x,t) = 0.5(x-t)^2
      -- we compute dl/di = (x-t) * dx/di
      local dloss_dinput = (input_minimax - target_minimax)

      -- derivate input_minimax wrt its weight
      if math.max(i_v2,i_v1) == math.min(i_v2,i_v1)+1 then -- horizontal edge
         local off = math.min(i_v2,i_v1)
         self.gradInput:select(3,1):storage()[off] = dloss_dinput
      else -- vertical edge
         local off = math.min(i_v2,i_v1)
         self.gradInput:select(3,2):storage()[off] = dloss_dinput
      end
   end

   -- return gradients
   return self.gradInput
end
