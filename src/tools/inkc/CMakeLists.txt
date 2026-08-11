add_executable(inkc main.cpp)
target_compile_features(inkc PRIVATE cxx_std_17)
target_link_libraries(inkc PRIVATE ink::cli)

if(MSVC)
  target_compile_options(inkc PRIVATE /utf-8)
endif()
