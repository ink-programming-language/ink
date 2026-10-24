#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/instruction/address_of_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/dereference_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    class SemanticAddressTest : public testing::Test
    {
      protected:
        SemanticAddressTest()
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

        static const frontend::parser::Expr &syntax(const frontend::parser::ParseResult &Input)
        {
          const auto *Statement = frontend::parser::cast<frontend::parser::DeclStmt>(Input.Unit->root()->statements()[0]);
          return *frontend::parser::cast<frontend::parser::VarDecl>(Statement->declaration())->initializer();
        }

        void unregister(ir::Module &Module)
        {
          auto &Scopes = Context.scopeStore();
          for (const auto &Function : Module.globalFunctions())
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
          Scopes.removeTarget(Module);
        }

        void TearDown() override
        {
          unregister(Output);
        }

        core::SourceManager Sources;
        core::CollectingDiagnosticConsumer Diagnostics;
        core::DiagnosticEngine DiagnosticEngine;
        frontend::FrontendContext Frontend;
        SemanticContext Context;
        ir::Module Output;
        Analyzer Analysis;
    };
  } // namespace

  // A native output argument receives a local's address without loading its initialized integer value.
  TEST_F(SemanticAddressTest, PassesInitializedLocalAddressToFunction)
  {
    auto Input = read("import \"C\" func consume(Written: *u32): i32; func main(): i32 { var Written: u32 = 0; return consume(&Written); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Addresses = instructions<ir::AddressOfInstruction>(*Function);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Addresses.size(), 1U);
    ASSERT_EQ(Calls.size(), 1U);
    EXPECT_EQ(Addresses[0]->storage(), Allocations[0]);
    EXPECT_EQ(Addresses[0]->type(), Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(32, false)));
    EXPECT_EQ(Calls[0]->argument(0)->value(), Addresses[0]);
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // An uninitialized output argument warns about taking its address, while preserving the call and avoiding a read or fabricated initializer.
  TEST_F(SemanticAddressTest, WarnsAndPassesUninitializedLocalAddressToFunction)
  {
    const std::string_view Source = "import \"C\" func consume(Written: *u32): i32; func main(): i32 { var Written: u32; return consume(&Written); }";
    auto Input = read(Source);
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    const auto &Warning = Diagnostics.diagnostics()[0];
    EXPECT_EQ(Warning.Kind, core::DiagnosticKind::SemanticUninitializedAddress);
    EXPECT_EQ(core::diagnosticDefaultSeverity(Warning.Kind), core::DiagnosticSeverity::Warning);
    ASSERT_EQ(Warning.Arguments.size(), 1U);
    EXPECT_EQ(Warning.Arguments[0].Name, core::DiagnosticArgumentName::SymbolName);
    const auto *SymbolName = std::get_if<std::string>(&Warning.Arguments[0].Value);
    ASSERT_NE(SymbolName, nullptr);
    EXPECT_EQ(*SymbolName, "Written");
    const auto AddressOffset = Source.find("&Written");
    EXPECT_EQ(Warning.Span, core::SourceRange::fromByteOffsets(AddressOffset, AddressOffset + std::string_view("&Written").size()));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Addresses = instructions<ir::AddressOfInstruction>(*Function);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Addresses.size(), 1U);
    ASSERT_EQ(Calls.size(), 1U);
    EXPECT_FALSE(Allocations[0]->isInitialized());
    EXPECT_EQ(Addresses[0]->storage(), Allocations[0]);
    EXPECT_EQ(Addresses[0]->type(), Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(32, false)));
    ASSERT_NE(Calls[0]->argument(0), nullptr);
    EXPECT_EQ(Calls[0]->argument(0)->value(), Addresses[0]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Calls[0]);
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
    EXPECT_TRUE(instructions<ir::StoreInstruction>(*Function).empty());
  }

  // Taking a const local's address preserves readonly pointee permission, while reading it produces an ordinary integer value.
  TEST_F(SemanticAddressTest, PreservesConstLocalPointeeType)
  {
    auto Input = read("import \"C\" func consume(Value: *const i32): void; func main(): i32 { const Value = 7; consume(&Value); return *&Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Addresses = instructions<ir::AddressOfInstruction>(*Function);
    const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Addresses.size(), 2U);
    ASSERT_EQ(Dereferences.size(), 1U);
    ASSERT_EQ(Loads.size(), 1U);
    const auto *ConstInteger = Context.typePool().get<ink::ir::IntegerType>(32, true, false);
    EXPECT_EQ(Allocations[0]->type(), ConstInteger);
    EXPECT_FALSE(Allocations[0]->isMutable());
    for (const auto *Address : Addresses)
    {
      EXPECT_EQ(Address->storage(), Allocations[0]);
      EXPECT_EQ(Address->type(), Context.typePool().get<ink::ir::PointerType>(ConstInteger));
    }
    EXPECT_EQ(Dereferences[0]->pointer(), Addresses[1]);
    EXPECT_EQ(Dereferences[0]->type(), ConstInteger);
    EXPECT_FALSE(Dereferences[0]->isMutable());
    EXPECT_EQ(Loads[0]->pointer(), Dereferences[0]);
    EXPECT_EQ(Loads[0]->type(), Context.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Dereferencing an address of a local reads the object exactly once after preserving the separate address and storage identities.
  TEST_F(SemanticAddressTest, LoadsLocalThroughAddressAndDereference)
  {
    auto Input = read("func main(): i32 { var Value = 42; return *((&Value)); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Addresses = instructions<ir::AddressOfInstruction>(*Function);
    const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Addresses.size(), 1U);
    ASSERT_EQ(Dereferences.size(), 1U);
    ASSERT_EQ(Loads.size(), 1U);
    EXPECT_EQ(Addresses[0]->storage(), Allocations[0]);
    EXPECT_EQ(Dereferences[0]->pointer(), Addresses[0]);
    EXPECT_EQ(Loads[0]->pointer(), Dereferences[0]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[0]);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Addressing a dereferenced call evaluates the pointer-producing function once and never reads the pointed-to object.
  TEST_F(SemanticAddressTest, AddressesDereferencedCallWithoutReadingPointee)
  {
    auto Input = read("import \"C\" func acquire(): *i32; func main(): *i32 { return &*acquire(); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Calls = instructions<ir::CallInstruction>(*Function);
    const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
    const auto Addresses = instructions<ir::AddressOfInstruction>(*Function);
    ASSERT_EQ(Calls.size(), 1U);
    ASSERT_EQ(Dereferences.size(), 1U);
    ASSERT_EQ(Addresses.size(), 1U);
    EXPECT_EQ(Dereferences[0]->pointer(), Calls[0]);
    EXPECT_EQ(Addresses[0]->storage(), Dereferences[0]);
    EXPECT_EQ(Addresses[0]->type(), Calls[0]->type());
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Function).empty());
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Addresses[0]);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A pointer stored in a const local is loaded once before dereference and the pointed-to value is loaded separately.
  TEST_F(SemanticAddressTest, LoadsPointerLocalBeforeLoadingPointee)
  {
    auto Input = read("func main(Input: *i32): i32 { const Pointer = Input; return *Pointer; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Dereferences.size(), 1U);
    ASSERT_EQ(Loads.size(), 2U);
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_EQ(Dereferences[0]->pointer(), Loads[0]);
    EXPECT_TRUE(Dereferences[0]->isMutable());
    EXPECT_EQ(Loads[1]->pointer(), Dereferences[0]);
    ASSERT_NE(returned(*Function), nullptr);
    EXPECT_EQ(returned(*Function)->returnedValue(), Loads[1]);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Nested dereference loads each pointer layer once and strips only the final object's top-level const qualifier.
  TEST_F(SemanticAddressTest, LoadsMultiplePointerLayers)
  {
    auto Input = read("func main(Input: **const i32): i32 { return **Input; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Dereferences.size(), 2U);
    ASSERT_EQ(Loads.size(), 2U);
    EXPECT_EQ(Dereferences[0]->pointer(), Function->parameter(0));
    EXPECT_EQ(Loads[0]->pointer(), Dereferences[0]);
    EXPECT_EQ(Dereferences[1]->pointer(), Loads[0]);
    EXPECT_EQ(Dereferences[1]->type(), Context.typePool().get<ink::ir::IntegerType>(32, true, false));
    EXPECT_FALSE(Dereferences[1]->isMutable());
    EXPECT_EQ(Loads[1]->pointer(), Dereferences[1]);
    EXPECT_EQ(Loads[1]->type(), Context.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Explicitly dereferencing mutable or const function storage pointers loads the callable value before the indirect call.
  TEST_F(SemanticAddressTest, CallsDereferencedFunctionPointer)
  {
    auto Input = read("func invoke(Callback: *func(i32): i32): i32 { return (*Callback)(7); } func invokeConst(Callback: *const func(i32): i32): i32 { return (*Callback)(7); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    for (const char *Name : {"invoke", "invokeConst"})
    {
      const auto *Function = function(Name);
      ASSERT_NE(Function, nullptr);
      const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
      const auto Loads = instructions<ir::LoadInstruction>(*Function);
      const auto Calls = instructions<ir::CallInstruction>(*Function);
      ASSERT_EQ(Dereferences.size(), 1U);
      ASSERT_EQ(Loads.size(), 1U);
      ASSERT_EQ(Calls.size(), 1U);
      EXPECT_EQ(Dereferences[0]->pointer(), Function->parameter(0));
      EXPECT_EQ(Loads[0]->pointer(), Dereferences[0]);
      EXPECT_EQ(Calls[0]->callee(), Loads[0]);
      EXPECT_EQ(Calls[0]->directCallee(), nullptr);
      EXPECT_EQ(Loads[0]->type(), Calls[0]->functionType());
      EXPECT_TRUE(Loads[0]->type()->isMutable());
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Mutable and const callable locals retain addressable storage, and dereferencing their addresses loads exactly once before calling.
  TEST_F(SemanticAddressTest, CallsFunctionValueThroughLocalStorageAddress)
  {
    auto Input = read("func foo(): i32 { return 1; } func mutableLocal(): i32 { var F = foo; return (*(&F))(); } func constLocal(): i32 { const F = foo; return (*(&F))(); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_NE(function("foo"), nullptr);
    for (const char *Name : {"mutableLocal", "constLocal"})
    {
      const auto *Function = function(Name);
      ASSERT_NE(Function, nullptr);
      const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
      const auto Addresses = instructions<ir::AddressOfInstruction>(*Function);
      const auto Dereferences = instructions<ir::DereferenceInstruction>(*Function);
      const auto Loads = instructions<ir::LoadInstruction>(*Function);
      const auto Calls = instructions<ir::CallInstruction>(*Function);
      ASSERT_EQ(Allocations.size(), 1U);
      ASSERT_EQ(Addresses.size(), 1U);
      ASSERT_EQ(Dereferences.size(), 1U);
      ASSERT_EQ(Loads.size(), 1U);
      ASSERT_EQ(Calls.size(), 1U);
      EXPECT_EQ(Allocations[0]->isMutable(), std::string_view(Name) == "mutableLocal");
      EXPECT_EQ(Addresses[0]->storage(), Allocations[0]);
      EXPECT_EQ(Dereferences[0]->pointer(), Addresses[0]);
      EXPECT_EQ(Dereferences[0]->type(), Allocations[0]->type());
      EXPECT_EQ(Loads[0]->pointer(), Dereferences[0]);
      EXPECT_EQ(Loads[0]->type(), function("foo")->functionType());
      EXPECT_EQ(Calls[0]->callee(), Loads[0]);
      EXPECT_EQ(Calls[0]->directCallee(), nullptr);
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Array pointers retain their complete array object type when dereferenced and when its address is recovered.
  TEST_F(SemanticAddressTest, PreservesArrayPointeeType)
  {
    auto Input = read("func copy(Input: *[i32; 4]): [i32; 4] { return *Input; } func address(Input: *[i32; 4]): *[i32; 4] { return &*Input; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *Copy = function("copy");
    const auto *Address = function("address");
    ASSERT_NE(Copy, nullptr);
    ASSERT_NE(Address, nullptr);
    const auto CopyDereferences = instructions<ir::DereferenceInstruction>(*Copy);
    const auto CopyLoads = instructions<ir::LoadInstruction>(*Copy);
    const auto Addresses = instructions<ir::AddressOfInstruction>(*Address);
    ASSERT_EQ(CopyDereferences.size(), 1U);
    ASSERT_EQ(CopyLoads.size(), 1U);
    ASSERT_EQ(Addresses.size(), 1U);
    const auto *ArrayType = Context.typePool().get<ink::ir::ArrayType>(Context.typePool().get<ink::ir::IntegerType>(32, true), 4);
    EXPECT_EQ(CopyDereferences[0]->type(), ArrayType);
    EXPECT_EQ(CopyLoads[0]->type(), ArrayType);
    EXPECT_EQ(Addresses[0]->type(), Context.typePool().get<ink::ir::PointerType>(ArrayType));
    EXPECT_TRUE(instructions<ir::LoadInstruction>(*Address).empty());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Shared expression analysis exposes dereferenced storage without a read; value emission appends a load even without a declaration initialization marker.
  TEST_F(SemanticAddressTest, SeparatesDereferencedStorageFromValueEmission)
  {
    auto Input = read("var Result = *Pointer;");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    auto *Function = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true, false))}));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_EQ(State.nameResolver().bind(name("Pointer"), *Function->parameter(0)), BindResult::Inserted);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    const auto *Storage = Analysis.analyzeExpr(State, syntax(Input), {.ExpectedType = Types.get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Storage, nullptr);
    ASSERT_TRUE(ir::DereferenceInstruction::classof(Storage));
    EXPECT_TRUE(Storage->isStorage());
    EXPECT_FALSE(Storage->isMutable());
    EXPECT_FALSE(Storage->isInitialized());
    EXPECT_EQ(Storage->type(), Types.get<ink::ir::IntegerType>(32, true, false));
    EXPECT_EQ(Block->instructionCount(), 1U);
    const auto *Value = Analysis.emitValue(State, syntax(Input), {.ExpectedType = Types.get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Value, nullptr);
    ASSERT_TRUE(ir::LoadInstruction::classof(Value));
    EXPECT_EQ(Value->type(), Types.get<ink::ir::IntegerType>(32, true));
    EXPECT_FALSE(Value->isStorage());
    EXPECT_EQ(Block->instructionCount(), 3U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Pointer and reference type constructors remain descriptors instead of being interpreted as runtime address operations.
  TEST_F(SemanticAddressTest, RetainsUnaryTypeConstructors)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Source;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"var Result = *const i32;", Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true, false))},
        {"var Result = &i32;", Types.get<ink::ir::ReferenceType>(Types.get<ink::ir::IntegerType>(32, true))},
        {"var Result = **i32;", Types.get<ink::ir::PointerType>(Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true)))},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      Context.setSource(Input.Unit->input().lexedFile().sourceId());
      EXPECT_EQ(Analysis.analyzeExpr(State, syntax(Input)), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Taking the address of a nonstorage value reports an error without reading the operand.
  TEST_F(SemanticAddressTest, RejectsNonstorageAddresses)
  {
    const char *Cases[] = {
        "func main(): void { &1; }",
        "func main(): void { &true; }",
        "func main(Value: i32): void { &Value; }",
        "func value(): i32 { return 1; } func main(): void { &value; }",
        "func value(): i32 { return 1; } func main(): void { &value(); }",
        "func main(): void { var Value = 1; &(&Value); }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidAddressOperand);
      EXPECT_EQ(Diagnostics.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[0].Kind), core::DiagnosticSeverity::Error);
      const auto *Main = Destination.globalFunction(Destination.globalFunctionCount() - 1);
      ASSERT_NE(Main, nullptr);
      EXPECT_TRUE(instructions<ir::LoadInstruction>(*Main).empty());
      unregister(Destination);
    }
  }

  // Dereference rejects nonpointer values, the null pointer constant and opaque pointers with a user diagnostic before creating pointee storage.
  TEST_F(SemanticAddressTest, RejectsInvalidDereferenceOperands)
  {
    const char *Cases[] = {
        "func main(): void { *1; }",
        "func main(): void { *true; }",
        "func main(): void { *nullptr; }",
        "func main(Input: *void): void { *Input; }",
        "func main(Input: *const void): void { *Input; }",
        "func main(): void { var Value = 1; *Value; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidDereference);
      EXPECT_EQ(Diagnostics.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[0].Kind), core::DiagnosticSeverity::Error);
      ASSERT_EQ(Destination.globalFunctionCount(), 1U);
      EXPECT_TRUE(instructions<ir::DereferenceInstruction>(*Destination.globalFunction(0)).empty());
      unregister(Destination);
    }
  }

  // An uninitialized pointer read warns without blocking dereference or address recovery, and the pointee has no declaration initialization warning.
  TEST_F(SemanticAddressTest, WarnsAndDereferencesUninitializedPointer)
  {
    struct Case
    {
        const char *Source;
        bool AddressResult;
    };
    const Case Cases[] = {
        {"func main(): i32 { var Pointer: *i32; return *Pointer; }", false},
        {"func main(): *i32 { var Pointer: *i32; return &*Pointer; }", true},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_TRUE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUninitializedRead);
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[0].Kind), core::DiagnosticSeverity::Warning);
      ASSERT_EQ(Destination.globalFunctionCount(), 1U);
      const auto &Function = *Destination.globalFunction(0);
      const auto Allocations = instructions<ir::AllocaInstruction>(Function);
      const auto Loads = instructions<ir::LoadInstruction>(Function);
      const auto Dereferences = instructions<ir::DereferenceInstruction>(Function);
      const auto Addresses = instructions<ir::AddressOfInstruction>(Function);
      ASSERT_EQ(Allocations.size(), 1U);
      ASSERT_EQ(Loads.size(), Entry.AddressResult ? 1U : 2U);
      ASSERT_EQ(Dereferences.size(), 1U);
      EXPECT_FALSE(Allocations[0]->isInitialized());
      EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
      EXPECT_EQ(Dereferences[0]->pointer(), Loads[0]);
      ASSERT_NE(returned(Function), nullptr);
      if (Entry.AddressResult)
      {
        ASSERT_EQ(Addresses.size(), 1U);
        EXPECT_EQ(Addresses[0]->storage(), Dereferences[0]);
        EXPECT_EQ(returned(Function)->returnedValue(), Addresses[0]);
      }
      else
      {
        EXPECT_TRUE(Addresses.empty());
        EXPECT_EQ(Loads[1]->pointer(), Dereferences[0]);
        EXPECT_EQ(returned(Function)->returnedValue(), Loads[1]);
      }
      unregister(Destination);
    }
  }

  // An uninitialized integer read still reaches the invalid-dereference error after its warning and load, without producing pointee storage.
  TEST_F(SemanticAddressTest, ReportsInvalidDereferenceAfterUninitializedReadWarning)
  {
    auto Input = read("func main(): void { var Value: i32; *Value; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUninitializedRead);
    EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[0].Kind), core::DiagnosticSeverity::Warning);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticInvalidDereference);
    EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostics.diagnostics()[1].Kind), core::DiagnosticSeverity::Error);
    const auto *Function = function("main");
    ASSERT_NE(Function, nullptr);
    const auto Allocations = instructions<ir::AllocaInstruction>(*Function);
    const auto Loads = instructions<ir::LoadInstruction>(*Function);
    ASSERT_EQ(Allocations.size(), 1U);
    ASSERT_EQ(Loads.size(), 1U);
    EXPECT_FALSE(Allocations[0]->isInitialized());
    EXPECT_EQ(Loads[0]->pointer(), Allocations[0]);
    EXPECT_TRUE(instructions<ir::DereferenceInstruction>(*Function).empty());
  }

  // Result expectations reject pointee qualifier changes and mismatched dereferenced values without changing how either operand is typed.
  TEST_F(SemanticAddressTest, RejectsMismatchedExpectedResultTypes)
  {
    const char *Cases[] = {
        "func main(): *i32 { const Value = 1; return &Value; }",
        "func main(): *const i32 { var Value = 1; return &Value; }",
        "func main(Input: *i32): u8 { return *Input; }",
        "func main(Input: **const i32): *i32 { return *Input; }",
        "func main(Input: *const i32): *i32 { return &*Input; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
      unregister(Destination);
    }
  }

  // A pointer result expectation does not contextualize an address operand literal, and an integer result expectation does not contextualize null dereference.
  TEST_F(SemanticAddressTest, DoesNotForwardExpectedTypeToOperands)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Expected;
    };
    const Case Cases[] = {
        {"func main(): *i32 { return &1; }", core::DiagnosticKind::SemanticInvalidAddressOperand},
        {"func main(): i32 { return *nullptr; }", core::DiagnosticKind::SemanticInvalidDereference},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Entry.Expected);
      unregister(Destination);
    }
  }

  // Module descriptors remain invalid runtime operands for both address operators rather than becoming pointer or reference types.
  TEST_F(SemanticAddressTest, RejectsModuleOperandsAsTypeMismatches)
  {
    ASSERT_EQ(NameResolver(Context).bind(name("Package"), Output), BindResult::Inserted);
    for (const char *Source : {"func main(): void { &Package; }", "func main(): void { *Package; }"})
    {
      SCOPED_TRACE(Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
      unregister(Destination);
    }
  }
} // namespace ink::semantic::test
