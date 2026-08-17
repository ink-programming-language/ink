add_library(ink_parser STATIC cst.cpp parser.cpp)
add_library(ink::parser ALIAS ink_parser)

target_compile_features(ink_parser PUBLIC cxx_std_17)
target_include_directories(ink_parser PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_parser PUBLIC ink::core ink::tokenizer)
ink_disable_exceptions(ink_parser)

if(MSVC)
  target_compile_options(ink_parser PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_parser PRIVATE -Wall -Wextra -Wpedantic)
endif()
