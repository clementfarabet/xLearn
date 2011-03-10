--==============================================================================
-- File: sys
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

do 

   local Kinect = torch.class('Kinect')

   function Kinect:__init(...)
      -- usage
      toolBox.unpack_class(
         self, {...}, 'Kinect',
         'Init  Kinect device to use it as data source:\n'
            .. ' + relies on libfreenect, which must be installed\n',
         {arg='width', type='number', help='width', default=640},
         {arg='height', type='number', help='height', default=480}
      )
      -- singleton for thread cleaning on exit
      if Kinect.singleton == nil then
         Kinect.singleton = libkinect.new()
      end
      self.rgbd = torch.Tensor(self.width,self.height,4)
      self.rgb = torch.Tensor(self.width,self.height,3)
      self.depth = torch.Tensor(self.width,self.height)
      self.hist = torch.Tensor(2048)
      -- align the kinect
      libkinect.tilt(0)
   end

   function Kinect:getRGBD(...)
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

   function Kinect:tilt(angle)
      -- angle should be between -30 and 30
      libkinect.tilt(angle)
   end

   function Kinect:__tostring__()
      str = 'Wrapper for Kinect'
      return str
   end

end
