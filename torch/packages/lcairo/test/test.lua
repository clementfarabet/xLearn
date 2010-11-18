require "torch"
require "lab"
require "lcairo"

w1=lcairo.Window(200,200)
cr=lcairo.Cairo(w1)
cr:move_to(10,10)
cr:line_to(20,20)
cr:stroke()
cr:move_to(10,100)
cr:set_source_rgba(1,0,1,1)
cr:set_font_size(20);
cr:show_text("Torch 5.1")
lcairo.surface_write_to_png(w1._surf,"a.png")
w1:refresh()

pdf=lcairo.PDF("a.pdf",30,30)
cr=lcairo.Cairo(pdf)
cr:move_to(20,10)
cr:line_to(10,20)
cr:stroke()
cr:show_page()
pdf:close()

ps=lcairo.PS("a.ps",30,30)
cr=lcairo.Cairo(ps)
cr:move_to(10,10)
cr:line_to(20,20)
cr:stroke()
cr:show_page()
ps:close()

im=lcairo.Image(200,200)
cr=lcairo.Cairo(im)
cr:move_to(20,10)
cr:line_to(10,20)
cr:stroke()
im:write_to_png("a.png")



x1=torch.Tensor(32,32)
for i=1,32 do for j=1,32 do 
  x1[i][j]=math.sin(i*0.1)*math.cos(j*0.3) 
end end
w=lcairo.Window(32,32)
w:fromTensor(x1)
w:refresh()


x2=torch.Tensor(32,32)
w:toTensor(x2)

w2=lcairo.Window(32,32)
w2:fromTensor(x2)
w2:refresh()


x=torch.Tensor(41,41)
x:fill(-2)
max=-1
f=io.open("adaptive_train.txt")
while 1 do
 l=f:read()
 if not l then break end
 t1,t2,train,test=string.match(l,"(%S+)%s+(%S+)%s+(%S+)%s+(%S+)")
 t1i=1+(tonumber(t1)/10)
 t2i=1+(tonumber(t2)/10)
 x[t1i][t2i]=tonumber(train)
 v=x[t1i][t2i]
 if v>max then max=v print(t1,t2,train,test) end
end
x=x-x:min()
x=x*1/x:max()

x[20][20]=0;

im=lcairo.Image(x:size(1),x:size(2))
im:fromTensor(x)
im:write_to_png("adpt.png")

w=lcairo.Window(480,480)
cr=lcairo.Cairo(w)
-- nb scale before set_source)surface
cr:scale(12,12)
cr:set_source_surface(im,0.5,0.5)
-- set pixel filter
pat = cr:get_source() -- current pattern
pat:set_filter(1)
cr:rectangle(0,0,41,41)
cr:fill()
w:refresh()

im=lcairo.Image("adpt.png")
w3=lcairo.Window(64,64)
cr=lcairo.Cairo(w3)
cr:set_source_surface(im,0.5,0.5)
cr:rectangle(0,0,41,41)
cr:fill()
w3:refresh()

im=nil

--work but unused
--cr:set_source(pat)


