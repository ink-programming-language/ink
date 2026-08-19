#ifndef INK_ABI_LINKAGE_IDENTITY_H
#define INK_ABI_LINKAGE_IDENTITY_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ink::abi
{
  struct NominalIdentity;

  enum class TypeKind : std::uint8_t
  {
    Void,
    Bool,
    Byte,
    I32,
    PointerSize,
    F16,
    F32,
    F64,
    Const,
    Pointer,
    Reference,
    Slice,
    Array,
    Tuple,
    Nominal,
  };

  class Type final
  {
    public:
      Type() noexcept;
      ~Type();
      Type(const Type &Other);
      Type &operator=(const Type &Other);
      Type(Type &&Other) noexcept;
      Type &operator=(Type &&Other) noexcept;

      static Type voidType() noexcept;
      static Type boolType() noexcept;
      static Type byteType() noexcept;
      static Type i32Type() noexcept;
      static Type pointerSizeType() noexcept;
      static Type f16Type() noexcept;
      static Type f32Type() noexcept;
      static Type f64Type() noexcept;
      static Type constType(Type Element);
      static Type pointerType(Type Element);
      static Type referenceType(Type Element);
      static Type sliceType(Type Element);
      static Type arrayType(std::uint64_t Length, Type Element);
      static Type tupleType(std::vector<Type> Elements);
      static Type nominalType(NominalIdentity Identity);

      TypeKind kind() const noexcept;
      const Type *elementType() const noexcept;
      std::uint64_t arrayLength() const noexcept;
      const std::vector<Type> &elements() const noexcept;
      const NominalIdentity *nominalIdentity() const noexcept;

    private:
      explicit Type(TypeKind Kind) noexcept;

      TypeKind KindValue = TypeKind::Void;
      std::uint64_t ArrayLength = 0;
      std::vector<Type> Elements;
      std::shared_ptr<const NominalIdentity> Nominal;
  };

  bool operator==(const Type &Left, const Type &Right) noexcept;
  bool operator!=(const Type &Left, const Type &Right) noexcept;

  enum class ClosedArgumentKind : std::uint8_t
  {
    Type,
    Value,
  };

  class ClosedArgument final
  {
    public:
      static ClosedArgument type(Type ArgumentType);
      static ClosedArgument value(Type ArgumentType, std::string CanonicalBits);

      ClosedArgumentKind kind() const noexcept;
      const Type &argumentType() const noexcept;
      const std::string &canonicalBits() const noexcept;

    private:
      ClosedArgument(ClosedArgumentKind Kind, Type ArgumentType, std::string CanonicalBits);

      ClosedArgumentKind KindValue;
      Type ArgumentType;
      std::string CanonicalBits;
  };

  bool operator==(const ClosedArgument &Left, const ClosedArgument &Right) noexcept;
  bool operator!=(const ClosedArgument &Left, const ClosedArgument &Right) noexcept;

  struct ClosedInstanceKey
  {
      std::vector<ClosedArgument> Arguments;
  };

  bool operator==(const ClosedInstanceKey &Left, const ClosedInstanceKey &Right) noexcept;
  bool operator!=(const ClosedInstanceKey &Left, const ClosedInstanceKey &Right) noexcept;

  struct OwnerType
  {
      std::string Name;
      ClosedInstanceKey Instance;
  };

  bool operator==(const OwnerType &Left, const OwnerType &Right) noexcept;
  bool operator!=(const OwnerType &Left, const OwnerType &Right) noexcept;

  struct Scope
  {
      std::vector<std::string> PackagePath;
      std::string Module;
      std::vector<OwnerType> EnclosingTypes;
      std::string DeclarationName;
  };

  bool operator==(const Scope &Left, const Scope &Right) noexcept;
  bool operator!=(const Scope &Left, const Scope &Right) noexcept;

  struct NominalIdentity
  {
      Scope Path;
      ClosedInstanceKey Instance;
  };

  bool operator==(const NominalIdentity &Left, const NominalIdentity &Right) noexcept;
  bool operator!=(const NominalIdentity &Left, const NominalIdentity &Right) noexcept;

  enum class FunctionKind : std::uint8_t
  {
    Function,
    Constructor,
    Destructor,
  };

  struct FunctionIdentity
  {
      FunctionKind Kind = FunctionKind::Function;
      Scope Path;
      ClosedInstanceKey Instance;
      std::vector<Type> Parameters;
      Type Result;
  };

  bool operator==(const FunctionIdentity &Left, const FunctionIdentity &Right) noexcept;
  bool operator!=(const FunctionIdentity &Left, const FunctionIdentity &Right) noexcept;

  enum class GlobalMutability : std::uint8_t
  {
    Mutable,
    Immutable,
  };

  struct GlobalIdentity
  {
      Scope Path;
      GlobalMutability Mutability = GlobalMutability::Mutable;
      Type ValueType;
  };

  bool operator==(const GlobalIdentity &Left, const GlobalIdentity &Right) noexcept;
  bool operator!=(const GlobalIdentity &Left, const GlobalIdentity &Right) noexcept;
} // namespace ink::abi

#endif
