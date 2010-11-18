local ImageSource, parent = torch.class('nn.ImageSource', 'nn.Module')

local help_desc = 
[[an image/video source, to abstract cameras/videos/images]]

local help_example = 
[[-- grab a frame from camera, resize it, and convert it to YUV
grabber = nn.ImageSource('camera')
resize = nn.ImageRescale(320,240,3)
converter = nn.ImageTransform('rgb2y')]]

function ImageSource:__init(type, source)
   -- parent init
   parent.__init(self)

   if not type then
      error(toolBox.usage('nn.ImageSource',
                          help_desc,
                          help_example,
                          {type='string', help='source type = camera | video | lena', req=true},
                          {type='string', help='extra info for camera = opencv | camiface | libv4l'}))
   end

   -- error messages
   self.ERROR_UNKNOWN = "# ERROR: unknown type"
   self.ERROR_NOTFOUND = "# ERROR: source not found"
   self.WARNING_NOCAMERA = "# WARNING: no camera found, defaulting to lena"
   self.ERROR_NOBACKWARD = '# ERROR: ImageSource has no gradient !!'
   
   if type == 'camera' then
      if source == 'opencv' then
         require 'opencv'
         self.source = 'cam-cv'
      elseif source == 'v4linux' then
         require 'libv4l'
         self.source = 'v4linux'
      elseif source == 'camiface' then
         require 'camiface'
         self.source = 'cam-iface'
         self.camera = Camera()
      else
         -- no source specified, auto-detect
         if toolBox.OS == 'linux' then
            require 'opencv'
            self.source = 'cam-cv'
         elseif toolBox.OS == 'macos' then
            require 'camiface'
            self.source = 'cam-iface'
            self.camera = Camera()
         else
            print(self.WARNING_NOCAMERA)
            self.source = 'lena'
         end
      end
   elseif type == 'video' then
      error(self.ERROR_UNKNOWN)
   elseif type == 'lena' then
      self.source = 'lena'
   else
      error(self.ERROR_UNKNOWN)
   end      
end

function ImageSource:forward()
   if self.source == 'cam-cv' then
      self.output:resize(640,480,3)
      libopencv.captureFromCam(self.output)
   elseif self.source == 'v4linux' then
      self.output:resize(640,480,3)
      libv4l.grabFrame(self.output)
   elseif self.source == 'cam-iface' then
      self.output:resize(640,480,3)
      self.camera:getFrame(self.output)
   elseif self.source == 'lena' then
      self.output:resize(640,480,3)
      if not self.lena then
         self.lena = image.lena()
         image.scale(self.lena, self.output, 'bilinear')
      end
   end
   return self.output
end

function ImageSource:backward()
   error(self.ERROR_NOBACKWARD)
end

function ImageSource:write(file)
   parent.write(self, file)
   
end

function ImageSource:read(file)
   parent.read(self, file)
end
