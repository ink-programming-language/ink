file(GLOB_RECURSE INK_VM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_vm STATIC ${INK_VM_SOURCES})
add_library(ink::vm ALIAS ink_vm)

target_compile_features(ink_vm PUBLIC cxx_std_23)
target_include_directories(ink_vm PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_vm PUBLIC ink::core PRIVATE ink::runtime)
ink_disable_exceptions(ink_vm)

if(MSVC)
  target_compile_options(ink_vm PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_vm PRIVATE -Wall -Wextra -Wpedantic)
endif()
