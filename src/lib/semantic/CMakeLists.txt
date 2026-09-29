add_library(ink_semantic STATIC analyzer/analyzer_break.cpp analyzer/analyzer_class.cpp analyzer/analyzer_classic_for.cpp analyzer/analyzer_comptime.cpp analyzer/analyzer_continue.cpp analyzer/analyzer_decl.cpp analyzer/analyzer_defer.cpp analyzer/analyzer_enum.cpp analyzer/analyzer_expr.cpp analyzer/analyzer_for_in.cpp analyzer/analyzer_function.cpp analyzer/analyzer_if.cpp analyzer/analyzer_import.cpp analyzer/analyzer_interface.cpp analyzer/analyzer_return.cpp analyzer/analyzer_stmt.cpp analyzer/analyzer_switch.cpp analyzer/analyzer_type.cpp analyzer/analyzer_while.cpp analyzer/analyzer_yield.cpp analyzer/analyzer.cpp name_resolve/name_resolver.cpp name_resolve/scope_store.cpp)
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
