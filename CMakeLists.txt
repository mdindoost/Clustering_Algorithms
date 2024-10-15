cmake_minimum_required(VERSION 3.5)
project(Clustering_Algorithms)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #Disables compiler-specific extensions, promoting portability.

# Add the subdirectory containing your source files
add_subdirectory(src)
