#include "ink/parser/ast_walker.h"
#include <unordered_set>
#include <algorithm>
namespace ink::parser
{
  template <typename T>
  static bool validRecords(ASTArray<T> Records)
  {
    for (const auto &Record : Records)
    {
      if (!valid(Record))
      {
        return false;
      }
    }
    return true;
  }
  static bool valid(const Parameter &Record)
  {
    return Record.range().isValid() && Record.type() != nullptr && !(Record.defaultValue() && Record.variadic());
  }
  static bool valid(const FunctionTypeParameter &Record)
  {
    return Record.range().isValid() && Record.type() != nullptr;
  }
  static bool valid(const Argument &Record)
  {
    return Record.range().isValid() && Record.value() != nullptr && (Record.form() == ArgumentKind::Named) == Record.name().has_value();
  }
  static bool valid(const PathSegment &Record)
  {
    return Record.range().isValid() && (Record.hasGenericArguments() || Record.arguments().empty()) && validRecords(Record.arguments());
  }
  static bool valid(const Attribute &Record)
  {
    return Record.range().isValid() && !Record.path().empty() && (Record.suffix() == AttributeSuffix::Value) == (Record.value() != nullptr) && (Record.suffix() == AttributeSuffix::Arguments || Record.arguments().empty()) && validRecords(Record.arguments());
  }
  static bool valid(const BaseSpec &Record)
  {
    return Record.range().isValid() && Record.type() != nullptr;
  }
  static bool valid(const MatchArm &Record)
  {
    return Record.range().isValid() && Record.pattern() != nullptr && Record.value() != nullptr;
  }
  static bool valid(const SwitchClause &Record)
  {
    return Record.range().isValid() && Record.isDefault() == (Record.value() == nullptr) && std::all_of(Record.statements().begin(), Record.statements().end(), [](auto *Statement)
                                                                                                        {
                                                                                                          return Statement != nullptr;
                                                                                                        });
  }
  static bool valid(const ImportEntry &Record)
  {
    return Record.range().isValid() && !Record.path().empty();
  }
  bool verifyAST(const ASTNodeBase *Root, std::size_t SourceSize, std::string *Reason)
  {
    if (!Root)
    {
      if (Reason)
      {
        *Reason = "null root";
      }
      return false;
    }
    std::vector<const ASTNodeBase *> Pending{Root};
    std::unordered_set<const ASTNodeBase *> Seen;
    while (!Pending.empty())
    {
      auto *Node = Pending.back();
      Pending.pop_back();
      bool Valid = Node->getSourceRange().isValid() && Node->getSourceRange().getEnd().getByteOffset() <= SourceSize;
      if (!Seen.insert(Node).second)
      {
        Valid = false;
      }
      switch (Node->getKind())
      {
      case ASTKind::ModuleAST:
      {
        const auto *Value = static_cast<const ModuleAST *>(Node);
        Valid = Valid && std::all_of(Value->statements().begin(), Value->statements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::TypeSyntax:
      {
        const auto *Value = static_cast<const TypeSyntax *>(Node);
        Valid = Valid && Value->expression() != nullptr;
        break;
      }
      case ASTKind::MissingExpr:
      {
        break;
      }
      case ASTKind::ErrorExpr:
      {
        break;
      }
      case ASTKind::MissingStmt:
      {
        break;
      }
      case ASTKind::ErrorStmt:
      {
        break;
      }
      case ASTKind::MissingDecl:
      {
        const auto *Value = static_cast<const MissingDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes());
        break;
      }
      case ASTKind::ErrorDecl:
      {
        const auto *Value = static_cast<const ErrorDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes());
        break;
      }
      case ASTKind::MissingBindingPattern:
      {
        break;
      }
      case ASTKind::ErrorBindingPattern:
      {
        break;
      }
      case ASTKind::MissingMatchPattern:
      {
        break;
      }
      case ASTKind::ErrorMatchPattern:
      {
        break;
      }
      case ASTKind::NameExpr:
      {
        break;
      }
      case ASTKind::LiteralExpr:
      {
        break;
      }
      case ASTKind::UnaryExpr:
      {
        const auto *Value = static_cast<const UnaryExpr *>(Node);
        Valid = Valid && Value->operand() != nullptr;
        break;
      }
      case ASTKind::ComptimeExpr:
      {
        const auto *Value = static_cast<const ComptimeExpr *>(Node);
        Valid = Valid && Value->operand() != nullptr;
        break;
      }
      case ASTKind::BinaryExpr:
      {
        const auto *Value = static_cast<const BinaryExpr *>(Node);
        Valid = Valid && Value->left() != nullptr && Value->right() != nullptr;
        break;
      }
      case ASTKind::ConditionalExpr:
      {
        const auto *Value = static_cast<const ConditionalExpr *>(Node);
        Valid = Valid && Value->condition() != nullptr && Value->then() != nullptr && Value->elseValue() != nullptr;
        break;
      }
      case ASTKind::ParenExpr:
      {
        const auto *Value = static_cast<const ParenExpr *>(Node);
        Valid = Valid && Value->expression() != nullptr;
        break;
      }
      case ASTKind::TupleExpr:
      {
        const auto *Value = static_cast<const TupleExpr *>(Node);
        Valid = Valid && std::all_of(Value->elements().begin(), Value->elements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::ArrayExpr:
      {
        const auto *Value = static_cast<const ArrayExpr *>(Node);
        Valid = Valid && std::all_of(Value->elements().begin(), Value->elements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::ArrayRepeatExpr:
      {
        const auto *Value = static_cast<const ArrayRepeatExpr *>(Node);
        Valid = Valid && Value->value() != nullptr && Value->count() != nullptr;
        break;
      }
      case ASTKind::CallExpr:
      {
        const auto *Value = static_cast<const CallExpr *>(Node);
        Valid = Valid && Value->callee() != nullptr && validRecords(Value->arguments());
        break;
      }
      case ASTKind::IndexExpr:
      {
        const auto *Value = static_cast<const IndexExpr *>(Node);
        Valid = Valid && Value->object() != nullptr && Value->index() != nullptr;
        break;
      }
      case ASTKind::MemberExpr:
      {
        const auto *Value = static_cast<const MemberExpr *>(Node);
        Valid = Valid && Value->object() != nullptr;
        break;
      }
      case ASTKind::GenericApplyExpr:
      {
        const auto *Value = static_cast<const GenericApplyExpr *>(Node);
        Valid = Valid && Value->object() != nullptr && validRecords(Value->arguments());
        break;
      }
      case ASTKind::PostfixUpdateExpr:
      {
        const auto *Value = static_cast<const PostfixUpdateExpr *>(Node);
        Valid = Valid && Value->operand() != nullptr;
        break;
      }
      case ASTKind::FunctionTypeExpr:
      {
        const auto *Value = static_cast<const FunctionTypeExpr *>(Node);
        Valid = Valid && validRecords(Value->parameters()) && Value->returnType() != nullptr;
        break;
      }
      case ASTKind::LambdaExpr:
      {
        const auto *Value = static_cast<const LambdaExpr *>(Node);
        Valid = Valid && validRecords(Value->genericParameters()) && validRecords(Value->parameters()) && Value->body() != nullptr;
        break;
      }
      case ASTKind::MatchExpr:
      {
        const auto *Value = static_cast<const MatchExpr *>(Node);
        Valid = Valid && Value->value() != nullptr && validRecords(Value->arms());
        break;
      }
      case ASTKind::BlockExpr:
      {
        const auto *Value = static_cast<const BlockExpr *>(Node);
        Valid = Valid && Value->body() != nullptr;
        break;
      }
      case ASTKind::ExprItem:
      {
        const auto *Value = static_cast<const ExprItem *>(Node);
        Valid = Valid && Value->expression() != nullptr;
        break;
      }
      case ASTKind::AssignmentItem:
      {
        const auto *Value = static_cast<const AssignmentItem *>(Node);
        Valid = Valid && Value->left() != nullptr && Value->right() != nullptr;
        break;
      }
      case ASTKind::SimpleStmt:
      {
        const auto *Value = static_cast<const SimpleStmt *>(Node);
        Valid = Valid && !Value->items().empty();
        Valid = Valid && std::all_of(Value->items().begin(), Value->items().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::BlockStmt:
      {
        const auto *Value = static_cast<const BlockStmt *>(Node);
        Valid = Valid && std::all_of(Value->statements().begin(), Value->statements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::DeclStmt:
      {
        const auto *Value = static_cast<const DeclStmt *>(Node);
        Valid = Valid && Value->declaration() != nullptr;
        break;
      }
      case ASTKind::IfStmt:
      {
        const auto *Value = static_cast<const IfStmt *>(Node);
        Valid = Valid && Value->condition() != nullptr && Value->thenBranch() != nullptr;
        break;
      }
      case ASTKind::WhileStmt:
      {
        const auto *Value = static_cast<const WhileStmt *>(Node);
        Valid = Valid && Value->condition() != nullptr && Value->body() != nullptr;
        break;
      }
      case ASTKind::ClassicForStmt:
      {
        const auto *Value = static_cast<const ClassicForStmt *>(Node);
        Valid = Valid && std::all_of(Value->step().begin(), Value->step().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     }) &&
                Value->body() != nullptr;
        break;
      }
      case ASTKind::ForInStmt:
      {
        const auto *Value = static_cast<const ForInStmt *>(Node);
        Valid = Valid && Value->binding() != nullptr && Value->iterable() != nullptr && Value->body() != nullptr;
        break;
      }
      case ASTKind::SwitchStmt:
      {
        const auto *Value = static_cast<const SwitchStmt *>(Node);
        Valid = Valid && Value->value() != nullptr && validRecords(Value->clauses());
        break;
      }
      case ASTKind::ReturnStmt:
      {
        break;
      }
      case ASTKind::BreakStmt:
      {
        break;
      }
      case ASTKind::ContinueStmt:
      {
        break;
      }
      case ASTKind::YieldStmt:
      {
        const auto *Value = static_cast<const YieldStmt *>(Node);
        Valid = Valid && Value->value() != nullptr;
        break;
      }
      case ASTKind::DeferStmt:
      {
        const auto *Value = static_cast<const DeferStmt *>(Node);
        Valid = Valid && Value->body() != nullptr;
        break;
      }
      case ASTKind::ComptimeStmt:
      {
        const auto *Value = static_cast<const ComptimeStmt *>(Node);
        Valid = Valid && Value->body() != nullptr;
        break;
      }
      case ASTKind::DirectImportStmt:
      {
        const auto *Value = static_cast<const DirectImportStmt *>(Node);
        Valid = Valid && validRecords(Value->imports());
        break;
      }
      case ASTKind::FromImportStmt:
      {
        const auto *Value = static_cast<const FromImportStmt *>(Node);
        Valid = Valid && validRecords(Value->imports());
        break;
      }
      case ASTKind::VarDecl:
      {
        const auto *Value = static_cast<const VarDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes()) && Value->binding() != nullptr && (Value->form() == VarDeclForm::Initialized) == (Value->initializer() != nullptr);
        Valid = Valid && (Value->initializer() || (!Value->constant() && isa<NameBindingPattern>(Value->binding())));
        break;
      }
      case ASTKind::FieldDecl:
      {
        const auto *Value = static_cast<const FieldDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes()) && validRecords(Value->payload());
        Valid = Valid && (Value->tail() == FieldTailKind::Typed) == (Value->type() != nullptr) && (Value->tail() == FieldTailKind::Payload) == !Value->payload().empty();
        Valid = Valid && (Value->tail() != FieldTailKind::None || Value->initializer() == nullptr) && (Value->tail() != FieldTailKind::InitializerOnly || Value->initializer() != nullptr);
        break;
      }
      case ASTKind::FunctionDecl:
      {
        const auto *Value = static_cast<const FunctionDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes()) && validRecords(Value->genericParameters()) && validRecords(Value->parameters()) && Value->returnType() != nullptr;
        Valid = Valid && (Value->bodyKind() == FunctionBodyKind::Definition) == (Value->body() != nullptr);
        break;
      }
      case ASTKind::ClassDecl:
      {
        const auto *Value = static_cast<const ClassDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes()) && validRecords(Value->genericParameters()) && validRecords(Value->bases());
        Valid = Valid && (Value->form() == AggregateForm::Definition) == (Value->body() != nullptr) && Value->semicolonRange().isValid();
        break;
      }
      case ASTKind::EnumDecl:
      {
        const auto *Value = static_cast<const EnumDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes()) && validRecords(Value->genericParameters()) && validRecords(Value->bases());
        Valid = Valid && Value->form() == AggregateForm::Definition && Value->body() != nullptr && Value->semicolonRange().isValid();
        break;
      }
      case ASTKind::InterfaceDecl:
      {
        const auto *Value = static_cast<const InterfaceDecl *>(Node);
        Valid = Valid && validRecords(Value->attributes()) && validRecords(Value->genericParameters()) && validRecords(Value->bases());
        Valid = Valid && Value->form() == AggregateForm::Definition && Value->body() != nullptr && Value->semicolonRange().isValid();
        break;
      }
      case ASTKind::WildcardBindingPattern:
      {
        break;
      }
      case ASTKind::NameBindingPattern:
      {
        break;
      }
      case ASTKind::TupleBindingPattern:
      {
        const auto *Value = static_cast<const TupleBindingPattern *>(Node);
        Valid = Valid && std::all_of(Value->elements().begin(), Value->elements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::ArrayBindingPattern:
      {
        const auto *Value = static_cast<const ArrayBindingPattern *>(Node);
        Valid = Valid && std::all_of(Value->elements().begin(), Value->elements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::WildcardMatchPattern:
      {
        break;
      }
      case ASTKind::NameMatchPattern:
      {
        const auto *Value = static_cast<const NameMatchPattern *>(Node);
        Valid = Valid && !Value->path().empty() && (Value->hasArguments() || Value->arguments().empty());
        Valid = Valid && validRecords(Value->path()) && std::all_of(Value->arguments().begin(), Value->arguments().end(), [](auto *Child)
                                                                    {
                                                                      return Child != nullptr;
                                                                    });
        break;
      }
      case ASTKind::TupleMatchPattern:
      {
        const auto *Value = static_cast<const TupleMatchPattern *>(Node);
        Valid = Valid && std::all_of(Value->elements().begin(), Value->elements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::ArrayMatchPattern:
      {
        const auto *Value = static_cast<const ArrayMatchPattern *>(Node);
        Valid = Valid && std::all_of(Value->elements().begin(), Value->elements().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      case ASTKind::LiteralMatchPattern:
      {
        const auto *Value = static_cast<const LiteralMatchPattern *>(Node);
        Valid = Valid && Value->value() != nullptr;
        break;
      }
      case ASTKind::GroupedMatchPattern:
      {
        const auto *Value = static_cast<const GroupedMatchPattern *>(Node);
        Valid = Valid && Value->pattern() != nullptr;
        break;
      }
      case ASTKind::OrMatchPattern:
      {
        const auto *Value = static_cast<const OrMatchPattern *>(Node);
        Valid = Valid && Value->alternatives().size() >= 2;
        Valid = Valid && std::all_of(Value->alternatives().begin(), Value->alternatives().end(), [](auto *Child)
                                     {
                                       return Child != nullptr;
                                     });
        break;
      }
      }
      if (!Valid)
      {
        if (Reason)
        {
          *Reason = std::string("invalid ") + astKindName(Node->getKind());
        }
        return false;
      }
      forEachChild(Node, [&](const ASTNodeBase *Child)
                   {
                     if (!Node->getSourceRange().fullyContains(Child->getSourceRange()))
                     {
                       Valid = false;
                     }
                     Pending.push_back(Child);
                   });
      if (!Valid)
      {
        if (Reason)
        {
          *Reason = std::string("child outside ") + astKindName(Node->getKind());
        }
        return false;
      }
    }
    return true;
  }
} // namespace ink::parser
