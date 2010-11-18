local Max, parent = torch.class('nn.Max', 'nn.Module')

local help_desc =
[[Applies a max operation over dimension dimension. 
Hence, if an nxpxq Tensor was given as input, and 
dimension = 2 then an nxq matrix would be output.]]

function Max:__init(dimension)
   parent.__init(self)

   -- usage
   if dimension and type(dimension) ~= 'number' then
      error(toolBox.usage('nn.Max', help_desc, nil,
                          {type='number', help='dimension on which the max is done'}))
   end

   dimension = dimension or 2
   self.dimension = dimension
   self.indices = torch.Tensor()
end

function Max:write(file)
   parent.write(self, file)
   file:writeInt(self.dimension)
   file:writeObject(self.indices)
end

function Max:read(file)
   parent.read(self, file)
   self.dimension = file:readInt()
   self.indices = file:readObject()
end
