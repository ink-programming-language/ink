file(GLOB_RECURSE INK_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc" "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")
list(SORT INK_TEST_SOURCES)

add_executable(ink_tests ${INK_TEST_SOURCES})
target_compile_features(ink_tests PRIVATE cxx_std_17)
target_link_libraries(ink_tests PRIVATE ink::core ink::tokenizer GTest::gmock_main)

if(MSVC)
  target_compile_options(ink_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
gtest_discover_tests(ink_tests DISCOVERY_TIMEOUT 60)
