

local CSubTable, parent = torch.class('nn.CSubTable', 'nn.Module')

function CSubTable:__init()
   parent.__init(self)
   self.gradInput = {}
   self.gradInput[1] = torch.Tensor()
   self.gradInput[2] = torch.Tensor()
end

function CSubTable:forward(input)
   self.output:resizeAs(input[1]):copy(input[1])
   self.output:add(-1,input[2])
   return self.output
end

function CSubTable:backward(input, gradOutput)
   self.gradInput[1]:resizeAs(input[1]):copy(gradOutput)
   self.gradInput[2]:resizeAs(input[1]):copy(gradOutput):mul(-1)
   return self.gradInput
end


function CSubTable:write(file)
   parent.write(self, file)
end

function CSubTable:read(file)
   parent.read(self, file)
end
