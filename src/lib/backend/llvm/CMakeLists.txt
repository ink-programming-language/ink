add_library(ink_backend_llvm STATIC function_lowering.cpp llvm_backend.cpp lowering_context.cpp type_lowering.cpp value_lowering.cpp)
add_library(ink::llvm_backend ALIAS ink_backend_llvm)

target_compile_features(ink_backend_llvm PUBLIC cxx_std_17)
target_include_directories(ink_backend_llvm PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_backend_llvm PUBLIC ink::ir ink::llvm PRIVATE LLVMCore)
ink_disable_exceptions(ink_backend_llvm)

if(MSVC)
  target_compile_options(ink_backend_llvm PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_backend_llvm PRIVATE -Wall -Wextra -Wpedantic)
endif()
