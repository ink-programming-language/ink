file(GLOB INK_CORE_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/*_test.cpp")
file(GLOB INK_TOKENIZER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/*_test.cpp")
file(GLOB INK_PARSER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/*_test.cpp")
list(SORT INK_CORE_TEST_SOURCES)
list(SORT INK_TOKENIZER_TEST_SOURCES)
list(SORT INK_PARSER_TEST_SOURCES)

# Preserve the tokenizer tests' C++17 UTF-8 literal behavior while linking every test into one executable.
add_library(ink_tokenizer_test_objects OBJECT ${INK_TOKENIZER_TEST_SOURCES})
target_compile_features(ink_tokenizer_test_objects PRIVATE cxx_std_17)
target_compile_definitions(ink_tokenizer_test_objects PRIVATE INK_LEXICAL_RULES_PATH="${PROJECT_SOURCE_DIR}/docs/Ink-Lexical-Rules.md")
target_link_libraries(ink_tokenizer_test_objects PRIVATE ink::core ink::tokenizer GTest::gtest)
ink_disable_exceptions(ink_tokenizer_test_objects)

add_executable(ink_tests ${INK_CORE_TEST_SOURCES} ${INK_PARSER_TEST_SOURCES})
add_dependencies(ink_tests ink_tokenize ink_parse)
target_compile_features(ink_tests PRIVATE cxx_std_20)
target_link_libraries(ink_tests PRIVATE ink_tokenizer_test_objects ink::parser ink::llvm LLVMBitstreamReader GTest::gtest_main)
ink_disable_exceptions(ink_tests)

if(MSVC)
  target_compile_options(ink_tokenizer_test_objects PRIVATE /utf-8)
  target_compile_options(ink_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
# Keep grammar corpus case IDs stable instead of including pointer-valued parameter dumps in CTest names.
gtest_discover_tests(ink_tests DISCOVERY_TIMEOUT 60 NO_PRETTY_VALUES PROPERTIES TIMEOUT 30)
add_test(NAME ParserProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/parser-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/parser/process_test.cmake")
add_test(NAME ParserAdversarialProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/parser-adversarial-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/parser/adversarial_process_test.cmake")
set_tests_properties(ParserAdversarialProcessTest PROPERTIES TIMEOUT 120 LABELS "parser;adversarial")
add_test(NAME TokenizerProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_TOKENIZER=$<TARGET_FILE:ink_tokenize>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/tokenizer-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/process_test.cmake")
