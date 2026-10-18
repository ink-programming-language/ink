#include "ink/semantic/analyzer/analyzer.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    class SemanticVariableTest : public testing::Test
    {
      protected:
        SemanticVariableTest()
            : Frontend(Sources, DiagnosticEngine),
              Context(Sources, DiagnosticEngine, target::TargetContext::native()),
              Output(Context.irContext())
        {
          DiagnosticEngine.addConsumer(Diagnostics);
        }

        frontend::parser::ParseResult read(std::string_view Source)
        {
          Diagnostics.clear();
          return frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, std::string(Source)));
        }

        bool analyze(const frontend::parser::ParseResult &Input)
        {
          return Analysis.analyze(Context, Input, Output);
        }

        static core::Name name(std::string_view Text)
        {
          return core::NamePool::instance().intern(Text);
        }

        ir::Function *function(std::string_view Text)
        {
          const Binding *Found = NameResolver(Context).lookupMember(Output, name(Text));
          return Found && Found->targets().size() == 1 && ir::Function::classof(Found->targets()[0]) ? static_cast<ir::Function *>(Found->targets()[0]) : nullptr;
        }

        template <typename InstructionType>
        static std::vector<const InstructionType *> instructions(const ir::Function &Function)
        {
          std::vector<const InstructionType *> Result;
          for (const auto &Block : Function.blocks())
          {
            for (const auto &Instruction : Block->instructions())
            {
              if (InstructionType::classof(Instruction.get()))
              {
                Result.push_back(static_cast<const InstructionType *>(Instruction.get()));
              }
            }
          }
          return Result;
        }

        static const ir::ReturnInstruction *returned(const ir::Function &Function)
        {
          const auto *Block = Function.entryBlock();
          const auto *Last = Block && Block->instructionCount() ? Block->instruction(Block->instructionCount() - 1) : nullptr;
          return ir::ReturnInstruction::classof(Last) ? static_cast<const ir::ReturnInstruction *>(Last) : nullptr;
        }

        void expectDiagnostic(core::DiagnosticKind Kind)
        {
          ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
          EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Kind);
        }

        void TearDown() override
        {
          auto &Scopes = Context.scopeStore();
          for (const auto &Function : Output.globalFunctions())
          {
            for (const auto &Block : Function->blocks())
            {
              for (const auto &Instruction : Block->instructions())
              {
                Scopes.removeTarget(*Instruction);
              }
            }
            for (const auto &Parameter : Function->parameters())
            {
              Scopes.removeTarget(*Parameter);
            }
            Scopes.removeTarget(*Function);
          }
          Scopes.removeTarget(Output);
        }

        core::SourceManager Sources;
        core::CollectingDiagnosticConsumer Diagnostics;
        core::DiagnosticEngine DiagnosticEngine;
        frontend::FrontendContext Frontend;
        SemanticContext Context;
        ir::Module Output;
        Analyzer Analysis;
    };

    class SemanticVariableDeathTest : public SemanticVariableTest
    {
    };
  } // namespace

  // Unannotated local literals select the ordinary default types before storage and return loads are produced.
  TEST_F(SemanticVariableTest, InfersDefaultLiteralTypes)
  {
    auto Input = read("func integer(): i32 { var Value = 42; return Value; } func floating(): f64 { var Value = 1.25; return Value; } func boolean(): bool { var Value = true; return Value; } func character(): u32 { var Value = 'A'; return Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    struct Case
    {
        const char *Name;
        const ir::Type *Type;
    };
    const Case Cases[] = {
        {"integer", Context.typePool().getInt32Type()},
        {"floating", Context.typePool().getFloat64Type()},
        {"boolean", Context.typePool().getBoolType()},
        {"character", Context.typePool().getUInt32Type()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      const auto *Function = function(Entry.Name);
      ASSERT_NE(Function, nullptr);
      const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
      const auto Stores = instructions<ir::StoreInstruction>(*Function);
      const auto Loads = instructions<ir::LoadInstruction>(*Function);
      ASSERT_EQ(Allocations.size(), 1U);
      ASSERT_EQ(Stores.size(), 1U);
      ASSERT_EQ(Loads.size(), 1U);
      EXPECT_EQ(Allocations[0]->allocatedType(), Entry.Type);
      EXPECT_EQ(Stores[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Stores[0]->storedValue()->type(), Entry.Type);
      EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Loads[0]->type(), Entry.Type);
      ASSERT_NE(returned(*Function), nullptr);
      EXPECT_EQ(returned(*Function)->returnedValue(), Loads[0]);
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Explicit variable types constrain literal materialization, numeric negation and floating precision.
  TEST_F(SemanticVariableTest, AppliesDeclaredTypesToInitializers)
  {
    auto Input = read("func byte(): u8 { var Value: u8 = 255; return Value; } func negative(): i8 { var Value: i8 = -127; return Value; } func floating(): f32 { var Value: f32 = 1.25; return Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    struct Case
    {
        const char *Name;
        const ir::Type *Type;
    };
    const Case Cases[] = {
        {"byte", Context.typePool().getUInt8Type()},
        {"negative", Context.typePool().getInt8Type()},
        {"floating", Context.typePool().getFloat32Type()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      const auto *Function = function(Entry.Name);
      ASSERT_NE(Function, nullptr);
      const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
      const auto Stores = instructions<ir::StoreInstruction>(*Function);
      ASSERT_EQ(Allocations.size(), 1U);
      ASSERT_EQ(Stores.size(), 1U);
      EXPECT_EQ(Stores[0]->storedValue()->type(), Entry.Type);
      EXPECT_EQ(Allocations[0]->allocatedType(), Entry.Type);
    }
    const auto ByteStores = instructions<ir::StoreInstruction>(*function("byte"));
    ASSERT_TRUE(ir::IntegerConstant::classof(ByteStores[0]->storedValue()));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(ByteStores[0]->storedValue())->bits(), 255U);
    const auto NegativeStores = instructions<ir::StoreInstruction>(*function("negative"));
    ASSERT_TRUE(ir::NegInstruction::classof(NegativeStores[0]->storedValue()));
    const auto *Negation = static_cast<const ir::NegInstruction *>(NegativeStores[0]->storedValue());
    ASSERT_TRUE(ir::IntegerConstant::classof(Negation->operand()));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Negation->operand())->bits(), 127U);
  }

  // A second local initialized from the first receives its loaded value rather than the first allocation address.
  TEST_F(SemanticVariableTest, CopiesRuntimeValuesBetweenLocals)
  {
    auto Input = read("func copy(Input: i32): i32 { var First = Input; var Second = (First); return ((Second)); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("copy");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 2U);
    ASSERT_EQ(Stores.size(), 2U);
    ASSERT_EQ(Loads.size(), 2U);
    EXPECT_EQ(Stores[0]->storedValue(), Function->parameter(0));
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_EQ(Stores[1]->storedValue(), Loads[0]);
    EXPECT_EQ(Stores[1]->pointer(), Allocations[1]);
    EXPECT_EQ(Loads[1]->pointer(), Allocations[1]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[1]);
  }

  // A typed initializer compares another local's declared element type and accepts a matching u8 storage read.
  TEST_F(SemanticVariableTest, InitializesTypedLocalFromMatchingLocal)
  {
    auto Input = read("func copy(): u8 { var First: u8 = 1; var Second: u8 = First; return Second; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("copy");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 2U);
    ASSERT_EQ(Stores.size(), 2U);
    ASSERT_EQ(Loads.size(), 2U);
    EXPECT_EQ(Allocations[0]->allocatedType(), Context.typePool().getUInt8Type());
    EXPECT_EQ(Allocations[1]->allocatedType(), Context.typePool().getUInt8Type());
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_EQ(Stores[1]->storedValue(), Loads[0]);
    EXPECT_EQ(Stores[1]->pointer(), Allocations[1]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[1]);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A typed initializer rejects a different local element type before allocating the destination variable.
  TEST_F(SemanticVariableTest, RejectsTypedInitializerFromMismatchedLocal)
  {
    auto Input = read("func mismatch(): void { var First: u8 = 1; var Wrong: i32 = First; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    expectDiagnostic(core::DiagnosticKind::SemanticTypeMismatch);
    const auto *Function = function("mismatch");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    EXPECT_EQ(Allocations[0]->allocatedType(), Context.typePool().getUInt8Type());
    EXPECT_EQ(instructions<ir::StoreInstruction>(*Function).size(), 1U);
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
  }

  // Call arguments and numeric negation each load local storage before consuming its declared value type.
  TEST_F(SemanticVariableTest, LoadsCallArgumentsAndUnaryOperands)
  {
    auto Input = read("func identity(Value: i32): i32 { return Value; } func use(Input: i32): i32 { var Copy = Input; identity(Copy); return -Copy; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("use");
    ASSERT_NE(Function, nullptr);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    const auto Negations = instructions<ir::NegInstruction>(*Function);
    ASSERT_EQ(Loads.size(), 2U);
    ASSERT_EQ(Calls.size(), 1U);
    ASSERT_EQ(Negations.size(), 1U);
    ASSERT_NE(Calls[0]->argument(0), nullptr);
    EXPECT_EQ(Calls[0]->argument(0)->value(), Loads[0]);
    EXPECT_EQ(Negations[0]->operand(), Loads[1]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Negations[0]);
  }

  // Const accepts a runtime call initializer once and serves later reads from the initialized local storage.
  TEST_F(SemanticVariableTest, InitializesConstFromOneRuntimeCall)
  {
    auto Input = read("func identity(Value: i32): i32 { return Value; } func use(Input: i32): i32 { const Copy = identity(Input); identity(Copy); return Copy; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("use");
    ASSERT_NE(Function, nullptr);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(instructions<ir::AllocaInstruction>(*Function).size(), 1U);
    ASSERT_EQ(Calls.size(), 2U);
    ASSERT_EQ(Stores.size(), 1U);
    ASSERT_EQ(Loads.size(), 2U);
    ASSERT_NE(Calls[0]->argument(0), nullptr);
    ASSERT_NE(Calls[1]->argument(0), nullptr);
    EXPECT_EQ(Calls[0]->argument(0)->value(), Function->parameter(0));
    EXPECT_EQ(Stores[0]->storedValue(), Calls[0]);
    EXPECT_EQ(Calls[1]->argument(0)->value(), Loads[0]);
    EXPECT_EQ(Loads[0]->pointer(), Stores[0]->pointer());
    EXPECT_EQ(Loads[1]->pointer(), Stores[0]->pointer());
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[1]);
  }

  // Declared pointer types survive null initialization, and opaque pointer values remain valid local contents.
  TEST_F(SemanticVariableTest, PreservesPointerTypesInLocalStorage)
  {
    auto Input = read("func nullPointer(): *i32 { const Pointer: *i32 = nullptr; var Copy = (Pointer); return Copy; } func opaque(Input: *void): *void { var Pointer: *void = Input; return Pointer; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *NullFunction = function("nullPointer");
    const auto *OpaqueFunction = function("opaque");
    ASSERT_NE(NullFunction, nullptr);
    ASSERT_NE(OpaqueFunction, nullptr);
    const auto NullAllocations = instructions<ir::AllocaInstruction>(*NullFunction);
    const auto NullStores = instructions<ir::StoreInstruction>(*NullFunction);
    const auto NullLoads = instructions<ir::LoadInstruction>(*NullFunction);
    ASSERT_EQ(NullAllocations.size(), 2U);
    ASSERT_EQ(NullStores.size(), 2U);
    ASSERT_EQ(NullLoads.size(), 2U);
    const auto *PointerType = Context.typePool().getPointerType(Context.typePool().getInt32Type());
    EXPECT_EQ(NullAllocations[0]->allocatedType(), PointerType);
    EXPECT_EQ(NullAllocations[1]->allocatedType(), PointerType);
    EXPECT_TRUE(ir::NullPointerConstant::classof(NullStores[0]->storedValue()));
    EXPECT_EQ(NullLoads[0]->type(), PointerType);
    EXPECT_EQ(NullStores[1]->storedValue(), NullLoads[0]);
    EXPECT_EQ(NullLoads[1]->type(), PointerType);
    const auto OpaqueStores = instructions<ir::StoreInstruction>(*OpaqueFunction);
    const auto OpaqueLoads = instructions<ir::LoadInstruction>(*OpaqueFunction);
    ASSERT_EQ(OpaqueStores.size(), 1U);
    ASSERT_EQ(OpaqueLoads.size(), 1U);
    EXPECT_EQ(OpaqueStores[0]->storedValue(), OpaqueFunction->parameter(0));
    EXPECT_EQ(OpaqueLoads[0]->type(), OpaqueFunction->parameter(0)->type());
  }

  // A parenthesized local callable is loaded before an indirect call, and a const argument is loaded independently.
  TEST_F(SemanticVariableTest, CallsFunctionStoredInLocalVariable)
  {
    auto Input = read("func identity(Value: i32): i32 { return Value; } func use(Input: i32): i32 { var Selected = identity; const Copy = (Input); return ((Selected))((Copy)); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("use");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 2U);
    ASSERT_EQ(Stores.size(), 2U);
    ASSERT_EQ(Loads.size(), 2U);
    ASSERT_EQ(Calls.size(), 1U);
    EXPECT_EQ(Stores[0]->storedValue(), function("identity"));
    EXPECT_EQ(Stores[1]->storedValue(), Function->parameter(0));
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_EQ(Loads[1]->pointer(), Allocations[1]);
    EXPECT_EQ(Calls[0]->callee(), Loads[0]);
    EXPECT_EQ(Calls[0]->directCallee(), nullptr);
    ASSERT_NE(Calls[0]->argument(0), nullptr);
    EXPECT_EQ(Calls[0]->argument(0)->value(), Loads[1]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Calls[0]);
  }

  // A typed local may remain unused without a fabricated zero initializer or a spurious uninitialized-read error.
  TEST_F(SemanticVariableTest, AllowsUnusedUninitializedTypedLocal)
  {
    auto Input = read("func unused(): void { var Value: i32; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("unused");
    ASSERT_NE(Function, nullptr);
    ASSERT_EQ(instructions<ir::AllocaInstruction>(*Function).size(), 1U);
    EXPECT_TRUE(instructions<ir::StoreInstruction>(*Function).empty());
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Reading an uninitialized local reports the source variable rather than creating a load from its allocation.
  TEST_F(SemanticVariableTest, RejectsUninitializedLocalRead)
  {
    auto Input = read("func read(): i32 { var Value: i32; return Value; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    expectDiagnostic(core::DiagnosticKind::SemanticUninitializedRead);
    const auto *Function = function("read");
    ASSERT_NE(Function, nullptr);
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
  }

  // Missing type information produces the dedicated diagnostic and does not allocate storage for an unknown type.
  TEST_F(SemanticVariableTest, RejectsMissingTypeAndInitializer)
  {
    auto Input = read("func missing(): void { var Value; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    expectDiagnostic(core::DiagnosticKind::SemanticMissingVariableType);
    const auto *Function = function("missing");
    ASSERT_NE(Function, nullptr);
    EXPECT_TRUE(instructions<ir::AllocaInstruction>(*Function).empty());
  }

  // Declared local types reject incompatible existing values and out-of-range literals without numeric coercion.
  TEST_F(SemanticVariableTest, RejectsInvalidInitializerTypesAndRanges)
  {
    auto Input = read("func mismatch(Input: i32): void { var Value: u8 = Input; } func overflow(): void { var Value: u8 = 256; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticIntegerOutOfRange);
    ASSERT_NE(function("mismatch"), nullptr);
    ASSERT_NE(function("overflow"), nullptr);
    EXPECT_TRUE(instructions<ir::AllocaInstruction>(*function("mismatch")).empty());
    EXPECT_TRUE(instructions<ir::AllocaInstruction>(*function("overflow")).empty());
  }

  // Return and call target types are still enforced after local storage has been converted to its value.
  TEST_F(SemanticVariableTest, ChecksLoadedReturnAndArgumentTypes)
  {
    auto Input = read("func accepts(Value: i32): void {} func returnMismatch(): i32 { var Value: u8 = 1; return Value; } func callMismatch(): void { var Value: u8 = 1; accepts(Value); }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Both var and const occupy an exclusive binding in their lexical block, preserving the first declaration on conflict.
  TEST_F(SemanticVariableTest, RejectsDuplicateNamesInOneBlock)
  {
    auto Input = read("func duplicate(): i32 { var Value = 1; const Value = 2; return Value; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    expectDiagnostic(core::DiagnosticKind::SemanticDuplicateName);
    const auto *Function = function("duplicate");
    ASSERT_NE(Function, nullptr);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    ASSERT_EQ(instructions<ir::AllocaInstruction>(*Function).size(), 1U);
    ASSERT_EQ(Stores.size(), 1U);
    ASSERT_TRUE(ir::IntegerConstant::classof(Stores[0]->storedValue()));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Stores[0]->storedValue())->bits(), 1U);
  }

  // A shadowing declaration reads the outer name in its initializer, then leaving its block restores that outer binding.
  TEST_F(SemanticVariableTest, RestoresOuterBindingAfterShadowing)
  {
    auto Input = read("func shadow(Input: i32): i32 { var Value = Input; { const Value = (Value); Value; } return Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("shadow");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 2U);
    ASSERT_EQ(Stores.size(), 2U);
    ASSERT_EQ(Loads.size(), 3U);
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_EQ(Stores[1]->storedValue(), Loads[0]);
    EXPECT_EQ(Loads[1]->pointer(), Allocations[1]);
    ASSERT_NE(returned(*Function), nullptr);
    ASSERT_TRUE(ir::LoadInstruction::classof(returned(*Function)->returnedValue()));
    EXPECT_EQ(static_cast<const ir::LoadInstruction *>(returned(*Function)->returnedValue())->pointer(), Allocations[0]);
    EXPECT_EQ(NameResolver(Context).lookupMember(Output, name("Value")), nullptr);
  }

  // Failed declarations do not reserve their names, so later declarations in the same block can initialize and return them.
  TEST_F(SemanticVariableTest, LeavesNoBindingAfterDeclarationFailure)
  {
    auto Input = read("func retry(): i32 { var Value: u8 = true; var Value = 7; return Value; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    expectDiagnostic(core::DiagnosticKind::SemanticTypeMismatch);
    const auto *Function = function("retry");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Stores.size(), 1U);
    ASSERT_TRUE(ir::IntegerConstant::classof(Stores[0]->storedValue()));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Stores[0]->storedValue())->bits(), 7U);
    ASSERT_NE(returned(*Function), nullptr);
    ASSERT_TRUE(ir::LoadInstruction::classof(returned(*Function)->returnedValue()));
    EXPECT_EQ(static_cast<const ir::LoadInstruction *>(returned(*Function)->returnedValue())->pointer(), Allocations[0]);
  }

  // Attributes, destructuring, module storage and unit storage remain explicitly unsupported while their semantics are absent.
  TEST_F(SemanticVariableDeathTest, RejectsUnsupportedDeclarationForms)
  {
    struct Case
    {
        const char *Source;
        const char *Description;
    };
    constexpr Case Cases[] = {
        {"func unsupported(): void { [tag] var Value = 1; }", "variable attributes"},
        {"func unsupported(): void { var (First, Second) = 1; }", "variable binding patterns"},
        {"func unsupported(): void { var [First, Rest...] = 1; }", "variable binding patterns"},
        {"func unsupported(): void { var _ = 1; }", "variable binding patterns"},
        {"var Value = 1;", "VarDecl"},
        {"const Value = 1;", "VarDecl"},
        {"func unsupported(): void { var Value: void; }", "unit variable storage"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), std::string("initial semantic analyzer does not support ") + Entry.Description);
    }
  }

  // Compile-time declarations retain the established unsupported-comptime policy instead of becoming ordinary local storage.
  TEST_F(SemanticVariableDeathTest, RejectsComptimeDeclarations)
  {
    auto Input = read("func unsupported(): void { comptime var Value = 1; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_DEATH(analyze(Input), "initial semantic analyzer does not support");
  }
} // namespace ink::semantic::test
