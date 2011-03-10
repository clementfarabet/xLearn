--------------------------------------------------------------------------------
-- Displayer
-- a class to display to a QT painter/window with efficient mem managment
--------------------------------------------------------------------------------
do
   ----------------------------------------------------------------------
   -- register class + constructor
   --
   local Displayer = torch.class('Displayer')

   function Displayer:__init()
      -- needs QT
      toolBox.useQT()
      
      -- internal storages
      self.qtimg = nil
      self.tensor = nil
      self.disptensor = torch.Tensor()
   end

   function Displayer:scale(tensor)
      -- dims
      local w = tensor:size(1)
      local h = tensor:size(2)
      if tensor:nDimension() == 3 then
         local c = tensor:size(3)
         self.disptensor:resize(w,h,c)
      else
         self.disptensor:resize(w,h)
      end
      -- copy tensor to disptensor
      self.disptensor:copy(tensor)
      
      -- rescale min
      if not self.min then
         self.min = self.disptensor:min()
      end
      if self.min ~= 0 then self.disptensor:add(-self.min) end

      -- rescale for max
      if not self.max then
         self.max = self.disptensor:max()
      else
         self.max = self.max - self.min
      end
      if self.max ~= 0 then self.disptensor:div(self.max) end
      
      -- saturate
      libxlearn.saturate(self.disptensor)
   end

   function Displayer:show(args)
      -- parse args
      if not args.tensor then error('ERROR <Displayer.show()> must receive a tensor') end
      self.zoom = args.zoom or 1
      self.globalzoom = args.globalzoom or 1
      self.offset_x = args.offset_x or 0 
      self.offset_y = args.offset_y or 0
      self.raw = args.raw or false
      self.min = args.min
      self.max = args.max
      self.color = args.color or 'black'
      self.legend = args.legend
      self.fontsize = args.fontsize or 16
      self.painter = args.painter 
         or self.painter
         or qtwidget.newwindow(args.tensor:size(1)*self.zoom*self.globalzoom,
                               args.tensor:size(2)*self.zoom*self.globalzoom,
                               self.legend)
      -- rescale for display
      local tensor_used
      if not self.raw then 
         self:scale(args.tensor)
         tensor_used = self.disptensor
      else
         tensor_used = args.tensor
      end

      -- changed size ?
      if not self.tensor or 
                 tensor_used:size(1) ~= self.tensor:size(1) or 
                 tensor_used:size(2) ~= self.tensor:size(2) then
         -- create a QImage
         self.qtimg = qt.QImage.fromTensor(tensor_used)
      else
         -- just copy tensor to QImage
         qt.QImage.copyTensor(tensor_used, self.qtimg)
      end
      self.tensor = tensor_used

      -- display image
      self.painter:image(self.offset_x*self.globalzoom, 
                         self.offset_y*self.globalzoom,
                         self.tensor:size(1)*self.zoom*self.globalzoom, 
                         self.tensor:size(2)*self.zoom*self.globalzoom,
                         self.qtimg, 0, 0, 
                         self.tensor:size(1), self.tensor:size(2))
      -- print legend
      if (self.legend ~= nil) then
         self.painter:setcolor(self.color)
         self.painter:setfont(qt.QFont{serif=false,italic=false,size=fontsize})
         self.painter:moveto(self.offset_x*self.globalzoom + 3, 
                             self.offset_y*self.globalzoom-1)
         self.painter:show(self.legend)
      end
   end
end
