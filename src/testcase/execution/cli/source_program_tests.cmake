# Each source runs its default main with exit-code/stdout assertions and an optional error diagnostic.
# STDOUT_HEX additionally checks raw bytes; LABELS supplements the common execution/source labels.
function(add_source_program_test Name Source ExpectedStatus ExpectedOutput ExpectedError)
  cmake_parse_arguments(PARSE_ARGV 5 Test "" "STDOUT_HEX" "LABELS")
  if(Test_UNPARSED_ARGUMENTS OR Test_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "Invalid source program test options for '${Name}'")
  endif()
  set(SourcePath "${CMAKE_CURRENT_SOURCE_DIR}/execution/${Source}")
  if(ExpectedStatus STREQUAL "0")
    file(READ "${SourcePath}" SourceText)
    if(ExpectedOutput STREQUAL "" OR NOT SourceText MATCHES "func[ \t\r\n]+main[ \t\r\n]*\\(")
      message(FATAL_ERROR "Successful source program '${Source}' must define main and assert nonempty stdout")
    endif()
  endif()
  add_test(NAME "ExecutionSourceTest.${Name}" COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_SOURCE=${SourcePath}" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/execution-programs" "-DINK_EXPECTED_STATUS=${ExpectedStatus}" "-DINK_EXPECTED_STDOUT=${ExpectedOutput}" "-DINK_EXPECTED_STDOUT_HEX=${Test_STDOUT_HEX}" "-DINK_EXPECTED_ERROR=${ExpectedError}" -P "${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/source_program_test.cmake")
  set_tests_properties("ExecutionSourceTest.${Name}" PROPERTIES TIMEOUT 30 LABELS "execution;source;${Test_LABELS}")
endfunction()

# Platform-specific ABI and exact newline bytes are fixture expectations, not separate runners.
if(WIN32)
  add_source_program_test(HelloWorld programs/hello_world.windows.ink 0 "hello, world|" "" STDOUT_HEX "68656c6c6f2c20776f726c640d0a")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(HelloWorld programs/hello_world.linux.ink 0 "hello, world|" "" STDOUT_HEX "68656c6c6f2c20776f726c640a")
endif()

# Six executable examples retain all 56 result checks, printing the case name, any operand effects, and PASS.
# Their puts calls use the host C runtime, whose execution adapter supports Windows and Linux.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(Logical.Main.TruthTables programs/logical_truth_tables.ink 0 "Not.false|PASS|Not.true|PASS|And.false.false|PASS|And.false.true|PASS|And.true.false|PASS|And.true.true|PASS|Or.false.false|PASS|Or.false.true|PASS|Or.true.false|PASS|Or.true.true|PASS|EqualBool.false.false|PASS|EqualBool.false.true|PASS|EqualBool.true.false|PASS|EqualBool.true.true|PASS|NotEqualBool.false.false|PASS|NotEqualBool.false.true|PASS|NotEqualBool.true.false|PASS|NotEqualBool.true.true|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Comparisons programs/logical_comparisons.ink 0 "Compare.equal|PASS|Compare.not_equal|PASS|Compare.less|PASS|Compare.less_equal|PASS|Compare.greater|PASS|Compare.greater_equal|PASS|Compare.signed_i8|PASS|Compare.unsigned_u8|PASS|Compare.signed_i128|PASS|Compare.unsigned_u128|PASS|Compare.literal_comparisons|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Contexts programs/logical_contexts.ink 0 "Context.precedence|PASS|Context.parentheses|PASS|Context.repeated_not|PASS|Context.bool_comparison_precedence|PASS|Context.local_assignment|PASS|Context.nested_condition|PASS|Context.call_arguments|PASS|Context.nested_merge|PASS|Context.inferred_local|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.ShortCircuit programs/logical_short_circuit.ink 0 "ShortCircuit.skip_and|PASS|ShortCircuit.skip_or|PASS|ShortCircuit.nested_skip|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Comptime programs/logical_comptime.ink 0 "Comptime.logical_true|PASS|Comptime.logical_false|PASS|Comptime.comparisons|PASS|Comptime.ordinary_call|PASS|Comptime.skip_call|PASS|Comptime.direct_expression|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Effects programs/logical_effects.ink 0 "Effects.skip_and|left|PASS|Effects.skip_or|left|PASS|Effects.reach_and|left|right|PASS|Effects.reach_or|left|right|PASS|Effects.nested|left|right|tail|PASS|Effects.reuse_result|left|right|PASS|Effects.compare_order|first|second|PASS|Effects.not_once|left|PASS|Effects.precedence_skip|left|PASS|" "" LABELS logical)
endif()

# Deliberately invalid input belongs to CLI fixtures, separate from runnable example programs.
add_source_program_test(Logical.Invalid.NotInteger cli/inputs/logical_invalid_not.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.AndLeftInteger cli/inputs/logical_invalid_and_left.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.SkippedAndRightInteger cli/inputs/logical_invalid_and_right.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.OrLeftInteger cli/inputs/logical_invalid_or_left.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.SkippedOrRightInteger cli/inputs/logical_invalid_or_right.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.CompareWidths cli/inputs/logical_invalid_compare_widths.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.CompareSignedness cli/inputs/logical_invalid_compare_signedness.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.CompareBoolOrder cli/inputs/logical_invalid_compare_bool.ink 1 "" "INK-S0004" LABELS logical)

# Reaching a selected missing function must still fail in each short-circuit shape.
add_source_program_test(Logical.ShortCircuit.reach_and cli/inputs/logical_reach_and.ink 1 "" "INK-E0013" LABELS logical)
add_source_program_test(Logical.ShortCircuit.reach_or cli/inputs/logical_reach_or.ink 1 "" "INK-E0013" LABELS logical)
add_source_program_test(Logical.ShortCircuit.nested_reach cli/inputs/logical_nested_reach.ink 1 "" "INK-E0013" LABELS logical)
