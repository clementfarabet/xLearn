
require("lcairo")
ps = lcairo.PS("a.ps",600,555)

function fun(n) 
	return  250+math.sin(n)*200, 250+0.25*math.cos(n*1.2)*n
end

cr = lcairo.Cairo(ps)
cr:set_source_rgb(0.0,0.0,0.0)
cr:set_line_width(0.0);
cr:move_to(0.0,0.0)

cr=lcairo.Cairo(ps)
for n=0,math.pi*200,0.01 do
	cr:set_line_width(1.0/(n/200))
	x,y = fun(n)
	cr:line_to(x,y)
end
cr:stroke()

cr=lcairo.Cairo(ps)
cr:rectangle(1,1,500,500)
cr:stroke()
cr:show_page()

