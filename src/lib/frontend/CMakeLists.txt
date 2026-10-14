file(GLOB_RECURSE INK_FRONTEND_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(ink_frontend STATIC ${INK_FRONTEND_SOURCES})
add_library(ink::frontend ALIAS ink_frontend)

target_compile_features(ink_frontend PUBLIC cxx_std_23)
target_include_directories(ink_frontend PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>")
target_link_libraries(ink_frontend PUBLIC ink::core PRIVATE ink::llvm utf8proc::utf8proc)
ink_disable_exceptions(ink_frontend)

if(MSVC)
  target_compile_options(ink_frontend PRIVATE /utf-8 /W4 /permissive-)
else()
  target_compile_options(ink_frontend PRIVATE -Wall -Wextra -Wpedantic)
endif()
