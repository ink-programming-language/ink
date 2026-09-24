#include "ink/semantic/model/context.h"

#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/interface_type.h"

#include "hash.h"

#include <algorithm>
#include <functional>
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
          return combineHash(std::hash<const Type *>{}(Key.ElementType), std::hash<std::uint64_t>{}(Key.ElementCount));
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
          return combineHash(std::hash<const Type *>{}(Key.TargetType), static_cast<std::uint8_t>(Key.Access));
        }
    };

    bool isValidAccess(AccessKind Access) noexcept
    {
      return Access == AccessKind::ReadOnly || Access == AccessKind::ReadWrite;
    }

    std::size_t functionTypeHash(const Type &ReturnType, std::span<const Type *const> ParameterTypes) noexcept
    {
      std::size_t Hash = combineHash(std::hash<const Type *>{}(&ReturnType), ParameterTypes.size());
      for (const Type *ParameterType : ParameterTypes)
      {
        Hash = combineHash(Hash, std::hash<const Type *>{}(ParameterType));
      }
      return Hash;
    }

    bool matchesCallArguments(const FunctionType &Signature, std::span<const Value *const> Arguments) noexcept
    {
      const auto Parameters = Signature.parameterTypes();
      if (Arguments.size() != Parameters.size())
      {
        return false;
      }
      for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
      {
        if (!Arguments[Index] || &Arguments[Index]->type() != Parameters[Index])
        {
          return false;
        }
      }
      return true;
    }
  } // namespace

  class SemanticContext::Impl
  {
    public:
      std::unique_ptr<BuiltinType> MetaType;
      std::unique_ptr<BuiltinType> VoidType;
      std::unique_ptr<BuiltinType> BoolType;
      // Model destructors never traverse borrowed AST/type/value edges.
      std::vector<std::unique_ptr<Decl>> Declarations;
      std::vector<std::unique_ptr<Variable>> Variables;
      std::unordered_map<std::uint64_t, std::unique_ptr<IntegerType>> IntegerTypes;
      std::unordered_map<std::uint32_t, std::unique_ptr<FloatType>> FloatTypes;
      std::vector<std::unique_ptr<UserDefinedType>> UserDefinedTypes;
      std::unordered_map<ArrayTypeKey, std::unique_ptr<ArrayType>, ArrayTypeKeyHash> ArrayTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<SliceType>, AccessTypeKeyHash> SliceTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<PointerType>, AccessTypeKeyHash> PointerTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<ReferenceType>, AccessTypeKeyHash> ReferenceTypes;
      std::unordered_multimap<std::size_t, std::unique_ptr<FunctionType>> FunctionTypes;
      // Constants are destroyed before the types they reference.
      std::unique_ptr<ConstantPool> Constants;
      std::vector<std::unique_ptr<ExprValue>> ExpressionValues;
      std::vector<std::unique_ptr<Function>> Functions;
      std::vector<std::unique_ptr<CallInstruction>> Calls;
  };

  SemanticContext::SemanticContext(core::CompilationContext &Compilation)
      : Compilation(Compilation),
        Storage(std::make_unique<Impl>())
  {
    Storage->MetaType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Meta, nullptr));
    Storage->VoidType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Void, Storage->MetaType.get()));
    Storage->BoolType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Bool, Storage->MetaType.get()));
    Storage->Constants.reset(new ConstantPool(*this));
  }

  SemanticContext::~SemanticContext() = default;

  ConstantPool &SemanticContext::constantPool() noexcept
  {
    return *Storage->Constants;
  }

  const ConstantPool &SemanticContext::constantPool() const noexcept
  {
    return *Storage->Constants;
  }

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

  const FunctionType *SemanticContext::getFunctionType(const Type &ReturnType, std::span<const Type *const> ParameterTypes)
  {
    if (&ReturnType.context() != this)
    {
      return nullptr;
    }
    for (const Type *ParameterType : ParameterTypes)
    {
      if (!ParameterType || &ParameterType->context() != this)
      {
        return nullptr;
      }
    }
    const std::size_t Hash = functionTypeHash(ReturnType, ParameterTypes);
    const auto Candidates = Storage->FunctionTypes.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const FunctionType &Candidate = *Entry->second;
      if (&Candidate.returnType() == &ReturnType && std::equal(Candidate.parameterTypes().begin(), Candidate.parameterTypes().end(), ParameterTypes.begin(), ParameterTypes.end()))
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<FunctionType>(new FunctionType(*this, getMetaType(), ReturnType, ParameterTypes));
    const FunctionType *Pointer = Result.get();
    Storage->FunctionTypes.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const ClassType *SemanticContext::createClassType(Name TypeName)
  {
    if (!Names.contains(TypeName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ClassType>(new ClassType(*this, getMetaType(), TypeName));
    const ClassType *Pointer = Result.get();
    Storage->UserDefinedTypes.push_back(std::move(Result));
    return Pointer;
  }

  const EnumType *SemanticContext::createEnumType(Name TypeName)
  {
    if (!Names.contains(TypeName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<EnumType>(new EnumType(*this, getMetaType(), TypeName));
    const EnumType *Pointer = Result.get();
    Storage->UserDefinedTypes.push_back(std::move(Result));
    return Pointer;
  }

  const InterfaceType *SemanticContext::createInterfaceType(Name TypeName)
  {
    if (!Names.contains(TypeName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<InterfaceType>(new InterfaceType(*this, getMetaType(), TypeName));
    const InterfaceType *Pointer = Result.get();
    Storage->UserDefinedTypes.push_back(std::move(Result));
    return Pointer;
  }

  const IntegerConstant *SemanticContext::getIntegerConstant(const IntegerType &ValueType, const IntegerBits &Payload)
  {
    return constantPool().getIntegerConstant(ValueType, Payload);
  }

  const BoolConstant &SemanticContext::getBoolConstant(bool Payload) const noexcept
  {
    return constantPool().getBoolConstant(Payload);
  }

  const StringConst *SemanticContext::getStringConst(const SliceType &ValueType, std::string_view Payload)
  {
    return constantPool().getStringConst(ValueType, Payload);
  }

  const FloatConst *SemanticContext::getFloatConst(const FloatType &ValueType, const FloatBits &Payload)
  {
    return constantPool().getFloatConst(ValueType, Payload);
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

  const Function *SemanticContext::createFunction(Name FunctionName, const FunctionType &Signature)
  {
    if (!Names.contains(FunctionName) || &Signature.context() != this)
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<Function>(new Function(FunctionName, Signature));
    const Function *Pointer = Result.get();
    Storage->Functions.push_back(std::move(Result));
    return Pointer;
  }

  const CallInstruction *SemanticContext::createCallInstruction(const Value &Callee, std::span<const Value *const> Arguments)
  {
    if (&Callee.type().context() != this || !FunctionType::classof(&Callee.type()))
    {
      return nullptr;
    }
    const auto &Signature = static_cast<const FunctionType &>(Callee.type());
    if (!matchesCallArguments(Signature, Arguments))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<CallInstruction>(new CallInstruction(Callee, Arguments));
    const CallInstruction *Pointer = Result.get();
    Storage->Calls.push_back(std::move(Result));
    return Pointer;
  }

  Variable *SemanticContext::createVariable(Name VariableName, BindingMutability Mutability, const Value *Initializer)
  {
    if (!Names.contains(VariableName) || (Mutability != BindingMutability::Mutable && Mutability != BindingMutability::Immutable) || (Initializer && &Initializer->type().context() != this))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<Variable>(new Variable(*this, VariableName, Mutability, Initializer));
    Variable *Pointer = Result.get();
    Storage->Variables.push_back(std::move(Result));
    return Pointer;
  }

  const FunctionDecl *SemanticContext::createFunctionDecl(Name DeclName, const parser::FunctionDecl &AST)
  {
    if (!Names.contains(DeclName) || AST.genericParameters().empty())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<FunctionDecl>(new FunctionDecl(DeclName, AST));
    const FunctionDecl *Pointer = Result.get();
    Storage->Declarations.push_back(std::move(Result));
    return Pointer;
  }

  const ClassDecl *SemanticContext::createClassDecl(Name DeclName, const parser::ClassDecl &AST)
  {
    if (!Names.contains(DeclName) || AST.genericParameters().empty())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ClassDecl>(new ClassDecl(DeclName, AST));
    const ClassDecl *Pointer = Result.get();
    Storage->Declarations.push_back(std::move(Result));
    return Pointer;
  }
} // namespace ink::semantic
