file(GLOB_RECURSE INK_PARSER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_parser STATIC ${INK_PARSER_SOURCES})
add_library(ink::parser ALIAS ink_parser)

target_compile_features(ink_parser PUBLIC cxx_std_20)
target_include_directories(ink_parser PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_parser PUBLIC ink::core ink::tokenizer PRIVATE ink::llvm)
ink_disable_exceptions(ink_parser)

if(MSVC)
  target_compile_options(ink_parser PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_parser PRIVATE -Wall -Wextra -Wpedantic)
endif()
