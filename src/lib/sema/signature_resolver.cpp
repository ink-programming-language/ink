#include "ink/sema/signature_resolver.h"

#include "sema_internal.h"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::sema
{
  namespace
  {
    class TypeSyntaxResolver
    {
    public:
      TypeSyntaxResolver(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) : Model(Model), Diagnostics(Diagnostics), States(Model.astFile().expressions().size(), 0)
      {
      }

      type::TypeId resolve(ast::AstExprId Id)
      {
        const std::size_t Index = static_cast<std::size_t>(Id.value() - Model.astFile().expressions().Begin.value());
        if (States[Index] == 2)
        {
          return Model.expressionType(Id);
        }
        if (States[Index] == 1)
        {
          return SemanticModelAccess::types(Model).errorType();
        }
        States[Index] = 1;
        const ast::Expression &Node = Model.astContext().expression(Id);
        type::TypeId Result = SemanticModelAccess::types(Model).errorType();
        if (Node.Kind == ast::AstKind::ErrorExpression)
        {
          Result = SemanticModelAccess::types(Model).errorType();
        }
        else if (Node.Kind == ast::AstKind::BuiltinTypeExpression || Node.Kind == ast::AstKind::TypeNameExpression)
        {
          Result = resolveName(Id, Node);
        }
        else if (Node.Kind == ast::AstKind::TypeGroupExpression)
        {
          const ast::GroupPayload *Payload = std::get_if<ast::GroupPayload>(&Node.Payload);
          Result = Payload ? resolve(Payload->Value) : SemanticModelAccess::types(Model).errorType();
        }
        else if (Node.Kind == ast::AstKind::FunctionTypeExpression)
        {
          reportUnsupported(Id, Node);
          Result = SemanticModelAccess::types(Model).errorType();
        }
        else if (Node.Kind == ast::AstKind::UnsupportedExpression)
        {
          reportUnsupported(Id, Node);
        }
        else
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnknownTypeName, Node.Header.Range, std::string("type syntax"), ast::astKindName(Node.Kind));
        }
        SemanticModelAccess::setTypeSyntax(Model, Id, Result);
        States[Index] = 2;
        return Result;
      }

    private:
      type::TypeId resolveName(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::NamePayload *Payload = std::get_if<ast::NamePayload>(&Node.Payload);
        if (!Payload || !Model.strings().contains(Payload->Name))
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnknownTypeName, Node.Header.Range);
          return SemanticModelAccess::types(Model).errorType();
        }
        const std::string_view Name = Model.strings().string(Payload->Name);
        if (Name == "i32")
        {
          return SemanticModelAccess::types(Model).i32Type();
        }
        if (Name == "i64")
        {
          return SemanticModelAccess::types(Model).i64Type();
        }
        if (Name == "u32")
        {
          return SemanticModelAccess::types(Model).u32Type();
        }
        if (Name == "u64")
        {
          return SemanticModelAccess::types(Model).u64Type();
        }
        if (Name == "bool")
        {
          return SemanticModelAccess::types(Model).boolType();
        }
        if (Name == "void")
        {
          return SemanticModelAccess::types(Model).voidType();
        }
        if (Name == "never")
        {
          return SemanticModelAccess::types(Model).neverType();
        }
        if (Name == "unit")
        {
          return SemanticModelAccess::types(Model).unitType();
        }
        if (Node.Kind == ast::AstKind::BuiltinTypeExpression)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("C0 builtin type"), std::string(Name));
        }
        else
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnknownTypeName, Node.Header.Range, std::nullopt, std::string(Name));
        }
        SemanticModelAccess::setResolvedName(Model, Id, {ResolvedNameStatus::Unresolved, {}});
        return SemanticModelAccess::types(Model).errorType();
      }

      void reportUnsupported(ast::AstExprId Id, const ast::Expression &Node)
      {
        const ast::AstNodeRef Ref = ast::AstNodeRef::expression(Id);
        if (!SemanticModelAccess::markUnsupported(Model, Ref))
        {
          return;
        }
        const ast::UnsupportedPayload *Payload = std::get_if<ast::UnsupportedPayload>(&Node.Payload);
        const std::string Feature = Payload ? ast::unsupportedFeatureName(Payload->Feature) : ast::astKindName(Node.Kind);
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::nullopt, Feature);
      }

      SemanticModel &Model;
      std::vector<core::Diagnostic> &Diagnostics;
      std::vector<unsigned char> States;
    };
  } // namespace

  SignatureResolver::SignatureResolver(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept : Model(Model), Diagnostics(Diagnostics)
  {
  }

  void SignatureResolver::run()
  {
    TypeSyntaxResolver Resolver(Model, Diagnostics);
    for (const Symbol &ReadOnlySymbol : Model.symbols())
    {
      Symbol &CurrentSymbol = SemanticModelAccess::symbol(Model, ReadOnlySymbol.Id);
      const ast::Declaration &Node = Model.astContext().declaration(CurrentSymbol.Declaration);
      if (CurrentSymbol.Kind == SymbolKind::Function)
      {
        const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Node.Payload);
        if (!Payload)
        {
          continue;
        }
        bool HasError = Payload->Flavor != ast::AstFunctionFlavor::Function;
        if (Payload->Flavor != ast::AstFunctionFlavor::Function)
        {
          emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, Node.Header.Range, std::string("ordinary function"), ast::astFunctionFlavorName(Payload->Flavor));
        }
        std::vector<type::TypeId> ParameterTypes;
        for (const ast::AstNodeRef Ref : Model.astContext().list(Payload->Parameters))
        {
          if (Ref.Category != ast::AstNodeCategory::Declaration)
          {
            HasError = true;
            continue;
          }
          const ast::AstDeclId ParameterId = ast::AstDeclId::fromValue(Ref.Index);
          const ast::Declaration &ParameterNode = Model.astContext().declaration(ParameterId);
          const ast::ParameterPayload *Parameter = std::get_if<ast::ParameterPayload>(&ParameterNode.Payload);
          if (!Parameter)
          {
            HasError = true;
            continue;
          }
          if (Parameter->Flavor != ast::AstParameterFlavor::Function || Parameter->IsPack)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, ParameterNode.Header.Range, std::string("ordinary function parameter"), Parameter->IsPack ? std::string("parameter pack") : ast::astParameterFlavorName(Parameter->Flavor));
            HasError = true;
          }
          if (Parameter->DefaultValue)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, ParameterNode.Header.Range, std::string("parameter without a default argument"), std::string("default argument"));
            HasError = true;
          }
          const type::TypeId ParameterType = Resolver.resolve(Parameter->Type);
          if (ParameterType == SemanticModelAccess::types(Model).voidType() || ParameterType == SemanticModelAccess::types(Model).neverType())
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, ParameterNode.Header.Range, std::string("first-slice value parameter type"), typeName(Model, ParameterType));
            HasError = true;
          }
          SemanticModelAccess::setDeclaredType(Model, ParameterId, ParameterType);
          if (const std::optional<SymbolId> ParameterSymbol = Model.declarationSymbol(ParameterId))
          {
            SemanticModelAccess::symbol(Model, *ParameterSymbol).Type = ParameterType;
          }
          ParameterTypes.push_back(ParameterType);
          HasError |= ParameterType == SemanticModelAccess::types(Model).errorType();
        }
        const type::TypeId ResultType = Payload->ResultType ? Resolver.resolve(*Payload->ResultType) : SemanticModelAccess::types(Model).voidType();
        SemanticModelAccess::setDeclaredType(Model, CurrentSymbol.Declaration, ResultType);
        HasError |= ResultType == SemanticModelAccess::types(Model).errorType();
        CurrentSymbol.Type = HasError ? SemanticModelAccess::types(Model).errorType() : SemanticModelAccess::types(Model).functionType(std::move(ParameterTypes), ResultType);
      }
      else if (CurrentSymbol.Kind == SymbolKind::Binding)
      {
        const ast::BindingPayload *Payload = std::get_if<ast::BindingPayload>(&Node.Payload);
        if (Payload && Payload->Type)
        {
          const type::TypeId Declared = Resolver.resolve(*Payload->Type);
          SemanticModelAccess::setDeclaredType(Model, CurrentSymbol.Declaration, Declared);
          CurrentSymbol.Type = Declared;
        }
      }
    }
  }
} // namespace ink::sema
