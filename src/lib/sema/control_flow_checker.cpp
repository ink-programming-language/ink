#include "ink/sema/control_flow_checker.h"

#include "sema_internal.h"

#include <string>

namespace ink::sema
{
  namespace
  {
    class ControlFlowEngine
    {
    public:
      ControlFlowEngine(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) : Model(Model), Diagnostics(Diagnostics), Types(SemanticModelAccess::types(Model))
      {
      }

      void run()
      {
        for (const Symbol &Symbol : Model.symbols())
        {
          if (Symbol.Kind != SymbolKind::Function)
          {
            continue;
          }
          const ast::Declaration &Node = Model.astContext().declaration(Symbol.Declaration);
          const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Node.Payload);
          if (!Payload || !Payload->Body)
          {
            continue;
          }
          const bool Returns = checkStatement(*Payload->Body, 0);
          const type::TypeId Result = Model.declaredType(Symbol.Declaration);
          if (!Returns && Result != Types.errorType() && Result != Types.voidType())
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::MissingReturn, Node.Header.Range, typeName(Model, Result), std::string("fallthrough"));
          }
        }
      }

    private:
      bool checkStatement(ast::AstStmtId Id, std::size_t LoopDepth)
      {
        const ast::Statement &Node = Model.astContext().statement(Id);
        if (const ast::BlockPayload *Payload = std::get_if<ast::BlockPayload>(&Node.Payload))
        {
          bool Returns = false;
          for (const ast::AstNodeRef Ref : Model.astContext().list(Payload->Items))
          {
            bool ItemReturns = false;
            if (Ref.Category == ast::AstNodeCategory::Declaration)
            {
              const ast::Declaration &Declaration = Model.astContext().declaration(ast::AstDeclId::fromValue(Ref.Index));
              if (const ast::BindingPayload *Binding = std::get_if<ast::BindingPayload>(&Declaration.Payload); Declaration.Kind == ast::AstKind::BindingDeclaration && Binding && Binding->Initializer)
              {
                ItemReturns = Model.expressionType(*Binding->Initializer) == Types.neverType();
              }
            }
            else if (Ref.Category == ast::AstNodeCategory::Statement)
            {
              ItemReturns = checkStatement(ast::AstStmtId::fromValue(Ref.Index), LoopDepth);
            }
            Returns |= !Returns && ItemReturns;
          }
          return Returns;
        }
        if (const ast::IfStatementPayload *Payload = std::get_if<ast::IfStatementPayload>(&Node.Payload))
        {
          const bool ThenReturns = checkStatement(Payload->ThenBlock, LoopDepth);
          const bool ElseReturns = Payload->ElseBlock && checkStatement(*Payload->ElseBlock, LoopDepth);
          return ThenReturns && ElseReturns;
        }
        if (const ast::WhileStatementPayload *Payload = std::get_if<ast::WhileStatementPayload>(&Node.Payload))
        {
          checkStatement(Payload->Body, LoopDepth + 1);
          return false;
        }
        if (Node.Kind == ast::AstKind::ReturnStatement)
        {
          return true;
        }
        if (const ast::ExpressionStatementPayload *Payload = std::get_if<ast::ExpressionStatementPayload>(&Node.Payload))
        {
          return Model.expressionType(Payload->Value) == Types.neverType();
        }
        if (Node.Kind == ast::AstKind::BreakStatement || Node.Kind == ast::AstKind::ContinueStatement)
        {
          if (LoopDepth == 0)
          {
            emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::InvalidControlFlow, Node.Header.Range, std::string("enclosing loop"), Node.Kind == ast::AstKind::BreakStatement ? std::string("break") : std::string("continue"));
          }
          return false;
        }
        return false;
      }

      SemanticModel &Model;
      std::vector<core::Diagnostic> &Diagnostics;
      type::TypeContext &Types;
    };
  } // namespace

  ControlFlowChecker::ControlFlowChecker(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept : Model(Model), Diagnostics(Diagnostics)
  {
  }

  void ControlFlowChecker::run()
  {
    ControlFlowEngine(Model, Diagnostics).run();
  }
} // namespace ink::sema
