add_executable(ink_interpreter main.cpp)
target_compile_features(ink_interpreter PRIVATE cxx_std_17)
target_link_libraries(ink_interpreter PRIVATE ink::cli ink::ir ink::execution)

if(MSVC)
  target_compile_options(ink_interpreter PRIVATE /utf-8)
endif()
