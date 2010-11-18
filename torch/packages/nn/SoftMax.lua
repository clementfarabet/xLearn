local SoftMax, parent = torch.class('nn.SoftMax', 'nn.Module')

local help_desc = 
[[Applies the Softmax function to an n-dimensional
input Tensor, rescaling them so that the elements 
of the n-dimensional output Tensor lie in the range
(0,1) and sum to 1.

Softmax is defined as:
f_i(x) = exp(x_i-shift) / sum_j exp(x_j-shift), 
where shift = max_i x_i if self.computeShift is 
true (default).

if self.computeShift is set to false, then shift 
should be specified with self.shift.]]

function SoftMax:__init(args)
   parent.__init(self)

   -- usage
   if args then
      error(toolBox.usage('nn.SoftMax', help_desc, nil,
                          {type='nil', help='no arg required'}))
   end

   self.shift = 0
   self.computeShift = true
end

function SoftMax:write(file)
   parent.write(self, file)
   file:writeDouble(self.shift)
   file:writeBool(self.computeShift)
end

function SoftMax:read(file)
   parent.read(self, file)
   self.shift = file:readDouble()
   self.computeShift = file:readBool()
end
