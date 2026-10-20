include("${PROJECT_SOURCE_DIR}/cmake/CheckFrontendDependencies.cmake")
ink_check_frontend_target_dependencies()
add_test(NAME FrontendDependencyBoundaryTest COMMAND "${CMAKE_COMMAND}" "-DINK_SOURCE_DIR=${PROJECT_SOURCE_DIR}" -P "${PROJECT_SOURCE_DIR}/cmake/CheckFrontendDependencies.cmake")

file(GLOB_RECURSE INK_CORE_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/*_test.cpp")
file(GLOB_RECURSE INK_IR_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ir/*_test.cpp")
file(GLOB_RECURSE INK_TOKENIZER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/*_test.cpp")
file(GLOB_RECURSE INK_PARSER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/*_test.cpp")
list(SORT INK_CORE_TEST_SOURCES)
list(SORT INK_IR_TEST_SOURCES)
list(SORT INK_TOKENIZER_TEST_SOURCES)
list(SORT INK_PARSER_TEST_SOURCES)

# Keep tokenizer-specific test definitions on their object target while linking every active test into one executable.
# UTF-8 test literals are converted explicitly from char8_t to the tokenizer's byte strings.
add_library(ink_tokenizer_test_objects OBJECT ${INK_TOKENIZER_TEST_SOURCES})
target_compile_features(ink_tokenizer_test_objects PRIVATE cxx_std_23)
target_compile_definitions(ink_tokenizer_test_objects PRIVATE INK_LEXICAL_RULES_PATH="${PROJECT_SOURCE_DIR}/docs/Ink-Lexical-Rules.md")
target_link_libraries(ink_tokenizer_test_objects PRIVATE ink::core GTest::gtest)
ink_disable_exceptions(ink_tokenizer_test_objects)

# Historical semantic sources live under docs/reference/semantic-tests/ and are not compiled or counted as active tests.
# Keep the rebuilt entry, type, expression, address, function, variable, call and name-resolution test sources explicit.
add_executable(ink_tests ${INK_CORE_TEST_SOURCES} ${INK_IR_TEST_SOURCES} ${INK_PARSER_TEST_SOURCES} semantic/analyzer_entry_test.cpp semantic/analyzer_type_test.cpp semantic/analyzer_expr_test.cpp semantic/analyzer_address_test.cpp semantic/analyzer_function_decl_test.cpp semantic/analyzer_variable_test.cpp semantic/analyzer_call_test.cpp semantic/name_resolver_test.cpp semantic/scope_store_test.cpp)
add_dependencies(ink_tests ink_tokenize ink_parse inkc)
target_compile_features(ink_tests PRIVATE cxx_std_23)
target_link_libraries(ink_tests PRIVATE ink_tokenizer_test_objects ink::ir ink::frontend ink::semantic ink::llvm GTest::gtest_main GTest::gmock spdlog::spdlog)
ink_disable_exceptions(ink_tests)

if(MSVC)
  target_compile_options(ink_tokenizer_test_objects PRIVATE /utf-8)
  target_compile_options(ink_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
# Keep grammar corpus case IDs stable instead of including pointer-valued parameter dumps in CTest names.
gtest_discover_tests(ink_tests DISCOVERY_TIMEOUT 60 NO_PRETTY_VALUES PROPERTIES TIMEOUT 30)
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ice_test_properties.cmake")
add_test(NAME ParserProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/parser-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/parser/process_test.cmake")
add_test(NAME ParserAdversarialProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/parser-adversarial-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/parser/adversarial_process_test.cmake")
set_tests_properties(ParserAdversarialProcessTest PROPERTIES TIMEOUT 120 LABELS "parser;adversarial")
add_test(NAME TokenizerProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_TOKENIZER=$<TARGET_FILE:ink_tokenize>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/tokenizer-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/process_test.cmake")
add_test(NAME InkcProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/inkc-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/inkc/process_test.cmake")
