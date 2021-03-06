find_package(TBB REQUIRED)
set(vtkCommonExecutionModel_SYSTEM_INCLUDE_DIRS ${TBB_INCLUDE_DIRS})

SET(Module_SRCS
  vtkAlgorithm.cxx
  vtkAlgorithmOutput.cxx
  vtkAnnotationLayersAlgorithm.cxx
  vtkArrayDataAlgorithm.cxx
  vtkCachedStreamingDemandDrivenPipeline.cxx
  vtkCastToConcrete.cxx
  vtkCompositeDataPipeline.cxx
  vtkCompositeDataSetAlgorithm.cxx
  vtkComputingResources.cxx
  vtkDataObjectAlgorithm.cxx
  vtkDataSetAlgorithm.cxx
  vtkDemandDrivenPipeline.cxx
  vtkDirectedGraphAlgorithm.cxx
  vtkExecutionScheduler.cxx
  vtkExecutionSchedulerManager.cxx
  vtkExecutiveCollection.cxx
  vtkExecutive.cxx
  vtkExtentSplitter.cxx
  vtkFilteringInformationKeyManager.cxx
  vtkGraphAlgorithm.cxx
  vtkHierarchicalBoxDataSetAlgorithm.cxx
  vtkHyperOctreeAlgorithm.cxx
  vtkHyperTreeGridAlgorithm.cxx
  vtkImageAlgorithm.cxx
  vtkImageInPlaceFilter.cxx
  vtkImageProgressIterator.cxx
  vtkImageToStructuredGrid.cxx
  vtkImageToStructuredPoints.cxx
  vtkInformationExecutivePortKey.cxx
  vtkInformationExecutivePortVectorKey.cxx
  vtkMultiBlockDataSetAlgorithm.cxx
  vtkMultiTimeStepAlgorithm.cxx
  vtkOnePieceExtentTranslator.cxx
  vtkPassInputTypeAlgorithm.cxx
  vtkPiecewiseFunctionAlgorithm.cxx
  vtkPiecewiseFunctionShiftScale.cxx
  vtkPointSetAlgorithm.cxx
  vtkPolyDataAlgorithm.cxx
  vtkRectilinearGridAlgorithm.cxx
  vtkScalarTree.cxx
  vtkSimpleImageToImageFilter.cxx
  vtkSimpleScalarTree.cxx
  vtkStreamingDemandDrivenPipeline.cxx
  vtkStructuredGridAlgorithm.cxx
  vtkTableAlgorithm.cxx
  vtkTableExtentTranslator.cxx
  vtkThreadedImageAlgorithm.cxx
  vtkThreadedStreamingPipeline.cxx
  vtkThreadedCompositeDataPipeline.cxx
  vtkTreeAlgorithm.cxx
  vtkTrivialProducer.cxx
  vtkUndirectedGraphAlgorithm.cxx
  vtkUnstructuredGridAlgorithm.cxx
  vtkSelectionAlgorithm.cxx
  vtkExtentRCBPartitioner.cxx
  vtkUniformGridPartitioner.cxx
  # New AMR classes
  vtkUniformGridAMRAlgorithm.cxx
  vtkOverlappingAMRAlgorithm.cxx
  vtkNonOverlappingAMRAlgorithm.cxx
  )

set_source_files_properties(
  vtkExecutive
  vtkHyperOctreeAlgorithm
  vtkHyperTreeGridAlgorithm
  vtkImageAlgorithm
  vtkImageInPlaceFilter
  vtkInformationExecutivePortKey
  vtkInformationExecutivePortVectorKey
  vtkScalarTree
  vtkSimpleImageToImageFilter
  vtkThreadedImageAlgorithm
  ABSTRACT
  )

set_source_files_properties(
  vtkExecutionSchedulerManager
  vtkFilteringInformationKeyManager
  vtkImageProgressIterator
  WRAP_EXCLUDE
  )

vtk_module_library(vtkCommonExecutionModel ${Module_SRCS})
target_link_libraries(vtkCommonExecutionModel ${TBB_LIBRARY})