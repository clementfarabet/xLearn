do
   ------------------------------------------------------------
   -- FFMpeg Wrapper.
   --
   -- Provides a simple/generic Video class.
   --
   -- To load: require 'ffmpeg'
   ------------------------------------------------------------
   module('ffmpeg', package.seeall)

   ------------------------------------------------------------
   -- dependencies
   ------------------------------------------------------------
   torch.include('ffmpeg', 'Video.lua')
end
