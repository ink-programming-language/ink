#include "ink/semantic/analyzer.h"

#include "ink/parser/parser.h"
#include "ink/semantic/model/context.h"

#include <gtest/gtest.h>

namespace ink::semantic::test
{
  class SemanticAnalyzerTest : public testing::Test
  {
    protected:
      SemanticAnalyzerTest()
          : Frontend(Compilation),
            Context(Compilation)
      {
        Compilation.diagnosticEngine().addConsumer(Diagnostics);
      }

      Module *read(std::string Source)
      {
        Diagnostics.clear();
        auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, std::move(Source)));
        ParsedSuccessfully = Parsed.succeeded();
        if (Parsed.Unit)
        {
          SourceId = Parsed.Unit->input().lexedFile().sourceId();
        }
        // Deliberately destroy the parsed unit before tests inspect the result.
        return analyze(Context, Parsed, "test");
      }

      const Function *function(const Module &ModuleValue, std::string_view Name)
      {
        for (const Value *Child : ModuleValue.entryBlock().values())
        {
          if (Function::classof(Child))
          {
            const auto *FunctionValue = static_cast<const Function *>(Child);
            if (Context.namePool().text(FunctionValue->name()) == Name)
            {
              return FunctionValue;
            }
          }
        }
        return nullptr;
      }

      template <typename Node>
      const Node *instruction(const Function &FunctionValue, std::size_t Index)
      {
        const BasicBlock *Block = FunctionValue.entryBlock();
        if (!Block || Index >= Block->values().size() || !Node::classof(Block->values()[Index]))
        {
          return nullptr;
        }
        return static_cast<const Node *>(Block->values()[Index]);
      }

      core::CompilationContext Compilation;
      core::FrontendContext Frontend;
      SemanticContext Context;
      core::CollectingDiagnosticConsumer Diagnostics;
      core::SourceId SourceId;
      bool ParsedSuccessfully = false;
  };

  // The requested source becomes a self-contained graph with direct load/add/return edges.
  TEST_F(SemanticAnalyzerTest, BuildsRequestedProgramWithoutBorrowingAST)
  {
    const Module *Model = read(R"(func main(): int32 { var a: int32 = 2; var b: int32 = 4; print("hello， world"); return a+b; })");
    ASSERT_NE(Model, nullptr);
    EXPECT_TRUE(ParsedSuccessfully);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_EQ(Context.namePool().text(Model->name()), "test");
    ASSERT_EQ(Model->entryBlock().values().size(), 2U);
    const Function *Main = function(*Model, "main");
    const Function *Print = function(*Model, "print");
    ASSERT_NE(Main, nullptr);
    ASSERT_NE(Print, nullptr);
    EXPECT_FALSE(Print->hasBody());
    ASSERT_EQ(Print->parameters().size(), 1U);
    EXPECT_EQ(Context.namePool().text(Print->parameters()[0]->name()), "text");
    EXPECT_EQ(Print->type().returnType().typeKind(), TypeKind::Void);
    ASSERT_EQ(Main->blocks().size(), 1U);
    ASSERT_EQ(Main->entryBlock()->values().size(), 9U);
    const auto *A = instruction<AllocaInstruction>(*Main, 0);
    const auto *InitializeA = instruction<StoreInstruction>(*Main, 1);
    const auto *B = instruction<AllocaInstruction>(*Main, 2);
    const auto *InitializeB = instruction<StoreInstruction>(*Main, 3);
    const auto *Call = instruction<CallInstruction>(*Main, 4);
    const auto *ReadA = instruction<LoadInstruction>(*Main, 5);
    const auto *ReadB = instruction<LoadInstruction>(*Main, 6);
    const auto *Sum = instruction<AddInstruction>(*Main, 7);
    const auto *Return = instruction<ReturnInstruction>(*Main, 8);
    ASSERT_NE(A, nullptr);
    ASSERT_NE(InitializeA, nullptr);
    ASSERT_NE(B, nullptr);
    ASSERT_NE(InitializeB, nullptr);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(ReadA, nullptr);
    ASSERT_NE(ReadB, nullptr);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(&InitializeA->address(), A);
    EXPECT_EQ(&InitializeB->address(), B);
    ASSERT_TRUE(IntegerConstant::classof(&InitializeA->storedValue()));
    ASSERT_TRUE(IntegerConstant::classof(&InitializeB->storedValue()));
    EXPECT_EQ(static_cast<const IntegerConstant &>(InitializeA->storedValue()).value().words()[0], 2U);
    EXPECT_EQ(static_cast<const IntegerConstant &>(InitializeB->storedValue()).value().words()[0], 4U);
    EXPECT_EQ(Call->directCallee(), Print);
    ASSERT_EQ(Call->arguments().size(), 1U);
    ASSERT_TRUE(StringConstant::classof(Call->arguments()[0]));
    EXPECT_EQ(static_cast<const StringConstant *>(Call->arguments()[0])->value(), "hello， world");
    EXPECT_EQ(&ReadA->address(), A);
    EXPECT_EQ(&ReadB->address(), B);
    EXPECT_EQ(&Sum->left(), ReadA);
    EXPECT_EQ(&Sum->right(), ReadB);
    EXPECT_EQ(Return->returnedValue(), Sum);
    EXPECT_EQ(Return->function(), Main);
    EXPECT_EQ(Return->type().typeKind(), TypeKind::Void);
    EXPECT_EQ(Main->outer(), &Model->entryBlock());
    EXPECT_EQ(Main->entryBlock()->outer(), Main);
    for (const Value *Operation : Main->entryBlock()->values())
    {
      EXPECT_EQ(Operation->outer(), Main->entryBlock());
      EXPECT_FALSE(ExprValue::classof(Operation));
    }
  }

  // Signatures are collected before bodies, and parameters are values owned by the callee.
  TEST_F(SemanticAnalyzerTest, ResolvesForwardCallsAndFunctionParameters)
  {
    const Module *Model = read("func main(): int32 { return sum(2, 4); } func sum(a: int32, b: int32): int32 { return a+b; }");
    ASSERT_NE(Model, nullptr);
    const Function *Main = function(*Model, "main");
    const Function *Sum = function(*Model, "sum");
    ASSERT_NE(Main, nullptr);
    ASSERT_NE(Sum, nullptr);
    const auto *Call = instruction<CallInstruction>(*Main, 0);
    const auto *Add = instruction<AddInstruction>(*Sum, 0);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Add, nullptr);
    EXPECT_EQ(Call->directCallee(), Sum);
    ASSERT_EQ(Sum->parameters().size(), 2U);
    EXPECT_EQ(&Add->left(), Sum->parameters()[0]);
    EXPECT_EQ(&Add->right(), Sum->parameters()[1]);
    EXPECT_EQ(Sum->parameters()[1]->index(), 1U);
    EXPECT_EQ(&Sum->parameters()[1]->function(), Sum);
    EXPECT_EQ(Sum->parameters()[1]->outer(), Sum);
    EXPECT_EQ(Sum->parameters()[0]->parameterKind(), ParameterKind::Positional);
    EXPECT_EQ(Sum->parameters()[1]->parameterKind(), ParameterKind::Positional);
    EXPECT_EQ(Context.namePool().text(Sum->parameters()[0]->name()), "a");
    EXPECT_EQ(Context.namePool().text(Sum->parameters()[1]->name()), "b");
  }

  // Shadowed names resolve to separate slots, with the outer binding restored after the block.
  TEST_F(SemanticAnalyzerTest, PreservesLexicalScopesAndAssignmentOrder)
  {
    const Module *Model = read("func main(): int32 { var a = 2; { var a = 4; a = a+1; } return a; }");
    ASSERT_NE(Model, nullptr);
    const Function *Main = function(*Model, "main");
    ASSERT_NE(Main, nullptr);
    const auto *Outer = instruction<AllocaInstruction>(*Main, 0);
    const auto *Inner = instruction<AllocaInstruction>(*Main, 2);
    const auto *ReadInner = instruction<LoadInstruction>(*Main, 4);
    const auto *Assign = instruction<StoreInstruction>(*Main, 6);
    const auto *ReadOuter = instruction<LoadInstruction>(*Main, 7);
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    ASSERT_NE(ReadInner, nullptr);
    ASSERT_NE(Assign, nullptr);
    ASSERT_NE(ReadOuter, nullptr);
    EXPECT_NE(Outer, Inner);
    EXPECT_EQ(&ReadInner->address(), Inner);
    EXPECT_EQ(&Assign->address(), Inner);
    EXPECT_EQ(&ReadOuter->address(), Outer);
  }

  // A later assignment initializes a slot, including assignments from nested straight-line scopes.
  TEST_F(SemanticAnalyzerTest, TracksInitializationBeforeLoads)
  {
    const Module *Model = read("func main(): int32 { var a: int32; { a = 2; } return a; }");
    ASSERT_NE(Model, nullptr);
    const Function *Main = function(*Model, "main");
    ASSERT_NE(Main, nullptr);
    ASSERT_EQ(Main->entryBlock()->values().size(), 4U);
    EXPECT_NE(instruction<StoreInstruction>(*Main, 1), nullptr);
    EXPECT_NE(instruction<LoadInstruction>(*Main, 2), nullptr);
  }

  // An initializer is resolved before the new binding becomes visible, so it can read an outer name.
  TEST_F(SemanticAnalyzerTest, ResolvesShadowingInitializerInOuterScope)
  {
    const Module *Model = read("func main(): int32 { var a = 2; { var a = a+4; return a; } }");
    ASSERT_NE(Model, nullptr);
    const Function *Main = function(*Model, "main");
    ASSERT_NE(Main, nullptr);
    const auto *Outer = instruction<AllocaInstruction>(*Main, 0);
    const auto *Read = instruction<LoadInstruction>(*Main, 2);
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Read, nullptr);
    EXPECT_EQ(&Read->address(), Outer);
  }

  // Both explicit and implicit void returns become terminators; decoded strings retain embedded NULs.
  TEST_F(SemanticAnalyzerTest, BuildsVoidFunctionsAndDecodedStringValues)
  {
    const Module *Model = read(R"(func main(): void { var text: string = "a\0b"; print(text); } func done(): void { return; })");
    ASSERT_NE(Model, nullptr);
    const Function *Main = function(*Model, "main");
    const Function *Done = function(*Model, "done");
    ASSERT_NE(Main, nullptr);
    ASSERT_NE(Done, nullptr);
    const auto *Store = instruction<StoreInstruction>(*Main, 1);
    const auto *Return = instruction<ReturnInstruction>(*Main, 4);
    ASSERT_NE(Store, nullptr);
    ASSERT_NE(Return, nullptr);
    ASSERT_TRUE(StringConstant::classof(&Store->storedValue()));
    EXPECT_EQ(static_cast<const StringConstant &>(Store->storedValue()).value(), std::string_view("a\0b", 3));
    EXPECT_EQ(Return->returnedValue(), nullptr);
    EXPECT_NE(instruction<ReturnInstruction>(*Done, 0), nullptr);
  }

  // Contextual literal conversion accepts signed minima, unsigned maxima and base-prefixed spellings.
  TEST_F(SemanticAnalyzerTest, ConvertsIntegerBoundariesWithoutTruncation)
  {
    struct Case
    {
        const char *Type;
        const char *Literal;
        std::uint64_t Bits;
    };
    const Case Cases[] = {
        {"int8", "-128", 128},
        {"int8", "127", 127},
        {"uint8", "0xff", 255},
        {"int16", "0o177", 127},
        {"int32", "0b101", 5},
        {"int64", "-9223372036854775808", 9223372036854775808ULL},
        {"uint64", "18446744073709551615", 18446744073709551615ULL},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Literal);
      const Module *Model = read(std::string("func main(): ") + Entry.Type + " { return " + Entry.Literal + "; }");
      ASSERT_NE(Model, nullptr);
      const Function *Main = function(*Model, "main");
      ASSERT_NE(Main, nullptr);
      const auto *Return = instruction<ReturnInstruction>(*Main, 0);
      ASSERT_NE(Return, nullptr);
      ASSERT_TRUE(IntegerConstant::classof(Return->returnedValue()));
      EXPECT_EQ(static_cast<const IntegerConstant *>(Return->returnedValue())->value().words()[0], Entry.Bits);
    }
  }

  // User errors publish one source-associated diagnostic and never expose a partial module.
  TEST_F(SemanticAnalyzerTest, ReportsNameTypeCallAndReturnErrors)
  {
    using core::DiagnosticKind;
    struct Case
    {
        const char *Source;
        DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"func main(): int32 { return missing; }", DiagnosticKind::SemanticUnknownName},
        {"func main(): void { x = 1; }", DiagnosticKind::SemanticUnknownName},
        {"func main(): typo { return 1; }", DiagnosticKind::SemanticUnknownType},
        {"func main(): void { var a = 1; var a = 2; }", DiagnosticKind::SemanticDuplicateName},
        {"func main(): void {} func main(): void {}", DiagnosticKind::SemanticDuplicateName},
        {"func print(): void {}", DiagnosticKind::SemanticDuplicateName},
        {"func f(x: int32, x: int32): void {}", DiagnosticKind::SemanticDuplicateName},
        {"func main(): void { var a: int32; print(a); }", DiagnosticKind::SemanticUninitializedRead},
        {"func main(): void { var a = a; }", DiagnosticKind::SemanticUnknownName},
        {"func main(): void { var a; }", DiagnosticKind::SemanticMissingVariableType},
        {"func main(): void { var a: void; }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): void { var a = print(\"x\"); }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): void { print(2); }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): void { print(); }", DiagnosticKind::SemanticArgumentCount},
        {"func main(): void { var a = 2; a(); }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): void { var a = 2; a = true; }", DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: int32): void { x = 2; }", DiagnosticKind::SemanticInvalidAssignment},
        {"func main(): void { return 2+4; }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): void { return print(\"x\"); }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): int32 { return; }", DiagnosticKind::SemanticTypeMismatch},
        {"func main(): int32 { var a = 2; }", DiagnosticKind::SemanticMissingReturn},
        {"func main(): void { return; print(\"x\"); }", DiagnosticKind::SemanticUnreachableStatement},
        {"func main(): int8 { return 128; }", DiagnosticKind::SemanticIntegerOutOfRange},
        {"func main(): int8 { return -129; }", DiagnosticKind::SemanticIntegerOutOfRange},
        {"func main(): uint8 { return -1; }", DiagnosticKind::SemanticIntegerOutOfRange},
        {"func main(): uint64 { return 18446744073709551616; }", DiagnosticKind::SemanticIntegerOutOfRange},
        {"func main(): int32 { var a: uint32 = 2; return a; }", DiagnosticKind::SemanticTypeMismatch},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      EXPECT_EQ(read(Entry.Source), nullptr);
      ASSERT_TRUE(ParsedSuccessfully);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      const auto &Diagnostic = Diagnostics.diagnostics()[0];
      EXPECT_EQ(Diagnostic.Kind, Entry.Kind);
      EXPECT_EQ(Diagnostic.Source, SourceId);
      EXPECT_TRUE(Diagnostic.Span.isValid());
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_FALSE(core::DiagnosticFormatter{}.format(Diagnostic).Message.empty());
    }
  }

  // Unsupported valid syntax reports a compiler limitation instead of being silently ignored.
  TEST_F(SemanticAnalyzerTest, RejectsUnimplementedLanguageFeatures)
  {
    const char *Sources[] = {
        "var global = 2;",
        "func main(): void { if (true) { return; } }",
        "func main(): int32 { return 2*4; }",
        "func main(): void { const a = 2; }",
        "func main(): void { var a = 1.5; }",
        "func main(): void { var a = 2; a += 4; }",
        "func f[T: type](x: T): T { return x; }",
        "func f(x: int32 = 2): void {}",
        "func main(): void { print(text = \"x\"); }",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      EXPECT_EQ(read(Source), nullptr);
      ASSERT_TRUE(ParsedSuccessfully);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnsupported);
      EXPECT_EQ(Diagnostics.diagnostics()[0].classification(), core::DiagnosticClass::InternalCompilerError);
    }
  }

  // Deep left-associated expressions stop at an explicit semantic limit rather than overflowing the stack.
  TEST_F(SemanticAnalyzerTest, BoundsExpressionRecursion)
  {
    std::string Source = "func main(): int32 { return 1";
    for (std::size_t Index = 0; Index < 600; ++Index)
    {
      Source += "+1";
    }
    Source += "; }";
    EXPECT_EQ(read(std::move(Source)), nullptr);
    ASSERT_TRUE(ParsedSuccessfully);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticNestingLimit);
  }

  // Lexer/parser failures are preserved and cannot be mistaken for semantic success.
  TEST_F(SemanticAnalyzerTest, DoesNotAnalyzeFailedParsing)
  {
    EXPECT_EQ(read("func main(): void { var a = ; }"), nullptr);
    EXPECT_FALSE(ParsedSuccessfully);
    ASSERT_FALSE(Diagnostics.diagnostics().empty());
    for (const auto &Diagnostic : Diagnostics.diagnostics())
    {
      EXPECT_NE(core::diagnosticDomain(Diagnostic.Kind), core::DiagnosticDomain::Semantic);
    }
  }

  // Each analysis owns its namespace, and a failed call cannot pollute a later module in the same context.
  TEST_F(SemanticAnalyzerTest, SeparatesRepeatedAnalysisSessions)
  {
    EXPECT_EQ(read("func main(): int32 { return missing; }"), nullptr);
    const Module *First = read("func main(): bool { return true; }");
    ASSERT_NE(First, nullptr);
    const Module *Second = read("func main(): bool { return false; }");
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_NE(function(*First, "main"), function(*Second, "main"));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }
} // namespace ink::semantic::test
