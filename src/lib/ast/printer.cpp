#include "ink/ast/printer.h"

#include <cstddef>
#include <cstdint>
#include <locale>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ast
{
  namespace
  {
    class OutputFormatGuard
    {
    public:
      explicit OutputFormatGuard(std::ostream &Output) : Output(Output), Flags(Output.flags()), Precision(Output.precision()), Fill(Output.fill()), Locale(Output.getloc())
      {
        Output.imbue(std::locale::classic());
        Output.setf(std::ios::dec, std::ios::basefield);
        Output.unsetf(std::ios::showbase | std::ios::showpos | std::ios::uppercase);
      }

      ~OutputFormatGuard() noexcept
      {
        Output.flags(Flags);
        Output.precision(Precision);
        Output.fill(Fill);
        try
        {
          Output.imbue(Locale);
        }
        catch (...)
        {
        }
      }

    private:
      std::ostream &Output;
      std::ios::fmtflags Flags;
      std::streamsize Precision;
      char Fill;
      std::locale Locale;
    };

    void indent(std::ostream &Output, std::size_t Depth)
    {
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Output << "  ";
      }
    }

    void printNodeRef(std::ostream &Output, const AstFile &File, AstNodeRef Ref)
    {
      Output << astNodeCategoryName(Ref.Category) << '#';
      switch (Ref.Category)
      {
      case AstNodeCategory::Declaration:
        if (File.declarations().contains(AstDeclId::fromValue(Ref.Index)))
        {
          Output << Ref.Index - File.declarations().Begin.value();
          return;
        }
        break;
      case AstNodeCategory::Expression:
        if (File.expressions().contains(AstExprId::fromValue(Ref.Index)))
        {
          Output << Ref.Index - File.expressions().Begin.value();
          return;
        }
        break;
      case AstNodeCategory::Statement:
        if (File.statements().contains(AstStmtId::fromValue(Ref.Index)))
        {
          Output << Ref.Index - File.statements().Begin.value();
          return;
        }
        break;
      case AstNodeCategory::Pattern:
        if (File.patterns().contains(AstPatternId::fromValue(Ref.Index)))
        {
          Output << Ref.Index - File.patterns().Begin.value();
          return;
        }
        break;
      case AstNodeCategory::Unknown:
        break;
      }
      Output << "foreign";
    }

    void printEscaped(std::ostream &Output, std::string_view Value)
    {
      constexpr char HexDigits[] = "0123456789ABCDEF";
      Output << '"';
      bool PreviousHexEscape = false;
      for (const unsigned char Byte : Value)
      {
        switch (Byte)
        {
        case '\\':
          Output << "\\\\";
          PreviousHexEscape = false;
          break;
        case '"':
          Output << "\\\"";
          PreviousHexEscape = false;
          break;
        case '\n':
          Output << "\\n";
          PreviousHexEscape = false;
          break;
        case '\r':
          Output << "\\r";
          PreviousHexEscape = false;
          break;
        case '\t':
          Output << "\\t";
          PreviousHexEscape = false;
          break;
        default:
          if (Byte >= 0x20U && Byte <= 0x7EU && !(PreviousHexEscape && ((Byte >= '0' && Byte <= '9') || (Byte >= 'A' && Byte <= 'F') || (Byte >= 'a' && Byte <= 'f'))))
          {
            Output << static_cast<char>(Byte);
            PreviousHexEscape = false;
          }
          else
          {
            Output << "\\x" << HexDigits[Byte >> 4U] << HexDigits[Byte & 0x0FU];
            PreviousHexEscape = true;
          }
          break;
        }
      }
      Output << '"';
    }

    void printString(std::ostream &Output, const core::StringInterner &Strings, core::InternedStringId Id)
    {
      if (!Strings.contains(Id))
      {
        Output << "<invalid-string>";
        return;
      }
      printEscaped(Output, Strings.string(Id));
    }

    void printOrigin(std::ostream &Output, const std::optional<CstOrigin> &Origin)
    {
      if (!Origin)
      {
        Output << "none";
        return;
      }
      Output << "cst#" << Origin->node();
      if (Origin->hasElement())
      {
        Output << ".child#" << Origin->element();
      }
    }

    void printRecoveryOrigin(std::ostream &Output, CstOrigin Origin)
    {
      if (!Origin.isValid())
      {
        Output << "none";
        return;
      }
      Output << "cst#" << Origin.node();
      if (Origin.hasElement())
      {
        Output << ".child#" << Origin.element();
      }
    }

    struct PrintableChild
    {
      AstNodeRef Ref;
      std::string Label;
    };

    void appendListChildren(const AstContext &Context, AstNodeList List, std::string_view Label, std::vector<PrintableChild> &Children)
    {
      if (!Context.contains(List))
      {
        return;
      }
      const AstNodeListView Nodes = Context.list(List);
      for (std::size_t Index = 0; Index < Nodes.size(); ++Index)
      {
        Children.push_back({Nodes[Index], std::string(Label) + '[' + std::to_string(Index) + ']'});
      }
    }

    void appendOptionalExpression(const std::optional<AstExprId> &Id, std::string Label, std::vector<PrintableChild> &Children)
    {
      if (Id)
      {
        Children.push_back({AstNodeRef::expression(*Id), std::move(Label)});
      }
    }

    void appendOptionalStatement(const std::optional<AstStmtId> &Id, std::string Label, std::vector<PrintableChild> &Children)
    {
      if (Id)
      {
        Children.push_back({AstNodeRef::statement(*Id), std::move(Label)});
      }
    }

    struct PrintFrame
    {
      AstNodeRef Ref;
      std::string Label;
      std::size_t Depth = 0;
      std::vector<PrintableChild> Children;
      std::size_t NextChild = 0;
      bool Entered = false;
    };

    std::uint64_t nodeKey(AstNodeRef Ref) noexcept
    {
      return (static_cast<std::uint64_t>(Ref.Category) << 32U) | Ref.Index;
    }
  } // namespace

  void AstPrinter::print(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings, std::ostream &Output) const
  {
    const OutputFormatGuard Guard(Output);
    if (!Context.contains(File))
    {
      Output << "invalid-ast-file\n";
      return;
    }
    if (File.Strings != &Strings)
    {
      Output << "invalid-ast-string-owner\n";
      return;
    }

    const AstNodeRef Root = AstNodeRef::declaration(File.root());
    Output << "ast-file source=" << File.sourceFile().value() << " size=" << File.sourceSize() << " root=";
    printNodeRef(Output, File, Root);
    Output << '\n';

    std::unordered_set<std::uint64_t> Printed;
    std::vector<PrintFrame> Work;
    Work.push_back({Root, "root", 0});
    while (!Work.empty())
    {
      PrintFrame &Current = Work.back();
      if (!Current.Entered)
      {
        Current.Entered = true;
        indent(Output, Current.Depth);
        Output << Current.Label << ": ";
        printNodeRef(Output, File, Current.Ref);
        if (!Context.contains(Current.Ref))
        {
          Output << " <invalid-node>\n";
          Work.pop_back();
          continue;
        }
        if (!Printed.insert(nodeKey(Current.Ref)).second)
        {
          Output << " <duplicate-node>\n";
          Work.pop_back();
          continue;
        }

        const AstNodeView View = Context.node(Current.Ref);
        const AstNodeHeader &Header = *View.Header;
        Output << ' ' << astKindName(View.Kind) << " range=[" << Header.Range.Start << ", " << Header.Range.End << ") origin=";
        printOrigin(Output, Header.Origin);
        Output << " recoveries=" << Header.Recoveries.Count << " supplemental=" << Header.Supplemental.Count;

        switch (Current.Ref.Category)
        {
        case AstNodeCategory::Declaration:
        {
          const Declaration &Node = Context.declaration(AstDeclId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::SourceFile:
            if (const SourceFilePayload *Payload = std::get_if<SourceFilePayload>(&Node.Payload))
            {
              Output << " items=" << Payload->Items.Count;
              appendListChildren(Context, Payload->Items, "items", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::ErrorDeclaration:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              Output << " recovered=" << Payload->Recovered.Count;
              appendListChildren(Context, Payload->Recovered, "recovered", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::ImportDeclaration:
            if (const ImportPayload *Payload = std::get_if<ImportPayload>(&Node.Payload))
            {
              Output << " path=";
              printString(Output, Strings, Payload->Path);
              Output << " alias=";
              if (Payload->Alias)
              {
                printString(Output, Strings, *Payload->Alias);
              }
              else
              {
                Output << "none";
              }
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::BindingDeclaration:
            if (const BindingPayload *Payload = std::get_if<BindingPayload>(&Node.Payload))
            {
              Output << " mode=" << astBindingModeName(Payload->Mode) << " top-level=" << (Payload->TopLevel ? "true" : "false");
              Current.Children.push_back({AstNodeRef::pattern(Payload->Pattern), "pattern"});
              appendOptionalExpression(Payload->Type, "type", Current.Children);
              appendOptionalExpression(Payload->Initializer, "initializer", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::ParameterDeclaration:
            if (const ParameterPayload *Payload = std::get_if<ParameterPayload>(&Node.Payload))
            {
              Output << " flavor=" << astParameterFlavorName(Payload->Flavor) << " name=";
              printString(Output, Strings, Payload->Name);
              Output << " pack=" << (Payload->IsPack ? "true" : "false");
              Current.Children.push_back({AstNodeRef::expression(Payload->Type), "type"});
              appendOptionalExpression(Payload->DefaultValue, "default", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::FunctionDeclaration:
            if (const FunctionPayload *Payload = std::get_if<FunctionPayload>(&Node.Payload))
            {
              Output << " flavor=" << astFunctionFlavorName(Payload->Flavor) << " name=";
              printString(Output, Strings, Payload->Name);
              Output << " parameters=" << Payload->Parameters.Count;
              appendListChildren(Context, Payload->Parameters, "parameters", Current.Children);
              appendOptionalExpression(Payload->ResultType, "result", Current.Children);
              appendOptionalStatement(Payload->Body, "body", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::UnsupportedDeclaration:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              Output << " feature=" << unsupportedFeatureName(Payload->Feature) << " children=" << Payload->Children.Count;
              appendListChildren(Context, Payload->Children, "children", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          default:
            Output << " payload=<invalid-kind>";
            break;
          }
          break;
        }
        case AstNodeCategory::Expression:
        {
          const Expression &Node = Context.expression(AstExprId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::ErrorExpression:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              Output << " recovered=" << Payload->Recovered.Count;
              appendListChildren(Context, Payload->Recovered, "recovered", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::LiteralExpression:
            if (const LiteralPayload *Payload = std::get_if<LiteralPayload>(&Node.Payload))
            {
              Output << " literal=" << astLiteralKindName(Payload->Kind) << " spelling=";
              printString(Output, Strings, Payload->Spelling);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::NameExpression:
          case AstKind::TypeNameExpression:
          case AstKind::BuiltinTypeExpression:
            if (const NamePayload *Payload = std::get_if<NamePayload>(&Node.Payload))
            {
              Output << " name=";
              printString(Output, Strings, Payload->Name);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::ThisExpression:
            if (!std::holds_alternative<ThisPayload>(Node.Payload))
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::GroupExpression:
          case AstKind::TypeGroupExpression:
            if (const GroupPayload *Payload = std::get_if<GroupPayload>(&Node.Payload))
            {
              Current.Children.push_back({AstNodeRef::expression(Payload->Value), "value"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::UnaryExpression:
          case AstKind::ComptimeExpression:
            if (const UnaryPayload *Payload = std::get_if<UnaryPayload>(&Node.Payload))
            {
              Output << " operator=";
              printString(Output, Strings, Payload->Operator);
              Current.Children.push_back({AstNodeRef::expression(Payload->Operand), "operand"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::BinaryExpression:
            if (const BinaryPayload *Payload = std::get_if<BinaryPayload>(&Node.Payload))
            {
              Output << " operator=";
              printString(Output, Strings, Payload->Operator);
              Current.Children.push_back({AstNodeRef::expression(Payload->Left), "left"});
              Current.Children.push_back({AstNodeRef::expression(Payload->Right), "right"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::CallExpression:
            if (const CallPayload *Payload = std::get_if<CallPayload>(&Node.Payload))
            {
              Output << " arguments=" << Payload->Arguments.Count;
              Current.Children.push_back({AstNodeRef::expression(Payload->Callee), "callee"});
              appendListChildren(Context, Payload->Arguments, "arguments", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::IfExpression:
            if (const IfExpressionPayload *Payload = std::get_if<IfExpressionPayload>(&Node.Payload))
            {
              Current.Children.push_back({AstNodeRef::expression(Payload->Condition), "condition"});
              Current.Children.push_back({AstNodeRef::expression(Payload->ThenValue), "then"});
              Current.Children.push_back({AstNodeRef::expression(Payload->ElseValue), "else"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::FunctionTypeExpression:
            if (const FunctionTypePayload *Payload = std::get_if<FunctionTypePayload>(&Node.Payload))
            {
              Output << " parameters=" << Payload->Parameters.Count;
              appendListChildren(Context, Payload->Parameters, "parameters", Current.Children);
              appendOptionalExpression(Payload->Result, "result", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::UnsupportedExpression:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              Output << " feature=" << unsupportedFeatureName(Payload->Feature) << " children=" << Payload->Children.Count;
              appendListChildren(Context, Payload->Children, "children", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          default:
            Output << " payload=<invalid-kind>";
            break;
          }
          break;
        }
        case AstNodeCategory::Statement:
        {
          const Statement &Node = Context.statement(AstStmtId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::ErrorStatement:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              Output << " recovered=" << Payload->Recovered.Count;
              appendListChildren(Context, Payload->Recovered, "recovered", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::BlockStatement:
            if (const BlockPayload *Payload = std::get_if<BlockPayload>(&Node.Payload))
            {
              Output << " items=" << Payload->Items.Count;
              appendListChildren(Context, Payload->Items, "items", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::AssignmentStatement:
            if (const AssignmentPayload *Payload = std::get_if<AssignmentPayload>(&Node.Payload))
            {
              Output << " operator=";
              printString(Output, Strings, Payload->Operator);
              Current.Children.push_back({AstNodeRef::expression(Payload->Left), "left"});
              Current.Children.push_back({AstNodeRef::expression(Payload->Right), "right"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::ExpressionStatement:
            if (const ExpressionStatementPayload *Payload = std::get_if<ExpressionStatementPayload>(&Node.Payload))
            {
              Current.Children.push_back({AstNodeRef::expression(Payload->Value), "value"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::IfStatement:
            if (const IfStatementPayload *Payload = std::get_if<IfStatementPayload>(&Node.Payload))
            {
              Current.Children.push_back({Payload->Condition, "condition"});
              Current.Children.push_back({AstNodeRef::statement(Payload->ThenBlock), "then"});
              appendOptionalStatement(Payload->ElseBlock, "else", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::WhileStatement:
            if (const WhileStatementPayload *Payload = std::get_if<WhileStatementPayload>(&Node.Payload))
            {
              Current.Children.push_back({Payload->Condition, "condition"});
              Current.Children.push_back({AstNodeRef::statement(Payload->Body), "body"});
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::ReturnStatement:
            if (const ReturnPayload *Payload = std::get_if<ReturnPayload>(&Node.Payload))
            {
              appendOptionalExpression(Payload->Value, "value", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::BreakStatement:
          case AstKind::ContinueStatement:
            if (!std::holds_alternative<ControlStatementPayload>(Node.Payload))
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::UnsupportedStatement:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              Output << " feature=" << unsupportedFeatureName(Payload->Feature) << " children=" << Payload->Children.Count;
              appendListChildren(Context, Payload->Children, "children", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          default:
            Output << " payload=<invalid-kind>";
            break;
          }
          break;
        }
        case AstNodeCategory::Pattern:
        {
          const Pattern &Node = Context.pattern(AstPatternId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::ErrorPattern:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              Output << " recovered=" << Payload->Recovered.Count;
              appendListChildren(Context, Payload->Recovered, "recovered", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::BindingPattern:
            if (const BindingPatternPayload *Payload = std::get_if<BindingPatternPayload>(&Node.Payload))
            {
              Output << " name=";
              printString(Output, Strings, Payload->Name);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::WildcardPattern:
            if (!std::holds_alternative<WildcardPatternPayload>(Node.Payload))
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::TuplePattern:
            if (const TuplePatternPayload *Payload = std::get_if<TuplePatternPayload>(&Node.Payload))
            {
              Output << " elements=" << Payload->Elements.Count;
              appendListChildren(Context, Payload->Elements, "elements", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::VariantPattern:
            if (const VariantPatternPayload *Payload = std::get_if<VariantPatternPayload>(&Node.Payload))
            {
              Output << " name=";
              printString(Output, Strings, Payload->Name);
              Output << " elements=" << Payload->Elements.Count;
              appendListChildren(Context, Payload->Elements, "elements", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          case AstKind::UnsupportedPattern:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              Output << " feature=" << unsupportedFeatureName(Payload->Feature) << " children=" << Payload->Children.Count;
              appendListChildren(Context, Payload->Children, "children", Current.Children);
            }
            else
            {
              Output << " payload=<mismatch>";
            }
            break;
          default:
            Output << " payload=<invalid-kind>";
            break;
          }
          break;
        }
        case AstNodeCategory::Unknown:
          Output << " payload=<invalid-category>";
          break;
        }

        appendListChildren(Context, Header.Supplemental, "supplemental", Current.Children);
        Output << '\n';
        if (Context.contains(Header.Recoveries))
        {
          const AstRecoveryView Recoveries = Context.recoveries(Header.Recoveries);
          for (std::size_t Index = 0; Index < Recoveries.size(); ++Index)
          {
            const AstRecovery &Recovery = Recoveries[Index];
            indent(Output, Current.Depth + 1);
            Output << "recovery[" << Index << "]: " << astRecoveryKindName(Recovery.Kind) << " range=[" << Recovery.Range.Start << ", " << Recovery.Range.End << ") origin=";
            printRecoveryOrigin(Output, Recovery.Origin);
            Output << " expected=" << astExpectedKindName(Recovery.ExpectedKind) << " spelling=";
            printString(Output, Strings, Recovery.Spelling);
            Output << '\n';
          }
        }
      }

      if (Current.NextChild == Current.Children.size())
      {
        Work.pop_back();
        continue;
      }
      PrintableChild Child = std::move(Current.Children[Current.NextChild++]);
      const std::size_t ChildDepth = Current.Depth + 1;
      Work.push_back({Child.Ref, std::move(Child.Label), ChildDepth});
    }
  }

  std::string AstPrinter::print(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings) const
  {
    std::ostringstream Output;
    print(Context, File, Strings, Output);
    return Output.str();
  }

  void printAst(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings, std::ostream &Output)
  {
    AstPrinter().print(Context, File, Strings, Output);
  }

  std::string printAst(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings)
  {
    return AstPrinter().print(Context, File, Strings);
  }
} // namespace ink::ast
