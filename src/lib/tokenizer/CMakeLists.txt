add_library(ink_tokenizer STATIC token.cpp tokenizer.cpp unicode.cpp)
add_library(ink::tokenizer ALIAS ink_tokenizer)

target_compile_features(ink_tokenizer PUBLIC cxx_std_17)
target_include_directories(ink_tokenizer PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")

if(MSVC)
  target_compile_options(ink_tokenizer PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_tokenizer PRIVATE -Wall -Wextra -Wpedantic)
endif()
