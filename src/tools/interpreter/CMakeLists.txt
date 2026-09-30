file(GLOB_RECURSE INK_INTERPRETER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(ink_interpreter ${INK_INTERPRETER_SOURCES})
target_compile_features(ink_interpreter PRIVATE cxx_std_17)
target_link_libraries(ink_interpreter PRIVATE ink::cli ink::ir ink::execution)
ink_disable_exceptions(ink_interpreter)

if(MSVC)
  target_compile_options(ink_interpreter PRIVATE /utf-8)
endif()
