add_library(ink_execution STATIC execution_engine.cpp native_symbol_registry.cpp runtime.cpp)
add_library(ink::execution ALIAS ink_execution)

target_compile_features(ink_execution PUBLIC cxx_std_17)
target_include_directories(ink_execution PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_execution PUBLIC ink::core ink::ir PRIVATE ink::libffi ${CMAKE_DL_LIBS})

if(MSVC)
  target_compile_options(ink_execution PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_execution PRIVATE -Wall -Wextra -Wpedantic)
endif()
