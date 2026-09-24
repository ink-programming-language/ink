#include "ink/semantic/model/context.h"

#include "ink/semantic/model/class_type.h"
#include "ink/semantic/model/enum_type.h"
#include "ink/semantic/model/interface_type.h"

#include <llvm/ADT/Hashing.h>

#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  namespace
  {
    struct ArrayTypeKey
    {
        const Type *ElementType;
        std::uint64_t ElementCount;

        bool operator==(const ArrayTypeKey &) const noexcept = default;
    };

    struct ArrayTypeKeyHash
    {
        std::size_t operator()(const ArrayTypeKey &Key) const noexcept
        {
          return llvm::hash_combine(Key.ElementType, Key.ElementCount);
        }
    };

    struct AccessTypeKey
    {
        const Type *TargetType;
        AccessKind Access;

        bool operator==(const AccessTypeKey &) const noexcept = default;
    };

    struct AccessTypeKeyHash
    {
        std::size_t operator()(const AccessTypeKey &Key) const noexcept
        {
          return llvm::hash_combine(Key.TargetType, static_cast<std::uint8_t>(Key.Access));
        }
    };

    bool isValidAccess(AccessKind Access) noexcept
    {
      return Access == AccessKind::ReadOnly || Access == AccessKind::ReadWrite;
    }
  } // namespace

  class SemanticContext::Impl
  {
    public:
      std::unique_ptr<BuiltinType> MetaType;
      std::unique_ptr<BuiltinType> VoidType;
      std::unique_ptr<BuiltinType> BoolType;
      // Model destructors never traverse borrowed declaration/type edges.
      std::vector<std::unique_ptr<Decl>> Declarations;
      std::unordered_map<std::uint64_t, std::unique_ptr<IntegerType>> IntegerTypes;
      std::unordered_map<std::uint32_t, std::unique_ptr<FloatType>> FloatTypes;
      std::unordered_map<const TypeDecl *, std::unique_ptr<UserDefinedType>> UserDefinedTypes;
      std::unordered_map<ArrayTypeKey, std::unique_ptr<ArrayType>, ArrayTypeKeyHash> ArrayTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<SliceType>, AccessTypeKeyHash> SliceTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<PointerType>, AccessTypeKeyHash> PointerTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<ReferenceType>, AccessTypeKeyHash> ReferenceTypes;
      std::unordered_multimap<std::size_t, std::unique_ptr<IntegerConstant>> IntegerConstants;
      std::unique_ptr<BoolConstant> FalseValue;
      std::unique_ptr<BoolConstant> TrueValue;
      std::vector<std::unique_ptr<ExprValue>> ExpressionValues;
  };

  SemanticContext::SemanticContext(core::CompilationContext &Compilation)
      : Compilation(Compilation),
        Storage(std::make_unique<Impl>())
  {
    Storage->MetaType.reset(new BuiltinType(*this, TypeKind::Meta, nullptr));
    Storage->VoidType.reset(new BuiltinType(*this, TypeKind::Void, Storage->MetaType.get()));
    Storage->BoolType.reset(new BuiltinType(*this, TypeKind::Bool, Storage->MetaType.get()));
    Storage->FalseValue.reset(new BoolConstant(*Storage->BoolType, false));
    Storage->TrueValue.reset(new BoolConstant(*Storage->BoolType, true));
  }

  SemanticContext::~SemanticContext() = default;

  const BuiltinType &SemanticContext::getMetaType() const noexcept
  {
    return *Storage->MetaType;
  }

  const BuiltinType &SemanticContext::getVoidType() const noexcept
  {
    return *Storage->VoidType;
  }

  const BuiltinType &SemanticContext::getBoolType() const noexcept
  {
    return *Storage->BoolType;
  }

  const IntegerType *SemanticContext::getIntegerType(std::uint32_t BitWidth, bool Signed)
  {
    if (BitWidth == 0)
    {
      return nullptr;
    }
    const std::uint64_t Key = (static_cast<std::uint64_t>(BitWidth) << 1) | static_cast<std::uint64_t>(Signed);
    auto [Entry, Inserted] = Storage->IntegerTypes.try_emplace(Key);
    if (Inserted)
    {
      Entry->second.reset(new IntegerType(*this, getMetaType(), BitWidth, Signed));
    }
    return Entry->second.get();
  }

  const FloatType *SemanticContext::getFloatType(std::uint32_t BitWidth)
  {
    if (BitWidth != 16 && BitWidth != 32 && BitWidth != 64)
    {
      return nullptr;
    }
    auto [Entry, Inserted] = Storage->FloatTypes.try_emplace(BitWidth);
    if (Inserted)
    {
      Entry->second.reset(new FloatType(*this, getMetaType(), BitWidth));
    }
    return Entry->second.get();
  }

  const ArrayType *SemanticContext::getArrayType(const Type &ElementType, std::uint64_t ElementCount)
  {
    if (&ElementType.context() != this)
    {
      return nullptr;
    }
    auto [Entry, Inserted] = Storage->ArrayTypes.try_emplace(ArrayTypeKey{&ElementType, ElementCount});
    if (Inserted)
    {
      Entry->second.reset(new ArrayType(*this, getMetaType(), ElementType, ElementCount));
    }
    return Entry->second.get();
  }

  const SliceType *SemanticContext::getSliceType(const Type &ElementType, AccessKind Access)
  {
    if (&ElementType.context() != this || !isValidAccess(Access))
    {
      return nullptr;
    }
    auto [Entry, Inserted] = Storage->SliceTypes.try_emplace(AccessTypeKey{&ElementType, Access});
    if (Inserted)
    {
      Entry->second.reset(new SliceType(*this, getMetaType(), ElementType, Access));
    }
    return Entry->second.get();
  }

  const PointerType *SemanticContext::getPointerType(const Type &PointeeType, AccessKind Access)
  {
    if (&PointeeType.context() != this || !isValidAccess(Access))
    {
      return nullptr;
    }
    auto [Entry, Inserted] = Storage->PointerTypes.try_emplace(AccessTypeKey{&PointeeType, Access});
    if (Inserted)
    {
      Entry->second.reset(new PointerType(*this, getMetaType(), PointeeType, Access));
    }
    return Entry->second.get();
  }

  const ReferenceType *SemanticContext::getReferenceType(const Type &ReferentType, AccessKind Access)
  {
    if (&ReferentType.context() != this || !isValidAccess(Access))
    {
      return nullptr;
    }
    auto [Entry, Inserted] = Storage->ReferenceTypes.try_emplace(AccessTypeKey{&ReferentType, Access});
    if (Inserted)
    {
      Entry->second.reset(new ReferenceType(*this, getMetaType(), ReferentType, Access));
    }
    return Entry->second.get();
  }

  const UserDefinedType *SemanticContext::getUserDefinedType(const TypeDecl &Declaration)
  {
    if (&Declaration.context() != this)
    {
      return nullptr;
    }
    const auto Existing = Storage->UserDefinedTypes.find(&Declaration);
    if (Existing != Storage->UserDefinedTypes.end())
    {
      return Existing->second.get();
    }
    std::unique_ptr<UserDefinedType> Result;
    switch (Declaration.kind())
    {
    case DeclKind::Class:
      Result.reset(new ClassType(*this, getMetaType(), Declaration));
      break;
    case DeclKind::Enum:
      Result.reset(new EnumType(*this, getMetaType(), Declaration));
      break;
    case DeclKind::Interface:
      Result.reset(new InterfaceType(*this, getMetaType(), Declaration));
      break;
    default:
      return nullptr;
    }
    const UserDefinedType *Pointer = Result.get();
    Storage->UserDefinedTypes.emplace(&Declaration, std::move(Result));
    return Pointer;
  }

  const IntegerConstant *SemanticContext::getIntegerConstant(const IntegerType &ValueType, const llvm::APInt &Payload)
  {
    if (&ValueType.context() != this || ValueType.bitWidth() != Payload.getBitWidth())
    {
      return nullptr;
    }
    const std::size_t Hash = llvm::hash_combine(&ValueType, llvm::hash_value(Payload));
    const auto Candidates = Storage->IntegerConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const IntegerConstant &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && Candidate.value() == Payload)
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<IntegerConstant>(new IntegerConstant(ValueType, Payload));
    const IntegerConstant *Pointer = Result.get();
    Storage->IntegerConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const BoolConstant &SemanticContext::getBoolConstant(bool Payload) const noexcept
  {
    return Payload ? *Storage->TrueValue : *Storage->FalseValue;
  }

  const ExprValue *SemanticContext::createExprValue(const Type &ValueType, const parser::Expr &Expression, core::SourceId Source)
  {
    if (&ValueType.context() != this)
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ExprValue>(new ExprValue(ValueType, Expression, Source));
    const ExprValue *Pointer = Result.get();
    Storage->ExpressionValues.push_back(std::move(Result));
    return Pointer;
  }

  VarDecl *SemanticContext::createVarDecl(Name DeclName, BindingMutability Mutability, DeclSource Source, const Value *Initializer)
  {
    if (!Names.contains(DeclName) || (Mutability != BindingMutability::Mutable && Mutability != BindingMutability::Immutable) || (Initializer && &Initializer->type().context() != this))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<VarDecl>(new VarDecl(*this, DeclName, Mutability, Source, Initializer));
    VarDecl *Pointer = Result.get();
    Storage->Declarations.push_back(std::move(Result));
    return Pointer;
  }

  TypeDecl *SemanticContext::createTypeDecl(Name DeclName, DeclKind Kind, DeclSource Source)
  {
    if (!Names.contains(DeclName) || (Kind != DeclKind::Class && Kind != DeclKind::Enum && Kind != DeclKind::Interface))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<TypeDecl>(new TypeDecl(*this, Kind, DeclName, Source));
    TypeDecl *Pointer = Result.get();
    Storage->Declarations.push_back(std::move(Result));
    return Pointer;
  }
} // namespace ink::semantic
