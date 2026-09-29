#include "analyzer_internal.h"

#include "ink/parser/ast.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace ink::semantic
{
  using namespace ink::ir;

  namespace
  {
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
  } // namespace

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
    case TypeKind::Function:
      return "function";
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
    State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), describeType(Target), Expression.IntegerLiteral ? "integer literal" : describeType(Expression.ValueObject->type()));
    return nullptr;
  }

  Analyzer::ExpressionResult Analyzer::analyzeExpr(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return {};
    }
    if (parser::ParenExpr::classof(&Node))
    {
      return analyzeExpr(State, *static_cast<const parser::ParenExpr &>(Node).expression(), Depth + 1);
    }
    if (parser::LiteralExpr::classof(&Node))
    {
      const auto &Literal = static_cast<const parser::LiteralExpr &>(Node);
      if (Literal.literalKind() == tokenizer::TokenKind::IntegerLiteral)
      {
        return {nullptr, &Literal};
      }
      if (Literal.literalKind() == tokenizer::TokenKind::StringLiteral)
      {
        const auto *String = std::get_if<tokenizer::StringInfo>(&State.Input.token(Literal.token()).Payload);
        if (!String)
        {
          State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
          return {};
        }
        const auto *U8 = State.Context.typePool().getType<TypeKind::Integer>(8, false);
        const auto *Slice = State.Context.typePool().getType<TypeKind::Slice>(*U8, AccessKind::ReadOnly);
        return {State.Context.constantPool().getStringConstant(*Slice, String->Decoded)};
      }
    }
    if (parser::NameExpr::classof(&Node))
    {
      const auto NameToken = static_cast<const parser::NameExpr &>(Node).name();
      const Name Symbol = State.Context.namePool().find(NameToken.Text);
      const auto *Binding = State.Resolver.lookup(Symbol);
      if (!Binding)
      {
        if (State.Resolver.lookup<Decl *>(Symbol))
        {
          reportUnsupported(State, Node);
          return {};
        }
        if (NameToken.Text == "true" || NameToken.Text == "false")
        {
          return {&State.Context.constantPool().getBoolConstant(NameToken.Text == "true")};
        }
        State.report<core::DiagnosticKind::SemanticUnknownName>(NameToken.Range, NameToken.Text);
        return {};
      }
      if (Binding->targets().size() != 1)
      {
        State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
        return {};
      }
      const Value *Result = Binding->targets().front();
      if (FunctionParameter::classof(Result) && &static_cast<const FunctionParameter &>(*Result).function() != State.CurrentFunction)
      {
        State.report<core::DiagnosticKind::SemanticInvalidCapture>(Node.getSourceRange(), NameToken.Text);
        return {};
      }
      return {Result};
    }
    if (parser::UnaryExpr::classof(&Node))
    {
      const auto &Unary = static_cast<const parser::UnaryExpr &>(Node);
      if (Unary.op() == tokenizer::TokenKind::Plus || Unary.op() == tokenizer::TokenKind::Minus)
      {
        ExpressionResult Result = analyzeExpr(State, *Unary.operand(), Depth + 1);
        if (!Result)
        {
          return {};
        }
        if (Result.IntegerLiteral)
        {
          Result.Negative = Result.Negative != (Unary.op() == tokenizer::TokenKind::Minus);
          return Result;
        }
      }
    }
    if (parser::CallExpr::classof(&Node))
    {
      return analyzeCallExpr(State, static_cast<const parser::CallExpr &>(Node), Depth);
    }
    reportUnsupported(State, Node);
    return {};
  }

  Analyzer::ExpressionResult Analyzer::analyzeCallExpr(AnalysisState &State, const parser::CallExpr &Node, std::size_t Depth)
  {
    if (Node.optional())
    {
      reportUnsupported(State, Node);
      return {};
    }
    const parser::Expr *CalleeNode = Node.callee();
    std::size_t CalleeDepth = Depth + 1;
    while (parser::ParenExpr::classof(CalleeNode) && CalleeDepth < State.ExpressionDepthLimit)
    {
      CalleeNode = static_cast<const parser::ParenExpr &>(*CalleeNode).expression();
      ++CalleeDepth;
    }
    if (CalleeDepth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(CalleeNode->getSourceRange());
      return {};
    }
    std::vector<const Value *> Candidates;
    if (parser::NameExpr::classof(CalleeNode))
    {
      const Name Symbol = State.Context.namePool().find(static_cast<const parser::NameExpr &>(*CalleeNode).name().Text);
      if (const auto *Binding = State.Resolver.lookup(Symbol); Binding && Binding->targets().size() > 1)
      {
        Candidates.assign(Binding->targets().begin(), Binding->targets().end());
      }
    }
    if (Candidates.empty())
    {
      const ExpressionResult Callee = analyzeExpr(State, *CalleeNode, CalleeDepth);
      if (!Callee)
      {
        return {};
      }
      if (!Callee.ValueObject || !FunctionType::classof(&Callee.ValueObject->type()))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(CalleeNode->getSourceRange(), "callable value", Callee.IntegerLiteral ? "integer literal" : describeType(Callee.ValueObject->type()));
        return {};
      }
      Candidates.push_back(Callee.ValueObject);
    }

    std::vector<ExpressionResult> Arguments;
    bool Succeeded = true;
    for (const auto &Argument : Node.arguments())
    {
      if (Argument.form() != parser::ArgumentKind::Positional)
      {
        State.report<core::DiagnosticKind::SemanticUnsupported>(Argument.range(), "named or spread call arguments");
        return {};
      }
      Arguments.push_back(analyzeExpr(State, *Argument.value(), Depth + 1));
      Succeeded = static_cast<bool>(Arguments.back()) && Succeeded;
    }
    if (!Succeeded)
    {
      return {};
    }

    const auto IsCFunction = [](const Value &Callee)
    {
      return Function::classof(&Callee) && static_cast<const Function &>(Callee).languageLinkage() == LanguageLinkage::C;
    };
    const Value *Selected = Candidates.front();
    if (Candidates.size() > 1)
    {
      struct CandidateMatch
      {
          const Value *Callee;
          std::vector<unsigned> Ranks;
      };
      std::vector<CandidateMatch> Matches;
      for (const Value *Candidate : Candidates)
      {
        const auto Parameters = static_cast<const FunctionType &>(Candidate->type()).parameterTypes();
        if (Parameters.size() != Arguments.size())
        {
          continue;
        }
        CandidateMatch Match{Candidate, {}};
        for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
        {
          const ExpressionResult &Argument = Arguments[Index];
          const Type &Parameter = *Parameters[Index];
          if (Argument.IntegerLiteral && IntegerType::classof(&Parameter))
          {
            const auto &Integer = static_cast<const IntegerType &>(Parameter);
            if (!integerBits(State.Input, *Argument.IntegerLiteral, Argument.Negative, Integer))
            {
              break;
            }
            Match.Ranks.push_back(Integer.isSigned() && Integer.bitWidth() == 32 ? 0U : 1U);
          }
          else if (Argument.ValueObject && &Argument.ValueObject->type() == &Parameter)
          {
            Match.Ranks.push_back(0);
          }
          else if (Argument.ValueObject && acceptsCString(*Argument.ValueObject, Parameter, IsCFunction(*Candidate)))
          {
            Match.Ranks.push_back(1);
          }
          else
          {
            break;
          }
        }
        if (Match.Ranks.size() == Arguments.size())
        {
          Matches.push_back(std::move(Match));
        }
      }
      if (Matches.empty())
      {
        State.report<core::DiagnosticKind::SemanticNoMatchingOverload>(Node.getSourceRange());
        return {};
      }
      // A candidate must be no worse for every argument and better for at least one.
      Selected = nullptr;
      for (const CandidateMatch &Match : Matches)
      {
        const bool Dominated = std::any_of(Matches.begin(), Matches.end(), [&Match](const CandidateMatch &Other)
        {
          bool Better = false;
          for (std::size_t Index = 0; Index < Match.Ranks.size(); ++Index)
          {
            if (Other.Ranks[Index] > Match.Ranks[Index])
            {
              return false;
            }
            Better = Better || Other.Ranks[Index] < Match.Ranks[Index];
          }
          return Better;
        });
        if (!Dominated)
        {
          if (Selected)
          {
            State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
            return {};
          }
          Selected = Match.Callee;
        }
      }
    }
    const auto Parameters = static_cast<const FunctionType &>(Selected->type()).parameterTypes();
    if (Parameters.size() != Arguments.size())
    {
      State.report<core::DiagnosticKind::SemanticArgumentCount>(Node.getSourceRange(), Parameters.size(), Arguments.size());
      return {};
    }
    std::vector<const Value *> Converted;
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      const Value *Argument = convertExpression(State, Arguments[Index], *Parameters[Index], *Node.arguments()[Index].value(), IsCFunction(*Selected));
      if (!Argument)
      {
        return {};
      }
      Converted.push_back(Argument);
    }
    const Value *Result = State.Builder.createCallInstruction(*Selected, Converted);
    if (!Result)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
    }
    return {Result};
  }
} // namespace ink::semantic
