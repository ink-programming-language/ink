if(NOT DEFINED INK_COMPILER OR NOT DEFINED INK_TEST_DIRECTORY)
  message(FATAL_ERROR "Ink compiler executable and test directory are required")
endif()
file(MAKE_DIRECTORY "${INK_TEST_DIRECTORY}/source directory with spaces")
file(WRITE "${INK_TEST_DIRECTORY}/source.ink" "")
file(WRITE "${INK_TEST_DIRECTORY}/source directory with spaces/hello world.ink" "")
file(WRITE "${INK_TEST_DIRECTORY}/-source.ink" "")

function(expect_result Name ExpectedStatus OutputPattern)
  if(NOT "${Status}" STREQUAL "${ExpectedStatus}")
    message(FATAL_ERROR "${Name}: expected exit ${ExpectedStatus}, got ${Status}: stdout=${Output}: stderr=${Error}")
  endif()
  if(ExpectedStatus STREQUAL "0")
    if(NOT Error STREQUAL "")
      message(FATAL_ERROR "${Name}: successful invocation wrote stderr: ${Error}")
    endif()
    if(OutputPattern STREQUAL "")
      if(NOT Output STREQUAL "")
        message(FATAL_ERROR "${Name}: successful compilation must be silent: ${Output}")
      endif()
    elseif(NOT Output MATCHES "${OutputPattern}")
      message(FATAL_ERROR "${Name}: expected output matching '${OutputPattern}': ${Output}")
    endif()
  elseif(NOT Output STREQUAL "" OR NOT Error MATCHES "^inkc: error:")
    message(FATAL_ERROR "${Name}: invocation errors must use the compiler diagnostic and stderr only: stdout=${Output}: stderr=${Error}")
  endif()
endfunction()

function(expect_source_diagnostic Name ExpectedLocation ExpectedCode ExpectedRange ExpectedLine)
  string(FIND "${Error}" "${ExpectedLocation}" LocationIndex)
  string(FIND "${Error}" "${ExpectedCode}" CodeIndex)
  string(FIND "${Error}" "${ExpectedRange}" RangeIndex)
  string(FIND "${Error}" "${ExpectedLine}" SourceIndex)
  string(FIND "${Error}" "^" MarkerIndex)
  if(NOT Output STREQUAL "" OR LocationIndex LESS 0 OR CodeIndex LESS 0 OR RangeIndex LESS 0 OR SourceIndex LESS 0 OR MarkerIndex LESS 0)
    message(FATAL_ERROR "${Name}: expected source location, diagnostic code, byte range, source line and caret: stdout=${Output}: stderr=${Error}")
  endif()
endfunction()

function(expect_source_error Name ExpectedLocation ExpectedCode ExpectedRange ExpectedLine)
  if(NOT Status STREQUAL "1" OR Error MATCHES "INK-S" OR Error MATCHES "internal compiler error")
    message(FATAL_ERROR "${Name}: expected source failure without semantic analysis, got ${Status}: stdout=${Output}: stderr=${Error}")
  endif()
  expect_source_diagnostic("${Name}" "${ExpectedLocation}" "${ExpectedCode}" "${ExpectedRange}" "${ExpectedLine}")
endfunction()

# A short input option compiles an empty source into an empty module.
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" WORKING_DIRECTORY "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Short input option" "0" "")

# The long input option supports separate and equals-attached values.
execute_process(COMMAND "${INK_COMPILER}" --input "source.ink" WORKING_DIRECTORY "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Long input option" "0" "")
execute_process(COMMAND "${INK_COMPILER}" "--input=source.ink" WORKING_DIRECTORY "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Equals-attached input option" "0" "")

# Spaces in a quoted input path remain part of one argument.
execute_process(COMMAND "${INK_COMPILER}" -i "source directory with spaces/hello world.ink" WORKING_DIRECTORY "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Input path with spaces" "0" "")

# A value beginning with a dash is still an input path when supplied after -i.
execute_process(COMMAND "${INK_COMPILER}" -i "-source.ink" WORKING_DIRECTORY "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Input path beginning with a dash" "0" "")

# A terminator after a valid input does not create a positional argument.
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" -- WORKING_DIRECTORY "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Trailing option terminator" "0" "")

# Help and version remain available without a required input path.
execute_process(COMMAND "${INK_COMPILER}" -h RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Short help option" "0" "-i")
execute_process(COMMAND "${INK_COMPILER}" --help RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Long help option" "0" "--input")
execute_process(COMMAND "${INK_COMPILER}" -V RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Short version option" "0" "inkc")
execute_process(COMMAND "${INK_COMPILER}" --version RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Long version option" "0" "inkc")

# Empty invocations and a lone terminator report a missing required input.
execute_process(COMMAND "${INK_COMPILER}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Missing required input" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -- RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Terminator without input" "2" "")

# Both input spellings report a missing value through the normal error return.
execute_process(COMMAND "${INK_COMPILER}" -i RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Missing short-option value" "2" "")
execute_process(COMMAND "${INK_COMPILER}" --input RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Missing long-option value" "2" "")

# Explicit empty values must not become valid input paths.
execute_process(COMMAND "${INK_COMPILER}" -i "" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Empty separate input value" "2" "")
execute_process(COMMAND "${INK_COMPILER}" "--input=" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Empty equals-attached input value" "2" "")

# Repeating an input under either alias is an invocation error.
execute_process(COMMAND "${INK_COMPILER}" -i "first.ink" --input "second.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Repeated input aliases" "2" "")

# Unknown short, long, and malformed options never use the library's fatal path.
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" -z RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Unknown short option" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" --unknown RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Unknown long option" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" --- RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Malformed option syntax" "2" "")

# A grouped unknown option followed by a missing input value is also recoverable.
execute_process(COMMAND "${INK_COMPILER}" -xi RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Grouped unknown option with missing input" "2" "")

# Positional paths are rejected before and after the option terminator.
execute_process(COMMAND "${INK_COMPILER}" "source.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Bare positional input" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" -- "extra.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Positional argument after terminator" "2" "")

# A user-supplied empty positional argument must not be discarded as the parser guard.
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" "" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Trailing empty positional argument" "2" "")
execute_process(COMMAND "${INK_COMPILER}" "" -i "source.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Leading empty positional argument" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "source.ink" -- "" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Empty positional argument after terminator" "2" "")

# Explicit values for help and version are rejected without bool-conversion failures.
execute_process(COMMAND "${INK_COMPILER}" "--help=bad" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Unexpected help value" "2" "")
execute_process(COMMAND "${INK_COMPILER}" "--version=bad" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Unexpected version value" "2" "")

# Help does not hide malformed supplied input or an unknown option.
execute_process(COMMAND "${INK_COMPILER}" --help -i RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Help with missing supplied input value" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "" --help RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Help with empty supplied input value" "2" "")
execute_process(COMMAND "${INK_COMPILER}" --help --unknown RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Help with unknown option" "2" "")

# Nested empty blocks exercise semantic traversal without unsupported declarations.
file(WRITE "${INK_TEST_DIRECTORY}/blocks.ink" "{}\n{ { } }\n")
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/blocks.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Nested empty blocks" "0" "")

# Supported expression statements complete both semantic passes without producing output.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/literals.ink" CONTENT "0; 1; (14); \"Hello, world!\\n\"; 1.5; 'a'; ((true)); false;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/literals.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Literal expression statements" "0" "")

# Top-level negation needs an instruction block and fails an insertion check or construction ICE because constant folding is deferred.
foreach(Expression IN ITEMS "-128" "-32768" "-2147483647" "-(2147483647)" "-0x7FFFFFFF" "-1.5" "-0.0")
  file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/negative-literals.ink" CONTENT "${Expression};" NEWLINE_STYLE LF)
  execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/negative-literals.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
  if(Status STREQUAL "0" OR Status MATCHES "[Tt]imeout")
    message(FATAL_ERROR "Top-level negation must fail an insertion check or construction ICE: ${Status}: stdout=${Output}: stderr=${Error}")
  endif()
  if(Error MATCHES "INK-S0013")
    string(LENGTH "${Expression}" ExpressionLength)
    expect_source_diagnostic("Top-level negation without a block" "negative-literals.ink:1:1:" "internal compiler error[INK-S0013]" "[bytes 0, ${ExpressionLength})" "${Expression};")
  elseif(NOT Error MATCHES "check failed: canAppend")
    message(FATAL_ERROR "Top-level negation must fail an insertion check or construction ICE: ${Status}: stdout=${Output}: stderr=${Error}")
  endif()
endforeach()

# Unknown names return a source diagnostic instead of the former unsupported-SimpleStmt panic.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/unknown-name.ink" CONTENT "Missing;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/unknown-name.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Unknown expression name must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Unknown expression name" "unknown-name.ink:1:1:" "error[INK-S0001]" "[bytes 0, 7)" "Missing;")

# Untyped integers use i32 and report numeric overflow with the complete literal source span.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/integer-overflow.ink" CONTENT "2147483648;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/integer-overflow.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Integer overflow must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Integer literal overflow" "integer-overflow.ink:1:1:" "error[INK-S0005]" "[bytes 0, 10)" "2147483648;")

# A positive operand above the i32 maximum fails before unary negation with its own source range.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/negative-integer-overflow.ink" CONTENT "-2147483649;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/negative-integer-overflow.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Negative integer overflow must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Negative integer literal overflow" "negative-integer-overflow.ink:1:2:" "error[INK-S0005]" "[bytes 1, 11)" "-2147483649;")

# A semantic type result cannot be consumed as a runtime expression statement.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/type-value.ink" CONTENT "i32;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/type-value.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "A type in a runtime statement must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Type in runtime statement" "type-value.ink:1:1:" "error[INK-S0004]" "[bytes 0, 3)" "i32;")

# Missing sources and directory paths fail as file I/O errors before tokenization.
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/does-not-exist.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Missing source file" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Directory input" "2" "")

# A lexical error after a valid declaration prevents semantic analysis.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/lexical.ink" CONTENT "func f(): void {}\n@" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/lexical.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_source_error("Invalid token" "lexical.ink:2:1:" "error[INK-T0004]" "[bytes 18, 19)" "@")
if(Error MATCHES "INK-P")
  message(FATAL_ERROR "Lexical errors must not produce duplicate parser diagnostics: ${Error}")
endif()

# Invalid UTF-8 is diagnosed at its byte location without entering semantic analysis.
string(ASCII 255 InvalidUtf8)
file(WRITE "${INK_TEST_DIRECTORY}/invalid-utf8.ink" "${InvalidUtf8}")
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/invalid-utf8.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_source_error("Invalid UTF-8" "invalid-utf8.ink:1:1:" "error[INK-T0001]" "[bytes 0, 1)" "\\xFF")

# A parser insertion diagnostic keeps its zero-width span and prevents semantic analysis.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/syntax.ink" CONTENT "func f(): void {}\nvar a = ;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/syntax.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_source_error("Missing initializer" "syntax.ink:2:9:" "error[INK-P0002]" "[bytes 26, 26)" "var a = ;")

# Fixed-parameter definitions support direct, indirect and nested positional calls alongside implicit void returns.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/functions.ink" CONTENT "func identity(Value: i32): i32 { { return Value; } } func apply(Callback: func(i32): i32): i32 { return Callback(identity(7)); } func main(): i32 { return apply(identity); } func empty(): void {}" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/functions.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Function definitions" "0" "")

# Local addresses and nested dereferences support argument and return values, including the Windows WriteFile pointer signature.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/pointer-operations.ink" CONTENT "import \"C\" func WriteFile(File: *void, Buffer: *const u8, BytesToWrite: u32, BytesWritten: *u32, Overlapped: *void): i32; func pass(Value: u32): u32 { return Value; } func count(): u32 { var Value: u32 = 14; var Pointer = &Value; var Indirect = &Pointer; return pass(**Indirect); } func main(): i32 { var Written: u32 = 0; return WriteFile(nullptr, \"hello, world\\r\\n\", count(), &Written, nullptr); }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/pointer-operations.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Address and dereference expressions" "0" "")

# Dereferencing an integer reports the complete unary expression instead of an unsupported-node panic.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/dereference-integer.ink" CONTENT "func main(): i32 { return *42; }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/dereference-integer.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Dereferencing an integer must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Dereferencing an integer" "dereference-integer.ink:1:27:" "error[INK-S0027]" "[bytes 26, 29)" "func main(): i32 { return *42; }")

# An integer temporary has no addressable local storage, and the diagnostic includes the address-of operator.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/address-temporary.ink" CONTENT "func main(): *i32 { return &42; }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/address-temporary.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Taking a temporary's address must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Address of a temporary" "address-temporary.ink:1:28:" "error[INK-S0026]" "[bytes 27, 30)" "func main(): *i32 { return &42; }")

# Dereferencing an uninitialized pointer warns on stderr and still completes semantic analysis successfully.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/uninitialized-pointer.ink" CONTENT "func main(): i32 { var Pointer: *i32; return *Pointer; }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/uninitialized-pointer.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "0" OR Error MATCHES "error\\[")
  message(FATAL_ERROR "Dereferencing an uninitialized pointer must succeed with a warning: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Uninitialized pointer read" "uninitialized-pointer.ink:1:47:" "warning[INK-S0006]" "[bytes 46, 53)" "func main(): i32 { var Pointer: *i32; return *Pointer; }")

# Taking an uninitialized local's address emits a distinct warning without failing compilation or treating the address as a read.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/uninitialized-address.ink" CONTENT "func main(): *i32 { var Value: i32; return &Value; }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/uninitialized-address.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "0" OR Error MATCHES "error\\[" OR Error MATCHES "INK-S0006")
  message(FATAL_ERROR "Taking an uninitialized local's address must succeed with only an address warning: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Uninitialized local address" "uninitialized-address.ink:1:44:" "warning[INK-S0062]" "[bytes 43, 49)" "func main(): *i32 { var Value: i32; return &Value; }")

# Passing an uninitialized local still generates its value and completes the call after reporting a read warning.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/uninitialized-argument.ink" CONTENT "func pass(Value: i32): i32 { return Value; } func main(): i32 { var Value: i32; return pass(Value); }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/uninitialized-argument.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "0" OR Error MATCHES "error\\[")
  message(FATAL_ERROR "Passing an uninitialized local must succeed with a read warning: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Uninitialized argument read" "uninitialized-argument.ink:1:93:" "warning[INK-S0006]" "[bytes 92, 97)" "func pass(Value: i32): i32 { return Value; } func main(): i32 { var Value: i32; return pass(Value); }")

# An initialization warning does not suppress the invalid-dereference error or change its failing exit status.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/uninitialized-invalid-dereference.ink" CONTENT "func main(): i32 { var Value: i32; return *Value; }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/uninitialized-invalid-dereference.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Dereferencing an uninitialized integer must fail with both a warning and a user error: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Uninitialized invalid operand read" "uninitialized-invalid-dereference.ink:1:44:" "warning[INK-S0006]" "[bytes 43, 48)" "func main(): i32 { var Value: i32; return *Value; }")
expect_source_diagnostic("Invalid dereference after initialization warning" "uninitialized-invalid-dereference.ink:1:43:" "error[INK-S0027]" "[bytes 42, 48)" "func main(): i32 { var Value: i32; return *Value; }")

# String constants convert to read-only byte pointers in calls, returns and local initializers without changing CLI output.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/string-pointer.ink" CONTENT "import \"C\" func consume(Message: *const u8): i32; func text(): *const u8 { return \"hello\"; } func main(): i32 { const Message: *const u8 = \"hello\"; consume(Message); consume(text()); return consume((\"hello\")); }" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/string-pointer.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Implicit string pointer conversion" "0" "")

# String pointer conversion does not expose writable storage and retains the literal's source range in its diagnostic.
set(MutableStringSource "import \"C\" func consume(Message: *u8): i32;\nfunc main(): i32 { return consume(\"hello\"); }")
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/mutable-string-pointer.ink" CONTENT "${MutableStringSource}" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/mutable-string-pointer.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Writable string pointers must fail with a user diagnostic: ${Status}: ${Error}")
endif()
string(FIND "${MutableStringSource}" "\"hello\"" StringStart)
math(EXPR StringEnd "${StringStart} + 7")
expect_source_diagnostic("Writable string pointer" "mutable-string-pointer.ink:2:35:" "error[INK-S0004]" "[bytes ${StringStart}, ${StringEnd})" "func main(): i32 { return consume(\"hello\"); }")

# Incorrect call arity is a recoverable user diagnostic with the call expression's source range.
set(CallAritySource "func identity(Value: i32): i32 { return Value; }\nfunc main(): i32 { return identity(); }")
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/call-arity.ink" CONTENT "${CallAritySource}" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/call-arity.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "Incorrect call arity must fail with a user diagnostic: ${Status}: ${Error}")
endif()
string(FIND "${CallAritySource}" "identity()" CallStart)
math(EXPR CallEnd "${CallStart} + 10")
expect_source_diagnostic("Incorrect call arity" "call-arity.ink:2:27:" "error[INK-S0008]" "[bytes ${CallStart}, ${CallEnd})" "func main(): i32 { return identity(); }")

# A return outside any function reports a recoverable semantic diagnostic with its source range.
file(CONFIGURE OUTPUT "${INK_TEST_DIRECTORY}/return-outside.ink" CONTENT "return;" NEWLINE_STYLE LF)
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/return-outside.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "1" OR Error MATCHES "internal compiler error")
  message(FATAL_ERROR "A return outside a function must fail with a user diagnostic: ${Status}: ${Error}")
endif()
expect_source_diagnostic("Return outside function" "return-outside.ink:1:1:" "error[INK-S0016]" "[bytes 0, 7)" "return;")

# Unsupported function attributes retain the semantic unsupported-node panic and complete source context.
file(WRITE "${INK_TEST_DIRECTORY}/unsupported.ink" "[tag] func f(): void {}\n")
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/unsupported.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(Status STREQUAL "0" OR Status MATCHES "[Tt]imeout" OR NOT Error MATCHES "FunctionDecl")
  message(FATAL_ERROR "Semantic analysis did not report its unsupported function: ${Status}: stdout=${Output}: stderr=${Error}")
endif()
expect_source_diagnostic("Unsupported function" "unsupported.ink:1:2:" "internal compiler error[INK-S0012]" "[bytes 1, 23)" "[tag] func f(): void {}")
if(NOT Error MATCHES "\\^~+")
  message(FATAL_ERROR "The unsupported declaration must underline its source range: ${Error}")
endif()
