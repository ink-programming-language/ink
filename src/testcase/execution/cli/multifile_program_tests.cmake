# Each case owns its source files and compile/link expectations in a separate directory.
file(GLOB INK_MULTIFILE_CASES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/execution/multifile/*/case.cmake")
list(SORT INK_MULTIFILE_CASES)
foreach(CaseManifest IN LISTS INK_MULTIFILE_CASES)
  get_filename_component(CaseDirectory "${CaseManifest}" DIRECTORY)
  get_filename_component(CaseName "${CaseDirectory}" NAME)
  add_test(NAME "ExecutionMultiFileTest.${CaseName}" COMMAND "${CMAKE_COMMAND}" "-DINK_COMPILER=$<TARGET_FILE:inkc>" "-DINK_CASE_DIRECTORY=${CaseDirectory}" "-DINK_TEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/execution-multifile/${CaseName}" -P "${CMAKE_CURRENT_SOURCE_DIR}/execution/cli/multifile_program_test.cmake")
  set_tests_properties("ExecutionMultiFileTest.${CaseName}" PROPERTIES TIMEOUT 120 LABELS "execution;source;bytecode;multifile")
endforeach()
