#include "ink/sema/type_checker.h"

#include "sema_internal.h"

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::sema
{
  namespace
  {
    int integerDigitValue(char Character) noexcept
    {
      if (Character >= '0' && Character <= '9')
      {
        return Character - '0';
      }
      if (Character >= 'a' && Character <= 'f')
      {
        return Character - 'a' + 10;
      }
      if (Character >= 'A' && Character <= 'F')
      {
        return Character - 'A' + 10;
      }
      return -1;
    }

    std::optional<std::int32_t> parseI32Literal(std::string_view Spelling)
    {
      unsigned Base = 10;
      std::size_t Position = 0;
      if (Spelling.size() >= 2 && Spelling[0] == '0')
      {
        if (Spelling[1] == 'b')
        {
          Base = 2;
          Position = 2;
        }
        else if (Spelling[1] == 'o')
        {
          Base = 8;
          Position = 2;
        }
        else if (Spelling[1] == 'x')
        {
          Base = 16;
          Position = 2;
        }
      }

      std::uint64_t Value = 0;
      bool SawDigit = false;
      bool PreviousWasDigit = false;
      while (Position < Spelling.size())
      {
        const char Character = Spelling[Position];
        if (Character == '_')
        {
          const int NextDigit = Position + 1 < Spelling.size() ? integerDigitValue(Spelling[Position + 1]) : -1;
          if (!PreviousWasDigit || NextDigit < 0 || static_cast<unsigned>(NextDigit) >= Base)
          {
            return std::nullopt;
          }
          PreviousWasDigit = false;
          ++Position;
          continue;
        }
        const int Digit = integerDigitValue(Character);
        if (Digit < 0 || static_cast<unsigned>(Digit) >= Base)
        {
          break;
        }
        const std::uint64_t Maximum = static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max());
        if (Value > (Maximum - static_cast<unsigned>(Digit)) / Base)
        {
          return std::nullopt;
        }
        Value = Value * Base + static_cast<unsigned>(Digit);
        SawDigit = true;
        PreviousWasDigit = true;
        ++Position;
      }
      if (!SawDigit || !PreviousWasDigit)
      {
        return std::nullopt;
      }
      const std::string_view Suffix = Spelling.substr(Position);
      if (!Suffix.empty() && Suffix != "i32")
      {
        return std::nullopt;
      }
      return static_cast<std::int32_t>(Value);
    }

    class TypeCheckingEngine
    {
    public:
      TypeCheckingEngine(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) : Model(Model), Diagnostics(Diagnostics), Types(SemanticModelAccess::types(Model)), DirectCalleeNames(Model.astFile().expressions().size(), false)
      {
        recordDirectCalleeNames();
      }

      void run()
      {
        const ast::Declaration &Root = Model.astContext().declaration(Model.astFile().root());
        const ast::SourceFilePayload *Source = std::get_if<ast::SourceFilePayload>(&Root.Payload);
        if (!Source)
        {
          return;
        }
        for (const ast::AstNodeRef Ref : Model.astContext().list(Source->Items))
        {
          if (Ref.Category == ast::AstNodeCategory::Declaration)
          {
            const ast::AstDeclId Id = ast::AstDeclId::fromValue(Ref.Index);
            if (Model.astContext().declaration(Id).Kind == ast::AstKind::BindingDeclaration)
            {
              checkBinding(Id);
              const ast::Declaration &Binding = Model.astContext().declaration(Id);
              if (SemanticModelAccess::markUnsupported(Model, ast::AstNodeRef::declaration(Id)))
              {
                emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Binding.Header.Range, std::nullopt, std::string("top-level binding"));
              }
            }
          }
        }
        for (const Symbol &Symbol : Model.symbols())
        {
          if (Symbol.Kind == SymbolKind::Function)
          {
            checkFunction(Symbol.Declaration);
          }
        }
        for (std::uint32_t Index = Model.astFile().expressions().Begin.value(); Index < Model.astFile().expressions().End.value(); ++Index)
        {
          const ast::AstExprId Id = ast::AstExprId::fromValue(Index);
          if (!SemanticModelAccess::expressionChecked(Model, Id) && Model.astContext().expression(Id).Kind == ast::AstKind::UnsupportedExpression)
          {
            checkExpression(Id);
          }
        }
      }

    private:
      struct ExpressionWork
      {
        ast::AstExprId Id;
        bool Finish = false;
      };

      std::size_t expressionIndex(ast::AstExprId Id) const noexcept
      {
        return static_cast<std::size_t>(Id.value() - Model.astFile().expressions().Begin.value());
      }

      void markDirectCallee(ast::AstExprId Id)
      {
        const ast::Expression &Node = Model.astContext().expression(Id);
        if (Node.Kind == ast::AstKind::GroupExpression)
        {
          markDirectCallee(std::get<ast::GroupPayload>(Node.Payload).Value);
        }
        else if (Node.Kind == ast::AstKind::NameExpression)
        {
          DirectCalleeNames[expressionIndex(Id)] = true;
        }
      }

      void recordDirectCalleeNames()
      {
        for (std::uint32_t Index = Model.astFile().expressions().Begin.value(); Index < Model.astFile().expressions().End.value(); ++Index)
        {
          const ast::Expression &Node = Model.astContext().expression(ast::AstExprId::fromValue(Index));
          if (const ast::CallPayload *Payload = std::get_if<ast::CallPayload>(&Node.Payload); Node.Kind == ast::AstKind::CallExpression && Payload)
          {
            markDirectCallee(Payload->Callee);
          }
        }
      }

      std::optional<SymbolId> directCalleeSymbol(ast::AstExprId Id) const
      {
        const ast::Expression &Node = Model.astContext().expression(Id);
        if (Node.Kind == ast::AstKind::GroupExpression)
        {
          return directCalleeSymbol(std::get<ast::GroupPayload>(Node.Payload).Value);
        }
        if (Node.Kind != ast::AstKind::NameExpression)
        {
          return std::nullopt;
        }
        const ResolvedName Resolution = Model.resolvedName(Id);
        return Resolution.Status == ResolvedNameStatus::Resolved ? std::optional<SymbolId>{Resolution.Symbol} : std::nullopt;
      }

      bool isError(type::TypeId Type) const noexcept
      {
        return Type == Types.errorType();
      }

      std::string operatorSpelling(core::InternedStringId Operator) const
      {
        return Model.strings().contains(Operator) ? std::string(Model.strings().string(Operator)) : std::string("<missing operator>");
      }

      void reportUnsupported(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::AstNodeRef Ref = ast::AstNodeRef::expression(Id);
        if (!SemanticModelAccess::markUnsupported(Model, Ref))
        {
          return;
        }
        std::string Feature = ast::astKindName(Node.Kind);
        if (const ast::UnsupportedPayload *UnsupportedPayload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
        {
          Feature = ast::unsupportedFeatureName(UnsupportedPayload->Feature);
        }
        else if (const ast::LiteralPayload *LiteralPayload = std::get_if<ast::LiteralPayload>(&Node.Payload))
        {
          Feature = ast::astLiteralKindName(LiteralPayload->Kind);
        }
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::nullopt, std::move(Feature));
      }

      void reportUnsupportedStatement(ast::AstStmtId Id, const ast::Statement &Node)
      {
        const ast::AstNodeRef Ref = ast::AstNodeRef::statement(Id);
        if (!SemanticModelAccess::markUnsupported(Model, Ref))
        {
          return;
        }
        std::string Feature = ast::astKindName(Node.Kind);
        if (const ast::UnsupportedPayload *Payload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
        {
          Feature = ast::unsupportedFeatureName(Payload->Feature);
        }
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::nullopt, std::move(Feature));
      }

      std::vector<ast::AstExprId> expressionChildren(ast::AstExprId Id) const
      {
        std::vector<ast::AstExprId> Result;
        for (const ast::AstNodeRef Ref : directChildren(Model, ast::AstNodeRef::expression(Id)))
        {
          if (Ref.Category == ast::AstNodeCategory::Expression)
          {
            Result.push_back(ast::AstExprId::fromValue(Ref.Index));
          }
        }
        return Result;
      }

      type::TypeId checkExpression(ast::AstExprId Id)
      {
        if (SemanticModelAccess::expressionChecked(Model, Id))
        {
          return Model.expressionType(Id);
        }
        std::vector<ExpressionWork> Work;
        Work.push_back({Id, false});
        while (!Work.empty())
        {
          const ExpressionWork Current = Work.back();
          Work.pop_back();
          if (SemanticModelAccess::expressionChecked(Model, Current.Id))
          {
            continue;
          }
          if (!Current.Finish)
          {
            Work.push_back({Current.Id, true});
            const std::vector<ast::AstExprId> Children = expressionChildren(Current.Id);
            for (auto Iterator = Children.rbegin(); Iterator != Children.rend(); ++Iterator)
            {
              if (!SemanticModelAccess::expressionChecked(Model, *Iterator))
              {
                Work.push_back({*Iterator, false});
              }
            }
            continue;
          }
          evaluateExpression(Current.Id);
        }
        return Model.expressionType(Id);
      }

      void evaluateExpression(ast::AstExprId Id)
      {
        const ast::Expression &Node = Model.astContext().expression(Id);
        switch (Node.Kind)
        {
        case ast::AstKind::ErrorExpression:
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        case ast::AstKind::LiteralExpression:
          evaluateLiteral(Id, Node);
          return;
        case ast::AstKind::NameExpression:
          evaluateName(Id);
          return;
        case ast::AstKind::GroupExpression:
          evaluateGroup(Id, Node);
          return;
        case ast::AstKind::UnaryExpression:
          evaluateUnary(Id, Node);
          return;
        case ast::AstKind::ComptimeExpression:
          evaluateComptime(Id, Node);
          return;
        case ast::AstKind::BinaryExpression:
          evaluateBinary(Id, Node);
          return;
        case ast::AstKind::CallExpression:
          evaluateCall(Id, Node);
          return;
        case ast::AstKind::IfExpression:
          evaluateIfExpression(Id, Node);
          return;
        case ast::AstKind::UnsupportedExpression:
          reportUnsupported(Id, Node);
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        case ast::AstKind::TypeNameExpression:
        case ast::AstKind::BuiltinTypeExpression:
        case ast::AstKind::TypeGroupExpression:
        case ast::AstKind::FunctionTypeExpression:
        case ast::AstKind::ThisExpression:
          reportUnsupported(Id, Node);
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        default:
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
      }

      void evaluateLiteral(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::LiteralPayload *Payload = std::get_if<ast::LiteralPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        if (Payload->Kind == ast::AstLiteralKind::Bool)
        {
          if (Model.strings().contains(Payload->Spelling))
          {
            const std::string_view Spelling = Model.strings().string(Payload->Spelling);
            if (Spelling == "true" || Spelling == "false")
            {
              SemanticModelAccess::setExpressionType(Model, Id, Types.boolType(), ExpressionCategory::Value);
              SemanticModelAccess::setConstantValue(Model, Id, ConstantValue{Spelling == "true"});
              return;
            }
          }
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("boolean literal"), std::string("invalid boolean spelling"));
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        if (Payload->Kind == ast::AstLiteralKind::Integer)
        {
          if (Model.strings().contains(Payload->Spelling))
          {
            const std::string_view Spelling = Model.strings().string(Payload->Spelling);
            if (const std::optional<std::int32_t> Value = parseI32Literal(Spelling))
            {
              SemanticModelAccess::setExpressionType(Model, Id, Types.i32Type(), ExpressionCategory::Value);
              SemanticModelAccess::setConstantValue(Model, Id, ConstantValue{*Value});
              return;
            }
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("representable i32 integer literal"), std::string(Spelling));
          }
          else
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("representable i32 integer literal"), std::string("invalid spelling identity"));
          }
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        reportUnsupported(Id, Node);
        SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
      }

      void evaluateName(ast::AstExprId Id)
      {
        const ResolvedName Resolution = Model.resolvedName(Id);
        if (Resolution.Status != ResolvedNameStatus::Resolved || !Model.contains(Resolution.Symbol))
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const Symbol &Symbol = Model.symbol(Resolution.Symbol);
        if (Symbol.Kind == SymbolKind::Function && !DirectCalleeNames[expressionIndex(Id)])
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Model.astContext().expression(Id).Header.Range, std::string("direct function call"), std::string("function value"));
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const ExpressionCategory Category = Symbol.Kind == SymbolKind::Function ? ExpressionCategory::Value : ExpressionCategory::Place;
        SemanticModelAccess::setExpressionType(Model, Id, Symbol.Type, Category, Category == ExpressionCategory::Place && Symbol.Mutable);
      }

      void evaluateGroup(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::GroupPayload *Payload = std::get_if<ast::GroupPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        SemanticModelAccess::setExpressionType(Model, Id, Model.expressionType(Payload->Value), Model.expressionCategory(Payload->Value), Model.expressionWritable(Payload->Value));
        SemanticModelAccess::setConstantValue(Model, Id, Model.constantValue(Payload->Value));
      }

      void evaluateUnary(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::UnaryPayload *Payload = std::get_if<ast::UnaryPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeId Operand = Model.expressionType(Payload->Operand);
        if (isError(Operand))
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const std::string Operator = operatorSpelling(Payload->Operator);
        if ((Operator == "+" || Operator == "-" || Operator == "~") && Operand == Types.i32Type())
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.i32Type(), ExpressionCategory::Value);
          return;
        }
        if (Operator == "!" && Operand == Types.boolType())
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.boolType(), ExpressionCategory::Value);
          return;
        }
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidUnaryOperator, Node.Header.Range, typeName(Model, Operand), Operator);
        SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
      }

      void evaluateComptime(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::UnaryPayload *Payload = std::get_if<ast::UnaryPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeId Operand = Model.expressionType(Payload->Operand);
        if (isError(Operand))
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeKind Kind = Types.type(Operand).Kind;
        if (Kind != type::TypeKind::Bool && Kind != type::TypeKind::I32 && Kind != type::TypeKind::I64 && Kind != type::TypeKind::U32 && Kind != type::TypeKind::U64)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("force-value representable comptime type"), typeName(Model, Operand));
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        SemanticModelAccess::setExpressionType(Model, Id, Operand, ExpressionCategory::Value);
        SemanticModelAccess::setConstantValue(Model, Id, Model.constantValue(Payload->Operand));
      }

      void evaluateBinary(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::BinaryPayload *Payload = std::get_if<ast::BinaryPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const std::string Operator = operatorSpelling(Payload->Operator);
        if (Operator == "/" || Operator == "%")
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("integer division and remainder with TargetContext PDB rules"), Operator);
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeId Left = Model.expressionType(Payload->Left);
        const type::TypeId Right = Model.expressionType(Payload->Right);
        if (isError(Left) || isError(Right))
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const bool Arithmetic = Operator == "+" || Operator == "-" || Operator == "*" || Operator == "&" || Operator == "|" || Operator == "^";
        const bool OrderedComparison = Operator == "<" || Operator == "<=" || Operator == ">" || Operator == ">=";
        const bool Equality = Operator == "==" || Operator == "!=";
        const bool Logical = Operator == "&&" || Operator == "||";
        if (Arithmetic && Left == Types.i32Type() && Right == Types.i32Type())
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.i32Type(), ExpressionCategory::Value);
          return;
        }
        if (OrderedComparison && Left == Types.i32Type() && Right == Types.i32Type())
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.boolType(), ExpressionCategory::Value);
          return;
        }
        if (Equality && Left == Right && (Left == Types.i32Type() || Left == Types.boolType()))
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.boolType(), ExpressionCategory::Value);
          return;
        }
        if (Logical && Left == Types.boolType() && Right == Types.boolType())
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.boolType(), ExpressionCategory::Value);
          return;
        }
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidBinaryOperator, Node.Header.Range, typeName(Model, Left) + " and " + typeName(Model, Right), Operator);
        SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
      }

      void evaluateCall(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::CallPayload *Payload = std::get_if<ast::CallPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeId Callee = Model.expressionType(Payload->Callee);
        if (isError(Callee))
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        if (Model.typeContext().type(Callee).Kind != type::TypeKind::Function)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::NotCallable, Node.Header.Range, std::string("function"), typeName(Model, Callee));
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const std::optional<SymbolId> CalleeSymbol = directCalleeSymbol(Payload->Callee);
        if (!CalleeSymbol || !Model.contains(*CalleeSymbol) || Model.symbol(*CalleeSymbol).Kind != SymbolKind::Function)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("direct function call"), std::string("indirect call"));
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::FunctionType &Function = Model.typeContext().function(Callee);
        const ast::AstNodeListView Arguments = Model.astContext().list(Payload->Arguments);
        if (Arguments.size() != Function.Parameters.size())
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::ArgumentCountMismatch, Node.Header.Range, std::to_string(Function.Parameters.size()), std::to_string(Arguments.size()));
        }
        const std::size_t Count = Arguments.size() < Function.Parameters.size() ? Arguments.size() : Function.Parameters.size();
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          if (Arguments[Index].Category != ast::AstNodeCategory::Expression)
          {
            continue;
          }
          const type::TypeId Actual = Model.expressionType(ast::AstExprId::fromValue(Arguments[Index].Index));
          if (!isError(Actual) && Actual != Function.Parameters[Index])
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::TypeMismatch, nodeRange(Model, Arguments[Index]), typeName(Model, Function.Parameters[Index]), typeName(Model, Actual));
          }
        }
        SemanticModelAccess::setExpressionType(Model, Id, Function.Result, ExpressionCategory::Value);
      }

      void evaluateIfExpression(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::IfExpressionPayload *Payload = std::get_if<ast::IfExpressionPayload>(&Node.Payload);
        if (!Payload)
        {
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeId Condition = Model.expressionType(Payload->Condition);
        const type::TypeId Then = Model.expressionType(Payload->ThenValue);
        const type::TypeId Else = Model.expressionType(Payload->ElseValue);
        if (!isError(Condition) && Condition != Types.boolType())
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidConditionType, Model.astContext().expression(Payload->Condition).Header.Range, typeName(Model, Types.boolType()), typeName(Model, Condition));
        }
        if (!isError(Then) && !isError(Else) && Then != Else)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::TypeMismatch, Node.Header.Range, typeName(Model, Then), typeName(Model, Else));
          SemanticModelAccess::setExpressionType(Model, Id, Types.errorType(), ExpressionCategory::Error);
          return;
        }
        const type::TypeId Result = isError(Then) ? Else : Then;
        SemanticModelAccess::setExpressionType(Model, Id, Result, isError(Result) ? ExpressionCategory::Error : ExpressionCategory::Value);
      }

      void checkBinding(ast::AstDeclId Id)
      {
        const ast::Declaration &Node = Model.astContext().declaration(Id);
        const ast::BindingPayload *Payload = std::get_if<ast::BindingPayload>(&Node.Payload);
        if (!Payload)
        {
          return;
        }
        const type::TypeId Declared = Model.declaredType(Id);
        const bool HasDeclaredType = Payload->Type.has_value();
        type::TypeId Initializer = Types.errorType();
        if (!Payload->TopLevel && !Payload->Initializer)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("initialized local binding"), std::string("binding without initializer"));
        }
        if (Payload->Initializer)
        {
          Initializer = checkExpression(*Payload->Initializer);
        }
        if (HasDeclaredType && !isError(Declared) && !isError(Initializer) && Declared != Initializer)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::TypeMismatch, Node.Header.Range, typeName(Model, Declared), typeName(Model, Initializer));
        }
        const type::TypeId BindingType = HasDeclaredType ? Declared : Initializer;
        if (!isError(BindingType) && (Types.type(BindingType).Kind == type::TypeKind::Void || Types.type(BindingType).Kind == type::TypeKind::Function))
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("first-slice binding value type"), typeName(Model, BindingType));
        }
        SemanticModelAccess::setDeclaredType(Model, Id, BindingType);
        if (const std::optional<SymbolId> Symbol = Model.declarationSymbol(Id))
        {
          SemanticModelAccess::symbol(Model, *Symbol).Type = BindingType;
        }
      }

      void checkFunction(ast::AstDeclId Id)
      {
        const ast::Declaration &Node = Model.astContext().declaration(Id);
        const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Node.Payload);
        if (!Payload)
        {
          return;
        }
        for (const ast::AstNodeRef Ref : Model.astContext().list(Payload->Parameters))
        {
          if (Ref.Category != ast::AstNodeCategory::Declaration)
          {
            continue;
          }
          const ast::AstDeclId ParameterId = ast::AstDeclId::fromValue(Ref.Index);
          const ast::ParameterPayload *Parameter = std::get_if<ast::ParameterPayload>(&Model.astContext().declaration(ParameterId).Payload);
          if (Parameter && Parameter->DefaultValue)
          {
            const type::TypeId Actual = checkExpression(*Parameter->DefaultValue);
            const type::TypeId Expected = Model.declaredType(ParameterId);
            if (!isError(Actual) && !isError(Expected) && Actual != Expected)
            {
              emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::TypeMismatch, Model.astContext().expression(*Parameter->DefaultValue).Header.Range, typeName(Model, Expected), typeName(Model, Actual));
            }
          }
        }
        if (Payload->Body)
        {
          checkBlock(*Payload->Body, Model.declaredType(Id));
        }
      }

      void checkBlock(ast::AstStmtId Id, type::TypeId FunctionResult)
      {
        const ast::Statement &Node = Model.astContext().statement(Id);
        const ast::BlockPayload *Payload = std::get_if<ast::BlockPayload>(&Node.Payload);
        if (!Payload)
        {
          checkStatement(Id, FunctionResult);
          return;
        }
        for (const ast::AstNodeRef Ref : Model.astContext().list(Payload->Items))
        {
          if (Ref.Category == ast::AstNodeCategory::Declaration)
          {
            const ast::AstDeclId Declaration = ast::AstDeclId::fromValue(Ref.Index);
            if (Model.astContext().declaration(Declaration).Kind == ast::AstKind::BindingDeclaration)
            {
              checkBinding(Declaration);
            }
          }
          else if (Ref.Category == ast::AstNodeCategory::Statement)
          {
            checkStatement(ast::AstStmtId::fromValue(Ref.Index), FunctionResult);
          }
          else if (Ref.Category == ast::AstNodeCategory::Expression)
          {
            checkExpression(ast::AstExprId::fromValue(Ref.Index));
          }
        }
      }

      void checkStatement(ast::AstStmtId Id, type::TypeId FunctionResult)
      {
        const ast::Statement &Node = Model.astContext().statement(Id);
        if (const ast::BlockPayload *BlockPayload = std::get_if<ast::BlockPayload>(&Node.Payload))
        {
          static_cast<void>(BlockPayload);
          checkBlock(Id, FunctionResult);
        }
        else if (const ast::AssignmentPayload *AssignmentPayload = std::get_if<ast::AssignmentPayload>(&Node.Payload))
        {
          const type::TypeId Left = checkExpression(AssignmentPayload->Left);
          const type::TypeId Right = checkExpression(AssignmentPayload->Right);
          if (Model.expressionCategory(AssignmentPayload->Left) != ExpressionCategory::Place)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidAssignmentTarget, Model.astContext().expression(AssignmentPayload->Left).Header.Range);
          }
          else if (!Model.expressionWritable(AssignmentPayload->Left))
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::AssignmentToImmutable, Model.astContext().expression(AssignmentPayload->Left).Header.Range);
          }
          if (!isError(Left) && !isError(Right) && Left != Right)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::TypeMismatch, Node.Header.Range, typeName(Model, Left), typeName(Model, Right));
          }
          const std::string Operator = operatorSpelling(AssignmentPayload->Operator);
          const bool TargetDependentCompound = Operator == "/=" || Operator == "%=";
          const bool SupportedCompound = Operator == "+=" || Operator == "-=" || Operator == "*=" || Operator == "&=" || Operator == "|=" || Operator == "^=";
          if (TargetDependentCompound)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("integer compound assignment with TargetContext PDB rules"), Operator);
          }
          else if (Operator != "=" && !isError(Left) && (!SupportedCompound || Left != Types.i32Type() || Right != Types.i32Type()))
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidBinaryOperator, Node.Header.Range, std::string("i32 operands"), Operator);
          }
        }
        else if (const ast::ExpressionStatementPayload *ExpressionPayload = std::get_if<ast::ExpressionStatementPayload>(&Node.Payload))
        {
          checkExpression(ExpressionPayload->Value);
        }
        else if (const ast::IfStatementPayload *IfPayload = std::get_if<ast::IfStatementPayload>(&Node.Payload))
        {
          checkCondition(IfPayload->Condition);
          checkBlock(IfPayload->ThenBlock, FunctionResult);
          if (IfPayload->ElseBlock)
          {
            checkBlock(*IfPayload->ElseBlock, FunctionResult);
          }
        }
        else if (const ast::WhileStatementPayload *WhilePayload = std::get_if<ast::WhileStatementPayload>(&Node.Payload))
        {
          checkCondition(WhilePayload->Condition);
          checkBlock(WhilePayload->Body, FunctionResult);
        }
        else if (const ast::ReturnPayload *ReturnPayload = std::get_if<ast::ReturnPayload>(&Node.Payload))
        {
          const type::TypeId Actual = ReturnPayload->Value ? checkExpression(*ReturnPayload->Value) : Types.voidType();
          if (!isError(FunctionResult) && !isError(Actual) && FunctionResult != Actual)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::ReturnTypeMismatch, Node.Header.Range, typeName(Model, FunctionResult), typeName(Model, Actual));
          }
        }
        else if (Node.Kind == ast::AstKind::UnsupportedStatement)
        {
          reportUnsupportedStatement(Id, Node);
        }
      }

      void checkCondition(ast::AstNodeRef Ref)
      {
        if (Ref.Category != ast::AstNodeCategory::Expression)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidConditionType, nodeRange(Model, Ref), std::string("bool expression"), ast::astNodeCategoryName(Ref.Category));
          return;
        }
        const ast::AstExprId Id = ast::AstExprId::fromValue(Ref.Index);
        const type::TypeId Condition = checkExpression(Id);
        if (!isError(Condition) && Condition != Types.boolType())
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidConditionType, Model.astContext().expression(Id).Header.Range, typeName(Model, Types.boolType()), typeName(Model, Condition));
        }
      }

      SemanticModel &Model;
      std::vector<core::Diagnostic> &Diagnostics;
      type::TypeContext &Types;
      std::vector<bool> DirectCalleeNames;
    };
  } // namespace

  TypeChecker::TypeChecker(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept : Model(Model), Diagnostics(Diagnostics)
  {
  }

  void TypeChecker::run()
  {
    TypeCheckingEngine(Model, Diagnostics).run();
  }
} // namespace ink::sema
