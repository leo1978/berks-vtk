vtk_module(vtkFiltersExtraction
  GROUPS
    StandAlone
  DEPENDS
    vtkCommonExecutionModel
    vtkFiltersCore
  TEST_DEPENDS
    vtkIOLegacy
    vtkIOXML
    vtkRenderingOpenGL
    vtkTestingRendering
    vtkInteractionStyle
  )
