local RecursiveFoveaHessian, parent = torch.class('nn.RecursiveFoveaHessian', 'nn.RecursiveFovea')

-- Basically a container so I follow the SequentialHessian.lua
-- really nice that passing the ... works for the init(...)

function RecursiveFoveaHessian:__init(...)
   parent.__init(self,...)

   self.hessianPostProcessed = {}
   self.hessianUpsampledNarrowed = {}
   self.hessianUpsampledPadded = {}
   self.hessianUpsampled = {}
   self.hessianProcessed = {}
   self.hessianConcatenated = {}
   self.hessianNarrowed = {}
   self.hessianPadded = {}
   self.hessianPreProcessed = {}
   self.hessianPyramid = {}
   self.hessianPredicted = {}

   self.hessianInput = torch.Tensor()
end

function RecursiveFoveaHessian:configure(fov, sub, input_w, input_h)
   parent.configure(self, fov, sub, input_w, input_h)
   -- now redo the padders and upsamplers to hessian versions
      -- configure downsamplers, padders and upsamplers
   for idx = 1,nscales do
      -- downsamplers (for pyramid)
      local r = self.ratios[idx]
      if self.bilinear then
	 error('dont have a bilinear downsampler with hessian')
      else
         self.downsamplers[idx] = 
	    nn.SpatialSubSamplingHessian(self.nInputPlane, r, r, r, r)
         self.downsamplers[idx].weight:fill(1/(r^2))
         self.downsamplers[idx].bias:zero()
      end

      -- padders
      -- self.padders[idx] = nn.SpatialPaddingHessian(
      -- 	 self.padders[idx].padl, 
      -- 	 self.padders[idx].padr, 
      -- 	 self.padders[idx].padt, 
      -- 	 self.padders[idx].padb)
      -- self.upsampledPadders[idx] = nn.SpatialPaddingHessian(
      -- 	 self.upsampledPadders[idx].padl, 
      -- 	 self.upsampledPadders[idx].padr, 
      -- 	 self.upsampledPadders[idx].padt, 
      -- 	 self.upsampledPadders[idx].padb) 
   end
end

function RecursiveFoveaHessian:updateLearningRates(factor)
   for idx = 1,#self.processors do
      if self.processors[idx].updateLearningRates then
	 self.processors[idx]:updateLearningRates(factor)
      end
   end
end

function RecursiveFoveaHessian:boundLearningRates(mu)
   for idx = 1,#self.processors do
      if self.processors[idx].boundLearningRates then
	 self.processors[idx]:boundLearningRates(mu)
      end
   end
end

function RecursiveFoveaHessian:zeroHessianParameters()
   for idx = 1,#self.processors do
      if self.processors[idx].zeroHessianParameters then
         self.processors[idx]:zeroHessianParameters()
      end
   end
end

function RecursiveFoveaHessian:getMinMaxSecondDerivatives()
   local min = 1/0
   local max = -1/0
   for idx = 1,#self.processors do
      local currentModule = self.processors[idx]
	 -- check if the processor is a Sequential
      if currentModule.getMinMaxSecondDerivatives then
	 local cur_min, cur_max = 
	    currentModule:getMinMaxSecondDerivatives() 
	 min = math.min(min,cur_min)
	 max = math.max(max,cur_max)
      end
   end
   return min,max
end

-- this function is different because in the fovea we must bbprop
-- through the internal criterions, and the bbprop doesn't flow from
-- one scale to the other, but is recomputed by a new criterion at
-- each scale. Basically copies the fovea:backward() function
function RecursiveFoveaHessian:backwardHessian(input)
   -- local params
   local nscales = #self.ratios
   local fov = self.fov
   local sub = self.sub
   local corners = self.corners

   -- (9) backprop through criterions using generated targets (from
   -- prev forward call)
   for idx = 1,nscales do
      -- bprop through criterion
      self.hessianPredicted[idx] = 
	 self.criterions[idx]:backwardHessian(self.predicted[idx], 
					      self.targets[idx])

      -- then remap partial hessian vector 
      self.hessianPostProcessed[idx] = 
	 self.hessianPostProcessed[idx] or torch.Tensor()
      self.hessianPostProcessed[idx]:resizeAs(self.postProcessed[idx]):zero()
      if self.focused then
         local bs = self.batchSize
         self.hessianPostProcessed[idx]:narrow(1,corners[idx].x,bs):narrow(2,corners[idx].y,bs):copy(self.hessianPredicted[idx])
      else
         self.hessianPostProcessed[idx]:copy(self.hessianPredicted[idx])
      end
   end

   -- (8) backprop through post processors
   for idx = 1,nscales do
      if self.postProcessors[idx] then
         self.hessianProcessed[idx] = 
	    self.postProcessors[idx]:backwardHessian(self.processed[idx], 
						     self.hessianPostProcessed[idx])
      else
         self.hessianProcessed[idx] = self.hessianPostProcessed[idx]
      end
   end

   -- (7) recursive gradient: not done for now (needs to see if it's
   -- really worth it)
   --

   -- (6) backprop through processors
   for idx = 1,nscales do
      self.hessianConcatenated[idx] = 
	 self.processors[idx]:backwardHessian(self.concatenated[idx], 
					      self.hessianProcessed[idx])
   end

   -- (5) bprop through concatenators
   for idx = 1,nscales do
      self.hessianNarrowed[idx] = 
	 self.hessianConcatenated[idx]:narrow(3, 1, 
					      self.narrowed[idx]:size(3))
   end

   -- (4) bprop through narrow
   for idx = 1,nscales do
      self.hessianPadded[idx] = self.hessianPadded[idx] or torch.Tensor()
      self.hessianPadded[idx]:resizeAs(self.padded[idx]):zero()
      self.hessianPadded[idx]:narrow(1,1,self.narrowed_size[idx].w):narrow(2,1,self.narrowed_size[idx].h):copy(self.hessianNarrowed[idx])
   end

   -- (3) bprop through padders
   for idx = 1,nscales do
      self.hessianPreProcessed[idx] = self.padders[idx]:backwardHessian(self.preProcessed[idx], self.hessianPadded[idx])
   end

   -- (2) bprop through preProcessors
   for idx = 1,nscales do
      if self.preProcessors[idx] then
         self.hessianPyramid[idx] = self.preProcessors[idx]:backward(self.pyramid[idx], self.hessianPreProcessed[idx])
      else
         self.hessianPyramid[idx] = self.hessianPreProcessed[idx]
      end
   end

   -- (1) bprop through pyramid
   self.hessianInput:resizeAs(input):zero()
   for idx = 1,nscales do
      local partialHessian = 
	 self.downsamplers[idx]:backwardHessian(input, 
						self.hessianPyramid[idx])
      self.hessianInput:add(partialHessian)
   end
   return self.hessianInput
end
