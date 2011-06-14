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

if not kinect then
   ------------------------------------------------------------
   -- Wrapper for kinect
   --
   -- To load: require 'kinect'
   --
   -- @release 2011 Benoit Corda
   ------------------------------------------------------------
   local display = image.display
   local torch = torch
   module('kinect')
   ------------------------------------------------------------
   -- Include classes
   ------------------------------------------------------------
   torch.include('kinect', 'Kinect.lua')

   ------------------------------------------------------------
   -- Activate the first existing kinect and display RGB and Depth
   ------------------------------------------------------------
   testme = function()
               k = Device(640,480)
               k:getRGBD()
               k:tilt(20)
               display(k.rgbd)
            end
   

end

return kinect