add_library(ink_cli STATIC application.cpp io.cpp)
add_library(ink::cli ALIAS ink_cli)

target_compile_features(ink_cli PUBLIC cxx_std_17)
target_include_directories(ink_cli PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_cli PRIVATE spdlog::spdlog)
ink_disable_exceptions(ink_cli)

if(WIN32)
  target_link_libraries(ink_cli PRIVATE shell32)
endif()

if(MSVC)
  target_compile_options(ink_cli PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()
