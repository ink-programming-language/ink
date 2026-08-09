#include "ink/frontend/ir_generator.h"

#include "ink/ast/ast_context.h"
#include "ink/ir/builder.h"
#include "ink/ir/opcode.h"
#include "ink/type/type.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ink::frontend
{
  namespace
  {
    class GenerationFailure : public std::runtime_error
    {
    public:
      GenerationFailure(core::SourceRange Range, std::string Message) : std::runtime_error(Message), Range(Range)
      {
      }

      core::SourceRange Range;
    };

    struct LoweredSymbol
    {
      std::optional<ir::IrValueId> Value;
      std::optional<ir::IrValueId> Place;
    };

    struct LoopTargets
    {
      ir::IrBlockId Continue;
      ir::IrBlockId Break;
    };

    class GeneratorImpl
    {
    public:
      explicit GeneratorImpl(const sema::VerifiedSemanticModule &SemanticModule) : SemanticModule(SemanticModule), Model(SemanticModule.model()), Ast(Model.astContext()), File(Model.astFile()), Strings(Model.strings()), Types(Model.typeContext()), TypeIds(Types.size()), FunctionIds(Model.symbols().size())
      {
      }

      IrGenerationResult generate()
      {
        try
        {
          declareFunctions();
          lowerFunctions();
          IrGenerationResult Result;
          Result.Module = Builder.finish();
          Result.PendingForceValues = std::move(PendingForceValues);
          return Result;
        }
        catch (const GenerationFailure &Failure)
        {
          return {std::nullopt, {}, {{Failure.Range, Failure.what()}}};
        }
      }

    private:
      const sema::VerifiedSemanticModule &SemanticModule;
      const sema::SemanticModel &Model;
      const ast::AstContext &Ast;
      const ast::AstFile &File;
      const core::StringInterner &Strings;
      const type::TypeContext &Types;
      ir::IrBuilder Builder;
      std::vector<std::optional<ir::IrTypeId>> TypeIds;
      std::vector<std::optional<ir::IrFunctionId>> FunctionIds;
      std::vector<PendingForceValue> PendingForceValues;
      std::vector<LoweredSymbol> Symbols;
      std::vector<bool> TerminatedBlocks;
      std::vector<LoopTargets> Loops;
      ir::IrFunctionId CurrentFunction;
      ir::IrBlockId CurrentBlock;
      bool CurrentReachable = false;

      [[noreturn]] void fail(core::SourceRange Range, std::string Message) const
      {
        throw GenerationFailure(Range, std::move(Message));
      }

      ir::IrOriginId origin(core::SourceRange Range)
      {
        return Builder.addOrigin(Model.sourceFile(), Range);
      }

      ir::IrTypeId mapType(type::TypeId Type)
      {
        if (!Types.contains(Type))
        {
          fail({}, "semantic model contains an unknown type ID");
        }
        std::optional<ir::IrTypeId> &Cached = TypeIds[Type.value()];
        if (Cached)
        {
          return *Cached;
        }
        const type::Type &SemanticType = Types.type(Type);
        switch (SemanticType.Kind)
        {
        case type::TypeKind::Unit:
          Cached = Builder.unitType();
          break;
        case type::TypeKind::Bool:
          Cached = Builder.boolType();
          break;
        case type::TypeKind::I32:
          Cached = Builder.integerType(32, ir::IrSignedness::Signed);
          break;
        case type::TypeKind::I64:
          Cached = Builder.integerType(64, ir::IrSignedness::Signed);
          break;
        case type::TypeKind::U32:
          Cached = Builder.integerType(32, ir::IrSignedness::Unsigned);
          break;
        case type::TypeKind::U64:
          Cached = Builder.integerType(64, ir::IrSignedness::Unsigned);
          break;
        case type::TypeKind::Never:
          Cached = Builder.neverType();
          break;
        case type::TypeKind::Function:
        {
          const type::FunctionType &Function = Types.function(Type);
          std::vector<ir::IrTypeId> Parameters;
          Parameters.reserve(Function.Parameters.size());
          for (type::TypeId Parameter : Function.Parameters)
          {
            Parameters.push_back(mapValueType(Parameter, {}));
          }
          Cached = Builder.functionType(Parameters, mapResultType(Function.Result, {}));
          break;
        }
        case type::TypeKind::Void:
          fail({}, "void is a function channel and cannot be lowered as an InkIR value type");
        case type::TypeKind::Error:
          fail({}, "verified semantic module contains ErrorType");
        }
        return *Cached;
      }

      type::TypeKind typeKind(type::TypeId Type, core::SourceRange Range) const
      {
        if (!Types.contains(Type))
        {
          fail(Range, "semantic model contains an unknown type ID");
        }
        return Types.type(Type).Kind;
      }

      bool isUnsignedInteger(type::TypeId Type, core::SourceRange Range) const
      {
        switch (typeKind(Type, Range))
        {
        case type::TypeKind::U32:
        case type::TypeKind::U64:
          return true;
        case type::TypeKind::I32:
        case type::TypeKind::I64:
          return false;
        default:
          fail(Range, "integer operation has a non-integer semantic operand type");
        }
      }

      std::uint64_t allOnes(type::TypeId Type, core::SourceRange Range) const
      {
        switch (typeKind(Type, Range))
        {
        case type::TypeKind::I32:
        case type::TypeKind::U32:
          return 0xFFFFFFFFULL;
        case type::TypeKind::I64:
        case type::TypeKind::U64:
          return 0xFFFFFFFFFFFFFFFFULL;
        default:
          fail(Range, "bitwise complement has a non-integer semantic operand type");
        }
      }

      ir::IrComparePredicate orderedPredicate(std::string_view Operator, type::TypeId Type, core::SourceRange Range) const
      {
        const bool Unsigned = isUnsignedInteger(Type, Range);
        if (Operator == "<")
        {
          return Unsigned ? ir::IrComparePredicate::UnsignedLess : ir::IrComparePredicate::SignedLess;
        }
        if (Operator == "<=")
        {
          return Unsigned ? ir::IrComparePredicate::UnsignedLessEqual : ir::IrComparePredicate::SignedLessEqual;
        }
        if (Operator == ">")
        {
          return Unsigned ? ir::IrComparePredicate::UnsignedGreater : ir::IrComparePredicate::SignedGreater;
        }
        if (Operator == ">=")
        {
          return Unsigned ? ir::IrComparePredicate::UnsignedGreaterEqual : ir::IrComparePredicate::SignedGreaterEqual;
        }
        fail(Range, "ordered comparison has an unknown operator");
      }

      ir::IrTypeId mapValueType(type::TypeId Type, core::SourceRange Range)
      {
        if (Types.contains(Type) && Types.type(Type).Kind == type::TypeKind::Void)
        {
          fail(Range, "void cannot be used as a value type");
        }
        return mapType(Type);
      }

      std::optional<ir::IrTypeId> mapResultType(type::TypeId Type, core::SourceRange Range)
      {
        if (!Types.contains(Type))
        {
          fail(Range, "function result has an unknown semantic type");
        }
        if (Types.type(Type).Kind == type::TypeKind::Void)
        {
          return std::nullopt;
        }
        return mapValueType(Type, Range);
      }

      bool isTrapIntrinsic(sema::SymbolId SymbolId) const
      {
        if (!Model.contains(SymbolId))
        {
          return false;
        }
        const sema::Symbol &Symbol = Model.symbol(SymbolId);
        if (Symbol.Kind != sema::SymbolKind::Function || !Types.contains(Symbol.Type) || Types.type(Symbol.Type).Kind != type::TypeKind::Function)
        {
          return false;
        }
        const ast::Declaration &Declaration = Ast.declaration(Symbol.Declaration);
        const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Declaration.Payload);
        if (Payload == nullptr || Payload->Body || !Ast.list(Payload->Parameters).empty() || Strings.string(Payload->Name) != "trap")
        {
          return false;
        }
        const type::FunctionType &Function = Types.function(Symbol.Type);
        return Function.Parameters.empty() && Types.contains(Function.Result) && Types.type(Function.Result).Kind == type::TypeKind::Never;
      }

      void declareFunctions()
      {
        for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
        {
          const ast::AstDeclId DeclarationId = ast::AstDeclId::fromValue(Index);
          const ast::Declaration &Declaration = Ast.declaration(DeclarationId);
          if (Declaration.Kind != ast::AstKind::FunctionDeclaration)
          {
            continue;
          }
          const std::optional<sema::SymbolId> SymbolId = Model.declarationSymbol(DeclarationId);
          const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Declaration.Payload);
          if (!SymbolId || !Payload || !Model.contains(*SymbolId))
          {
            fail(Declaration.Header.Range, "verified function declaration has no semantic symbol");
          }
          const sema::Symbol &Symbol = Model.symbol(*SymbolId);
          if (isTrapIntrinsic(*SymbolId))
          {
            continue;
          }
          const ir::IrTypeId Signature = mapType(Symbol.Type);
          const std::string Name(Strings.string(Payload->Name));
          const ir::IrFunctionKind Kind = Payload->Body ? ir::IrFunctionKind::Definition : ir::IrFunctionKind::External;
          FunctionIds[SymbolId->value()] = Builder.addFunction(Name, Signature, Kind, origin(Declaration.Header.Range));
        }
      }

      void lowerFunctions()
      {
        for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
        {
          const ast::AstDeclId DeclarationId = ast::AstDeclId::fromValue(Index);
          const ast::Declaration &Declaration = Ast.declaration(DeclarationId);
          if (Declaration.Kind != ast::AstKind::FunctionDeclaration)
          {
            if (Declaration.Kind == ast::AstKind::BindingDeclaration && std::get<ast::BindingPayload>(Declaration.Payload).TopLevel)
            {
              fail(Declaration.Header.Range, "top-level storage is not enabled in the first InkIR source lowering slice");
            }
            continue;
          }
          const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Declaration.Payload);
          if (Payload && Payload->Body)
          {
            lowerFunction(DeclarationId, Declaration, *Payload);
          }
        }
      }

      void lowerFunction(ast::AstDeclId DeclarationId, const ast::Declaration &Declaration, const ast::FunctionPayload &Payload)
      {
        const std::optional<sema::SymbolId> FunctionSymbol = Model.declarationSymbol(DeclarationId);
        if (!FunctionSymbol || FunctionSymbol->value() >= FunctionIds.size() || !FunctionIds[FunctionSymbol->value()])
        {
          fail(Declaration.Header.Range, "function was not declared before InkIR body lowering");
        }
        CurrentFunction = *FunctionIds[FunctionSymbol->value()];
        Symbols.assign(Model.symbols().size(), {});
        Loops.clear();
        const type::FunctionType &FunctionType = Types.function(Model.symbol(*FunctionSymbol).Type);
        std::vector<ir::IrTypeId> ParameterTypes;
        ParameterTypes.reserve(FunctionType.Parameters.size());
        for (type::TypeId Parameter : FunctionType.Parameters)
        {
          ParameterTypes.push_back(mapValueType(Parameter, Declaration.Header.Range));
        }
        const ir::IrBuiltBlock Entry = addBlock(ParameterTypes, Declaration.Header.Range);
        Builder.setEntryBlock(CurrentFunction, Entry.Block);
        setCurrent(Entry.Block);
        const ast::AstNodeListView Parameters = Ast.list(Payload.Parameters);
        if (Parameters.size() != Entry.Arguments.size())
        {
          fail(Declaration.Header.Range, "semantic function parameter count changed before InkIR lowering");
        }
        for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
        {
          if (Parameters[Index].Category != ast::AstNodeCategory::Declaration)
          {
            fail(Declaration.Header.Range, "function parameter list contains a non-declaration node");
          }
          const ast::AstDeclId Parameter = ast::AstDeclId::fromValue(Parameters[Index].Index);
          const std::optional<sema::SymbolId> Symbol = Model.declarationSymbol(Parameter);
          if (!Symbol)
          {
            fail(Ast.declaration(Parameter).Header.Range, "verified parameter has no semantic symbol");
          }
          Symbols[Symbol->value()].Value = Entry.Arguments[Index];
        }
        lowerStatement(*Payload.Body);
        if (CurrentReachable && !isTerminated(CurrentBlock))
        {
          if (Types.type(FunctionType.Result).Kind == type::TypeKind::Void)
          {
            terminate(Builder.createReturn(CurrentBlock, std::nullopt, origin(Declaration.Header.Range)));
          }
          else
          {
            fail(Declaration.Header.Range, "verified value-returning function reached InkIR lowering with a fallthrough path");
          }
        }
      }

      ir::IrBuiltBlock addBlock(const std::vector<ir::IrTypeId> &Arguments, core::SourceRange Range)
      {
        ir::IrBuiltBlock Block = Builder.addBlock(CurrentFunction, Arguments, origin(Range));
        if (Block.Block.value() >= TerminatedBlocks.size())
        {
          TerminatedBlocks.resize(static_cast<std::size_t>(Block.Block.value()) + 1);
        }
        return Block;
      }

      void setCurrent(ir::IrBlockId Block)
      {
        CurrentBlock = Block;
        CurrentReachable = Block.isValid();
      }

      bool isTerminated(ir::IrBlockId Block) const
      {
        return Block.isValid() && Block.value() < TerminatedBlocks.size() && TerminatedBlocks[Block.value()];
      }

      void terminate(ir::IrOperationId)
      {
        if (!CurrentReachable || isTerminated(CurrentBlock))
        {
          fail({}, "InkIR lowering attempted to terminate an inactive block");
        }
        TerminatedBlocks[CurrentBlock.value()] = true;
      }

      void branchFrom(ir::IrBlockId Block, ir::IrBlockId Target, const std::vector<ir::IrValueId> &Arguments, core::SourceRange Range)
      {
        setCurrent(Block);
        terminate(Builder.createBranch(Block, Target, Arguments, origin(Range)));
      }

      ir::IrValueId requireValue(std::optional<ir::IrValueId> Value, core::SourceRange Range, const char *Context)
      {
        if (!Value)
        {
          fail(Range, std::string(Context) + " requires a value-producing expression");
        }
        return *Value;
      }

      std::optional<ir::IrValueId> lowerExpression(ast::AstExprId Id)
      {
        const ast::Expression &Node = Ast.expression(Id);
        switch (Node.Kind)
        {
        case ast::AstKind::LiteralExpression:
          return lowerLiteral(Id, Node);
        case ast::AstKind::NameExpression:
          return lowerName(Id, Node);
        case ast::AstKind::GroupExpression:
          return lowerExpression(std::get<ast::GroupPayload>(Node.Payload).Value);
        case ast::AstKind::UnaryExpression:
          return lowerUnary(Id, Node);
        case ast::AstKind::ComptimeExpression:
          return lowerComptime(Id, Node);
        case ast::AstKind::BinaryExpression:
          return lowerBinary(Id, Node);
        case ast::AstKind::CallExpression:
          return lowerCall(Id, Node);
        case ast::AstKind::IfExpression:
          return lowerIfExpression(Id, Node);
        default:
          fail(Node.Header.Range, std::string("AST expression kind cannot be lowered to the first InkIR slice: ") + ast::astKindName(Node.Kind));
        }
      }

      std::optional<ir::IrValueId> lowerLiteral(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::LiteralPayload *Payload = std::get_if<ast::LiteralPayload>(&Node.Payload);
        if (!Payload)
        {
          fail(Node.Header.Range, "literal expression payload is malformed");
        }
        const std::optional<sema::ConstantValue> Constant = Model.constantValue(Id);
        if (!Constant)
        {
          fail(Node.Header.Range, "literal is not representable by the first semantic constant model");
        }
        if (const bool *Value = std::get_if<bool>(&*Constant))
        {
          return Builder.createBoolConstant(CurrentBlock, Builder.boolConstant(*Value), origin(Node.Header.Range));
        }
        const std::int32_t Value = std::get<std::int32_t>(*Constant);
        const ir::IrTypeId Type = mapValueType(Model.expressionType(Id), Node.Header.Range);
        return Builder.createIntegerConstant(CurrentBlock, Builder.integerConstant(Type, static_cast<std::uint32_t>(Value)), origin(Node.Header.Range));
      }

      std::optional<ir::IrValueId> lowerName(ast::AstExprId Id, const ast::Expression &Node)
      {
        const sema::ResolvedName Resolution = Model.resolvedName(Id);
        if (Resolution.Status != sema::ResolvedNameStatus::Resolved || !Model.contains(Resolution.Symbol) || Resolution.Symbol.value() >= Symbols.size())
        {
          fail(Node.Header.Range, "verified name expression has no unique semantic resolution");
        }
        const LoweredSymbol &Symbol = Symbols[Resolution.Symbol.value()];
        if (Symbol.Place)
        {
          return Builder.createLoad(CurrentBlock, *Symbol.Place, origin(Node.Header.Range));
        }
        if (Symbol.Value)
        {
          return *Symbol.Value;
        }
        if (Model.symbol(Resolution.Symbol).Kind == sema::SymbolKind::Function)
        {
          fail(Node.Header.Range, "first-slice InkIR cannot materialize a function as an ordinary value");
        }
        fail(Node.Header.Range, "resolved symbol has no lowered storage or value");
      }

      std::optional<ir::IrValueId> lowerUnary(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::UnaryPayload &Payload = std::get<ast::UnaryPayload>(Node.Payload);
        const ir::IrValueId Operand = requireValue(lowerExpression(Payload.Operand), Node.Header.Range, "unary operator");
        const std::string_view Operator = Strings.string(Payload.Operator);
        if (Operator == "+")
        {
          return Operand;
        }
        if (Operator == "-")
        {
          return Builder.createIntegerNegate(CurrentBlock, Operand, origin(Node.Header.Range));
        }
        if (Operator == "!")
        {
          return Builder.createBoolUnary(CurrentBlock, ir::IrOpcode::BoolNot, Operand, origin(Node.Header.Range));
        }
        if (Operator == "~")
        {
          const type::TypeId SemanticType = Model.expressionType(Id);
          const ir::IrTypeId Type = mapValueType(SemanticType, Node.Header.Range);
          const ir::IrValueId Ones = Builder.createIntegerConstant(CurrentBlock, Builder.integerConstant(Type, allOnes(SemanticType, Node.Header.Range)), origin(Node.Header.Range));
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntXor, Operand, Ones, origin(Node.Header.Range));
        }
        fail(Node.Header.Range, "unary operator has no first-slice InkIR opcode");
      }

      std::optional<ir::IrValueId> lowerComptime(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::UnaryPayload &Payload = std::get<ast::UnaryPayload>(Node.Payload);
        const ir::IrBlockId RuntimeBlock = CurrentBlock;
        const ir::IrBuiltBlock StagingBlock = addBlock({}, Node.Header.Range);
        setCurrent(StagingBlock.Block);
        const std::optional<ir::IrValueId> Lowered = lowerExpression(Payload.Operand);
        if (!Lowered)
        {
          fail(Node.Header.Range, "comptime requires a value-producing expression");
        }
        const ir::IrValueId Input = *Lowered;
        if (CurrentReachable && !isTerminated(CurrentBlock))
        {
          terminate(Builder.createUnreachable(CurrentBlock, origin(Node.Header.Range)));
        }
        setCurrent(RuntimeBlock);
        const type::TypeKind ResultKind = typeKind(Model.expressionType(Id), Node.Header.Range);
        const ir::IrTypeId ResultType = mapValueType(Model.expressionType(Id), Node.Header.Range);
        ir::IrValueId Output;
        if (ResultKind == type::TypeKind::Bool)
        {
          Output = Builder.createBoolConstant(CurrentBlock, Builder.boolConstant(false), origin(Node.Header.Range));
        }
        else if (ResultKind == type::TypeKind::I32 || ResultKind == type::TypeKind::I64 || ResultKind == type::TypeKind::U32 || ResultKind == type::TypeKind::U64)
        {
          Output = Builder.createIntegerConstant(CurrentBlock, Builder.integerConstant(ResultType, 0), origin(Node.Header.Range));
        }
        else
        {
          fail(Node.Header.Range, "comptime result is not representable by a first-slice InkIR constant");
        }
        const ir::IrPlanNodeId PlanNode = Builder.addForceValuePlan(Input, Output, origin(Node.Header.Range));
        PendingForceValues.push_back({PlanNode, CurrentFunction, Input, Output});
        return Output;
      }

      std::optional<ir::IrValueId> lowerBinary(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::BinaryPayload &Payload = std::get<ast::BinaryPayload>(Node.Payload);
        const std::string_view Operator = Strings.string(Payload.Operator);
        if (Operator == "&&" || Operator == "||")
        {
          return lowerLogical(Id, Node, Payload, Operator == "&&");
        }
        const ir::IrValueId Left = requireValue(lowerExpression(Payload.Left), Node.Header.Range, "binary operator");
        const ir::IrValueId Right = requireValue(lowerExpression(Payload.Right), Node.Header.Range, "binary operator");
        const type::TypeId OperandType = Model.expressionType(Payload.Left);
        if (Operator == "+")
        {
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntAdd, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "-")
        {
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntSub, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "*")
        {
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntMul, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "&")
        {
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntAnd, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "|")
        {
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntOr, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "^")
        {
          return Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntXor, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "==")
        {
          return Builder.createIntegerCompare(CurrentBlock, ir::IrComparePredicate::Equal, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "!=")
        {
          return Builder.createIntegerCompare(CurrentBlock, ir::IrComparePredicate::NotEqual, Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "<")
        {
          return Builder.createIntegerCompare(CurrentBlock, orderedPredicate(Operator, OperandType, Node.Header.Range), Left, Right, origin(Node.Header.Range));
        }
        if (Operator == "<=")
        {
          return Builder.createIntegerCompare(CurrentBlock, orderedPredicate(Operator, OperandType, Node.Header.Range), Left, Right, origin(Node.Header.Range));
        }
        if (Operator == ">")
        {
          return Builder.createIntegerCompare(CurrentBlock, orderedPredicate(Operator, OperandType, Node.Header.Range), Left, Right, origin(Node.Header.Range));
        }
        if (Operator == ">=")
        {
          return Builder.createIntegerCompare(CurrentBlock, orderedPredicate(Operator, OperandType, Node.Header.Range), Left, Right, origin(Node.Header.Range));
        }
        fail(Node.Header.Range, "binary operator has no first-slice InkIR opcode");
      }

      std::optional<ir::IrValueId> lowerLogical(ast::AstExprId Id, const ast::Expression &Node, const ast::BinaryPayload &Payload, bool IsAnd)
      {
        const ir::IrValueId Left = requireValue(lowerExpression(Payload.Left), Node.Header.Range, "logical operator");
        const ir::IrBlockId ConditionBlock = CurrentBlock;
        const ir::IrBuiltBlock RightBlock = addBlock({}, Ast.expression(Payload.Right).Header.Range);
        const ir::IrBuiltBlock ShortBlock = addBlock({}, Node.Header.Range);
        const ir::IrValueId ShortValue = [&]()
        {
          setCurrent(ShortBlock.Block);
          return Builder.createBoolConstant(CurrentBlock, Builder.boolConstant(!IsAnd), origin(Node.Header.Range));
        }();
        const ir::IrBlockId ShortEnd = CurrentBlock;
        setCurrent(RightBlock.Block);
        const ir::IrValueId Right = requireValue(lowerExpression(Payload.Right), Node.Header.Range, "logical operator");
        const ir::IrBlockId RightEnd = CurrentBlock;
        const ir::IrBuiltBlock Merge = addBlock({mapValueType(Model.expressionType(Id), Node.Header.Range)}, Node.Header.Range);
        branchFrom(ShortEnd, Merge.Block, {ShortValue}, Node.Header.Range);
        branchFrom(RightEnd, Merge.Block, {Right}, Node.Header.Range);
        setCurrent(ConditionBlock);
        if (IsAnd)
        {
          terminate(Builder.createConditionalBranch(ConditionBlock, Left, RightBlock.Block, {}, ShortBlock.Block, {}, origin(Node.Header.Range)));
        }
        else
        {
          terminate(Builder.createConditionalBranch(ConditionBlock, Left, ShortBlock.Block, {}, RightBlock.Block, {}, origin(Node.Header.Range)));
        }
        setCurrent(Merge.Block);
        return Merge.Arguments.front();
      }

      std::optional<ir::IrValueId> lowerCall(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::CallPayload &Payload = std::get<ast::CallPayload>(Node.Payload);
        const std::optional<sema::SymbolId> CalleeSymbol = directCallee(Payload.Callee);
        if (!CalleeSymbol || !Model.contains(*CalleeSymbol) || Model.symbol(*CalleeSymbol).Kind != sema::SymbolKind::Function)
        {
          fail(Node.Header.Range, "direct call has no lowered function target");
        }
        if (isTrapIntrinsic(*CalleeSymbol))
        {
          if (!Ast.list(Payload.Arguments).empty())
          {
            fail(Node.Header.Range, "trap intrinsic call must not have arguments");
          }
          terminate(Builder.createTrap(CurrentBlock, ir::IrTrapKind::User, origin(Node.Header.Range)));
          CurrentReachable = false;
          return std::nullopt;
        }
        if (CalleeSymbol->value() >= FunctionIds.size() || !FunctionIds[CalleeSymbol->value()])
        {
          fail(Node.Header.Range, "direct call has no lowered function target");
        }
        std::vector<ir::IrValueId> Arguments;
        for (ast::AstNodeRef Argument : Ast.list(Payload.Arguments))
        {
          if (Argument.Category != ast::AstNodeCategory::Expression)
          {
            fail(Node.Header.Range, "call argument is not an expression");
          }
          const ast::AstExprId ArgumentId = ast::AstExprId::fromValue(Argument.Index);
          Arguments.push_back(requireValue(lowerExpression(ArgumentId), Ast.expression(ArgumentId).Header.Range, "call argument"));
        }
        const ir::IrBuiltOperation Call = Builder.createDirectCall(CurrentBlock, *FunctionIds[CalleeSymbol->value()], Arguments, origin(Node.Header.Range));
        const type::TypeKind ResultKind = Types.type(Model.expressionType(Id)).Kind;
        if (ResultKind == type::TypeKind::Never)
        {
          terminate(Builder.createUnreachable(CurrentBlock, origin(Node.Header.Range)));
          CurrentReachable = false;
          return std::nullopt;
        }
        if (ResultKind == type::TypeKind::Void)
        {
          return std::nullopt;
        }
        if (Call.Results.size() != 1)
        {
          fail(Node.Header.Range, "value-returning direct call did not create one InkIR result");
        }
        return Call.Results.front();
      }

      std::optional<ir::IrValueId> lowerIfExpression(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::IfExpressionPayload &Payload = std::get<ast::IfExpressionPayload>(Node.Payload);
        const ir::IrValueId Condition = requireValue(lowerExpression(Payload.Condition), Ast.expression(Payload.Condition).Header.Range, "if expression condition");
        const ir::IrBlockId ConditionBlock = CurrentBlock;
        const ir::IrBuiltBlock ThenBlock = addBlock({}, Ast.expression(Payload.ThenValue).Header.Range);
        const ir::IrBuiltBlock ElseBlock = addBlock({}, Ast.expression(Payload.ElseValue).Header.Range);
        setCurrent(ThenBlock.Block);
        const std::optional<ir::IrValueId> ThenValue = lowerExpression(Payload.ThenValue);
        const ir::IrBlockId ThenEnd = CurrentBlock;
        const bool ThenContinues = CurrentReachable && !isTerminated(ThenEnd);
        setCurrent(ElseBlock.Block);
        const std::optional<ir::IrValueId> ElseValue = lowerExpression(Payload.ElseValue);
        const ir::IrBlockId ElseEnd = CurrentBlock;
        const bool ElseContinues = CurrentReachable && !isTerminated(ElseEnd);
        const type::TypeKind ResultKind = typeKind(Model.expressionType(Id), Node.Header.Range);
        if ((ThenContinues && !ThenValue && ResultKind != type::TypeKind::Void) || (ElseContinues && !ElseValue && ResultKind != type::TypeKind::Void))
        {
          fail(Node.Header.Range, "reachable if expression branch does not produce its semantic result");
        }
        if (ResultKind == type::TypeKind::Never && (ThenContinues || ElseContinues))
        {
          fail(Node.Header.Range, "never-typed if expression has a reachable continuation");
        }
        setCurrent(ConditionBlock);
        terminate(Builder.createConditionalBranch(ConditionBlock, Condition, ThenBlock.Block, {}, ElseBlock.Block, {}, origin(Node.Header.Range)));
        if (!ThenContinues && !ElseContinues)
        {
          CurrentReachable = false;
          return std::nullopt;
        }
        const bool ProducesValue = ResultKind != type::TypeKind::Void;
        const ir::IrBuiltBlock Merge = addBlock(ProducesValue ? std::vector<ir::IrTypeId>{mapValueType(Model.expressionType(Id), Node.Header.Range)} : std::vector<ir::IrTypeId>{}, Node.Header.Range);
        if (ThenContinues)
        {
          branchFrom(ThenEnd, Merge.Block, ProducesValue ? std::vector<ir::IrValueId>{*ThenValue} : std::vector<ir::IrValueId>{}, Node.Header.Range);
        }
        if (ElseContinues)
        {
          branchFrom(ElseEnd, Merge.Block, ProducesValue ? std::vector<ir::IrValueId>{*ElseValue} : std::vector<ir::IrValueId>{}, Node.Header.Range);
        }
        setCurrent(Merge.Block);
        return ProducesValue ? std::optional<ir::IrValueId>{Merge.Arguments.front()} : std::nullopt;
      }

      std::optional<sema::SymbolId> directCallee(ast::AstExprId Id) const
      {
        const ast::Expression &Node = Ast.expression(Id);
        if (Node.Kind == ast::AstKind::GroupExpression)
        {
          return directCallee(std::get<ast::GroupPayload>(Node.Payload).Value);
        }
        if (Node.Kind != ast::AstKind::NameExpression)
        {
          return std::nullopt;
        }
        const sema::ResolvedName Resolution = Model.resolvedName(Id);
        if (Resolution.Status != sema::ResolvedNameStatus::Resolved)
        {
          return std::nullopt;
        }
        return Resolution.Symbol;
      }

      ir::IrValueId lowerPlace(ast::AstExprId Id)
      {
        const ast::Expression &Node = Ast.expression(Id);
        if (Node.Kind == ast::AstKind::GroupExpression)
        {
          return lowerPlace(std::get<ast::GroupPayload>(Node.Payload).Value);
        }
        if (Node.Kind != ast::AstKind::NameExpression)
        {
          fail(Node.Header.Range, "first-slice assignment target is not a lowered local place");
        }
        const sema::ResolvedName Resolution = Model.resolvedName(Id);
        if (Resolution.Status != sema::ResolvedNameStatus::Resolved || Resolution.Symbol.value() >= Symbols.size() || !Symbols[Resolution.Symbol.value()].Place)
        {
          fail(Node.Header.Range, "assignment target has no mutable InkIR place");
        }
        return *Symbols[Resolution.Symbol.value()].Place;
      }

      void lowerDeclaration(ast::AstDeclId Id)
      {
        const ast::Declaration &Node = Ast.declaration(Id);
        if (Node.Kind != ast::AstKind::BindingDeclaration)
        {
          fail(Node.Header.Range, "non-binding declaration appeared in a first-slice statement block");
        }
        const ast::BindingPayload &Payload = std::get<ast::BindingPayload>(Node.Payload);
        if (Payload.TopLevel)
        {
          fail(Node.Header.Range, "top-level storage is not enabled in the first InkIR source lowering slice");
        }
        const std::optional<sema::SymbolId> Symbol = Model.patternSymbol(Payload.Pattern);
        if (!Symbol || Symbol->value() >= Symbols.size())
        {
          fail(Node.Header.Range, "verified binding has no semantic symbol");
        }
        if (!Payload.Initializer)
        {
          fail(Node.Header.Range, "first-slice local binding requires an initializer");
        }
        const std::optional<ir::IrValueId> LoweredInitializer = lowerExpression(*Payload.Initializer);
        if (!LoweredInitializer)
        {
          if (!CurrentReachable)
          {
            return;
          }
          fail(Node.Header.Range, "binding initializer requires a value-producing expression");
        }
        const ir::IrValueId Initializer = *LoweredInitializer;
        if (Model.symbol(*Symbol).Mutable)
        {
          const ir::IrTypeId Type = mapValueType(Model.symbol(*Symbol).Type, Node.Header.Range);
          const ir::IrValueId Place = Builder.createAlloca(CurrentBlock, Type, ir::IrPlaceAccess::ReadWrite, origin(Node.Header.Range));
          Builder.createStore(CurrentBlock, Place, Initializer, origin(Node.Header.Range));
          Symbols[Symbol->value()].Place = Place;
        }
        else
        {
          Symbols[Symbol->value()].Value = Initializer;
        }
      }

      void lowerStatement(ast::AstStmtId Id)
      {
        if (!CurrentReachable)
        {
          return;
        }
        const ast::Statement &Node = Ast.statement(Id);
        switch (Node.Kind)
        {
        case ast::AstKind::BlockStatement:
        {
          for (ast::AstNodeRef Item : Ast.list(std::get<ast::BlockPayload>(Node.Payload).Items))
          {
            if (!CurrentReachable)
            {
              break;
            }
            if (Item.Category == ast::AstNodeCategory::Declaration)
            {
              lowerDeclaration(ast::AstDeclId::fromValue(Item.Index));
            }
            else if (Item.Category == ast::AstNodeCategory::Statement)
            {
              lowerStatement(ast::AstStmtId::fromValue(Item.Index));
            }
            else
            {
              fail(Node.Header.Range, "statement block contains an invalid item category");
            }
          }
          return;
        }
        case ast::AstKind::AssignmentStatement:
        {
          const ast::AssignmentPayload &Payload = std::get<ast::AssignmentPayload>(Node.Payload);
          const ir::IrValueId Place = lowerPlace(Payload.Left);
          ir::IrValueId Value = requireValue(lowerExpression(Payload.Right), Node.Header.Range, "assignment right-hand side");
          const std::string_view Operator = Strings.string(Payload.Operator);
          if (Operator != "=")
          {
            const ir::IrValueId Existing = Builder.createLoad(CurrentBlock, Place, origin(Node.Header.Range));
            if (Operator == "+=")
            {
              Value = Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntAdd, Existing, Value, origin(Node.Header.Range));
            }
            else if (Operator == "-=")
            {
              Value = Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntSub, Existing, Value, origin(Node.Header.Range));
            }
            else if (Operator == "*=")
            {
              Value = Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntMul, Existing, Value, origin(Node.Header.Range));
            }
            else if (Operator == "&=")
            {
              Value = Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntAnd, Existing, Value, origin(Node.Header.Range));
            }
            else if (Operator == "|=")
            {
              Value = Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntOr, Existing, Value, origin(Node.Header.Range));
            }
            else if (Operator == "^=")
            {
              Value = Builder.createIntegerBinary(CurrentBlock, ir::IrOpcode::IntXor, Existing, Value, origin(Node.Header.Range));
            }
            else
            {
              fail(Node.Header.Range, "compound assignment operator has no first-slice InkIR lowering");
            }
          }
          Builder.createStore(CurrentBlock, Place, Value, origin(Node.Header.Range));
          return;
        }
        case ast::AstKind::ExpressionStatement:
          lowerExpression(std::get<ast::ExpressionStatementPayload>(Node.Payload).Value);
          return;
        case ast::AstKind::IfStatement:
          lowerIfStatement(Node);
          return;
        case ast::AstKind::WhileStatement:
          lowerWhileStatement(Node);
          return;
        case ast::AstKind::ReturnStatement:
        {
          const ast::ReturnPayload &Payload = std::get<ast::ReturnPayload>(Node.Payload);
          std::optional<ir::IrValueId> Value;
          if (Payload.Value)
          {
            Value = lowerExpression(*Payload.Value);
          }
          if (CurrentReachable)
          {
            terminate(Builder.createReturn(CurrentBlock, Value, origin(Node.Header.Range)));
            CurrentReachable = false;
          }
          return;
        }
        case ast::AstKind::BreakStatement:
          if (Loops.empty())
          {
            fail(Node.Header.Range, "break reached InkIR lowering outside a loop");
          }
          terminate(Builder.createBranch(CurrentBlock, Loops.back().Break, {}, origin(Node.Header.Range)));
          CurrentReachable = false;
          return;
        case ast::AstKind::ContinueStatement:
          if (Loops.empty())
          {
            fail(Node.Header.Range, "continue reached InkIR lowering outside a loop");
          }
          terminate(Builder.createBranch(CurrentBlock, Loops.back().Continue, {}, origin(Node.Header.Range)));
          CurrentReachable = false;
          return;
        default:
          fail(Node.Header.Range, std::string("AST statement kind cannot be lowered to the first InkIR slice: ") + ast::astKindName(Node.Kind));
        }
      }

      ir::IrValueId lowerCondition(ast::AstNodeRef Condition, core::SourceRange Range)
      {
        if (Condition.Category != ast::AstNodeCategory::Expression)
        {
          fail(Range, "pattern conditions are not enabled in the first InkIR slice");
        }
        return requireValue(lowerExpression(ast::AstExprId::fromValue(Condition.Index)), Range, "control-flow condition");
      }

      void lowerIfStatement(const ast::Statement &Node)
      {
        const ast::IfStatementPayload &Payload = std::get<ast::IfStatementPayload>(Node.Payload);
        const ir::IrValueId Condition = lowerCondition(Payload.Condition, Node.Header.Range);
        const ir::IrBlockId ConditionBlock = CurrentBlock;
        const ir::IrBuiltBlock ThenBlock = addBlock({}, Ast.statement(Payload.ThenBlock).Header.Range);
        const ir::IrBuiltBlock ElseBlock = addBlock({}, Payload.ElseBlock ? Ast.statement(*Payload.ElseBlock).Header.Range : Node.Header.Range);
        setCurrent(ThenBlock.Block);
        lowerStatement(Payload.ThenBlock);
        const ir::IrBlockId ThenEnd = CurrentBlock;
        const bool ThenContinues = CurrentReachable && !isTerminated(ThenEnd);
        setCurrent(ElseBlock.Block);
        if (Payload.ElseBlock)
        {
          lowerStatement(*Payload.ElseBlock);
        }
        const ir::IrBlockId ElseEnd = CurrentBlock;
        const bool ElseContinues = CurrentReachable && !isTerminated(ElseEnd);
        setCurrent(ConditionBlock);
        terminate(Builder.createConditionalBranch(ConditionBlock, Condition, ThenBlock.Block, {}, ElseBlock.Block, {}, origin(Node.Header.Range)));
        if (!ThenContinues && !ElseContinues)
        {
          CurrentReachable = false;
          return;
        }
        const ir::IrBuiltBlock Merge = addBlock({}, Node.Header.Range);
        if (ThenContinues)
        {
          branchFrom(ThenEnd, Merge.Block, {}, Node.Header.Range);
        }
        if (ElseContinues)
        {
          branchFrom(ElseEnd, Merge.Block, {}, Node.Header.Range);
        }
        setCurrent(Merge.Block);
      }

      void lowerWhileStatement(const ast::Statement &Node)
      {
        const ast::WhileStatementPayload &Payload = std::get<ast::WhileStatementPayload>(Node.Payload);
        const ir::IrBlockId Preheader = CurrentBlock;
        const ir::IrBuiltBlock ConditionBlock = addBlock({}, Node.Header.Range);
        const ir::IrBuiltBlock BodyBlock = addBlock({}, Ast.statement(Payload.Body).Header.Range);
        const ir::IrBuiltBlock ExitBlock = addBlock({}, Node.Header.Range);
        branchFrom(Preheader, ConditionBlock.Block, {}, Node.Header.Range);
        setCurrent(ConditionBlock.Block);
        const ir::IrValueId Condition = lowerCondition(Payload.Condition, Node.Header.Range);
        terminate(Builder.createConditionalBranch(CurrentBlock, Condition, BodyBlock.Block, {}, ExitBlock.Block, {}, origin(Node.Header.Range)));
        Loops.push_back({ConditionBlock.Block, ExitBlock.Block});
        setCurrent(BodyBlock.Block);
        lowerStatement(Payload.Body);
        if (CurrentReachable && !isTerminated(CurrentBlock))
        {
          terminate(Builder.createBranch(CurrentBlock, ConditionBlock.Block, {}, origin(Node.Header.Range)));
        }
        Loops.pop_back();
        setCurrent(ExitBlock.Block);
      }
    };
  } // namespace

  bool IrGenerationResult::succeeded() const noexcept
  {
    return Module.has_value() && Errors.empty();
  }

  IrGenerator::IrGenerator(const sema::VerifiedSemanticModule &SemanticModule) noexcept : SemanticModule(SemanticModule)
  {
  }

  IrGenerationResult IrGenerator::generate() const
  {
    return GeneratorImpl(SemanticModule).generate();
  }

  IrGenerationResult generateIr(const sema::VerifiedSemanticModule &SemanticModule)
  {
    return IrGenerator(SemanticModule).generate();
  }
} // namespace ink::frontend
