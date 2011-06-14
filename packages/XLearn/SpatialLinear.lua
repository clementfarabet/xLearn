local SpatialLinear, parent = torch.class('nn.SpatialLinear', 'nn.Module')

function SpatialLinear:__init(fanin, fanout)
   parent.__init(self)

   self.fanin = fanin or 1
   self.fanout = fanout or 1

   self.weightDecay = 0   
   self.weight = torch.Tensor(self.fanin, self.fanout)
   self.bias = torch.Tensor(self.fanout)
   self.gradWeight = torch.Tensor(self.fanin, self.fanout)
   self.gradBias = torch.Tensor(self.fanout)
   
   self.output = torch.Tensor(1,1,fanout)
   self.gradInput = torch.Tensor(1,1,fanin)

   self:reset()
end

function SpatialLinear:reset(stdv)
   if stdv then
      stdv = stdv * math.sqrt(3)
   else
      stdv = 1./math.sqrt(self.weight:size(1))
   end

   -- we do this so the initialization is exactly
   -- the same than in previous torch versions
   for i=1,self.weight:size(2) do
      self.weight:select(2, i):apply(function()
                                        return random.uniform(-stdv, stdv)
                                     end)
      self.bias[i] = random.uniform(-stdv, stdv)
   end
end

function SpatialLinear:zeroGradParameters()
   self.gradWeight:zero()
   self.gradBias:zero()
end

function SpatialLinear:updateParameters(learningRate)
   self.weight:add(-learningRate, self.gradWeight)
   self.bias:add(-learningRate, self.gradBias)
end

function SpatialLinear:decayParameters(decay)
   self.weight:add(-decay, self.weight)
   self.bias:add(-decay, self.bias)
end

function SpatialLinear:forward(input)
   -- resize output
   self.output:resize(input:size(1), input:size(2), self.fanout)
   if (input:size(3) ~= self.fanin) then
      error('# ERROR: SpatialLinear input has wrong dim 3')
   end

   -- dot product scan
   for k = 1,self.fanout do
      local outk = self.output:select(3,k)
      outk:fill(self.bias[k])
      for l = 1,self.fanin do
         outk:add(self.weight[l][k], input:select(3,l))
      end
   end
   
   return self.output
end

function SpatialLinear:backward(input, gradOutput)
   -- resize input
   self.gradInput:resize(input:size(1), input:size(2), self.fanin)
   
   for x=1,input:size(1) do
      for y=1,input:size(2) do
         self.gradWeight:addT1outT1(1, input[x][y], gradOutput[x][y])
         self.gradBias:add(gradOutput[x][y])
         
         if self.weightDecay ~= 0 then
            self.gradWeight:add(self.weightDecay, self.weight)
         end
         
         self.gradInput[x][y]:zero()
         self.gradInput[x][y]:addT2dotT1(1, self.weight, gradOutput[x][y])
      end
   end
   
   return self.gradInput
end

function SpatialLinear:write(file)
   parent.write(self, file)
   file:writeInt(self.fanin)
   file:writeInt(self.fanout)
   file:writeDouble(self.weightDecay)
   file:writeObject(self.weight)
   file:writeObject(self.bias)
   file:writeObject(self.gradWeight)
   file:writeObject(self.gradBias)
end

function SpatialLinear:read(file)
   parent.read(self, file)
   self.fanin = file:readInt()
   self.fanout = file:readInt()
   self.weightDecay = file:readDouble()
   self.weight = file:readObject()
   self.bias = file:readObject()
   self.gradWeight = file:readObject()
   self.gradBias = file:readObject()
end
