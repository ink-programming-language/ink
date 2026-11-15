file(GLOB INK_VM_RUNTIME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_vm_runtime STATIC ${INK_VM_RUNTIME_SOURCES})
add_library(ink::vm_runtime ALIAS ink_vm_runtime)

target_compile_features(ink_vm_runtime PUBLIC cxx_std_23)
target_include_directories(ink_vm_runtime PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_vm_runtime PUBLIC ink::core PRIVATE ink::runtime_stub)
ink_disable_exceptions(ink_vm_runtime)

if(MSVC)
    target_compile_options(ink_vm_runtime PRIVATE /utf-8 /W4 /permissive-)
else()
    target_compile_options(ink_vm_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()
