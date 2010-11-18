local Threshold, parent = torch.class('nn.Threshold','nn.Module')

function Threshold:__init(th,v)
   parent.__init(self)

   -- usage
   if (th and type(th) ~= 'number') or (v and type(v) ~= 'number') then
      error(toolBox.usage('nn.Threshold',
                          'a threhold module, if input < threshold, then output = value',
                          nil,
                          {type='number', help='threshold'},
                          {type='number', help='value'}))
   end

   self.threshold = th
   self.val = v or 0
end

-- function Threshold:forward(input)
--    self.output:resizeAs(input):copy(input)
--    self.output:apply(function (x) 
-- 			if x > self.threshold then 
-- 			   return x 
-- 			else 
-- 			   return self.val
-- 			end
-- 		     end)
--    return self.output
-- end

function Threshold:backward(input, gradOutput)
   self.gradInput:resizeAs(input):copy(gradOutput)
   self.gradInput:map(input,function(x,inx)
			       if inx > self.threshold then
				  return x
			       else
				  return 0
			       end
			    end)
   return self.gradInput
end

function Threshold:write(file)
   parent.write(self,file)
   file:writeDouble(self.threshold)
   file:writeDouble(self.val)
end

function Threshold:read(file)
   parent.read(self,file)
   self.threshold = file:readDouble()
   self.val = file:readDouble()
end
