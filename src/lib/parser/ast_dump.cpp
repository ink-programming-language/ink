#include "ink/parser/ast_walker.h"
#include "ink/parser/ast_visitor.h"
#include "ink/parser/parser.h"
#include <sstream>
#include <algorithm>
namespace ink::parser
{
  namespace
  {
    void printValue(std::ostringstream &Output, SourceRange Range)
    {
      Output << '[' << Range.getBegin().getByteOffset() << ", " << Range.getEnd().getByteOffset() << ')';
    }
    void printValue(std::ostringstream &Output, const NameToken &Name)
    {
      Output << Name.Text << '@';
      printValue(Output, Name.Range);
    }
    void printValue(std::ostringstream &Output, const RestBinding &Rest)
    {
      printValue(Output, Rest.Name);
      Output << " wildcard=" << Rest.Wildcard << " ellipsis=";
      printValue(Output, Rest.EllipsisRange);
    }
    void printValue(std::ostringstream &Output, TokenKind Kind)
    {
      Output << tokenizer::tokenKindName(Kind);
    }
    template <typename T>
    void printValue(std::ostringstream &Output, T *Node)
    {
      if (Node)
      {
        Output << astKindName(Node->getKind()) << '@';
        printValue(Output, Node->getSourceRange());
      }
      else
      {
        Output << "none";
      }
    }
    template <typename T>
    void printValue(std::ostringstream &Output, const std::optional<T> &Value)
    {
      if (Value)
      {
        printValue(Output, *Value);
      }
      else
      {
        Output << "none";
      }
    }
    template <typename T> requires std::is_integral_v<T>
    void printValue(std::ostringstream &Output, T Value)
    {
      Output << Value;
    }
    void printValue(std::ostringstream &Output, ArgumentKind Value)
    {
      switch (Value)
      {
      case ArgumentKind::Named:
        Output << "Named";
        return;
      case ArgumentKind::Positional:
        Output << "Positional";
        return;
      case ArgumentKind::SpreadPositional:
        Output << "SpreadPositional";
        return;
      }
    }
    void printValue(std::ostringstream &Output, AttributeSuffix Value)
    {
      switch (Value)
      {
      case AttributeSuffix::None:
        Output << "None";
        return;
      case AttributeSuffix::Arguments:
        Output << "Arguments";
        return;
      case AttributeSuffix::Value:
        Output << "Value";
        return;
      }
    }
    void printValue(std::ostringstream &Output, VarDeclForm Value)
    {
      switch (Value)
      {
      case VarDeclForm::Uninitialized:
        Output << "Uninitialized";
        return;
      case VarDeclForm::Initialized:
        Output << "Initialized";
        return;
      }
    }
    void printValue(std::ostringstream &Output, FieldTailKind Value)
    {
      switch (Value)
      {
      case FieldTailKind::None:
        Output << "None";
        return;
      case FieldTailKind::Typed:
        Output << "Typed";
        return;
      case FieldTailKind::InitializerOnly:
        Output << "InitializerOnly";
        return;
      case FieldTailKind::Payload:
        Output << "Payload";
        return;
      }
    }
    void printValue(std::ostringstream &Output, FunctionBodyKind Value)
    {
      switch (Value)
      {
      case FunctionBodyKind::DeclarationOnly:
        Output << "DeclarationOnly";
        return;
      case FunctionBodyKind::Definition:
        Output << "Definition";
        return;
      }
    }
    void printValue(std::ostringstream &Output, AggregateForm Value)
    {
      switch (Value)
      {
      case AggregateForm::Forward:
        Output << "Forward";
        return;
      case AggregateForm::Definition:
        Output << "Definition";
        return;
      }
    }
    void printValue(std::ostringstream &Output, const Parameter &Record);
    void printValue(std::ostringstream &Output, const FunctionTypeParameter &Record);
    void printValue(std::ostringstream &Output, const Argument &Record);
    void printValue(std::ostringstream &Output, const PathSegment &Record);
    void printValue(std::ostringstream &Output, const Attribute &Record);
    void printValue(std::ostringstream &Output, const BaseSpec &Record);
    void printValue(std::ostringstream &Output, const MatchArm &Record);
    void printValue(std::ostringstream &Output, const SwitchClause &Record);
    void printValue(std::ostringstream &Output, const ImportEntry &Record);
    template <typename T>
    void printValue(std::ostringstream &Output, ASTArray<T> Values)
    {
      Output << '[';
      bool First = true;
      for (const auto &Value : Values)
      {
        if (!First)
        {
          Output << ", ";
        }
        First = false;
        printValue(Output, Value);
      }
      Output << ']';
    }
    template <typename T>
    void printValue(std::ostringstream &Output, ConstNodeArray<T> Values)
    {
      Output << Values.size() << " children";
    }
    void printValue(std::ostringstream &Output, const Parameter &Record)
    {
      Output << '{';
      Output << " Name=";
      printValue(Output, Record.name());
      Output << " Type=";
      printValue(Output, Record.type());
      Output << " DefaultValue=";
      printValue(Output, Record.defaultValue());
      Output << " Variadic=";
      printValue(Output, Record.variadic());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const FunctionTypeParameter &Record)
    {
      Output << '{';
      Output << " Name=";
      printValue(Output, Record.name());
      Output << " Type=";
      printValue(Output, Record.type());
      Output << " Variadic=";
      printValue(Output, Record.variadic());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const Argument &Record)
    {
      Output << '{';
      Output << " Form=";
      printValue(Output, Record.form());
      Output << " Name=";
      printValue(Output, Record.name());
      Output << " Value=";
      printValue(Output, Record.value());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const PathSegment &Record)
    {
      Output << '{';
      Output << " Name=";
      printValue(Output, Record.name());
      Output << " HasGenericArguments=";
      printValue(Output, Record.hasGenericArguments());
      Output << " Arguments=";
      printValue(Output, Record.arguments());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const Attribute &Record)
    {
      Output << '{';
      Output << " Path=";
      printValue(Output, Record.path());
      Output << " Suffix=";
      printValue(Output, Record.suffix());
      Output << " Arguments=";
      printValue(Output, Record.arguments());
      Output << " Value=";
      printValue(Output, Record.value());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const BaseSpec &Record)
    {
      Output << '{';
      Output << " Implements=";
      printValue(Output, Record.implements());
      Output << " Type=";
      printValue(Output, Record.type());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const MatchArm &Record)
    {
      Output << '{';
      Output << " Pattern=";
      printValue(Output, Record.pattern());
      Output << " Guard=";
      printValue(Output, Record.guard());
      Output << " Value=";
      printValue(Output, Record.value());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const SwitchClause &Record)
    {
      Output << '{';
      Output << " Default=";
      printValue(Output, Record.isDefault());
      Output << " Value=";
      printValue(Output, Record.value());
      Output << " Statements=";
      printValue(Output, Record.statements());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    void printValue(std::ostringstream &Output, const ImportEntry &Record)
    {
      Output << '{';
      Output << " Path=";
      printValue(Output, Record.path());
      Output << " Alias=";
      printValue(Output, Record.alias());
      Output << " Range=";
      printValue(Output, Record.range());
      Output << " }";
    }
    class ASTDumpVisitor : public ConstASTVisitor<ASTDumpVisitor>
    {
      public:
        explicit ASTDumpVisitor(std::ostringstream &Output)
            : Output(Output)
        {
        }
        void visitModuleAST(const ModuleAST *Node)
        {
          Output << "ModuleAST ";
          printValue(Output, Node->getSourceRange());
        }
        void visitTypeSyntax(const TypeSyntax *Node)
        {
          Output << "TypeSyntax ";
          printValue(Output, Node->getSourceRange());
        }
        void visitMissingExpr(const MissingExpr *Node)
        {
          Output << "MissingExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=missing";
        }
        void visitErrorExpr(const ErrorExpr *Node)
        {
          Output << "ErrorExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=error";
        }
        void visitMissingStmt(const MissingStmt *Node)
        {
          Output << "MissingStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=missing";
        }
        void visitErrorStmt(const ErrorStmt *Node)
        {
          Output << "ErrorStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=error";
        }
        void visitMissingDecl(const MissingDecl *Node)
        {
          Output << "MissingDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=missing";
          Output << " Attributes=";
          printValue(Output, Node->attributes());
        }
        void visitErrorDecl(const ErrorDecl *Node)
        {
          Output << "ErrorDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=error";
          Output << " Attributes=";
          printValue(Output, Node->attributes());
        }
        void visitMissingBindingPattern(const MissingBindingPattern *Node)
        {
          Output << "MissingBindingPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=missing";
        }
        void visitErrorBindingPattern(const ErrorBindingPattern *Node)
        {
          Output << "ErrorBindingPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=error";
        }
        void visitMissingMatchPattern(const MissingMatchPattern *Node)
        {
          Output << "MissingMatchPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=missing";
        }
        void visitErrorMatchPattern(const ErrorMatchPattern *Node)
        {
          Output << "ErrorMatchPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " recovery=error";
        }
        void visitNameExpr(const NameExpr *Node)
        {
          Output << "NameExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Name=";
          printValue(Output, Node->name());
        }
        void visitLiteralExpr(const LiteralExpr *Node)
        {
          Output << "LiteralExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Token=";
          printValue(Output, Node->token());
          Output << " LiteralKind=";
          printValue(Output, Node->literalKind());
        }
        void visitUnaryExpr(const UnaryExpr *Node)
        {
          Output << "UnaryExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Op=";
          printValue(Output, Node->op());
          Output << " OperatorRange=";
          printValue(Output, Node->operatorRange());
        }
        void visitComptimeExpr(const ComptimeExpr *Node)
        {
          Output << "ComptimeExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitBinaryExpr(const BinaryExpr *Node)
        {
          Output << "BinaryExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Op=";
          printValue(Output, Node->op());
          Output << " OperatorRange=";
          printValue(Output, Node->operatorRange());
        }
        void visitConditionalExpr(const ConditionalExpr *Node)
        {
          Output << "ConditionalExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitParenExpr(const ParenExpr *Node)
        {
          Output << "ParenExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitTupleExpr(const TupleExpr *Node)
        {
          Output << "TupleExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitArrayExpr(const ArrayExpr *Node)
        {
          Output << "ArrayExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitArrayRepeatExpr(const ArrayRepeatExpr *Node)
        {
          Output << "ArrayRepeatExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitCallExpr(const CallExpr *Node)
        {
          Output << "CallExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Arguments=";
          printValue(Output, Node->arguments());
          Output << " Optional=";
          printValue(Output, Node->optional());
        }
        void visitIndexExpr(const IndexExpr *Node)
        {
          Output << "IndexExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Optional=";
          printValue(Output, Node->optional());
        }
        void visitMemberExpr(const MemberExpr *Node)
        {
          Output << "MemberExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Access=";
          printValue(Output, Node->access());
          Output << " Member=";
          printValue(Output, Node->member());
        }
        void visitGenericApplyExpr(const GenericApplyExpr *Node)
        {
          Output << "GenericApplyExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Arguments=";
          printValue(Output, Node->arguments());
        }
        void visitPostfixUpdateExpr(const PostfixUpdateExpr *Node)
        {
          Output << "PostfixUpdateExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Op=";
          printValue(Output, Node->op());
          Output << " OperatorRange=";
          printValue(Output, Node->operatorRange());
        }
        void visitFunctionTypeExpr(const FunctionTypeExpr *Node)
        {
          Output << "FunctionTypeExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Parameters=";
          printValue(Output, Node->parameters());
        }
        void visitLambdaExpr(const LambdaExpr *Node)
        {
          Output << "LambdaExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " GenericParameters=";
          printValue(Output, Node->genericParameters());
          Output << " Parameters=";
          printValue(Output, Node->parameters());
        }
        void visitMatchExpr(const MatchExpr *Node)
        {
          Output << "MatchExpr ";
          printValue(Output, Node->getSourceRange());
          Output << " Arms=";
          printValue(Output, Node->arms());
        }
        void visitBlockExpr(const BlockExpr *Node)
        {
          Output << "BlockExpr ";
          printValue(Output, Node->getSourceRange());
        }
        void visitExprItem(const ExprItem *Node)
        {
          Output << "ExprItem ";
          printValue(Output, Node->getSourceRange());
        }
        void visitAssignmentItem(const AssignmentItem *Node)
        {
          Output << "AssignmentItem ";
          printValue(Output, Node->getSourceRange());
          Output << " Op=";
          printValue(Output, Node->op());
          Output << " OperatorRange=";
          printValue(Output, Node->operatorRange());
        }
        void visitSimpleStmt(const SimpleStmt *Node)
        {
          Output << "SimpleStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitBlockStmt(const BlockStmt *Node)
        {
          Output << "BlockStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " Synthetic=";
          printValue(Output, Node->synthetic());
        }
        void visitDeclStmt(const DeclStmt *Node)
        {
          Output << "DeclStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitIfStmt(const IfStmt *Node)
        {
          Output << "IfStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitWhileStmt(const WhileStmt *Node)
        {
          Output << "WhileStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitClassicForStmt(const ClassicForStmt *Node)
        {
          Output << "ClassicForStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitForInStmt(const ForInStmt *Node)
        {
          Output << "ForInStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitSwitchStmt(const SwitchStmt *Node)
        {
          Output << "SwitchStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " Clauses=";
          printValue(Output, Node->clauses());
        }
        void visitReturnStmt(const ReturnStmt *Node)
        {
          Output << "ReturnStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitBreakStmt(const BreakStmt *Node)
        {
          Output << "BreakStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitContinueStmt(const ContinueStmt *Node)
        {
          Output << "ContinueStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitYieldStmt(const YieldStmt *Node)
        {
          Output << "YieldStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " Return=";
          printValue(Output, Node->isReturn());
        }
        void visitDeferStmt(const DeferStmt *Node)
        {
          Output << "DeferStmt ";
          printValue(Output, Node->getSourceRange());
        }
        void visitComptimeStmt(const ComptimeStmt *Node)
        {
          Output << "ComptimeStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " KeywordRange=";
          printValue(Output, Node->keywordRange());
        }
        void visitDirectImportStmt(const DirectImportStmt *Node)
        {
          Output << "DirectImportStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " Imports=";
          printValue(Output, Node->imports());
        }
        void visitFromImportStmt(const FromImportStmt *Node)
        {
          Output << "FromImportStmt ";
          printValue(Output, Node->getSourceRange());
          Output << " RelativeLevel=";
          printValue(Output, Node->relativeLevel());
          Output << " RelativeTokens=";
          printValue(Output, Node->relativeTokens());
          Output << " Path=";
          printValue(Output, Node->path());
          Output << " Imports=";
          printValue(Output, Node->imports());
        }
        void visitVarDecl(const VarDecl *Node)
        {
          Output << "VarDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " Attributes=";
          printValue(Output, Node->attributes());
          Output << " Constant=";
          printValue(Output, Node->constant());
          Output << " Form=";
          printValue(Output, Node->form());
        }
        void visitFieldDecl(const FieldDecl *Node)
        {
          Output << "FieldDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " Attributes=";
          printValue(Output, Node->attributes());
          Output << " Name=";
          printValue(Output, Node->name());
          Output << " Tail=";
          printValue(Output, Node->tail());
          Output << " Payload=";
          printValue(Output, Node->payload());
        }
        void visitFunctionDecl(const FunctionDecl *Node)
        {
          Output << "FunctionDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " Attributes=";
          printValue(Output, Node->attributes());
          Output << " Name=";
          printValue(Output, Node->name());
          Output << " GenericParameters=";
          printValue(Output, Node->genericParameters());
          Output << " Parameters=";
          printValue(Output, Node->parameters());
          Output << " BodyKind=";
          printValue(Output, Node->bodyKind());
        }
        void visitClassDecl(const ClassDecl *Node)
        {
          Output << "ClassDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " Attributes=";
          printValue(Output, Node->attributes());
          Output << " Name=";
          printValue(Output, Node->name());
          Output << " GenericParameters=";
          printValue(Output, Node->genericParameters());
          Output << " Bases=";
          printValue(Output, Node->bases());
          Output << " Form=";
          printValue(Output, Node->form());
          Output << " SemicolonRange=";
          printValue(Output, Node->semicolonRange());
        }
        void visitEnumDecl(const EnumDecl *Node)
        {
          Output << "EnumDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " Attributes=";
          printValue(Output, Node->attributes());
          Output << " Name=";
          printValue(Output, Node->name());
          Output << " GenericParameters=";
          printValue(Output, Node->genericParameters());
          Output << " Bases=";
          printValue(Output, Node->bases());
          Output << " Form=";
          printValue(Output, Node->form());
          Output << " SemicolonRange=";
          printValue(Output, Node->semicolonRange());
        }
        void visitInterfaceDecl(const InterfaceDecl *Node)
        {
          Output << "InterfaceDecl ";
          printValue(Output, Node->getSourceRange());
          Output << " Attributes=";
          printValue(Output, Node->attributes());
          Output << " Name=";
          printValue(Output, Node->name());
          Output << " GenericParameters=";
          printValue(Output, Node->genericParameters());
          Output << " Bases=";
          printValue(Output, Node->bases());
          Output << " Form=";
          printValue(Output, Node->form());
          Output << " SemicolonRange=";
          printValue(Output, Node->semicolonRange());
        }
        void visitWildcardBindingPattern(const WildcardBindingPattern *Node)
        {
          Output << "WildcardBindingPattern ";
          printValue(Output, Node->getSourceRange());
        }
        void visitNameBindingPattern(const NameBindingPattern *Node)
        {
          Output << "NameBindingPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " Name=";
          printValue(Output, Node->name());
        }
        void visitTupleBindingPattern(const TupleBindingPattern *Node)
        {
          Output << "TupleBindingPattern ";
          printValue(Output, Node->getSourceRange());
        }
        void visitArrayBindingPattern(const ArrayBindingPattern *Node)
        {
          Output << "ArrayBindingPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " Rest=";
          printValue(Output, Node->rest());
        }
        void visitWildcardMatchPattern(const WildcardMatchPattern *Node)
        {
          Output << "WildcardMatchPattern ";
          printValue(Output, Node->getSourceRange());
        }
        void visitNameMatchPattern(const NameMatchPattern *Node)
        {
          Output << "NameMatchPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " Path=";
          printValue(Output, Node->path());
          Output << " HasArguments=";
          printValue(Output, Node->hasArguments());
        }
        void visitTupleMatchPattern(const TupleMatchPattern *Node)
        {
          Output << "TupleMatchPattern ";
          printValue(Output, Node->getSourceRange());
        }
        void visitArrayMatchPattern(const ArrayMatchPattern *Node)
        {
          Output << "ArrayMatchPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " Rest=";
          printValue(Output, Node->rest());
        }
        void visitLiteralMatchPattern(const LiteralMatchPattern *Node)
        {
          Output << "LiteralMatchPattern ";
          printValue(Output, Node->getSourceRange());
          Output << " Negative=";
          printValue(Output, Node->negative());
        }
        void visitGroupedMatchPattern(const GroupedMatchPattern *Node)
        {
          Output << "GroupedMatchPattern ";
          printValue(Output, Node->getSourceRange());
        }
        void visitOrMatchPattern(const OrMatchPattern *Node)
        {
          Output << "OrMatchPattern ";
          printValue(Output, Node->getSourceRange());
        }

      private:
        std::ostringstream &Output;
    };
  } // namespace
  std::string dumpAST(const ASTNodeBase *Root)
  {
    std::ostringstream Output;
    ASTDumpVisitor Visitor(Output);
    std::size_t Depth = 0;
    ASTWalker{}.walk(Root, [&](const ASTNodeBase *Node)
                     {
                       Output << std::string(std::min<std::size_t>(Depth, 64) * 2, ' ');
                       Visitor.visit(Node);
                       Output << '\n';
                       ++Depth;
                       return WalkAction::Continue;
                     },
                     [&](const ASTNodeBase *)
                     {
                       --Depth;
                     });
    return Output.str();
  }
  std::string dumpAST(const ParsedUnit &Unit)
  {
    std::ostringstream Output;
    Output << dumpAST(Unit.root());
    for (const auto &Entry : Unit.recoveryInfo().Entries)
    {
      Output << "Recovery " << tokenizer::tokenKindName(Entry.Token.Expected) << ' ';
      printValue(Output, Entry.Token.Range);
      Output << " status=" << (Entry.Token.Status == ExpectStatus::Inserted ? "inserted" : "recovered");
      if (Entry.Skipped)
      {
        Output << " skipped=";
        printValue(Output, *Entry.Skipped);
      }
      Output << '\n';
    }
    return Output.str();
  }
} // namespace ink::parser
