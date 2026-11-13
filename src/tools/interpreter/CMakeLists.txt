add_executable(ink_interpreter main.cpp)

target_compile_features(ink_interpreter PRIVATE cxx_std_23)
target_link_libraries(ink_interpreter PRIVATE ink::core ink::llvm spdlog::spdlog)

ink_disable_exceptions(ink_interpreter)
set_target_properties(ink_interpreter PROPERTIES OUTPUT_NAME ink-interpreter)

if(MSVC)
  target_compile_options(ink_interpreter PRIVATE /utf-8)
endif()
