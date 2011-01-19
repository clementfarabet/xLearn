--------------------------------------------------------------------------------
-- Video: a class to handle videos
-- 
-- Authors: Marco, Clement
--------------------------------------------------------------------------------
do
   local vid = torch.class('Video')

   ----------------------------------------------------------------------
   -- __init()
   -- loads arbitrary videos, using FFMPEG (and a temp cache)
   -- returns a table (list) of images
   --
   function vid:__init(...)
      -- usage
      local args, path, w, h, fps, length, channel, loaded, fmt = toolBox.unpack(
         {...},
         'video.loadVideo',
         'loads a video into a table of tensors:\n'
            .. ' + relies on ffpmeg, which must be installed\n'
            .. ' + creates a local scratch/ to hold jpegs',
         {arg='path', type='string', help='path to video'},
         {arg='width', type='number', help='width', default=500},
         {arg='height', type='number', help='height', default=376},
         {arg='fps', type='number', help='frames per second', default=5},
         {arg='length', type='number', help='length, in seconds', default=5},
         {arg='channel', type='number', help='video channel', default=0},
         {arg='load', type='boolean', help='loads frames after conversion', default=true},
         {arg='encoding', type='string', help='format of dumped frames', default='jpg'}
      )

      -- check ffmpeg existence
      local res = toolBox.exec('ffmpeg')
      if res:find('not found') then 
         local c = toolBox.COLORS
         error(c.Red .. 'ffmpeg required, please install it (apt-get install ffmpeg)' .. c.none)
      end

      -- record meta params
      self.path = path
      self.w = w
      self.h = h
      self.fps = fps
      self.length = length
      self.loaded = loaded
      self.fmt = fmt

      -- load channel(s)
      if type(channel) ~= 'table' then channel = {channel} end
      for i = 1,#channel do
         self[i] = {}
         self:loadChannel(channel[i], self[i])
         self[i].channel = channel
      end

      -- cleanup disk
      if loaded and self.path then
         self:clear()
      end
   end

   function vid:mktempdir()
      -- make cache
      local date = os.date():gsub(' ','_')
      local path_cache = paths.concat('scratch',date)
      os.execute('mkdir -p ' .. path_cache)
      return paths.concat(path_cache,'')
   end
      
   ----------------------------------------------------------------------
   -- loadChannel()
   -- loads a channel
   --
   function vid:loadChannel(channel, where)
      where.path = self:mktempdir()

      -- file name format
      where.sformat = 'frame-%04d.'..self.fmt

      -- process video
      if self.path then 
	 local ffmpeg_cmd = 'ffmpeg -i ' .. self.path .. 
	    ' -r ' .. self.fps .. 
	    ' -t ' .. self.length ..
	    ' -map 0.' .. channel ..
	    ' -s ' .. self.w .. 'x' .. self.h .. 
	    ' -qscale 1' ..
	    ' ' .. paths.concat(where.path, where.sformat)
	 print(ffmpeg_cmd)
         os.execute(ffmpeg_cmd)
      end

      -- load Images
      local idx = 1
      for file in paths.files(where.path) do
         if file ~= '.' and file ~= '..' then
            local fname = paths.concat(where.path,string.format(where.sformat,idx))
            if not self.loaded then
               table.insert(where, fname)
            else
               table.insert(where, image.load(fname))
            end
            idx = idx + 1
         end
      end
   end

   
   ----------------------------------------------------------------------
   -- get_frame
   -- as there are two ways to store, you can't index self[1] directly
   function vid:get_frame(c,i)
      if self.loaded then
	 return self[c][i]
      else 
	 if self.fmt == 'png' then 
	    -- png is loaded in RGBA
	    return image.load(self[c][i]):narrow(3,1,3)
	 else
	    return image.load(self[c][i])
	 end
      end
   end

   ----------------------------------------------------------------------
   -- save()
   --
   function vid:save(opath)
      -- warning: -r must come before -i
      local ffmpeg_cmd =  ('ffmpeg' ..
                           ' -r ' .. self.fps ..
                           ' -i ' .. paths.concat(self[1].path, self[1].sformat) ..
                           ' -vcodec mjpeg -qscale 1 -an ' ..
                           ' ' .. opath .. '.avi')
      print(ffmpeg_cmd)
      os.execute(ffmpeg_cmd)
   end


   ----------------------------------------------------------------------
   -- play()
   -- plays a video
   --
   function vid:play(...)
      -- usage
      local args, zoom, loop, fps, channel = toolBox.unpack(
         {...},
         'video.playVideo',
         'plays a video:\n'
            .. ' + video must have been loaded with video.loadVideo()\n'
            .. ' + or else, it must be a list of tensors',
         {arg='zoom', type='number', help='zoom', default=1},
         {arg='loop', type='boolean', help='loop', default=false},
         {arg='fps', type='number', help='fps [default = given by seq.fps]'},
         {arg='channel', type='number', help='video channel', default=1}
      )

      -- plays vid
      local p =  qtwidget.newwindow(self.w*zoom,self.h*zoom)
      local disp = Displayer()
      local frame = torch.Tensor()
      local pause = 1 / (fps or self.fps) - 0.03
      while true do
         for i,frame in ipairs(self[channel]) do
            if not self.loaded then frame = image.load(frame) end
            disp:show{tensor=frame,painter=p,legend='playing sequence',zoom=zoom}
            if pause and pause>0 then libxlearn.usleep(pause*1e6) end
            collectgarbage()
         end
         if not loop then break end
      end
   end


   ----------------------------------------------------------------------
   -- play3D()
   -- plays a 3D video
   --
   function vid:play3D(...)
      -- usage
      local _, zoom, loop, fps = toolBox.unpack(
         {...},
         'video.playVideo3D',
         'plays a video:\n'
            .. ' + video must have been loaded with video.loadVideo()\n'
            .. ' + or else, it must be a list of pairs of tensors',
         {arg='zoom', type='number', help='zoom', default=1},
         {arg='loop', type='boolean', help='loop', default=false},
         {arg='fps', type='number', help='fps [default = given by seq.fps]'}
      )

      -- plays vid
      local p =  qtwidget.newwindow(self.w*zoom,self.h*zoom)
      local disp = Displayer()
      local framel
      local framer
      local frame = torch.Tensor()
      local pause = 1 / (fps or self.fps) - 0.08
      while true do
         for i = 1,#self[1] do
            -- left/right
            framel = self[1][i]
            framer = self[2][i]
            -- optional load
            if not self.loaded then 
               framel = image.load(framel)
               framer = image.load(framer)
            end
            -- merged
            frame:resize(framel:size(1),framel:size(2),3)
            frame:select(3,1):copy(framel:select(3,1))
            frame:select(3,2):copy(framer:select(3,1))
            frame:select(3,3):copy(framer:select(3,1))
            -- disp
            disp:show{tensor=frame,
                      painter=p,
                      legend='playing 3D sequence [left=RED, right=CYAN]',
                      zoom=zoom}
            if pause and pause>0 then libxlearn.usleep(pause*1e6) end
         end
         if not loop then break end
      end
   end


   ----------------------------------------------------------------------
   -- clear()
   --
   function vid:clear()
      for i = 1,#self do
         local clear = 'rm -rf ' .. self[i].path
         print('clearing video')
         os.execute(clear)
         print(clear)
      end
   end


   ----------------------------------------------------------------------
   -- playYouTube3D() 
   -- plays a video in a format which is acceptable for YouTube 3D (loads jpgs from disk as prepared by Video2imgs)
   --
   function vid:playYouTube3D(...)
      -- usage
      local _, zoom, savefname = toolBox.unpack(
         {...},
         'video.playYouTube3D',
         'plays a video:\n'
            .. ' + video must have been loaded with video.loadVideo()\n'
            .. ' + or else, it must be a list of pairs of tensors',
         {arg='zoom', type='number', help='zoom', default=1},
         {arg='savefname', type='string', help='filename in which output movie will be saved'}
      )

      -- enforce 16:9 ratio
      local h  = self.h
      local w  = (h * 16 / 9)
      local w2 = w/2
      local frame = torch.Tensor(w,h,3)
      print('Frame [' .. frame:size(1) .. ', ' .. frame:size(2) .. ', ' .. frame:size(3) .. ']')

      -- create output
      local outp = Video{width=w, height=h, fps=self.fps, 
                         length=self.length, encoding='png'}

      -- plays vid
      zoom = zoom or 1
      local p =	 qtwidget.newwindow(w*zoom,h*zoom)
      local disp = Displayer()
      local pause = 1 / (self.fps or 5) - 0.08
      local idx = 1
      for i = 1,#self[1] do
         -- left/right
         local frameL = self[1][i]
         local frameR = self[2][i]
         -- optional load
         if not self.loaded then 
            frameL = image.load(frameL)
            frameR = image.load(frameR)
         end
	 print('FrameL [' .. frameL:size(1) .. ', ' .. frameL:size(2) .. ', ' .. frameL:size(3) .. ']')
	 
         -- left
         if not (frameL:size(1) == w2 and frameL:size(2) == h) then
            image.scale(frameL,frame:narrow(1,1,w2),'bilinear')
         else
            frame:narrow(1,1,w2):copy(frameL)
         end
         -- right
         if not (frameR:size(1) == w2 and frameR:size(2) == h) then
            image.scale(frameR,frame:narrow(1,w2,w2),'bilinear')
         else 
            frame:narrow(1,w2,w2):copy(frameR)
         end
         frameL = nil 
         frameR = nil
         collectgarbage() -- force GC ...
         -- disp
         disp:show{tensor=frame,painter=p,legend='playing 3D sequence',zoom=zoom}
         if savefname then
            local ofname = paths.concat(outp[1].path, string.format(outp[1].sformat, idx))
            table.insert(outp[1], ofname)
            image.save(ofname,frame)
         end
         idx = idx + 1
         -- need pause to take the image loading time into account.
         if pause and pause>0 then libxlearn.usleep(pause*1e6) end
      end

      -- convert pngs to AVI
      if savefname then
         outp:save(savefname)
      end
   end
end
