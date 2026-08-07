file(GLOB_RECURSE INK_TOKENIZER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/*_test.cc" "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/*_test.cpp")
list(SORT INK_TOKENIZER_TEST_SOURCES)

add_executable(ink_tokenizer_tests ${INK_TOKENIZER_TEST_SOURCES})
target_compile_features(ink_tokenizer_tests PRIVATE cxx_std_17)
target_link_libraries(ink_tokenizer_tests PRIVATE ink::tokenizer GTest::gmock_main)

if(MSVC)
  target_compile_options(ink_tokenizer_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
gtest_discover_tests(ink_tokenizer_tests DISCOVERY_TIMEOUT 60)
