require "torch"
require('libmpeg2')

-- plays file
function mpeg2.playFile(file,zoom)
   local dec = mpeg2.Decoder(file)
   local frame = torch.Tensor(1,1,3)
   local disp = Displayer()
   local p = nil
   while dec:newFrame(frame) == 1 do
      p = p or qtwidget.newwindow(frame:size(1)*zoom, frame:size(2)*zoom)
      disp:show{tensor=frame,painter=p,legend='playing mpeg2',zoom=zoom}
   end
end

-- reads file, returns sequence
function mpeg2.decodeFile(file,w,h,size,step)
   local dec = mpeg2.Decoder(file)
   local seq = {}
   print('# mpeg2: decoding file ' .. file)
   local i = 1
   local frame = torch.Tensor(1,1,3)
   local frameResized = torch.Tensor(w or 640, h or 480, 3)
   local stepc = 0
   step = step or 1
   while true do
      local valid = dec:newFrame(frame)
      if (stepc % step) == 0 then
         image.scale(frame,frameResized,'bilinear')
         frameResized:mul(255):add(0.5)
         local frameByte = torch.ByteTensor(w or 640, h or 480, 3)
         frameByte:copy(frameResized)
         table.insert(seq,frameByte)
         if size then toolBox.dispProgress(i,math.floor(size/step)) end
         if valid ~= 1 or i >= (size/step) then break end
         i=i+1
      end
      stepc = stepc + 1
   end
   return seq
end

-- plays sequence
function mpeg2.playSequence(seq,zoom,loop,mpause)
   local p =  qtwidget.newwindow(seq[1]:size(1)*zoom,seq[1]:size(2)*zoom)
   local disp = Displayer()
   local frame = torch.Tensor()
   while true do
      for i,frameByte in ipairs(seq) do
         frame:resize(frameByte:size(1),frameByte:size(2),frameByte:size(3))
         frame:copy(frameByte)
         disp:show{tensor=frame,painter=p,legend='playing sequence',zoom=zoom}
         if mpause and mpause>0 then libxlearn.usleep(mpause*1000) end
      end
      if not loop then break end
   end
end
