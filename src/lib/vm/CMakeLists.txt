add_library(ink_vm STATIC instruction/opcode.cpp object/object_file_reader.cpp object/object_file_validator.cpp object/object_file_writer.cpp)
add_library(ink::vm ALIAS ink_vm)

target_compile_features(ink_vm PUBLIC cxx_std_23)
target_include_directories(ink_vm PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_vm PUBLIC ink::core)
ink_disable_exceptions(ink_vm)

if(MSVC)
  target_compile_options(ink_vm PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_vm PRIVATE -Wall -Wextra -Wpedantic)
endif()
