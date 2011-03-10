
local Mult, parent = torch.class('nn.Mult','nn.Module')

function Mult:__init(val)
   parent.__init(self)
   self.val = val or 1.0
end

function Mult:forward(input)
   self.output:resizeAs(input):copy(input)
   self.output:mul(self.val)
   return self.output
end

function Mult:backward(input, gradOutput)
   self.gradInput:resizeAs(input):copy(gradOutput)
   self.gradInput:zero()
   error('not implemented !!!')
   return self.gradInput
end

function Mult:write(file)
   parent.write(self,file)
   file:writeDouble(self.val)
end

function Mult:read(file)
   parent.read(self,file)
   self.val = file:readDouble()
end
