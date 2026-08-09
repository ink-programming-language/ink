#include "ink/ast/printer.h"
#include "ink/ast/verifier.h"

#include <gtest/gtest.h>

#include <iomanip>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ink::ast
{
  namespace
  {
    struct ManualFile
    {
      AstFile File;
      AstDeclId Binding;
      AstExprId Literal;
      AstPatternId Pattern;
    };

    class GroupingNumpunct : public std::numpunct<char>
    {
    protected:
      char do_thousands_sep() const override
      {
        return '_';
      }

      std::string do_grouping() const override
      {
        return "\1";
      }
    };

    AstNodeHeader header(core::SourceRange Range, std::optional<CstOrigin> Origin = std::nullopt, AstRecoveryRange Recoveries = {}, AstNodeList Supplemental = {})
    {
      return {Range, Origin, Recoveries, Supplemental};
    }

    Pattern bindingPattern(core::InternedStringId Name, core::SourceRange Range = {0, 1})
    {
      return {header(Range), AstKind::BindingPattern, BindingPatternPayload{Name}};
    }

    Expression integerLiteral(core::InternedStringId Spelling, core::SourceRange Range = {0, 1})
    {
      return {header(Range), AstKind::LiteralExpression, LiteralPayload{AstLiteralKind::Integer, Spelling}};
    }

    Declaration bindingDeclaration(AstPatternId PatternId, AstExprId Initializer, core::SourceRange Range = {0, 1}, std::optional<CstOrigin> Origin = std::nullopt)
    {
      return {header(Range, Origin), AstKind::BindingDeclaration, BindingPayload{AstBindingMode::Const, true, PatternId, std::nullopt, Initializer}};
    }

    Declaration sourceFileDeclaration(AstNodeList Items, core::SourceRange Range = {0, 1}, std::optional<CstOrigin> Origin = std::nullopt, AstRecoveryRange Recoveries = {}, AstNodeList Supplemental = {})
    {
      return {header(Range, Origin, Recoveries, Supplemental), AstKind::SourceFile, SourceFilePayload{Items}};
    }

    ManualFile addValidBindingFile(AstContext &Context, core::StringInterner &Strings, core::SourceFileId SourceFile)
    {
      const AstPatternId PatternId = Context.addPattern(bindingPattern(Strings.intern("Value")));
      const AstExprId LiteralId = Context.addExpression(integerLiteral(Strings.intern("1")));
      const AstDeclId BindingId = Context.addDeclaration(bindingDeclaration(PatternId, LiteralId));
      const AstNodeList Items = Context.addList({AstNodeRef::declaration(BindingId)});
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Items));
      return {Context.createFile(SourceFile, Root, 1, Strings, {}), BindingId, LiteralId, PatternId};
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

    void expectRejected(const AstVerificationResult &Result)
    {
      EXPECT_FALSE(Result.succeeded()) << "malformed AST unexpectedly passed verification";
      EXPECT_FALSE(Result.Errors.empty());
    }

    // Verifies a compact AST built without CST origins is valid when its payloads, typed IDs, ranges, and ownership are consistent.
    TEST(AstVerifierTest, AcceptsConsistentOriginFreeAst)
    {
      AstContext Context;
      core::StringInterner Strings;
      const ManualFile Built = addValidBindingFile(Context, Strings, core::SourceFileId::fromValue(0));

      const AstVerificationResult Result = verifyAst(Context, Built.File, Strings);

      EXPECT_TRUE(Result.succeeded()) << verificationErrors(Result);
      EXPECT_FALSE(Context.declaration(Built.File.root()).Header.Origin.has_value());
      EXPECT_FALSE(Context.declaration(Built.Binding).Header.Origin.has_value());
      EXPECT_FALSE(Context.expression(Built.Literal).Header.Origin.has_value());
      EXPECT_FALSE(Context.pattern(Built.Pattern).Header.Origin.has_value());
    }

    // Verifies a node kind paired with the wrong tagged payload alternative is rejected.
    TEST(AstVerifierTest, RejectsPayloadMismatch)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstDeclId Root = Context.addDeclaration({header({0, 0}), AstKind::SourceFile, ErrorPayload{}});
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies a kind stored in the wrong typed arena is rejected independently of host variant safety.
    TEST(AstVerifierTest, RejectsKindCategoryMismatch)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstDeclId Root = Context.addDeclaration({header({0, 0}), AstKind::LiteralExpression, ErrorPayload{}});
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies invalid typed child IDs and invalid interned-string IDs cannot escape payload validation.
    TEST(AstVerifierTest, RejectsInvalidTypedAndStringIds)
    {
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1")));
        const AstDeclId Binding = Context.addDeclaration(bindingDeclaration(AstPatternId::fromValue(99), Literal));
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Binding)})));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 1, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstPatternId Pattern = Context.addPattern(bindingPattern(Strings.intern("Value")));
        const AstExprId Literal = Context.addExpression(integerLiteral({}));
        const AstDeclId Binding = Context.addDeclaration(bindingDeclaration(Pattern, Literal));
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Binding)})));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 1, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
    }

    // Verifies nodes and payload children whose byte ranges leave their source or parent range are rejected.
    TEST(AstVerifierTest, RejectsInvalidAndNonNestedSourceRanges)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstPatternId Pattern = Context.addPattern(bindingPattern(Strings.intern("Value")));
      const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1"), {1, 2}));
      const AstDeclId Binding = Context.addDeclaration(bindingDeclaration(Pattern, Literal));
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Binding)})));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 1, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies payload and supplemental list slices must stay inside the file-owned list table.
    TEST(AstVerifierTest, RejectsInvalidListRanges)
    {
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration({5, 1}, {0, 0}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstNodeList Items = Context.addList({});
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Items, {0, 0}, std::nullopt, {}, {9, 1}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
    }

    // Verifies recovery slices and recovery origins must be file-owned and point to real CST child positions.
    TEST(AstVerifierTest, RejectsInvalidRecoveryRangesAndOrigins)
    {
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({}), {0, 0}, std::nullopt, {4, 1}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
      {
        AstContext Context;
        core::StringInterner Strings;
        const core::InternedStringId Spelling = Strings.intern(";");
        const AstRecoveryRange Recoveries = Context.addRecoveries({{AstRecoveryKind::MissingToken, {0, 0}, CstOrigin::element(0, 1), AstExpectedKind::Symbol, Spelling}});
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({}), {0, 0}, std::nullopt, Recoveries));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {1});
        expectRejected(verifyAst(Context, File, Strings));
      }
    }

    // Verifies duplicate child ownership and duplicate recovery-element origins are both rejected deterministically.
    TEST(AstVerifierTest, RejectsDuplicateNodesAndRecoveryOrigins)
    {
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstPatternId Pattern = Context.addPattern(bindingPattern(Strings.intern("Value")));
        const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1")));
        const AstDeclId Binding = Context.addDeclaration(bindingDeclaration(Pattern, Literal));
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Binding), AstNodeRef::declaration(Binding)})));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 1, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
      {
        AstContext Context;
        core::StringInterner Strings;
        const core::InternedStringId Spelling = Strings.intern(";");
        const AstRecovery Duplicate{AstRecoveryKind::MissingToken, {0, 0}, CstOrigin::element(0, 0), AstExpectedKind::Symbol, Spelling};
        const AstRecoveryRange Recoveries = Context.addRecoveries({Duplicate, Duplicate});
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({}), {0, 0}, std::nullopt, Recoveries));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {1});
        expectRejected(verifyAst(Context, File, Strings));
      }
    }

    // Verifies arena nodes not reachable from the SourceFile root are rejected as orphans.
    TEST(AstVerifierTest, RejectsOrphanedNodes)
    {
      AstContext Context;
      core::StringInterner Strings;
      Context.addExpression(integerLiteral(Strings.intern("1")));
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({}), {0, 0}));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies a SourceFile item list cannot point directly to a LiteralExpression even when the reference is otherwise valid.
    TEST(AstVerifierTest, RejectsSourceFileToLiteralEdge)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1")));
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::expression(Literal)})));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 1, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies a file cannot reference a valid node ID owned by an earlier file in the same context.
    TEST(AstVerifierTest, RejectsForeignFileNodeReferences)
    {
      AstContext Context;
      core::StringInterner Strings;
      const ManualFile First = addValidBindingFile(Context, Strings, core::SourceFileId::fromValue(0));
      ASSERT_TRUE(verifyAst(Context, First.File, Strings).succeeded());
      const AstDeclId SecondRoot = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(First.Binding)})));
      const AstFile Second = Context.createFile(core::SourceFileId::fromValue(1), SecondRoot, 1, Strings, {});

      expectRejected(verifyAst(Context, Second, Strings));
    }

    // Verifies required declaration identifiers cannot use an interned empty string as a valid semantic name.
    TEST(AstVerifierTest, RejectsEmptyRequiredIdentifier)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstDeclId Function = Context.addDeclaration({header({0, 0}), AstKind::FunctionDeclaration, FunctionPayload{AstFunctionFlavor::Function, Strings.intern(""), Context.addList({}), std::nullopt, std::nullopt}});
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Function)}), {0, 0}));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies SourceFile roots and parameter declarations cannot be nested in the SourceFile item list.
    TEST(AstVerifierTest, RejectsInvalidTopLevelDeclarationRoles)
    {
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstDeclId Nested = Context.addDeclaration(sourceFileDeclaration(Context.addList({}), {0, 0}));
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Nested)}), {0, 0}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstExprId Type = Context.addExpression({header({0, 0}), AstKind::BuiltinTypeExpression, NamePayload{Strings.intern("i32")}});
        const AstDeclId Parameter = Context.addDeclaration({header({0, 0}), AstKind::ParameterDeclaration, ParameterPayload{AstParameterFlavor::Function, Strings.intern("Value"), Type, std::nullopt, false}});
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Parameter)}), {0, 0}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
    }

    // Verifies a binding's top-level flag must agree with placement in the SourceFile item list.
    TEST(AstVerifierTest, RejectsBindingTopLevelFlagMismatch)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstPatternId Pattern = Context.addPattern(bindingPattern(Strings.intern("Value"), {0, 0}));
      const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1"), {0, 0}));
      const AstDeclId Binding = Context.addDeclaration({header({0, 0}), AstKind::BindingDeclaration, BindingPayload{AstBindingMode::Const, false, Pattern, std::nullopt, Literal}});
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Binding)}), {0, 0}));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies declaration type slots reject ordinary value expressions even when the typed expression ID and ownership are otherwise valid.
    TEST(AstVerifierTest, RejectsValueExpressionInTypeSlot)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1"), {0, 0}));
      const AstDeclId Parameter = Context.addDeclaration({header({0, 0}), AstKind::ParameterDeclaration, ParameterPayload{AstParameterFlavor::Function, Strings.intern("Value"), Literal, std::nullopt, false}});
      const AstDeclId Function = Context.addDeclaration({header({0, 0}), AstKind::FunctionDeclaration, FunctionPayload{AstFunctionFlavor::Function, Strings.intern("f"), Context.addList({AstNodeRef::declaration(Parameter)}), std::nullopt, std::nullopt}});
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Function)}), {0, 0}));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies type-group nesting and UnsupportedExpression tags cannot disguise value-only expressions as declaration types.
    TEST(AstVerifierTest, RejectsNestedValueAndNonTypeUnsupportedExpressionsInTypeSlots)
    {
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1"), {0, 0}));
        const AstExprId TypeGroup = Context.addExpression({header({0, 0}), AstKind::TypeGroupExpression, GroupPayload{Literal}});
        const AstDeclId Parameter = Context.addDeclaration({header({0, 0}), AstKind::ParameterDeclaration, ParameterPayload{AstParameterFlavor::Function, Strings.intern("Value"), TypeGroup, std::nullopt, false}});
        const AstDeclId Function = Context.addDeclaration({header({0, 0}), AstKind::FunctionDeclaration, FunctionPayload{AstFunctionFlavor::Function, Strings.intern("f"), Context.addList({AstNodeRef::declaration(Parameter)}), std::nullopt, std::nullopt}});
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Function)}), {0, 0}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
      {
        AstContext Context;
        core::StringInterner Strings;
        const AstExprId Unsupported = Context.addExpression({header({0, 0}), AstKind::UnsupportedExpression, UnsupportedPayload{UnsupportedFeature::CallArgument, Context.addList({})}});
        const AstDeclId Parameter = Context.addDeclaration({header({0, 0}), AstKind::ParameterDeclaration, ParameterPayload{AstParameterFlavor::Function, Strings.intern("Value"), Unsupported, std::nullopt, false}});
        const AstDeclId Function = Context.addDeclaration({header({0, 0}), AstKind::FunctionDeclaration, FunctionPayload{AstFunctionFlavor::Function, Strings.intern("f"), Context.addList({AstNodeRef::declaration(Parameter)}), std::nullopt, std::nullopt}});
        const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({AstNodeRef::declaration(Function)}), {0, 0}));
        const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});
        expectRejected(verifyAst(Context, File, Strings));
      }
    }

    // Verifies Supplemental cannot hide a modeled declaration from the SourceFile item list while still making its arena nodes reachable.
    TEST(AstVerifierTest, RejectsModeledNodeInSupplemental)
    {
      AstContext Context;
      core::StringInterner Strings;
      const AstPatternId Pattern = Context.addPattern(bindingPattern(Strings.intern("Hidden"), {0, 0}));
      const AstExprId Literal = Context.addExpression(integerLiteral(Strings.intern("1"), {0, 0}));
      const AstDeclId Binding = Context.addDeclaration({header({0, 0}), AstKind::BindingDeclaration, BindingPayload{AstBindingMode::Const, false, Pattern, std::nullopt, Literal}});
      const AstDeclId Root = Context.addDeclaration(sourceFileDeclaration(Context.addList({}), {0, 0}, std::nullopt, {}, Context.addList({AstNodeRef::declaration(Binding)})));
      const AstFile File = Context.createFile(core::SourceFileId::fromValue(0), Root, 0, Strings, {});

      expectRejected(verifyAst(Context, File, Strings));
    }

    // Verifies deterministic printing refuses a string interner different from the one that owns the AST file.
    TEST(AstVerifierTest, PrinterRejectsForeignStringInterner)
    {
      AstContext Context;
      core::StringInterner Strings;
      const ManualFile Built = addValidBindingFile(Context, Strings, core::SourceFileId::fromValue(0));
      core::StringInterner ForeignStrings;
      ForeignStrings.intern("WrongValue");
      ForeignStrings.intern("WrongLiteral");

      EXPECT_EQ(printAst(Context, Built.File, ForeignStrings), "invalid-ast-string-owner\n");
    }

    // Verifies AST numeric output remains decimal and locale-independent even when the caller configures a grouped hexadecimal stream.
    TEST(AstVerifierTest, PrinterUsesDeterministicNumericFormatting)
    {
      AstContext Context;
      core::StringInterner Strings;
      const ManualFile Built = addValidBindingFile(Context, Strings, core::SourceFileId::fromValue(15));
      const std::string Expected = printAst(Context, Built.File, Strings);
      std::ostringstream Altered;
      Altered.imbue(std::locale(std::locale::classic(), new GroupingNumpunct));
      Altered << std::hex << std::showbase << std::uppercase;

      printAst(Context, Built.File, Strings, Altered);

      EXPECT_EQ(Altered.str(), Expected);
      EXPECT_EQ(Altered.flags() & std::ios::basefield, std::ios::hex);
    }
  } // namespace
} // namespace ink::ast
