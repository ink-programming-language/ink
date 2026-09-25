add_library(ink_semantic STATIC analyze/analyzer.cpp context.cpp name_resolve/name_resolver.cpp model/constant/constant_pool.cpp model/function/basic_block.cpp model/function/function_parameter.cpp model/instruction/return_instruction.cpp model/instruction/store_instruction.cpp model/module/module.cpp model/name/name_pool.cpp)
add_library(ink::semantic ALIAS ink_semantic)

target_compile_features(ink_semantic PUBLIC cxx_std_20)
target_include_directories(ink_semantic PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_semantic PUBLIC ink::core PRIVATE ink::parser)
ink_disable_exceptions(ink_semantic)

if(MSVC)
  target_compile_options(ink_semantic PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_semantic PRIVATE -Wall -Wextra -Wpedantic)
endif()
