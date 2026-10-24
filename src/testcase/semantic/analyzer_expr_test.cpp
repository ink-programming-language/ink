#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/config/numeric_literal.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/numeric/integer_bits.h"
#include "ink/core/support/core_define.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/instruction/address_of_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/dereference_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/context.h"
#include "../core/support/environment_test_support.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  namespace
  {
    static_assert(std::is_same_v<decltype(std::declval<Analyzer &>().emitValue(std::declval<AnalyzerState &>(), std::declval<const frontend::parser::Expr &>())), const ir::Value *>);
    static_assert(std::is_same_v<decltype(std::declval<Analyzer &>().analyzeExpr(std::declval<AnalyzerState &>(), std::declval<const frontend::parser::Expr &>())), const ir::Value *>);

    class InvalidExpression final : public frontend::parser::Expr
    {
      public:
        explicit InvalidExpression(core::SourceRange Range)
            : Expr(frontend::parser::ASTKind::SimpleStmt, Range)
        {
        }
    };

    class SemanticAnalyzerExprTest : public testing::Test
    {
      protected:
        SemanticAnalyzerExprTest()
            : Frontend(Sources, DiagnosticEngine),
              Context(Sources, DiagnosticEngine, target::TargetContext::native()),
              Output(Context.irContext())
        {
          DiagnosticEngine.addConsumer(Diagnostics);
        }

        frontend::parser::ParseResult read(std::string_view Expression)
        {
          Diagnostics.clear();
          return frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "var Result = " + std::string(Expression) + ";"));
        }

        const frontend::parser::Expr &syntax(const frontend::parser::ParseResult &Input, std::size_t StatementIndex = 0)
        {
          const auto *Statement = frontend::parser::cast<frontend::parser::DeclStmt>(Input.Unit->root()->statements()[StatementIndex]);
          return *frontend::parser::cast<frontend::parser::VarDecl>(Statement->declaration())->initializer();
        }

        const ir::Value *analyze(const frontend::parser::ParseResult &Input, ExprHints Hints = {})
        {
          AnalyzerState State(Context, Input, Output);
          if (!createInsertBlock(State))
          {
            return nullptr;
          }
          return analyze(State, Input, Hints);
        }

        ir::BasicBlock *createInsertBlock(AnalyzerState &State)
        {
          auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
          EXPECT_NE(Function, nullptr);
          if (!Function)
          {
            return nullptr;
          }
          auto *Block = Function->createBlock();
          EXPECT_NE(Block, nullptr);
          if (!Block || !State.irBuilder().setInsertPoint(*Block))
          {
            return nullptr;
          }
          return Block;
        }

        const ir::Value *analyze(AnalyzerState &State, const frontend::parser::ParseResult &Input, ExprHints Hints = {})
        {
          Context.setSource(Input.Unit->input().lexedFile().sourceId());
          return Analysis.emitValue(State, syntax(Input), Hints);
        }

        const ir::Value *analyzeExpr(AnalyzerState &State, const frontend::parser::ParseResult &Input, ExprHints Hints = {})
        {
          Context.setSource(Input.Unit->input().lexedFile().sourceId());
          return Analysis.analyzeExpr(State, syntax(Input), Hints);
        }

        void expectNegation(const ir::Value *Result, const ir::Value *Operand)
        {
          ASSERT_NE(Result, nullptr);
          ASSERT_NE(Operand, nullptr);
          ASSERT_TRUE(ir::NegInstruction::classof(Result));
          EXPECT_EQ(Result->type(), Operand->type());
          EXPECT_EQ(static_cast<const ir::NegInstruction *>(Result)->operand(), Operand);
        }

        void expectFailure(const ir::Value *Result, core::DiagnosticKind Kind)
        {
          EXPECT_EQ(Result, nullptr);
          ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
          EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Kind);
        }

        void expectUninitializedWarning(const frontend::parser::ParseResult &Input, core::SourceRange Range)
        {
          ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
          const auto &Diagnostic = Diagnostics.diagnostics()[0];
          EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticUninitializedRead);
          EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostic).Severity, core::DiagnosticSeverity::Warning);
          EXPECT_EQ(Diagnostic.Source, Input.Unit->input().lexedFile().sourceId());
          EXPECT_EQ(Diagnostic.Span, Range);
        }

        void expectMessage(std::string_view Message)
        {
          ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
          EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostics.diagnostics()[0]).Message, Message);
        }

        static core::Name name(std::string_view Text)
        {
          return core::NamePool::instance().intern(Text);
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

  // Shared expression analysis returns an uninitialized variable's original storage and object type without reading it.
  TEST_F(SemanticAnalyzerExprTest, AnalyzesVariableStorageWithoutReading)
  {
    auto Input = read("((Stored))");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    auto *Block = createInsertBlock(State);
    ASSERT_NE(Block, nullptr);
    const auto *DeclaredType = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Storage = State.irBuilder().createAlloca(*DeclaredType);
    ASSERT_NE(Storage, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    const auto *Result = Analysis.analyzeExpr(State, syntax(Input), {.ExpectedType = DeclaredType});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Storage);
    EXPECT_EQ(Result->type(), DeclaredType);
    EXPECT_TRUE(Result->isStorage());
    EXPECT_TRUE(Result->isMutable());
    const auto *Variable = State.nameResolver().lookupLocal(name("Stored"));
    ASSERT_NE(Variable, nullptr);
    EXPECT_FALSE(Result->isInitialized());
    EXPECT_EQ(Block->instructionCount(), 1U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Context.scopeStore().removeTarget(*Storage);
  }

  // Values, type descriptors and module descriptors retain their borrowed IR identities through transparent parentheses.
  TEST_F(SemanticAnalyzerExprTest, PreservesValueTypeAndModuleIdentities)
  {
    auto &Types = Context.typePool();
    ir::Module Package(Context.irContext());
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    const auto *Integer = Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Integer, nullptr);
    struct Case
    {
        const char *Text;
        const ir::Value *Value;
    };
    const Case Cases[] = {
        {"14", Integer},
        {"((14))", Integer},
        {"i32", Types.get<ink::ir::IntegerType>(32, true)},
        {"((i32))", Types.get<ink::ir::IntegerType>(32, true)},
        {"*i32", Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true))},
        {"Pkg", &Package},
        {"((Pkg))", &Package},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      Context.setSource(Input.Unit->input().lexedFile().sourceId());
      EXPECT_EQ(Analysis.analyzeExpr(State, syntax(Input)), Entry.Value);
      EXPECT_EQ(analyze(State, Input), Entry.Value);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    Context.scopeStore().removeTarget(Package);
  }

  // Emitting a pointer-valued function parameter returns the pointer itself without inferring a load from its type.
  TEST_F(SemanticAnalyzerExprTest, EmitsPointerParametersWithoutLoading)
  {
    auto Input = read("((Argument))");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const auto *PointerType = Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, false));
    const auto *Signature = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(PointerType)});
    ASSERT_NE(Signature, nullptr);
    auto *Function = Output.createGlobalFunction(*Signature);
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Function->parameter(0)), BindResult::Inserted);
    const auto *Result = analyze(State, Input, {.ExpectedType = PointerType});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Function->parameter(0));
    EXPECT_EQ(Result->type(), PointerType);
    EXPECT_EQ(Block->instructionCount(), 0U);
    Context.scopeStore().removeTarget(*Function->parameter(0));
  }

  // Nested parentheses preserve variable storage without reading it and emit exactly one load when its value is requested.
  TEST_F(SemanticAnalyzerExprTest, EmitsOneLoadThroughNestedParentheses)
  {
    auto Input = read("(((Stored)))");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    auto *Block = createInsertBlock(State);
    ASSERT_NE(Block, nullptr);
    const auto *DeclaredType = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Storage = State.irBuilder().createAlloca(*DeclaredType);
    const auto *InitialValue = Output.constantPool().get<ink::ir::IntegerConstant>(DeclaredType, static_cast<std::int64_t>(14));
    ASSERT_NE(Storage, nullptr);
    ASSERT_NE(InitialValue, nullptr);
    ASSERT_NE(State.irBuilder().createInitialize(*InitialValue, *Storage), nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
    const auto *Variable = State.nameResolver().lookupLocal(name("Stored"));
    ASSERT_NE(Variable, nullptr);
    const auto *Address = analyzeExpr(State, Input);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address, Storage);
    EXPECT_EQ(Address->type(), Storage->type());
    EXPECT_EQ(State.nameResolver().lookupLocal(name("Stored")), Variable);
    EXPECT_EQ(Address->type(), DeclaredType);
    EXPECT_TRUE(Address->isStorage());
    EXPECT_EQ(Block->instructionCount(), 2U);
    const auto *Result = analyze(State, Input);
    ASSERT_NE(Result, nullptr);
    ASSERT_TRUE(ir::LoadInstruction::classof(Result));
    EXPECT_EQ(Result->type(), DeclaredType);
    EXPECT_FALSE(Result->isStorage());
    EXPECT_FALSE(Result->isMutable());
    EXPECT_EQ(static_cast<const ir::LoadInstruction *>(Result)->pointer(), Storage);
    EXPECT_EQ(Block->instructionCount(), 3U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Context.scopeStore().removeTarget(*Storage);
  }

  // Taking a storage address produces a separate pointer value that can be emitted without reading the uninitialized object.
  TEST_F(SemanticAnalyzerExprTest, DistinguishesStorageFromItsAddressValue)
  {
    auto Input = read("((Stored)); var AddressResult = ((Address))");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_EQ(Input.Unit->root()->statements().size(), 2U);
    AnalyzerState State(Context, Input, Output);
    auto *Block = createInsertBlock(State);
    ASSERT_NE(Block, nullptr);
    const auto *DeclaredType = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Storage = State.irBuilder().createAlloca(*DeclaredType);
    ASSERT_NE(Storage, nullptr);
    auto *Address = State.irBuilder().createAddressOf(*Storage);
    ASSERT_NE(Address, nullptr);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Stored"), *Storage), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Address"), *Address), BindResult::Inserted);
    const auto *Variable = Resolver.lookupLocal(name("Stored"));
    ASSERT_NE(Variable, nullptr);
    EXPECT_EQ(Storage->type(), DeclaredType);
    EXPECT_TRUE(Storage->isMutable());
    const auto *VariableResult = analyzeExpr(State, Input, {.ExpectedType = DeclaredType});
    const auto PointerResult = Analysis.emitValue(State, syntax(Input, 1), {.ExpectedType = Address->type()});
    ASSERT_TRUE(VariableResult);
    ASSERT_TRUE(PointerResult);
    EXPECT_EQ(VariableResult, Storage);
    EXPECT_EQ(Resolver.lookupLocal(name("Stored")), Variable);
    EXPECT_EQ(VariableResult->type(), DeclaredType);
    EXPECT_TRUE(VariableResult->isStorage());
    EXPECT_EQ(PointerResult, Address);
    EXPECT_EQ(PointerResult->type(), Context.typePool().get<ink::ir::PointerType>(DeclaredType));
    EXPECT_FALSE(PointerResult->isStorage());
    EXPECT_EQ(Block->instructionCount(), 2U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Context.scopeStore().removeTarget(*Storage);
    Context.scopeStore().removeTarget(*Address);
  }

  // A saved scope preserves initialized variable identity across traversals and permits loading its stored value in a new analyzer state.
  TEST_F(SemanticAnalyzerExprTest, ReusesVariableBindingsAcrossAnalyzerStates)
  {
    auto Input = read("((Stored)); var ReadResult = -Stored");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_EQ(Input.Unit->root()->statements().size(), 2U);
    AnalyzerState FirstState(Context, Input, Output);
    auto *Block = createInsertBlock(FirstState);
    ASSERT_NE(Block, nullptr);
    const auto *DeclaredType = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Storage = FirstState.irBuilder().createAlloca(*DeclaredType, false);
    ASSERT_NE(Storage, nullptr);
    const auto *InitialValue = Output.constantPool().get<ink::ir::IntegerConstant>(DeclaredType, static_cast<std::int64_t>(14));
    ASSERT_NE(InitialValue, nullptr);
    ASSERT_NE(FirstState.irBuilder().createInitialize(*InitialValue, *Storage), nullptr);
    Scope &SavedScope = FirstState.nameResolver().enterScope();
    ASSERT_EQ(FirstState.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
    const auto *Variable = FirstState.nameResolver().lookupLocal(name("Stored"));
    ASSERT_NE(Variable, nullptr);
    EXPECT_EQ(Storage->type(), Context.typePool().get(DeclaredType, false));
    EXPECT_FALSE(Storage->isMutable());
    EXPECT_TRUE(Storage->isInitialized());
    const auto *InitialResult = analyzeExpr(FirstState, Input);
    ASSERT_NE(InitialResult, nullptr);
    EXPECT_EQ(InitialResult, Storage);
    EXPECT_EQ(InitialResult->type(), Storage->type());
    EXPECT_EQ(FirstState.nameResolver().lookupLocal(name("Stored")), Variable);
    ASSERT_TRUE(FirstState.nameResolver().exitScope());
    EXPECT_EQ(FirstState.nameResolver().lookup(name("Stored")), nullptr);
    ASSERT_TRUE(FirstState.nameResolver().enterScope(SavedScope));
    const auto *ReenteredResult = analyzeExpr(FirstState, Input);
    ASSERT_NE(ReenteredResult, nullptr);
    EXPECT_EQ(ReenteredResult, Storage);
    EXPECT_EQ(ReenteredResult->type(), Storage->type());
    EXPECT_EQ(FirstState.nameResolver().lookupLocal(name("Stored")), Variable);
    ASSERT_TRUE(FirstState.nameResolver().exitScope());
    AnalyzerState SecondState(Context, Input, Output);
    ASSERT_TRUE(SecondState.irBuilder().setInsertPoint(*Block));
    ASSERT_TRUE(SecondState.nameResolver().enterScope(SavedScope));
    const auto *ReusedResult = analyzeExpr(SecondState, Input, {.ExpectedType = DeclaredType});
    ASSERT_NE(ReusedResult, nullptr);
    EXPECT_EQ(ReusedResult, Storage);
    EXPECT_EQ(ReusedResult->type(), Storage->type());
    EXPECT_EQ(SecondState.nameResolver().lookupLocal(name("Stored")), Variable);
    const auto ReadResult = Analysis.emitValue(SecondState, syntax(Input, 1));
    ASSERT_TRUE(ReadResult);
    EXPECT_EQ(ReadResult->type(), DeclaredType);
    ASSERT_TRUE(ir::NegInstruction::classof(ReadResult));
    const auto *LoadedValue = static_cast<const ir::NegInstruction *>(ReadResult)->operand();
    ASSERT_TRUE(ir::LoadInstruction::classof(LoadedValue));
    EXPECT_EQ(LoadedValue->type(), DeclaredType);
    EXPECT_EQ(static_cast<const ir::LoadInstruction *>(LoadedValue)->pointer(), Storage);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    ASSERT_TRUE(SecondState.nameResolver().exitScope());
    Context.scopeStore().removeTarget(*Storage);
  }

  // A saved scope preserves an uninitialized variable's storage while emitting its value in a new analyzer state warns and creates a load.
  TEST_F(SemanticAnalyzerExprTest, WarnsAndLoadsUninitializedVariablesAcrossAnalyzerStates)
  {
    auto Input = read("((Stored))");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState FirstState(Context, Input, Output);
    auto *Block = createInsertBlock(FirstState);
    ASSERT_NE(Block, nullptr);
    const auto *DeclaredType = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Storage = FirstState.irBuilder().createAlloca(*DeclaredType);
    ASSERT_NE(Storage, nullptr);
    Scope &SavedScope = FirstState.nameResolver().enterScope();
    ASSERT_EQ(FirstState.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
    const auto *Variable = FirstState.nameResolver().lookupLocal(name("Stored"));
    ASSERT_NE(Variable, nullptr);
    EXPECT_EQ(Storage->type(), DeclaredType);
    EXPECT_FALSE(Storage->isInitialized());
    const auto *InitialResult = analyzeExpr(FirstState, Input);
    ASSERT_NE(InitialResult, nullptr);
    EXPECT_EQ(InitialResult, Storage);
    EXPECT_EQ(InitialResult->type(), Storage->type());
    EXPECT_EQ(FirstState.nameResolver().lookupLocal(name("Stored")), Variable);
    ASSERT_TRUE(FirstState.nameResolver().exitScope());
    AnalyzerState SecondState(Context, Input, Output);
    ASSERT_TRUE(SecondState.irBuilder().setInsertPoint(*Block));
    ASSERT_TRUE(SecondState.nameResolver().enterScope(SavedScope));
    const auto *ReusedResult = analyzeExpr(SecondState, Input);
    ASSERT_NE(ReusedResult, nullptr);
    EXPECT_EQ(ReusedResult, Storage);
    EXPECT_EQ(ReusedResult->type(), Storage->type());
    EXPECT_EQ(SecondState.nameResolver().lookupLocal(name("Stored")), Variable);
    const auto *Read = analyze(SecondState, Input);
    ASSERT_TRUE(ir::LoadInstruction::classof(Read));
    EXPECT_EQ(Read->type(), DeclaredType);
    EXPECT_EQ(static_cast<const ir::LoadInstruction *>(Read)->pointer(), Storage);
    expectUninitializedWarning(Input, syntax(Input).getSourceRange());
    EXPECT_FALSE(Storage->isInitialized());
    ASSERT_EQ(Block->instructionCount(), 2U);
    EXPECT_EQ(Block->instruction(1), Read);
    ASSERT_TRUE(SecondState.nameResolver().exitScope());
    Context.scopeStore().removeTarget(*Storage);
  }

  // Multiple names for one storage object share its type, mutability and declaration initialization without binding metadata.
  TEST_F(SemanticAnalyzerExprTest, PreservesStoragePropertiesThroughAliases)
  {
    auto Input = read("Stored; var AliasResult = Alias");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    auto *Block = createInsertBlock(State);
    ASSERT_NE(Block, nullptr);
    auto &Types = Context.typePool();
    auto *Storage = State.irBuilder().createAlloca(*Types.get<ink::ir::IntegerType>(32, true), false);
    const auto *Constant = Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Storage, nullptr);
    ASSERT_NE(Constant, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
    ASSERT_EQ(State.nameResolver().bind(name("Alias"), *Storage), BindResult::Inserted);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    for (std::size_t Index = 0; Index != 2; ++Index)
    {
      const auto *Result = Analysis.analyzeExpr(State, syntax(Input, Index));
      ASSERT_EQ(Result, Storage);
      EXPECT_EQ(Result->type(), Types.get(Types.get<ink::ir::IntegerType>(32, true), false));
      EXPECT_TRUE(Result->isStorage());
      EXPECT_FALSE(Result->isMutable());
      EXPECT_FALSE(Result->isInitialized());
      const auto *Read = Analysis.emitValue(State, syntax(Input, Index));
      ASSERT_TRUE(ir::LoadInstruction::classof(Read));
      EXPECT_EQ(static_cast<const ir::LoadInstruction *>(Read)->pointer(), Storage);
      EXPECT_FALSE(Read->isStorage());
      EXPECT_FALSE(Storage->isInitialized());
      expectUninitializedWarning(Input, syntax(Input, Index).getSourceRange());
      EXPECT_EQ(Block->instructionCount(), Index + 2);
      Diagnostics.clear();
    }
    ASSERT_NE(State.irBuilder().createInitialize(*Constant, *Storage), nullptr);
    for (std::size_t Index = 0; Index != 2; ++Index)
    {
      const auto *Place = Analysis.analyzeExpr(State, syntax(Input, Index));
      ASSERT_EQ(Place, Storage);
      EXPECT_TRUE(Place->isInitialized());
      EXPECT_FALSE(Place->isMutable());
      const auto *Read = Analysis.emitValue(State, syntax(Input, Index));
      ASSERT_TRUE(ir::LoadInstruction::classof(Read));
      EXPECT_EQ(static_cast<const ir::LoadInstruction *>(Read)->pointer(), Storage);
      EXPECT_FALSE(Read->isStorage());
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Context.scopeStore().removeTarget(*Storage);
  }

  // Decimal leading zeroes and explicit binary, octal and hexadecimal forms produce canonical i32 constants.
  TEST_F(SemanticAnalyzerExprTest, InfersIntegerConstantsAcrossRadices)
  {
    struct Case
    {
        const char *Text;
        std::int64_t Value;
    };
    constexpr Case Cases[] = {
        {"0", 0},
        {"1", 1},
        {"14", 14},
        {"00014", 14},
        {"0b1110", 14},
        {"0O16", 14},
        {"0xE", 14},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Result = analyze(Input);
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result, Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(Entry.Value)));
      EXPECT_EQ(Result->type(), Context.typePool().get<ink::ir::IntegerType>(32, true));
      EXPECT_EQ(analyze(Input), Result);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Integer-literal defaults retain u64 width through parentheses without changing float, character or boolean literal types.
  TEST_F(SemanticAnalyzerExprTest, AppliesDefaultHintsOnlyToIntegerLiterals)
  {
    auto &Types = Context.typePool();
    auto &Constants = Output.constantPool();
    struct Case
    {
        const char *Text;
        const ir::Value *Expected;
    };
    const Case Cases[] = {
        {"4294967295", Constants.get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(4294967295ULL))},
        {"((18446744073709551615))", Constants.get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(std::numeric_limits<std::uint64_t>::max()))},
        {"1.5", Constants.get<ink::ir::FloatConstant>(1.5)},
        {"'a'", Constants.get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(97))},
        {"true", Constants.get<ink::ir::BoolConstant>(true)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      ASSERT_NE(Entry.Expected, nullptr);
      EXPECT_EQ(analyze(Input, {.DefaultIntegerLiteralType = Types.get<ink::ir::IntegerType>(64, false)}), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Expected types override integer defaults through parentheses, retaining destination overflow checks.
  TEST_F(SemanticAnalyzerExprTest, GivesExpectedTypesPrecedenceOverLiteralDefaults)
  {
    auto &Types = Context.typePool();
    auto Valid = read("((255))");
    auto Overflow = read("((256))");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Overflow.succeeded());
    const ExprHints Hints{.ExpectedType = Types.get<ink::ir::IntegerType>(8, false), .DefaultIntegerLiteralType = Types.get<ink::ir::IntegerType>(64, false)};
    EXPECT_EQ(analyze(Valid, Hints), Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(8, false), static_cast<std::uint64_t>(255)));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(Overflow, Hints), core::DiagnosticKind::SemanticIntegerOutOfRange);
    expectMessage("integer literal is out of range for u8");
  }

  // Parentheses preserve a narrow integer-literal default and its range instead of reverting to the ordinary i32 default.
  TEST_F(SemanticAnalyzerExprTest, PreservesNarrowIntegerDefaultsThroughParentheses)
  {
    auto &Types = Context.typePool();
    auto Valid = read("((255))");
    auto Overflow = read("((256))");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Overflow.succeeded());
    const ExprHints Hints{.DefaultIntegerLiteralType = Types.get<ink::ir::IntegerType>(8, false)};
    EXPECT_EQ(analyze(Valid, Hints), Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(8, false), static_cast<std::uint64_t>(255)));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(Overflow, Hints), core::DiagnosticKind::SemanticIntegerOutOfRange);
    expectMessage("integer literal is out of range for u8");
  }

  // Array counts choose their own u64 default instead of inheriting the outer expression's narrow integer default.
  TEST_F(SemanticAnalyzerExprTest, KeepsArrayCountHintsIndependentOfOuterExpressions)
  {
    auto Input = read("[u8; 256]");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const auto *Expected = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), 256);
    ASSERT_NE(Expected, nullptr);
    EXPECT_EQ(analyze(Input, {.DefaultIntegerLiteralType = Types.get<ink::ir::IntegerType>(8, false)}), Expected);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Unary operands, member receivers and repeated-array elements use independent defaults before reporting an invalid or unsupported operation.
  TEST_F(SemanticAnalyzerExprTest, ResetsLiteralHintsForDistinctOperandPositions)
  {
    for (const char *Text : {"*(256)", "&(256)"})
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      expectFailure(analyze(Input, {.DefaultIntegerLiteralType = Context.typePool().get<ink::ir::IntegerType>(8, false)}), Text[0] == '*' ? core::DiagnosticKind::SemanticInvalidDereference : core::DiagnosticKind::SemanticInvalidAddressOperand);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(Input).getSourceRange());
    }
    constexpr const char *Cases[] = {
        "((256)).Missing",
        "[256; 1]",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input, {.DefaultIntegerLiteralType = Context.typePool().get<ink::ir::IntegerType>(8, false)}), "INK-S0012");
    }
  }

  // Expected integer types accept their largest positive values, including the Windows handle bit pattern and u64 maximum.
  TEST_F(SemanticAnalyzerExprTest, AcceptsExpectedIntegerTypeBoundaries)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::IntegerType *Type;
        std::uint64_t Bits;
    };
    const Case Cases[] = {
        {"127", Types.get<ink::ir::IntegerType>(8, true), 127},
        {"255", Types.get<ink::ir::IntegerType>(8, false), 255},
        {"32767", Types.get<ink::ir::IntegerType>(16, true), 32767},
        {"65535", Types.get<ink::ir::IntegerType>(16, false), 65535},
        {"2147483647", Types.get<ink::ir::IntegerType>(32, true), 2147483647},
        {"4294967295", Types.get<ink::ir::IntegerType>(32, false), 4294967295ULL},
        {"0xFFFFFFF5", Types.get<ink::ir::IntegerType>(32, false), 0xFFFFFFF5ULL},
        {"9223372036854775807", Types.get<ink::ir::IntegerType>(64, true), 9223372036854775807ULL},
        {"18446744073709551615", Types.get<ink::ir::IntegerType>(64, false), std::numeric_limits<std::uint64_t>::max()},
        {"0xFFFFFFFFFFFFFFFF", Types.get<ink::ir::IntegerType>(64, false), std::numeric_limits<std::uint64_t>::max()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Result = analyze(Input, {.ExpectedType = Entry.Type});
      ASSERT_NE(Result, nullptr);
      const auto Bits = core::IntegerBits::fromUnsigned(Entry.Type->width(), Entry.Bits);
      ASSERT_TRUE(Bits.has_value());
      EXPECT_EQ(Result, Output.constantPool().get<ink::ir::IntegerConstant>(Entry.Type, *Bits));
      EXPECT_EQ(Result->type(), Entry.Type);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Integer conversion rejects destination overflow and numbers above uint64 without truncating their bits.
  TEST_F(SemanticAnalyzerExprTest, RejectsIntegerOverflow)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::Type *Type;
        const char *Message;
    };
    const Case Cases[] = {
        {"128", Types.get<ink::ir::IntegerType>(8, true), "integer literal is out of range for i8"},
        {"256", Types.get<ink::ir::IntegerType>(8, false), "integer literal is out of range for u8"},
        {"32768", Types.get<ink::ir::IntegerType>(16, true), "integer literal is out of range for i16"},
        {"65536", Types.get<ink::ir::IntegerType>(16, false), "integer literal is out of range for u16"},
        {"2147483648", nullptr, "integer literal is out of range for i32"},
        {"0xFFFFFFF5", nullptr, "integer literal is out of range for i32"},
        {"4294967296", Types.get<ink::ir::IntegerType>(32, false), "integer literal is out of range for u32"},
        {"9223372036854775808", Types.get<ink::ir::IntegerType>(64, true), "integer literal is out of range for i64"},
        {"18446744073709551616", Types.get<ink::ir::IntegerType>(64, false), "integer literal is out of range for u64"},
        {"0x10000000000000000", Types.get<ink::ir::IntegerType>(64, false), "integer literal is out of range for u64"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      expectFailure(analyze(Input, {.ExpectedType = Entry.Type}), core::DiagnosticKind::SemanticIntegerOutOfRange);
      expectMessage(Entry.Message);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(Input).getSourceRange());
    }
  }

  // Unary minus emits same-typed instructions for signed literal operands across widths, parentheses and nondecimal spellings.
  TEST_F(SemanticAnalyzerExprTest, AcceptsNegativeIntegerLiteralBoundaries)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::IntegerType *Type;
        std::int64_t Value;
    };
    const Case Cases[] = {
        {"-127", Types.get<ink::ir::IntegerType>(8, true), -std::numeric_limits<std::int8_t>::max()},
        {"(-127)", Types.get<ink::ir::IntegerType>(8, true), -std::numeric_limits<std::int8_t>::max()},
        {"-(127)", Types.get<ink::ir::IntegerType>(8, true), -std::numeric_limits<std::int8_t>::max()},
        {"-0b1111111", Types.get<ink::ir::IntegerType>(8, true), -std::numeric_limits<std::int8_t>::max()},
        {"-32767", Types.get<ink::ir::IntegerType>(16, true), -std::numeric_limits<std::int16_t>::max()},
        {"-((32767))", Types.get<ink::ir::IntegerType>(16, true), -std::numeric_limits<std::int16_t>::max()},
        {"-0o77777", Types.get<ink::ir::IntegerType>(16, true), -std::numeric_limits<std::int16_t>::max()},
        {"-2147483647", Types.get<ink::ir::IntegerType>(32, true), -std::numeric_limits<std::int32_t>::max()},
        {"-0x7FFFFFFF", Types.get<ink::ir::IntegerType>(32, true), -std::numeric_limits<std::int32_t>::max()},
        {"-9223372036854775807", Types.get<ink::ir::IntegerType>(64, true), -std::numeric_limits<std::int64_t>::max()},
        {"-((9223372036854775807))", Types.get<ink::ir::IntegerType>(64, true), -std::numeric_limits<std::int64_t>::max()},
        {"-0x7FFFFFFFFFFFFFFF", Types.get<ink::ir::IntegerType>(64, true), -std::numeric_limits<std::int64_t>::max()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Expected = Output.constantPool().get<ink::ir::IntegerConstant>(Entry.Type, static_cast<std::int64_t>(-Entry.Value));
      ASSERT_NE(Expected, nullptr);
      expectNegation(analyze(Input, {.ExpectedType = Entry.Type}), Expected);
      expectNegation(analyze(Input, {.DefaultIntegerLiteralType = Entry.Type}), Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    auto Input = read("-2147483647");
    ASSERT_TRUE(Input.succeeded());
    expectNegation(analyze(Input), Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max())));
    auto Narrow = read("-(127)");
    ASSERT_TRUE(Narrow.succeeded());
    expectNegation(analyze(Narrow, {.ExpectedType = Types.get<ink::ir::IntegerType>(8, true), .DefaultIntegerLiteralType = Types.get<ink::ir::IntegerType>(64, false)}), Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(8, true), static_cast<std::int64_t>(std::numeric_limits<std::int8_t>::max())));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Positive literal operands must fit before negation, so signed minimum literals and larger magnitudes still report integer overflow.
  TEST_F(SemanticAnalyzerExprTest, RejectsNegativeIntegerOverflow)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::IntegerType *Type;
    };
    const Case Cases[] = {
        {"-128", Types.get<ink::ir::IntegerType>(8, true)},
        {"-129", Types.get<ink::ir::IntegerType>(8, true)},
        {"-0x81", Types.get<ink::ir::IntegerType>(8, true)},
        {"-32768", Types.get<ink::ir::IntegerType>(16, true)},
        {"-32769", Types.get<ink::ir::IntegerType>(16, true)},
        {"-2147483648", Types.get<ink::ir::IntegerType>(32, true)},
        {"-2147483649", Types.get<ink::ir::IntegerType>(32, true)},
        {"-9223372036854775808", Types.get<ink::ir::IntegerType>(64, true)},
        {"-9223372036854775809", Types.get<ink::ir::IntegerType>(64, true)},
        {"-18446744073709551616", Types.get<ink::ir::IntegerType>(64, true)},
        {"-(-128)", Types.get<ink::ir::IntegerType>(8, true)},
        {"-(-32768)", Types.get<ink::ir::IntegerType>(16, true)},
        {"-(-2147483648)", Types.get<ink::ir::IntegerType>(32, true)},
        {"-(-9223372036854775808)", Types.get<ink::ir::IntegerType>(64, true)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      SCOPED_TRACE(Entry.Type->toString());
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      expectFailure(analyze(Input, {.ExpectedType = Entry.Type}), core::DiagnosticKind::SemanticIntegerOutOfRange);
      expectMessage("integer literal is out of range for " + Entry.Type->toString());
    }
  }

  // Integer zero, nonzero unsigned operands and repeated negation remain explicit instructions without constant folding.
  TEST_F(SemanticAnalyzerExprTest, LowersIntegerZeroAndRepeatedNegationWithoutFolding)
  {
    auto &Types = Context.typePool();
    const ir::IntegerType *IntegerTypes[] = {
        Types.get<ink::ir::IntegerType>(8, true),
        Types.get<ink::ir::IntegerType>(8, false),
        Types.get<ink::ir::IntegerType>(16, true),
        Types.get<ink::ir::IntegerType>(16, false),
        Types.get<ink::ir::IntegerType>(32, true),
        Types.get<ink::ir::IntegerType>(32, false),
        Types.get<ink::ir::IntegerType>(64, true),
        Types.get<ink::ir::IntegerType>(64, false),
    };
    for (const auto *Type : IntegerTypes)
    {
      SCOPED_TRACE(Type->toString());
      for (const char *Text : {"-((0))", "-1"})
      {
        auto Input = read(Text);
        ASSERT_TRUE(Input.succeeded());
        const auto Bits = core::IntegerBits::fromUnsigned(Type->width(), std::string_view(Text) == "-1" ? 1 : 0);
        ASSERT_TRUE(Bits.has_value());
        expectNegation(analyze(Input, {.ExpectedType = Type}), Output.constantPool().get<ink::ir::IntegerConstant>(Type, *Bits));
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
      }
    }
    auto Input = read("-(-127)");
    ASSERT_TRUE(Input.succeeded());
    const auto *Result = analyze(Input, {.ExpectedType = Types.get<ink::ir::IntegerType>(8, true)});
    ASSERT_NE(Result, nullptr);
    ASSERT_TRUE(ir::NegInstruction::classof(Result));
    const auto *Inner = static_cast<const ir::NegInstruction *>(Result)->operand();
    expectNegation(Result, Inner);
    expectNegation(Inner, Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(8, true), static_cast<std::int64_t>(127)));
    auto Unsigned = read("-18446744073709551615");
    ASSERT_TRUE(Unsigned.succeeded());
    expectNegation(analyze(Unsigned, {.ExpectedType = Types.get<ink::ir::IntegerType>(64, false)}), Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(std::numeric_limits<std::uint64_t>::max())));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Bound integer operands retain their declared types in negation instructions, including signed minima and nonzero unsigned constants.
  TEST_F(SemanticAnalyzerExprTest, LowersBoundIntegerConstantNegationWithoutFolding)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const ir::IntegerType *Type;
        std::int64_t Minimum;
    };
    const Case Cases[] = {
        {Types.get<ink::ir::IntegerType>(8, true), std::numeric_limits<std::int8_t>::min()},
        {Types.get<ink::ir::IntegerType>(16, true), std::numeric_limits<std::int16_t>::min()},
        {Types.get<ink::ir::IntegerType>(32, true), std::numeric_limits<std::int32_t>::min()},
        {Types.get<ink::ir::IntegerType>(64, true), std::numeric_limits<std::int64_t>::min()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Type->toString());
      for (std::int64_t Value : {std::int64_t{14}, std::int64_t{-14}, Entry.Minimum})
      {
        SCOPED_TRACE(Value);
        auto Input = read("-Count");
        ASSERT_TRUE(Input.succeeded());
        AnalyzerState State(Context, Input, Output);
        auto *Block = createInsertBlock(State);
        ASSERT_NE(Block, nullptr);
        State.nameResolver().enterScope();
        const auto *Constant = Output.constantPool().get<ink::ir::IntegerConstant>(Entry.Type, static_cast<std::int64_t>(Value));
        ASSERT_NE(Constant, nullptr);
        ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
        expectNegation(analyze(State, Input, {.DefaultIntegerLiteralType = Types.get<ink::ir::IntegerType>(64, false)}), Constant);
        EXPECT_EQ(Block->instructionCount(), 1U);
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
        ASSERT_TRUE(State.nameResolver().exitScope());
      }
    }
    for (std::uint64_t Value : {std::uint64_t{0}, std::uint64_t{1}})
    {
      auto Input = read("-Count");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      auto *Block = createInsertBlock(State);
      ASSERT_NE(Block, nullptr);
      State.nameResolver().enterScope();
      const auto *Constant = Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(16, false), static_cast<std::uint64_t>(Value));
      ASSERT_NE(Constant, nullptr);
      ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
      expectNegation(analyze(State, Input), Constant);
      EXPECT_EQ(Block->instructionCount(), 1U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // Negation of each numeric parameter type appends a same-typed instruction owned by the current block and usable as a return value.
  TEST_F(SemanticAnalyzerExprTest, LowersNumericParameterNegation)
  {
    auto &Types = Context.typePool();
    const ir::Type *NumericTypes[] = {
        Types.get<ink::ir::IntegerType>(8, true),
        Types.get<ink::ir::IntegerType>(8, false),
        Types.get<ink::ir::IntegerType>(16, true),
        Types.get<ink::ir::IntegerType>(16, false),
        Types.get<ink::ir::IntegerType>(32, true),
        Types.get<ink::ir::IntegerType>(32, false),
        Types.get<ink::ir::IntegerType>(64, true),
        Types.get<ink::ir::IntegerType>(64, false),
        Types.get<ink::ir::FloatType>(32),
        Types.get<ink::ir::FloatType>(64),
    };
    for (const auto *Type : NumericTypes)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-Argument");
      ASSERT_TRUE(Input.succeeded());
      const auto *Signature = Types.get<ink::ir::FunctionType>(Type, {ir::FunctionTypeParameter(Type)});
      auto *Function = Output.createGlobalFunction(*Signature);
      ASSERT_NE(Function, nullptr);
      auto *Block = Function->createBlock();
      ASSERT_NE(Block, nullptr);
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Function->parameter(0)), BindResult::Inserted);
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
      const auto *Result = analyze(State, Input, {.ExpectedType = Type});
      ASSERT_NE(Result, nullptr);
      ASSERT_TRUE(ir::NegInstruction::classof(Result));
      const auto *Negation = static_cast<const ir::NegInstruction *>(Result);
      EXPECT_EQ(Negation->type(), Type);
      EXPECT_EQ(Negation->operand(), Function->parameter(0));
      ASSERT_EQ(Block->instructionCount(), 1U);
      EXPECT_EQ(Block->instruction(0), Negation);
      EXPECT_EQ(State.irBuilder().insertFunction(), Function);
      EXPECT_EQ(State.irBuilder().insertBlock(), Block);
      const auto *Return = State.irBuilder().createReturn(*Result);
      ASSERT_NE(Return, nullptr);
      EXPECT_EQ(Return->returnedValue(), Result);
      EXPECT_EQ(Block->instructionCount(), 2U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // Existing numeric instruction results can be negated repeatedly and consumed by calls, integer comparisons and returns.
  TEST_F(SemanticAnalyzerExprTest, LowersChainedNegationOfInstructionResults)
  {
    auto &Types = Context.typePool();
    const ir::Type *NumericTypes[] = {Types.get<ink::ir::IntegerType>(32, true), Types.get<ink::ir::FloatType>(64)};
    for (const auto *Type : NumericTypes)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-(-Produced)");
      ASSERT_TRUE(Input.succeeded());
      auto *Function = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Type));
      auto *Producer = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Type));
      auto *Consumer = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Type)}));
      ASSERT_NE(Function, nullptr);
      ASSERT_NE(Producer, nullptr);
      ASSERT_NE(Consumer, nullptr);
      auto *Block = Function->createBlock();
      ASSERT_NE(Block, nullptr);
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
      auto *Produced = State.irBuilder().createCall(*Producer);
      ASSERT_NE(Produced, nullptr);
      ASSERT_EQ(State.nameResolver().bind(name("Produced"), *Produced), BindResult::Inserted);
      const auto *Result = analyze(State, Input);
      ASSERT_NE(Result, nullptr);
      ASSERT_TRUE(ir::NegInstruction::classof(Result));
      const auto *Outer = static_cast<const ir::NegInstruction *>(Result);
      ASSERT_TRUE(ir::NegInstruction::classof(Outer->operand()));
      const auto *Inner = static_cast<const ir::NegInstruction *>(Outer->operand());
      EXPECT_EQ(Inner->operand(), Produced);
      EXPECT_EQ(Inner->type(), Type);
      EXPECT_EQ(Outer->type(), Type);
      ASSERT_EQ(Block->instructionCount(), 3U);
      EXPECT_EQ(Block->instruction(0), Produced);
      EXPECT_EQ(Block->instruction(1), Inner);
      EXPECT_EQ(Block->instruction(2), Outer);
      const auto *Call = State.irBuilder().createCall(*Consumer, {ir::Argument(*Result)});
      ASSERT_NE(Call, nullptr);
      EXPECT_EQ(Call->argument(0)->value(), Result);
      if (ir::IntegerType::classof(Type))
      {
        const auto *Compare = State.irBuilder().createICmp(ir::ICmpPredicate::Equal, *Result, *Produced);
        ASSERT_NE(Compare, nullptr);
        EXPECT_EQ(Compare->left(), Result);
        EXPECT_EQ(Compare->right(), Produced);
      }
      const auto *Return = State.irBuilder().createReturn(*Result);
      ASSERT_NE(Return, nullptr);
      EXPECT_EQ(Return->returnedValue(), Result);
      EXPECT_EQ(Block->instructionCount(), ir::IntegerType::classof(Type) ? 6U : 5U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // Foreign-context operands, another function's values and detached instructions violate the negation analyzer's model contract.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForForeignAndDetachedNegationOperands)
  {
    auto &Types = Context.typePool();
    const auto *Signature = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Types.get<ink::ir::IntegerType>(32, true))});
    auto *Current = Output.createGlobalFunction(*Signature);
    auto *Other = Output.createGlobalFunction(*Signature);
    auto *Producer = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::IntegerType>(32, true)));
    ASSERT_NE(Current, nullptr);
    ASSERT_NE(Other, nullptr);
    ASSERT_NE(Producer, nullptr);
    auto *CurrentBlock = Current->createBlock();
    auto *OtherBlock = Other->createBlock();
    ASSERT_NE(CurrentBlock, nullptr);
    ASSERT_NE(OtherBlock, nullptr);
    ir::IRBuilder OtherBuilder(Context.irContext());
    ASSERT_TRUE(OtherBuilder.setInsertPoint(*OtherBlock));
    auto *OtherCall = OtherBuilder.createCall(*Producer);
    ASSERT_NE(OtherCall, nullptr);
    ir::CallInstruction Detached(*Producer);
    ir::IRContext ForeignContext(target::TargetContext::native());
    ir::Module Foreign(ForeignContext);
    auto *ForeignFunction = Foreign.createGlobalFunction(*Foreign.typePool().get<ink::ir::FunctionType>(Foreign.typePool().get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Foreign.typePool().get<ink::ir::IntegerType>(32, true))}));
    const auto *ForeignConstant = Foreign.constantPool().get<ink::ir::IntegerConstant>(Foreign.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(1));
    ASSERT_NE(ForeignFunction, nullptr);
    ASSERT_NE(ForeignConstant, nullptr);
    ir::Value *Operands[] = {
        Other->parameter(0),
        OtherCall,
        &Detached,
        ForeignFunction->parameter(0),
        const_cast<ir::IntegerConstant *>(ForeignConstant),
    };
    for (auto *Operand : Operands)
    {
      SCOPED_TRACE(ir::valueKindName(Operand->kind()));
      auto Input = read("-Operand");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Operand"), *Operand), BindResult::Inserted);
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*CurrentBlock));
      EXPECT_DEATH(analyze(State, Input), "INK-S0013");
      EXPECT_EQ(CurrentBlock->instructionCount(), 0U);
      EXPECT_EQ(OtherBlock->instructionCount(), 1U);
      EXPECT_EQ(State.irBuilder().insertBlock(), CurrentBlock);
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // A terminated block triggers the insertion-state check, while an instruction without a selected function reports an internal model error.
  TEST_F(SemanticAnalyzerExprTest, ChecksTerminatedNegationBlockAndReportsICEForUnownedInstruction)
  {
    auto Input = read("-Operand");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    auto *Function = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Types.get<ink::ir::IntegerType>(32, true))}));
    auto *Producer = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::IntegerType>(32, true)));
    ASSERT_NE(Function, nullptr);
    ASSERT_NE(Producer, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    auto *Produced = State.irBuilder().createCall(*Producer);
    ASSERT_NE(Produced, nullptr);
    const auto *Return = State.irBuilder().createReturn();
    ASSERT_NE(Return, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Operand"), *Function->parameter(0)), BindResult::Inserted);
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(analyze(State, Input), "check failed:");
#endif
    EXPECT_EQ(Block->instructionCount(), 2U);
    EXPECT_EQ(Block->instruction(1), Return);
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    State.irBuilder().clearInsertPoint();
    Diagnostics.clear();
    State.nameResolver().enterScope();
    ASSERT_EQ(State.nameResolver().bind(name("Operand"), *Produced), BindResult::Inserted);
    EXPECT_DEATH(analyze(State, Input), "INK-S0013");
    EXPECT_EQ(Block->instructionCount(), 2U);
    EXPECT_EQ(State.irBuilder().insertBlock(), nullptr);
    ASSERT_TRUE(State.nameResolver().exitScope());
  }

  // Numeric constant negation checks for an insertion block because it always constructs an instruction instead of folding a value.
  TEST_F(SemanticAnalyzerExprTest, ChecksInsertBlockForConstantNegation)
  {
#if !INK_ENABLE_ASSERTIONS
    GTEST_SKIP() << "Insertion-state assertions are disabled.";
#endif
    constexpr const char *Cases[] = {
        "-0",
        "-1",
        "-1.5",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      EXPECT_EQ(State.irBuilder().insertBlock(), nullptr);
      EXPECT_DEATH(analyze(State, Input), "check failed:");
    }
  }

  // Negating uninitialized numeric storage reports a read warning and emits both a load and a same-typed negation.
  TEST_F(SemanticAnalyzerExprTest, WarnsAndLowersUninitializedNumericStorageNegation)
  {
    const ir::Type *Types[] = {
        Context.typePool().get<ink::ir::IntegerType>(32, true),
        Context.typePool().get<ink::ir::FloatType>(64),
    };
    for (const auto *Type : Types)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-((Stored))");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      auto *Block = createInsertBlock(State);
      ASSERT_NE(Block, nullptr);
      auto *Storage = State.irBuilder().createAlloca(*Type);
      ASSERT_NE(Storage, nullptr);
      Storage->setName(name("Stored"));
      ASSERT_EQ(State.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
      const auto *Result = analyze(State, Input);
      ASSERT_TRUE(ir::NegInstruction::classof(Result));
      const auto *Read = static_cast<const ir::NegInstruction *>(Result)->operand();
      ASSERT_TRUE(ir::LoadInstruction::classof(Read));
      EXPECT_EQ(static_cast<const ir::LoadInstruction *>(Read)->pointer(), Storage);
      expectNegation(Result, Read);
      EXPECT_EQ(Result->type(), Type);
      const auto *Unary = frontend::parser::cast<frontend::parser::UnaryExpr>(&syntax(Input));
      expectUninitializedWarning(Input, Unary->operand()->getSourceRange());
      EXPECT_FALSE(Storage->isInitialized());
      ASSERT_EQ(Block->instructionCount(), 3U);
      EXPECT_EQ(Block->instruction(0), Storage);
      EXPECT_EQ(Block->instruction(1), Read);
      EXPECT_EQ(Block->instruction(2), Result);
      Context.scopeStore().removeTarget(*Storage);
    }
  }

  // Reading uninitialized boolean storage still reaches the unsupported-operation diagnostic for nonnumeric negation.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnsupportedForUninitializedBooleanStorageNegation)
  {
    auto Input = read("-((Stored))");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    ASSERT_NE(createInsertBlock(State), nullptr);
    auto *Storage = State.irBuilder().createAlloca(*Context.typePool().get<ink::ir::BoolType>());
    ASSERT_NE(Storage, nullptr);
    Storage->setName(name("Stored"));
    ASSERT_EQ(State.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
    EXPECT_DEATH(analyze(State, Input), "internal compiler error\\[INK-S0012\\]: initial semantic analyzer does not support UnaryExpr");
    Context.scopeStore().removeTarget(*Storage);
  }

  // Global reads use the shared storage diagnostic, while nonnumeric negation remains an unsupported operation.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnsupportedForGlobalStorageAndNonnumericNegationOperands)
  {
    auto &Types = Context.typePool();
    auto *Variable = Output.createGlobalVariable(*Types.get<ink::ir::IntegerType>(32, true));
    auto *Function = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    ir::Module Package(Context.irContext());
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Function, nullptr);
    const auto *Initializer = Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Initializer, nullptr);
    ASSERT_TRUE(Variable->setInitializer(Initializer));
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Callable"), *Function), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("nullptr"), const_cast<ir::NullPointerConstant &>(*Output.constantPool().get<ink::ir::NullPointerConstant>())), BindResult::Inserted);
    constexpr std::pair<const char *, const char *> Cases[] = {
        {"-Written", "variable storage reads"},
        {"-true", "UnaryExpr"},
        {"-((true))", "UnaryExpr"},
        {"-i32", "UnaryExpr"},
        {"-Pkg", "UnaryExpr"},
        {"-nullptr", "UnaryExpr"},
        {"-\"abc\"", "UnaryExpr"},
        {"-Callable", "UnaryExpr"},
    };
    for (const auto &[Text, Description] : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      for (bool WithInsertBlock : {false, true})
      {
        SCOPED_TRACE(WithInsertBlock);
        Diagnostics.clear();
        AnalyzerState State(Context, Input, Output);
        if (WithInsertBlock)
        {
          ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
        }
        EXPECT_DEATH(analyze(State, Input), std::string("internal compiler error\\[INK-S0012\\]: initial semantic analyzer does not support ") + Description);
        EXPECT_EQ(Block->instructionCount(), 0U);
        EXPECT_EQ(Variable->initializer(), Initializer);
        EXPECT_EQ(State.irBuilder().insertBlock(), WithInsertBlock ? Block : nullptr);
      }
    }
  }

  // Negation of current-function boolean, pointer, function and unit parameters reports the unsupported-operation diagnostic.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnsupportedForNonnumericParameterNegationOperands)
  {
    auto &Types = Context.typePool();
    const auto *CallableType = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>());
    ASSERT_NE(CallableType, nullptr);
    const ir::Type *ParameterTypes[] = {
        Types.get<ink::ir::BoolType>(),
        Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true)),
        CallableType,
        Types.get<ink::ir::VoidType>(),
    };
    for (const auto *Type : ParameterTypes)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-Argument");
      ASSERT_TRUE(Input.succeeded());
      const auto *Signature = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Type)});
      ASSERT_NE(Signature, nullptr);
      auto *Function = Output.createGlobalFunction(*Signature);
      ASSERT_NE(Function, nullptr);
      auto *Block = Function->createBlock();
      ASSERT_NE(Block, nullptr);
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Function->parameter(0)), BindResult::Inserted);
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
      EXPECT_DEATH(analyze(State, Input), "INK-S0012");
      EXPECT_EQ(Block->instructionCount(), 0U);
      EXPECT_EQ(State.irBuilder().insertBlock(), Block);
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // A nondecimal integer token must contain its two-byte prefix and at least one digit.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForTruncatedNondecimalIntegerTokens)
  {
    struct Case
    {
        const char *Text;
        core::NumericBase Base;
    };
    constexpr Case Cases[] = {
        {"0", core::NumericBase::Binary},
        {"0b", core::NumericBase::Binary},
        {"0", core::NumericBase::Octal},
        {"0o", core::NumericBase::Octal},
        {"0", core::NumericBase::Hexadecimal},
        {"0x", core::NumericBase::Hexadecimal},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      SCOPED_TRACE(static_cast<unsigned>(Entry.Base));
      const std::size_t Length = std::string_view(Entry.Text).size();
      auto Lexed = frontend::tokenizer::tokenize(Frontend, "\"" + std::string(Entry.Text) + "\";");
      ASSERT_TRUE(Lexed.succeeded());
      auto &Token = const_cast<frontend::tokenizer::Token &>(Lexed.tokens().front());
      Token.Kind = frontend::tokenizer::TokenKind::IntegerLiteral;
      Token.Span = core::SourceRange::fromByteOffsets(1, Length + 1);
      Token.Payload = frontend::tokenizer::NumericInfo{Entry.Base};
      auto Input = frontend::parser::parse(Frontend, std::move(Lexed));
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(Analysis.analyze(Context, Input, Output), "INK-T0023");
    }
  }

  // Invalid digits and partially consumed integer spellings are tokenizer contract failures rather than integer overflow.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForMalformedIntegerTokenDigits)
  {
    struct Case
    {
        const char *Text;
        core::NumericBase Base;
    };
    constexpr Case Cases[] = {
        {"abc", core::NumericBase::Decimal},
        {"1a", core::NumericBase::Decimal},
        {"0b2", core::NumericBase::Binary},
        {"0b102", core::NumericBase::Binary},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      const std::size_t Length = std::string_view(Entry.Text).size();
      auto Lexed = frontend::tokenizer::tokenize(Frontend, "\"" + std::string(Entry.Text) + "\";");
      ASSERT_TRUE(Lexed.succeeded());
      auto &Token = const_cast<frontend::tokenizer::Token &>(Lexed.tokens().front());
      Token.Kind = frontend::tokenizer::TokenKind::IntegerLiteral;
      Token.Span = core::SourceRange::fromByteOffsets(1, Length + 1);
      Token.Payload = frontend::tokenizer::NumericInfo{Entry.Base};
      auto Input = frontend::parser::parse(Frontend, std::move(Lexed));
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(Analysis.analyze(Context, Input, Output), "INK-T0023");
    }
  }

  // Missing numeric payloads and unsupported bases must terminate with the tokenizer contract ICE.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForInvalidIntegerTokenMetadata)
  {
    const frontend::tokenizer::TokenPayload Payloads[] = {
        std::monostate{},
        frontend::tokenizer::NumericInfo{static_cast<core::NumericBase>(3)},
    };
    for (const auto &Payload : Payloads)
    {
      SCOPED_TRACE(Payload.index());
      auto Lexed = frontend::tokenizer::tokenize(Frontend, "0;");
      ASSERT_TRUE(Lexed.succeeded());
      // Lexed and its token storage are mutable; the accessor exposes only a const view.
      auto &Token = const_cast<frontend::tokenizer::Token &>(Lexed.tokens().front());
      Token.Payload = Payload;
      auto Input = frontend::parser::parse(Frontend, std::move(Lexed));
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(Analysis.analyze(Context, Input, Output), "INK-T0023");
    }
  }

  // A real expression object carrying a statement kind must fail public expression dispatch with the AST contract ICE.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForWrongExpressionCategory)
  {
    auto Input = read("14");
    ASSERT_TRUE(Input.succeeded());
    auto *Expression = Input.Unit->context().make<InvalidExpression>(syntax(Input).getSourceRange());
    AnalyzerState State(Context, Input, Output);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    EXPECT_DEATH(Analysis.emitValue(State, *Expression), "INK-S0061.*invalid AST: expression node kind must belong to the expression AST category");
  }

  // Literal references at the end of the token array and at the invalid-token sentinel violate the AST input contract.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForOutOfRangeLiteralTokenIndices)
  {
    auto Input = read("14");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    const frontend::parser::TokenId Indices[] = {Input.Unit->input().size(), frontend::parser::InvalidTokenId};
    for (const auto Index : Indices)
    {
      SCOPED_TRACE(Index);
      auto *Literal = Input.Unit->context().make<frontend::parser::LiteralExpr>(syntax(Input).getSourceRange(), Index, frontend::parser::TokenKind::IntegerLiteral);
      EXPECT_DEATH(Analysis.emitValue(State, *Literal), "internal compiler error\\[INK-P0048\\]: token index " + std::to_string(Index) + " is out of bounds for input of " + std::to_string(Input.Unit->input().size()) + " tokens");
    }
  }

  // A LiteralExpr cannot reinterpret an integer token as a float, character or string token.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForMismatchedLiteralTokenKinds)
  {
    auto Input = read("14");
    ASSERT_TRUE(Input.succeeded());
    const auto &Original = static_cast<const frontend::parser::LiteralExpr &>(syntax(Input));
    AnalyzerState State(Context, Input, Output);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    const frontend::parser::TokenKind Kinds[] = {frontend::parser::TokenKind::FloatLiteral, frontend::parser::TokenKind::CharLiteral, frontend::parser::TokenKind::StringLiteral};
    for (const auto Kind : Kinds)
    {
      SCOPED_TRACE(static_cast<unsigned>(Kind));
      auto *Literal = Input.Unit->context().make<frontend::parser::LiteralExpr>(Original.getSourceRange(), Original.token(), Kind);
      EXPECT_DEATH(Analysis.emitValue(State, *Literal), "INK-S0061.*invalid AST: LiteralExpr kind must match its referenced token kind");
    }
  }

  // Character literal tokens require CharInfo even when their source spelling is otherwise valid.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForMissingCharacterMetadata)
  {
    const frontend::tokenizer::TokenPayload Payloads[] = {
        std::monostate{},
        frontend::tokenizer::NumericInfo{core::NumericBase::Decimal},
    };
    for (const auto &Payload : Payloads)
    {
      SCOPED_TRACE(Payload.index());
      auto Lexed = frontend::tokenizer::tokenize(Frontend, "'a';");
      ASSERT_TRUE(Lexed.succeeded());
      // Lexed owns mutable token storage; its public accessor exposes a const view.
      const_cast<frontend::tokenizer::Token &>(Lexed.tokens().front()).Payload = Payload;
      auto Input = frontend::parser::parse(Frontend, std::move(Lexed));
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(Analysis.analyze(Context, Input, Output), "INK-S0061.*invalid AST: character LiteralExpr must reference a token with CharInfo metadata");
    }
  }

  // Character metadata cannot contain either surrogate boundary or a code point above the Unicode scalar range.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForNonScalarCharacterMetadata)
  {
    constexpr char32_t Values[] = {0xd800, 0xdfff, 0x110000};
    for (const auto Value : Values)
    {
      SCOPED_TRACE(static_cast<std::uint32_t>(Value));
      auto Lexed = frontend::tokenizer::tokenize(Frontend, "'a';");
      ASSERT_TRUE(Lexed.succeeded());
      // Lexed owns mutable token storage; its public accessor exposes a const view.
      const_cast<frontend::tokenizer::Token &>(Lexed.tokens().front()).Payload = frontend::tokenizer::CharInfo{Value};
      auto Input = frontend::parser::parse(Frontend, std::move(Lexed));
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(Analysis.analyze(Context, Input, Output), "INK-S0061.*invalid AST: character LiteralExpr metadata must contain a Unicode scalar value");
    }
  }

  // String literal tokens require StringInfo and cannot use absent or character metadata.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForMissingStringMetadata)
  {
    const frontend::tokenizer::TokenPayload Payloads[] = {
        std::monostate{},
        frontend::tokenizer::CharInfo{U'a'},
    };
    for (const auto &Payload : Payloads)
    {
      SCOPED_TRACE(Payload.index());
      auto Lexed = frontend::tokenizer::tokenize(Frontend, "\"text\";");
      ASSERT_TRUE(Lexed.succeeded());
      // Lexed owns mutable token storage; its public accessor exposes a const view.
      const_cast<frontend::tokenizer::Token &>(Lexed.tokens().front()).Payload = Payload;
      auto Input = frontend::parser::parse(Frontend, std::move(Lexed));
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(Analysis.analyze(Context, Input, Output), "INK-S0061.*invalid AST: string LiteralExpr must reference a token with StringInfo metadata");
    }
  }

  // Function type expressions must retain their required return annotation before signature analysis starts.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForMissingFunctionTypeReturnAnnotation)
  {
    auto Input = read("func(): void");
    ASSERT_TRUE(Input.succeeded());
    auto *Statement = frontend::parser::cast<frontend::parser::DeclStmt>(Input.Unit->root()->statements()[0]);
    auto *Original = frontend::parser::cast<frontend::parser::FunctionTypeExpr>(frontend::parser::cast<frontend::parser::VarDecl>(Statement->declaration())->initializer());
    auto *Expression = Input.Unit->context().make<frontend::parser::FunctionTypeExpr>(Original->getSourceRange(), Original->parameters(), nullptr);
    AnalyzerState State(Context, Input, Output);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    EXPECT_DEATH(Analysis.emitValue(State, *Expression), "INK-S0061.*invalid AST: FunctionTypeExpr requires a return type annotation");
  }

  // Every function type parameter needs a type annotation even when the enclosing return annotation is valid.
  TEST_F(SemanticAnalyzerExprTest, ReportsInvalidASTForMissingFunctionTypeParameterAnnotation)
  {
    auto Input = read("func(): void");
    ASSERT_TRUE(Input.succeeded());
    auto *Statement = frontend::parser::cast<frontend::parser::DeclStmt>(Input.Unit->root()->statements()[0]);
    auto *Original = frontend::parser::cast<frontend::parser::FunctionTypeExpr>(frontend::parser::cast<frontend::parser::VarDecl>(Statement->declaration())->initializer());
    const frontend::parser::FunctionTypeParameter Parameters[] = {frontend::parser::FunctionTypeParameter(std::nullopt, nullptr, false, Original->getSourceRange())};
    auto &AST = Input.Unit->context();
    auto *Expression = AST.make<frontend::parser::FunctionTypeExpr>(Original->getSourceRange(), AST.copyArray<frontend::parser::FunctionTypeParameter>(Parameters), Original->returnType());
    AnalyzerState State(Context, Input, Output);
    Context.setSource(Input.Unit->input().lexedFile().sourceId());
    EXPECT_DEATH(Analysis.emitValue(State, *Expression), "INK-S0061.*invalid AST: FunctionTypeExpr parameter requires a type annotation");
  }

  // Integer literals cannot initialize bool, float or pointer destinations through implicit numeric conversions.
  TEST_F(SemanticAnalyzerExprTest, RejectsNonintegerDestinationsForIntegerLiterals)
  {
    auto Input = read("1");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const ir::Type *ExpectedTypes[] = {Types.get<ink::ir::BoolType>(), Types.get<ink::ir::FloatType>(64), Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(8, false))};
    for (const auto *ExpectedType : ExpectedTypes)
    {
      Diagnostics.clear();
      expectFailure(analyze(Input, {.ExpectedType = ExpectedType}), core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // Floating literals use f64 by default and their requested f32 type when representable.
  TEST_F(SemanticAnalyzerExprTest, ConstructsDefaultAndExpectedFloatConstants)
  {
    auto Fraction = read("1.5");
    auto Exponent = read("1E+2");
    ASSERT_TRUE(Fraction.succeeded());
    ASSERT_TRUE(Exponent.succeeded());
    const auto *Default = analyze(Fraction);
    ASSERT_NE(Default, nullptr);
    EXPECT_EQ(Default, Output.constantPool().get<ink::ir::FloatConstant>(1.5));
    EXPECT_EQ(Default->type(), Context.typePool().get<ink::ir::FloatType>(64));
    const auto *Narrow = analyze(Fraction, {.ExpectedType = Context.typePool().get<ink::ir::FloatType>(32)});
    ASSERT_NE(Narrow, nullptr);
    EXPECT_EQ(Narrow, Output.constantPool().get<ink::ir::FloatConstant>(1.5F));
    EXPECT_EQ(Narrow->type(), Context.typePool().get<ink::ir::FloatType>(32));
    EXPECT_EQ(analyze(Exponent), Output.constantPool().get<ink::ir::FloatConstant>(100.0));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Floating-point operands preserve contextual width and zero bits in explicit negation instructions, including chained and bound constants.
  TEST_F(SemanticAnalyzerExprTest, LowersFloatingPointLiteralAndConstantNegationWithoutFolding)
  {
    auto &Types = Context.typePool();
    auto &Constants = Output.constantPool();
    struct Case
    {
        const char *Text;
        const ir::FloatType *Type;
        const ir::FloatConstant *Operand;
        bool Repeated;
    };
    const Case Cases[] = {
        {"-1.5", Types.get<ink::ir::FloatType>(64), Constants.get<ink::ir::FloatConstant>(1.5), false},
        {"-(1.5)", Types.get<ink::ir::FloatType>(32), Constants.get<ink::ir::FloatConstant>(1.5F), false},
        {"-0.0", Types.get<ink::ir::FloatType>(64), Constants.get<ink::ir::FloatConstant>(0.0), false},
        {"-(0.0)", Types.get<ink::ir::FloatType>(32), Constants.get<ink::ir::FloatConstant>(0.0F), false},
        {"-(-1.5)", Types.get<ink::ir::FloatType>(64), Constants.get<ink::ir::FloatConstant>(1.5), true},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      ASSERT_NE(Entry.Operand, nullptr);
      const auto *Result = analyze(Input, {.ExpectedType = Entry.Type});
      if (Entry.Repeated)
      {
        ASSERT_NE(Result, nullptr);
        ASSERT_TRUE(ir::NegInstruction::classof(Result));
        const auto *Inner = static_cast<const ir::NegInstruction *>(Result)->operand();
        expectNegation(Result, Inner);
        expectNegation(Inner, Entry.Operand);
      }
      else
      {
        expectNegation(Result, Entry.Operand);
      }
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    auto Input = read("-Fraction");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    auto *Block = createInsertBlock(State);
    ASSERT_NE(Block, nullptr);
    const auto *Constant = Constants.get<ink::ir::FloatConstant>(-1.5F);
    ASSERT_NE(Constant, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Fraction"), const_cast<ir::FloatConstant &>(*Constant)), BindResult::Inserted);
    expectNegation(analyze(State, Input), Constant);
    EXPECT_EQ(Block->instructionCount(), 1U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Float overflow and nonfloating destinations report recoverable mismatches instead of materializing infinity.
  TEST_F(SemanticAnalyzerExprTest, RejectsFloatOverflowAndWrongDestination)
  {
    auto NarrowOverflow = read("1e100");
    auto WideOverflow = read("1e1000");
    auto Fraction = read("1.5");
    ASSERT_TRUE(NarrowOverflow.succeeded());
    ASSERT_TRUE(WideOverflow.succeeded());
    ASSERT_TRUE(Fraction.succeeded());
    expectFailure(analyze(NarrowOverflow, {.ExpectedType = Context.typePool().get<ink::ir::FloatType>(32)}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected f32, got an invalid or out-of-range floating-point literal");
    Diagnostics.clear();
    expectFailure(analyze(WideOverflow), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected f64, got an invalid or out-of-range floating-point literal");
    Diagnostics.clear();
    expectFailure(analyze(Fraction, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(32, true)}), core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Characters use decoded Unicode scalars as u32 values and only fit smaller integer destinations when in range.
  TEST_F(SemanticAnalyzerExprTest, ConstructsAndChecksDecodedCharacterScalars)
  {
    auto Supplementary = read("'\\U0001F600'");
    auto Ascii = read("'a'");
    auto Nul = read("'\\0'");
    ASSERT_TRUE(Supplementary.succeeded());
    ASSERT_TRUE(Ascii.succeeded());
    ASSERT_TRUE(Nul.succeeded());
    EXPECT_EQ(analyze(Supplementary), Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(0x1F600)));
    EXPECT_EQ(analyze(Ascii, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(8, false)}), Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(8, false), static_cast<std::uint64_t>(97)));
    EXPECT_EQ(analyze(Nul), Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(0)));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(Supplementary, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(8, false)}), core::DiagnosticKind::SemanticIntegerOutOfRange);
    Diagnostics.clear();
    expectFailure(analyze(Ascii, {.ExpectedType = Context.typePool().get<ink::ir::BoolType>()}), core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Strings retain tokenizer-decoded UTF-8, embedded NUL, raw backslashes and normalized multiline newlines without a terminator.
  TEST_F(SemanticAnalyzerExprTest, PreservesDecodedStringBytesAndArrayLengths)
  {
    struct Case
    {
        const char *Text;
        std::string Bytes;
        std::size_t ByteCount;
    };
    const Case Cases[] = {
        {"\"Hello, world!\\n\"", "Hello, world!\n", 14},
        {"\"\\u4E2D\\U0001F600\"", "\xE4\xB8\xAD\xF0\x9F\x98\x80", 7},
        {"\"a\\0b\"", std::string("a\0b", 3), 3},
        {"r\"\\n\\x41\"", "\\n\\x41", 6},
        {"\"\"\"a\r\nb\"\"\"", "a\nb", 3},
        {"\"\"", "", 0},
        {"\"\\x5Cn\"", "\\n", 2},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Result = analyze(Input);
      ASSERT_NE(Result, nullptr);
      ASSERT_TRUE(ir::StringConstant::classof(Result));
      const auto *String = static_cast<const ir::StringConstant *>(Result);
      EXPECT_EQ(String->value(), Entry.Bytes);
      EXPECT_EQ(String->byteLength(), Entry.ByteCount);
      EXPECT_EQ(Result->type(), Context.typePool().get<ink::ir::ArrayType>(Context.typePool().get<ink::ir::IntegerType>(8, false), Entry.ByteCount));
      EXPECT_EQ(Result, Output.constantPool().get<ink::ir::StringConstant>(Entry.Bytes));
      EXPECT_EQ(Output.globalVariableCount(), 0U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // String constants own their decoded bytes after the parse unit dies and equivalent spellings reuse one pooled object.
  TEST_F(SemanticAnalyzerExprTest, OwnsAndInternsDecodedStringContents)
  {
    const ir::StringConstant *Stored = nullptr;
    {
      auto Input = read("\"A\\0B\"");
      ASSERT_TRUE(Input.succeeded());
      const auto *Result = analyze(Input);
      ASSERT_NE(Result, nullptr);
      ASSERT_TRUE(ir::StringConstant::classof(Result));
      Stored = static_cast<const ir::StringConstant *>(Result);
    }
    EXPECT_EQ(Stored->value(), std::string("A\0B", 3));
    auto Equivalent = read("\"\\x41\\x00\\x42\"");
    ASSERT_TRUE(Equivalent.succeeded());
    EXPECT_EQ(analyze(Equivalent), Stored);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Array destinations must match the decoded bytes exactly, and strings cannot become writable or differently typed pointers.
  TEST_F(SemanticAnalyzerExprTest, ChecksExpectedStringArrayType)
  {
    auto Input = read("\"ab\"");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    EXPECT_TRUE(analyze(Input, {.ExpectedType = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), 2)}));
    struct Case
    {
        const ir::Type *ExpectedType;
        const char *Message;
    };
    const Case Cases[] = {
        {Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), 3), "expected [u8; 3], got [u8; 2]"},
        {Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(32, true), 2), "expected [i32; 2], got [u8; 2]"},
        {Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(8, false)), "expected *u8, got [u8; 2]"},
        {Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(8, true, false)), "expected *const i8, got [u8; 2]"},
        {Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, false, false)), "expected *const u32, got [u8; 2]"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Message);
      Diagnostics.clear();
      expectFailure(analyze(Input, {.ExpectedType = Entry.ExpectedType}), core::DiagnosticKind::SemanticTypeMismatch);
      expectMessage(Entry.Message);
      EXPECT_EQ(Output.globalVariableCount(), 0U);
    }
  }

  // Read-only byte pointer contexts materialize exact string bytes once, including empty strings, embedded NUL and nested parentheses.
  TEST_F(SemanticAnalyzerExprTest, ConvertsStringConstantsToReadOnlyBytePointers)
  {
    struct Case
    {
        const char *Text;
        std::string Bytes;
    };
    const Case Cases[] = {
        {"\"abc\"", "abc"},
        {"(((\"a\\0b\")))", std::string("a\0b", 3)},
        {"\"\"", ""},
    };
    auto &Types = Context.typePool();
    const auto *PointerType = Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(8, false, false));
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      auto *Block = createInsertBlock(State);
      ASSERT_NE(Block, nullptr);
      const auto GlobalsBefore = Output.globalVariableCount();
      const auto *Result = analyzeExpr(State, Input, {.ExpectedType = PointerType});
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result->type(), PointerType);
      EXPECT_FALSE(Result->isStorage());
      ASSERT_TRUE(ir::GetElementPtrInstruction::classof(Result));
      const auto *ElementAddress = static_cast<const ir::GetElementPtrInstruction *>(Result);
      ASSERT_TRUE(ir::AddressOfInstruction::classof(ElementAddress->pointer()));
      const auto *ArrayAddress = static_cast<const ir::AddressOfInstruction *>(ElementAddress->pointer());
      ASSERT_TRUE(ir::GlobalVariable::classof(ArrayAddress->storage()));
      const auto *Storage = static_cast<const ir::GlobalVariable *>(ArrayAddress->storage());
      EXPECT_EQ(Output.globalVariableCount(), GlobalsBefore + 1);
      EXPECT_EQ(Output.globalVariable(GlobalsBefore), Storage);
      EXPECT_FALSE(Storage->isMutable());
      EXPECT_TRUE(Storage->isInitialized());
      EXPECT_EQ(Storage->type(), Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), Entry.Bytes.size(), false));
      EXPECT_EQ(Storage->initializer(), Output.constantPool().get<ink::ir::StringConstant>(Entry.Bytes));
      EXPECT_EQ(ElementAddress->sourceElementType(), Storage->type());
      ASSERT_EQ(ElementAddress->indexCount(), 2U);
      for (const auto *Index : ElementAddress->indices())
      {
        ASSERT_TRUE(ir::IntegerConstant::classof(Index));
        EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Index)->bits().tryGetUInt64(), 0U);
      }
      EXPECT_EQ(ArrayAddress->parentBlock(), Block);
      EXPECT_EQ(ElementAddress->parentBlock(), Block);
      ASSERT_EQ(Block->instructionCount(), 2U);
      EXPECT_EQ(Block->instruction(0), ArrayAddress);
      EXPECT_EQ(Block->instruction(1), ElementAddress);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // An initialized array variable remains storage and cannot acquire implicit string-constant pointer conversion.
  TEST_F(SemanticAnalyzerExprTest, RejectsImplicitStringPointersFromArrayStorage)
  {
    auto Input = read("((Stored))");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const auto *InitialValue = Output.constantPool().get<ink::ir::StringConstant>("abc");
    ASSERT_NE(InitialValue, nullptr);
    for (bool Mutable : {true, false})
    {
      SCOPED_TRACE(Mutable);
      Diagnostics.clear();
      AnalyzerState State(Context, Input, Output);
      auto *Block = createInsertBlock(State);
      ASSERT_NE(Block, nullptr);
      auto *Storage = State.irBuilder().createAlloca(*InitialValue->type(), Mutable);
      ASSERT_NE(Storage, nullptr);
      ASSERT_NE(State.irBuilder().createInitialize(*InitialValue, *Storage), nullptr);
      ASSERT_EQ(State.nameResolver().bind(name("Stored"), *Storage), BindResult::Inserted);
      expectFailure(analyze(State, Input, {.ExpectedType = Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(8, false, false))}), core::DiagnosticKind::SemanticTypeMismatch);
      EXPECT_EQ(Block->instructionCount(), 2U);
      EXPECT_EQ(Output.globalVariableCount(), 0U);
      Context.scopeStore().removeTarget(*Storage);
    }
  }

  // Converting a string without a writable insertion point reports an internal construction error.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForStringPointerConversionWithoutWritableBlock)
  {
    auto Input = read("\"abc\"");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    const ExprHints Hints{.ExpectedType = Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(8, false, false))};
    EXPECT_DEATH(analyzeExpr(State, Input, Hints), "INK-S0013");
    auto *Block = createInsertBlock(State);
    ASSERT_NE(Block, nullptr);
    ASSERT_NE(State.irBuilder().createReturn(), nullptr);
    EXPECT_DEATH(analyzeExpr(State, Input, Hints), "INK-S0013");
  }

  // Unbound true and false names produce pooled bool constants while a nearer binding still shadows the fallback.
  TEST_F(SemanticAnalyzerExprTest, ResolvesBooleanFallbackAfterLexicalLookup)
  {
    auto TrueInput = read("true");
    auto FalseInput = read("false");
    ASSERT_TRUE(TrueInput.succeeded());
    ASSERT_TRUE(FalseInput.succeeded());
    EXPECT_EQ(analyze(TrueInput), Output.constantPool().get<ink::ir::BoolConstant>(true));
    EXPECT_EQ(analyze(FalseInput), Output.constantPool().get<ink::ir::BoolConstant>(false));
    AnalyzerState State(Context, TrueInput, Output);
    auto *Shadow = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    ASSERT_NE(Shadow, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("true"), *Shadow), BindResult::Inserted);
    EXPECT_EQ(analyzeExpr(State, TrueInput), Shadow);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(FalseInput, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(32, true)}), core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Unknown expression names report exactly one diagnostic attributed to the currently selected input and identifier span.
  TEST_F(SemanticAnalyzerExprTest, AttributesUnknownNameDiagnosticsToInput)
  {
    auto First = read("Missing");
    auto Second = read("AlsoMissing");
    ASSERT_TRUE(First.succeeded());
    ASSERT_TRUE(Second.succeeded());
    const frontend::parser::ParseResult *Inputs[] = {&First, &Second, &First};
    for (const auto *Input : Inputs)
    {
      Diagnostics.clear();
      expectFailure(analyze(*Input), core::DiagnosticKind::SemanticUnknownName);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Source, Input->Unit->input().lexedFile().sourceId());
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(*Input).getSourceRange());
    }
  }

  // Constant bindings retain pooled value identity and require compatible expected types without implicit integer widening.
  TEST_F(SemanticAnalyzerExprTest, ResolvesBoundConstantsWithoutChangingTypes)
  {
    auto Input = read("Count");
    ASSERT_TRUE(Input.succeeded());
    const auto *Constant = Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Constant, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
    const auto *Result = analyze(State, Input, {.ExpectedType = Constant->type()});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Constant);
    EXPECT_EQ(Result->type(), Constant->type());
    EXPECT_EQ(analyze(State, Input, {.DefaultIntegerLiteralType = Context.typePool().get<ink::ir::IntegerType>(64, false)}), Constant);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(64, true)}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected i64, got i32");
    Diagnostics.clear();
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(64, false)}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected u64, got i32");
  }

  // A bound null pointer literal remains the pool's marker value while initializing a concrete pointer type.
  TEST_F(SemanticAnalyzerExprTest, AcceptsCompatibleNullPointerConstantBinding)
  {
    auto Input = read("Null");
    ASSERT_TRUE(Input.succeeded());
    const auto *Null = Output.constantPool().get<ink::ir::NullPointerConstant>();
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Null"), const_cast<ir::NullPointerConstant &>(*Null)), BindResult::Inserted);
    const auto *Result = analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(8, false))});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Null);
    EXPECT_EQ(Result->type(), Null->type());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(64, false)}), core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Global names preserve their storage and initializer without changing the block, while emitting a global value remains unsupported.
  TEST_F(SemanticAnalyzerExprTest, PreservesGlobalStorageAndBuilderState)
  {
    auto Input = read("Written");
    ASSERT_TRUE(Input.succeeded());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Function, nullptr);
    const auto *Initializer = Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(0));
    ASSERT_TRUE(Variable->setInitializer(Initializer));
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    auto *Return = State.irBuilder().createReturn();
    ASSERT_NE(Return, nullptr);
    const auto *Result = analyzeExpr(State, Input, {.ExpectedType = Variable->type()});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Variable);
    EXPECT_EQ(Result->type(), Variable->type());
    const auto *Found = State.nameResolver().lookupLocal(name("Written"));
    ASSERT_NE(Found, nullptr);
    EXPECT_TRUE(Result->isStorage());
    EXPECT_EQ(Variable->initializer(), Initializer);
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    ASSERT_EQ(Block->instructionCount(), 1U);
    EXPECT_EQ(Block->instruction(0), Return);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_DEATH(analyze(State, Input), "INK-S0012");
    EXPECT_FALSE(analyzeExpr(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(64, false)}));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Variable->initializer(), Initializer);
    EXPECT_EQ(Block->instructionCount(), 1U);
  }

  // The nearest lexical binding selects storage independently of an outer constant and exiting restores the outer value.
  TEST_F(SemanticAnalyzerExprTest, ResolvesNearestBindingWithoutMergingScopes)
  {
    auto Input = read("Output");
    ASSERT_TRUE(Input.succeeded());
    const auto *Outer = Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(7));
    auto *Inner = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    AnalyzerState State(Context, Input, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Output"), const_cast<ir::IntegerConstant &>(*Outer)), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Outer);
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(name("Output"), *Inner), BindResult::Inserted);
    EXPECT_EQ(analyzeExpr(State, Input), Inner);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(analyze(State, Input), Outer);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Ordinary function bindings and singleton overload sets preserve the same callable identity and complete function type.
  TEST_F(SemanticAnalyzerExprTest, ResolvesFunctionAndSingletonOverloadBindings)
  {
    auto OrdinaryInput = read("WriteFile");
    auto OverloadInput = read("GetStdHandle");
    ASSERT_TRUE(OrdinaryInput.succeeded());
    ASSERT_TRUE(OverloadInput.succeeded());
    const auto *Signature = Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::IntegerType>(32, false));
    auto *Function = Output.createGlobalFunction(*Signature);
    ASSERT_NE(Function, nullptr);
    AnalyzerState OrdinaryState(Context, OrdinaryInput, Output);
    AnalyzerState OverloadState(Context, OverloadInput, Output);
    ASSERT_EQ(OrdinaryState.nameResolver().bind(name("WriteFile"), *Function), BindResult::Inserted);
    ASSERT_EQ(OrdinaryState.nameResolver().bindOverload(name("GetStdHandle"), *Function), BindResult::Inserted);
    const ir::Value *Results[] = {analyze(OrdinaryState, OrdinaryInput, {.ExpectedType = Signature}), analyze(OverloadState, OverloadInput, {.ExpectedType = Signature})};
    for (const auto *Result : Results)
    {
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result, Function);
      EXPECT_EQ(Result->type(), Signature);
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Multiple visible function overloads remain ambiguous even when one signature matches the requested expression type.
  TEST_F(SemanticAnalyzerExprTest, RejectsAmbiguousOverloadNames)
  {
    auto Input = read("Function");
    ASSERT_TRUE(Input.succeeded());
    auto *First = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    auto *Second = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::IntegerType>(32, false)));
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bindOverload(name("Function"), *First), BindResult::Inserted);
    ASSERT_EQ(State.nameResolver().bindOverload(name("Function"), *Second), BindResult::Inserted);
    expectFailure(analyze(State, Input, {.ExpectedType = First->type()}), core::DiagnosticKind::SemanticAmbiguousName);
  }

  // Registered types have the compile-time metatype while modules remain untyped semantic expression values.
  TEST_F(SemanticAnalyzerExprTest, ResolvesTypeAndModuleNamesAsSemanticValues)
  {
    ir::Module Package(Context.irContext());
    auto *Type = Output.createClassType(4, 4);
    ASSERT_NE(Type, nullptr);
    ir::Value *Targets[] = {Type, &Package};
    for (auto *Target : Targets)
    {
      auto Input = read("Target");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Target"), *Target), BindResult::Inserted);
      const auto *Result = analyze(State, Input);
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result, Target);
      EXPECT_EQ(Result->type(), ir::Type::classof(Result) ? Context.typePool().get<ink::ir::MetaType>() : nullptr);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(32, true)}), core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // Binding a control-flow label as an expression value violates the analyzer's internal model contract.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForBasicBlockNames)
  {
    auto Input = read("Block");
    ASSERT_TRUE(Input.succeeded());
    auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Block"), *Block), BindResult::Inserted);
    EXPECT_DEATH(analyze(State, Input), "INK-S0013");
  }

  // Builtin type names use ordinary expression lookup and a lexical value shadows the builtin through parentheses.
  TEST_F(SemanticAnalyzerExprTest, ResolvesAndShadowsBuiltinTypeExpressions)
  {
    auto Input = read("((i32))");
    ASSERT_TRUE(Input.succeeded());
    const auto *Result = analyze(Input);
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Context.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_EQ(Result->type(), Context.typePool().get<ink::ir::MetaType>());
    expectFailure(analyze(Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(32, true)}), core::DiagnosticKind::SemanticTypeMismatch);
    Diagnostics.clear();
    auto *Shadow = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    ASSERT_NE(Shadow, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("i32"), *Shadow), BindResult::Inserted);
    const auto *Shadowed = analyzeExpr(State, Input);
    ASSERT_NE(Shadowed, nullptr);
    EXPECT_EQ(Shadowed, Shadow);
    EXPECT_EQ(Shadowed->type(), Context.typePool().get<ink::ir::IntegerType>(32, false));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Type-valued expectations accept builtin and bound type descriptors, including the self-typed metatype.
  TEST_F(SemanticAnalyzerExprTest, AcceptsTypeValuesWithMetaTypeExpectation)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"type", Types.get<ink::ir::MetaType>()},
        {"((type))", Types.get<ink::ir::MetaType>()},
        {"i32", Types.get<ink::ir::IntegerType>(32, true)},
        {"*bool", Types.get<ink::ir::PointerType>(Types.get<ink::ir::BoolType>())},
        {"[u8; 4]", Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), 4)},
        {"func(i32): bool", Types.get<ink::ir::FunctionType>(Types.get<ink::ir::BoolType>(), {ir::FunctionTypeParameter(Types.get<ink::ir::IntegerType>(32, true))})},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input, {.ExpectedType = Types.get<ink::ir::MetaType>()}), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    auto Input = read("T");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("T"), const_cast<ir::IntegerType &>(*Types.get<ink::ir::IntegerType>(32, true))), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input, {.ExpectedType = Types.get<ink::ir::MetaType>()}), Types.get<ink::ir::IntegerType>(32, true));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A type-valued expectation rejects ordinary values, variable storage and modules instead of treating their descriptors as type values.
  TEST_F(SemanticAnalyzerExprTest, RejectsNonTypeValuesWithMetaTypeExpectation)
  {
    for (const char *Text : {"14", "true", "1.5", "\"hello\""})
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      expectFailure(analyze(Input, {.ExpectedType = Context.typePool().get<ink::ir::MetaType>()}), core::DiagnosticKind::SemanticTypeMismatch);
    }
    ir::Module Package(Context.irContext());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, true));
    ASSERT_NE(Variable, nullptr);
    ir::Value *Targets[] = {Variable, &Package};
    for (auto *Target : Targets)
    {
      auto Input = read("Target");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Target"), *Target), BindResult::Inserted);
      expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::MetaType>()}), core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // A bound type descriptor from a foreign IR context violates the model contract before metatype matching.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForForeignTypeValuesWithMetaTypeExpectation)
  {
    SemanticContext OtherContext(Sources, DiagnosticEngine, target::TargetContext::native());
    const ir::Type *Targets[] = {OtherContext.typePool().get<ink::ir::IntegerType>(32, true), OtherContext.typePool().get<ink::ir::MetaType>()};
    for (const auto *Target : Targets)
    {
      auto Input = read("Foreign");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Foreign"), const_cast<ir::Type &>(*Target)), BindResult::Inserted);
      EXPECT_DEATH(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::MetaType>()}), "INK-S0013");
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // The metatype name follows ordinary lexical lookup and a local runtime value can shadow it.
  TEST_F(SemanticAnalyzerExprTest, AllowsLexicalShadowingOfMetaTypeName)
  {
    auto Input = read("((type))");
    ASSERT_TRUE(Input.succeeded());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, true));
    ASSERT_NE(Variable, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("type"), *Variable), BindResult::Inserted);
    EXPECT_EQ(analyzeExpr(State, Input), Variable);
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().get<ink::ir::MetaType>()}), core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Alternating 32-bit and 64-bit contexts resolves every builtin to the active context's canonical type or bool constant.
  TEST_F(SemanticAnalyzerExprTest, ResolvesBuiltinValuesInEachTargetContext)
  {
    SemanticContext FirstContext(Sources, DiagnosticEngine, target::TargetContext(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian));
    SemanticContext SecondContext(Sources, DiagnosticEngine, target::TargetContext(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian));
    ir::Module FirstOutput(FirstContext.irContext());
    ir::Module SecondOutput(SecondContext.irContext());
    auto &FirstTypes = FirstContext.typePool();
    auto &SecondTypes = SecondContext.typePool();
    struct Case
    {
        const char *Text;
        const ir::Value *First;
        const ir::Value *Second;
    };
    const Case Cases[] = {
        {"type", FirstTypes.get<ink::ir::MetaType>(), SecondTypes.get<ink::ir::MetaType>()},
        {"void", FirstTypes.get<ink::ir::VoidType>(), SecondTypes.get<ink::ir::VoidType>()},
        {"bool", FirstTypes.get<ink::ir::BoolType>(), SecondTypes.get<ink::ir::BoolType>()},
        {"u8", FirstTypes.get<ink::ir::IntegerType>(8, false), SecondTypes.get<ink::ir::IntegerType>(8, false)},
        {"i32", FirstTypes.get<ink::ir::IntegerType>(32, true), SecondTypes.get<ink::ir::IntegerType>(32, true)},
        {"u32", FirstTypes.get<ink::ir::IntegerType>(32, false), SecondTypes.get<ink::ir::IntegerType>(32, false)},
        {"i64", FirstTypes.get<ink::ir::IntegerType>(64, true), SecondTypes.get<ink::ir::IntegerType>(64, true)},
        {"u64", FirstTypes.get<ink::ir::IntegerType>(64, false), SecondTypes.get<ink::ir::IntegerType>(64, false)},
        {"f32", FirstTypes.get<ink::ir::FloatType>(32), SecondTypes.get<ink::ir::FloatType>(32)},
        {"f64", FirstTypes.get<ink::ir::FloatType>(64), SecondTypes.get<ink::ir::FloatType>(64)},
        {"true", FirstContext.irContext().constantPool().get<ink::ir::BoolConstant>(true), SecondContext.irContext().constantPool().get<ink::ir::BoolConstant>(true)},
        {"false", FirstContext.irContext().constantPool().get<ink::ir::BoolConstant>(false), SecondContext.irContext().constantPool().get<ink::ir::BoolConstant>(false)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      ASSERT_NE(Entry.First, Entry.Second);
      FirstContext.setSource(Input.Unit->input().lexedFile().sourceId());
      SecondContext.setSource(Input.Unit->input().lexedFile().sourceId());
      AnalyzerState FirstState(FirstContext, Input, FirstOutput);
      AnalyzerState SecondState(SecondContext, Input, SecondOutput);
      EXPECT_EQ(Analysis.emitValue(FirstState, syntax(Input)), Entry.First);
      EXPECT_EQ(Analysis.emitValue(SecondState, syntax(Input)), Entry.Second);
      EXPECT_EQ(Analysis.emitValue(FirstState, syntax(Input)), Entry.First);
      EXPECT_EQ(Analysis.emitValue(SecondState, syntax(Input)), Entry.Second);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Parentheses preserve runtime constant identity, addressable storage and expected-type adaptation.
  TEST_F(SemanticAnalyzerExprTest, PreservesRuntimeResultsThroughParentheses)
  {
    auto Literal = read("((255))");
    auto Storage = read("((Written))");
    ASSERT_TRUE(Literal.succeeded());
    ASSERT_TRUE(Storage.succeeded());
    EXPECT_EQ(analyze(Literal, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(8, false)}), Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(8, false), static_cast<std::uint64_t>(255)));
    auto *Variable = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    ASSERT_NE(Variable, nullptr);
    AnalyzerState State(Context, Storage, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Written"), *Variable), BindResult::Inserted);
    const auto *Result = analyzeExpr(State, Storage, {.ExpectedType = Variable->type()});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Variable);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Module and type members resolve semantic values, constants and storage through the same expression path.
  TEST_F(SemanticAnalyzerExprTest, ResolvesSemanticAndRuntimeMemberExpressions)
  {
    ir::Module Package(Context.irContext());
    ir::Module Nested(Context.irContext());
    auto *Type = Package.createClassType(4, 4);
    auto *Variable = Package.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    const auto *Constant = Package.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Type, nullptr);
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Constant, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_EQ(Resolver.bind(name("T"), *Type), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Nested"), Nested), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(*Type), nullptr);
    ASSERT_EQ(Resolver.bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
    ASSERT_TRUE(Resolver.exitScope());
    ASSERT_TRUE(Resolver.exitScope());
    struct Case
    {
        const char *Text;
        const ir::Value *Value;
        const ir::Type *Type;
    };
    const Case Cases[] = {
        {"((Pkg)).T", Type, Context.typePool().get<ink::ir::MetaType>()},
        {"(Pkg.Nested)", &Nested, nullptr},
        {"(Pkg.T).Count", Constant, Constant->type()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Result = analyze(Input);
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result, Entry.Value);
      EXPECT_EQ(Result->type(), Entry.Type);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    auto StorageInput = read("((Pkg).Written)");
    ASSERT_TRUE(StorageInput.succeeded());
    AnalyzerState StorageState(Context, StorageInput, Output);
    const auto *Storage = analyzeExpr(StorageState, StorageInput);
    ASSERT_NE(Storage, nullptr);
    EXPECT_EQ(Storage, Variable);
    EXPECT_EQ(Storage->type(), Variable->type());
    const auto *Found = StorageState.nameResolver().lookupMember(Package, name("Written"));
    ASSERT_NE(Found, nullptr);
    EXPECT_TRUE(Storage->isStorage());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Module and type variable members preserve storage identity and emit loads, warning only for uninitialized member storage.
  TEST_F(SemanticAnalyzerExprTest, AnalyzesVariableMembersAndEmitsValues)
  {
    auto SetupInput = read("0");
    ASSERT_TRUE(SetupInput.succeeded());
    AnalyzerState SetupState(Context, SetupInput, Output);
    auto *Block = createInsertBlock(SetupState);
    ASSERT_NE(Block, nullptr);
    ir::Module Package(Context.irContext());
    auto *Type = Package.createClassType(4, 4);
    ASSERT_NE(Type, nullptr);
    const auto *DeclaredType = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Storage = SetupState.irBuilder().createAlloca(*DeclaredType);
    auto *PendingStorage = SetupState.irBuilder().createAlloca(*DeclaredType);
    const auto *InitialValue = Output.constantPool().get<ink::ir::IntegerConstant>(DeclaredType, static_cast<std::int64_t>(14));
    ASSERT_NE(Storage, nullptr);
    ASSERT_NE(PendingStorage, nullptr);
    ASSERT_NE(InitialValue, nullptr);
    ASSERT_NE(SetupState.irBuilder().createInitialize(*InitialValue, *Storage), nullptr);
    auto &Resolver = SetupState.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_EQ(Resolver.bind(name("T"), *Type), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Stored"), *Storage), BindResult::Inserted);
    const auto *ModuleVariable = Resolver.lookupLocal(name("Stored"));
    ASSERT_EQ(Resolver.bind(name("Pending"), *PendingStorage), BindResult::Inserted);
    const auto *PendingVariable = Resolver.lookupLocal(name("Pending"));
    ASSERT_NE(Resolver.enterScope(*Type), nullptr);
    ASSERT_EQ(Resolver.bind(name("Stored"), *Storage), BindResult::Inserted);
    const auto *TypeVariable = Resolver.lookupLocal(name("Stored"));
    ASSERT_NE(ModuleVariable, nullptr);
    ASSERT_NE(PendingVariable, nullptr);
    ASSERT_NE(TypeVariable, nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    ASSERT_TRUE(Resolver.exitScope());
    struct Case
    {
        const char *Text;
        const ir::Value *Storage;
        const ir::Value *Owner;
        const Binding *Variable;
        bool Initialized;
    };
    const Case Cases[] = {
        {"Pkg.Stored", Storage, &Package, ModuleVariable, true},
        {"((Pkg)).Stored", Storage, &Package, ModuleVariable, true},
        {"((Pkg.T).Stored)", Storage, Type, TypeVariable, true},
        {"((Pkg.Pending))", PendingStorage, &Package, PendingVariable, false},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
      const auto InstructionCount = Block->instructionCount();
      const auto *Address = analyzeExpr(State, Input);
      ASSERT_NE(Address, nullptr);
      EXPECT_EQ(Address, Entry.Storage);
      EXPECT_EQ(Address->type(), Entry.Storage->type());
      const auto *Found = State.nameResolver().lookupMember(*Entry.Owner, Entry.Variable->name());
      ASSERT_EQ(Found, Entry.Variable);
      EXPECT_EQ(Address->type(), DeclaredType);
      EXPECT_TRUE(Address->isStorage());
      EXPECT_TRUE(Address->isMutable());
      EXPECT_EQ(Address->isInitialized(), Entry.Initialized);
      EXPECT_EQ(Block->instructionCount(), InstructionCount);
      const auto *Result = analyze(State, Input);
      ASSERT_NE(Result, nullptr);
      ASSERT_TRUE(ir::LoadInstruction::classof(Result));
      EXPECT_EQ(Result->type(), DeclaredType);
      EXPECT_EQ(static_cast<const ir::LoadInstruction *>(Result)->pointer(), Entry.Storage);
      EXPECT_EQ(Block->instructionCount(), InstructionCount + 1);
      EXPECT_EQ(Entry.Storage->isInitialized(), Entry.Initialized);
      if (Entry.Initialized)
      {
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
      }
      else
      {
        expectUninitializedWarning(Input, syntax(Input).getSourceRange());
      }
    }
    Context.scopeStore().removeTarget(*Storage);
    Context.scopeStore().removeTarget(*PendingStorage);
    Context.scopeStore().removeTarget(*Type);
    Context.scopeStore().removeTarget(Package);
  }

  // Failed member lookup reports one unknown-name diagnostic and never falls back to an outer lexical binding.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnknownMemberWithoutLexicalFallback)
  {
    auto Input = read("(Pkg).Missing");
    ASSERT_TRUE(Input.succeeded());
    ir::Module Package(Context.irContext());
    auto *Type = Output.createClassType(4, 4);
    ASSERT_NE(Type, nullptr);
    AnalyzerState State(Context, Input, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Missing"), *Type), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    expectFailure(analyze(State, Input), core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Source, Input.Unit->input().lexedFile().sourceId());
  }

  // Current-function parameters resolve as values, while absent or mismatched owning functions are internal model errors.
  TEST_F(SemanticAnalyzerExprTest, ValidatesParameterFunctionOwnership)
  {
    auto Input = read("Argument");
    ASSERT_TRUE(Input.succeeded());
    const auto *Signature = Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Context.typePool().get<ink::ir::IntegerType>(32, false))});
    auto *Current = Output.createGlobalFunction(*Signature);
    auto *Other = Output.createGlobalFunction(*Signature);
    ASSERT_NE(Current, nullptr);
    ASSERT_NE(Other, nullptr);
    auto *Block = Current->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Current->parameter(0)), BindResult::Inserted);
    const auto *Result = analyze(State, Input);
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Current->parameter(0));
    EXPECT_EQ(Result->type(), Context.typePool().get<ink::ir::IntegerType>(32, false));
    State.nameResolver().enterScope();
    ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Other->parameter(0)), BindResult::Inserted);
    EXPECT_DEATH(analyze(State, Input), "INK-S0013");
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    ASSERT_TRUE(State.nameResolver().exitScope());
    State.irBuilder().clearInsertPoint();
    Diagnostics.clear();
    EXPECT_DEATH(analyze(State, Input), "INK-S0013");
    EXPECT_EQ(State.irBuilder().insertBlock(), nullptr);
    EXPECT_EQ(Block->instructionCount(), 0U);
  }

  // Appended instructions retain identity in their function; mismatched owners and detached instructions or blocks report internal model errors.
  TEST_F(SemanticAnalyzerExprTest, ResolvesCurrentFunctionInstructionValues)
  {
    auto &Types = Context.typePool();
    auto *Current = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Other = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Callee = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::IntegerType>(32, false)));
    ASSERT_NE(Current, nullptr);
    ASSERT_NE(Other, nullptr);
    ASSERT_NE(Callee, nullptr);
    auto *CurrentBlock = Current->createBlock();
    auto *SiblingBlock = Current->createBlock();
    auto *OtherBlock = Other->createBlock();
    ASSERT_NE(CurrentBlock, nullptr);
    ASSERT_NE(SiblingBlock, nullptr);
    ASSERT_NE(OtherBlock, nullptr);
    auto *Call = CurrentBlock->appendInstruction(std::make_unique<ir::CallInstruction>(*Callee));
    ASSERT_NE(Call, nullptr);
    ir::CallInstruction Detached(*Callee);
    ir::BasicBlock DetachedBlock;
    auto *AttachedToDetachedBlock = DetachedBlock.appendInstruction(std::make_unique<ir::CallInstruction>(*Callee));
    ASSERT_NE(AttachedToDetachedBlock, nullptr);
    ir::Instruction *DetachedValues[] = {&Detached, AttachedToDetachedBlock};
    for (const auto *Expression : {"Handle", "(((Handle)))"})
    {
      SCOPED_TRACE(Expression);
      auto Input = read(Expression);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Handle"), *Call), BindResult::Inserted);
      for (auto *Block : {CurrentBlock, SiblingBlock})
      {
        ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
        const auto *Result = analyze(State, Input);
        ASSERT_NE(Result, nullptr);
        EXPECT_EQ(Result, Call);
        EXPECT_EQ(Result->type(), Types.get<ink::ir::IntegerType>(32, false));
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
        EXPECT_EQ(State.irBuilder().insertBlock(), Block);
      }
      ASSERT_TRUE(State.irBuilder().setInsertPoint(*OtherBlock));
      EXPECT_DEATH(analyze(State, Input), "INK-S0013");
      EXPECT_EQ(State.irBuilder().insertBlock(), OtherBlock);
      for (auto *Value : DetachedValues)
      {
        State.nameResolver().enterScope();
        ASSERT_EQ(State.nameResolver().bind(name("Handle"), *Value), BindResult::Inserted);
        Diagnostics.clear();
        ASSERT_TRUE(State.irBuilder().setInsertPoint(*CurrentBlock));
        EXPECT_DEATH(analyze(State, Input), "INK-S0013");
        EXPECT_EQ(State.irBuilder().insertBlock(), CurrentBlock);
        Diagnostics.clear();
        State.irBuilder().clearInsertPoint();
        EXPECT_DEATH(analyze(State, Input), "INK-S0013");
        EXPECT_EQ(State.irBuilder().insertBlock(), nullptr);
        ASSERT_TRUE(State.nameResolver().exitScope());
      }
      EXPECT_EQ(State.recursionDepth(), 0U);
      EXPECT_EQ(CurrentBlock->instructionCount(), 1U);
      EXPECT_EQ(CurrentBlock->instruction(0), Call);
      EXPECT_EQ(SiblingBlock->instructionCount(), 0U);
      EXPECT_EQ(OtherBlock->instructionCount(), 0U);
      EXPECT_EQ(DetachedBlock.instructionCount(), 1U);
      EXPECT_EQ(DetachedBlock.instruction(0), AttachedToDetachedBlock);
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // Manually constructed storage whose type has no runtime layout violates the analyzer's internal model contract.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForStorageWithoutRuntimeLayout)
  {
    auto Input = read("Pack");
    ASSERT_TRUE(Input.succeeded());
    ir::GlobalVariable Variable(*Context.typePool().get<ink::ir::VariadicType>());
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Pack"), Variable), BindResult::Inserted);
    EXPECT_DEATH(analyze(State, Input), "INK-S0013");
  }

  // Names may refer to another module in the same IR context, while foreign-context targets report an internal model error.
  TEST_F(SemanticAnalyzerExprTest, EnforcesNameTargetContextCompatibility)
  {
    auto Input = read("Target");
    ASSERT_TRUE(Input.succeeded());
    ir::Module Sibling(Context.irContext());
    ir::IRContext ForeignContext(target::TargetContext::native());
    ir::Module Foreign(ForeignContext);
    auto *Shared = Sibling.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    ASSERT_NE(Shared, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Target"), *Shared), BindResult::Inserted);
    EXPECT_EQ(analyzeExpr(State, Input), Shared);
    auto *ForeignStorage = Foreign.createGlobalVariable(*Foreign.typePool().get<ink::ir::IntegerType>(32, false));
    auto *ForeignFunction = Foreign.createGlobalFunction(*Foreign.typePool().get<ink::ir::FunctionType>(Foreign.typePool().get<ink::ir::VoidType>()));
    auto *ForeignType = Foreign.createClassType(4, 4);
    const auto *ForeignConstant = Foreign.constantPool().get<ink::ir::IntegerConstant>(Foreign.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(1));
    ASSERT_NE(ForeignStorage, nullptr);
    ASSERT_NE(ForeignFunction, nullptr);
    ASSERT_NE(ForeignType, nullptr);
    ASSERT_NE(ForeignConstant, nullptr);
    ir::Value *ForeignTargets[] = {ForeignStorage, ForeignFunction, ForeignType, &Foreign, const_cast<ir::IntegerConstant *>(ForeignConstant)};
    for (auto *Target : ForeignTargets)
    {
      Diagnostics.clear();
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Target"), *Target), BindResult::Inserted);
      EXPECT_DEATH(analyze(State, Input), "INK-S0013");
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // Foreign expected-type hints violate the analyzer contract before literal construction or name lookup.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForForeignExpectedTypeHints)
  {
    ir::IRContext Foreign(target::TargetContext::native());
    struct Case
    {
        const char *Text;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"1", Foreign.typePool().get<ink::ir::IntegerType>(32, true)},
        {"1.5", Foreign.typePool().get<ink::ir::FloatType>(64)},
        {"'a'", Foreign.typePool().get<ink::ir::IntegerType>(32, false)},
        {"true", Foreign.typePool().get<ink::ir::BoolType>()},
        {"\"a\"", Foreign.typePool().get<ink::ir::ArrayType>(Foreign.typePool().get<ink::ir::IntegerType>(8, false), 1)},
        {"Missing", Foreign.typePool().get<ink::ir::IntegerType>(32, true)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input, {.ExpectedType = Entry.Expected}), "INK-S0013");
    }
  }

  // Foreign integer-default hints violate the contract even when unused or overridden by an explicit local expected type.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForForeignDefaultIntegerLiteralHints)
  {
    ir::IRContext Foreign(target::TargetContext::native());
    const auto *ForeignDefault = Foreign.typePool().get<ink::ir::IntegerType>(64, false);
    constexpr const char *Cases[] = {
        "1",
        "1.5",
        "true",
        "Missing",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input, {.DefaultIntegerLiteralType = ForeignDefault}), "INK-S0013");
    }
    auto Input = read("1");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_DEATH(analyze(Input, {.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(32, true), .DefaultIntegerLiteralType = ForeignDefault}), "INK-S0013");
  }

  // A registered target without layout reaches literal-category and value-compatibility checks instead of failing at expression entry.
  TEST_F(SemanticAnalyzerExprTest, DefersLayoutlessExpectedTypesToExpressionConsumers)
  {
    auto &Types = Context.typePool();
    const auto *ExpectedType = Types.get<ink::ir::VariadicType>();
    ASSERT_TRUE(Context.irContext().containsType(ExpectedType));
    ASSERT_FALSE(ExpectedType->hasLayout());
    auto LiteralInput = read("(1)");
    ASSERT_TRUE(LiteralInput.succeeded());
    const auto *Parentheses = frontend::parser::cast<frontend::parser::ParenExpr>(&syntax(LiteralInput));
    ASSERT_NE(Parentheses, nullptr);
    expectFailure(analyze(LiteralInput, {.ExpectedType = ExpectedType}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected an integer type, got " + ExpectedType->toString());
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, Parentheses->expression()->getSourceRange());
    auto BoundInput = read("Count");
    ASSERT_TRUE(BoundInput.succeeded());
    AnalyzerState State(Context, BoundInput, Output);
    const auto *Constant = Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Constant, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
    expectFailure(analyze(State, BoundInput, {.ExpectedType = ExpectedType}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected " + ExpectedType->toString() + ", got i32");
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(BoundInput).getSourceRange());
  }

  // Local targets without layout preserve unknown-name diagnostics through parentheses and numeric negation.
  TEST_F(SemanticAnalyzerExprTest, PreservesUnknownNamesWithLayoutlessExpectedTypeHints)
  {
    const auto *ExpectedType = Context.typePool().get<ink::ir::VariadicType>();
    ASSERT_FALSE(ExpectedType->hasLayout());
    constexpr const char *Cases[] = {
        "Missing",
        "((Missing))",
        "-((Missing))",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Expression = &syntax(Input);
      while (frontend::parser::ParenExpr::classof(Expression) || frontend::parser::UnaryExpr::classof(Expression))
      {
        Expression = frontend::parser::ParenExpr::classof(Expression) ? static_cast<const frontend::parser::ParenExpr *>(Expression)->expression() : static_cast<const frontend::parser::UnaryExpr *>(Expression)->operand();
      }
      ASSERT_TRUE(frontend::parser::NameExpr::classof(Expression));
      expectFailure(analyze(Input, {.ExpectedType = ExpectedType, .DefaultIntegerLiteralType = Context.typePool().get<ink::ir::IntegerType>(8, false)}), core::DiagnosticKind::SemanticUnknownName);
      expectMessage("unknown name 'Missing'");
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, Expression->getSourceRange());
      EXPECT_EQ(Diagnostics.diagnostics()[0].Source, Input.Unit->input().lexedFile().sourceId());
    }
  }

  // Leaf analysis consumes one semantic recursion level and restores the caller's depth after both successful and failed lookup.
  TEST_F(SemanticAnalyzerExprTest, EnforcesAndRestoresRecursionDepth)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("1"));
    auto Valid = read("14");
    auto Missing = read("Missing");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Missing.succeeded());
    AnalyzerState ValidState(Context, Valid, Output);
    EXPECT_TRUE(analyze(ValidState, Valid));
    EXPECT_EQ(ValidState.recursionDepth(), 0U);
    AnalyzerState State(Context, Missing, Output);
    expectFailure(analyze(State, Missing), core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(State.recursionDepth(), 0U);
    Diagnostics.clear();
    const auto *Constant = Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(14));
    ASSERT_NE(Constant, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Missing"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Missing), Constant);
    EXPECT_EQ(analyze(State, Missing), Constant);
    EXPECT_EQ(State.recursionDepth(), 0U);
    ASSERT_TRUE(Limit.set("0"));
    EXPECT_DEATH(analyze(Valid), "INK-S0014");
  }

  // Negative-literal analysis obeys the depth guard for every unary, parenthesis and literal, restoring depth after recoverable overflow.
  TEST_F(SemanticAnalyzerExprTest, PreservesRecursionGuardsForNegativeLiterals)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("2"));
    auto Valid = read("-127");
    auto Overflow = read("-129");
    auto Parenthesized = read("-(127)");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Overflow.succeeded());
    ASSERT_TRUE(Parenthesized.succeeded());
    const ExprHints Hints{.ExpectedType = Context.typePool().get<ink::ir::IntegerType>(8, true)};
    AnalyzerState ValidState(Context, Valid, Output);
    ASSERT_NE(createInsertBlock(ValidState), nullptr);
    expectNegation(analyze(ValidState, Valid, Hints), Output.constantPool().get<ink::ir::IntegerConstant>(Context.typePool().get<ink::ir::IntegerType>(8, true), static_cast<std::int64_t>(127)));
    EXPECT_EQ(ValidState.recursionDepth(), 0U);
    AnalyzerState OverflowState(Context, Overflow, Output);
    expectFailure(analyze(OverflowState, Overflow, Hints), core::DiagnosticKind::SemanticIntegerOutOfRange);
    EXPECT_EQ(OverflowState.recursionDepth(), 0U);
    EXPECT_DEATH(analyze(Parenthesized, Hints), "INK-S0014");
    ASSERT_TRUE(Limit.set("1"));
    EXPECT_DEATH(analyze(Valid, Hints), "INK-S0014");
  }

  // Global pointer reads and runtime array repetition retain their unsupported-operation diagnostics.
  TEST_F(SemanticAnalyzerExprTest, KeepsGlobalPointerReadsAndArrayRepetitionUnsupported)
  {
    auto *Variable = Output.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, false));
    auto *Pointer = Output.createGlobalVariable(*Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(32, false)));
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Pointer, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Pointer"), *Pointer), BindResult::Inserted);
    constexpr const char *Cases[] = {
        "*Pointer",
        "[Written; 3]",
        "[14; 3]",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "INK-S0012");
    }
  }

  // Dereferencing a current-function pointer parameter selects the runtime operation instead of requiring a type expression.
  TEST_F(SemanticAnalyzerExprTest, LoadsPointerParameterPointee)
  {
    auto Input = read("*Argument");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const auto *Signature = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {ir::FunctionTypeParameter(Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, false)))});
    auto *Function = Output.createGlobalFunction(*Signature);
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Function->parameter(0)), BindResult::Inserted);
    const auto *Result = analyze(State, Input);
    ASSERT_TRUE(ir::LoadInstruction::classof(Result));
    const auto *Storage = static_cast<const ir::LoadInstruction *>(Result)->pointer();
    ASSERT_TRUE(ir::DereferenceInstruction::classof(Storage));
    EXPECT_EQ(static_cast<const ir::DereferenceInstruction *>(Storage)->pointer(), Function->parameter(0));
    EXPECT_EQ(Result->type(), Types.get<ink::ir::IntegerType>(32, false));
    EXPECT_EQ(Block->instructionCount(), 2U);
    EXPECT_EQ(Block->instruction(0), Storage);
    EXPECT_EQ(Block->instruction(1), Result);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Context.scopeStore().removeTarget(*Function->parameter(0));
  }

  // Explicit comptime expressions follow the common unsupported-feature policy regardless of their operand category.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnsupportedComptimeExpressions)
  {
    constexpr const char *Cases[] = {
        "comptime i32",
        "comptime 14",
        "comptime (*i32)",
        "comptime -128",
        "-comptime 128",
        "-(comptime 128)",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "INK-S0012");
    }
  }

  // Call analysis reports an unknown callee by name before attempting to construct a call instruction.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnknownCallCallee)
  {
    auto Input = read("WriteFile()");
    ASSERT_TRUE(Input.succeeded());
    expectFailure(analyze(Input), core::DiagnosticKind::SemanticUnknownName);
    expectMessage("unknown name 'WriteFile'");
  }
} // namespace ink::semantic::test
