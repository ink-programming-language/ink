add_library(ink_target STATIC target_context.cpp target_key.cpp)
add_library(ink::target ALIAS ink_target)

target_compile_features(ink_target PUBLIC cxx_std_17)
target_include_directories(ink_target PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")

if(MSVC)
  target_compile_options(ink_target PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_target PRIVATE -Wall -Wextra -Wpedantic)
endif()
