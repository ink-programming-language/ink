file(GLOB_RECURSE INK_ABI_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_abi STATIC ${INK_ABI_SOURCES})
add_library(ink::abi ALIAS ink_abi)

target_compile_features(ink_abi PUBLIC cxx_std_20)
target_include_directories(ink_abi PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_abi PRIVATE utf8proc::utf8proc)
ink_disable_exceptions(ink_abi)

if(MSVC)
  target_compile_options(ink_abi PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_abi PRIVATE -Wall -Wextra -Wpedantic)
endif()
