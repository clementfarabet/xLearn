local Min, parent = torch.class('nn.Min', 'nn.Module')

local help_desc =
[[Applies a max operation over dimension dimension. 
Hence, if an nxpxq Tensor was given as input, and 
dimension = 2 then an nxq matrix would be output.]]

function Min:__init(dimension)
   parent.__init(self)

   -- usage
   if dimension and type(dimension) ~= 'number' then
      error(toolBox.usage('nn.Min', help_desc, nil,
                          {type='number', help='dimension on which the min is done'}))
   end

   dimension = dimension or 2
   self.dimension = dimension
   self.indices = torch.Tensor()
end

function Min:write(file)
   parent.write(self, file)
   file:writeInt(self.dimension)
   file:writeObject(self.indices)
end

function Min:read(file)
   parent.read(self, file)
   self.dimension = file:readInt()
   self.indices = file:readObject()
end
