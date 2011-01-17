#!/usr/bin/env qlua
----------------------------------------------------------------------
-- Test Script:
-- Attention Detector
--
require 'NeuFlow'

-- debug

bypass_neuflow = false

------------------------------------------------------------
-- parse args
--
op = OptionParser('%prog [options]')
op:add_option{'-s', '--source', action='store', dest='source', 
              help='image source, can be one of: camera | lena'}
options,args = op:parse_args()

target = options.target or 'cpu'
last_name = target
last_fps = -1
counting = 0
----------------------------------------------------------------------
-- INIT: initialize the neuFlow context
-- a mem manager, the dataflow core, and the compiler
--
neuFlow = NeuFlow{prog_name           = 'Attention',
                  nb_convs            = 1,
                  offset_code         = bootloader.entry_point_b,
                  offset_data_1D      = bootloader.entry_point_b + 320*kB,
                  offset_data_2D      = bootloader.entry_point_b + 360*kB,
                  offset_heap         = bootloader.entry_point_b + 550*kB,
                  global_msg_level    = 'none',
                  compiler_msg_level  = 'none',
                  core_msg_level      = 'none',
                  interface_msg_level = 'none'}


----------------------------------------------------------------------
-- ELABORATION: describe the algorithm to be run on neuFlow, and 
-- how it should interact with the host (data exchange)
-- note: any copy**Host() inserted here needs to be matched by
-- a copy**Dev() in the EXEC section.
--


-- preprocess
-- camera and converter
source = nn.ImageSource(options.source or 'camera')
rgb2y = nn.ImageTransform('rgb2y')


-- image sizes

size = 380
inputs = torch.Tensor(size,size,3)
intensity = torch.Tensor(size,size)
intensitydiv = torch.Tensor(size,size,1)
intermediate = torch.Tensor(size,size,4)
output = torch.Tensor(size,size,2)

---------------------
-- subtracter module
---------------------
sub = nn.Sequential()
sub:add(nn.CCSub())
--sub:add(nn.Tanh())
sub:add(nn.AbsModule())

--------------------
-- addition module
--------------------

addnet = nn.Sequential()
addnet:add(nn.CCAdd())

addnet2 = nn.Sequential()
addnet2:add(nn.CCAdd())

absnet = nn.Sequential()
absnet:add(nn.AbsModule())

---------------------
-- edgenet module
---------------------

edgenet = nn.Sequential()
m = nn.SpatialConvolution(1,1,3,3,1,1)
m.weight[1][1][1][1] = 0
m.weight[1][2][1][1] = -1
m.weight[1][3][1][1] = 0
m.weight[2][1][1][1] = -1
m.weight[2][2][1][1] = 4
m.weight[2][3][1][1] = -1
m.weight[3][1][1][1] = 0
m.weight[3][2][1][1] = -1
m.weight[3][3][1][1] = 0
m.bias:fill(0)
n = nn.SpatialPadding(1,1,1,1)
o = nn.AbsModule()
edgenet:add(m)
edgenet:add(n)
edgenet:add(o)


--------------------------
-- edgestrip module
--------------------------

edgestrip = nn.Sequential()
m = nn.SpatialConvolution(1,1,3,3,1,1)
m.weight[1][1][1][1] = 0
m.weight[1][2][1][1] = 0
m.weight[1][3][1][1] = 0
m.weight[2][1][1][1] = 0
m.weight[2][2][1][1] = 1
m.weight[2][3][1][1] = 0
m.weight[3][1][1][1] = 0
m.weight[3][2][1][1] = 0
m.weight[3][3][1][1] = 0
m.bias:fill(0)
edgestrip:add(m)


--------------------------------------------------
-- finder functions (borrowed from old attention)
--------------------------------------------------

function find_max_index(input)

   local max_col = lab.max(input)
   local max_val, idx_col = lab.max(max_col, 2)

   local max_row = lab.max(input,2)
   local max_val, idx_row = lab.max(max_row)

   return idx_row[1][1], idx_col[1][1]
end


function return_section(input)
   local x_range = math.floor((input:size(1))/pointSize)
   local y_range = math.floor((input:size(2))/pointSize)

	section_holder = torch.Tensor(x_range,y_range)


	for i=1,x_range do
		for j=1,y_range do
			local start_x = (i-1)*pointSize+1
			local start_y = (j-1)*pointSize+1
			x, y = find_max_index(input:sub(start_x,start_x+pointSize-1,start_y,start_y+pointSize-1))					
			section_holder[i][j] = input[x+start_x-1][y+start_y-1]
		end
	end
	return section_holder
end

-----------------

kernel = lab.new({0,-1,0},{-1,4,-1},{0,-1,0})


-- copy first image

intensity_dev_a = neuFlow:copyFromHost(intensity)
intensity_dev_b = neuFlow:copyFromHost(intensity)
intensity_dev_c = neuFlow:allocHeap(intensity)

output_dev = neuFlow:allocHeap(output)
intermed_dev = neuFlow:allocHeap(intermediate)
kernel_dev = neuFlow:allocDataPacked(kernel)

-- global loop
neuFlow:beginLoop('main') do

   -- copy input image to dev
   neuFlow:copy(intensity_dev_a, intensity_dev_b)
   neuFlow:copyFromHost(intensity, intensity_dev_a)

   intensity_dev = neuFlow:copyFromHost(intensitydiv)

   inputs_dev = neuFlow:copyFromHost(inputs)
   
   -- do the diff

   TD_dev = neuFlow:compile(sub, {intensity_dev_a[1], intensity_dev_b[1]})

   -- RG

   neuFlow.core:subtract(inputs_dev[1], inputs_dev[2], intermed_dev[1]) 
   neuFlow.core:divide(intermed_dev[1], intensity_dev[1], output_dev[1])    

   -- BY
   
   BY_dev = neuFlow:compile(addnet, {inputs_dev[1], inputs_dev[2]})
   neuFlow.core:multiplyScalar(BY_dev[1], 0.5, intermed_dev[1])
   neuFlow.core:subtract(inputs_dev[3], intermed_dev[1], intermed_dev[2])
   neuFlow.core:divide(intermed_dev[2], intensity_dev[1], output_dev[2])
     
   -- Edge

   neuFlow.core:convolBank({intensity_dev_a[1]}, 
                           {kernel_dev[1]},
                           {intermed_dev[3]}) 
   Edge_dev = neuFlow:compile(absnet, {intermed_dev[3]})
   

   -- copy result to host

  output1 = neuFlow:copyToHost(TD_dev)   
  output2 = neuFlow:copyToHost(output_dev[1])
  output3 = neuFlow:copyToHost(output_dev[2])
  output4 = neuFlow:copyToHost(Edge_dev)

   -- sum with parameters
  
   neuFlow.core:multiplyScalar(TD_dev[1], 0.4, TD_dev[1])
   neuFlow.core:multiplyScalar(output_dev[1], 0.1, output_dev[1])
   neuFlow.core:multiplyScalar(output_dev[2], 0.1, output_dev[2])
   neuFlow.core:multiplyScalar(intensity_dev_a[1], 0.1, intensity_dev_c[1])
   neuFlow.core:multiplyScalar(Edge_dev[1], 0.3, Edge_dev[1])

   
   neuFlow.core:add(TD_dev[1], output_dev[1], intermed_dev[1])
   neuFlow.core:add(intermed_dev[1], output_dev[2], intermed_dev[2])
   neuFlow.core:add(intermed_dev[2], intensity_dev_c[1], intermed_dev[3])
   neuFlow.core:add(intermed_dev[3], Edge_dev[1], intermed_dev[4])   
  
   sal_dev = neuFlow:compile(edgestrip, {intermed_dev[4]})
   salience = neuFlow:copyToHost(sal_dev[1])

end neuFlow:endLoop('main')


----------------------------------------------------------------------
-- LOAD: load the bytecode on the device, and execute it
--
--neuFlow:loadBytecode()

bytecode_loaded = false

if target == 'neuflow' then
   neuFlow:loadBytecode()
   bytecode_loaded = true
end


----------------------------------------------------------------------
-- EXEC: this part executes the host code, and interacts with the dev
--


-- window
-- setup GUI (external UI file)

toolBox.useQT()
widget = qtuiloader.load('client-attention.ui')
painter = qt.QtLuaPainter(widget.frame)


--painter = qtwidget.newwindow(1500,1200)

-- displayers
disp_input = Displayer()
disp_output1 = Displayer()
disp_output2 = Displayer()
disp_output3 = Displayer()
disp_output4 = Displayer()
disp_salience = Displayer()
disp_intensity = Displayer()

-- send dummy input
-- neuFlow:copyToDev(intensity)
-- neuFlow:copyToDev(intensity)

intensityA = torch.Tensor(size, size)
intensityB = torch.Tensor(size, size)

softStart = source:forward()
image.scale(((rgb2y:forward(softStart)):select(3,1)), intensityB, 'bilinear')

error = torch.Tensor()

-- process loop
loop = 1
function process()
  
   if(target== 'neuflow') and (counting == 1) then
   
   neuFlow:copyToDev(intensity)
   neuFlow:copyToDev(intensity)
   counting = counting+1
   
   end

   count = 0 
   neuFlow.profiler:start('whole-loop','fps')
   count = count + 1

   camFrame = source:forward()
   camFrameYa = rgb2y:forward(camFrame)
   image.scale(camFrameYa, intensitydiv, 'bilinear')
   intensitydiv = intensitydiv+1
   camFrameY = camFrameYa:select(3,1)
   
  
   --if ((count%2) == 0) and (options.source == 'lena') then camFrameY = camFrameY:t() end
   
   image.scale(camFrameY, intensity, 'bilinear')
   image.scale(camFrame, inputs, 'bilinear')
   
   -- execute code on neuFlow if not bypass_neuflow:
   if (target == 'neuflow') then
   
   neuFlow:copyToDev(intensity)
   neuFlow:copyToDev(intensitydiv) 
   neuFlow:copyToDev(inputs)

   neuFlow:copyFromDev(output1)
   neuFlow:copyFromDev(output2)
   neuFlow:copyFromDev(output3)
   neuFlow:copyFromDev(output4)

   neuFlow:copyFromDev(salience)


   	if (target == 'neuflow') and (checkerror) then
   
   	intensityA:copy(intensityB)
   	intensityB:copy(intensity) 
   	outTD =  (intensityB - intensityA):abs()

        output1s = torch.Tensor(output1:size(1), output1:size(2),1)
        output2s = torch.Tensor(output2:size(1), output2:size(2),1)
        output3s = torch.Tensor(output3:size(1), output3:size(1),1)
        output4s = torch.Tensor(output4:size(1), output4:size(2),1)
        saliences = torch.Tensor(salience:size(1), salience:size(2),1)

        output1s:copy(output1)
        output2s:copy(output2)
        output3s:copy(output3)
        output4s:copy(output4)
        saliences:copy(salience)
   
        error:resizeAs(output1s):copy(outTD):mul(-1):add(output1s):abs()
        print('max error TD ='..error:max())

        outRG = ((inputs:select(3,1))-(inputs:select(3,2))):cdiv(intensitydiv)

        error:resizeAs(output2s):copy(outRG):mul(-1):add(output2s):abs()
        print('max error RG ='..error:max())
   
        mid = ((inputs:select(3,1))+(inputs:select(3,2))):mul(0.5)
        outBY = ((inputs:select(3,3))-(mid)):cdiv(intensitydiv)
        error:resizeAs(output3s):copy(outBY):mul(-1):add(output3s):abs()
        print('max error BY ='..error:max())

        intensitys = torch.Tensor(intensity:size(1), intensity:size(2),1)
        intensitys:copy(intensity)
        
        output4s1 = edgestrip:forward(output4s)
       
        outE = edgenet:forward(intensitys) 
        outE1 = edgestrip:forward(outE)

        error:resizeAs(output4s1):copy(outE1):mul(-1):add(output4s1):abs()
        print('max error E ='..error:max())

        salience_soft = (outTD:mul(0.4)):add(outRG:mul(0.1)):add(outBY:mul(0.1)):add(intensitys:mul(0.1)):add(outE:mul(0.3))
        sals = torch.Tensor(salience_soft:size(1), salience_soft:size(2),1)
        sals:copy(salience_soft)
        sal_soft = edgestrip:forward(sals)
   	
        error:resizeAs(saliences):copy(sal_soft):mul(-1):add(saliences):abs()
        print('max error salience ='..error:max())
        
 

   	end

   else

   one_map = torch.Tensor(size,size,1)
   two_map = torch.Tensor(size,size,1)
   three_map = torch.Tensor(size,size,1)
   four_map = torch.Tensor(size,size,1)
   I_map = torch.Tensor(size,size,1)
   profiler_cpu = neuFlow.profiler:start('compute-cpu')
   neuFlow.profiler:setColor('compute-cpu', 'blue')
   
   intensityA:copy(intensityB)
   intensityB:copy(intensity)

   output1 = (intensityB - intensityA):abs()
   output1:resize(output1:size(1), output1:size(2), 1)
   one_map:copy(output1)
  
   output2 = (inputs:select(3,1) - inputs:select(3,2)):cdiv(intensitydiv)
   output2:resize(output2:size(1), output2:size(2), 1)   
   two_map:copy(output2)

   mid = ((inputs:select(3,1))+(inputs:select(3,2))):mul(0.5)
   output3 = ((inputs:select(3,3))-(mid)):cdiv(intensitydiv)
   output3:resize(output3:size(1), output3:size(2),1)
   three_map:copy(output3) 


   intensitymap = torch.Tensor(size,size,1)
   intensitymap:copy(intensityB)
   output4 = edgenet:forward((intensitymap))
   four_map:copy(output4)

   I_map:copy(intensitydiv-1)   

   temp = (one_map:mul(0.4)):add(two_map:mul(0.1)):add(three_map:mul(0.1)):add(I_map:mul(0.1)):add(four_map:mul(0.3))
   salience = edgestrip:forward(temp)   

   neuFlow.profiler:lap('compute-cpu')

   end

   --if options.source == 'lena' then

--      error:resizeAs(output2):copy(outRG):mul(-1):add(output2):abs()
  --    print('max error RG = '..error:max())

 --     error:resizeAs(output3):copy(outBY):mul(-1):add(output3):abs()
--      print('max error BY = '..error:max())
      
  --    error:resizeAs(output1):copy(outTD):mul(-1):add(output1):abs()
    --  print('max error TD ='..error:max())

    --  output4E = edgestrip:forward(output4)

   --   error:resizeAs(output4E):copy(outE):mul(-1):add(output4E):abs()
    --  print('max error E ='..error:max())
      
 --  end
  
  sal = salience:select(3,1)

  neuFlow.profiler:start('scale-salience-down')
  saliencetemp = return_section(sal) 
  neuFlow.profiler:lap('scale-salience-down')
	  

   painter:gbegin()
   painter:showpage()
   neuFlow.profiler:start('display')
   disp_intensity:show{tensor=intensity, painter=painter, offset_x=1, offset_y=30,
                   min=0, max=1,
                   legend='Intensity', globalzoom=0.4}

   disp_output1:show{tensor=output1, painter=painter,
                    offset_x=size+2, offset_y=30,
                    legend='TD', globalzoom=0.4,
                    min=0, max=1}        

   disp_output2:show{tensor=output2, painter=painter,
                    offset_x=size+2, offset_y=size+60,
                    legend='RG', globalzoom=0.4,
                    min=0, max=1}           

   disp_output3:show{tensor=output3, painter=painter,
                    offset_x=1, offset_y=size+60,
                    legend='BY', globalzoom=0.4,
                    min=0, max=1}  

   disp_output4:show{tensor=output4, painter=painter,
		    offset_x=2*size+3, offset_y=30,
                    legend = 'Edge', globalzoom=0.4,
                    min=0, max=1}

   disp_input:show{tensor=inputs, painter=painter,
                    offset_x=2*size+3, offset_y=30,
                    legend = 'Original', globalzoom=0.7,
                    min=0, max=1}
 
   disp_salience:show{tensor=salience, painter=painter,
		    offset_x=2*size+3, offset_y=size+60,
		    legend = 'Salience', globalzoom=0.7,
                    min=0, max=1}

           for i=1,nPoints do
	        local r = 99-(4*i)
                local b = 6+(4*i)
                local col = '#'..tostring(r..'00'..b)                  

	    	local x, y = find_max_index(saliencetemp)
	    	saliencetemp[x][y]=0
	    	x = 1+pointSize*(x-1)
	    	y = 1+pointSize*(y-1)
	    	image.qtdrawbox{ painter=painter,
	                     x=x+(2*size+3),
	                     y=y+(20),
	                     w=pointSize,
	                     h=pointSize,
	                     globalzoom=0.7, 
			     color = col,
	                     legend=i
	                     }	
	       end

   neuFlow.profiler:lap('display')

   -- time
   neuFlow.profiler:lap('whole-loop')
   neuFlow.profiler:displayAll{painter=painter, x=10, y=340, zoom=0.6}
   
   
   local x = 30
   local y = 510
   local zoom = 0.6
   painter:setfont(qt.QFont{serif=false,italic=false,size=24*zoom})
   
   painter:setcolor("red")
   local str
   str = string.format('compare to <%s> = %f fps',
		       last_name,
		       last_fps)
   
   -- disp line:
   painter:moveto(x,y);
   painter:show(str)
   
   -- if we have both cpu and neuflow timimgs saved
   if(last_fps ~= -1) then

      x = 300
      y = 550
      painter:setfont(qt.QFont{serif=false,italic=false,size=28*zoom})
      painter:setcolor("red")
      
      local speedup = neuFlow.profiler.events['compute-cpu'].reald/neuFlow.profiler.events['on-board-processing'].reald
      str = string.format('speedup = %f ',
			  speedup)
      
      -- disp line:
      painter:moveto(x,y);
      painter:show(str)
   end
   painter:gend()
end


-- Create QT events
local timer = qt.QTimer()
timer.interval = 2
timer.singleShot = true
timer:start()
qt.connect(timer,'timeout()',
           function() 
              nPoints = widget.spinBox.value
              pointSize = widget.spinBox_2.value
              checkerror = widget.checkBox.checked
              process()
              timer:start()
           end)

qt.connect(qt.QtLuaListener(widget.pushButton_2),
           'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)',
           function (...) 
	      -- only if we are changing the process source,
	      --change the fps
	      if (target ~= 'neuflow') then
		last_fps = 1/neuFlow.profiler.list[1].reald
	      end
              target = 'neuflow'
              counting = counting+1
	      if not bytecode_loaded then
		 neuFlow:loadBytecode()
		 bytecode_loaded = true
	      end
            

	      last_name = 'cpu'
	   end)

qt.connect(qt.QtLuaListener(widget.pushButton),
           'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)',
           function (...) 
	      -- only if we are changing the process source,
	      --change the fps
	      if (target ~= 'cpu') then
		 last_fps = 1/neuFlow.profiler.list[1].reald
	      end
              target = 'cpu'
	      last_name = 'neuflow'
	   end)



-- Close Process
qt.connect(qt.QtLuaListener(widget),
           'sigShow(bool)',
           function(b) 
              if b then timer:start() end 
           end )

widget.windowTitle = "Attention Detection"
widget:show()


