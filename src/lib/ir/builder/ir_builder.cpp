#include "ink/ir/builder/ir_builder.h"

#include "ink/core/support/core_define.h"
#include "ink/core/support/return_macros.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/nominal_type/nominal_type.h"
#include "ink/ir/module/module.h"

#include <memory>
#include <utility>

namespace ink::ir
{
  namespace
  {
    bool validPredicate(ICmpPredicate Predicate) noexcept
    {
      switch (Predicate)
      {
        case ICmpPredicate::Equal:
        case ICmpPredicate::NotEqual:
        case ICmpPredicate::UnsignedGreaterThan:
        case ICmpPredicate::UnsignedGreaterEqual:
        case ICmpPredicate::UnsignedLessThan:
        case ICmpPredicate::UnsignedLessEqual:
        case ICmpPredicate::SignedGreaterThan:
        case ICmpPredicate::SignedGreaterEqual:
        case ICmpPredicate::SignedLessThan:
        case ICmpPredicate::SignedLessEqual:
          return true;
      }
      return false;
    }

    const Type *indexedFieldType(const NominalType &Aggregate, const Value &Index) noexcept
    {
      RETURN_IF(!IntegerConstant::classof(&Index), nullptr);
      const auto FieldIndex = static_cast<const IntegerConstant &>(Index).tryGetNonNegativeUInt64();
      RETURN_IF(!FieldIndex || *FieldIndex >= Aggregate.fieldCount(), nullptr);
      return Aggregate.field(static_cast<std::size_t>(*FieldIndex))->type();
    }
  } // namespace

  IRBuilder::IRBuilder(IRContext &Context) noexcept
      : Context(Context)
  {
  }

  BasicBlock *IRBuilder::insertBlock() const noexcept
  {
    return InsertBlock;
  }

  Function *IRBuilder::insertFunction() const noexcept
  {
    return InsertBlock ? InsertBlock->function() : nullptr;
  }

  bool IRBuilder::setInsertPoint(BasicBlock &Block) noexcept
  {
    RETURN_IF(!Block.function() || !Context.containsType(Block.function()->functionType()), false);
    InsertBlock = &Block;
    return true;
  }

  void IRBuilder::clearInsertPoint() noexcept
  {
    InsertBlock = nullptr;
  }

  bool IRBuilder::canAppend() const noexcept
  {
    RETURN_IF(!insertFunction() || !Context.containsType(insertFunction()->functionType()), false);
    return !InsertBlock->hasTerminator();
  }

  bool IRBuilder::validOperand(const Value &Operand) const noexcept
  {
    const Type *OperandType = Operand.type();
    RETURN_IF(!insertFunction() || !Context.containsType(OperandType) || !OperandType->hasLayout(), false);
    RETURN_IF(Constant::classof(&Operand), Context.constantPool().owns(static_cast<const Constant *>(&Operand)));
    if (Parameter::classof(&Operand))
    {
      const auto &ParameterValue = static_cast<const Parameter &>(Operand);
      return ParameterValue.function() == insertFunction() && !ParameterValue.isVariadic();
    }
    if (Instruction::classof(&Operand))
    {
      const auto &InstructionValue = static_cast<const Instruction &>(Operand);
      const BasicBlock *Parent = InstructionValue.parentBlock();
      // A unit-producing call is a value; terminators and stores are not. Function membership does not prove dominance.
      return !InstructionValue.isTerminator() && !StoreInstruction::classof(&InstructionValue) && Parent && Parent->function() == insertFunction();
    }
    // Globals describe storage. Using their contents or address requires an explicit operation, not an implicit load here.
    return Function::classof(&Operand);
  }

  bool IRBuilder::canUseAs(const Value &Operand, const Type &ExpectedType) const noexcept
  {
    RETURN_IF(!Context.containsType(&ExpectedType) || !validOperand(Operand), false);
    RETURN_IF(Constant::classof(&Operand), static_cast<const Constant &>(Operand).canInitialize(&ExpectedType));
    return Operand.type() == &ExpectedType;
  }

  bool IRBuilder::validTarget(const BasicBlock &Target) const noexcept
  {
    return insertFunction() && Target.function() == insertFunction();
  }

  const Type *IRBuilder::memoryPointeeType(const Value &Pointer) const noexcept
  {
    RETURN_IF(!validOperand(Pointer) || !PointerType::classof(Pointer.type()), nullptr);
    const auto &PointerValueType = static_cast<const PointerType &>(*Pointer.type());
    const Type *PointeeType = PointerValueType.pointeeType();
    RETURN_IF(PointerValueType.isOpaque() || !Context.containsType(PointeeType) || !PointeeType->hasLayout(), nullptr);
    return PointeeType;
  }

  template <typename InstructionT, typename... ArgumentTypes>
  InstructionT *IRBuilder::append(ArgumentTypes &&...Arguments)
  {
    auto InstructionValue = std::make_unique<InstructionT>(std::forward<ArgumentTypes>(Arguments)...);
    return static_cast<InstructionT *>(InsertBlock->appendInstruction(std::move(InstructionValue)));
  }

  CallInstruction *IRBuilder::createCall(const Function &Callee, std::vector<Argument> Arguments)
  {
    return createCall(*Callee.functionType(), Callee, std::move(Arguments));
  }

  CallInstruction *IRBuilder::createCall(const FunctionType &Signature, const Value &Callee, std::vector<Argument> Arguments)
  {
    check(canAppend());
    RETURN_IF(!Context.containsType(&Signature) || !validOperand(Callee) || Arguments.size() != Signature.parameterCount(), nullptr);
    const Type *CalleeType = Callee.type();
    RETURN_IF(CalleeType != &Signature && (!PointerType::classof(CalleeType) || static_cast<const PointerType *>(CalleeType)->pointeeType() != &Signature), nullptr);
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      const FunctionTypeParameter &Descriptor = Signature.parameters()[Index];
      RETURN_IF(Descriptor.Variadic || !Arguments[Index].value() || !canUseAs(*Arguments[Index].value(), *Descriptor.ParameterType), nullptr);
    }
    return append<CallInstruction>(Signature, Callee, std::move(Arguments));
  }

  ICmpInstruction *IRBuilder::createICmp(ICmpPredicate Predicate, const Value &Left, const Value &Right)
  {
    check(canAppend());
    RETURN_IF(!validPredicate(Predicate) || !validOperand(Left) || !validOperand(Right), nullptr);
    const Type *LeftType = Left.type();
    const Type *RightType = Right.type();
    bool Compatible = LeftType == RightType && (IntegerType::classof(LeftType) || PointerType::classof(LeftType));
    if (!Compatible && (Predicate == ICmpPredicate::Equal || Predicate == ICmpPredicate::NotEqual))
    {
      Compatible = (PointerType::classof(LeftType) && canUseAs(Right, *LeftType)) || (PointerType::classof(RightType) && canUseAs(Left, *RightType));
    }
    RETURN_IF(!Compatible, nullptr);
    return append<ICmpInstruction>(*Context.typePool().getBoolType(), Predicate, Left, Right);
  }

  NegInstruction *IRBuilder::createNeg(const Value &Operand)
  {
    check(canAppend());
    // TODO: Fold numeric constant operands when constant evaluation is supported by the Builder.
    const Type *OperandType = Operand.type();
    RETURN_IF(!validOperand(Operand) || (!IntegerType::classof(OperandType) && !FloatType::classof(OperandType)), nullptr);
    return append<NegInstruction>(Operand);
  }

  AllocaInstruction *IRBuilder::createAlloca(const Type &AllocatedType)
  {
    check(canAppend());
    RETURN_IF(!Context.containsType(&AllocatedType) || !AllocatedType.hasLayout() || VoidType::classof(&AllocatedType), nullptr);
    const PointerType *ResultType = typePool().getPointerType(&AllocatedType);
    RETURN_IF_NULL(ResultType, nullptr);
    return append<AllocaInstruction>(*ResultType, AllocatedType);
  }

  LoadInstruction *IRBuilder::createLoad(const Type &LoadedType, const Value &Pointer)
  {
    check(canAppend());
    RETURN_IF(!Context.containsType(&LoadedType) || memoryPointeeType(Pointer) != &LoadedType, nullptr);
    return append<LoadInstruction>(LoadedType, Pointer);
  }

  StoreInstruction *IRBuilder::createStore(const Value &StoredValue, const Value &Pointer)
  {
    check(canAppend());
    const Type *StoredType = memoryPointeeType(Pointer);
    RETURN_IF(!StoredType || !canUseAs(StoredValue, *StoredType), nullptr);
    return append<StoreInstruction>(*Context.typePool().getVoidType(), StoredValue, Pointer);
  }

  GetElementPtrInstruction *IRBuilder::createGetElementPtr(const Type &SourceElementType, const Value &Pointer, std::vector<const Value *> Indices, bool InBounds)
  {
    check(canAppend());
    RETURN_IF(!Context.containsType(&SourceElementType) || !SourceElementType.hasLayout() || VoidType::classof(&SourceElementType) || !validOperand(Pointer), nullptr);
    const Type *PointerValueType = Pointer.type();
    RETURN_IF(&Pointer != Context.constantPool().getNullPointerConstant() && (!PointerType::classof(PointerValueType) || static_cast<const PointerType *>(PointerValueType)->pointeeType() != &SourceElementType), nullptr);
    const Type *SelectedType = &SourceElementType;
    for (std::size_t Index = 0; Index < Indices.size(); ++Index)
    {
      const Value *IndexValue = Indices[Index];
      RETURN_IF(!IndexValue || !validOperand(*IndexValue) || !IntegerType::classof(IndexValue->type()), nullptr);
      if (Index == 0)
      {
        continue;
      }
      if (ArrayType::classof(SelectedType))
      {
        SelectedType = static_cast<const ArrayType *>(SelectedType)->elementType();
      }
      else if (NominalType::classof(SelectedType))
      {
        SelectedType = indexedFieldType(*static_cast<const NominalType *>(SelectedType), *IndexValue);
      }
      else
      {
        return nullptr;
      }
      RETURN_IF(!Context.containsType(SelectedType) || !SelectedType->hasLayout(), nullptr);
    }
    const PointerType *ResultType = typePool().getPointerType(SelectedType);
    RETURN_IF_NULL(ResultType, nullptr);
    return append<GetElementPtrInstruction>(*ResultType, SourceElementType, Pointer, std::move(Indices), InBounds);
  }

  BranchInstruction *IRBuilder::createBranch(BasicBlock &Target)
  {
    check(canAppend());
    RETURN_IF(!validTarget(Target), nullptr);
    return append<BranchInstruction>(*Context.typePool().getVoidType(), Target);
  }

  ConditionalBranchInstruction *IRBuilder::createConditionalBranch(const Value &Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget)
  {
    check(canAppend());
    RETURN_IF(!canUseAs(Condition, *Context.typePool().getBoolType()) || !validTarget(TrueTarget) || !validTarget(FalseTarget), nullptr);
    return append<ConditionalBranchInstruction>(*Context.typePool().getVoidType(), Condition, TrueTarget, FalseTarget);
  }

  ReturnInstruction *IRBuilder::createReturn()
  {
    check(canAppend());
    RETURN_IF(insertFunction()->functionType()->returnType() != Context.typePool().getVoidType(), nullptr);
    return append<ReturnInstruction>(*Context.typePool().getVoidType());
  }

  ReturnInstruction *IRBuilder::createReturn(const Value &ReturnedValue)
  {
    check(canAppend());
    RETURN_IF(insertFunction()->functionType()->returnType() == Context.typePool().getVoidType() || !canUseAs(ReturnedValue, *insertFunction()->functionType()->returnType()), nullptr);
    return append<ReturnInstruction>(*Context.typePool().getVoidType(), &ReturnedValue);
  }
} // namespace ink::ir
