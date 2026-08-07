cmake_minimum_required(VERSION 3.20)

project(ink LANGUAGES CXX)

include(CTest)

add_subdirectory(src/lib)
add_subdirectory(src/tools)

if(BUILD_TESTING)
  set(INSTALL_GTEST OFF CACHE BOOL "Disable GoogleTest installation" FORCE)
  set(BUILD_GMOCK ON CACHE BOOL "Build GoogleMock with GoogleTest" FORCE)
  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "Use the shared MSVC runtime" FORCE)
  endif()

  set(INK_GOOGLETEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/googletest")
  if(NOT EXISTS "${INK_GOOGLETEST_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "GoogleTest is not initialized. Run: git submodule update --init src/third_party/googletest")
  endif()

  add_subdirectory("${INK_GOOGLETEST_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/third_party/googletest" EXCLUDE_FROM_ALL)
  add_subdirectory(src/testcase)
  add_subdirectory(testcast)
  add_custom_target(run_all_tests COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}" --build-config "$<CONFIG>" --output-on-failure DEPENDS ink_tests ink_tokenizer_tests COMMENT "Running all Ink tests" USES_TERMINAL VERBATIM)
endif()
