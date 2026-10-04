if(NOT DEFINED INK_COMPILER OR NOT DEFINED INK_TEST_DIRECTORY)
  message(FATAL_ERROR "Compiler executable and test directory are required")
endif()
file(MAKE_DIRECTORY "${INK_TEST_DIRECTORY}")
file(REAL_PATH "${INK_TEST_DIRECTORY}" TestRoot)
string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef RunId)
set(RunDirectory "${TestRoot}/inkc-${RunId}")
file(MAKE_DIRECTORY "${RunDirectory}")

# Redirect output to files so assertions also exercise the executable's real process streams.
function(check_compiler Name ExpectedStatus OutputPattern ErrorPattern StdinFile)
  set(OutputFile "${RunDirectory}/${Name}.stdout")
  set(ErrorFile "${RunDirectory}/${Name}.stderr")
  if(StdinFile STREQUAL "")
    execute_process(COMMAND "${INK_COMPILER}" ${ARGN} RESULT_VARIABLE Status OUTPUT_FILE "${OutputFile}" ERROR_FILE "${ErrorFile}" TIMEOUT 10)
  else()
    execute_process(COMMAND "${INK_COMPILER}" ${ARGN} INPUT_FILE "${StdinFile}" RESULT_VARIABLE Status OUTPUT_FILE "${OutputFile}" ERROR_FILE "${ErrorFile}" TIMEOUT 10)
  endif()
  file(READ "${OutputFile}" Output)
  file(READ "${ErrorFile}" Error)
  if(ExpectedStatus STREQUAL "panic")
    if(NOT "${Status}" STREQUAL "3" AND NOT "${Status}" MATCHES "[Aa]bort")
      message(FATAL_ERROR "${Name}: expected panic, got '${Status}'; stderr: ${Error}; artifacts: ${RunDirectory}")
    endif()
    if(NOT Error MATCHES "panic at ")
      message(FATAL_ERROR "${Name}: expected the ICE panic path; stderr: ${Error}; artifacts: ${RunDirectory}")
    endif()
  elseif(NOT "${Status}" STREQUAL "${ExpectedStatus}")
    message(FATAL_ERROR "${Name}: expected exit ${ExpectedStatus}, got '${Status}'; stderr: ${Error}; artifacts: ${RunDirectory}")
  endif()
  if(NOT Output MATCHES "${OutputPattern}")
    message(FATAL_ERROR "${Name}: unexpected stdout: ${Output}; artifacts: ${RunDirectory}")
  endif()
  if(NOT Error MATCHES "${ErrorPattern}")
    message(FATAL_ERROR "${Name}: unexpected stderr: ${Error}; artifacts: ${RunDirectory}")
  endif()
endfunction()

function(check_source_failure Name Source ExpectedStatus Diagnostic)
  set(Input "${RunDirectory}/${Name}.ink")
  file(WRITE "${Input}" "${Source}")
  check_compiler("${Name}" "${ExpectedStatus}" "^$" "${Diagnostic}" "" --interpret -i "${Input}")
endfunction()

# Help must document interpreter mode without requiring an input file or printing an error.
check_compiler(help 0 "--interpret" "^$" "" --help)
file(READ "${RunDirectory}/help.stdout" Help)
if(NOT Help MATCHES "--entry")
  message(FATAL_ERROR "Help does not document entry selection: ${Help}; artifacts: ${RunDirectory}")
endif()

# Default entry selection executes main rather than another zero-argument function in the same module.
set(Input "${RunDirectory}/entries.ink")
file(WRITE "${Input}" "func main(): i32 { return 0; } func custom(): i32 { return 23; }")
check_compiler(default_entry 0 "^$" "^$" "" --interpret -i "${Input}")

# Explicit entry selection preserves the selected i32 result as the process exit code.
check_compiler(custom_entry 23 "^$" "^$" "" --interpret --entry custom -i "${Input}")

# Standalone interpretation shares signature predeclaration with independently compiled source modules.
set(ForwardInput "${RunDirectory}/forward helper.ink")
file(WRITE "${ForwardInput}" "func main(): i32 { return helper(); } private func helper(): i32 { return 42; }")
check_compiler(forward_helper 42 "^$" "^$" "" --interpret -i "${ForwardInput}")

# A void entry in a UTF-8-named file succeeds without publishing an IR dump.
set(VoidInput "${RunDirectory}/入口源码.ink")
file(WRITE "${VoidInput}" "func main(): void { var Local: i32 = 7; }")
check_compiler(void_utf8_path 0 "^$" "^$" "" --interpret -i "${VoidInput}")

# Redirected stdin uses the same analysis and entry-selection path as a named source file.
check_compiler(stdin_entry 23 "^$" "^$" "${Input}" --interpret --entry custom -i -)

# An unreadable file is an invocation failure and does not reach source analysis.
check_compiler(missing_file 2 "^$" "cannot open" "" --interpret -i "${RunDirectory}/missing-inkc-process-input-71e4935b.ink")

# Omitting an execution mode or naming an unknown mode is a normal command-line error.
check_compiler(missing_mode 2 "^$" "--interpret" "" -i "${Input}")
check_compiler(unknown_mode 2 "^$" "--unknown-mode" "" --unknown-mode -i "${Input}")

# Interpreter mode rejects an IR-output request before creating the requested output file.
set(IrOutput "${RunDirectory}/conflicting-output.ir")
check_compiler(conflicting_mode 2 "^$" "-oir" "" --interpret -i "${Input}" -oir "${IrOutput}")
if(EXISTS "${IrOutput}")
  message(FATAL_ERROR "Conflicting interpreter and IR-output modes created an output file; artifacts: ${RunDirectory}")
endif()

# A missing entry is diagnosed before any function executes.
check_compiler(missing_entry 2 "^$" "entry 'absent' was not found" "" --interpret --entry absent -i "${Input}")

# Entry validation distinguishes invalid signatures, linkage, compile-time bodies and ambiguous overload sets.
check_source_failure(entry_arguments "func main(Value: i32): i32 { return Value; }" 2 "must take no arguments")
check_source_failure(entry_return_type "func main(): bool { return true; }" 2 "must return void or i32")
check_source_failure(entry_external "import \"C\" func main(): i32;" 2 "cannot be a native import")
check_source_failure(entry_comptime "comptime func main(): i32 { return 0; }" 2 "cannot be a comptime function")
check_source_failure(entry_missing_body "func main(): i32;" 1 "INK-S0028")
check_source_failure(entry_ambiguous "func main(): i32 { return 0; } func main(Value: i32): i32 { return Value; }" 2 "is ambiguous")

# Lexical, syntactic and semantic source errors retain their stage diagnostics and source-error exit status.
check_source_failure(lexical_error "@" 1 "INK-T")
check_source_failure(parse_error "func main(): i32 { return ;" 1 "INK-P")
check_source_failure(semantic_error "func main(): i32 { return UnknownValue; }" 1 "INK-S")

# Runtime user failures retain their execution code, entry source and context without compile-time wording or duplicate diagnostics.
check_source_failure(missing_symbol "import \"C\" func InkMissingCompilerProcessSymbol71e4935b(): i32; func main(): i32 { return InkMissingCompilerProcessSymbol71e4935b(); }" 1 "error\\[INK-E0014\\]: execution of entry 'main' failed: external symbol was not found")
foreach(Name IN ITEMS missing_symbol)
  file(READ "${RunDirectory}/${Name}.stderr" Error)
  if(NOT Error MATCHES "${Name}\\.ink:1: error\\[INK-E")
    message(FATAL_ERROR "${Name}: execution diagnostic lost its entry source: ${Error}; artifacts: ${RunDirectory}")
  endif()
  if(Error MATCHES "compile-time|INK-S0021")
    message(FATAL_ERROR "${Name}: runtime failure retained obsolete compile-time wording: ${Error}; artifacts: ${RunDirectory}")
  endif()
  string(REGEX MATCHALL "error\\[INK-" Diagnostics "${Error}")
  list(LENGTH Diagnostics DiagnosticCount)
  if(NOT DiagnosticCount EQUAL 1)
    message(FATAL_ERROR "${Name}: expected one execution diagnostic, got ${DiagnosticCount}: ${Error}; artifacts: ${RunDirectory}")
  endif()
endforeach()

# Ordinary Ink declarations without bodies fail during analysis, even when a runtime entry could call them.
check_source_failure(missing_body "func F(): i32; func main(): i32 { return F(); }" 1 "error\\[INK-S0028\\]")

# Source branches share the same IR execution path for runtime entries and compile-time function calls.
set(BranchInput "${RunDirectory}/branches.ink")
configure_file("${CMAKE_CURRENT_LIST_DIR}/inputs/interpreter_branches.ink" "${BranchInput}" COPYONLY)

# Runtime bool arguments choose one branch and retain definite initialization through the merge.
check_compiler(branch_true 19 "^$" "^$" "" --interpret -i "${BranchInput}")
check_compiler(branch_false 23 "^$" "^$" "" --interpret --entry alternate -i "${BranchInput}")

# A call in an unselected branch is not executed, and compile-time calls execute the same branch IR.
check_compiler(branch_skipped_call 17 "^$" "^$" "" --interpret --entry skipped -i "${BranchInput}")
check_compiler(branch_comptime 31 "^$" "^$" "" --interpret --entry compiled -i "${BranchInput}")

# Branch checking rejects non-bool conditions, incomplete initialization, and incomplete return paths.
check_source_failure(branch_condition_type "func main(): i32 { if (1) { return 1; } else { return 0; } }" 1 "INK-S0004")
check_source_failure(branch_uninitialized "func Choose(Flag: bool): i32 { var Result: i32; if (Flag) { Result = 1; } return Result; } func main(): i32 { return Choose(false); }" 1 "INK-S0006")
check_source_failure(branch_missing_return "func Choose(Flag: bool): i32 { if (Flag) { return 1; } } func main(): i32 { return Choose(false); }" 1 "INK-S0009")

# The same arithmetic failure in compile-time evaluation keeps its concrete execution code and compile-time context.
check_source_failure(comptime_division_by_zero "comptime var Result: i32 = 1 / 0; func main(): i32 { return 0; }" 1 "error\\[INK-E0019\\]: compile-time execution failed: division by zero")
check_source_failure(comptime_invalid_shift "comptime var Result: i32 = 1 << 32; func main(): i32 { return 0; }" 1 "error\\[INK-E0020\\]: compile-time execution failed: invalid shift count")

# Native modes share entry checks and reject conflicting output modes or invalid optimization settings.
set(LLVMOutput "${RunDirectory}/native output.ll")
set(ObjectOutput "${RunDirectory}/native output.obj")
check_compiler(llvm_output 0 "^$" "^$" "" --emit-llvm "${LLVMOutput}" --opt-level 2 --entry custom -i "${Input}")
file(READ "${LLVMOutput}" LLVMText)
if(NOT LLVMText MATCHES "target triple" OR NOT LLVMText MATCHES "define.*@main")
  message(FATAL_ERROR "LLVM output lacks its native target or entry wrapper; artifacts: ${RunDirectory}")
endif()
check_compiler(native_conflict 2 "^$" "--emit" "" --emit-llvm "${LLVMOutput}" --emit-object "${ObjectOutput}" -i "${Input}")
check_compiler(native_interpret_conflict 2 "^$" "--interpret|--emit-object" "" --interpret --emit-object "${ObjectOutput}" -i "${Input}")
check_compiler(native_invalid_optimization 2 "^$" "--opt-level" "" --emit-object "${ObjectOutput}" --opt-level 4 -i "${Input}")
check_compiler(interpret_optimization 2 "^$" "--opt-level" "" --interpret --opt-level 2 -i "${Input}")
check_compiler(native_missing_entry 2 "^$" "entry 'absent' was not found" "" --emit-object "${ObjectOutput}" --entry absent -i "${Input}")
if(EXISTS "${ObjectOutput}")
  message(FATAL_ERROR "Invalid native invocation created an object file; artifacts: ${RunDirectory}")
endif()

# An unresolved class in a function signature must fail during shared analysis in every execution mode.
set(OpaqueInput "${RunDirectory}/opaque.ink")
file(WRITE "${OpaqueInput}" "class Node; func take(P: *Node): i32 { return 0; } func main(): i32 { return 42; }")
check_compiler(opaque_interpret 1 "^$" "INK-S0056" "" --interpret -i "${OpaqueInput}")
check_compiler(opaque_bytecode 1 "^$" "INK-S0056" "" --emit-bytecode "${RunDirectory}/opaque.inkobj" -i "${OpaqueInput}")
check_compiler(opaque_native 1 "^$" "INK-S0056" "" --emit-object "${ObjectOutput}" -i "${OpaqueInput}")
if(EXISTS "${ObjectOutput}" OR EXISTS "${RunDirectory}/opaque.inkobj")
  message(FATAL_ERROR "An incomplete class signature produced a compiled artifact; artifacts: ${RunDirectory}")
endif()

# Execution resource exhaustion follows the established internal-error panic policy.
set(ENV{INK_EXECUTION_MAX_CALL_DEPTH} "8")
check_source_failure(recursive_budget "func main(): i32 { return main(); }" panic "internal compiler error\\[INK-E0022\\]: execution of entry 'main' failed: execution budget exceeded")

# Remove only this successful run's private directory; failed runs keep their source and captured streams.
file(REAL_PATH "${RunDirectory}" ResolvedRunDirectory)
get_filename_component(ResolvedParent "${ResolvedRunDirectory}" DIRECTORY)
if(NOT ResolvedParent STREQUAL TestRoot)
  message(FATAL_ERROR "Refusing to clean a process-test directory outside ${TestRoot}: ${ResolvedRunDirectory}")
endif()
file(REMOVE_RECURSE "${ResolvedRunDirectory}")
