
require("lcairo")
w1 = lcairo.Window(320,240)

cr = lcairo.Cairo(w1)
cr:set_source_rgb(0.0,0.0,0.0)
cr:set_line_width(2.0);
cr:move_to(0.0,0.0)
cr:line_to(200.0,200.0)
cr:stroke();
w1:refresh()

surf = lcairo.Image(512,512)
x=torch.Tensor(512,512)
for i=1,512 do
for j=1,512 do
 	x[j][i]=j/512
end
end
surf:fromTensor(x)

for n=0,math.pi*20,0.01 do
  cr:line_to( 250+math.sin(n)*200, 250+math.cos(n*1.2)*n )
end
cr:stroke()

cr=lcairo.Cairo(w1)
cr:set_source_surface(surf,0,0)
cr:rectangle(10,10,512,512)
cr:fill()
w1:refresh()

--w1:interactive()

while w1:valid()==1 do
	msg = w1:messages_next()
	if msg~="null" then
		print(msg)
	end
end

