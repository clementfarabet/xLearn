local Mean, parent = torch.class('nn.Mean', 'nn.Module')

local help_desc =
[[Applies a mean operation over dimension dimension. 
Hence, if an nxpxq Tensor was given as input, and 
dimension = 2 then an nxq matrix would be output.]]

function Mean:__init(dimension)
   parent.__init(self)

   -- usage
   if dimension and type(dimension) ~= 'number' then
      error(toolBox.usage('nn.Mean', help_desc, nil,
                          {type='number', help='dimension on which the mean is done'}))
   end

   dimension = dimension or 2
   self.dimension = dimension
end

function Mean:write(file)
   parent.write(self, file)
   file:writeInt(self.dimension)
end

function Mean:read(file)
   parent.read(self, file)
   self.dimension = file:readInt()
end
