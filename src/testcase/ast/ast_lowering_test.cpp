#include "ink/ast/cst_lowering.h"
#include "ink/ast/printer.h"
#include "ink/ast/verifier.h"
#include "ink/frontend/compilation_session.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ink::ast
{
  namespace
  {
    parser::ParsedFile parseSource(frontend::CompilationSession &Session, core::SourceFileId File, parser::ParserOptions Options = {})
    {
      std::optional<parser::ParsedFile> Parsed = Session.parse(File, {}, Options);
      if (!Parsed)
      {
        throw std::runtime_error("AST lowering test source must tokenize successfully");
      }
      return std::move(*Parsed);
    }

    AstFile lowerParsed(frontend::CompilationSession &Session, const parser::ParsedFile &Parsed)
    {
      return lowerCst(Parsed, Parsed.sourceFileId(), Session.astContext(), Session.stringInterner());
    }

    std::size_t countKind(const AstContext &Context, const AstFile &File, AstKind Kind)
    {
      std::size_t Result = 0;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        Result += Context.declaration(AstDeclId::fromValue(Index)).Kind == Kind ? 1U : 0U;
      }
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        Result += Context.expression(AstExprId::fromValue(Index)).Kind == Kind ? 1U : 0U;
      }
      for (std::uint32_t Index = File.statements().Begin.value(); Index < File.statements().End.value(); ++Index)
      {
        Result += Context.statement(AstStmtId::fromValue(Index)).Kind == Kind ? 1U : 0U;
      }
      for (std::uint32_t Index = File.patterns().Begin.value(); Index < File.patterns().End.value(); ++Index)
      {
        Result += Context.pattern(AstPatternId::fromValue(Index)).Kind == Kind ? 1U : 0U;
      }
      return Result;
    }

    std::size_t countErrorNodes(const AstContext &Context, const AstFile &File)
    {
      return countKind(Context, File, AstKind::ErrorDeclaration) + countKind(Context, File, AstKind::ErrorExpression) + countKind(Context, File, AstKind::ErrorStatement) + countKind(Context, File, AstKind::ErrorPattern);
    }

    std::size_t countUnsupportedFeature(const AstContext &Context, const AstFile &File, UnsupportedFeature Feature)
    {
      std::size_t Result = 0;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Context.declaration(AstDeclId::fromValue(Index)).Payload);
        Result += Payload && Payload->Feature == Feature ? 1U : 0U;
      }
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Context.expression(AstExprId::fromValue(Index)).Payload);
        Result += Payload && Payload->Feature == Feature ? 1U : 0U;
      }
      for (std::uint32_t Index = File.statements().Begin.value(); Index < File.statements().End.value(); ++Index)
      {
        const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Context.statement(AstStmtId::fromValue(Index)).Payload);
        Result += Payload && Payload->Feature == Feature ? 1U : 0U;
      }
      for (std::uint32_t Index = File.patterns().Begin.value(); Index < File.patterns().End.value(); ++Index)
      {
        const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Context.pattern(AstPatternId::fromValue(Index)).Payload);
        Result += Payload && Payload->Feature == Feature ? 1U : 0U;
      }
      return Result;
    }

    std::size_t countRecoveries(const AstContext &Context, const AstFile &File, AstRecoveryKind Kind)
    {
      std::size_t Result = 0;
      const AstTableRange Range = File.recoveries();
      for (std::size_t Index = Range.Begin; Index < Range.End; ++Index)
      {
        Result += Context.allRecoveries()[Index].Kind == Kind ? 1U : 0U;
      }
      return Result;
    }

    std::string verificationErrors(const AstVerificationResult &Result)
    {
      std::ostringstream Output;
      for (const AstVerificationError &Error : Result.Errors)
      {
        Output << '\n' << astVerificationErrorKindName(Error.Kind) << ": " << Error.Message;
      }
      return Output.str();
    }

    // Verifies modeled syntax lowers to canonical typed payloads, omits trivia, and gives the SourceFile root the complete byte range.
    TEST(AstLoweringTest, LowersCanonicalPayloadsWithoutTrivia)
    {
      const std::string Source = "func add(Left: i32, Right: i32) -> i32 { return Left + Right; } // trailing trivia\n";
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("canonical.ink", Source);
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_TRUE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstContext &Context = Session.astContext();
      const AstVerificationResult Verification = verifyAst(Context, File, Session.stringInterner());

      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_EQ(File.sourceFile(), SourceFile);
      EXPECT_EQ(File.sourceSize(), Source.size());
      const Declaration &Root = Context.declaration(File.root());
      EXPECT_EQ(Root.Kind, AstKind::SourceFile);
      EXPECT_EQ(Root.Header.Range, (core::SourceRange{0, Source.size()}));
      EXPECT_TRUE(Root.Header.Supplemental.empty());
      const SourceFilePayload *RootPayload = std::get_if<SourceFilePayload>(&Root.Payload);
      ASSERT_NE(RootPayload, nullptr);
      const AstNodeListView RootItems = Context.list(RootPayload->Items);
      ASSERT_EQ(RootItems.size(), 1U);
      ASSERT_EQ(RootItems[0].Category, AstNodeCategory::Declaration);

      const Declaration &Function = Context.declaration(AstDeclId::fromValue(RootItems[0].Index));
      ASSERT_EQ(Function.Kind, AstKind::FunctionDeclaration);
      const FunctionPayload *FunctionData = std::get_if<FunctionPayload>(&Function.Payload);
      ASSERT_NE(FunctionData, nullptr);
      EXPECT_EQ(Session.stringInterner().string(FunctionData->Name), "add");
      const AstNodeListView Parameters = Context.list(FunctionData->Parameters);
      ASSERT_EQ(Parameters.size(), 2U);
      EXPECT_EQ(Context.declaration(AstDeclId::fromValue(Parameters[0].Index)).Kind, AstKind::ParameterDeclaration);
      EXPECT_EQ(Context.declaration(AstDeclId::fromValue(Parameters[1].Index)).Kind, AstKind::ParameterDeclaration);
      ASSERT_TRUE(FunctionData->ResultType.has_value());
      EXPECT_EQ(Context.expression(*FunctionData->ResultType).Kind, AstKind::BuiltinTypeExpression);
      ASSERT_TRUE(FunctionData->Body.has_value());

      const Statement &Body = Context.statement(*FunctionData->Body);
      const BlockPayload *Block = std::get_if<BlockPayload>(&Body.Payload);
      ASSERT_NE(Block, nullptr);
      const AstNodeListView BodyItems = Context.list(Block->Items);
      ASSERT_EQ(BodyItems.size(), 1U);
      const Statement &Return = Context.statement(AstStmtId::fromValue(BodyItems[0].Index));
      const ReturnPayload *ReturnData = std::get_if<ReturnPayload>(&Return.Payload);
      ASSERT_NE(ReturnData, nullptr);
      ASSERT_TRUE(ReturnData->Value.has_value());
      const Expression &Binary = Context.expression(*ReturnData->Value);
      const BinaryPayload *BinaryData = std::get_if<BinaryPayload>(&Binary.Payload);
      ASSERT_NE(BinaryData, nullptr);
      EXPECT_EQ(Session.stringInterner().string(BinaryData->Operator), "+");
      EXPECT_EQ(Context.expression(BinaryData->Left).Kind, AstKind::NameExpression);
      EXPECT_EQ(Context.expression(BinaryData->Right).Kind, AstKind::NameExpression);

      EXPECT_EQ(countKind(Context, File, AstKind::FunctionDeclaration), 1U);
      EXPECT_EQ(countKind(Context, File, AstKind::ParameterDeclaration), 2U);
      EXPECT_EQ(countKind(Context, File, AstKind::BinaryExpression), 1U);
      for (std::uint32_t Index = 0; Index < Session.stringInterner().size(); ++Index)
      {
        const std::string_view Value = Session.stringInterner().string(core::InternedStringId::fromValue(Index));
        EXPECT_EQ(Value.find("trivia"), std::string_view::npos);
        EXPECT_EQ(Value.find('\n'), std::string_view::npos);
      }
    }

    // Verifies assignment lowering interns exactly the compound operator and does not absorb the statement terminator.
    TEST(AstLoweringTest, PreservesCompoundAssignmentOperator)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("assignment.ink", "func mutate() { Left += Right; }");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_TRUE(Parsed.succeeded());
      const AstFile File = lowerParsed(Session, Parsed);
      ASSERT_TRUE(verifyAst(Session.astContext(), File, Session.stringInterner()).succeeded());

      const AssignmentPayload *Payload = nullptr;
      for (std::uint32_t Index = File.statements().Begin.value(); Index < File.statements().End.value(); ++Index)
      {
        const Statement &Node = Session.astContext().statement(AstStmtId::fromValue(Index));
        if (Node.Kind == AstKind::AssignmentStatement)
        {
          Payload = std::get_if<AssignmentPayload>(&Node.Payload);
          break;
        }
      }

      ASSERT_NE(Payload, nullptr);
      EXPECT_EQ(Session.stringInterner().string(Payload->Operator), "+=");
      EXPECT_EQ(Session.astContext().expression(Payload->Left).Kind, AstKind::NameExpression);
      EXPECT_EQ(Session.astContext().expression(Payload->Right).Kind, AstKind::NameExpression);
    }

    // Verifies binding lowering ignores attribute identifiers and equals signs when selecting the binding name, type, and initializer.
    TEST(AstLoweringTest, IgnoresAttributeSyntaxWhenBuildingBindingPayload)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("attribute-binding.ink", "[reflect(level = 1)] const Value: i32 = 2;");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_FALSE(Parsed.succeeded());
      ASSERT_FALSE(Parsed.diagnostics().empty());
      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());
      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);

      const BindingPayload *Payload = nullptr;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        const Declaration &Node = Session.astContext().declaration(AstDeclId::fromValue(Index));
        if (Node.Kind == AstKind::BindingDeclaration)
        {
          Payload = std::get_if<BindingPayload>(&Node.Payload);
          break;
        }
      }

      ASSERT_NE(Payload, nullptr);
      const Pattern &PatternNode = Session.astContext().pattern(Payload->Pattern);
      const BindingPatternPayload *PatternData = std::get_if<BindingPatternPayload>(&PatternNode.Payload);
      ASSERT_NE(PatternData, nullptr);
      EXPECT_EQ(Session.stringInterner().string(PatternData->Name), "Value");
      ASSERT_TRUE(Payload->Type.has_value());
      EXPECT_EQ(Session.astContext().expression(*Payload->Type).Kind, AstKind::BuiltinTypeExpression);
      ASSERT_TRUE(Payload->Initializer.has_value());
      const LiteralPayload *Initializer = std::get_if<LiteralPayload>(&Session.astContext().expression(*Payload->Initializer).Payload);
      ASSERT_NE(Initializer, nullptr);
      EXPECT_EQ(Session.stringInterner().string(Initializer->Spelling), "2");
    }

    // Verifies Error CST nodes and MissingToken elements become distinct error placeholders and structured recovery entries.
    TEST(AstLoweringTest, LowersErrorAndMissingRecovery)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("broken.ink", "func Broken(Value: i32 { ; return; }");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_FALSE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());

      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_GT(countErrorNodes(Session.astContext(), File), 0U);
      EXPECT_GT(countRecoveries(Session.astContext(), File, AstRecoveryKind::MissingToken), 0U);
      EXPECT_GT(countRecoveries(Session.astContext(), File, AstRecoveryKind::UnexpectedSyntax), 0U);
    }

    // Verifies a missing terminator without an Error CST sibling is represented only by MissingToken recovery metadata.
    TEST(AstLoweringTest, LowersMissingOnlyRecovery)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("missing-only.ink", "const Value = 1");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_FALSE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());

      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_EQ(countErrorNodes(Session.astContext(), File), 0U);
      EXPECT_GT(countRecoveries(Session.astContext(), File, AstRecoveryKind::MissingToken), 0U);
      EXPECT_EQ(countRecoveries(Session.astContext(), File, AstRecoveryKind::UnexpectedSyntax), 0U);
      bool FoundStructuredMissing = false;
      const AstTableRange RecoveryRange = File.recoveries();
      for (std::size_t Index = RecoveryRange.Begin; Index < RecoveryRange.End; ++Index)
      {
        const AstRecovery &Recovery = Session.astContext().allRecoveries()[Index];
        if (Recovery.Kind != AstRecoveryKind::MissingToken)
        {
          continue;
        }
        FoundStructuredMissing = true;
        EXPECT_TRUE(Recovery.Origin.isValid());
        EXPECT_TRUE(Recovery.Origin.hasElement());
        EXPECT_NE(Recovery.ExpectedKind, AstExpectedKind::Unknown);
        EXPECT_TRUE(Session.stringInterner().contains(Recovery.Spelling));
        EXPECT_FALSE(Session.stringInterner().string(Recovery.Spelling).empty());
      }
      EXPECT_TRUE(FoundStructuredMissing);
    }

    // Verifies repeated lowering in one arena has an identical deterministic dump and matches the compact AST golden.
    TEST(AstLoweringTest, RepeatedLoweringMatchesGolden)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("golden.ink", "const Value = 1;");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      const AstFile First = lowerParsed(Session, Parsed);
      const AstFile Second = lowerParsed(Session, Parsed);
      const std::string FirstDump = printAst(Session.astContext(), First, Session.stringInterner());
      const std::string SecondDump = printAst(Session.astContext(), Second, Session.stringInterner());
      const std::string Expected = R"AST(ast-file source=0 size=16 root=decl#1
root: decl#1 SourceFile range=[0, 16) origin=cst#0 recoveries=0 supplemental=0 items=1
  items[0]: decl#0 BindingDeclaration range=[0, 16) origin=cst#1 recoveries=0 supplemental=0 mode=Const top-level=true
    pattern: pattern#0 BindingPattern range=[6, 11) origin=cst#1 recoveries=0 supplemental=0 name="Value"
    initializer: expr#0 LiteralExpression range=[14, 15) origin=cst#3 recoveries=0 supplemental=0 literal=Integer spelling="1"
)AST";

      EXPECT_EQ(FirstDump, SecondDump);
      EXPECT_EQ(FirstDump, Expected);
    }

    // Verifies syntactically valid but unmodeled class syntax becomes an Unsupported payload rather than an Error node.
    TEST(AstLoweringTest, LowersUnmodeledSyntaxAsUnsupported)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("unsupported.ink", "class Box { var Value: i32; }");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_TRUE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());

      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_EQ(countErrorNodes(Session.astContext(), File), 0U);
      EXPECT_GT(countKind(Session.astContext(), File, AstKind::UnsupportedDeclaration), 0U);
      bool FoundClass = false;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        const Declaration &Node = Session.astContext().declaration(AstDeclId::fromValue(Index));
        if (Node.Kind == AstKind::UnsupportedDeclaration)
        {
          const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload);
          ASSERT_NE(Payload, nullptr);
          FoundClass |= Payload->Feature == UnsupportedFeature::Class;
        }
      }
      EXPECT_TRUE(FoundClass);
    }

    // Verifies every unmodeled semantic distinction remains explicit instead of collapsing into a supported declaration, call, tuple, import, or type.
    TEST(AstLoweringTest, PreservesUnmodeledSemanticsAsUnsupported)
    {
      const std::string Source = "import core.io; from application.model import User, Session as CurrentSession; public const Exported = 1; [nothrow] @trace async extern \"C\" func build(Value: const i32) const: Base(name = 1) { target(name = 1); target(...); const Pair = (Value,); } const Callback: type = async func(i32) -> bool;";
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("semantic-markers.ink", Source);
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_TRUE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());

      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_EQ(countErrorNodes(Session.astContext(), File), 0U);
      EXPECT_EQ(countKind(Session.astContext(), File, AstKind::ImportDeclaration), 1U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::Import), 1U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::Attribute), 1U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::Decorator), 1U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::DeclarationModifier), 4U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::ConstructorInitializer), 1U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::CallArgument), 3U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::ComplexType), 2U);
      EXPECT_EQ(countUnsupportedFeature(Session.astContext(), File, UnsupportedFeature::Tuple), 1U);
    }

    // Verifies a parenthesized nominal value-shaped CST in a type slot becomes an explicit unsupported type instead of an invalid TypeGroupExpression.
    TEST(AstLoweringTest, MarksParenthesizedNominalTypeAsUnsupported)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("parenthesized-type.ink", "func f(Value: (User));");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_TRUE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstContext &Context = Session.astContext();
      const AstVerificationResult Verification = verifyAst(Context, File, Session.stringInterner());
      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_EQ(countKind(Context, File, AstKind::TypeGroupExpression), 0U);
      EXPECT_EQ(countUnsupportedFeature(Context, File, UnsupportedFeature::ComplexType), 1U);

      const ParameterPayload *Parameter = nullptr;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        Parameter = std::get_if<ParameterPayload>(&Context.declaration(AstDeclId::fromValue(Index)).Payload);
        if (Parameter)
        {
          break;
        }
      }
      ASSERT_NE(Parameter, nullptr);
      const Expression &Type = Context.expression(Parameter->Type);
      ASSERT_EQ(Type.Kind, AstKind::UnsupportedExpression);
      const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Type.Payload);
      ASSERT_NE(Payload, nullptr);
      EXPECT_EQ(Payload->Feature, UnsupportedFeature::ComplexType);
    }

    // Verifies expression-form comptime is modeled explicitly so staging can force its typed operand without interpreting an Unsupported marker.
    TEST(AstLoweringTest, ModelsComptimeExpressionForStaging)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("comptime-expression.ink", "const Answer: i32 = comptime add(20, 22);");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_TRUE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstContext &Context = Session.astContext();
      const AstVerificationResult Verification = verifyAst(Context, File, Session.stringInterner());
      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      ASSERT_EQ(countKind(Context, File, AstKind::ComptimeExpression), 1U);

      const UnaryPayload *Comptime = nullptr;
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        const Expression &Node = Context.expression(AstExprId::fromValue(Index));
        if (Node.Kind == AstKind::ComptimeExpression)
        {
          Comptime = std::get_if<UnaryPayload>(&Node.Payload);
          break;
        }
      }
      ASSERT_NE(Comptime, nullptr);
      EXPECT_EQ(Session.stringInterner().string(Comptime->Operator), "comptime");
      EXPECT_EQ(Context.expression(Comptime->Operand).Kind, AstKind::CallExpression);
    }

    // Verifies missing required names, parameter/result types, and if-expression else values become typed Error placeholders rather than valid empty or optional payload state.
    TEST(AstLoweringTest, FillsRequiredMalformedSlotsWithErrorNodes)
    {
      const std::string Source = "import ; import core as; func (); func typed(MissingType: ) -> ; func named(: i32); const MissingInitializer = ; const MissingImplicit: i32; const Broken = if (Ready) Value;";
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("required-errors.ink", Source);
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_FALSE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstContext &Context = Session.astContext();
      const AstVerificationResult Verification = verifyAst(Context, File, Session.stringInterner());
      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_GE(countKind(Context, File, AstKind::ErrorDeclaration), 4U);
      EXPECT_GE(countKind(Context, File, AstKind::ErrorExpression), 5U);

      const FunctionPayload *Typed = nullptr;
      const FunctionPayload *Named = nullptr;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        const Declaration &Node = Context.declaration(AstDeclId::fromValue(Index));
        const FunctionPayload *Payload = std::get_if<FunctionPayload>(&Node.Payload);
        if (!Payload)
        {
          continue;
        }
        const std::string_view Name = Session.stringInterner().string(Payload->Name);
        Typed = Name == "typed" ? Payload : Typed;
        Named = Name == "named" ? Payload : Named;
      }
      ASSERT_NE(Typed, nullptr);
      const AstNodeListView TypedParameters = Context.list(Typed->Parameters);
      ASSERT_EQ(TypedParameters.size(), 1U);
      const ParameterPayload *MissingType = std::get_if<ParameterPayload>(&Context.declaration(AstDeclId::fromValue(TypedParameters[0].Index)).Payload);
      ASSERT_NE(MissingType, nullptr);
      EXPECT_EQ(Context.expression(MissingType->Type).Kind, AstKind::ErrorExpression);
      ASSERT_TRUE(Typed->ResultType.has_value());
      EXPECT_EQ(Context.expression(*Typed->ResultType).Kind, AstKind::ErrorExpression);

      ASSERT_NE(Named, nullptr);
      const AstNodeListView NamedParameters = Context.list(Named->Parameters);
      ASSERT_EQ(NamedParameters.size(), 1U);
      EXPECT_EQ(Context.declaration(AstDeclId::fromValue(NamedParameters[0].Index)).Kind, AstKind::ErrorDeclaration);

      bool FoundIf = false;
      bool FoundMissingInitializer = false;
      bool FoundMissingImplicit = false;
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        const BindingPayload *Payload = std::get_if<BindingPayload>(&Context.declaration(AstDeclId::fromValue(Index)).Payload);
        if (!Payload || !Payload->Initializer)
        {
          continue;
        }
        const BindingPatternPayload *Pattern = std::get_if<BindingPatternPayload>(&Context.pattern(Payload->Pattern).Payload);
        if (Pattern && Session.stringInterner().string(Pattern->Name) == "MissingInitializer")
        {
          FoundMissingInitializer = true;
          EXPECT_EQ(Context.expression(*Payload->Initializer).Kind, AstKind::ErrorExpression);
        }
        if (Pattern && Session.stringInterner().string(Pattern->Name) == "MissingImplicit")
        {
          FoundMissingImplicit = true;
          EXPECT_EQ(Context.expression(*Payload->Initializer).Kind, AstKind::ErrorExpression);
        }
      }
      EXPECT_TRUE(FoundMissingInitializer);
      EXPECT_TRUE(FoundMissingImplicit);

      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        const Expression &Node = Context.expression(AstExprId::fromValue(Index));
        const IfExpressionPayload *Payload = std::get_if<IfExpressionPayload>(&Node.Payload);
        if (Payload)
        {
          FoundIf = true;
          EXPECT_EQ(Context.expression(Payload->ElseValue).Kind, AstKind::ErrorExpression);
        }
      }
      EXPECT_TRUE(FoundIf);
    }

    // Verifies a parenthesized expression with a missing value retains a Group node whose required child is an ErrorExpression.
    TEST(AstLoweringTest, FillsMissingGroupValueWithErrorExpression)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("missing-group.ink", "const MissingGroup = (;");
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_FALSE(Parsed.succeeded());
      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());
      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);

      const GroupPayload *Group = nullptr;
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        Group = std::get_if<GroupPayload>(&Session.astContext().expression(AstExprId::fromValue(Index)).Payload);
        if (Group)
        {
          break;
        }
      }
      ASSERT_NE(Group, nullptr);
      EXPECT_EQ(Session.astContext().expression(Group->Value).Kind, AstKind::ErrorExpression);
    }

    // Verifies synthesized required-expression errors use the MissingToken byte anchor and CST element origin instead of the containing statement range.
    TEST(AstLoweringTest, UsesMissingTokenSiteForSynthesizedErrors)
    {
      const std::string Source = "func f(){ A = ; }";
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("precise-error.ink", Source);
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
      ASSERT_FALSE(Parsed.succeeded());
      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());
      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);

      const AssignmentPayload *Assignment = nullptr;
      for (std::uint32_t Index = File.statements().Begin.value(); Index < File.statements().End.value(); ++Index)
      {
        Assignment = std::get_if<AssignmentPayload>(&Session.astContext().statement(AstStmtId::fromValue(Index)).Payload);
        if (Assignment)
        {
          break;
        }
      }
      ASSERT_NE(Assignment, nullptr);
      const Expression &Right = Session.astContext().expression(Assignment->Right);
      const std::size_t MissingOffset = Source.find(';');
      EXPECT_EQ(Right.Kind, AstKind::ErrorExpression);
      EXPECT_EQ(Right.Header.Range, (core::SourceRange{MissingOffset, MissingOffset}));
      ASSERT_TRUE(Right.Header.Origin.has_value());
      EXPECT_TRUE(Right.Header.Origin->hasElement());
    }

    // Verifies every major declaration, statement, expression, type, pattern, and contextual-comptime CST family lowers safely.
    TEST(AstLoweringTest, LowersAllSyntaxFamiliesSafely)
    {
      const std::vector<std::string> Sources = {
          "import core.io; import ..platform.window as window; from application.model import User, Session as CurrentSession;",
          "[nothrow, reflect(level = 1)] @trace(kind = \"io\") public extern \"C\" static async func load<T: type, N: ptrsize = 4, Rest: type...>(path: const Data&, count: i32 = 1, values: Data...) const -> Result; class Box<T: type> : Base { public var Value: T; func get() -> T { return Value; } } interface Printable { func print() -> void; } enum Result { Success(i32), Failure = 2 }",
          "func statements() { var Local: i32 = 0; const Fixed = 1; Local = Fixed; Local + Fixed; if (Ready) { Local += 1; } else if (Fallback) {} else {} if (match .some(Value) = Candidate) {} match (Candidate) { .some(Value) => { Value; } _ => return; } while (Ready) { break; continue; } while (match .some(Value) = Candidate) {} for (var Item in Values) { Item; } for (const Index in Begin .. End) {} for (const _ in Values) {} return Local; defer cleanup(); defer { cleanup(); } throw; throw Failure; throw Failure from Cause; try {} catch Error as Failure {} catch as Remaining {} }",
          "func expressions() { const Precedence = A + B * C << D & E ^ F | G && H || I; const Conditional = if (Ready) First else Second; const Matched = match (Value) { .some(Item) => Item, _ => Fallback, }; const Postfix = Object.method::<T>(Value, ...Arguments, name = Named)[Index][Low:High].field->next; const Aggregate = Record { First, Second: Value }; const Tuple = (1, 2, ...Items); const Array = [1, 2, 3]; const Unary = comptime await - + ! ~ * & Value; const Literals = (true, false, null, 1, 1.0, 'a', \"text\"); const Builtins = (i32, this); const FunctionType = func(i32, ...Types) -> bool; const ConstantType = const Data*; const Anonymous = class { var Value: i32; }; }",
          "func types(Value: const Map::<String, Data*>[Count][]&&, Handler: async func((i32, String), ...Types) -> const Result&) -> Result**; const Callback: type = (func(i32) -> bool)*; func consume(Value: (Map::<String, Vector::<i32>>));",
          "comptime { const Generated = 1; func helper(); } comptime if (Enabled) { const Active = 1; } else { const Disabled = 0; } comptime match (Choice) { .some => { const Selected = 1; } _ => { const Default = 0; } } comptime for (const Item in Items) { const Repeated = Item; } comptime while (Enabled) { const Waiting = 1; } func local() { comptime if (Enabled) { return; } else {} } class Members { comptime if (Enabled) { var Value: i32; } else { func fallback(); } }",
      };

      frontend::CompilationSession Session;
      for (std::size_t Index = 0; Index < Sources.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        const core::SourceFileId SourceFile = Session.addSource("syntax-family-" + std::to_string(Index) + ".ink", Sources[Index]);
        const parser::ParsedFile Parsed = parseSource(Session, SourceFile);
        ASSERT_TRUE(Parsed.succeeded());
        const AstFile File = lowerParsed(Session, Parsed);
        const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());
        EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      }
    }

    // Verifies iterative lowering and verification handle 1500 nested unary expressions without host-stack recursion.
    TEST(AstLoweringTest, HandlesFifteenHundredUnaryExpressions)
    {
      constexpr std::size_t Depth = 1500;
      std::string Source = "const Deep = ";
      Source.append(Depth, '!');
      Source += "true;";
      frontend::CompilationSession Session;
      const core::SourceFileId SourceFile = Session.addSource("deep.ink", Source);
      parser::ParserOptions Options;
      Options.MaxSyntaxNestingDepth = Depth + 16;
      const parser::ParsedFile Parsed = parseSource(Session, SourceFile, Options);
      ASSERT_TRUE(Parsed.succeeded());

      const AstFile File = lowerParsed(Session, Parsed);
      const AstVerificationResult Verification = verifyAst(Session.astContext(), File, Session.stringInterner());

      EXPECT_TRUE(Verification.succeeded()) << verificationErrors(Verification);
      EXPECT_EQ(countKind(Session.astContext(), File, AstKind::UnaryExpression), Depth);
    }

    // Verifies files lowered into one context own adjacent non-overlapping arena, list, and recovery table ranges.
    TEST(AstLoweringTest, KeepsMultipleFileRangesDisjoint)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId FirstSource = Session.addSource("first.ink", "func first() { const First = 1; return First }");
      const core::SourceFileId SecondSource = Session.addSource("second.ink", "func second() { const Second = 2; return Second }");
      const parser::ParsedFile FirstParsed = parseSource(Session, FirstSource);
      const AstFile First = lowerParsed(Session, FirstParsed);
      const parser::ParsedFile SecondParsed = parseSource(Session, SecondSource);
      const AstFile Second = lowerParsed(Session, SecondParsed);

      EXPECT_EQ(First.declarations().End, Second.declarations().Begin);
      EXPECT_EQ(First.expressions().End, Second.expressions().Begin);
      EXPECT_EQ(First.statements().End, Second.statements().Begin);
      EXPECT_EQ(First.patterns().End, Second.patterns().Begin);
      EXPECT_EQ(First.listElements().End, Second.listElements().Begin);
      EXPECT_EQ(First.recoveries().End, Second.recoveries().Begin);
      EXPECT_GT(Second.recoveries().Begin, 0U);
      EXPECT_EQ(First.sourceFile(), FirstSource);
      EXPECT_EQ(Second.sourceFile(), SecondSource);
      EXPECT_TRUE(verifyAst(Session.astContext(), First, Session.stringInterner()).succeeded());
      EXPECT_TRUE(verifyAst(Session.astContext(), Second, Session.stringInterner()).succeeded());
    }

    // Verifies explicit lowering rejects both an invalid file ID and a file ID different from ParsedFile ownership.
    TEST(AstLoweringTest, RejectsMismatchedSourceFileIdentity)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId First = Session.addSource("first.ink", "const First = 1;");
      const core::SourceFileId Second = Session.addSource("second.ink", "const Second = 2;");
      const parser::ParsedFile Parsed = parseSource(Session, First);

      EXPECT_THROW(lowerCst(Parsed, {}, Session.astContext(), Session.stringInterner()), std::invalid_argument);
      EXPECT_THROW(lowerCst(Parsed, Second, Session.astContext(), Session.stringInterner()), std::invalid_argument);
    }
  } // namespace
} // namespace ink::ast
