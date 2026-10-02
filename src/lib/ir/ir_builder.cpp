#include "ink/ir/ir_builder.h"

#include <algorithm>

namespace ink::ir
{
  namespace
  {
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

  IRBuilder::IRBuilder(IRContext &Context) noexcept
      : Context(Context)
  {
  }

  IRBuilder::InsertPointGuard::InsertPointGuard(IRBuilder &Builder) noexcept
      : Builder(Builder),
        SavedPoint(Builder.saveInsertPoint())
  {
  }

  IRBuilder::InsertPointGuard::~InsertPointGuard()
  {
    // A removed or reparented anchor clears the point instead of restoring a stale location.
    Builder.restoreInsertPoint(SavedPoint);
  }

  bool IRBuilder::setInsertPoint(BasicBlock &Block, Value *Before) noexcept
  {
    return restoreInsertPoint({&Block, Before});
  }

  bool IRBuilder::setInsertPoint(Value &Before) noexcept
  {
    if (!BasicBlock::classof(Before.outer()))
    {
      clearInsertPoint();
      return false;
    }
    return setInsertPoint(static_cast<BasicBlock &>(*Before.outer()), &Before);
  }

  void IRBuilder::clearInsertPoint() noexcept
  {
    Point = {};
  }

  IRBuilder::InsertPoint IRBuilder::saveInsertPoint() const noexcept
  {
    return Point;
  }

  bool IRBuilder::restoreInsertPoint(InsertPoint SavedPoint) noexcept
  {
    if (SavedPoint.Block ? !isValidInsertPoint(*SavedPoint.Block, SavedPoint.Before) : SavedPoint.Before != nullptr)
    {
      clearInsertPoint();
      return false;
    }
    Point = SavedPoint;
    return true;
  }

  bool IRBuilder::isValidInsertPoint(const BasicBlock &Block, const Value *Before) const noexcept
  {
    if (&Block.context() != &Context)
    {
      return false;
    }
    // Check membership by address before inspecting the anchor; an erased anchor is never dereferenced.
    return !Before || std::any_of(Block.Values.begin(), Block.Values.end(), [Before](const auto &Child)
    {
      return Child.get() == Before;
    });
  }

  bool IRBuilder::canInsertAt(const BasicBlock &Block, const Value *Before, bool IsTerminator) const noexcept
  {
    if (!isValidInsertPoint(Block, Before) || (IsTerminator && Before))
    {
      return false;
    }
    return !Block.terminator() || (Before && !IsTerminator);
  }

  bool IRBuilder::canInsertReturn(const BasicBlock &Block, const Value *ReturnedValue, const Value *Before) const noexcept
  {
    if (!canInsertAt(Block, Before, true) || !Function::classof(Block.outer()))
    {
      return false;
    }
    const auto &FunctionValue = static_cast<const Function &>(*Block.outer());
    const Type &ReturnType = FunctionValue.functionType().returnType();
    return ReturnType.typeKind() == TypeKind::Void ? ReturnedValue == nullptr : (ReturnedValue && &ReturnedValue->context() == &Context && &ReturnedValue->type() == &ReturnType);
  }

  bool IRBuilder::isValidBranchTarget(const BasicBlock &Target) const noexcept
  {
    return &Target.context() == &Context && (!Target.outer() || Function::classof(Target.outer()));
  }

  bool IRBuilder::canInsertBranch(const BasicBlock &Block, const BasicBlock &Target, const Value *Before) const noexcept
  {
    return canInsertAt(Block, Before, true) && Function::classof(Block.outer()) && &Target.context() == &Context && Target.outer() == Block.outer();
  }

  bool IRBuilder::canInsertValue(const BasicBlock &Block, const Value &Child, const Value *Before) const noexcept
  {
    if (&Block.context() != &Context || &Child.context() != &Context || Child.outer() || Type::classof(&Child) || Constant::classof(&Child))
    {
      return false;
    }
    const bool IsReturn = ReturnInstruction::classof(&Child);
    if (CStringInstruction::classof(&Child) && !Function::classof(Block.outer()))
    {
      return false;
    }
    if (!canInsertAt(Block, Before, Child.isTerminator()))
    {
      return false;
    }
    if (IsReturn && !canInsertReturn(Block, static_cast<const ReturnInstruction &>(Child).returnedValue(), Before))
    {
      return false;
    }
    if (BranchInstruction::classof(&Child) && !canInsertBranch(Block, static_cast<const BranchInstruction &>(Child).target(), Before))
    {
      return false;
    }
    if (ConditionalBranchInstruction::classof(&Child))
    {
      const auto &Branch = static_cast<const ConditionalBranchInstruction &>(Child);
      if (!canInsertBranch(Block, Branch.trueTarget(), Before) || !canInsertBranch(Block, Branch.falseTarget(), Before))
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
    return true;
  }

  void IRBuilder::insertOwnedValue(BasicBlock &Block, std::unique_ptr<Value> Child, Value *Before)
  {
    const auto Position = Before ? std::find_if(Block.Values.begin(), Block.Values.end(), [Before](const auto &Entry)
    {
      return Entry.get() == Before;
    })
                                 : Block.Values.end();
    Child->Outer = &Block;
    Block.Values.insert(Position, std::move(Child));
  }

  std::unique_ptr<Value> IRBuilder::removeValue(BasicBlock &Block, Value &Child) noexcept
  {
    if (&Block.context() != &Context || &Child.context() != &Context || Child.outer() != &Block)
    {
      return nullptr;
    }
    const auto Position = std::find_if(Block.Values.begin(), Block.Values.end(), [&Child](const auto &Entry)
    {
      return Entry.get() == &Child;
    });
    if (Position == Block.Values.end())
    {
      return nullptr;
    }
    std::unique_ptr<Value> Result = std::move(*Position);
    Block.Values.erase(Position);
    Child.Outer = nullptr;
    return Result;
  }

  bool IRBuilder::eraseValue(BasicBlock &Block, Value &Child) noexcept
  {
    return static_cast<bool>(removeValue(Block, Child));
  }

  std::unique_ptr<Module> IRBuilder::removeModule(Module &ModuleValue) noexcept
  {
    const auto Position = std::find_if(Context.Modules.begin(), Context.Modules.end(), [&ModuleValue](const auto &Entry)
    {
      return Entry.get() == &ModuleValue;
    });
    if (Position == Context.Modules.end())
    {
      return nullptr;
    }
    std::unique_ptr<Module> Result = std::move(*Position);
    Context.Modules.erase(Position);
    return Result;
  }

  bool IRBuilder::eraseModule(Module &ModuleValue) noexcept
  {
    return static_cast<bool>(removeModule(ModuleValue));
  }

  const ClassType *IRBuilder::createClassType(Name TypeName)
  {
    return Context.typePool().createClassType(TypeName);
  }

  const EnumType *IRBuilder::createEnumType(Name TypeName)
  {
    return Context.typePool().createEnumType(TypeName);
  }

  const InterfaceType *IRBuilder::createInterfaceType(Name TypeName)
  {
    return Context.typePool().createInterfaceType(TypeName);
  }

  std::unique_ptr<Function> IRBuilder::createFunction(Name FunctionName, const FunctionType &Signature, std::span<const ParameterKind> ParameterKinds, std::span<const Name> ParameterNames, CallingConvention Convention, LanguageLinkage Linkage)
  {
    if (!Context.namePool().contains(FunctionName) || &Signature.context() != &Context || (!ParameterKinds.empty() && ParameterKinds.size() != Signature.parameterTypes().size()) || (!ParameterNames.empty() && ParameterNames.size() != Signature.parameterTypes().size()))
    {
      return nullptr;
    }
    if ((Convention != CallingConvention::C && Convention != CallingConvention::Fast && Convention != CallingConvention::Cold) || (Linkage != LanguageLinkage::Ink && Linkage != LanguageLinkage::C))
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
      if (ParameterName.valid() && !Context.namePool().contains(ParameterName))
      {
        return nullptr;
      }
    }
    auto Result = std::unique_ptr<Function>(new Function(FunctionName, Signature, Convention, Linkage));
    Function *Pointer = Result.get();
    for (const Type *ParameterType : Signature.parameterTypes())
    {
      const std::size_t Index = Pointer->Parameters.size();
      const ParameterKind Kind = ParameterKinds.empty() ? ParameterKind::Positional : ParameterKinds[Index];
      const Name ParameterName = ParameterNames.empty() ? Name{} : ParameterNames[Index];
      auto Parameter = std::unique_ptr<FunctionParameter>(new FunctionParameter(ParameterName, *ParameterType, Index, Kind));
      Parameter->Outer = Pointer;
      Pointer->Parameters.push_back(std::move(Parameter));
    }
    return Result;
  }

  BasicBlock *IRBuilder::createFunctionBody(Function &FunctionValue)
  {
    if (&FunctionValue.context() != &Context || FunctionValue.hasBody())
    {
      return nullptr;
    }
    return createBasicBlock(FunctionValue);
  }

  std::unique_ptr<BasicBlock> IRBuilder::createBasicBlock()
  {
    return std::unique_ptr<BasicBlock>(new BasicBlock(Context));
  }

  BasicBlock *IRBuilder::createBasicBlock(Function &FunctionValue)
  {
    if (&FunctionValue.context() != &Context)
    {
      return nullptr;
    }
    auto Result = createBasicBlock();
    BasicBlock *Block = Result.get();
    FunctionValue.Blocks.push_back(std::move(Result));
    Block->Outer = &FunctionValue;
    return Block;
  }

  Module *IRBuilder::createModule(Name ModuleName)
  {
    if (!Context.namePool().contains(ModuleName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<Module>(new Module(Context, ModuleName, createBasicBlock()));
    Module *Pointer = Result.get();
    Context.Modules.push_back(std::move(Result));
    Pointer->EntryBlock->Outer = Pointer;
    return Pointer;
  }

  std::unique_ptr<CallInstruction> IRBuilder::createDetachedCallInstruction(const Value &Callee, std::span<const Value *const> Arguments)
  {
    if (&Callee.context() != &Context || !FunctionType::classof(&Callee.type()))
    {
      return nullptr;
    }
    const auto &Signature = static_cast<const FunctionType &>(Callee.type());
    if (!matchesCallArguments(Signature, Arguments))
    {
      return nullptr;
    }
    return std::unique_ptr<CallInstruction>(new CallInstruction(Callee, Arguments));
  }

  std::unique_ptr<CStringInstruction> IRBuilder::createDetachedCStringInstruction(const StringConstant &Source)
  {
    if (&Source.context() != &Context || !Context.constantPool().owns(Source) || !Source.tryGetCString())
    {
      return nullptr;
    }
    const auto &Slice = static_cast<const SliceType &>(Source.type());
    const auto *Pointer = Context.typePool().getType<TypeKind::Pointer>(Slice.elementType(), AccessKind::ReadWrite);
    return std::unique_ptr<CStringInstruction>(new CStringInstruction(*Pointer, Source));
  }

  std::unique_ptr<AllocaInstruction> IRBuilder::createDetachedAllocaInstruction(const Type &AllocatedType)
  {
    if (&AllocatedType.context() != &Context || !isMemoryValueType(AllocatedType))
    {
      return nullptr;
    }
    const PointerType *ResultType = Context.typePool().getType<TypeKind::Pointer>(AllocatedType, AccessKind::ReadWrite);
    return std::unique_ptr<AllocaInstruction>(new AllocaInstruction(*ResultType));
  }

  std::unique_ptr<LoadInstruction> IRBuilder::createDetachedLoadInstruction(const Value &Address)
  {
    if (&Address.context() != &Context || !PointerType::classof(&Address.type()))
    {
      return nullptr;
    }
    const auto &AddressType = static_cast<const PointerType &>(Address.type());
    if (!isMemoryValueType(AddressType.pointeeType()))
    {
      return nullptr;
    }
    return std::unique_ptr<LoadInstruction>(new LoadInstruction(Address));
  }

  std::unique_ptr<StoreInstruction> IRBuilder::createDetachedStoreInstruction(const Value &Address, const Value &StoredValue)
  {
    if (&Address.context() != &Context || &StoredValue.context() != &Context || !PointerType::classof(&Address.type()))
    {
      return nullptr;
    }
    const auto &AddressType = static_cast<const PointerType &>(Address.type());
    if (AddressType.access() != AccessKind::ReadWrite || !isMemoryValueType(AddressType.pointeeType()) || &AddressType.pointeeType() != &StoredValue.type())
    {
      return nullptr;
    }
    return std::unique_ptr<StoreInstruction>(new StoreInstruction(Address, StoredValue));
  }

  std::unique_ptr<AddInstruction> IRBuilder::createDetachedAddInstruction(const Value &Left, const Value &Right)
  {
    if (&Left.context() != &Context || &Right.context() != &Context || !IntegerType::classof(&Left.type()) || &Left.type() != &Right.type())
    {
      return nullptr;
    }
    return std::unique_ptr<AddInstruction>(new AddInstruction(Left, Right));
  }

  std::unique_ptr<LogicalNotInstruction> IRBuilder::createDetachedLogicalNotInstruction(const Value &Operand)
  {
    if (&Operand.context() != &Context || Operand.type().typeKind() != TypeKind::Bool)
    {
      return nullptr;
    }
    return std::unique_ptr<LogicalNotInstruction>(new LogicalNotInstruction(Operand));
  }

  std::unique_ptr<LogicalAndInstruction> IRBuilder::createDetachedLogicalAndInstruction(const Value &Left, const Value &Right)
  {
    if (&Left.context() != &Context || &Right.context() != &Context || Left.type().typeKind() != TypeKind::Bool || &Left.type() != &Right.type())
    {
      return nullptr;
    }
    return std::unique_ptr<LogicalAndInstruction>(new LogicalAndInstruction(Left, Right));
  }

  std::unique_ptr<LogicalOrInstruction> IRBuilder::createDetachedLogicalOrInstruction(const Value &Left, const Value &Right)
  {
    if (&Left.context() != &Context || &Right.context() != &Context || Left.type().typeKind() != TypeKind::Bool || &Left.type() != &Right.type())
    {
      return nullptr;
    }
    return std::unique_ptr<LogicalOrInstruction>(new LogicalOrInstruction(Left, Right));
  }

  std::unique_ptr<CompareInstruction> IRBuilder::createDetachedCompareInstruction(ComparisonPredicate Predicate, const Value &Left, const Value &Right)
  {
    if (&Left.context() != &Context || &Right.context() != &Context || &Left.type() != &Right.type() || (!IntegerType::classof(&Left.type()) && Left.type().typeKind() != TypeKind::Bool))
    {
      return nullptr;
    }
    switch (Predicate)
    {
    case ComparisonPredicate::Equal:
    case ComparisonPredicate::NotEqual:
      break;
    case ComparisonPredicate::Less:
    case ComparisonPredicate::LessEqual:
    case ComparisonPredicate::Greater:
    case ComparisonPredicate::GreaterEqual:
      if (!IntegerType::classof(&Left.type()))
      {
        return nullptr;
      }
      break;
    default:
      return nullptr;
    }
    return std::unique_ptr<CompareInstruction>(new CompareInstruction(Context.typePool().getType<TypeKind::Bool>(), Predicate, Left, Right));
  }

  std::unique_ptr<ReturnInstruction> IRBuilder::createDetachedReturnInstruction(const Value *ReturnedValue)
  {
    if (ReturnedValue && (&ReturnedValue->context() != &Context || ReturnedValue->type().typeKind() == TypeKind::Void))
    {
      return nullptr;
    }
    return std::unique_ptr<ReturnInstruction>(new ReturnInstruction(Context, ReturnedValue));
  }

  std::unique_ptr<BranchInstruction> IRBuilder::createDetachedBranchInstruction(const BasicBlock &Target)
  {
    if (!isValidBranchTarget(Target))
    {
      return nullptr;
    }
    return std::unique_ptr<BranchInstruction>(new BranchInstruction(Context.typePool().getType<TypeKind::Void>(), Target));
  }

  std::unique_ptr<ConditionalBranchInstruction> IRBuilder::createDetachedConditionalBranchInstruction(const Value &Condition, const BasicBlock &TrueTarget, const BasicBlock &FalseTarget)
  {
    if (&Condition.context() != &Context || Condition.type().typeKind() != TypeKind::Bool || !isValidBranchTarget(TrueTarget) || !isValidBranchTarget(FalseTarget) || (TrueTarget.outer() && FalseTarget.outer() && TrueTarget.outer() != FalseTarget.outer()))
    {
      return nullptr;
    }
    return std::unique_ptr<ConditionalBranchInstruction>(new ConditionalBranchInstruction(Context.typePool().getType<TypeKind::Void>(), Condition, TrueTarget, FalseTarget));
  }

  ModuleDecl *IRBuilder::createModuleDecl(Module &Owner, const parser::ModuleAST &AST)
  {
    if (&Owner.context() != &Context || Owner.DeclarationRoot)
    {
      return nullptr;
    }
    Owner.DeclarationRoot.reset(new ModuleDecl(Owner, Owner.name(), AST));
    return Owner.DeclarationRoot.get();
  }

  FunctionDecl *IRBuilder::createFunctionDecl(Decl &Parent, Name DeclName, const parser::FunctionDecl &AST)
  {
    if (&Parent.module().context() != &Context || !Context.namePool().contains(DeclName) || AST.genericParameters().empty())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<FunctionDecl>(new FunctionDecl(Parent, DeclName, AST));
    FunctionDecl *Pointer = Result.get();
    Parent.Children.push_back(std::move(Result));
    return Pointer;
  }

  ClassDecl *IRBuilder::createClassDecl(Decl &Parent, Name DeclName, const parser::ClassDecl &AST)
  {
    if (&Parent.module().context() != &Context || !Context.namePool().contains(DeclName) || AST.genericParameters().empty())
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ClassDecl>(new ClassDecl(Parent, DeclName, AST));
    ClassDecl *Pointer = Result.get();
    Parent.Children.push_back(std::move(Result));
    return Pointer;
  }

  bool IRBuilder::canInsert() const noexcept
  {
    return Point.Block && canInsertAt(*Point.Block, Point.Before, false);
  }

  CallInstruction *IRBuilder::createCallInstruction(const Value &Callee, std::span<const Value *const> Arguments)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedCallInstruction(Callee, Arguments));
  }

  CStringInstruction *IRBuilder::createCStringInstruction(const StringConstant &Source)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedCStringInstruction(Source));
  }

  AllocaInstruction *IRBuilder::createAllocaInstruction(const Type &AllocatedType)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedAllocaInstruction(AllocatedType));
  }

  LoadInstruction *IRBuilder::createLoadInstruction(const Value &Address)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedLoadInstruction(Address));
  }

  StoreInstruction *IRBuilder::createStoreInstruction(const Value &Address, const Value &StoredValue)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedStoreInstruction(Address, StoredValue));
  }

  AddInstruction *IRBuilder::createAddInstruction(const Value &Left, const Value &Right)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedAddInstruction(Left, Right));
  }

  LogicalNotInstruction *IRBuilder::createLogicalNotInstruction(const Value &Operand)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedLogicalNotInstruction(Operand));
  }

  LogicalAndInstruction *IRBuilder::createLogicalAndInstruction(const Value &Left, const Value &Right)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedLogicalAndInstruction(Left, Right));
  }

  LogicalOrInstruction *IRBuilder::createLogicalOrInstruction(const Value &Left, const Value &Right)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedLogicalOrInstruction(Left, Right));
  }

  CompareInstruction *IRBuilder::createCompareInstruction(ComparisonPredicate Predicate, const Value &Left, const Value &Right)
  {
    if (!canInsert())
    {
      return nullptr;
    }
    return insert(createDetachedCompareInstruction(Predicate, Left, Right));
  }

  ReturnInstruction *IRBuilder::createReturnInstruction(const Value *ReturnedValue)
  {
    if (!Point.Block || !canInsertReturn(*Point.Block, ReturnedValue, Point.Before))
    {
      return nullptr;
    }
    return insert(createDetachedReturnInstruction(ReturnedValue));
  }

  BranchInstruction *IRBuilder::createBranchInstruction(const BasicBlock &Target)
  {
    if (!Point.Block || !canInsertBranch(*Point.Block, Target, Point.Before))
    {
      return nullptr;
    }
    return insert(createDetachedBranchInstruction(Target));
  }

  ConditionalBranchInstruction *IRBuilder::createConditionalBranchInstruction(const Value &Condition, const BasicBlock &TrueTarget, const BasicBlock &FalseTarget)
  {
    if (!Point.Block || !canInsertBranch(*Point.Block, TrueTarget, Point.Before) || !canInsertBranch(*Point.Block, FalseTarget, Point.Before))
    {
      return nullptr;
    }
    return insert(createDetachedConditionalBranchInstruction(Condition, TrueTarget, FalseTarget));
  }
} // namespace ink::ir
