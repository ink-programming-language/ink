#include "ink/ir/verification/verifier.h"

#include "verifier_internal.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_visitor.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/type_pool.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::ir
{
  namespace
  {
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

    class FunctionVerifier final : public InstructionVisitor<FunctionVerifier>
    {
      public:
        FunctionVerifier(const Function &FunctionValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context)
            : FunctionValue(FunctionValue),
              Diagnostics(Diagnostics),
              Context(Context)
        {
        }

        void verify()
        {
          verifySignature();
          collectDefinitions();
          if (Blocks.empty())
          {
            return;
          }
          buildControlFlow();
          computeDominators();
          for (const BlockInfo &Info : Blocks)
          {
            for (const Instruction &Node : Info.Block->instructions())
            {
              detail::verifyTypeValue(Node.getType(), Diagnostics, instructionContext(Node));
              verifyTypeDependencies(Node.getType(), &Node);
              visit(Node);
            }
          }
        }

        void visitAddInstruction(const AddInstruction &Node)
        {
          const Value &TypeValue = Node.getLHS().getType();
          require(&TypeValue == &Node.getRHS().getType() && &Node.getType() == &TypeValue, Node, "add operand and result types must match");
          require(IntegerType::classof(&TypeValue) || FloatType::classof(&TypeValue), Node, "add requires integer or floating-point operands");
          verifyOperand(Node, Node.getLHS(), 0);
          verifyOperand(Node, Node.getRHS(), 1);
        }

        void visitAllocaInstruction(const AllocaInstruction &Node)
        {
          require(PointerType::classof(&Node.getType()), Node, "alloca result must have pointer type");
          const Value &AllocatedType = static_cast<const PointerType &>(Node.getType()).pointeeType();
          require(!VoidType::classof(&AllocatedType), Node, "alloca cannot allocate void");
          detail::verifyTypeValue(AllocatedType, Diagnostics, instructionContext(Node));
        }

        void visitLoadInstruction(const LoadInstruction &Node)
        {
          const Value &SourceType = Node.getSource().getType();
          require(PointerType::classof(&SourceType), Node, "load source must have pointer type");
          require(&static_cast<const PointerType &>(SourceType).pointeeType() == &Node.getType(), Node, "load result type must match its source pointee type");
          require(!VoidType::classof(&Node.getType()), Node, "load cannot produce void");
          verifyOperand(Node, Node.getSource(), 0);
        }

        void visitStoreInstruction(const StoreInstruction &Node)
        {
          require(VoidType::classof(&Node.getType()), Node, "store result must have void type");
          const Value &DestinationType = Node.getDestination().getType();
          require(PointerType::classof(&DestinationType), Node, "store destination must have pointer type");
          require(&static_cast<const PointerType &>(DestinationType).pointeeType() == &Node.getSource().getType(), Node, "store source type must match its destination pointee type");
          verifyOperand(Node, Node.getDestination(), 0);
          verifyOperand(Node, Node.getSource(), 1);
        }

        void visitReturnInstruction(const ReturnInstruction &Node)
        {
          require(VoidType::classof(&Node.getType()), Node, "return instruction result must have void type");
          const Value &ReturnType = FunctionValue.signature().returnType();
          const Value *Returned = Node.getReturnValue();
          if (VoidType::classof(&ReturnType))
          {
            require(Returned == nullptr, Node, "void function cannot return a value");
            return;
          }
          require(Returned != nullptr, Node, "non-void function must return a value");
          require(&Returned->getType() == &ReturnType, Node, "return value type does not match the function signature");
          verifyOperand(Node, *Returned, 0);
        }

        void visitBranchInstruction(const BranchInstruction &Node)
        {
          require(VoidType::classof(&Node.getType()), Node, "branch result must have void type");
        }

        void visitCondBranchInstruction(const CondBranchInstruction &Node)
        {
          require(VoidType::classof(&Node.getType()), Node, "conditional branch result must have void type");
          require(BoolType::classof(&Node.getCondition().getType()), Node, "conditional branch requires a boolean condition");
          verifyOperand(Node, Node.getCondition(), 0);
        }

        void visitPhiInstruction(const PhiInstruction &Node)
        {
          require(!VoidType::classof(&Node.getType()), Node, "phi cannot produce void");
          const Definition &Location = Definitions.at(&Node);
          const auto &Predecessors = Blocks[Location.Block].Predecessors;
          require(!Predecessors.empty(), Node, "phi requires at least one predecessor");
          require(Node.incomingCount() == Predecessors.size(), Node, "phi incoming edges do not match CFG predecessors");
          std::vector<std::size_t> IncomingBlocks;
          std::unordered_map<const BasicBlock *, const Value *> IncomingValues;
          IncomingBlocks.reserve(Node.incomingCount());
          for (std::size_t Index = 0; Index < Node.incomingCount(); ++Index)
          {
            const Value &Incoming = Node.incomingValue(Index);
            require(&Incoming.getType() == &Node.getType(), Node, "phi incoming type does not match its result type");
            const BasicBlock &Predecessor = Node.incomingBlock(Index);
            const auto Found = BlockIndices.find(&Predecessor);
            require(Found != BlockIndices.end(), Node, "phi incoming edges do not match CFG predecessors");
            IncomingBlocks.push_back(Found->second);
            const auto [Previous, Inserted] = IncomingValues.emplace(&Predecessor, &Incoming);
            require(Inserted || Previous->second == &Incoming, Node, "phi has different values for repeated predecessor edges");
          }
          std::sort(IncomingBlocks.begin(), IncomingBlocks.end());
          std::vector<std::size_t> Expected = Predecessors;
          std::sort(Expected.begin(), Expected.end());
          require(IncomingBlocks == Expected, Node, "phi incoming edges do not match CFG predecessors");
          for (std::size_t Index = 0; Index < Node.incomingCount(); ++Index)
          {
            verifyOperand(Node, Node.incomingValue(Index), Index, &Node.incomingBlock(Index));
          }
        }

      private:
        [[noreturn]] void fail(std::string_view Description) const
        {
          detail::failVerification(Diagnostics, Context, Description);
        }

        std::string instructionContext(const Instruction &Node) const
        {
          const Definition &Location = Definitions.at(&Node);
          return fmt::format("{} / block #{} / instruction #{} ({})", Context, Location.Block, Location.Position, Node.getName());
        }

        void require(bool Condition, const Instruction &Node, std::string_view Description) const
        {
          if (!Condition)
          {
            detail::failVerification(Diagnostics, instructionContext(Node), Description);
          }
        }

        void verifySignature()
        {
          if (!FunctionType::classof(&FunctionValue.getType()))
          {
            fail("function signature must have function type");
          }
          detail::verifyTypeValue(FunctionValue.getType(), Diagnostics, Context);
          verifyTypeDependencies(FunctionValue.getType());
          if (FunctionValue.visibility() != SymbolVisibility::Private && FunctionValue.visibility() != SymbolVisibility::Public)
          {
            fail("function has invalid visibility");
          }
          if (FunctionValue.nativeLinkage() != NativeLinkage::None && FunctionValue.nativeLinkage() != NativeLinkage::Import && FunctionValue.nativeLinkage() != NativeLinkage::Export)
          {
            fail("function has invalid native linkage");
          }
          const auto Parameters = FunctionValue.signature().parameters();
          if (FunctionValue.argumentCount() != Parameters.size())
          {
            fail("function argument count does not match its signature");
          }
          std::size_t Index = 0;
          for (const Argument &ArgumentValue : FunctionValue.arguments())
          {
            if (ArgumentValue.parent() != &FunctionValue || ArgumentValue.index() != Index)
            {
              fail("function argument has invalid ownership or index");
            }
            if (&ArgumentValue.getType() != &Parameters[Index].type())
            {
              fail("function argument type does not match its signature");
            }
            Arguments.insert(&ArgumentValue);
            ++Index;
          }
        }

        void collectDefinitions()
        {
          for (const BasicBlock &Block : FunctionValue.basicBlocks())
          {
            if (Block.parent() != &FunctionValue)
            {
              fail("basic block belongs to another function");
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
#define INSTRUCTION(Kind, Number, Class, Spelling) case InstructionKind::Kind: break;
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
                default:
                  detail::failVerification(Diagnostics, fmt::format("{} / block #{} / instruction #{}", Context, BlockIndex, Position), "instruction has an invalid kind");
              }
              require(Node.parent() == &Block, Node, "instruction belongs to another basic block");
              if (Node.getKind() == InstructionKind::Phi)
              {
                require(!SawNonPhi, Node, "phi instructions must form a prefix at the start of a block");
              }
              else
              {
                SawNonPhi = true;
              }
              require(!Node.isTerminator() || Position + 1 == Block.instructionCount(), Node, "terminator must be the last instruction in its basic block");
              Last = &Node;
              ++Position;
            }
            if (!Last || !Last->isTerminator())
            {
              detail::failVerification(Diagnostics, fmt::format("{} / block #{}", Context, BlockIndex), "basic block is missing a terminator");
            }
          }
        }

        void addEdge(std::size_t Source, const BasicBlock &Target)
        {
          const auto Found = BlockIndices.find(&Target);
          if (Found == BlockIndices.end())
          {
            detail::failVerification(Diagnostics, fmt::format("{} / block #{}", Context, Source), "branch target does not belong to this function");
          }
          Blocks[Source].Successors.push_back(Found->second);
          Blocks[Found->second].Predecessors.push_back(Source);
        }

        void buildControlFlow()
        {
          for (std::size_t Index = 0; Index < Blocks.size(); ++Index)
          {
            const Instruction &Last = *Blocks[Index].Block->terminator();
            if (BranchInstruction::classof(&Last))
            {
              addEdge(Index, static_cast<const BranchInstruction &>(Last).getTarget());
            }
            else if (CondBranchInstruction::classof(&Last))
            {
              const auto &Branch = static_cast<const CondBranchInstruction &>(Last);
              addEdge(Index, Branch.getTrueTarget());
              addEdge(Index, Branch.getFalseTarget());
            }
          }
          if (!Blocks.front().Predecessors.empty())
          {
            fail("entry block must not have predecessors");
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

        void verifyTypeDependencies(const Value &TypeValue, const Instruction *User = nullptr, const BasicBlock *IncomingBlock = nullptr) const
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
                fail("function signature cannot depend on an instruction");
              }
              verifyOperand(*User, Current, 0, IncomingBlock, true);
              continue;
            }
            if (Current.valueKind() != ValueKind::Type)
            {
              continue;
            }
            const auto *Concrete = static_cast<const Type *>(&Current);
            switch (Concrete->getKind())
            {
              case TypeKind::Pointer:
                Worklist.push_back(&static_cast<const PointerType &>(*Concrete).pointeeType());
                break;
              case TypeKind::Array:
                Worklist.push_back(&static_cast<const ArrayType &>(*Concrete).elementType());
                break;
              case TypeKind::Function:
              {
                const auto &Signature = static_cast<const FunctionType &>(*Concrete);
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
                const auto &Record = static_cast<const RecordType &>(*Concrete);
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
                  Worklist.push_back(&MethodValue.function().getType());
                }
                break;
              }
              default:
                break;
            }
          }
        }

        void verifyOperand(const Instruction &User, const Value &Operand, std::size_t Index, const BasicBlock *IncomingBlock = nullptr, bool TypeDependency = false) const
        {
          const std::string OperandContext = fmt::format("{} / {}", instructionContext(User), TypeDependency ? std::string("type dependency") : fmt::format("operand #{}", Index));
          const auto Reject = [&](std::string_view Description)
          {
            detail::failVerification(Diagnostics, OperandContext, Description);
          };
          detail::verifyTypeValue(Operand.getType(), Diagnostics, OperandContext);
          verifyTypeDependencies(Operand.getType(), &User, IncomingBlock);
          if (VoidType::classof(&Operand.getType()))
          {
            Reject("void value cannot be used as an operand");
          }
          switch (Operand.valueKind())
          {
            case ValueKind::Type:
              detail::verifyTypeValue(Operand, Diagnostics, OperandContext);
              verifyTypeDependencies(Operand, &User, IncomingBlock);
              return;
            case ValueKind::IntegerConstant:
            case ValueKind::FloatConstant:
            case ValueKind::StringConstant:
              detail::verifyConstant(Operand, Diagnostics, OperandContext);
              return;
            case ValueKind::Function:
              if (!FunctionType::classof(&Operand.getType()))
              {
                Reject("function operand has an invalid signature");
              }
              return;
            case ValueKind::GlobalVariable:
              if (!PointerType::classof(&Operand.getType()))
              {
                Reject("global variable operand must have pointer type");
              }
              if (VoidType::classof(&static_cast<const PointerType &>(Operand.getType()).pointeeType()))
              {
                Reject("global variable cannot store void");
              }
              return;
            case ValueKind::Argument:
              if (!Arguments.contains(static_cast<const Argument *>(&Operand)))
              {
                Reject("operand belongs to another function");
              }
              return;
            case ValueKind::Instruction:
              break;
            default:
              Reject("operand has an unsupported value kind");
              return;
          }
          const auto &InstructionValue = static_cast<const Instruction &>(Operand);
          if (!InstructionValue.parent())
          {
            Reject("operand instruction is detached");
          }
          if (InstructionValue.parent()->parent() != &FunctionValue)
          {
            Reject("operand belongs to another function");
          }
          const auto Found = Definitions.find(&InstructionValue);
          if (Found == Definitions.end())
          {
            Reject("operand instruction is not owned by this function");
          }
          const Definition &Defined = Found->second;
          const Definition &Used = Definitions.at(&User);
          const std::size_t UseBlock = IncomingBlock ? BlockIndices.at(IncomingBlock) : Used.Block;
          const std::size_t UsePosition = IncomingBlock ? Blocks[UseBlock].Block->instructionCount() - 1 : Used.Position;
          if (Defined.Block == UseBlock)
          {
            if (Defined.Position >= UsePosition)
            {
              Reject("definition does not precede its use");
            }
          }
          else if (Blocks[UseBlock].Reachable && !Dominators[UseBlock][Defined.Block])
          {
            Reject("definition does not dominate its use");
          }
        }

        const Function &FunctionValue;
        const core::DiagnosticEngine &Diagnostics;
        std::string Context;
        std::unordered_set<const Argument *> Arguments;
        std::vector<BlockInfo> Blocks;
        std::unordered_map<const BasicBlock *, std::size_t> BlockIndices;
        std::unordered_map<const Instruction *, Definition> Definitions;
        std::vector<std::vector<bool>> Dominators;
    };
  } // namespace

  namespace detail
  {
    void verifyFunctionBody(const Function &FunctionValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context)
    {
      FunctionVerifier(FunctionValue, Diagnostics, Context).verify();
    }
  } // namespace detail

  void verifyFunction(const Function &FunctionValue, const core::DiagnosticEngine &Diagnostics)
  {
    const std::string_view Name = core::NamePool::instance().text(FunctionValue.name());
    detail::verifyFunctionBody(FunctionValue, Diagnostics, Name.empty() ? std::string("unnamed function") : fmt::format("function '{}'", Name));
  }
} // namespace ink::ir
