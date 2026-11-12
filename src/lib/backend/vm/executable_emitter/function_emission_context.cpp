#include "ink/backend/vm/executable_emitter/function_emission_context.h"

#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/module/machine_function.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/support/core_define.h"

namespace ink::vm
{
  void FunctionEmissionContext::reset() noexcept
  {
    UnResolvedBasicBlock.clear();
    ResolvedBasicBlock.clear();
    PendingStaticRelocationBasicBlock.clear();
  }

  void FunctionEmissionContext::scanUnResolvedBasicBlock(const MachineFunction &Function)
  {
    UnResolvedBasicBlock.clear();
    for (const MachineBasicBlock &Block : Function.basicBlocks())
    {
      UnResolvedBasicBlock.try_emplace(&Block, 0);
    }
  }

  void FunctionEmissionContext::resolveBasicBlock(const MachineBasicBlock &Block, std::uint64_t ImageOffset)
  {
    ResolvedBasicBlock.insert_or_assign(&Block, ImageOffset);
    UnResolvedBasicBlock.erase(&Block);
  }

  std::optional<std::uint64_t> FunctionEmissionContext::resolvedBasicBlockOffset(const MachineBasicBlock &Block) const noexcept
  {
    const auto Position = ResolvedBasicBlock.find(&Block);
    if (Position == ResolvedBasicBlock.end())
    {
      return std::nullopt;
    }
    return Position->second;
  }

  bool FunctionEmissionContext::hasUnResolvedBasicBlock(const MachineBasicBlock &Block) const noexcept
  {
    return UnResolvedBasicBlock.contains(&Block);
  }

  void FunctionEmissionContext::addBasicBlockStaticRelocation(StaticRelocationBasicBlock Relocation)
  {
    PendingStaticRelocationBasicBlock.push_back(Relocation);
  }

  void FunctionEmissionContext::resolveStaticRelocations(ExecutableEmissionContext &ExecutableContext)
  {
    const auto &Diagnostics = ExecutableContext.getDiagnostics();
    for (const auto &Relocation : PendingStaticRelocationBasicBlock)
    {
      const auto Position = ResolvedBasicBlock.find(Relocation.Block);
      REPORT_ICE_IF(Position == ResolvedBasicBlock.end(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "basic-block static relocation", "basic block has no image offset after function layout");
      const auto Bits = ExecutableContext.resolveRegisterRelativeOffset(RegisterId::PC, Position->second, 0, Relocation.InstructionOffset);
      ExecutableContext.patchInteger(Relocation.ImageOffset, Bits, 64);
    }
    PendingStaticRelocationBasicBlock.clear();
  }
} // namespace ink::vm
