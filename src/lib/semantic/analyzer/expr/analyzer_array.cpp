#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/execution/runtime/runtime_value.h"

#include <algorithm>
#include <vector>

namespace ink::semantic
{
  using namespace ir;

  namespace
  {
    bool isFloatLiteral(const parser::Expr &Node, std::size_t Limit)
    {
      const parser::Expr *Expression = &Node;
      for (std::size_t Depth = 0; Depth < Limit; ++Depth)
      {
        if (parser::ParenExpr::classof(Expression))
        {
          Expression = static_cast<const parser::ParenExpr &>(*Expression).expression();
        }
        else if (parser::UnaryExpr::classof(Expression) && (static_cast<const parser::UnaryExpr &>(*Expression).op() == tokenizer::TokenKind::Minus || static_cast<const parser::UnaryExpr &>(*Expression).op() == tokenizer::TokenKind::Plus))
        {
          Expression = static_cast<const parser::UnaryExpr &>(*Expression).operand();
        }
        else
        {
          return parser::LiteralExpr::classof(Expression) && static_cast<const parser::LiteralExpr &>(*Expression).literalKind() == tokenizer::TokenKind::FloatLiteral;
        }
      }
      return false;
    }

    std::optional<std::uint64_t> nonnegativeInteger(const IntegerConstant &Value)
    {
      const auto &Type = static_cast<const IntegerType &>(Value.type());
      const auto Words = Value.value().words();
      if ((Type.isSigned() && (Words.back() & (std::uint64_t{1} << ((Type.bitWidth() - 1) % 64)))) || std::any_of(Words.begin() + 1, Words.end(), [](std::uint64_t Word)
                                                                                                                  {
                                                                                                                    return Word != 0;
                                                                                                                  }))
      {
        return std::nullopt;
      }
      return Words.front();
    }

    bool hasMutableArrayRoot(const parser::Expr &Node, NameResolver &Resolver, SemanticContext &Context, std::size_t Limit)
    {
      const parser::Expr *Root = &Node;
      for (std::size_t Depth = 0; Depth < Limit; ++Depth)
      {
        if (parser::ParenExpr::classof(Root))
        {
          Root = static_cast<const parser::ParenExpr &>(*Root).expression();
        }
        else if (parser::IndexExpr::classof(Root))
        {
          Root = static_cast<const parser::IndexExpr &>(*Root).object();
        }
        else if (parser::MemberExpr::classof(Root))
        {
          const auto &Member = static_cast<const parser::MemberExpr &>(*Root);
          if (Member.access() == tokenizer::TokenKind::Arrow)
          {
            return true;
          }
          Root = Member.object();
        }
        else if (parser::UnaryExpr::classof(Root))
        {
          return static_cast<const parser::UnaryExpr &>(*Root).op() == tokenizer::TokenKind::Star;
        }
        else if (parser::NameExpr::classof(Root))
        {
          const auto Name = static_cast<const parser::NameExpr &>(*Root).name();
          if (Name.Text == "this")
          {
            return true;
          }
          const auto *Binding = Resolver.lookup(Context.namePool().find(Name.Text));
          if (!Binding || Binding->targets().size() != 1)
          {
            return false;
          }
          const auto &Execution = Context.comptimeState();
          const auto Found = Execution.Variables.find(Binding->targets().front());
          return Found != Execution.Variables.end() && !Found->second.Constant && !Found->second.Comptime;
        }
        else
        {
          return false;
        }
      }
      return false;
    }
  } // namespace

  std::optional<std::uint64_t> Analyzer::analyzeArrayLength(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    if (isFloatLiteral(Node, State.ExpressionDepthLimit))
    {
      State.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Node.getSourceRange());
      return std::nullopt;
    }
    const Type *Integer = State.Context.typePool().getType<TypeKind::Integer>(128, true);
    AnalysisState::EvaluationGuard Guard(State, true, Integer);
    const ExpressionResult Count = analyzeExpr(State, Node, Depth);
    if (!Count)
    {
      return std::nullopt;
    }
    const auto Length = Count.ValueObject && IntegerConstant::classof(Count.ValueObject) ? nonnegativeInteger(static_cast<const IntegerConstant &>(*Count.ValueObject)) : std::nullopt;
    if (!Length)
    {
      State.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Node.getSourceRange());
      return std::nullopt;
    }
    // Bound materialization work before building host vectors, including zero-size elements.
    if (*Length > core::ConfigManager::getSize<core::ConfigKind::ExecutionMaxStorageBytes>() / sizeof(execution::RuntimeValue))
    {
      State.report<core::DiagnosticKind::SemanticArrayTooLarge>(Node.getSourceRange());
      return std::nullopt;
    }
    return Length;
  }

  bool Analyzer::checkArrayMaterialization(AnalysisState &State, const Type &Element, std::uint64_t Count, const parser::ASTNodeBase &Node)
  {
    const std::size_t Limit = core::ConfigManager::getSize<core::ConfigKind::ExecutionMaxStorageBytes>();
    std::vector<std::uint64_t> Counts{Count};
    const Type *Nested = &Element;
    while (ArrayType::classof(Nested))
    {
      const auto &Array = static_cast<const ArrayType &>(*Nested);
      Counts.push_back(Array.elementCount());
      Nested = &Array.elementType();
    }
    std::size_t Size = 0;
    for (auto Iterator = Counts.rbegin(); Iterator != Counts.rend(); ++Iterator)
    {
      if (*Iterator == 0)
      {
        Size = 0;
        continue;
      }
      if (Size > Limit || sizeof(execution::RuntimeValue) > Limit - Size || *Iterator > Limit / (Size + sizeof(execution::RuntimeValue)))
      {
        State.report<core::DiagnosticKind::SemanticArrayTooLarge>(Node.getSourceRange());
        return false;
      }
      Size = static_cast<std::size_t>(*Iterator) * (Size + sizeof(execution::RuntimeValue));
    }
    return true;
  }

  Analyzer::ExpressionResult Analyzer::analyzeArrayExpr(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    const auto *Expected = State.ExpectedType && ArrayType::classof(State.ExpectedType) ? static_cast<const ArrayType *>(State.ExpectedType) : nullptr;
    const bool Repeated = parser::ArrayRepeatExpr::classof(&Node);
    std::vector<const parser::Expr *> Nodes;
    std::uint64_t Count = 0;
    if (Repeated)
    {
      const auto &Repeat = static_cast<const parser::ArrayRepeatExpr &>(Node);
      const auto Length = analyzeArrayLength(State, *Repeat.count(), Depth + 1);
      if (!Length)
      {
        return {};
      }
      Count = *Length;
      Nodes.push_back(Repeat.value());
    }
    else
    {
      for (const parser::Expr *Element : static_cast<const parser::ArrayExpr &>(Node).elements())
      {
        Nodes.push_back(Element);
      }
      Count = Nodes.size();
    }
    if (Expected && Count != Expected->elementCount())
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), describeType(*Expected), "array of length " + std::to_string(Count));
      return {};
    }
    const Type *ElementType = Expected ? &Expected->elementType() : nullptr;
    if (!ElementType && Nodes.empty())
    {
      State.report<core::DiagnosticKind::SemanticEmptyArrayNeedsType>(Node.getSourceRange());
      return {};
    }
    std::vector<ExpressionResult> Analyzed;
    Analyzed.reserve(Nodes.size());
    for (const parser::Expr *Element : Nodes)
    {
      const bool DeferredLiteral = State.Evaluating && !ElementType && findDeferredIntegerLiteral(*Element, Depth + 1, State.ExpressionDepthLimit);
      if (DeferredLiteral && !prepareIntegerLiteral(State, *Element, Depth + 1))
      {
        return {};
      }
      AnalysisState::EvaluationGuard Guard(State, State.Evaluating && !DeferredLiteral, ElementType);
      const ExpressionResult Value = analyzeExpr(State, *Element, Depth + 1);
      if (!Value)
      {
        return {};
      }
      if (!ElementType && Value.ValueObject)
      {
        ElementType = &Value.ValueObject->type();
      }
      Analyzed.push_back(Value);
    }
    if (!ElementType)
    {
      ElementType = State.Context.typePool().getType<TypeKind::Integer>(32, true);
    }
    if (ElementType->typeKind() != TypeKind::Bool && ElementType->typeKind() != TypeKind::Integer && ElementType->typeKind() != TypeKind::Float && ElementType->typeKind() != TypeKind::Pointer && ElementType->typeKind() != TypeKind::Reference && ElementType->typeKind() != TypeKind::Slice && ElementType->typeKind() != TypeKind::Array && ElementType->typeKind() != TypeKind::Class)
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "array element value type", describeType(*ElementType));
      return {};
    }
    if (!checkArrayMaterialization(State, *ElementType, Count, Node))
    {
      return {};
    }
    std::vector<const Value *> Elements;
    std::vector<const Constant *> Constants;
    for (std::size_t Index = 0; Index < Analyzed.size(); ++Index)
    {
      const Value *Element = convertExpression(State, Analyzed[Index], *ElementType, *Nodes[Index]);
      if (!Element)
      {
        return {};
      }
      Elements.push_back(Element);
      if (Constant::classof(Element))
      {
        Constants.push_back(static_cast<const Constant *>(Element));
      }
    }
    const ArrayType *Type = State.Context.typePool().getType<TypeKind::Array>(*ElementType, Count);
    if (Count)
    {
      for (const auto &Element : Analyzed)
      {
        takeTemporary(State, Element.TemporaryAddress);
      }
    }
    if (Constants.size() == Elements.size())
    {
      if (Repeated)
      {
        const Constant *Element = Constants.front();
        Constants.assign(static_cast<std::size_t>(Count), Element);
      }
      return trackTemporary(State, State.Context.constantPool().getArrayConstant(*Type, Constants), Node);
    }
    if (State.Evaluating)
    {
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    return trackTemporary(State, State.Builder.createArrayInstruction(*Type, Elements, Repeated), Node);
  }

  const Value *Analyzer::analyzeArrayIndex(AnalysisState &State, const parser::Expr &Node, std::uint64_t Count, std::size_t Depth)
  {
    if (isFloatLiteral(Node, State.ExpressionDepthLimit))
    {
      State.report<core::DiagnosticKind::SemanticInvalidArrayIndex>(Node.getSourceRange());
      return nullptr;
    }
    AnalysisState::EvaluationGuard Guard(State, State.Evaluating, nullptr);
    const ExpressionResult Index = analyzeExpr(State, Node, Depth);
    if (!Index)
    {
      return nullptr;
    }
    const Value *Value = Index.IntegerLiteral ? convertExpression(State, Index, *State.Context.typePool().getType<TypeKind::Integer>(128, true), Node) : Index.ValueObject;
    if (!Value || !IntegerType::classof(&Value->type()))
    {
      State.report<core::DiagnosticKind::SemanticInvalidArrayIndex>(Node.getSourceRange());
      return nullptr;
    }
    if (IntegerConstant::classof(Value))
    {
      const auto Offset = nonnegativeInteger(static_cast<const IntegerConstant &>(*Value));
      if (!Offset || *Offset >= Count)
      {
        State.report<core::DiagnosticKind::SemanticArrayIndexOutOfBounds>(Node.getSourceRange(), Count);
        return nullptr;
      }
    }
    return Value;
  }

  Analyzer::ExpressionResult Analyzer::analyzeIndexExpr(AnalysisState &State, const parser::IndexExpr &Node, std::size_t Depth)
  {
    if (Node.optional())
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "array index", "optional index");
      return {};
    }
    AnalysisState::EvaluationGuard Guard(State, State.Evaluating, nullptr);
    // Read mutable storage after the index has been evaluated, preserving aliases and side effects.
    if (!State.Evaluating && hasMutableArrayRoot(*Node.object(), State.Resolver, State.Context, State.ExpressionDepthLimit))
    {
      const Value *Address = resolveAddress(State, Node, Depth + 1);
      return {Address ? State.Builder.createLoadInstruction(*Address) : nullptr};
    }
    const ExpressionResult Object = analyzeExpr(State, *Node.object(), Depth + 1);
    if (!Object)
    {
      return {};
    }
    if (!Object.ValueObject || !ArrayType::classof(&Object.ValueObject->type()))
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.object()->getSourceRange(), "array", Object.ValueObject ? describeType(Object.ValueObject->type()) : "non-array value");
      return {};
    }
    const auto &Type = static_cast<const ArrayType &>(Object.ValueObject->type());
    const Value *Index = analyzeArrayIndex(State, *Node.index(), Type.elementCount(), Depth + 1);
    if (!Index)
    {
      return {};
    }
    if (ArrayConstant::classof(Object.ValueObject) && IntegerConstant::classof(Index))
    {
      return {static_cast<const ArrayConstant &>(*Object.ValueObject).elements()[static_cast<std::size_t>(*nonnegativeInteger(static_cast<const IntegerConstant &>(*Index)))]};
    }
    if (State.Evaluating)
    {
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    return {State.Builder.createArrayExtractInstruction(*Object.ValueObject, *Index)};
  }
} // namespace ink::semantic
