local SpatialLogSoftMax, parent = torch.class('nn.SpatialLogSoftMax', 'nn.Module')

function SpatialLogSoftMax:__init()
   parent.__init(self)
end

-- define fprop in C
SpatialLogSoftMax.forward_c = inline.load [[
      // get all params
      const void* torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
      THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
      THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

      // dims
      int width = input->size[0];
      int height = input->size[1];

      // select planes
      THTensor *input_row = THTensor_new();
      THTensor *input_point = THTensor_new();
      THTensor *output_row = THTensor_new();
      THTensor *output_point = THTensor_new();

      // process the whole plane
      int x,y;
      for (y=0; y<height; y++) {
         THTensor_select(input_row, input, 1, y);
         THTensor_select(output_row, output, 1, y);
         for (x=0; x<width; x++) {
            THTensor_select(input_point, input_row, 0, x);
            THTensor_select(output_point, output_row, 0, x);

            double sum = THLogZero;

            TH_TENSOR_APPLY2(double, output_point, double, input_point, \
                             double z = *input_point_p; \
                             *output_point_p = z; \
                             sum = THLogAdd(sum, z);)

            THTensor_add(output_point, -sum);
         }
      }

      // cleanup
      THTensor_free(input_row);
      THTensor_free(input_point);
      THTensor_free(output_row);
      THTensor_free(output_point);
      return 1;
]]

function SpatialLogSoftMax:forward(input)
   self.output:resizeAs(input)
   self:forward_c(input)
   return self.output
end

-- define bprop in C
SpatialLogSoftMax.backward_c = inline.load [[
      // get all params
      const void* torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
      THTensor *input = luaT_checkudata(L, 2, torch_Tensor_id);
      THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor_id);
      THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor_id);
      THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor_id);

      // dims
      int width = input->size[0];
      int height = input->size[1];

      // resize gradInput
      THTensor_zero(gradInput);

      // select planes
      THTensor *gradOutput_row = THTensor_new();
      THTensor *gradOutput_point = THTensor_new();
      THTensor *gradInput_row = THTensor_new();
      THTensor *gradInput_point = THTensor_new();
      THTensor *output_row = THTensor_new();
      THTensor *output_point = THTensor_new();

      // compute gradients for each point
      int x,y;
      for (y=0; y<height; y++) {
         THTensor_select(gradInput_row, gradInput, 1, y);
         THTensor_select(gradOutput_row, gradOutput, 1, y);
         THTensor_select(output_row, output, 1, y);
         for (x=0; x<width; x++) {
            THTensor_select(gradInput_point, gradInput_row, 0, x);
            THTensor_select(gradOutput_point, gradOutput_row, 0, x);
            THTensor_select(output_point, output_row, 0, x);

            double sum = THTensor_sum(gradOutput_point);

            TH_TENSOR_APPLY3(double, gradInput_point, \
                             double, gradOutput_point, \
                             double, output_point, \
                             *gradInput_point_p = *gradOutput_point_p - exp(*output_point_p)*sum;);
         }
      }

      // cleanup
      THTensor_free(gradInput_row);
      THTensor_free(gradInput_point);
      THTensor_free(gradOutput_row);
      THTensor_free(gradOutput_point);
      THTensor_free(output_row);
      THTensor_free(output_point);

      return 1;
]]

function SpatialLogSoftMax:backward(input, gradOutput)
   self.gradInput:resizeAs(input)
   self:backward_c(input, gradOutput)
   return self.gradInput
end

function SpatialLogSoftMax.testme()
   local lsm = nn.LogSoftMax()
   local slsm = nn.SpatialLogSoftMax()
   local inp = lab.rand(4,4,3)
   local gradout = lab.rand(4,4,3)/10
   print('input:',inp)
   print('gradOutput:',gradout)
   local outp = slsm:forward(inp)
   print('output:',outp)
   local outp_r = torch.Tensor():resizeAs(outp)
   for i = 1,4 do
      for j = 1,4 do
         outp_r[i][j]:copy( lsm:forward(inp[i][j]) )
      end
   end
   print('output (groundtruth):',outp_r)
   local gradin = slsm:backward(inp,gradout)
   print('gradInput:',gradin)
   local gradin_r = torch.Tensor():resizeAs(gradin)
   for i = 1,4 do
      for j = 1,4 do
         lsm:forward(inp[i][j])
         gradin_r[i][j]:copy( lsm:backward(inp[i][j],gradout[i][j]) )
      end
   end
   print('gradInput (groundtruth):',gradin_r)
   print('error on output',(outp-outp_r):abs():sum())
   print('error on gradInput',(gradin-gradin_r):abs():sum())
end