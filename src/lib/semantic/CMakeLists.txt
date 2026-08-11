file(GLOB INK_SEMANTIC_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_library(ink_semantic STATIC INK_SEMANTIC_SOURCES)
add_library(ink::semantic ALIAS ink_semantic)

target_compile_features(ink_semantic PUBLIC cxx_std_17)
target_include_directories(ink_semantic PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_semantic PUBLIC ink::core ink::tokenizer ink::parser)

if(MSVC)
  target_compile_options(ink_semantic PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_semantic PRIVATE -Wall -Wextra -Wpedantic)
endif()
