# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_SOURCE_DIR = /Users/dpatra/Research/terrier

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo

# Include any dependencies generated for this target.
include test/execution/CMakeFiles/region_test.dir/depend.make

# Include the progress variables for this target.
include test/execution/CMakeFiles/region_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/execution/CMakeFiles/region_test.dir/flags.make

test/execution/CMakeFiles/region_test.dir/region_test.cpp.o: test/execution/CMakeFiles/region_test.dir/flags.make
test/execution/CMakeFiles/region_test.dir/region_test.cpp.o: ../test/execution/region_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/execution/CMakeFiles/region_test.dir/region_test.cpp.o"
	cd /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/region_test.dir/region_test.cpp.o -c /Users/dpatra/Research/terrier/test/execution/region_test.cpp

test/execution/CMakeFiles/region_test.dir/region_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/region_test.dir/region_test.cpp.i"
	cd /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dpatra/Research/terrier/test/execution/region_test.cpp > CMakeFiles/region_test.dir/region_test.cpp.i

test/execution/CMakeFiles/region_test.dir/region_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/region_test.dir/region_test.cpp.s"
	cd /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dpatra/Research/terrier/test/execution/region_test.cpp -o CMakeFiles/region_test.dir/region_test.cpp.s

# Object files for target region_test
region_test_OBJECTS = \
"CMakeFiles/region_test.dir/region_test.cpp.o"

# External object files for target region_test
region_test_EXTERNAL_OBJECTS =

relwithdebinfo/region_test: test/execution/CMakeFiles/region_test.dir/region_test.cpp.o
relwithdebinfo/region_test: test/execution/CMakeFiles/region_test.dir/build.make
relwithdebinfo/region_test: relwithdebinfo/libtest_util.a
relwithdebinfo/region_test: relwithdebinfo/libterrier.a
relwithdebinfo/region_test: googletest_ep-prefix/src/googletest_ep/lib/libgtest.a
relwithdebinfo/region_test: googletest_ep-prefix/src/googletest_ep/lib/libgmock_main.a
relwithdebinfo/region_test: gflags_ep-prefix/src/gflags_ep/lib/libgflags.a
relwithdebinfo/region_test: relwithdebinfo/libutil_static.a
relwithdebinfo/region_test: relwithdebinfo/libterrier.a
relwithdebinfo/region_test: gflags_ep-prefix/src/gflags_ep/lib/libgflags.a
relwithdebinfo/region_test: /usr/local/lib/libevent.dylib
relwithdebinfo/region_test: /usr/local/lib/libevent_pthreads.dylib
relwithdebinfo/region_test: /usr/local/lib/libtbb.dylib
relwithdebinfo/region_test: /usr/local/lib/libpqxx.dylib
relwithdebinfo/region_test: /usr/local/lib/libpq.dylib
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMMCJIT.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMExecutionEngine.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMRuntimeDyld.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86CodeGen.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMAsmPrinter.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMGlobalISel.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMSelectionDAG.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMCodeGen.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMTarget.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMipo.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMBitWriter.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMIRReader.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMAsmParser.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMInstrumentation.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMLinker.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMScalarOpts.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMAggressiveInstCombine.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMInstCombine.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMVectorize.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMTransformUtils.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMAnalysis.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMProfileData.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86AsmParser.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86Desc.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMObject.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMBitReader.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMCore.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMMCParser.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86AsmPrinter.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86Disassembler.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86Info.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMMCDisassembler.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMMC.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMBinaryFormat.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMDebugInfoCodeView.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMDebugInfoMSF.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMX86Utils.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMSupport.a
relwithdebinfo/region_test: /usr/local/Cellar/llvm@8/8.0.1/lib/libLLVMDemangle.a
relwithdebinfo/region_test: /usr/lib/libsqlite3.dylib
relwithdebinfo/region_test: relwithdebinfo/libpg_query.a
relwithdebinfo/region_test: test/execution/CMakeFiles/region_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../relwithdebinfo/region_test"
	cd /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/region_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/execution/CMakeFiles/region_test.dir/build: relwithdebinfo/region_test

.PHONY : test/execution/CMakeFiles/region_test.dir/build

test/execution/CMakeFiles/region_test.dir/clean:
	cd /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution && $(CMAKE_COMMAND) -P CMakeFiles/region_test.dir/cmake_clean.cmake
.PHONY : test/execution/CMakeFiles/region_test.dir/clean

test/execution/CMakeFiles/region_test.dir/depend:
	cd /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dpatra/Research/terrier /Users/dpatra/Research/terrier/test/execution /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution /Users/dpatra/Research/terrier/cmake-build-relwithdebinfo/test/execution/CMakeFiles/region_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/execution/CMakeFiles/region_test.dir/depend

