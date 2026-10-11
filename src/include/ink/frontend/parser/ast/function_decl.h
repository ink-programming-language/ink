#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include "ink/frontend/parser/ast/parameter.h"
#include "ink/core/function_binding.h"

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
      FunctionDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<Parameter> Parameters, TypeSyntax *ReturnType, FunctionBodyKind BodyKind, BlockStmt *Body, Expr *Linkage = nullptr, core::FunctionBinding NativeSymbol = core::FunctionBinding::Local)
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
      TypeSyntax *returnType() noexcept
      {
        return ReturnType;
      }
      const TypeSyntax *returnType() const noexcept
      {
        return ReturnType;
      }
      FunctionBodyKind bodyKind() const noexcept
      {
        return BodyKind;
      }
      core::FunctionBinding nativeSymbolKind() const noexcept
      {
        return NativeSymbol;
      }
      // Null without native import/export; a string literal, or MissingExpr during recovery, otherwise.
      Expr *linkage() noexcept
      {
        return Linkage;
      }
      const Expr *linkage() const noexcept
      {
        return Linkage;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
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
      core::FunctionBinding NativeSymbol;
  };
} // namespace ink::frontend::parser
