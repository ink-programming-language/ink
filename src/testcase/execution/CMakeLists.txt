add_executable(ink_execution_tests runtime_world_tests.cpp)

target_compile_features(ink_execution_tests PRIVATE cxx_std_17)
target_link_libraries(ink_execution_tests PRIVATE ink::execution ink::ir ink::target GTest::gmock_main)

if(MSVC)
  target_compile_options(ink_execution_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
gtest_discover_tests(ink_execution_tests DISCOVERY_TIMEOUT 60)
