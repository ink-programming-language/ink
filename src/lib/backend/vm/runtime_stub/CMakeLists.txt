file(GLOB INK_RUNTIME_STUB_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_runtime_stub STATIC ${INK_RUNTIME_STUB_SOURCES})
add_library(ink::runtime_stub ALIAS ink_runtime_stub)

target_compile_features(ink_runtime_stub PUBLIC cxx_std_23)
target_include_directories(ink_runtime_stub PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_runtime_stub PUBLIC ink::core PRIVATE ink::runtime)
ink_disable_exceptions(ink_runtime_stub)

if(WIN32)
    add_subdirectory(windows)
    target_link_libraries(ink_runtime_stub PRIVATE ink::runtime_stub_windows)
endif()

if(MSVC)
    target_compile_options(ink_runtime_stub PRIVATE /utf-8 /W4 /permissive-)
else()
    target_compile_options(ink_runtime_stub PRIVATE -Wall -Wextra -Wpedantic)
endif()
