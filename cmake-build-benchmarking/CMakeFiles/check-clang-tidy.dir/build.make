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

# Utility rule file for check-clang-tidy.

# Include the progress variables for this target.
include CMakeFiles/check-clang-tidy.dir/progress.make

CMakeFiles/check-clang-tidy:
	../build-support/run-clang-tidy.py -clang-tidy-binary /usr/local/opt/llvm@6/bin/clang-tidy -p /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking

check-clang-tidy: CMakeFiles/check-clang-tidy
check-clang-tidy: CMakeFiles/check-clang-tidy.dir/build.make

.PHONY : check-clang-tidy

# Rule to build all files generated by this target.
CMakeFiles/check-clang-tidy.dir/build: check-clang-tidy

.PHONY : CMakeFiles/check-clang-tidy.dir/build

CMakeFiles/check-clang-tidy.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/check-clang-tidy.dir/cmake_clean.cmake
.PHONY : CMakeFiles/check-clang-tidy.dir/clean

CMakeFiles/check-clang-tidy.dir/depend:
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vivianhuang/Desktop/terrier /Users/vivianhuang/Desktop/terrier /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/CMakeFiles/check-clang-tidy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/check-clang-tidy.dir/depend

