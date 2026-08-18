#include "resolver.h"

#include "module_draft.h"

#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/model/function.h"

#include <cstddef>
#include <utility>

namespace ink::ir::text
{
  namespace
  {
    template <core::DiagnosticKind Kind, typename... ArgumentTypes>
    bool fail(core::Diagnostic &Error, const Token &Location, ArgumentTypes &&...Arguments)
    {
      Error = core::makeDiagnostic<Kind>(Location.Span, std::forward<ArgumentTypes>(Arguments)...);
      return false;
    }

    BlockTarget &blockTargetAt(const BranchFixup &Fixup)
    {
      switch (Fixup.TargetKind)
      {
      case BranchFixupTarget::Branch:
        return static_cast<BranchInstruction &>(*Fixup.InstructionValue).Target;
      case BranchFixupTarget::ConditionalTrue:
        return static_cast<ConditionalBranchInstruction &>(*Fixup.InstructionValue).TrueTarget;
      case BranchFixupTarget::ConditionalFalse:
        return static_cast<ConditionalBranchInstruction &>(*Fixup.InstructionValue).FalseTarget;
      }
      return static_cast<BranchInstruction &>(*Fixup.InstructionValue).Target;
    }

    std::optional<BlockId> findBlock(const Function &FunctionValue, const Name &BlockName)
    {
      for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
      {
        if (FunctionValue.Blocks[BlockIndex].Name == BlockName)
        {
          return BlockId{BlockIndex};
        }
      }
      return std::nullopt;
    }
  } // namespace

  bool resolveReferences(ModuleDraft &Draft, core::Diagnostic &Error)
  {
    for (const CallFixup &Fixup : Draft.CallFixups)
    {
      const auto Callee = Draft.FunctionNames.find(Fixup.CalleeName);
      if (Callee == Draft.FunctionNames.end())
      {
        return fail<core::DiagnosticKind::IrUnknownCallTarget>(Error, Fixup.Location, Fixup.CalleeName);
      }
      Fixup.InstructionValue->Callee = Callee->second;
    }
    for (const GlobalFixup &Fixup : Draft.GlobalFixups)
    {
      const auto Global = Draft.ByteConstantNames.find(Fixup.GlobalName);
      if (Global == Draft.ByteConstantNames.end())
      {
        return fail<core::DiagnosticKind::IrUnknownGlobalByteConstant>(Error, Fixup.Location, Fixup.GlobalName);
      }
      Fixup.Address->resolveByteConstant(Global->second);
    }
    for (const GlobalVariableFixup &Fixup : Draft.GlobalVariableFixups)
    {
      const auto Global = Draft.GlobalVariableNames.find(Fixup.GlobalName);
      if (Global == Draft.GlobalVariableNames.end())
      {
        return fail<core::DiagnosticKind::IrExpected>(Error, Fixup.Location, "a declared global variable");
      }
      Fixup.Address->resolveGlobal(Global->second);
    }
    for (const BranchFixup &Fixup : Draft.BranchFixups)
    {
      const Function *FunctionValue = Draft.Builder.function(Fixup.Function);
      const std::optional<BlockId> Target = FunctionValue == nullptr ? std::nullopt : findBlock(*FunctionValue, Fixup.BlockName);
      if (!Target.has_value())
      {
        return fail<core::DiagnosticKind::IrUnknownBasicBlockTarget>(Error, Fixup.Location, Fixup.BlockName);
      }
      blockTargetAt(Fixup).Block = *Target;
    }
    for (const PhiFixup &Fixup : Draft.PhiFixups)
    {
      const Function *FunctionValue = Draft.Builder.function(Fixup.Function);
      const std::optional<BlockId> Predecessor = FunctionValue == nullptr ? std::nullopt : findBlock(*FunctionValue, Fixup.BlockName);
      if (!Predecessor.has_value())
      {
        return fail<core::DiagnosticKind::IrUnknownBasicBlockTarget>(Error, Fixup.Location, Fixup.BlockName);
      }
      Fixup.InstructionValue->IncomingValues[Fixup.IncomingIndex].Predecessor = *Predecessor;
    }
    if (Draft.InitializerName.has_value())
    {
      const auto InitializerFunction = Draft.FunctionNames.find(*Draft.InitializerName);
      if (InitializerFunction == Draft.FunctionNames.end())
      {
        return fail<core::DiagnosticKind::IrUnknownCallTarget>(Error, Draft.InitializerLocation, *Draft.InitializerName);
      }
      Draft.Builder.setInitializer(InitializerFunction->second);
    }
    if (Draft.FinalizerName.has_value())
    {
      const auto FinalizerFunction = Draft.FunctionNames.find(*Draft.FinalizerName);
      if (FinalizerFunction == Draft.FunctionNames.end())
      {
        return fail<core::DiagnosticKind::IrUnknownCallTarget>(Error, Draft.FinalizerLocation, *Draft.FinalizerName);
      }
      Draft.Builder.setFinalizer(FinalizerFunction->second);
    }
    return true;
  }
} // namespace ink::ir::text
