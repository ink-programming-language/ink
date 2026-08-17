#ifndef INK_IR_MODULE_H
#define INK_IR_MODULE_H

#include "ink/ir/function.h"

#include <optional>
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

  struct GlobalVariable
  {
    std::string Name;
    const Type *ValueType = nullptr;
    bool Mutable = true;
  };

  class Module
  {
  public:
    explicit Module(IRContext &Context) noexcept
        : Context(&Context)
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

    ModuleId Id;
    std::vector<ByteConstant> ByteConstants;
    std::vector<GlobalVariable> Globals;
    std::vector<const StructType *> StructTypes;
    std::vector<Function> Functions;
    std::optional<FunctionId> Initializer;
    std::optional<FunctionId> Finalizer;

  private:
    IRContext *Context;
  };
} // namespace ink::ir

#endif
