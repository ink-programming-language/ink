add_library(ink_frontend STATIC compilation_session.cpp compiler.cpp ir_generator.cpp)
add_library(ink::frontend ALIAS ink_frontend)

target_compile_features(ink_frontend PUBLIC cxx_std_17)
target_include_directories(ink_frontend PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_frontend PUBLIC ink::core ink::tokenizer ink::parser ink::ast ink::type ink::target ink::sema ink::ir)

if(MSVC)
  target_compile_options(ink_frontend PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_frontend PRIVATE -Wall -Wextra -Wpedantic)
endif()
