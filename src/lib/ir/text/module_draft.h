#ifndef INK_LIB_IR_TEXT_MODULE_DRAFT_H
#define INK_LIB_IR_TEXT_MODULE_DRAFT_H

#include "ink/ir/builder.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/model/name.h"
#include "token.h"

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::ir::text
{
  struct CallFixup
  {
      CallInstruction *InstructionValue = nullptr;
      Name CalleeName;
      Token Location;
  };

  enum class BranchFixupTarget
  {
    Branch,
    ConditionalTrue,
    ConditionalFalse,
  };

  struct BranchFixup
  {
      FunctionId Function;
      Instruction *InstructionValue = nullptr;
      BranchFixupTarget TargetKind = BranchFixupTarget::Branch;
      Name BlockName;
      Token Location;
  };

  struct PhiFixup
  {
      FunctionId Function;
      PhiInstruction *InstructionValue = nullptr;
      std::size_t IncomingIndex = 0;
      Name BlockName;
      Token Location;
  };

  struct GlobalFixup
  {
      GlobalAddressOperand *Address = nullptr;
      Name GlobalName;
      Token Location;
  };

  struct GlobalVariableFixup
  {
      GlobalVariableAddressOperand *Address = nullptr;
      Name GlobalName;
      Token Location;
  };

  class ModuleDraft final
  {
    public:
      explicit ModuleDraft(IRContext &Context) noexcept
          : Builder(Context)
      {
      }

      IRBuilder Builder;
      std::unordered_set<Name> GlobalSymbolNames;
      std::unordered_map<Name, const StructType *> TypeNames;
      std::unordered_map<Name, ByteConstantId> ByteConstantNames;
      std::unordered_map<Name, GlobalId> GlobalVariableNames;
      std::unordered_map<Name, FunctionId> FunctionNames;
      std::vector<CallFixup> CallFixups;
      std::vector<BranchFixup> BranchFixups;
      std::vector<PhiFixup> PhiFixups;
      std::vector<GlobalFixup> GlobalFixups;
      std::vector<GlobalVariableFixup> GlobalVariableFixups;
      bool HasModuleDeclaration = false;
      std::optional<Name> InitializerName;
      std::optional<Name> FinalizerName;
      Token InitializerLocation;
      Token FinalizerLocation;
  };
} // namespace ink::ir::text

#endif
