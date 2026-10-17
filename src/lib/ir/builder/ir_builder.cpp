#include "ink/ir/builder/ir_builder.h"

#include "ink/core/support/core_define.h"
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
      if (!IntegerConstant::classof(&Index))
      {
        return nullptr;
      }
      const auto FieldIndex = static_cast<const IntegerConstant &>(Index).tryGetNonNegativeUInt64();
      if (!FieldIndex || *FieldIndex >= Aggregate.fieldCount())
      {
        return nullptr;
      }
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
    if (!Block.function() || !Context.containsType(Block.function()->functionType()))
    {
      return false;
    }
    InsertBlock = &Block;
    return true;
  }

  void IRBuilder::clearInsertPoint() noexcept
  {
    InsertBlock = nullptr;
  }

  bool IRBuilder::canAppend() const noexcept
  {
    if (!insertFunction() || !Context.containsType(insertFunction()->functionType()))
    {
      return false;
    }
    return !InsertBlock->hasTerminator();
  }

  bool IRBuilder::validOperand(const Value &Operand) const noexcept
  {
    const Type *OperandType = Operand.type();
    if (!insertFunction() || !Context.containsType(OperandType) || !OperandType->hasLayout())
    {
      return false;
    }
    if (Constant::classof(&Operand))
    {
      return Context.constantPool().owns(static_cast<const Constant *>(&Operand));
    }
    if (Parameter::classof(&Operand))
    {
      const auto &ParameterValue = static_cast<const Parameter &>(Operand);
      return ParameterValue.function() == insertFunction() && !ParameterValue.isVariadic();
    }
    if (Instruction::classof(&Operand))
    {
      const auto &InstructionValue = static_cast<const Instruction &>(Operand);
      const BasicBlock *Parent = InstructionValue.parentBlock();
      // A unit-producing call is a value; terminators are not. Function membership does not prove dominance.
      return !InstructionValue.isTerminator() && Parent && Parent->function() == insertFunction();
    }
    // Globals describe storage. Using their contents or address requires an explicit operation, not an implicit load here.
    return Function::classof(&Operand);
  }

  bool IRBuilder::canUseAs(const Value &Operand, const Type &ExpectedType) const noexcept
  {
    if (!Context.containsType(&ExpectedType) || !validOperand(Operand))
    {
      return false;
    }
    if (Constant::classof(&Operand))
    {
      return static_cast<const Constant &>(Operand).canInitialize(&ExpectedType);
    }
    return Operand.type() == &ExpectedType;
  }

  bool IRBuilder::validTarget(const BasicBlock &Target) const noexcept
  {
    return insertFunction() && Target.function() == insertFunction();
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
    if (!Context.containsType(&Signature) || !validOperand(Callee) || Arguments.size() != Signature.parameterCount())
    {
      return nullptr;
    }
    const Type *CalleeType = Callee.type();
    if (CalleeType != &Signature && (!PointerType::classof(CalleeType) || static_cast<const PointerType *>(CalleeType)->pointeeType() != &Signature))
    {
      return nullptr;
    }
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      const FunctionTypeParameter &Descriptor = Signature.parameters()[Index];
      if (Descriptor.Variadic || !Arguments[Index].value() || !canUseAs(*Arguments[Index].value(), *Descriptor.ParameterType))
      {
        return nullptr;
      }
    }
    return append<CallInstruction>(Signature, Callee, std::move(Arguments));
  }

  ICmpInstruction *IRBuilder::createICmp(ICmpPredicate Predicate, const Value &Left, const Value &Right)
  {
    check(canAppend());
    if (!validPredicate(Predicate) || !validOperand(Left) || !validOperand(Right))
    {
      return nullptr;
    }
    const Type *LeftType = Left.type();
    const Type *RightType = Right.type();
    bool Compatible = LeftType == RightType && (IntegerType::classof(LeftType) || PointerType::classof(LeftType));
    if (!Compatible && (Predicate == ICmpPredicate::Equal || Predicate == ICmpPredicate::NotEqual))
    {
      Compatible = (PointerType::classof(LeftType) && canUseAs(Right, *LeftType)) || (PointerType::classof(RightType) && canUseAs(Left, *RightType));
    }
    if (!Compatible)
    {
      return nullptr;
    }
    return append<ICmpInstruction>(*Context.typePool().getBoolType(), Predicate, Left, Right);
  }

  NegInstruction *IRBuilder::createNeg(const Value &Operand)
  {
    check(canAppend());
    // TODO: Fold numeric constant operands when constant evaluation is supported by the Builder.
    const Type *OperandType = Operand.type();
    if (!validOperand(Operand) || (!IntegerType::classof(OperandType) && !FloatType::classof(OperandType)))
    {
      return nullptr;
    }
    return append<NegInstruction>(Operand);
  }

  GetElementPtrInstruction *IRBuilder::createGetElementPtr(const Type &SourceElementType, const Value &Pointer, std::vector<const Value *> Indices, bool InBounds)
  {
    check(canAppend());
    if (!Context.containsType(&SourceElementType) || !SourceElementType.hasLayout() || VoidType::classof(&SourceElementType) || !validOperand(Pointer))
    {
      return nullptr;
    }
    const Type *PointerValueType = Pointer.type();
    if (&Pointer != Context.constantPool().getNullPointerConstant() && (!PointerType::classof(PointerValueType) || static_cast<const PointerType *>(PointerValueType)->pointeeType() != &SourceElementType))
    {
      return nullptr;
    }
    const Type *SelectedType = &SourceElementType;
    for (std::size_t Index = 0; Index < Indices.size(); ++Index)
    {
      const Value *IndexValue = Indices[Index];
      if (!IndexValue || !validOperand(*IndexValue) || !IntegerType::classof(IndexValue->type()))
      {
        return nullptr;
      }
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
      if (!Context.containsType(SelectedType) || !SelectedType->hasLayout())
      {
        return nullptr;
      }
    }
    const PointerType *ResultType = typePool().getPointerType(SelectedType);
    if (!ResultType)
    {
      return nullptr;
    }
    return append<GetElementPtrInstruction>(*ResultType, SourceElementType, Pointer, std::move(Indices), InBounds);
  }

  BranchInstruction *IRBuilder::createBranch(BasicBlock &Target)
  {
    check(canAppend());
    if (!validTarget(Target))
    {
      return nullptr;
    }
    return append<BranchInstruction>(*Context.typePool().getVoidType(), Target);
  }

  ConditionalBranchInstruction *IRBuilder::createConditionalBranch(const Value &Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget)
  {
    check(canAppend());
    if (!canUseAs(Condition, *Context.typePool().getBoolType()) || !validTarget(TrueTarget) || !validTarget(FalseTarget))
    {
      return nullptr;
    }
    return append<ConditionalBranchInstruction>(*Context.typePool().getVoidType(), Condition, TrueTarget, FalseTarget);
  }

  ReturnInstruction *IRBuilder::createReturn()
  {
    check(canAppend());
    if (insertFunction()->functionType()->returnType() != Context.typePool().getVoidType())
    {
      return nullptr;
    }
    return append<ReturnInstruction>(*Context.typePool().getVoidType());
  }

  ReturnInstruction *IRBuilder::createReturn(const Value &ReturnedValue)
  {
    check(canAppend());
    if (insertFunction()->functionType()->returnType() == Context.typePool().getVoidType() || !canUseAs(ReturnedValue, *insertFunction()->functionType()->returnType()))
    {
      return nullptr;
    }
    return append<ReturnInstruction>(*Context.typePool().getVoidType(), &ReturnedValue);
  }
} // namespace ink::ir
