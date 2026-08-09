add_library(ink_backend STATIC llvm_aot.cpp)
add_library(ink::backend ALIAS ink_backend)

llvm_map_components_to_libnames(INK_BACKEND_LLVM_LIBS Core Support Target CodeGen MC nativecodegen)

target_compile_features(ink_backend PUBLIC cxx_std_17)
target_include_directories(ink_backend PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_backend PUBLIC ink::ir ink::target PRIVATE ink::llvm ${INK_BACKEND_LLVM_LIBS})

if(MSVC)
  target_compile_options(ink_backend PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_backend PRIVATE -Wall -Wextra -Wpedantic)
endif()
