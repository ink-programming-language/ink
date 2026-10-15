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

# Missing sources and directory paths fail as file I/O errors before tokenization.
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/does-not-exist.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Missing source file" "2" "")
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_result("Directory input" "2" "")

# A lexical error after a valid declaration prevents unsupported semantic analysis.
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

# A syntactically valid function reaches the current semantic unsupported-node panic with source context.
file(WRITE "${INK_TEST_DIRECTORY}/unsupported.ink" "func f(): void {}\n")
execute_process(COMMAND "${INK_COMPILER}" -i "${INK_TEST_DIRECTORY}/unsupported.ink" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(Status STREQUAL "0" OR Status MATCHES "[Tt]imeout" OR NOT Error MATCHES "FunctionDecl")
  message(FATAL_ERROR "Semantic analysis did not report its unsupported function: ${Status}: stdout=${Output}: stderr=${Error}")
endif()
expect_source_diagnostic("Unsupported function" "unsupported.ink:1:1:" "internal compiler error[INK-S0012]" "[bytes 0, 17)" "func f(): void {}")
if(NOT Error MATCHES "\\^~+")
  message(FATAL_ERROR "The unsupported declaration must underline its source range: ${Error}")
endif()
