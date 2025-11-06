cmake_minimum_required(VERSION 3.10)
project(Clustering_Algorithms)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer vendored packages under external/install
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/install")
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Global include/link paths for your vendored tree
include_directories(${CMAKE_SOURCE_DIR}/external/install/include)
link_directories(${CMAKE_SOURCE_DIR}/external/install/lib64)

# RPATH so binaries can find vendored shared libs
set(CMAKE_INSTALL_RPATH "$ORIGIN/../external/install/lib64")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# -------------------------
# Existing executables
# -------------------------
add_executable(leiden_test
  src/leiden_wrapper.cpp
  src/run_leiden.cpp
)
add_executable(leiden_clustering
  src/leiden_clustering.cpp
  src/run_leiden.cpp
)
target_link_libraries(leiden_test igraph libleidenalg)
target_link_libraries(leiden_clustering igraph libleidenalg)

# -------------------------
# New: igraph backend (TSV + Parquet)
# Robust Arrow/Parquet discovery with fallbacks
# -------------------------
# Try config packages first (what we installed to external/install)
find_package(Arrow   CONFIG QUIET)
find_package(Parquet CONFIG QUIET)

# If imported targets don't exist, fall back to raw libs
if(NOT TARGET Arrow::arrow)
  message(WARNING "Arrow::arrow target not found; falling back to find_library()")
  find_library(ARROW_LIB NAMES arrow libarrow PATHS "${CMAKE_SOURCE_DIR}/external/install/lib64" REQUIRED)
endif()

if(NOT TARGET Parquet::parquet)
  message(WARNING "Parquet::parquet target not found; falling back to find_library()")
  find_library(PARQUET_LIB NAMES parquet libparquet PATHS "${CMAKE_SOURCE_DIR}/external/install/lib64" REQUIRED)
endif()

add_executable(leiden_igraph src/leiden_igraph.cpp)

# Be explicit: add include dir for igraph/arrow headers on this target
target_include_directories(leiden_igraph PRIVATE
  ${CMAKE_SOURCE_DIR}/external/install/include
)

# Link: prefer imported targets; otherwise link raw libraries we found
if(TARGET Arrow::arrow)
  target_link_libraries(leiden_igraph PRIVATE Arrow::arrow)
else()
  target_link_libraries(leiden_igraph PRIVATE ${ARROW_LIB})
endif()

if(TARGET Parquet::parquet)
  target_link_libraries(leiden_igraph PRIVATE Parquet::parquet)
else()
  target_link_libraries(leiden_igraph PRIVATE ${PARQUET_LIB})
endif()

# igraph is a plain library in your tree; link it directly
target_link_libraries(leiden_igraph PRIVATE igraph)
