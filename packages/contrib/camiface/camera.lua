----------------------------------
-- a camera interface
----------------------------------

do
   local Camera = torch.class('Camera')

   function Camera:__init()
      self.threadID = libcamiface.forkProcess('frame_grabber-quicktime')
      self.sharedMemFile = 'shared-mem'
      toolBox.sleep(2)
   end

   function Camera:getFrame(tensor, ...)
      local dumptofile = arg[1] or false
      libcamiface.getSharedFrame(self.sharedMemFile, tensor, dumptofile)
   end

   function Camera:stop()
      print('stopping camera')
      libcamiface.killProcess(self.threadID)
   end
end
