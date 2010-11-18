require "lcairo"

im=lcairo.Image("a.png")

w=lcairo.Window(320,320)
while w:valid() == 1 do
  print("frame:",i)
  cr=lcairo.Cairo(w)
  cr:set_source_surface(im)
  cr:rectangle(0,0,320,320)
  cr:fill()
  cr=nil
  w:refresh()
  collectgarbage()
end

