if(NOT DEFINED INK_COMPILER OR NOT DEFINED INK_TEST_DIRECTORY)
  message(FATAL_ERROR "Compiler executable and test directory are required")
endif()
file(MAKE_DIRECTORY "${INK_TEST_DIRECTORY}")
string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef RunId)
set(RunDirectory "${INK_TEST_DIRECTORY}/bytecode-${RunId}")
file(MAKE_DIRECTORY "${RunDirectory}")

function(run_bytecode_command Name ExpectedStatus)
  execute_process(COMMAND "${INK_COMPILER}" ${ARGN} RESULT_VARIABLE Status OUTPUT_VARIABLE Output ERROR_VARIABLE Error TIMEOUT 30)
  file(WRITE "${RunDirectory}/${Name}.stdout" "${Output}")
  file(WRITE "${RunDirectory}/${Name}.stderr" "${Error}")
  if(NOT "${Status}" STREQUAL "${ExpectedStatus}")
    message(FATAL_ERROR "${Name}: expected ${ExpectedStatus}, got '${Status}'; stderr: ${Error}; artifacts: ${RunDirectory}")
  endif()
  if(NOT Output STREQUAL "")
    message(FATAL_ERROR "${Name}: unexpected stdout '${Output}'; artifacts: ${RunDirectory}")
  endif()
  if(ExpectedStatus EQUAL 0 AND NOT Error STREQUAL "")
    message(FATAL_ERROR "${Name}: unexpected stderr '${Error}'; artifacts: ${RunDirectory}")
  endif()
endfunction()

# Checked-in source fixtures keep language behavior visible independently of command construction.
foreach(Name IN ITEMS native nested_overloads private boundaries)
  configure_file("${CMAKE_CURRENT_LIST_DIR}/inputs/bytecode_${Name}.ink" "${RunDirectory}/${Name}.ink" COPYONLY)
endforeach()

# Dotted basenames and alternate extensions must not publish the same module identity as a directory-based .ink path.
file(MAKE_DIRECTORY "${RunDirectory}/pkt")
file(WRITE "${RunDirectory}/pkt.value.ink" [=[// This flat filename would collide with the module name of pkt/value.ink.
func main(): i32
{
  return 1;
}
]=])
file(WRITE "${RunDirectory}/pkt/value.ink" [=[// This directory-based source is the canonical spelling of module pkt.value.
func main(): i32
{
  return 42;
}
]=])
file(WRITE "${RunDirectory}/pkt/value.txt" [=[// Removing an arbitrary extension must not let this source impersonate pkt/value.ink.
func main(): i32
{
  return 7;
}
]=])
run_bytecode_command(reject_dotted_module_path 1 --emit-bytecode "${RunDirectory}/dotted_path.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/pkt.value.ink")
run_bytecode_command(reject_alternate_extension 1 --emit-bytecode "${RunDirectory}/alternate_extension.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/pkt/value.txt")
foreach(Name IN ITEMS reject_dotted_module_path reject_alternate_extension)
  file(READ "${RunDirectory}/${Name}.stderr" Error)
  if(NOT Error MATCHES "module source path must use an unambiguous \\.ink module name")
    message(FATAL_ERROR "${Name}: expected the ambiguous module-path diagnostic, got '${Error}'; artifacts: ${RunDirectory}")
  endif()
endforeach()
if(EXISTS "${RunDirectory}/dotted_path.inkobj" OR EXISTS "${RunDirectory}/alternate_extension.inkobj")
  message(FATAL_ERROR "An ambiguous module path published an object; artifacts: ${RunDirectory}")
endif()

# The canonical source still emits the expected module identity and runs its own implementation.
run_bytecode_command(compile_canonical_module_path 0 --emit-bytecode "${RunDirectory}/canonical_path.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/pkt/value.ink")
run_bytecode_command(link_canonical_module_path 0 --link-bytecode "${RunDirectory}/canonical_path.inkobj" --entry "pkt.value#main" -o "${RunDirectory}/canonical_path.inkbc")
run_bytecode_command(run_canonical_module_path 42 --run-bytecode -i "${RunDirectory}/canonical_path.inkbc")

# Import-free interpretation retains compatibility with arbitrary source filenames and returns the program's status.
configure_file("${RunDirectory}/pkt.value.ink" "${RunDirectory}/points.windows.ink" COPYONLY)
run_bytecode_command(interpret_dotted_filename 1 --interpret -i "${RunDirectory}/points.windows.ink")
file(READ "${RunDirectory}/interpret_dotted_filename.stderr" Error)
if(NOT Error STREQUAL "")
  message(FATAL_ERROR "Import-free interpretation rejected a dotted filename: ${Error}; artifacts: ${RunDirectory}")
endif()

# Compilation, linking and execution happen in separate processes; native addresses must be resolved after loading.
run_bytecode_command(compile_native 0 --emit-bytecode "${RunDirectory}/native.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/native.ink")
run_bytecode_command(link_native 0 --link-bytecode "${RunDirectory}/native.inkobj" --entry "native#main" -o "${RunDirectory}/native.inkbc")

# An object is not executable, and a truncated file must fail during loading.
run_bytecode_command(reject_object_execution 2 --run-bytecode -i "${RunDirectory}/native.inkobj")
file(WRITE "${RunDirectory}/truncated.inkbc" "INKBC")
run_bytecode_command(reject_truncated_file 2 --run-bytecode -i "${RunDirectory}/truncated.inkbc")

# A saved executable needs neither its source nor its input object and can rebind native symbols on repeated fresh loads.
file(RENAME "${RunDirectory}/native.ink" "${RunDirectory}/native.source-unavailable")
file(RENAME "${RunDirectory}/native.inkobj" "${RunDirectory}/native.object-unavailable")
run_bytecode_command(run_saved_native_image 17 --run-bytecode -i "${RunDirectory}/native.inkbc")
run_bytecode_command(run_saved_native_image_again 17 --run-bytecode -i "${RunDirectory}/native.inkbc")

# C ABI local and exported definitions execute their own bodies and retain source visibility through fresh loads.
run_bytecode_command(interpret_boundaries 42 --interpret -i "${RunDirectory}/boundaries.ink")
run_bytecode_command(compile_boundaries 0 --emit-bytecode "${RunDirectory}/boundaries.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/boundaries.ink")
run_bytecode_command(link_boundaries 0 --link-bytecode "${RunDirectory}/boundaries.inkobj" --entry "boundaries#main" -o "${RunDirectory}/boundaries.inkbc")
run_bytecode_command(reject_private_native_export_entry 2 --link-bytecode "${RunDirectory}/boundaries.inkobj" --entry "boundaries#inkPrivateEntryBoundary" -o "${RunDirectory}/private_native_entry.inkbc")
if(EXISTS "${RunDirectory}/private_native_entry.inkbc")
  message(FATAL_ERROR "Rejected private native export entry published an executable; artifacts: ${RunDirectory}")
endif()
file(RENAME "${RunDirectory}/boundaries.ink" "${RunDirectory}/boundaries.source-unavailable")
file(RENAME "${RunDirectory}/boundaries.inkobj" "${RunDirectory}/boundaries.object-unavailable")
run_bytecode_command(run_saved_boundaries 42 --run-bytecode -i "${RunDirectory}/boundaries.inkbc")

# Same-named nested functions under overloaded parents retain distinct private identities and execute their own bodies.
run_bytecode_command(compile_nested_overloads 0 --emit-bytecode "${RunDirectory}/nested_overloads.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/nested_overloads.ink")
run_bytecode_command(link_nested_overloads 0 --link-bytecode "${RunDirectory}/nested_overloads.inkobj" --entry "nested_overloads#main" -o "${RunDirectory}/nested_overloads.inkbc")
run_bytecode_command(run_nested_overloads 42 --run-bytecode -i "${RunDirectory}/nested_overloads.inkbc")

# A private source function works through its public wrapper but cannot become an executable entry.
run_bytecode_command(compile_private 0 --emit-bytecode "${RunDirectory}/private.inkobj" --module-root "${RunDirectory}" -i "${RunDirectory}/private.ink")
run_bytecode_command(link_private_wrapper 0 --link-bytecode "${RunDirectory}/private.inkobj" --entry "private#main" -o "${RunDirectory}/private.inkbc")
run_bytecode_command(run_private_wrapper 42 --run-bytecode -i "${RunDirectory}/private.inkbc")
run_bytecode_command(reject_private_entry 2 --link-bytecode "${RunDirectory}/private.inkobj" --entry "private#hidden" -o "${RunDirectory}/private_entry.inkbc")
if(EXISTS "${RunDirectory}/private_entry.inkbc")
  message(FATAL_ERROR "Rejected private entry published an executable; artifacts: ${RunDirectory}")
endif()

# Removed metadata flags cannot override source visibility, source imports or the path-derived module identity.
foreach(Flag IN ITEMS export module-visible import-symbol module)
  set(Value hidden)
  if(Flag STREQUAL "import-symbol")
    set(Value "hidden=provider#answer")
  elseif(Flag STREQUAL "module")
    set(Value renamed)
  endif()
  run_bytecode_command("reject_${Flag}" 2 --emit-bytecode "${RunDirectory}/rejected_${Flag}.inkobj" --module-root "${RunDirectory}" "--${Flag}" "${Value}" -i "${RunDirectory}/private.ink")
  file(READ "${RunDirectory}/reject_${Flag}.stderr" Error)
  if(NOT Error MATCHES "--${Flag}" OR EXISTS "${RunDirectory}/rejected_${Flag}.inkobj")
    message(FATAL_ERROR "Removed option --${Flag} was not rejected before object publication: ${Error}; artifacts: ${RunDirectory}")
  endif()
endforeach()
