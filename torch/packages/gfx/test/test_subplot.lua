require "gfx"
require "random"

-- make some data 



tests={1,1,1,1}

w=gfx.Window(700,700)
w:begin()

if tests[1]==1 then
	x={}; sy={}; cy={}; 
	for i=-3,4,0.05 do
		sy[#sy+1]=math.cos(i); cy[#cy+1]=math.sin(i); x[#x+1]=i;
	end	
	w1 = w:subplot(2,2,1)
	w1:axis(-2,3,-1.5,1.5);
	w1:plot({-3,4},{-1,1},'r:')
	w1:plot(x,sy,'m-') 
	w1:plot(x,cy,'k-o')
	w1:title('Torch Demo 1');
	w1:xlabel('data')
	w1:ylabel('more data')
	w1:legend('linear','sine','cosine')
--	w1:writePNG('demo1.png')
--	w1:writePDF('test1.pdf');
--	w1:writePS('test1.ps');
end

if tests[2]==1 then
	w2 = w:subplot(2,2,2)
	w2:hist(lab.rand(100))
	w2:title('Torch Demo 2');
end


if tests[3]==1 then
	w3 = w:subplot(2,2,3)
	w3:plot({1,2,3,5,8,10},{1,0,1,0,3,4},'m:x')
	w3:title('Torch Demo 3');
	w3:xlabel('x-axis')
	w3:ylabel('y-axis')
--	w3:writePNG('demo2.png')
--	w3:writePDF('test2.pdf');
end
 
if tests[4]==1 then
	w4 = w:subplot(2,2,4)
	w4:hist(lab.randn(200,3),20)
	w4:title('Torch Demo 4');
	w4:legend('gauss1','gauss2','gauss3')
	--w6:writePNG('hist.png')
	--w6:writePDF('test6.pdf');
end 

w:theEnd()
w:writePNG('all3.png')
w:writePDF('all3.pdf')
w:writePS('all3.ps')