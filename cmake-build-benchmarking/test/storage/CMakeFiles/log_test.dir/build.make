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

# Include any dependencies generated for this target.
include test/storage/CMakeFiles/log_test.dir/depend.make

# Include the progress variables for this target.
include test/storage/CMakeFiles/log_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/storage/CMakeFiles/log_test.dir/flags.make

test/storage/CMakeFiles/log_test.dir/log_test.cpp.o: test/storage/CMakeFiles/log_test.dir/flags.make
test/storage/CMakeFiles/log_test.dir/log_test.cpp.o: ../test/storage/log_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/storage/CMakeFiles/log_test.dir/log_test.cpp.o"
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/log_test.dir/log_test.cpp.o -c /Users/vivianhuang/Desktop/terrier/test/storage/log_test.cpp

test/storage/CMakeFiles/log_test.dir/log_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/log_test.dir/log_test.cpp.i"
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vivianhuang/Desktop/terrier/test/storage/log_test.cpp > CMakeFiles/log_test.dir/log_test.cpp.i

test/storage/CMakeFiles/log_test.dir/log_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/log_test.dir/log_test.cpp.s"
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vivianhuang/Desktop/terrier/test/storage/log_test.cpp -o CMakeFiles/log_test.dir/log_test.cpp.s

# Object files for target log_test
log_test_OBJECTS = \
"CMakeFiles/log_test.dir/log_test.cpp.o"

# External object files for target log_test
log_test_EXTERNAL_OBJECTS =

release/log_test: test/storage/CMakeFiles/log_test.dir/log_test.cpp.o
release/log_test: test/storage/CMakeFiles/log_test.dir/build.make
release/log_test: release/libtest_util.a
release/log_test: release/libterrier.a
release/log_test: googletest_ep-prefix/src/googletest_ep/lib/libgtest.a
release/log_test: googletest_ep-prefix/src/googletest_ep/lib/libgtest_main.a
release/log_test: release/libpg_query.a
release/log_test: /usr/local/lib/libevent.dylib
release/log_test: /usr/local/lib/libevent_pthreads.dylib
release/log_test: /usr/local/lib/libtbb.dylib
release/log_test: /usr/local/lib/libpqxx.dylib
release/log_test: /usr/local/lib/libpq.dylib
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMCJIT.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMExecutionEngine.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMRuntimeDyld.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86CodeGen.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMAsmPrinter.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMDebugInfoCodeView.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMDebugInfoMSF.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMGlobalISel.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMSelectionDAG.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMCodeGen.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMTarget.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMBitWriter.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMScalarOpts.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMInstCombine.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMTransformUtils.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMAnalysis.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMProfileData.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86AsmParser.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Desc.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMObject.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMBitReader.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMCParser.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86AsmPrinter.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Disassembler.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Info.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMCDisassembler.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMC.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Utils.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMCore.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMBinaryFormat.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMSupport.a
release/log_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMDemangle.a
release/log_test: /usr/lib/libsqlite3.dylib
release/log_test: gflags_ep-prefix/src/gflags_ep/lib/libgflags.a
release/log_test: /usr/lib/libpthread.dylib
release/log_test: test/storage/CMakeFiles/log_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../release/log_test"
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/log_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/storage/CMakeFiles/log_test.dir/build: release/log_test

.PHONY : test/storage/CMakeFiles/log_test.dir/build

test/storage/CMakeFiles/log_test.dir/clean:
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage && $(CMAKE_COMMAND) -P CMakeFiles/log_test.dir/cmake_clean.cmake
.PHONY : test/storage/CMakeFiles/log_test.dir/clean

test/storage/CMakeFiles/log_test.dir/depend:
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vivianhuang/Desktop/terrier /Users/vivianhuang/Desktop/terrier/test/storage /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/test/storage/CMakeFiles/log_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/storage/CMakeFiles/log_test.dir/depend

