# Tests with test images in Baseline/Hybrid
#
unset(tests)
if(VTK_DATA_ROOT)
  set(tests
    3DMorph
    TestFacetReader
    TestGreedyTerrainDecimation
    TestGridWarp3D
    TestGridWarpCubic
    TestGridWarpLinear
    TestGridWarpNearest
    TestImageStencilWithPolydata
    TestPCA
    TestProcrustes
    TestProcrustesRigidCentroid
    TestProjectedTerrainPath
    WarpPolyData
    appendImplicitModel
    appendImplicitModelNoBounds
    banana
    depthSort
    earth
    imageToPolyData
    largeImageOffset
    largeImageParallel
  )
endif()

foreach( tfile ${tests})
  add_test(NAME ${vtk-module}Tcl-${tfile} COMMAND ${VTK_TCL_EXE}
    ${vtkTestingRendering_SOURCE_DIR}/rtImageTest.tcl
    ${CMAKE_CURRENT_SOURCE_DIR}/${tfile}.tcl
    -D ${VTK_DATA_ROOT}
    -T ${VTK_TEST_OUTPUT_DIR}
    -V Baseline/Hybrid/${tfile}.png
   )
endforeach()
