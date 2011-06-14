local ImageSource, parent = torch.class('nn.ImageSource', 'nn.Module')

local help_desc = [[
An image/video source, to abstract cameras/videos/images.

-- grab a frame from camera, resize it, and convert it to YUV
grabber = nn.ImageSource('camera')
resize = nn.ImageRescale(320,240,3)
converter = nn.ImageTransform('rgb2y')]]

function ImageSource:__init(...)
   -- parent init
   parent.__init(self)

   -- parse args
   local args, type, source, idx, nbuffers, fps, length, width, height, path = toolBox.unpack(
      {...},
      'ImageSource', help_desc,
      {arg='type', type='string', help='source type = camera | video | lena', req=true},
      {arg='driver', type='string', help='camera driver: opencv | camiface | libv4'},
      {arg='cam_idx', type='number', help='optional camera index', default=0},
      {arg='cam_buffers', type='number', help='optional number of buffers (v4l2 only)', default=1},
      {arg='fps', type='number', help='optional frame rate (v4l2 + video file)', default=30},
      {arg='length', type='number', help='optional length (video file only, seconds)', default=5},
      {arg='width', type='number', help='width', default=640},
      {arg='height', type='number', help='height', default=480},
      {arg='path', type='string', help='path to video, for source == video'}
   )

  


   -- store args
   local defs = {...}
   if(defs[3]) then
      self.camidx = defs[3]--idx
   else self.camidx = idx end
   self.nbuffers = nbuffers
   self.fps = fps
   self.length = length
   self.width = width
   self.height = height

   -- default VGA buffer (for camera drivers)
   self.VGA = torch.Tensor(640, 480, 3)

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
      print('loading video from file ' .. path)
      self.video = Video{path=path,fps=fps,length=length,width=width,height=height}
      self.source = 'video'
   elseif type == 'lena' then
      self.source = 'lena'
   else
      error(self.ERROR_UNKNOWN)
   end      
end

function ImageSource:forward()
   self.output:resize(self.width,self.height,3)
   if self.source == 'cam-cv' then
      libopencv.captureFromCam(self.VGA,self.camidx)
      image.scale(self.VGA, self.output, 'bilinear')
   elseif self.source == 'v4linux' then
      libv4l.grabFrame(self.VGA,'/dev/video'..self.camidx,self.nbuffers,self.fps)
      image.scale(self.VGA, self.output, 'bilinear')
   elseif self.source == 'cam-iface' then
      self.camera:getFrame(self.VGA)
      image.scale(self.VGA, self.output, 'bilinear')
   elseif self.source == 'lena' then
      if not self.lena then
         self.lena = image.lena()
         self.lenas = torch.Tensor():resizeAs(self.output)
         image.scale(self.lena, self.lenas, 'bilinear')
      end
      self.output:copy(self.lenas)
   elseif self.source == 'video' then
      local next = self.video:forward()
      image.scale(next, self.output, 'bilinear')
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

function ImageSource:__show()
   image.display(self:forward())
end
