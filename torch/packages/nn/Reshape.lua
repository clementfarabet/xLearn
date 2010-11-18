local Reshape, parent = torch.class('nn.Reshape', 'nn.Module')

local help_desc = 
[[Reshapes an nxpxqx.. Tensor into a dimension1xdimension2x... Tensor, 
taking the elements column-wise.]]

local help_example =
[[> x=torch.Tensor(4,4)
> for i=1,4 do
>  for j=1,4 do
>   x[i][j]=(i-1)*4+j;
>  end
> end
> print(x)

  1   2   3   4
  5   6   7   8
  9  10  11  12
 13  14  15  16
[torch.Tensor of dimension 4x4]

> print(nn.Reshape(2,8):forward(x))

  1   9   2  10   3  11   4  12
  5  13   6  14   7  15   8  16
[torch.Tensor of dimension 2x8]

> print(nn.Reshape(8,2):forward(x))

  1   3
  5   7
  9  11
 13  15
  2   4
  6   8
 10  12
 14  16
[torch.Tensor of dimension 8x2]

> print(nn.Reshape(16):forward(x))

  1
  5
  9
 13
  2
  6
 10
 14
  3
  7
 11
 15
  4
  8
 12
 16
[torch.Tensor of dimension 16] ]]

function Reshape:__init(...)
   parent.__init(self)

   -- usage
   local args = {...}
   if not args[1] then
      error(toolBox.usage('nn.Reshape', help_desc, help_example,
                          {type='number', help='dimension 1', req=true},
                          {type='number', help='dimension 2'},
                          {type='...', help='dimension N'}))
   end

   self.size = torch.LongStorage()
   local n = select('#', ...)
   if n == 1 and torch.typename(select(1, ...)) == 'torch.LongStorage' then
      self.size:resize(#select(1, ...)):copy(select(1, ...))
   else
      self.size:resize(n)
      for i=1,n do
         self.size[i] = select(i, ...)
      end
   end
   self.output:resize(self.size)
end

function Reshape:forward(input)
   return self.output:copy(input)
end

function Reshape:backward(input, gradOutput)
   self.gradInput:resizeAs(input)
   return self.gradInput:copy(gradOutput)
end

function Reshape:write(file)
   parent.write(self, file)
   file:writeObject(self.size)
end

function Reshape:read(file, version)
   parent.read(self, file)
   if version > 0 then
      self.size = file:readObject()
   else
      local size = file:readObject()
      self.size = torch.LongStorage(size:size())
      self.size:copy(size)
   end
end
