add_executable(ink_fib_benchmark ink_fib_benchmark.cpp)
target_compile_features(ink_fib_benchmark PRIVATE cxx_std_23)
target_link_libraries(ink_fib_benchmark PRIVATE ink::vm spdlog::spdlog)
ink_disable_exceptions(ink_fib_benchmark)

add_executable(cpp_fib_benchmark fib.cpp)
target_compile_features(cpp_fib_benchmark PRIVATE cxx_std_23)
target_link_libraries(cpp_fib_benchmark PRIVATE spdlog::spdlog)
ink_disable_exceptions(cpp_fib_benchmark)

if(MSVC)
    target_compile_options(ink_fib_benchmark PRIVATE /utf-8)
    target_compile_options(cpp_fib_benchmark PRIVATE /utf-8)
endif()
