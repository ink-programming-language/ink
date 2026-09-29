#ifndef INK_IR_FUNCTION_H
#define INK_IR_FUNCTION_H

#include "ink/ir/function/basic_block.h"
#include "ink/ir/function/function_type.h"
#include "ink/ir/function/function_parameter.h"
#include "ink/ir/name/name.h"

#include <vector>

namespace ink::ir
{
  // A closed callable identity. Generic definitions must be instantiated before creating this value.
  class Function final : public Value
  {
    public:
      Name name() const noexcept
      {
        return FunctionName;
      }

      const FunctionType &functionType() const noexcept
      {
        return static_cast<const FunctionType &>(type());
      }

      CallingConvention callingConvention() const noexcept
      {
        return Convention;
      }

      LanguageLinkage languageLinkage() const noexcept
      {
        return Linkage;
      }

      bool hasBody() const noexcept
      {
        return !Blocks.empty();
      }

      const std::vector<std::unique_ptr<FunctionParameter>> &parameters() const noexcept
      {
        return Parameters;
      }

      // The first block is the entry; declarations have no blocks and return null.
      BasicBlock *entryBlock() noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front().get();
      }

      const BasicBlock *entryBlock() const noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front().get();
      }

      // Owned blocks in insertion order, not execution order. IRBuilder creates and attaches blocks.
      const std::vector<std::unique_ptr<BasicBlock>> &blocks() const noexcept
      {
        return Blocks;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Function;
      }

    private:
      Function(Name FunctionName, const FunctionType &Signature, CallingConvention Convention, LanguageLinkage Linkage) noexcept
          : Value(Signature.context(), ValueKind::Function, Signature),
            FunctionName(FunctionName),
            Convention(Convention),
            Linkage(Linkage)
      {
      }

      Name FunctionName;
      CallingConvention Convention;
      LanguageLinkage Linkage;
      // Blocks are destroyed before the parameters referenced by their instructions.
      std::vector<std::unique_ptr<FunctionParameter>> Parameters;
      std::vector<std::unique_ptr<BasicBlock>> Blocks;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
