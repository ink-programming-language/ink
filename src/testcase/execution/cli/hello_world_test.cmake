if(NOT DEFINED INK_COMPILER OR NOT DEFINED INK_SOURCE OR NOT DEFINED INK_TEST_DIRECTORY OR NOT DEFINED INK_WINDOWS_STDOUT)
  message(FATAL_ERROR "Compiler, Ink source, test directory and stdout platform are required")
endif()

file(MAKE_DIRECTORY "${INK_TEST_DIRECTORY}")
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef RunId)
set(OutputFile "${INK_TEST_DIRECTORY}/hello-world-${RunId}.stdout")

# Run the checked-in source through the actual compiler entry point; only the interpreted program writes stdout.
execute_process(COMMAND "${INK_COMPILER}" --interpret --entry main -i "${INK_SOURCE}" RESULT_VARIABLE Status OUTPUT_FILE "${OutputFile}" ERROR_VARIABLE Error TIMEOUT 15)
if(NOT "${Status}" STREQUAL "0" OR NOT Error STREQUAL "")
  message(FATAL_ERROR "Hello world failed: status=${Status}, stderr=${Error}, stdout=${OutputFile}")
endif()

# Keep the exact byte assertion: the Windows CRT writes CRLF in its default stdout text mode.
file(READ "${OutputFile}" Output HEX)
if(INK_WINDOWS_STDOUT)
  set(Expected "68656c6c6f2c20776f726c640d0a")
else()
  set(Expected "68656c6c6f2c20776f726c640a")
endif()
if(NOT Output STREQUAL Expected)
  message(FATAL_ERROR "Hello world stdout differs: expected=${Expected}, actual=${Output}, file=${OutputFile}")
endif()
file(REMOVE "${OutputFile}")
