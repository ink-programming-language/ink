#ifndef INK_SEMANTIC_TYPE_H
#define INK_SEMANTIC_TYPE_H

#include "ink/semantic/value.h"

#include <type_traits>

namespace ink::semantic
{
  class SemanticContext;
  class TypeDecl;
  class BuiltinType;
  class UserDefinedType;
  class IntegerType;
  class FloatType;
  class ArrayType;
  class SliceType;
  class PointerType;
  class ReferenceType;
  class ClassType;
  class EnumType;
  class InterfaceType;

  enum class TypeKind : std::uint8_t
  {
#define INK_SEMANTIC_TYPE(Name, Base) Name,
#include "ink/semantic/Types.def"
#undef INK_SEMANTIC_TYPE
  };

  // Access through a pointer/reference/slice, independent of binding mutability.
  enum class AccessKind : std::uint8_t
  {
    ReadOnly,
    ReadWrite,
  };

  class Type : public Value
  {
    public:
      TypeKind typeKind() const noexcept
      {
        return Kind;
      }

      const Type &type() const noexcept final
      {
        return *MetaType;
      }

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Type;
      }

    private:
      // Only the context's metatype supplies null and is its own value type.
      Type(SemanticContext &Context, TypeKind Kind, const Type *MetaType) noexcept
          : Value(ValueKind::Type),
            Context(Context),
            Kind(Kind),
            MetaType(MetaType ? MetaType : this)
      {
      }

      const SemanticContext &Context;
      TypeKind Kind;
      const Type *MetaType;

      friend class BuiltinType;
      friend class UserDefinedType;
  };

  // Includes language-defined type constructors, even for user-defined elements.
  class BuiltinType : public Type
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        if (!Type::classof(ValueObject))
        {
          return false;
        }
        switch (static_cast<const Type *>(ValueObject)->typeKind())
        {
#define INK_SEMANTIC_TYPE(Name, Base) \
  case TypeKind::Name:                \
    return std::is_same_v<Base, BuiltinType>;
#include "ink/semantic/Types.def"
#undef INK_SEMANTIC_TYPE
        default:
          return false;
        }
      }

    private:
      BuiltinType(SemanticContext &Context, TypeKind Kind, const Type *MetaType) noexcept
          : Type(Context, Kind, MetaType)
      {
      }

      friend class SemanticContext;
      friend class IntegerType;
      friend class FloatType;
      friend class ArrayType;
      friend class SliceType;
      friend class PointerType;
      friend class ReferenceType;
  };

  // Nominal identity is the declaration, never just its interned name.
  class UserDefinedType : public Type
  {
    public:
      const TypeDecl &declaration() const noexcept
      {
        return Declaration;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        if (!Type::classof(ValueObject))
        {
          return false;
        }
        switch (static_cast<const Type *>(ValueObject)->typeKind())
        {
#define INK_SEMANTIC_TYPE(Name, Base) \
  case TypeKind::Name:                \
    return std::is_same_v<Base, UserDefinedType>;
#include "ink/semantic/Types.def"
#undef INK_SEMANTIC_TYPE
        default:
          return false;
        }
      }

    private:
      UserDefinedType(SemanticContext &Context, TypeKind Kind, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : Type(Context, Kind, &MetaType),
            Declaration(Declaration)
      {
      }

      const TypeDecl &Declaration;

      friend class ClassType;
      friend class EnumType;
      friend class InterfaceType;
  };

  class IntegerType final : public BuiltinType
  {
    public:
      std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      bool isSigned() const noexcept
      {
        return Signed;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Integer;
      }

    private:
      IntegerType(SemanticContext &Context, const Type &MetaType, std::uint32_t BitWidth, bool Signed) noexcept
          : BuiltinType(Context, TypeKind::Integer, &MetaType),
            BitWidth(BitWidth),
            Signed(Signed)
      {
      }

      std::uint32_t BitWidth;
      bool Signed;

      friend class SemanticContext;
  };

  // IEEE binary16, binary32 or binary64; never a host C++ floating-point type.
  class FloatType final : public BuiltinType
  {
    public:
      std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Float;
      }

    private:
      FloatType(SemanticContext &Context, const Type &MetaType, std::uint32_t BitWidth) noexcept
          : BuiltinType(Context, TypeKind::Float, &MetaType),
            BitWidth(BitWidth)
      {
      }

      std::uint32_t BitWidth;

      friend class SemanticContext;
  };

  class ArrayType final : public BuiltinType
  {
    public:
      const Type &elementType() const noexcept
      {
        return ElementType;
      }

      std::uint64_t elementCount() const noexcept
      {
        return ElementCount;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Array;
      }

    private:
      ArrayType(SemanticContext &Context, const Type &MetaType, const Type &ElementType, std::uint64_t ElementCount) noexcept
          : BuiltinType(Context, TypeKind::Array, &MetaType),
            ElementType(ElementType),
            ElementCount(ElementCount)
      {
      }

      const Type &ElementType;
      std::uint64_t ElementCount;

      friend class SemanticContext;
  };

  // A view with runtime length. Ownership/allocation belongs to a container value.
  class SliceType final : public BuiltinType
  {
    public:
      const Type &elementType() const noexcept
      {
        return ElementType;
      }

      AccessKind access() const noexcept
      {
        return Access;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Slice;
      }

    private:
      SliceType(SemanticContext &Context, const Type &MetaType, const Type &ElementType, AccessKind Access) noexcept
          : BuiltinType(Context, TypeKind::Slice, &MetaType),
            ElementType(ElementType),
            Access(Access)
      {
      }

      const Type &ElementType;
      AccessKind Access;

      friend class SemanticContext;
  };

  class PointerType final : public BuiltinType
  {
    public:
      const Type &pointeeType() const noexcept
      {
        return PointeeType;
      }

      AccessKind access() const noexcept
      {
        return Access;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Pointer;
      }

    private:
      PointerType(SemanticContext &Context, const Type &MetaType, const Type &PointeeType, AccessKind Access) noexcept
          : BuiltinType(Context, TypeKind::Pointer, &MetaType),
            PointeeType(PointeeType),
            Access(Access)
      {
      }

      const Type &PointeeType;
      AccessKind Access;

      friend class SemanticContext;
  };

  // A reference type is distinct from an expression's place/value category.
  class ReferenceType final : public BuiltinType
  {
    public:
      const Type &referentType() const noexcept
      {
        return ReferentType;
      }

      AccessKind access() const noexcept
      {
        return Access;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Reference;
      }

    private:
      ReferenceType(SemanticContext &Context, const Type &MetaType, const Type &ReferentType, AccessKind Access) noexcept
          : BuiltinType(Context, TypeKind::Reference, &MetaType),
            ReferentType(ReferentType),
            Access(Access)
      {
      }

      const Type &ReferentType;
      AccessKind Access;

      friend class SemanticContext;
  };

  class ClassType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Class;
      }

    private:
      ClassType(SemanticContext &Context, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : UserDefinedType(Context, TypeKind::Class, MetaType, Declaration)
      {
      }

      friend class SemanticContext;
  };

  class EnumType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Enum;
      }

    private:
      EnumType(SemanticContext &Context, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : UserDefinedType(Context, TypeKind::Enum, MetaType, Declaration)
      {
      }

      friend class SemanticContext;
  };

  class InterfaceType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Interface;
      }

    private:
      InterfaceType(SemanticContext &Context, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : UserDefinedType(Context, TypeKind::Interface, MetaType, Declaration)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
