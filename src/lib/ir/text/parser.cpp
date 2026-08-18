#include "parser.h"

#include "format.h"
#include "ink/ir/builder.h"
#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/memory.h"
#include "ink/ir/model/constant.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/operand.h"
#include "ink/ir/model/struct_type.h"
#include "module_draft.h"

#include <charconv>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  using core::Diagnostic;
  using core::DiagnosticKind;

  namespace
  {
    using text::BranchFixupTarget;
    using text::ModuleDraft;
    using text::Token;
    using text::TokenKind;

    struct ParsedBlockTarget
    {
        BlockTarget Target;
        Name BlockName;
        Token Location;
    };

    struct ParsedOperand
    {
        ValueHandle ParsedValue;
        Token Location;
    };

    struct InstructionParseState
    {
        FunctionId Function;
        std::optional<ValueId> Result;
    };

    class TextParser
    {
      public:
        TextParser(IRContext &Context, ModuleDraft &Draft, std::vector<Token> Tokens)
            : Context(Context),
              Draft(Draft),
              Builder(Draft.Builder),
              Tokens(std::move(Tokens))
        {
        }

        bool parse(Diagnostic &Error)
        {
          if (!parseModule())
          {
            Error = std::move(ParseDiagnostic);
            return false;
          }
          return true;
        }

      private:
        bool parseModule()
        {
          if (!expectIdentifier("inkir"))
          {
            return false;
          }
          const Token *Version = expect(TokenKind::Integer, "IR format version");
          if (Version == nullptr)
          {
            return false;
          }
          const std::optional<std::uint64_t> VersionNumber = parseUnsigned(*Version, "IR format version");
          if (!VersionNumber)
          {
            return false;
          }
          if (*VersionNumber != text::CurrentFormatVersion)
          {
            return fail<DiagnosticKind::IrUnsupportedFormatVersion>(*Version, *VersionNumber, text::CurrentFormatVersion);
          }

          while (!at(TokenKind::End))
          {
            if (atIdentifier("module"))
            {
              if (!parseModuleDeclaration())
              {
                return false;
              }
            }
            else if (atIdentifier("initializer"))
            {
              if (!parseLifecycleFunction(true))
              {
                return false;
              }
            }
            else if (atIdentifier("finalizer"))
            {
              if (!parseLifecycleFunction(false))
              {
                return false;
              }
            }
            else if (at(TokenKind::TypeName))
            {
              if (!parseStructType())
              {
                return false;
              }
            }
            else if (at(TokenKind::GlobalName))
            {
              if (atIdentifier("global", 2))
              {
                if (!parseGlobalVariable())
                {
                  return false;
                }
              }
              else if (!parseByteConstant())
              {
                return false;
              }
            }
            else if (atIdentifier("declare"))
            {
              if (atIdentifier("import", 1))
              {
                if (atIdentifier("global", 2))
                {
                  if (!parseImportedGlobalVariable())
                  {
                    return false;
                  }
                }
                else if (!parseImportedFunction())
                {
                  return false;
                }
              }
              else if (!parseExternalFunction())
              {
                return false;
              }
            }
            else if (atIdentifier("define"))
            {
              if (!parseFunctionDefinition())
              {
                return false;
              }
            }
            else
            {
              return fail<DiagnosticKind::IrExpectedTopLevelDeclaration>(current());
            }
          }
          return true;
        }

        bool parseModuleDeclaration()
        {
          const Token Keyword = consume();
          if (Draft.HasModuleDeclaration)
          {
            return fail<DiagnosticKind::IrExpected>(Keyword, "only one module declaration");
          }
          const Token *Name = expect(TokenKind::Identifier, "a module name");
          if (Name == nullptr)
          {
            return false;
          }
          Builder.setModuleName(Name->Text);
          Draft.HasModuleDeclaration = true;
          return true;
        }

        bool parseLifecycleFunction(bool IsInitializer)
        {
          const Token Keyword = consume();
          std::optional<Name> &Name = IsInitializer ? Draft.InitializerName : Draft.FinalizerName;
          Token &Location = IsInitializer ? Draft.InitializerLocation : Draft.FinalizerLocation;
          if (Name.has_value())
          {
            return fail<DiagnosticKind::IrExpected>(Keyword, IsInitializer ? "only one initializer declaration" : "only one finalizer declaration");
          }
          const Token *FunctionName = expect(TokenKind::GlobalName, "a lifecycle function name");
          if (FunctionName == nullptr)
          {
            return false;
          }
          Name = FunctionName->Text;
          Location = *FunctionName;
          return true;
        }

        const Token &current(std::size_t Offset = 0) const
        {
          const std::size_t Target = Index + Offset;
          return Tokens[Target < Tokens.size() ? Target : Tokens.size() - 1];
        }

        bool at(TokenKind Kind, std::size_t Offset = 0) const
        {
          return current(Offset).Kind == Kind;
        }

        bool atIdentifier(std::string_view Name, std::size_t Offset = 0) const
        {
          return at(TokenKind::Identifier, Offset) && current(Offset).Text == Name;
        }

        const Token &consume()
        {
          return Tokens[Index++];
        }

        const Token *expect(TokenKind Kind, std::string_view Expected)
        {
          if (!at(Kind))
          {
            fail<DiagnosticKind::IrExpected>(current(), Expected);
            return nullptr;
          }
          return &consume();
        }

        bool expectIdentifier(std::string_view Name)
        {
          if (!atIdentifier(Name))
          {
            return fail<DiagnosticKind::IrExpectedIdentifier>(current(), Name);
          }
          consume();
          return true;
        }

        template <DiagnosticKind Kind, typename... ArgumentTypes>
        bool fail(const Token &Location, ArgumentTypes &&...Arguments)
        {
          ParseDiagnostic = core::makeDiagnostic<Kind>(Location.Span, std::forward<ArgumentTypes>(Arguments)...);
          return false;
        }

        std::optional<std::uint64_t> parseUnsigned(const Token &TokenValue, std::string_view Description)
        {
          if (!TokenValue.Text.empty() && TokenValue.Text.front() == '-')
          {
            fail<DiagnosticKind::IrNegativeNumericValue>(TokenValue, Description);
            return std::nullopt;
          }
          std::uint64_t Result = 0;
          const char *Begin = TokenValue.Text.data();
          const char *End = Begin + TokenValue.Text.size();
          const auto Conversion = std::from_chars(Begin, End, Result);
          if (Conversion.ec != std::errc() || Conversion.ptr != End)
          {
            fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
            return std::nullopt;
          }
          return Result;
        }

        std::optional<std::int64_t> parseSigned(const Token &TokenValue, std::string_view Description)
        {
          std::int64_t Result = 0;
          const char *Begin = TokenValue.Text.data();
          const char *End = Begin + TokenValue.Text.size();
          const auto Conversion = std::from_chars(Begin, End, Result);
          if (Conversion.ec != std::errc() || Conversion.ptr != End)
          {
            fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
            return std::nullopt;
          }
          return Result;
        }

        std::optional<std::uint64_t> parseHexadecimal(const Token &TokenValue, std::string_view Description)
        {
          std::uint64_t Result = 0;
          const char *Begin = TokenValue.Text.data();
          const char *End = Begin + TokenValue.Text.size();
          const auto Conversion = std::from_chars(Begin, End, Result, 16);
          if (Conversion.ec != std::errc() || Conversion.ptr != End)
          {
            fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
            return std::nullopt;
          }
          return Result;
        }

        std::optional<FloatFormat> parseFloatFormat()
        {
          const Token *Format = expect(TokenKind::Identifier, "a floating-point format");
          if (Format == nullptr)
          {
            return std::nullopt;
          }
          if (Format->Text == "f16")
          {
            return FloatFormat::F16;
          }
          if (Format->Text == "f32")
          {
            return FloatFormat::F32;
          }
          if (Format->Text == "f64")
          {
            return FloatFormat::F64;
          }
          fail<DiagnosticKind::IrUnknownFloatFormat>(*Format, Format->Text);
          return std::nullopt;
        }

        std::optional<std::size_t> parseIndex(const Token &TokenValue, std::string_view Description)
        {
          const std::optional<std::uint64_t> Value = parseUnsigned(TokenValue, Description);
          if (!Value)
          {
            return std::nullopt;
          }
          if (*Value > std::numeric_limits<std::size_t>::max())
          {
            fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
            return std::nullopt;
          }
          return static_cast<std::size_t>(*Value);
        }

        std::optional<std::size_t> parseId(const Token &TokenValue, std::string_view Description)
        {
          const std::optional<std::size_t> Value = parseIndex(TokenValue, Description);
          if (!Value || *Value == InvalidId)
          {
            if (Value)
            {
              fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
            }
            return std::nullopt;
          }
          return Value;
        }

        std::optional<const Type *> parseType()
        {
          if (at(TokenKind::TypeName))
          {
            const Token &Name = consume();
            const auto Type = Draft.TypeNames.find(Name.Text);
            if (Type == Draft.TypeNames.end())
            {
              fail<DiagnosticKind::IrUnknownStructType>(Name, Name.Text);
              return std::nullopt;
            }
            return Type->second;
          }
          if (atIdentifier("const"))
          {
            consume();
            if (!expectIdentifier("byte"))
            {
              return std::nullopt;
            }
            if (at(TokenKind::Star))
            {
              consume();
              return &Context.getType(TypeKind::ConstBytePointer);
            }
            if (at(TokenKind::LeftBracket) && at(TokenKind::RightBracket, 1))
            {
              consume();
              consume();
              return &Context.getType(TypeKind::ConstByteSlice);
            }
            fail<DiagnosticKind::IrExpected>(current(), "'*' or '[]' after 'const byte'");
            return std::nullopt;
          }
          if (atIdentifier("byte") && at(TokenKind::Star, 1))
          {
            consume();
            consume();
            return &Context.getType(TypeKind::BytePointer);
          }
          if (atIdentifier("byte") && at(TokenKind::LeftBracket, 1) && at(TokenKind::RightBracket, 2))
          {
            consume();
            consume();
            consume();
            return &Context.getType(TypeKind::ByteSlice);
          }
          const Token *Type = expect(TokenKind::Identifier, "an IR type");
          if (Type == nullptr)
          {
            return std::nullopt;
          }
#define INK_IR_TYPE(Name, Spelling)          \
  if (Type->Text == Spelling)                \
  {                                          \
    return &Context.getType(TypeKind::Name); \
  }
#include "ink/ir/ir.def"
          fail<DiagnosticKind::IrUnknownType>(*Type, Type->Text);
          return std::nullopt;
        }

        bool parseStructType()
        {
          const Token *Name = expect(TokenKind::TypeName, "a struct type name");
          if (Name == nullptr)
          {
            return false;
          }
          if (Draft.TypeNames.find(Name->Text) != Draft.TypeNames.end())
          {
            return fail<DiagnosticKind::IrDuplicateStructType>(*Name, Name->Text);
          }
          if (expect(TokenKind::Equal, "'='") == nullptr || !expectIdentifier("type"))
          {
            return false;
          }
          StructLayoutConstraints LayoutConstraints;
          while (atIdentifier("align") || atIdentifier("pack"))
          {
            const bool IsAlignment = atIdentifier("align");
            consume();
            const std::optional<std::size_t> Value = parseParenthesizedLayoutValue(IsAlignment ? "struct alignment" : "struct packing");
            if (!Value)
            {
              return false;
            }
            if (IsAlignment)
            {
              LayoutConstraints.ExplicitAlignment = *Value;
            }
            else
            {
              LayoutConstraints.Packing = *Value;
            }
          }
          if (expect(TokenKind::LeftBrace, "'{'") == nullptr)
          {
            return false;
          }

          std::vector<StructField> Fields;
          if (!at(TokenKind::RightBrace))
          {
            std::optional<StructField> FirstField = parseStructField();
            if (!FirstField.has_value())
            {
              return false;
            }
            Fields.push_back(std::move(*FirstField));
            while (at(TokenKind::Comma))
            {
              consume();
              std::optional<StructField> Field = parseStructField();
              if (!Field.has_value())
              {
                return false;
              }
              Fields.push_back(std::move(*Field));
            }
          }
          if (expect(TokenKind::RightBrace, "'}'") == nullptr)
          {
            return false;
          }

          const StructType &TypeValue = Builder.createStructType(Name->Text, std::move(Fields), std::move(LayoutConstraints));
          Draft.TypeNames.emplace(Name->Text, &TypeValue);
          return true;
        }

        std::optional<std::size_t> parseParenthesizedLayoutValue(std::string_view Description)
        {
          if (expect(TokenKind::LeftParenthesis, "'('") == nullptr)
          {
            return std::nullopt;
          }
          const Token *ValueToken = expect(TokenKind::Integer, Description);
          if (ValueToken == nullptr)
          {
            return std::nullopt;
          }
          const std::optional<std::size_t> Value = parseIndex(*ValueToken, Description);
          if (!Value || expect(TokenKind::RightParenthesis, "')'") == nullptr)
          {
            return std::nullopt;
          }
          return Value;
        }

        std::optional<Attribute> parseAttribute()
        {
          const Token *KindToken = expect(TokenKind::Identifier, "a built-in attribute name");
          if (KindToken == nullptr)
          {
            return std::nullopt;
          }
          const std::optional<AttributeKind> Kind = attributeKindFromSpelling(KindToken->Text);
          if (!Kind.has_value())
          {
            fail<DiagnosticKind::IrExpected>(*KindToken, "a known built-in attribute name");
            return std::nullopt;
          }
          std::vector<AttributeArgument> Arguments;
          if (at(TokenKind::LeftParenthesis))
          {
            consume();
            if (!at(TokenKind::RightParenthesis))
            {
              while (true)
              {
                const Token *Key = expect(TokenKind::Identifier, "an attribute argument name");
                if (Key == nullptr || expect(TokenKind::Equal, "'='") == nullptr)
                {
                  return std::nullopt;
                }
                const Token ValueLocation = current();
                const std::optional<const Type *> ValueType = parseType();
                if (!ValueType.has_value())
                {
                  return std::nullopt;
                }
                const std::optional<const Constant *> Value = parseConstantValue(**ValueType);
                if (!Value.has_value())
                {
                  fail<DiagnosticKind::IrExpected>(ValueLocation, "a constant attribute argument value");
                  return std::nullopt;
                }
                Arguments.emplace_back(Key->Text, **Value);
                if (!at(TokenKind::Comma))
                {
                  break;
                }
                consume();
              }
            }
            if (expect(TokenKind::RightParenthesis, "')'") == nullptr)
            {
              return std::nullopt;
            }
          }
          return Attribute(*Kind, std::move(Arguments));
        }

        std::optional<std::vector<Attribute>> parseAttributes()
        {
          std::vector<Attribute> Attributes;
          if (!at(TokenKind::LeftBracket))
          {
            return Attributes;
          }
          consume();
          if (!at(TokenKind::RightBracket))
          {
            while (true)
            {
              std::optional<Attribute> AttributeValue = parseAttribute();
              if (!AttributeValue.has_value())
              {
                return std::nullopt;
              }
              Attributes.push_back(std::move(*AttributeValue));
              if (!at(TokenKind::Comma))
              {
                break;
              }
              consume();
            }
          }
          if (expect(TokenKind::RightBracket, "']'") == nullptr)
          {
            return std::nullopt;
          }
          return Attributes;
        }

        std::optional<StructField> parseStructField()
        {
          ink::ir::Name Name;
          if (at(TokenKind::Identifier) && at(TokenKind::Colon, 1))
          {
            Name = consume().Text;
            consume();
          }
          const std::optional<const Type *> FieldType = parseType();
          if (!FieldType.has_value())
          {
            return std::nullopt;
          }
          FieldLayoutConstraints LayoutConstraints;
          while (atIdentifier("align") || atIdentifier("offset"))
          {
            const bool IsAlignment = atIdentifier("align");
            consume();
            const std::optional<std::size_t> Value = parseParenthesizedLayoutValue(IsAlignment ? "field alignment" : "field offset");
            if (!Value)
            {
              return std::nullopt;
            }
            if (IsAlignment)
            {
              LayoutConstraints.ExplicitAlignment = *Value;
            }
            else
            {
              LayoutConstraints.ExplicitOffset = *Value;
            }
          }
          std::optional<std::vector<Attribute>> Attributes = parseAttributes();
          if (!Attributes.has_value())
          {
            return std::nullopt;
          }
          return StructField(std::move(Name), *FieldType, std::move(*Attributes), std::move(LayoutConstraints));
        }

        bool reserveGlobalSymbol(const Token &Name)
        {
          if (!Draft.GlobalSymbolNames.insert(Name.Text).second)
          {
            return fail<DiagnosticKind::IrDuplicateGlobalSymbol>(Name, Name.Text);
          }
          return true;
        }

        bool parseByteConstant()
        {
          const Token *Name = expect(TokenKind::GlobalName, "a global symbol name");
          if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::Equal, "'='") == nullptr || !expectIdentifier("private") || !expectIdentifier("constant") || expect(TokenKind::LeftBracket, "'['") == nullptr)
          {
            return false;
          }
          const Token *SizeToken = expect(TokenKind::Integer, "a byte constant size");
          if (SizeToken == nullptr)
          {
            return false;
          }
          const std::optional<std::size_t> DeclaredSize = parseIndex(*SizeToken, "byte constant size");
          if (!DeclaredSize || !expectIdentifier("x") || !expectIdentifier("byte") || expect(TokenKind::RightBracket, "']'") == nullptr || !expectIdentifier("c"))
          {
            return false;
          }
          const Token *Data = expect(TokenKind::String, "a byte string literal");
          if (Data == nullptr)
          {
            return false;
          }
          if (Data->Text.size() != *DeclaredSize)
          {
            return fail<DiagnosticKind::IrByteConstantSizeMismatch>(*SizeToken, *DeclaredSize, Data->Text.size());
          }
          const ByteConstantId Id{Builder.module().ByteConstants.size()};
          Draft.ByteConstantNames.emplace(Name->Text, Id);
          Builder.addByteConstant(Name->Text, Data->Text);
          return true;
        }

        bool parseGlobalVariable()
        {
          const Token *Name = expect(TokenKind::GlobalName, "a global variable name");
          if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::Equal, "'='") == nullptr || !expectIdentifier("global"))
          {
            return false;
          }

          bool Mutable;
          if (atIdentifier("mutable"))
          {
            consume();
            Mutable = true;
          }
          else if (atIdentifier("constant"))
          {
            consume();
            Mutable = false;
          }
          else
          {
            return fail<DiagnosticKind::IrExpected>(current(), "'mutable' or 'constant'");
          }

          const std::optional<const Type *> ValueType = parseType();
          if (!ValueType)
          {
            return false;
          }
          const GlobalId Id{Builder.module().Globals.size()};
          Draft.GlobalVariableNames.emplace(Name->Text, Id);
          Builder.addGlobal({Name->Text, *ValueType, Mutable});
          return true;
        }

        bool parseImportedGlobalVariable()
        {
          if (!expectIdentifier("declare") || !expectIdentifier("import") || !expectIdentifier("global"))
          {
            return false;
          }
          bool Mutable;
          if (atIdentifier("mutable"))
          {
            consume();
            Mutable = true;
          }
          else if (atIdentifier("constant"))
          {
            consume();
            Mutable = false;
          }
          else
          {
            return fail<DiagnosticKind::IrExpected>(current(), "'mutable' or 'constant'");
          }
          const std::optional<const Type *> ValueType = parseType();
          if (!ValueType)
          {
            return false;
          }
          const Token *Name = expect(TokenKind::GlobalName, "an imported global variable name");
          if (Name == nullptr || !reserveGlobalSymbol(*Name) || !expectIdentifier("from") || !expectIdentifier("module"))
          {
            return false;
          }
          const Token *Module = expect(TokenKind::Identifier, "an imported module name");
          if (Module == nullptr || expect(TokenKind::Comma, "','") == nullptr || !expectIdentifier("symbol"))
          {
            return false;
          }
          const Token *ImportName = expect(TokenKind::GlobalName, "an imported target global variable name");
          if (ImportName == nullptr)
          {
            return false;
          }
          GlobalVariable Global;
          Global.Name = Name->Text;
          Global.ValueType = *ValueType;
          Global.Mutable = Mutable;
          Global.Kind = GlobalVariableKind::Imported;
          Global.Import = ImportInfo{Module->Text, ImportName->Text};
          const GlobalId Id{Builder.module().Globals.size()};
          Draft.GlobalVariableNames.emplace(Global.Name, Id);
          Builder.addGlobal(std::move(Global));
          return true;
        }

        std::optional<Parameter> parseParameter(bool IncludeSsaName, std::size_t ParameterIndex)
        {
          ink::ir::Name ParameterName;
          if (at(TokenKind::Identifier) && at(TokenKind::Colon, 1))
          {
            ParameterName = consume().Text;
            consume();
          }
          const std::optional<const Type *> ParameterType = parseType();
          if (!ParameterType.has_value())
          {
            return std::nullopt;
          }
          if (IncludeSsaName)
          {
            const Token *ValueName = expect(TokenKind::ValueName, "a function parameter SSA value");
            if (ValueName == nullptr)
            {
              return std::nullopt;
            }
            const std::optional<std::size_t> ParsedParameterIndex = parseId(*ValueName, "function parameter SSA value");
            if (!ParsedParameterIndex.has_value())
            {
              return std::nullopt;
            }
            if (*ParsedParameterIndex != ParameterIndex)
            {
              fail<DiagnosticKind::IrNonConsecutiveParameterSsa>(*ValueName, ParameterIndex, *ParsedParameterIndex);
              return std::nullopt;
            }
          }
          const Constant *DefaultValue = nullptr;
          if (at(TokenKind::Equal))
          {
            consume();
            const Token ValueLocation = current();
            const std::optional<const Type *> DefaultType = parseType();
            if (!DefaultType.has_value())
            {
              return std::nullopt;
            }
            const std::optional<const Constant *> ParsedDefaultValue = parseConstantValue(**DefaultType);
            if (!ParsedDefaultValue.has_value())
            {
              fail<DiagnosticKind::IrExpected>(ValueLocation, "a constant function parameter default value");
              return std::nullopt;
            }
            DefaultValue = *ParsedDefaultValue;
          }
          return Parameter(std::move(ParameterName), *ParameterType, DefaultValue);
        }

        std::optional<std::vector<Parameter>> parseParameters(bool IncludeSsaNames)
        {
          std::vector<Parameter> Result;
          if (at(TokenKind::RightParenthesis))
          {
            return Result;
          }
          while (true)
          {
            std::optional<Parameter> ParameterValue = parseParameter(IncludeSsaNames, Result.size());
            if (!ParameterValue.has_value())
            {
              return std::nullopt;
            }
            Result.push_back(std::move(*ParameterValue));
            if (!at(TokenKind::Comma))
            {
              break;
            }
            consume();
          }
          return Result;
        }

        bool parseExternalFunction()
        {
          if (!expectIdentifier("declare") || !expectIdentifier("extern"))
          {
            return false;
          }
          const Token *Convention = expect(TokenKind::String, "the external calling convention");
          if (Convention == nullptr)
          {
            return false;
          }
          if (Convention->Text != "C")
          {
            return fail<DiagnosticKind::IrUnsupportedCallingConvention>(*Convention, Convention->Text);
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return false;
          }
          Function FunctionValue(**ResultType);
          FunctionValue.Kind = FunctionKind::External;
          FunctionValue.Convention = CallingConvention::C;
          const Token *Name = expect(TokenKind::GlobalName, "an external function name");
          if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
          {
            return false;
          }
          FunctionValue.Name = Name->Text;
          std::optional<std::vector<Parameter>> Parameters = parseParameters(false);
          if (!Parameters.has_value() || expect(TokenKind::RightParenthesis, "')'") == nullptr)
          {
            return false;
          }
          FunctionValue.Parameters = std::move(*Parameters);
          std::optional<std::vector<Attribute>> Attributes = parseAttributes();
          if (!Attributes.has_value())
          {
            return false;
          }
          FunctionValue.Attributes = std::move(*Attributes);
          const FunctionId Id{Builder.module().Functions.size()};
          Draft.FunctionNames.emplace(FunctionValue.Name, Id);
          Builder.addFunction(std::move(FunctionValue));
          return true;
        }

        bool parseImportedFunction()
        {
          if (!expectIdentifier("declare") || !expectIdentifier("import"))
          {
            return false;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return false;
          }
          Function FunctionValue(**ResultType);
          FunctionValue.Kind = FunctionKind::Imported;
          const Token *Name = expect(TokenKind::GlobalName, "an imported function name");
          if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
          {
            return false;
          }
          FunctionValue.Name = Name->Text;
          std::optional<std::vector<Parameter>> Parameters = parseParameters(false);
          if (!Parameters.has_value() || expect(TokenKind::RightParenthesis, "')'") == nullptr || !expectIdentifier("from") || !expectIdentifier("module"))
          {
            return false;
          }
          FunctionValue.Parameters = std::move(*Parameters);
          const Token *Module = expect(TokenKind::Identifier, "an imported module name");
          if (Module == nullptr || expect(TokenKind::Comma, "','") == nullptr || !expectIdentifier("symbol"))
          {
            return false;
          }
          const Token *ImportName = expect(TokenKind::GlobalName, "an imported target function name");
          if (ImportName == nullptr)
          {
            return false;
          }
          FunctionValue.Import = ImportInfo{Module->Text, ImportName->Text};
          std::optional<std::vector<Attribute>> Attributes = parseAttributes();
          if (!Attributes.has_value())
          {
            return false;
          }
          FunctionValue.Attributes = std::move(*Attributes);
          const FunctionId Id{Builder.module().Functions.size()};
          Draft.FunctionNames.emplace(FunctionValue.Name, Id);
          Builder.addFunction(std::move(FunctionValue));
          return true;
        }

        bool startsBasicBlock() const
        {
          return at(TokenKind::Identifier) && at(TokenKind::Colon, 1);
        }

        bool startsConstantValue() const
        {
          return at(TokenKind::Integer) || atIdentifier("floatbits") || (atIdentifier("c") && at(TokenKind::String, 1)) || atIdentifier("null") || at(TokenKind::LeftBrace) || atIdentifier("zeroinitializer");
        }

        std::optional<const Constant *> parseConstantValue(const Type &ConstantType)
        {
          if (at(TokenKind::Integer))
          {
            const Token &Integer = consume();
            if (ConstantType.kind() == TypeKind::PointerSize && (Integer.Text.empty() || Integer.Text.front() != '-'))
            {
              const std::optional<std::uint64_t> Value = parseUnsigned(Integer, "integer constant");
              return Value.has_value() ? std::optional<const Constant *>(&Context.constantPool().getIntegerConstant(ConstantType, *Value)) : std::nullopt;
            }
            const std::optional<std::int64_t> Value = parseSigned(Integer, "integer constant");
            return Value.has_value() ? std::optional<const Constant *>(&Context.constantPool().getIntegerConstant(ConstantType, *Value)) : std::nullopt;
          }
          if (atIdentifier("floatbits"))
          {
            consume();
            if (expect(TokenKind::LeftParenthesis, "'(' after 'floatbits'") == nullptr)
            {
              return std::nullopt;
            }
            const std::optional<FloatFormat> Format = parseFloatFormat();
            if (!Format || expect(TokenKind::Comma, "',' after a floating-point format") == nullptr)
            {
              return std::nullopt;
            }
            const Token *BitPatternToken = expect(TokenKind::Hexadecimal, "a hexadecimal floating-point bit pattern");
            if (BitPatternToken == nullptr)
            {
              return std::nullopt;
            }
            const std::size_t ExpectedDigits = floatFormatBitWidth(*Format) / 4;
            if (BitPatternToken->Text.size() != ExpectedDigits)
            {
              fail<DiagnosticKind::IrFloatBitPatternWidthMismatch>(*BitPatternToken, floatFormatName(*Format), ExpectedDigits, BitPatternToken->Text.size());
              return std::nullopt;
            }
            const std::optional<std::uint64_t> BitPattern = parseHexadecimal(*BitPatternToken, "floating-point bit pattern");
            if (!BitPattern || expect(TokenKind::RightParenthesis, "')' after a floating-point bit pattern") == nullptr)
            {
              return std::nullopt;
            }
            return &Context.constantPool().getFloatConstant(ConstantType, *Format, *BitPattern);
          }
          if (atIdentifier("c") && at(TokenKind::String, 1))
          {
            consume();
            return &Context.constantPool().getStringConstant(ConstantType, consume().Text);
          }
          if (atIdentifier("null"))
          {
            consume();
            return &Context.constantPool().getNullConstant(ConstantType);
          }
          if (at(TokenKind::LeftBrace))
          {
            consume();
            std::vector<std::reference_wrapper<const Constant>> Elements;
            if (!at(TokenKind::RightBrace))
            {
              while (true)
              {
                const std::optional<const Type *> ElementType = parseType();
                if (!ElementType.has_value())
                {
                  return std::nullopt;
                }
                const std::optional<const Constant *> Element = parseConstantValue(**ElementType);
                if (!Element.has_value())
                {
                  if (!startsConstantValue())
                  {
                    fail<DiagnosticKind::IrExpected>(current(), "a constant aggregate element");
                  }
                  return std::nullopt;
                }
                Elements.emplace_back(**Element);
                if (!at(TokenKind::Comma))
                {
                  break;
                }
                consume();
              }
            }
            if (expect(TokenKind::RightBrace, "'}' after an aggregate constant") == nullptr)
            {
              return std::nullopt;
            }
            return &Context.constantPool().getAggregateConstant(ConstantType, Elements);
          }
          if (atIdentifier("zeroinitializer"))
          {
            consume();
            return &Context.constantPool().getZeroInitializer(ConstantType);
          }
          return std::nullopt;
        }

        std::optional<ParsedOperand> parseOperandValue(const Type &OperandType)
        {
          ParsedOperand Result;
          Result.Location = current();
          if (at(TokenKind::ValueName))
          {
            const Token &Value = consume();
            const std::optional<std::size_t> Id = parseId(Value, "SSA value");
            if (!Id)
            {
              return std::nullopt;
            }
            Result.ParsedValue = std::make_unique<ValueOperand>(OperandType, ValueId{*Id});
            return Result;
          }
          if (startsConstantValue())
          {
            const std::optional<const Constant *> ConstantValue = parseConstantValue(OperandType);
            if (!ConstantValue.has_value())
            {
              return std::nullopt;
            }
            Result.ParsedValue = **ConstantValue;
            return Result;
          }
          if (at(TokenKind::GlobalName))
          {
            const Token &Global = consume();
            if (!at(TokenKind::LeftBracket))
            {
              auto Address = std::make_unique<GlobalVariableAddressOperand>(OperandType, GlobalId{});
              Draft.GlobalVariableFixups.push_back({Address.get(), Global.Text, Global});
              Result.ParsedValue = std::move(Address);
              Result.Location = Global;
              return Result;
            }
            consume();
            const Token *Offset = expect(TokenKind::Integer, "a global byte offset");
            if (Offset == nullptr)
            {
              return std::nullopt;
            }
            const std::optional<std::size_t> ByteOffset = parseIndex(*Offset, "global byte offset");
            if (!ByteOffset || expect(TokenKind::RightBracket, "']'") == nullptr)
            {
              return std::nullopt;
            }
            auto Address = std::make_unique<GlobalAddressOperand>(OperandType, ByteConstantId{}, *ByteOffset);
            Draft.GlobalFixups.push_back({Address.get(), Global.Text, Global});
            Result.ParsedValue = std::move(Address);
            Result.Location = Global;
            return Result;
          }
          fail<DiagnosticKind::IrExpectedOperand>(current());
          return std::nullopt;
        }

        std::optional<ParsedOperand> parseOperand()
        {
          const std::optional<const Type *> Type = parseType();
          if (!Type)
          {
            return std::nullopt;
          }
          return parseOperandValue(**Type);
        }

        std::optional<std::vector<ParsedOperand>> parseArguments()
        {
          std::vector<ParsedOperand> Result;
          if (at(TokenKind::RightParenthesis))
          {
            return Result;
          }
          std::optional<ParsedOperand> FirstOperand = parseOperand();
          if (!FirstOperand)
          {
            return std::nullopt;
          }
          Result.push_back(std::move(*FirstOperand));
          while (at(TokenKind::Comma))
          {
            consume();
            std::optional<ParsedOperand> Operand = parseOperand();
            if (!Operand)
            {
              return std::nullopt;
            }
            Result.push_back(std::move(*Operand));
          }
          return Result;
        }

        std::unique_ptr<Instruction> parseCall(const InstructionParseState &State)
        {
          if (!expectIdentifier("call"))
          {
            return nullptr;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return nullptr;
          }
          auto Call = std::make_unique<CallInstruction>(**ResultType);
          Call->Result = State.Result;
          const Token *Callee = expect(TokenKind::GlobalName, "a declared function name");
          if (Callee == nullptr)
          {
            return nullptr;
          }
          Draft.CallFixups.push_back({Call.get(), Callee->Text, *Callee});
          if (expect(TokenKind::LeftParenthesis, "'('") == nullptr)
          {
            return nullptr;
          }
          std::optional<std::vector<ParsedOperand>> Arguments = parseArguments();
          if (!Arguments || expect(TokenKind::RightParenthesis, "')'") == nullptr)
          {
            return nullptr;
          }
          for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments->size(); ++ArgumentIndex)
          {
            Call->Arguments.push_back(std::move((*Arguments)[ArgumentIndex].ParsedValue));
          }
          return Call;
        }

        std::unique_ptr<Instruction> parseImport(const InstructionParseState &)
        {
          if (!expectIdentifier("import"))
          {
            return nullptr;
          }
          const Token *Module = expect(TokenKind::Identifier, "a module name");
          if (Module == nullptr)
          {
            return nullptr;
          }
          return std::make_unique<ImportInstruction>(Module->Text);
        }

        std::unique_ptr<Instruction> parseReturn(const InstructionParseState &)
        {
          if (!expectIdentifier("ret"))
          {
            return nullptr;
          }
          auto Return = std::make_unique<ReturnInstruction>();
          if (atIdentifier("void"))
          {
            consume();
            return Return;
          }
          std::optional<ParsedOperand> Value = parseOperand();
          if (!Value)
          {
            return nullptr;
          }
          Return->ReturnValue = std::move(Value->ParsedValue);
          return Return;
        }

        std::unique_ptr<Instruction> parseInsertValue(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrInsertValueRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("insertvalue"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Aggregate = parseOperand();
          if (!Aggregate || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Element = parseOperand();
          if (!Element || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          const Token *FieldIndex = expect(TokenKind::Integer, "a struct field index");
          if (FieldIndex == nullptr)
          {
            return nullptr;
          }
          const std::optional<std::size_t> ParsedFieldIndex = parseIndex(*FieldIndex, "struct field index");
          if (!ParsedFieldIndex)
          {
            return nullptr;
          }

          auto Insert = std::make_unique<InsertValueInstruction>(Aggregate->ParsedValue->type());
          Insert->Result = *State.Result;
          Insert->Aggregate = std::move(Aggregate->ParsedValue);
          Insert->Element = std::move(Element->ParsedValue);
          Insert->FieldIndex = *ParsedFieldIndex;
          return Insert;
        }

        std::unique_ptr<Instruction> parseExtractValue(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrExtractValueRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("extractvalue"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Aggregate = parseOperand();
          if (!Aggregate || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          const Token *FieldIndex = expect(TokenKind::Integer, "a struct field index");
          if (FieldIndex == nullptr)
          {
            return nullptr;
          }
          const std::optional<std::size_t> ParsedFieldIndex = parseIndex(*FieldIndex, "struct field index");
          if (!ParsedFieldIndex)
          {
            return nullptr;
          }
          if (Aggregate->ParsedValue->type().kind() != TypeKind::Struct)
          {
            fail<DiagnosticKind::IrExtractAggregateMustBeStruct>(Aggregate->Location);
            return nullptr;
          }
          const StructType &AggregateType = static_cast<const StructType &>(Aggregate->ParsedValue->type());
          if (*ParsedFieldIndex >= AggregateType.fieldCount())
          {
            fail<DiagnosticKind::IrFieldIndexOutOfRange>(*FieldIndex, *ParsedFieldIndex, AggregateType.fieldCount());
            return nullptr;
          }

          auto Extract = std::make_unique<ExtractValueInstruction>(*AggregateType.fieldType(*ParsedFieldIndex));
          Extract->Result = *State.Result;
          Extract->Aggregate = std::move(Aggregate->ParsedValue);
          Extract->FieldIndex = *ParsedFieldIndex;
          return Extract;
        }

        std::unique_ptr<Instruction> parseAlloca(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrAllocaRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("alloca"))
          {
            return nullptr;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Size = parseOperand();
          if (!Size)
          {
            return nullptr;
          }
          auto Alloca = std::make_unique<AllocaInstruction>(**ResultType);
          Alloca->Result = *State.Result;
          Alloca->Size = std::move(Size->ParsedValue);
          return Alloca;
        }

        std::unique_ptr<Instruction> parseLoad(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrLoadRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("load"))
          {
            return nullptr;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Pointer = parseOperand();
          if (!Pointer)
          {
            return nullptr;
          }
          auto Load = std::make_unique<LoadInstruction>(**ResultType);
          Load->Result = *State.Result;
          Load->Pointer = std::move(Pointer->ParsedValue);
          return Load;
        }

        std::unique_ptr<Instruction> parseGetElementPointer(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrGetElementPointerRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("getelementptr"))
          {
            return nullptr;
          }
          const std::optional<const Type *> ElementType = parseType();
          if (!ElementType || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Pointer = parseOperand();
          if (!Pointer || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> IndexValue = parseOperand();
          if (!IndexValue)
          {
            return nullptr;
          }
          auto GetElementPointer = std::make_unique<GetElementPointerInstruction>(Pointer->ParsedValue->type(), **ElementType);
          GetElementPointer->Result = *State.Result;
          GetElementPointer->Pointer = std::move(Pointer->ParsedValue);
          GetElementPointer->Index = std::move(IndexValue->ParsedValue);
          while (at(TokenKind::Comma))
          {
            consume();
            std::optional<ParsedOperand> FieldIndex = parseOperand();
            if (!FieldIndex)
            {
              return nullptr;
            }
            GetElementPointer->FieldIndices.push_back(std::move(FieldIndex->ParsedValue));
          }
          return GetElementPointer;
        }

        std::unique_ptr<Instruction> parseStore(const InstructionParseState &)
        {
          if (!expectIdentifier("store"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> StoredValue = parseOperand();
          if (!StoredValue || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Pointer = parseOperand();
          if (!Pointer)
          {
            return nullptr;
          }
          auto Store = std::make_unique<StoreInstruction>();
          Store->StoredValue = std::move(StoredValue->ParsedValue);
          Store->Pointer = std::move(Pointer->ParsedValue);
          return Store;
        }

        std::unique_ptr<Instruction> parseLifetimeEnd(const InstructionParseState &)
        {
          if (!expectIdentifier("lifetime.end"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Slice = parseOperand();
          if (!Slice)
          {
            return nullptr;
          }
          auto LifetimeEnd = std::make_unique<LifetimeEndInstruction>();
          LifetimeEnd->Slice = std::move(Slice->ParsedValue);
          return LifetimeEnd;
        }

        std::unique_ptr<Instruction> parseSliceData(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrSliceDataRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("slice.data"))
          {
            return nullptr;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Slice = parseOperand();
          if (!Slice)
          {
            return nullptr;
          }
          auto SliceData = std::make_unique<SliceDataInstruction>(**ResultType);
          SliceData->Result = *State.Result;
          SliceData->Slice = std::move(Slice->ParsedValue);
          return SliceData;
        }

        std::unique_ptr<Instruction> parseSliceLength(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrSliceLengthRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("slice.length"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Slice = parseOperand();
          if (!Slice)
          {
            return nullptr;
          }
          auto SliceLength = std::make_unique<SliceLengthInstruction>(Context.getType(TypeKind::PointerSize));
          SliceLength->Result = *State.Result;
          SliceLength->Slice = std::move(Slice->ParsedValue);
          return SliceLength;
        }

        std::unique_ptr<Instruction> parseAdd(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrAddRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("add"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Left = parseOperand();
          if (!Left || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Right = parseOperand();
          if (!Right)
          {
            return nullptr;
          }
          auto Add = std::make_unique<AddInstruction>(Left->ParsedValue->type());
          Add->Result = *State.Result;
          Add->Left = std::move(Left->ParsedValue);
          Add->Right = std::move(Right->ParsedValue);
          return Add;
        }

        std::optional<ComparePredicate> parseComparePredicate()
        {
          const Token *Predicate = expect(TokenKind::Identifier, "an integer comparison predicate");
          if (Predicate == nullptr)
          {
            return std::nullopt;
          }
          if (Predicate->Text == "eq")
          {
            return ComparePredicate::Equal;
          }
          if (Predicate->Text == "ne")
          {
            return ComparePredicate::NotEqual;
          }
          if (Predicate->Text == "lt")
          {
            return ComparePredicate::LessThan;
          }
          if (Predicate->Text == "le")
          {
            return ComparePredicate::LessEqual;
          }
          if (Predicate->Text == "gt")
          {
            return ComparePredicate::GreaterThan;
          }
          if (Predicate->Text == "ge")
          {
            return ComparePredicate::GreaterEqual;
          }
          fail<DiagnosticKind::IrUnknownComparePredicate>(*Predicate, Predicate->Text);
          return std::nullopt;
        }

        std::unique_ptr<Instruction> parseCompare(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrCompareRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("icmp"))
          {
            return nullptr;
          }
          const std::optional<ComparePredicate> Predicate = parseComparePredicate();
          if (!Predicate)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Left = parseOperand();
          if (!Left || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Right = parseOperand();
          if (!Right)
          {
            return nullptr;
          }
          auto Compare = std::make_unique<CompareInstruction>(Context.getType(TypeKind::Bool));
          Compare->Result = *State.Result;
          Compare->Predicate = *Predicate;
          Compare->Left = std::move(Left->ParsedValue);
          Compare->Right = std::move(Right->ParsedValue);
          return Compare;
        }

        std::unique_ptr<Instruction> parsePhi(const InstructionParseState &State)
        {
          if (!State.Result.has_value())
          {
            fail<DiagnosticKind::IrPhiRequiresResult>(current());
            return nullptr;
          }
          if (!expectIdentifier("phi"))
          {
            return nullptr;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return nullptr;
          }

          auto Phi = std::make_unique<PhiInstruction>(**ResultType);
          Phi->Result = *State.Result;
          while (true)
          {
            if (expect(TokenKind::LeftBracket, "'[' to begin a phi incoming value") == nullptr)
            {
              return nullptr;
            }
            std::optional<ParsedOperand> IncomingValue = parseOperandValue(**ResultType);
            if (!IncomingValue || expect(TokenKind::Comma, "',' between a phi value and predecessor") == nullptr)
            {
              return nullptr;
            }
            const Token *Predecessor = expect(TokenKind::Identifier, "a phi predecessor block");
            if (Predecessor == nullptr || expect(TokenKind::RightBracket, "']' to end a phi incoming value") == nullptr)
            {
              return nullptr;
            }

            const std::size_t IncomingIndex = Phi->IncomingValues.size();
            Phi->IncomingValues.push_back({std::move(IncomingValue->ParsedValue), BlockId{}});
            Draft.PhiFixups.push_back({State.Function, Phi.get(), IncomingIndex, Predecessor->Text, *Predecessor});
            if (!at(TokenKind::Comma))
            {
              break;
            }
            consume();
          }
          return Phi;
        }

        std::optional<ParsedBlockTarget> parseBlockTarget()
        {
          const Token *Name = expect(TokenKind::Identifier, "a basic block target");
          if (Name == nullptr)
          {
            return std::nullopt;
          }
          return ParsedBlockTarget{{}, Name->Text, *Name};
        }

        std::unique_ptr<Instruction> parseBranch(const InstructionParseState &State)
        {
          if (!expectIdentifier("br"))
          {
            return nullptr;
          }
          std::optional<ParsedBlockTarget> Target = parseBlockTarget();
          if (!Target)
          {
            return nullptr;
          }
          auto Branch = std::make_unique<BranchInstruction>();
          Branch->Target = std::move(Target->Target);
          Draft.BranchFixups.push_back({State.Function, Branch.get(), BranchFixupTarget::Branch, std::move(Target->BlockName), std::move(Target->Location)});
          return Branch;
        }

        std::unique_ptr<Instruction> parseConditionalBranch(const InstructionParseState &State)
        {
          if (!expectIdentifier("condbr"))
          {
            return nullptr;
          }
          std::optional<ParsedOperand> Condition = parseOperand();
          if (!Condition || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedBlockTarget> TrueTarget = parseBlockTarget();
          if (!TrueTarget || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedBlockTarget> FalseTarget = parseBlockTarget();
          if (!FalseTarget)
          {
            return nullptr;
          }
          auto Branch = std::make_unique<ConditionalBranchInstruction>();
          Branch->Condition = std::move(Condition->ParsedValue);
          Branch->TrueTarget = std::move(TrueTarget->Target);
          Branch->FalseTarget = std::move(FalseTarget->Target);
          Draft.BranchFixups.push_back({State.Function, Branch.get(), BranchFixupTarget::ConditionalTrue, std::move(TrueTarget->BlockName), std::move(TrueTarget->Location)});
          Draft.BranchFixups.push_back({State.Function, Branch.get(), BranchFixupTarget::ConditionalFalse, std::move(FalseTarget->BlockName), std::move(FalseTarget->Location)});
          return Branch;
        }

        std::unique_ptr<Instruction> parseInstruction(FunctionId Function)
        {
          std::optional<ValueId> ResultValue;
          if (at(TokenKind::ValueName))
          {
            const Token &Result = consume();
            const std::optional<std::size_t> Id = parseId(Result, "SSA result");
            if (!Id || expect(TokenKind::Equal, "'=' after an SSA result") == nullptr)
            {
              return nullptr;
            }
            ResultValue = ValueId{*Id};
          }
          const std::optional<InstructionKind> Kind = at(TokenKind::Identifier) ? instructionKindFromMnemonic(current().Text) : std::nullopt;
          if (!Kind.has_value())
          {
            if (ResultValue.has_value())
            {
              fail<DiagnosticKind::IrExpectedValueProducingInstruction>(current());
            }
            else
            {
              fail<DiagnosticKind::IrExpectedInstruction>(current());
            }
            return nullptr;
          }
          if (ResultValue.has_value() && instructionResultPolicy(*Kind) == InstructionResultPolicy::Forbidden)
          {
            fail<DiagnosticKind::IrInstructionCannotDefineResult>(current(), current().Text);
            return nullptr;
          }
          const InstructionParseState State{Function, ResultValue};
          switch (*Kind)
          {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                                        \
    return parse##Name(State);
#include "ink/ir/ir.def"
          }
          return nullptr;
        }

        std::optional<BasicBlock> parseBasicBlock(FunctionId Function)
        {
          BasicBlock Result;
          const Token *Name = expect(TokenKind::Identifier, "a basic block label");
          if (Name == nullptr)
          {
            return std::nullopt;
          }
          Result.Name = Name->Text;
          if (expect(TokenKind::Colon, "':' after a basic block label") == nullptr)
          {
            return std::nullopt;
          }
          while (!at(TokenKind::RightBrace) && !startsBasicBlock())
          {
            std::unique_ptr<Instruction> InstructionValue = parseInstruction(Function);
            if (!InstructionValue)
            {
              return std::nullopt;
            }
            Result.Instructions.push_back(std::move(InstructionValue));
          }
          return Result;
        }

        bool parseFunctionDefinition()
        {
          if (!expectIdentifier("define"))
          {
            return false;
          }
          const std::optional<const Type *> ResultType = parseType();
          if (!ResultType)
          {
            return false;
          }
          Function FunctionValue(**ResultType);
          const Token *Name = expect(TokenKind::GlobalName, "a function name");
          if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
          {
            return false;
          }
          FunctionValue.Name = Name->Text;
          std::optional<std::vector<Parameter>> Parameters = parseParameters(true);
          if (!Parameters.has_value() || expect(TokenKind::RightParenthesis, "')'") == nullptr)
          {
            return false;
          }
          FunctionValue.Parameters = std::move(*Parameters);
          std::optional<std::vector<Attribute>> Attributes = parseAttributes();
          if (!Attributes.has_value() || expect(TokenKind::LeftBrace, "'{'") == nullptr)
          {
            return false;
          }
          FunctionValue.Attributes = std::move(*Attributes);
          const std::size_t FunctionIndex = Builder.module().Functions.size();
          while (!at(TokenKind::RightBrace))
          {
            if (at(TokenKind::End))
            {
              return fail<DiagnosticKind::IrUnterminatedFunctionDefinition>(current());
            }
            std::optional<BasicBlock> Block = parseBasicBlock(FunctionId{FunctionIndex});
            if (!Block)
            {
              return false;
            }
            FunctionValue.Blocks.push_back(std::move(*Block));
          }
          consume();
          const FunctionId Id{Builder.module().Functions.size()};
          Draft.FunctionNames.emplace(FunctionValue.Name, Id);
          Builder.addFunction(std::move(FunctionValue));
          return true;
        }

        IRContext &Context;
        ModuleDraft &Draft;
        IRBuilder &Builder;
        std::vector<Token> Tokens;
        std::size_t Index = 0;
        Diagnostic ParseDiagnostic;
    };

  } // namespace

  bool text::parse(ModuleDraft &Draft, std::vector<Token> Tokens, core::Diagnostic &Error)
  {
    return TextParser(Draft.Builder.context(), Draft, std::move(Tokens)).parse(Error);
  }

} // namespace ink::ir
