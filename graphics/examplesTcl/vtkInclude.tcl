catch {load vtktcl}
set VTK_ASCII 1
set VTK_BINARY 2
set VTK_CELL_SIZE 512
set VTK_DIFFERENCE 2
set VTK_DIFFERENCE_OPERATOR 2
set VTK_EXTRACT_CELL_SEEDED_REGIONS 2
set VTK_EXTRACT_LARGEST_REGION 4
set VTK_EXTRACT_POINT_SEEDED_REGIONS 1
set VTK_EXTRACT_SPECIFIED_REGIONS 3
set VTK_FLAT    0
set VTK_GOURAUD 1
set VTK_HEXAHEDRON 12
set VTK_INSERT_POINTS 0
set VTK_INSERT_CELLS 1
set VTK_INTEGRATE_BACKWARD 1
set VTK_INTEGRATE_BOTH_DIRECTIONS 2
set VTK_INTEGRATE_FORWARD 0
set VTK_INTERSECTION 1
set VTK_INTERSECTION_OPERATOR 1
set VTK_LARGE_FLOAT 1.0e29
set VTK_LARGE_INTEGER 2147483646
set VTK_LINE 3
set VTK_MAX_CONTOURS 256
set VTK_MAX_SPHERE_RESOLUTION 1024
set VTK_NORMAL_EXTRUSION 2
set VTK_NULL_ELEMENT 0
set VTK_NUMBER_STATISTICS 12
set VTK_PHONG   2
set VTK_PIXEL 8
set VTK_POINTS    0
set VTK_POINT_EXTRUSION 3
set VTK_POLYGON 7
set VTK_POLY_LINE 4
set VTK_POLY_VERTEX 2
set VTK_QUAD 9
set VTK_SCALE_BY_SCALAR 0
set VTK_SCALE_BY_VECTOR 1
set VTK_SINGLE_POINT 0
set VTK_START_FROM_LOCATION 1
set VTK_START_FROM_POSITION 0
set VTK_STEREO_CRYSTAL_EYES 1
set VTK_STEREO_RED_BLUE     2
set VTK_SURFACE   2
set VTK_TENSOR_MAXDIM 3
set VTK_TETRA 10
set VTK_TOL 1.e-05
set VTK_TRIANGLE 5
set VTK_TRIANGLE_STRIP 6
set VTK_UNION 0
set VTK_UNION_OPERATOR 0
set VTK_USE_NORMAL 1
set VTK_USE_VECTOR 0
set VTK_VARY_RADIUS_OFF 0
set VTK_VARY_RADIUS_BY_SCALAR 1
set VTK_VARY_RADIUS_BY_VECTOR 2
set VTK_VECTOR_EXTRUSION 1
set VTK_VERTEX 1
set VTK_VOXEL 11
set VTK_WHOLE_MULTI_GRID_NO_IBLANKING 2
set VTK_WHOLE_SINGLE_GRID_NO_IBLANKING 0
set VTK_WIREFRAME 1
set VTK_XYZ_GRID 7
set VTK_XY_PLANE 4
set VTK_XZ_PLANE 6
set VTK_X_LINE 1
set VTK_YZ_PLANE 5
set VTK_Y_LINE 2
set VTK_Z_LINE 3


# A method to make an instance of a class with a unique name.
proc new {className} {
   set counterName [format {%sCounter} $className]
   global $counterName
   if {[info exists $counterName] == 0} {
      set $counterName 0
   }
   # Choose a name that is not being used
   set instanceName [format {%s%d} $className [incr $counterName]]
   while {[info commands $instanceName] != ""} {
      set instanceName [format {%s%d} $className [incr $counterName]]
   }
   # make the vtk object
   $className $instanceName

   return $instanceName
}