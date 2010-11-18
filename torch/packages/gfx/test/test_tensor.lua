require("gfx")
require "lcairo"

w=300
h=200

--create a window
w1=lcairo.Window(w,h)

--do some drawing on it
cr=lcairo.Cairo(w1)
cr:set_source_rgb(0,0,1)
cr:rectangle(50,50,150,20)
cr:fill()
--cr=nil

--write to png
w1:write_to_png("test1.png")

--create a tensor (matching dimensions)
x=torch.Tensor(w,h,3)

--copy the window to the tensor
w1:toTensor(x)


--how to look at innards of tensor...
print(x:size())
for j=1,x:size()[1] do
for i=1,x:size()[2] do
	if (x[j][i][1]>0.0) then
--		print (x[j][i]) --red
	end
end
end

--create new image surface
s=lcairo.Image(w,h)

--copy tensor to image surface
s:fromTensor(x)
while w1:valid() == 1 do
	cr=lcairo.Cairo(w1)
	cr:set_source_surface(s,0,0)
	cr:rectangle(50,50,60,20)
	cr:fill()
	w1:refresh()
end

--write out to png
s:write_to_png("test2.png")

--os.execute("sleep 1")
