----------------------------------------------------------------------
-- A simple script that trains a ConvNet on the MNIST dataset,
-- using stochastic gradient descent.
--
-- C.Farabet
----------------------------------------------------------------------

require 'XLearn'


----------------------------------------------------------------------
-- parse options
--
op = OptionParser('%prog [options]')
op:add_option{'-l', '--load', action='store', dest='network',
              help='path to existing [trained] network'}
op:add_option{'-s', '--save', action='store', dest='saveto',
              help='file name to save network [saving is done after each epoch]'}
op:add_option{'-d', '--dataset', action='store', dest='dataset',
              help='path to dataset'}
op:add_option{'-s', '--show', action='store', dest='nb_samples',
              help='show N samples from dataset'}
op:add_option{'-l', '--live', action='store_true', dest='livedisp',
              help='live display (during training)'}
op:add_option{'-p', '--preprocessonly', action='store_true', dest='preprocessonly',
              help='preprocess only'}
options,args = op:parse_args()


----------------------------------------------------------------------
-- To save networks
--
os.execute('mkdir -p scratch')


----------------------------------------------------------------------
-- ConvNet to train: CSCSCF
--

local nbClasses = 10
local connex = {6,16,120}
local fanin = {1,6,16}

-- Build network
convnet = nn.Sequential()
convnet:add(nn.SpatialConvolution(1,connex[1], 5, 5))
convnet:add(nn.Tanh())
convnet:add(nn.SpatialSubSampling(connex[1], 2, 2, 2, 2))
convnet:add(nn.Tanh())
convnet:add(nn.SpatialConvolution(connex[1],connex[2], 5, 5))
convnet:add(nn.Tanh())
convnet:add(nn.SpatialSubSampling(connex[2], 2, 2, 2, 2))
convnet:add(nn.Tanh())
convnet:add(nn.SpatialConvolution(connex[2],connex[3], 5, 5))
convnet:add(nn.Tanh())
convnet:add(nn.Reshape(connex[3]))
convnet:add(nn.Linear(connex[3],nbClasses))


----------------------------------------------------------------------
-- learning criterion: we modify the fprop to compute useful
-- error information
--
criterion = nn.MSECriterion()
criterion.sizeAverage = true


----------------------------------------------------------------------
-- trainer: std stochastic trainer
--
trainer = nn.StochasticTrainer(convnet, criterion)
trainer:setShuffle(false)
trainer.learningRate = 0.1
trainer.learningRateDecay = 0
trainer.maxEpoch = 50


----------------------------------------------------------------------
-- load datasets
--
path_dataset = options.dataset or '../datasets/mnist/'
path_trainData = paths.concat(path_dataset,'train-images-idx3-ubyte')
path_trainLabels = paths.concat(path_dataset,'train-labels-idx1-ubyte')
path_testData = paths.concat(path_dataset,'t10k-images-idx3-ubyte')
path_testLabels = paths.concat(path_dataset,'t10k-labels-idx1-ubyte')

trainData = {}
testData = {}

-- load data+labels
local data = toolBox.loadIDX(path_trainData):resize(28,28,10000)
local labels = toolBox.loadIDX(path_trainLabels):resize(10000)
for i=1,data:size(3) do
   local target = torch.Tensor(1,1,nbClasses):fill(-1)
   target[1][1][labels[i]+1] = 1
   local sample = torch.Tensor(32,32,1):fill(0)
   sample:narrow(1,3,28):narrow(2,3,28):copy(data:narrow(3,i,1)):mul(0.01)
   trainData[i] = {sample,target}
end
trainData.size = function (self) return #self end

-- load data+labels
data = toolBox.loadIDX(path_testData):resize(28,28,2000)
labels = toolBox.loadIDX(path_testLabels):resize(2000)
for i=1,data:size(3) do
   local target = torch.Tensor(1,1,nbClasses):fill(-1)
   target[1][1][labels[i]+1] = 1
   local sample = torch.Tensor(32,32,1):fill(0)
   sample:narrow(1,3,28):narrow(2,3,28):copy(data:narrow(3,i,1)):mul(0.01)
   testData[i] = {sample,target}
end
testData.size = function (self) return #self end

-- display ?
if options.nb_samples then
   
end


----------------------------------------------------------------------
-- training hooks
--
-- saving net
filename = paths.concat('scratch', (options.saveto or 'network-mnist')..'-'..os.date("%Y_%m_%d@%X"))
print('# network will be saved to ' .. filename)

trainer.hookTrainSample = function(trainer, sample)
   -- get prediction and target again:
   local _,prediction = lab.max(trainer.module.output)
   local _,target = lab.max(sample[2][1][1])

   -- compute the percentage of wrong labels
   local err = prediction[1] == target[1] and 0 or 1
   trainer.percentErrorTrain = trainer.percentErrorTrain + err

   -- accumulate a vector of diffs, to produce confusion matrix
   trainer.confusionTraining[prediction[1]][target[1]] = trainer.confusionTraining[prediction[1]][target[1]] + 1
   -- disp current sample
   if options.livedisp then
      if not trainer.window then
         toolBox.useQT()
         trainer.window = qtwidget.newwindow(trainData.patchSize*2,trainData.patchSize*2)
      end
      local trainSample = trainData['last']
      trainer.window:gbegin()
      trainer.window:showpage()
      image.qtdisplay{painter=trainer.window, tensor=trainSample[1], 
                      offset_y=20,
                      zoom=2, legend=trainSample[3]}
      trainer.window:gend()
   end
end

trainer.hookTrainEpoch = function(trainer)
   -- print error
   trainer.percentErrorTrain = trainer.percentErrorTrain / #trainData
   print('# current error (percent) = ' .. trainer.percentErrorTrain*100)
   -- and confusion
   print('# confusionTraining matrix:')
   print(trainer.confusionTraining)
   -- reset confusionTraining matrix and error
   trainer.confusionTraining:zero()
   trainer.percentErrorTrain = 0
   -- run on test_set
   trainer:test(testData)
   -- save net
   local file = torch.DiskFile(filename, 'w')
   trainer.module:write(file)
   file:close()
   print('# saving network to '..filename)
end

trainer.hookTestSample = trainer.hookTrainSample

trainer.hookTestEpoch = function(trainer)
   -- print error
   trainer.percentErrorTest = trainer.percentErrorTest / #testData
   print('# current Test error (percent) = ' .. trainer.percentErrorTest*100)
   -- and confusionTraining
   print('# confusionTesting matrix:')
   print(trainer.confusionTesting)
   -- reset confusionTraining matrix and error
   trainer.confusionTesting:zero()
   trainer.percentErrorTest = 0
end

trainer.hookTestSample = function(trainer, sample)
   local _,prediction = lab.max(trainer.module.output)
   local _,target = lab.max(sample[2][1][1])
   local err = prediction[1] == target[1] and 0 or 1
   trainer.percentErrorTest = trainer.percentErrorTest + err
   io.write('p',prediction[1],'t',target[1],'err',trainer.percentErrorTest,' ')
   -- accumulate a vector of diffs, to produce confusion matrix
   trainer.confusionTesting[prediction[1]][target[1]] = trainer.confusionTesting[prediction[1]][target[1]] + 1
   -- disp current sample
   if options.livedisp then
      if not trainer.window then
         toolBox.useQT()
         trainer.window = qtwidget.newwindow(testData.patchSize*2,testData.patchSize*2)
      end

      local testSample = testData['last']
      trainer.window:gbegin()
      trainer.window:showpage()
      image.qtdisplay{painter=trainer.window, tensor=testSample[1], 
                      offset_y=20,
                      zoom=2, legend=testSample[3]}
      trainer.window:gend()
   end
end
-- init confusion matrices and error
trainer.confusionTraining = lab.zeros(nbClasses,nbClasses)
trainer.percentErrorTrain = 0
trainer.confusionTesting = lab.zeros(nbClasses,nbClasses)
trainer.percentErrorTest = 0

----------------------------------------------------------------------
-- run trainer
--
if not options.preprocessonly then
   trainer:train(trainData)
end
