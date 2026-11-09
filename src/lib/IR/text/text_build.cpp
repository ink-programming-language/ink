#include "text_internal.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/instruction/instruction_builder.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

namespace ink::ir::text
{
  namespace
  {
    enum class DefinitionKind
    {
      Record,
      Function,
      Global,
      Argument,
      Block,
      Instruction,
    };

    struct Definition
    {
        DefinitionKind Kind;
        const InstructionSyntax *Instruction = nullptr;
    };

    struct FunctionState
    {
        const FunctionSyntax *Syntax;
        RecordType *Record = nullptr;
        MemberKind Kind = MemberKind::Instance;
        Function *Value = nullptr;
    };

    struct BlockState
    {
        const BlockSyntax *Syntax;
        Function *Owner;
        BasicBlock *Value;
        std::size_t Next = 0;
    };

    struct InstructionState
    {
        const InstructionSyntax *Syntax;
        Instruction *Value;
    };

    // Unresolved data edges borrow these temporary values until every instruction exists and all edges are rebound.
    // Type metadata never retains a temporary: its dependencies must resolve to their final graph identity first.
    class PendingValue final : public Value
    {
      public:
        explicit PendingValue(ValueRef ValueType) noexcept
            : Value(ValueType)
        {
        }
    };

    class Builder final
    {
      public:
        explicit Builder(const ModuleSyntax &Syntax)
            : Syntax(Syntax),
              Owned(std::make_unique<ParsedModule>(name(Syntax.Name))),
              Types(Owned->types()),
              Constants(Owned->constants()),
              ModuleValue(Owned->module()),
              Instructions(Owned->context().builder())
        {
        }

        std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> run()
        {
          if (!indexDefinitions() || !createRecords() || !createDeclarations() || !fillRecords() || !createInstructions() || !bindOperands())
          {
            return std::unexpected(std::move(*Failure));
          }
          std::string Description;
          if (!validateTextGraph(ModuleValue, Description))
          {
            return std::unexpected(error(0, Description));
          }
          Instructions.clearInsertPoint();
          return std::move(Owned);
        }

      private:
        static core::Name name(std::string_view Text)
        {
          return core::NamePool::instance().intern(Text);
        }

        bool fail(std::size_t Offset, std::string_view Description)
        {
          if (!Failure)
          {
            Failure = error(Offset, Description);
          }
          return false;
        }

        static bool validVisibility(SymbolVisibility Visibility) noexcept
        {
          return Visibility == SymbolVisibility::Private || Visibility == SymbolVisibility::Public;
        }

        static bool validLinkage(NativeLinkage Linkage) noexcept
        {
          return Linkage == NativeLinkage::None || Linkage == NativeLinkage::Import || Linkage == NativeLinkage::Export;
        }

        static bool validMemberKind(MemberKind Kind) noexcept
        {
          return Kind == MemberKind::Instance || Kind == MemberKind::Static;
        }

        bool addDefinition(std::string_view Id, DefinitionKind Kind, std::size_t Offset, const InstructionSyntax *InstructionNode = nullptr)
        {
          const char Prefix = Kind == DefinitionKind::Function || Kind == DefinitionKind::Global ? '@' : '%';
          if (Id.size() < 2 || Id.front() != Prefix)
          {
            return fail(Offset, "definition has an empty ID or an invalid ID prefix");
          }
          if (!Definitions.emplace(Id, Definition{Kind, InstructionNode}).second)
          {
            return fail(Offset, "textual IDs must be unique throughout the module");
          }
          return true;
        }

        bool indexFunction(const FunctionSyntax &Node)
        {
          if (!validVisibility(Node.Visibility) || !validLinkage(Node.Linkage))
          {
            return fail(Node.Offset, "function visibility or native linkage is invalid");
          }
          if (!addDefinition(Node.Id, DefinitionKind::Function, Node.Offset))
          {
            return false;
          }
          for (const ArgumentSyntax &ArgumentNode : Node.Arguments)
          {
            if (!addDefinition(ArgumentNode.Id, DefinitionKind::Argument, ArgumentNode.Offset))
            {
              return false;
            }
          }
          for (const BlockSyntax &BlockNode : Node.Blocks)
          {
            if (!addDefinition(BlockNode.Id, DefinitionKind::Block, BlockNode.Offset))
            {
              return false;
            }
            bool SawNonPhi = false;
            for (std::size_t Index = 0; Index < BlockNode.Instructions.size(); ++Index)
            {
              const InstructionSyntax &InstructionNode = BlockNode.Instructions[Index];
              bool Known = false;
              switch (InstructionNode.Kind)
              {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
  case InstructionKind::Kind:                      \
    Known = true;                                  \
    break;
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
              }
              if (!Known)
              {
                return fail(InstructionNode.Offset, "instruction kind is invalid");
              }
              if (InstructionNode.Kind == InstructionKind::Phi)
              {
                if (SawNonPhi)
                {
                  return fail(InstructionNode.Offset, "phi instructions must form a prefix at the start of a block");
                }
              }
              else
              {
                SawNonPhi = true;
              }
              if (Instruction::isTerminatorKind(InstructionNode.Kind) && Index + 1 != BlockNode.Instructions.size())
              {
                return fail(InstructionNode.Offset, "a terminator must be the last instruction in its block");
              }
              if (!InstructionNode.Id.empty() && !addDefinition(InstructionNode.Id, DefinitionKind::Instruction, InstructionNode.Offset, &InstructionNode))
              {
                return false;
              }
            }
            if (BlockNode.Instructions.empty() || !Instruction::isTerminatorKind(BlockNode.Instructions.back().Kind))
            {
              return fail(BlockNode.Offset, "a basic block must end in a terminator");
            }
          }
          return true;
        }

        bool indexDefinitions()
        {
          for (const RecordSyntax &Node : Syntax.Records)
          {
            if (!addDefinition(Node.Id, DefinitionKind::Record, Node.Offset))
            {
              return false;
            }
            for (const FieldSyntax &FieldNode : Node.Fields)
            {
              if (!validMemberKind(FieldNode.Kind) || !validVisibility(FieldNode.Visibility))
              {
                return fail(FieldNode.Offset, "field kind or visibility is invalid");
              }
            }
            for (const MethodSyntax &MethodNode : Node.Methods)
            {
              if (!validMemberKind(MethodNode.Kind) || !indexFunction(MethodNode.Function))
              {
                return Failure ? false : fail(MethodNode.Offset, "method kind is invalid");
              }
            }
          }
          for (const GlobalSyntax &Node : Syntax.Globals)
          {
            if (!validVisibility(Node.Visibility) || !validLinkage(Node.Linkage))
            {
              return fail(Node.Offset, "global visibility or native linkage is invalid");
            }
            if (!addDefinition(Node.Id, DefinitionKind::Global, Node.Offset))
            {
              return false;
            }
          }
          for (const FunctionSyntax &Node : Syntax.Functions)
          {
            if (!indexFunction(Node))
            {
              return false;
            }
          }
          return true;
        }

        bool createRecords()
        {
          for (const RecordSyntax &Node : Syntax.Records)
          {
            RecordType *Record;
            if (Node.IsObject)
            {
              ObjectType &Object = Types.createObjectType();
              Record = &Object;
              if (Node.Registered)
              {
                ModuleValue.addObjectType(Object);
              }
            }
            else
            {
              StructType &Struct = Types.createStructType();
              Record = &Struct;
              if (Node.Registered)
              {
                ModuleValue.addStructType(Struct);
              }
            }
            Values.emplace(Node.Id, Record);
            Records.emplace(&Node, Record);
            for (const MethodSyntax &MethodNode : Node.Methods)
            {
              Functions.push_back({&MethodNode.Function, Record, MethodNode.Kind});
            }
          }
          for (const FunctionSyntax &Node : Syntax.Functions)
          {
            Functions.push_back({&Node});
          }
          return true;
        }

        const Value *resolve(const Expression &Node, bool TypePosition, bool AllowInstructions, std::size_t Depth = 0)
        {
          if (Depth >= 128)
          {
            fail(Node.Offset, "type or value dependency nesting exceeds the supported depth of 128");
            return nullptr;
          }
          const Value *Result = nullptr;
          if (Node.Kind == ExpressionKind::Reference)
          {
            const auto Definition = Definitions.find(Node.Text);
            if (Definition == Definitions.end())
            {
              fail(Node.Offset, "reference names an undefined textual ID");
              return nullptr;
            }
            if (Definition->second.Kind == DefinitionKind::Block)
            {
              fail(Node.Offset, "a basic block ID cannot be used as a value or type");
              return nullptr;
            }
            if (!AllowInstructions && Definition->second.Kind == DefinitionKind::Instruction)
            {
              fail(Node.Offset, "declaration type metadata cannot depend on a function-body instruction");
              return nullptr;
            }
            const auto Found = Values.find(Node.Text);
            if (Found == Values.end())
            {
              return nullptr;
            }
            Result = Found->second;
          }
          else
          {
            switch (Node.Kind)
            {
            case ExpressionKind::MetaType:
              Result = &Types.getMetaType();
              break;
            case ExpressionKind::BoolType:
              Result = &Types.getBoolType();
              break;
            case ExpressionKind::VoidType:
              Result = &Types.getVoidType();
              break;
            case ExpressionKind::IntegerType:
              if (Node.Width != 8 && Node.Width != 16 && Node.Width != 32 && Node.Width != 64)
              {
                fail(Node.Offset, "integer width must be 8, 16, 32 or 64");
                return nullptr;
              }
              Result = &Types.getIntegerType(static_cast<IntegerType::Width>(Node.Width), Node.Signed ? IntegerType::Signedness::Signed : IntegerType::Signedness::Unsigned);
              break;
            case ExpressionKind::FloatType:
              if (Node.Width != 32 && Node.Width != 64)
              {
                fail(Node.Offset, "floating-point width must be 32 or 64");
                return nullptr;
              }
              Result = &Types.getFloatType(static_cast<FloatType::Width>(Node.Width));
              break;
            case ExpressionKind::PointerType:
            case ExpressionKind::ArrayType:
            {
              if (Node.Children.size() != 1)
              {
                fail(Node.Offset, "pointer and array types require one element type");
                return nullptr;
              }
              const Value *Element = resolve(Node.Children.front(), true, AllowInstructions, Depth + 1);
              if (!Element)
              {
                return nullptr;
              }
              if (Node.Kind == ExpressionKind::PointerType)
              {
                Result = &Types.getPointerType(*Element);
              }
              else
              {
                if (VoidType::classof(Element))
                {
                  fail(Node.Offset, "array element type must not be void");
                  return nullptr;
                }
                Result = &Types.getArrayType(*Element, Node.Count);
              }
              break;
            }
            case ExpressionKind::FunctionType:
            {
              if (Node.Children.empty() || Node.ParameterKinds.size() + 1 != Node.Children.size())
              {
                fail(Node.Offset, "function type has inconsistent fixed-parameter metadata");
                return nullptr;
              }
              if ((Node.ABI.ABI != ABIKind::Ink && Node.ABI.ABI != ABIKind::C) || (Node.ABI.Convention != CallingConvention::Default && Node.ABI.Convention != CallingConvention::CDecl && Node.ABI.Convention != CallingConvention::StdCall && Node.ABI.Convention != CallingConvention::FastCall && Node.ABI.Convention != CallingConvention::VectorCall))
              {
                fail(Node.Offset, "function ABI or calling convention is invalid");
                return nullptr;
              }
              const Value *ReturnType = resolve(Node.Children.front(), true, AllowInstructions, Depth + 1);
              if (!ReturnType)
              {
                return nullptr;
              }
              std::vector<Parameter> Parameters;
              for (std::size_t Index = 0; Index < Node.ParameterKinds.size(); ++Index)
              {
                const auto Kind = Node.ParameterKinds[Index];
                if (Kind != Parameter::Kind::Positional && Kind != Parameter::Kind::Named)
                {
                  fail(Node.Offset, "fixed parameter kind is invalid");
                  return nullptr;
                }
                const Value *ParameterType = resolve(Node.Children[Index + 1], true, AllowInstructions, Depth + 1);
                if (!ParameterType)
                {
                  return nullptr;
                }
                if (VoidType::classof(ParameterType))
                {
                  fail(Node.Children[Index + 1].Offset, "fixed parameter type must not be void");
                  return nullptr;
                }
                Parameters.emplace_back(*ParameterType, Kind);
              }
              Result = &Types.getFunctionType(*ReturnType, Parameters, Node.ABI, Node.Variadic);
              break;
            }
            case ExpressionKind::IntegerConstant:
            case ExpressionKind::FloatConstant:
            {
              if (Node.Children.size() != 1)
              {
                fail(Node.Offset, "a numeric constant requires one type");
                return nullptr;
              }
              const Value *ValueType = resolve(Node.Children.front(), true, AllowInstructions, Depth + 1);
              if (!ValueType)
              {
                return nullptr;
              }
              if (Node.Kind == ExpressionKind::IntegerConstant)
              {
                if (!IntegerType::classof(ValueType))
                {
                  fail(Node.Offset, "integer constant requires an integer type");
                  return nullptr;
                }
                Result = Constants.getInteger(static_cast<const IntegerType &>(*ValueType), Node.Bits);
              }
              else
              {
                if (!FloatType::classof(ValueType))
                {
                  fail(Node.Offset, "floating-point constant requires a floating-point type");
                  return nullptr;
                }
                Result = Constants.getFloatBits(static_cast<const FloatType &>(*ValueType), Node.Bits);
              }
              if (!Result)
              {
                fail(Node.Offset, "numeric constant has bits outside its declared width");
                return nullptr;
              }
              break;
            }
            case ExpressionKind::StringConstant:
              Result = &Constants.getString(Node.Text);
              break;
            case ExpressionKind::Reference:
              break;
            }
          }
          if (TypePosition && Result && (!Result->isTypeValue() || (Result->valueKind() != ValueKind::Type && Result->valueKind() != ValueKind::Argument && Result->valueKind() != ValueKind::Instruction)))
          {
            fail(Node.Offset, "type position requires a concrete type or a type-valued argument/instruction");
            return nullptr;
          }
          return Result;
        }

        bool precedingFunctionReady(std::size_t Index) const
        {
          const FunctionState &Current = Functions[Index];
          for (std::size_t Previous = 0; Previous < Index; ++Previous)
          {
            if (Functions[Previous].Record == Current.Record && !Functions[Previous].Value)
            {
              return false;
            }
          }
          return true;
        }

        bool createFunction(FunctionState &State, const FunctionType &Signature)
        {
          const FunctionSyntax &Node = *State.Syntax;
          if (Signature.parameters().size() != Node.Arguments.size())
          {
            return fail(Node.Offset, "function argument IDs do not match its fixed parameter count");
          }
          if (State.Record)
          {
            State.Value = &State.Record->addMethod(Signature, name(Node.Name), State.Kind, Node.Visibility, Node.Linkage).function();
          }
          else
          {
            State.Value = &ModuleValue.createFunction(Signature, name(Node.Name), Node.Visibility, Node.Linkage);
          }
          Values.emplace(Node.Id, State.Value);
          for (std::size_t Index = 0; Index < Node.Arguments.size(); ++Index)
          {
            Argument &ArgumentValue = State.Value->argument(Index);
            ArgumentValue.setName(name(Node.Arguments[Index].Name));
            Values.emplace(Node.Arguments[Index].Id, &ArgumentValue);
          }
          for (const BlockSyntax &BlockNode : Node.Blocks)
          {
            BasicBlock &Block = State.Value->createBasicBlock(name(BlockNode.Name));
            Blocks.emplace(BlockNode.Id, &Block);
            BlockStates.push_back({&BlockNode, State.Value, &Block});
          }
          return true;
        }

        bool createDeclarations()
        {
          std::size_t Remaining = Functions.size() + Syntax.Globals.size();
          std::size_t NextGlobal = 0;
          while (Remaining != 0)
          {
            const std::size_t Previous = Remaining;
            for (std::size_t Index = 0; Index < Functions.size(); ++Index)
            {
              FunctionState &State = Functions[Index];
              if (State.Value || !precedingFunctionReady(Index))
              {
                continue;
              }
              const Value *Signature = resolve(State.Syntax->Signature, true, false);
              if (!Signature)
              {
                if (Failure)
                {
                  return false;
                }
                continue;
              }
              if (!FunctionType::classof(Signature))
              {
                return fail(State.Syntax->Signature.Offset, "a function declaration requires a concrete function type");
              }
              if (!createFunction(State, static_cast<const FunctionType &>(*Signature)))
              {
                return false;
              }
              --Remaining;
            }
            while (NextGlobal < Syntax.Globals.size())
            {
              const GlobalSyntax &Node = Syntax.Globals[NextGlobal];
              const Value *ValueType = resolve(Node.Type, true, false);
              if (!ValueType)
              {
                if (Failure)
                {
                  return false;
                }
                break;
              }
              if (!PointerType::classof(ValueType))
              {
                return fail(Node.Type.Offset, "a global variable requires a pointer result type");
              }
              const auto &Pointer = static_cast<const PointerType &>(*ValueType);
              if (VoidType::classof(&Pointer.pointeeType()))
              {
                return fail(Node.Type.Offset, "a global variable stored type must not be void");
              }
              GlobalVariable &Global = ModuleValue.createGlobalVariable(Pointer, name(Node.Name), Node.Visibility, Node.Linkage);
              Values.emplace(Node.Id, &Global);
              ++NextGlobal;
              --Remaining;
            }
            if (Remaining == Previous)
            {
              return fail(0, "declaration dependencies cannot be resolved without changing declaration order");
            }
          }
          return true;
        }

        bool fillRecords()
        {
          for (const RecordSyntax &Node : Syntax.Records)
          {
            RecordType &Record = *Records.find(&Node)->second;
            for (const Expression &BaseNode : Node.Bases)
            {
              const Value *Base = resolve(BaseNode, true, false);
              if (!Base)
              {
                return Failure ? false : fail(BaseNode.Offset, "record base dependency is unresolved");
              }
              if (Type::classof(Base) && !RecordType::classof(Base))
              {
                return fail(BaseNode.Offset, "a concrete base type must be a struct or object");
              }
              Record.addBase(*Base);
            }
            for (const FieldSyntax &FieldNode : Node.Fields)
            {
              const Value *FieldType = resolve(FieldNode.Type, true, false);
              if (!FieldType)
              {
                return Failure ? false : fail(FieldNode.Type.Offset, "record field type dependency is unresolved");
              }
              if (VoidType::classof(FieldType))
              {
                return fail(FieldNode.Type.Offset, "field type must not be void");
              }
              Record.addField(name(FieldNode.Name), *FieldType, FieldNode.Kind, FieldNode.Visibility);
            }
          }
          return true;
        }

        const Value *operandType(const Expression &Node, std::size_t Depth = 0)
        {
          if (Depth >= 128)
          {
            fail(Node.Offset, "operand type dependency nesting exceeds the supported depth of 128");
            return nullptr;
          }
          const Value *Resolved = resolve(Node, false, true, Depth);
          if (Resolved)
          {
            return &Resolved->getType();
          }
          if (Failure || Node.Kind != ExpressionKind::Reference)
          {
            return nullptr;
          }
          const auto Found = Definitions.find(Node.Text);
          if (Found == Definitions.end() || Found->second.Kind != DefinitionKind::Instruction || !Found->second.Instruction)
          {
            fail(Node.Offset, "operand does not refer to an available value");
            return nullptr;
          }
          return resolve(Found->second.Instruction->ResultType, true, true, Depth);
        }

        const Value *operand(const Expression &Node)
        {
          const Value *Resolved = resolve(Node, false, true);
          if (Resolved || Failure)
          {
            return Resolved;
          }
          const Value *ValueType = operandType(Node);
          if (!ValueType)
          {
            return nullptr;
          }
          auto OwnedValue = std::make_unique<PendingValue>(*ValueType);
          const Value *Result = OwnedValue.get();
          PendingValues.push_back(std::move(OwnedValue));
          return Result;
        }

        BasicBlock *block(std::string_view Id, const Function &Owner, std::size_t Offset)
        {
          const auto Found = Blocks.find(Id);
          if (Found == Blocks.end())
          {
            fail(Offset, "branch or phi target does not name a basic block");
            return nullptr;
          }
          if (Found->second->parent() != &Owner)
          {
            fail(Offset, "branch or phi target belongs to another function");
            return nullptr;
          }
          return Found->second;
        }

        bool shape(const InstructionSyntax &Node, std::size_t OperandCount, std::size_t TargetCount, bool IsPhi = false)
        {
          if (Node.Operands.size() != OperandCount || Node.Targets.size() != TargetCount || (!IsPhi && !Node.Incoming.empty()))
          {
            return fail(Node.Offset, "instruction has an incorrect operand, target or incoming-list shape");
          }
          return true;
        }

        Instruction *createInstruction(const InstructionSyntax &Node, BlockState &State)
        {
          const Value *ResultType = resolve(Node.ResultType, true, true);
          if (!ResultType)
          {
            return nullptr;
          }
          if (VoidType::classof(ResultType) != Node.Id.empty())
          {
            fail(Node.Offset, "non-void instructions require a result ID and void instructions cannot define one");
            return nullptr;
          }
          Instructions.setInsertPoint(*State.Value);
          switch (Node.Kind)
          {
          case InstructionKind::Add:
          {
            if (!shape(Node, 2, 0))
            {
              return nullptr;
            }
            const Value *LHS = operand(Node.Operands[0]);
            const Value *RHS = operand(Node.Operands[1]);
            if (!LHS || !RHS)
            {
              return nullptr;
            }
            if ((!IntegerType::classof(ResultType) && !FloatType::classof(ResultType)) || &LHS->getType() != ResultType || &RHS->getType() != ResultType)
            {
              fail(Node.Offset, "add requires matching integer or floating-point operand and result types");
              return nullptr;
            }
            return &Instructions.createAdd(*LHS, *RHS);
          }
          case InstructionKind::Alloca:
          {
            if (!shape(Node, 1, 0))
            {
              return nullptr;
            }
            const Value *AllocatedType = resolve(Node.Operands[0], true, true);
            if (!AllocatedType)
            {
              return nullptr;
            }
            if (VoidType::classof(AllocatedType) || !PointerType::classof(ResultType) || &static_cast<const PointerType &>(*ResultType).pointeeType() != AllocatedType)
            {
              fail(Node.Offset, "alloca requires a non-void allocated type and its exact pointer result type");
              return nullptr;
            }
            return &Instructions.createAlloca(*AllocatedType);
          }
          case InstructionKind::Load:
          {
            if (!shape(Node, 1, 0))
            {
              return nullptr;
            }
            const Value *Source = operand(Node.Operands[0]);
            if (!Source)
            {
              return nullptr;
            }
            if (!PointerType::classof(&Source->getType()) || VoidType::classof(ResultType) || &static_cast<const PointerType &>(Source->getType()).pointeeType() != ResultType)
            {
              fail(Node.Offset, "load result type must match its pointer source's non-void pointee type");
              return nullptr;
            }
            return &Instructions.createLoad(*Source);
          }
          case InstructionKind::Store:
          {
            if (!shape(Node, 2, 0))
            {
              return nullptr;
            }
            const Value *Destination = operand(Node.Operands[0]);
            const Value *Source = operand(Node.Operands[1]);
            if (!Destination || !Source)
            {
              return nullptr;
            }
            if (!VoidType::classof(ResultType) || !PointerType::classof(&Destination->getType()) || &static_cast<const PointerType &>(Destination->getType()).pointeeType() != &Source->getType())
            {
              fail(Node.Offset, "store requires a void result and a pointer to its source's exact type");
              return nullptr;
            }
            return &Instructions.createStore(*Destination, *Source);
          }
          case InstructionKind::Call:
          {
            if (Node.Operands.empty() || !shape(Node, Node.Operands.size(), 0))
            {
              fail(Node.Offset, "call requires a callee and an argument list");
              return nullptr;
            }
            const Value *Callee = operand(Node.Operands.front());
            if (!Callee)
            {
              return nullptr;
            }
            if (!FunctionType::classof(&Callee->getType()))
            {
              fail(Node.Offset, "call callee must have a function type");
              return nullptr;
            }
            const auto &Signature = static_cast<const FunctionType &>(Callee->getType());
            const auto Parameters = Signature.parameters();
            const std::size_t ArgumentCount = Node.Operands.size() - 1;
            if (&Signature.returnType() != ResultType || ArgumentCount < Parameters.size() || (!Signature.isVariadic() && ArgumentCount != Parameters.size()))
            {
              fail(Node.Offset, "call result type or argument count does not match its callee signature");
              return nullptr;
            }
            std::vector<ValueRef> Arguments;
            Arguments.reserve(ArgumentCount);
            for (std::size_t Index = 0; Index < ArgumentCount; ++Index)
            {
              const Value *Argument = operand(Node.Operands[Index + 1]);
              if (!Argument)
              {
                return nullptr;
              }
              if (VoidType::classof(&Argument->getType()) || (Index < Parameters.size() && &Argument->getType() != &Parameters[Index].type()))
              {
                fail(Node.Offset, "call arguments must be non-void and match the fixed parameter types");
                return nullptr;
              }
              Arguments.emplace_back(*Argument);
            }
            return &Instructions.createCall(*Callee, Arguments);
          }
          case InstructionKind::Phi:
            if (!shape(Node, 0, 0, true))
            {
              return nullptr;
            }
            if (VoidType::classof(ResultType))
            {
              fail(Node.Offset, "phi result type must not be void");
              return nullptr;
            }
            return &Instructions.createPhi(*ResultType);
          case InstructionKind::Return:
          {
            const Value &ReturnType = State.Owner->signature().returnType();
            const bool IsVoid = VoidType::classof(&ReturnType);
            if (!shape(Node, IsVoid ? 0 : 1, 0))
            {
              return nullptr;
            }
            if (!VoidType::classof(ResultType))
            {
              fail(Node.Offset, "return instruction result type must be void");
              return nullptr;
            }
            if (IsVoid)
            {
              return &Instructions.createReturn();
            }
            const Value *Returned = operand(Node.Operands[0]);
            if (!Returned)
            {
              return nullptr;
            }
            if (&Returned->getType() != &ReturnType)
            {
              fail(Node.Offset, "return value type does not match the function signature");
              return nullptr;
            }
            return &Instructions.createReturn(*Returned);
          }
          case InstructionKind::Branch:
          {
            if (!shape(Node, 0, 1))
            {
              return nullptr;
            }
            BasicBlock *Target = block(Node.Targets[0], *State.Owner, Node.Offset);
            if (!Target)
            {
              return nullptr;
            }
            if (!VoidType::classof(ResultType))
            {
              fail(Node.Offset, "branch result type must be void");
              return nullptr;
            }
            return &Instructions.createBranch(*Target);
          }
          case InstructionKind::CondBranch:
          {
            if (!shape(Node, 1, 2))
            {
              return nullptr;
            }
            const Value *Condition = operand(Node.Operands[0]);
            BasicBlock *TrueTarget = block(Node.Targets[0], *State.Owner, Node.Offset);
            BasicBlock *FalseTarget = block(Node.Targets[1], *State.Owner, Node.Offset);
            if (!Condition || !TrueTarget || !FalseTarget)
            {
              return nullptr;
            }
            if (!VoidType::classof(ResultType) || !BoolType::classof(&Condition->getType()))
            {
              fail(Node.Offset, "conditional branch requires a void result and a boolean condition");
              return nullptr;
            }
            return &Instructions.createCondBranch(*Condition, *TrueTarget, *FalseTarget);
          }
          }
          fail(Node.Offset, "instruction kind is invalid");
          return nullptr;
        }

        bool createInstructions()
        {
          std::size_t Remaining = 0;
          for (const BlockState &State : BlockStates)
          {
            Remaining += State.Syntax->Instructions.size();
          }
          while (Remaining != 0)
          {
            const std::size_t Previous = Remaining;
            for (BlockState &State : BlockStates)
            {
              while (State.Next < State.Syntax->Instructions.size())
              {
                const InstructionSyntax &Node = State.Syntax->Instructions[State.Next];
                Instruction *Created = createInstruction(Node, State);
                if (!Created)
                {
                  if (Failure)
                  {
                    return false;
                  }
                  break;
                }
                if (!Node.Id.empty())
                {
                  Values.emplace(Node.Id, Created);
                }
                InstructionStates.push_back({&Node, Created});
                ++State.Next;
                --Remaining;
              }
            }
            if (Remaining == Previous)
            {
              return fail(0, "instruction type dependencies contain a cycle or cannot be resolved");
            }
          }
          return true;
        }

        bool bindOperands()
        {
          for (const InstructionState &State : InstructionStates)
          {
            const InstructionSyntax &Node = *State.Syntax;
            if (Node.Kind == InstructionKind::Alloca)
            {
              continue;
            }
            if (Node.Kind == InstructionKind::Phi)
            {
              auto &Phi = static_cast<PhiInstruction &>(*State.Value);
              for (const IncomingSyntax &Incoming : Node.Incoming)
              {
                const Value *IncomingValue = resolve(Incoming.Value, false, true);
                BasicBlock *Predecessor = block(Incoming.Block, *State.Value->parent()->parent(), Incoming.Offset);
                if (!IncomingValue || !Predecessor)
                {
                  return Failure ? false : fail(Incoming.Offset, "phi incoming value is unresolved");
                }
                if (&IncomingValue->getType() != &Phi.getType())
                {
                  return fail(Incoming.Offset, "phi incoming value type does not match its result type");
                }
                Phi.addIncoming(*IncomingValue, *Predecessor);
              }
              continue;
            }
            for (std::size_t Index = 0; Index < Node.Operands.size(); ++Index)
            {
              const Value *Operand = resolve(Node.Operands[Index], false, true);
              if (!Operand)
              {
                return Failure ? false : fail(Node.Operands[Index].Offset, "instruction operand remains unresolved");
              }
              if (!State.Value->setOperand(Index, *Operand))
              {
                return fail(Node.Operands[Index].Offset, "instruction operand type differs from its declared type");
              }
            }
          }
          PendingValues.clear();
          return true;
        }

        const ModuleSyntax &Syntax;
        std::unique_ptr<ParsedModule> Owned;
        TypePool &Types;
        ConstantPool &Constants;
        Module &ModuleValue;
        InstructionBuilder &Instructions;
        std::optional<core::Diagnostic> Failure;
        std::unordered_map<std::string_view, Definition> Definitions;
        std::unordered_map<std::string_view, const Value *> Values;
        std::unordered_map<std::string_view, BasicBlock *> Blocks;
        std::unordered_map<const RecordSyntax *, RecordType *> Records;
        std::vector<FunctionState> Functions;
        std::vector<BlockState> BlockStates;
        std::vector<InstructionState> InstructionStates;
        std::vector<std::unique_ptr<PendingValue>> PendingValues;
    };
  } // namespace

  std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> buildModule(const ModuleSyntax &Syntax)
  {
    return Builder(Syntax).run();
  }
} // namespace ink::ir::text
