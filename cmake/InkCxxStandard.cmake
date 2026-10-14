include(CheckCXXSourceCompiles)

function(ink_check_cxx23_support)
  # Keep the probe's language mode local so bundled third-party targets retain their own requirements.
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  check_cxx_source_compiles([=[
#include <type_traits>
#include <utility>

struct Context
{
    int Value = 0;

    auto &value(this auto &&Self) noexcept
    {
      return std::forward_like<decltype(Self) &>(Self.Value);
    }
};

static_assert(std::is_same_v<decltype(std::declval<Context &>().value()), int &>);
static_assert(std::is_same_v<decltype(std::declval<const Context &>().value()), const int &>);
static_assert(std::is_same_v<decltype(std::declval<Context &&>().value()), int &>);
static_assert(std::is_same_v<decltype(std::declval<const Context &&>().value()), const int &>);

int main()
{
  return 0;
}
]=] INK_HAS_CXX23_ACCESSORS)
  if(NOT INK_HAS_CXX23_ACCESSORS)
    message(FATAL_ERROR "Ink requires C++23 explicit object parameters and std::forward_like; select a compiler and standard library that support both features.")
  endif()
endfunction()
