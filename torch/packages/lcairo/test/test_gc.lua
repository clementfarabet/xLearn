
require "torch"
require "lcairo"



for i=1,100000 do 
--_image=lcairo.create_image_from_png("adpt.png")
--_image=nil
  im=lcairo.Image("adpt.png")
  collectgarbage()
end
