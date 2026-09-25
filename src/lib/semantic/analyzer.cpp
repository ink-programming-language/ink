#include "ink/semantic/analyzer.h"

#include "ink/parser/parser.h"
#include "ink/semantic/model/context.h"

#include <charconv>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

namespace ink::semantic
{
  namespace
  {
    using core::DiagnosticKind;
    using core::SourceRange;
    using tokenizer::TokenKind;

    std::string typeName(const Type &ValueType)
    {
      if (IntegerType::classof(&ValueType))
      {
        const auto &Integer = static_cast<const IntegerType &>(ValueType);
        return std::string(Integer.isSigned() ? "int" : "uint") + std::to_string(Integer.bitWidth());
      }
      switch (ValueType.typeKind())
      {
      case TypeKind::Void:
        return "void";
      case TypeKind::Bool:
        return "bool";
      case TypeKind::Slice:
        return "string";
      case TypeKind::Function:
        return "function";
      default:
        return "value";
      }
    }

    bool isReservedName(std::string_view Name)
    {
      return Name == "true" || Name == "false";
    }

    struct Binding
    {
        const Value *Direct = nullptr;
        AllocaInstruction *Address = nullptr;
        bool Initialized = false;
    };

    struct FunctionDefinition
    {
        const parser::FunctionDecl *AST;
        Function *Model;
    };

    class DepthGuard
    {
      public:
        explicit DepthGuard(std::size_t &Depth)
            : Depth(Depth)
        {
          ++Depth;
        }

        ~DepthGuard()
        {
          --Depth;
        }

      private:
        std::size_t &Depth;
    };

    class Analyzer
    {
      public:
        Analyzer(SemanticContext &Context, const parser::ParsedUnit &Input)
            : Context(Context),
              Input(Input),
              StringType(*Context.getSliceType(*Context.getIntegerType(8, false), AccessKind::ReadOnly))
        {
        }

        Module *run(std::string_view ModuleName)
        {
          Result = Context.createModule(Context.namePool().intern(ModuleName));
          if (!Result)
          {
            error<DiagnosticKind::SemanticConstructionFailed>(Input.root()->getSourceRange());
            return nullptr;
          }
          const Type *PrintParameters[] = {&StringType};
          const Name PrintParameterNames[] = {Context.namePool().intern("text")};
          Function *Print = Context.createFunction(Context.namePool().intern("print"), *Context.getFunctionType(Context.getVoidType(), PrintParameters), {}, PrintParameterNames);
          if (!Print || !Context.appendValue(Result->entryBlock(), *Print))
          {
            error<DiagnosticKind::SemanticConstructionFailed>(Input.root()->getSourceRange());
            return nullptr;
          }
          Functions.emplace("print", Print);
          for (const parser::Stmt *Statement : Input.root()->statements())
          {
            const auto *Declaration = parser::dyn_cast<parser::DeclStmt>(Statement);
            const auto *FunctionAST = Declaration ? parser::dyn_cast<parser::FunctionDecl>(Declaration->declaration()) : nullptr;
            if (!FunctionAST)
            {
              unsupported(Statement->getSourceRange(), "module-level statements other than functions");
              return nullptr;
            }
            if (!declareFunction(*FunctionAST))
            {
              return nullptr;
            }
          }
          for (const FunctionDefinition &Definition : Definitions)
          {
            if (Definition.AST->body() && !analyzeBody(Definition))
            {
              return nullptr;
            }
          }
          return Result;
        }

      private:
        template <DiagnosticKind Kind, typename... Arguments>
        void error(SourceRange Range, Arguments &&... Values)
        {
          auto Diagnostic = core::makeDiagnostic<Kind>(Range, std::forward<Arguments>(Values)...);
          Diagnostic.Source = Input.input().lexedFile().sourceId();
          Context.compilationContext().diagnosticEngine().report(Diagnostic);
        }

        void unsupported(SourceRange Range, std::string_view Feature)
        {
          error<DiagnosticKind::SemanticUnsupported>(Range, Feature);
        }

        bool checkType(const Type &Actual, const Type &Expected, SourceRange Range)
        {
          if (&Actual == &Expected)
          {
            return true;
          }
          error<DiagnosticKind::SemanticTypeMismatch>(Range, typeName(Expected), typeName(Actual));
          return false;
        }

        const Type *resolveType(const parser::TypeSyntax &Syntax, bool AllowVoid = false)
        {
          const auto *Name = parser::dyn_cast<parser::NameExpr>(Syntax.expression());
          if (!Name)
          {
            unsupported(Syntax.getSourceRange(), "compound type expressions");
            return nullptr;
          }
          const std::string_view Text = Name->name().Text;
          if (Text == "void")
          {
            if (AllowVoid)
            {
              return &Context.getVoidType();
            }
            error<DiagnosticKind::SemanticTypeMismatch>(Syntax.getSourceRange(), "storable type", "void");
            return nullptr;
          }
          if (Text == "bool")
          {
            return &Context.getBoolType();
          }
          if (Text == "string")
          {
            return &StringType;
          }
          for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
          {
            if (Text == "int" + std::to_string(Width))
            {
              return Context.getIntegerType(Width, true);
            }
            if (Text == "uint" + std::to_string(Width))
            {
              return Context.getIntegerType(Width, false);
            }
          }
          error<DiagnosticKind::SemanticUnknownType>(Name->getSourceRange(), Text);
          return nullptr;
        }

        bool declareFunction(const parser::FunctionDecl &AST)
        {
          if (!AST.attributes().empty() || !AST.genericParameters().empty())
          {
            unsupported(AST.getSourceRange(), "function attributes or generics");
            return false;
          }
          if (Functions.contains(AST.name().Text) || isReservedName(AST.name().Text))
          {
            error<DiagnosticKind::SemanticDuplicateName>(AST.name().Range, AST.name().Text);
            return false;
          }
          const Type *ReturnType = resolveType(*AST.returnType(), true);
          if (!ReturnType)
          {
            return false;
          }
          std::vector<const Type *> ParameterTypes;
          std::vector<Name> ParameterNames;
          std::unordered_map<std::string_view, bool> SeenParameterNames;
          for (const parser::Parameter &Parameter : AST.parameters())
          {
            if (Parameter.variadic() || Parameter.defaultValue() || !Parameter.type())
            {
              unsupported(Parameter.range(), "variadic, defaulted or untyped parameters");
              return false;
            }
            if (isReservedName(Parameter.name().Text) || !SeenParameterNames.emplace(Parameter.name().Text, true).second)
            {
              error<DiagnosticKind::SemanticDuplicateName>(Parameter.name().Range, Parameter.name().Text);
              return false;
            }
            const Type *ParameterType = resolveType(*Parameter.type());
            if (!ParameterType)
            {
              return false;
            }
            ParameterTypes.push_back(ParameterType);
            ParameterNames.push_back(Context.namePool().intern(Parameter.name().Text));
          }
          Function *Model = Context.createFunction(Context.namePool().intern(AST.name().Text), *Context.getFunctionType(*ReturnType, ParameterTypes), {}, ParameterNames);
          if (!Model || !Context.appendValue(Result->entryBlock(), *Model))
          {
            error<DiagnosticKind::SemanticConstructionFailed>(AST.getSourceRange());
            return false;
          }
          Functions.emplace(AST.name().Text, Model);
          Definitions.push_back({&AST, Model});
          return true;
        }

        template <typename Instruction>
        Instruction *emit(Instruction *Node, SourceRange Range)
        {
          if (!Node || !Context.appendValue(*Block, *Node))
          {
            error<DiagnosticKind::SemanticConstructionFailed>(Range);
            return nullptr;
          }
          return Node;
        }

        bool analyzeBody(const FunctionDefinition &Definition)
        {
          CurrentFunction = Definition.Model;
          Block = Context.createFunctionBody(*CurrentFunction);
          Returned = false;
          Scopes.clear();
          Scopes.emplace_back();
          for (std::size_t Index = 0; Index < Definition.AST->parameters().size(); ++Index)
          {
            Scopes.back().emplace(Definition.AST->parameters()[Index].name().Text, Binding{CurrentFunction->parameters()[Index], nullptr, true});
          }
          if (!Block)
          {
            error<DiagnosticKind::SemanticConstructionFailed>(Definition.AST->getSourceRange());
            return false;
          }
          for (const parser::Stmt *Statement : Definition.AST->body()->statements())
          {
            if (!analyzeStatement(*Statement))
            {
              return false;
            }
          }
          if (!Returned)
          {
            if (CurrentFunction->type().returnType().typeKind() != TypeKind::Void)
            {
              error<DiagnosticKind::SemanticMissingReturn>(Definition.AST->body()->getSourceRange(), Definition.AST->name().Text);
              return false;
            }
            return emit(Context.createReturnInstruction(), Definition.AST->body()->getSourceRange()) != nullptr;
          }
          return true;
        }

        Binding *findBinding(std::string_view Name)
        {
          for (auto Scope = Scopes.rbegin(); Scope != Scopes.rend(); ++Scope)
          {
            const auto Found = Scope->find(Name);
            if (Found != Scope->end())
            {
              return &Found->second;
            }
          }
          return nullptr;
        }

        const Value *integerLiteral(const parser::LiteralExpr &Literal, const Type *Expected, bool Negative = false)
        {
          const IntegerType *Integer = Expected && IntegerType::classof(Expected) ? static_cast<const IntegerType *>(Expected) : Context.getIntegerType(32, true);
          const auto &Token = Input.input().token(Literal.token());
          const auto *Info = std::get_if<tokenizer::NumericInfo>(&Token.Payload);
          std::string_view Spelling = Input.input().spelling(Literal.token());
          const unsigned Base = Info ? Info->Base : 10;
          if (Base != 10)
          {
            Spelling.remove_prefix(2);
          }
          std::uint64_t Magnitude = 0;
          const auto Conversion = std::from_chars(Spelling.data(), Spelling.data() + Spelling.size(), Magnitude, static_cast<int>(Base));
          const std::uint32_t Width = Integer->bitWidth();
          const std::uint64_t Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
          const std::uint64_t Limit = Integer->isSigned() ? (std::uint64_t{1} << (Width - 1)) - (Negative ? 0 : 1) : Mask;
          if (Conversion.ec != std::errc{} || Conversion.ptr != Spelling.data() + Spelling.size() || Magnitude > Limit || (Negative && !Integer->isSigned()))
          {
            error<DiagnosticKind::SemanticIntegerOutOfRange>(Literal.getSourceRange(), typeName(*Integer));
            return nullptr;
          }
          const std::uint64_t Bits = Negative ? (std::uint64_t{0} - Magnitude) & Mask : Magnitude;
          return Context.getIntegerConstant(*Integer, IntegerBits(Width, Bits));
        }

        const Value *analyzeExpression(const parser::Expr &Expression, const Type *Expected = nullptr)
        {
          DepthGuard Guard(Depth);
          if (Depth > 256)
          {
            error<DiagnosticKind::SemanticNestingLimit>(Expression.getSourceRange());
            return nullptr;
          }
          const Value *ResultValue = expressionValue(Expression, Expected);
          if (ResultValue && Expected && !checkType(ResultValue->type(), *Expected, Expression.getSourceRange()))
          {
            return nullptr;
          }
          return ResultValue;
        }

        const Value *expressionValue(const parser::Expr &Expression, const Type *Expected)
        {
          const SourceRange Range = Expression.getSourceRange();
          if (const auto *Parentheses = parser::dyn_cast<parser::ParenExpr>(&Expression))
          {
            return analyzeExpression(*Parentheses->expression(), Expected);
          }
          if (const auto *Literal = parser::dyn_cast<parser::LiteralExpr>(&Expression))
          {
            if (Literal->literalKind() == TokenKind::IntegerLiteral)
            {
              return integerLiteral(*Literal, Expected);
            }
            if (Literal->literalKind() == TokenKind::StringLiteral)
            {
              const auto *Info = std::get_if<tokenizer::StringInfo>(&Input.input().token(Literal->token()).Payload);
              if (Info)
              {
                return Context.getStringConstant(StringType, Info->Decoded);
              }
              error<DiagnosticKind::SemanticConstructionFailed>(Range);
              return nullptr;
            }
            unsupported(Range, "non-integer, non-string literals");
            return nullptr;
          }
          if (const auto *Name = parser::dyn_cast<parser::NameExpr>(&Expression))
          {
            const std::string_view Text = Name->name().Text;
            if (isReservedName(Text))
            {
              return &Context.getBoolConstant(Text == "true");
            }
            if (Binding *Local = findBinding(Text))
            {
              if (!Local->Initialized)
              {
                error<DiagnosticKind::SemanticUninitializedRead>(Range, Text);
                return nullptr;
              }
              return Local->Address ? emit(Context.createLoadInstruction(*Local->Address), Range) : Local->Direct;
            }
            const auto Found = Functions.find(Text);
            if (Found != Functions.end())
            {
              return Found->second;
            }
            error<DiagnosticKind::SemanticUnknownName>(Range, Text);
            return nullptr;
          }
          if (const auto *Unary = parser::dyn_cast<parser::UnaryExpr>(&Expression))
          {
            const auto *Literal = parser::dyn_cast<parser::LiteralExpr>(Unary->operand());
            if ((Unary->op() == TokenKind::Minus || Unary->op() == TokenKind::Plus) && Literal && Literal->literalKind() == TokenKind::IntegerLiteral)
            {
              return integerLiteral(*Literal, Expected, Unary->op() == TokenKind::Minus);
            }
            unsupported(Range, "unary operators other than signs on integer literals");
            return nullptr;
          }
          if (const auto *Binary = parser::dyn_cast<parser::BinaryExpr>(&Expression))
          {
            if (Binary->op() != TokenKind::Plus)
            {
              unsupported(Binary->operatorRange(), "binary operators other than integer addition");
              return nullptr;
            }
            const Value *Left = analyzeExpression(*Binary->left(), Expected);
            if (!Left)
            {
              return nullptr;
            }
            if (!IntegerType::classof(&Left->type()))
            {
              error<DiagnosticKind::SemanticTypeMismatch>(Binary->left()->getSourceRange(), "integer", typeName(Left->type()));
              return nullptr;
            }
            const Value *Right = analyzeExpression(*Binary->right(), &Left->type());
            return Right ? emit(Context.createAddInstruction(*Left, *Right), Range) : nullptr;
          }
          if (const auto *Call = parser::dyn_cast<parser::CallExpr>(&Expression))
          {
            if (Call->optional())
            {
              unsupported(Range, "optional calls");
              return nullptr;
            }
            const Value *Callee = analyzeExpression(*Call->callee());
            if (!Callee)
            {
              return nullptr;
            }
            if (!FunctionType::classof(&Callee->type()))
            {
              error<DiagnosticKind::SemanticTypeMismatch>(Call->callee()->getSourceRange(), "function", typeName(Callee->type()));
              return nullptr;
            }
            const auto ParameterTypes = static_cast<const FunctionType &>(Callee->type()).parameterTypes();
            if (ParameterTypes.size() != Call->arguments().size())
            {
              error<DiagnosticKind::SemanticArgumentCount>(Range, ParameterTypes.size(), Call->arguments().size());
              return nullptr;
            }
            std::vector<const Value *> Arguments;
            for (const parser::Argument &Argument : Call->arguments())
            {
              if (Argument.form() != parser::ArgumentKind::Positional)
              {
                unsupported(Argument.range(), "named or spread call arguments");
                return nullptr;
              }
              const Value *ArgumentValue = analyzeExpression(*Argument.value(), ParameterTypes[Arguments.size()]);
              if (!ArgumentValue)
              {
                return nullptr;
              }
              Arguments.push_back(ArgumentValue);
            }
            return emit(Context.createCallInstruction(*Callee, Arguments), Range);
          }
          unsupported(Range, "this expression");
          return nullptr;
        }

        bool analyzeVariable(const parser::VarDecl &Declaration)
        {
          const auto *Name = parser::dyn_cast<parser::NameBindingPattern>(Declaration.binding());
          if (!Name || Declaration.constant() || !Declaration.attributes().empty())
          {
            unsupported(Declaration.getSourceRange(), "const declarations, destructuring or variable attributes");
            return false;
          }
          const std::string_view Text = Name->name().Text;
          if (isReservedName(Text) || Scopes.back().contains(Text))
          {
            error<DiagnosticKind::SemanticDuplicateName>(Name->getSourceRange(), Text);
            return false;
          }
          const Type *VariableType = Declaration.type() ? resolveType(*Declaration.type()) : nullptr;
          if (Declaration.type() && !VariableType)
          {
            return false;
          }
          const Value *InitialValue = nullptr;
          if (Declaration.initializer())
          {
            InitialValue = analyzeExpression(*Declaration.initializer(), VariableType);
            if (!InitialValue)
            {
              return false;
            }
            VariableType = &InitialValue->type();
          }
          if (!VariableType)
          {
            error<DiagnosticKind::SemanticMissingVariableType>(Declaration.getSourceRange());
            return false;
          }
          if (VariableType->typeKind() == TypeKind::Void || FunctionType::classof(VariableType))
          {
            error<DiagnosticKind::SemanticTypeMismatch>(Declaration.getSourceRange(), "storable type", typeName(*VariableType));
            return false;
          }
          AllocaInstruction *Address = emit(Context.createAllocaInstruction(*VariableType), Declaration.getSourceRange());
          if (!Address || (InitialValue && !emit(Context.createStoreInstruction(*Address, *InitialValue), Declaration.getSourceRange())))
          {
            return false;
          }
          Scopes.back().emplace(Text, Binding{nullptr, Address, InitialValue != nullptr});
          return true;
        }

        bool analyzeItem(const parser::SimpleItem &Item)
        {
          if (const auto *Expression = parser::dyn_cast<parser::ExprItem>(&Item))
          {
            return analyzeExpression(*Expression->expression()) != nullptr;
          }
          const auto *Assignment = parser::dyn_cast<parser::AssignmentItem>(&Item);
          if (!Assignment || Assignment->op() != TokenKind::Assign || !parser::isa<parser::ExprItem>(Assignment->right()))
          {
            unsupported(Item.getSourceRange(), "compound or chained assignments");
            return false;
          }
          const auto *Name = parser::dyn_cast<parser::NameExpr>(Assignment->left());
          Binding *Local = Name ? findBinding(Name->name().Text) : nullptr;
          if (Name && !Local && !Functions.contains(Name->name().Text) && !isReservedName(Name->name().Text))
          {
            error<DiagnosticKind::SemanticUnknownName>(Name->getSourceRange(), Name->name().Text);
            return false;
          }
          if (!Local || !Local->Address)
          {
            error<DiagnosticKind::SemanticInvalidAssignment>(Assignment->left()->getSourceRange());
            return false;
          }
          const auto *Right = parser::cast<parser::ExprItem>(Assignment->right());
          const Value *Assigned = analyzeExpression(*Right->expression(), &Local->Address->allocatedType());
          if (!Assigned || !emit(Context.createStoreInstruction(*Local->Address, *Assigned), Item.getSourceRange()))
          {
            return false;
          }
          Local->Initialized = true;
          return true;
        }

        bool analyzeStatement(const parser::Stmt &Statement)
        {
          DepthGuard Guard(Depth);
          const SourceRange Range = Statement.getSourceRange();
          if (Depth > 256)
          {
            error<DiagnosticKind::SemanticNestingLimit>(Range);
            return false;
          }
          if (Returned)
          {
            error<DiagnosticKind::SemanticUnreachableStatement>(Range);
            return false;
          }
          if (const auto *Declaration = parser::dyn_cast<parser::DeclStmt>(&Statement))
          {
            if (const auto *Variable = parser::dyn_cast<parser::VarDecl>(Declaration->declaration()))
            {
              return analyzeVariable(*Variable);
            }
          }
          if (const auto *Simple = parser::dyn_cast<parser::SimpleStmt>(&Statement))
          {
            for (const parser::SimpleItem *Item : Simple->items())
            {
              if (!analyzeItem(*Item))
              {
                return false;
              }
            }
            return true;
          }
          if (const auto *Return = parser::dyn_cast<parser::ReturnStmt>(&Statement))
          {
            const Type &ReturnType = CurrentFunction->type().returnType();
            const Value *ReturnedValue = Return->value() ? analyzeExpression(*Return->value(), &ReturnType) : nullptr;
            if (Return->value() && !ReturnedValue)
            {
              return false;
            }
            if (!Return->value() && !checkType(Context.getVoidType(), ReturnType, Range))
            {
              return false;
            }
            if (ReturnedValue && ReturnType.typeKind() == TypeKind::Void)
            {
              error<DiagnosticKind::SemanticTypeMismatch>(Range, "return without a value", "void expression");
              return false;
            }
            Returned = true;
            return emit(Context.createReturnInstruction(ReturnedValue), Range) != nullptr;
          }
          if (const auto *Nested = parser::dyn_cast<parser::BlockStmt>(&Statement))
          {
            Scopes.emplace_back();
            bool Valid = true;
            for (const parser::Stmt *Child : Nested->statements())
            {
              if (!analyzeStatement(*Child))
              {
                Valid = false;
                break;
              }
            }
            Scopes.pop_back();
            return Valid;
          }
          unsupported(Range, "this statement or local declaration");
          return false;
        }

        SemanticContext &Context;
        const parser::ParsedUnit &Input;
        const SliceType &StringType;
        Module *Result = nullptr;
        std::unordered_map<std::string_view, Function *> Functions;
        std::vector<FunctionDefinition> Definitions;
        std::vector<std::unordered_map<std::string_view, Binding>> Scopes;
        Function *CurrentFunction = nullptr;
        BasicBlock *Block = nullptr;
        bool Returned = false;
        std::size_t Depth = 0;
    };
  } // namespace

  Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName)
  {
    if (!Input.succeeded() || !Input.Unit->root())
    {
      return nullptr;
    }
    return Analyzer(Context, *Input.Unit).run(ModuleName);
  }
} // namespace ink::semantic
