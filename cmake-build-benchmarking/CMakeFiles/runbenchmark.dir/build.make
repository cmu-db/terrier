# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/vivianhuang/Desktop/terrier

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking

# Utility rule file for runbenchmark.

# Include the progress variables for this target.
include CMakeFiles/runbenchmark.dir/progress.make

CMakeFiles/runbenchmark:
	ctest -L benchmark

runbenchmark: CMakeFiles/runbenchmark
runbenchmark: CMakeFiles/runbenchmark.dir/build.make

.PHONY : runbenchmark

# Rule to build all files generated by this target.
CMakeFiles/runbenchmark.dir/build: runbenchmark

.PHONY : CMakeFiles/runbenchmark.dir/build

CMakeFiles/runbenchmark.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/runbenchmark.dir/cmake_clean.cmake
.PHONY : CMakeFiles/runbenchmark.dir/clean

CMakeFiles/runbenchmark.dir/depend:
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vivianhuang/Desktop/terrier /Users/vivianhuang/Desktop/terrier /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/CMakeFiles/runbenchmark.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/runbenchmark.dir/depend

