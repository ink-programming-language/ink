add_library(ink_type STATIC type_context.cpp)
add_library(ink::type ALIAS ink_type)

target_compile_features(ink_type PUBLIC cxx_std_17)
target_include_directories(ink_type PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")

if(MSVC)
  target_compile_options(ink_type PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_type PRIVATE -Wall -Wextra -Wpedantic)
endif()
