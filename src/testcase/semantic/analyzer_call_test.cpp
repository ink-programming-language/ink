#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/instruction/address_of_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    enum class MalformedCall
    {
      MissingCallee,
      MissingArgumentValue,
      InvalidArgumentKind,
      MissingArgumentName,
      EmptyArgumentName,
      UnexpectedPositionalName,
      UnexpectedSpreadName,
      MissingParenthesizedCallee,
      MissingMemberObject,
    };

    class SemanticCallTest : public testing::Test
    {
      protected:
        SemanticCallTest()
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

        static const ir::CallInstruction *call(const ir::Function &Function, std::size_t Index)
        {
          const auto *Instruction = Function.entryBlock() ? Function.entryBlock()->instruction(Index) : nullptr;
          return ir::CallInstruction::classof(Instruction) ? static_cast<const ir::CallInstruction *>(Instruction) : nullptr;
        }

        static const ir::ReturnInstruction *returned(const ir::Function &Function)
        {
          const auto *Block = Function.entryBlock();
          const auto *Instruction = Block && Block->instructionCount() ? Block->instruction(Block->instructionCount() - 1) : nullptr;
          return ir::ReturnInstruction::classof(Instruction) ? static_cast<const ir::ReturnInstruction *>(Instruction) : nullptr;
        }

        static frontend::parser::CallExpr &callSyntax(frontend::parser::ParseResult &Input)
        {
          using namespace frontend::parser;
          auto *Statement = cast<DeclStmt>(Input.Unit->root()->statements()[0]);
          return *cast<CallExpr>(cast<VarDecl>(Statement->declaration())->initializer());
        }

        frontend::parser::CallExpr *malformedCall(frontend::parser::ParseResult &Input, MalformedCall Failure)
        {
          using namespace frontend::parser;
          auto &AST = Input.Unit->context();
          auto &Original = callSyntax(Input);
          auto *Callee = Original.callee();
          auto OriginalArgument = Original.arguments()[0];
          auto Form = OriginalArgument.form();
          auto ArgumentName = OriginalArgument.name();
          auto *Value = OriginalArgument.value();
          const auto CalleeName = cast<NameExpr>(Callee)->name();
          switch (Failure)
          {
            case MalformedCall::MissingCallee:
              Callee = nullptr;
              break;
            case MalformedCall::MissingArgumentValue:
              Value = nullptr;
              break;
            case MalformedCall::InvalidArgumentKind:
              Form = static_cast<ArgumentKind>(255);
              break;
            case MalformedCall::MissingArgumentName:
              Form = ArgumentKind::Named;
              break;
            case MalformedCall::EmptyArgumentName:
            {
              Form = ArgumentKind::Named;
              auto EmptyName = CalleeName;
              EmptyName.Text = {};
              ArgumentName = EmptyName;
              break;
            }
            case MalformedCall::UnexpectedPositionalName:
              ArgumentName = CalleeName;
              break;
            case MalformedCall::UnexpectedSpreadName:
              Form = ArgumentKind::SpreadPositional;
              ArgumentName = CalleeName;
              break;
            case MalformedCall::MissingParenthesizedCallee:
              Callee = AST.make<ParenExpr>(Original.getSourceRange(), nullptr);
              break;
            case MalformedCall::MissingMemberObject:
              Callee = AST.make<MemberExpr>(Original.getSourceRange(), nullptr, TokenKind::Dot, CalleeName);
              break;
          }
          const auto Arguments = AST.copyArray<Argument>(std::vector<Argument>{Argument(Form, ArgumentName, Value, OriginalArgument.range())});
          return AST.make<CallExpr>(Original.getSourceRange(), Callee, Arguments, false);
        }

        void unregister(ir::Module &Module)
        {
          auto &Scopes = Context.scopeStore();
          for (const auto &Function : Module.globalFunctions())
          {
            for (const auto &Parameter : Function->parameters())
            {
              Scopes.removeTarget(*Parameter);
            }
            Scopes.removeTarget(*Function);
          }
          for (const auto &Variable : Module.globalVariables())
          {
            Scopes.removeTarget(*Variable);
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

  // Earlier declarations are callable directly and through parentheses with typed positional arguments.
  TEST_F(SemanticCallTest, CallsEarlierFunctionDirectlyAndThroughParentheses)
  {
    auto Input = read("func identity(Value: i32): i32 { return Value; } func main(): i32 { identity(1); return (identity)(2); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Identity = function("identity");
    auto *Main = function("main");
    ASSERT_NE(Identity, nullptr);
    ASSERT_NE(Main, nullptr);
    ASSERT_EQ(Main->entryBlock()->instructionCount(), 3U);
    for (std::size_t Index = 0; Index < 2; ++Index)
    {
      const auto *Call = call(*Main, Index);
      ASSERT_NE(Call, nullptr);
      EXPECT_EQ(Call->directCallee(), Identity);
      EXPECT_EQ(Call->callee(), Identity);
      EXPECT_EQ(Call->functionType(), Identity->functionType());
      EXPECT_EQ(Call->type(), Context.typePool().getInt32Type());
      ASSERT_EQ(Call->argumentCount(), 1U);
      EXPECT_FALSE(Call->argument(0)->hasName());
      ASSERT_TRUE(ir::IntegerConstant::classof(Call->argument(0)->value()));
      EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Call->argument(0)->value())->bits(), Index + 1U);
    }
    ASSERT_NE(returned(*Main), nullptr);
    EXPECT_EQ(returned(*Main)->returnedValue(), call(*Main, 1));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Native imports lower to a direct call while the imported declaration remains bodyless.
  TEST_F(SemanticCallTest, CallsNativeCImport)
  {
    auto Input = read("import \"C\" func external(Value: i32): i32; func main(): i32 { return external(42); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *External = function("external");
    auto *Main = function("main");
    ASSERT_NE(External, nullptr);
    ASSERT_NE(Main, nullptr);
    EXPECT_TRUE(External->isDeclaration());
    EXPECT_EQ(External->languageLinkage(), ir::LanguageLinkage::C);
    const auto *Call = call(*Main, 0);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), External);
    EXPECT_EQ(Call->functionType(), External->functionType());
    ASSERT_NE(returned(*Main), nullptr);
    EXPECT_EQ(returned(*Main)->returnedValue(), Call);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Native call arguments and return expressions share read-only string storage while each function owns its address instructions.
  TEST_F(SemanticCallTest, ConvertsStringArgumentsAndReturnsWithSharedStorage)
  {
    auto Input = read("import \"C\" func consume(Value: *const u8): i32; func first(): i32 { return consume(((\"abc\"))); } func second(): *const u8 { return \"abc\"; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const auto *First = function("first");
    const auto *Second = function("second");
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_EQ(Output.globalVariableCount(), 1U);
    const auto *Storage = Output.globalVariable(0);
    ASSERT_NE(Storage, nullptr);
    EXPECT_FALSE(Storage->isMutable());
    EXPECT_EQ(Storage->initializer(), Output.constantPool().getStringConstant("abc"));
    ASSERT_EQ(First->entryBlock()->instructionCount(), 4U);
    ASSERT_EQ(Second->entryBlock()->instructionCount(), 3U);
    const auto *Call = call(*First, 2);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), function("consume"));
    ASSERT_EQ(Call->argumentCount(), 1U);
    ASSERT_NE(returned(*First), nullptr);
    EXPECT_EQ(returned(*First)->returnedValue(), Call);
    ASSERT_NE(returned(*Second), nullptr);
    struct Case
    {
        const ir::Function *Function;
        const ir::Value *Value;
    };
    const Case Cases[] = {
        {First, Call->argument(0)->value()},
        {Second, returned(*Second)->returnedValue()},
    };
    for (const auto &Entry : Cases)
    {
      ASSERT_TRUE(ir::GetElementPtrInstruction::classof(Entry.Value));
      const auto *ElementAddress = static_cast<const ir::GetElementPtrInstruction *>(Entry.Value);
      EXPECT_EQ(ElementAddress->type(), Context.typePool().getPointerType(Context.typePool().getUInt8Type(false)));
      EXPECT_EQ(ElementAddress->parentBlock(), Entry.Function->entryBlock());
      ASSERT_TRUE(ir::AddressOfInstruction::classof(ElementAddress->pointer()));
      const auto *ArrayAddress = static_cast<const ir::AddressOfInstruction *>(ElementAddress->pointer());
      EXPECT_EQ(ArrayAddress->storage(), Storage);
      EXPECT_EQ(ArrayAddress->parentBlock(), Entry.Function->entryBlock());
      EXPECT_EQ(Entry.Function->entryBlock()->instruction(0), ArrayAddress);
      EXPECT_EQ(Entry.Function->entryBlock()->instruction(1), ElementAddress);
    }
    EXPECT_NE(Call->argument(0)->value(), returned(*Second)->returnedValue());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Function values and pointers to function signatures retain their explicit indirect-call signature.
  TEST_F(SemanticCallTest, CallsFunctionAndFunctionPointerParameters)
  {
    auto Input = read("func identity(Value: i32): i32 { return Value; } func invoke(Callback: func(i32): i32, Value: i32): i32 { return Callback(Value); } func invokePointer(Callback: *func(i32): i32, Value: i32): i32 { return Callback(Value); } func main(): i32 { return invoke(identity, 7); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_NE(function("identity"), nullptr);
    for (const char *Name : {"invoke", "invokePointer"})
    {
      auto *Invoker = function(Name);
      ASSERT_NE(Invoker, nullptr);
      const auto *Call = call(*Invoker, 0);
      ASSERT_NE(Call, nullptr);
      EXPECT_EQ(Call->directCallee(), nullptr);
      EXPECT_EQ(Call->callee(), Invoker->parameter(0));
      EXPECT_EQ(Call->functionType(), function("identity")->functionType());
      ASSERT_EQ(Call->argumentCount(), 1U);
      EXPECT_EQ(Call->argument(0)->value(), Invoker->parameter(1));
      ASSERT_NE(returned(*Invoker), nullptr);
      EXPECT_EQ(returned(*Invoker)->returnedValue(), Call);
    }
    ASSERT_NE(function("main"), nullptr);
    const auto *MainCall = call(*function("main"), 0);
    ASSERT_NE(MainCall, nullptr);
    EXPECT_EQ(MainCall->directCallee(), function("invoke"));
    EXPECT_EQ(MainCall->argument(0)->value(), function("identity"));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A callee-producing call and nested positional arguments lower once each in source order before the outer call.
  TEST_F(SemanticCallTest, EvaluatesCalleeAndArgumentsOnceInSourceOrder)
  {
    auto Input = read("func combine(First: i32, Second: i32): i32 { return First; } func make(): func(i32, i32): i32 { return combine; } func first(): i32 { return 1; } func second(): i32 { return 2; } func main(): i32 { return make()(first(), second()); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Main = function("main");
    ASSERT_NE(Main, nullptr);
    ASSERT_EQ(Main->entryBlock()->instructionCount(), 5U);
    const auto *Make = call(*Main, 0);
    const auto *First = call(*Main, 1);
    const auto *Second = call(*Main, 2);
    const auto *Outer = call(*Main, 3);
    ASSERT_NE(Make, nullptr);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Outer, nullptr);
    EXPECT_EQ(Make->directCallee(), function("make"));
    EXPECT_EQ(First->directCallee(), function("first"));
    EXPECT_EQ(Second->directCallee(), function("second"));
    EXPECT_EQ(Outer->directCallee(), nullptr);
    EXPECT_EQ(Outer->callee(), Make);
    EXPECT_EQ(Outer->functionType(), function("combine")->functionType());
    ASSERT_EQ(Outer->argumentCount(), 2U);
    EXPECT_EQ(Outer->argument(0)->value(), First);
    EXPECT_EQ(Outer->argument(1)->value(), Second);
    ASSERT_NE(returned(*Main), nullptr);
    EXPECT_EQ(returned(*Main)->returnedValue(), Outer);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Void calls work as statements and as unit values supplied to fixed unit parameters.
  TEST_F(SemanticCallTest, AcceptsVoidStatementsAndUnitArguments)
  {
    auto Input = read("func unit(): void {} func consume(Value: void): void {} func main(): void { unit(); consume(unit()); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Main = function("main");
    ASSERT_NE(Main, nullptr);
    ASSERT_EQ(Main->entryBlock()->instructionCount(), 4U);
    ASSERT_NE(call(*Main, 0), nullptr);
    ASSERT_NE(call(*Main, 1), nullptr);
    const auto *Consume = call(*Main, 2);
    ASSERT_NE(Consume, nullptr);
    EXPECT_EQ(call(*Main, 0)->directCallee(), function("unit"));
    EXPECT_EQ(call(*Main, 1)->directCallee(), function("unit"));
    EXPECT_EQ(Consume->directCallee(), function("consume"));
    EXPECT_EQ(Consume->argument(0)->value(), call(*Main, 1));
    EXPECT_EQ(Consume->type(), Context.typePool().getVoidType());
    ASSERT_NE(returned(*Main), nullptr);
    EXPECT_FALSE(returned(*Main)->hasReturnedValue());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A function is bound before its body so a recursive call references the same owned function object.
  TEST_F(SemanticCallTest, BuildsSelfRecursiveReference)
  {
    auto Input = read("func recurse(Value: i32): i32 { return recurse(Value); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Recurse = function("recurse");
    ASSERT_NE(Recurse, nullptr);
    const auto *Call = call(*Recurse, 0);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), Recurse);
    EXPECT_EQ(Call->argument(0)->value(), Recurse->parameter(0));
    ASSERT_NE(returned(*Recurse), nullptr);
    EXPECT_EQ(returned(*Recurse)->returnedValue(), Call);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Single-pass analysis reports a later callee as unknown and still analyzes its later declaration.
  TEST_F(SemanticCallTest, DoesNotPredeclareLaterCallees)
  {
    auto Input = read("func first(): i32 { return later(); } func later(): i32 { return 1; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    ASSERT_NE(function("first"), nullptr);
    EXPECT_EQ(function("first")->entryBlock()->instructionCount(), 0U);
    ASSERT_NE(function("later"), nullptr);
    EXPECT_NE(returned(*function("later")), nullptr);
  }

  // Parameter types guide integer literal materialization and accept pooled null pointer values for pointer arguments.
  TEST_F(SemanticCallTest, TypesLiteralAndNullPointerArguments)
  {
    const auto *Null = Output.constantPool().getNullPointerConstant();
    ASSERT_EQ(NameResolver(Context).bind(name("Null"), const_cast<ir::NullPointerConstant &>(*Null)), BindResult::Inserted);
    auto Input = read("func byte(Value: u8): u8 { return Value; } func pointer(Value: *u8): void {} func main(): u8 { pointer(Null); return byte(255); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Main = function("main");
    ASSERT_NE(Main, nullptr);
    const auto *Pointer = call(*Main, 0);
    const auto *Byte = call(*Main, 1);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Byte, nullptr);
    EXPECT_EQ(Pointer->argument(0)->value(), Null);
    EXPECT_EQ(Byte->argument(0)->value()->type(), Context.typePool().getUInt8Type());
    ASSERT_TRUE(ir::IntegerConstant::classof(Byte->argument(0)->value()));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Byte->argument(0)->value())->bits(), 255U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Invalid calls retain user diagnostics at the call, argument or callee range and never append an invalid call.
  TEST_F(SemanticCallTest, DiagnosesInvalidCallsWithoutAppendingInstructions)
  {
    ASSERT_EQ(NameResolver(Context).bind(name("Package"), Output), BindResult::Inserted);
    const auto *Null = Output.constantPool().getNullPointerConstant();
    ASSERT_EQ(NameResolver(Context).bind(name("Null"), const_cast<ir::NullPointerConstant &>(*Null)), BindResult::Inserted);
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
        std::string_view Highlight;
    };
    const Case Cases[] = {
        {"func f(Value: i32): void {} func main(): void { f(); }", core::DiagnosticKind::SemanticArgumentCount, "f()"},
        {"func f(Value: i32): void {} func main(): void { f(1, 2); }", core::DiagnosticKind::SemanticArgumentCount, "f(1, 2)"},
        {"func f(Value: bool): void {} func main(): void { f(1); }", core::DiagnosticKind::SemanticTypeMismatch, "1"},
        {"func f(Value: u8): void {} func main(Value: i32): void { f(Value); }", core::DiagnosticKind::SemanticTypeMismatch, "Value"},
        {"func callback(Value: u8): u8 { return Value; } func invoke(Callback: func(i32): i32): void {} func main(): void { invoke(callback); }", core::DiagnosticKind::SemanticTypeMismatch, "callback"},
        {"func f(Value: u8): void {} func main(): void { f(256); }", core::DiagnosticKind::SemanticIntegerOutOfRange, "256"},
        {"func main(): void { true(); }", core::DiagnosticKind::SemanticTypeMismatch, "true"},
        {"func main(): void { 1(2); }", core::DiagnosticKind::SemanticTypeMismatch, "1"},
        {"func main(Value: *i32): void { Value(); }", core::DiagnosticKind::SemanticTypeMismatch, "Value"},
        {"func main(): void { Null(); }", core::DiagnosticKind::SemanticTypeMismatch, "Null"},
        {"func main(): void { Package(); }", core::DiagnosticKind::SemanticTypeMismatch, "Package"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      const auto Offset = std::string_view(Entry.Source).rfind(Entry.Highlight);
      ASSERT_NE(Offset, std::string_view::npos);
      ir::Module Destination(Context.irContext());
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Entry.Kind);
      EXPECT_EQ(Diagnostics.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Source, Input.Unit->input().lexedFile().sourceId());
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, core::SourceRange::fromByteOffsets(Offset, Offset + Entry.Highlight.size()));
      ASSERT_GT(Destination.globalFunctionCount(), 0U);
      auto *Main = Destination.globalFunction(Destination.globalFunctionCount() - 1);
      ASSERT_NE(Main, nullptr);
      EXPECT_EQ(Main->entryBlock()->instructionCount(), 0U);
      unregister(Destination);
    }
  }

  // A later argument failure preserves preceding argument evaluation while suppressing the invalid outer call.
  TEST_F(SemanticCallTest, DoesNotAppendOuterCallAfterArgumentFailure)
  {
    auto Input = read("func side(): i32 { return 1; } func consume(First: i32, Second: bool): void {} func main(): void { consume(side(), 1); }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    auto *Main = function("main");
    ASSERT_NE(Main, nullptr);
    ASSERT_EQ(Main->entryBlock()->instructionCount(), 1U);
    ASSERT_NE(call(*Main, 0), nullptr);
    EXPECT_EQ(call(*Main, 0)->directCallee(), function("side"));
    EXPECT_FALSE(Main->entryBlock()->hasTerminator());
  }

  // A resolved module member can be called without changing its declaration identity.
  TEST_F(SemanticCallTest, CallsSingleModuleMember)
  {
    auto Definitions = read("func identity(Value: i32): i32 { return Value; }");
    ASSERT_TRUE(Definitions.succeeded());
    ASSERT_TRUE(analyze(Definitions));
    ASSERT_EQ(NameResolver(Context).bind(name("Package"), Output), BindResult::Inserted);
    auto Input = read("func main(): i32 { return Package.identity(14); }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Main = function("main");
    ASSERT_NE(Main, nullptr);
    ASSERT_NE(call(*Main, 0), nullptr);
    EXPECT_EQ(call(*Main, 0)->directCallee(), function("identity"));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Named, spread, optional, generic and variadic calls remain explicit unsupported semantic paths.
  TEST_F(SemanticCallTest, ReportsUnsupportedCallForms)
  {
    struct Case
    {
        const char *Source;
        const char *Description;
    };
    const Case Cases[] = {
        {"func f(Value: i32): void {} func main(): void { f(Value = 1); }", "named call arguments"},
        {"func f(Value: i32): void {} func main(): void { f((1)...); }", "spread call arguments"},
        {"func f(): void {} func main(): void { f?.(); }", "optional calls"},
        {"func f(Value: i32): void {} func main(): void { f::[i32](1); }", "GenericApplyExpr"},
        {"func invoke(Callback: func(i32...): void): void { Callback(1); }", "variadic calls"},
        {"func f(Value: i32...): void {}", "FunctionDecl"},
        {"func f(Value: i32 = 1): void {}", "FunctionDecl"},
        {"func f(): void {} f();", "module-level calls"},
        {"func main(): void { i32(1); }", "type construction and conversion calls"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), std::string("internal compiler error\\[INK-S0012\\]: initial semantic analyzer does not support ") + Entry.Description);
    }
  }

  // Overloaded names remain unsupported as direct, parenthesized and member callees until overload selection is implemented.
  TEST_F(SemanticCallTest, ReportsUnsupportedOverloadSelection)
  {
    auto Definitions = read("func choose(Value: i32): void {} func choose(Value: u8): void {}");
    ASSERT_TRUE(Definitions.succeeded());
    ASSERT_TRUE(analyze(Definitions));
    ASSERT_EQ(NameResolver(Context).bind(name("Package"), Output), BindResult::Inserted);
    for (const char *Source : {"func main(): void { choose(1); }", "func main(): void { (choose)(1); }", "func main(): void { Package.choose(1); }"})
    {
      SCOPED_TRACE(Source);
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "internal compiler error\\[INK-S0012\\]: initial semantic analyzer does not support function overload selection");
    }
  }

  // Reading global storage as a callee or argument uses the shared unsupported-storage diagnostic.
  TEST_F(SemanticCallTest, ReportsUnsupportedGlobalStorageReads)
  {
    auto &Types = Context.typePool();
    auto *CallableStorage = Output.createGlobalVariable(*Types.getFunctionType(Types.getVoidType()), name("CallableStorage"));
    auto *Stored = Output.createGlobalVariable(*Types.getInt32Type(), name("Stored"));
    ASSERT_NE(CallableStorage, nullptr);
    ASSERT_NE(Stored, nullptr);
    ASSERT_EQ(NameResolver(Context).bind(name("CallableStorage"), *CallableStorage), BindResult::Inserted);
    ASSERT_EQ(NameResolver(Context).bind(name("Stored"), *Stored), BindResult::Inserted);
    const char *Cases[] = {
        "func main(): void { CallableStorage(); }",
        "func consume(Value: i32): void {} func main(): void { consume(Stored); }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "internal compiler error\\[INK-S0012\\]: initial semantic analyzer does not support variable storage reads");
    }
  }

  // Malformed calls and malformed callee wrappers retain their specific AST invariant instead of reaching call construction.
  TEST_F(SemanticCallTest, ReportsMalformedCallASTWithConcreteReason)
  {
    struct Case
    {
        MalformedCall Failure;
        const char *Description;
    };
    const Case Cases[] = {
        {MalformedCall::MissingCallee, "CallExpr requires a callee expression"},
        {MalformedCall::MissingArgumentValue, "call argument requires a value expression"},
        {MalformedCall::InvalidArgumentKind, "call argument kind must be Named, Positional or SpreadPositional"},
        {MalformedCall::MissingArgumentName, "named call argument requires a name"},
        {MalformedCall::EmptyArgumentName, "named call argument requires a nonempty name"},
        {MalformedCall::UnexpectedPositionalName, "positional or spread call argument must not have a name"},
        {MalformedCall::UnexpectedSpreadName, "positional or spread call argument must not have a name"},
        {MalformedCall::MissingParenthesizedCallee, "ParenExpr requires an expression"},
        {MalformedCall::MissingMemberObject, "MemberExpr requires an object expression"},
    };
    auto &Types = Context.typePool();
    auto *Target = Output.createGlobalFunction(*Types.getFunctionType(Types.getInt32Type(), {ir::FunctionTypeParameter(Types.getInt32Type())}), name("Target"));
    auto *Caller = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType()));
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(Caller, nullptr);
    auto *Block = Caller->createBlock();
    ASSERT_NE(Block, nullptr);
    ASSERT_EQ(NameResolver(Context).bind(name("Target"), *Target), BindResult::Inserted);
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Description);
      auto Input = read("var Result = Target(1);");
      ASSERT_TRUE(Input.succeeded());
      const auto *Node = malformedCall(Input, Entry.Failure);
      ASSERT_NE(Node, nullptr);
      AnalyzerState State(Context, Input, Output);
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
      Context.setSource(Input.Unit->input().lexedFile().sourceId());
      EXPECT_DEATH(Analysis.emitValue(State, *Node), std::string("internal compiler error\\[INK-S0061\\]: invalid AST: ") + Entry.Description);
      EXPECT_EQ(Block->instructionCount(), 0U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // A valid call in a terminated insertion block reports the semantic construction ICE before the builder assertion.
  TEST_F(SemanticCallTest, ReportsTerminatedInsertionStateAsConstructionFailure)
  {
    auto Input = read("var Result = Target(1);");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    auto *Target = Output.createGlobalFunction(*Types.getFunctionType(Types.getInt32Type(), {ir::FunctionTypeParameter(Types.getInt32Type())}), name("Target"));
    auto *Caller = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType()));
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(Caller, nullptr);
    auto *Block = Caller->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_NE(State.irBuilder().createReturn(), nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Target"), *Target), BindResult::Inserted);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    EXPECT_DEATH(Analysis.emitValue(State, callSyntax(Input)), "internal compiler error\\[INK-S0013\\]");
    EXPECT_EQ(Block->instructionCount(), 1U);
  }
} // namespace ink::semantic::test
