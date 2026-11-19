#include "ink/ir/text/module_text.h"

#include "module_text_format.h"

#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/instruction/instruction_visitor.h"
#include "ink/ir/type/type_visitor.h"
#include "ink/ir/verification/verifier.h"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir
{
    namespace
    {
        class SlotTracker final
        {
            public:
                explicit SlotTracker(const Module& ModuleValue)
                {
                    for (const ModuleImport& Import : ModuleValue.imports())
                    {
                        ImportIds.emplace(&Import, "#" + std::to_string(ImportCount++));
                    }
                    for (const StructType& Record : ModuleValue.structTypes())
                    {
                        RegisteredRecords.insert(&Record);
                        reserveRecord(Record);
                    }
                    for (const ObjectType& Record : ModuleValue.objectTypes())
                    {
                        RegisteredRecords.insert(&Record);
                        reserveRecord(Record);
                    }
                    for (const Function& FunctionValue : ModuleValue.functions())
                    {
                        indexFunction(FunctionValue);
                    }
                    for (const GlobalVariable& Global : ModuleValue.globalVariables())
                    {
                        ValueIds.emplace(&Global, "@g" + std::to_string(GlobalCount++));
                    }
                    for (const GlobalVariable& Global : ModuleValue.globalVariables())
                    {
                        collectType(Global.getType());
                    }
                    for (const Function& FunctionValue : ModuleValue.functions())
                    {
                        collectFunctionTypes(FunctionValue);
                    }
                    for (std::size_t Index = 0; Index < OrderedRecords.size(); ++Index)
                    {
                        indexRecord(*OrderedRecords[Index]);
                    }
                }

                std::span<const RecordType* const> records() const noexcept
                {
                    return OrderedRecords;
                }

                bool isRegistered(const RecordType& Record) const noexcept
                {
                    return RegisteredRecords.contains(&Record);
                }

                const std::string* findImportId(const ModuleImport& Import) const noexcept
                {
                    const auto Found = ImportIds.find(&Import);
                    return Found == ImportIds.end() ? nullptr : &Found->second;
                }

                const std::string* findValueId(const Value& Node) const noexcept
                {
                    const auto Found = ValueIds.find(&Node);
                    return Found == ValueIds.end() ? nullptr : &Found->second;
                }

                const std::string* findBlockId(const BasicBlock& Block) const noexcept
                {
                    const auto Found = BlockIds.find(&Block);
                    return Found == BlockIds.end() ? nullptr : &Found->second;
                }

            private:
                void reserveRecord(const RecordType& Record)
                {
                    if (ValueIds.emplace(&Record, "%t" + std::to_string(OrderedRecords.size())).second)
                    {
                        OrderedRecords.push_back(&Record);
                    }
                }

                void indexFunction(const Function& FunctionValue)
                {
                    if (!ValueIds.emplace(&FunctionValue, "@f" + std::to_string(FunctionCount)).second)
                    {
                        return;
                    }
                    ++FunctionCount;
                    for (const Argument& ArgumentValue : FunctionValue.arguments())
                    {
                        ValueIds.emplace(&ArgumentValue, "%a" + std::to_string(ArgumentCount++));
                    }
                    for (const BasicBlock& Block : FunctionValue.basicBlocks())
                    {
                        BlockIds.emplace(&Block, "%bb" + std::to_string(BlockCount++));
                        for (const Instruction& Node : Block.instructions())
                        {
                            if (!VoidType::classof(&Node.getType()))
                            {
                                ValueIds.emplace(&Node, "%v" + std::to_string(InstructionCount++));
                            }
                        }
                    }
                }

                void indexRecord(const RecordType& Record)
                {
                    reserveRecord(Record);
                    if (!IndexedRecords.insert(&Record).second)
                    {
                        return;
                    }
                    for (const Method& MethodValue : Record.methods())
                    {
                        indexFunction(MethodValue.function());
                    }
                    for (const Value* Base : Record.bases())
                    {
                        collectType(*Base);
                    }
                    for (const Field& FieldValue : Record.fields())
                    {
                        collectType(FieldValue.type());
                    }
                    for (const Method& MethodValue : Record.methods())
                    {
                        collectFunctionTypes(MethodValue.function());
                    }
                }

                void collectType(const Value& Node)
                {
                    if (!Type::classof(&Node) || !CollectedTypes.insert(&Node).second)
                    {
                        return;
                    }
                    const auto& TypeValue = static_cast<const Type&>(Node);
                    switch (TypeValue.getKind())
                    {
                    case TypeKind::Pointer:
                        collectType(static_cast<const PointerType&>(TypeValue).pointeeType());
                        return;
                    case TypeKind::Array:
                        collectType(static_cast<const ArrayType&>(TypeValue).elementType());
                        return;
                    case TypeKind::Function:
                        {
                            const auto& Signature = static_cast<const FunctionType&>(TypeValue);
                            collectType(Signature.returnType());
                            for (const Parameter& ParameterValue : Signature.parameters())
                            {
                                collectType(ParameterValue.type());
                            }
                            return;
                        }
                    case TypeKind::Struct:
                    case TypeKind::Object:
                        reserveRecord(static_cast<const RecordType&>(TypeValue));
                        return;
                    default:
                        return;
                    }
                }

                void collectFunctionTypes(const Function& FunctionValue)
                {
                    collectType(FunctionValue.getType());
                    for (const BasicBlock& Block : FunctionValue.basicBlocks())
                    {
                        for (const Instruction& Node : Block.instructions())
                        {
                            collectType(Node.getType());
                            for (std::size_t Index = 0; Index < Node.operandCount(); ++Index)
                            {
                                const Value& Operand = Node.operand(Index);
                                collectType(Operand.getType());
                                if (Type::classof(&Operand))
                                {
                                    collectType(Operand);
                                }
                            }
                        }
                    }
                }

                std::unordered_map<const ModuleImport*, std::string> ImportIds = {};
                std::unordered_map<const Value*, std::string> ValueIds = {};
                std::unordered_map<const BasicBlock*, std::string> BlockIds = {};
                std::unordered_set<const Value*> CollectedTypes = {};
                std::unordered_set<const RecordType*> RegisteredRecords = {};
                std::unordered_set<const RecordType*> IndexedRecords = {};
                std::vector<const RecordType*> OrderedRecords = {};
                std::size_t ImportCount = 0;
                std::size_t FunctionCount = 0;
                std::size_t GlobalCount = 0;
                std::size_t ArgumentCount = 0;
                std::size_t BlockCount = 0;
                std::size_t InstructionCount = 0;
        };

        class ModuleTextWriter final : public TypeVisitor<ModuleTextWriter, bool>, public InstructionVisitor<ModuleTextWriter, bool>
        {
            public:
                explicit ModuleTextWriter(const Module& ModuleValue)
                    : ModuleValue(ModuleValue),
                      Slots(ModuleValue)
                {
                }

                std::expected<std::string, core::Diagnostic> write()
                {
                    Output += detail::ModulePathKeyword;
                    Output += " = ";
                    detail::appendQuotedBytes(Output, {}, detail::nameText(ModuleValue.path().qualifiedName()));
                    Output.push_back('\n');

                    for (const ModuleImport& Import : ModuleValue.imports())
                    {
                        if (!writeImport(Import))
                        {
                            return std::unexpected(std::move(*Failure));
                        }
                    }
                    for (const RecordType* Record : Slots.records())
                    {
                        if (!writeRecord(*Record))
                        {
                            return std::unexpected(std::move(*Failure));
                        }
                    }
                    for (const GlobalVariable& Global : ModuleValue.globalVariables())
                    {
                        if (!writeGlobal(Global))
                        {
                            return std::unexpected(std::move(*Failure));
                        }
                    }
                    for (const Function& FunctionValue : ModuleValue.functions())
                    {
                        if (!writeFunction(FunctionValue, 0))
                        {
                            return std::unexpected(std::move(*Failure));
                        }
                    }
                    return std::move(Output);
                }

                bool visitMetaType(const MetaType& TypeValue)
                {
                    Output += TypeValue.getName();
                    return true;
                }

                bool visitBoolType(const BoolType& TypeValue)
                {
                    Output += TypeValue.getName();
                    return true;
                }

                bool visitVoidType(const VoidType& TypeValue)
                {
                    Output += TypeValue.getName();
                    return true;
                }

                bool visitIntegerType(const IntegerType& TypeValue)
                {
                    Output.push_back(TypeValue.isSigned() ? 'i' : 'u');
                    Output += std::to_string(TypeValue.bitWidth());
                    return true;
                }

                bool visitFloatType(const FloatType& TypeValue)
                {
                    Output.push_back('f');
                    Output += std::to_string(TypeValue.bitWidth());
                    return true;
                }

                bool visitPointerType(const PointerType& TypeValue)
                {
                    Output += detail::enumSpelling(detail::TypeSyntaxKind::Pointer, detail::KeywordTypeEntries);
                    Output.push_back('<');
                    if (!writeType(TypeValue.pointeeType(), TypeDepth + 1))
                    {
                        return false;
                    }
                    Output.push_back('>');
                    return true;
                }

                bool visitArrayType(const ArrayType& TypeValue)
                {
                    Output.push_back('[');
                    Output += std::to_string(TypeValue.elementCount());
                    Output += " x ";
                    if (!writeType(TypeValue.elementType(), TypeDepth + 1))
                    {
                        return false;
                    }
                    Output.push_back(']');
                    return true;
                }

                bool visitStructType(const StructType& TypeValue)
                {
                    return writeValueReference(TypeValue);
                }

                bool visitObjectType(const ObjectType& TypeValue)
                {
                    return writeValueReference(TypeValue);
                }

                bool visitFunctionType(const FunctionType& TypeValue)
                {
                    const std::size_t NextDepth = TypeDepth + 1;
                    Output += detail::enumSpelling(detail::TypeSyntaxKind::Function, detail::KeywordTypeEntries);
                    if (TypeValue.abi() != ABIKind::Ink || TypeValue.callingConvention() != CallingConvention::Default)
                    {
                        Output.push_back('<');
                        Output += detail::enumSpelling(TypeValue.abi(), detail::ABIEntries);
                        Output.push_back(' ');
                        Output += detail::enumSpelling(TypeValue.callingConvention(), detail::ConventionEntries);
                        Output.push_back('>');
                    }
                    Output.push_back('(');
                    const auto Parameters = TypeValue.parameters();
                    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
                    {
                        if (Index != 0)
                        {
                            Output += ", ";
                        }
                        if (Parameters[Index].kind() != Parameter::Kind::Positional)
                        {
                            Output += detail::enumSpelling(Parameters[Index].kind(), detail::ParameterKindEntries);
                            Output.push_back(' ');
                        }
                        if (!writeType(Parameters[Index].type(), NextDepth))
                        {
                            return false;
                        }
                    }
                    if (TypeValue.isVariadic())
                    {
                        if (!Parameters.empty())
                        {
                            Output += ", ";
                        }
                        Output += detail::EllipsisToken;
                    }
                    Output += ") ";
                    Output += detail::ArrowToken;
                    Output.push_back(' ');
                    return writeType(TypeValue.returnType(), NextDepth);
                }

                bool visitAddInstruction(const AddInstruction& Node)
                {
                    return writeType(Node.getType()) && appendChar(' ') && writeValue(Node.getLHS()) && appendText(", ") && writeValue(Node.getRHS());
                }

                bool visitAllocaInstruction(const AllocaInstruction& Node)
                {
                    return writeType(Node.getAllocatedType());
                }

                bool visitLoadInstruction(const LoadInstruction& Node)
                {
                    return writeType(Node.getType()) && appendText(", ") && writeTypedOperand(Node.getSource());
                }

                bool visitStoreInstruction(const StoreInstruction& Node)
                {
                    return writeTypedOperand(Node.getSource()) && appendText(", ") && writeTypedOperand(Node.getDestination());
                }

                bool visitPhiInstruction(const PhiInstruction& Node)
                {
                    if (!writeType(Node.getType()) || !appendChar(' '))
                    {
                        return false;
                    }
                    const auto IncomingList = Node.incoming();
                    for (std::size_t Index = 0; Index < IncomingList.size(); ++Index)
                    {
                        if (Index != 0)
                        {
                            Output += ", ";
                        }
                        Output.push_back('[');
                        if (!writeValue(IncomingList[Index].getValue()) || !appendText(", ") || !writeBlockReference(IncomingList[Index].getBlock()))
                        {
                            return false;
                        }
                        Output.push_back(']');
                    }
                    return true;
                }

                bool visitCallInstruction(const CallInstruction& Node)
                {
                    if (!writeType(Node.getType()) || !appendChar(' ') || !writeValueReference(Node.getCallee()))
                    {
                        return false;
                    }
                    Output.push_back('(');
                    for (std::size_t Index = 0; Index < Node.argumentCount(); ++Index)
                    {
                        if (Index != 0)
                        {
                            Output += ", ";
                        }
                        if (!writeTypedOperand(Node.argument(Index)))
                        {
                            return false;
                        }
                    }
                    Output.push_back(')');
                    return true;
                }

                bool visitReturnInstruction(const ReturnInstruction& Node)
                {
                    if (const Value* Returned = Node.getReturnValue())
                    {
                        return writeTypedOperand(*Returned);
                    }
                    Output += detail::VoidKeyword;
                    return true;
                }

                bool visitBranchInstruction(const BranchInstruction& Node)
                {
                    Output += detail::LabelKeyword;
                    Output.push_back(' ');
                    return writeBlockReference(Node.getTarget());
                }

                bool visitCondBranchInstruction(const CondBranchInstruction& Node)
                {
                    return writeTypedOperand(Node.getCondition()) && appendText(", ") && appendText(detail::LabelKeyword) && appendChar(' ') && writeBlockReference(Node.getTrueTarget()) && appendText(", ") && appendText(detail::LabelKeyword) && appendChar(' ') && writeBlockReference(Node.getFalseTarget());
                }

            private:
                bool fail(std::string_view Description)
                {
                    if (!Failure)
                    {
                        Failure = detail::makeTextDiagnostic(0, Description);
                    }
                    return false;
                }

                bool appendChar(char Value)
                {
                    Output.push_back(Value);
                    return true;
                }

                bool appendText(std::string_view Text)
                {
                    Output.append(Text);
                    return true;
                }

                void appendIndent(std::size_t Level)
                {
                    Output.append(Level * 2, ' ');
                }

                void appendOptionalName(core::Name Value)
                {
                    const std::string_view Spelling = detail::nameText(Value);
                    if (!Spelling.empty())
                    {
                        Output.push_back(' ');
                        detail::appendQuotedBytes(Output, {}, Spelling);
                    }
                }

                bool writeImportReference(const ModuleImport& Import)
                {
                    const std::string* Id = Slots.findImportId(Import);
                    if (!Id)
                    {
                        return fail("module text cannot represent a module import owned outside the module");
                    }
                    Output += *Id;
                    return true;
                }

                bool writeImport(const ModuleImport& Import)
                {
                    Output += detail::ImportKeyword;
                    Output.push_back(' ');
                    if (!writeImportReference(Import))
                    {
                        return false;
                    }
                    Output += " = ";
                    detail::appendQuotedBytes(Output, {}, detail::nameText(Import.qualifiedName()));
                    Output.push_back('\n');
                    return true;
                }

                bool writeValueReference(const Value& Node)
                {
                    const std::string* Id = Slots.findValueId(Node);
                    if (!Id)
                    {
                        return fail("module text cannot represent a value owned outside the module and its reachable records");
                    }
                    Output += *Id;
                    return true;
                }

                bool writeBlockReference(const BasicBlock& Block)
                {
                    const std::string* Id = Slots.findBlockId(Block);
                    if (!Id)
                    {
                        return fail("module text cannot represent a block owned outside the module and its reachable records");
                    }
                    Output += *Id;
                    return true;
                }

                bool writeType(const Value& Node, std::size_t Depth = 0)
                {
                    if (Depth >= detail::MaxTypeExpressionDepth)
                    {
                        return fail("module text type nesting exceeds the supported depth of 128");
                    }
                    if (Type::classof(&Node))
                    {
                        const std::size_t SavedDepth = TypeDepth;
                        TypeDepth = Depth;
                        const bool Ok = TypeVisitor<ModuleTextWriter, bool>::visit(static_cast<const Type&>(Node));
                        TypeDepth = SavedDepth;
                        return Ok;
                    }
                    if (Node.valueKind() == ValueKind::Argument || Node.valueKind() == ValueKind::Instruction)
                    {
                        return writeValueReference(Node);
                    }
                    return fail("module text encountered an unsupported type value");
                }

                bool writeValue(const Value& Node)
                {
                    if (Type::classof(&Node))
                    {
                        return writeType(Node);
                    }
                    switch (Node.valueKind())
                    {
                    case ValueKind::IntegerConstant:
                        Output += std::to_string(static_cast<const IntegerConstant&>(Node).bits());
                        return true;
                    case ValueKind::FloatConstant:
                        Output += std::to_string(static_cast<const FloatConstant&>(Node).bits());
                        return true;
                    case ValueKind::StringConstant:
                        detail::appendQuotedBytes(Output, "c", static_cast<const StringConstant&>(Node).value());
                        return true;
                    case ValueKind::Argument:
                    case ValueKind::Instruction:
                    case ValueKind::Function:
                    case ValueKind::GlobalVariable:
                        return writeValueReference(Node);
                    default:
                        return fail("module text encountered an unsupported value kind");
                    }
                }

                bool writeTypedOperand(const Value& Node)
                {
                    return writeType(Node.getType()) && appendChar(' ') && writeValue(Node);
                }

                bool writeRecord(const RecordType& Record)
                {
                    if (!writeValueReference(Record))
                    {
                        return false;
                    }
                    Output += " = ";
                    Output += detail::TypeKeyword;
                    Output.push_back(' ');
                    Output += detail::enumSpelling(ObjectType::classof(&Record), detail::RecordKindEntries);
                    if (const ModuleImport* ImportSource = ModuleValue.findRecordImportSource(Record))
                    {
                        Output.push_back(' ');
                        Output += detail::FromKeyword;
                        Output.push_back(' ');
                        if (!writeImportReference(*ImportSource))
                        {
                            return false;
                        }
                    }
                    else if (!Slots.isRegistered(Record))
                    {
                        Output.push_back(' ');
                        Output += detail::BorrowedKeyword;
                    }
                    appendOptionalName(Record.name());
                    const auto Bases = Record.bases();
                    if (!Bases.empty())
                    {
                        Output.push_back(' ');
                        Output += detail::ExtendsKeyword;
                        Output.push_back('(');
                        for (std::size_t Index = 0; Index < Bases.size(); ++Index)
                        {
                            if (Index != 0)
                            {
                                Output += ", ";
                            }
                            if (!writeType(*Bases[Index]))
                            {
                                return false;
                            }
                        }
                        Output.push_back(')');
                    }
                    Output += "\n{\n";
                    for (const Field& FieldValue : Record.fields())
                    {
                        appendIndent(1);
                        Output += detail::FieldKeyword;
                        Output.push_back(' ');
                        Output += detail::enumSpelling(FieldValue.visibility(), detail::VisibilityEntries);
                        Output.push_back(' ');
                        Output += detail::enumSpelling(FieldValue.kind(), detail::MemberKindEntries);
                        Output.push_back(' ');
                        if (!writeType(FieldValue.type()))
                        {
                            return false;
                        }
                        appendOptionalName(FieldValue.name());
                        Output.push_back('\n');
                    }
                    for (const Method& MethodValue : Record.methods())
                    {
                        appendIndent(1);
                        Output += detail::MethodKeyword;
                        Output.push_back(' ');
                        Output += detail::enumSpelling(MethodValue.kind(), detail::MemberKindEntries);
                        Output.push_back(' ');
                        if (!writeFunction(MethodValue.function(), 1, false))
                        {
                            return false;
                        }
                    }
                    Output += "}\n";
                    return true;
                }

                bool writeGlobal(const GlobalVariable& Global)
                {
                    if (!writeValueReference(Global))
                    {
                        return false;
                    }
                    appendOptionalName(Global.name());
                    Output += " = ";
                    Output += detail::enumSpelling(Global.visibility(), detail::VisibilityEntries);
                    if (const ModuleImport* ImportSource = Global.importSource())
                    {
                        Output.push_back(' ');
                        Output += detail::FromKeyword;
                        Output.push_back(' ');
                        if (!writeImportReference(*ImportSource))
                        {
                            return false;
                        }
                    }
                    else if (Global.nativeLinkage() != NativeLinkage::None)
                    {
                        Output.push_back(' ');
                        Output += detail::enumSpelling(Global.nativeLinkage(), detail::LinkageEntries);
                    }
                    Output.push_back(' ');
                    Output += detail::GlobalKeyword;
                    Output.push_back(' ');
                    if (!writeType(Global.valueType()))
                    {
                        return false;
                    }
                    Output.push_back('\n');
                    return true;
                }

                bool writeFunction(const Function& FunctionValue, std::size_t IndentLevel, bool EmitLeadingIndent = true)
                {
                    if (EmitLeadingIndent)
                    {
                        appendIndent(IndentLevel);
                    }
                    const bool HasBody = FunctionValue.basicBlockCount() != 0;
                    Output += detail::enumSpelling(HasBody, detail::FunctionKindEntries);
                    Output.push_back(' ');
                    Output += detail::enumSpelling(FunctionValue.visibility(), detail::VisibilityEntries);
                    if (const ModuleImport* ImportSource = FunctionValue.importSource())
                    {
                        Output.push_back(' ');
                        Output += detail::FromKeyword;
                        Output.push_back(' ');
                        if (!writeImportReference(*ImportSource))
                        {
                            return false;
                        }
                    }
                    else if (FunctionValue.nativeLinkage() != NativeLinkage::None)
                    {
                        Output.push_back(' ');
                        Output += detail::enumSpelling(FunctionValue.nativeLinkage(), detail::LinkageEntries);
                    }
                    const FunctionType& Signature = FunctionValue.signature();
                    if (Signature.abi() != ABIKind::Ink)
                    {
                        Output.push_back(' ');
                        Output += detail::enumSpelling(Signature.abi(), detail::ABIEntries);
                    }
                    if (Signature.callingConvention() != CallingConvention::Default)
                    {
                        Output.push_back(' ');
                        Output += detail::enumSpelling(Signature.callingConvention(), detail::ConventionEntries);
                    }
                    Output.push_back(' ');
                    if (!writeType(Signature.returnType()) || !appendChar(' ') || !writeValueReference(FunctionValue))
                    {
                        return false;
                    }
                    appendOptionalName(FunctionValue.name());
                    Output.push_back('(');
                    const auto Parameters = Signature.parameters();
                    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
                    {
                        if (Index != 0)
                        {
                            Output += ", ";
                        }
                        if (Parameters[Index].kind() != Parameter::Kind::Positional)
                        {
                            Output += detail::enumSpelling(Parameters[Index].kind(), detail::ParameterKindEntries);
                            Output.push_back(' ');
                        }
                        const Argument& ArgumentValue = FunctionValue.argument(Index);
                        if (!writeType(Parameters[Index].type()) || !appendChar(' ') || !writeValueReference(ArgumentValue))
                        {
                            return false;
                        }
                        appendOptionalName(ArgumentValue.name());
                    }
                    if (Signature.isVariadic())
                    {
                        if (!Parameters.empty())
                        {
                            Output += ", ";
                        }
                        Output += detail::EllipsisToken;
                    }
                    Output.push_back(')');
                    if (!HasBody)
                    {
                        Output.push_back('\n');
                        return true;
                    }
                    Output.push_back('\n');
                    appendIndent(IndentLevel);
                    Output += "{\n";
                    for (const BasicBlock& Block : FunctionValue.basicBlocks())
                    {
                        appendIndent(IndentLevel + 1);
                        if (!writeBlockReference(Block))
                        {
                            return false;
                        }
                        appendOptionalName(Block.name());
                        Output += ":\n";
                        for (const Instruction& Node : Block.instructions())
                        {
                            appendIndent(IndentLevel + 2);
                            if (!VoidType::classof(&Node.getType()))
                            {
                                if (!writeValueReference(Node))
                                {
                                    return false;
                                }
                                Output += " = ";
                            }
                            Output += detail::instructionDescriptor(Node.getKind()).Mnemonic;
                            Output.push_back(' ');
                            if (!InstructionVisitor<ModuleTextWriter, bool>::visit(Node))
                            {
                                return false;
                            }
                            Output.push_back('\n');
                        }
                    }
                    appendIndent(IndentLevel);
                    Output += "}\n";
                    return true;
                }

                const Module& ModuleValue;
                SlotTracker Slots;
                std::string Output = {};
                std::size_t TypeDepth = 0;
                std::optional<core::Diagnostic> Failure = std::nullopt;
        };
    } // namespace

    std::expected<std::string, core::Diagnostic> writeModuleText(const Module& ModuleValue)
    {
        std::string Error;
        if (!verifyModule(ModuleValue, &Error))
        {
            return std::unexpected(detail::makeTextDiagnostic(0, Error));
        }
        return ModuleTextWriter(ModuleValue).write();
    }
} // namespace ink::ir
