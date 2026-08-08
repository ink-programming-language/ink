add_executable(ink_tokenize main.cpp)
target_compile_features(ink_tokenize PRIVATE cxx_std_17)
target_link_libraries(ink_tokenize PRIVATE ink::cli ink::tokenizer)
set_target_properties(ink_tokenize PROPERTIES OUTPUT_NAME ink-tokenize)

if(MSVC)
  target_compile_options(ink_tokenize PRIVATE /utf-8)
endif()
