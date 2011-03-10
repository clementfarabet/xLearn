--==============================================================================
-- File: sys
--
-- Description: A wrapper for using mincut maxflow algorithm from 
-- ##################################################################

-- 2. License & disclaimer.

--     Copyright 2001-2006 Vladimir Kolmogorov (v.kolmogorov@cs.ucl.ac.uk), Yuri Boykov (yuri@csd.uwo.ca).

--     This software can be used for research purposes only.
--     If you require another license, you may consider using version 2.21
--     (which implements exactly the same algorithm, but does not have the option of reusing search trees).

--     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
--     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
--     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
--     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
--     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
--     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
--     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
--     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
--     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
--     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
--     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-- ##################################################################
--              
--
-- Created: February 15, 2011, 2.34PM
--
-- Author: Benoit Corda -- cordaben@gmail.com
--==============================================================================

require 'libmincut'

local segment_help_desc =
[[Computes a segmentation map given an input image.

The map is computed using mincut ideas,
as presented in "An Experimental Comparison of Min-Cut/Max-Flow Algorithms for Energy Minimization in Vision.", by
Yuri Boykov and Vladimir Kolmogorov.

The input image must be a WxHx4 tensor, where the channels are
RGB and the depth (from the kinect)

Or just a RGB image with another tensor for depth]]

if not MinCutsegmLoaded then
   require 'XLearn'
   mincut = {}

   mincut.segmentation
      = function(...)
           local args, img = toolBox.unpack(
              {...},
              ' mincut.segmentation', segment_help_desc,
              {arg='image', type='torch.Tensor', help='input image (WxHx3 or WxHx4 (with depth))', req=true},
              {arg='depth', type='torch.Tensor', help='input depth WxH', req=false},
              {arg='result', type='torch.Tensor', help='output result (WxHx3 tensor)', req=false},
              {arg='threshold', type='int', help='threshold for segmentation  between 0 and 2048', default=250}
           )
           local w = args.image:size(1)
           local h = args.image:size(2)
           if args.result == nil then
              args.result = torch.Tensor(w,h,3)
           end
           local input = args.image
           local d = args.depth
           -- if 4 channels split into 2 tensors
           if args.image:size(3) == 4 then
              input = args.image:narrow(3,1,3)
              d = args.image:select(3,4)
           end
           if d == nil then
              error([[# ERROR<mincut.segmentation>: you must provide either a 4 channels 
                          image tensor or a depth]])
           end
           args.result:resizeAs(input)
           -- run the segmentation
           libmincut.segment(input,d,args.result,args.threshold)

           return args.result
        end
   
   mincut.testme
      = function()
           require 'XLearn'
           require 'Kinect'
           local a = Kinect(640,480)
           local d = Displayer()
           local res = torch.Tensor(640,480,3)
           --while true do   
           a:getRGBD()
           mincut.segmentation{image = a.rgbd,result=res,threshold=1024}
           d:show{tensor = res,legend="Result of the mincut segmentation"}
           image.display{image=a.rgbd,zoom=0.5,legend="Input Data"}
           --end
           return res
        end

   MinCutsegmLoaded = true
end

return mincut