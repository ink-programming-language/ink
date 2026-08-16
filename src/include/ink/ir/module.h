#ifndef INK_IR_MODULE_H
#define INK_IR_MODULE_H

#include "ink/ir/function.h"

#include <string>
#include <vector>

namespace ink::ir
{
  class IRContext;

  struct ByteConstant
  {
    std::string Name;
    std::string Data;
  };

  class Module
  {
  public:
    explicit Module(IRContext &Context) noexcept : Context(&Context)
    {
    }

    IRContext &context() noexcept
    {
      return *Context;
    }

    IRContext &context() const noexcept
    {
      return *Context;
    }

    std::vector<ByteConstant> ByteConstants;
    std::vector<const StructType *> StructTypes;
    std::vector<Function> Functions;

  private:
    IRContext *Context;
  };
} // namespace ink::ir

#endif
