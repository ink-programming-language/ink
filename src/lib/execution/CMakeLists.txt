add_library(ink_execution STATIC engine/execution_engine.cpp engine/execution_frame.cpp engine/function_executor.cpp instruction/aggregate_instruction.cpp instruction/arithmetic_instruction.cpp instruction/branch_instruction.cpp instruction/call_instruction.cpp instruction/comparison_instruction.cpp instruction/instruction_dispatch.cpp instruction/memory_instruction.cpp instruction/module_instruction.cpp instruction/return_instruction.cpp module/compiling_module_provider.cpp module/dynamic_module_load_result.cpp module/module_instance.cpp module/module_loader.cpp native/native_abi.cpp native/native_symbol_registry.cpp runtime/read.cpp runtime/runtime.cpp runtime/runtime_memory.cpp runtime/runtime_value.cpp runtime/write.cpp)
add_library(ink::execution ALIAS ink_execution)

target_compile_features(ink_execution PUBLIC cxx_std_17)
target_include_directories(ink_execution PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ink_execution PUBLIC ink::core ink::ir PRIVATE ink::libffi ${CMAKE_DL_LIBS})
ink_disable_exceptions(ink_execution)

if(MSVC)
  target_compile_options(ink_execution PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_execution PRIVATE -Wall -Wextra -Wpedantic)
endif()
