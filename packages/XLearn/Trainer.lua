local Trainer = torch.class('nn.Trainer')

function Trainer:__init()
   self.learningRate = 0.01
   self.learningRateDecay = 0
   self.maxIteration = 25
   self.shuffleIndices = true
end

function Trainer:train(dataset)
end

function Trainer:write(file)
   file:writeDouble(self.learningRate)
   file:writeDouble(self.learningRateDecay)
   file:writeInt(self.maxIteration)
   file:writeBool(self.shuffleIndices)
end

function Trainer:read(file)
   self.learningRate = file:readDouble()
   self.learningRateDecay = file:readDouble()
   self.maxIteration = file:readInt()
   self.shuffleIndices = file:readBool()
end

function Trainer:share(mlp, ...)
   for i,v in ipairs(arg) do
      if self[v] ~=nil then self[v]:set(mlp[v]) end
   end
end

function Trainer:setShuffle(bool)
      self.shuffleIndices = bool
end

function Trainer:clone(...)
   local f = torch.MemoryFile("rw"):binary()
   f:writeObject(self)
   f:seek(1)
   local clone = f:readObject()
   f:close()
   if select('#',...) > 0 then
      clone:share(self,...)
   end
   return clone
end
