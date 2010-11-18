local LogSigmoid, parent = torch.class('nn.LogSigmoid', 'nn.Module')

local help_desc =
[[Applies the LogSigmoid function to an n-dimensioanl input Tensor.

LogSigmoid is defined as f_i(x) = log(1/(1+ exp(-x_i))).]]

function LogSigmoid:__init(args)
   parent.__init(self)

   -- usage
   if args then
      error(toolBox.usage('nn.LogSigmoid', help_desc, nil,
                          {type='nil', help='no arg required'}))
   end

   self.buffer = torch.Tensor()
end

function LogSigmoid:write(file)
   parent.write(self, file)
   file:writeObject(self.buffer)
end

function LogSigmoid:read(file)
   parent.read(self, file)
   self.buffer = file:readObject()
end
