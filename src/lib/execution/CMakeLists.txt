add_library(ink_execution STATIC runtime_value.cpp runtime_world.cpp interpreter.cpp)
add_library(ink::execution ALIAS ink_execution)

target_compile_features(ink_execution PUBLIC cxx_std_17)
target_include_directories(ink_execution PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_execution PUBLIC ink::ir ink::target)

if(MSVC)
  target_compile_options(ink_execution PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_execution PRIVATE -Wall -Wextra -Wpedantic)
endif()
