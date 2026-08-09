add_library(ink_sema STATIC semantic_model.cpp declaration_collector.cpp signature_resolver.cpp name_resolver.cpp type_checker.cpp control_flow_checker.cpp verifier.cpp analyzer.cpp)
add_library(ink::sema ALIAS ink_sema)

target_compile_features(ink_sema PUBLIC cxx_std_17)
target_include_directories(ink_sema PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_sema PUBLIC ink::core ink::ast ink::type)

if(MSVC)
  target_compile_options(ink_sema PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_sema PRIVATE -Wall -Wextra -Wpedantic)
endif()
