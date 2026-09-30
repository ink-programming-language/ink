add_library(ink_semantic STATIC analyzer/analyzer.cpp analyzer/decl/analyzer_class.cpp analyzer/decl/analyzer_decl.cpp analyzer/decl/analyzer_enum.cpp analyzer/decl/analyzer_function.cpp analyzer/decl/analyzer_interface.cpp analyzer/expr/analyzer_call.cpp analyzer/expr/analyzer_conversion.cpp analyzer/expr/analyzer_expr.cpp analyzer/expr/analyzer_literal.cpp analyzer/expr/analyzer_name.cpp analyzer/expr/analyzer_paren.cpp analyzer/expr/analyzer_unary.cpp analyzer/stmt/analyzer_break.cpp analyzer/stmt/analyzer_classic_for.cpp analyzer/stmt/analyzer_comptime.cpp analyzer/stmt/analyzer_continue.cpp analyzer/stmt/analyzer_defer.cpp analyzer/stmt/analyzer_for_in.cpp analyzer/stmt/analyzer_if.cpp analyzer/stmt/analyzer_import.cpp analyzer/stmt/analyzer_return.cpp analyzer/stmt/analyzer_stmt.cpp analyzer/stmt/analyzer_switch.cpp analyzer/stmt/analyzer_while.cpp analyzer/stmt/analyzer_yield.cpp analyzer/type/analyzer_type.cpp name_resolve/name_resolver.cpp name_resolve/scope_store.cpp)
add_library(ink::semantic ALIAS ink_semantic)

target_compile_features(ink_semantic PUBLIC cxx_std_20)
target_include_directories(ink_semantic PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_semantic PUBLIC ink::ir PRIVATE ink::parser)
ink_disable_exceptions(ink_semantic)

if(MSVC)
  target_compile_options(ink_semantic PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_semantic PRIVATE -Wall -Wextra -Wpedantic)
endif()
