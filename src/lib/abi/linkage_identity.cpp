#include "ink/abi/linkage_identity.h"

#include <utility>

namespace ink::abi
{
  Type::Type() noexcept = default;

  Type::Type(TypeKind Kind) noexcept
      : KindValue(Kind)
  {
  }

  Type::~Type() = default;
  Type::Type(const Type &Other) = default;
  Type &Type::operator=(const Type &Other) = default;
  Type::Type(Type &&Other) noexcept
      : KindValue(Other.KindValue),
        ArrayLength(Other.ArrayLength),
        Elements(std::move(Other.Elements)),
        Nominal(std::move(Other.Nominal))
  {
    Other.KindValue = TypeKind::Void;
    Other.ArrayLength = 0;
    Other.Elements.clear();
    Other.Nominal.reset();
  }

  Type &Type::operator=(Type &&Other) noexcept
  {
    if (this == &Other)
    {
      return *this;
    }
    KindValue = Other.KindValue;
    ArrayLength = Other.ArrayLength;
    Elements = std::move(Other.Elements);
    Nominal = std::move(Other.Nominal);
    Other.KindValue = TypeKind::Void;
    Other.ArrayLength = 0;
    Other.Elements.clear();
    Other.Nominal.reset();
    return *this;
  }

  Type Type::voidType() noexcept
  {
    return Type(TypeKind::Void);
  }

  Type Type::boolType() noexcept
  {
    return Type(TypeKind::Bool);
  }

  Type Type::byteType() noexcept
  {
    return Type(TypeKind::Byte);
  }

  Type Type::i32Type() noexcept
  {
    return Type(TypeKind::I32);
  }

  Type Type::pointerSizeType() noexcept
  {
    return Type(TypeKind::PointerSize);
  }

  Type Type::f16Type() noexcept
  {
    return Type(TypeKind::F16);
  }

  Type Type::f32Type() noexcept
  {
    return Type(TypeKind::F32);
  }

  Type Type::f64Type() noexcept
  {
    return Type(TypeKind::F64);
  }

  Type Type::constType(Type Element)
  {
    Type Result(TypeKind::Const);
    Result.Elements.push_back(std::move(Element));
    return Result;
  }

  Type Type::pointerType(Type Element)
  {
    Type Result(TypeKind::Pointer);
    Result.Elements.push_back(std::move(Element));
    return Result;
  }

  Type Type::referenceType(Type Element)
  {
    Type Result(TypeKind::Reference);
    Result.Elements.push_back(std::move(Element));
    return Result;
  }

  Type Type::sliceType(Type Element)
  {
    Type Result(TypeKind::Slice);
    Result.Elements.push_back(std::move(Element));
    return Result;
  }

  Type Type::arrayType(std::uint64_t Length, Type Element)
  {
    Type Result(TypeKind::Array);
    Result.ArrayLength = Length;
    Result.Elements.push_back(std::move(Element));
    return Result;
  }

  Type Type::tupleType(std::vector<Type> Elements)
  {
    Type Result(TypeKind::Tuple);
    Result.Elements = std::move(Elements);
    return Result;
  }

  Type Type::nominalType(NominalIdentity Identity)
  {
    Type Result(TypeKind::Nominal);
    Result.Nominal = std::make_shared<const NominalIdentity>(std::move(Identity));
    return Result;
  }

  TypeKind Type::kind() const noexcept
  {
    return KindValue;
  }

  const Type *Type::elementType() const noexcept
  {
    switch (KindValue)
    {
      case TypeKind::Const:
      case TypeKind::Pointer:
      case TypeKind::Reference:
      case TypeKind::Slice:
      case TypeKind::Array:
        return Elements.empty() ? nullptr : &Elements.front();
      case TypeKind::Void:
      case TypeKind::Bool:
      case TypeKind::Byte:
      case TypeKind::I32:
      case TypeKind::PointerSize:
      case TypeKind::F16:
      case TypeKind::F32:
      case TypeKind::F64:
      case TypeKind::Tuple:
      case TypeKind::Nominal:
        return nullptr;
    }
    return nullptr;
  }

  std::uint64_t Type::arrayLength() const noexcept
  {
    return ArrayLength;
  }

  const std::vector<Type> &Type::elements() const noexcept
  {
    return Elements;
  }

  const NominalIdentity *Type::nominalIdentity() const noexcept
  {
    return Nominal.get();
  }

  bool operator==(const Type &Left, const Type &Right) noexcept
  {
    if (Left.kind() != Right.kind() || Left.arrayLength() != Right.arrayLength() || Left.elements() != Right.elements())
    {
      return false;
    }
    const NominalIdentity *LeftNominal = Left.nominalIdentity();
    const NominalIdentity *RightNominal = Right.nominalIdentity();
    if (LeftNominal == nullptr || RightNominal == nullptr)
    {
      return LeftNominal == RightNominal;
    }
    return *LeftNominal == *RightNominal;
  }

  bool operator!=(const Type &Left, const Type &Right) noexcept
  {
    return !(Left == Right);
  }

  ClosedArgument::ClosedArgument(ClosedArgumentKind Kind, Type ArgumentType, std::string CanonicalBits)
      : KindValue(Kind),
        ArgumentType(std::move(ArgumentType)),
        CanonicalBits(std::move(CanonicalBits))
  {
  }

  ClosedArgument ClosedArgument::type(Type ArgumentType)
  {
    return ClosedArgument(ClosedArgumentKind::Type, std::move(ArgumentType), {});
  }

  ClosedArgument ClosedArgument::value(Type ArgumentType, std::string CanonicalBits)
  {
    return ClosedArgument(ClosedArgumentKind::Value, std::move(ArgumentType), std::move(CanonicalBits));
  }

  ClosedArgumentKind ClosedArgument::kind() const noexcept
  {
    return KindValue;
  }

  const Type &ClosedArgument::argumentType() const noexcept
  {
    return ArgumentType;
  }

  const std::string &ClosedArgument::canonicalBits() const noexcept
  {
    return CanonicalBits;
  }

  bool operator==(const ClosedArgument &Left, const ClosedArgument &Right) noexcept
  {
    return Left.kind() == Right.kind() && Left.argumentType() == Right.argumentType() && Left.canonicalBits() == Right.canonicalBits();
  }

  bool operator!=(const ClosedArgument &Left, const ClosedArgument &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const ClosedInstanceKey &Left, const ClosedInstanceKey &Right) noexcept
  {
    return Left.Arguments == Right.Arguments;
  }

  bool operator!=(const ClosedInstanceKey &Left, const ClosedInstanceKey &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const OwnerType &Left, const OwnerType &Right) noexcept
  {
    return Left.Name == Right.Name && Left.Instance == Right.Instance;
  }

  bool operator!=(const OwnerType &Left, const OwnerType &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const Scope &Left, const Scope &Right) noexcept
  {
    return Left.PackagePath == Right.PackagePath && Left.Module == Right.Module && Left.EnclosingTypes == Right.EnclosingTypes && Left.DeclarationName == Right.DeclarationName;
  }

  bool operator!=(const Scope &Left, const Scope &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const NominalIdentity &Left, const NominalIdentity &Right) noexcept
  {
    return Left.Path == Right.Path && Left.Instance == Right.Instance;
  }

  bool operator!=(const NominalIdentity &Left, const NominalIdentity &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const FunctionIdentity &Left, const FunctionIdentity &Right) noexcept
  {
    return Left.Kind == Right.Kind && Left.Path == Right.Path && Left.Instance == Right.Instance && Left.Parameters == Right.Parameters && Left.Result == Right.Result;
  }

  bool operator!=(const FunctionIdentity &Left, const FunctionIdentity &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const GlobalIdentity &Left, const GlobalIdentity &Right) noexcept
  {
    return Left.Path == Right.Path && Left.Mutability == Right.Mutability && Left.ValueType == Right.ValueType;
  }

  bool operator!=(const GlobalIdentity &Left, const GlobalIdentity &Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::abi
