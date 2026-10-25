#include "ink/semantic/analyzer/analyzer.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    enum class MalformedFunction
    {
      MissingDefinitionBody,
      UnexpectedDeclarationBody,
      MissingReturnType,
      MissingReturnTypeExpression,
      MissingNativeLinkage,
      NonLiteralNativeLinkage,
      NonStringNativeLinkage,
      OutOfBoundsNativeLinkageToken,
      InputEndNativeLinkageToken,
      MismatchedNativeLinkageToken,
      UnexpectedLocalLinkage,
      MissingParameterType,
      MissingParameterName,
      MissingFunctionName,
      InvalidBodyKind,
      InvalidNativeKind,
      InvalidVisibility,
    };

    class SemanticFunctionDeclTest : public testing::Test
    {
      protected:
        SemanticFunctionDeclTest()
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
          Scopes.removeTarget(Module);
        }

        void TearDown() override
        {
          unregister(Output);
        }

        const Binding *lookup(std::string_view Text)
        {
          return NameResolver(Context).lookupMember(Output, name(Text));
        }

        ir::Function *function(std::string_view Text)
        {
          const Binding *Found = lookup(Text);
          return Found && Found->targets().size() == 1 && ir::Function::classof(Found->targets()[0]) ? static_cast<ir::Function *>(Found->targets()[0]) : nullptr;
        }

        static core::Name name(std::string_view Text)
        {
          return core::NamePool::instance().intern(Text);
        }

        static void expectExport(const ir::Module &Module, std::string_view Text, std::initializer_list<const ir::Value *> Declarations)
        {
          const auto *Export = Module.findExport(ir::SymbolId{name(Text)});
          ASSERT_NE(Export, nullptr);
          EXPECT_EQ(Export->Symbol.SymbolName, name(Text));
          ASSERT_EQ(Export->Entries.size(), Declarations.size());
          std::size_t Index = 0;
          for (const auto *Declaration : Declarations)
          {
            EXPECT_EQ(Export->Entries[Index].Declaration, Declaration);
            EXPECT_EQ(Export->Entries[Index].SymbolType, Declaration->type());
            ++Index;
          }
        }

        static const ir::ReturnInstruction *returnInstruction(const ir::Function &Function)
        {
          const ir::BasicBlock *Block = Function.entryBlock();
          const ir::Instruction *Last = Block && Block->instructionCount() ? Block->instruction(Block->instructionCount() - 1) : nullptr;
          return ir::ReturnInstruction::classof(Last) ? static_cast<const ir::ReturnInstruction *>(Last) : nullptr;
        }

        void invalidateFunction(frontend::parser::ParseResult &Input, MalformedFunction Failure)
        {
          using namespace frontend::parser;
          auto *Statement = cast<DeclStmt>(Input.Unit->root()->statements()[0]);
          auto *Declaration = cast<FunctionDecl>(Statement->declaration());
          auto &AST = Input.Unit->context();
          const auto Range = Declaration->getSourceRange();
          const auto Attributes = Declaration->attributes();
          auto FunctionName = Declaration->name();
          const auto Generics = Declaration->genericParameters();
          auto Parameters = Declaration->parameters();
          auto *ReturnType = Declaration->returnType();
          auto BodyKind = Declaration->bodyKind();
          auto *Body = Declaration->body();
          auto *Linkage = Declaration->linkage();
          auto NativeKind = Declaration->nativeSymbolKind();
          auto Visibility = Declaration->visibility();
          const bool Comptime = Declaration->isComptime();
          switch (Failure)
          {
            case MalformedFunction::MissingDefinitionBody:
              Body = nullptr;
              break;
            case MalformedFunction::UnexpectedDeclarationBody:
              BodyKind = FunctionBodyKind::DeclarationOnly;
              break;
            case MalformedFunction::MissingReturnType:
              ReturnType = nullptr;
              break;
            case MalformedFunction::MissingReturnTypeExpression:
              ReturnType = AST.make<TypeSyntax>(ReturnType->getSourceRange(), nullptr);
              break;
            case MalformedFunction::MissingNativeLinkage:
              Linkage = nullptr;
              break;
            case MalformedFunction::NonLiteralNativeLinkage:
              Linkage = AST.make<NameExpr>(FunctionName.Range, FunctionName);
              break;
            case MalformedFunction::NonStringNativeLinkage:
            {
              auto *LiteralStatement = cast<SimpleStmt>(Input.Unit->root()->statements()[1]);
              Linkage = cast<ExprItem>(LiteralStatement->items()[0])->expression();
              break;
            }
            case MalformedFunction::MismatchedNativeLinkageToken:
              Linkage = AST.make<LiteralExpr>(FunctionName.Range, FunctionName.Id, TokenKind::StringLiteral);
              break;
            case MalformedFunction::OutOfBoundsNativeLinkageToken:
              Linkage = AST.make<LiteralExpr>(FunctionName.Range, InvalidTokenId, TokenKind::StringLiteral);
              break;
            case MalformedFunction::InputEndNativeLinkageToken:
              Linkage = AST.make<LiteralExpr>(FunctionName.Range, Input.Unit->input().size(), TokenKind::StringLiteral);
              break;
            case MalformedFunction::UnexpectedLocalLinkage:
              NativeKind = NativeSymbolKind::None;
              break;
            case MalformedFunction::MissingParameterType:
            case MalformedFunction::MissingParameterName:
            {
              auto Original = Parameters[0];
              auto ParameterName = Original.name();
              auto *ParameterType = Original.type();
              if (Failure == MalformedFunction::MissingParameterType)
              {
                ParameterType = nullptr;
              }
              else
              {
                ParameterName.Text = {};
              }
              Parameters = AST.copyArray<Parameter>(std::vector<Parameter>{Parameter(ParameterName, ParameterType, nullptr, false, Original.range())});
              break;
            }
            case MalformedFunction::MissingFunctionName:
              FunctionName.Text = {};
              break;
            case MalformedFunction::InvalidBodyKind:
              BodyKind = static_cast<FunctionBodyKind>(255);
              break;
            case MalformedFunction::InvalidNativeKind:
              NativeKind = static_cast<NativeSymbolKind>(255);
              break;
            case MalformedFunction::InvalidVisibility:
              Visibility = static_cast<DeclarationVisibility>(255);
              break;
          }
          auto *Replacement = AST.make<FunctionDecl>(Range, Attributes, FunctionName, Generics, Parameters, ReturnType, BodyKind, Body, Linkage, NativeKind);
          Replacement->setVisibility(Visibility);
          Replacement->setComptime(Comptime);
          auto *ReplacementStatement = AST.make<DeclStmt>(Statement->getSourceRange(), Replacement);
          ReplacementStatement->setComptime(Statement->isComptime());
          auto *Root = Input.Unit->root();
          const auto RootRange = Root->getSourceRange();
          const bool RootComptime = Root->isComptime();
          const auto OriginalStatements = Root->statements();
          std::vector<Stmt *> Statements(OriginalStatements.begin(), OriginalStatements.end());
          Statements[0] = ReplacementStatement;
          const auto ReplacementStatements = AST.copyArray<Stmt *>(Statements);
          // Only the complete root is transparently replaced; every declaration base pointer names a newly created object.
          std::destroy_at(Root);
          std::construct_at(Root, RootRange, ReplacementStatements);
          Root->setComptime(RootComptime);
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

  // Empty void bodies and explicit returns terminate once, including empty nested lexical blocks.
  TEST_F(SemanticFunctionDeclTest, BuildsVoidDefinitionsWithOneReturn)
  {
    auto Input = read("func empty(): void {} func nested(): void { { {} } } func explicitReturn(): void { return; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Output.globalFunctionCount(), 3U);
    for (const auto &Function : Output.globalFunctions())
    {
      ASSERT_EQ(Function->blockCount(), 1U);
      EXPECT_EQ(Function->entryBlock()->function(), Function.get());
      ASSERT_EQ(Function->entryBlock()->instructionCount(), 1U);
      const auto *Return = returnInstruction(*Function);
      ASSERT_NE(Return, nullptr);
      EXPECT_FALSE(Return->hasReturnedValue());
      EXPECT_TRUE(Function->entryBlock()->hasTerminator());
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Default and explicit public Ink functions publish exports after their bodies succeed, while private functions remain local.
  TEST_F(SemanticFunctionDeclTest, PublishesDefaultAndExplicitPublicFunctions)
  {
    auto Input = read("func defaultExport(): void {} public func explicitExport(): void {} private func hidden(): void {}");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Default = function("defaultExport");
    auto *Explicit = function("explicitExport");
    auto *Hidden = function("hidden");
    ASSERT_NE(Default, nullptr);
    ASSERT_NE(Explicit, nullptr);
    ASSERT_NE(Hidden, nullptr);
    expectExport(Output, "defaultExport", {Default});
    expectExport(Output, "explicitExport", {Explicit});
    EXPECT_EQ(Output.findExport(ir::SymbolId{name("hidden")}), nullptr);
    EXPECT_EQ(Output.exports().size(), 2U);
    EXPECT_EQ(Default->visibility(), ir::FunctionVisibility::Public);
    EXPECT_EQ(Explicit->visibility(), ir::FunctionVisibility::Public);
    EXPECT_EQ(Hidden->visibility(), ir::FunctionVisibility::Private);
    EXPECT_EQ(Default->symbolLinkage(), ir::SymbolLinkage::Local);
    EXPECT_EQ(Explicit->symbolLinkage(), ir::SymbolLinkage::Local);
    EXPECT_EQ(Hidden->symbolLinkage(), ir::SymbolLinkage::Local);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Fixed parameters retain their names and canonical types and remain visible through nested blocks.
  TEST_F(SemanticFunctionDeclTest, BindsParametersAndReturnsThroughNestedBlocks)
  {
    auto Input = read("private func identity(Value: i32, Other: *u8): i32 { { { return Value; } } }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    ASSERT_EQ(Output.globalFunctionCount(), 1U);
    auto *Identity = function("identity");
    ASSERT_NE(Identity, nullptr);
    EXPECT_EQ(Identity->visibility(), ir::FunctionVisibility::Private);
    EXPECT_EQ(Identity->languageLinkage(), ir::LanguageLinkage::Ink);
    EXPECT_EQ(Identity->symbolLinkage(), ir::SymbolLinkage::Local);
    ASSERT_EQ(Identity->parameterCount(), 2U);
    EXPECT_EQ(Identity->parameter(0)->name(), name("Value"));
    EXPECT_EQ(Identity->parameter(0)->type(), Context.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_EQ(Identity->parameter(1)->type(), Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(8, false)));
    EXPECT_EQ(Identity->parameter(0)->function(), Identity);
    EXPECT_FALSE(Identity->parameter(0)->isVariadic());
    const auto *Return = returnInstruction(*Identity);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Identity->parameter(0));
    EXPECT_EQ(Identity->entryBlock()->instructionCount(), 1U);
    const Binding *Parameter = NameResolver(Context).lookupMember(*Identity, name("Value"));
    ASSERT_NE(Parameter, nullptr);
    EXPECT_EQ(Parameter->targets()[0], Identity->parameter(0));
    EXPECT_EQ(lookup("Value"), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // The unit type is a valid zero-size parameter type and remains represented in the fixed signature.
  TEST_F(SemanticFunctionDeclTest, AcceptsUnitParameter)
  {
    auto Input = read("func unit(Value: void): void {}");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Unit = function("unit");
    ASSERT_NE(Unit, nullptr);
    ASSERT_EQ(Unit->parameterCount(), 1U);
    EXPECT_EQ(Unit->parameter(0)->type(), Context.typePool().get<ink::ir::VoidType>());
    EXPECT_EQ(Unit->functionType()->parameters()[0].ParameterType, Context.typePool().get<ink::ir::VoidType>());
    EXPECT_NE(returnInstruction(*Unit), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Return expectations type integer literals before materialization and preserve unary negation in the function body.
  TEST_F(SemanticFunctionDeclTest, AppliesReturnTypeToLiteralsAndNegation)
  {
    auto Input = read("func byte(): u8 { return 255; } func negative(): i8 { return -127; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Byte = function("byte");
    auto *Negative = function("negative");
    ASSERT_NE(Byte, nullptr);
    ASSERT_NE(Negative, nullptr);
    const auto *ByteReturn = returnInstruction(*Byte);
    const auto *NegativeReturn = returnInstruction(*Negative);
    ASSERT_NE(ByteReturn, nullptr);
    ASSERT_NE(NegativeReturn, nullptr);
    ASSERT_TRUE(ir::IntegerConstant::classof(ByteReturn->returnedValue()));
    EXPECT_EQ(ByteReturn->returnedValue()->type(), Context.typePool().get<ink::ir::IntegerType>(8, false));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(ByteReturn->returnedValue())->bits().tryGetUInt64(), 255U);
    ASSERT_TRUE(ir::NegInstruction::classof(NegativeReturn->returnedValue()));
    EXPECT_EQ(NegativeReturn->returnedValue()->type(), Context.typePool().get<ink::ir::IntegerType>(8, true));
    EXPECT_EQ(Negative->entryBlock()->instructionCount(), 2U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A pooled null pointer remains a valid initializer for a concrete pointer return type.
  TEST_F(SemanticFunctionDeclTest, ReturnsCompatibleNullPointerBinding)
  {
    const auto *Null = Output.constantPool().get<ink::ir::NullPointerConstant>();
    ASSERT_NE(Null, nullptr);
    ASSERT_EQ(NameResolver(Context).bind(name("Null"), const_cast<ir::NullPointerConstant &>(*Null)), BindResult::Inserted);
    auto Input = read("func emptyPointer(): *u8 { return Null; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Function = function("emptyPointer");
    ASSERT_NE(Function, nullptr);
    const auto *Return = returnInstruction(*Function);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Null);
    EXPECT_EQ(Function->functionType()->returnType(), Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(8, false)));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Different parameter lists form an overload set, while parameter spellings do not affect signature interning.
  TEST_F(SemanticFunctionDeclTest, BindsDistinctOverloads)
  {
    auto Input = read("func choose(Value: i32): void {} func choose(Value: u8): void {} func renamed(Other: i32): void {}");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    const Binding *Overloads = lookup("choose");
    ASSERT_NE(Overloads, nullptr);
    EXPECT_TRUE(Overloads->isOverloadSet());
    ASSERT_EQ(Overloads->targets().size(), 2U);
    auto *Renamed = function("renamed");
    ASSERT_NE(Renamed, nullptr);
    EXPECT_EQ(Overloads->targets()[0]->type(), Renamed->type());
    EXPECT_NE(Overloads->targets()[0]->type(), Overloads->targets()[1]->type());
    expectExport(Output, "choose", {Overloads->targets()[0], Overloads->targets()[1]});
    expectExport(Output, "renamed", {Renamed});
    EXPECT_EQ(Output.exports().size(), 2U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A later overload with an invalid body leaves the previously successful overload as the group's only exported candidate.
  TEST_F(SemanticFunctionDeclTest, FailedOverloadDoesNotEnterExistingExportGroup)
  {
    auto Input = read("func choose(Value: i32): i32 { return Value; } func choose(Value: u8): u8 { return Missing; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    const auto *Overloads = lookup("choose");
    ASSERT_NE(Overloads, nullptr);
    ASSERT_EQ(Overloads->targets().size(), 2U);
    expectExport(Output, "choose", {Overloads->targets()[0]});
    EXPECT_EQ(Output.exports().size(), 1U);
  }

  // Function parameters never become bindings in a sibling function or the enclosing module.
  TEST_F(SemanticFunctionDeclTest, KeepsParameterScopesSeparate)
  {
    auto Input = read("func first(Value: i32): i32 { return Value; } func second(): void { Value; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(lookup("Value"), nullptr);
    ASSERT_NE(function("first"), nullptr);
    ASSERT_NE(function("second"), nullptr);
    EXPECT_NE(returnInstruction(*function("first")), nullptr);
  }

  // Independent output modules may define the same name, and another call resumes the selected module's bindings.
  TEST_F(SemanticFunctionDeclTest, IsolatesModulesAndResumesTheirScopes)
  {
    ir::Module Other(Context.irContext());
    auto First = read("func same(): void {}");
    auto Second = read("func same(Value: i32): void {}");
    auto More = read("func next(): void {}");
    ASSERT_TRUE(First.succeeded());
    ASSERT_TRUE(Second.succeeded());
    ASSERT_TRUE(More.succeeded());
    ASSERT_TRUE(analyze(First));
    ASSERT_TRUE(Analysis.analyze(Context, Second, Other));
    ASSERT_TRUE(analyze(More));
    ASSERT_EQ(Output.globalFunctionCount(), 2U);
    ASSERT_EQ(Other.globalFunctionCount(), 1U);
    const auto *FirstBinding = lookup("same");
    const auto *OtherBinding = NameResolver(Context).lookupMember(Other, name("same"));
    ASSERT_NE(FirstBinding, nullptr);
    ASSERT_NE(OtherBinding, nullptr);
    EXPECT_EQ(FirstBinding->targets().size(), 1U);
    EXPECT_EQ(OtherBinding->targets().size(), 1U);
    EXPECT_NE(FirstBinding->targets()[0], OtherBinding->targets()[0]);
    EXPECT_EQ(NameResolver(Context).lookupMember(Other, name("next")), nullptr);
    EXPECT_EQ(NameResolver(Context).lookup(name("same")), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    unregister(Other);
  }

  // Reanalyzing a declaration against the same output finds the existing binding and reports one conflict.
  TEST_F(SemanticFunctionDeclTest, RejectsDuplicateOnRepeatedAnalysis)
  {
    auto Input = read("func same(): void {}");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticDuplicateName);
    EXPECT_EQ(Output.globalFunctionCount(), 1U);
    expectExport(Output, "same", {Output.globalFunction(0)});
    EXPECT_EQ(Output.exports().size(), 1U);
  }

  // Invalid signatures diagnose user errors before publishing a callable in the module.
  TEST_F(SemanticFunctionDeclTest, RejectsInvalidSignatures)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
        std::string_view Highlight;
    };
    const Case Cases[] = {
        {"func f(Value: Missing): void {}", core::DiagnosticKind::SemanticUnknownName, "Missing"},
        {"func f(): Missing {}", core::DiagnosticKind::SemanticUnknownName, "Missing"},
        {"func f(Value: type): void {}", core::DiagnosticKind::SemanticTypeMismatch, "type"},
        {"func f(): type {}", core::DiagnosticKind::SemanticTypeMismatch, "type"},
        {"func f(Value: ((type))): void {}", core::DiagnosticKind::SemanticTypeMismatch, "((type))"},
        {"func f(Value: Missing): ((type)) {}", core::DiagnosticKind::SemanticTypeMismatch, "((type))"},
        {"func f(Value: i32, Value: u8): void {}", core::DiagnosticKind::SemanticDuplicateParameterName, "Value"},
        {"func f(): void;", core::DiagnosticKind::SemanticFunctionRequiresBody, "func f(): void;"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(analyze(Input));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      const auto &Diagnostic = Diagnostics.diagnostics()[0];
      EXPECT_EQ(Diagnostic.Kind, Entry.Kind);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_EQ(Diagnostic.Source, Input.Unit->input().lexedFile().sourceId());
      const auto Offset = std::string_view(Entry.Source).rfind(Entry.Highlight);
      ASSERT_NE(Offset, std::string_view::npos);
      EXPECT_EQ(Diagnostic.Span, core::SourceRange::fromByteOffsets(Offset, Offset + Entry.Highlight.size()));
      EXPECT_EQ(Output.globalFunctionCount(), 0U);
      EXPECT_EQ(lookup("f"), nullptr);
      EXPECT_TRUE(Output.exports().empty());
    }
  }

  // Duplicate definitions and overloads differing only in return type collide without changing the existing binding.
  TEST_F(SemanticFunctionDeclTest, RejectsConflictingDefinitions)
  {
    const char *Cases[] = {
        "func f(): void {} func f(): void {}",
        "func f(Value: i32): i32 { return Value; } func f(Other: i32): u8 { return 0; }",
        "import \"C\" func f(Value: i32): void; import \"C\" func f(Value: u8): void;",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticDuplicateName);
      EXPECT_EQ(Destination.globalFunctionCount(), 1U);
      unregister(Destination);
    }
  }

  // Return errors preserve their concrete reason and do not add a second missing-return diagnostic for the same body.
  TEST_F(SemanticFunctionDeclTest, DiagnosesInvalidReturnsOnce)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"func f(): i32 {}", core::DiagnosticKind::SemanticMissingReturn},
        {"func f(): i32 { return; }", core::DiagnosticKind::SemanticMissingReturn},
        {"func f(): void { return 1; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(Value: i32): u8 { return Value; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(): i32 { return true; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(): u8 { return 256; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): i32 { return Missing; }", core::DiagnosticKind::SemanticUnknownName},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      ir::Module Destination(Context.irContext());
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(Analysis.analyze(Context, Input, Destination));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Entry.Kind);
      ASSERT_EQ(Destination.globalFunctionCount(), 1U);
      EXPECT_FALSE(Destination.globalFunction(0)->entryBlock()->hasTerminator());
      EXPECT_TRUE(Destination.exports().empty());
      unregister(Destination);
    }
  }

  // A nested return terminates the enclosing body and rejects later statements without emitting further instructions.
  TEST_F(SemanticFunctionDeclTest, RejectsStatementsAfterNestedReturn)
  {
    auto Input = read("func f(): i32 { { return 1; } Missing; -2; }");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnreachableStatement);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticUnreachableStatement);
    auto *Function = function("f");
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->blockCount(), 1U);
    EXPECT_EQ(Function->entryBlock()->instructionCount(), 1U);
    EXPECT_NE(returnInstruction(*Function), nullptr);
  }

  // A failed body restores its function and scope cursor before later declarations and module statements are analyzed.
  TEST_F(SemanticFunctionDeclTest, RestoresStateAfterBodyFailure)
  {
    auto Input = read("func broken(Value: i32): i32 { return Missing; } func valid(Other: u8): u8 { return Other; } return;");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::SemanticReturnOutsideFunction);
    auto *Valid = function("valid");
    ASSERT_NE(Valid, nullptr);
    const auto *Return = returnInstruction(*Valid);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Valid->parameter(0));
    EXPECT_EQ(lookup("Value"), nullptr);
    EXPECT_EQ(lookup("Other"), nullptr);
    EXPECT_EQ(Output.findExport(ir::SymbolId{name("broken")}), nullptr);
    expectExport(Output, "valid", {Valid});
    EXPECT_EQ(Output.exports().size(), 1U);
  }

  // The single source pass does not predeclare later functions, but still analyzes them after an earlier body fails.
  TEST_F(SemanticFunctionDeclTest, DoesNotPredeclareLaterFunctions)
  {
    auto Input = read("func first(): void { later; } func later(): void {}");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(analyze(Input));
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    auto *Later = function("later");
    ASSERT_NE(Later, nullptr);
    EXPECT_NE(returnInstruction(*Later), nullptr);
  }

  // Native C declarations decode their linkage and preserve direction, source visibility and body ownership.
  TEST_F(SemanticFunctionDeclTest, BuildsNativeImportAndExport)
  {
    auto Input = read("private import \"\\x43\" func external(Message: *u8): i32; public export \"C\" func exposed(Value: i32): i32 { return Value; }");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    auto *Imported = function("external");
    auto *Exported = function("exposed");
    ASSERT_NE(Imported, nullptr);
    ASSERT_NE(Exported, nullptr);
    EXPECT_EQ(Imported->symbolLinkage(), ir::SymbolLinkage::Import);
    EXPECT_EQ(Imported->languageLinkage(), ir::LanguageLinkage::C);
    EXPECT_EQ(Imported->visibility(), ir::FunctionVisibility::Private);
    EXPECT_TRUE(Imported->isDeclaration());
    EXPECT_EQ(Imported->parameterCount(), 1U);
    EXPECT_EQ(Exported->symbolLinkage(), ir::SymbolLinkage::Export);
    EXPECT_EQ(Exported->languageLinkage(), ir::LanguageLinkage::C);
    EXPECT_EQ(Exported->visibility(), ir::FunctionVisibility::Public);
    EXPECT_FALSE(Exported->isDeclaration());
    EXPECT_NE(returnInstruction(*Exported), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Native imports never enter module exports, while native exports still respect Ink visibility.
  TEST_F(SemanticFunctionDeclTest, ExcludesNativeImportsFromModuleExports)
  {
    auto Input = read("import \"C\" func defaultImport(): void; public import \"C\" func publicImport(): void; private import \"C\" func privateImport(): void; export \"C\" func defaultExport(): void {} public export \"C\" func publicExport(): void {} private export \"C\" func privateExport(): void {}");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(analyze(Input));
    struct Case
    {
        const char *Name;
        ir::SymbolLinkage Linkage;
        ir::FunctionVisibility Visibility;
        bool Exported;
    };
    const Case Cases[] = {
        {"defaultImport", ir::SymbolLinkage::Import, ir::FunctionVisibility::Public, false},
        {"publicImport", ir::SymbolLinkage::Import, ir::FunctionVisibility::Public, false},
        {"privateImport", ir::SymbolLinkage::Import, ir::FunctionVisibility::Private, false},
        {"defaultExport", ir::SymbolLinkage::Export, ir::FunctionVisibility::Public, true},
        {"publicExport", ir::SymbolLinkage::Export, ir::FunctionVisibility::Public, true},
        {"privateExport", ir::SymbolLinkage::Export, ir::FunctionVisibility::Private, false},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      const auto *Function = function(Entry.Name);
      ASSERT_NE(Function, nullptr);
      EXPECT_EQ(Function->symbolLinkage(), Entry.Linkage);
      EXPECT_EQ(Function->visibility(), Entry.Visibility);
      EXPECT_EQ(Function->languageLinkage(), ir::LanguageLinkage::C);
      EXPECT_EQ(Function->isDeclaration(), Entry.Linkage == ir::SymbolLinkage::Import);
      if (Entry.Exported)
      {
        expectExport(Output, Entry.Name, {Function});
      }
      else
      {
        EXPECT_EQ(Output.findExport(ir::SymbolId{name(Entry.Name)}), nullptr);
      }
    }
    EXPECT_EQ(Output.exports().size(), 2U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Native imports reject bodies, while native exports use the shared missing-body diagnostic.
  TEST_F(SemanticFunctionDeclTest, RejectsInvalidNativeBodies)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"import \"C\" func f(): void {}", core::DiagnosticKind::SemanticNativeImportHasBody},
        {"export \"C\" func f(): void;", core::DiagnosticKind::SemanticFunctionRequiresBody},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_FALSE(analyze(Input));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Entry.Kind);
      EXPECT_EQ(Output.globalFunctionCount(), 0U);
    }
  }

  // Malformed function fields report precise AST invariants, while invalid linkage indices use the shared token-reference ICE.
  TEST_F(SemanticFunctionDeclTest, ReportsMalformedFunctionASTWithConcreteReason)
  {
    struct Case
    {
        const char *Source;
        MalformedFunction Failure;
        const char *Description;
    };
    const Case Cases[] = {
        {"func f(Value: i32): void {}", MalformedFunction::MissingDefinitionBody, "FunctionDecl definition requires a body"},
        {"func f(Value: i32): void {}", MalformedFunction::UnexpectedDeclarationBody, "FunctionDecl declaration-only form must not have a body"},
        {"func f(Value: i32): void {}", MalformedFunction::MissingReturnType, "FunctionDecl requires a return type annotation"},
        {"func f(Value: i32): void {}", MalformedFunction::MissingReturnTypeExpression, "TypeSyntax requires an expression"},
        {"import \"C\" func f(Value: i32): void;", MalformedFunction::MissingNativeLinkage, "FunctionDecl native import/export requires a linkage expression"},
        {"import \"C\" func f(Value: i32): void;", MalformedFunction::NonLiteralNativeLinkage, "FunctionDecl linkage must be a string literal"},
        {"import \"C\" func f(Value: i32): void; 1;", MalformedFunction::NonStringNativeLinkage, "FunctionDecl linkage must be a string literal"},
        {"import \"C\" func f(Value: i32): void;", MalformedFunction::OutOfBoundsNativeLinkageToken, "FunctionDecl linkage literal must reference a token in its input"},
        {"import \"C\" func f(Value: i32): void;", MalformedFunction::InputEndNativeLinkageToken, "FunctionDecl linkage literal must reference a token in its input"},
        {"import \"C\" func f(Value: i32): void;", MalformedFunction::MismatchedNativeLinkageToken, "FunctionDecl linkage literal must reference a string token with StringInfo metadata"},
        {"import \"C\" func f(Value: i32): void;", MalformedFunction::UnexpectedLocalLinkage, "FunctionDecl without native import/export must not have a linkage expression"},
        {"func f(Value: i32): void {}", MalformedFunction::MissingParameterType, "FunctionDecl parameter requires a type annotation"},
        {"func f(Value: i32): void {}", MalformedFunction::MissingParameterName, "FunctionDecl parameter requires a name"},
        {"func f(Value: i32): void {}", MalformedFunction::MissingFunctionName, "FunctionDecl requires a name"},
        {"func f(Value: i32): void {}", MalformedFunction::InvalidBodyKind, "FunctionDecl body kind must be DeclarationOnly or Definition"},
        {"func f(Value: i32): void {}", MalformedFunction::InvalidNativeKind, "FunctionDecl native symbol kind must be None, Import or Export"},
        {"func f(Value: i32): void {}", MalformedFunction::InvalidVisibility, "FunctionDecl visibility must be Default, Public or Private"},
        {"func f[T: type](Value: i32): void {}", MalformedFunction::MissingDefinitionBody, "FunctionDecl definition requires a body"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Description);
      auto Input = read(Entry.Source);
      ASSERT_TRUE(Input.succeeded());
      invalidateFunction(Input, Entry.Failure);
      ASSERT_TRUE(Input.succeeded());
      const bool InvalidTokenReference = Entry.Failure == MalformedFunction::OutOfBoundsNativeLinkageToken || Entry.Failure == MalformedFunction::InputEndNativeLinkageToken;
      const std::string Expected = InvalidTokenReference ? "internal compiler error\\[INK-P0048\\]: token index .* is out of bounds for input of .* tokens" : std::string("internal compiler error\\[INK-S0061\\]: invalid AST: ") + Entry.Description;
      EXPECT_DEATH(analyze(Input), Expected);
      EXPECT_EQ(Output.globalFunctionCount(), 0U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Generics, attributes, default arguments, variadics and local function declarations remain explicit unsupported paths.
  TEST_F(SemanticFunctionDeclTest, ReportsUnsupportedDeclarationFeatures)
  {
    const char *Cases[] = {
        "func f[T: type](Value: T): T {}",
        "[tag] func f(): void {}",
        "func f(Value: i32 = 1): void {}",
        "func f(Value: i32...): void {}",
        "func outer(): void { func inner(): void {} }",
        "import \"C++\" func f(): void;",
        "comptime func f(): void {}",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      auto Input = read(Source);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "internal compiler error\\[INK-S0012\\]");
    }
  }
} // namespace ink::semantic::test
