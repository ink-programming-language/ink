#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ink::semantic
{
  using namespace ink::ir;

  // Accumulate at the target width without host signed arithmetic or truncation.
  std::optional<IntegerBits> integerBits(const parser::TokenBuffer &Input, const parser::LiteralExpr &Literal, bool Negative, const IntegerType &Target)
  {
    const auto *Numeric = std::get_if<tokenizer::NumericInfo>(&Input.token(Literal.token()).Payload);
    if (!Numeric || (Numeric->Base != 2 && Numeric->Base != 8 && Numeric->Base != 10 && Numeric->Base != 16))
    {
      return std::nullopt;
    }
    std::string_view Digits = Input.spelling(Literal.token());
    if (Numeric->Base != 10)
    {
      Digits.remove_prefix(2);
    }
    const std::uint32_t Width = Target.bitWidth();
    std::vector<std::uint32_t> Limbs((Width + 31U) / 32U, 0);
    for (char Digit : Digits)
    {
      std::uint64_t Carry = Digit >= '0' && Digit <= '9' ? Digit - '0' : (Digit >= 'a' && Digit <= 'f' ? Digit - 'a' + 10 : Digit - 'A' + 10);
      if (Carry >= Numeric->Base)
      {
        return std::nullopt;
      }
      for (std::uint32_t &Limb : Limbs)
      {
        const std::uint64_t Product = static_cast<std::uint64_t>(Limb) * Numeric->Base + Carry;
        Limb = static_cast<std::uint32_t>(Product);
        Carry = Product >> 32U;
      }
      if (Carry || (Width % 32U && (Limbs.back() >> (Width % 32U))))
      {
        return std::nullopt;
      }
    }
    const bool Nonzero = std::any_of(Limbs.begin(), Limbs.end(), [](std::uint32_t Limb)
    {
      return Limb != 0;
    });
    if (Negative && Nonzero && !Target.isSigned())
    {
      return std::nullopt;
    }
    if (Target.isSigned())
    {
      const std::uint32_t SignBit = std::uint32_t{1} << ((Width - 1U) % 32U);
      if (Limbs.back() & SignBit)
      {
        const bool MinimumMagnitude = Limbs.back() == SignBit && std::all_of(Limbs.begin(), Limbs.end() - 1, [](std::uint32_t Limb)
        {
          return Limb == 0;
        });
        if (!Negative || !MinimumMagnitude)
        {
          return std::nullopt;
        }
      }
    }
    if (Negative)
    {
      std::uint64_t Carry = 1;
      for (std::uint32_t &Limb : Limbs)
      {
        const std::uint64_t Sum = static_cast<std::uint32_t>(~Limb) + Carry;
        Limb = static_cast<std::uint32_t>(Sum);
        Carry = Sum >> 32U;
      }
      if (Width % 32U)
      {
        Limbs.back() &= (std::uint32_t{1} << (Width % 32U)) - 1U;
      }
    }
    std::vector<std::uint64_t> Words((Width + 63U) / 64U, 0);
    for (std::size_t Index = 0; Index < Limbs.size(); ++Index)
    {
      Words[Index / 2] |= static_cast<std::uint64_t>(Limbs[Index]) << ((Index % 2) * 32U);
    }
    return IntegerBits(Width, Words);
  }

  bool acceptsCString(const Value &ValueObject, const Type &Target, bool CArgument)
  {
    if (!CArgument || !StringConstant::classof(&ValueObject) || !PointerType::classof(&Target))
    {
      return false;
    }
    const auto &Pointer = static_cast<const PointerType &>(Target);
    const auto &Slice = static_cast<const SliceType &>(ValueObject.type());
    return Pointer.access() == AccessKind::ReadWrite && &Pointer.pointeeType() == &Slice.elementType();
  }

  std::string describeType(const Type &ValueType)
  {
    switch (ValueType.typeKind())
    {
    case TypeKind::Void:
      return "void";
    case TypeKind::Bool:
      return "bool";
    case TypeKind::Integer:
    {
      const auto &Integer = static_cast<const IntegerType &>(ValueType);
      return std::string(Integer.isSigned() ? "i" : "u") + std::to_string(Integer.bitWidth());
    }
    case TypeKind::Float:
      return "f" + std::to_string(static_cast<const FloatType &>(ValueType).bitWidth());
    case TypeKind::Pointer:
      return "*" + describeType(static_cast<const PointerType &>(ValueType).pointeeType());
    case TypeKind::Reference:
      return "reference";
    case TypeKind::Slice:
      return "slice";
    case TypeKind::Array:
      return "[" + describeType(static_cast<const ArrayType &>(ValueType).elementType()) + "; " + std::to_string(static_cast<const ArrayType &>(ValueType).elementCount()) + "]";
    case TypeKind::Function:
      return "function";
    case TypeKind::Class:
      return std::string(static_cast<const ClassType &>(ValueType).identity());
    default:
      return "non-scalar value";
    }
  }

  const Value *Analyzer::convertExpression(AnalysisState &State, const ExpressionResult &Expression, const Type &Target, const parser::Expr &Node, bool CArgument)
  {
    if (Expression.IntegerLiteral && IntegerType::classof(&Target))
    {
      const auto &Integer = static_cast<const IntegerType &>(Target);
      const auto Bits = integerBits(State.Input, *Expression.IntegerLiteral, Expression.Negative, Integer);
      if (!Bits)
      {
        State.report<core::DiagnosticKind::SemanticIntegerOutOfRange>(Node.getSourceRange(), describeType(Target));
        return nullptr;
      }
      return State.Context.constantPool().getIntegerConstant(Integer, *Bits);
    }
    if (Expression.ValueObject)
    {
      if (&Expression.ValueObject->type() == &Target)
      {
        return Expression.ValueObject;
      }
      if (State.Evaluating && CArgument && StringConstant::classof(Expression.ValueObject) && PointerType::classof(&Target))
      {
        const Type &Pointee = static_cast<const PointerType &>(Target).pointeeType();
        if (Pointee.typeKind() == TypeKind::Void || (IntegerType::classof(&Pointee) && static_cast<const IntegerType &>(Pointee).bitWidth() == 8 && !static_cast<const IntegerType &>(Pointee).isSigned()))
        {
          // The libffi bridge copies the bytes into call-local native storage.
          // Preserve the constant here instead of constructing runtime IR.
          return Expression.ValueObject;
        }
      }
      if (acceptsCString(*Expression.ValueObject, Target, CArgument))
      {
        const auto &String = static_cast<const StringConstant &>(*Expression.ValueObject);
        if (!String.tryGetCString())
        {
          State.report<core::DiagnosticKind::SemanticEmbeddedNull>(Node.getSourceRange());
          return nullptr;
        }
        if (!State.CurrentFunction)
        {
          State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "C string argument inside a function", "module-level string argument");
          return nullptr;
        }
        auto *Result = State.Builder.createCStringInstruction(String);
        if (!Result)
        {
          State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        }
        return Result;
      }
    }
    State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), describeType(Target), Expression.IntegerLiteral ? "integer literal" : (Expression.ValueObject ? describeType(Expression.ValueObject->type()) : "void"));
    return nullptr;
  }
} // namespace ink::semantic
