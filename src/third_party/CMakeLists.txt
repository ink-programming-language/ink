if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/llvm/llvm/CMakeLists.txt")
  message(FATAL_ERROR "LLVM is not initialized. Run: git submodule update --init src/third_party/llvm")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/spdlog/CMakeLists.txt")
  message(FATAL_ERROR "spdlog is not initialized. Run: git submodule update --init src/third_party/spdlog")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/utf8proc/CMakeLists.txt")
  message(FATAL_ERROR "utf8proc is not initialized. Run: git submodule update --init src/third_party/utf8proc")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libffi/include/ffi.h.in")
  message(FATAL_ERROR "libffi is not initialized. Run: git submodule update --init src/third_party/libffi")
endif()

add_subdirectory(llvm/llvm "${CMAKE_BINARY_DIR}/third_party/llvm" EXCLUDE_FROM_ALL)
add_subdirectory(spdlog "${CMAKE_BINARY_DIR}/third_party/spdlog" EXCLUDE_FROM_ALL)
add_subdirectory(utf8proc "${CMAKE_BINARY_DIR}/third_party/utf8proc" EXCLUDE_FROM_ALL)
include("${CMAKE_CURRENT_SOURCE_DIR}/libffi.cmake")

if(NOT TARGET utf8proc::utf8proc)
  add_library(utf8proc::utf8proc ALIAS utf8proc)
endif()
