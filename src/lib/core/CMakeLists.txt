add_library(ink_core STATIC diagnostic.cpp)
add_library(ink::core ALIAS ink_core)

target_compile_features(ink_core PUBLIC cxx_std_17)
target_include_directories(ink_core PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_core PRIVATE spdlog::spdlog)

if(MSVC)
  target_compile_options(ink_core PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
