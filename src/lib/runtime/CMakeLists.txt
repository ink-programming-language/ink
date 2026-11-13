file(GLOB_RECURSE INK_RUNTIME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(ink_runtime STATIC ${INK_RUNTIME_SOURCES})
add_library(ink::runtime ALIAS ink_runtime)

target_compile_features(ink_runtime PUBLIC c_std_11)
target_include_directories(ink_runtime PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")

if(MSVC)
  target_compile_options(ink_runtime PRIVATE /utf-8 /W4)
else()
  target_compile_options(ink_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()
