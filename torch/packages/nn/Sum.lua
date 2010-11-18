local Sum, parent = torch.class('nn.Sum', 'nn.Module')

local help_desc =
[[Applies a sum operation over dimension dimension.
Hence, if an nxpxq Tensor was given as input, and 
dimension = 2 then an nxq matrix would be output.]]

function Sum:__init(dimension)
   parent.__init(self)

   -- usage
   if dimension and type(dimension) ~= 'number' then
      error(toolBox.usage('nn.Sum', help_desc, nil,
                          {type='number', help='dimension on which the sum is done'}))
   end

   dimension = dimension or 2
   self.dimension = dimension
end

function Sum:write(file)
   parent.write(self, file)
   file:writeInt(self.dimension)
end

function Sum:read(file)
   parent.read(self, file)
   self.dimension = file:readInt()
end
