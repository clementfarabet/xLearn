local ImageSource, parent = torch.class('nn.ImageSource', 'nn.Module')

local help_desc = 
[[an image/video source, to abstract cameras/videos/images]]

local help_example = 
[[-- grab a frame from camera, resize it, and convert it to YUV
grabber = nn.ImageSource('camera')
resize = nn.ImageRescale(320,240,3)
converter = nn.ImageTransform('rgb2y')]]

function ImageSource:__init(type, source, idx, nbuffers, fps)
   -- parent init
   parent.__init(self)

   if not type then
      error(toolBox.usage('nn.ImageSource',
                          help_desc,
                          help_example,
                          {type='string', help='source type = camera | video | lena', req=true},
                          {type='string', help='driver for camera = opencv | camiface | libv4l'},
                          {type='number', help='optional camera index (default is 0)'},
                          {type='number', help='optional number of buffers (v4l2 only, default is 1)'},
                          {type='number', help='optional frame rate (v4l2 only, default is 30)'}))
   end

   -- error messages
   self.ERROR_UNKNOWN = "# ERROR: unknown type"
   self.ERROR_NOTFOUND = "# ERROR: source not found"
   self.WARNING_NOCAMERA = "# WARNING: no camera found, defaulting to lena"
   self.ERROR_NOBACKWARD = '# ERROR: ImageSource has no gradient !!'
   
   -- extra args
   self.camidx = idx or 0
   self.nbuffers = nbuffers or 1
   self.fps = fps or 30

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
            if paths.dirp(paths.concat(paths.install_lua_path,'v4l')) then
               require 'libv4l'
               self.source = 'v4linux'
            elseif paths.dirp(paths.concat(paths.install_lua_path,'opencv')) then
               require 'opencv'
               self.source = 'cam-cv'
            else
               error('<nn.ImageSource> please install XLearn with Video4Linux or OpenCV bindings')
            end
         elseif toolBox.OS == 'macos' then
            if paths.dirp(paths.concat(paths.install_lua_path,'camiface')) then
               require 'camiface'
               self.source = 'cam-iface'
               self.camera = Camera()
            else
               error('<nn.ImageSource> please install XLearn with libcamiface support')
            end
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
      libopencv.captureFromCam(self.output,self.camidx)
   elseif self.source == 'v4linux' then
      self.output:resize(640,480,3)
      libv4l.grabFrame(self.output,'/dev/video'..self.camidx,self.nbuffers,self.fps)
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
