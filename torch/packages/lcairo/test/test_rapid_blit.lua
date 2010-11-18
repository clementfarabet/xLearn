--rapid blit test
require("lcairo")
require("torch")

w1=lcairo.Window(280,280,"testing")
s=lcairo.Image("logo.png")
t=0
while w1:valid()==1 do
	t = t+0.1
	cr = lcairo.Cairo(w1)
	cr:translate(100+100*math.sin(t),100+100*math.cos(t))
	cr:set_source_surface(s,0,0)
	cr:rectangle(0,0,s.width,s.height)
	cr:fill()
	w1:refresh()
end



