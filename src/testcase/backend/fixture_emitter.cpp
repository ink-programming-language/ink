#include "ink/backend/backend.h"
#include "ink/execution/interpreter.h"
#include "ink/execution/runtime_world.h"
#include "ink/ir/ir.h"
#include "ink/target/target_context.h"

#include <filesystem>
#include <iostream>
#include <string>

namespace
{
  int fail(const std::string &Message)
  {
    std::cerr << Message << '\n';
    return 1;
  }
}

int main(int ArgumentCount, char **Arguments)
{
  if (ArgumentCount != 2)
  {
    return fail("usage: ink-backend-fixture-emitter <output-object>");
  }
  ink::ir::IrBuilder Builder;
  const ink::ir::IrTypeId I32 = Builder.integerType(32, ink::ir::IrSignedness::Signed);
  const ink::ir::IrTypeId HelperSignature = Builder.functionType({I32}, I32);
  const ink::ir::IrFunctionId Helper = Builder.addFunction("ink_fixture_double", HelperSignature, ink::ir::IrFunctionKind::Definition);
  const ink::ir::IrBuiltBlock HelperEntry = Builder.addBlock(Helper, {I32});
  const ink::ir::IrValueId Doubled = Builder.createIntegerBinary(HelperEntry.Block, ink::ir::IrOpcode::IntAdd, HelperEntry.Arguments[0], HelperEntry.Arguments[0]);
  Builder.createReturn(HelperEntry.Block, Doubled);
  const ink::ir::IrTypeId EntrySignature = Builder.functionType({}, I32);
  const ink::ir::IrFunctionId Entry = Builder.addFunction("ink_fixture_entry", EntrySignature, ink::ir::IrFunctionKind::Definition);
  const ink::ir::IrBuiltBlock EntryBlock = Builder.addBlock(Entry);
  const ink::ir::IrBuiltBlock TrueBlock = Builder.addBlock(Entry, {I32});
  const ink::ir::IrBuiltBlock FalseBlock = Builder.addBlock(Entry, {I32});
  const ink::ir::IrBuiltBlock MergeBlock = Builder.addBlock(Entry, {I32});
  const ink::ir::IrConstantId TwentyConstant = Builder.integerConstant(I32, 20);
  const ink::ir::IrConstantId TwoConstant = Builder.integerConstant(I32, 2);
  const ink::ir::IrConstantId ZeroConstant = Builder.integerConstant(I32, 0);
  const ink::ir::IrValueId Twenty = Builder.createIntegerConstant(EntryBlock.Block, TwentyConstant);
  const ink::ir::IrValueId Two = Builder.createIntegerConstant(EntryBlock.Block, TwoConstant);
  const ink::ir::IrValueId Zero = Builder.createIntegerConstant(EntryBlock.Block, ZeroConstant);
  const ink::ir::IrBuiltOperation Call = Builder.createDirectCall(EntryBlock.Block, Helper, {Twenty});
  const ink::ir::IrValueId Place = Builder.createAlloca(EntryBlock.Block, I32, ink::ir::IrPlaceAccess::ReadWrite);
  Builder.createStore(EntryBlock.Block, Place, Call.Results[0]);
  const ink::ir::IrValueId Loaded = Builder.createLoad(EntryBlock.Block, Place);
  const ink::ir::IrValueId Answer = Builder.createIntegerBinary(EntryBlock.Block, ink::ir::IrOpcode::IntAdd, Loaded, Two);
  const ink::ir::IrValueId IsAnswer = Builder.createIntegerCompare(EntryBlock.Block, ink::ir::IrComparePredicate::Equal, Answer, Answer);
  Builder.createConditionalBranch(EntryBlock.Block, IsAnswer, TrueBlock.Block, {Answer}, FalseBlock.Block, {Zero});
  Builder.createBranch(TrueBlock.Block, MergeBlock.Block, {TrueBlock.Arguments[0]});
  Builder.createBranch(FalseBlock.Block, MergeBlock.Block, {FalseBlock.Arguments[0]});
  Builder.createReturn(MergeBlock.Block, MergeBlock.Arguments[0]);
  ink::ir::IrStagedVerificationResult Staged = ink::ir::verifyStaged(Builder.finish());
  if (!Staged.succeeded())
  {
    return fail("fixture InkIR failed staged verification");
  }
  const ink::target::TargetContext Host = ink::target::TargetContext::host();
  ink::ir::IrClosedVerificationResult Closed = ink::ir::closeAndVerify(Staged.takeVerified(), Host.key());
  if (!Closed.succeeded())
  {
    return fail("fixture InkIR failed closed verification");
  }
  ink::execution::RuntimeWorld World(Closed.verified().targetKey());
  const ink::execution::ExecutionResult Interpreted = ink::execution::interpret(Closed.verified(), Entry, World, {});
  if (!Interpreted.returned() || !Interpreted.Value || Interpreted.Value->type() != I32 || Interpreted.Value->bits() != 42)
  {
    return fail("fixture RuntimeWorld result did not equal i32 42");
  }
  const ink::backend::BackendResult<void> Emitted = ink::backend::emitObject(Closed.verified(), std::filesystem::u8path(Arguments[1]));
  if (!Emitted.succeeded())
  {
    return fail(std::string(ink::backend::backendErrorCodeName(Emitted.error().Code)) + ": " + Emitted.error().Message);
  }
  return 0;
}
