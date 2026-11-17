add_executable(ink_nbody_benchmark ink_nbody_benchmark.cpp)
target_compile_features(ink_nbody_benchmark PRIVATE cxx_std_23)
target_link_libraries(ink_nbody_benchmark PRIVATE ink::vm spdlog::spdlog)
ink_disable_exceptions(ink_nbody_benchmark)

add_executable(cpp_nbody_benchmark nbody.cpp)

target_compile_features(cpp_nbody_benchmark PRIVATE cxx_std_23)

target_link_libraries(cpp_nbody_benchmark PRIVATE spdlog::spdlog)
ink_disable_exceptions(cpp_nbody_benchmark)

if(MSVC)
    target_compile_options(ink_nbody_benchmark PRIVATE /utf-8)
    target_compile_options(cpp_nbody_benchmark PRIVATE /utf-8)
endif()
