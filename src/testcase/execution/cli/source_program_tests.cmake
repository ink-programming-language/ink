# Each source runs its default main with exit-code/stdout assertions and an optional error diagnostic.
# STDIN_FILE supplies deterministic input; BYTECODE repeats checks after emitting, linking and reloading the program.
# STDOUT_HEX checks raw bytes; BYTECODE_ERROR overrides saved-image error text; LABELS supplements common labels.
function(add_source_program_test Name Source ExpectedStatus ExpectedOutput ExpectedError)
  cmake_parse_arguments(PARSE_ARGV 5 Test "BYTECODE" "STDOUT_HEX;STDIN_FILE;BYTECODE_ERROR" "LABELS")
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
  set(InputPath "")
  if(Test_STDIN_FILE)
    set(InputPath "${CMAKE_CURRENT_SOURCE_DIR}/execution/${Test_STDIN_FILE}")
    if(NOT EXISTS "${InputPath}")
      message(FATAL_ERROR "Missing standard input fixture '${Test_STDIN_FILE}'")
    endif()
  endif()
  add_test(NAME "ExecutionSourceTest.${Name}" COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_SOURCE=${SourcePath}" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/execution-programs" "-DINK_EXPECTED_STATUS=${ExpectedStatus}" "-DINK_EXPECTED_STDOUT=${ExpectedOutput}" "-DINK_EXPECTED_STDOUT_HEX=${Test_STDOUT_HEX}" "-DINK_EXPECTED_ERROR=${ExpectedError}" "-DINK_STDIN_FILE=${InputPath}" "-DINK_TEST_BYTECODE=${Test_BYTECODE}" "-DINK_BYTECODE_ERROR=${Test_BYTECODE_ERROR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/source_program_test.cmake")
  if(Test_BYTECODE)
    set_tests_properties("ExecutionSourceTest.${Name}" PROPERTIES TIMEOUT 90 LABELS "execution;source;bytecode;${Test_LABELS}")
  else()
    set_tests_properties("ExecutionSourceTest.${Name}" PROPERTIES TIMEOUT 30 LABELS "execution;source;${Test_LABELS}")
  endif()
endfunction()

# Platform-specific ABI and exact newline bytes are fixture expectations, not separate runners.
if(WIN32)
  add_source_program_test(HelloWorld programs/hello_world.windows.ink 0 "hello, world|" "" STDOUT_HEX "68656c6c6f2c20776f726c640d0a")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(HelloWorld programs/hello_world.linux.ink 0 "hello, world|" "" STDOUT_HEX "68656c6c6f2c20776f726c640a")
endif()

# Successful programs print each case name, any operand effects, and PASS after checking the result.
# Their puts calls use the host C runtime, whose execution adapter supports Windows and Linux.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(Logical.Main.TruthTables programs/logical_truth_tables.ink 0 "Not.false|PASS|Not.true|PASS|And.false.false|PASS|And.false.true|PASS|And.true.false|PASS|And.true.true|PASS|Or.false.false|PASS|Or.false.true|PASS|Or.true.false|PASS|Or.true.true|PASS|EqualBool.false.false|PASS|EqualBool.false.true|PASS|EqualBool.true.false|PASS|EqualBool.true.true|PASS|NotEqualBool.false.false|PASS|NotEqualBool.false.true|PASS|NotEqualBool.true.false|PASS|NotEqualBool.true.true|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Comparisons programs/logical_comparisons.ink 0 "Compare.equal|PASS|Compare.not_equal|PASS|Compare.less|PASS|Compare.less_equal|PASS|Compare.greater|PASS|Compare.greater_equal|PASS|Compare.signed_i8|PASS|Compare.unsigned_u8|PASS|Compare.signed_i128|PASS|Compare.unsigned_u128|PASS|Compare.literal_comparisons|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Contexts programs/logical_contexts.ink 0 "Context.precedence|PASS|Context.parentheses|PASS|Context.repeated_not|PASS|Context.bool_comparison_precedence|PASS|Context.local_assignment|PASS|Context.nested_condition|PASS|Context.call_arguments|PASS|Context.nested_merge|PASS|Context.inferred_local|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.ShortCircuit programs/logical_short_circuit.ink 0 "ShortCircuit.skip_and|PASS|ShortCircuit.skip_or|PASS|ShortCircuit.nested_skip|PASS|" "" LABELS logical)
  add_source_program_test(Logical.Main.Comptime programs/logical_comptime.ink 0 "Comptime.logical_true|PASS|Comptime.logical_false|PASS|Comptime.comparisons|PASS|Comptime.ordinary_call|PASS|Comptime.skip_call|PASS|Comptime.direct_expression|PASS|" "" LABELS logical comptime)
  add_source_program_test(Logical.Main.Effects programs/logical_effects.ink 0 "Effects.skip_and|left|PASS|Effects.skip_or|left|PASS|Effects.reach_and|left|right|PASS|Effects.reach_or|left|right|PASS|Effects.nested|left|right|tail|PASS|Effects.reuse_result|left|right|PASS|Effects.compare_order|first|second|PASS|Effects.not_once|left|PASS|Effects.precedence_skip|left|PASS|" "" LABELS logical)

  # Three-input logic checks every input combination against three independent truth-table columns.
  add_source_program_test(Logical.Main.NestedTruthTables programs/logical_nested_truth_tables.ink 0 "Nested.false.false.false|PASS|PASS|PASS|Nested.false.false.true|PASS|PASS|PASS|Nested.false.true.false|PASS|PASS|PASS|Nested.false.true.true|PASS|PASS|PASS|Nested.true.false.false|PASS|PASS|PASS|Nested.true.false.true|PASS|PASS|PASS|Nested.true.true.false|PASS|PASS|PASS|Nested.true.true.true|PASS|PASS|PASS|" "" LABELS logical)

  # Both control-flow arms and their shared continuation must preserve initialization, scope and side-effect order.
  add_source_program_test(ControlFlow.Main.Branches programs/control_flow.ink 0 "Control.select_true|PASS|Control.select_false|PASS|Control.negative|PASS|Control.zero|PASS|Control.positive|PASS|Control.nested_true_true|PASS|Control.nested_true_false|PASS|Control.nested_false_true|PASS|Control.nested_false_false|PASS|Control.returning_arm|PASS|Control.surviving_arm|PASS|Control.shadow_true|PASS|Control.shadow_false|PASS|Control.empty_true|PASS|Control.empty_false|PASS|Control.effects_true|condition|then|merge|PASS|Control.effects_false|condition|else|merge|PASS|Control.skip_missing_true|PASS|Control.skip_missing_false|PASS|" "" LABELS control_flow)
  add_source_program_test(Comptime.Main.ControlFlow programs/comptime_control_flow.ink 0 "Static.before_write|PASS|Static.after_write|PASS|Static.for_continue_break|PASS|Static.while_continue_break|PASS|Static.nested_loops|PASS|Static.zero_iterations|PASS|Static.unrolled_true|PASS|Static.unrolled_false|PASS|Static.unrolled_while|PASS|Static.selected_branch|PASS|" "" LABELS comptime control_flow)

  # Runtime addition and compile-time arithmetic cover integer widths, boundaries and evaluation order.
  add_source_program_test(Arithmetic.Main.Widths programs/arithmetic_widths.ink 0 "Arithmetic.signed_i8|PASS|Arithmetic.unsigned_u8|PASS|Arithmetic.signed_i16|PASS|Arithmetic.unsigned_u16|PASS|Arithmetic.signed_i32|PASS|Arithmetic.unsigned_u32|PASS|Arithmetic.signed_i64|PASS|Arithmetic.unsigned_u64|PASS|Arithmetic.signed_i128|PASS|Arithmetic.unsigned_u128|PASS|" "" LABELS arithmetic)
  add_source_program_test(Arithmetic.Main.Contexts programs/arithmetic_contexts.ink 0 "Arithmetic.grouping|PASS|Arithmetic.signed_literals|PASS|Arithmetic.inferred_locals|PASS|Arithmetic.repeated_assignment|PASS|Arithmetic.shadowed_locals|PASS|Arithmetic.branch_positive|PASS|Arithmetic.branch_negative|PASS|Arithmetic.operand_order|first|second|third|PASS|" "" LABELS arithmetic)
  add_source_program_test(Arithmetic.Main.Comptime programs/comptime_arithmetic.ink 0 "ComptimeArithmetic.precedence|PASS|ComptimeArithmetic.parentheses|PASS|ComptimeArithmetic.subtraction_association|PASS|ComptimeArithmetic.division_association|PASS|ComptimeArithmetic.signed_division|PASS|ComptimeArithmetic.signed_remainder|PASS|ComptimeArithmetic.multiply|PASS|ComptimeArithmetic.unary|PASS|ComptimeArithmetic.bitwise_and|PASS|ComptimeArithmetic.bitwise_or|PASS|ComptimeArithmetic.bitwise_xor|PASS|ComptimeArithmetic.bitwise_precedence|PASS|ComptimeArithmetic.shift_boundaries|PASS|ComptimeArithmetic.compound_arithmetic|PASS|ComptimeArithmetic.compound_bitwise|PASS|ComptimeArithmetic.update_values|PASS|ComptimeArithmetic.signed_byte|PASS|ComptimeArithmetic.unsigned_byte|PASS|ComptimeArithmetic.signed_shift|PASS|ComptimeArithmetic.wide_arithmetic|PASS|ComptimeArithmetic.runtime_initializer|PASS|" "" LABELS arithmetic comptime)

  # Ordinary calls preserve typed arguments, independent recursive frames and observable call ordering.
  add_source_program_test(Function.Main.Calls programs/function_calls.ink 0 "Function.zero_arguments|PASS|Function.multiple_arguments|PASS|Function.nested_calls|PASS|Function.parenthesized_callee|PASS|Function.overload_i32|PASS|Function.overload_literal|PASS|Function.overload_u8|PASS|Function.overload_bool|PASS|Function.overload_arity|PASS|Function.boolean_payload|PASS|Function.nested_declaration|PASS|Function.wide_payload|PASS|Function.recursive_base|PASS|Function.recursive_frames|PASS|" "" LABELS function)
  add_source_program_test(Function.Main.Storage programs/function_storage.ink 0 "Storage.by_value_argument|PASS|Storage.repeated_calls|PASS|Storage.nested_frames|PASS|Storage.fresh_outer_frame|PASS|Storage.return_value_copy|PASS|Storage.early_negative_return|PASS|Storage.early_zero_return|PASS|Storage.final_return|PASS|Storage.both_branches_return|PASS|Storage.recursive_locals|PASS|" "" LABELS function)
  add_source_program_test(Function.Main.Effects programs/function_effects.ink 0 "FunctionEffects.argument_order|first|second|third|combine|PASS|FunctionEffects.nested_argument_order|second|first|third|second|first|third|combine|PASS|FunctionEffects.expression_order|first|second|PASS|FunctionEffects.reuse_result|first|PASS|FunctionEffects.discard_result|first|PASS|FunctionEffects.void_returns|explicit|implicit|PASS|FunctionEffects.conditional_void|early|fallthrough|PASS|" "" LABELS function)
endif()

# C-runtime lookup is supported on Windows and Linux. The five programs contain 40 PASS checks.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(Memory.Main.AddressOf programs/address_of.ink 0 "AddressOf.PASS|" "" LABELS memory external)
  add_source_program_test(External.Main.Scalars programs/external_scalars.ink 0 "External.abs_negative|PASS|External.abs_zero|PASS|External.abs_positive|PASS|External.abs_nested|PASS|External.abs_boundary|PASS|External.llabs_wide|PASS|External.atoi_signed|PASS|External.atoi_nested|PASS|External.atoi_empty|PASS|" "" LABELS external)
  add_source_program_test(External.Main.Comptime programs/external_comptime.ink 0 "External.comptime_initializer|External.comptime_stored|PASS|External.comptime_effect_once|PASS|External.comptime_function|PASS|External.comptime_ordinary|PASS|External.comptime_nested|PASS|External.comptime_strings|PASS|External.comptime_skipped_symbol|PASS|" "" LABELS external comptime)

  # These three sources declare native size_t as u64 and therefore require a 64-bit host.
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_source_program_test(External.Main.Strings programs/external_strings.ink 0 "External.string_empty|PASS|External.string_ascii|PASS|External.string_utf8|PASS|External.string_embedded_nul|PASS|External.string_equal|PASS|External.string_less|PASS|External.string_greater|PASS|External.bytes_after_nul|PASS|External.bytes_prefix|PASS|External.bytes_terminator|PASS|" "" LABELS external)
    add_source_program_test(External.Main.Pointers programs/external_pointers.ink 0 "External.pointer_local|PASS|External.pointer_nested|PASS|External.pointer_terminator|PASS|External.pointer_last_match|PASS|External.pointer_substring|PASS|External.pointer_mutation|PASS|External.pointer_independent_copies|PASS|External.pointer_alias|PASS|External.pointer_counted_search|PASS|" "" LABELS external)
    add_source_program_test(External.Main.Effects programs/external_effects.ink 0 "External.effects_repeated|native|native|PASS|External.effects_reuse|native|PASS|External.effects_arguments|buffer|byte|count|PASS|External.effects_void_and_zero_args|PASS|External.effects_nested_calls|outer|native|PASS|" "" LABELS external)
  endif()

  # These failures enter the native execution adapter and require a supported host.
  add_source_program_test(External.Invalid.MissingSymbol cli/inputs/external_missing_symbol.ink 1 "" "INK-E0014" LABELS external)
  add_source_program_test(External.Invalid.ComptimeMissingSymbol cli/inputs/external_comptime_missing_symbol.ink 1 "" "INK-E0014" LABELS external comptime)
  add_source_program_test(External.Invalid.UnsupportedInteger cli/inputs/external_unsupported_integer.ink 1 "" "INK-E0015" LABELS external)
  add_source_program_test(External.Invalid.UnsupportedFloat cli/inputs/external_unsupported_float.ink 1 "" "INK-E0015" LABELS external)
  add_source_program_test(External.Raw.EscapedPointer cli/inputs/external_expired_pointer.ink 0 "PASS|" "" LABELS external)
endif()

# Array programs exercise the same named checks in source and saved-bytecode execution.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(Array.Main.Basics programs/arrays_basics.ink 0 "ArraysBasics.inferred_literals|PASS|ArraysBasics.contextual_elements|PASS|ArraysBasics.typed_value_inference|PASS|ArraysBasics.boolean_elements|PASS|ArraysBasics.repeated_values|PASS|ArraysBasics.compiletime_length|PASS|ArraysBasics.nested_elements|PASS|ArraysBasics.nested_repetition|PASS|ArraysBasics.empty_arrays|PASS|ArraysBasics.zero_length_inner_arrays|PASS|" "" BYTECODE LABELS arrays)
  add_source_program_test(Array.Main.Widths programs/arrays_widths.ink 0 "ArraysWidths.i8|PASS|ArraysWidths.u8|PASS|ArraysWidths.i16|PASS|ArraysWidths.u16|PASS|ArraysWidths.i32|PASS|ArraysWidths.u32|PASS|ArraysWidths.i64|PASS|ArraysWidths.u64|PASS|ArraysWidths.i128|PASS|ArraysWidths.u128|PASS|" "" BYTECODE LABELS arrays)
  add_source_program_test(Array.Main.Values programs/arrays_values.ink 0 "ArraysValues.independent_copies|PASS|ArraysValues.nested_copies|PASS|ArraysValues.whole_array_assignment|PASS|ArraysValues.stable_element_alias|PASS|ArraysValues.stable_nested_alias|PASS|ArraysValues.parameters_and_returns|PASS|ArraysValues.temporary_array_reads|PASS|ArraysValues.constant_array_reads|PASS|ArraysValues.pointer_element_copies|PASS|ArraysValues.pointer_to_array|PASS|ArraysValues.parameter_snapshot_during_mutation|PASS|ArraysValues.nested_parameters_and_returns|PASS|ArraysValues.repeated_calls_fresh_arrays|PASS|" "" BYTECODE LABELS arrays)
  add_source_program_test(Array.Main.Effects programs/arrays_effects.ink 0 "ArraysEffects.literal_order|next|next|next|PASS|ArraysEffects.nested_literal_order|next|next|next|next|PASS|ArraysEffects.repeat_once|next|PASS|ArraysEffects.zero_repeat_once|next|PASS|ArraysEffects.assignment_index_before_rhs|index|rhs|PASS|ArraysEffects.load_after_index|index_write|PASS|ArraysEffects.argument_capture|mutate|capture|PASS|ArraysEffects.runtime_array_return|make|PASS|ArraysEffects.temporary_array_return|make|PASS|" "" BYTECODE LABELS arrays)
  add_source_program_test(Array.Main.Comptime programs/arrays_comptime.ink 0 "ArraysComptime.nested_writes|PASS|ArraysComptime.independent_copies|PASS|ArraysComptime.compound_arithmetic|PASS|ArraysComptime.compound_bitwise|PASS|ArraysComptime.local_const_length|PASS|ArraysComptime.frozen_function_return|PASS|ArraysComptime.repeated_side_effect|PASS|ArraysComptime.zero_repeat_side_effect|PASS|ArraysComptime.sibling_assignment|PASS|" "" BYTECODE LABELS arrays comptime)
endif()

# Reopen the six-byte input fixture for each execution to independently verify read, short reads and EOF.
set(ArrayReadOutput "ArrayRead.initialized_repeat|PASS|ArrayRead.interior_offset|PASS|ArrayRead.zero_count|PASS|ArrayRead.short_read|PASS|ArrayRead.value_copy|PASS|ArrayRead.element_write|PASS|ArrayRead.copy_independence|PASS|ArrayRead.eof|PASS|")
if(WIN32)
  add_source_program_test(Array.Main.Read programs/arrays_read.windows.ink 0 "${ArrayReadOutput}" "" BYTECODE STDIN_FILE programs/arrays_read.input LABELS arrays external)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_source_program_test(Array.Main.Read programs/arrays_read.linux.ink 0 "${ArrayReadOutput}" "" BYTECODE STDIN_FILE programs/arrays_read.input LABELS arrays external)
endif()

# Reject invalid array types, constructors, indices and mutation before executing main.
add_source_program_test(Array.Invalid.EmptyInference cli/inputs/array_empty_inference.ink 1 "" "INK-S0052" LABELS arrays)
add_source_program_test(Array.Invalid.NegativeLength cli/inputs/array_length_negative.ink 1 "" "INK-S0051" LABELS arrays)
add_source_program_test(Array.Invalid.BoolLength cli/inputs/array_length_bool.ink 1 "" "INK-S0051" LABELS arrays)
add_source_program_test(Array.Invalid.FloatLength cli/inputs/array_length_float.ink 1 "" "INK-S0051" LABELS arrays)
add_source_program_test(Array.Invalid.UnrepresentableLength cli/inputs/array_length_unrepresentable.ink 1 "" "INK-S0051" LABELS arrays)
add_source_program_test(Array.Invalid.ExcessiveLength cli/inputs/array_length_too_large.ink 1 "" "INK-S0055" LABELS arrays)
add_source_program_test(Array.Invalid.RuntimeLength cli/inputs/array_length_runtime.ink 1 "" "INK-E0008" LABELS arrays)
add_source_program_test(Array.Invalid.StorageOverflow cli/inputs/array_storage_overflow.ink 1 "" "INK-S0055" LABELS arrays)
add_source_program_test(Array.Invalid.ElementType cli/inputs/array_element_type.ink 1 "" "INK-S0004" LABELS arrays)
add_source_program_test(Array.Invalid.LengthMismatch cli/inputs/array_length_mismatch.ink 1 "" "INK-S0004" LABELS arrays)
add_source_program_test(Array.Invalid.NestedTypeMismatch cli/inputs/array_nested_type_mismatch.ink 1 "" "INK-S0004" LABELS arrays)
add_source_program_test(Array.Invalid.ByteOverflow cli/inputs/array_u8_overflow.ink 1 "" "INK-S0005" LABELS arrays)
add_source_program_test(Array.Invalid.BoolIndex cli/inputs/array_index_bool.ink 1 "" "INK-S0053" LABELS arrays)
add_source_program_test(Array.Invalid.FloatIndex cli/inputs/array_index_float.ink 1 "" "INK-S0053" LABELS arrays)
add_source_program_test(Array.Invalid.NonarrayIndex cli/inputs/array_index_nonarray.ink 1 "" "INK-S0004" LABELS arrays)
add_source_program_test(Array.Invalid.ConstantNegativeIndex cli/inputs/array_index_negative.ink 1 "" "INK-S0054" LABELS arrays)
add_source_program_test(Array.Invalid.ConstantReadBounds cli/inputs/array_index_out_of_bounds.ink 1 "" "INK-S0054" LABELS arrays)
add_source_program_test(Array.Invalid.ConstantWriteBounds cli/inputs/array_index_write_out_of_bounds.ink 1 "" "INK-S0054" LABELS arrays)
add_source_program_test(Array.Invalid.ConstantAddressBounds cli/inputs/array_index_address_out_of_bounds.ink 1 "" "INK-S0054" LABELS arrays)
add_source_program_test(Array.Invalid.EmptyArrayIndex cli/inputs/array_index_empty.ink 1 "" "INK-S0054" LABELS arrays)
add_source_program_test(Array.Invalid.ConstWrite cli/inputs/array_const_write.ink 1 "" "INK-S0026" LABELS arrays)
add_source_program_test(Array.Invalid.ConstAddress cli/inputs/array_const_address.ink 1 "" "INK-S0026" LABELS arrays)
add_source_program_test(Array.Invalid.UninitializedRead cli/inputs/array_uninitialized_read.ink 1 "" "INK-S0006" LABELS arrays)
add_source_program_test(Array.Invalid.UninitializedWrite cli/inputs/array_uninitialized_write.ink 1 "" "INK-S0006" LABELS arrays)
add_source_program_test(Array.Invalid.ComptimeBounds cli/inputs/array_comptime_out_of_bounds.ink 1 "" "INK-S0054" LABELS arrays)

# Array bounds remain checked; escaped raw addresses may be transported without dereference.
add_source_program_test(Array.Runtime.NegativeIndex cli/inputs/array_runtime_negative_index.ink 1 "" "INK-E0025" BYTECODE BYTECODE_ERROR "array index out of bounds" LABELS arrays)
add_source_program_test(Array.Runtime.ReadOutOfBounds cli/inputs/array_runtime_read_out_of_bounds.ink 1 "" "INK-E0025" BYTECODE BYTECODE_ERROR "array index out of bounds" LABELS arrays)
add_source_program_test(Array.Runtime.WriteOutOfBounds cli/inputs/array_runtime_write_out_of_bounds.ink 1 "" "INK-E0025" BYTECODE BYTECODE_ERROR "array index out of bounds" LABELS arrays)
add_source_program_test(Array.Runtime.AddressOutOfBounds cli/inputs/array_runtime_address_out_of_bounds.ink 1 "" "INK-E0025" BYTECODE BYTECODE_ERROR "array index out of bounds" LABELS arrays)
add_source_program_test(Array.Runtime.EmptyIndex cli/inputs/array_runtime_empty_index.ink 1 "" "INK-E0025" BYTECODE BYTECODE_ERROR "array index out of bounds" LABELS arrays)
add_source_program_test(Array.Runtime.WideIndex cli/inputs/array_runtime_wide_index.ink 1 "" "INK-E0025" BYTECODE BYTECODE_ERROR "array index out of bounds" LABELS arrays)
add_source_program_test(Array.Runtime.EscapedElement cli/inputs/array_runtime_escaped_element.ink 0 "PASS|" "" BYTECODE LABELS arrays)

# Declaration checks fail before host symbol lookup and do not depend on the host ABI.
add_source_program_test(External.Invalid.ArgumentType cli/inputs/external_invalid_argument_type.ink 1 "" "INK-S0004" LABELS external)
add_source_program_test(External.Invalid.ArgumentCount cli/inputs/external_invalid_argument_count.ink 1 "" "INK-S0008" LABELS external)
add_source_program_test(External.Invalid.RuntimeEmbeddedNul cli/inputs/external_runtime_embedded_nul.ink 1 "" "INK-S0019" LABELS external)

# Deliberately invalid input belongs to CLI fixtures, separate from runnable example programs.
add_source_program_test(Logical.Invalid.NotInteger cli/inputs/logical_invalid_not.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.AndLeftInteger cli/inputs/logical_invalid_and_left.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.SkippedAndRightInteger cli/inputs/logical_invalid_and_right.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.OrLeftInteger cli/inputs/logical_invalid_or_left.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.SkippedOrRightInteger cli/inputs/logical_invalid_or_right.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.CompareWidths cli/inputs/logical_invalid_compare_widths.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.CompareSignedness cli/inputs/logical_invalid_compare_signedness.ink 1 "" "INK-S0004" LABELS logical)
add_source_program_test(Logical.Invalid.CompareBoolOrder cli/inputs/logical_invalid_compare_bool.ink 1 "" "INK-S0004" LABELS logical)

# Reaching the selected wrapper must perform native lookup and fail in each short-circuit shape.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_source_program_test(Logical.ShortCircuit.reach_and cli/inputs/logical_reach_and.ink 1 "" "INK-E0014" LABELS logical external)
  add_source_program_test(Logical.ShortCircuit.reach_or cli/inputs/logical_reach_or.ink 1 "" "INK-E0014" LABELS logical external)
  add_source_program_test(Logical.ShortCircuit.nested_reach cli/inputs/logical_nested_reach.ink 1 "" "INK-E0014" LABELS logical external)
endif()

# Semantic control-flow failures are separate fixtures so each diagnostic is asserted independently.
add_source_program_test(ControlFlow.Invalid.Condition cli/inputs/control_invalid_condition.ink 1 "" "INK-S0004" LABELS control_flow)
add_source_program_test(ControlFlow.Invalid.UninitializedMerge cli/inputs/control_uninitialized_merge.ink 1 "" "INK-S0006" LABELS control_flow)
add_source_program_test(ControlFlow.Invalid.UninitializedReturningArm cli/inputs/control_uninitialized_returning_arm.ink 1 "" "INK-S0006" LABELS control_flow)
add_source_program_test(ControlFlow.Invalid.EscapedLocal cli/inputs/control_escaped_local.ink 1 "" "INK-S0001" LABELS control_flow)
add_source_program_test(ControlFlow.Invalid.UnselectedRuntimeName cli/inputs/control_unselected_runtime_name.ink 1 "" "INK-S0001" LABELS control_flow)

# Arithmetic failures distinguish type mismatches, literal range errors and execution diagnostics.
add_source_program_test(Arithmetic.Invalid.Widths cli/inputs/arithmetic_invalid_widths.ink 1 "" "INK-S0004" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.Signedness cli/inputs/arithmetic_invalid_signedness.ink 1 "" "INK-S0004" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.Bool cli/inputs/arithmetic_invalid_bool.ink 1 "" "INK-S0004" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.SignedRange cli/inputs/arithmetic_invalid_signed_range.ink 1 "" "INK-S0005" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.SignedMinimum cli/inputs/arithmetic_invalid_signed_minimum.ink 1 "" "INK-S0005" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.UnsignedNegative cli/inputs/arithmetic_invalid_unsigned_negative.ink 1 "" "INK-S0005" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.UnsignedRange cli/inputs/arithmetic_invalid_unsigned_range.ink 1 "" "INK-S0005" LABELS arithmetic)
add_source_program_test(Arithmetic.Invalid.DivisionZero cli/inputs/comptime_arithmetic_division_zero.ink 1 "" "INK-E0019" LABELS arithmetic comptime)
add_source_program_test(Arithmetic.Invalid.RemainderZero cli/inputs/comptime_arithmetic_remainder_zero.ink 1 "" "INK-E0019" LABELS arithmetic comptime)
add_source_program_test(Arithmetic.Invalid.ShiftNegative cli/inputs/comptime_arithmetic_shift_negative.ink 1 "" "INK-E0020" LABELS arithmetic comptime)
add_source_program_test(Arithmetic.Invalid.ShiftWidth cli/inputs/comptime_arithmetic_shift_width.ink 1 "" "INK-E0020" LABELS arithmetic comptime)

# Invalid calls and declarations each assert their source diagnostic independently.
add_source_program_test(Function.Invalid.MissingArgument cli/inputs/function_invalid_missing_argument.ink 1 "" "INK-S0008" LABELS function)
add_source_program_test(Function.Invalid.ExtraArgument cli/inputs/function_invalid_extra_argument.ink 1 "" "INK-S0008" LABELS function)
add_source_program_test(Function.Invalid.ArgumentType cli/inputs/function_invalid_argument_type.ink 1 "" "INK-S0004" LABELS function)
add_source_program_test(Function.Invalid.ArgumentWidth cli/inputs/function_invalid_argument_width.ink 1 "" "INK-S0004" LABELS function)
add_source_program_test(Function.Invalid.AmbiguousOverload cli/inputs/function_invalid_ambiguous_overload.ink 1 "" "INK-S0017" LABELS function)
add_source_program_test(Function.Invalid.NoMatchingOverload cli/inputs/function_invalid_no_matching_overload.ink 1 "" "INK-S0018" LABELS function)
add_source_program_test(Function.Invalid.MissingReturn cli/inputs/function_invalid_missing_return.ink 1 "" "INK-S0009" LABELS function)
add_source_program_test(Function.Invalid.EmptyReturn cli/inputs/function_invalid_empty_return.ink 1 "" "INK-S0009" LABELS function)
add_source_program_test(Function.Invalid.VoidReturn cli/inputs/function_invalid_void_return.ink 1 "" "INK-S0004" LABELS function)
add_source_program_test(Function.Invalid.ReturnType cli/inputs/function_invalid_return_type.ink 1 "" "INK-S0004" LABELS function)
add_source_program_test(Function.Invalid.VoidValue cli/inputs/function_invalid_void_value.ink 1 "" "INK-S0004" LABELS function)
add_source_program_test(Function.Invalid.Capture cli/inputs/function_invalid_capture.ink 1 "" "INK-S0020" LABELS function)
add_source_program_test(Function.Invalid.NotCallable cli/inputs/function_invalid_not_callable.ink 1 "" "INK-S0004" LABELS function)
add_source_program_test(Function.Invalid.DuplicateParameter cli/inputs/function_invalid_duplicate_parameter.ink 1 "" "INK-S0015" LABELS function)
add_source_program_test(Function.Invalid.DuplicateDefinition cli/inputs/function_invalid_duplicate_definition.ink 1 "" "INK-S0003" LABELS function)
add_source_program_test(Function.Invalid.MissingBody cli/inputs/function_missing_body.ink 1 "" "INK-S0028" LABELS function)
