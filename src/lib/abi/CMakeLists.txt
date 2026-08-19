add_library(ink_abi STATIC codec.cpp demangle.cpp linkage_identity.cpp mangle.cpp name_mangling.cpp)
add_library(ink::abi ALIAS ink_abi)

target_compile_features(ink_abi PUBLIC cxx_std_17)
target_include_directories(ink_abi PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_abi PRIVATE utf8proc::utf8proc)
ink_disable_exceptions(ink_abi)

if(MSVC)
  target_compile_options(ink_abi PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_abi PRIVATE -Wall -Wextra -Wpedantic)
endif()
