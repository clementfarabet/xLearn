
require 'XLearn'
--require 'debugger'
require 'pink'

-- load sample and cleanup
img = image.lena():select(3,2)
img = image.convolve(img, image.gaussian(3,0.5), 'same')

-- convert input to edge graph
step = 'edge graph'
graph = pink.image2graph(img)
if _DEBUG_ then 
   image.displayList{images={graph},win_w=512,win_h=1024,legend=step}
   pause(step) 
end

-- convert back (khalimsky)
step = 'khalimsky of input edge graph'
back = pink.graph2image(graph)
if _DEBUG_ then
   image.displayList{images={back},win_w=1024,win_h=1024,legend=step}
   pause(step)
end

-- compute saliency
step = 'saliency'
saliency = pink.saliency(graph,'surface')
if _DEBUG_ then 
   image.displayList{images={saliency},win_w=512,win_h=1024,legend=step}
   pause(step) 
end
step = 'khalimsky of saliency'
sal_khal = pink.graph2image(saliency)
image.displayList{images={sal_khal},win_w=1024,win_h=1024,legend=step}

-- compute omega-saliency
step = 'omega-saliency'
osaliency = pink.saliency(saliency,'omega',img)
if _DEBUG_ then 
   image.displayList{images={osaliency},win_w=512,win_h=1024,legend=step}
   pause(step) 
end
step = 'khalimsky of omega-saliency'
osal_khal = pink.graph2image(osaliency)
image.displayList{images={osal_khal},win_w=1024,win_h=1024,legend=step}
