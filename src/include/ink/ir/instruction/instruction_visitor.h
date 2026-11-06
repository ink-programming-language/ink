#pragma once

#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/phi_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"

#include <cstdlib>
#include <type_traits>

namespace ink::ir
{
  // Dispatches only; the derived visitor must handle every concrete instruction explicitly.
  template <typename Derived, typename Result = void>
  class InstructionVisitor
  {
    public:
      Result visit(Instruction &Value)
      {
        return visitImpl(Value);
      }

      Result visit(const Instruction &Value)
      {
        return visitImpl(Value);
      }

    private:
      template <typename Node>
      Result visitImpl(Node &Value)
      {
        switch (Value.getKind())
        {
#define INSTRUCTION(Kind, Number, Class, Spelling) case InstructionKind::Kind: return static_cast<Derived &>(*this).visit##Class(static_cast<std::conditional_t<std::is_const_v<Node>, const Class, Class> &>(Value));
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
        }
        std::abort();
      }
  };
} // namespace ink::ir
