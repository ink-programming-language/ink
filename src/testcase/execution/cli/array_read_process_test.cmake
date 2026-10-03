if(NOT DEFINED INK_COMPILER OR NOT DEFINED INK_TEST_DIRECTORY)
  message(FATAL_ERROR "Compiler executable and test directory are required")
endif()
string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef RunId)
set(RunDirectory "${INK_TEST_DIRECTORY}/arrays-${RunId}")
file(MAKE_DIRECTORY "${RunDirectory}")

if(WIN32)
  set(ReadName "_read")
  set(ReadDeclaration "import \"C\" func _read(Fd: i32, Buffer: *u8, Count: u32): i32;")
else()
  set(ReadName "read")
  if(INK_POINTER_BYTES EQUAL 4)
    set(ReadDeclaration "import \"C\" func read(Fd: i32, Buffer: *u8, Count: u32): i32;")
  else()
    set(ReadDeclaration "import \"C\" func read(Fd: i32, Buffer: *u8, Count: u64): i64;")
  endif()
endif()

# A real OS-backed read fills an interior array address, returns short counts and EOF, and never consumes input for a zero count.
set(ReadBody [=[
func main(): i32
{
  var Buffer: [u8; 8] = [0; 8];
  var First = @ReadName@(0, &Buffer[1], 3);
  if (First != 3) { return 1; }
  if (Buffer[0] != 0 || Buffer[1] != 65 || Buffer[2] != 66 || Buffer[3] != 67 || Buffer[4] != 0) { return 2; }
  var Before = Buffer;
  var Empty = @ReadName@(0, &Buffer[0], 0);
  if (Empty != 0) { return 3; }
  var Short = @ReadName@(0, &Buffer[3], 5);
  if (Short != 3) { return 4; }
  if (Buffer[3] != 68 || Buffer[4] != 69 || Buffer[5] != 70 || Buffer[6] != 0 || Buffer[7] != 0) { return 5; }
  if (Before[3] != 67 || Before[4] != 0) { return 6; }
  var Eof = @ReadName@(0, &Buffer[0], 8);
  if (Eof != 0 || Buffer[0] != 0 || Buffer[1] != 65) { return 7; }
  return 42;
}
]=])
string(CONFIGURE "${ReadBody}" ReadBody @ONLY)
file(WRITE "${RunDirectory}/arrayread.ink" "${ReadDeclaration}\n${ReadBody}")
file(WRITE "${RunDirectory}/input.bin" "ABCDEF")

function(run_array_command Name ExpectedStatus)
  execute_process(COMMAND "${INK_COMPILER}" ${ARGN} INPUT_FILE "${RunDirectory}/input.bin" RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 20)
  file(WRITE "${RunDirectory}/${Name}.stdout" "${Output}")
  file(WRITE "${RunDirectory}/${Name}.stderr" "${Error}")
  if(NOT "${Status}" STREQUAL "${ExpectedStatus}" OR NOT Output STREQUAL "" OR NOT Error STREQUAL "")
    message(FATAL_ERROR "${Name}: expected ${ExpectedStatus}, got '${Status}'; stdout '${Output}'; stderr '${Error}'; artifacts: ${RunDirectory}")
  endif()
endfunction()

run_array_command(interpret 42 --interpret -i "${RunDirectory}/arrayread.ink")
run_array_command(compile 0 --emit-bytecode "${RunDirectory}/arrayread.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/arrayread.ink")
run_array_command(link 0 --link-bytecode "${RunDirectory}/arrayread.inkobj" --entry "arrayread#main" -o "${RunDirectory}/arrayread.inkbc")
run_array_command(run_bytecode 42 --run-bytecode -i "${RunDirectory}/arrayread.inkbc")
run_array_command(run_bytecode_again 42 --run-bytecode -i "${RunDirectory}/arrayread.inkbc")

# Dynamic negative and one-past indices produce user diagnostics in both source and saved bytecode execution.
foreach(Index IN ITEMS -1 2)
  file(WRITE "${RunDirectory}/bounds.ink" "func Pick(Index: i32): i32 { var Values = [10, 20]; return Values[Index]; } func main(): i32 { return Pick(${Index}); }")
  run_array_command(compile_bounds 0 --emit-bytecode "${RunDirectory}/bounds.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/bounds.ink")
  run_array_command(link_bounds 0 --link-bytecode "${RunDirectory}/bounds.inkobj" --entry "bounds#main" -o "${RunDirectory}/bounds.inkbc")
  foreach(Mode IN ITEMS source bytecode)
    if(Mode STREQUAL "source")
      set(Arguments --interpret -i "${RunDirectory}/bounds.ink")
      set(ExpectedDiagnostic "INK-E0025")
    else()
      set(Arguments --run-bytecode -i "${RunDirectory}/bounds.inkbc")
      set(ExpectedDiagnostic "array index out of bounds")
    endif()
    execute_process(COMMAND "${INK_COMPILER}" ${Arguments} RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 20)
    if(NOT Status EQUAL 1 OR NOT Output STREQUAL "" OR NOT Error MATCHES "${ExpectedDiagnostic}")
      message(FATAL_ERROR "${Mode} index ${Index}: expected user bounds diagnostic, got '${Status}', '${Output}', '${Error}'; artifacts: ${RunDirectory}")
    endif()
  endforeach()
endforeach()
