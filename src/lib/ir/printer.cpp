#include "ink/ir/printer.h"

#include <iomanip>
#include <locale>
#include <sstream>

namespace ink::ir
{
  namespace
  {
    const char *signednessPrefix(IrSignedness Signedness) noexcept
    {
      switch (Signedness)
      {
      case IrSignedness::Signed:
        return "i";
      case IrSignedness::Unsigned:
        return "u";
      case IrSignedness::Signless:
        return "si";
      }
      return "si";
    }

    const char *comparePredicateName(IrComparePredicate Predicate) noexcept
    {
      switch (Predicate)
      {
      case IrComparePredicate::Equal:
        return "eq";
      case IrComparePredicate::NotEqual:
        return "ne";
      case IrComparePredicate::SignedLess:
        return "slt";
      case IrComparePredicate::SignedLessEqual:
        return "sle";
      case IrComparePredicate::SignedGreater:
        return "sgt";
      case IrComparePredicate::SignedGreaterEqual:
        return "sge";
      case IrComparePredicate::UnsignedLess:
        return "ult";
      case IrComparePredicate::UnsignedLessEqual:
        return "ule";
      case IrComparePredicate::UnsignedGreater:
        return "ugt";
      case IrComparePredicate::UnsignedGreaterEqual:
        return "uge";
      }
      return "unknown";
    }

    const char *trapKindName(IrTrapKind Kind) noexcept
    {
      switch (Kind)
      {
      case IrTrapKind::User:
        return "user";
      case IrTrapKind::Bounds:
        return "bounds";
      case IrTrapKind::DivisionByZero:
        return "division_by_zero";
      case IrTrapKind::Overflow:
        return "overflow";
      case IrTrapKind::Unreachable:
        return "unreachable";
      }
      return "unknown";
    }

    std::string escapeString(const std::string &Text)
    {
      std::ostringstream Output;
      Output.imbue(std::locale::classic());
      for (const unsigned char Character : Text)
      {
        switch (Character)
        {
        case '\\':
          Output << "\\\\";
          break;
        case '"':
          Output << "\\\"";
          break;
        case '\n':
          Output << "\\n";
          break;
        case '\r':
          Output << "\\r";
          break;
        case '\t':
          Output << "\\t";
          break;
        default:
          if (Character < 0x20 || Character == 0x7f)
          {
            Output << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(Character) << std::dec;
          }
          else
          {
            Output << static_cast<char>(Character);
          }
          break;
        }
      }
      return Output.str();
    }

    class Printer
    {
    public:
      Printer(const IrModule &ModuleValue, const char *StageValue, const target::TargetKey *TargetValue = nullptr) : Module(ModuleValue), Stage(StageValue), Target(TargetValue)
      {
        Output.imbue(std::locale::classic());
      }

      std::string print()
      {
        Output << "ink.module stage=" << Stage;
        if (Target != nullptr)
        {
          Output << " target=\"" << escapeString(Target->canonicalString()) << "\"";
        }
        Output << " {\n";
        printTypes();
        printConstants();
        printOrigins();
        printFunctions();
        printPlans();
        Output << "}\n";
        return Output.str();
      }

    private:
      const IrModule &Module;
      const char *Stage;
      const target::TargetKey *Target;
      std::ostringstream Output;

      void printTypeReference(IrTypeId Type)
      {
        if (!Type.isValid())
        {
          Output << "!invalid";
          return;
        }
        Output << "!t" << Type.value();
      }

      void printValue(IrValueId Value)
      {
        if (!Value.isValid())
        {
          Output << "%invalid";
          return;
        }
        Output << "%v" << Value.value();
      }

      void printTypes()
      {
        Output << "  types {\n";
        for (std::size_t Index = 0; Index < Module.typeCount(); ++Index)
        {
          const auto TypeId = IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
          const auto &Type = Module.type(TypeId);
          Output << "    !t" << Index << " = ";
          switch (Type.Kind)
          {
          case IrTypeKind::Unit:
            Output << "unit";
            break;
          case IrTypeKind::Never:
            Output << "never";
            break;
          case IrTypeKind::Bool:
            Output << "bool";
            break;
          case IrTypeKind::Integer:
            Output << signednessPrefix(Type.Signedness) << Type.BitWidth;
            break;
          case IrTypeKind::Place:
            Output << "place<" << (Type.Access == IrPlaceAccess::ReadWrite ? "rw, " : "ro, ");
            printTypeReference(Type.ElementType);
            Output << ">";
            break;
          case IrTypeKind::Function:
            Output << "fn(";
            for (std::uint32_t ParameterIndex = 0; ParameterIndex < Type.Parameters.Count; ++ParameterIndex)
            {
              if (ParameterIndex != 0)
              {
                Output << ", ";
              }
              printTypeReference(Module.typeReference(Type.Parameters.First + ParameterIndex));
            }
            Output << ") -> ";
            if (Type.Result)
            {
              printTypeReference(*Type.Result);
            }
            else
            {
              Output << "void";
            }
            break;
          case IrTypeKind::Unknown:
            Output << "unknown";
            break;
          }
          Output << "\n";
        }
        Output << "  }\n";
      }

      void printConstants()
      {
        Output << "  constants {\n";
        for (std::size_t Index = 0; Index < Module.constantCount(); ++Index)
        {
          const auto &Constant = Module.constant(IrConstantId::fromValue(static_cast<std::uint32_t>(Index)));
          Output << "    #c" << Index << " = ";
          if (Constant.Kind == IrConstantKind::Bool)
          {
            Output << (Constant.Bits == 0 ? "false" : "true");
          }
          else
          {
            Output << "0x" << std::hex << std::setw(16) << std::setfill('0') << Constant.Bits << std::dec << std::setfill(' ');
          }
          Output << " : ";
          printTypeReference(Constant.Type);
          Output << "\n";
        }
        Output << "  }\n";
      }

      void printOrigins()
      {
        Output << "  origins {\n";
        for (std::size_t Index = 0; Index < Module.originCount(); ++Index)
        {
          const auto &Origin = Module.origin(IrOriginId::fromValue(static_cast<std::uint32_t>(Index)));
          Output << "    #o" << Index << " = file(" << Origin.File.value() << ") [" << Origin.Range.Start << ", " << Origin.Range.End << ")\n";
        }
        Output << "  }\n";
      }

      void printFunctions()
      {
        for (std::size_t FunctionIndex = 0; FunctionIndex < Module.functionCount(); ++FunctionIndex)
        {
          const auto FunctionId = IrFunctionId::fromValue(static_cast<std::uint32_t>(FunctionIndex));
          const auto &Function = Module.function(FunctionId);
          Output << "  func @f" << FunctionIndex << " \"" << escapeString(Function.Name) << "\" : ";
          printTypeReference(Function.Signature);
          if (Function.Kind == IrFunctionKind::External)
          {
            Output << " external\n";
            continue;
          }
          Output << " {\n";
          for (std::uint32_t BlockIndex = 0; BlockIndex < Function.Blocks.Count; ++BlockIndex)
          {
            printBlock(Module.functionBlock(Function.Blocks.First + BlockIndex));
          }
          Output << "  }\n";
        }
      }

      void printBlock(IrBlockId BlockId)
      {
        const auto &Block = Module.block(BlockId);
        Output << "    ^bb" << BlockId.value() << "(";
        for (std::uint32_t ArgumentIndex = 0; ArgumentIndex < Block.Arguments.Count; ++ArgumentIndex)
        {
          if (ArgumentIndex != 0)
          {
            Output << ", ";
          }
          const auto ValueId = IrValueId::fromValue(Block.Arguments.First + ArgumentIndex);
          printValue(ValueId);
          Output << ": ";
          printTypeReference(Module.value(ValueId).Type);
        }
        Output << ")";
        printOrigin(Block.Origin);
        Output << ":\n";
        for (std::uint32_t OperationIndex = 0; OperationIndex < Block.Operations.Count; ++OperationIndex)
        {
          printOperation(Module.blockOperation(Block.Operations.First + OperationIndex));
        }
      }

      void printOperation(IrOperationId OperationId)
      {
        const auto &Operation = Module.operation(OperationId);
        Output << "      ";
        for (std::uint32_t ResultIndex = 0; ResultIndex < Operation.Results.Count; ++ResultIndex)
        {
          if (ResultIndex != 0)
          {
            Output << ", ";
          }
          printValue(Module.operationResult(Operation.Results.First + ResultIndex));
        }
        if (!Operation.Results.empty())
        {
          Output << " = ";
        }
        Output << irOpcodeName(Operation.Opcode);
        for (std::uint32_t OperandIndex = 0; OperandIndex < Operation.Operands.Count; ++OperandIndex)
        {
          Output << (OperandIndex == 0 ? " " : ", ");
          printValue(Module.operationOperand(Operation.Operands.First + OperandIndex));
        }
        printPayload(Operation.Payload);
        for (std::uint32_t SuccessorIndex = 0; SuccessorIndex < Operation.Successors.Count; ++SuccessorIndex)
        {
          const auto &Successor = Module.operationSuccessor(Operation.Successors.First + SuccessorIndex);
          Output << (SuccessorIndex == 0 ? " -> " : ", ") << "^bb" << Successor.Block.value() << "(";
          for (std::uint32_t ArgumentIndex = 0; ArgumentIndex < Successor.Arguments.Count; ++ArgumentIndex)
          {
            if (ArgumentIndex != 0)
            {
              Output << ", ";
            }
            printValue(Module.successorArgument(Successor.Arguments.First + ArgumentIndex));
          }
          Output << ")";
        }
        if (!Operation.Results.empty())
        {
          Output << " : ";
          for (std::uint32_t ResultIndex = 0; ResultIndex < Operation.Results.Count; ++ResultIndex)
          {
            if (ResultIndex != 0)
            {
              Output << ", ";
            }
            printTypeReference(Module.value(Module.operationResult(Operation.Results.First + ResultIndex)).Type);
          }
        }
        printOrigin(Operation.Origin);
        Output << "\n";
      }

      void printPayload(const IrOperationPayload &Payload)
      {
        if (std::holds_alternative<IrConstantPayload>(Payload))
        {
          Output << " {constant=#c" << std::get<IrConstantPayload>(Payload).Constant.value() << "}";
        }
        else if (std::holds_alternative<IrComparePayload>(Payload))
        {
          Output << " {predicate=" << comparePredicateName(std::get<IrComparePayload>(Payload).Predicate) << "}";
        }
        else if (std::holds_alternative<IrTypePayload>(Payload))
        {
          Output << " {type=";
          printTypeReference(std::get<IrTypePayload>(Payload).Type);
          Output << "}";
        }
        else if (std::holds_alternative<IrDirectCallPayload>(Payload))
        {
          Output << " {callee=@f" << std::get<IrDirectCallPayload>(Payload).Callee.value() << "}";
        }
        else if (std::holds_alternative<IrTrapPayload>(Payload))
        {
          Output << " {kind=" << trapKindName(std::get<IrTrapPayload>(Payload).Kind) << "}";
        }
      }

      void printOrigin(IrOriginId Origin)
      {
        if (Origin.isValid())
        {
          Output << " loc(#o" << Origin.value() << ")";
        }
      }

      void printPlans()
      {
        if (Module.planNodeCount() == 0)
        {
          return;
        }
        Output << "  elaboration_plan {\n";
        for (std::size_t Index = 0; Index < Module.planNodeCount(); ++Index)
        {
          const auto &Plan = Module.planNode(IrPlanNodeId::fromValue(static_cast<std::uint32_t>(Index)));
          Output << "    #p" << Index << " = " << irPlanOpcodeName(Plan.Opcode) << " ";
          printValue(Plan.Input);
          Output << " -> ";
          printValue(Plan.Output);
          Output << " : ";
          printTypeReference(Plan.ResultType);
          printOrigin(Plan.Origin);
          Output << "\n";
        }
        Output << "  }\n";
      }
    };
  }

  std::string printIr(const IrModule &Module)
  {
    return Printer(Module, "raw").print();
  }

  std::string printIr(const UnverifiedStagedModule &Module)
  {
    return Printer(Module.module(), "unverified-staged").print();
  }

  std::string printIr(const VerifiedStagedModule &Module)
  {
    return Printer(Module.module(), "staged").print();
  }

  std::string printIr(const VerifiedClosedModule &Module)
  {
    return Printer(Module.module(), "closed", &Module.targetKey()).print();
  }
} // namespace ink::ir
