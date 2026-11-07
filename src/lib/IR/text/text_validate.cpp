#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/phi_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir::text
{
  namespace
  {
    constexpr std::size_t MaximumValidationDepth = 128;
    // Graph traversal also enters enclosing records/signatures and follows nominal references across expressions.
    constexpr std::size_t MaximumTypeGraphDepth = 256;

    class ValidationDepth final
    {
      public:
        explicit ValidationDepth(std::size_t &Depth) noexcept
            : Depth(Depth)
        {
          ++Depth;
        }

        ~ValidationDepth()
        {
          --Depth;
        }

        ValidationDepth(const ValidationDepth &) = delete;
        ValidationDepth &operator=(const ValidationDepth &) = delete;

      private:
        std::size_t &Depth;
    };

    bool validVisibility(SymbolVisibility Visibility) noexcept
    {
      return Visibility == SymbolVisibility::Private || Visibility == SymbolVisibility::Public;
    }

    bool validLinkage(NativeLinkage Linkage) noexcept
    {
      return Linkage == NativeLinkage::None || Linkage == NativeLinkage::Import || Linkage == NativeLinkage::Export;
    }

    bool validMemberKind(MemberKind Kind) noexcept
    {
      return Kind == MemberKind::Instance || Kind == MemberKind::Static;
    }

    bool validParameterKind(Parameter::Kind Kind) noexcept
    {
      return Kind == Parameter::Kind::Positional || Kind == Parameter::Kind::Named;
    }

    bool validABI(CallABI ABI) noexcept
    {
      const bool KnownABI = ABI.ABI == ABIKind::Ink || ABI.ABI == ABIKind::C;
      const bool KnownConvention = ABI.Convention == CallingConvention::Default || ABI.Convention == CallingConvention::CDecl || ABI.Convention == CallingConvention::StdCall || ABI.Convention == CallingConvention::FastCall || ABI.Convention == CallingConvention::VectorCall;
      return KnownABI && KnownConvention;
    }

    struct Definition
    {
        std::size_t Block;
        std::size_t Position;
    };

    struct BlockInfo
    {
        const BasicBlock *Block;
        std::vector<std::size_t> Predecessors = {};
        std::vector<std::size_t> Successors = {};
        bool Reachable = false;
    };

    // The text reader handles user input, so validation returns the first error instead of reporting an ICE.
    class TextGraphValidator final
    {
      public:
        explicit TextGraphValidator(std::string &Error)
            : Error(Error)
        {
        }

        bool validate(const Module &ModuleValue)
        {
          std::unordered_set<const RecordType *> Registered;
          for (const StructType &Record : ModuleValue.structTypes())
          {
            if (!Registered.insert(&Record).second)
            {
              return fail("module", "record type is registered more than once");
            }
            if (!checkType(Record, false, "struct"))
            {
              return false;
            }
          }
          for (const ObjectType &Record : ModuleValue.objectTypes())
          {
            if (!Registered.insert(&Record).second)
            {
              return fail("module", "record type is registered more than once");
            }
            if (!checkType(Record, false, "class"))
            {
              return false;
            }
          }
          for (const GlobalVariable &Global : ModuleValue.globalVariables())
          {
            if (Global.parent() != &ModuleValue || !validVisibility(Global.visibility()) || !validLinkage(Global.nativeLinkage()))
            {
              return fail("global", "invalid parent, visibility, or native linkage");
            }
            if (!checkType(Global.getType(), false, "global"))
            {
              return false;
            }
            if (!PointerType::classof(&Global.getType()) || VoidType::classof(&Global.valueType()))
            {
              return fail("global", "stored type must be a non-void type");
            }
          }
          std::size_t FunctionIndex = 0;
          for (const Function &FunctionValue : ModuleValue.functions())
          {
            if (FunctionValue.parent() != &ModuleValue)
            {
              return fail("function", "module-owned function has an incorrect parent");
            }
            if (!FunctionValidation(*this, FunctionValue, "function #" + std::to_string(FunctionIndex++)).validate())
            {
              return false;
            }
          }
          // Body types can discover records that are reachable but not explicitly registered with this module.
          for (std::size_t Index = 0; Index < Records.size(); ++Index)
          {
            const RecordType &Record = *Records[Index];
            if (!checkType(Record, false, "record #" + std::to_string(Index)))
            {
              return false;
            }
            std::size_t MethodIndex = 0;
            for (const Method &MethodValue : Record.methods())
            {
              if (!MethodValue.hasFunction())
              {
                return fail("method", "method has no owned function");
              }
              if (MethodValue.function().parent() != nullptr)
              {
                return fail("method", "record-owned function must not have a module parent");
              }
              if (!FunctionValidation(*this, MethodValue.function(), "record #" + std::to_string(Index) + " / method #" + std::to_string(MethodIndex++)).validate())
              {
                return false;
              }
            }
          }
          for (const RecordType *Record : Records)
          {
            if (!checkInheritance(*Record) || !checkRecordLayout(*Record))
            {
              return false;
            }
          }
          for (const Type *TypeValue : BodyTypes)
          {
            if (!checkStorageLayout(*TypeValue))
            {
              return false;
            }
          }
          return true;
        }

      private:
        class FunctionValidation final
        {
          public:
            FunctionValidation(TextGraphValidator &Owner, const Function &FunctionValue, std::string Context)
                : Owner(Owner),
                  FunctionValue(FunctionValue),
                  Context(std::move(Context))
            {
            }

            bool validate()
            {
              if (!checkSignature() || !collectDefinitions())
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
              for (const BlockInfo &Info : Blocks)
              {
                for (const Instruction &Node : Info.Block->instructions())
                {
                  if (!Owner.checkType(Node.getType(), true, instructionContext(Node)) || !checkTypeDependencies(Node.getType(), &Node) || !checkInstruction(Node))
                  {
                    return false;
                  }
                }
              }
              return true;
            }

          private:
            bool fail(std::string_view Description) const
            {
              return Owner.fail(Context, Description);
            }

            bool fail(const Instruction &Node, std::string_view Description) const
            {
              return Owner.fail(instructionContext(Node), Description);
            }

            std::string instructionContext(const Instruction &Node) const
            {
              const auto Found = Definitions.find(&Node);
              if (Found == Definitions.end())
              {
                return Context + " / unowned instruction";
              }
              return Context + " / block #" + std::to_string(Found->second.Block) + " / instruction #" + std::to_string(Found->second.Position);
            }

            bool checkSignature()
            {
              if (!FunctionType::classof(&FunctionValue.getType()))
              {
                return fail("signature must have function type");
              }
              if (!Owner.checkType(FunctionValue.getType(), false, Context) || !checkTypeDependencies(FunctionValue.getType()))
              {
                return false;
              }
              if (!validVisibility(FunctionValue.visibility()) || !validLinkage(FunctionValue.nativeLinkage()))
              {
                return fail("invalid visibility or native linkage");
              }
              const auto Parameters = FunctionValue.signature().parameters();
              if (FunctionValue.argumentCount() != Parameters.size())
              {
                return fail("argument count does not match signature");
              }
              std::size_t Index = 0;
              for (const Argument &ArgumentValue : FunctionValue.arguments())
              {
                if (ArgumentValue.parent() != &FunctionValue || ArgumentValue.index() != Index || &ArgumentValue.getType() != &Parameters[Index].type())
                {
                  return fail("argument ownership, index, or type does not match signature");
                }
                Arguments.insert(&ArgumentValue);
                ++Index;
              }
              return true;
            }

            bool collectDefinitions()
            {
              for (const BasicBlock &Block : FunctionValue.basicBlocks())
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
                const Instruction *Last = nullptr;
                for (const Instruction &Node : Block.instructions())
                {
                  Definitions.emplace(&Node, Definition{BlockIndex, Position});
                  switch (Node.getKind())
                  {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
  case InstructionKind::Kind:                      \
    break;
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
                  default:
                    return fail(Node, "invalid instruction kind");
                  }
                  if (Node.parent() != &Block)
                  {
                    return fail(Node, "instruction belongs to another basic block");
                  }
                  if (Node.getKind() == InstructionKind::Phi)
                  {
                    if (SawNonPhi)
                    {
                      return fail(Node, "phi instructions must form a prefix at the start of a block");
                    }
                  }
                  else
                  {
                    SawNonPhi = true;
                  }
                  if (Node.isTerminator() && Position + 1 != Block.instructionCount())
                  {
                    return fail(Node, "terminator must be the last instruction in its basic block");
                  }
                  Last = &Node;
                  ++Position;
                }
                if (!Last || !Last->isTerminator())
                {
                  return fail("basic block is missing a terminator");
                }
              }
              return true;
            }

            bool addEdge(std::size_t Source, const BasicBlock &Target)
            {
              const auto Found = BlockIndices.find(&Target);
              if (Found == BlockIndices.end())
              {
                return fail("branch target does not belong to this function");
              }
              Blocks[Source].Successors.push_back(Found->second);
              Blocks[Found->second].Predecessors.push_back(Source);
              return true;
            }

            bool buildControlFlow()
            {
              for (std::size_t Index = 0; Index < Blocks.size(); ++Index)
              {
                const Instruction &Last = *Blocks[Index].Block->terminator();
                if (BranchInstruction::classof(&Last))
                {
                  if (!addEdge(Index, static_cast<const BranchInstruction &>(Last).getTarget()))
                  {
                    return false;
                  }
                }
                else if (CondBranchInstruction::classof(&Last))
                {
                  const auto &Branch = static_cast<const CondBranchInstruction &>(Last);
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
              for (const BlockInfo &Info : Blocks)
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

            bool checkTypeDependencies(const Value &TypeValue, const Instruction *User = nullptr, const BasicBlock *IncomingBlock = nullptr) const
            {
              std::vector<const Value *> Worklist = {&TypeValue};
              std::unordered_set<const Value *> Visited;
              while (!Worklist.empty())
              {
                const Value &Current = *Worklist.back();
                Worklist.pop_back();
                if (!Visited.insert(&Current).second)
                {
                  continue;
                }
                if (Current.valueKind() == ValueKind::Instruction)
                {
                  if (!User)
                  {
                    return fail("signature cannot depend on an instruction");
                  }
                  if (&Current.getType() != &MetaType::instance())
                  {
                    return fail(*User, "type dependency must have the canonical MetaType");
                  }
                  if (!checkInstructionUse(*User, static_cast<const Instruction &>(Current), IncomingBlock))
                  {
                    return false;
                  }
                  continue;
                }
                if (Current.valueKind() != ValueKind::Type)
                {
                  continue;
                }
                const auto &Concrete = static_cast<const Type &>(Current);
                switch (Concrete.getKind())
                {
                case TypeKind::Pointer:
                  Worklist.push_back(&static_cast<const PointerType &>(Concrete).pointeeType());
                  break;
                case TypeKind::Array:
                  Worklist.push_back(&static_cast<const ArrayType &>(Concrete).elementType());
                  break;
                case TypeKind::Function:
                {
                  const auto &Signature = static_cast<const FunctionType &>(Concrete);
                  Worklist.push_back(&Signature.returnType());
                  for (const Parameter &ParameterValue : Signature.parameters())
                  {
                    Worklist.push_back(&ParameterValue.type());
                  }
                  break;
                }
                case TypeKind::Struct:
                case TypeKind::Object:
                {
                  const auto &Record = static_cast<const RecordType &>(Concrete);
                  for (const Value *Base : Record.bases())
                  {
                    Worklist.push_back(Base);
                  }
                  for (const Field &FieldValue : Record.fields())
                  {
                    Worklist.push_back(&FieldValue.type());
                  }
                  for (const Method &MethodValue : Record.methods())
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

            bool checkOperand(const Instruction &User, const Value &Operand, const BasicBlock *IncomingBlock = nullptr) const
            {
              if (!Owner.checkType(Operand.getType(), true, instructionContext(User)) || !checkTypeDependencies(Operand.getType(), &User, IncomingBlock))
              {
                return false;
              }
              if (VoidType::classof(&Operand.getType()))
              {
                return fail(User, "void value cannot be used as an operand");
              }
              switch (Operand.valueKind())
              {
              case ValueKind::Type:
                return Owner.checkType(Operand, true, instructionContext(User)) && checkTypeDependencies(Operand, &User, IncomingBlock);
              case ValueKind::IntegerConstant:
              case ValueKind::FloatConstant:
              case ValueKind::StringConstant:
                return Owner.checkConstant(Operand, instructionContext(User));
              case ValueKind::Function:
                return FunctionType::classof(&Operand.getType()) || fail(User, "function operand has an invalid signature");
              case ValueKind::GlobalVariable:
              {
                if (!PointerType::classof(&Operand.getType()))
                {
                  return fail(User, "global operand must have pointer type");
                }
                return !VoidType::classof(&static_cast<const PointerType &>(Operand.getType()).pointeeType()) || fail(User, "global cannot store void");
              }
              case ValueKind::Argument:
                return Arguments.contains(static_cast<const Argument *>(&Operand)) || fail(User, "operand belongs to another function");
              case ValueKind::Instruction:
                break;
              default:
                return fail(User, "unsupported operand kind");
              }
              return checkInstructionUse(User, static_cast<const Instruction &>(Operand), IncomingBlock);
            }

            bool checkInstructionUse(const Instruction &User, const Instruction &InstructionValue, const BasicBlock *IncomingBlock) const
            {
              const auto Defined = Definitions.find(&InstructionValue);
              const auto Used = Definitions.find(&User);
              if (!InstructionValue.parent() || InstructionValue.parent()->parent() != &FunctionValue || Defined == Definitions.end() || Used == Definitions.end())
              {
                return fail(User, "operand instruction is not owned by this function");
              }
              std::size_t UseBlock = Used->second.Block;
              std::size_t UsePosition = Used->second.Position;
              if (IncomingBlock)
              {
                const auto Found = BlockIndices.find(IncomingBlock);
                if (Found == BlockIndices.end())
                {
                  return fail(User, "phi predecessor does not belong to this function");
                }
                UseBlock = Found->second;
                UsePosition = Blocks[UseBlock].Block->instructionCount() - 1;
              }
              if (Defined->second.Block == UseBlock)
              {
                return Defined->second.Position < UsePosition || fail(User, "definition does not precede its use");
              }
              return !Blocks[UseBlock].Reachable || Dominators[UseBlock][Defined->second.Block] || fail(User, "definition does not dominate its use");
            }

            bool checkPhi(const PhiInstruction &Node) const
            {
              if (VoidType::classof(&Node.getType()))
              {
                return fail(Node, "phi cannot produce void");
              }
              const auto Location = Definitions.find(&Node);
              if (Location == Definitions.end())
              {
                return fail(Node, "phi is not owned by this function");
              }
              const auto &Predecessors = Blocks[Location->second.Block].Predecessors;
              if (Predecessors.empty() || Node.incomingCount() != Predecessors.size())
              {
                return fail(Node, "phi incoming edges do not match CFG predecessors");
              }
              std::vector<std::size_t> IncomingBlocks;
              std::unordered_map<const BasicBlock *, const Value *> IncomingValues;
              for (std::size_t Index = 0; Index < Node.incomingCount(); ++Index)
              {
                const Value &Incoming = Node.incomingValue(Index);
                const BasicBlock &Predecessor = Node.incomingBlock(Index);
                const auto Found = BlockIndices.find(&Predecessor);
                if (&Incoming.getType() != &Node.getType())
                {
                  return fail(Node, "phi incoming type does not match its result type");
                }
                if (Found == BlockIndices.end())
                {
                  return fail(Node, "phi predecessor does not belong to this function");
                }
                IncomingBlocks.push_back(Found->second);
                const auto [Previous, Inserted] = IncomingValues.emplace(&Predecessor, &Incoming);
                if (!Inserted && Previous->second != &Incoming)
                {
                  return fail(Node, "phi has different values for repeated predecessor edges");
                }
              }
              std::sort(IncomingBlocks.begin(), IncomingBlocks.end());
              std::vector<std::size_t> Expected = Predecessors;
              std::sort(Expected.begin(), Expected.end());
              if (IncomingBlocks != Expected)
              {
                return fail(Node, "phi incoming edges do not match CFG predecessors");
              }
              for (std::size_t Index = 0; Index < Node.incomingCount(); ++Index)
              {
                if (!checkOperand(Node, Node.incomingValue(Index), &Node.incomingBlock(Index)))
                {
                  return false;
                }
              }
              return true;
            }

            bool checkInstruction(const Instruction &Node) const
            {
              switch (Node.getKind())
              {
              case InstructionKind::Add:
              {
                const auto &Add = static_cast<const AddInstruction &>(Node);
                const Value &OperandType = Add.getLHS().getType();
                if (&OperandType != &Add.getRHS().getType() || &Node.getType() != &OperandType || (!IntegerType::classof(&OperandType) && !FloatType::classof(&OperandType)))
                {
                  return fail(Node, "add requires matching integer or floating-point operand and result types");
                }
                return checkOperand(Node, Add.getLHS()) && checkOperand(Node, Add.getRHS());
              }
              case InstructionKind::Alloca:
              {
                if (!PointerType::classof(&Node.getType()))
                {
                  return fail(Node, "alloca result must have pointer type");
                }
                return !VoidType::classof(&static_cast<const PointerType &>(Node.getType()).pointeeType()) || fail(Node, "alloca cannot allocate void");
              }
              case InstructionKind::Load:
              {
                const auto &Load = static_cast<const LoadInstruction &>(Node);
                const Value &SourceType = Load.getSource().getType();
                if (!PointerType::classof(&SourceType) || &static_cast<const PointerType &>(SourceType).pointeeType() != &Node.getType() || VoidType::classof(&Node.getType()))
                {
                  return fail(Node, "load requires a pointer to its non-void result type");
                }
                return checkOperand(Node, Load.getSource());
              }
              case InstructionKind::Store:
              {
                const auto &Store = static_cast<const StoreInstruction &>(Node);
                const Value &DestinationType = Store.getDestination().getType();
                if (!VoidType::classof(&Node.getType()) || !PointerType::classof(&DestinationType) || &static_cast<const PointerType &>(DestinationType).pointeeType() != &Store.getSource().getType())
                {
                  return fail(Node, "store source type must match its destination pointee type");
                }
                return checkOperand(Node, Store.getDestination()) && checkOperand(Node, Store.getSource());
              }
              case InstructionKind::Return:
              {
                const auto &Return = static_cast<const ReturnInstruction &>(Node);
                const Value &ReturnType = FunctionValue.signature().returnType();
                const Value *Returned = Return.getReturnValue();
                if (!VoidType::classof(&Node.getType()))
                {
                  return fail(Node, "return instruction result must have void type");
                }
                if (VoidType::classof(&ReturnType))
                {
                  return Returned == nullptr || fail(Node, "void function cannot return a value");
                }
                if (!Returned || &Returned->getType() != &ReturnType)
                {
                  return fail(Node, "return value type does not match the function signature");
                }
                return checkOperand(Node, *Returned);
              }
              case InstructionKind::Branch:
                return VoidType::classof(&Node.getType()) || fail(Node, "branch result must have void type");
              case InstructionKind::CondBranch:
              {
                const auto &Branch = static_cast<const CondBranchInstruction &>(Node);
                if (!VoidType::classof(&Node.getType()) || !BoolType::classof(&Branch.getCondition().getType()))
                {
                  return fail(Node, "conditional branch requires a boolean condition and void result");
                }
                return checkOperand(Node, Branch.getCondition());
              }
              case InstructionKind::Phi:
                return checkPhi(static_cast<const PhiInstruction &>(Node));
              }
              return fail(Node, "unsupported instruction kind");
            }

            TextGraphValidator &Owner;
            const Function &FunctionValue;
            std::string Context;
            std::unordered_set<const Argument *> Arguments;
            std::vector<BlockInfo> Blocks;
            std::unordered_map<const BasicBlock *, std::size_t> BlockIndices;
            std::unordered_map<const Instruction *, Definition> Definitions;
            std::vector<std::vector<bool>> Dominators;
        };

        bool fail(std::string_view Context, std::string_view Description) const
        {
          if (Error.empty())
          {
            Error = std::string(Context) + ": " + std::string(Description);
          }
          return false;
        }

        bool checkType(const Value &TypeValue, bool AllowInstruction, std::string_view Context)
        {
          if (TypeDepth >= MaximumTypeGraphDepth)
          {
            return fail(Context, "type graph exceeds supported validation depth");
          }
          const ValidationDepth Depth(TypeDepth);
          if (&TypeValue.getType() != &MetaType::instance())
          {
            return fail(Context, "type position requires a value with the canonical MetaType");
          }
          if (TypeValue.valueKind() != ValueKind::Type)
          {
            if (TypeValue.valueKind() != ValueKind::Argument && TypeValue.valueKind() != ValueKind::Instruction)
            {
              return fail(Context, "unsupported type-valued object");
            }
            return AllowInstruction || TypeValue.valueKind() != ValueKind::Instruction || fail(Context, "declaration type metadata cannot depend on a body instruction");
          }
          const auto &Concrete = static_cast<const Type &>(TypeValue);
          auto &Visited = AllowInstruction ? BodyTypes : DeclarationTypes;
          if (!Visited.insert(&Concrete).second)
          {
            return true;
          }
          BodyTypes.insert(&Concrete);
          const TypeSemantics Expected = Concrete.getKind() == TypeKind::Object ? TypeSemantics::Reference : TypeSemantics::Value;
          if (Concrete.getSemantics() != Expected)
          {
            return fail(Context, "type semantics do not match its concrete kind");
          }
          switch (Concrete.getKind())
          {
          case TypeKind::Meta:
            return &Concrete == &MetaType::instance() || fail(Context, "MetaType must be the canonical instance");
          case TypeKind::Integer:
          {
            const auto &Integer = static_cast<const IntegerType &>(Concrete);
            const auto Width = Integer.bitWidth();
            if (Width != 8 && Width != 16 && Width != 32 && Width != 64)
            {
              return fail(Context, "unsupported integer bit width");
            }
            return Integer.signedness() == IntegerType::Signedness::Signed || Integer.signedness() == IntegerType::Signedness::Unsigned || fail(Context, "invalid integer signedness");
          }
          case TypeKind::Float:
          {
            const auto Width = static_cast<const FloatType &>(Concrete).bitWidth();
            return Width == 32 || Width == 64 || fail(Context, "unsupported floating-point bit width");
          }
          case TypeKind::Bool:
          case TypeKind::Void:
            return true;
          case TypeKind::Array:
          {
            const Value &Element = static_cast<const ArrayType &>(Concrete).elementType();
            return (!VoidType::classof(&Element) || fail(Context, "array element type must not be void")) && checkType(Element, AllowInstruction, Context);
          }
          case TypeKind::Pointer:
            return checkType(static_cast<const PointerType &>(Concrete).pointeeType(), AllowInstruction, Context);
          case TypeKind::Function:
          {
            const auto &Signature = static_cast<const FunctionType &>(Concrete);
            if (!validABI(Signature.callABI()) || !checkType(Signature.returnType(), AllowInstruction, Context))
            {
              return Error.empty() ? fail(Context, "invalid function ABI or calling convention") : false;
            }
            for (const Parameter &ParameterValue : Signature.parameters())
            {
              if (!validParameterKind(ParameterValue.kind()) || VoidType::classof(&ParameterValue.type()))
              {
                return fail(Context, "fixed parameters require a valid kind and non-void type");
              }
              if (!checkType(ParameterValue.type(), AllowInstruction, Context))
              {
                return false;
              }
            }
            return true;
          }
          case TypeKind::Struct:
          case TypeKind::Object:
          {
            const auto &Record = static_cast<const RecordType &>(Concrete);
            if (KnownRecords.insert(&Record).second)
            {
              Records.push_back(&Record);
            }
            for (const Value *Base : Record.bases())
            {
              if (!Base)
              {
                return fail(Context, "record has a null base type");
              }
              if (!checkType(*Base, false, Context))
              {
                return false;
              }
              if (Base->valueKind() == ValueKind::Type && !RecordType::classof(Base))
              {
                return fail(Context, "concrete base type must be a record type");
              }
            }
            for (const Field &FieldValue : Record.fields())
            {
              if (!validMemberKind(FieldValue.kind()) || !validVisibility(FieldValue.visibility()) || VoidType::classof(&FieldValue.type()))
              {
                return fail(Context, "field requires a valid kind, visibility, and non-void type");
              }
              if (!checkType(FieldValue.type(), false, Context))
              {
                return false;
              }
            }
            for (const Method &MethodValue : Record.methods())
            {
              if (!validMemberKind(MethodValue.kind()) || !MethodValue.hasFunction())
              {
                return fail(Context, "method requires a valid kind and owned function");
              }
              const Function &FunctionValue = MethodValue.function();
              if (FunctionValue.parent() || !FunctionType::classof(&FunctionValue.getType()) || !validVisibility(FunctionValue.visibility()) || !validLinkage(FunctionValue.nativeLinkage()))
              {
                return fail(Context, "method has invalid ownership, signature, visibility, or native linkage");
              }
              if (!checkType(FunctionValue.getType(), false, Context))
              {
                return false;
              }
            }
            return true;
          }
          }
          return fail(Context, "invalid type kind");
        }

        bool checkConstant(const Value &ConstantValue, std::string_view Context)
        {
          if (!checkType(ConstantValue.getType(), true, Context))
          {
            return false;
          }
          if (ConstantValue.valueKind() == ValueKind::IntegerConstant)
          {
            if (!IntegerType::classof(&ConstantValue.getType()))
            {
              return fail(Context, "integer constant requires an integer type");
            }
            const auto Width = static_cast<const IntegerType &>(ConstantValue.getType()).bitWidth();
            const auto Bits = static_cast<const IntegerConstant &>(ConstantValue).bits();
            return Width == 64 || (Bits >> Width) == 0 || fail(Context, "integer constant has bits outside its width");
          }
          if (ConstantValue.valueKind() == ValueKind::FloatConstant)
          {
            if (!FloatType::classof(&ConstantValue.getType()))
            {
              return fail(Context, "floating-point constant requires a floating-point type");
            }
            const auto Width = static_cast<const FloatType &>(ConstantValue.getType()).bitWidth();
            const auto Bits = static_cast<const FloatConstant &>(ConstantValue).bits();
            return Width == 64 || (Bits >> Width) == 0 || fail(Context, "floating-point constant has bits outside its width");
          }
          if (ConstantValue.valueKind() == ValueKind::StringConstant)
          {
            if (!ArrayType::classof(&ConstantValue.getType()))
            {
              return fail(Context, "string constant requires an array type");
            }
            const auto &Array = static_cast<const ArrayType &>(ConstantValue.getType());
            if (!IntegerType::classof(&Array.elementType()))
            {
              return fail(Context, "string constant requires unsigned 8-bit array elements");
            }
            const auto &Element = static_cast<const IntegerType &>(Array.elementType());
            if (Element.bitWidth() != 8 || Element.isSigned() || Array.elementCount() != static_cast<const StringConstant &>(ConstantValue).value().size())
            {
              return fail(Context, "string constant type must match its unsigned byte payload");
            }
            return true;
          }
          return fail(Context, "unsupported constant kind");
        }

        bool checkInheritance(const RecordType &Record)
        {
          if (InheritanceDepth >= MaximumValidationDepth)
          {
            return fail("record", "inheritance graph exceeds supported validation depth");
          }
          const ValidationDepth Depth(InheritanceDepth);
          if (CompleteInheritance.contains(&Record))
          {
            return true;
          }
          if (!ActiveInheritance.insert(&Record).second)
          {
            return fail("record", "inheritance contains a cycle");
          }
          for (const Value *Base : Record.bases())
          {
            if (RecordType::classof(Base) && !checkInheritance(static_cast<const RecordType &>(*Base)))
            {
              return false;
            }
          }
          ActiveInheritance.erase(&Record);
          CompleteInheritance.insert(&Record);
          return true;
        }

        bool checkStorageLayout(const Value &TypeValue)
        {
          if (LayoutDepth >= MaximumValidationDepth)
          {
            return fail("type", "storage graph exceeds supported validation depth");
          }
          const ValidationDepth Depth(LayoutDepth);
          if (!Type::classof(&TypeValue))
          {
            return true;
          }
          const auto &Concrete = static_cast<const Type &>(TypeValue);
          if (Concrete.getKind() == TypeKind::Struct)
          {
            return checkRecordLayout(static_cast<const StructType &>(Concrete));
          }
          if (Concrete.getKind() != TypeKind::Array || CompleteLayout.contains(&Concrete))
          {
            return true;
          }
          if (!ActiveLayout.insert(&Concrete).second)
          {
            return fail("type", "recursive by-value layout");
          }
          if (!checkStorageLayout(static_cast<const ArrayType &>(Concrete).elementType()))
          {
            return false;
          }
          ActiveLayout.erase(&Concrete);
          CompleteLayout.insert(&Concrete);
          return true;
        }

        bool checkRecordLayout(const RecordType &Record)
        {
          if (LayoutDepth >= MaximumValidationDepth)
          {
            return fail("record", "storage graph exceeds supported validation depth");
          }
          const ValidationDepth Depth(LayoutDepth);
          if (CompleteLayout.contains(&Record))
          {
            return true;
          }
          if (!ActiveLayout.insert(&Record).second)
          {
            return fail("record", "recursive by-value layout");
          }
          for (const Value *Base : Record.bases())
          {
            if (RecordType::classof(Base) && !checkRecordLayout(static_cast<const RecordType &>(*Base)))
            {
              return false;
            }
          }
          for (const Field &FieldValue : Record.fields())
          {
            if (!FieldValue.isStatic() && !checkStorageLayout(FieldValue.type()))
            {
              return false;
            }
          }
          ActiveLayout.erase(&Record);
          CompleteLayout.insert(&Record);
          return true;
        }

        std::string &Error;
        std::size_t TypeDepth = 0;
        std::size_t InheritanceDepth = 0;
        std::size_t LayoutDepth = 0;
        std::unordered_set<const Type *> DeclarationTypes;
        std::unordered_set<const Type *> BodyTypes;
        std::unordered_set<const RecordType *> KnownRecords;
        std::vector<const RecordType *> Records;
        std::unordered_set<const RecordType *> ActiveInheritance;
        std::unordered_set<const RecordType *> CompleteInheritance;
        std::unordered_set<const Type *> ActiveLayout;
        std::unordered_set<const Type *> CompleteLayout;
    };
  } // namespace

  bool validateTextGraph(const Module &ModuleValue, std::string &Error)
  {
    Error.clear();
    return TextGraphValidator(Error).validate(ModuleValue);
  }
} // namespace ink::ir::text
