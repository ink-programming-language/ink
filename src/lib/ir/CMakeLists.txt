add_library(ink_ir STATIC analysis/type_layout.cpp analysis/verifier.cpp compilation/compilation_session.cpp compilation/module_name.cpp compilation/source_module_compiler.cpp instruction/control_flow.cpp instruction/instruction.cpp model/attribute.cpp model/constant.cpp model/constant_pool.cpp model/context.cpp model/function.cpp model/module.cpp model/name.cpp model/operand.cpp model/parameter.cpp model/struct_type.cpp model/type.cpp model/value.cpp serialization.cpp)
add_library(ink::ir ALIAS ink_ir)

target_compile_features(ink_ir PUBLIC cxx_std_17)
target_include_directories(ink_ir PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_ir PUBLIC ink::core PRIVATE ink::tokenizer)
ink_disable_exceptions(ink_ir)

if(MSVC)
  target_compile_options(ink_ir PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()
