#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/text/module_text.h"

#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <array>
#include <memory>

int main()
{
  spdlog::logger Output("ir-example-output", std::make_shared<spdlog::sinks::stdout_sink_mt>());
  Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
  spdlog::logger Errors("ir-example-errors", std::make_shared<spdlog::sinks::stderr_sink_mt>());
  Errors.set_pattern("%v");

  using namespace ink::ir;
  ink::core::NamePool &Names = ink::core::NamePool::instance();
  TypePool Types;
  ConstantPool Constants(Types);
  Module ModuleValue(Names.intern("RecordsAndMethods"));
  const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
  const IntegerType &UInt64 = Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned);
  StructType &Point = Types.createStructType();
  ObjectType &Counter = Types.createObjectType();
  ModuleValue.addStructType(Point);
  ModuleValue.addObjectType(Counter);

  Point.addField(Names.intern("X"), Int32);
  Point.addField(Names.intern("Y"), Int32);
  Counter.addField(Names.intern("Value"), Int32, MemberKind::Instance, SymbolVisibility::Private);
  Counter.addField(Names.intern("Origin"), Point, MemberKind::Instance, SymbolVisibility::Public);
  Counter.addField(Names.intern("Next"), Counter, MemberKind::Instance, SymbolVisibility::Private);
  Counter.addField(Names.intern("LiveCount"), UInt64, MemberKind::Static, SymbolVisibility::Private);

  InstructionBuilder Builder(Types);
  Function &Sum = Point.addMethod(Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Int32)}), Names.intern("sum"), MemberKind::Static, SymbolVisibility::Public).function();
  Sum.argument(0).setName(Names.intern("x"));
  Sum.argument(1).setName(Names.intern("y"));
  Builder.setInsertPoint(Sum.createBasicBlock(Names.intern("entry")));
  AddInstruction &Total = Builder.createAdd(Sum.argument(0), Sum.argument(1));
  static_cast<void>(Builder.createReturn(Total));

  // Instance membership does not inject an argument: this example declares its reference-semantic self explicitly.
  Function &ApplyDelta = Counter.addMethod(Types.getFunctionType(Int32, std::array{Parameter(Counter), Parameter(Types.getBoolType()), Parameter(Int32), Parameter(Int32, Parameter::Kind::Named)}), Names.intern("applyDelta"), MemberKind::Instance, SymbolVisibility::Public).function();
  ApplyDelta.argument(0).setName(Names.intern("self"));
  ApplyDelta.argument(1).setName(Names.intern("enabled"));
  ApplyDelta.argument(2).setName(Names.intern("current"));
  ApplyDelta.argument(3).setName(Names.intern("delta"));
  BasicBlock &Entry = ApplyDelta.createBasicBlock(Names.intern("entry"));
  BasicBlock &Apply = ApplyDelta.createBasicBlock(Names.intern("apply"));
  BasicBlock &Merge = ApplyDelta.createBasicBlock(Names.intern("merge"));
  Builder.setInsertPoint(Entry);
  static_cast<void>(Builder.createCondBranch(ApplyDelta.argument(1), Apply, Merge));
  Builder.setInsertPoint(Apply);
  AddInstruction &Updated = Builder.createAdd(ApplyDelta.argument(2), ApplyDelta.argument(3));
  static_cast<void>(Builder.createBranch(Merge));
  Builder.setInsertPoint(Merge);
  PhiInstruction &Selected = Builder.createPhi(Int32);
  Selected.addIncoming(ApplyDelta.argument(2), Entry);
  Selected.addIncoming(Updated, Apply);
  static_cast<void>(Builder.createReturn(Selected));

  Function &Scratch = Counter.addMethod(Types.getFunctionType(Int32), Names.intern("scratch"), MemberKind::Static, SymbolVisibility::Private).function();
  Builder.setInsertPoint(Scratch.createBasicBlock(Names.intern("entry")));
  AllocaInstruction &Slot = Builder.createAlloca(Int32);
  const IntegerConstant *FortyTwo = Constants.getSignedInteger(Int32, 42);
  if (!FortyTwo)
  {
    Errors.error("Cannot create the example's i32 constant");
    return 1;
  }
  static_cast<void>(Builder.createStore(Slot, *FortyTwo));
  LoadInstruction &Loaded = Builder.createLoad(Slot);
  static_cast<void>(Builder.createReturn(Loaded));
  ModuleValue.createGlobalVariable(Types.getPointerType(UInt64), Names.intern("LiveCounters"), SymbolVisibility::Public);

  auto Text = writeModuleText(ModuleValue);
  if (!Text)
  {
    Errors.error("{}", ink::core::DiagnosticFormatter{}.format(Text.error()).Message);
    return 1;
  }
  auto Parsed = readModuleText(*Text);
  if (!Parsed)
  {
    Errors.error("{}", ink::core::DiagnosticFormatter{}.format(Parsed.error()).Message);
    return 1;
  }
  auto Rewritten = writeModuleText((*Parsed)->module());
  if (!Rewritten)
  {
    Errors.error("{}", ink::core::DiagnosticFormatter{}.format(Rewritten.error()).Message);
    return 1;
  }
  if (*Rewritten != *Text)
  {
    Errors.error("The example's canonical IR text did not survive a round trip");
    return 1;
  }
  Output.info("{}", *Text);
  return 0;
}
