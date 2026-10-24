#include "ink/semantic/analyzer/analyzer.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/instruction/address_of_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
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
      EXPECT_EQ(Allocations[0]->type(), Entry.Type);
      EXPECT_TRUE(Allocations[0]->isStorage());
      EXPECT_TRUE(Allocations[0]->isMutable());
      EXPECT_TRUE(Allocations[0]->isInitialized());
      EXPECT_EQ(Stores[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Stores[0]->storedValue()->type(), Entry.Type);
      EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Loads[0]->type(), Entry.Type);
      EXPECT_FALSE(Loads[0]->isStorage());
      EXPECT_FALSE(Loads[0]->isMutable());
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

  // Explicit read-only byte pointer locals store converted string addresses and load the pointer value on return.
  TEST_F(SemanticVariableTest, InitializesTypedPointerLocalsFromStringConstants)
  {
    auto Input = read("func mutablePointer(): *const u8 { var Message: *const u8 = \"abc\"; return Message; } func immutablePointer(): *const u8 { const Message: *const u8 = \"abc\"; return Message; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Output.globalVariableCount(), 1U);
    const auto *Storage = Output.globalVariable(0);
    const auto *PointerType = Context.typePool().getPointerType(Context.typePool().getUInt8Type(false));
    for (const char *Name : {"mutablePointer", "immutablePointer"})
    {
      SCOPED_TRACE(Name);
      const auto *Function = function(Name);
      ASSERT_NE(Function, nullptr);
      const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
      const auto Stores = instructions<ir::StoreInstruction>(*Function);
      const auto Loads = instructions<ir::LoadInstruction>(*Function);
      const auto ArrayAddresses = instructions<ir::AddressOfInstruction>(*Function);
      const auto ElementAddresses = instructions<ir::GetElementPtrInstruction>(*Function);
      ASSERT_EQ(Allocations.size(), 1U);
      ASSERT_EQ(Stores.size(), 1U);
      ASSERT_EQ(Loads.size(), 1U);
      ASSERT_EQ(ArrayAddresses.size(), 1U);
      ASSERT_EQ(ElementAddresses.size(), 1U);
      EXPECT_EQ(Allocations[0]->allocatedType()->unqualifiedType(), PointerType);
      EXPECT_EQ(ArrayAddresses[0]->storage(), Storage);
      EXPECT_EQ(ElementAddresses[0]->pointer(), ArrayAddresses[0]);
      EXPECT_EQ(ElementAddresses[0]->type(), PointerType);
      EXPECT_EQ(Stores[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Stores[0]->storedValue(), ElementAddresses[0]);
      EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Loads[0]->type(), PointerType);
      ASSERT_NE(returned(*Function), nullptr);
      EXPECT_EQ(returned(*Function)->returnedValue(), Loads[0]);
    }
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
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    EXPECT_EQ(Allocations[0]->type(), Context.typePool().getQualifiedType(Context.typePool().getInt32Type(), false));
    EXPECT_TRUE(Allocations[0]->isStorage());
    EXPECT_FALSE(Allocations[0]->isMutable());
    EXPECT_TRUE(Allocations[0]->isInitialized());
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
    EXPECT_EQ(NullAllocations[0]->allocatedType(), Context.typePool().getQualifiedType(PointerType, false));
    EXPECT_EQ(NullAllocations[1]->allocatedType(), PointerType);
    EXPECT_EQ(NullAllocations[0]->type(), Context.typePool().getQualifiedType(PointerType, false));
    EXPECT_EQ(NullAllocations[1]->type(), PointerType);
    EXPECT_TRUE(NullAllocations[0]->isStorage());
    EXPECT_TRUE(NullAllocations[1]->isStorage());
    EXPECT_FALSE(NullAllocations[0]->isMutable());
    EXPECT_TRUE(NullAllocations[1]->isMutable());
    EXPECT_TRUE(ir::NullPointerConstant::classof(NullStores[0]->storedValue()));
    EXPECT_EQ(NullLoads[0]->type(), PointerType);
    EXPECT_FALSE(NullLoads[0]->isStorage());
    EXPECT_EQ(NullStores[1]->storedValue(), NullLoads[0]);
    EXPECT_EQ(NullLoads[1]->type(), PointerType);
    const auto OpaqueStores = instructions<ir::StoreInstruction>(*OpaqueFunction);
    const auto OpaqueLoads = instructions<ir::LoadInstruction>(*OpaqueFunction);
    ASSERT_EQ(OpaqueStores.size(), 1U);
    ASSERT_EQ(OpaqueLoads.size(), 1U);
    EXPECT_EQ(OpaqueStores[0]->storedValue(), OpaqueFunction->parameter(0));
    EXPECT_EQ(OpaqueLoads[0]->type(), OpaqueFunction->parameter(0)->type());
  }

  // Copying a readonly pointer through var and const declarations removes only each source slot's outer const qualifier.
  TEST_F(SemanticVariableTest, PreservesReadonlyPointeeAcrossConstLocalCopies)
  {
    auto Input = read("func readonly(Input: *const i32): *const i32 { var Open = Input; const Fixed = Open; var Copy = Fixed; return Copy; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("readonly");
    ASSERT_NE(Function, nullptr);
    auto &Types = Context.typePool();
    const auto *ConstInteger = Types.getQualifiedType(Types.getInt32Type(), false);
    const auto *ReadonlyPointer = Types.getPointerType(ConstInteger);
    const auto *ConstReadonlyPointer = Types.getQualifiedType(ReadonlyPointer, false);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 3U);
    ASSERT_EQ(Loads.size(), 3U);
    EXPECT_EQ(Allocations[0]->type(), ReadonlyPointer);
    EXPECT_EQ(Allocations[1]->type(), ConstReadonlyPointer);
    EXPECT_EQ(Allocations[2]->type(), ReadonlyPointer);
    EXPECT_TRUE(Allocations[0]->isMutable());
    EXPECT_FALSE(Allocations[1]->isMutable());
    EXPECT_TRUE(Allocations[2]->isMutable());
    for (const auto *Load : Loads)
    {
      EXPECT_EQ(Load->type(), ReadonlyPointer);
    }
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[2]);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Matching by-value top-level const never erases a pointee qualifier or treats different nested pointer permissions as identical.
  TEST_F(SemanticVariableTest, RejectsImplicitPointeeQualifierChanges)
  {
    for (const char *Source : {"func drop(Input: *const i32): void { var Copy: *i32 = Input; }", "func add(Input: *i32): void { var Copy: *const i32 = Input; }", "func nested(Input: **const i32): void { var Copy: *const *i32 = Input; }"})
    {
      SCOPED_TRACE(Source);
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(analyze(Input));
      expectDiagnostic(core::DiagnosticKind::SemanticTypeMismatch);
    }
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

  // A typed local may remain unused without a fabricated zero initializer or a spurious uninitialized-read warning.
  TEST_F(SemanticVariableTest, AllowsUnusedUninitializedTypedLocal)
  {
    auto Input = read("func unused(): void { var Value: i32; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("unused");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    EXPECT_EQ(Allocations[0]->type(), Context.typePool().getInt32Type());
    EXPECT_TRUE(Allocations[0]->isStorage());
    EXPECT_TRUE(Allocations[0]->isMutable());
    EXPECT_FALSE(Allocations[0]->isInitialized());
    EXPECT_TRUE(instructions<ir::StoreInstruction>(*Function).empty());
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Reading an uninitialized local reports a warning while preserving its load, return value and uninitialized storage marker.
  TEST_F(SemanticVariableTest, WarnsAndLoadsUninitializedLocal)
  {
    auto Input = read("func read(): i32 { var Value: i32; return Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUninitializedRead);
    EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[0].Kind), core::DiagnosticSeverity::Warning);
    const auto *Function = function("read");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Loads.size(), 1U);
    EXPECT_FALSE(Allocations[0]->isInitialized());
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[0]);
    EXPECT_TRUE(instructions<ir::StoreInstruction>(*Function).empty());
  }

  // Uninitialized reads in initializers and call arguments warn independently while producing both the copied local and the call.
  TEST_F(SemanticVariableTest, WarnsAndEmitsUninitializedInitializerAndArgument)
  {
    constexpr std::string_view Source = "func accepts(Value: i32): void {} func copy(): void { var Value: i32; var Copy = Value; accepts(Value); }";
    auto Input = read(Source);
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    for (const auto &Diagnostic : Diagnostics.diagnostics())
    {
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticUninitializedRead);
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostic.Kind), core::DiagnosticSeverity::Warning);
      EXPECT_EQ(Diagnostic.Source, Input.Unit->input().lexedFile().sourceId());
    }
    const std::size_t InitializerOffset = Source.find("Value; accepts");
    const std::size_t ArgumentOffset = Source.rfind("Value");
    ASSERT_NE(InitializerOffset, std::string_view::npos);
    ASSERT_NE(ArgumentOffset, std::string_view::npos);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, core::SourceRange::fromByteOffsets(InitializerOffset, InitializerOffset + std::string_view("Value").size()));
    EXPECT_EQ(Diagnostics.diagnostics()[1].Span, core::SourceRange::fromByteOffsets(ArgumentOffset, ArgumentOffset + std::string_view("Value").size()));
    const auto *Function = function("copy");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    const auto Stores = instructions<ir::StoreInstruction>(*Function);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 2U);
    ASSERT_EQ(Loads.size(), 2U);
    ASSERT_EQ(Stores.size(), 1U);
    ASSERT_EQ(Calls.size(), 1U);
    EXPECT_FALSE(Allocations[0]->isInitialized());
    EXPECT_TRUE(Allocations[1]->isInitialized());
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_EQ(Loads[1]->pointer(), Allocations[0]);
    EXPECT_EQ(Stores[0]->storedValue(), Loads[0]);
    ASSERT_NE(Calls[0]->argument(0), nullptr);
    EXPECT_EQ(Calls[0]->argument(0)->value(), Loads[1]);
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

  // Local declarations preserve initializer error ranges and reject layout-free type annotations before analyzing their initializers.
  TEST_F(SemanticVariableTest, RejectsInvalidInitializerTypesAndRanges)
  {
    constexpr std::string_view Source = "func mismatch(Input: i32): void { var Value: u8 = Input; } func overflow(): void { var Value: u8 = 256; } func noLayout(): void { var Value: ((type)) = Missing; }";
    auto Input = read(Source);
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 3U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticIntegerOutOfRange);
    EXPECT_EQ(Diagnostics.diagnostics()[2].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, core::SourceRange::fromByteOffsets(Source.rfind("Input"), Source.rfind("Input") + std::string_view("Input").size()));
    EXPECT_EQ(Diagnostics.diagnostics()[1].Span, core::SourceRange::fromByteOffsets(Source.find("256"), Source.find("256") + std::string_view("256").size()));
    EXPECT_EQ(Diagnostics.diagnostics()[2].Span, core::SourceRange::fromByteOffsets(Source.find("((type))"), Source.find("((type))") + std::string_view("((type))").size()));
    for (const auto &Diagnostic : Diagnostics.diagnostics())
    {
      EXPECT_EQ(Diagnostic.Source, Input.Unit->input().lexedFile().sourceId());
    }
    ASSERT_NE(function("mismatch"), nullptr);
    ASSERT_NE(function("overflow"), nullptr);
    ASSERT_NE(function("noLayout"), nullptr);
    EXPECT_TRUE(instructions<ir::AllocaInstruction>(*function("mismatch")).empty());
    EXPECT_TRUE(instructions<ir::AllocaInstruction>(*function("overflow")).empty());
    EXPECT_TRUE(instructions<ir::AllocaInstruction>(*function("noLayout")).empty());
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

  // Shadowed variables retain independent initialization warnings, and both initialized and uninitialized reads still produce loads.
  TEST_F(SemanticVariableTest, KeepsInitializationWithEachShadowedStorage)
  {
    auto Input = read("func innerUninitialized(): void { var Value = 1; { var Value: i32; Value; } Value; } func outerUninitialized(): void { var Value: i32; { var Value = 1; Value; } Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUninitializedRead);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticUninitializedRead);
    EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[0].Kind), core::DiagnosticSeverity::Warning);
    EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[1].Kind), core::DiagnosticSeverity::Warning);
    const auto *InnerUninitialized = function("innerUninitialized");
    const auto *OuterUninitialized = function("outerUninitialized");
    ASSERT_NE(InnerUninitialized, nullptr);
    ASSERT_NE(OuterUninitialized, nullptr);
    const auto InnerAllocations = instructions<ir::AllocaInstruction>(*InnerUninitialized);
    const auto OuterAllocations = instructions<ir::AllocaInstruction>(*OuterUninitialized);
    const auto InnerLoads = instructions<ir::LoadInstruction>(*InnerUninitialized);
    const auto OuterLoads = instructions<ir::LoadInstruction>(*OuterUninitialized);
    ASSERT_EQ(InnerAllocations.size(), 2U);
    ASSERT_EQ(OuterAllocations.size(), 2U);
    ASSERT_EQ(InnerLoads.size(), 2U);
    ASSERT_EQ(OuterLoads.size(), 2U);
    EXPECT_TRUE(InnerAllocations[0]->isInitialized());
    EXPECT_FALSE(InnerAllocations[1]->isInitialized());
    EXPECT_FALSE(OuterAllocations[0]->isInitialized());
    EXPECT_TRUE(OuterAllocations[1]->isInitialized());
    EXPECT_EQ(InnerLoads[0]->pointer(), InnerAllocations[1]);
    EXPECT_EQ(InnerLoads[1]->pointer(), InnerAllocations[0]);
    EXPECT_EQ(OuterLoads[0]->pointer(), OuterAllocations[1]);
    EXPECT_EQ(OuterLoads[1]->pointer(), OuterAllocations[0]);
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

  // Inference copies const-typed parameters into mutable values while preserving nested qualifiers and explicit annotations.
  TEST_F(SemanticVariableTest, InfersCopiesWithoutTopLevelConst)
  {
    auto &Types = Context.typePool();
    const auto *Pointer = Types.getPointerType(Types.getUInt8Type(false));
    const auto *ConstPointer = Types.getQualifiedType(Pointer, false);
    ASSERT_NE(ConstPointer, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("ConstPointer"), const_cast<ir::Type &>(*ConstPointer)), BindResult::Inserted);
    auto Input = read("func copy(Input: ConstPointer): *const u8 { var MutableCopy = Input; const ImmutableCopy = Input; var Explicit: ConstPointer = Input; return MutableCopy; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("copy");
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->parameter(0)->type(), ConstPointer);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 3U);
    EXPECT_EQ(Allocations[0]->type(), Pointer);
    EXPECT_TRUE(Allocations[0]->isMutable());
    EXPECT_EQ(Allocations[1]->type(), ConstPointer);
    EXPECT_FALSE(Allocations[1]->isMutable());
    EXPECT_EQ(Allocations[2]->type(), ConstPointer);
    EXPECT_FALSE(Allocations[2]->isMutable());
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue()->type(), Pointer);
    EXPECT_FALSE(Pointer->pointeeType()->isMutable());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // By-value call results drop only top-level const, leaving return signatures and pointee qualifications intact.
  TEST_F(SemanticVariableTest, CopiesQualifiedCallResultsWithoutChangingSignatures)
  {
    auto &Types = Context.typePool();
    const auto *ConstInteger = Types.getInt32Type(false);
    const auto *Pointer = Types.getPointerType(Types.getUInt8Type(false));
    const auto *ConstPointer = Types.getQualifiedType(Pointer, false);
    ASSERT_NE(ConstPointer, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("ConstInteger"), const_cast<ir::IntegerType &>(*ConstInteger)), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("ConstPointer"), const_cast<ir::Type &>(*ConstPointer)), BindResult::Inserted);
    auto Input = read("func integerSource(): ConstInteger { return 1; } func pointerSource(Input: ConstPointer): ConstPointer { return Input; } func copy(Input: ConstPointer): *const u8 { var Integer = integerSource(); var Pointer = pointerSource(Input); return Pointer; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *IntegerSource = function("integerSource");
    const auto *PointerSource = function("pointerSource");
    const auto *Copy = function("copy");
    ASSERT_NE(IntegerSource, nullptr);
    ASSERT_NE(PointerSource, nullptr);
    ASSERT_NE(Copy, nullptr);
    EXPECT_EQ(IntegerSource->functionType()->returnType(), ConstInteger);
    EXPECT_EQ(PointerSource->functionType()->returnType(), ConstPointer);
    const auto Calls = instructions<ir::CallInstruction>(*Copy);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Copy);
    ASSERT_EQ(Calls.size(), 2U);
    ASSERT_EQ(Allocations.size(), 2U);
    EXPECT_EQ(Calls[0]->type(), Types.getInt32Type());
    EXPECT_EQ(Calls[0]->functionType()->returnType(), ConstInteger);
    EXPECT_EQ(Calls[1]->type(), Pointer);
    EXPECT_EQ(Calls[1]->functionType()->returnType(), ConstPointer);
    EXPECT_EQ(Allocations[0]->type(), Types.getInt32Type());
    EXPECT_EQ(Allocations[1]->type(), Pointer);
    EXPECT_TRUE(Allocations[0]->isMutable());
    EXPECT_TRUE(Allocations[1]->isMutable());
    EXPECT_FALSE(Pointer->pointeeType()->isMutable());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Const-qualified void signatures retain the same implicit and explicit unit-return rules as ordinary void.
  TEST_F(SemanticVariableTest, TreatsQualifiedVoidAsUnitReturn)
  {
    const auto *ConstVoid = Context.typePool().getVoidType(false);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("ConstVoid"), const_cast<ir::VoidType &>(*ConstVoid)), BindResult::Inserted);
    auto Input = read("func implicitUnit(): ConstVoid {} func explicitUnit(): ConstVoid { return; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    for (const char *Name : {"implicitUnit", "explicitUnit"})
    {
      const auto *Function = function(Name);
      ASSERT_NE(Function, nullptr);
      EXPECT_EQ(Function->functionType()->returnType(), ConstVoid);
      ASSERT_NE(returned(*Function), nullptr);
      EXPECT_EQ(returned(*Function)->returnedValue(), nullptr);
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    auto InvalidInput = read("func invalidUnit(): ConstVoid { return 1; }");
    ASSERT_TRUE(InvalidInput.succeeded());
    EXPECT_FALSE(analyze(InvalidInput));
    expectDiagnostic(core::DiagnosticKind::SemanticTypeMismatch);
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
