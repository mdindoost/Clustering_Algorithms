# src/CMakeLists.txt

# Include directories for igraph and libleidenalg
include_directories(
    ${CMAKE_SOURCE_DIR}/igraph/install/include
    ${CMAKE_SOURCE_DIR}/libleidenalg/install/include
)

# Link directories for igraph and libleidenalg
link_directories(
    ${CMAKE_SOURCE_DIR}/igraph/install/lib64
    ${CMAKE_SOURCE_DIR}/libleidenalg/install/lib64
)

# Create executable
add_executable(clustering_app main.cpp MyGraph.cpp)

# Link libraries
target_link_libraries(clustering_app
    igraph
    libleidenalg
)
# Create executable for the runLeiden example
add_executable(example_runLeiden example_runLeiden.cpp runLeiden.cpp)

# Link libraries to example_runLeiden
target_link_libraries(example_runLeiden
    igraph
    libleidenalg
)