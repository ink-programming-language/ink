#include "ink/ir/analysis/verifier.h"

#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/compilation/module_name.h"
#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/memory.h"
#include "ink/ir/model/constant.h"
#include "ink/ir/model/operand.h"
#include "ink/ir/model/struct_type.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir
{
  using core::Diagnostic;
  using core::DiagnosticClass;
  using core::DiagnosticKind;

  namespace
  {
    class DiagnosticCollector
    {
      public:
        explicit DiagnosticCollector(DiagnosticClass Class)
            : Class(Class)
        {
        }

        template <DiagnosticKind Kind, typename... ArgumentTypes>
        void add(ArgumentTypes &&...Arguments)
        {
          Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
          DiagnosticEntry.Class = Class;
          Diagnostics.push_back(std::move(DiagnosticEntry));
        }

        void report(IRContext &Context) const
        {
          for (const Diagnostic &DiagnosticEntry : Diagnostics)
          {
            Context.diagnosticEngine().report(DiagnosticEntry);
          }
        }

        bool empty() const noexcept
        {
          return Diagnostics.empty();
        }

      private:
        DiagnosticClass Class;
        std::vector<Diagnostic> Diagnostics;
    };

    bool isValidType(const Type *TypeValue)
    {
      if (TypeValue == nullptr)
      {
        return false;
      }
      switch (TypeValue->kind())
      {
#define INK_IR_TYPE(Name, Spelling) \
  case TypeKind::Name:              \
    return true;
#include "ink/ir/ir.def"
      case TypeKind::Struct:
        return true;
      case TypeKind::Count:
        return false;
      }
      return false;
    }

    bool isValidType(const Module &ModuleValue, const Type *TypeValue)
    {
      if (!isValidType(TypeValue))
      {
        return false;
      }
      if (TypeValue->kind() != TypeKind::Struct)
      {
        return true;
      }
      return std::find(ModuleValue.StructTypes.begin(), ModuleValue.StructTypes.end(), static_cast<const StructType *>(TypeValue)) != ModuleValue.StructTypes.end();
    }

    bool isByteSliceType(const Type *TypeValue)
    {
      return TypeValue != nullptr && (TypeValue->kind() == TypeKind::ByteSlice || TypeValue->kind() == TypeKind::ConstByteSlice);
    }

    bool isBytePointerType(const Type *TypeValue)
    {
      return TypeValue != nullptr && (TypeValue->kind() == TypeKind::BytePointer || TypeValue->kind() == TypeKind::ConstBytePointer);
    }

    const GlobalVariable *referencedGlobal(const Module &ModuleValue, const Value &OperandValue)
    {
      if (OperandValue.kind() != ValueKind::GlobalVariableAddressOperand)
      {
        return nullptr;
      }
      const GlobalId Global = static_cast<const GlobalVariableAddressOperand &>(OperandValue).global();
      if (!Global.valid() || Global.value() >= ModuleValue.Globals.size())
      {
        return nullptr;
      }
      return &ModuleValue.Globals[Global.value()];
    }

    bool isAddType(const Type *TypeValue)
    {
      return TypeValue != nullptr && (TypeValue->kind() == TypeKind::Byte || TypeValue->kind() == TypeKind::I32 || TypeValue->kind() == TypeKind::PointerSize);
    }

    bool isComparableType(const Type *TypeValue)
    {
      if (TypeValue == nullptr)
      {
        return false;
      }
      return TypeValue->kind() == TypeKind::Bool || TypeValue->kind() == TypeKind::Byte || TypeValue->kind() == TypeKind::I32 || TypeValue->kind() == TypeKind::PointerSize || TypeValue->kind() == TypeKind::BytePointer || TypeValue->kind() == TypeKind::ConstBytePointer;
    }

    struct SsaDefinition
    {
        const Type *DefinitionType = nullptr;
        std::size_t BlockIndex = InvalidId;
        std::size_t InstructionIndex = 0;
    };

    using SsaDefinitionMap = std::unordered_map<std::size_t, SsaDefinition>;

    struct FunctionControlFlow
    {
        std::vector<std::vector<std::size_t>> Predecessors;
        std::vector<std::vector<std::size_t>> Successors;
        std::vector<bool> Reachable;
        std::vector<std::vector<bool>> Dominators;
    };

    struct InstructionResult
    {
        ValueId Id;
        const Type *ResultType = nullptr;
        const char *Operation = nullptr;
    };

    std::optional<InstructionResult> instructionResult(const Instruction &InstructionValue)
    {
      switch (InstructionValue.kind())
      {
      case InstructionKind::Call:
      {
        const CallInstruction &Call = static_cast<const CallInstruction &>(InstructionValue);
        if (Call.Result.has_value())
        {
          return InstructionResult{*Call.Result, Call.ResultType, "call"};
        }
        return std::nullopt;
      }
      case InstructionKind::Alloca:
      {
        const AllocaInstruction &Alloca = static_cast<const AllocaInstruction &>(InstructionValue);
        return InstructionResult{Alloca.Result, Alloca.ResultType, "alloca"};
      }
      case InstructionKind::GetElementPointer:
      {
        const GetElementPointerInstruction &GetElementPointer = static_cast<const GetElementPointerInstruction &>(InstructionValue);
        return InstructionResult{GetElementPointer.Result, GetElementPointer.ResultType, "getelementptr"};
      }
      case InstructionKind::Load:
      {
        const LoadInstruction &Load = static_cast<const LoadInstruction &>(InstructionValue);
        return InstructionResult{Load.Result, Load.ResultType, "load"};
      }
      case InstructionKind::SliceData:
      {
        const SliceDataInstruction &SliceData = static_cast<const SliceDataInstruction &>(InstructionValue);
        return InstructionResult{SliceData.Result, SliceData.ResultType, "slice.data"};
      }
      case InstructionKind::SliceLength:
      {
        const SliceLengthInstruction &SliceLength = static_cast<const SliceLengthInstruction &>(InstructionValue);
        return InstructionResult{SliceLength.Result, SliceLength.ResultType, "slice.length"};
      }
      case InstructionKind::Phi:
      {
        const PhiInstruction &Phi = static_cast<const PhiInstruction &>(InstructionValue);
        return InstructionResult{Phi.Result, Phi.ResultType, "phi"};
      }
      case InstructionKind::Add:
      {
        const AddInstruction &Add = static_cast<const AddInstruction &>(InstructionValue);
        return InstructionResult{Add.Result, Add.ResultType, "add"};
      }
      case InstructionKind::Compare:
      {
        const CompareInstruction &Compare = static_cast<const CompareInstruction &>(InstructionValue);
        return InstructionResult{Compare.Result, Compare.ResultType, "icmp"};
      }
      case InstructionKind::InsertValue:
      {
        const InsertValueInstruction &Insert = static_cast<const InsertValueInstruction &>(InstructionValue);
        return InstructionResult{Insert.Result, Insert.ResultType, "insertvalue"};
      }
      case InstructionKind::ExtractValue:
      {
        const ExtractValueInstruction &Extract = static_cast<const ExtractValueInstruction &>(InstructionValue);
        return InstructionResult{Extract.Result, Extract.ResultType, "extractvalue"};
      }
      case InstructionKind::Store:
      case InstructionKind::Import:
      case InstructionKind::LifetimeEnd:
      case InstructionKind::Branch:
      case InstructionKind::ConditionalBranch:
      case InstructionKind::Return:
        return std::nullopt;
      }
      return std::nullopt;
    }

    bool sameValue(const Value &Left, const Value &Right)
    {
      if (Left.kind() != Right.kind() || &Left.type() != &Right.type())
      {
        return false;
      }
      switch (Left.kind())
      {
      case ValueKind::IntegerConstant:
      case ValueKind::FloatConstant:
      case ValueKind::StringConstant:
      case ValueKind::NullConstant:
      case ValueKind::ZeroInitializer:
      case ValueKind::AggregateConstant:
        return constantsEqual(static_cast<const Constant &>(Left), static_cast<const Constant &>(Right));
      case ValueKind::ValueOperand:
        return static_cast<const ValueOperand &>(Left).id() == static_cast<const ValueOperand &>(Right).id();
      case ValueKind::GlobalAddressOperand:
      {
        const GlobalAddressOperand &LeftAddress = static_cast<const GlobalAddressOperand &>(Left);
        const GlobalAddressOperand &RightAddress = static_cast<const GlobalAddressOperand &>(Right);
        return LeftAddress.global() == RightAddress.global() && LeftAddress.byteOffset() == RightAddress.byteOffset();
      }
      case ValueKind::GlobalVariableAddressOperand:
        return static_cast<const GlobalVariableAddressOperand &>(Left).global() == static_cast<const GlobalVariableAddressOperand &>(Right).global();
      }
      return false;
    }

    void verifyIntegerConstant(const IntegerConstant &Constant, const core::TargetContext &Target, const Name &FunctionName, DiagnosticCollector &Diagnostics)
    {
      const std::int64_t SignedValue = Constant.signedValue();
      const std::uint64_t UnsignedValue = Constant.unsignedValue();
      switch (Constant.type().kind())
      {
      case TypeKind::Bool:
        if (Constant.isNegative() || UnsignedValue > 1)
        {
          Diagnostics.add<DiagnosticKind::IrBoolConstantOutOfRange>(FunctionName, SignedValue);
        }
        return;
      case TypeKind::Byte:
        if (Constant.isNegative() || UnsignedValue > 255)
        {
          Diagnostics.add<DiagnosticKind::IrByteConstantOutOfRange>(FunctionName, SignedValue);
        }
        return;
      case TypeKind::I32:
        if ((Constant.isNegative() && SignedValue < std::numeric_limits<std::int32_t>::min()) || (!Constant.isNegative() && UnsignedValue > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())))
        {
          Diagnostics.add<DiagnosticKind::IrI32ConstantOutOfRange>(FunctionName, SignedValue);
        }
        return;
      case TypeKind::PointerSize:
      {
        const std::uint64_t MaximumValue = Target.maximumPointerSizeValue();
        if (Constant.isNegative())
        {
          Diagnostics.add<DiagnosticKind::IrPointerSizeConstantNegative>(FunctionName, SignedValue);
        }
        else if (UnsignedValue > MaximumValue)
        {
          Diagnostics.add<DiagnosticKind::IrPointerSizeConstantOutOfRange>(FunctionName, UnsignedValue, MaximumValue);
        }
        return;
      }
      case TypeKind::Void:
      case TypeKind::BytePointer:
      case TypeKind::ConstBytePointer:
      case TypeKind::ByteSlice:
      case TypeKind::ConstByteSlice:
      case TypeKind::F16:
      case TypeKind::F32:
      case TypeKind::F64:
      case TypeKind::Struct:
        Diagnostics.add<DiagnosticKind::IrIntegerConstantInvalidType>(FunctionName, typeKindName(Constant.type().kind()));
        return;
      case TypeKind::Count:
        break;
      }
      Diagnostics.add<DiagnosticKind::IrUnknownIntegerConstantType>(FunctionName);
    }

    void verifyFloatConstant(const FloatConstant &Constant, const Name &FunctionName, DiagnosticCollector &Diagnostics)
    {
      if (!isFloatingPointType(Constant.type().kind()))
      {
        Diagnostics.add<DiagnosticKind::IrFloatConstantInvalidType>(FunctionName, typeKindName(Constant.type().kind()));
        return;
      }
      const std::size_t FormatBitWidth = floatFormatBitWidth(Constant.format());
      if (FormatBitWidth == 0)
      {
        Diagnostics.add<DiagnosticKind::IrFloatConstantUnknownFormat>(FunctionName);
        return;
      }
      if (FormatBitWidth != floatingPointBitWidth(Constant.type().kind()))
      {
        Diagnostics.add<DiagnosticKind::IrFloatConstantFormatMismatch>(FunctionName, floatFormatName(Constant.format()), typeKindName(Constant.type().kind()));
        return;
      }
      if (FormatBitWidth < 64 && Constant.bitPattern() >= (std::uint64_t{1} << FormatBitWidth))
      {
        Diagnostics.add<DiagnosticKind::IrFloatConstantBitPatternOutOfRange>(FunctionName, floatFormatName(Constant.format()));
      }
    }

    void verifyStringConstant(const StringConstant &Constant, const core::TargetContext &Target, const Name &FunctionName, DiagnosticCollector &Diagnostics)
    {
      if (Constant.type().kind() != TypeKind::ConstByteSlice)
      {
        Diagnostics.add<DiagnosticKind::IrStringConstantInvalidType>(FunctionName, typeKindName(Constant.type().kind()));
        return;
      }
      if (Constant.data().size() > Target.maximumPointerSizeValue())
      {
        Diagnostics.add<DiagnosticKind::IrStringConstantTooLarge>(FunctionName, Constant.data().size(), Target.maximumPointerSizeValue());
      }
    }

    void verifyNullConstant(const NullConstant &Constant, const Name &FunctionName, DiagnosticCollector &Diagnostics)
    {
      if (Constant.type().kind() != TypeKind::BytePointer && Constant.type().kind() != TypeKind::ConstBytePointer)
      {
        Diagnostics.add<DiagnosticKind::IrNullConstantInvalidType>(FunctionName, typeKindName(Constant.type().kind()));
      }
    }

    void verifyConstant(const Module &ModuleValue, const Constant &ConstantValue, const Name &OwnerName, DiagnosticCollector &Diagnostics)
    {
      if (!isValidType(ModuleValue, &ConstantValue.type()) || ConstantValue.type().kind() == TypeKind::Void)
      {
        Diagnostics.add<DiagnosticKind::IrOperandInvalidType>(OwnerName);
        return;
      }
      if (!ModuleValue.context().constantPool().owns(ConstantValue))
      {
        Diagnostics.add<DiagnosticKind::IrConstantPoolMismatch>(OwnerName);
        return;
      }
      switch (ConstantValue.kind())
      {
      case ValueKind::IntegerConstant:
        verifyIntegerConstant(static_cast<const IntegerConstant &>(ConstantValue), ModuleValue.context().compilationContext().targetContext(), OwnerName, Diagnostics);
        return;
      case ValueKind::FloatConstant:
        verifyFloatConstant(static_cast<const FloatConstant &>(ConstantValue), OwnerName, Diagnostics);
        return;
      case ValueKind::StringConstant:
        verifyStringConstant(static_cast<const StringConstant &>(ConstantValue), ModuleValue.context().compilationContext().targetContext(), OwnerName, Diagnostics);
        return;
      case ValueKind::NullConstant:
        verifyNullConstant(static_cast<const NullConstant &>(ConstantValue), OwnerName, Diagnostics);
        return;
      case ValueKind::ZeroInitializer:
        return;
      case ValueKind::AggregateConstant:
      {
        const AggregateConstant &Aggregate = static_cast<const AggregateConstant &>(ConstantValue);
        if (Aggregate.type().kind() != TypeKind::Struct)
        {
          Diagnostics.add<DiagnosticKind::IrAggregateConstantInvalidType>(OwnerName);
          return;
        }
        const StructType &Struct = static_cast<const StructType &>(Aggregate.type());
        if (Aggregate.elements().size() != Struct.fieldCount())
        {
          Diagnostics.add<DiagnosticKind::IrAggregateConstantFieldCountMismatch>(OwnerName, Struct.fieldCount(), Aggregate.elements().size());
        }
        for (std::size_t ElementIndex = 0; ElementIndex < Aggregate.elements().size(); ++ElementIndex)
        {
          const Constant &Element = Aggregate.elements()[ElementIndex].get();
          if (ElementIndex < Struct.fieldCount() && &Element.type() != Struct.fieldType(ElementIndex))
          {
            Diagnostics.add<DiagnosticKind::IrAggregateConstantElementTypeMismatch>(OwnerName, ElementIndex);
          }
          verifyConstant(ModuleValue, Element, OwnerName, Diagnostics);
        }
        return;
      }
      case ValueKind::ValueOperand:
      case ValueKind::GlobalAddressOperand:
      case ValueKind::GlobalVariableAddressOperand:
        Diagnostics.add<DiagnosticKind::IrUnknownOperandKind>(OwnerName);
        return;
      }
    }

    struct FunctionAttributeDiagnosticReporter
    {
        const Name &FunctionName;
        DiagnosticCollector &Diagnostics;

        void unknownAttribute() const
        {
          Diagnostics.add<DiagnosticKind::IrUnknownFunctionAttribute>(FunctionName);
        }

        void invalidArgumentName(const Attribute &AttributeValue, const AttributeArgument &Argument) const
        {
          Diagnostics.add<DiagnosticKind::IrInvalidFunctionAttributeArgumentName>(FunctionName, attributeKindSpelling(AttributeValue.kind()), Argument.key());
        }

        void duplicateArgumentName(const Attribute &AttributeValue, const AttributeArgument &Argument) const
        {
          Diagnostics.add<DiagnosticKind::IrDuplicateFunctionAttributeArgumentName>(FunctionName, attributeKindSpelling(AttributeValue.kind()), Argument.key());
        }
    };

    struct StructFieldAttributeDiagnosticReporter
    {
        const Name &TypeName;
        std::size_t FieldIndex;
        DiagnosticCollector &Diagnostics;

        void unknownAttribute() const
        {
          Diagnostics.add<DiagnosticKind::IrUnknownStructFieldAttribute>(TypeName, FieldIndex);
        }

        void invalidArgumentName(const Attribute &AttributeValue, const AttributeArgument &Argument) const
        {
          Diagnostics.add<DiagnosticKind::IrInvalidStructFieldAttributeArgumentName>(TypeName, FieldIndex, attributeKindSpelling(AttributeValue.kind()), Argument.key());
        }

        void duplicateArgumentName(const Attribute &AttributeValue, const AttributeArgument &Argument) const
        {
          Diagnostics.add<DiagnosticKind::IrDuplicateStructFieldAttributeArgumentName>(TypeName, FieldIndex, attributeKindSpelling(AttributeValue.kind()), Argument.key());
        }
    };

    template <typename DiagnosticReporter>
    void verifyAttributes(const Module &ModuleValue, const std::vector<Attribute> &Attributes, const Name &OwnerName, DiagnosticCollector &Diagnostics, const DiagnosticReporter &Reporter)
    {
      for (const Attribute &AttributeValue : Attributes)
      {
        if (AttributeValue.kind() >= AttributeKind::Count)
        {
          Reporter.unknownAttribute();
          continue;
        }
        std::unordered_set<Name> ArgumentNames;
        for (const AttributeArgument &Argument : AttributeValue.arguments())
        {
          if (!Argument.key().valid())
          {
            Reporter.invalidArgumentName(AttributeValue, Argument);
          }
          else if (!ArgumentNames.insert(Argument.key()).second)
          {
            Reporter.duplicateArgumentName(AttributeValue, Argument);
          }
          verifyConstant(ModuleValue, Argument.value(), OwnerName, Diagnostics);
        }
      }
    }

    bool definitionDominatesUse(const SsaDefinition &Definition, std::size_t UseBlockIndex, std::size_t UseInstructionIndex, const FunctionControlFlow &ControlFlow)
    {
      if (Definition.BlockIndex == InvalidId)
      {
        return true;
      }
      if (Definition.BlockIndex == UseBlockIndex)
      {
        return Definition.InstructionIndex < UseInstructionIndex;
      }
      if (UseBlockIndex >= ControlFlow.Reachable.size())
      {
        return false;
      }
      if (Definition.BlockIndex >= ControlFlow.Reachable.size() || !ControlFlow.Reachable[Definition.BlockIndex])
      {
        return false;
      }
      return ControlFlow.Dominators[UseBlockIndex][Definition.BlockIndex];
    }

    void verifyOperand(const Module &ModuleValue, const Value &OperandValue, const SsaDefinitionMap &Definitions, const FunctionControlFlow &ControlFlow, std::size_t UseBlockIndex, std::size_t UseInstructionIndex, const Name &FunctionName, DiagnosticCollector &Diagnostics)
    {
      if (isConstantKind(OperandValue.kind()))
      {
        verifyConstant(ModuleValue, static_cast<const Constant &>(OperandValue), FunctionName, Diagnostics);
        return;
      }
      if (!isValidType(ModuleValue, &OperandValue.type()) || OperandValue.type().kind() == TypeKind::Void)
      {
        Diagnostics.add<DiagnosticKind::IrOperandInvalidType>(FunctionName);
        return;
      }

      if (OperandValue.kind() == ValueKind::ValueOperand)
      {
        const ValueId Id = static_cast<const ValueOperand &>(OperandValue).id();
        const auto Definition = Definitions.find(Id.value());
        if (!Id.valid() || Definition == Definitions.end() || !definitionDominatesUse(Definition->second, UseBlockIndex, UseInstructionIndex, ControlFlow))
        {
          Diagnostics.add<DiagnosticKind::IrUnavailableSsaValue>(FunctionName, Id.value());
          return;
        }
        if (Definition->second.DefinitionType != &OperandValue.type())
        {
          Diagnostics.add<DiagnosticKind::IrSsaOperandTypeMismatch>(FunctionName, Id.value());
        }
        return;
      }

      if (OperandValue.kind() == ValueKind::GlobalVariableAddressOperand)
      {
        const GlobalId Global = static_cast<const GlobalVariableAddressOperand &>(OperandValue).global();
        if (!isBytePointerType(&OperandValue.type()) || !Global.valid())
        {
          Diagnostics.add<DiagnosticKind::InvalidIrModule>();
          return;
        }
        if (Global.value() >= ModuleValue.Globals.size())
        {
          Diagnostics.add<DiagnosticKind::InvalidIrModule>();
          return;
        }
        const GlobalVariable &Variable = ModuleValue.Globals[Global.value()];
        const bool IsInitializer = ModuleValue.Initializer.has_value() && ModuleValue.Initializer->valid() && ModuleValue.Initializer->value() < ModuleValue.Functions.size() && ModuleValue.Functions[ModuleValue.Initializer->value()].Name == FunctionName;
        if (!Variable.Mutable && OperandValue.type().kind() == TypeKind::BytePointer && (Variable.Kind == GlobalVariableKind::Imported || !IsInitializer))
        {
          Diagnostics.add<DiagnosticKind::InvalidIrModule>();
        }
        return;
      }

      if (OperandValue.kind() != ValueKind::GlobalAddressOperand)
      {
        Diagnostics.add<DiagnosticKind::IrUnknownOperandKind>(FunctionName);
        return;
      }

      const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(OperandValue);
      if (OperandValue.type().kind() != TypeKind::ConstBytePointer)
      {
        Diagnostics.add<DiagnosticKind::IrGlobalAddressWrongType>(FunctionName);
        return;
      }
      if (!Address.global().valid() || Address.global().value() >= ModuleValue.ByteConstants.size())
      {
        Diagnostics.add<DiagnosticKind::IrInvalidGlobalByteConstantReference>(FunctionName, Address.global().value());
        return;
      }
      if (Address.byteOffset() > ModuleValue.ByteConstants[Address.global().value()].Data.size())
      {
        Diagnostics.add<DiagnosticKind::IrGlobalAddressOffsetOutOfRange>(FunctionName, Address.byteOffset(), ModuleValue.ByteConstants[Address.global().value()].Data.size());
      }
    }

    void verifyFunctionSignature(const Module &ModuleValue, const Function &FunctionValue, DiagnosticCollector &Diagnostics)
    {
      if (!isValidType(ModuleValue, FunctionValue.ResultType))
      {
        Diagnostics.add<DiagnosticKind::IrFunctionUnknownResultType>(FunctionValue.Name);
      }
      else if (isByteSliceType(FunctionValue.ResultType))
      {
        Diagnostics.add<DiagnosticKind::IrSliceFunctionResultForbidden>(FunctionValue.Name, typeKindName(FunctionValue.ResultType->kind()));
      }
      std::unordered_set<Name> ParameterNames;
      for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.parameterCount(); ++ParameterIndex)
      {
        const Parameter &ParameterValue = FunctionValue.parameter(ParameterIndex);
        const Type *ParameterType = ParameterValue.type();
        if (!ParameterValue.name().empty() && !ParameterValue.name().valid())
        {
          Diagnostics.add<DiagnosticKind::IrInvalidFunctionParameterName>(FunctionValue.Name, ParameterIndex, ParameterValue.name());
        }
        else if (!ParameterValue.name().empty() && !ParameterNames.insert(ParameterValue.name()).second)
        {
          Diagnostics.add<DiagnosticKind::IrDuplicateFunctionParameterName>(FunctionValue.Name, ParameterValue.name());
        }
        if (!isValidType(ModuleValue, ParameterType) || ParameterType->kind() == TypeKind::Void)
        {
          Diagnostics.add<DiagnosticKind::IrFunctionInvalidParameterType>(FunctionValue.Name, ParameterIndex);
        }
        else if (FunctionValue.Kind == FunctionKind::External && isByteSliceType(ParameterType))
        {
          Diagnostics.add<DiagnosticKind::IrSliceInExternalSignature>(FunctionValue.Name, typeKindName(ParameterType->kind()));
        }
        if (ParameterValue.defaultValue() != nullptr)
        {
          if (ParameterType == nullptr || &ParameterValue.defaultValue()->type() != ParameterType)
          {
            Diagnostics.add<DiagnosticKind::IrFunctionParameterDefaultTypeMismatch>(FunctionValue.Name, ParameterIndex);
          }
          verifyConstant(ModuleValue, *ParameterValue.defaultValue(), FunctionValue.Name, Diagnostics);
        }
      }
      const FunctionAttributeDiagnosticReporter AttributeReporter{FunctionValue.Name, Diagnostics};
      verifyAttributes(ModuleValue, FunctionValue.Attributes, FunctionValue.Name, Diagnostics, AttributeReporter);
      if (FunctionValue.Kind != FunctionKind::Definition && FunctionValue.Kind != FunctionKind::Imported && FunctionValue.Kind != FunctionKind::External)
      {
        Diagnostics.add<DiagnosticKind::IrFunctionUnknownKind>(FunctionValue.Name);
        return;
      }
      if (FunctionValue.Kind != FunctionKind::Imported && FunctionValue.Import.has_value())
      {
        Diagnostics.add<DiagnosticKind::InvalidIrModule>();
      }
      if (FunctionValue.Kind == FunctionKind::External)
      {
        if (FunctionValue.Convention != CallingConvention::C)
        {
          Diagnostics.add<DiagnosticKind::IrExternalFunctionWrongCallingConvention>(FunctionValue.Name);
        }
        if (!FunctionValue.Blocks.empty())
        {
          Diagnostics.add<DiagnosticKind::IrExternalFunctionHasBasicBlocks>(FunctionValue.Name);
        }
      }
      else if (FunctionValue.Kind == FunctionKind::Imported)
      {
        if (FunctionValue.Convention != CallingConvention::Ink)
        {
          Diagnostics.add<DiagnosticKind::IrImportedFunctionWrongCallingConvention>(FunctionValue.Name);
        }
        if (!FunctionValue.Blocks.empty())
        {
          Diagnostics.add<DiagnosticKind::IrImportedFunctionHasBasicBlocks>(FunctionValue.Name);
        }
        if (!FunctionValue.Import.has_value())
        {
          Diagnostics.add<DiagnosticKind::IrImportedFunctionInvalidModule>(FunctionValue.Name);
          Diagnostics.add<DiagnosticKind::IrImportedFunctionInvalidTargetName>(FunctionValue.Name, ink::ir::Name{});
        }
        else
        {
          if (!isValidModuleName(FunctionValue.Import->Module) || (ModuleValue.Name.has_value() && FunctionValue.Import->Module == *ModuleValue.Name))
          {
            Diagnostics.add<DiagnosticKind::IrImportedFunctionInvalidModule>(FunctionValue.Name);
          }
          if (!FunctionValue.Import->Symbol.valid())
          {
            Diagnostics.add<DiagnosticKind::IrImportedFunctionInvalidTargetName>(FunctionValue.Name, FunctionValue.Import->Symbol);
          }
        }
      }
      else
      {
        if (FunctionValue.Convention != CallingConvention::Ink)
        {
          Diagnostics.add<DiagnosticKind::IrDefinedFunctionWrongCallingConvention>(FunctionValue.Name);
        }
        if (FunctionValue.hasAttribute(AttributeKind::SideEffect))
        {
          Diagnostics.add<DiagnosticKind::IrDefinedFunctionHasExternalSideEffects>(FunctionValue.Name);
        }
        if (FunctionValue.Blocks.empty())
        {
          Diagnostics.add<DiagnosticKind::IrDefinedFunctionHasNoBasicBlocks>(FunctionValue.Name);
        }
      }
    }

    bool verifySsaResult(ValueId Id, const char *Operation, const Name &FunctionName, std::size_t &ExpectedValueId, std::unordered_set<std::size_t> &DefinedValues, DiagnosticCollector &Diagnostics)
    {
      const bool IsValidAndUnique = Id.valid() && DefinedValues.insert(Id.value()).second;
      if (!IsValidAndUnique)
      {
        Diagnostics.add<DiagnosticKind::IrInvalidOrDuplicateSsaResult>(Operation, FunctionName, Id.value());
        ++ExpectedValueId;
        return false;
      }
      if (Id.value() != ExpectedValueId)
      {
        Diagnostics.add<DiagnosticKind::IrNonConsecutiveSsaResult>(Operation, FunctionName, ExpectedValueId, Id.value());
      }
      ++ExpectedValueId;
      return IsValidAndUnique;
    }

    SsaDefinitionMap collectSsaDefinitions(const Function &FunctionValue, DiagnosticCollector &Diagnostics)
    {
      SsaDefinitionMap Definitions;
      std::unordered_set<std::size_t> DefinedValues;
      for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.parameterCount(); ++ParameterIndex)
      {
        DefinedValues.insert(ParameterIndex);
        Definitions.emplace(ParameterIndex, SsaDefinition{FunctionValue.parameterType(ParameterIndex), InvalidId, 0});
      }
      std::size_t ExpectedValueId = FunctionValue.parameterCount();
      for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
      {
        const BasicBlock &Block = FunctionValue.Blocks[BlockIndex];
        for (std::size_t InstructionIndex = 0; InstructionIndex < Block.Instructions.size(); ++InstructionIndex)
        {
          const std::unique_ptr<Instruction> &InstructionPointer = Block.Instructions[InstructionIndex];
          if (!InstructionPointer)
          {
            continue;
          }
          const std::optional<InstructionResult> Result = instructionResult(*InstructionPointer);
          if (!Result.has_value())
          {
            continue;
          }
          if (verifySsaResult(Result->Id, Result->Operation, FunctionValue.Name, ExpectedValueId, DefinedValues, Diagnostics))
          {
            Definitions.emplace(Result->Id.value(), SsaDefinition{Result->ResultType, BlockIndex, InstructionIndex});
          }
        }
      }
      return Definitions;
    }

    bool validControlFlowTarget(const Function &FunctionValue, const BlockTarget &Target)
    {
      return Target.Block.valid() && Target.Block.value() != 0 && Target.Block.value() < FunctionValue.Blocks.size();
    }

    void addControlFlowEdge(std::size_t SourceBlock, const BlockTarget &Target, const Function &FunctionValue, FunctionControlFlow &ControlFlow)
    {
      if (!validControlFlowTarget(FunctionValue, Target))
      {
        return;
      }
      ControlFlow.Successors[SourceBlock].push_back(Target.Block.value());
      ControlFlow.Predecessors[Target.Block.value()].push_back(SourceBlock);
    }

    FunctionControlFlow buildControlFlow(const Function &FunctionValue)
    {
      FunctionControlFlow Result;
      Result.Predecessors.resize(FunctionValue.Blocks.size());
      Result.Successors.resize(FunctionValue.Blocks.size());
      for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
      {
        const BasicBlock &Block = FunctionValue.Blocks[BlockIndex];
        if (Block.Instructions.empty() || !Block.Instructions.back())
        {
          continue;
        }
        const Instruction &Terminator = *Block.Instructions.back();
        if (Terminator.kind() == InstructionKind::Branch)
        {
          addControlFlowEdge(BlockIndex, static_cast<const BranchInstruction &>(Terminator).Target, FunctionValue, Result);
        }
        else if (Terminator.kind() == InstructionKind::ConditionalBranch)
        {
          const ConditionalBranchInstruction &Branch = static_cast<const ConditionalBranchInstruction &>(Terminator);
          addControlFlowEdge(BlockIndex, Branch.TrueTarget, FunctionValue, Result);
          addControlFlowEdge(BlockIndex, Branch.FalseTarget, FunctionValue, Result);
        }
      }

      Result.Reachable.assign(FunctionValue.Blocks.size(), false);
      if (!FunctionValue.Blocks.empty())
      {
        std::vector<std::size_t> Worklist{0};
        Result.Reachable[0] = true;
        while (!Worklist.empty())
        {
          const std::size_t BlockIndex = Worklist.back();
          Worklist.pop_back();
          for (const std::size_t Successor : Result.Successors[BlockIndex])
          {
            if (!Result.Reachable[Successor])
            {
              Result.Reachable[Successor] = true;
              Worklist.push_back(Successor);
            }
          }
        }
      }

      const std::size_t BlockCount = FunctionValue.Blocks.size();
      Result.Dominators.assign(BlockCount, std::vector<bool>(BlockCount, false));
      if (BlockCount == 0)
      {
        return Result;
      }
      Result.Dominators[0][0] = true;
      for (std::size_t BlockIndex = 1; BlockIndex < BlockCount; ++BlockIndex)
      {
        if (!Result.Reachable[BlockIndex])
        {
          continue;
        }
        for (std::size_t Candidate = 0; Candidate < BlockCount; ++Candidate)
        {
          Result.Dominators[BlockIndex][Candidate] = Result.Reachable[Candidate];
        }
      }
      bool Changed = true;
      while (Changed)
      {
        Changed = false;
        for (std::size_t BlockIndex = 1; BlockIndex < BlockCount; ++BlockIndex)
        {
          if (!Result.Reachable[BlockIndex])
          {
            continue;
          }
          std::vector<bool> NewDominators(BlockCount, false);
          bool HasReachablePredecessor = false;
          for (const std::size_t Predecessor : Result.Predecessors[BlockIndex])
          {
            if (!Result.Reachable[Predecessor])
            {
              continue;
            }
            if (!HasReachablePredecessor)
            {
              NewDominators = Result.Dominators[Predecessor];
              HasReachablePredecessor = true;
            }
            else
            {
              for (std::size_t Candidate = 0; Candidate < BlockCount; ++Candidate)
              {
                NewDominators[Candidate] = NewDominators[Candidate] && Result.Dominators[Predecessor][Candidate];
              }
            }
          }
          NewDominators[BlockIndex] = true;
          if (NewDominators != Result.Dominators[BlockIndex])
          {
            Result.Dominators[BlockIndex] = std::move(NewDominators);
            Changed = true;
          }
        }
      }
      return Result;
    }

    void verifyBlockTarget(const Function &FunctionValue, const BlockTarget &Target, DiagnosticCollector &Diagnostics)
    {
      if (!Target.Block.valid() || Target.Block.value() >= FunctionValue.Blocks.size())
      {
        Diagnostics.add<DiagnosticKind::IrBranchInvalidTarget>(FunctionValue.Name);
        return;
      }
      if (Target.Block.value() == 0)
      {
        Diagnostics.add<DiagnosticKind::IrBranchTargetsEntryBlock>(FunctionValue.Name);
      }
    }

    void verifyPhiInstruction(const Module &ModuleValue, const Function &FunctionValue, std::size_t BlockIndex, const PhiInstruction &Phi, const SsaDefinitionMap &Definitions, const FunctionControlFlow &ControlFlow, DiagnosticCollector &Diagnostics)
    {
      const BasicBlock &Block = FunctionValue.Blocks[BlockIndex];
      if (!isValidType(ModuleValue, Phi.ResultType) || Phi.ResultType->kind() == TypeKind::Void)
      {
        Diagnostics.add<DiagnosticKind::IrPhiInvalidResultType>(FunctionValue.Name, Block.Name);
      }
      if (BlockIndex == 0)
      {
        Diagnostics.add<DiagnosticKind::IrPhiInEntryBlock>(FunctionValue.Name, Block.Name);
      }
      if (Phi.IncomingValues.empty())
      {
        Diagnostics.add<DiagnosticKind::IrPhiHasNoIncomingValues>(FunctionValue.Name, Block.Name);
      }
      const std::vector<std::size_t> &Predecessors = ControlFlow.Predecessors[BlockIndex];
      if (Phi.IncomingValues.size() != Predecessors.size())
      {
        Diagnostics.add<DiagnosticKind::IrPhiIncomingCountMismatch>(FunctionValue.Name, Block.Name, Predecessors.size(), Phi.IncomingValues.size());
      }

      std::vector<std::size_t> ExpectedCounts(FunctionValue.Blocks.size(), 0);
      std::vector<std::size_t> SeenCounts(FunctionValue.Blocks.size(), 0);
      std::vector<const Value *> FirstIncomingValues(FunctionValue.Blocks.size(), nullptr);
      for (const std::size_t Predecessor : Predecessors)
      {
        ++ExpectedCounts[Predecessor];
      }
      for (std::size_t IncomingIndex = 0; IncomingIndex < Phi.IncomingValues.size(); ++IncomingIndex)
      {
        const PhiIncoming &Incoming = Phi.IncomingValues[IncomingIndex];
        if (!Incoming.Value)
        {
          Diagnostics.add<DiagnosticKind::IrPhiNullIncomingValue>(FunctionValue.Name, Block.Name, IncomingIndex);
        }
        else if (&Incoming.Value->type() != Phi.ResultType)
        {
          Diagnostics.add<DiagnosticKind::IrPhiIncomingTypeMismatch>(FunctionValue.Name, Block.Name, IncomingIndex);
        }

        if (!Incoming.Predecessor.valid() || Incoming.Predecessor.value() >= FunctionValue.Blocks.size())
        {
          Diagnostics.add<DiagnosticKind::IrPhiInvalidIncomingBlock>(FunctionValue.Name, Block.Name, IncomingIndex);
          continue;
        }
        const std::size_t Predecessor = Incoming.Predecessor.value();
        ++SeenCounts[Predecessor];
        if (SeenCounts[Predecessor] > ExpectedCounts[Predecessor])
        {
          Diagnostics.add<DiagnosticKind::IrPhiIncomingBlockNotPredecessor>(FunctionValue.Name, Block.Name, IncomingIndex);
        }
        if (Incoming.Value)
        {
          if (FirstIncomingValues[Predecessor] != nullptr && !sameValue(*FirstIncomingValues[Predecessor], *Incoming.Value))
          {
            Diagnostics.add<DiagnosticKind::IrPhiDuplicateIncomingBlock>(FunctionValue.Name, Block.Name, IncomingIndex);
          }
          else if (FirstIncomingValues[Predecessor] == nullptr)
          {
            FirstIncomingValues[Predecessor] = Incoming.Value.get();
          }
          const std::size_t UseInstructionIndex = FunctionValue.Blocks[Predecessor].Instructions.size();
          verifyOperand(ModuleValue, *Incoming.Value, Definitions, ControlFlow, Predecessor, UseInstructionIndex, FunctionValue.Name, Diagnostics);
        }
      }
    }

    void verifyFunctionBody(const Module &ModuleValue, const Function &FunctionValue, DiagnosticCollector &Diagnostics)
    {
      if (FunctionValue.Kind != FunctionKind::Definition)
      {
        return;
      }
      if (!isValidType(ModuleValue, FunctionValue.ResultType))
      {
        return;
      }

      const SsaDefinitionMap Definitions = collectSsaDefinitions(FunctionValue, Diagnostics);
      const FunctionControlFlow ControlFlow = buildControlFlow(FunctionValue);
      std::unordered_set<Name> BlockNames;
      const bool IsFinalizer = ModuleValue.Finalizer.has_value() && ModuleValue.Finalizer->valid() && ModuleValue.Finalizer->value() < ModuleValue.Functions.size() && &ModuleValue.Functions[ModuleValue.Finalizer->value()] == &FunctionValue;

      for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
      {
        const BasicBlock &Block = FunctionValue.Blocks[BlockIndex];
        if (!Block.Name.valid())
        {
          Diagnostics.add<DiagnosticKind::IrInvalidBasicBlockName>(FunctionValue.Name, Block.Name);
        }
        else if (!BlockNames.insert(Block.Name).second)
        {
          Diagnostics.add<DiagnosticKind::IrDuplicateBasicBlockName>(FunctionValue.Name, Block.Name);
        }
        if (Block.Instructions.empty())
        {
          Diagnostics.add<DiagnosticKind::IrEmptyBasicBlock>(FunctionValue.Name, Block.Name);
          continue;
        }

        bool SeenNonPhi = false;
        for (std::size_t InstructionIndex = 0; InstructionIndex < Block.Instructions.size(); ++InstructionIndex)
        {
          const std::unique_ptr<Instruction> &InstructionPointer = Block.Instructions[InstructionIndex];
          if (!InstructionPointer)
          {
            Diagnostics.add<DiagnosticKind::IrNullInstruction>(FunctionValue.Name, Block.Name);
            SeenNonPhi = true;
            continue;
          }
          const Instruction &InstructionValue = *InstructionPointer;
          const bool IsLast = InstructionIndex + 1 == Block.Instructions.size();
          const InstructionKind Kind = InstructionValue.kind();
          if (Kind != InstructionKind::Call && Kind != InstructionKind::Import && Kind != InstructionKind::Alloca && Kind != InstructionKind::GetElementPointer && Kind != InstructionKind::Load && Kind != InstructionKind::Store && Kind != InstructionKind::LifetimeEnd && Kind != InstructionKind::SliceData && Kind != InstructionKind::SliceLength && Kind != InstructionKind::Phi && Kind != InstructionKind::Add && Kind != InstructionKind::Compare && Kind != InstructionKind::InsertValue && Kind != InstructionKind::ExtractValue && Kind != InstructionKind::Branch && Kind != InstructionKind::ConditionalBranch && Kind != InstructionKind::Return)
          {
            Diagnostics.add<DiagnosticKind::IrUnknownInstructionKind>(FunctionValue.Name, Block.Name);
            SeenNonPhi = true;
            continue;
          }
          if (Kind == InstructionKind::Phi)
          {
            if (SeenNonPhi)
            {
              Diagnostics.add<DiagnosticKind::IrPhiMustBeFirstInBlock>(FunctionValue.Name, Block.Name);
            }
          }
          else
          {
            SeenNonPhi = true;
          }
          if (isTerminator(Kind) != IsLast)
          {
            if (IsLast)
            {
              Diagnostics.add<DiagnosticKind::IrBlockMissingTerminator>(FunctionValue.Name, Block.Name);
            }
            else
            {
              Diagnostics.add<DiagnosticKind::IrEarlyTerminator>(FunctionValue.Name, Block.Name);
            }
          }

          if (Kind == InstructionKind::Phi)
          {
            verifyPhiInstruction(ModuleValue, FunctionValue, BlockIndex, static_cast<const PhiInstruction &>(InstructionValue), Definitions, ControlFlow, Diagnostics);
            continue;
          }

          if (Kind == InstructionKind::Import)
          {
            const ImportInstruction &Import = static_cast<const ImportInstruction &>(InstructionValue);
            if (!isValidModuleName(Import.Module) || (ModuleValue.Name.has_value() && Import.Module == *ModuleValue.Name) || IsFinalizer)
            {
              Diagnostics.add<DiagnosticKind::InvalidIrModule>();
            }
            continue;
          }

          if (Kind == InstructionKind::Alloca)
          {
            const AllocaInstruction &Alloca = static_cast<const AllocaInstruction &>(InstructionValue);
            if (!isValidType(ModuleValue, Alloca.ResultType) || Alloca.ResultType->kind() != TypeKind::ByteSlice)
            {
              Diagnostics.add<DiagnosticKind::IrAllocaInvalidResultType>(FunctionValue.Name);
            }
            if (BlockIndex != 0)
            {
              Diagnostics.add<DiagnosticKind::IrAllocaOutsideEntryBlock>(FunctionValue.Name);
            }
            if (!Alloca.Size)
            {
              Diagnostics.add<DiagnosticKind::IrAllocaNullSize>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Alloca.Size, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (Alloca.Size->type().kind() != TypeKind::PointerSize)
              {
                Diagnostics.add<DiagnosticKind::IrAllocaSizeNotPointerSize>(FunctionValue.Name);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::GetElementPointer)
          {
            const GetElementPointerInstruction &GetElementPointer = static_cast<const GetElementPointerInstruction &>(InstructionValue);
            if (!isValidType(ModuleValue, GetElementPointer.ResultType) || !isBytePointerType(GetElementPointer.ResultType))
            {
              Diagnostics.add<DiagnosticKind::IrGetElementPointerInvalidResultType>(FunctionValue.Name);
            }
            if (!isValidType(ModuleValue, GetElementPointer.ElementType) || !computeTypeLayout(*GetElementPointer.ElementType, ModuleValue.context().compilationContext().targetContext()).has_value())
            {
              Diagnostics.add<DiagnosticKind::IrGetElementPointerUnsupportedElementType>(FunctionValue.Name);
            }
            if (!GetElementPointer.Pointer)
            {
              Diagnostics.add<DiagnosticKind::IrGetElementPointerNullPointer>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *GetElementPointer.Pointer, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (!isBytePointerType(&GetElementPointer.Pointer->type()))
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerInvalidPointerType>(FunctionValue.Name);
              }
              if (GetElementPointer.ResultType != &GetElementPointer.Pointer->type())
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerResultTypeMismatch>(FunctionValue.Name);
              }
            }
            if (!GetElementPointer.Index)
            {
              Diagnostics.add<DiagnosticKind::IrGetElementPointerNullIndex>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *GetElementPointer.Index, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (GetElementPointer.Index->type().kind() != TypeKind::PointerSize)
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerIndexNotPointerSize>(FunctionValue.Name);
              }
            }
            const Type *IndexedType = isValidType(ModuleValue, GetElementPointer.ElementType) ? GetElementPointer.ElementType : nullptr;
            for (std::size_t FieldIndexPosition = 0; FieldIndexPosition < GetElementPointer.FieldIndices.size(); ++FieldIndexPosition)
            {
              const std::size_t PathIndex = FieldIndexPosition + 1;
              const ValueHandle &FieldIndexValue = GetElementPointer.FieldIndices[FieldIndexPosition];
              if (!FieldIndexValue)
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerNullFieldIndex>(FunctionValue.Name, PathIndex);
                IndexedType = nullptr;
                continue;
              }
              verifyOperand(ModuleValue, *FieldIndexValue, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (FieldIndexValue->type().kind() != TypeKind::I32)
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerFieldIndexNotI32>(FunctionValue.Name, PathIndex);
                IndexedType = nullptr;
                continue;
              }
              if (FieldIndexValue->kind() != ValueKind::IntegerConstant)
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerFieldIndexNotConstant>(FunctionValue.Name, PathIndex);
                IndexedType = nullptr;
                continue;
              }
              const IntegerConstant &FieldIndexConstant = static_cast<const IntegerConstant &>(*FieldIndexValue);
              if (FieldIndexConstant.isNegative())
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerFieldIndexNegative>(FunctionValue.Name, PathIndex, FieldIndexConstant.signedValue());
                IndexedType = nullptr;
                continue;
              }
              if (FieldIndexConstant.unsignedValue() > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max()))
              {
                IndexedType = nullptr;
                continue;
              }
              if (IndexedType == nullptr)
              {
                continue;
              }
              if (IndexedType->kind() != TypeKind::Struct)
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerFieldIndexIntoNonStruct>(FunctionValue.Name, PathIndex);
                IndexedType = nullptr;
                continue;
              }
              const StructType &Struct = static_cast<const StructType &>(*IndexedType);
              const std::size_t FieldIndex = static_cast<std::size_t>(FieldIndexConstant.unsignedValue());
              if (FieldIndex >= Struct.fieldCount())
              {
                Diagnostics.add<DiagnosticKind::IrGetElementPointerFieldIndexOutOfRange>(FunctionValue.Name, PathIndex, FieldIndex, Struct.fieldCount());
                IndexedType = nullptr;
                continue;
              }
              IndexedType = Struct.fieldType(FieldIndex);
            }
            continue;
          }

          if (Kind == InstructionKind::Load)
          {
            const LoadInstruction &Load = static_cast<const LoadInstruction &>(InstructionValue);
            if (!isValidType(ModuleValue, Load.ResultType) || !isMemoryValueType(*Load.ResultType))
            {
              Diagnostics.add<DiagnosticKind::IrLoadUnsupportedResultType>(FunctionValue.Name);
            }
            if (!Load.Pointer)
            {
              Diagnostics.add<DiagnosticKind::IrLoadNullPointer>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Load.Pointer, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (!isBytePointerType(&Load.Pointer->type()))
              {
                Diagnostics.add<DiagnosticKind::IrLoadInvalidPointerType>(FunctionValue.Name);
              }
              const GlobalVariable *Global = referencedGlobal(ModuleValue, *Load.Pointer);
              if (Global != nullptr && Load.ResultType != Global->ValueType)
              {
                Diagnostics.add<DiagnosticKind::InvalidIrModule>();
              }
            }
            continue;
          }

          if (Kind == InstructionKind::Store)
          {
            const StoreInstruction &Store = static_cast<const StoreInstruction &>(InstructionValue);
            if (!Store.StoredValue)
            {
              Diagnostics.add<DiagnosticKind::IrStoreNullValue>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Store.StoredValue, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (!isMemoryValueType(Store.StoredValue->type()))
              {
                Diagnostics.add<DiagnosticKind::IrStoreUnsupportedValueType>(FunctionValue.Name);
              }
            }
            if (!Store.Pointer)
            {
              Diagnostics.add<DiagnosticKind::IrStoreNullPointer>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Store.Pointer, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (Store.Pointer->type().kind() != TypeKind::BytePointer)
              {
                Diagnostics.add<DiagnosticKind::IrStoreDestinationNotMutablePointer>(FunctionValue.Name);
              }
              const GlobalVariable *Global = referencedGlobal(ModuleValue, *Store.Pointer);
              if (Global != nullptr && Store.StoredValue && &Store.StoredValue->type() != Global->ValueType)
              {
                Diagnostics.add<DiagnosticKind::InvalidIrModule>();
              }
            }
            continue;
          }

          if (Kind == InstructionKind::LifetimeEnd)
          {
            const LifetimeEndInstruction &LifetimeEnd = static_cast<const LifetimeEndInstruction &>(InstructionValue);
            if (!LifetimeEnd.Slice)
            {
              Diagnostics.add<DiagnosticKind::IrLifetimeEndNullSlice>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *LifetimeEnd.Slice, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (LifetimeEnd.Slice->type().kind() != TypeKind::ByteSlice)
              {
                Diagnostics.add<DiagnosticKind::IrLifetimeEndInvalidSliceType>(FunctionValue.Name);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::SliceData)
          {
            const SliceDataInstruction &SliceData = static_cast<const SliceDataInstruction &>(InstructionValue);
            const bool HasValidResultType = isValidType(ModuleValue, SliceData.ResultType) && (SliceData.ResultType->kind() == TypeKind::BytePointer || SliceData.ResultType->kind() == TypeKind::ConstBytePointer);
            if (!HasValidResultType)
            {
              Diagnostics.add<DiagnosticKind::IrSliceDataInvalidResultType>(FunctionValue.Name);
            }
            if (!SliceData.Slice)
            {
              Diagnostics.add<DiagnosticKind::IrSliceDataNullSlice>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *SliceData.Slice, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (!isByteSliceType(&SliceData.Slice->type()))
              {
                Diagnostics.add<DiagnosticKind::IrSliceDataInvalidSliceType>(FunctionValue.Name);
              }
              else if (SliceData.Slice->type().kind() == TypeKind::ConstByteSlice && HasValidResultType && SliceData.ResultType->kind() == TypeKind::BytePointer)
              {
                Diagnostics.add<DiagnosticKind::IrSliceDataDropsConst>(FunctionValue.Name);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::SliceLength)
          {
            const SliceLengthInstruction &SliceLength = static_cast<const SliceLengthInstruction &>(InstructionValue);
            if (!isValidType(ModuleValue, SliceLength.ResultType) || SliceLength.ResultType->kind() != TypeKind::PointerSize)
            {
              Diagnostics.add<DiagnosticKind::IrSliceLengthInvalidResultType>(FunctionValue.Name);
            }
            if (!SliceLength.Slice)
            {
              Diagnostics.add<DiagnosticKind::IrSliceLengthNullSlice>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *SliceLength.Slice, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (!isByteSliceType(&SliceLength.Slice->type()))
              {
                Diagnostics.add<DiagnosticKind::IrSliceLengthInvalidSliceType>(FunctionValue.Name);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::Add)
          {
            const AddInstruction &Add = static_cast<const AddInstruction &>(InstructionValue);
            if (!isValidType(ModuleValue, Add.ResultType) || !isAddType(Add.ResultType))
            {
              Diagnostics.add<DiagnosticKind::IrAddInvalidResultType>(FunctionValue.Name);
            }
            const Value *Operands[] = {Add.Left.get(), Add.Right.get()};
            for (std::size_t OperandIndex = 0; OperandIndex < 2; ++OperandIndex)
            {
              if (Operands[OperandIndex] == nullptr)
              {
                Diagnostics.add<DiagnosticKind::IrAddNullOperand>(FunctionValue.Name, OperandIndex);
                continue;
              }
              verifyOperand(ModuleValue, *Operands[OperandIndex], Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (&Operands[OperandIndex]->type() != Add.ResultType)
              {
                Diagnostics.add<DiagnosticKind::IrAddOperandTypeMismatch>(FunctionValue.Name, OperandIndex);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::Compare)
          {
            const CompareInstruction &Compare = static_cast<const CompareInstruction &>(InstructionValue);
            if (!isValidType(ModuleValue, Compare.ResultType) || Compare.ResultType->kind() != TypeKind::Bool)
            {
              Diagnostics.add<DiagnosticKind::IrCompareInvalidResultType>(FunctionValue.Name);
            }
            if (Compare.Predicate >= ComparePredicate::Count)
            {
              Diagnostics.add<DiagnosticKind::IrCompareInvalidPredicate>(FunctionValue.Name);
            }
            const Value *Operands[] = {Compare.Left.get(), Compare.Right.get()};
            for (std::size_t OperandIndex = 0; OperandIndex < 2; ++OperandIndex)
            {
              if (Operands[OperandIndex] == nullptr)
              {
                Diagnostics.add<DiagnosticKind::IrCompareNullOperand>(FunctionValue.Name, OperandIndex);
                continue;
              }
              verifyOperand(ModuleValue, *Operands[OperandIndex], Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
            }
            if (Compare.Left && Compare.Right)
            {
              if (&Compare.Left->type() != &Compare.Right->type())
              {
                Diagnostics.add<DiagnosticKind::IrCompareOperandTypeMismatch>(FunctionValue.Name);
              }
              else if (!isComparableType(&Compare.Left->type()))
              {
                Diagnostics.add<DiagnosticKind::IrCompareUnsupportedType>(FunctionValue.Name, typeKindName(Compare.Left->type().kind()));
              }
              else if ((Compare.Left->type().kind() == TypeKind::Bool || Compare.Left->type().kind() == TypeKind::BytePointer || Compare.Left->type().kind() == TypeKind::ConstBytePointer) && Compare.Predicate != ComparePredicate::Equal && Compare.Predicate != ComparePredicate::NotEqual && Compare.Predicate < ComparePredicate::Count)
              {
                Diagnostics.add<DiagnosticKind::IrComparePredicateUnsupportedForType>(FunctionValue.Name, comparePredicateName(Compare.Predicate), typeKindName(Compare.Left->type().kind()));
              }
            }
            continue;
          }

          if (Kind == InstructionKind::Call)
          {
            const CallInstruction *Call = static_cast<const CallInstruction *>(&InstructionValue);
            if (!Call->Callee.valid() || Call->Callee.value() >= ModuleValue.Functions.size())
            {
              Diagnostics.add<DiagnosticKind::IrCallInvalidCallee>(FunctionValue.Name);
              continue;
            }
            const Function *Callee = &ModuleValue.Functions[Call->Callee.value()];
            if ((ModuleValue.Initializer.has_value() && *ModuleValue.Initializer == Call->Callee) || (ModuleValue.Finalizer.has_value() && *ModuleValue.Finalizer == Call->Callee))
            {
              Diagnostics.add<DiagnosticKind::InvalidIrModule>();
            }
            const Name &CalleeName = Callee->Name;
            if (!isValidType(ModuleValue, Call->ResultType))
            {
              Diagnostics.add<DiagnosticKind::IrCallUnknownResultType>(CalleeName);
              continue;
            }
            if (Call->ResultType != Callee->ResultType)
            {
              Diagnostics.add<DiagnosticKind::IrCallResultTypeMismatch>(CalleeName);
            }
            if ((Call->ResultType->kind() == TypeKind::Void) == Call->Result.has_value())
            {
              if (Call->ResultType->kind() == TypeKind::Void)
              {
                Diagnostics.add<DiagnosticKind::IrVoidCallDefinesResult>(CalleeName);
              }
              else
              {
                Diagnostics.add<DiagnosticKind::IrNonVoidCallMissingResult>(CalleeName);
              }
            }
            if (Call->Arguments.size() != Callee->parameterCount())
            {
              Diagnostics.add<DiagnosticKind::IrCallArgumentCountMismatch>(CalleeName, Callee->parameterCount(), Call->Arguments.size());
            }
            const std::size_t CheckedArgumentCount = Call->Arguments.size() < Callee->parameterCount() ? Call->Arguments.size() : Callee->parameterCount();
            for (std::size_t ArgumentIndex = 0; ArgumentIndex < Call->Arguments.size(); ++ArgumentIndex)
            {
              if (!Call->Arguments[ArgumentIndex])
              {
                Diagnostics.add<DiagnosticKind::IrNullCallArgument>(CalleeName, ArgumentIndex);
                continue;
              }
              verifyOperand(ModuleValue, *Call->Arguments[ArgumentIndex], Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (ArgumentIndex < CheckedArgumentCount && &Call->Arguments[ArgumentIndex]->type() != Callee->parameterType(ArgumentIndex))
              {
                Diagnostics.add<DiagnosticKind::IrCallArgumentTypeMismatch>(CalleeName, ArgumentIndex);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::InsertValue)
          {
            const InsertValueInstruction &Insert = static_cast<const InsertValueInstruction &>(InstructionValue);
            const StructType *AggregateType = nullptr;
            if (!isValidType(ModuleValue, Insert.ResultType) || Insert.ResultType->kind() != TypeKind::Struct)
            {
              Diagnostics.add<DiagnosticKind::IrInsertResultMustBeStruct>(FunctionValue.Name);
            }
            else
            {
              AggregateType = static_cast<const StructType *>(Insert.ResultType);
            }
            if (!Insert.Aggregate)
            {
              Diagnostics.add<DiagnosticKind::IrInsertNullAggregate>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Insert.Aggregate, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (&Insert.Aggregate->type() != Insert.ResultType)
              {
                Diagnostics.add<DiagnosticKind::IrInsertAggregateTypeMismatch>(FunctionValue.Name);
              }
            }
            if (!Insert.Element)
            {
              Diagnostics.add<DiagnosticKind::IrInsertNullElement>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Insert.Element, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
            }
            if (AggregateType != nullptr)
            {
              if (Insert.FieldIndex >= AggregateType->fieldCount())
              {
                Diagnostics.add<DiagnosticKind::IrInsertFieldIndexOutOfRange>(FunctionValue.Name, Insert.FieldIndex, AggregateType->fieldCount());
              }
              else if (Insert.Element && &Insert.Element->type() != AggregateType->fieldType(Insert.FieldIndex))
              {
                Diagnostics.add<DiagnosticKind::IrInsertElementTypeMismatch>(FunctionValue.Name);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::ExtractValue)
          {
            const ExtractValueInstruction &Extract = static_cast<const ExtractValueInstruction &>(InstructionValue);
            const StructType *AggregateType = nullptr;
            if (!Extract.Aggregate)
            {
              Diagnostics.add<DiagnosticKind::IrExtractNullAggregate>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Extract.Aggregate, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (Extract.Aggregate->type().kind() != TypeKind::Struct)
              {
                Diagnostics.add<DiagnosticKind::IrExtractAggregateMustBeStruct>();
              }
              else
              {
                AggregateType = static_cast<const StructType *>(&Extract.Aggregate->type());
              }
            }
            if (!isValidType(ModuleValue, Extract.ResultType) || Extract.ResultType->kind() == TypeKind::Void)
            {
              Diagnostics.add<DiagnosticKind::IrExtractInvalidResultType>(FunctionValue.Name);
            }
            if (AggregateType != nullptr)
            {
              if (Extract.FieldIndex >= AggregateType->fieldCount())
              {
                Diagnostics.add<DiagnosticKind::IrExtractFieldIndexOutOfRange>(FunctionValue.Name, Extract.FieldIndex, AggregateType->fieldCount());
              }
              else if (Extract.ResultType != AggregateType->fieldType(Extract.FieldIndex))
              {
                Diagnostics.add<DiagnosticKind::IrExtractResultTypeMismatch>(FunctionValue.Name);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::Branch)
          {
            const BranchInstruction &Branch = static_cast<const BranchInstruction &>(InstructionValue);
            verifyBlockTarget(FunctionValue, Branch.Target, Diagnostics);
            continue;
          }

          if (Kind == InstructionKind::ConditionalBranch)
          {
            const ConditionalBranchInstruction &Branch = static_cast<const ConditionalBranchInstruction &>(InstructionValue);
            if (!Branch.Condition)
            {
              Diagnostics.add<DiagnosticKind::IrConditionalBranchNullCondition>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Branch.Condition, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
              if (Branch.Condition->type().kind() != TypeKind::Bool)
              {
                Diagnostics.add<DiagnosticKind::IrConditionalBranchConditionNotBool>(FunctionValue.Name);
              }
            }
            verifyBlockTarget(FunctionValue, Branch.TrueTarget, Diagnostics);
            verifyBlockTarget(FunctionValue, Branch.FalseTarget, Diagnostics);
            continue;
          }

          const ReturnInstruction &Return = static_cast<const ReturnInstruction &>(InstructionValue);
          if (FunctionValue.ResultType->kind() == TypeKind::Void)
          {
            if (Return.ReturnValue)
            {
              Diagnostics.add<DiagnosticKind::IrVoidFunctionReturnsValue>(FunctionValue.Name);
            }
            continue;
          }
          if (!Return.ReturnValue)
          {
            Diagnostics.add<DiagnosticKind::IrNonVoidFunctionMissingReturnValue>(FunctionValue.Name);
            continue;
          }
          verifyOperand(ModuleValue, *Return.ReturnValue, Definitions, ControlFlow, BlockIndex, InstructionIndex, FunctionValue.Name, Diagnostics);
          if (&Return.ReturnValue->type() != FunctionValue.ResultType)
          {
            Diagnostics.add<DiagnosticKind::IrReturnTypeMismatch>(FunctionValue.Name);
          }
        }
      }
    }
  } // namespace

  VerificationResult verify(IRContext &Context, const Module &ModuleValue)
  {
    return verify(Context, ModuleValue, DiagnosticClass::InternalCompilerError);
  }

  VerificationResult verify(IRContext &Context, const Module &ModuleValue, DiagnosticClass Class)
  {
    DiagnosticCollector Diagnostics(Class);
    std::unordered_set<Name> GlobalNames;
    std::unordered_set<Name> TypeNames;
    std::unordered_set<const StructType *> DeclaredStructTypes;
    if (ModuleValue.Name.has_value() && !isValidModuleName(*ModuleValue.Name))
    {
      Diagnostics.add<DiagnosticKind::InvalidIrModule>();
    }

    for (const StructType *TypeValue : ModuleValue.StructTypes)
    {
      if (TypeValue == nullptr)
      {
        Diagnostics.add<DiagnosticKind::IrNullStructTypeDeclaration>();
        continue;
      }
      if (!DeclaredStructTypes.insert(TypeValue).second)
      {
        Diagnostics.add<DiagnosticKind::IrStructTypeDeclaredMoreThanOnce>(TypeValue->name());
      }
      if (!TypeValue->name().valid())
      {
        Diagnostics.add<DiagnosticKind::IrInvalidStructTypeName>(TypeValue->name());
      }
      else if (!TypeNames.insert(TypeValue->name()).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateStructType>(TypeValue->name());
      }
      if (TypeValue->fields().empty())
      {
        Diagnostics.add<DiagnosticKind::IrEmptyStructType>(TypeValue->name());
      }
      std::unordered_set<Name> FieldNames;
      for (std::size_t FieldIndex = 0; FieldIndex < TypeValue->fieldCount(); ++FieldIndex)
      {
        const StructField &Field = TypeValue->field(FieldIndex);
        const Type *FieldType = Field.type();
        if (!Field.name().empty() && !Field.name().valid())
        {
          Diagnostics.add<DiagnosticKind::IrInvalidStructFieldName>(TypeValue->name(), FieldIndex, Field.name());
        }
        else if (!Field.name().empty() && !FieldNames.insert(Field.name()).second)
        {
          Diagnostics.add<DiagnosticKind::IrDuplicateStructFieldName>(TypeValue->name(), Field.name());
        }
        if (!isValidType(ModuleValue, FieldType) || FieldType->kind() == TypeKind::Void)
        {
          Diagnostics.add<DiagnosticKind::IrInvalidStructFieldType>(TypeValue->name(), FieldIndex);
        }
        else if (isByteSliceType(FieldType))
        {
          Diagnostics.add<DiagnosticKind::IrSliceStructFieldForbidden>(TypeValue->name(), FieldIndex, typeKindName(FieldType->kind()));
        }
        else if (FieldType->kind() == TypeKind::Struct && (FieldType == TypeValue || DeclaredStructTypes.find(static_cast<const StructType *>(FieldType)) == DeclaredStructTypes.end()))
        {
          Diagnostics.add<DiagnosticKind::IrStructFieldForwardOrSelfReference>(TypeValue->name(), FieldIndex);
        }
        const StructFieldAttributeDiagnosticReporter AttributeReporter{TypeValue->name(), FieldIndex, Diagnostics};
        verifyAttributes(ModuleValue, Field.attributes(), TypeValue->name(), Diagnostics, AttributeReporter);
      }
      const StructLayoutConstraints &StructConstraints = TypeValue->layoutConstraints();
      bool HasLayoutConstraints = StructConstraints.ExplicitAlignment.has_value() || StructConstraints.Packing.has_value();
      for (const StructField &Field : TypeValue->fields())
      {
        HasLayoutConstraints = HasLayoutConstraints || Field.layoutConstraints().ExplicitAlignment.has_value() || Field.layoutConstraints().ExplicitOffset.has_value();
      }
      if (HasLayoutConstraints && !computeTypeLayout(*TypeValue, ModuleValue.context().compilationContext().targetContext()).has_value())
      {
        Diagnostics.add<DiagnosticKind::IrInvalidStructLayoutConstraints>(TypeValue->name());
      }
    }

    for (const ByteConstant &Constant : ModuleValue.ByteConstants)
    {
      if (!Constant.Name.valid())
      {
        Diagnostics.add<DiagnosticKind::IrInvalidGlobalByteConstantName>(Constant.Name);
      }
      else if (!GlobalNames.insert(Constant.Name).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateGlobalSymbol>(Constant.Name);
      }
    }

    for (const GlobalVariable &Global : ModuleValue.Globals)
    {
      if (!Global.Name.valid())
      {
        Diagnostics.add<DiagnosticKind::InvalidIrModule>();
      }
      else if (!GlobalNames.insert(Global.Name).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateGlobalSymbol>(Global.Name);
      }
      if (!isValidType(ModuleValue, Global.ValueType) || !isMemoryValueType(*Global.ValueType))
      {
        Diagnostics.add<DiagnosticKind::InvalidIrModule>();
      }
      if (Global.Kind != GlobalVariableKind::Definition && Global.Kind != GlobalVariableKind::Imported)
      {
        Diagnostics.add<DiagnosticKind::IrGlobalVariableUnknownKind>(Global.Name);
      }
      else if (Global.Kind == GlobalVariableKind::Imported)
      {
        if (!Global.Import.has_value())
        {
          Diagnostics.add<DiagnosticKind::IrImportedGlobalInvalidModule>(Global.Name);
          Diagnostics.add<DiagnosticKind::IrImportedGlobalInvalidTargetName>(Global.Name, ink::ir::Name{});
        }
        else
        {
          if (!isValidModuleName(Global.Import->Module) || (ModuleValue.Name.has_value() && Global.Import->Module == *ModuleValue.Name))
          {
            Diagnostics.add<DiagnosticKind::IrImportedGlobalInvalidModule>(Global.Name);
          }
          if (!Global.Import->Symbol.valid())
          {
            Diagnostics.add<DiagnosticKind::IrImportedGlobalInvalidTargetName>(Global.Name, Global.Import->Symbol);
          }
        }
      }
      else if (Global.Import.has_value())
      {
        Diagnostics.add<DiagnosticKind::InvalidIrModule>();
      }
    }

    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      if (!FunctionValue.Name.valid())
      {
        Diagnostics.add<DiagnosticKind::IrInvalidFunctionName>(FunctionValue.Name);
      }
      else if (!GlobalNames.insert(FunctionValue.Name).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateGlobalSymbol>(FunctionValue.Name);
      }
      verifyFunctionSignature(ModuleValue, FunctionValue, Diagnostics);
    }
    const auto VerifyLifecycleFunction = [&ModuleValue, &Diagnostics](const std::optional<FunctionId> &FunctionIdValue)
    {
      if (!FunctionIdValue.has_value())
      {
        return;
      }
      if (!FunctionIdValue->valid() || FunctionIdValue->value() >= ModuleValue.Functions.size())
      {
        Diagnostics.add<DiagnosticKind::InvalidIrModule>();
        return;
      }
      const Function &FunctionValue = ModuleValue.Functions[FunctionIdValue->value()];
      if (FunctionValue.Kind != FunctionKind::Definition || FunctionValue.ResultType == nullptr || FunctionValue.ResultType->kind() != TypeKind::Void || FunctionValue.parameterCount() != 0)
      {
        Diagnostics.add<DiagnosticKind::InvalidIrModule>();
      }
    };
    VerifyLifecycleFunction(ModuleValue.Initializer);
    VerifyLifecycleFunction(ModuleValue.Finalizer);
    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      verifyFunctionBody(ModuleValue, FunctionValue, Diagnostics);
    }
    const bool Succeeded = Diagnostics.empty();
    Diagnostics.report(Context);
    return VerificationResult(Succeeded);
  }

  VerificationResult verify(const Module &ModuleValue)
  {
    return verify(ModuleValue.context(), ModuleValue);
  }
} // namespace ink::ir
