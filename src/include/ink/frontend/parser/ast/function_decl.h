#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include "ink/frontend/parser/ast/parameter.h"
#include <utility>

namespace ink::frontend::parser
{
  class FunctionDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FunctionDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FunctionDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<Parameter> Parameters, TypeSyntax *ReturnType, FunctionBodyKind BodyKind, BlockStmt *Body, Expr *Linkage = nullptr, NativeSymbolKind NativeSymbol = NativeSymbolKind::None)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            GenericParameters(GenericParameters),
            Parameters(Parameters),
            ReturnType(ReturnType),
            BodyKind(BodyKind),
            Body(Body),
            Linkage(Linkage),
            NativeSymbol(NativeSymbol)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<Parameter> parameters() const noexcept
      {
        return Parameters;
      }
      auto *returnType(this auto &&Self) noexcept
      {
        return Self.ReturnType ? &std::forward_like<decltype(Self) &>(*Self.ReturnType) : nullptr;
      }
      FunctionBodyKind bodyKind() const noexcept
      {
        return BodyKind;
      }
      NativeSymbolKind nativeSymbolKind() const noexcept
      {
        return NativeSymbol;
      }
      // Null without native import/export; a string literal, or MissingExpr during recovery, otherwise.
      auto *linkage(this auto &&Self) noexcept
      {
        return Self.Linkage ? &std::forward_like<decltype(Self) &>(*Self.Linkage) : nullptr;
      }
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      ASTArray<Parameter> GenericParameters;
      ASTArray<Parameter> Parameters;
      TypeSyntax *ReturnType;
      FunctionBodyKind BodyKind;
      BlockStmt *Body;
      Expr *Linkage;
      NativeSymbolKind NativeSymbol;
  };
} // namespace ink::frontend::parser
