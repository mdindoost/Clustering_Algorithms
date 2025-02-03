# igraph-config.cmake
#
# igraph CMake package
#
# The following variables are set:
#
#   - IGRAPH_VERSION          - The igraph version string.
#   - IGRAPH_INTEGER_SIZE     - The integer size igraph was configured with (32 or 64).
#   - IGRAPH_GLPK_SUPPORT     - Whether igraph was compiled with GLPK support.
#   - IGRAPH_GRAPHML_SUPPORT  - Whether igraph was compiled with GraphML support.
#

set(IGRAPH_VERSION "0.10.15")
set(IGRAPH_INTEGER_SIZE 64)
set(IGRAPH_GLPK_SUPPORT ON)
set(IGRAPH_GRAPHML_SUPPORT OFF)

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was igraph-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/igraph-targets.cmake")

# Check whether C++ support is enabled; this is needed to ensure that programs
# that are dependent on igraph will get linked with the C++ linker and not the
# "plain" C linker
get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
if("CXX" IN_LIST LANGUAGES)
  # This is okay
else()
  message(FATAL_ERROR "Please enable C++ support in your project if you are linking to igraph.")
endif()

# Turn on CMP0012 because the following if() conditionals will use "ON" and
# "OFF" verbatim and they must be evaluated as booleans
cmake_policy(PUSH)
cmake_policy(SET CMP0012 NEW)
if(ON)
  find_package(OpenMP)
endif()
cmake_policy(POP)

check_required_components(igraph)
