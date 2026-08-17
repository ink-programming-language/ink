#ifndef INK_IR_MODULE_H
#define INK_IR_MODULE_H

#include "ink/ir/model/function.h"
#include "ink/ir/model/global_variable.h"
#include "ink/ir/model/name.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::ir
{
  class IRContext;

  struct ByteConstant
  {
      ink::ir::Name Name;
      std::string Data;
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

      std::optional<FunctionId> findFunction(const ink::ir::Name &SymbolName) const noexcept;
      std::optional<GlobalId> findGlobal(const ink::ir::Name &SymbolName) const noexcept;

      // Empty only for anonymous standalone IR; serialized package modules use their canonical name.
      std::optional<ink::ir::Name> Name;
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
