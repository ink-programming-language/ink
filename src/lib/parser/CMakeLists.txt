add_library(ink_parser STATIC ast_context.cpp ast_children.cpp ast_walker.cpp ast_dump.cpp ast_verifier.cpp ast_serialization.cpp token_cursor.cpp parser.cpp parser_diagnostic_emitter.cpp parser_recovery.cpp parser_transaction.cpp parser_expr.cpp parser_type.cpp parser_stmt.cpp parser_decl.cpp parser_pattern.cpp)
add_library(ink::parser ALIAS ink_parser)

target_compile_features(ink_parser PUBLIC cxx_std_20)
target_include_directories(ink_parser PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_parser PUBLIC ink::core ink::tokenizer PRIVATE ink::llvm)
ink_disable_exceptions(ink_parser)

if(MSVC)
  target_compile_options(ink_parser PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_parser PRIVATE -Wall -Wextra -Wpedantic)
endif()
