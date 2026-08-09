add_executable(ink_backend_tests backend_test.cpp)
target_compile_features(ink_backend_tests PRIVATE cxx_std_17)
target_link_libraries(ink_backend_tests PRIVATE ink::backend ink::frontend ink::ir ink::target GTest::gmock_main)

if(MSVC)
  target_compile_options(ink_backend_tests PRIVATE /utf-8)
endif()

include(GoogleTest)
gtest_discover_tests(ink_backend_tests DISCOVERY_TIMEOUT 60)

add_executable(ink_backend_fixture_emitter fixture_emitter.cpp)
target_compile_features(ink_backend_fixture_emitter PRIVATE cxx_std_17)
target_link_libraries(ink_backend_fixture_emitter PRIVATE ink::backend ink::execution ink::ir ink::target)

if(MSVC)
  target_compile_options(ink_backend_fixture_emitter PRIVATE /utf-8)
endif()

set(INK_BACKEND_FIXTURE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixture/$<CONFIG>")
set(INK_BACKEND_FIXTURE_OBJECT "${INK_BACKEND_FIXTURE_DIRECTORY}/ink_backend_fixture${CMAKE_CXX_OUTPUT_EXTENSION}")
add_custom_command(OUTPUT "${INK_BACKEND_FIXTURE_OBJECT}" COMMAND "${CMAKE_COMMAND}" -E make_directory "${INK_BACKEND_FIXTURE_DIRECTORY}" COMMAND "${CMAKE_COMMAND}" -E rm -f "${INK_BACKEND_FIXTURE_OBJECT}" COMMAND "$<TARGET_FILE:ink_backend_fixture_emitter>" "${INK_BACKEND_FIXTURE_OBJECT}" DEPENDS ink_backend_fixture_emitter VERBATIM)
set_source_files_properties("${INK_BACKEND_FIXTURE_OBJECT}" PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE)

add_executable(ink_backend_fixture_runner fixture_runner.cpp "${INK_BACKEND_FIXTURE_OBJECT}")
target_compile_features(ink_backend_fixture_runner PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(ink_backend_fixture_runner PRIVATE /utf-8)
endif()

add_test(NAME InkBackendAotDifferentialTest COMMAND "$<TARGET_FILE:ink_backend_fixture_runner>")
