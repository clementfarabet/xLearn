local ImageTransform, parent = torch.class('nn.ImageTransform', 'nn.Module')

local help_desc = 
[[an image transformer
? provides typical RGB->YUV, YUV->RGB, RGB->Y transforms]]

local help_example = 
[[-- grab a frame from camera, resize it, and convert it to YUV
grabber = nn.ImageSource('camera')
resize = nn.ImageRescale(320,240,3)
converter = nn.ImageTransform('rgb2y')]]

function ImageTransform:__init(type)
   -- parent init
   parent.__init(self)
   
   -- error messages
   self.ERROR_UNKNOWN = "# ERROR: unknown transform"

   if not type then
      error(toolBox.usage('nn.ImageTransform',
                          help_desc,
                          help_example,
                          {type='string', help='transform = yuv2rgb | rgb2yuv | rgb2y', req=true}))
   end

   if type == 'yuv2rgb' then
      self.linear = nn.SpatialLinear(3,3)
      -- R
      self.linear.weight[1][1] = 1
      self.linear.weight[2][1] = 0
      self.linear.weight[3][1] = 1.13983
      self.linear.bias[1] = 0
      -- G
      self.linear.weight[1][2] = 1
      self.linear.weight[2][2] = -0.39465
      self.linear.weight[3][2] = -0.58060
      self.linear.bias[2] = 0
      -- B
      self.linear.weight[1][3] = 1
      self.linear.weight[2][3] = 2.03211
      self.linear.weight[3][3] = 0
      self.linear.bias[3] = 0
   elseif type == 'rgb2yuv' then
      self.linear = nn.SpatialLinear(3,3)
      -- Y
      self.linear.weight[1][1] = 0.299
      self.linear.weight[2][1] = 0.587
      self.linear.weight[3][1] = 0.114
      self.linear.bias[1] = 0
      -- U
      self.linear.weight[1][2] = -0.14713
      self.linear.weight[2][2] = -0.28886
      self.linear.weight[3][2] = 0.436
      self.linear.bias[2] = 0
      -- V
      self.linear.weight[1][3] = 0.615
      self.linear.weight[2][3] = -0.51499
      self.linear.weight[3][3] = -0.10001
      self.linear.bias[3] = 0
   elseif type == 'rgb2y' then
      self.linear = nn.SpatialLinear(3,1)
      -- Y
      self.linear.weight[1][1] = 0.299
      self.linear.weight[2][1] = 0.587
      self.linear.weight[3][1] = 0.114
      self.linear.bias[1] = 0
   else
      error(self.ERROR_UNKNOWN)
   end      
end

function ImageTransform:reset()
   -- do nothing
end

function ImageTransform:zeroGradParameters()
   -- do nothing
end

function ImageTransform:upgradeParameters()
   -- do nothing
end

function ImageTransform:forward(input)
   self.output = self.linear:forward(input)
   return self.output
end

function ImageTransform:backward()
   self.gradInput = self.linear:backward()
   return self.gradInput
end

function ImageTransform:write(file)
   parent.write(self, file)
   file:writeObject(self.linear)
end

function ImageTransform:read(file)
   parent.read(self, file)
   self.linear = file:readObject()
end