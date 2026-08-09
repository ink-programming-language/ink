add_library(ink_ir STATIC builder.cpp module.cpp opcode.cpp plan.cpp printer.cpp verifier.cpp)
add_library(ink::ir ALIAS ink_ir)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_custom_target(ink_ir_schema_check COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/tools/ir_schema/generate_ir_schema.py" --check DEPENDS "${PROJECT_SOURCE_DIR}/tools/ir_schema/generate_ir_schema.py" "${PROJECT_SOURCE_DIR}/tools/ir_schema/schema.py" "${PROJECT_SOURCE_DIR}/src/include/ink/ir/generated/opcode.def" "${PROJECT_SOURCE_DIR}/src/include/ink/ir/generated/plan_opcode.def" VERBATIM)
add_dependencies(ink_ir ink_ir_schema_check)

target_compile_features(ink_ir PUBLIC cxx_std_17)
target_include_directories(ink_ir PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_ir PUBLIC ink::core ink::target)

if(MSVC)
  target_compile_options(ink_ir PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()
