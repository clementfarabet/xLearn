
SET(src AbsModule.c
        AbsModuleHessian.c
        LcEncoder.c
        LcDecoder.c
        SpatialConvolutionTable.c
        SpatialConvolutionTableHessian.c
        SpatialMaxPooling.c
        SpatialSubSamplingHessian.c
        Threshold.c
        TanhHessian.c
        SparseCriterion.c
        toolBox.c
        init.c)

SET(luasrc AbsModule.lua
           AbsModuleHessian.lua
           AutoEncoderTrainer.lua
           core.lua
           CAddTable.lua
           CDivTable.lua
           CMulTable.lua
           CSubTable.lua
           ContrastNormalization.lua
           DataSet.lua
           DataSetLabelMe.lua
           DataList.lua
           Decider.lua
           Displayer.lua
           DrLimCriterion.lua
           GaborLayer.lua
           ImageRescale.lua
           ImageTransform.lua
           ImageSource.lua
           LcDecoder.lua
           LcEncoder.lua
           CCAdd.lua
           CCSub.lua
           LocalConnected.lua
           LocalNorm.lua
	   LocalNorm_hardware.lua
	   MalisCriterion.lua
           MSECriterionHessian.lua
	   NllCriterion.lua
           OptionParser.lua
           ParzenDecider.lua
           Power.lua
           Replicate.lua
           xPrompt.lua
           luaX.lua
           SequentialHessian.lua
           SpatialConvolutionTable.lua
           SpatialConvolutionTableHessian.lua
           SpatialLinear.lua
           SpatialLinearHessian.lua
           SpatialMaxPooling.lua
           SpatialPadding.lua
           SpatialSubSamplingHessian.lua
           Sqrt.lua
           Square.lua
           StochasticTrainer.lua
           StochasticHessianTrainer.lua
           TanhHessian.lua
           Threshold.lua
           Mult.lua
           Profiler.lua
           PyramidPacker.lua
           PyramidUnPacker.lua
           Trainer.lua
           UnsupDataSet.lua
           imageBox.lua
           init.lua
           lena.png
           table.lua
           toolBox.lua
           SparseCriterion.lua
           Video.lua
           VideoDataSet.lua)

ADD_TORCH_PACKAGE(XLearn "${src}" "${luasrc}")

TARGET_LINK_LIBRARIES(XLearn luaT TH)
