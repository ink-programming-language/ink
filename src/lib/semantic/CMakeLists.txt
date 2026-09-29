add_library(ink_semantic STATIC analyzer/analyzer.cpp analyzer/analyzer_class.cpp analyzer/analyzer_comptime.cpp analyzer/analyzer_control_flow.cpp analyzer/analyzer_decl.cpp analyzer/analyzer_enum.cpp analyzer/analyzer_function.cpp analyzer/analyzer_import.cpp analyzer/analyzer_interface.cpp analyzer/analyzer_stmt.cpp ir_builder.cpp name_resolve/name_resolver.cpp name_resolve/scope_store.cpp model/constant/constant_pool.cpp model/decl/decl.cpp model/function/basic_block.cpp model/function/function_parameter.cpp model/instruction/return_instruction.cpp model/instruction/store_instruction.cpp model/module/module.cpp model/name/name_pool.cpp model/type/type_pool.cpp model/value.cpp)
add_library(ink::semantic ALIAS ink_semantic)

target_compile_features(ink_semantic PUBLIC cxx_std_20)
target_include_directories(ink_semantic PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_semantic PUBLIC ink::core PRIVATE ink::parser)
ink_disable_exceptions(ink_semantic)

if(MSVC)
  target_compile_options(ink_semantic PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_semantic PRIVATE -Wall -Wextra -Wpedantic)
endif()
