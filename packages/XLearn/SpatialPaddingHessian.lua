local SpatialPaddingHessian, parent = torch.class('nn.SpatialPaddingHessian', 'nn.SpatialPadding')

function SpatialPaddingHessian:__init(pad_l, pad_r, pad_t, pad_b)
   parent.__init(self,pad_l, pad_r, pad_t, pad_b)
   self.hessianInput = torch.Tensor()
end

-- this is the same func as backward except that it operates on a
-- different matrix.  Seems crazy to make a new object so I am putting
-- the function in here [MS]
function SpatialPaddingHessian:backwardHessian(input, hessianOutput)
   self.hessianInput:resizeAs(input):zero()
   -- crop hessianInput if necessary
   local cg_input = self.hessianInput
   if self.pad_l < 0 then cg_input = cg_input:narrow(1, 1 - self.pad_l, cg_input:size(1) + self.pad_l) end
   if self.pad_r < 0 then cg_input = cg_input:narrow(1, 1, cg_input:size(1) + self.pad_r) end
   if self.pad_t < 0 then cg_input = cg_input:narrow(2, 1 - self.pad_t, cg_input:size(2) + self.pad_t) end
   if self.pad_b < 0 then cg_input = cg_input:narrow(2, 1, cg_input:size(2) + self.pad_b) end
   -- crop hessianOutout if necessary
   local cg_output = hessianOutput
   if self.pad_l > 0 then cg_output = cg_output:narrow(1, 1 + self.pad_l, cg_output:size(1) - self.pad_l) end
   if self.pad_r > 0 then cg_output = cg_output:narrow(1, 1, cg_output:size(1) - self.pad_r) end
   if self.pad_t > 0 then cg_output = cg_output:narrow(2, 1 + self.pad_t, cg_output:size(2) - self.pad_t) end
   if self.pad_b > 0 then cg_output = cg_output:narrow(2, 1, cg_output:size(2) - self.pad_b) end
   -- copy hessianOuput to hessianInput
   cg_input:copy(cg_output)
   return self.hessianInput
end


