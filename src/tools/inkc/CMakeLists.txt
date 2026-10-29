add_executable(inkc main.cpp)
target_compile_features(inkc PRIVATE cxx_std_23)
target_link_libraries(inkc PRIVATE ink::frontend cxxopts::cxxopts spdlog::spdlog)
target_compile_definitions(inkc PRIVATE CXXOPTS_NO_EXCEPTIONS CXXOPTS_NO_REGEX)
ink_disable_exceptions(inkc)

if(MSVC)
  target_compile_options(inkc PRIVATE /utf-8)
endif()
