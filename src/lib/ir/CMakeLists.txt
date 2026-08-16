add_library(ink_ir STATIC context.cpp instruction.cpp serialization.cpp type.cpp value.cpp verifier.cpp)
add_library(ink::ir ALIAS ink_ir)

target_compile_features(ink_ir PUBLIC cxx_std_17)
target_include_directories(ink_ir PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_ir PUBLIC ink::core)

if(MSVC)
  target_compile_options(ink_ir PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()
