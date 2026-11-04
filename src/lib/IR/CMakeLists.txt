add_library(ink_ir STATIC type/type_kind.cpp type/type_pool.cpp constant/constant_pool.cpp)
add_library(ink::ir ALIAS ink_ir)

target_compile_features(ink_ir PUBLIC cxx_std_23)
target_include_directories(ink_ir PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_ir PUBLIC ink::core)
ink_disable_exceptions(ink_ir)

if(MSVC)
  target_compile_options(ink_ir PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()
