local CCAdd, parent = torch.class('nn.CCAdd', 'nn.Module')

function CCAdd:__init(args)
   parent.__init(self)

   -- usage
   if args then
      error(toolBox.usage('nn.CCAdd',
                          'a component-wise adder between two input tensors',
                          'adder = nn.CCAdd()\n'..
                             'addition = abs:forward{tensor1, tensor2}',
                          {type='nil', help='no arg required'}))
   end
end

function CCAdd:forward(inputs)
   self.output:resizeAs(inputs[2]):copy(inputs[2]):add(inputs[1])
   return self.output
end

function CCAdd:backward(input, gradOutput)
   error('WARNING: backward not implemented')
   return self.gradInput
end
