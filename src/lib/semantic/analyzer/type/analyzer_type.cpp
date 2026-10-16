#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"

#include <charconv>
#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace ink::semantic
{
  namespace
  {
    const ir::Value *singleTarget(AnalyzerState &State, const frontend::parser::Expr &Node, const Binding &Found)
    {
      if (Found.isOverloadSet() || Found.targets().size() != 1)
      {
        State.semanticContext().report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a type or module", "an overload set");
        return nullptr;
      }
      return Found.targets().front();
    }

    const ir::Value *analyzeTypeExpr(AnalyzerState &State, const frontend::parser::Expr &Node);

    const ir::Value *analyzeNameTypeExpr(AnalyzerState &State, const frontend::parser::NameExpr &Node)
    {
      auto &Context = State.semanticContext();
      const auto Name = Node.name();
      if (const Binding *Found = State.nameResolver().lookup(core::NamePool::instance().find(Name.Text)))
      {
        return singleTarget(State, Node, *Found);
      }
      if (const ir::Type *Builtin = lookupBuiltinType(Context.typePool(), Name.Text))
      {
        return Builtin;
      }
      Context.report<core::DiagnosticKind::SemanticUnknownType>(Name.Range, Name.Text);
      return nullptr;
    }

    const ir::Value *analyzeParenTypeExpr(AnalyzerState &State, const frontend::parser::ParenExpr &Node)
    {
      return analyzeTypeExpr(State, *Node.expression());
    }

    const ir::Value *analyzeMemberTypeExpr(AnalyzerState &State, const frontend::parser::MemberExpr &Node)
    {
      auto &Context = State.semanticContext();
      if (Node.access() != frontend::parser::TokenKind::Dot)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a type member accessed with '.'", "a value member access");
        return nullptr;
      }
      const ir::Value *Owner = analyzeTypeExpr(State, *Node.object());
      if (!Owner)
      {
        return nullptr;
      }
      if (ir::Module::classof(Owner))
      {
        if (&static_cast<const ir::Module *>(Owner)->irContext() != &Context.irContext())
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.object()->getSourceRange(), "a module in the current IR context", "a foreign module");
          return nullptr;
        }
      }
      else if (!requireType(Context, Node.object()->getSourceRange(), Owner))
      {
        return nullptr;
      }
      const auto Name = Node.member();
      const Binding *Found = State.nameResolver().lookupMember(*Owner, core::NamePool::instance().find(Name.Text));
      if (!Found)
      {
        Context.report<core::DiagnosticKind::SemanticUnknownType>(Name.Range, Name.Text);
        return nullptr;
      }
      return singleTarget(State, Node, *Found);
    }

    const ir::Type *analyzeUnaryTypeExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node)
    {
      auto &Context = State.semanticContext();
      using namespace frontend::parser;
      if (Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a supported type expression", astKindName(Node.getKind()));
        return nullptr;
      }
      const ir::Type *Operand = requireType(Context, Node.operand()->getSourceRange(), analyzeTypeExpr(State, *Node.operand()));
      if (!Operand)
      {
        return nullptr;
      }
      auto &Types = Context.typePool();
      return Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.getPointerType(Operand)) : Types.getReferenceType(Operand);
    }

    const ir::Type *analyzeArrayTypeExpr(AnalyzerState &State, const frontend::parser::ArrayRepeatExpr &Node)
    {
      auto &Context = State.semanticContext();
      using namespace frontend::parser;
      const ir::Type *Element = requireType(Context, Node.value()->getSourceRange(), analyzeTypeExpr(State, *Node.value()));
      if (!Element)
      {
        return nullptr;
      }
      if (!Element->hasLayout())
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "an array element with runtime layout", ir::valueKindName(Element->kind()));
        return nullptr;
      }
      // TODO: Evaluate compile-time integer expressions for the element count; currently only integer literals are accepted.
      if (Node.count()->getKind() != ASTKind::LiteralExpr || static_cast<const LiteralExpr *>(Node.count())->literalKind() != TokenKind::IntegerLiteral)
      {
        Context.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Node.count()->getSourceRange());
        return nullptr;
      }
      const auto &Literal = static_cast<const LiteralExpr &>(*Node.count());
      const auto &Input = State.input().Unit->input();
      const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Input.token(Literal.token()).Payload);
      if (!Numeric || (Numeric->Base != 2 && Numeric->Base != 8 && Numeric->Base != 10 && Numeric->Base != 16))
      {
        Context.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Literal.getSourceRange());
        return nullptr;
      }
      std::string_view Text = Input.spelling(Literal.token());
      if (Numeric->Base != 10)
      {
        Text.remove_prefix(2);
      }
      std::uint64_t Count = 0;
      const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Count, static_cast<int>(Numeric->Base));
      if (Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Literal.getSourceRange());
        return nullptr;
      }
      const ir::Type *Result = Context.typePool().getArrayType(Element, Count);
      if (!Result)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "an array with representable size and valid alignment", "an invalid array layout");
      }
      return Result;
    }

    const ir::Type *analyzeFunctionTypeExpr(AnalyzerState &State, const frontend::parser::FunctionTypeExpr &Node)
    {
      auto &Context = State.semanticContext();
      const auto &ReturnExpr = *Node.returnType()->expression();
      const ir::Type *Return = requireType(Context, ReturnExpr.getSourceRange(), analyzeTypeExpr(State, ReturnExpr));
      if (!Return)
      {
        return nullptr;
      }
      std::vector<ir::FunctionTypeParameter> Parameters;
      Parameters.reserve(Node.parameters().size());
      for (const auto &Parameter : Node.parameters())
      {
        const auto &ParameterExpr = *Parameter.type()->expression();
        const ir::Type *Type = requireType(Context, ParameterExpr.getSourceRange(), analyzeTypeExpr(State, ParameterExpr));
        if (!Type)
        {
          return nullptr;
        }
        Parameters.emplace_back(Type, Parameter.variadic());
      }
      const ir::Type *Result = Context.typePool().getFunctionType(Return, Parameters);
      if (!Result)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a function signature with runtime layouts", "a non-runtime signature type");
      }
      return Result;
    }

    const ir::Value *analyzeTypeExpr(AnalyzerState &State, const frontend::parser::Expr &Node)
    {
      auto &Context = State.semanticContext();
      if (State.recursionDepth() >= State.recursionDepthLimit())
      {
        Context.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
        return nullptr;
      }
      BlockDepthGuard DepthGuard(State.recursionDepth());
      // TODO: Honor Node.isComptime() and evaluate marked type expressions at compile time.
      using namespace frontend::parser;
      switch (Node.getKind())
      {
        case ASTKind::NameExpr:
          return analyzeNameTypeExpr(State, static_cast<const NameExpr &>(Node));
        case ASTKind::ParenExpr:
          return analyzeParenTypeExpr(State, static_cast<const ParenExpr &>(Node));
        case ASTKind::MemberExpr:
          return analyzeMemberTypeExpr(State, static_cast<const MemberExpr &>(Node));
        case ASTKind::UnaryExpr:
          return analyzeUnaryTypeExpr(State, static_cast<const UnaryExpr &>(Node));
        case ASTKind::ArrayRepeatExpr:
          return analyzeArrayTypeExpr(State, static_cast<const ArrayRepeatExpr &>(Node));
        case ASTKind::FunctionTypeExpr:
          return analyzeFunctionTypeExpr(State, static_cast<const FunctionTypeExpr &>(Node));
        default:
          // TODO: Analyze generic type applications (GenericApplyExpr, e.g. Foo::[i32]).
          // TODO: Evaluate type-valued calls, conditionals, match/do blocks, indexing and tuples.
          break;
      }
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a supported type expression", astKindName(Node.getKind()));
      return nullptr;
    }
  } // namespace

  const ir::Type *Analyzer::analyzeType(AnalyzerState &State, const frontend::parser::TypeSyntax &Node)
  {
    const auto &Expression = *Node.expression();
    return requireType(State.semanticContext(), Expression.getSourceRange(), analyzeTypeExpr(State, Expression));
  }
} // namespace ink::semantic
