cmake_minimum_required(VERSION 3.10)
project(Clustering_Algorithms)

set(CMAKE_CXX_STANDARD 17)

# Set paths for headers and libraries
include_directories(${CMAKE_SOURCE_DIR}/external/install/include)
link_directories(${CMAKE_SOURCE_DIR}/external/install/lib64)

# Define relative runtime path
set(CMAKE_INSTALL_RPATH "$ORIGIN/../external/install/lib64")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# Define new clustering executable
add_executable(leiden_test src/leiden_wrapper.cpp src/run_leiden.cpp)
add_executable(leiden_clustering src/leiden_clustering.cpp src/run_leiden.cpp)

# Link with igraph and libleidenalg
target_link_libraries(leiden_test igraph libleidenalg)
target_link_libraries(leiden_clustering igraph libleidenalg)
