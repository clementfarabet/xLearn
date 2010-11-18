
----------------------------------------------------------------------
--- reads binary and dumps hex.
-- This function reads binary file and produces the hexa(text) file 
-- for mem simulation.
--
function toolBox.readBinWriteHex(input, output, word_width, requested_size_b)
   -- Files
   local file_hex = assert(io.open(output, "w"))
   local file_bin = assert(io.open(input, "rb"))

   print('# Converting bin code ['..input..'] to hex file ['
         ..output..'] using '..word_width..'bit words')

   -- convert word width in bytes
   word_width = word_width / 8

   -- get max nb of lines
   local requested_size_w = (requested_size_b or 0) / word_width

   -- Convert binary input to Hex output
   local lines = 0
   while true do
      local bytes = file_bin:read(word_width)
      if not bytes then 
	 file_bin:close()
	 break end
      local mem_word = {}
      local i = 0
      for b in string.gfind(bytes, ".") do
         mem_word[i] = b
         i = i+1
      end
      for i = #mem_word,0,-1 do
         file_hex:write(string.format("%02X", string.byte(mem_word[i])))
      end
      file_hex:write("\n")
      lines = lines + 1
      if lines == requested_size_w then
         break
      end
   end

   -- optional pad
   for k = 1,requested_size_w-lines do
      for i = 1,word_width do
         file_hex:write("00")
      end
      file_hex:write("\n")
   end

   file_hex:close()
end


function table.reverse(t)
   local out = {}
   for i = #t,1,-1 do
      table.insert(out, t[i])
   end
   return out
end

----------------------------------------------------------------------
--- extends math domain with approx functions.
--
function math.approx_old(args)
   -- parse args
   local mapping = args.mapping
   local min = args.min or -1.0
   local max = args.max or 1.0
   local nbSegments = args.nbSegments or 16
   local rangeSize = max - min
   local currentX = min
   local even = args.even or false
   local odd = args.odd or false
   local verbose = args.verbose or false
   local testStep = args.testStep or 0.01
   local Q = args.Q or nil
   
   

   if not args.mapping then error('# ERROR <math> : please provide a function') end
   if odd and even then error('# ERROR <math> : no mapping can be odd and even, com\'on!') end

   -- symmetric function
   if even or odd then
      rangeSize = max
      currentX = 0
   end

   -- get size of one segment
   local segmentSize = 1.0*rangeSize / nbSegments

   -- generate coefs for each segment
   coefs = {}
   for i = 1,nbSegments do
      local x1 = currentX
      local x2 = currentX + segmentSize
      local y1 = mapping(x1)
      local y2 = mapping(x2)
      local a = (y2 -y1) / (x2 - x1)
      local b = y1 - a*x1
      table.insert(coefs, {min=currentX,a=a,b=b})
      currentX = currentX + segmentSize
   end

   -- reverse coefs list
   coefs = table.reverse(coefs)

   -- and append type
   coefs.even = even
   coefs.odd = odd
   
   -- test on range
   local maxError = 0
   for x = min,max,testStep do
      local approxed
      local match = false
      local neg
      -- sym functions
      if (odd or even) and (x<0) then
	 xx = -x
	 neg = true
      else
	 xx = x
	 neg = false
      end
      -- generate approximation
      for i,coef in ipairs(coefs) do
	 if xx > coef.min then
	    approxed = coef.a * xx + coef.b
	    match = true
	    break
	 end
      end
      if not match then
	 approxed = coefs[#coefs].a * xx + coefs[#coefs].b
      end
      -- sym functions
      if odd and neg then approxed = -approxed end
      -- compare approx with true function
      if math.abs(mapping(x) - approxed) > maxError then
	 maxError = math.abs(mapping(x) - approxed)
      end
   end
   
   -- Quantize
   if Q then
      for i,coef in ipairs(coefs) do
	 coef.a = math.floor(coef.a*2^Q)
	 coef.b = math.floor(coef.b*2^Q)
	 coef.min = math.floor(coef.min*2^Q)
      end
   end

   -- verbose
   if verbose then
      io.write('# generated '..#coefs..' linears coefs: \n  (')
      for i,coef in ipairs(coefs) do 
	 io.write('(a='..coef.a..',b='..coef.b..',above='..coef.min..')')
      end
      print(')\n  max error is '..maxError)
   end

   -- return coefs 
   return coefs, maxError
end



-- This function returns true
-- if file with the name 'filename' exists, 
-- otherwise the function returns false 
function file_exists(filename)
   local file = io.open(filename)
   if file then
      io.close(file)
      return true
   else
      return false
   end
end

-- Lua does not have round() !
function math.round(num)
   num = num + 0.5
   return math.floor(num)
end

-- Functions that writes segments for linear 
-- approximations to file 
function write_coefs(coefs, filename)
   file = torch.DiskFile(filename, 'w')
   local odd
   local even
   if(coefs.odd == true)then
      odd = 1
   else odd = 0
   end
   if(coefs.even == true)then
      even = 1
   else even = 0
   end
   file:writeInt(odd)
   file:writeInt(even)
   file:writeInt(coefs.num_of_segs)
   
   for i,seg in ipairs(coefs) do
      
      file:writeInt(seg.a) 
      file:writeInt(seg.b) 
      file:writeInt(seg.min) 
    
   end
   file:close()
end

-- Functions that reads segments for linear 
-- approximations from file 
function read_coefs(filename)
   local coefs = {}
   
   file = torch.DiskFile(filename, 'r')
   local odd = file:readInt()
   local even = file:readInt()
   if(odd == 1)then
      coefs.odd = true
   else coefs.odd = false
   end
   if(even == 1)then
      coefs.even = true
   else coefs.even = false
   end

   coefs.num_of_segs = file:readInt()
   
   for i = 1,coefs.num_of_segs  do
      local a = file:readInt()
      local b = file:readInt()
      local min = file:readInt()

      table.insert(coefs, {min=min,a=a,b=b})
          
   end
   file:close()

   return coefs
end


-- The current function to find segments for 
-- linear approximation.
function math.approx2(args)
   print('in approx2')
   
   -- generating points
   local step = 1/256
   local start_range = args.min or 0
   local end_range = args.max or num.max
   -- error type:
   -- 0 - for relative error
   -- 1 - for absolute error
   local error_type = args.error_type or 0
   
   -- even -> f(-x) = f(x)
   -- odd  -> f(-x) = -f(x)
   -- so if the function is even or odd 
   -- generate segments for 0 to args.max
   -- and the hardware will take care of the negative values
   if (args.even or args.odd) then 
      start_range = 0
   end
   local num_of_points = (end_range-start_range)/step + 1
   local points = torch.Tensor(num_of_points) 
  
   print("num of points = ", num_of_points)
   points[1] = start_range 
   for i = 2,num_of_points do
      points[i] = points[i-1] + step
      -- DEBUG
      --print('point #', i,'is ', points[i])
   end

   -- generate mapping
   local mapping_func = args.mapping or math.sqrt
   local mapping = torch.Tensor(num_of_points)
   for i = 1,num_of_points do
      mapping[i] = mapping_func(points[i])
      -- DEBUG
      -- if (points[i] < 3) then
 	-- print('point ', points[i],'mapping is ', mapping[i])
       --end
   end

   -- set num of segments
   local num_of_segs = args.nbSegments or 8;

   -- in this table we store the segments,
   -- the information we need for each segment is: 
   -- a, b (to approx by a*x+b)
   -- and the starting point of the segment 
   local seg_table = {}

   -- set precision 
   local epsilon = args.epsilon or 1/256
   --25/256  -- sqrt(x/3)
   --19.7/256  --sqrt 
   --32.21/256 -- stdsigm[0,5.5]  
   --11.7/256  -- tanh[0,5], tanhabs [0,5] 
  
   

   local lo = epsilon
   local hi = 2*epsilon
   local got_less = false
   local real_num_of_segs = 1

   -- This wile loop will run the segments finding code
   -- for given precision
   -- If the num of segments is not equal to the number of segments required
   -- inrease the precision
   -- We use bisection algorithm to find good precision for given number of segments
   while (true) do
   
      real_num_of_segs = 1
      local curr_start_idx = 1
      local curr_a = 0
      local curr_b = 0
      
      local curr_mapping = torch.Tensor(num_of_points)
      local curr_mapping_fixed = torch.Tensor(num_of_points)
      local curr_approx = torch.Tensor(num_of_points)
      local curr_err = torch.Tensor(num_of_points)
      local points_fixed = torch.Tensor(num_of_points)
      local ax_fixed = torch.Tensor(num_of_points)
      local curr_approx_fixed = torch.Tensor(num_of_points)
      seg_table = {}

      -- This for loop is to perform the seach of segments
      for i = 2,num_of_points do
	 curr_a = (mapping[i] - mapping[curr_start_idx])/(points[i] - points[curr_start_idx])
	 curr_b = mapping[curr_start_idx] - curr_a*points[curr_start_idx]
	 
	 
	 local new_segment = false
	 
	 -- This code is to calculate the approximation
	 -- for all the points we covered so far in current segment 
	 -- and to fill up the error array
	 -- If at some point max error is greater than epsilon 
	 -- it means we need to save segment
	 local curr_idx = curr_start_idx
      
	 -- curr_mapping = mapping [curr_start_idx : i]
	 curr_mapping:resize(i - curr_start_idx+1)
	 curr_mapping:copy(mapping:narrow(1,curr_idx,i - curr_start_idx+1))
	 
	 -- for linear approx we do:
	 --       linear = a*t + b
	 -- now we want to use fixed point linear approx:
	 --
	 -- a_fixed = round(a*256)
	 -- t_fixed = round(t*256)
	 -- at_fixed = floor(a_fixed*t_fixed/256)
	 -- b_fixed = round(b*256)
	 -- linear_fixed = at_fixed + b_fixed
	 -- 
	 -- and go back:
	 --
	 -- linear = linear_fixed/256
	 --
	 -- note: we use floor for at_fixed because hardware 
	 -- does floor instead of round (which should be fixed really)
 
	 local curr_a_fixed = math.round(curr_a*256)
	 local curr_b_fixed = math.round(curr_b*256)

	 points_fixed:resize(i - curr_start_idx+1)
	 points_fixed:copy(points:narrow(1,curr_idx,i - curr_start_idx+1))
	 points_fixed:mul(256)
	 	 
	 ax_fixed:resize(i - curr_start_idx+1)
	 ax_fixed:copy(points_fixed)
	 ax_fixed:mul(curr_a_fixed)
	 ax_fixed:div(256)
	 ax_fixed:floor()

	 curr_approx_fixed:resize(i - curr_start_idx+1)
	 curr_approx_fixed:copy(ax_fixed)
	 curr_approx_fixed:add(curr_b_fixed)


	 curr_approx:resize(i - curr_start_idx+1)
	 curr_approx:copy(curr_approx_fixed)
	 curr_approx:div(256)
	 
	 curr_mapping_fixed:resize(i - curr_start_idx+1) 
 	 curr_mapping_fixed:copy(mapping:narrow(1,curr_idx,i - curr_start_idx+1))
 	 curr_mapping_fixed:mul(256)
	 curr_mapping_fixed:add(0.5) -- two step round, step 1 
	 curr_mapping_fixed:floor()  -- two step round, step 2
	 curr_mapping_fixed:div(256)

	 


	 -- curr_err = abs( curr_mapping - curr_approx)/ curr_mapping
	 curr_err:resize(i - curr_start_idx+1)
	 curr_err:copy(curr_approx)
	 curr_err:mul(-1)
	 curr_err:add(curr_mapping_fixed)
	 curr_err:abs()

	 if (error_type == 0) then -- we need relative error
	    curr_err:cdiv(curr_mapping_fixed:abs())
	    -- we might divide by 0
	    -- so we check the first 100 points of mapping
	    -- we assume that there are no zeros after 100 points...
	    --
	    -- note: the right thing here is to check evrything 
	    -- but it's too slow on lua...
	    
	    for idx_1 = 1, math.min(100,curr_err:size(1)) do
	       -- dividing by zero case
	       if(curr_mapping_fixed[idx_1] < 1/1024) then
		  if(curr_approx[idx_1] > 1/1024) then
		     curr_err[idx_1] = 1
		  else  curr_err[idx_1] = 0
		  end
	       end
	    end
	 end
	 
	 
-- DEBUG:
	--  if(i == num_of_points) then
-- 	    print('points_fixed:\n',points_fixed)
	    
-- 	    print('a_fixed = ', curr_a_fixed)
-- 	    print('ax_fixed:\n',ax_fixed)
-- 	    print('b_fixed = ', curr_b_fixed)
-- 	    print('curr_approx_fixed:\n',curr_approx_fixed)
-- 	    print('curr_approx_rec:\n',curr_approx_rec)
-- 	    print('curr_err:\n', curr_err)
-- 	    print('curr_err_fixed:\n', curr_err_fixed)
	 
-- 	    os.exit()
-- 	 end
	 -- DEBUG:
	 -- for j = 1, i - curr_start_idx do
-- 	  	local approx = curr_a*points[curr_idx] + curr_b
-- 	  	local err = math.abs(approx - mapping[curr_idx])
-- 	 	print('------------------------')
-- 	 	print('curr_idx = ', curr_idx, 'point = ', points[curr_idx],
-- 	    	       'curr_a = ', curr_a, 'curr_b = ', curr_b,
-- 	    	       'approx = ', approx, 'mapping = ',mapping[curr_idx],'error = ',err)
-- 	 	print('>>>>>>>>>>>>>>')
-- 	 	print('curr_idx = ', curr_idx, 'point = ', points[curr_idx],
-- 	 	      'curr_a = ', curr_a, 'curr_b = ', curr_b,
-- 	 	      'curr_approx = ', curr_approx[j], 'curr_mapping = ', curr_mapping[j],'curr_error = ',curr_err[j])
-- 		print('curr_idx = ', curr_idx, 'point_fixed = ', points_fixed[curr_idx],
-- 	 	      'curr_a_fixed = ', curr_a_fixed, 'curr_b_fixed = ', curr_b_fixed,
-- 	 	      'curr_approx_fixed = ', curr_approx_fixed[j], 'curr_mapping = ', curr_mapping[j],'curr_error = ',curr_err[j])
-- 	  	 curr_idx = curr_idx + 1
-- 	  	 -- if(err[j] > epsilon) then
-- 	 --  	    new_segment = true
-- 	 --  	    break
-- 	 --  	 end
-- 	 end

	 if (curr_err:max() > epsilon) then
	    new_segment = true
	 end
	 
	 if (new_segment) then
	    local a = (mapping[i-1] - mapping[curr_start_idx])/(points[i-1] - points[curr_start_idx])
	    local b = mapping[curr_start_idx] - a*points[curr_start_idx]
	    local strat_point = points[curr_start_idx]
	    table.insert(seg_table, {min=strat_point,a=a,b=b})
	    real_num_of_segs = real_num_of_segs + 1
	    curr_start_idx = i-1
	 end
	 
      end
      table.insert(seg_table, {min=points[curr_start_idx],a=curr_a,b=curr_b})
      print('real num of segments = ', real_num_of_segs)
--      print('segments:')
--      for i = 1, real_num_of_segs do
--  	 print('a(',i,') = ', seg_table[i].a)
--   	 print('b(',i,') = ', seg_table[i].b)
--   	 print('min(',i,') = ', seg_table[i].min)
--       end

     if (hi - lo > 1/1024) then
      --os.exit()
     -- if ( real_num_of_segs ~= num_of_segs) then
	 print('WARNING: did not reach the needed number of segments for precision: ', epsilon)
	 
	 if ( real_num_of_segs > num_of_segs) then
	    if (got_less) then
	       --lo = epsilon
	       hi = epsilon
	       epsilon = lo + (hi-lo)/2
	    else
	       epsilon = epsilon * 2
	       hi = epsilon
	       lo = epsilon/2
	    end
	 else
	    got_less = true
	    hi = epsilon
	    epsilon = hi - (hi-lo)/2
	 end
	 -- DEBUG
	 print('hi = ', hi*256, 'lo = ', lo*256, 'epsilon = ', epsilon*256)
	 
	 print('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> setting precision to: ', epsilon)
	 
      else 
	 print('WARNING: got to hi - lo < 1/512.... ')
	 print('number of segments reached = ', real_num_of_segs)
	 print('precision reached = ', epsilon)
	 print('exiting...')
	 break
      end
            
   end
   
   -- if for the current epsilon we got less than 8 segments
   -- we need to add more segments because hardware expects 8!
   if(real_num_of_segs < num_of_segs) then
      for i = 1, num_of_segs-real_num_of_segs do
	  table.insert(seg_table, {min=num.max,a=1,b=0})
      end
   end

   -- Prepare segments to return in the needed format:
   -- fixed point and order reversed
   local Q = args.Q or 8 
   
   seg_table = table.reverse(seg_table)
   seg_table.num_of_segs = 0
   for i,seg in ipairs(seg_table) do
      
      seg.a = math.round(seg.a*2^Q)
      seg.b = math.round(seg.b*2^Q)
      seg.min = math.round(seg.min*2^Q)
      
      seg_table.num_of_segs = seg_table.num_of_segs + 1
      
      print('a = ', seg.a)
      print('b = ', seg.b)
      print('min = ', seg.min)
   end

   -- and append type
   seg_table.even = args.even or false
   seg_table.odd = args.odd or false
   
   return seg_table

end


-- This function is used if we want segments to 
-- use linear approximation - mapping, on a linear function already
function math.approx_line(args)
   print('in approx_line')
   local num_of_segs = args.nbSegments or 8
   local even = args.even or false
   local odd = args.odd or false
   local Q = args.Q or nil
   local min = args.min or num.min
   local max = args.max or num.max

   local a = args.a
   local b = args.b

   if (args.even or args.odd) then 
      min = 0
   end

   local seg_table = {}

   table.insert(seg_table, {min=min,a=a,b=b})
   local real_num_of_segs = 1
   
   -- if for the set epsilon we got less than 8 segments
   -- we need to add more segments because hardware expects 8!
   if(real_num_of_segs < num_of_segs) then
      for i = 1, num_of_segs-real_num_of_segs do
	  table.insert(seg_table, {min=num.max,a=1,b=0})
      end
   end

   -- Prepare segments to return in the needed format:
   -- fixed point and order reversed
   local Q = args.Q or 8 
   
   seg_table = table.reverse(seg_table)
   seg_table.num_of_segs = 0
   for i,seg in ipairs(seg_table) do
      
      seg.a = math.round(seg.a*2^Q)
      seg.b = math.round(seg.b*2^Q)
      seg.min = math.round(seg.min*2^Q)
      
      seg_table.num_of_segs = seg_table.num_of_segs + 1
      
      -- print('a = ', seg.a)
--       print('b = ', seg.b)
--       print('min = ', seg.min)
   end

   -- and append type we put segments on the whole range now
   seg_table.even = args.even or false
   seg_table.odd = args.odd or false
   
   return seg_table
   

end

-- Function checks if the file with the given name exist
-- if not the function generates linear approximation segments
-- an writes them to file with the given file name
function math.approx(args)
   --print('coefpath = ',coefpath)
   --print('home_path = ',home_path)
      
   local filename = 'coef_'..args.name
   local coefs
   local filepath = coefpath..'/'..filename
   -- if (file_exists(paths.concat(paths.install_lua_path, 'NeuFlow/',filename))) then
   if (file_exists(filepath)) then 
      print('Reading from file segments for: ', filename)
      --coefs = read_coefs(paths.concat(paths.install_lua_path, 'NeuFlow/',filename))
      coefs = read_coefs(filepath)
   else
      print('There are no segments for requested mapping, generating segments for: ', filename)
      coefs = math.approx2(args)
      print('Now writing segments to file: ', filename)
      --write_coefs(coefs, paths.concat(paths.install_lua_path, 'NeuFlow/',filename))
      write_coefs(coefs, filepath)
      --print('Segments have been written. Now update_install and run again, exiting...')
      --os.exit()
   end
   return coefs
end