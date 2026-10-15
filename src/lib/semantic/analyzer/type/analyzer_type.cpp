#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace ink::semantic
{
  namespace
  {
    const ir::Type *builtinType(ir::TypePool &Types, std::string_view Name)
    {
      struct Entry
      {
          std::string_view Name;
          const ir::Type *Type;
      };
      const Entry Entries[] = {
          {"void", Types.getVoidType()},
          {"bool", Types.getBoolType()},
          {"u8", Types.getUInt8Type()},
          {"i32", Types.getInt32Type()},
          {"u32", Types.getUInt32Type()},
          {"i64", Types.getInt64Type()},
          {"u64", Types.getUInt64Type()},
          {"f32", Types.getFloat32Type()},
          {"f64", Types.getFloat64Type()},
      };
      for (const auto &Candidate : Entries)
      {
        if (Candidate.Name == Name)
        {
          return Candidate.Type;
        }
      }
      return nullptr;
    }

    const ir::Value *singleTarget(SemanticContext &Context, const frontend::parser::Expr &Node, const Binding &Found)
    {
      if (Found.isOverloadSet() || Found.targets().size() != 1)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a type or module", "an overload set");
        return nullptr;
      }
      return Found.targets().front();
    }

    const ir::Type *requireType(SemanticContext &Context, const frontend::parser::Expr &Node, const ir::Value *Target)
    {
      if (!Target)
      {
        return nullptr;
      }
      if (!ir::Type::classof(Target))
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a type", ir::valueKindName(Target->kind()));
        return nullptr;
      }
      const auto *Type = static_cast<const ir::Type *>(Target);
      if (!Context.irContext().containsType(Type))
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a type in the current IR context", "a foreign or unregistered type");
        return nullptr;
      }
      return Type;
    }

    const ir::Value *analyzeTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::Expr &Node);

    const ir::Value *analyzeNameTypeExpr(SemanticContext &Context, const NameResolver &Resolver, const frontend::parser::NameExpr &Node)
    {
      const auto Name = Node.name();
      if (const Binding *Found = Resolver.lookup(core::NamePool::instance().find(Name.Text)))
      {
        return singleTarget(Context, Node, *Found);
      }
      if (const ir::Type *Builtin = builtinType(Context.typePool(), Name.Text))
      {
        return Builtin;
      }
      Context.report<core::DiagnosticKind::SemanticUnknownType>(Name.Range, Name.Text);
      return nullptr;
    }

    const ir::Value *analyzeParenTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::ParenExpr &Node)
    {
      return analyzeTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, *Node.expression());
    }

    const ir::Value *analyzeMemberTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::MemberExpr &Node)
    {
      if (Node.access() != frontend::parser::TokenKind::Dot)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a type member accessed with '.'", "a value member access");
        return nullptr;
      }
      const ir::Value *Owner = analyzeTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, *Node.object());
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
      else if (!requireType(Context, *Node.object(), Owner))
      {
        return nullptr;
      }
      const auto Name = Node.member();
      const Binding *Found = Resolver.lookupMember(*Owner, core::NamePool::instance().find(Name.Text));
      if (!Found)
      {
        Context.report<core::DiagnosticKind::SemanticUnknownType>(Name.Range, Name.Text);
        return nullptr;
      }
      return singleTarget(Context, Node, *Found);
    }

    const ir::Type *analyzeUnaryTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::UnaryExpr &Node)
    {
      using namespace frontend::parser;
      if (Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a supported type expression", astKindName(Node.getKind()));
        return nullptr;
      }
      const ir::Type *Operand = requireType(Context, *Node.operand(), analyzeTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, *Node.operand()));
      if (!Operand)
      {
        return nullptr;
      }
      auto &Types = Context.typePool();
      return Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.getPointerType(Operand)) : Types.getReferenceType(Operand);
    }

    const ir::Type *analyzeArrayTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::ArrayRepeatExpr &Node)
    {
      using namespace frontend::parser;
      const ir::Type *Element = requireType(Context, *Node.value(), analyzeTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, *Node.value()));
      if (!Element)
      {
        return nullptr;
      }
      if (!Element->hasLayout())
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "an array element with runtime layout", ir::valueKindName(Element->kind()));
        return nullptr;
      }
      if (Node.count()->getKind() != ASTKind::LiteralExpr || static_cast<const LiteralExpr *>(Node.count())->literalKind() != TokenKind::IntegerLiteral)
      {
        Context.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Node.count()->getSourceRange());
        return nullptr;
      }
      const auto &Literal = static_cast<const LiteralExpr &>(*Node.count());
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

    const ir::Type *analyzeFunctionTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::FunctionTypeExpr &Node)
    {
      const auto &ReturnExpr = *Node.returnType()->expression();
      const ir::Type *Return = requireType(Context, ReturnExpr, analyzeTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, ReturnExpr));
      if (!Return)
      {
        return nullptr;
      }
      std::vector<ir::FunctionTypeParameter> Parameters;
      Parameters.reserve(Node.parameters().size());
      for (const auto &Parameter : Node.parameters())
      {
        const auto &ParameterExpr = *Parameter.type()->expression();
        const ir::Type *Type = requireType(Context, ParameterExpr, analyzeTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, ParameterExpr));
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

    const ir::Value *analyzeTypeExpr(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const NameResolver &Resolver, std::size_t &RecursionDepth, std::size_t RecursionDepthLimit, const frontend::parser::Expr &Node)
    {
      if (RecursionDepth >= RecursionDepthLimit)
      {
        Context.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
        return nullptr;
      }
      BlockDepthGuard DepthGuard(RecursionDepth);
      using namespace frontend::parser;
      switch (Node.getKind())
      {
        case ASTKind::NameExpr:
          return analyzeNameTypeExpr(Context, Resolver, static_cast<const NameExpr &>(Node));
        case ASTKind::ParenExpr:
          return analyzeParenTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, static_cast<const ParenExpr &>(Node));
        case ASTKind::MemberExpr:
          return analyzeMemberTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, static_cast<const MemberExpr &>(Node));
        case ASTKind::UnaryExpr:
          return analyzeUnaryTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, static_cast<const UnaryExpr &>(Node));
        case ASTKind::ArrayRepeatExpr:
          return analyzeArrayTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, static_cast<const ArrayRepeatExpr &>(Node));
        case ASTKind::FunctionTypeExpr:
          return analyzeFunctionTypeExpr(Context, Input, Resolver, RecursionDepth, RecursionDepthLimit, static_cast<const FunctionTypeExpr &>(Node));
        default:
          break;
      }
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a supported type expression", astKindName(Node.getKind()));
      return nullptr;
    }
  } // namespace

  const ir::Type *Analyzer::analyzeType(AnalyzerState &State, const frontend::parser::TypeSyntax &Node)
  {
    const auto &Expression = *Node.expression();
    return requireType(State.Context, Expression, analyzeTypeExpr(State.Context, State.Input.Unit->input(), State.Resolver, State.RecursionDepth, State.RecursionDepthLimit, Expression));
  }
} // namespace ink::semantic
