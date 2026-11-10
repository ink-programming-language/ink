#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <array>
#include <cstdint>
#include <memory>

int main()
{
  spdlog::logger Output("vm-link-example-output", std::make_shared<spdlog::sinks::stdout_sink_mt>());
  Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
  spdlog::logger Errors("vm-link-example-errors", std::make_shared<spdlog::sinks::stderr_sink_mt>());
  Errors.set_pattern("%v");

  using namespace ink::vm;
  auto &Names = ink::core::NamePool::instance();
  const auto Scratch = Names.intern("scratch");
  const auto Counter = Names.intern("counter");
  const auto Helper = Names.intern("helper");
  const auto BufferA = Names.intern("buffer_a");
  const auto BufferB = Names.intern("buffer_b");
  const NativeDataImport HostBuffer{Names.intern("host_buffer")};
  MachineModule A(Names.intern("A"));
  MachineModule B(Names.intern("B"));
  const std::array<std::uint8_t, 8> FirstBytes{1};
  const std::array<std::uint8_t, 8> SecondBytes{2};
  const std::array<std::uint8_t, 8> CounterBytes{40};
  if (!A.createGlobalVariable(8, 8, FirstBytes, Scratch) || !B.createGlobalVariable(8, 8, CounterBytes, Counter, MachineSymbolLinkage::External) || !B.createGlobalVariable(8, 8, SecondBytes, Scratch))
  {
    Errors.error("Cannot create the example's global variables");
    return 1;
  }
  A.declareSymbol(Counter, MachineSymbolKind::GlobalVariable);
  A.declareSymbol(Helper, MachineSymbolKind::Function);
  A.declareNativeData(BufferA, HostBuffer);
  B.declareNativeData(BufferB, HostBuffer);

  MachineInstructionBuilder Builder;
  auto &Main = A.createFunction(Names.intern("main"), MachineSymbolLinkage::External);
  auto &Entry = Main.createBasicBlock(Names.intern("entry"));
  auto &Done = Main.createBasicBlock(Names.intern("done"));
  Builder.setInsertPoint(Entry);
  Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(Scratch));
  Builder.create<Opcode::Add64mmi>(AddressOperand(Counter), AddressOperand(Counter), ImmediateOperand(1));
  // This takes a VM function address; it does not call the function.
  Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 8), ImmediateOperand(Helper));
  Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 16), AddressOperand(BufferA, 8));
  Builder.create<Opcode::Jmp>(&Done);
  Builder.setInsertPoint(Done);
  Builder.create<Opcode::Ret>();

  auto &HelperEntry = B.createFunction(Helper, MachineSymbolLinkage::External).createBasicBlock(Names.intern("entry"));
  Builder.setInsertPoint(HelperEntry);
  Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(Scratch));
  Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 16), AddressOperand(BufferB, 8));
  Builder.create<Opcode::Ret>();

  auto AText = writeMachineModuleText(A);
  auto BText = writeMachineModuleText(B);
  if (!AText || !BText)
  {
    Errors.error("{}", ink::core::DiagnosticFormatter{}.format(!AText ? AText.error() : BText.error()).Message);
    return 1;
  }
  ink::core::CollectingDiagnosticConsumer Collector;
  ink::core::DiagnosticEngine Diagnostics;
  Diagnostics.addConsumer(Collector);
  const std::array<const MachineModule *, 2> Inputs{&A, &B};
  auto Linked = ModuleLinker::link(Inputs, Diagnostics, Names.intern("A_B"));
  if (!Linked)
  {
    for (const auto &Diagnostic : Collector.diagnostics())
    {
      Errors.error("{}", ink::core::DiagnosticFormatter{}.format(Diagnostic).Message);
    }
    return 1;
  }
  auto LinkedText = writeMachineModuleText(*Linked);
  if (!LinkedText)
  {
    Errors.error("{}", ink::core::DiagnosticFormatter{}.format(LinkedText.error()).Message);
    return 1;
  }
  Output.info("; === A ===\n{}\n; === B ===\n{}\n; === A_B ===\n{}", *AText, *BText, *LinkedText);
  return 0;
}
