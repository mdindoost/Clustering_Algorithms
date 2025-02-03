# - Config file for the libleidenalg package
# It defines the following variables
#  LIBLEIDENALG_INCLUDEDIR - include directories for libleidenalg
#  LIBLEIDENALG_LIBRARIES    - libraries to link against

set(LIBLEIDENALG_VERSION "0.11.1")


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was libleidenalgConfig.cmake.in                            ########

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

# Compute paths
set_and_check(LIBLEIDENALG_INCLUDEDIR "${PACKAGE_PREFIX_DIR}/include")

# Our library dependencies (contains definitions for IMPORTED targets)
if(NOT TARGET libleidenalg AND NOT LIBLEIDENALG_BINARY_DIR)
  include("${CMAKE_CURRENT_LIST_DIR}/libleidenalgTargets.cmake")
endif()

# These are IMPORTED targets created by libleidenalgTargets.cmake
set_and_check(LIBLEIDENALG_LIBRARIES "${PACKAGE_PREFIX_DIR}/lib64")

check_required_components(libleidenalg)

