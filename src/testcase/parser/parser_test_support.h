#ifndef INK_TESTCASE_PARSER_PARSER_TEST_SUPPORT_H
#define INK_TESTCASE_PARSER_PARSER_TEST_SUPPORT_H

#include "ink/parser/parser.h"

#include "../diagnostic_test_support.h"
#include "../tokenizer/tokenizer_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::parser
{
  inline ParsedFile parse(tokenizer::TokenizedBuffer LexedFile, ParserOptions Options = {})
  {
    ink::test::SharedDiagnosticTestContext &TestContext = ink::test::sharedDiagnosticTestContext();
    const std::vector<core::Diagnostic> LexicalDiagnostics = tokenizer::testDiagnostics(LexedFile);
    const std::size_t Checkpoint = TestContext.checkpoint();
    const core::SourceId Source = LexedFile.sourceId();
    core::FrontendContext Context(TestContext.compilationContext());
    ParsedFile Result = parse(Context, std::move(LexedFile), Options);
    TestContext.record(Source, Checkpoint, LexicalDiagnostics);
    return Result;
  }
} // namespace ink::parser

namespace ink::parser::test
{
  inline ParsedFile parseSource(std::string Source, ParserOptions Options = {})
  {
    ink::test::SharedDiagnosticTestContext &TestContext = ink::test::sharedDiagnosticTestContext();
    const std::size_t Checkpoint = TestContext.checkpoint();
    core::FrontendContext Context(TestContext.compilationContext());
    tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(Context, std::move(Source), tokenizer::TokenizerOptions{0, true});
    if (!LexedFile.succeeded())
    {
      ADD_FAILURE() << "parser test source must tokenize successfully";
      tokenizer::TokenizedBuffer EmptyFile = tokenizer::tokenize(Context, std::string());
      ParsedFile Result = ink::parser::parse(Context, std::move(EmptyFile), Options);
      TestContext.record(Result.lexedFile().sourceId(), Checkpoint);
      return Result;
    }
    ParsedFile Result = ink::parser::parse(Context, std::move(LexedFile), Options);
    TestContext.record(Result.lexedFile().sourceId(), Checkpoint);
    return Result;
  }

  inline const std::vector<core::Diagnostic> &testDiagnostics(const ParsedFile &File) noexcept
  {
    return ink::test::sharedDiagnosticTestContext().diagnostics(File.lexedFile().sourceId());
  }

  inline bool diagnosticsEqual(const ParsedFile &Left, const ParsedFile &Right)
  {
    const std::vector<core::Diagnostic> &LeftDiagnostics = testDiagnostics(Left);
    const std::vector<core::Diagnostic> &RightDiagnostics = testDiagnostics(Right);
    if (LeftDiagnostics.size() != RightDiagnostics.size())
    {
      return false;
    }
    for (std::size_t Index = 0; Index < LeftDiagnostics.size(); ++Index)
    {
      core::Diagnostic LeftDiagnostic = LeftDiagnostics[Index];
      core::Diagnostic RightDiagnostic = RightDiagnostics[Index];
      LeftDiagnostic.Source = {};
      RightDiagnostic.Source = {};
      if (LeftDiagnostic != RightDiagnostic)
      {
        return false;
      }
    }
    return true;
  }

  inline std::vector<AstNodeId> nodesOfKind(const ParsedFile &File, AstKind Kind)
  {
    std::vector<AstNodeId> Result;
    for (AstNodeId Id = 0; Id < File.ast().size(); ++Id)
    {
      if (File.ast().node(Id).kind() == Kind)
      {
        Result.push_back(Id);
      }
    }
    return Result;
  }

  inline std::size_t countKind(const ParsedFile &File, AstKind Kind)
  {
    return nodesOfKind(File, Kind).size();
  }

  inline bool hasKind(const ParsedFile &File, AstKind Kind)
  {
    return countKind(File, Kind) != 0;
  }

  inline bool hasDiagnostic(const ParsedFile &File, core::DiagnosticKind Kind)
  {
    return std::any_of(testDiagnostics(File).begin(), testDiagnostics(File).end(), [Kind](const core::Diagnostic &Diagnostic)
                       {
                         return Diagnostic.Kind == Kind;
                       });
  }

  inline bool hasExpectedDiagnostic(const ParsedFile &File, std::string_view Expected, std::size_t Anchor)
  {
    for (const core::Diagnostic &Diagnostic : testDiagnostics(File))
    {
      if (Diagnostic.Kind != core::DiagnosticKind::ExpectedToken || Diagnostic.Span != core::SourceRange{Anchor, Anchor})
      {
        continue;
      }
      for (const core::DiagnosticArgument &Argument : Diagnostic.Arguments)
      {
        const std::string *Spelling = std::get_if<std::string>(&Argument.Value);
        if (Argument.Name == core::DiagnosticArgumentName::Expected && Spelling != nullptr && *Spelling == Expected)
        {
          return true;
        }
      }
    }
    return false;
  }

  inline std::string tokenText(const ParsedFile &File, AstTokenId Id)
  {
    if (Id >= File.lexedFile().tokens().size())
    {
      ADD_FAILURE() << "AST token reference is out of bounds";
      return {};
    }
    return std::string(File.lexedFile().raw(File.lexedFile().tokens()[Id]));
  }

  inline std::string nodeText(const ParsedFile &File, AstNodeId Id)
  {
    if (Id >= File.ast().size())
    {
      ADD_FAILURE() << "AST node reference is out of bounds";
      return {};
    }
    const core::SourceRange Span = File.ast().node(Id).Span;
    if (Span.Start > Span.End || Span.End > File.lexedFile().source().size())
    {
      ADD_FAILURE() << "AST node range is out of source bounds";
      return {};
    }
    return std::string(File.lexedFile().source().substr(Span.Start, Span.size()));
  }

  inline std::vector<std::string> nodeTextsOfKind(const ParsedFile &File, AstKind Kind)
  {
    std::vector<std::string> Result;
    for (AstNodeId Id : nodesOfKind(File, Kind))
    {
      Result.push_back(nodeText(File, Id));
    }
    return Result;
  }

  inline std::vector<AstTokenId> namedTokens(const AstNode &Node)
  {
    std::vector<AstTokenId> Result;
    const auto Visit = [&Result](const auto &Value)
    {
      using NodeType = std::decay_t<decltype(Value)>;
      if constexpr (std::is_same_v<NodeType, ImportDeclaration>)
      {
        Result = Value.Package;
        Result.push_back(Value.Member);
        Result.push_back(Value.Alias);
      }
      else if constexpr (std::is_same_v<NodeType, FunctionDeclaration>)
      {
        Result = {Value.Name, Value.Linkage};
      }
      else if constexpr (std::is_same_v<NodeType, ClassDeclaration> || std::is_same_v<NodeType, InterfaceDeclaration> || std::is_same_v<NodeType, EnumDeclaration> || std::is_same_v<NodeType, ClassFieldDeclaration> || std::is_same_v<NodeType, EnumFieldDeclaration> || std::is_same_v<NodeType, GenericParameter> || std::is_same_v<NodeType, NamePattern> || std::is_same_v<NodeType, NameExpression>)
      {
        Result.push_back(Value.Name);
      }
      else if constexpr (std::is_same_v<NodeType, FunctionTypeExpression>)
      {
        Result.push_back(Value.Linkage);
      }
      else if constexpr (std::is_same_v<NodeType, MemberExpression>)
      {
        Result.push_back(Value.Member);
      }
      else if constexpr (std::is_same_v<NodeType, LiteralExpression>)
      {
        Result.push_back(Value.Token);
      }
    };
    visitAstNode(Node, Visit);
    Result.erase(std::remove(Result.begin(), Result.end(), InvalidAstTokenId), Result.end());
    return Result;
  }

  inline std::string astSnapshot(const ParsedFile &File)
  {
    std::ostringstream Result;
    Result << "root=" << File.ast().root() << '\n';
    for (AstNodeId Id = 0; Id < File.ast().size(); ++Id)
    {
      const AstNode &Node = File.ast().node(Id);
      Result << Id << ':' << astKindName(Node.kind()) << ':' << Node.Span.Start << ':' << Node.Span.End << ':' << static_cast<unsigned>(Node.Flags);
      for (AstTokenId Token : namedTokens(Node))
      {
        Result << ":token=" << Token;
      }
      for (const AstChildEdge &Child : File.ast().children(Id))
      {
        Result << ':' << Child.Role << '[' << Child.Index << "]=" << Child.Id;
      }
      const auto Visit = [&Result](const auto &Value)
      {
        using NodeType = std::decay_t<decltype(Value)>;
        if constexpr (std::is_same_v<NodeType, Error>)
        {
          Result << ":expected=" << Value.Expected;
        }
        if constexpr (std::is_same_v<NodeType, ImportDeclaration>)
        {
          Result << ":member=" << Value.IsMemberImport << ":alias=" << Value.Alias;
        }
        if constexpr (std::is_same_v<NodeType, FunctionDeclaration> || std::is_same_v<NodeType, ClassDeclaration> || std::is_same_v<NodeType, InterfaceDeclaration> || std::is_same_v<NodeType, EnumDeclaration> || std::is_same_v<NodeType, ClassFieldDeclaration> || std::is_same_v<NodeType, BindingDeclaration>)
        {
          Result << ":access=" << static_cast<unsigned>(Value.Access);
        }
        if constexpr (std::is_same_v<NodeType, FunctionDeclaration>)
        {
          Result << ":method=" << Value.IsClassMethod;
        }
        if constexpr (std::is_same_v<NodeType, FunctionDeclaration> || std::is_same_v<NodeType, ClassFieldDeclaration> || std::is_same_v<NodeType, ReferenceTypeExpression> || std::is_same_v<NodeType, PointerTypeExpression>)
        {
          Result << ":const=" << Value.IsConst;
        }
        if constexpr (std::is_same_v<NodeType, BindingDeclaration> || std::is_same_v<NodeType, ForInStatement>)
        {
          Result << ":binding=" << static_cast<unsigned>(Value.Binding);
        }
        if constexpr (std::is_same_v<NodeType, FunctionParameter> || std::is_same_v<NodeType, GenericParameter>)
        {
          Result << ":variadic=" << Value.IsVariadic;
        }
        if constexpr (std::is_same_v<NodeType, BinaryExpression> || std::is_same_v<NodeType, UnaryExpression> || std::is_same_v<NodeType, AssignmentStatement>)
        {
          Result << ":operator=" << static_cast<unsigned>(Value.Operator);
        }
        if constexpr (std::is_same_v<NodeType, MemberExpression>)
        {
          Result << ":pointer=" << Value.IsPointer;
        }
        if constexpr (std::is_same_v<NodeType, BuiltinTypeExpression>)
        {
          Result << ":type=" << static_cast<unsigned>(Value.Type);
        }
        if constexpr (std::is_same_v<NodeType, ReceiverExpression>)
        {
          Result << ":receiver=" << static_cast<unsigned>(Value.Receiver);
        }
        if constexpr (std::is_same_v<NodeType, CallExpression> || std::is_same_v<NodeType, GenericInstantiationExpression>)
        {
          for (const AstArgument &Argument : Value.Arguments)
          {
            Result << ":argument=" << Argument.Expression << ':' << Argument.IsPackExpansion << ':' << Argument.Span.Start << ':' << Argument.Span.End;
          }
        }
        if constexpr (std::is_same_v<NodeType, FunctionTypeExpression>)
        {
          for (const FunctionTypeParameter &Parameter : Value.Parameters)
          {
            Result << ":parameter=" << Parameter.TypeExpression << ':' << Parameter.IsVariadic << ':' << Parameter.Span.Start << ':' << Parameter.Span.End;
          }
        }
      };
      visitAstNode(Node, Visit);
      Result << '\n';
    }
    return Result.str();
  }

  inline void expectAstIntegrity(const ParsedFile &File)
  {
    const AstTree &Tree = File.ast();
    ASSERT_FALSE(Tree.empty());
    ASSERT_LT(Tree.root(), Tree.size());
    EXPECT_EQ(Tree.node(Tree.root()).kind(), AstKind::SourceFile);
    EXPECT_EQ(Tree.node(Tree.root()).Span, (core::SourceRange{0, File.lexedFile().source().size()}));
    std::vector<std::size_t> Visits(Tree.size(), 0);
    std::vector<AstNodeId> Work = {Tree.root()};
    while (!Work.empty())
    {
      const AstNodeId Id = Work.back();
      Work.pop_back();
      ASSERT_LT(Id, Tree.size());
      ASSERT_EQ(Visits[Id]++, 0u) << "AST node " << Id << " is reached more than once";
      const AstNode &Node = Tree.node(Id);
      SCOPED_TRACE(Id);
      EXPECT_LE(Node.Span.Start, Node.Span.End);
      EXPECT_LE(Node.Span.End, File.lexedFile().source().size());
      EXPECT_EQ(File.span(Id), Node.Span);
      if (File.succeeded())
      {
        EXPECT_EQ(Node.Flags, AstNodeFlags::None);
        EXPECT_NE(Node.kind(), AstKind::Error);
      }
      for (AstTokenId Token : namedTokens(Node))
      {
        ASSERT_LT(Token, File.lexedFile().tokens().size());
        const tokenizer::Token &TokenValue = File.lexedFile().tokens()[Token];
        EXPECT_FALSE(TokenValue.isTrivia());
        EXPECT_LE(Node.Span.Start, TokenValue.Span.Start);
        EXPECT_GE(Node.Span.End, TokenValue.Span.End);
      }
      for (const AstChildEdge &Child : Tree.children(Id))
      {
        ASSERT_LT(Child.Id, Tree.size());
        const AstNode &ChildNode = Tree.node(Child.Id);
        EXPECT_LE(Node.Span.Start, ChildNode.Span.Start);
        EXPECT_GE(Node.Span.End, ChildNode.Span.End);
        EXPECT_EQ(Node.Flags | ChildNode.Flags, Node.Flags);
        Work.push_back(Child.Id);
      }
    }
    for (AstNodeId Id = 0; Id < Visits.size(); ++Id)
    {
      EXPECT_EQ(Visits[Id], 1u) << "AST node " << Id << " is not reachable from the source file";
    }
  }
} // namespace ink::parser::test

#endif
