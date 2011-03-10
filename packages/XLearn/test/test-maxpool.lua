require 'nn'
require 'atn'

dofile('./test/test-jac.lua')

-- test the forward and backward function with non-table version
-- and check the jacobian

-- create input state sizes
inf = math.random(2,4)
oi = math.random(3,5)
oj = math.random(3,5)
ki = math.random(3,5)
kj = math.random(3,5)
di = math.random(1,2)
dj = math.random(1,2)
ini = (oi-1)*di+ki
inj = (oj-1)*dj+kj

-- create classes
-- ki=3
-- kj=3
-- di=1
-- dj=1
-- inf=1
-- ini=ki
-- inj=kj
mp = nn.SpatialMaxPooling(ki,kj,di,dj)

input = lab.rand(ini,inj,inf)*2-1
output = mp:forward(input)

-- for i=1,input:size(3) do
--    print(input:select(3,i))
-- end
-- for i=1,input:size(3) do
--    print(output:select(3,i))
-- end

for k=1,inf do
   local inmap = input:select(3,k)
   local out = output:select(3,k)
   local maxmap = mp.indices:select(3,k)
   local inu = inmap:unfold(1,ki,di):unfold(2,kj,dj)
   for i=1,out:size(1) do
      for j=1,out:size(2) do
	 local localinput = inu:select(1,i):select(1,j)
	 local omax = localinput:max()
	 local maxi = maxmap[i][j][1]
	 local maxj = maxmap[i][j][2]
	 if omax ~= out[i][j] then
	    print('omax = ' .. omax .. ' out[i][j] = ' .. out[i][j])
	    error('screwed ' .. k .. ' ' .. i .. ' ' .. j)
	 end
	 if localinput[maxi][maxj] ~= omax then
	    print('maxmap ...')
	    print(maxmap:select(3,1))
	    print(maxmap:select(3,2))
	    print('input')
	    print(localinput)
	    print('output')
	    print(out)
	    print(omax)
	    error('screwed input ' .. k .. ' ' .. i .. ' ' .. j)
	 end
      end
   end
end

print('forward OK')


function test_maxpool(from, ini, inj, ki, kj , si, sj)
   local module = nn.SpatialMaxPooling(ki,kj,di,dj)
   local input = torch.Tensor(ini, inj, inf):zero()
   
   io.write("input")
   test_jac(module, input)

   testwriting(module, input)
end

test_maxpool(inf,ini,inj,ki,kj,di,dj)
