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
include src/CMakeFiles/terrier_static.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/terrier_static.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/terrier_static.dir/flags.make

# Object files for target terrier_static
terrier_static_OBJECTS =

# External object files for target terrier_static
terrier_static_EXTERNAL_OBJECTS = \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/common/notifiable_task.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/common/utility.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/catalog_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/index_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/main_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/network_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/parser_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/settings_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/storage_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/test_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/loggers/transaction_logger.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/main/db_main.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/command_factory.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/connection_dispatcher_task.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/connection_handle.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/connection_handle_factory.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/connection_handler_task.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/network_io_wrapper.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/postgres/postgres_network_commands.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/postgres/postgres_protocol_interpreter.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/postgres/postgres_protocol_utils.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/network/terrier_server.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/optimizer/cost_model/trivial_cost_model.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/optimizer/group_expression.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/optimizer/logical_operators.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/optimizer/operator_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/optimizer/physical_operators.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/parser/expression/abstract_expression.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/parser/postgresparser.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/parser/select_statement.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/parser/table_ref.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/abstract_join_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/abstract_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/abstract_scan_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/aggregate_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/analyze_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_database_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_function_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_index_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_namespace_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_table_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_trigger_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_view_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/csv_scan_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/delete_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_database_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_index_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_namespace_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_table_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_trigger_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_view_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/export_external_file_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/hash_join_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/hash_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/index_scan_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/insert_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/limit_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/nested_loop_join_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/order_by_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/projection_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/result_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/seq_scan_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/set_op_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/planner/plannodes/update_plan_node.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/settings/settings_callbacks.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/settings/settings_manager.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/block_layout.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/data_table.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/garbage_collector.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/projected_columns.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/projected_row.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/record_buffer.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/sql_table.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/storage_util.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/tuple_access_strategy.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/write_ahead_log/log_io.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/storage/write_ahead_log/log_manager.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/traffic_cop/sqlite.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/transaction/transaction_manager.cpp.o" \
"/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_objlib.dir/__/third_party/bwtree/bwtree.cpp.o"

release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/common/notifiable_task.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/common/utility.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/catalog_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/index_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/main_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/network_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/parser_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/settings_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/storage_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/test_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/loggers/transaction_logger.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/main/db_main.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/command_factory.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/connection_dispatcher_task.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/connection_handle.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/connection_handle_factory.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/connection_handler_task.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/network_io_wrapper.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/postgres/postgres_network_commands.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/postgres/postgres_protocol_interpreter.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/postgres/postgres_protocol_utils.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/network/terrier_server.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/optimizer/cost_model/trivial_cost_model.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/optimizer/group_expression.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/optimizer/logical_operators.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/optimizer/operator_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/optimizer/physical_operators.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/parser/expression/abstract_expression.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/parser/postgresparser.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/parser/select_statement.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/parser/table_ref.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/abstract_join_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/abstract_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/abstract_scan_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/aggregate_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/analyze_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_database_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_function_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_index_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_namespace_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_table_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_trigger_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/create_view_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/csv_scan_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/delete_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_database_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_index_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_namespace_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_table_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_trigger_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/drop_view_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/export_external_file_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/hash_join_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/hash_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/index_scan_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/insert_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/limit_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/nested_loop_join_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/order_by_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/projection_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/result_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/seq_scan_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/set_op_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/planner/plannodes/update_plan_node.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/settings/settings_callbacks.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/settings/settings_manager.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/block_layout.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/data_table.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/garbage_collector.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/projected_columns.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/projected_row.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/record_buffer.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/sql_table.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/storage_util.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/tuple_access_strategy.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/write_ahead_log/log_io.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/storage/write_ahead_log/log_manager.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/traffic_cop/sqlite.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/transaction/transaction_manager.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_objlib.dir/__/third_party/bwtree/bwtree.cpp.o
release/libterrier.a: src/CMakeFiles/terrier_static.dir/build.make
release/libterrier.a: src/CMakeFiles/terrier_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX static library ../release/libterrier.a"
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src && $(CMAKE_COMMAND) -P CMakeFiles/terrier_static.dir/cmake_clean_target.cmake
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/terrier_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/terrier_static.dir/build: release/libterrier.a

.PHONY : src/CMakeFiles/terrier_static.dir/build

src/CMakeFiles/terrier_static.dir/clean:
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src && $(CMAKE_COMMAND) -P CMakeFiles/terrier_static.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/terrier_static.dir/clean

src/CMakeFiles/terrier_static.dir/depend:
	cd /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/vivianhuang/Desktop/terrier /Users/vivianhuang/Desktop/terrier/src /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src /Users/vivianhuang/Desktop/terrier/cmake-build-benchmarking/src/CMakeFiles/terrier_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/terrier_static.dir/depend

