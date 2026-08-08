add_executable(ink_parse main.cpp)
target_compile_features(ink_parse PRIVATE cxx_std_17)
target_link_libraries(ink_parse PRIVATE ink::cli ink::parser)
set_target_properties(ink_parse PROPERTIES OUTPUT_NAME ink-parse)

if(MSVC)
  target_compile_options(ink_parse PRIVATE /utf-8)
endif()
