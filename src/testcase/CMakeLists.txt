file(GLOB_RECURSE INK_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc" "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")
list(SORT INK_TEST_SOURCES)

add_executable(ink_tests ${INK_TEST_SOURCES})
add_dependencies(ink_tests ink_tokenize ink_parse inkc)
target_compile_features(ink_tests PRIVATE cxx_std_17)
target_link_libraries(ink_tests PRIVATE ink::cli ink::core ink::tokenizer ink::parser ink::ir ink::execution GTest::gmock_main)

if(MSVC)
  target_compile_options(ink_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
gtest_discover_tests(ink_tests DISCOVERY_TIMEOUT 60)
add_test(NAME InkTokenizerProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_TOKENIZER=$<TARGET_FILE:ink_tokenize>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/cli-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/cli/tokenizer_process_test.cmake")
add_test(NAME InkParserProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/cli-parser-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/cli/parser_process_test.cmake")
add_test(NAME InkCompilerProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/cli-compiler-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/cli/inkc_process_test.cmake")
