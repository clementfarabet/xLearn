local CCSub, parent = torch.class('nn.CCSub', 'nn.Module')

function CCSub:__init(args)
   parent.__init(self)

   -- usage
   if args then
      error(toolBox.usage('nn.CCSub',
                          'a component-wise subtracter between two input tensors',
                          'subtracter = nn.CCSub()\n'..
                             'subtraction = subtracter:forward{tensor1, tensor2}',
                          {type='nil', help='no arg required'}))
   end
end

function CCSub:forward(inputs)
   self.output:resizeAs(inputs[2]):copy(inputs[2]):mul(-1):add(inputs[1])
   return self.output
end

function CCSub:backward(input, gradOutput)
   error('WARNING: backward not implemented')
   return self.gradInput
end
