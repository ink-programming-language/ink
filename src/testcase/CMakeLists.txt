file(GLOB_RECURSE INK_CORE_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/*_test.cpp")
file(GLOB_RECURSE INK_TOKENIZER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/*_test.cpp")
file(GLOB_RECURSE INK_PARSER_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/*_test.cpp")
file(GLOB_RECURSE INK_IR_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ir/*_test.cpp")
file(GLOB_RECURSE INK_SEMANTIC_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/semantic/*_test.cpp")
file(GLOB_RECURSE INK_EXECUTION_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/execution/*_test.cpp")
file(GLOB_RECURSE INK_BACKEND_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/backend/*_test.cpp")
list(SORT INK_CORE_TEST_SOURCES)
list(SORT INK_TOKENIZER_TEST_SOURCES)
list(SORT INK_PARSER_TEST_SOURCES)
list(SORT INK_IR_TEST_SOURCES)
list(SORT INK_SEMANTIC_TEST_SOURCES)
list(SORT INK_EXECUTION_TEST_SOURCES)
list(SORT INK_BACKEND_TEST_SOURCES)

# Preserve the tokenizer tests' C++17 UTF-8 literal behavior while linking every test into one executable.
add_library(ink_tokenizer_test_objects OBJECT ${INK_TOKENIZER_TEST_SOURCES})
target_compile_features(ink_tokenizer_test_objects PRIVATE cxx_std_17)
target_compile_definitions(ink_tokenizer_test_objects PRIVATE INK_LEXICAL_RULES_PATH="${PROJECT_SOURCE_DIR}/docs/Ink-Lexical-Rules.md")
target_link_libraries(ink_tokenizer_test_objects PRIVATE ink::core ink::tokenizer GTest::gtest)
ink_disable_exceptions(ink_tokenizer_test_objects)

# Compile model tests with IR dependencies only, keeping semantic headers out of this boundary.
add_library(ink_ir_test_objects OBJECT ${INK_IR_TEST_SOURCES})
target_compile_features(ink_ir_test_objects PRIVATE cxx_std_20)
target_link_libraries(ink_ir_test_objects PRIVATE ink::ir ink::llvm GTest::gtest GTest::gmock)
ink_disable_exceptions(ink_ir_test_objects)

add_executable(ink_tests ${INK_CORE_TEST_SOURCES} ${INK_PARSER_TEST_SOURCES} ${INK_SEMANTIC_TEST_SOURCES} ${INK_EXECUTION_TEST_SOURCES} ${INK_BACKEND_TEST_SOURCES})
# Expose the C ABI fixtures to the same process symbol lookup used by compile-time external calls.
set_target_properties(ink_tests PROPERTIES ENABLE_EXPORTS ON)
add_dependencies(ink_tests ink_tokenize ink_parse inkc)
add_dependencies(ink_tests ink_aot_runtime)
target_compile_features(ink_tests PRIVATE cxx_std_20)
target_link_libraries(ink_tests PRIVATE ink_tokenizer_test_objects ink_ir_test_objects ink::parser ink::semantic ink::llvm_backend ink::llvm LLVMBitstreamReader GTest::gtest_main GTest::gmock spdlog::spdlog)
ink_disable_exceptions(ink_tests)

if(MSVC)
  target_compile_options(ink_tokenizer_test_objects PRIVATE /utf-8)
  target_compile_options(ink_tests PRIVATE /utf-8)
  target_compile_options(ink_ir_test_objects PRIVATE /utf-8)
endif()

include(GoogleTest)
# Keep grammar corpus case IDs stable instead of including pointer-valued parameter dumps in CTest names.
gtest_discover_tests(ink_tests DISCOVERY_TIMEOUT 60 NO_PRETTY_VALUES PROPERTIES TIMEOUT 30)
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ice_test_properties.cmake")
add_test(NAME ParserProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/parser-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/parser/process_test.cmake")
add_test(NAME ParserAdversarialProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_PARSER=$<TARGET_FILE:ink_parse>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/parser-adversarial-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/parser/adversarial_process_test.cmake")
set_tests_properties(ParserAdversarialProcessTest PROPERTIES TIMEOUT 120 LABELS "parser;adversarial")
add_test(NAME TokenizerProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_TOKENIZER=$<TARGET_FILE:ink_tokenize>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/tokenizer-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer/process_test.cmake")

add_test(NAME InkcProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/inkc-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/inkc_process_test.cmake")
set_tests_properties(InkcProcessTest PROPERTIES TIMEOUT 60 LABELS "execution;cli")

add_test(NAME BytecodeProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/bytecode-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/bytecode_process_test.cmake")
set_tests_properties(BytecodeProcessTest PROPERTIES TIMEOUT 120 LABELS "execution;bytecode;cli")

add_test(NAME ArrayReadProcessTest COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_POINTER_BYTES=${CMAKE_SIZEOF_VOID_P}" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/array-read-process" -P "${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/array_read_process_test.cmake")
set_tests_properties(ArrayReadProcessTest PROPERTIES TIMEOUT 120 LABELS "execution;arrays;ffi;bytecode;cli")

include("${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/source_program_tests.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/execution/class_program_tests.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/multifile_program_tests.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/backend/tests.cmake")
