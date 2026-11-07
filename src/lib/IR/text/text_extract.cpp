#include "text_internal.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/instruction/instruction_visitor.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::ir::text
{
  namespace
  {
    class Extractor final
    {
      public:
        explicit Extractor(const Module &ModuleValue)
            : ModuleValue(ModuleValue)
        {
        }

        std::expected<ModuleSyntax, core::Diagnostic> run()
        {
          std::string Description;
          if (!validateTextGraph(ModuleValue, Description))
          {
            return std::unexpected(error(0, Description));
          }
          for (const StructType &Record : ModuleValue.structTypes())
          {
            Registered.insert(&Record);
            reserveRecord(Record);
          }
          for (const ObjectType &Record : ModuleValue.objectTypes())
          {
            Registered.insert(&Record);
            reserveRecord(Record);
          }
          for (const Function &FunctionValue : ModuleValue.functions())
          {
            registerFunction(FunctionValue);
          }
          for (const GlobalVariable &Global : ModuleValue.globalVariables())
          {
            ValueIds.emplace(&Global, "@g" + std::to_string(GlobalCount++));
          }
          for (const GlobalVariable &Global : ModuleValue.globalVariables())
          {
            collectType(Global.getType());
          }
          for (const Function &FunctionValue : ModuleValue.functions())
          {
            collectFunction(FunctionValue);
          }
          // Process newly discovered records as a queue: method bodies can link arbitrarily long record chains.
          for (std::size_t Index = 0; Index < Records.size(); ++Index)
          {
            registerRecord(*Records[Index]);
          }
          ModuleSyntax Result;
          Result.Name = spelling(ModuleValue.name());
          for (const RecordType *Record : Records)
          {
            RecordSyntax Node;
            Node.Id = reference(*Record);
            Node.IsObject = ObjectType::classof(Record);
            Node.Registered = Registered.contains(Record);
            for (const Value *Base : Record->bases())
            {
              Node.Bases.push_back(expression(*Base));
            }
            for (const Field &FieldValue : Record->fields())
            {
              FieldSyntax FieldNode;
              FieldNode.Name = spelling(FieldValue.name());
              FieldNode.Type = expression(FieldValue.type());
              FieldNode.Kind = FieldValue.kind();
              FieldNode.Visibility = FieldValue.visibility();
              Node.Fields.push_back(std::move(FieldNode));
            }
            for (const Method &MethodValue : Record->methods())
            {
              MethodSyntax MethodNode;
              MethodNode.Kind = MethodValue.kind();
              MethodNode.Function = function(MethodValue.function());
              Node.Methods.push_back(std::move(MethodNode));
            }
            Result.Records.push_back(std::move(Node));
          }
          for (const GlobalVariable &Global : ModuleValue.globalVariables())
          {
            GlobalSyntax Node;
            Node.Id = reference(Global);
            Node.Name = spelling(Global.name());
            Node.Type = expression(Global.getType());
            Node.Visibility = Global.visibility();
            Node.Linkage = Global.nativeLinkage();
            Result.Globals.push_back(std::move(Node));
          }
          for (const Function &FunctionValue : ModuleValue.functions())
          {
            Result.Functions.push_back(function(FunctionValue));
          }
          if (Failure)
          {
            return std::unexpected(std::move(*Failure));
          }
          return Result;
        }

      private:
        static std::string spelling(core::Name Name)
        {
          return std::string(core::NamePool::instance().text(Name));
        }

        void fail(std::string_view Description)
        {
          if (!Failure)
          {
            Failure = error(0, Description);
          }
        }

        std::string reference(const Value &Node)
        {
          const auto Found = ValueIds.find(&Node);
          if (Found == ValueIds.end())
          {
            fail("module text cannot represent a value owned outside the module and its reachable records");
            return {};
          }
          return Found->second;
        }

        std::string blockReference(const BasicBlock &Block)
        {
          const auto Found = BlockIds.find(&Block);
          if (Found == BlockIds.end())
          {
            fail("module text cannot represent a block owned outside the module and its reachable records");
            return {};
          }
          return Found->second;
        }

        void registerFunction(const Function &FunctionValue)
        {
          if (!ValueIds.emplace(&FunctionValue, "@f" + std::to_string(FunctionCount)).second)
          {
            return;
          }
          ++FunctionCount;
          for (const Argument &ArgumentValue : FunctionValue.arguments())
          {
            ValueIds.emplace(&ArgumentValue, "%a" + std::to_string(ArgumentCount++));
          }
          for (const BasicBlock &Block : FunctionValue.basicBlocks())
          {
            BlockIds.emplace(&Block, "%bb" + std::to_string(BlockCount++));
            for (const Instruction &Node : Block.instructions())
            {
              if (!VoidType::classof(&Node.getType()))
              {
                ValueIds.emplace(&Node, "%v" + std::to_string(InstructionCount++));
              }
            }
          }
        }

        void reserveRecord(const RecordType &Record)
        {
          if (!ValueIds.emplace(&Record, "%t" + std::to_string(Records.size())).second)
          {
            return;
          }
          Records.push_back(&Record);
        }

        void registerRecord(const RecordType &Record)
        {
          reserveRecord(Record);
          if (!CollectedRecords.insert(&Record).second)
          {
            return;
          }
          for (const Method &MethodValue : Record.methods())
          {
            registerFunction(MethodValue.function());
          }
          for (const Value *Base : Record.bases())
          {
            collectType(*Base);
          }
          for (const Field &FieldValue : Record.fields())
          {
            collectType(FieldValue.type());
          }
          for (const Method &MethodValue : Record.methods())
          {
            collectFunction(MethodValue.function());
          }
        }

        void collectType(const Value &Node)
        {
          if (!Type::classof(&Node) || !CollectedTypes.insert(&Node).second)
          {
            return;
          }
          const auto &TypeValue = static_cast<const Type &>(Node);
          switch (TypeValue.getKind())
          {
          case TypeKind::Pointer:
            collectType(static_cast<const PointerType &>(TypeValue).pointeeType());
            return;
          case TypeKind::Array:
            collectType(static_cast<const ArrayType &>(TypeValue).elementType());
            return;
          case TypeKind::Function:
          {
            const auto &Signature = static_cast<const FunctionType &>(TypeValue);
            collectType(Signature.returnType());
            for (const Parameter &ParameterValue : Signature.parameters())
            {
              collectType(ParameterValue.type());
            }
            return;
          }
          case TypeKind::Struct:
          case TypeKind::Object:
            reserveRecord(static_cast<const RecordType &>(TypeValue));
            return;
          default:
            return;
          }
        }

        void collectFunction(const Function &FunctionValue)
        {
          collectType(FunctionValue.getType());
          for (const BasicBlock &Block : FunctionValue.basicBlocks())
          {
            for (const Instruction &Node : Block.instructions())
            {
              collectType(Node.getType());
              for (std::size_t Index = 0; Index < Node.operandCount(); ++Index)
              {
                const Value &Operand = Node.operand(Index);
                collectType(Operand.getType());
                if (Type::classof(&Operand))
                {
                  collectType(Operand);
                }
              }
            }
          }
        }

        Expression expression(const Value &Node, std::size_t Depth = 0)
        {
          Expression Result;
          if (Depth >= 128)
          {
            fail("module text type nesting exceeds the supported depth of 128");
            return Result;
          }
          if (Type::classof(&Node))
          {
            const auto &TypeValue = static_cast<const Type &>(Node);
            switch (TypeValue.getKind())
            {
            case TypeKind::Meta:
              Result.Kind = ExpressionKind::MetaType;
              break;
            case TypeKind::Bool:
              Result.Kind = ExpressionKind::BoolType;
              break;
            case TypeKind::Void:
              Result.Kind = ExpressionKind::VoidType;
              break;
            case TypeKind::Integer:
            {
              const auto &Integer = static_cast<const IntegerType &>(TypeValue);
              Result.Kind = ExpressionKind::IntegerType;
              Result.Width = Integer.bitWidth();
              Result.Signed = Integer.isSigned();
              break;
            }
            case TypeKind::Float:
              Result.Kind = ExpressionKind::FloatType;
              Result.Width = static_cast<const FloatType &>(TypeValue).bitWidth();
              break;
            case TypeKind::Pointer:
              Result.Kind = ExpressionKind::PointerType;
              Result.Children.push_back(expression(static_cast<const PointerType &>(TypeValue).pointeeType(), Depth + 1));
              break;
            case TypeKind::Array:
            {
              const auto &Array = static_cast<const ArrayType &>(TypeValue);
              Result.Kind = ExpressionKind::ArrayType;
              Result.Count = Array.elementCount();
              Result.Children.push_back(expression(Array.elementType(), Depth + 1));
              break;
            }
            case TypeKind::Function:
            {
              const auto &Signature = static_cast<const FunctionType &>(TypeValue);
              Result.Kind = ExpressionKind::FunctionType;
              Result.ABI = Signature.callABI();
              Result.Variadic = Signature.isVariadic();
              Result.Children.push_back(expression(Signature.returnType(), Depth + 1));
              for (const Parameter &ParameterValue : Signature.parameters())
              {
                Result.ParameterKinds.push_back(ParameterValue.kind());
                Result.Children.push_back(expression(ParameterValue.type(), Depth + 1));
              }
              break;
            }
            case TypeKind::Struct:
            case TypeKind::Object:
              Result.Text = reference(Node);
              break;
            }
            return Result;
          }
          switch (Node.valueKind())
          {
          case ValueKind::IntegerConstant:
            Result.Kind = ExpressionKind::IntegerConstant;
            Result.Bits = static_cast<const IntegerConstant &>(Node).bits();
            Result.Children.push_back(expression(Node.getType(), Depth + 1));
            break;
          case ValueKind::FloatConstant:
            Result.Kind = ExpressionKind::FloatConstant;
            Result.Bits = static_cast<const FloatConstant &>(Node).bits();
            Result.Children.push_back(expression(Node.getType(), Depth + 1));
            break;
          case ValueKind::StringConstant:
            Result.Kind = ExpressionKind::StringConstant;
            Result.Text = static_cast<const StringConstant &>(Node).value();
            break;
          case ValueKind::Argument:
          case ValueKind::Instruction:
          case ValueKind::Function:
          case ValueKind::GlobalVariable:
            Result.Text = reference(Node);
            break;
          default:
            fail("module text encountered an unsupported value kind");
            break;
          }
          return Result;
        }

        FunctionSyntax function(const Function &FunctionValue)
        {
          FunctionSyntax Result;
          Result.Id = reference(FunctionValue);
          Result.Name = spelling(FunctionValue.name());
          Result.Signature = expression(FunctionValue.getType());
          Result.Visibility = FunctionValue.visibility();
          Result.Linkage = FunctionValue.nativeLinkage();
          for (const Argument &ArgumentValue : FunctionValue.arguments())
          {
            Result.Arguments.push_back({reference(ArgumentValue), spelling(ArgumentValue.name())});
          }
          for (const BasicBlock &Block : FunctionValue.basicBlocks())
          {
            BlockSyntax BlockNode;
            BlockNode.Id = blockReference(Block);
            BlockNode.Name = spelling(Block.name());
            for (const Instruction &Node : Block.instructions())
            {
              InstructionSyntax InstructionNode;
              if (!VoidType::classof(&Node.getType()))
              {
                InstructionNode.Id = reference(Node);
              }
              InstructionNode.Kind = Node.getKind();
              InstructionNode.ResultType = expression(Node.getType());
              if (Node.getKind() == InstructionKind::Alloca)
              {
                InstructionNode.Operands.push_back(expression(static_cast<const AllocaInstruction &>(Node).getAllocatedType()));
              }
              else if (Node.getKind() == InstructionKind::Phi)
              {
                const auto &Phi = static_cast<const PhiInstruction &>(Node);
                for (const PhiInstruction::Incoming &Incoming : Phi.incoming())
                {
                  InstructionNode.Incoming.push_back({expression(Incoming.getValue()), blockReference(Incoming.getBlock())});
                }
              }
              else
              {
                for (std::size_t Index = 0; Index < Node.operandCount(); ++Index)
                {
                  InstructionNode.Operands.push_back(expression(Node.operand(Index)));
                }
              }
              if (Node.getKind() == InstructionKind::Branch)
              {
                InstructionNode.Targets.push_back(blockReference(static_cast<const BranchInstruction &>(Node).getTarget()));
              }
              else if (Node.getKind() == InstructionKind::CondBranch)
              {
                const auto &Branch = static_cast<const CondBranchInstruction &>(Node);
                InstructionNode.Targets.push_back(blockReference(Branch.getTrueTarget()));
                InstructionNode.Targets.push_back(blockReference(Branch.getFalseTarget()));
              }
              BlockNode.Instructions.push_back(std::move(InstructionNode));
            }
            Result.Blocks.push_back(std::move(BlockNode));
          }
          return Result;
        }

        const Module &ModuleValue;
        std::optional<core::Diagnostic> Failure;
        std::unordered_map<const Value *, std::string> ValueIds;
        std::unordered_map<const BasicBlock *, std::string> BlockIds;
        std::unordered_set<const Value *> CollectedTypes;
        std::unordered_set<const RecordType *> Registered;
        std::unordered_set<const RecordType *> CollectedRecords;
        std::vector<const RecordType *> Records;
        std::size_t FunctionCount = 0;
        std::size_t GlobalCount = 0;
        std::size_t ArgumentCount = 0;
        std::size_t BlockCount = 0;
        std::size_t InstructionCount = 0;
    };
  } // namespace

  std::expected<ModuleSyntax, core::Diagnostic> extractModule(const Module &ModuleValue)
  {
    return Extractor(ModuleValue).run();
  }
} // namespace ink::ir::text
