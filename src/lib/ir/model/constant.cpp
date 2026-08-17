#include "ink/ir/model/constant.h"

namespace ink::ir
{
  const char *floatFormatName(FloatFormat Format) noexcept
  {
    switch (Format)
    {
    case FloatFormat::F16:
      return "f16";
    case FloatFormat::F32:
      return "f32";
    case FloatFormat::F64:
      return "f64";
    }
    return "unknown";
  }

  std::size_t floatFormatBitWidth(FloatFormat Format) noexcept
  {
    switch (Format)
    {
    case FloatFormat::F16:
      return 16;
    case FloatFormat::F32:
      return 32;
    case FloatFormat::F64:
      return 64;
    }
    return 0;
  }
} // namespace ink::ir
