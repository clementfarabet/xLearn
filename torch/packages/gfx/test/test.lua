require "gfx"
require "random"

--works
--require "image"


-- make some data 

x={}; sy={}; cy={}; 
for i=-3,4,0.05 do
  sy[#sy+1]=math.cos(i); cy[#cy+1]=math.sin(i); x[#x+1]=i;
end

tests={1,0,0,0,0}
tests={1,1,0,0,0}
tests={1,1,1,0,0}
tests={1,1,1,1,0}
tests={1,1,1,1,1}
tests={1,0,0,0,0,0}
tests={0,0,0,0,1,0}
tests={1,1,1,1,1,1} 
--tests={1,1,0,0,0,0} 

if tests[1]==1 then
  
  w1 = gfx.Window(500,500,'Torch LuaCairo Demo 1');
  w1:axis(-2,3,-1.5,1.5);
  w1:plot({-3,4},{-1,1},'r:')
  w1:plot(x,sy,'m-') 
  w1:plot(x,cy,'k-o')
  w1:title('Torch LuaCairo Demo 1');
  w1:xlabel('data')
  w1:ylabel('more data')
  w1:legend('linear','sine','cosine')
  w1:writePNG('demo1.png')
  w1:writePDF('test1.pdf');
  w1:writePS('test1.ps');
  
end

if tests[2]==1 then
  w2 = gfx.Window(500,500,'Torch LuaCairo Demo 2');
  w2:plot({1,2,3,5,8,10},{1,0,1,0,3,4},'m:x')
  w2:title('Torch LuaCairo Demo 2');
--  w2:writePNG('demo2.png')
--  w2:writePDF('test2.pdf');
end


-- x=x:t();
--for i=0,1,0.05 do 
x = gfx:tensorFromPNG('logo.png',4)

--print(x)
--os.exit(1)

if tests[3]==1 then
  w3 = gfx.Window(250,230,'Torch -- Liberty For All 3');
  w3:blit(x,3);
--  w3:writePDF('test3.pdf');
--  w3:writePS('test3.ps');
end
 
if tests[6]==1 then
  w6 = gfx.Window(500,500,'Torch -- Liberty For All 6');
  local methods=3;
  local x=torch.Tensor(1000,methods)  
  for i=1,1000 do
  for j=1,methods do
    x[i][j]=random.normal();
  end
  end 
  w6:hist(x,20)
  w6:title('Torch LuaCairo Demo 6');
  --bins=torch.Tensor(10);
  --for i=1,10 do bins[i]=i; end
  --p=w6:hist(x,bins)
  w6:legend('gauss1','gauss2','gauss3')
--  w6:writePNG('hist.png')
--  w6:writePDF('test6.pdf');
end

if tests[4]==1 then
  w4 = gfx.Window(250,230,'Torch -- Liberty For All 4');
  x4 = torch.Tensor(250,230,4);
  x5 = torch.Tensor(250,230,4);
  --  torch.image.scale(x,x4);
  theta=0;
  while w4:valid() == 1 do 
  --for i=1,20 do
   --     torch.image.rotate(x4,x5,theta);
    w4:blit(x+theta);
    theta=theta+0.01;
  --  os.execute("sleep 0.1")
  end
end

if tests[5]==1 then
  if torch.packageLuaPath("image") then
    require "image"
    w4 = gfx.Window(350,330,'Torch -- Liberty For All 5');
    x4 = torch.Tensor(350,330,4);
    x5 = torch.Tensor(350,330,4);
    image.scale(x,x4);
    theta=0;
    while w4:valid() == 1 do 
    --for i=1,200 do
      image.rotate(x4,x5,theta);
      w4:blit(x5);
      theta=theta+0.05;
    end
  else
    print("example 5 not run because image library not installed")
  end
end




--	w3:line(0,0,100,100);
--end
--w3:setPenColor({1,1,1}); 
--w3:text('(c) 2006 Torch Corporation',70,20,20)

--w3:writePNG('modified_logo.png')
--w3:writePdf('logo.pdf')


-- os.execute("Sleep 1")
