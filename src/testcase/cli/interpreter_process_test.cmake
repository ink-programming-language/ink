if(NOT EXISTS "${INK_INTERPRETER}")
  message(FATAL_ERROR "ink_interpreter executable does not exist: ${INK_INTERPRETER}")
endif()

file(MAKE_DIRECTORY "${INK_TEST_DIRECTORY}")
set(HelloInput "${INK_TEST_DIRECTORY}/hello.ir")
set(StandardErrorInput "${INK_TEST_DIRECTORY}/standard_error.ir")
set(ReturnInput "${INK_TEST_DIRECTORY}/return.ir")
set(InvalidInput "${INK_TEST_DIRECTORY}/invalid.ir")
set(UnresolvedInput "${INK_TEST_DIRECTORY}/unresolved.ir")
set(MissingInput "${INK_TEST_DIRECTORY}/missing.ir")
file(WRITE "${HelloInput}" [=[inkir 1

@str.0 = private constant [14 x byte] c"Hello, world!\0A"

declare extern "C" i32 @write(i32, const byte*, ptrsize) [sideeffect]

define void @main() {
entry:
  %0 = call i32 @write(i32 1, const byte* @str.0[0], ptrsize 14)
  ret void
}
]=])
file(WRITE "${StandardErrorInput}" [=[inkir 1

@str.0 = private constant [14 x byte] c"Runtime error\0A"

declare extern "C" i32 @write(i32, const byte*, ptrsize) [sideeffect]

define void @main() {
entry:
  %0 = call i32 @write(i32 2, const byte* @str.0[0], ptrsize 14)
  ret void
}
]=])
file(WRITE "${ReturnInput}" [=[inkir 1

define i32 @main() {
entry:
  ret i32 7
}
]=])
file(WRITE "${InvalidInput}" "inkir 2\n")
file(WRITE "${UnresolvedInput}" [=[inkir 1

declare extern "C" void @ink_interpreter_missing_symbol_xyz()

define void @main() {
entry:
  call void @ink_interpreter_missing_symbol_xyz()
  ret void
}
]=])
file(REMOVE "${MissingInput}")

execute_process(COMMAND "${INK_INTERPRETER}" --help RESULT_VARIABLE HelpResult OUTPUT_VARIABLE HelpOutput ERROR_VARIABLE HelpError)
if(NOT HelpResult EQUAL 0 OR NOT "${HelpError}" STREQUAL "")
  message(FATAL_ERROR "help process contract failed: result=${HelpResult}\nstdout=${HelpOutput}\nstderr=${HelpError}")
endif()
string(FIND "${HelpOutput}" "ink_interpreter [OPTIONS]" HelpUsageOffset)
if(HelpUsageOffset EQUAL -1)
  message(FATAL_ERROR "help output does not contain the interpreter invocation spelling: ${HelpOutput}")
endif()

execute_process(COMMAND "${INK_INTERPRETER}" -i "${HelloInput}" RESULT_VARIABLE HelloResult OUTPUT_VARIABLE HelloOutput ERROR_VARIABLE HelloError)
if(NOT HelloResult EQUAL 0 OR NOT "${HelloOutput}" STREQUAL "Hello, world!\n" OR NOT "${HelloError}" STREQUAL "")
  message(FATAL_ERROR "Hello World execution failed: result=${HelloResult}\nstdout=${HelloOutput}\nstderr=${HelloError}")
endif()

execute_process(COMMAND "${INK_INTERPRETER}" -i "${StandardErrorInput}" RESULT_VARIABLE StandardErrorResult OUTPUT_VARIABLE StandardErrorOutput ERROR_VARIABLE StandardErrorError)
if(NOT StandardErrorResult EQUAL 0 OR NOT "${StandardErrorOutput}" STREQUAL "" OR NOT "${StandardErrorError}" STREQUAL "Runtime error\n")
  message(FATAL_ERROR "Runtime stderr execution failed: result=${StandardErrorResult}\nstdout=${StandardErrorOutput}\nstderr=${StandardErrorError}")
endif()

execute_process(COMMAND "${INK_INTERPRETER}" -i "${ReturnInput}" RESULT_VARIABLE ReturnResult OUTPUT_VARIABLE ReturnOutput ERROR_VARIABLE ReturnError)
if(NOT ReturnResult EQUAL 7 OR NOT "${ReturnOutput}" STREQUAL "" OR NOT "${ReturnError}" STREQUAL "")
  message(FATAL_ERROR "i32 main result was not forwarded to the process: result=${ReturnResult}\nstdout=${ReturnOutput}\nstderr=${ReturnError}")
endif()

execute_process(COMMAND "${INK_INTERPRETER}" -i "${InvalidInput}" RESULT_VARIABLE InvalidResult OUTPUT_VARIABLE InvalidOutput ERROR_VARIABLE InvalidError)
if(NOT InvalidResult EQUAL 1 OR NOT "${InvalidOutput}" STREQUAL "")
  message(FATAL_ERROR "invalid IR process contract failed: result=${InvalidResult}\nstdout=${InvalidOutput}\nstderr=${InvalidError}")
endif()
string(FIND "${InvalidError}" "error[INK-I0012]: unsupported InkIR format version 2; expected 1" InvalidDiagnosticOffset)
if(InvalidDiagnosticOffset EQUAL -1)
  message(FATAL_ERROR "invalid IR diagnostic was not written to stderr: ${InvalidError}")
endif()

execute_process(COMMAND "${INK_INTERPRETER}" -i "${UnresolvedInput}" RESULT_VARIABLE UnresolvedResult OUTPUT_VARIABLE UnresolvedOutput ERROR_VARIABLE UnresolvedError)
if(NOT UnresolvedResult EQUAL 1 OR NOT "${UnresolvedOutput}" STREQUAL "")
  message(FATAL_ERROR "unresolved extern process contract failed: result=${UnresolvedResult}\nstdout=${UnresolvedOutput}\nstderr=${UnresolvedError}")
endif()
string(FIND "${UnresolvedError}" "error[INK-E0003]: could not resolve external function @ink_interpreter_missing_symbol_xyz" ExecutionDiagnosticOffset)
if(ExecutionDiagnosticOffset EQUAL -1)
  message(FATAL_ERROR "execution diagnostic was not written to stderr: ${UnresolvedError}")
endif()

execute_process(COMMAND "${INK_INTERPRETER}" -i "${MissingInput}" RESULT_VARIABLE MissingResult OUTPUT_VARIABLE MissingOutput ERROR_VARIABLE MissingError)
if(NOT MissingResult EQUAL 2 OR NOT "${MissingOutput}" STREQUAL "")
  message(FATAL_ERROR "missing IR process contract failed: result=${MissingResult}\nstdout=${MissingOutput}\nstderr=${MissingError}")
endif()
string(FIND "${MissingError}" "ink_interpreter: error: cannot open" MissingErrorOffset)
if(MissingErrorOffset EQUAL -1)
  message(FATAL_ERROR "missing IR error was not written to stderr: ${MissingError}")
endif()
