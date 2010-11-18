local ImageRescale, parent = torch.class('nn.ImageRescale', 'nn.Module')

local help_desc = 
[[an geometry rescaler for images
? can be inserted in larger modules to resize data on the fly]]

local help_example = 
[[-- grab a frame from camera, resize it, and convert it to YUV
grabber = nn.ImageSource('camera')
resize = nn.ImageRescale(320,240,3)
converter = nn.ImageTransform('rgb2y')]]

function ImageRescale:__init(...)
   -- parent init
   parent.__init(self)

   local args = {...}

   if not args[1] then
      error(toolBox.usage('nn.ImageRescale',
                          help_desc,
                          help_example,
                          {type='number', help='dimension 1', req=true},
                          {type='number', help='dimension 2', req=true},
                          {type='...', help='dimenstion N'}))
   end

   if type(args[1]) == 'number' then
      -- got a list of dimensions
      self.output:resize(...)
   else
      -- received a tensor as a template
      self.output:resizeAs(args[1])
   end
end

function ImageRescale:forward(input)
   if self.output:nDimension() ~= input:nDimension() then
      error('<ImageRescale:forward> inconsistent dimensions')
   end
   image.scale(input, self.output, 'bilinear')
   return self.output
end

function ImageRescale:backward(input, gradOutput)
   error('WARNING: backward not implemented')
   return self.gradInput
end
