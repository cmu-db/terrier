# CMake generated Testfile for 
# Source directory: /Users/vivianhuang/Desktop/terrier/benchmark/integration
# Build directory: /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/benchmark/integration
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tpcc_benchmark "/Users/vivianhuang/Desktop/terrier/build-support/run-test.sh" "/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking" "benchmark" "/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/release//tpcc_benchmark" "--color_print=false")
set_tests_properties(tpcc_benchmark PROPERTIES  LABELS "benchmark" _BACKTRACE_TRIPLES "/Users/vivianhuang/Desktop/terrier/cmake_modules/BuildUtils.cmake;239;add_test;/Users/vivianhuang/Desktop/terrier/cmake_modules/BuildUtils.cmake;335;ADD_TERRIER_BENCHMARK;/Users/vivianhuang/Desktop/terrier/benchmark/integration/CMakeLists.txt;1;ADD_TERRIER_BENCHMARKS;/Users/vivianhuang/Desktop/terrier/benchmark/integration/CMakeLists.txt;0;")
