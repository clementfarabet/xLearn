local Decider = torch.class('nn.Decider')

function Decider:__init()
   self.gradInput = torch.Tensor()
   self.output = torch.Tensor()
end

function Decider:infer(input)
end

function Decider:learn(input, gradOutput)
end

function Decider:write(file)
   file:writeObject(self.gradInput)
   file:writeObject(self.output)
end

function Decider:read(file)
   self.gradInput = file:readObject()
   self.output = file:readObject()
end

function Decider:clone(...)
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
