--==============================================================================
-- File: Kinect.lua
--
-- Description: A wrapper for using the kinect
--              
--
-- Created: February 2, 2011, 5:37PM
--
-- Author: Benoit Corda -- cordaben@gmail.com
--==============================================================================
require 'XLearn'
require 'libkinect'

local Device = torch.class('kinect.Device')

------------------------------------------------------------
-- Creates a Device object
------------------------------------------------------------
function Device:__init(...)
   -- usage
   toolBox.unpack_class(
      self, {...} or {}, 'Device',
      'Init  Kinect device to use it as data source:\n'
         .. ' + relies on libfreenect, which must be installed\n',
      {arg='width', type='number', help='width', default=640},
      {arg='height', type='number', help='height', default=480},
      {arg='deviceID', type='number', help='if you have several devices connected, first is 0', default=0}
   )
   -- singleton for thread cleaning on exit
   Device.singleton = Device.singleton or {}
   if Device.singleton[self.deviceID] == nil then
      Device.singleton[self.deviceID] = libkinect.new(self.deviceID)
   end
   self.rgbd = torch.Tensor(self.width,self.height,4)
   self.rgb = torch.Tensor(self.width,self.height,3)
   self.depth = torch.Tensor(self.width,self.height)
   self.hist = torch.Tensor(2048)
end

------------------------------------------------------------
-- only grab RGB camera data
------------------------------------------------------------
function Device:getRGB(...)
   libkinect.grabRgb(self.rgb)
   return self.rgb
end

------------------------------------------------------------
-- only grab Depth camera data
------------------------------------------------------------
function Device:getDepth(...)
   libkinect.grabDepth(self.depth)
   return self.depth
end

------------------------------------------------------------
-- grab Depth and RGB camera data synchronous
------------------------------------------------------------
function Device:getRGBD(...)
   libkinect.grabRgbD(self.rgbd,self.hist)
   kersize=5
   ker = image.gaussian1D{size=kersize, normalize=true}
   unhist = self.hist:unfold(1,kersize,1)
   conv = torch.Tensor():resizeAs(unhist):copy(unhist)
   histconv = torch.Tensor(2048-kersize+1):fill(0)
   histconv:addT2dotT1(1,conv,ker)
   self.hist:fill(0)
   self.hist:narrow(1,2,2044):copy(histconv)
   self.rgb = self.rgbd:narrow(3,1,3)
   self.depth = self.rgbd:select(3,4)
   return self.rgbd
end   

function Device:tilt(angle)
   -- angle should be between -30 and 30
   libkinect.tilt(angle)
end

-- function Device:__tostring__()
--    str = 'Wrapper for Kinect Device'
--    return str
-- end
