#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/config/numeric_literal.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"
#include "ink/frontend/parser/parser.h"
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
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
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

        const frontend::parser::Expr &syntax(const frontend::parser::ParseResult &Input)
        {
          const auto *Statement = frontend::parser::cast<frontend::parser::DeclStmt>(Input.Unit->root()->statements()[0]);
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
          auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
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
      EXPECT_EQ(Result, Output.constantPool().getSignedIntegerConstant(Context.typePool().getInt32Type(), Entry.Value));
      EXPECT_EQ(Result->type(), Context.typePool().getInt32Type());
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
        {"4294967295", Constants.getUnsignedIntegerConstant(Types.getUInt64Type(), 4294967295ULL)},
        {"((18446744073709551615))", Constants.getUnsignedIntegerConstant(Types.getUInt64Type(), std::numeric_limits<std::uint64_t>::max())},
        {"1.5", Constants.getFloatConstant(1.5)},
        {"'a'", Constants.getUnsignedIntegerConstant(Types.getUInt32Type(), 97)},
        {"true", Constants.getBoolConstant(true)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      ASSERT_NE(Entry.Expected, nullptr);
      EXPECT_EQ(analyze(Input, {.DefaultIntegerLiteralType = Types.getUInt64Type()}), Entry.Expected);
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
    const ExprHints Hints{.ExpectedType = Types.getUInt8Type(), .DefaultIntegerLiteralType = Types.getUInt64Type()};
    EXPECT_EQ(analyze(Valid, Hints), Output.constantPool().getUnsignedIntegerConstant(Types.getUInt8Type(), 255));
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
    const ExprHints Hints{.DefaultIntegerLiteralType = Types.getUInt8Type()};
    EXPECT_EQ(analyze(Valid, Hints), Output.constantPool().getUnsignedIntegerConstant(Types.getUInt8Type(), 255));
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
    const auto *Expected = Types.getArrayType(Types.getUInt8Type(), 256);
    ASSERT_NE(Expected, nullptr);
    EXPECT_EQ(analyze(Input, {.DefaultIntegerLiteralType = Types.getUInt8Type()}), Expected);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Unary operands, member receivers and repeated-array elements use independent defaults before reporting their unimplemented runtime operation.
  TEST_F(SemanticAnalyzerExprTest, ResetsLiteralHintsForDistinctOperandPositions)
  {
    constexpr const char *Cases[] = {
        "*(256)",
        "((256)).Missing",
        "[256; 1]",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input, {.DefaultIntegerLiteralType = Context.typePool().getUInt8Type()}), "INK-S0012");
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
        {"127", Types.getInt8Type(), 127},
        {"255", Types.getUInt8Type(), 255},
        {"32767", Types.getInt16Type(), 32767},
        {"65535", Types.getUInt16Type(), 65535},
        {"2147483647", Types.getInt32Type(), 2147483647},
        {"4294967295", Types.getUInt32Type(), 4294967295ULL},
        {"0xFFFFFFF5", Types.getUInt32Type(), 0xFFFFFFF5ULL},
        {"9223372036854775807", Types.getInt64Type(), 9223372036854775807ULL},
        {"18446744073709551615", Types.getUInt64Type(), std::numeric_limits<std::uint64_t>::max()},
        {"0xFFFFFFFFFFFFFFFF", Types.getUInt64Type(), std::numeric_limits<std::uint64_t>::max()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Result = analyze(Input, {.ExpectedType = Entry.Type});
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result, Output.constantPool().getIntegerConstantFromBits(Entry.Type, Entry.Bits));
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
        {"128", Types.getInt8Type(), "integer literal is out of range for i8"},
        {"256", Types.getUInt8Type(), "integer literal is out of range for u8"},
        {"32768", Types.getInt16Type(), "integer literal is out of range for i16"},
        {"65536", Types.getUInt16Type(), "integer literal is out of range for u16"},
        {"2147483648", nullptr, "integer literal is out of range for i32"},
        {"0xFFFFFFF5", nullptr, "integer literal is out of range for i32"},
        {"4294967296", Types.getUInt32Type(), "integer literal is out of range for u32"},
        {"9223372036854775808", Types.getInt64Type(), "integer literal is out of range for i64"},
        {"18446744073709551616", Types.getUInt64Type(), "integer literal is out of range for u64"},
        {"0x10000000000000000", Types.getUInt64Type(), "integer literal is out of range for u64"},
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
        {"-127", Types.getInt8Type(), -std::numeric_limits<std::int8_t>::max()},
        {"(-127)", Types.getInt8Type(), -std::numeric_limits<std::int8_t>::max()},
        {"-(127)", Types.getInt8Type(), -std::numeric_limits<std::int8_t>::max()},
        {"-0b1111111", Types.getInt8Type(), -std::numeric_limits<std::int8_t>::max()},
        {"-32767", Types.getInt16Type(), -std::numeric_limits<std::int16_t>::max()},
        {"-((32767))", Types.getInt16Type(), -std::numeric_limits<std::int16_t>::max()},
        {"-0o77777", Types.getInt16Type(), -std::numeric_limits<std::int16_t>::max()},
        {"-2147483647", Types.getInt32Type(), -std::numeric_limits<std::int32_t>::max()},
        {"-0x7FFFFFFF", Types.getInt32Type(), -std::numeric_limits<std::int32_t>::max()},
        {"-9223372036854775807", Types.getInt64Type(), -std::numeric_limits<std::int64_t>::max()},
        {"-((9223372036854775807))", Types.getInt64Type(), -std::numeric_limits<std::int64_t>::max()},
        {"-0x7FFFFFFFFFFFFFFF", Types.getInt64Type(), -std::numeric_limits<std::int64_t>::max()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      const auto *Expected = Output.constantPool().getSignedIntegerConstant(Entry.Type, -Entry.Value);
      ASSERT_NE(Expected, nullptr);
      expectNegation(analyze(Input, {.ExpectedType = Entry.Type}), Expected);
      expectNegation(analyze(Input, {.DefaultIntegerLiteralType = Entry.Type}), Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    auto Input = read("-2147483647");
    ASSERT_TRUE(Input.succeeded());
    expectNegation(analyze(Input), Output.constantPool().getSignedIntegerConstant(Types.getInt32Type(), std::numeric_limits<std::int32_t>::max()));
    auto Narrow = read("-(127)");
    ASSERT_TRUE(Narrow.succeeded());
    expectNegation(analyze(Narrow, {.ExpectedType = Types.getInt8Type(), .DefaultIntegerLiteralType = Types.getUInt64Type()}), Output.constantPool().getSignedIntegerConstant(Types.getInt8Type(), std::numeric_limits<std::int8_t>::max()));
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
        {"-128", Types.getInt8Type()},
        {"-129", Types.getInt8Type()},
        {"-0x81", Types.getInt8Type()},
        {"-32768", Types.getInt16Type()},
        {"-32769", Types.getInt16Type()},
        {"-2147483648", Types.getInt32Type()},
        {"-2147483649", Types.getInt32Type()},
        {"-9223372036854775808", Types.getInt64Type()},
        {"-9223372036854775809", Types.getInt64Type()},
        {"-18446744073709551616", Types.getInt64Type()},
        {"-(-128)", Types.getInt8Type()},
        {"-(-32768)", Types.getInt16Type()},
        {"-(-2147483648)", Types.getInt32Type()},
        {"-(-9223372036854775808)", Types.getInt64Type()},
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
        Types.getInt8Type(),
        Types.getUInt8Type(),
        Types.getInt16Type(),
        Types.getUInt16Type(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
    };
    for (const auto *Type : IntegerTypes)
    {
      SCOPED_TRACE(Type->toString());
      for (const char *Text : {"-((0))", "-1"})
      {
        auto Input = read(Text);
        ASSERT_TRUE(Input.succeeded());
        expectNegation(analyze(Input, {.ExpectedType = Type}), Output.constantPool().getIntegerConstantFromBits(Type, std::string_view(Text) == "-1" ? 1 : 0));
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
      }
    }
    auto Input = read("-(-127)");
    ASSERT_TRUE(Input.succeeded());
    const auto *Result = analyze(Input, {.ExpectedType = Types.getInt8Type()});
    ASSERT_NE(Result, nullptr);
    ASSERT_TRUE(ir::NegInstruction::classof(Result));
    const auto *Inner = static_cast<const ir::NegInstruction *>(Result)->operand();
    expectNegation(Result, Inner);
    expectNegation(Inner, Output.constantPool().getSignedIntegerConstant(Types.getInt8Type(), 127));
    auto Unsigned = read("-18446744073709551615");
    ASSERT_TRUE(Unsigned.succeeded());
    expectNegation(analyze(Unsigned, {.ExpectedType = Types.getUInt64Type()}), Output.constantPool().getUnsignedIntegerConstant(Types.getUInt64Type(), std::numeric_limits<std::uint64_t>::max()));
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
        {Types.getInt8Type(), std::numeric_limits<std::int8_t>::min()},
        {Types.getInt16Type(), std::numeric_limits<std::int16_t>::min()},
        {Types.getInt32Type(), std::numeric_limits<std::int32_t>::min()},
        {Types.getInt64Type(), std::numeric_limits<std::int64_t>::min()},
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
        const auto *Constant = Output.constantPool().getSignedIntegerConstant(Entry.Type, Value);
        ASSERT_NE(Constant, nullptr);
        ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
        expectNegation(analyze(State, Input, {.DefaultIntegerLiteralType = Types.getUInt64Type()}), Constant);
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
      const auto *Constant = Output.constantPool().getUnsignedIntegerConstant(Types.getUInt16Type(), Value);
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
        Types.getInt8Type(),
        Types.getUInt8Type(),
        Types.getInt16Type(),
        Types.getUInt16Type(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
        Types.getFloat32Type(),
        Types.getFloat64Type(),
    };
    for (const auto *Type : NumericTypes)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-Argument");
      ASSERT_TRUE(Input.succeeded());
      const auto *Signature = Types.getFunctionType(Type, {ir::FunctionTypeParameter(Type)});
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
    const ir::Type *NumericTypes[] = {Types.getInt32Type(), Types.getFloat64Type()};
    for (const auto *Type : NumericTypes)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-(-Produced)");
      ASSERT_TRUE(Input.succeeded());
      auto *Function = Output.createGlobalFunction(*Types.getFunctionType(Type));
      auto *Producer = Output.createGlobalFunction(*Types.getFunctionType(Type));
      auto *Consumer = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType(), {ir::FunctionTypeParameter(Type)}));
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
    const auto *Signature = Types.getFunctionType(Types.getVoidType(), {ir::FunctionTypeParameter(Types.getInt32Type())});
    auto *Current = Output.createGlobalFunction(*Signature);
    auto *Other = Output.createGlobalFunction(*Signature);
    auto *Producer = Output.createGlobalFunction(*Types.getFunctionType(Types.getInt32Type()));
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
    auto *ForeignFunction = Foreign.createGlobalFunction(*Foreign.typePool().getFunctionType(Foreign.typePool().getVoidType(), {ir::FunctionTypeParameter(Foreign.typePool().getInt32Type())}));
    const auto *ForeignConstant = Foreign.constantPool().getSignedIntegerConstant(Foreign.typePool().getInt32Type(), 1);
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
    auto *Function = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType(), {ir::FunctionTypeParameter(Types.getInt32Type())}));
    auto *Producer = Output.createGlobalFunction(*Types.getFunctionType(Types.getInt32Type()));
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
    EXPECT_DEATH(analyze(State, Input), "check failed: canAppend");
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
      EXPECT_DEATH(analyze(State, Input), "check failed: canAppend");
    }
  }

  // Negation of storage or a nonnumeric value remains unsupported with or without an insertion block.
  TEST_F(SemanticAnalyzerExprTest, ReportsUnsupportedForStorageAndNonnumericNegationOperands)
  {
    auto &Types = Context.typePool();
    auto *Variable = Output.createGlobalVariable(*Types.getInt32Type());
    auto *Function = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType()));
    ir::Module Package(Context.irContext());
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Function, nullptr);
    const auto *Initializer = Output.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 14);
    ASSERT_NE(Initializer, nullptr);
    ASSERT_TRUE(Variable->setInitializer(Initializer));
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Callable"), *Function), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("nullptr"), const_cast<ir::NullPointerConstant &>(*Output.constantPool().getNullPointerConstant())), BindResult::Inserted);
    constexpr const char *Cases[] = {
        "-Written",
        "-true",
        "-((true))",
        "-i32",
        "-Pkg",
        "-nullptr",
        "-\"abc\"",
        "-Callable",
    };
    for (const char *Text : Cases)
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
        EXPECT_DEATH(analyze(State, Input), "INK-S0012");
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
    const auto *CallableType = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(CallableType, nullptr);
    const ir::Type *ParameterTypes[] = {
        Types.getBoolType(),
        Types.getPointerType(Types.getInt32Type()),
        CallableType,
        Types.getVoidType(),
    };
    for (const auto *Type : ParameterTypes)
    {
      SCOPED_TRACE(Type->toString());
      auto Input = read("-Argument");
      ASSERT_TRUE(Input.succeeded());
      const auto *Signature = Types.getFunctionType(Types.getVoidType(), {ir::FunctionTypeParameter(Type)});
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

  // A restored nondecimal integer token must contain its two-byte prefix and at least one digit.
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
      std::vector<frontend::tokenizer::Token> Tokens = {
          {frontend::tokenizer::TokenKind::IntegerLiteral, core::SourceRange::fromByteOffsets(0, Length), frontend::tokenizer::NumericInfo{Entry.Base}},
          {frontend::tokenizer::TokenKind::Semicolon, core::SourceRange::fromByteOffsets(Length, Length + 1), {}},
          {frontend::tokenizer::TokenKind::EndOfFile, core::SourceRange::fromByteOffsets(Length + 1, Length + 1), {}},
      };
      auto Lexed = frontend::tokenizer::TokenizedBuffer::fromSnapshot(Sources, "<truncated integer snapshot>", std::string(Entry.Text) + ";", std::move(Tokens), true);
      ASSERT_TRUE(Lexed.has_value());
      auto Input = frontend::parser::parse(Frontend, std::move(*Lexed));
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
      std::vector<frontend::tokenizer::Token> Tokens = {
          {frontend::tokenizer::TokenKind::IntegerLiteral, core::SourceRange::fromByteOffsets(0, Length), frontend::tokenizer::NumericInfo{Entry.Base}},
          {frontend::tokenizer::TokenKind::Semicolon, core::SourceRange::fromByteOffsets(Length, Length + 1), {}},
          {frontend::tokenizer::TokenKind::EndOfFile, core::SourceRange::fromByteOffsets(Length + 1, Length + 1), {}},
      };
      auto Lexed = frontend::tokenizer::TokenizedBuffer::fromSnapshot(Sources, "<malformed integer snapshot>", std::string(Entry.Text) + ";", std::move(Tokens), true);
      ASSERT_TRUE(Lexed.has_value());
      auto Input = frontend::parser::parse(Frontend, std::move(*Lexed));
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
    EXPECT_DEATH(Analysis.analyzeExpr(State, *Expression), "INK-S0061.*invalid AST: expression node kind must belong to the expression AST category");
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
      EXPECT_DEATH(Analysis.analyzeExpr(State, *Literal), "INK-S0061.*invalid AST: LiteralExpr token index must reference a token in its input");
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
      EXPECT_DEATH(Analysis.analyzeExpr(State, *Literal), "INK-S0061.*invalid AST: LiteralExpr kind must match its referenced token kind");
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
    EXPECT_DEATH(Analysis.analyzeExpr(State, *Expression), "INK-S0061.*invalid AST: FunctionTypeExpr requires a return type annotation");
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
    EXPECT_DEATH(Analysis.analyzeExpr(State, *Expression), "INK-S0061.*invalid AST: FunctionTypeExpr parameter requires a type annotation");
  }

  // Integer literals cannot initialize bool, float or pointer destinations through implicit numeric conversions.
  TEST_F(SemanticAnalyzerExprTest, RejectsNonintegerDestinationsForIntegerLiterals)
  {
    auto Input = read("1");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const ir::Type *ExpectedTypes[] = {Types.getBoolType(), Types.getFloat64Type(), Types.getPointerType(Types.getUInt8Type())};
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
    EXPECT_EQ(Default, Output.constantPool().getFloatConstant(1.5));
    EXPECT_EQ(Default->type(), Context.typePool().getFloat64Type());
    const auto *Narrow = analyze(Fraction, {.ExpectedType = Context.typePool().getFloat32Type()});
    ASSERT_NE(Narrow, nullptr);
    EXPECT_EQ(Narrow, Output.constantPool().getFloatConstant(1.5F));
    EXPECT_EQ(Narrow->type(), Context.typePool().getFloat32Type());
    EXPECT_EQ(analyze(Exponent), Output.constantPool().getFloatConstant(100.0));
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
        {"-1.5", Types.getFloat64Type(), Constants.getFloatConstant(1.5), false},
        {"-(1.5)", Types.getFloat32Type(), Constants.getFloatConstant(1.5F), false},
        {"-0.0", Types.getFloat64Type(), Constants.getFloatConstant(0.0), false},
        {"-(0.0)", Types.getFloat32Type(), Constants.getFloatConstant(0.0F), false},
        {"-(-1.5)", Types.getFloat64Type(), Constants.getFloatConstant(1.5), true},
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
    const auto *Constant = Constants.getFloatConstant(-1.5F);
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
    expectFailure(analyze(NarrowOverflow, {.ExpectedType = Context.typePool().getFloat32Type()}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected f32, got an invalid or out-of-range floating-point literal");
    Diagnostics.clear();
    expectFailure(analyze(WideOverflow), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected f64, got an invalid or out-of-range floating-point literal");
    Diagnostics.clear();
    expectFailure(analyze(Fraction, {.ExpectedType = Context.typePool().getInt32Type()}), core::DiagnosticKind::SemanticTypeMismatch);
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
    EXPECT_EQ(analyze(Supplementary), Output.constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt32Type(), 0x1F600));
    EXPECT_EQ(analyze(Ascii, {.ExpectedType = Context.typePool().getUInt8Type()}), Output.constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt8Type(), 97));
    EXPECT_EQ(analyze(Nul), Output.constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt32Type(), 0));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(Supplementary, {.ExpectedType = Context.typePool().getUInt8Type()}), core::DiagnosticKind::SemanticIntegerOutOfRange);
    Diagnostics.clear();
    expectFailure(analyze(Ascii, {.ExpectedType = Context.typePool().getBoolType()}), core::DiagnosticKind::SemanticTypeMismatch);
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
      EXPECT_EQ(Result->type(), Context.typePool().getArrayType(Context.typePool().getUInt8Type(), Entry.ByteCount));
      EXPECT_EQ(Result, Output.constantPool().getStringConstant(Entry.Bytes));
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

  // String destinations must match the decoded byte array exactly; pointer decay and mismatched element counts are rejected.
  TEST_F(SemanticAnalyzerExprTest, ChecksExpectedStringArrayType)
  {
    auto Input = read("\"ab\"");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    EXPECT_TRUE(analyze(Input, {.ExpectedType = Types.getArrayType(Types.getUInt8Type(), 2)}));
    struct Case
    {
        const ir::Type *ExpectedType;
        const char *Message;
    };
    const Case Cases[] = {
        {Types.getArrayType(Types.getUInt8Type(), 3), "expected [u8; 3], got [u8; 2]"},
        {Types.getArrayType(Types.getInt32Type(), 2), "expected [i32; 2], got [u8; 2]"},
        {Types.getPointerType(Types.getUInt8Type()), "expected *u8, got [u8; 2]"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Message);
      Diagnostics.clear();
      expectFailure(analyze(Input, {.ExpectedType = Entry.ExpectedType}), core::DiagnosticKind::SemanticTypeMismatch);
      expectMessage(Entry.Message);
    }
  }

  // Unbound true and false names produce pooled bool constants while a nearer binding still shadows the fallback.
  TEST_F(SemanticAnalyzerExprTest, ResolvesBooleanFallbackAfterLexicalLookup)
  {
    auto TrueInput = read("true");
    auto FalseInput = read("false");
    ASSERT_TRUE(TrueInput.succeeded());
    ASSERT_TRUE(FalseInput.succeeded());
    EXPECT_EQ(analyze(TrueInput), Output.constantPool().getBoolConstant(true));
    EXPECT_EQ(analyze(FalseInput), Output.constantPool().getBoolConstant(false));
    AnalyzerState State(Context, TrueInput, Output);
    auto *Shadow = Output.createGlobalVariable(*Context.typePool().getUInt32Type());
    ASSERT_NE(Shadow, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("true"), *Shadow), BindResult::Inserted);
    EXPECT_EQ(analyze(State, TrueInput), Shadow);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(FalseInput, {.ExpectedType = Context.typePool().getInt32Type()}), core::DiagnosticKind::SemanticTypeMismatch);
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
    const auto *Constant = Output.constantPool().getSignedIntegerConstant(Context.typePool().getInt32Type(), 14);
    ASSERT_NE(Constant, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
    const auto *Result = analyze(State, Input, {.ExpectedType = Constant->type()});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Constant);
    EXPECT_EQ(Result->type(), Constant->type());
    EXPECT_EQ(analyze(State, Input, {.DefaultIntegerLiteralType = Context.typePool().getUInt64Type()}), Constant);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getInt64Type()}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected i64, got i32");
    Diagnostics.clear();
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getUInt64Type()}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected u64, got i32");
  }

  // A bound null pointer literal remains the pool's marker value while initializing a concrete pointer type.
  TEST_F(SemanticAnalyzerExprTest, AcceptsCompatibleNullPointerConstantBinding)
  {
    auto Input = read("Null");
    ASSERT_TRUE(Input.succeeded());
    const auto *Null = Output.constantPool().getNullPointerConstant();
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Null"), const_cast<ir::NullPointerConstant &>(*Null)), BindResult::Inserted);
    const auto *Result = analyze(State, Input, {.ExpectedType = Context.typePool().getPointerType(Context.typePool().getUInt8Type())});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Null);
    EXPECT_EQ(Result->type(), Null->type());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getUInt64Type()}), core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Global names preserve addressable storage and its initializer without emitting a load or changing the selected block.
  TEST_F(SemanticAnalyzerExprTest, PreservesGlobalStorageAndBuilderState)
  {
    auto Input = read("Written");
    ASSERT_TRUE(Input.succeeded());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getUInt32Type());
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Function, nullptr);
    const auto *Initializer = Output.constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt32Type(), 0);
    ASSERT_TRUE(Variable->setInitializer(Initializer));
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    auto *Return = State.irBuilder().createReturn();
    ASSERT_NE(Return, nullptr);
    const auto *Result = analyze(State, Input, {.ExpectedType = Variable->type()});
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result, Variable);
    EXPECT_EQ(Result->type(), Variable->type());
    EXPECT_EQ(Variable->initializer(), Initializer);
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    ASSERT_EQ(Block->instructionCount(), 1U);
    EXPECT_EQ(Block->instruction(0), Return);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getUInt64Type()}), core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Variable->initializer(), Initializer);
    EXPECT_EQ(Block->instructionCount(), 1U);
  }

  // The nearest lexical binding selects storage independently of an outer constant and exiting restores the outer value.
  TEST_F(SemanticAnalyzerExprTest, ResolvesNearestBindingWithoutMergingScopes)
  {
    auto Input = read("Output");
    ASSERT_TRUE(Input.succeeded());
    const auto *Outer = Output.constantPool().getSignedIntegerConstant(Context.typePool().getInt32Type(), 7);
    auto *Inner = Output.createGlobalVariable(*Context.typePool().getUInt32Type());
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    AnalyzerState State(Context, Input, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Output"), const_cast<ir::IntegerConstant &>(*Outer)), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Outer);
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(name("Output"), *Inner), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Inner);
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
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getUInt32Type());
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
    auto *First = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    auto *Second = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getUInt32Type()));
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
      EXPECT_EQ(Result->type(), ir::Type::classof(Result) ? Context.typePool().getMetaType() : nullptr);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getInt32Type()}), core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // Binding a control-flow label as an expression value violates the analyzer's internal model contract.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForBasicBlockNames)
  {
    auto Input = read("Block");
    ASSERT_TRUE(Input.succeeded());
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
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
    EXPECT_EQ(Result, Context.typePool().getInt32Type());
    EXPECT_EQ(Result->type(), Context.typePool().getMetaType());
    expectFailure(analyze(Input, {.ExpectedType = Context.typePool().getInt32Type()}), core::DiagnosticKind::SemanticTypeMismatch);
    Diagnostics.clear();
    auto *Shadow = Output.createGlobalVariable(*Context.typePool().getUInt32Type());
    ASSERT_NE(Shadow, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("i32"), *Shadow), BindResult::Inserted);
    const auto *Shadowed = analyze(State, Input);
    ASSERT_NE(Shadowed, nullptr);
    EXPECT_EQ(Shadowed, Shadow);
    EXPECT_EQ(Shadowed->type(), Context.typePool().getUInt32Type());
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
        {"type", Types.getMetaType()},
        {"((type))", Types.getMetaType()},
        {"i32", Types.getInt32Type()},
        {"*bool", Types.getPointerType(Types.getBoolType())},
        {"[u8; 4]", Types.getArrayType(Types.getUInt8Type(), 4)},
        {"func(i32): bool", Types.getFunctionType(Types.getBoolType(), {ir::FunctionTypeParameter(Types.getInt32Type())})},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input, {.ExpectedType = Types.getMetaType()}), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
    auto Input = read("T");
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("T"), const_cast<ir::IntegerType &>(*Types.getInt32Type())), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input, {.ExpectedType = Types.getMetaType()}), Types.getInt32Type());
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
      expectFailure(analyze(Input, {.ExpectedType = Context.typePool().getMetaType()}), core::DiagnosticKind::SemanticTypeMismatch);
    }
    ir::Module Package(Context.irContext());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    ir::Value *Targets[] = {Variable, &Package};
    for (auto *Target : Targets)
    {
      auto Input = read("Target");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Target"), *Target), BindResult::Inserted);
      expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getMetaType()}), core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // A bound type descriptor from a foreign IR context violates the model contract before metatype matching.
  TEST_F(SemanticAnalyzerExprTest, ReportsICEForForeignTypeValuesWithMetaTypeExpectation)
  {
    SemanticContext OtherContext(Sources, DiagnosticEngine, target::TargetContext::native());
    const ir::Type *Targets[] = {OtherContext.typePool().getInt32Type(), OtherContext.typePool().getMetaType()};
    for (const auto *Target : Targets)
    {
      auto Input = read("Foreign");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Foreign"), const_cast<ir::Type &>(*Target)), BindResult::Inserted);
      EXPECT_DEATH(analyze(State, Input, {.ExpectedType = Context.typePool().getMetaType()}), "INK-S0013");
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // The metatype name follows ordinary lexical lookup and a local runtime value can shadow it.
  TEST_F(SemanticAnalyzerExprTest, AllowsLexicalShadowingOfMetaTypeName)
  {
    auto Input = read("((type))");
    ASSERT_TRUE(Input.succeeded());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("type"), *Variable), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Variable);
    expectFailure(analyze(State, Input, {.ExpectedType = Context.typePool().getMetaType()}), core::DiagnosticKind::SemanticTypeMismatch);
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
        {"type", FirstTypes.getMetaType(), SecondTypes.getMetaType()},
        {"void", FirstTypes.getVoidType(), SecondTypes.getVoidType()},
        {"bool", FirstTypes.getBoolType(), SecondTypes.getBoolType()},
        {"u8", FirstTypes.getUInt8Type(), SecondTypes.getUInt8Type()},
        {"i32", FirstTypes.getInt32Type(), SecondTypes.getInt32Type()},
        {"u32", FirstTypes.getUInt32Type(), SecondTypes.getUInt32Type()},
        {"i64", FirstTypes.getInt64Type(), SecondTypes.getInt64Type()},
        {"u64", FirstTypes.getUInt64Type(), SecondTypes.getUInt64Type()},
        {"f32", FirstTypes.getFloat32Type(), SecondTypes.getFloat32Type()},
        {"f64", FirstTypes.getFloat64Type(), SecondTypes.getFloat64Type()},
        {"true", FirstContext.irContext().constantPool().getBoolConstant(true), SecondContext.irContext().constantPool().getBoolConstant(true)},
        {"false", FirstContext.irContext().constantPool().getBoolConstant(false), SecondContext.irContext().constantPool().getBoolConstant(false)},
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
      EXPECT_EQ(Analysis.analyzeExpr(FirstState, syntax(Input)), Entry.First);
      EXPECT_EQ(Analysis.analyzeExpr(SecondState, syntax(Input)), Entry.Second);
      EXPECT_EQ(Analysis.analyzeExpr(FirstState, syntax(Input)), Entry.First);
      EXPECT_EQ(Analysis.analyzeExpr(SecondState, syntax(Input)), Entry.Second);
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
    EXPECT_EQ(analyze(Literal, {.ExpectedType = Context.typePool().getUInt8Type()}), Output.constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt8Type(), 255));
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getUInt32Type());
    ASSERT_NE(Variable, nullptr);
    AnalyzerState State(Context, Storage, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Written"), *Variable), BindResult::Inserted);
    const auto *Result = analyze(State, Storage, {.ExpectedType = Variable->type()});
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
    auto *Variable = Package.createGlobalVariable(*Context.typePool().getUInt32Type());
    const auto *Constant = Package.constantPool().getSignedIntegerConstant(Context.typePool().getInt32Type(), 14);
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
        {"((Pkg)).T", Type, Context.typePool().getMetaType()},
        {"(Pkg.Nested)", &Nested, nullptr},
        {"(Pkg.T).Count", Constant, Constant->type()},
        {"Pkg.Written", Variable, Variable->type()},
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
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType(), {ir::FunctionTypeParameter(Context.typePool().getUInt32Type())});
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
    EXPECT_EQ(Result->type(), Context.typePool().getUInt32Type());
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
    auto *Current = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType()));
    auto *Other = Output.createGlobalFunction(*Types.getFunctionType(Types.getVoidType()));
    auto *Callee = Output.createGlobalFunction(*Types.getFunctionType(Types.getUInt32Type()));
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
        EXPECT_EQ(Result->type(), Types.getUInt32Type());
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
    ir::GlobalVariable Variable(*Context.typePool().getVariadicType());
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
    auto *Shared = Sibling.createGlobalVariable(*Context.typePool().getUInt32Type());
    ASSERT_NE(Shared, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Target"), *Shared), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Shared);
    auto *ForeignStorage = Foreign.createGlobalVariable(*Foreign.typePool().getUInt32Type());
    auto *ForeignFunction = Foreign.createGlobalFunction(*Foreign.typePool().getFunctionType(Foreign.typePool().getVoidType()));
    auto *ForeignType = Foreign.createClassType(4, 4);
    const auto *ForeignConstant = Foreign.constantPool().getUnsignedIntegerConstant(Foreign.typePool().getUInt32Type(), 1);
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
        {"1", Foreign.typePool().getInt32Type()},
        {"1.5", Foreign.typePool().getFloat64Type()},
        {"'a'", Foreign.typePool().getUInt32Type()},
        {"true", Foreign.typePool().getBoolType()},
        {"\"a\"", Foreign.typePool().getArrayType(Foreign.typePool().getUInt8Type(), 1)},
        {"Missing", Foreign.typePool().getInt32Type()},
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
    const auto *ForeignDefault = Foreign.typePool().getUInt64Type();
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
    EXPECT_DEATH(analyze(Input, {.ExpectedType = Context.typePool().getInt32Type(), .DefaultIntegerLiteralType = ForeignDefault}), "INK-S0013");
  }

  // A registered target without layout reaches literal-category and value-compatibility checks instead of failing at expression entry.
  TEST_F(SemanticAnalyzerExprTest, DefersLayoutlessExpectedTypesToExpressionConsumers)
  {
    auto &Types = Context.typePool();
    const auto *ExpectedType = Types.getVariadicType();
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
    const auto *Constant = Output.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 14);
    ASSERT_NE(Constant, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
    expectFailure(analyze(State, BoundInput, {.ExpectedType = ExpectedType}), core::DiagnosticKind::SemanticTypeMismatch);
    expectMessage("expected " + ExpectedType->toString() + ", got i32");
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(BoundInput).getSourceRange());
  }

  // Local targets without layout preserve unknown-name diagnostics through parentheses and numeric negation.
  TEST_F(SemanticAnalyzerExprTest, PreservesUnknownNamesWithLayoutlessExpectedTypeHints)
  {
    const auto *ExpectedType = Context.typePool().getVariadicType();
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
      expectFailure(analyze(Input, {.ExpectedType = ExpectedType, .DefaultIntegerLiteralType = Context.typePool().getUInt8Type()}), core::DiagnosticKind::SemanticUnknownName);
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
    const auto *Constant = Output.constantPool().getSignedIntegerConstant(Context.typePool().getInt32Type(), 14);
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
    const ExprHints Hints{.ExpectedType = Context.typePool().getInt8Type()};
    AnalyzerState ValidState(Context, Valid, Output);
    ASSERT_NE(createInsertBlock(ValidState), nullptr);
    expectNegation(analyze(ValidState, Valid, Hints), Output.constantPool().getSignedIntegerConstant(Context.typePool().getInt8Type(), 127));
    EXPECT_EQ(ValidState.recursionDepth(), 0U);
    AnalyzerState OverflowState(Context, Overflow, Output);
    expectFailure(analyze(OverflowState, Overflow, Hints), core::DiagnosticKind::SemanticIntegerOutOfRange);
    EXPECT_EQ(OverflowState.recursionDepth(), 0U);
    EXPECT_DEATH(analyze(Parenthesized, Hints), "INK-S0014");
    ASSERT_TRUE(Limit.set("1"));
    EXPECT_DEATH(analyze(Valid, Hints), "INK-S0014");
  }

  // Runtime operands select ordinary address, dereference and repeat operations instead of becoming type constructors.
  TEST_F(SemanticAnalyzerExprTest, KeepsRuntimeOperandOperationsUnsupported)
  {
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getUInt32Type());
    auto *Pointer = Output.createGlobalVariable(*Context.typePool().getPointerType(Context.typePool().getUInt32Type()));
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Pointer, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Written"), *Variable), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Pointer"), *Pointer), BindResult::Inserted);
    constexpr const char *Cases[] = {
        "&Written",
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
  TEST_F(SemanticAnalyzerExprTest, KeepsPointerParameterDereferenceUnsupported)
  {
    auto Input = read("*Argument");
    ASSERT_TRUE(Input.succeeded());
    auto &Types = Context.typePool();
    const auto *Signature = Types.getFunctionType(Types.getVoidType(), {ir::FunctionTypeParameter(Types.getPointerType(Types.getUInt32Type()))});
    auto *Function = Output.createGlobalFunction(*Signature);
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_EQ(State.nameResolver().bind(name("Argument"), *Function->parameter(0)), BindResult::Inserted);
    EXPECT_DEATH(analyze(State, Input), "INK-S0012");
    EXPECT_EQ(Block->instructionCount(), 0U);
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
