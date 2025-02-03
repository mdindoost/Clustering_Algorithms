cmake_minimum_required(VERSION 3.10)
project(Clustering_Algorithms)

set(CMAKE_CXX_STANDARD 17)

# Set paths for headers and libraries
include_directories(${CMAKE_SOURCE_DIR}/external/install/include)
link_directories(${CMAKE_SOURCE_DIR}/external/install/lib64)

# Define Leiden wrapper executable
add_executable(leiden_test src/leiden_wrapper.cpp src/run_leiden.cpp)

# Link with igraph and libleidenalg
target_link_libraries(leiden_test
    ${CMAKE_SOURCE_DIR}/external/install/lib64/libigraph.a
    ${CMAKE_SOURCE_DIR}/external/install/lib64/liblibleidenalg.so
)
