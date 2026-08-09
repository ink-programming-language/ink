add_library(ink_ast STATIC ast.cpp ast_context.cpp cst_lowering.cpp printer.cpp verifier.cpp)
add_library(ink::ast ALIAS ink_ast)

target_compile_features(ink_ast PUBLIC cxx_std_17)
target_include_directories(ink_ast PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_ast PUBLIC ink::core ink::parser)

if(MSVC)
  target_compile_options(ink_ast PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ast PRIVATE -Wall -Wextra -Wpedantic)
endif()
