#include "ink/core/diagnostic/diagnostic.h"

namespace ink::core
{
  std::uint32_t diagnosticNumber(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return Number;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return 0;
  }

  const char *diagnosticCode(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return Code;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "INK-0000";
  }

  const char *diagnosticKindName(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return #Name;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "Unknown";
  }

  const char *diagnosticDefaultMessage(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return DefaultMessage;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "unknown diagnostic";
  }

  const char *diagnosticFormatPattern(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return FormatPattern;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "unknown diagnostic";
  }

  DiagnosticDomain diagnosticDomain(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return DiagnosticDomain::Domain;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticDomain::Unknown;
  }

  DiagnosticClass diagnosticClass(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return DiagnosticClass::Class;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticClass::Unknown;
  }

  DiagnosticSeverity diagnosticDefaultSeverity(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return DiagnosticSeverity::DefaultSeverity;
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticSeverity::Unknown;
  }

  const char *diagnosticClassName(DiagnosticClass Class) noexcept
  {
    switch (Class)
    {
    case DiagnosticClass::Unknown:
      return "unknown";
    case DiagnosticClass::User:
      return "user";
    case DiagnosticClass::InternalCompilerError:
      return "internal compiler error";
    }
    return "unknown";
  }

  const char *diagnosticSeverityName(DiagnosticSeverity Severity) noexcept
  {
    switch (Severity)
    {
    case DiagnosticSeverity::Unknown:
      return "unknown";
    case DiagnosticSeverity::Error:
      return "error";
    case DiagnosticSeverity::Warning:
      return "warning";
    case DiagnosticSeverity::Note:
      return "note";
    }
    return "unknown";
  }

  const char *diagnosticArgumentName(DiagnosticArgumentName Name) noexcept
  {
    switch (Name)
    {
    case DiagnosticArgumentName::Unknown:
      return "Unknown";
    case DiagnosticArgumentName::Character:
      return "Character";
    case DiagnosticArgumentName::Context:
      return "Context";
    case DiagnosticArgumentName::RemainingNestingDepth:
      return "RemainingNestingDepth";
    case DiagnosticArgumentName::Expected:
      return "Expected";
    case DiagnosticArgumentName::Actual:
      return "Actual";
    case DiagnosticArgumentName::Description:
      return "Description";
    case DiagnosticArgumentName::TypeName:
      return "TypeName";
    case DiagnosticArgumentName::StructName:
      return "StructName";
    case DiagnosticArgumentName::SymbolName:
      return "SymbolName";
    case DiagnosticArgumentName::FunctionName:
      return "FunctionName";
    case DiagnosticArgumentName::BlockName:
      return "BlockName";
    case DiagnosticArgumentName::CalleeName:
      return "CalleeName";
    case DiagnosticArgumentName::Operation:
      return "Operation";
    case DiagnosticArgumentName::ArgumentIndex:
      return "ArgumentIndex";
    case DiagnosticArgumentName::ParameterIndex:
      return "ParameterIndex";
    case DiagnosticArgumentName::FieldIndex:
      return "FieldIndex";
    case DiagnosticArgumentName::ValueId:
      return "ValueId";
    case DiagnosticArgumentName::ByteConstantId:
      return "ByteConstantId";
    case DiagnosticArgumentName::GlobalId:
      return "GlobalId";
    case DiagnosticArgumentName::ModuleName:
      return "ModuleName";
    case DiagnosticArgumentName::RelatedModuleName:
      return "RelatedModuleName";
    case DiagnosticArgumentName::ExpectedModuleName:
      return "ExpectedModuleName";
    case DiagnosticArgumentName::ActualModuleName:
      return "ActualModuleName";
    case DiagnosticArgumentName::FunctionId:
      return "FunctionId";
    case DiagnosticArgumentName::ExpectedValue:
      return "ExpectedValue";
    case DiagnosticArgumentName::ActualValue:
      return "ActualValue";
    case DiagnosticArgumentName::Predicate:
      return "Predicate";
    case DiagnosticArgumentName::FieldType:
      return "FieldType";
    case DiagnosticArgumentName::OperandIndex:
      return "OperandIndex";
    case DiagnosticArgumentName::RequestedSize:
      return "RequestedSize";
    case DiagnosticArgumentName::AllocationLimit:
      return "AllocationLimit";
    case DiagnosticArgumentName::Index:
      return "Index";
    case DiagnosticArgumentName::RegionLength:
      return "RegionLength";
    case DiagnosticArgumentName::MaximumValue:
      return "MaximumValue";
    case DiagnosticArgumentName::Format:
      return "Format";
    case DiagnosticArgumentName::ExpectedDigits:
      return "ExpectedDigits";
    case DiagnosticArgumentName::ActualDigits:
      return "ActualDigits";
    case DiagnosticArgumentName::MaximumSize:
      return "MaximumSize";
    case DiagnosticArgumentName::ExpectedCount:
      return "ExpectedCount";
    case DiagnosticArgumentName::ActualCount:
      return "ActualCount";
    case DiagnosticArgumentName::DeclaredSize:
      return "DeclaredSize";
    case DiagnosticArgumentName::ActualSize:
      return "ActualSize";
    case DiagnosticArgumentName::FieldCount:
      return "FieldCount";
    case DiagnosticArgumentName::Offset:
      return "Offset";
    case DiagnosticArgumentName::Size:
      return "Size";
    case DiagnosticArgumentName::ActualVersion:
      return "ActualVersion";
    case DiagnosticArgumentName::SupportedVersion:
      return "SupportedVersion";
    case DiagnosticArgumentName::Alignment:
      return "Alignment";
    case DiagnosticArgumentName::MaximumAlignment:
      return "MaximumAlignment";
    case DiagnosticArgumentName::CallDepthLimit:
      return "CallDepthLimit";
    case DiagnosticArgumentName::ImportDepthLimit:
      return "ImportDepthLimit";
    case DiagnosticArgumentName::InstructionName:
      return "InstructionName";
    }
    return "Unknown";
  }

  bool operator==(const DiagnosticArgument &Left, const DiagnosticArgument &Right)
  {
    return Left.Name == Right.Name && Left.Value == Right.Value;
  }

  bool operator!=(const DiagnosticArgument &Left, const DiagnosticArgument &Right)
  {
    return !(Left == Right);
  }

  bool operator==(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right)
  {
    return Left.Kind == Right.Kind && Left.Span == Right.Span && Left.Arguments == Right.Arguments;
  }

  bool operator!=(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right)
  {
    return !(Left == Right);
  }

  std::uint32_t Diagnostic::number() const noexcept
  {
    return diagnosticNumber(Kind);
  }

  const char *Diagnostic::code() const noexcept
  {
    return diagnosticCode(Kind);
  }

  DiagnosticClass Diagnostic::classification() const noexcept
  {
    return Class == DiagnosticClass::Unknown ? diagnosticClass(Kind) : Class;
  }

  bool operator==(const Diagnostic &Left, const Diagnostic &Right)
  {
    return Left.Kind == Right.Kind && Left.Source == Right.Source && Left.Span == Right.Span && Left.Arguments == Right.Arguments && Left.Related == Right.Related && Left.Class == Right.Class;
  }

  bool operator!=(const Diagnostic &Left, const Diagnostic &Right)
  {
    return !(Left == Right);
  }
} // namespace ink::core
