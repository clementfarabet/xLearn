
require("lcairo")
pdf = lcairo.PDF("a.pdf",600,555)

function fun(n) 
	return  250+math.sin(n)*200, 250+0.25*math.cos(n*1.2)*n
end

cr = lcairo.Cairo(pdf)

cr:move_to(20,20)
cr:show_text("hello",10,10,10)
cr:show_page()

