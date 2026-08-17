#ifndef INK_TESTCASE_PARSER_PARSER_TEST_SUPPORT_H
#define INK_TESTCASE_PARSER_PARSER_TEST_SUPPORT_H

#include "ink/parser/parser.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ink::parser::test
{
  inline ParsedFile parseSource(std::string Source, ParserOptions Options = {})
  {
    tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(std::move(Source));
    if (!LexedFile.succeeded())
    {
      ADD_FAILURE() << "parser test source must tokenize successfully";
      return ink::parser::parse(tokenizer::tokenize(std::string()), Options);
    }
    return ink::parser::parse(std::move(LexedFile), Options);
  }

  inline std::vector<CstNodeId> nodesOfKind(const ParsedFile &File, CstKind Kind)
  {
    std::vector<CstNodeId> Result;
    for (CstNodeId Id = 0; Id < File.cst().nodes().size(); ++Id)
    {
      if (File.cst().nodes()[Id].Kind == Kind)
      {
        Result.push_back(Id);
      }
    }
    return Result;
  }

  inline std::size_t countKind(const ParsedFile &File, CstKind Kind)
  {
    return static_cast<std::size_t>(std::count_if(File.cst().nodes().begin(), File.cst().nodes().end(), [Kind](const CstNode &Node)
                                                  {
                                                    return Node.Kind == Kind;
                                                  }));
  }

  inline bool hasKind(const ParsedFile &File, CstKind Kind)
  {
    return countKind(File, Kind) != 0;
  }

  inline bool hasDiagnostic(const ParsedFile &File, core::DiagnosticKind Kind)
  {
    return std::any_of(File.diagnostics().begin(), File.diagnostics().end(), [Kind](const core::Diagnostic &Diagnostic)
                       {
                         return Diagnostic.Kind == Kind;
                       });
  }

  namespace detail
  {
    struct NodeLocationFrame
    {
        CstNodeId Id;
        std::size_t NodeStart;
    };

    inline std::size_t nodeTokenStart(const ParsedFile &File, CstNodeId Target)
    {
      const CstTree &Tree = File.cst();
      if (Target >= Tree.nodes().size())
      {
        ADD_FAILURE() << "requested CST node is out of range";
        return 0;
      }

      std::vector<NodeLocationFrame> Work = {{Tree.root(), 0}};
      std::vector<bool> Visited(Tree.nodes().size(), false);
      while (!Work.empty())
      {
        const NodeLocationFrame Current = Work.back();
        Work.pop_back();
        if (Current.Id >= Tree.nodes().size())
        {
          ADD_FAILURE() << "CST contains an out-of-range node reference";
          return 0;
        }
        if (Visited[Current.Id])
        {
          ADD_FAILURE() << "CST node is reachable more than once";
          return 0;
        }
        Visited[Current.Id] = true;
        if (Current.Id == Target)
        {
          return Current.NodeStart;
        }

        const CstNode &Node = Tree.node(Current.Id);
        if (Node.FirstChild > Tree.children().size() || Node.ChildCount > Tree.children().size() - Node.FirstChild)
        {
          ADD_FAILURE() << "CST child range is out of bounds";
          return 0;
        }
        std::size_t ConsumedTokens = 0;
        for (std::size_t Offset = 0; Offset < Node.ChildCount; ++Offset)
        {
          const CstElement &Element = Tree.children()[Node.FirstChild + Offset];
          if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
          {
            if (Child->Id >= Tree.nodes().size())
            {
              ADD_FAILURE() << "CST contains an out-of-range node reference";
              return 0;
            }
            Work.push_back({Child->Id, Current.NodeStart + ConsumedTokens});
            ConsumedTokens += Tree.node(Child->Id).TokenCount;
          }
          else if (std::holds_alternative<CstTokenRef>(Element))
          {
            ++ConsumedTokens;
          }
        }
      }
      ADD_FAILURE() << "requested CST node is unreachable from the root";
      return 0;
    }

    struct TextFrame
    {
        CstNodeId Id;
        std::size_t NodeStart;
        std::size_t NextChild = 0;
        std::size_t ConsumedTokens = 0;
    };
  } // namespace detail

  inline void appendNodeText(const ParsedFile &File, CstNodeId Id, std::string &Result)
  {
    const CstTree &Tree = File.cst();
    if (Id >= Tree.nodes().size())
    {
      ADD_FAILURE() << "requested CST node is out of range";
      return;
    }
    const std::size_t NodeStart = detail::nodeTokenStart(File, Id);
    std::vector<detail::TextFrame> Frames = {{Id, NodeStart}};
    std::vector<bool> ActiveNodes(Tree.nodes().size(), false);
    ActiveNodes[Id] = true;
    while (!Frames.empty())
    {
      detail::TextFrame &Frame = Frames.back();
      if (Frame.Id >= Tree.nodes().size())
      {
        ADD_FAILURE() << "CST contains an out-of-range node reference";
        return;
      }
      const CstNode &Node = Tree.node(Frame.Id);
      if (Node.FirstChild > Tree.children().size() || Node.ChildCount > Tree.children().size() - Node.FirstChild)
      {
        ADD_FAILURE() << "CST child range is out of bounds";
        return;
      }
      if (Frame.NextChild == Node.ChildCount)
      {
        ActiveNodes[Frame.Id] = false;
        Frames.pop_back();
        continue;
      }

      const CstElement &Element = Tree.children()[Node.FirstChild + Frame.NextChild];
      ++Frame.NextChild;
      if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
      {
        if (Child->Id >= Tree.nodes().size())
        {
          ADD_FAILURE() << "CST contains an out-of-range node reference";
          return;
        }
        if (ActiveNodes[Child->Id])
        {
          ADD_FAILURE() << "CST contains a node-reference cycle";
          return;
        }
        const std::size_t ChildStart = Frame.NodeStart + Frame.ConsumedTokens;
        Frame.ConsumedTokens += Tree.node(Child->Id).TokenCount;
        ActiveNodes[Child->Id] = true;
        Frames.push_back({Child->Id, ChildStart});
      }
      else if (const CstTokenRef *TokenReference = std::get_if<CstTokenRef>(&Element))
      {
        if (TokenReference->TokenOffset != Frame.ConsumedTokens)
        {
          ADD_FAILURE() << "CST token offset does not match child order";
          return;
        }
        const std::size_t TokenIndex = Frame.NodeStart + TokenReference->TokenOffset;
        if (TokenIndex >= File.lexedFile().tokens().size())
        {
          ADD_FAILURE() << "CST token reference is out of range";
          return;
        }
        const tokenizer::Token &Token = File.lexedFile().tokens()[TokenIndex];
        Result.append(File.lexedFile().raw(Token));
        ++Frame.ConsumedTokens;
      }
    }
  }

  inline std::string nodeText(const ParsedFile &File, CstNodeId Id)
  {
    std::string Result;
    appendNodeText(File, Id, Result);
    return Result;
  }

  inline std::vector<std::string> nodeTextsOfKind(const ParsedFile &File, CstKind Kind)
  {
    std::vector<std::string> Result;
    for (CstNodeId Id : nodesOfKind(File, Kind))
    {
      Result.push_back(nodeText(File, Id));
    }
    return Result;
  }

  inline std::vector<MissingToken> missingTokens(const ParsedFile &File)
  {
    std::vector<MissingToken> Result;
    for (const CstElement &Element : File.cst().children())
    {
      if (const MissingToken *Missing = std::get_if<MissingToken>(&Element))
      {
        Result.push_back(*Missing);
      }
    }
    return Result;
  }

  namespace detail
  {
    struct MeasuredNode
    {
        std::size_t TokenCount = 0;
        std::size_t TextLength = 0;
        CstNodeFlags Flags = CstNodeFlags::None;
    };

    struct MeasureFrame
    {
        CstNodeId Id;
        std::size_t NodeStart;
        std::size_t NextChild = 0;
        std::size_t ConsumedTokens = 0;
        MeasuredNode Result;
    };

    inline bool pushMeasureFrame(const ParsedFile &File, CstNodeId Id, std::size_t NodeStart, std::vector<std::size_t> &NodeVisits, std::vector<MeasureFrame> &Frames)
    {
      const CstTree &Tree = File.cst();
      if (Id >= Tree.nodes().size())
      {
        ADD_FAILURE() << "CST contains an out-of-range node reference " << Id;
        return false;
      }
      ++NodeVisits[Id];
      if (NodeVisits[Id] != 1)
      {
        ADD_FAILURE() << "CST node " << Id << " is reachable more than once";
        return false;
      }

      const CstNode &Node = Tree.nodes()[Id];
      if (Node.FirstChild > Tree.children().size() || Node.ChildCount > Tree.children().size() - Node.FirstChild)
      {
        ADD_FAILURE() << "CST node " << Id << " has an out-of-range child slice";
        return false;
      }
      MeasureFrame Frame{Id, NodeStart};
      if (Node.Kind == CstKind::Error)
      {
        Frame.Result.Flags |= CstNodeFlags::HasError;
      }
      Frames.push_back(Frame);
      return true;
    }

    inline MeasuredNode measureNode(const ParsedFile &File, CstNodeId Id, std::vector<std::size_t> &NodeVisits, std::vector<std::size_t> &TokenVisits, std::vector<std::size_t> &TokenOrder)
    {
      const CstTree &Tree = File.cst();
      std::vector<MeasureFrame> Frames;
      if (!pushMeasureFrame(File, Id, 0, NodeVisits, Frames))
      {
        return {};
      }
      MeasuredNode RootResult;
      while (!Frames.empty())
      {
        MeasureFrame &Frame = Frames.back();
        const CstNode &Node = Tree.node(Frame.Id);
        if (Frame.NextChild == Node.ChildCount)
        {
          EXPECT_EQ(Node.TokenCount, Frame.Result.TokenCount) << "metadata mismatch in node " << Frame.Id << " (" << cstKindName(Node.Kind) << ")";
          EXPECT_EQ(Node.TextLength, Frame.Result.TextLength) << "metadata mismatch in node " << Frame.Id << " (" << cstKindName(Node.Kind) << ")";
          EXPECT_EQ(static_cast<std::uint8_t>(Node.Flags), static_cast<std::uint8_t>(Frame.Result.Flags)) << "metadata mismatch in node " << Frame.Id << " (" << cstKindName(Node.Kind) << ")";

          const MeasuredNode Completed = Frame.Result;
          Frames.pop_back();
          if (Frames.empty())
          {
            RootResult = Completed;
          }
          else
          {
            MeasureFrame &Parent = Frames.back();
            Parent.Result.TokenCount += Completed.TokenCount;
            Parent.Result.TextLength += Completed.TextLength;
            Parent.Result.Flags |= Completed.Flags;
            Parent.ConsumedTokens += Completed.TokenCount;
          }
          continue;
        }

        const CstElement &Element = Tree.children()[Node.FirstChild + Frame.NextChild];
        ++Frame.NextChild;
        if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
        {
          const std::size_t ChildStart = Frame.NodeStart + Frame.ConsumedTokens;
          pushMeasureFrame(File, Child->Id, ChildStart, NodeVisits, Frames);
        }
        else if (const CstTokenRef *TokenReference = std::get_if<CstTokenRef>(&Element))
        {
          EXPECT_EQ(TokenReference->TokenOffset, Frame.ConsumedTokens) << "token offset mismatch in node " << Frame.Id;
          const std::size_t TokenIndex = Frame.NodeStart + TokenReference->TokenOffset;
          ++Frame.ConsumedTokens;
          if (TokenIndex >= File.lexedFile().tokens().size())
          {
            ADD_FAILURE() << "CST contains an out-of-range token reference " << TokenIndex;
            continue;
          }
          const tokenizer::Token &Token = File.lexedFile().tokens()[TokenIndex];
          ++TokenVisits[TokenIndex];
          TokenOrder.push_back(TokenIndex);
          ++Frame.Result.TokenCount;
          Frame.Result.TextLength += Token.Span.size();
        }
        else
        {
          Frame.Result.Flags |= CstNodeFlags::HasMissing;
        }
      }
      return RootResult;
    }
  } // namespace detail

  inline void expectFullFidelity(const ParsedFile &File)
  {
    const CstTree &Tree = File.cst();
    ASSERT_FALSE(Tree.nodes().empty());
    ASSERT_LT(Tree.root(), Tree.nodes().size());
    EXPECT_EQ(Tree.node(Tree.root()).Kind, CstKind::SourceFile);

    std::vector<std::size_t> NodeVisits(Tree.nodes().size(), 0);
    std::vector<std::size_t> TokenVisits(File.lexedFile().tokens().size(), 0);
    std::vector<std::size_t> TokenOrder;
    const detail::MeasuredNode RootMeasurement = detail::measureNode(File, Tree.root(), NodeVisits, TokenVisits, TokenOrder);

    for (CstNodeId Id = 0; Id < NodeVisits.size(); ++Id)
    {
      EXPECT_EQ(NodeVisits[Id], 1u) << "CST node " << Id << " is not reachable exactly once from the root";
    }
    ASSERT_EQ(TokenOrder.size(), File.lexedFile().tokens().size());
    for (std::size_t TokenIndex = 0; TokenIndex < TokenVisits.size(); ++TokenIndex)
    {
      EXPECT_EQ(TokenVisits[TokenIndex], 1u) << "token " << TokenIndex << " is not owned exactly once";
      EXPECT_EQ(TokenOrder[TokenIndex], TokenIndex) << "token DFS order differs at position " << TokenIndex;
    }

    std::string Rebuilt;
    for (std::size_t TokenIndex : TokenOrder)
    {
      Rebuilt.append(File.lexedFile().raw(File.lexedFile().tokens()[TokenIndex]));
    }
    EXPECT_EQ(Rebuilt, File.lexedFile().source());
    EXPECT_EQ(RootMeasurement.TokenCount, File.lexedFile().tokens().size());
    EXPECT_EQ(RootMeasurement.TextLength, File.lexedFile().source().size());
    EXPECT_EQ(File.span(Tree.root()), (core::SourceRange{0, File.lexedFile().source().size()}));
  }
} // namespace ink::parser::test

#endif
