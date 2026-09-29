# These corpora now start a child process for each fatal input instead of returning an error in-process.
foreach(TestName IN ITEMS ParserTest.ASTSerializationMutationCorpus ParserTest.ASTSerializationFramingFailures ParserTest.RecoveryBudgetCheckpoints)
  list(FIND ink_tests_TESTS "${TestName}" TestIndex)
  if(NOT TestIndex EQUAL -1)
    set_tests_properties("${TestName}" PROPERTIES TIMEOUT 300)
  endif()
endforeach()
