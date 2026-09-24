add_library(ink_semantic STATIC name_pool.cpp context.cpp decl.cpp)
add_library(ink::semantic ALIAS ink_semantic)

target_compile_features(ink_semantic PUBLIC cxx_std_20)
target_include_directories(ink_semantic PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_semantic PUBLIC ink::core ink::llvm)
ink_disable_exceptions(ink_semantic)

if(MSVC)
  target_compile_options(ink_semantic PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_semantic PRIVATE -Wall -Wextra -Wpedantic)
endif()
