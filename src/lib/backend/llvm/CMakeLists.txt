add_library(ink_backend_llvm STATIC llvm_backend.cpp lowering_context.cpp type_lowering.cpp function_lowering.cpp reflection/type_desc_lowering.cpp hybrid_lowering.cpp)
add_library(ink::llvm_backend ALIAS ink_backend_llvm)
target_compile_features(ink_backend_llvm PUBLIC cxx_std_20)
target_include_directories(ink_backend_llvm PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
llvm_map_components_to_libnames(INK_BACKEND_LLVM_LIBRARIES Core Support Analysis Passes Target CodeGen MC native)
target_link_libraries(ink_backend_llvm PUBLIC ink::ir ink::execution ink::llvm PRIVATE ${INK_BACKEND_LLVM_LIBRARIES})
ink_disable_exceptions(ink_backend_llvm)

add_library(ink_aot_runtime STATIC runtime.cpp)
target_compile_features(ink_aot_runtime PRIVATE cxx_std_20)
ink_disable_exceptions(ink_aot_runtime)

# Link the generated object with the VM and the small native panic helpers statically.
add_library(ink_hybrid_runtime INTERFACE)
add_library(ink::hybrid_runtime ALIAS ink_hybrid_runtime)
target_link_libraries(ink_hybrid_runtime INTERFACE ink::execution ink_aot_runtime)

if(MSVC)
  target_compile_options(ink_backend_llvm PRIVATE /utf-8 /W4 /permissive-)
  target_compile_options(ink_aot_runtime PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_backend_llvm PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(ink_aot_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()
