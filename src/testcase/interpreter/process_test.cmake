if(NOT DEFINED INK_INTERPRETER OR NOT DEFINED INK_TEST_DIRECTORY)
    message(FATAL_ERROR "Interpreter executable and test directory are required")
endif()
file(MAKE_DIRECTORY "${INK_TEST_DIRECTORY}")
set(Input "${INK_TEST_DIRECTORY}/module input.vm")
file(WRITE "${Input}" ".module \"demo\"\n.section .text\n.function main\nentry:\n    ret\n.endfunction\n")

function(expect_success Name)
    if(NOT Status STREQUAL "0" OR NOT Output STREQUAL "" OR NOT Error STREQUAL "")
        message(FATAL_ERROR "${Name}: expected successful execution without output: ${Status}: stdout=${Output}: stderr=${Error}")
    endif()
endfunction()

function(expect_diagnostic Name ExpectedStatus Code Description)
    string(FIND "${Error}" "ink-interpreter: error[${Code}]: " DiagnosticStart)
    string(REGEX MATCHALL "INK-V[0-9][0-9][0-9][0-9]" DiagnosticCodes "${Error}")
    list(LENGTH DiagnosticCodes DiagnosticCount)
    if(NOT Status STREQUAL ExpectedStatus OR NOT Output STREQUAL "" OR NOT DiagnosticStart EQUAL 0 OR NOT DiagnosticCount EQUAL 1 OR NOT Error MATCHES "${Description}")
        message(FATAL_ERROR "${Name}: expected exit ${ExpectedStatus} and one ${Code} diagnostic on stderr: ${Status}: stdout=${Output}: stderr=${Error}")
    endif()
endfunction()

# MachineModule assembly is decoded, emitted and executed from a path containing spaces.
execute_process(COMMAND "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_success("File input with spaces")

# Explicit and implicit standard input follow the same module-to-executable pipeline.
execute_process(COMMAND "${INK_INTERPRETER}" "-" INPUT_FILE "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_success("Explicit standard input")
execute_process(COMMAND "${INK_INTERPRETER}" INPUT_FILE "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_success("Default standard input")

# Global storage and an earlier helper move main away from image offset zero and exercise local call relocation.
file(WRITE "${Input}" [=[.module "relocated-entry"
.section .data
.global bytes, 3, 1
    .byte 1, 2, 3
.endglobal
.section .text
.function helper
entry:
    ret
.endfunction
.function main
entry:
    call @helper
    ret
.endfunction
]=])
execute_process(COMMAND "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_success("Entry after globals and helper")

# Indirect calls load both Ink and native function addresses from relocated global pointer slots.
file(WRITE "${Input}" [=[.module "indirect-calls"
.declare function @identity
.section .data
.global pointers, 16, 8
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.endglobal
.section .text
.function main
entry:
    store64i [@pointers], @helper
    store64i [@pointers + 8], @identity
    call [@pointers]
    add64rri sp, sp, 16
    store64i [sp - 8], 41
    call [@pointers + 8]
    add64rri sp, sp, -16
    ret
.endfunction
.function helper
entry:
    ret
.endfunction
]=])
execute_process(COMMAND "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_success("Indirect Ink and native calls")

# Malformed and empty inputs must report the parser diagnostic before emission or execution.
file(WRITE "${Input}" "not MachineModule assembly\n")
execute_process(COMMAND "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_diagnostic("Malformed module text" "2" "INK-V0001" "invalid VM text")
file(WRITE "${Input}" "")
execute_process(COMMAND "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_diagnostic("Empty module text" "2" "INK-V0001" "invalid VM text")

# A declared unresolved call reaches loading and reports the link diagnostic without a generic duplicate.
file(WRITE "${Input}" ".module \"missing-import\"\n.declare function @missing.interpreter.process.import\n.section .text\n.function main\nentry:\n    call @missing.interpreter.process.import\n    ret\n.endfunction\n")
execute_process(COMMAND "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_diagnostic("Unresolved import" "2" "INK-V0004" "missing.interpreter.process.import")
if(Error MATCHES "cannot load")
    message(FATAL_ERROR "A link diagnostic must not be followed by a generic load error: ${Error}")
endif()

# Valid recursion reports stack exhaustion when a call frame cannot fit.
file(WRITE "${Input}" ".module \"recursive-call\"\n.section .text\n.function main\nentry:\n    call @main\n    ret\n.endfunction\n")
execute_process(COMMAND "${CMAKE_COMMAND}" -E env "INK_VM_STACK_BYTES=16" "${INK_INTERPRETER}" "${Input}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
expect_diagnostic("Call stack overflow" "1" "INK-V0005" "stack overflow")

# File-reading failures retain the invocation error status without generating a VM execution diagnostic.
execute_process(COMMAND "${INK_INTERPRETER}" "${INK_TEST_DIRECTORY}/does-not-exist.bin" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
if(NOT Status STREQUAL "2" OR NOT Output STREQUAL "" OR NOT Error MATCHES "^ink-interpreter: error: cannot read" OR Error MATCHES "INK-V")
    message(FATAL_ERROR "Missing input was not reported as an invocation failure: ${Status}: stdout=${Output}: stderr=${Error}")
endif()

# Informational options finish before reading input and keep diagnostics off stderr.
foreach(Option IN ITEMS "-h" "--help" "-v" "--version")
    execute_process(COMMAND "${INK_INTERPRETER}" "${Option}" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 10)
    if(NOT Status STREQUAL "0" OR NOT Error STREQUAL "" OR NOT Output MATCHES "ink-interpreter")
        message(FATAL_ERROR "Interpreter information option failed for ${Option}: ${Status}: stdout=${Output}: stderr=${Error}")
    endif()
endforeach()
