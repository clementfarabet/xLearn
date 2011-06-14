local Module = torch.class('nn.Module')

function Module:__init()
   self.gradInput = torch.Tensor()
   self.output = torch.Tensor()
end

function Module:forward(input)
   return self.output
end

function Module:backward(input, gradOutput)
   return self.gradInput
end

function Module:zeroGradParameters()
end

function Module:updateParameters(learningRate)
end

function Module:decayParameters(decay)
end

function Module:empty()
   self.gradInput:resize()
   self.gradInput:storage():resize(0)
   self.output:resize()
   self.output:storage():resize(0)
end

function Module:write(file)
   file:writeObject(self.gradInput)
   file:writeObject(self.output)
end

function Module:read(file)
   self.gradInput = file:readObject()
   self.output = file:readObject()
end

function Module:writef(file,mode)
   local file = torch.DiskFile(file, 'w')
   if mode=='b' then file:binary() end
   self:write(file)
   file:close()
   return self
end

function Module:readf(file,mode)
   local file = torch.DiskFile(file, 'r')
   if mode=='b' then file:binary() end
   self:read(file)
   file:close()
   return self
end

function Module:writeb(file)
   return self:writef(file,'b')
end

function Module:readb(file)
   return self:readf(file,'b')
end

function Module:share(mlp, ...)
   local arg = {...}
   for i,v in ipairs(arg) do
      if self[v] ~=nil then self[v]:set(mlp[v]) end
   end
end

function Module:clone(...)
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
