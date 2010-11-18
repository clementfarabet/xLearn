local ParzenDecider, parent = torch.class('nn.ParzenDecider', 'nn.Decider')

function ParzenDecider:__init()
   parent.__init(self)
end

function ParzenDecider:infer(input)
end

function ParzenDecider:learn(input, gradOutput)
end

function ParzenDecider:write(file)
   error( toolBox.NOT_IMPLEMENTED )
   parent.write(self,file)
end

function ParzenDecider:read(file)
   error( toolBox.NOT_IMPLEMENTED )
   parent.read(self,file)
end

function ParzenDecider:clone(...)
   error( toolBox.NOT_IMPLEMENTED )
end
