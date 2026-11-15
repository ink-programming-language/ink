file(GLOB INK_RUNTIME_STUB_WINDOWS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_runtime_stub_windows STATIC ${INK_RUNTIME_STUB_WINDOWS_SOURCES})
add_library(ink::runtime_stub_windows ALIAS ink_runtime_stub_windows)

target_compile_features(ink_runtime_stub_windows PUBLIC cxx_std_23)
target_include_directories(ink_runtime_stub_windows PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_runtime_stub_windows PUBLIC ink::core PRIVATE kernel32)
ink_disable_exceptions(ink_runtime_stub_windows)

if(MSVC)
    target_compile_options(ink_runtime_stub_windows PRIVATE /utf-8 /W4 /permissive-)
else()
    target_compile_options(ink_runtime_stub_windows PRIVATE -Wall -Wextra -Wpedantic)
endif()
