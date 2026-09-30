file(GLOB_RECURSE INK_IR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_ir STATIC ${INK_IR_SOURCES})
add_library(ink::ir ALIAS ink_ir)

target_compile_features(ink_ir PUBLIC cxx_std_20)
target_include_directories(ink_ir PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
# Generic declarations retain parser AST references; IR never depends on semantic analysis.
target_link_libraries(ink_ir PUBLIC ink::core ink::parser PRIVATE ink::llvm)
ink_disable_exceptions(ink_ir)

if(MSVC)
  target_compile_options(ink_ir PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()
