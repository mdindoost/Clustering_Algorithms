# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /scratch/shared/apps/cmake/bin/cmake

# The command to remove a file.
RM = /scratch/shared/apps/cmake/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /scratch/users/md724/Clustering_Algorithms

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /scratch/users/md724/Clustering_Algorithms/build

# Include any dependencies generated for this target.
include src/CMakeFiles/example_runLeiden.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/example_runLeiden.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/example_runLeiden.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/example_runLeiden.dir/flags.make

src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o: src/CMakeFiles/example_runLeiden.dir/flags.make
src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o: /scratch/users/md724/Clustering_Algorithms/src/example_runLeiden.cpp
src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o: src/CMakeFiles/example_runLeiden.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/users/md724/Clustering_Algorithms/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o -MF CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o.d -o CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o -c /scratch/users/md724/Clustering_Algorithms/src/example_runLeiden.cpp

src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.i"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/users/md724/Clustering_Algorithms/src/example_runLeiden.cpp > CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.i

src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.s"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/users/md724/Clustering_Algorithms/src/example_runLeiden.cpp -o CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.s

src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o: src/CMakeFiles/example_runLeiden.dir/flags.make
src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o: /scratch/users/md724/Clustering_Algorithms/src/runLeiden.cpp
src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o: src/CMakeFiles/example_runLeiden.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/users/md724/Clustering_Algorithms/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o -MF CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o.d -o CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o -c /scratch/users/md724/Clustering_Algorithms/src/runLeiden.cpp

src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/example_runLeiden.dir/runLeiden.cpp.i"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/users/md724/Clustering_Algorithms/src/runLeiden.cpp > CMakeFiles/example_runLeiden.dir/runLeiden.cpp.i

src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/example_runLeiden.dir/runLeiden.cpp.s"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/users/md724/Clustering_Algorithms/src/runLeiden.cpp -o CMakeFiles/example_runLeiden.dir/runLeiden.cpp.s

# Object files for target example_runLeiden
example_runLeiden_OBJECTS = \
"CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o" \
"CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o"

# External object files for target example_runLeiden
example_runLeiden_EXTERNAL_OBJECTS =

src/example_runLeiden: src/CMakeFiles/example_runLeiden.dir/example_runLeiden.cpp.o
src/example_runLeiden: src/CMakeFiles/example_runLeiden.dir/runLeiden.cpp.o
src/example_runLeiden: src/CMakeFiles/example_runLeiden.dir/build.make
src/example_runLeiden: src/CMakeFiles/example_runLeiden.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/scratch/users/md724/Clustering_Algorithms/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable example_runLeiden"
	cd /scratch/users/md724/Clustering_Algorithms/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/example_runLeiden.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/example_runLeiden.dir/build: src/example_runLeiden
.PHONY : src/CMakeFiles/example_runLeiden.dir/build

src/CMakeFiles/example_runLeiden.dir/clean:
	cd /scratch/users/md724/Clustering_Algorithms/build/src && $(CMAKE_COMMAND) -P CMakeFiles/example_runLeiden.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/example_runLeiden.dir/clean

src/CMakeFiles/example_runLeiden.dir/depend:
	cd /scratch/users/md724/Clustering_Algorithms/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /scratch/users/md724/Clustering_Algorithms /scratch/users/md724/Clustering_Algorithms/src /scratch/users/md724/Clustering_Algorithms/build /scratch/users/md724/Clustering_Algorithms/build/src /scratch/users/md724/Clustering_Algorithms/build/src/CMakeFiles/example_runLeiden.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/example_runLeiden.dir/depend

