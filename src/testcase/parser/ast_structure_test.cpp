#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <llvm/Support/Casting.h>

namespace ink::parser
{
  namespace
  {
    using test::countKind;
    using test::expectAstIntegrity;
    using test::hasKind;
    using test::nodeText;
    using test::parseSource;
    using test::tokenText;

    // Verifies concrete nodes inherit the correct category and support checked base-to-derived casts without RTTI or slicing.
    TEST(ParserAstStructureTest, InheritedNodesSupportCheckedCategoryCasts)
    {
      static_assert(!std::is_copy_constructible_v<AstNode> && !std::is_destructible_v<AstNode>);
      static_assert(!std::is_copy_constructible_v<AstDeclaration> && !std::is_destructible_v<AstDeclaration>);
      static_assert(!std::is_copy_constructible_v<AstStatement> && !std::is_destructible_v<AstStatement>);
      static_assert(!std::is_copy_constructible_v<AstExpression> && !std::is_destructible_v<AstExpression>);
      static_assert(!std::is_copy_constructible_v<AstPattern> && !std::is_destructible_v<AstPattern>);
      static_assert(!std::is_polymorphic_v<AstNode>);
      static_assert(!std::is_copy_constructible_v<AstTree> && std::is_nothrow_move_constructible_v<AstTree>);
      const ParsedFile File = parseSource("let Value: int32 = 1 + 2; return Value;");
      ASSERT_TRUE(File.succeeded());
      const SourceFile &Root = File.ast().node(File.ast().root()).get<SourceFile>();
      const AstNode &Declaration = File.ast().node(Root.Statements[0]);
      EXPECT_NE(Declaration.as<AstNode>(), nullptr);
      EXPECT_NE(Declaration.as<AstDeclaration>(), nullptr);
      EXPECT_NE(Declaration.as<BindingDeclaration>(), nullptr);
      EXPECT_EQ(Declaration.as<AstExpression>(), nullptr);
      EXPECT_EQ(Declaration.as<ReturnStatement>(), nullptr);
      const BindingDeclaration &Binding = Declaration.get<BindingDeclaration>();
      const AstNode &Expression = File.ast().node(Binding.Initializer);
      EXPECT_NE(Expression.as<AstExpression>(), nullptr);
      EXPECT_TRUE(llvm::isa<BinaryExpression>(&Expression));
      EXPECT_EQ(llvm::dyn_cast<BinaryExpression>(&Expression), Expression.as<BinaryExpression>());
      EXPECT_EQ(llvm::dyn_cast<AstDeclaration>(&Expression), nullptr);
      EXPECT_NE(File.ast().node(Binding.Pattern).as<AstPattern>(), nullptr);
      EXPECT_NE(File.ast().node(Root.Statements[1]).as<AstStatement>(), nullptr);
      const auto ReadKind = [](const auto &Concrete)
      {
        return Concrete.kind();
      };
      EXPECT_EQ(visitAstNode(Expression, ReadKind), AstKind::BinaryExpression);
      expectAstIntegrity(File);
    }

    // Verifies moving parsed files preserves node addresses and token ownership while replacing a populated destination safely.
    TEST(ParserAstStructureTest, MovingParsedFilesPreservesNodeAddressesAndOwnedFields)
    {
      ParsedFile Original = parseSource("import core.io as io; func f(Value: int32) -> int32 { return Value; }");
      ASSERT_TRUE(Original.succeeded());
      const AstNodeId RootId = Original.ast().root();
      const AstNode *Root = &Original.ast().node(RootId);
      const std::string Source = Original.lexedFile().source();
      ParsedFile Moved(std::move(Original));
      EXPECT_FALSE(Original.succeeded());
      EXPECT_FALSE(Original.lexedFile().succeeded());
      EXPECT_TRUE(Original.ast().empty());
      EXPECT_EQ(Original.ast().root(), InvalidAstNodeId);
      EXPECT_EQ(&Moved.ast().node(RootId), Root);
      ParsedFile Destination = parseSource("let Before: int32 = 0;");
      Destination = std::move(Moved);
      EXPECT_FALSE(Moved.succeeded());
      EXPECT_TRUE(Moved.ast().empty());
      EXPECT_EQ(Moved.ast().root(), InvalidAstNodeId);
      EXPECT_EQ(&Destination.ast().node(RootId), Root);
      EXPECT_EQ(Destination.lexedFile().source(), Source);
      const ImportDeclaration &Import = Destination.ast().node(Root->get<SourceFile>().Statements[0]).get<ImportDeclaration>();
      ASSERT_EQ(Import.Package.size(), 2u);
      EXPECT_EQ(tokenText(Destination, Import.Package[1]), "io");
      EXPECT_EQ(tokenText(Destination, Import.Alias), "io");
      expectAstIntegrity(Destination);
    }

    // Verifies an empty source yields an empty SourceFile AST while the original token buffer retains its EOF sentinel.
    TEST(ParserAstStructureTest, EmptySourceHasAnEmptyStatementList)
    {
      const ParsedFile File = parseSource("");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      ASSERT_EQ(File.ast().size(), 1u);
      const AstNode &Root = File.ast().node(File.ast().root());
      EXPECT_TRUE(Root.get<SourceFile>().Statements.empty());
      EXPECT_EQ(Root.Flags, AstNodeFlags::None);
      ASSERT_EQ(File.lexedFile().tokens().size(), 1u);
      EXPECT_EQ(File.lexedFile().tokens()[0].Kind, tokenizer::TokenKind::EndOfFile);
      expectAstIntegrity(File);
    }

    // Verifies source bytes remain in the token buffer while declarations expose semantic fields instead of punctuation or trivia children.
    TEST(ParserAstStructureTest, DeclarationsExposeNamesTypesInitializersAndBodies)
    {
      const std::string Source = "/* leading */\r\nimport core.io;\r\npublic const Answer: int32 = 42;\r\nfunc compute(value: int32) -> int32\r\n{\r\n  // body\r\n  var copy: int32 = value;\r\n  copy += 1;\r\n  return copy;\r\n}\r\n// trailing\r\n";
      const ParsedFile File = parseSource(Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(test::testDiagnostics(File).empty());
      EXPECT_EQ(File.lexedFile().source(), Source);
      const std::vector<AstNodeId> &Statements = File.ast().node(File.ast().root()).get<SourceFile>().Statements;
      ASSERT_EQ(Statements.size(), 3u);
      EXPECT_EQ(File.ast().node(Statements[0]).kind(), AstKind::ImportDeclaration);
      const BindingDeclaration &Answer = File.ast().node(Statements[1]).get<BindingDeclaration>();
      EXPECT_EQ(Answer.Access, AccessKind::Public);
      EXPECT_EQ(Answer.Binding, BindingKind::Const);
      EXPECT_EQ(tokenText(File, File.ast().node(Answer.Pattern).get<NamePattern>().Name), "Answer");
      EXPECT_EQ(File.ast().node(Answer.TypeExpression).get<BuiltinTypeExpression>().Type, tokenizer::KeywordKind::Int32);
      EXPECT_EQ(nodeText(File, Answer.Initializer), "42");
      const FunctionDeclaration &Function = File.ast().node(Statements[2]).get<FunctionDeclaration>();
      EXPECT_EQ(tokenText(File, Function.Name), "compute");
      ASSERT_EQ(Function.Parameters.size(), 1u);
      EXPECT_EQ(File.ast().node(Function.ReturnType).get<BuiltinTypeExpression>().Type, tokenizer::KeywordKind::Int32);
      const BlockStatement &Body = File.ast().node(Function.Body).get<BlockStatement>();
      ASSERT_EQ(Body.Statements.size(), 3u);
      EXPECT_EQ(File.ast().node(Body.Statements[0]).get<BindingDeclaration>().Binding, BindingKind::Var);
      const AssignmentStatement &Assignment = File.ast().node(Body.Statements[1]).get<AssignmentStatement>();
      EXPECT_EQ(tokenizer::symbolSpelling(Assignment.Operator), "+=");
      EXPECT_EQ(nodeText(File, Assignment.Target), "copy");
      EXPECT_EQ(nodeText(File, Assignment.Value), "1");
      EXPECT_EQ(nodeText(File, File.ast().node(Body.Statements[2]).get<ReturnStatement>().Value), "copy");
      expectAstIntegrity(File);
    }

    // Verifies malformed syntax produces explicit error nodes and propagates error and missing flags to the source file.
    TEST(ParserAstStructureTest, RecoveryPropagatesErrorAndMissingFlags)
    {
      const ParsedFile File = parseSource("func broken(value: int32 { value++; return value }");
      ASSERT_FALSE(File.succeeded());
      ASSERT_FALSE(test::testDiagnostics(File).empty());
      const AstNode &Root = File.ast().node(File.ast().root());
      EXPECT_TRUE(hasFlag(Root.Flags, AstNodeFlags::HasError));
      EXPECT_TRUE(hasFlag(Root.Flags, AstNodeFlags::HasMissing));
      ASSERT_TRUE(hasKind(File, AstKind::Error));
      for (AstNodeId Id : test::nodesOfKind(File, AstKind::Error))
      {
        EXPECT_TRUE(hasFlag(File.ast().node(Id).Flags, AstNodeFlags::HasError));
      }
      expectAstIntegrity(File);
    }

    // Verifies every public AST span is ordered, bounded, and encloses its direct semantic children.
    TEST(ParserAstStructureTest, NodeSpansStayWithinSourceByteBounds)
    {
      const ParsedFile File = parseSource("const first: (int32, int32) = (1, 2);\nfunc second() -> int32 { return first[0]; }\n");
      ASSERT_TRUE(File.succeeded());
      expectAstIntegrity(File);
      for (AstNodeId Id = 0; Id < File.ast().size(); ++Id)
      {
        const core::SourceRange Span = File.span(Id);
        SCOPED_TRACE(Id);
        EXPECT_LE(Span.Start, Span.End);
        EXPECT_LE(Span.End, File.lexedFile().source().size());
      }
    }

    // Verifies missing operands beside comments retain bounded ranges and a valid ownership tree during error recovery.
    TEST(ParserAstStructureTest, RecoveredNodeSpansEncloseChildrenBesideMissingSyntax)
    {
      for (const std::string Source : {"func f( /*a*/: T) -> void;", "foo( /*a*/ , value);"})
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = parseSource(Source);
        ASSERT_FALSE(File.succeeded());
        EXPECT_EQ(File.lexedFile().source(), Source);
        expectAstIntegrity(File);
      }
    }

    // Verifies ten thousand prefix operators use arena references and can be walked and destroyed without recursive host-stack use.
    TEST(ParserAstStructureTest, DeepUnaryPrefixChainUsesStackSafeAstTraversal)
    {
      constexpr std::size_t PrefixCount = 10000;
      std::string Source = "const Deep: int32 = ";
      for (std::size_t Index = 0; Index < PrefixCount; ++Index)
      {
        Source.append("+ ");
      }
      Source.append("Value;");
      const ParsedFile File = parseSource(Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::UnaryExpression), PrefixCount);
      const BindingDeclaration &Binding = File.ast().node(test::nodesOfKind(File, AstKind::BindingDeclaration)[0]).get<BindingDeclaration>();
      AstNodeId Current = Binding.Initializer;
      for (std::size_t Index = 0; Index < PrefixCount; ++Index)
      {
        ASSERT_EQ(File.ast().node(Current).kind(), AstKind::UnaryExpression);
        const UnaryExpression &Unary = File.ast().node(Current).get<UnaryExpression>();
        EXPECT_EQ(tokenizer::symbolSpelling(Unary.Operator), "+");
        Current = Unary.Operand;
      }
      EXPECT_EQ(nodeText(File, Current), "Value");
      EXPECT_EQ(File.lexedFile().source(), Source);
      expectAstIntegrity(File);
    }

    // Verifies a five-thousand-operand additive expression retains a left-associated arena tree without recursive traversal.
    TEST(ParserAstStructureTest, DeepAdditiveChainUsesStackSafeAstTraversal)
    {
      constexpr std::size_t OperandCount = 5000;
      std::string Source = "const Deep: int32 = Value";
      for (std::size_t Index = 1; Index < OperandCount; ++Index)
      {
        Source.append(" + Value");
      }
      Source.push_back(';');
      const ParsedFile File = parseSource(Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::BinaryExpression), OperandCount - 1);
      const BindingDeclaration &Binding = File.ast().node(test::nodesOfKind(File, AstKind::BindingDeclaration)[0]).get<BindingDeclaration>();
      AstNodeId Current = Binding.Initializer;
      for (std::size_t Index = 1; Index < OperandCount; ++Index)
      {
        const BinaryExpression &Binary = File.ast().node(Current).get<BinaryExpression>();
        EXPECT_EQ(tokenizer::symbolSpelling(Binary.Operator), "+");
        EXPECT_EQ(nodeText(File, Binary.Right), "Value");
        Current = Binary.Left;
      }
      EXPECT_EQ(nodeText(File, Current), "Value");
      EXPECT_EQ(File.lexedFile().source(), Source);
      expectAstIntegrity(File);
    }

    // Verifies import paths, selected members, and aliases are explicit token references rather than nested grammar wrappers.
    TEST(ParserAstStructureTest, ImportsExposePackageMemberAndAlias)
    {
      const ParsedFile File = parseSource("import platform.window as window; from application.model import User as CurrentUser;");
      ASSERT_TRUE(File.succeeded());
      const std::vector<AstNodeId> Imports = test::nodesOfKind(File, AstKind::ImportDeclaration);
      ASSERT_EQ(Imports.size(), 2u);
      const ImportDeclaration &Module = File.ast().node(Imports[0]).get<ImportDeclaration>();
      ASSERT_EQ(Module.Package.size(), 2u);
      EXPECT_EQ(tokenText(File, Module.Package[0]), "platform");
      EXPECT_EQ(tokenText(File, Module.Package[1]), "window");
      EXPECT_FALSE(Module.IsMemberImport);
      EXPECT_EQ(Module.Member, InvalidAstTokenId);
      EXPECT_EQ(tokenText(File, Module.Alias), "window");
      const ImportDeclaration &Member = File.ast().node(Imports[1]).get<ImportDeclaration>();
      EXPECT_TRUE(Member.IsMemberImport);
      EXPECT_EQ(tokenText(File, Member.Member), "User");
      EXPECT_EQ(tokenText(File, Member.Alias), "CurrentUser");
      expectAstIntegrity(File);
    }

    // Verifies linkage, constness, access, generic defaults, parameter defaults, and parameter packs survive as structured declaration fields.
    TEST(ParserAstStructureTest, FunctionModifiersDefaultsAndPacksAreAttributes)
    {
      const ParsedFile File = parseSource("public extern \"C\" const func load[T: type, N: int32 = 4, Rest: type...](path: const ref T, count: int32 = 1, values: T...) -> Result; private class_method create(value: T) -> T { return value; } extern \"C\" func trace(format: Text, ...) -> void;");
      ASSERT_TRUE(File.succeeded());
      const std::vector<AstNodeId> Functions = test::nodesOfKind(File, AstKind::FunctionDeclaration);
      ASSERT_EQ(Functions.size(), 3u);
      const FunctionDeclaration &Load = File.ast().node(Functions[0]).get<FunctionDeclaration>();
      EXPECT_EQ(Load.Access, AccessKind::Public);
      EXPECT_TRUE(Load.IsConst);
      EXPECT_FALSE(Load.IsClassMethod);
      EXPECT_EQ(tokenText(File, Load.Linkage), "\"C\"");
      EXPECT_EQ(Load.Body, InvalidAstNodeId);
      ASSERT_EQ(Load.GenericParameters.size(), 3u);
      const GenericParameter &DefaultGeneric = File.ast().node(Load.GenericParameters[1]).get<GenericParameter>();
      EXPECT_EQ(tokenText(File, DefaultGeneric.Name), "N");
      EXPECT_EQ(nodeText(File, DefaultGeneric.DefaultValue), "4");
      EXPECT_TRUE(File.ast().node(Load.GenericParameters[2]).get<GenericParameter>().IsVariadic);
      ASSERT_EQ(Load.Parameters.size(), 3u);
      const FunctionParameter &Path = File.ast().node(Load.Parameters[0]).get<FunctionParameter>();
      EXPECT_TRUE(File.ast().node(Path.TypeExpression).get<ReferenceTypeExpression>().IsConst);
      EXPECT_EQ(nodeText(File, File.ast().node(Load.Parameters[1]).get<FunctionParameter>().DefaultValue), "1");
      EXPECT_TRUE(File.ast().node(Load.Parameters[2]).get<FunctionParameter>().IsVariadic);
      const FunctionDeclaration &Method = File.ast().node(Functions[1]).get<FunctionDeclaration>();
      EXPECT_EQ(Method.Access, AccessKind::Private);
      EXPECT_TRUE(Method.IsClassMethod);
      EXPECT_NE(Method.Body, InvalidAstNodeId);
      const FunctionDeclaration &Trace = File.ast().node(Functions[2]).get<FunctionDeclaration>();
      ASSERT_EQ(Trace.Parameters.size(), 2u);
      const FunctionParameter &BarePack = File.ast().node(Trace.Parameters[1]).get<FunctionParameter>();
      EXPECT_TRUE(BarePack.IsVariadic);
      EXPECT_EQ(BarePack.Pattern, InvalidAstNodeId);
      EXPECT_EQ(BarePack.TypeExpression, InvalidAstNodeId);
      expectAstIntegrity(File);
    }

    // Verifies class, interface, enum, and field declarations expose inheritance and qualifiers in their named fields.
    TEST(ParserAstStructureTest, NominalDeclarationsExposeInheritanceAndFields)
    {
      const ParsedFile File = parseSource("public class Box[T: type] extends makeBase(T) implements Printable, Sized { private const class_field Value: T = Default; } interface I extends First, Second {} enum E : int32 implements Printable { enum_field Ready; enum_field Busy = 2; }");
      ASSERT_TRUE(File.succeeded());
      const ClassDeclaration &Class = File.ast().node(test::nodesOfKind(File, AstKind::ClassDeclaration)[0]).get<ClassDeclaration>();
      EXPECT_EQ(Class.Access, AccessKind::Public);
      EXPECT_EQ(tokenText(File, Class.Name), "Box");
      EXPECT_EQ(Class.GenericParameters.size(), 1u);
      EXPECT_EQ(nodeText(File, Class.BaseType), "makeBase(T)");
      ASSERT_EQ(Class.Interfaces.size(), 2u);
      EXPECT_EQ(nodeText(File, Class.Interfaces[0]), "Printable");
      EXPECT_EQ(nodeText(File, Class.Interfaces[1]), "Sized");
      const BlockStatement &Body = File.ast().node(Class.Body).get<BlockStatement>();
      ASSERT_EQ(Body.Statements.size(), 1u);
      const ClassFieldDeclaration &Field = File.ast().node(Body.Statements[0]).get<ClassFieldDeclaration>();
      EXPECT_EQ(Field.Access, AccessKind::Private);
      EXPECT_TRUE(Field.IsConst);
      EXPECT_EQ(tokenText(File, Field.Name), "Value");
      EXPECT_EQ(nodeText(File, Field.TypeExpression), "T");
      EXPECT_EQ(nodeText(File, Field.Initializer), "Default");
      const InterfaceDeclaration &Interface = File.ast().node(test::nodesOfKind(File, AstKind::InterfaceDeclaration)[0]).get<InterfaceDeclaration>();
      EXPECT_EQ(Interface.BaseTypes.size(), 2u);
      const EnumDeclaration &Enum = File.ast().node(test::nodesOfKind(File, AstKind::EnumDeclaration)[0]).get<EnumDeclaration>();
      EXPECT_EQ(nodeText(File, Enum.BaseType), "int32");
      EXPECT_EQ(Enum.Interfaces.size(), 1u);
      const std::vector<AstNodeId> &Fields = File.ast().node(Enum.Body).get<BlockStatement>().Statements;
      ASSERT_EQ(Fields.size(), 2u);
      EXPECT_EQ(File.ast().node(Fields[0]).get<EnumFieldDeclaration>().Initializer, InvalidAstNodeId);
      EXPECT_EQ(nodeText(File, File.ast().node(Fields[1]).get<EnumFieldDeclaration>().Initializer), "2");
      expectAstIntegrity(File);
    }

    // Verifies for-in and control-style for loops expose different typed payloads and direct clause references.
    TEST(ParserAstStructureTest, ForFormsExposeTheirDistinctClauses)
    {
      const ParsedFile File = parseSource("for (let (Key, Value): Entry in Items) {} for (var I: int32 = 0; I < 10; I += 1, visit(I)) {} for (;;) {}");
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, AstKind::ForInStatement), 1u);
      ASSERT_EQ(countKind(File, AstKind::ForStatement), 2u);
      const ForInStatement &Iteration = File.ast().node(test::nodesOfKind(File, AstKind::ForInStatement)[0]).get<ForInStatement>();
      EXPECT_EQ(Iteration.Binding, BindingKind::Let);
      EXPECT_EQ(File.ast().node(Iteration.Pattern).kind(), AstKind::TuplePattern);
      EXPECT_EQ(nodeText(File, Iteration.TypeExpression), "Entry");
      EXPECT_EQ(nodeText(File, Iteration.Iterable), "Items");
      const std::vector<AstNodeId> Loops = test::nodesOfKind(File, AstKind::ForStatement);
      const ForStatement &Control = File.ast().node(Loops[0]).get<ForStatement>();
      ASSERT_EQ(Control.Initializers.size(), 1u);
      EXPECT_EQ(File.ast().node(Control.Initializers[0]).kind(), AstKind::BindingDeclaration);
      EXPECT_EQ(nodeText(File, Control.Condition), "I < 10");
      ASSERT_EQ(Control.Updates.size(), 2u);
      EXPECT_EQ(File.ast().node(Control.Updates[0]).kind(), AstKind::AssignmentStatement);
      const ForStatement &Empty = File.ast().node(Loops[1]).get<ForStatement>();
      EXPECT_TRUE(Empty.Initializers.empty());
      EXPECT_EQ(Empty.Condition, InvalidAstNodeId);
      EXPECT_TRUE(Empty.Updates.empty());
      expectAstIntegrity(File);
    }

    // Verifies calls, generic instantiations, and pointer member access retain their targets and inline argument-pack attributes.
    TEST(ParserAstStructureTest, PostfixPayloadsRetainTargetsArgumentsAndMemberModes)
    {
      const ParsedFile File = parseSource("Object.method::[T, Types...](Value, Arguments...)[Index].field->next;");
      ASSERT_TRUE(File.succeeded());
      const GenericInstantiationExpression &Generic = File.ast().node(test::nodesOfKind(File, AstKind::GenericInstantiationExpression)[0]).get<GenericInstantiationExpression>();
      EXPECT_EQ(nodeText(File, Generic.Target), "Object.method");
      ASSERT_EQ(Generic.Arguments.size(), 2u);
      EXPECT_FALSE(Generic.Arguments[0].IsPackExpansion);
      EXPECT_TRUE(Generic.Arguments[1].IsPackExpansion);
      EXPECT_EQ(nodeText(File, Generic.Arguments[1].Expression), "Types");
      const CallExpression &Call = File.ast().node(test::nodesOfKind(File, AstKind::CallExpression)[0]).get<CallExpression>();
      EXPECT_EQ(File.ast().node(Call.Callee).kind(), AstKind::GenericInstantiationExpression);
      ASSERT_EQ(Call.Arguments.size(), 2u);
      EXPECT_FALSE(Call.Arguments[0].IsPackExpansion);
      EXPECT_TRUE(Call.Arguments[1].IsPackExpansion);
      EXPECT_EQ(nodeText(File, Call.Arguments[1].Expression), "Arguments");
      const IndexExpression &Index = File.ast().node(test::nodesOfKind(File, AstKind::IndexExpression)[0]).get<IndexExpression>();
      EXPECT_EQ(File.ast().node(Index.Target).kind(), AstKind::CallExpression);
      EXPECT_EQ(nodeText(File, Index.Index), "Index");
      const std::vector<AstNodeId> Members = test::nodesOfKind(File, AstKind::MemberExpression);
      ASSERT_EQ(Members.size(), 3u);
      EXPECT_FALSE(File.ast().node(Members[0]).get<MemberExpression>().IsPointer);
      const MemberExpression &PointerMember = File.ast().node(Members[2]).get<MemberExpression>();
      EXPECT_TRUE(PointerMember.IsPointer);
      EXPECT_EQ(tokenText(File, PointerMember.Member), "next");
      expectAstIntegrity(File);
    }

    // Verifies function types retain linkage, typed and untyped variadic parameters, and unary result-type operands.
    TEST(ParserAstStructureTest, FunctionTypesExposeInlineParameterMetadata)
    {
      const ParsedFile File = parseSource("let F: type = extern \"C\" func(int32, const ptr T, ...) -> ref U; let G: type = func(T...) -> V;");
      ASSERT_TRUE(File.succeeded());
      const std::vector<AstNodeId> Types = test::nodesOfKind(File, AstKind::FunctionTypeExpression);
      ASSERT_EQ(Types.size(), 2u);
      const FunctionTypeExpression &Function = File.ast().node(Types[0]).get<FunctionTypeExpression>();
      EXPECT_EQ(tokenText(File, Function.Linkage), "\"C\"");
      ASSERT_EQ(Function.Parameters.size(), 3u);
      EXPECT_FALSE(Function.Parameters[0].IsVariadic);
      EXPECT_TRUE(File.ast().node(Function.Parameters[1].TypeExpression).get<PointerTypeExpression>().IsConst);
      EXPECT_TRUE(Function.Parameters[2].IsVariadic);
      EXPECT_EQ(Function.Parameters[2].TypeExpression, InvalidAstNodeId);
      EXPECT_EQ(File.ast().node(Function.ReturnType).kind(), AstKind::ReferenceTypeExpression);
      const FunctionTypeExpression &TypedPack = File.ast().node(Types[1]).get<FunctionTypeExpression>();
      ASSERT_EQ(TypedPack.Parameters.size(), 1u);
      EXPECT_TRUE(TypedPack.Parameters[0].IsVariadic);
      EXPECT_EQ(nodeText(File, TypedPack.Parameters[0].TypeExpression), "T");
      expectAstIntegrity(File);
    }
  } // namespace
} // namespace ink::parser
