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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.14.5/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.14.5/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/vivianhuang/desktop/terrier

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/vivianhuang/desktop/terrier/release

# Include any dependencies generated for this target.
include test/common/CMakeFiles/stat_registry_test.dir/depend.make

# Include the progress variables for this target.
include test/common/CMakeFiles/stat_registry_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/common/CMakeFiles/stat_registry_test.dir/flags.make

test/common/CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.o: test/common/CMakeFiles/stat_registry_test.dir/flags.make
test/common/CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.o: ../test/common/stat_registry_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/vivianhuang/desktop/terrier/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/common/CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.o"
	cd /Users/vivianhuang/desktop/terrier/release/test/common && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.o -c /Users/vivianhuang/desktop/terrier/test/common/stat_registry_test.cpp

test/common/CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.i"
	cd /Users/vivianhuang/desktop/terrier/release/test/common && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/vivianhuang/desktop/terrier/test/common/stat_registry_test.cpp > CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.i

test/common/CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.s"
	cd /Users/vivianhuang/desktop/terrier/release/test/common && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/vivianhuang/desktop/terrier/test/common/stat_registry_test.cpp -o CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.s

# Object files for target stat_registry_test
stat_registry_test_OBJECTS = \
"CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.o"

# External object files for target stat_registry_test
stat_registry_test_EXTERNAL_OBJECTS =

release/stat_registry_test: test/common/CMakeFiles/stat_registry_test.dir/stat_registry_test.cpp.o
release/stat_registry_test: test/common/CMakeFiles/stat_registry_test.dir/build.make
release/stat_registry_test: release/libtest_util.a
release/stat_registry_test: release/libterrier.a
release/stat_registry_test: googletest_ep-prefix/src/googletest_ep/lib/libgtest.a
release/stat_registry_test: googletest_ep-prefix/src/googletest_ep/lib/libgtest_main.a
release/stat_registry_test: release/libpg_query.a
release/stat_registry_test: /usr/local/lib/libevent.dylib
release/stat_registry_test: /usr/local/lib/libevent_pthreads.dylib
release/stat_registry_test: /usr/local/lib/libtbb.dylib
release/stat_registry_test: /usr/local/lib/libpqxx.dylib
release/stat_registry_test: /usr/local/lib/libpq.dylib
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMCJIT.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMExecutionEngine.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMRuntimeDyld.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86CodeGen.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMAsmPrinter.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMDebugInfoCodeView.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMDebugInfoMSF.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMGlobalISel.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMSelectionDAG.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMCodeGen.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMTarget.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMBitWriter.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMScalarOpts.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMInstCombine.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMTransformUtils.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMAnalysis.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMProfileData.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86AsmParser.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Desc.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMObject.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMBitReader.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMCParser.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86AsmPrinter.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Disassembler.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Info.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMCDisassembler.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMMC.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMX86Utils.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMCore.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMBinaryFormat.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMSupport.a
release/stat_registry_test: /usr/local/Cellar/llvm@6/6.0.1_1/lib/libLLVMDemangle.a
release/stat_registry_test: /usr/lib/libsqlite3.dylib
release/stat_registry_test: gflags_ep-prefix/src/gflags_ep/lib/libgflags.a
release/stat_registry_test: /usr/lib/libpthread.dylib
release/stat_registry_test: test/common/CMakeFiles/stat_registry_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/vivianhuang/desktop/terrier/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../release/stat_registry_test"
	cd /Users/vivianhuang/desktop/terrier/release/test/common && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/stat_registry_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/common/CMakeFiles/stat_registry_test.dir/build: release/stat_registry_test

.PHONY : test/common/CMakeFiles/stat_registry_test.dir/build

test/common/CMakeFiles/stat_registry_test.dir/clean:
	cd /Users/vivianhuang/desktop/terrier/release/test/common && $(CMAKE_COMMAND) -P CMakeFiles/stat_registry_test.dir/cmake_clean.cmake
.PHONY : test/common/CMakeFiles/stat_registry_test.dir/clean

test/common/CMakeFiles/stat_registry_test.dir/depend:
	cd /Users/vivianhuang/desktop/terrier/release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vivianhuang/desktop/terrier /Users/vivianhuang/desktop/terrier/test/common /Users/vivianhuang/desktop/terrier/release /Users/vivianhuang/desktop/terrier/release/test/common /Users/vivianhuang/desktop/terrier/release/test/common/CMakeFiles/stat_registry_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/common/CMakeFiles/stat_registry_test.dir/depend

