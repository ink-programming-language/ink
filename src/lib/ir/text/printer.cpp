#include "printer.h"

#include "format.h"

#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/instruction/memory.h"
#include "ink/ir/model/attribute.h"
#include "ink/ir/model/constant.h"
#include "ink/ir/model/function.h"
#include "ink/ir/model/module.h"
#include "ink/ir/model/operand.h"
#include "ink/ir/model/parameter.h"
#include "ink/ir/model/struct_type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string_view>
#include <vector>

namespace ink::ir::text
{
  namespace
  {
    template <typename Callback>
    void writeSeparated(std::ostringstream &Output, std::size_t Count, Callback WriteValue)
    {
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        if (Index != 0)
        {
          Output << ", ";
        }
        WriteValue(Index);
      }
    }

    std::string escapeBytes(std::string_view Data)
    {
      constexpr char HexadecimalDigits[] = "0123456789ABCDEF";
      std::string Result;
      for (const unsigned char Byte : Data)
      {
        if (Byte >= 0x20U && Byte <= 0x7EU && Byte != static_cast<unsigned char>('"') && Byte != static_cast<unsigned char>('\\'))
        {
          Result.push_back(static_cast<char>(Byte));
        }
        else
        {
          Result.push_back('\\');
          Result.push_back(HexadecimalDigits[Byte >> 4U]);
          Result.push_back(HexadecimalDigits[Byte & 0x0FU]);
        }
      }
      return Result;
    }

    class TextPrinter final
    {
      public:
        explicit TextPrinter(const Module &ModuleValue) noexcept
            : ModuleValue(ModuleValue)
        {
        }

        std::string print()
        {
          Output << "inkir " << CurrentFormatVersion << '\n';
          writeModuleHeader();
          writeStructTypes();
          writeByteConstants();
          writeGlobals();
          writeFunctions();
          return Output.str();
        }

      private:
        void writeModuleHeader()
        {
          if (ModuleValue.Name.has_value())
          {
            Output << "module " << *ModuleValue.Name << '\n';
          }
          if (ModuleValue.Initializer.has_value())
          {
            Output << "initializer @" << ModuleValue.Functions[ModuleValue.Initializer->value()].Name << '\n';
          }
          if (ModuleValue.Finalizer.has_value())
          {
            Output << "finalizer @" << ModuleValue.Functions[ModuleValue.Finalizer->value()].Name << '\n';
          }
        }

        void writeStructTypes()
        {
          for (const StructType *TypeValue : ModuleValue.StructTypes)
          {
            Output << "\n%" << TypeValue->name() << " = type";
            if (TypeValue->layoutConstraints().ExplicitAlignment.has_value())
            {
              Output << " align(" << *TypeValue->layoutConstraints().ExplicitAlignment << ')';
            }
            if (TypeValue->layoutConstraints().Packing.has_value())
            {
              Output << " pack(" << *TypeValue->layoutConstraints().Packing << ')';
            }
            Output << " {";
            writeSeparated(Output, TypeValue->fieldCount(), [this, TypeValue](std::size_t FieldIndex)
            {
              const StructField &Field = TypeValue->field(FieldIndex);
              if (!Field.name().empty())
              {
                Output << Field.name() << ": ";
              }
              writeType(*Field.type());
              if (Field.layoutConstraints().ExplicitAlignment.has_value())
              {
                Output << " align(" << *Field.layoutConstraints().ExplicitAlignment << ')';
              }
              if (Field.layoutConstraints().ExplicitOffset.has_value())
              {
                Output << " offset(" << *Field.layoutConstraints().ExplicitOffset << ')';
              }
              writeAttributes(Field.attributes());
            });
            Output << "}\n";
          }
        }

        void writeByteConstants()
        {
          for (const ByteConstant &Constant : ModuleValue.ByteConstants)
          {
            Output << "\n@" << Constant.Name << " = private constant [" << Constant.Data.size() << " x byte] c\"" << escapeBytes(Constant.Data) << "\"\n";
          }
        }

        void writeGlobals()
        {
          for (const GlobalVariable &Global : ModuleValue.Globals)
          {
            if (Global.Kind == GlobalVariableKind::Imported)
            {
              Output << "\ndeclare import global " << (Global.Mutable ? "mutable " : "constant ");
              writeType(*Global.ValueType);
              Output << " @" << Global.Name << " from module " << Global.Import->Module << ", symbol @" << Global.Import->Symbol << '\n';
              continue;
            }
            Output << "\n@" << Global.Name << " = global " << (Global.Mutable ? "mutable " : "constant ");
            writeType(*Global.ValueType);
            Output << '\n';
          }
        }

        void writeFunctions()
        {
          for (const Function &FunctionValue : ModuleValue.Functions)
          {
            if (FunctionValue.Kind == FunctionKind::Imported)
            {
              Output << "\ndeclare import ";
              writeType(*FunctionValue.ResultType);
              Output << " @" << FunctionValue.Name << '(';
              writeParameters(FunctionValue.Parameters, false);
              Output << ") from module " << FunctionValue.Import->Module << ", symbol @" << FunctionValue.Import->Symbol;
              writeAttributes(FunctionValue.Attributes);
              Output << '\n';
              continue;
            }
            if (FunctionValue.Kind == FunctionKind::External)
            {
              Output << "\ndeclare extern \"C\" ";
              writeType(*FunctionValue.ResultType);
              Output << " @" << FunctionValue.Name << '(';
              writeParameters(FunctionValue.Parameters, false);
              Output << ')';
              writeAttributes(FunctionValue.Attributes);
              Output << '\n';
              continue;
            }
            writeFunctionDefinition(FunctionValue);
          }
        }

        void writeFunctionDefinition(const Function &FunctionValue)
        {
          Output << "\ndefine ";
          writeType(*FunctionValue.ResultType);
          Output << " @" << FunctionValue.Name << '(';
          writeParameters(FunctionValue.Parameters, true);
          Output << ')';
          writeAttributes(FunctionValue.Attributes);
          Output << " {\n";
          for (const BasicBlock &Block : FunctionValue.Blocks)
          {
            Output << Block.Name << ":\n";
            for (const std::unique_ptr<Instruction> &InstructionValue : Block.Instructions)
            {
              writeInstruction(FunctionValue, *InstructionValue);
            }
          }
          Output << "}\n";
        }

        void writeType(const Type &TypeValue)
        {
          if (TypeValue.kind() == TypeKind::Struct)
          {
            Output << '%' << static_cast<const StructType &>(TypeValue).name();
            return;
          }
          Output << typeKindName(TypeValue.kind());
        }

        void writeHexadecimalBitPattern(std::uint64_t BitPattern, std::size_t BitWidth)
        {
          constexpr char HexadecimalDigits[] = "0123456789ABCDEF";
          Output << "0x";
          for (std::size_t DigitIndex = BitWidth / 4; DigitIndex > 0; --DigitIndex)
          {
            const std::size_t Shift = (DigitIndex - 1) * 4;
            Output << HexadecimalDigits[(BitPattern >> Shift) & 0x0FU];
          }
        }

        void writeOperandValue(const Value &OperandValue)
        {
          switch (OperandValue.kind())
          {
          case ValueKind::IntegerConstant:
          {
            const IntegerConstant &Constant = static_cast<const IntegerConstant &>(OperandValue);
            if (Constant.isNegative())
            {
              Output << Constant.signedValue();
            }
            else
            {
              Output << Constant.unsignedValue();
            }
            return;
          }
          case ValueKind::ValueOperand:
            Output << '%' << static_cast<const ValueOperand &>(OperandValue).id().value();
            return;
          case ValueKind::GlobalAddressOperand:
          {
            const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(OperandValue);
            Output << '@' << ModuleValue.ByteConstants[Address.global().value()].Name << '[' << Address.byteOffset() << ']';
            return;
          }
          case ValueKind::GlobalVariableAddressOperand:
          {
            const GlobalId Global = static_cast<const GlobalVariableAddressOperand &>(OperandValue).global();
            Output << '@' << ModuleValue.Globals[Global.value()].Name;
            return;
          }
          case ValueKind::ZeroInitializer:
            Output << "zeroinitializer";
            return;
          case ValueKind::FloatConstant:
          {
            const FloatConstant &Constant = static_cast<const FloatConstant &>(OperandValue);
            Output << "floatbits(" << floatFormatName(Constant.format()) << ',';
            writeHexadecimalBitPattern(Constant.bitPattern(), floatFormatBitWidth(Constant.format()));
            Output << ')';
            return;
          }
          case ValueKind::StringConstant:
            Output << "c\"" << escapeBytes(static_cast<const StringConstant &>(OperandValue).data()) << '"';
            return;
          case ValueKind::NullConstant:
            Output << "null";
            return;
          case ValueKind::AggregateConstant:
          {
            const AggregateConstant &Constant = static_cast<const AggregateConstant &>(OperandValue);
            Output << '{';
            writeSeparated(Output, Constant.elements().size(), [this, &Constant](std::size_t ElementIndex)
            {
              writeOperand(Constant.elements()[ElementIndex].get());
            });
            Output << '}';
            return;
          }
          }
        }

        void writeOperand(const Value &OperandValue)
        {
          writeType(OperandValue.type());
          Output << ' ';
          writeOperandValue(OperandValue);
        }

        void writeAttribute(const Attribute &AttributeValue)
        {
          Output << attributeKindSpelling(AttributeValue.kind());
          if (AttributeValue.arguments().empty())
          {
            return;
          }
          Output << '(';
          writeSeparated(Output, AttributeValue.arguments().size(), [this, &AttributeValue](std::size_t ArgumentIndex)
          {
            const AttributeArgument &Argument = AttributeValue.arguments()[ArgumentIndex];
            Output << Argument.key() << " = ";
            writeOperand(Argument.value());
          });
          Output << ')';
        }

        void writeAttributes(const std::vector<Attribute> &Attributes)
        {
          if (Attributes.empty())
          {
            return;
          }
          Output << " [";
          writeSeparated(Output, Attributes.size(), [this, &Attributes](std::size_t AttributeIndex)
          {
            writeAttribute(Attributes[AttributeIndex]);
          });
          Output << ']';
        }

        void writeParameters(const std::vector<Parameter> &Parameters, bool IncludeSsaNames)
        {
          writeSeparated(Output, Parameters.size(), [this, &Parameters, IncludeSsaNames](std::size_t ParameterIndex)
          {
            const Parameter &ParameterValue = Parameters[ParameterIndex];
            if (!ParameterValue.name().empty())
            {
              Output << ParameterValue.name() << ": ";
            }
            writeType(*ParameterValue.type());
            if (IncludeSsaNames)
            {
              Output << " %" << ParameterIndex;
            }
            if (ParameterValue.defaultValue() != nullptr)
            {
              Output << " = ";
              writeOperand(*ParameterValue.defaultValue());
            }
          });
        }

        void writeInstruction(const Function &FunctionValue, const Instruction &InstructionValue)
        {
          switch (InstructionValue.kind())
          {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                                       \
    write##Name(FunctionValue, static_cast<const Name##Instruction &>(InstructionValue)); \
    return;
#include "ink/ir/ir.def"
          }
        }

        void writeCall(const Function &, const CallInstruction &Call)
        {
          Output << "  ";
          if (Call.Result.has_value())
          {
            Output << '%' << Call.Result->value() << " = ";
          }
          Output << "call ";
          writeType(*Call.ResultType);
          Output << " @" << ModuleValue.Functions[Call.Callee.value()].Name << '(';
          writeSeparated(Output, Call.Arguments.size(), [this, &Call](std::size_t ArgumentIndex)
          {
            writeOperand(*Call.Arguments[ArgumentIndex]);
          });
          Output << ")\n";
        }

        void writeImport(const Function &, const ImportInstruction &Import)
        {
          Output << "  import " << Import.Module << '\n';
        }

        void writeAlloca(const Function &, const AllocaInstruction &Alloca)
        {
          Output << "  %" << Alloca.Result.value() << " = alloca ";
          writeType(*Alloca.ResultType);
          Output << ' ';
          writeOperand(*Alloca.Size);
          Output << '\n';
        }

        void writeGetElementPointer(const Function &, const GetElementPointerInstruction &GetElementPointer)
        {
          Output << "  %" << GetElementPointer.Result.value() << " = getelementptr ";
          writeType(*GetElementPointer.ElementType);
          Output << ", ";
          writeOperand(*GetElementPointer.Pointer);
          Output << ", ";
          writeOperand(*GetElementPointer.Index);
          for (const ValueHandle &FieldIndex : GetElementPointer.FieldIndices)
          {
            Output << ", ";
            writeOperand(*FieldIndex);
          }
          Output << '\n';
        }

        void writeLoad(const Function &, const LoadInstruction &Load)
        {
          Output << "  %" << Load.Result.value() << " = load ";
          writeType(*Load.ResultType);
          Output << ", ";
          writeOperand(*Load.Pointer);
          Output << '\n';
        }

        void writeStore(const Function &, const StoreInstruction &Store)
        {
          Output << "  store ";
          writeOperand(*Store.StoredValue);
          Output << ", ";
          writeOperand(*Store.Pointer);
          Output << '\n';
        }

        void writeLifetimeEnd(const Function &, const LifetimeEndInstruction &LifetimeEnd)
        {
          Output << "  lifetime.end ";
          writeOperand(*LifetimeEnd.Slice);
          Output << '\n';
        }

        void writeSliceData(const Function &, const SliceDataInstruction &SliceData)
        {
          Output << "  %" << SliceData.Result.value() << " = slice.data ";
          writeType(*SliceData.ResultType);
          Output << ' ';
          writeOperand(*SliceData.Slice);
          Output << '\n';
        }

        void writeSliceLength(const Function &, const SliceLengthInstruction &SliceLength)
        {
          Output << "  %" << SliceLength.Result.value() << " = slice.length ";
          writeOperand(*SliceLength.Slice);
          Output << '\n';
        }

        void writePhi(const Function &FunctionValue, const PhiInstruction &Phi)
        {
          Output << "  %" << Phi.Result.value() << " = phi ";
          writeType(*Phi.ResultType);
          Output << ' ';
          writeSeparated(Output, Phi.IncomingValues.size(), [this, &FunctionValue, &Phi](std::size_t IncomingIndex)
          {
            const PhiIncoming &Incoming = Phi.IncomingValues[IncomingIndex];
            Output << '[';
            writeOperandValue(*Incoming.Value);
            Output << ", " << FunctionValue.Blocks[Incoming.Predecessor.value()].Name << ']';
          });
          Output << '\n';
        }

        void writeAdd(const Function &, const AddInstruction &Add)
        {
          Output << "  %" << Add.Result.value() << " = add ";
          writeOperand(*Add.Left);
          Output << ", ";
          writeOperand(*Add.Right);
          Output << '\n';
        }

        void writeCompare(const Function &, const CompareInstruction &Compare)
        {
          Output << "  %" << Compare.Result.value() << " = icmp " << comparePredicateName(Compare.Predicate) << ' ';
          writeOperand(*Compare.Left);
          Output << ", ";
          writeOperand(*Compare.Right);
          Output << '\n';
        }

        void writeInsertValue(const Function &, const InsertValueInstruction &Insert)
        {
          Output << "  %" << Insert.Result.value() << " = insertvalue ";
          writeOperand(*Insert.Aggregate);
          Output << ", ";
          writeOperand(*Insert.Element);
          Output << ", " << Insert.FieldIndex << '\n';
        }

        void writeExtractValue(const Function &, const ExtractValueInstruction &Extract)
        {
          Output << "  %" << Extract.Result.value() << " = extractvalue ";
          writeOperand(*Extract.Aggregate);
          Output << ", " << Extract.FieldIndex << '\n';
        }

        void writeBlockTarget(const Function &FunctionValue, const BlockTarget &Target)
        {
          Output << FunctionValue.Blocks[Target.Block.value()].Name;
        }

        void writeBranch(const Function &FunctionValue, const BranchInstruction &Branch)
        {
          Output << "  br ";
          writeBlockTarget(FunctionValue, Branch.Target);
          Output << '\n';
        }

        void writeConditionalBranch(const Function &FunctionValue, const ConditionalBranchInstruction &Branch)
        {
          Output << "  condbr ";
          writeOperand(*Branch.Condition);
          Output << ", ";
          writeBlockTarget(FunctionValue, Branch.TrueTarget);
          Output << ", ";
          writeBlockTarget(FunctionValue, Branch.FalseTarget);
          Output << '\n';
        }

        void writeReturn(const Function &, const ReturnInstruction &Return)
        {
          Output << "  ret ";
          if (!Return.ReturnValue)
          {
            Output << "void\n";
            return;
          }
          writeOperand(*Return.ReturnValue);
          Output << '\n';
        }

        const Module &ModuleValue;
        std::ostringstream Output;
    };
  } // namespace

  std::string printModule(const Module &ModuleValue)
  {
    return TextPrinter(ModuleValue).print();
  }
} // namespace ink::ir::text
