require("lcairo")

image = lcairo.Image("logo.png")

--draw on the image
cr=lcairo.Cairo(image)
cr:move_to(10,10)
cr:line_to(100,100)
cr:stroke()

--open a window 
w1=lcairo.Window(300,200)
n=0
nc=0
while nc<10000  and w1:valid()==1 do
	--open a window blit image
	cr=lcairo.Cairo(w1)
	cr:translate(n,0)
	cr:set_source_surface(image,0,0)
	cr:rectangle(0,0,image.width,image.height)
	cr:fill()

	w1:refresh()

	n=math.mod(n+2,w1.width)
	nc=nc+1
end
print("ending..")
--os.execute("sleep 5")

