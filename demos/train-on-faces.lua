--------------------------------------------------------------------------------
-- A simple script that trains a ConvNet on a face dataset, using 
-- stochastic gradient descent.
-- 
-- TYPE:
-- $$ qlua train-on-faces.lua --help
--
-- Authors: Corda / Farabet
--------------------------------------------------------------------------------
require 'XLearn'

-- Option parsing
op = OptionParser('%prog [options]')
op:add_option{'-n', '--network', action='store', dest='network', 
              help='path to existing [trained] network'}
op:add_option{'-d', '--dataset', action='store', dest='dataset', 
              help='path to dataset'}
local options,args = op:parse_args()

-- criterion
local criterion = nn.MSECriterion()

function facesForward(self,prediction, target)
   local y,detected=lab.max(prediction)
   local y,known=lab.max(target)
   -- compute the percentage of wrong labels
   _,Tlabels = lab.max(target,3)
   _,Ilabels = lab.max(prediction,3)
   err = Tlabels:resize(1)[1] == Ilabels:resize(1)[1] and 0 or 1
   -- return 1 if mislabeled 0 othw
   return err
end
criterion.forward = facesForward

-- trainer
local trainer

-- make local scratch dir
os.execute('mkdir -p scratch')

-- First pass: create network
local convnet
if (options.network == nil) then
   -- Build network
   convnet = nn.Sequential()
   convnet:add(nn.LocalNorm(image.gaussian{width=7, amplitute=1}, 1))
   convnet:add(nn.SpatialConvolution(1, 6, 5, 5))
   convnet:add(nn.Tanh())
   convnet:add(nn.AbsModule())
   convnet:add(nn.SpatialSubSampling(6, 4, 4, 4, 4))
   convnet:add(nn.Tanh())
   convnet:add(nn.SpatialConvolutionTable(nn.SpatialConvolutionTable:KorayTable(6,20,4), 7, 7))
   convnet:add(nn.Tanh())
   convnet:add(nn.SpatialLinear(20,2))

   -- Learning Parameters
   trainer = nn.StochasticTrainer(convnet, criterion)
   trainer:setShuffle(false)
   trainer.learningRate = 0.001
   trainer.learningRateDecay = 0.01
   trainer.maxEpoch = 50
else
   -- reload trained network
   convnet = nn.Sequential()
   print('# reloading previously trained network')
   local file = torch.DiskFile(options.network, 'r')
   convnet:read(file)
   file:close()

   -- Learning Parameters
   trainer = nn.StochasticTrainer(convnet, criterion)
   trainer:setShuffle(false)
   trainer.learningRate = 0.0005
   trainer.learningRateDecay = 0.1
   trainer.maxEpoch = 50
end

-- Datasets path
datasetPath = options.dataset or '../datasets/faces_cut_yuv_32x32/'

dataFace = DataSet()
dataFace:load{dataSetFolder=datasetPath..'face', chanels=1, 
              cacheFile=datasetPath..'face'}
dataFace:shuffle()

dataFaceRendered = DataSet{dataSetFolder=datasetPath..'face-rendered', chanels=3, 
                           cacheFile=datasetPath..'face-rendered'}
dataFaceRendered:shuffle()
dataFaceRendered:apply(image.rgb2y)

dataBG = DataSet{dataSetFolder=datasetPath..'bg', chanels=1, 
                 cacheFile=datasetPath..'bg'}
dataBGext = DataSet{dataSetFolder=datasetPath..'bg-false-pos-interior-scene', chanels=1, 
                    cacheFile=datasetPath..'bg-false-pos-interior-scene'}
dataBG:appendDataSet(dataBGext)
dataBG:shuffle()

-- pop subset for testing
local testFace = dataFace:popSubset{ratio=0.2,outputSet = DataSet()}
local testBg = dataBG:popSubset{ratio=0.1,outputSet = DataSet()}

-- training set
local trainData = DataList()
trainData:appendDataSet(dataFace,'Faces')
trainData:appendDataSet(dataBG,'Bg')

-- testing set
local testData = DataList()
testData:appendDataSet(testFace,'Faces')
testData:appendDataSet(testBg,'Bg')

-- display sets
trainData:display{nbSamples=300, title='Training Set'}
testData:display{nbSamples=300, title='Test Set'}

-- training hooks
function saveNetTimeStamp(fileName, net)
   fileName = 'scratch/'..fileName..'-'..os.date("%Y_%m_%d@%X")
   local file = torch.DiskFile(fileName, 'w')
   net:write(file)
   file:close()
   print('# saving network to '..fileName)
end
function hookTrainSample(trainer, sample)
end
function hookTrainEpoch(trainer)
   print('# current error (percent) = ' .. trainer.currentError*100)
   trainer:test(testData)
   saveNetTimeStamp('network-faces', trainer.module)
end
function hookTestSample(trainer, sample)
end
function hookTestEpoch(trainer)
   print('# current error (percent) = ' .. trainer.currentError*100)
end
trainer.hookTrainSample = hookTrainSample
trainer.hookTrainEpoch = hookTrainEpoch
trainer.hookTestSample = hookTestSample
trainer.hookTestEpoch = hookTestEpoch

-- Run trainer
trainer:train(trainData)
