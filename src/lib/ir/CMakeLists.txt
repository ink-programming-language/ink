add_library(ink_ir STATIC constant/constant_pool.cpp context.cpp decl/decl.cpp function/basic_block.cpp function/function_parameter.cpp instruction/return_instruction.cpp instruction/store_instruction.cpp ir_builder.cpp module/module.cpp name/name_pool.cpp type/type_pool.cpp value.cpp)
add_library(ink::ir ALIAS ink_ir)

target_compile_features(ink_ir PUBLIC cxx_std_20)
target_include_directories(ink_ir PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
# Generic declarations retain parser AST references; IR never depends on semantic analysis.
target_link_libraries(ink_ir PUBLIC ink::core ink::parser)
ink_disable_exceptions(ink_ir)

if(MSVC)
  target_compile_options(ink_ir PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()
