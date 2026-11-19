#include "ink/ir/verification/verifier.h"

#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_visitor.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/type/type_visitor.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
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
        constexpr std::size_t MaximumValidationDepth = 128;
        constexpr std::size_t MaximumTypeGraphDepth = 256;

        class ScopedDepth final
        {
            public:
                explicit ScopedDepth(std::size_t& Depth) noexcept
                    : Depth(Depth)
                {
                    ++Depth;
                }

                ~ScopedDepth()
                {
                    --Depth;
                }

                ScopedDepth(const ScopedDepth&) = delete;
                ScopedDepth& operator=(const ScopedDepth&) = delete;

            private:
                std::size_t& Depth;
        };

        constexpr bool isValidVisibility(SymbolVisibility Visibility) noexcept
        {
            return Visibility == SymbolVisibility::Private || Visibility == SymbolVisibility::Public;
        }

        constexpr bool isValidLinkage(NativeLinkage Linkage) noexcept
        {
            return Linkage == NativeLinkage::None || Linkage == NativeLinkage::Import || Linkage == NativeLinkage::Export;
        }

        constexpr bool isValidMemberKind(MemberKind Kind) noexcept
        {
            return Kind == MemberKind::Instance || Kind == MemberKind::Static;
        }

        constexpr bool isValidParameterKind(Parameter::Kind Kind) noexcept
        {
            return Kind == Parameter::Kind::Positional || Kind == Parameter::Kind::Named;
        }

        constexpr bool isValidABI(ABIKind ABI) noexcept
        {
            return ABI == ABIKind::Ink || ABI == ABIKind::C;
        }

        constexpr bool isValidCallingConvention(CallingConvention Convention) noexcept
        {
            return Convention == CallingConvention::Default || Convention == CallingConvention::CDecl || Convention == CallingConvention::StdCall || Convention == CallingConvention::FastCall || Convention == CallingConvention::VectorCall;
        }

        constexpr bool isKnownTypeKind(TypeKind Kind) noexcept
        {
            switch (Kind)
            {
#define TYPE(Kind, Class, Spelling) \
    case TypeKind::Kind:            \
        return true;
#include "ink/ir/type/type.def"
#undef TYPE
            }
            return false;
        }

        class Verifier final : public TypeVisitor<Verifier, bool>
        {
            public:
                std::optional<VerificationError> verifyModuleGraph(const Module& ModuleValue)
                {
                    CurrentModule = &ModuleValue;
                    if (!ModuleValue.path().empty() && !ModuleValue.path().valid())
                    {
                        fail("module", "Module path is invalid");
                        return Failure;
                    }
                    std::unordered_set<core::Name> ImportPaths;
                    std::size_t ImportIndex = 0;
                    for (const ModuleImport& Import : ModuleValue.imports())
                    {
                        const std::string Context = fmt::format("module / import #{}", ImportIndex++);
                        if (Import.parent() != &ModuleValue)
                        {
                            fail(Context, "Module import has an incorrect parent");
                            return Failure;
                        }
                        if (!Import.path().valid())
                        {
                            fail(Context, "Module import path is invalid");
                            return Failure;
                        }
                        if (ModuleValue.path().valid() && Import.path() == ModuleValue.path())
                        {
                            fail(Context, "Module cannot import its own module path");
                            return Failure;
                        }
                        if (!ImportPaths.insert(Import.path().qualifiedName()).second)
                        {
                            fail(Context, "Module import path is duplicated");
                            return Failure;
                        }
                        KnownImports.insert(&Import);
                    }
                    std::unordered_set<const RecordType*> Registered;
                    std::size_t RecordIndex = 0;
                    for (const StructType& Record : ModuleValue.structTypes())
                    {
                        if (!Registered.insert(&Record).second)
                        {
                            fail("module", "Record type is registered more than once");
                            return Failure;
                        }
                        const std::string RecordContext = fmt::format("module / record #{}", RecordIndex++);
                        if (const ModuleImport* Source = ModuleValue.findRecordImportSource(Record))
                        {
                            if (!checkImportedDeclaration(*Source, Record.name(), RecordContext))
                            {
                                return Failure;
                            }
                        }
                        if (!checkType(Record, false, RecordContext))
                        {
                            return Failure;
                        }
                    }
                    for (const ObjectType& Record : ModuleValue.objectTypes())
                    {
                        if (!Registered.insert(&Record).second)
                        {
                            fail("module", "Record type is registered more than once");
                            return Failure;
                        }
                        const std::string RecordContext = fmt::format("module / record #{}", RecordIndex++);
                        if (const ModuleImport* Source = ModuleValue.findRecordImportSource(Record))
                        {
                            if (!checkImportedDeclaration(*Source, Record.name(), RecordContext))
                            {
                                return Failure;
                            }
                        }
                        if (!checkType(Record, false, RecordContext))
                        {
                            return Failure;
                        }
                    }
                    std::size_t GlobalIndex = 0;
                    for (const GlobalVariable& Global : ModuleValue.globalVariables())
                    {
                        const std::string Context = fmt::format("module / global #{}", GlobalIndex++);
                        if (Global.parent() != &ModuleValue || !isValidVisibility(Global.visibility()) || !isValidLinkage(Global.nativeLinkage()))
                        {
                            fail(Context, "Global variable has an invalid parent, visibility, or native linkage");
                            return Failure;
                        }
                        if (Global.isImported())
                        {
                            if (!checkImportedDeclaration(*Global.importSource(), Global.name(), Context))
                            {
                                return Failure;
                            }
                            if (Global.visibility() != SymbolVisibility::Public)
                            {
                                fail(Context, "Imported global variable must have public visibility");
                                return Failure;
                            }
                            if (Global.nativeLinkage() != NativeLinkage::None)
                            {
                                fail(Context, "Imported global variable must not specify native linkage");
                                return Failure;
                            }
                        }
                        if (!checkType(Global.getType(), false, Context))
                        {
                            return Failure;
                        }
                        if (!PointerType::classof(&Global.getType()))
                        {
                            fail(Context, "Global variable requires a pointer result type");
                            return Failure;
                        }
                        if (VoidType::classof(&Global.valueType()))
                        {
                            fail(Context, "Global variable stored type must not be void");
                            return Failure;
                        }
                    }
                    std::size_t FunctionIndex = 0;
                    for (const Function& FunctionValue : ModuleValue.functions())
                    {
                        const std::string Context = fmt::format("module / function #{}", FunctionIndex++);
                        if (FunctionValue.parent() != &ModuleValue)
                        {
                            fail(Context, "Module-owned function has an incorrect parent");
                            return Failure;
                        }
                        if (FunctionValue.isImported() && !checkImportedDeclaration(*FunctionValue.importSource(), FunctionValue.name(), Context))
                        {
                            return Failure;
                        }
                        if (!checkFunctionAttributes(FunctionValue, Context) || !FunctionVerifier(*this, FunctionValue, Context).verify())
                        {
                            return Failure;
                        }
                    }
                    for (std::size_t Index = 0; Index < Records.size(); ++Index)
                    {
                        const RecordType& Record = *Records[Index];
                        const std::string RecordContext = fmt::format("module / record #{}", Index);
                        if (!checkType(Record, false, RecordContext))
                        {
                            return Failure;
                        }
                        std::size_t MethodIndex = 0;
                        for (const Method& MethodValue : Record.methods())
                        {
                            const std::string MethodContext = fmt::format("{} / method #{}", RecordContext, MethodIndex++);
                            if (!MethodValue.hasFunction())
                            {
                                fail(MethodContext, "Method has no owned function");
                                return Failure;
                            }
                            if (!FunctionVerifier(*this, MethodValue.function(), MethodContext).verify())
                            {
                                return Failure;
                            }
                        }
                    }
                    if (!checkLayouts("module"))
                    {
                        return Failure;
                    }
                    return std::nullopt;
                }

                std::optional<VerificationError> verifyFunctionGraph(const Function& FunctionValue)
                {
                    const std::string_view Name = core::NamePool::instance().text(FunctionValue.name());
                    const std::string Context = Name.empty() ? std::string("unnamed function") : fmt::format("function '{}'", Name);
                    if (!FunctionVerifier(*this, FunctionValue, Context).verify() || !checkLayouts(Context))
                    {
                        return Failure;
                    }
                    return std::nullopt;
                }

                bool visitMetaType(const MetaType& TypeValue)
                {
                    return require(&TypeValue == &MetaType::instance(), "MetaType must be the canonical self-typed instance");
                }

                bool visitIntegerType(const IntegerType& TypeValue)
                {
                    const std::uint32_t Width = TypeValue.bitWidth();
                    if (!require(Width == 8 || Width == 16 || Width == 32 || Width == 64, "Integer type has an unsupported bit width"))
                    {
                        return false;
                    }
                    return require(TypeValue.signedness() == IntegerType::Signedness::Signed || TypeValue.signedness() == IntegerType::Signedness::Unsigned, "Integer type signedness is invalid");
                }

                bool visitFloatType(const FloatType& TypeValue)
                {
                    return require(TypeValue.bitWidth() == 32 || TypeValue.bitWidth() == 64, "Floating-point type has an unsupported bit width");
                }

                bool visitBoolType(const BoolType&)
                {
                    return true;
                }

                bool visitVoidType(const VoidType&)
                {
                    return true;
                }

                bool visitArrayType(const ArrayType& TypeValue)
                {
                    return require(!VoidType::classof(&TypeValue.elementType()), "Array element type must not be void") && checkType(TypeValue.elementType(), CurrentAllowInstructions, CurrentContext);
                }

                bool visitPointerType(const PointerType& TypeValue)
                {
                    return checkType(TypeValue.pointeeType(), CurrentAllowInstructions, CurrentContext);
                }

                bool visitStructType(const StructType& TypeValue)
                {
                    return checkRecord(TypeValue);
                }

                bool visitObjectType(const ObjectType& TypeValue)
                {
                    return checkRecord(TypeValue);
                }

                bool visitFunctionType(const FunctionType& TypeValue)
                {
                    if (!require(isValidABI(TypeValue.abi()) && isValidCallingConvention(TypeValue.callingConvention()), "Function ABI or calling convention is invalid"))
                    {
                        return false;
                    }
                    const bool SavedAllowInstructions = CurrentAllowInstructions;
                    const std::string_view SavedContext = CurrentContext;
                    if (!checkType(TypeValue.returnType(), SavedAllowInstructions, SavedContext))
                    {
                        return false;
                    }
                    for (const Parameter& ParameterValue : TypeValue.parameters())
                    {
                        if (!require(isValidParameterKind(ParameterValue.kind()), SavedContext, "Function parameter kind is invalid") || !require(!VoidType::classof(&ParameterValue.type()), SavedContext, "Function parameter type must not be void") || !checkType(ParameterValue.type(), SavedAllowInstructions, SavedContext))
                        {
                            return false;
                        }
                    }
                    return true;
                }

            private:
                struct InstructionLocation
                {
                        std::size_t Block;
                        std::size_t Position;
                };

                struct BlockInfo
                {
                        const BasicBlock* Block;
                        std::vector<std::size_t> Predecessors = {};
                        std::vector<std::size_t> Successors = {};
                        bool Reachable = false;
                };

                class FunctionVerifier final : public InstructionVisitor<FunctionVerifier, bool>
                {
                    public:
                        FunctionVerifier(Verifier& Owner, const Function& FunctionValue, std::string_view Context)
                            : Owner(Owner),
                              FunctionValue(FunctionValue),
                              Context(Context)
                        {
                        }

                        bool verify()
                        {
                            if (!verifySignature() || !collectDefinitions())
                            {
                                return false;
                            }
                            if (Blocks.empty())
                            {
                                return true;
                            }
                            if (!buildControlFlow())
                            {
                                return false;
                            }
                            computeDominators();
                            for (const BlockInfo& Info : Blocks)
                            {
                                for (const Instruction& Node : Info.Block->instructions())
                                {
                                    if (!Owner.checkType(Node.getType(), true, instructionContext(Node)) || !verifyTypeDependencies(Node.getType(), &Node) || !visit(Node))
                                    {
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }

                        bool visitAddInstruction(const AddInstruction& Node)
                        {
                            const Value& TypeValue = Node.getLHS().getType();
                            if (!require(&TypeValue == &Node.getRHS().getType() && &Node.getType() == &TypeValue, Node, "add operand and result types must match") || !require(IntegerType::classof(&TypeValue) || FloatType::classof(&TypeValue), Node, "add requires integer or floating-point operands"))
                            {
                                return false;
                            }
                            return verifyOperand(Node, Node.getLHS(), 0) && verifyOperand(Node, Node.getRHS(), 1);
                        }

                        bool visitCallInstruction(const CallInstruction& Node)
                        {
                            const Value& CalleeType = Node.getCallee().getType();
                            if (!require(FunctionType::classof(&CalleeType), Node, "call callee must have function type"))
                            {
                                return false;
                            }
                            const auto& Signature = static_cast<const FunctionType&>(CalleeType);
                            const auto Parameters = Signature.parameters();
                            if (!require(&Node.getType() == &Signature.returnType(), Node, "call result type must match the callee return type") || !require(Signature.isVariadic() ? Node.argumentCount() >= Parameters.size() : Node.argumentCount() == Parameters.size(), Node, "call argument count does not match the callee signature"))
                            {
                                return false;
                            }
                            for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
                            {
                                if (!require(&Node.argument(Index).getType() == &Parameters[Index].type(), Node, "call argument type does not match the callee signature"))
                                {
                                    return false;
                                }
                            }
                            for (std::size_t Index = 0; Index < Node.operandCount(); ++Index)
                            {
                                if (!verifyOperand(Node, Node.operand(Index), Index))
                                {
                                    return false;
                                }
                            }
                            return true;
                        }

                        bool visitAllocaInstruction(const AllocaInstruction& Node)
                        {
                            if (!require(PointerType::classof(&Node.getType()), Node, "alloca result must have pointer type"))
                            {
                                return false;
                            }
                            const Value& AllocatedType = static_cast<const PointerType&>(Node.getType()).pointeeType();
                            return require(!VoidType::classof(&AllocatedType), Node, "alloca cannot allocate void") && Owner.checkType(AllocatedType, true, instructionContext(Node)) && verifyTypeDependencies(AllocatedType, &Node);
                        }

                        bool visitLoadInstruction(const LoadInstruction& Node)
                        {
                            const Value& SourceType = Node.getSource().getType();
                            if (!require(PointerType::classof(&SourceType), Node, "load source must have pointer type") || !require(&static_cast<const PointerType&>(SourceType).pointeeType() == &Node.getType(), Node, "load result type must match its source pointee type") || !require(!VoidType::classof(&Node.getType()), Node, "load cannot produce void"))
                            {
                                return false;
                            }
                            return verifyOperand(Node, Node.getSource(), 0);
                        }

                        bool visitStoreInstruction(const StoreInstruction& Node)
                        {
                            if (!require(VoidType::classof(&Node.getType()), Node, "store result must have void type"))
                            {
                                return false;
                            }
                            const Value& DestinationType = Node.getDestination().getType();
                            if (!require(PointerType::classof(&DestinationType), Node, "store destination must have pointer type") || !require(&static_cast<const PointerType&>(DestinationType).pointeeType() == &Node.getSource().getType(), Node, "store source type must match its destination pointee type"))
                            {
                                return false;
                            }
                            return verifyOperand(Node, Node.getDestination(), 0) && verifyOperand(Node, Node.getSource(), 1);
                        }

                        bool visitReturnInstruction(const ReturnInstruction& Node)
                        {
                            if (!require(VoidType::classof(&Node.getType()), Node, "return instruction result must have void type"))
                            {
                                return false;
                            }
                            const Value& ReturnType = FunctionValue.signature().returnType();
                            const Value* Returned = Node.getReturnValue();
                            if (VoidType::classof(&ReturnType))
                            {
                                return require(Returned == nullptr, Node, "void function cannot return a value");
                            }
                            if (!require(Returned != nullptr, Node, "non-void function must return a value") || !require(&Returned->getType() == &ReturnType, Node, "return value type does not match the function signature"))
                            {
                                return false;
                            }
                            return verifyOperand(Node, *Returned, 0);
                        }

                        bool visitBranchInstruction(const BranchInstruction& Node)
                        {
                            return require(VoidType::classof(&Node.getType()), Node, "branch result must have void type");
                        }

                        bool visitCondBranchInstruction(const CondBranchInstruction& Node)
                        {
                            if (!require(VoidType::classof(&Node.getType()), Node, "conditional branch result must have void type") || !require(BoolType::classof(&Node.getCondition().getType()), Node, "conditional branch requires a boolean condition"))
                            {
                                return false;
                            }
                            return verifyOperand(Node, Node.getCondition(), 0);
                        }

                        bool visitPhiInstruction(const PhiInstruction& Node)
                        {
                            if (!require(!VoidType::classof(&Node.getType()), Node, "phi cannot produce void"))
                            {
                                return false;
                            }
                            const InstructionLocation& Location = Definitions.at(&Node);
                            const auto& Predecessors = Blocks[Location.Block].Predecessors;
                            if (!require(!Predecessors.empty(), Node, "phi requires at least one predecessor") || !require(Node.incomingCount() == Predecessors.size(), Node, "phi incoming edges do not match CFG predecessors"))
                            {
                                return false;
                            }
                            std::vector<std::size_t> IncomingBlocks;
                            std::unordered_map<const BasicBlock*, const Value*> IncomingValues;
                            IncomingBlocks.reserve(Node.incomingCount());
                            for (std::size_t Index = 0; Index < Node.incomingCount(); ++Index)
                            {
                                const Value& Incoming = Node.incomingValue(Index);
                                if (!require(&Incoming.getType() == &Node.getType(), Node, "phi incoming type does not match its result type"))
                                {
                                    return false;
                                }
                                const BasicBlock& Predecessor = Node.incomingBlock(Index);
                                const auto Found = BlockIndices.find(&Predecessor);
                                if (!require(Found != BlockIndices.end(), Node, "phi incoming edges do not match CFG predecessors"))
                                {
                                    return false;
                                }
                                IncomingBlocks.push_back(Found->second);
                                const auto [Previous, Inserted] = IncomingValues.emplace(&Predecessor, &Incoming);
                                if (!require(Inserted || Previous->second == &Incoming, Node, "phi has different values for repeated predecessor edges"))
                                {
                                    return false;
                                }
                            }
                            std::sort(IncomingBlocks.begin(), IncomingBlocks.end());
                            std::vector<std::size_t> Expected = Predecessors;
                            std::sort(Expected.begin(), Expected.end());
                            if (!require(IncomingBlocks == Expected, Node, "phi incoming edges do not match CFG predecessors"))
                            {
                                return false;
                            }
                            for (std::size_t Index = 0; Index < Node.incomingCount(); ++Index)
                            {
                                if (!verifyOperand(Node, Node.incomingValue(Index), Index, &Node.incomingBlock(Index)))
                                {
                                    return false;
                                }
                            }
                            return true;
                        }

                    private:
                        bool fail(std::string_view Description) const
                        {
                            return Owner.fail(Context, Description);
                        }

                        std::string instructionContext(const Instruction& Node) const
                        {
                            const InstructionLocation& Location = Definitions.at(&Node);
                            return fmt::format("{} / block #{} / instruction #{} ({})", Context, Location.Block, Location.Position, Node.getName());
                        }

                        bool require(bool Condition, const Instruction& Node, std::string_view Description) const
                        {
                            return Condition || Owner.fail(instructionContext(Node), Description);
                        }

                        bool verifySignature()
                        {
                            if (!FunctionType::classof(&FunctionValue.getType()))
                            {
                                return fail("function signature must have function type");
                            }
                            if (!Owner.checkType(FunctionValue.getType(), false, Context) || !verifyTypeDependencies(FunctionValue.getType()))
                            {
                                return false;
                            }
                            if (!isValidVisibility(FunctionValue.visibility()))
                            {
                                return fail("function has invalid visibility");
                            }
                            if (!isValidLinkage(FunctionValue.nativeLinkage()))
                            {
                                return fail("function has invalid native linkage");
                            }
                            if (FunctionValue.isImported())
                            {
                                if (!FunctionValue.parent() || FunctionValue.importSource()->parent() != FunctionValue.parent())
                                {
                                    return fail("imported function has an invalid module import owner");
                                }
                                if (!FunctionValue.importSource()->path().valid())
                                {
                                    return fail("imported function references an invalid module import path");
                                }
                                if (!core::NamePool::instance().contains(FunctionValue.name()))
                                {
                                    return fail("imported function requires a valid symbol name");
                                }
                                if (FunctionValue.visibility() != SymbolVisibility::Public)
                                {
                                    return fail("imported function must have public visibility");
                                }
                                if (FunctionValue.nativeLinkage() != NativeLinkage::None)
                                {
                                    return fail("imported function must not specify native linkage");
                                }
                                if (FunctionValue.basicBlockCount() != 0)
                                {
                                    return fail("imported function declaration cannot have a body");
                                }
                            }
                            const auto Parameters = FunctionValue.signature().parameters();
                            if (FunctionValue.argumentCount() != Parameters.size())
                            {
                                return fail("function argument count does not match its signature");
                            }
                            std::size_t Index = 0;
                            for (const Argument& ArgumentValue : FunctionValue.arguments())
                            {
                                if (ArgumentValue.parent() != &FunctionValue || ArgumentValue.index() != Index)
                                {
                                    return fail("function argument has invalid ownership or index");
                                }
                                if (&ArgumentValue.getType() != &Parameters[Index].type())
                                {
                                    return fail("function argument type does not match its signature");
                                }
                                Arguments.insert(&ArgumentValue);
                                ++Index;
                            }
                            return true;
                        }

                        bool collectDefinitions()
                        {
                            for (const BasicBlock& Block : FunctionValue.basicBlocks())
                            {
                                if (Block.parent() != &FunctionValue)
                                {
                                    return fail("basic block belongs to another function");
                                }
                                const std::size_t BlockIndex = Blocks.size();
                                BlockIndices.emplace(&Block, BlockIndex);
                                Blocks.push_back({&Block});
                                std::size_t Position = 0;
                                bool SawNonPhi = false;
                                const Instruction* Last = nullptr;
                                for (const Instruction& Node : Block.instructions())
                                {
                                    Definitions.emplace(&Node, InstructionLocation{BlockIndex, Position});
                                    switch (Node.getKind())
                                    {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
    case InstructionKind::Kind:                    \
        break;
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
                                    default:
                                        return Owner.fail(fmt::format("{} / block #{} / instruction #{}", Context, BlockIndex, Position), "instruction has an invalid kind");
                                    }
                                    if (!require(Node.parent() == &Block, Node, "instruction belongs to another basic block"))
                                    {
                                        return false;
                                    }
                                    if (Node.getKind() == InstructionKind::Phi)
                                    {
                                        if (!require(!SawNonPhi, Node, "phi instructions must form a prefix at the start of a block"))
                                        {
                                            return false;
                                        }
                                    }
                                    else
                                    {
                                        SawNonPhi = true;
                                    }
                                    if (!require(!Node.isTerminator() || Position + 1 == Block.instructionCount(), Node, "terminator must be the last instruction in its basic block"))
                                    {
                                        return false;
                                    }
                                    Last = &Node;
                                    ++Position;
                                }
                                if (!Last || !Last->isTerminator())
                                {
                                    return Owner.fail(fmt::format("{} / block #{}", Context, BlockIndex), "basic block is missing a terminator");
                                }
                            }
                            return true;
                        }

                        bool addEdge(std::size_t Source, const BasicBlock& Target)
                        {
                            const auto Found = BlockIndices.find(&Target);
                            if (Found == BlockIndices.end())
                            {
                                return Owner.fail(fmt::format("{} / block #{}", Context, Source), "branch target does not belong to this function");
                            }
                            Blocks[Source].Successors.push_back(Found->second);
                            Blocks[Found->second].Predecessors.push_back(Source);
                            return true;
                        }

                        bool buildControlFlow()
                        {
                            for (std::size_t Index = 0; Index < Blocks.size(); ++Index)
                            {
                                const Instruction& Last = *Blocks[Index].Block->terminator();
                                if (BranchInstruction::classof(&Last))
                                {
                                    if (!addEdge(Index, static_cast<const BranchInstruction&>(Last).getTarget()))
                                    {
                                        return false;
                                    }
                                }
                                else if (CondBranchInstruction::classof(&Last))
                                {
                                    const auto& Branch = static_cast<const CondBranchInstruction&>(Last);
                                    if (!addEdge(Index, Branch.getTrueTarget()) || !addEdge(Index, Branch.getFalseTarget()))
                                    {
                                        return false;
                                    }
                                }
                            }
                            if (!Blocks.front().Predecessors.empty())
                            {
                                return fail("entry block must not have predecessors");
                            }
                            std::vector<std::size_t> Worklist = {0};
                            Blocks.front().Reachable = true;
                            while (!Worklist.empty())
                            {
                                const std::size_t Current = Worklist.back();
                                Worklist.pop_back();
                                for (std::size_t Successor : Blocks[Current].Successors)
                                {
                                    if (!Blocks[Successor].Reachable)
                                    {
                                        Blocks[Successor].Reachable = true;
                                        Worklist.push_back(Successor);
                                    }
                                }
                            }
                            return true;
                        }

                        void computeDominators()
                        {
                            const std::size_t Count = Blocks.size();
                            std::vector<bool> Reachable;
                            Reachable.reserve(Count);
                            for (const BlockInfo& Info : Blocks)
                            {
                                Reachable.push_back(Info.Reachable);
                            }
                            Dominators.assign(Count, Reachable);
                            Dominators[0].assign(Count, false);
                            Dominators[0][0] = true;
                            bool Changed = true;
                            while (Changed)
                            {
                                Changed = false;
                                for (std::size_t Index = 1; Index < Count; ++Index)
                                {
                                    if (!Blocks[Index].Reachable)
                                    {
                                        continue;
                                    }
                                    std::vector<bool> Next = Reachable;
                                    for (std::size_t Predecessor : Blocks[Index].Predecessors)
                                    {
                                        if (!Blocks[Predecessor].Reachable)
                                        {
                                            continue;
                                        }
                                        for (std::size_t Candidate = 0; Candidate < Count; ++Candidate)
                                        {
                                            Next[Candidate] = Next[Candidate] && Dominators[Predecessor][Candidate];
                                        }
                                    }
                                    Next[Index] = true;
                                    if (Next != Dominators[Index])
                                    {
                                        Dominators[Index] = std::move(Next);
                                        Changed = true;
                                    }
                                }
                            }
                        }

                        bool verifyTypeDependencies(const Value& TypeValue, const Instruction* User = nullptr, const BasicBlock* IncomingBlock = nullptr) const
                        {
                            std::vector<const Value*> Worklist = {&TypeValue};
                            std::unordered_set<const Value*> Visited;
                            while (!Worklist.empty())
                            {
                                const Value& Current = *Worklist.back();
                                Worklist.pop_back();
                                if (!Visited.insert(&Current).second)
                                {
                                    continue;
                                }
                                if (Current.valueKind() == ValueKind::Instruction)
                                {
                                    if (!User)
                                    {
                                        return fail("function signature cannot depend on an instruction");
                                    }
                                    if (!verifyOperand(*User, Current, 0, IncomingBlock, true))
                                    {
                                        return false;
                                    }
                                    continue;
                                }
                                if (Current.valueKind() != ValueKind::Type)
                                {
                                    continue;
                                }
                                const auto* Concrete = static_cast<const Type*>(&Current);
                                switch (Concrete->getKind())
                                {
                                case TypeKind::Pointer:
                                    Worklist.push_back(&static_cast<const PointerType&>(*Concrete).pointeeType());
                                    break;
                                case TypeKind::Array:
                                    Worklist.push_back(&static_cast<const ArrayType&>(*Concrete).elementType());
                                    break;
                                case TypeKind::Function:
                                    {
                                        const auto& Signature = static_cast<const FunctionType&>(*Concrete);
                                        Worklist.push_back(&Signature.returnType());
                                        for (const Parameter& ParameterValue : Signature.parameters())
                                        {
                                            Worklist.push_back(&ParameterValue.type());
                                        }
                                        break;
                                    }
                                case TypeKind::Struct:
                                case TypeKind::Object:
                                    {
                                        const auto& Record = static_cast<const RecordType&>(*Concrete);
                                        for (const Value* Base : Record.bases())
                                        {
                                            if (Base)
                                            {
                                                Worklist.push_back(Base);
                                            }
                                        }
                                        for (const Field& FieldValue : Record.fields())
                                        {
                                            Worklist.push_back(&FieldValue.type());
                                        }
                                        for (const Method& MethodValue : Record.methods())
                                        {
                                            if (!MethodValue.hasFunction())
                                            {
                                                return fail("record method has no owned function");
                                            }
                                            Worklist.push_back(&MethodValue.function().getType());
                                        }
                                        break;
                                    }
                                default:
                                    break;
                                }
                            }
                            return true;
                        }

                        bool verifyOperand(const Instruction& User, const Value& Operand, std::size_t Index, const BasicBlock* IncomingBlock = nullptr, bool TypeDependency = false) const
                        {
                            const std::string OperandContext = fmt::format("{} / {}", instructionContext(User), TypeDependency ? std::string("type dependency") : fmt::format("operand #{}", Index));
                            const auto Reject = [&](std::string_view Description)
                            {
                                return Owner.fail(OperandContext, Description);
                            };
                            if (!Owner.checkType(Operand.getType(), true, OperandContext) || !verifyTypeDependencies(Operand.getType(), &User, IncomingBlock))
                            {
                                return false;
                            }
                            if (VoidType::classof(&Operand.getType()))
                            {
                                return Reject("void value cannot be used as an operand");
                            }
                            switch (Operand.valueKind())
                            {
                            case ValueKind::Type:
                                return Owner.checkType(Operand, true, OperandContext) && verifyTypeDependencies(Operand, &User, IncomingBlock);
                            case ValueKind::IntegerConstant:
                            case ValueKind::FloatConstant:
                            case ValueKind::StringConstant:
                                return Owner.checkConstant(Operand, OperandContext);
                            case ValueKind::Function:
                                {
                                    if (!FunctionType::classof(&Operand.getType()))
                                    {
                                        return Reject("function operand has an invalid signature");
                                    }
                                    const auto& ReferencedFunction = static_cast<const Function&>(Operand);
                                    const Module* ExpectedModule = Owner.CurrentModule ? Owner.CurrentModule : FunctionValue.parent();
                                    if (ExpectedModule && ReferencedFunction.parent() != nullptr && ReferencedFunction.parent() != ExpectedModule)
                                    {
                                        return Reject("function operand belongs to another module");
                                    }
                                    return true;
                                }
                            case ValueKind::GlobalVariable:
                                {
                                    if (!PointerType::classof(&Operand.getType()))
                                    {
                                        return Reject("global variable operand must have pointer type");
                                    }
                                    if (VoidType::classof(&static_cast<const PointerType&>(Operand.getType()).pointeeType()))
                                    {
                                        return Reject("global variable cannot store void");
                                    }
                                    const auto& ReferencedGlobal = static_cast<const GlobalVariable&>(Operand);
                                    const Module* ExpectedModule = Owner.CurrentModule ? Owner.CurrentModule : FunctionValue.parent();
                                    if (ExpectedModule && ReferencedGlobal.parent() != ExpectedModule)
                                    {
                                        return Reject("global variable operand belongs to another module");
                                    }
                                    return true;
                                }
                            case ValueKind::Argument:
                                return Arguments.contains(static_cast<const Argument*>(&Operand)) || Reject("operand belongs to another function");
                            case ValueKind::Instruction:
                                break;
                            default:
                                return Reject("operand has an unsupported value kind");
                            }
                            const auto& InstructionValue = static_cast<const Instruction&>(Operand);
                            if (!InstructionValue.parent())
                            {
                                return Reject("operand instruction is detached");
                            }
                            if (InstructionValue.parent()->parent() != &FunctionValue)
                            {
                                return Reject("operand belongs to another function");
                            }
                            const auto Found = Definitions.find(&InstructionValue);
                            if (Found == Definitions.end())
                            {
                                return Reject("operand instruction is not owned by this function");
                            }
                            const InstructionLocation& Defined = Found->second;
                            const InstructionLocation& Used = Definitions.at(&User);
                            const std::size_t UseBlock = IncomingBlock ? BlockIndices.at(IncomingBlock) : Used.Block;
                            const std::size_t UsePosition = IncomingBlock ? Blocks[UseBlock].Block->instructionCount() - 1 : Used.Position;
                            if (Defined.Block == UseBlock)
                            {
                                if (Defined.Position >= UsePosition)
                                {
                                    return Reject("definition does not precede its use");
                                }
                            }
                            else if (Blocks[UseBlock].Reachable && !Dominators[UseBlock][Defined.Block])
                            {
                                return Reject("definition does not dominate its use");
                            }
                            return true;
                        }

                        Verifier& Owner;
                        const Function& FunctionValue;
                        std::string Context;
                        std::unordered_set<const Argument*> Arguments = {};
                        std::vector<BlockInfo> Blocks = {};
                        std::unordered_map<const BasicBlock*, std::size_t> BlockIndices = {};
                        std::unordered_map<const Instruction*, InstructionLocation> Definitions = {};
                        std::vector<std::vector<bool>> Dominators = {};
                };

                bool fail(std::string_view Context, std::string_view Description)
                {
                    if (!Failure)
                    {
                        Failure = VerificationError{std::string(Context), std::string(Description)};
                    }
                    return false;
                }

                bool require(bool Condition, std::string_view Description)
                {
                    return Condition || fail(CurrentContext, Description);
                }

                bool require(bool Condition, std::string_view Context, std::string_view Description)
                {
                    return Condition || fail(Context, Description);
                }

                bool checkFunctionAttributes(const Function& FunctionValue, std::string_view Context)
                {
                    return require(isValidVisibility(FunctionValue.visibility()) && isValidLinkage(FunctionValue.nativeLinkage()), Context, "Function visibility or native linkage is invalid");
                }

                bool checkType(const Value& TypeValue, bool AllowInstructions, std::string_view Context)
                {
                    if (TypeDepth >= MaximumTypeGraphDepth)
                    {
                        return fail(Context, "type graph exceeds supported validation depth");
                    }
                    const ScopedDepth Depth(TypeDepth);
                    if (!require(&TypeValue.getType() == &MetaType::instance(), Context, "Type position requires a value with the canonical MetaType"))
                    {
                        return false;
                    }
                    if (TypeValue.valueKind() != ValueKind::Type)
                    {
                        if (!require(TypeValue.valueKind() == ValueKind::Argument || TypeValue.valueKind() == ValueKind::Instruction, Context, "Type position contains an unsupported type-valued object"))
                        {
                            return false;
                        }
                        return require(AllowInstructions || TypeValue.valueKind() != ValueKind::Instruction, Context, "Declaration type metadata must not depend on a function-body instruction");
                    }
                    const auto* Concrete = static_cast<const Type*>(&TypeValue);
                    if (!require(isKnownTypeKind(Concrete->getKind()), Context, "Type kind is invalid"))
                    {
                        return false;
                    }
                    const TypeSemantics Expected = Concrete->getKind() == TypeKind::Object ? TypeSemantics::Reference : TypeSemantics::Value;
                    if (!require(Concrete->getSemantics() == Expected, Context, "Type semantics do not match its concrete kind"))
                    {
                        return false;
                    }
                    if (DeclarationTypes.contains(Concrete) || (AllowInstructions && BodyTypes.contains(Concrete)))
                    {
                        return true;
                    }
                    if (!AllowInstructions)
                    {
                        DeclarationTypes.insert(Concrete);
                    }
                    if (BodyTypes.insert(Concrete).second)
                    {
                        OrderedBodyTypes.push_back(Concrete);
                    }
                    const bool SavedAllowInstructions = CurrentAllowInstructions;
                    const std::string_view SavedContext = CurrentContext;
                    CurrentAllowInstructions = AllowInstructions;
                    CurrentContext = Context;
                    const bool Ok = visit(*Concrete);
                    CurrentAllowInstructions = SavedAllowInstructions;
                    CurrentContext = SavedContext;
                    return Ok;
                }

                bool checkRecord(const RecordType& Record)
                {
                    const std::string_view RecordContext = CurrentContext;
                    if (KnownRecords.insert(&Record).second)
                    {
                        Records.push_back(&Record);
                    }
                    for (const Value* Base : Record.bases())
                    {
                        if (!require(Base != nullptr, RecordContext, "Record has a null base type") || !checkType(*Base, false, RecordContext) || !require(Base->valueKind() != ValueKind::Type || RecordType::classof(Base), RecordContext, "Concrete base type must be a record type"))
                        {
                            return false;
                        }
                    }
                    for (const Field& FieldValue : Record.fields())
                    {
                        if (!require(isValidMemberKind(FieldValue.kind()) && isValidVisibility(FieldValue.visibility()), RecordContext, "Field kind or visibility is invalid") || !require(!VoidType::classof(&FieldValue.type()), RecordContext, "Field type must not be void") || !checkType(FieldValue.type(), false, RecordContext))
                        {
                            return false;
                        }
                    }
                    for (const Method& MethodValue : Record.methods())
                    {
                        if (!require(isValidMemberKind(MethodValue.kind()), RecordContext, "Method kind is invalid") || !require(MethodValue.hasFunction(), RecordContext, "Method has no owned function"))
                        {
                            return false;
                        }
                        const Function& FunctionValue = MethodValue.function();
                        if (!require(FunctionValue.parent() == nullptr, RecordContext, "Record-owned method function must not have a module parent") || !require(!FunctionValue.isImported(), RecordContext, "Record-owned method function must not have a module import source") || !checkFunctionAttributes(FunctionValue, RecordContext) || !require(FunctionType::classof(&FunctionValue.getType()), RecordContext, "Method requires a concrete function signature") || !checkType(FunctionValue.getType(), false, RecordContext))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                bool checkConstant(const Value& ConstantValue, std::string_view Context)
                {
                    if (!checkType(ConstantValue.getType(), true, Context))
                    {
                        return false;
                    }
                    switch (ConstantValue.valueKind())
                    {
                    case ValueKind::IntegerConstant:
                        {
                            if (!IntegerType::classof(&ConstantValue.getType()))
                            {
                                return fail(Context, "Integer constant requires an integer type");
                            }
                            const auto Width = static_cast<const IntegerType&>(ConstantValue.getType()).bitWidth();
                            const auto Bits = static_cast<const IntegerConstant&>(ConstantValue).bits();
                            return Width == 64 || (Bits >> Width) == 0 || fail(Context, "Integer constant has bits outside its declared width");
                        }
                    case ValueKind::FloatConstant:
                        {
                            if (!FloatType::classof(&ConstantValue.getType()))
                            {
                                return fail(Context, "Floating-point constant requires a floating-point type");
                            }
                            const auto Width = static_cast<const FloatType&>(ConstantValue.getType()).bitWidth();
                            const auto Bits = static_cast<const FloatConstant&>(ConstantValue).bits();
                            return Width == 64 || (Bits >> Width) == 0 || fail(Context, "Floating-point constant has bits outside its declared width");
                        }
                    case ValueKind::StringConstant:
                        {
                            if (!ArrayType::classof(&ConstantValue.getType()))
                            {
                                return fail(Context, "String constant requires an array type");
                            }
                            const auto& Array = static_cast<const ArrayType&>(ConstantValue.getType());
                            const auto& String = static_cast<const StringConstant&>(ConstantValue);
                            if (!IntegerType::classof(&Array.elementType()))
                            {
                                return fail(Context, "String constant requires unsigned 8-bit array elements");
                            }
                            const auto& Element = static_cast<const IntegerType&>(Array.elementType());
                            if (Element.bitWidth() != 8 || Element.isSigned())
                            {
                                return fail(Context, "String constant requires unsigned 8-bit array elements");
                            }
                            return Array.elementCount() == String.value().size() || fail(Context, "String constant array length must equal its byte count");
                        }
                    default:
                        return fail(Context, "Value does not have a supported constant kind");
                    }
                }

                bool checkInheritance(const RecordType& Record, std::string_view Context, std::size_t& OutHeight)
                {
                    if (InheritanceDepth >= MaximumValidationDepth)
                    {
                        return fail(Context, "inheritance graph exceeds supported validation depth");
                    }
                    const ScopedDepth Depth(InheritanceDepth);
                    if (const auto Found = CompleteInheritance.find(&Record); Found != CompleteInheritance.end())
                    {
                        if (InheritanceDepth - 1 + Found->second > MaximumValidationDepth)
                        {
                            return fail(Context, "inheritance graph exceeds supported validation depth");
                        }
                        OutHeight = Found->second;
                        return true;
                    }
                    if (!ActiveInheritance.insert(&Record).second)
                    {
                        return fail(Context, "Record inheritance contains a cycle");
                    }
                    std::size_t MaxBaseHeight = 0;
                    for (const Value* Base : Record.bases())
                    {
                        if (RecordType::classof(Base))
                        {
                            std::size_t BaseHeight = 0;
                            if (!checkInheritance(static_cast<const RecordType&>(*Base), Context, BaseHeight))
                            {
                                return false;
                            }
                            MaxBaseHeight = std::max(MaxBaseHeight, BaseHeight);
                        }
                    }
                    ActiveInheritance.erase(&Record);
                    OutHeight = 1 + MaxBaseHeight;
                    CompleteInheritance.emplace(&Record, OutHeight);
                    return true;
                }

                bool checkStorageLayout(const Value& TypeValue, std::string_view Context, std::size_t& OutHeight)
                {
                    if (LayoutDepth >= MaximumValidationDepth)
                    {
                        return fail(Context, "storage graph exceeds supported validation depth");
                    }
                    const ScopedDepth Depth(LayoutDepth);
                    OutHeight = 1;
                    if (TypeValue.valueKind() != ValueKind::Type)
                    {
                        return true;
                    }
                    const auto* Concrete = static_cast<const Type*>(&TypeValue);
                    if (Concrete->getKind() == TypeKind::Object)
                    {
                        return true;
                    }
                    if (Concrete->getKind() == TypeKind::Struct)
                    {
                        std::size_t RecordHeight = 0;
                        if (!checkRecordLayout(static_cast<const StructType&>(*Concrete), Context, RecordHeight))
                        {
                            return false;
                        }
                        OutHeight = 1 + RecordHeight;
                        return true;
                    }
                    if (Concrete->getKind() == TypeKind::Array)
                    {
                        if (const auto Found = CompleteLayout.find(Concrete); Found != CompleteLayout.end())
                        {
                            if (LayoutDepth - 1 + Found->second > MaximumValidationDepth)
                            {
                                return fail(Context, "storage graph exceeds supported validation depth");
                            }
                            OutHeight = Found->second;
                            return true;
                        }
                        if (!ActiveLayout.insert(Concrete).second)
                        {
                            return fail(Context, "Type contains a recursive by-value layout");
                        }
                        std::size_t ElementHeight = 0;
                        if (!checkStorageLayout(static_cast<const ArrayType&>(*Concrete).elementType(), Context, ElementHeight))
                        {
                            return false;
                        }
                        ActiveLayout.erase(Concrete);
                        OutHeight = 1 + ElementHeight;
                        CompleteLayout.emplace(Concrete, OutHeight);
                    }
                    return true;
                }

                bool checkRecordLayout(const RecordType& Record, std::string_view Context, std::size_t& OutHeight)
                {
                    if (LayoutDepth >= MaximumValidationDepth)
                    {
                        return fail(Context, "storage graph exceeds supported validation depth");
                    }
                    const ScopedDepth Depth(LayoutDepth);
                    if (const auto Found = CompleteLayout.find(&Record); Found != CompleteLayout.end())
                    {
                        if (LayoutDepth - 1 + Found->second > MaximumValidationDepth)
                        {
                            return fail(Context, "storage graph exceeds supported validation depth");
                        }
                        OutHeight = Found->second;
                        return true;
                    }
                    if (!ActiveLayout.insert(&Record).second)
                    {
                        return fail(Context, "Record contains a recursive by-value layout");
                    }
                    std::size_t MaxChildHeight = 0;
                    for (const Value* Base : Record.bases())
                    {
                        if (RecordType::classof(Base))
                        {
                            std::size_t BaseHeight = 0;
                            if (!checkRecordLayout(static_cast<const RecordType&>(*Base), Context, BaseHeight))
                            {
                                return false;
                            }
                            MaxChildHeight = std::max(MaxChildHeight, BaseHeight);
                        }
                    }
                    for (const Field& FieldValue : Record.fields())
                    {
                        if (!FieldValue.isStatic())
                        {
                            std::size_t FieldHeight = 0;
                            if (!checkStorageLayout(FieldValue.type(), Context, FieldHeight))
                            {
                                return false;
                            }
                            MaxChildHeight = std::max(MaxChildHeight, FieldHeight);
                        }
                    }
                    ActiveLayout.erase(&Record);
                    OutHeight = 1 + MaxChildHeight;
                    CompleteLayout.emplace(&Record, OutHeight);
                    return true;
                }

                bool checkLayouts(std::string_view Context)
                {
                    for (const RecordType* Record : Records)
                    {
                        std::size_t Height = 0;
                        if (!checkInheritance(*Record, Context, Height) || !checkRecordLayout(*Record, Context, Height))
                        {
                            return false;
                        }
                    }
                    for (const Type* KnownType : OrderedBodyTypes)
                    {
                        std::size_t Height = 0;
                        if (ArrayType::classof(KnownType) && !checkStorageLayout(*KnownType, Context, Height))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                struct ImportedSymbolKey
                {
                        const ModuleImport* Source = nullptr;
                        core::Name Name = core::NAME_None;

                        bool operator==(const ImportedSymbolKey&) const noexcept = default;
                };

                struct ImportedSymbolKeyHash
                {
                        std::size_t operator()(const ImportedSymbolKey& Value) const noexcept
                        {
                            const std::size_t First = std::hash<const ModuleImport*>{}(Value.Source);
                            const std::size_t Second = std::hash<core::Name>{}(Value.Name);
                            return First ^ (Second + std::size_t{0x9e3779b9} + (First << 6) + (First >> 2));
                        }
                };

                bool checkImportedDeclaration(const ModuleImport& Source, core::Name SymbolName, std::string_view Context)
                {
                    if (Source.parent() != CurrentModule || !KnownImports.contains(&Source))
                    {
                        return fail(Context, "Imported declaration references a foreign or unregistered module import");
                    }
                    if (!Source.path().valid())
                    {
                        return fail(Context, "Imported declaration references an invalid module import path");
                    }
                    if (!core::NamePool::instance().contains(SymbolName))
                    {
                        return fail(Context, "Imported declaration requires a valid symbol name");
                    }
                    if (!ImportedSymbols.insert(ImportedSymbolKey{&Source, SymbolName}).second)
                    {
                        return fail(Context, "Imported symbol is declared more than once from the same module");
                    }
                    return true;
                }

                const Module* CurrentModule = nullptr;
                std::optional<VerificationError> Failure = std::nullopt;
                std::size_t TypeDepth = 0;
                std::size_t InheritanceDepth = 0;
                std::size_t LayoutDepth = 0;
                bool CurrentAllowInstructions = false;
                std::string_view CurrentContext = {};
                std::unordered_set<const ModuleImport*> KnownImports = {};
                std::unordered_set<ImportedSymbolKey, ImportedSymbolKeyHash> ImportedSymbols = {};
                std::unordered_set<const Type*> DeclarationTypes = {};
                std::unordered_set<const Type*> BodyTypes = {};
                std::vector<const Type*> OrderedBodyTypes = {};
                std::unordered_set<const RecordType*> KnownRecords = {};
                std::vector<const RecordType*> Records = {};
                std::unordered_set<const RecordType*> ActiveInheritance = {};
                std::unordered_map<const RecordType*, std::size_t> CompleteInheritance = {};
                std::unordered_set<const Type*> ActiveLayout = {};
                std::unordered_map<const Type*, std::size_t> CompleteLayout = {};
        };
    } // namespace

    std::optional<VerificationError> checkModule(const Module& ModuleValue)
    {
        return Verifier{}.verifyModuleGraph(ModuleValue);
    }

    std::optional<VerificationError> checkFunction(const Function& FunctionValue)
    {
        return Verifier{}.verifyFunctionGraph(FunctionValue);
    }

    bool verifyModule(const Module& ModuleValue, std::string* Error)
    {
        if (const auto Failure = checkModule(ModuleValue))
        {
            if (Error)
            {
                *Error = Failure->message();
            }
            return false;
        }
        if (Error)
        {
            Error->clear();
        }
        return true;
    }

    bool verifyFunction(const Function& FunctionValue, std::string* Error)
    {
        if (const auto Failure = checkFunction(FunctionValue))
        {
            if (Error)
            {
                *Error = Failure->message();
            }
            return false;
        }
        if (Error)
        {
            Error->clear();
        }
        return true;
    }

    void verifyModule(const Module& ModuleValue, const core::DiagnosticEngine& Diagnostics)
    {
        if (const auto Failure = checkModule(ModuleValue))
        {
            Diagnostics.reportICE<core::DiagnosticKind::IRVerificationFailed>({}, Failure->Context, Failure->Description);
        }
    }

    void verifyFunction(const Function& FunctionValue, const core::DiagnosticEngine& Diagnostics)
    {
        if (const auto Failure = checkFunction(FunctionValue))
        {
            Diagnostics.reportICE<core::DiagnosticKind::IRVerificationFailed>({}, Failure->Context, Failure->Description);
        }
    }
} // namespace ink::ir
