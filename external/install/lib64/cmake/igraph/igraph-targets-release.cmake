#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "igraph::igraph" for configuration "Release"
set_property(TARGET igraph::igraph APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(igraph::igraph PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libigraph.a"
  )

list(APPEND _cmake_import_check_targets igraph::igraph )
list(APPEND _cmake_import_check_files_for_igraph::igraph "${_IMPORT_PREFIX}/lib64/libigraph.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
