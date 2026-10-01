#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <charconv>

namespace ink::semantic
{
  using namespace ink::ir;

  namespace
  {
    const Type *builtinType(TypePool &Types, std::string_view Text)
    {
      if (Text == "void")
      {
        return &Types.getType<TypeKind::Void>();
      }
      if (Text == "bool")
      {
        return &Types.getType<TypeKind::Bool>();
      }
      if (Text == "type")
      {
        return &Types.getType<TypeKind::Meta>();
      }
      if (Text.size() < 2 || Text[1] == '0')
      {
        return nullptr;
      }
      std::uint32_t Width = 0;
      const auto Parsed = std::from_chars(Text.data() + 1, Text.data() + Text.size(), Width);
      if (Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size())
      {
        return nullptr;
      }
      if ((Text[0] == 'i' || Text[0] == 'u') && (Width == 8 || Width == 16 || Width == 32 || Width == 64 || Width == 128))
      {
        return Types.getType<TypeKind::Integer>(Width, Text[0] == 'i');
      }
      return Text[0] == 'f' ? Types.getType<TypeKind::Float>(Width) : nullptr;
    }
  } // namespace

  const Type *Analyzer::analyzeType(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    if (Depth >= State.TypeDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return nullptr;
    }
    if (Node.isComptime())
    {
      reportUnsupported(State, Node);
      return nullptr;
    }
    TypePool &Types = State.Context.typePool();
    if (parser::NameExpr::classof(&Node))
    {
      const auto NameToken = static_cast<const parser::NameExpr &>(Node).name();
      const Name TypeName = State.Context.namePool().find(NameToken.Text);
      if (const auto *Binding = State.Resolver.lookup(TypeName))
      {
        if (Binding->targets().size() == 1 && Type::classof(Binding->targets().front()))
        {
          return static_cast<const Type *>(Binding->targets().front());
        }
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "type", "value");
        return nullptr;
      }
      if (State.Resolver.lookup<Decl *>(TypeName))
      {
        reportUnsupported(State, Node);
        return nullptr;
      }
      if (const Type *Result = builtinType(Types, NameToken.Text))
      {
        return Result;
      }
      State.report<core::DiagnosticKind::SemanticUnknownType>(Node.getSourceRange(), NameToken.Text);
      return nullptr;
    }
    if (parser::ParenExpr::classof(&Node))
    {
      return analyzeType(State, *static_cast<const parser::ParenExpr &>(Node).expression(), Depth + 1);
    }
    if (parser::UnaryExpr::classof(&Node))
    {
      const auto &Unary = static_cast<const parser::UnaryExpr &>(Node);
      if (Unary.op() != tokenizer::TokenKind::Star && Unary.op() != tokenizer::TokenKind::Amp)
      {
        reportUnsupported(State, Node);
        return nullptr;
      }
      const Type *Target = analyzeType(State, *Unary.operand(), Depth + 1);
      if (!Target)
      {
        return nullptr;
      }
      if (Target->typeKind() == TypeKind::Meta || (Unary.op() == tokenizer::TokenKind::Amp && Target->typeKind() == TypeKind::Void))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Unary.operand()->getSourceRange(), "runtime pointee type", Target->typeKind() == TypeKind::Meta ? "type" : "void");
        return nullptr;
      }
      // The unqualified indirection syntax currently grants read/write access.
      if (Unary.op() == tokenizer::TokenKind::Star)
      {
        return Types.getType<TypeKind::Pointer>(*Target, AccessKind::ReadWrite);
      }
      return Types.getType<TypeKind::Reference>(*Target, AccessKind::ReadWrite);
    }
    reportUnsupported(State, Node);
    return nullptr;
  }
} // namespace ink::semantic
