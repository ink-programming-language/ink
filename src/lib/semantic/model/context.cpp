#include "ink/semantic/model/context.h"

#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/interface_type.h"

#include "hash.h"

#include <algorithm>
#include <functional>

namespace ink::semantic
{
  namespace
  {
    bool isValidAccess(AccessKind Access) noexcept
    {
      return Access == AccessKind::ReadOnly || Access == AccessKind::ReadWrite;
    }

    bool isMemoryValueType(const Type &ValueType) noexcept
    {
      const Type *ElementType = &ValueType;
      while (ArrayType::classof(ElementType))
      {
        ElementType = &static_cast<const ArrayType *>(ElementType)->elementType();
      }
      switch (ElementType->typeKind())
      {
      case TypeKind::Bool:
      case TypeKind::Integer:
      case TypeKind::Float:
      case TypeKind::Pointer:
      case TypeKind::Reference:
      case TypeKind::Slice:
        return true;
      default:
        return false;
      }
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

  std::size_t SemanticContext::ArrayTypeKeyHash::operator()(const ArrayTypeKey &Key) const noexcept
  {
    return combineHash(std::hash<const Type *>{}(Key.ElementType), std::hash<std::uint64_t>{}(Key.ElementCount));
  }

  std::size_t SemanticContext::AccessTypeKeyHash::operator()(const AccessTypeKey &Key) const noexcept
  {
    return combineHash(std::hash<const Type *>{}(Key.TargetType), static_cast<std::uint8_t>(Key.Access));
  }

  SemanticContext::SemanticContext(core::CompilationContext &Compilation)
      : Compilation(Compilation)
  {
    MetaType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Meta, nullptr));
    VoidType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Void, MetaType.get()));
    BoolType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Bool, MetaType.get()));
    LabelType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Label, MetaType.get()));
    ModuleType.reset(new BuiltinType(*this, ValueKind::BuiltinType, TypeKind::Module, MetaType.get()));
    Constants.reset(new ConstantPool(*this));
  }

  SemanticContext::~SemanticContext() = default;

  ConstantPool &SemanticContext::constantPool() noexcept
  {
    return *Constants;
  }

  const ConstantPool &SemanticContext::constantPool() const noexcept
  {
    return *Constants;
  }

  const BuiltinType &SemanticContext::getMetaType() const noexcept
  {
    return *MetaType;
  }

  const BuiltinType &SemanticContext::getVoidType() const noexcept
  {
    return *VoidType;
  }

  const BuiltinType &SemanticContext::getBoolType() const noexcept
  {
    return *BoolType;
  }

  const BuiltinType &SemanticContext::getLabelType() const noexcept
  {
    return *LabelType;
  }

  const BuiltinType &SemanticContext::getModuleType() const noexcept
  {
    return *ModuleType;
  }

  const IntegerType *SemanticContext::getIntegerType(std::uint32_t BitWidth, bool Signed)
  {
    if (BitWidth == 0)
    {
      return nullptr;
    }
    const std::uint64_t Key = (static_cast<std::uint64_t>(BitWidth) << 1) | static_cast<std::uint64_t>(Signed);
    auto [Entry, Inserted] = IntegerTypes.try_emplace(Key);
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
    auto [Entry, Inserted] = FloatTypes.try_emplace(BitWidth);
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
    auto [Entry, Inserted] = ArrayTypes.try_emplace(ArrayTypeKey{&ElementType, ElementCount});
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
    auto [Entry, Inserted] = SliceTypes.try_emplace(AccessTypeKey{&ElementType, Access});
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
    auto [Entry, Inserted] = PointerTypes.try_emplace(AccessTypeKey{&PointeeType, Access});
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
    auto [Entry, Inserted] = ReferenceTypes.try_emplace(AccessTypeKey{&ReferentType, Access});
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
    const auto Candidates = FunctionTypes.equal_range(Hash);
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
    FunctionTypes.emplace(Hash, std::move(Result));
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
    UserDefinedTypes.push_back(std::move(Result));
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
    UserDefinedTypes.push_back(std::move(Result));
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
    UserDefinedTypes.push_back(std::move(Result));
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

  const StringConstant *SemanticContext::getStringConstant(const SliceType &ValueType, std::string_view Payload)
  {
    return constantPool().getStringConstant(ValueType, Payload);
  }

  const FloatConstant *SemanticContext::getFloatConstant(const FloatType &ValueType, const FloatBits &Payload)
  {
    return constantPool().getFloatConstant(ValueType, Payload);
  }

  ExprValue *SemanticContext::createExprValue(const Type &ValueType, const parser::Expr &Expression, core::SourceId Source)
  {
    if (&ValueType.context() != this)
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ExprValue>(new ExprValue(ValueType, Expression, Source));
    ExprValue *Pointer = Result.get();
    ExpressionValues.push_back(std::move(Result));
    return Pointer;
  }

  Function *SemanticContext::createFunction(Name FunctionName, const FunctionType &Signature, std::span<const ParameterKind> ParameterKinds, std::span<const Name> ParameterNames)
  {
    if (!Names.contains(FunctionName) || &Signature.context() != this || (!ParameterKinds.empty() && ParameterKinds.size() != Signature.parameterTypes().size()) || (!ParameterNames.empty() && ParameterNames.size() != Signature.parameterTypes().size()))
    {
      return nullptr;
    }
    for (ParameterKind Kind : ParameterKinds)
    {
      if (Kind != ParameterKind::Positional && Kind != ParameterKind::Named && Kind != ParameterKind::Variadic)
      {
        return nullptr;
      }
    }
    for (Name ParameterName : ParameterNames)
    {
      if (ParameterName.valid() && !Names.contains(ParameterName))
      {
        return nullptr;
      }
    }
    auto Result = std::unique_ptr<Function>(new Function(FunctionName, Signature));
    Function *Pointer = Result.get();
    for (const Type *ParameterType : Signature.parameterTypes())
    {
      const std::size_t Index = Pointer->Parameters.size();
      const ParameterKind Kind = ParameterKinds.empty() ? ParameterKind::Positional : ParameterKinds[Index];
      const Name ParameterName = ParameterNames.empty() ? Name{} : ParameterNames[Index];
      auto Parameter = std::unique_ptr<FunctionParameter>(new FunctionParameter(ParameterName, *ParameterType, Index, Kind));
      Parameter->Outer = Pointer;
      Pointer->Parameters.push_back(Parameter.get());
      Parameters.push_back(std::move(Parameter));
    }
    Functions.push_back(std::move(Result));
    return Pointer;
  }

  BasicBlock *SemanticContext::createFunctionBody(Function &FunctionValue)
  {
    if (&FunctionValue.context() != this || FunctionValue.hasBody())
    {
      return nullptr;
    }
    return createBasicBlock(FunctionValue);
  }

  BasicBlock *SemanticContext::createBasicBlock()
  {
    auto Result = std::unique_ptr<BasicBlock>(new BasicBlock(*this));
    BasicBlock *Pointer = Result.get();
    BasicBlocks.push_back(std::move(Result));
    return Pointer;
  }

  BasicBlock *SemanticContext::createBasicBlock(Function &FunctionValue)
  {
    if (&FunctionValue.context() != this)
    {
      return nullptr;
    }
    BasicBlock *Block = createBasicBlock();
    FunctionValue.Blocks.push_back(Block);
    Block->Outer = &FunctionValue;
    return Block;
  }

  bool SemanticContext::appendValue(BasicBlock &Block, Value &Child)
  {
    if (&Block.context() != this || &Child.context() != this || Child.outer() || Type::classof(&Child) || Constant::classof(&Child))
    {
      return false;
    }
    if (ReturnInstruction::classof(&Child))
    {
      if (!Function::classof(Block.outer()))
      {
        return false;
      }
      const auto &FunctionValue = static_cast<const Function &>(*Block.outer());
      const Type &ReturnType = FunctionValue.type().returnType();
      const Value *ReturnedValue = static_cast<const ReturnInstruction &>(Child).returnedValue();
      if (ReturnType.typeKind() == TypeKind::Void ? ReturnedValue != nullptr : (!ReturnedValue || &ReturnedValue->type() != &ReturnType))
      {
        return false;
      }
    }
    for (const Value *Ancestor = &Block; Ancestor; Ancestor = Ancestor->outer())
    {
      if (Ancestor == &Child)
      {
        return false;
      }
    }
    Block.Values.push_back(&Child);
    Child.Outer = &Block;
    return true;
  }

  bool SemanticContext::removeValue(BasicBlock &Block, Value &Child) noexcept
  {
    if (&Block.context() != this || &Child.context() != this || Child.outer() != &Block)
    {
      return false;
    }
    const auto Position = std::find(Block.Values.begin(), Block.Values.end(), &Child);
    if (Position == Block.Values.end())
    {
      return false;
    }
    Block.Values.erase(Position);
    Child.Outer = nullptr;
    return true;
  }

  Module *SemanticContext::createModule(Name ModuleName)
  {
    if (!Names.contains(ModuleName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<Module>(new Module(*this, ModuleName, *createBasicBlock()));
    Module *Pointer = Result.get();
    Modules.push_back(std::move(Result));
    Pointer->EntryBlock.Outer = Pointer;
    return Pointer;
  }

  CallInstruction *SemanticContext::createCallInstruction(const Value &Callee, std::span<const Value *const> Arguments)
  {
    if (&Callee.context() != this || !FunctionType::classof(&Callee.type()))
    {
      return nullptr;
    }
    const auto &Signature = static_cast<const FunctionType &>(Callee.type());
    if (!matchesCallArguments(Signature, Arguments))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<CallInstruction>(new CallInstruction(Callee, Arguments));
    CallInstruction *Pointer = Result.get();
    Calls.push_back(std::move(Result));
    return Pointer;
  }

  AllocaInstruction *SemanticContext::createAllocaInstruction(const Type &AllocatedType)
  {
    if (&AllocatedType.context() != this || !isMemoryValueType(AllocatedType))
    {
      return nullptr;
    }
    const PointerType *ResultType = getPointerType(AllocatedType, AccessKind::ReadWrite);
    auto Result = std::unique_ptr<AllocaInstruction>(new AllocaInstruction(*ResultType));
    AllocaInstruction *Pointer = Result.get();
    Allocations.push_back(std::move(Result));
    return Pointer;
  }

  LoadInstruction *SemanticContext::createLoadInstruction(const Value &Address)
  {
    if (&Address.context() != this || !PointerType::classof(&Address.type()))
    {
      return nullptr;
    }
    const auto &AddressType = static_cast<const PointerType &>(Address.type());
    if (!isMemoryValueType(AddressType.pointeeType()))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<LoadInstruction>(new LoadInstruction(Address));
    LoadInstruction *Pointer = Result.get();
    Loads.push_back(std::move(Result));
    return Pointer;
  }

  StoreInstruction *SemanticContext::createStoreInstruction(const Value &Address, const Value &StoredValue)
  {
    if (&Address.context() != this || &StoredValue.context() != this || !PointerType::classof(&Address.type()))
    {
      return nullptr;
    }
    const auto &AddressType = static_cast<const PointerType &>(Address.type());
    if (AddressType.access() != AccessKind::ReadWrite || !isMemoryValueType(AddressType.pointeeType()) || &AddressType.pointeeType() != &StoredValue.type())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<StoreInstruction>(new StoreInstruction(Address, StoredValue));
    StoreInstruction *Pointer = Result.get();
    Stores.push_back(std::move(Result));
    return Pointer;
  }

  AddInstruction *SemanticContext::createAddInstruction(const Value &Left, const Value &Right)
  {
    if (&Left.context() != this || &Right.context() != this || !IntegerType::classof(&Left.type()) || &Left.type() != &Right.type())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<AddInstruction>(new AddInstruction(Left, Right));
    AddInstruction *Pointer = Result.get();
    Additions.push_back(std::move(Result));
    return Pointer;
  }

  ReturnInstruction *SemanticContext::createReturnInstruction(const Value *ReturnedValue)
  {
    if (ReturnedValue && (&ReturnedValue->context() != this || ReturnedValue->type().typeKind() == TypeKind::Void))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ReturnInstruction>(new ReturnInstruction(*this, ReturnedValue));
    ReturnInstruction *Pointer = Result.get();
    Returns.push_back(std::move(Result));
    return Pointer;
  }

  ModuleDecl *SemanticContext::createModuleDecl(Name DeclName, const parser::ModuleAST &AST)
  {
    if (!Names.contains(DeclName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ModuleDecl>(new ModuleDecl(DeclName, AST));
    ModuleDecl *Pointer = Result.get();
    Declarations.push_back(std::move(Result));
    return Pointer;
  }

  FunctionDecl *SemanticContext::createFunctionDecl(Name DeclName, const parser::FunctionDecl &AST)
  {
    if (!Names.contains(DeclName) || AST.genericParameters().empty())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<FunctionDecl>(new FunctionDecl(DeclName, AST));
    FunctionDecl *Pointer = Result.get();
    Declarations.push_back(std::move(Result));
    return Pointer;
  }

  ClassDecl *SemanticContext::createClassDecl(Name DeclName, const parser::ClassDecl &AST)
  {
    if (!Names.contains(DeclName) || AST.genericParameters().empty())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ClassDecl>(new ClassDecl(DeclName, AST));
    ClassDecl *Pointer = Result.get();
    Declarations.push_back(std::move(Result));
    return Pointer;
  }
} // namespace ink::semantic
