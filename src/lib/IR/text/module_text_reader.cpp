#include "ink/ir/text/module_text.h"

#include "module_text_format.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/verification/verifier.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::ir
{
    namespace
    {
        using detail::TypeSyntaxKind;

        constexpr bool isSpace(char Value) noexcept
        {
            return Value == ' ' || Value == '\t' || Value == '\n' || Value == '\r' || Value == '\f' || Value == '\v';
        }

        constexpr bool isWord(char Value) noexcept
        {
            return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z') || (Value >= '0' && Value <= '9') || Value == '_' || Value == '.' || Value == '$';
        }

        constexpr int hexDigit(char Value) noexcept
        {
            if (Value >= '0' && Value <= '9')
            {
                return Value - '0';
            }
            if (Value >= 'a' && Value <= 'f')
            {
                return Value - 'a' + 10;
            }
            if (Value >= 'A' && Value <= 'F')
            {
                return Value - 'A' + 10;
            }
            return -1;
        }

        bool parseUnsignedWord(std::string_view Text, std::uint64_t& Value) noexcept
        {
            int Base = 10;
            if (Text.starts_with("0x") || Text.starts_with("0X"))
            {
                Base = 16;
                Text.remove_prefix(2);
            }
            if (Text.empty())
            {
                return false;
            }
            const auto Result = std::from_chars(Text.data(), Text.data() + Text.size(), Value, Base);
            return Result.ec == std::errc{} && Result.ptr == Text.data() + Text.size();
        }

        enum class TokenKind : std::uint8_t
        {
            Word,
            Reference,
            String,
            ByteString,
            Punctuation,
            End,
        };

        struct Token
        {
                TokenKind Kind = TokenKind::End;
                std::string Text = {};
                std::size_t Offset = 0;
        };

        class Lexer final
        {
            public:
                explicit Lexer(std::string_view Input)
                    : Input(Input)
                {
                    if (Input.size() > core::SourceLocation::MaxByteOffset)
                    {
                        fail(0, "IR text exceeds the supported source offset range");
                    }
                    Current = lexNextToken();
                }

                const Token& current() const noexcept
                {
                    return Current;
                }

                const Token& peekNext()
                {
                    if (!Lookahead)
                    {
                        Lookahead = lexNextToken();
                    }
                    return *Lookahead;
                }

                void advance()
                {
                    if (Lookahead)
                    {
                        Current = std::move(*Lookahead);
                        Lookahead.reset();
                    }
                    else
                    {
                        Current = lexNextToken();
                    }
                }

                bool fail(std::size_t Offset, std::string_view Description)
                {
                    if (!Failure)
                    {
                        Failure = detail::makeTextDiagnostic(Offset, Description);
                    }
                    return false;
                }

                bool failCurrent(std::string_view Description)
                {
                    return fail(Current.Offset, Description);
                }

                const std::optional<core::Diagnostic>& error() const noexcept
                {
                    return Failure;
                }

                bool hasError() const noexcept
                {
                    return Failure.has_value();
                }

            private:
                Token lexQuoted(std::size_t Start, TokenKind Kind)
                {
                    std::string Decoded;
                    while (Position < Input.size())
                    {
                        const unsigned char Byte = static_cast<unsigned char>(Input[Position++]);
                        if (Byte == '"')
                        {
                            return {Kind, std::move(Decoded), Start};
                        }
                        if (Byte < 0x20 || Byte == 0x7f)
                        {
                            fail(Position - 1, "control bytes in quoted text must be escaped");
                            return {TokenKind::End, {}, Position};
                        }
                        if (Byte != '\\')
                        {
                            Decoded.push_back(static_cast<char>(Byte));
                            continue;
                        }
                        if (Position + 1 < Input.size() && hexDigit(Input[Position]) >= 0 && hexDigit(Input[Position + 1]) >= 0)
                        {
                            Decoded.push_back(static_cast<char>((hexDigit(Input[Position]) << 4) | hexDigit(Input[Position + 1])));
                            Position += 2;
                            continue;
                        }
                        if (Position < Input.size() && (Input[Position] == '\\' || Input[Position] == '"'))
                        {
                            Decoded.push_back(Input[Position++]);
                            continue;
                        }
                        fail(Position - 1, "expected a two-digit hexadecimal byte escape");
                        return {TokenKind::End, {}, Position};
                    }
                    fail(Start, "unterminated quoted text");
                    return {TokenKind::End, {}, Position};
                }

                Token lexNextToken()
                {
                    while (Position < Input.size() && !Failure)
                    {
                        const char Character = Input[Position];
                        if (isSpace(Character))
                        {
                            ++Position;
                            continue;
                        }
                        if (Character == ';')
                        {
                            while (Position < Input.size() && Input[Position] != '\n' && Input[Position] != '\r')
                            {
                                ++Position;
                            }
                            continue;
                        }
                        const std::size_t Start = Position;
                        if (Character == 'c' && Position + 1 < Input.size() && Input[Position + 1] == '"')
                        {
                            Position += 2;
                            return lexQuoted(Start, TokenKind::ByteString);
                        }
                        if (Character == '"')
                        {
                            ++Position;
                            return lexQuoted(Start, TokenKind::String);
                        }
                        if (Character == '%' || Character == '@' || Character == '#')
                        {
                            ++Position;
                            while (Position < Input.size() && isWord(Input[Position]))
                            {
                                ++Position;
                            }
                            if (Position == Start + 1)
                            {
                                fail(Start, "expected an identifier after '%', '@', or '#'");
                                return {TokenKind::End, {}, Position};
                            }
                            return {TokenKind::Reference, std::string(Input.substr(Start, Position - Start)), Start};
                        }
                        if (Input.substr(Position, detail::EllipsisToken.size()) == detail::EllipsisToken)
                        {
                            Position += detail::EllipsisToken.size();
                            return {TokenKind::Punctuation, std::string(detail::EllipsisToken), Start};
                        }
                        if (isWord(Character))
                        {
                            while (Position < Input.size() && isWord(Input[Position]))
                            {
                                ++Position;
                            }
                            return {TokenKind::Word, std::string(Input.substr(Start, Position - Start)), Start};
                        }
                        if (Input.substr(Position, detail::ArrowToken.size()) == detail::ArrowToken)
                        {
                            Position += detail::ArrowToken.size();
                            return {TokenKind::Punctuation, std::string(detail::ArrowToken), Start};
                        }
                        if (std::string_view("{}()[]<>:,=").find(Character) != std::string_view::npos)
                        {
                            ++Position;
                            return {TokenKind::Punctuation, std::string(1, Character), Start};
                        }
                        fail(Position, "unrecognized byte in IR text");
                        return {TokenKind::End, {}, Position};
                    }
                    return {TokenKind::End, {}, Position};
                }

                std::string_view Input;
                std::size_t Position = 0;
                Token Current = {};
                std::optional<Token> Lookahead = std::nullopt;
                std::optional<core::Diagnostic> Failure = std::nullopt;
        };

        struct TypeNode;

        struct ParameterNode
        {
                Parameter::Kind Kind = Parameter::Kind::Positional;
                std::unique_ptr<TypeNode> Type = nullptr;
                std::string ArgumentId = {};
                std::string ArgumentName = {};
                std::size_t Offset = 0;
        };

        struct TypeNode
        {
                TypeSyntaxKind Kind = TypeSyntaxKind::Void;
                std::string Reference = {};
                std::uint64_t Count = 0;
                std::uint32_t Width = 0;
                bool Signed = true;
                CallABI ABI = {};
                bool Variadic = false;
                std::unique_ptr<TypeNode> Element = nullptr;
                std::vector<ParameterNode> Parameters = {};
                std::size_t Offset = 0;
        };

        enum class OperandValueKind : std::uint8_t
        {
            Reference,
            Unsigned,
            ByteString,
            TypeValue,
        };

        struct TypedOperand
        {
                TypeNode Type = {};
                OperandValueKind Kind = OperandValueKind::Reference;
                std::string Text = {};
                std::uint64_t Bits = 0;
                std::unique_ptr<TypeNode> TypeLiteral = nullptr;
                std::size_t Offset = 0;
        };

        struct IncomingNode
        {
                TypedOperand Value = {};
                std::string BlockId = {};
                std::size_t Offset = 0;
        };

        struct InstructionNode
        {
                std::string Id = {};
                InstructionKind Kind = InstructionKind::Return;
                TypeNode ResultType = {};
                std::vector<TypedOperand> Operands = {};
                std::string CalleeId = {};
                std::size_t CalleeOffset = 0;
                std::vector<std::string> Targets = {};
                std::vector<IncomingNode> Incoming = {};
                std::size_t Offset = 0;
        };

        struct BlockNode
        {
                std::string Id = {};
                std::string Name = {};
                std::vector<InstructionNode> Instructions = {};
                std::size_t Offset = 0;
        };

        struct ImportNode
        {
                std::string Id = {};
                std::string Path = {};
                std::size_t Offset = 0;
                std::size_t PathOffset = 0;
        };

        struct FunctionNode
        {
                std::string Id = {};
                std::string Name = {};
                std::string ImportId = {};
                TypeNode Signature = {};
                SymbolVisibility Visibility = SymbolVisibility::Private;
                NativeLinkage Linkage = NativeLinkage::None;
                std::vector<BlockNode> Blocks = {};
                std::size_t Offset = 0;
                std::size_t ImportOffset = 0;
        };

        struct FieldNode
        {
                std::string Name = {};
                TypeNode Type = {};
                MemberKind Kind = MemberKind::Instance;
                SymbolVisibility Visibility = SymbolVisibility::Public;
                std::size_t Offset = 0;
        };

        struct MethodNode
        {
                MemberKind Kind = MemberKind::Instance;
                FunctionNode Function = {};
                std::size_t Offset = 0;
        };

        struct RecordNode
        {
                std::string Id = {};
                std::string Name = {};
                std::string ImportId = {};
                bool IsObject = false;
                bool Registered = true;
                std::vector<TypeNode> Bases = {};
                std::vector<FieldNode> Fields = {};
                std::vector<MethodNode> Methods = {};
                std::size_t Offset = 0;
                std::size_t ImportOffset = 0;
        };

        struct GlobalNode
        {
                std::string Id = {};
                std::string Name = {};
                std::string ImportId = {};
                TypeNode StoredType = {};
                SymbolVisibility Visibility = SymbolVisibility::Private;
                NativeLinkage Linkage = NativeLinkage::None;
                std::size_t Offset = 0;
                std::size_t ImportOffset = 0;
        };

        TypeNode cloneTypeNode(const TypeNode& Source)
        {
            TypeNode Copy;
            Copy.Kind = Source.Kind;
            Copy.Reference = Source.Reference;
            Copy.Count = Source.Count;
            Copy.Width = Source.Width;
            Copy.Signed = Source.Signed;
            Copy.ABI = Source.ABI;
            Copy.Variadic = Source.Variadic;
            Copy.Offset = Source.Offset;
            if (Source.Element)
            {
                Copy.Element = std::make_unique<TypeNode>(cloneTypeNode(*Source.Element));
            }
            Copy.Parameters.reserve(Source.Parameters.size());
            for (const ParameterNode& Param : Source.Parameters)
            {
                ParameterNode ParamCopy;
                ParamCopy.Kind = Param.Kind;
                ParamCopy.Type = std::make_unique<TypeNode>(cloneTypeNode(*Param.Type));
                ParamCopy.ArgumentId = Param.ArgumentId;
                ParamCopy.ArgumentName = Param.ArgumentName;
                ParamCopy.Offset = Param.Offset;
                Copy.Parameters.push_back(std::move(ParamCopy));
            }
            return Copy;
        }

        // Unresolved instruction operands borrow these temporary values until every instruction in the module exists.
        class PendingValue final : public Value
        {
            public:
                explicit PendingValue(ValueRef ValueType) noexcept
                    : Value(ValueType)
                {
                }
        };

        class ModuleTextReader final
        {
            public:
                explicit ModuleTextReader(std::string_view Text)
                    : Input(Text)
                {
                }

                std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> read()
                {
                    if (!parseModule() || !buildModuleGraph())
                    {
                        return std::unexpected(*Input.error());
                    }
                    return std::move(Owned);
                }

            private:
                enum class DefinitionKind : std::uint8_t
                {
                    Record,
                    Function,
                    Global,
                    Argument,
                    Block,
                    Instruction,
                };

                struct DefinitionEntry
                {
                        DefinitionKind Kind;
                        const InstructionNode* Instruction = nullptr;
                };

                struct FunctionBuildState
                {
                        const FunctionNode* Syntax = nullptr;
                        RecordType* Record = nullptr;
                        MemberKind Kind = MemberKind::Instance;
                        Function* Value = nullptr;
                };

                struct BlockBuildState
                {
                        const BlockNode* Syntax = nullptr;
                        Function* Owner = nullptr;
                        BasicBlock* Value = nullptr;
                        std::size_t NextInstruction = 0;
                };

                struct InstructionBuildState
                {
                        const InstructionNode* Syntax = nullptr;
                        Instruction* Value = nullptr;
                };

                bool peek(std::string_view Text) const noexcept
                {
                    const Token& Tok = Input.current();
                    return !Input.hasError() && Tok.Kind != TokenKind::String && Tok.Kind != TokenKind::ByteString && Tok.Kind != TokenKind::End && Tok.Text == Text;
                }

                bool consume(std::string_view Expected)
                {
                    if (!peek(Expected))
                    {
                        return Input.failCurrent(std::string("expected '") + std::string(Expected) + "'");
                    }
                    Input.advance();
                    return true;
                }

                bool consumeIf(std::string_view Expected)
                {
                    if (peek(Expected))
                    {
                        Input.advance();
                        return true;
                    }
                    return false;
                }

                template <typename Enum, std::size_t Count>
                bool parseRequiredEnum(Enum& Out, const detail::EnumEntry<Enum> (&Entries)[Count], std::string_view ErrorMessage)
                {
                    if (Input.current().Kind == TokenKind::Word)
                    {
                        if (const auto Parsed = detail::parseEnum(Input.current().Text, Entries))
                        {
                            Out = *Parsed;
                            Input.advance();
                            return true;
                        }
                    }
                    return Input.failCurrent(ErrorMessage);
                }

                template <typename Enum, std::size_t Count>
                void parseOptionalEnum(Enum& Out, Enum DefaultValue, const detail::EnumEntry<Enum> (&Entries)[Count])
                {
                    Out = DefaultValue;
                    if (Input.current().Kind == TokenKind::Word)
                    {
                        if (const auto Parsed = detail::parseEnum(Input.current().Text, Entries))
                        {
                            Out = *Parsed;
                            Input.advance();
                        }
                    }
                }

                bool parseReference(std::string& Out)
                {
                    if (Input.hasError() || Input.current().Kind != TokenKind::Reference || Input.current().Text.starts_with('#'))
                    {
                        return Input.failCurrent("expected a textual identifier");
                    }
                    Out = Input.current().Text;
                    Input.advance();
                    return true;
                }

                bool parseImportReference(std::string& Out)
                {
                    if (Input.hasError() || Input.current().Kind != TokenKind::Reference || !Input.current().Text.starts_with('#'))
                    {
                        return Input.failCurrent("expected a '#' module import identifier");
                    }
                    Out = Input.current().Text;
                    Input.advance();
                    return true;
                }

                bool parseQuoted(std::string& Out)
                {
                    if (Input.hasError() || Input.current().Kind != TokenKind::String)
                    {
                        return Input.failCurrent("expected quoted text");
                    }
                    Out = Input.current().Text;
                    Input.advance();
                    return true;
                }

                bool parseOptionalName(std::string& Out)
                {
                    if (Input.hasError())
                    {
                        return false;
                    }
                    if (Input.current().Kind == TokenKind::String)
                    {
                        return parseQuoted(Out);
                    }
                    Out.clear();
                    return true;
                }

                bool parseUnsigned(std::uint64_t& Out)
                {
                    if (Input.hasError() || Input.current().Kind != TokenKind::Word || !parseUnsignedWord(Input.current().Text, Out))
                    {
                        return Input.failCurrent("expected an unsigned 64-bit decimal or hexadecimal integer");
                    }
                    Input.advance();
                    return true;
                }

                bool parseParameterList(TypeNode& Signature, bool BindArguments)
                {
                    Signature.Variadic = false;
                    if (peek(")"))
                    {
                        return true;
                    }
                    for (;;)
                    {
                        if (consumeIf(detail::EllipsisToken))
                        {
                            Signature.Variadic = true;
                            return peek(")") || Input.failCurrent("ellipsis must be the last item in a parameter list");
                        }
                        ParameterNode Param;
                        Param.Offset = Input.current().Offset;
                        parseOptionalEnum(Param.Kind, Parameter::Kind::Positional, detail::ParameterKindEntries);
                        Param.Type = std::make_unique<TypeNode>();
                        if (!parseType(*Param.Type))
                        {
                            return false;
                        }
                        if (BindArguments && (!parseReference(Param.ArgumentId) || !parseOptionalName(Param.ArgumentName)))
                        {
                            return false;
                        }
                        Signature.Parameters.push_back(std::move(Param));
                        if (peek(")"))
                        {
                            return true;
                        }
                        if (!consume(","))
                        {
                            return false;
                        }
                        if (peek(")"))
                        {
                            return Input.failCurrent("trailing comma in parameter list");
                        }
                    }
                }

                bool parseType(TypeNode& Out)
                {
                    if (TypeParseDepth >= detail::MaxTypeExpressionDepth)
                    {
                        return Input.failCurrent("type expression nesting exceeds 128 levels");
                    }
                    ++TypeParseDepth;
                    struct DepthGuard
                    {
                            std::size_t& Depth;

                            ~DepthGuard()
                            {
                                --Depth;
                            }
                    } Guard{TypeParseDepth};

                    Out = {};
                    Out.Offset = Input.current().Offset;
                    if (Input.current().Kind == TokenKind::Reference && Input.current().Text.starts_with('%'))
                    {
                        Out.Kind = TypeSyntaxKind::Reference;
                        return parseReference(Out.Reference);
                    }
                    if (consumeIf("["))
                    {
                        Out.Kind = TypeSyntaxKind::Array;
                        Out.Element = std::make_unique<TypeNode>();
                        return parseUnsigned(Out.Count) && consume("x") && parseType(*Out.Element) && consume("]");
                    }
                    if (Input.current().Kind != TokenKind::Word)
                    {
                        return Input.failCurrent("expected a type expression");
                    }
                    const std::string_view Word = Input.current().Text;
                    if (const auto KeywordKind = detail::parseEnum(Word, detail::KeywordTypeEntries))
                    {
                        Out.Kind = *KeywordKind;
                        Input.advance();
                        if (Out.Kind == TypeSyntaxKind::Pointer)
                        {
                            Out.Element = std::make_unique<TypeNode>();
                            return consume("<") && parseType(*Out.Element) && consume(">");
                        }
                        if (Out.Kind == TypeSyntaxKind::Function)
                        {
                            Out.ABI = {ABIKind::Ink, CallingConvention::Default};
                            if (consumeIf("<"))
                            {
                                if (!parseRequiredEnum(Out.ABI.ABI, detail::ABIEntries, "expected function ABI") || !parseRequiredEnum(Out.ABI.Convention, detail::ConventionEntries, "expected calling convention") || !consume(">"))
                                {
                                    return false;
                                }
                            }
                            Out.Element = std::make_unique<TypeNode>();
                            return consume("(") && parseParameterList(Out, false) && consume(")") && consume(detail::ArrowToken) && parseType(*Out.Element);
                        }
                        return true;
                    }
                    if (Word.size() >= 2 && (Word[0] == 'i' || Word[0] == 'u' || Word[0] == 'f'))
                    {
                        std::uint64_t Width = 0;
                        if (parseUnsignedWord(Word.substr(1), Width))
                        {
                            const bool Float = Word[0] == 'f';
                            if ((Float && Width != 32 && Width != 64) || (!Float && Width != 8 && Width != 16 && Width != 32 && Width != 64))
                            {
                                return Input.failCurrent("unsupported primitive type width");
                            }
                            Out.Kind = Float ? TypeSyntaxKind::Float : TypeSyntaxKind::Integer;
                            Out.Width = static_cast<std::uint32_t>(Width);
                            Out.Signed = Word[0] != 'u';
                            Input.advance();
                            return true;
                        }
                    }
                    return Input.failCurrent("expected a recognized type");
                }

                bool parseValueWithType(const TypeNode& ExpectedType, TypedOperand& Out)
                {
                    Out = {};
                    Out.Offset = Input.current().Offset;
                    Out.Type = cloneTypeNode(ExpectedType);
                    if (ExpectedType.Kind == TypeSyntaxKind::Meta)
                    {
                        if (Input.current().Kind == TokenKind::Reference)
                        {
                            Out.Kind = OperandValueKind::Reference;
                            return parseReference(Out.Text);
                        }
                        Out.Kind = OperandValueKind::TypeValue;
                        Out.TypeLiteral = std::make_unique<TypeNode>();
                        return parseType(*Out.TypeLiteral);
                    }
                    if (Input.current().Kind == TokenKind::Reference)
                    {
                        Out.Kind = OperandValueKind::Reference;
                        return parseReference(Out.Text);
                    }
                    if (Input.current().Kind == TokenKind::ByteString)
                    {
                        Out.Kind = OperandValueKind::ByteString;
                        Out.Text = Input.current().Text;
                        Input.advance();
                        return true;
                    }
                    if (!parseUnsigned(Out.Bits))
                    {
                        return false;
                    }
                    if (ExpectedType.Kind == TypeSyntaxKind::Integer || ExpectedType.Kind == TypeSyntaxKind::Float)
                    {
                        Out.Kind = OperandValueKind::Unsigned;
                        return true;
                    }
                    return Input.fail(Out.Offset, "numeric constant requires an integer or floating-point type");
                }

                bool parseTypedOperand(TypedOperand& Out)
                {
                    TypeNode Type;
                    return parseType(Type) && parseValueWithType(Type, Out);
                }

                bool parseTargetBlock(std::string& Out)
                {
                    if (!parseReference(Out))
                    {
                        return false;
                    }
                    return Out.starts_with('%') || Input.failCurrent("branch target must be a '%' basic block identifier");
                }

                bool parseInstruction(InstructionNode& Out)
                {
                    Out = {};
                    Out.Offset = Input.current().Offset;
                    if (Input.current().Kind == TokenKind::Reference && Input.peekNext().Kind == TokenKind::Punctuation && Input.peekNext().Text == "=")
                    {
                        if (!parseReference(Out.Id) || !consume("="))
                        {
                            return false;
                        }
                    }
                    if (Input.current().Kind != TokenKind::Word)
                    {
                        return Input.failCurrent("expected an instruction opcode");
                    }
                    const detail::InstructionDescriptor* Descriptor = detail::findInstructionDescriptor(Input.current().Text);
                    if (!Descriptor)
                    {
                        return Input.fail(Out.Offset, "unknown instruction opcode");
                    }
                    Input.advance();
                    if ((Descriptor->RequiresResultId && Out.Id.empty()) || (!Descriptor->AllowsResultId && !Out.Id.empty()))
                    {
                        return Input.fail(Out.Offset, "only value-producing instructions require a result identifier");
                    }
                    Out.Kind = Descriptor->Kind;
                    if (Out.Kind == InstructionKind::Branch && !peek(detail::LabelKeyword))
                    {
                        Out.Kind = InstructionKind::CondBranch;
                    }

                    switch (Out.Kind)
                    {
                    case InstructionKind::Add:
                        Out.Operands.resize(2);
                        return parseType(Out.ResultType) && parseValueWithType(Out.ResultType, Out.Operands[0]) && consume(",") && parseValueWithType(Out.ResultType, Out.Operands[1]);
                    case InstructionKind::Alloca:
                        return parseType(Out.ResultType);
                    case InstructionKind::Load:
                        Out.Operands.resize(1);
                        return parseType(Out.ResultType) && consume(",") && parseTypedOperand(Out.Operands[0]);
                    case InstructionKind::Store:
                        Out.ResultType.Kind = TypeSyntaxKind::Void;
                        Out.ResultType.Offset = Out.Offset;
                        Out.Operands.resize(2);
                        return parseTypedOperand(Out.Operands[1]) && consume(",") && parseTypedOperand(Out.Operands[0]);
                    case InstructionKind::Phi:
                        if (!parseType(Out.ResultType))
                        {
                            return false;
                        }
                        for (;;)
                        {
                            IncomingNode Incoming;
                            Incoming.Offset = Input.current().Offset;
                            if (!consume("[") || !parseValueWithType(Out.ResultType, Incoming.Value) || !consume(",") || !parseReference(Incoming.BlockId) || !consume("]"))
                            {
                                return false;
                            }
                            Out.Incoming.push_back(std::move(Incoming));
                            if (!consumeIf(","))
                            {
                                return true;
                            }
                        }
                    case InstructionKind::Call:
                        if (!parseType(Out.ResultType))
                        {
                            return false;
                        }
                        if ((Out.ResultType.Kind != TypeSyntaxKind::Void) != !Out.Id.empty())
                        {
                            return Input.fail(Out.Offset, "only non-void calls define a result identifier");
                        }
                        Out.CalleeOffset = Input.current().Offset;
                        if (!parseReference(Out.CalleeId) || !consume("("))
                        {
                            return false;
                        }
                        if (consumeIf(")"))
                        {
                            return true;
                        }
                        for (;;)
                        {
                            TypedOperand Arg;
                            if (!parseTypedOperand(Arg))
                            {
                                return false;
                            }
                            Out.Operands.push_back(std::move(Arg));
                            if (consumeIf(")"))
                            {
                                return true;
                            }
                            if (!consume(",") || peek(")"))
                            {
                                return Input.failCurrent("invalid call argument list");
                            }
                        }
                    case InstructionKind::Return:
                        Out.ResultType.Kind = TypeSyntaxKind::Void;
                        Out.ResultType.Offset = Out.Offset;
                        if (consumeIf(detail::VoidKeyword))
                        {
                            return true;
                        }
                        Out.Operands.resize(1);
                        return parseTypedOperand(Out.Operands[0]);
                    case InstructionKind::Branch:
                        Out.ResultType.Kind = TypeSyntaxKind::Void;
                        Out.ResultType.Offset = Out.Offset;
                        Out.Targets.resize(1);
                        return consume(detail::LabelKeyword) && parseTargetBlock(Out.Targets[0]);
                    case InstructionKind::CondBranch:
                        Out.ResultType.Kind = TypeSyntaxKind::Void;
                        Out.ResultType.Offset = Out.Offset;
                        Out.Operands.resize(1);
                        Out.Targets.resize(2);
                        return parseTypedOperand(Out.Operands[0]) && consume(",") && consume(detail::LabelKeyword) && parseTargetBlock(Out.Targets[0]) && consume(",") && consume(detail::LabelKeyword) && parseTargetBlock(Out.Targets[1]);
                    }
                    return Input.fail(Out.Offset, "unknown instruction opcode");
                }

                bool atBlockHeader()
                {
                    if (Input.hasError() || Input.current().Kind != TokenKind::Reference)
                    {
                        return false;
                    }
                    const Token& Second = Input.peekNext();
                    return (Second.Kind == TokenKind::Punctuation && Second.Text == ":") || Second.Kind == TokenKind::String;
                }

                bool parseBlock(BlockNode& Out)
                {
                    Out = {};
                    Out.Offset = Input.current().Offset;
                    if (!parseReference(Out.Id) || !parseOptionalName(Out.Name) || !consume(":"))
                    {
                        return false;
                    }
                    while (!peek("}") && !atBlockHeader())
                    {
                        InstructionNode Inst;
                        if (!parseInstruction(Inst))
                        {
                            return false;
                        }
                        Out.Instructions.push_back(std::move(Inst));
                    }
                    return true;
                }

                bool parseFunction(FunctionNode& Out, bool IsRecordMethod = false)
                {
                    Out = {};
                    Out.Offset = Input.current().Offset;
                    bool HasBody = false;
                    if (!parseRequiredEnum(HasBody, detail::FunctionKindEntries, "expected 'declare' or 'define'") || !parseRequiredEnum(Out.Visibility, detail::VisibilityEntries, "expected 'private' or 'public'"))
                    {
                        return false;
                    }
                    bool SawExplicitLinkage = false;
                    if (Input.current().Kind == TokenKind::Word)
                    {
                        if (const auto Parsed = detail::parseEnum(Input.current().Text, detail::LinkageEntries))
                        {
                            Out.Linkage = *Parsed;
                            SawExplicitLinkage = true;
                            Input.advance();
                        }
                    }
                    if (consumeIf(detail::FromKeyword))
                    {
                        if (IsRecordMethod)
                        {
                            return Input.failCurrent("a record method cannot specify a module import source");
                        }
                        if (HasBody)
                        {
                            return Input.fail(Out.Offset, "an imported function declaration cannot have a body");
                        }
                        if (SawExplicitLinkage)
                        {
                            return Input.failCurrent("an imported function declaration cannot specify native linkage");
                        }
                        if (Out.Visibility != SymbolVisibility::Public)
                        {
                            return Input.fail(Out.Offset, "an imported function declaration must have public visibility");
                        }
                        Out.ImportOffset = Input.current().Offset;
                        if (!parseImportReference(Out.ImportId))
                        {
                            return false;
                        }
                        if (Input.current().Kind == TokenKind::Word && detail::parseEnum(Input.current().Text, detail::LinkageEntries).has_value())
                        {
                            return Input.failCurrent("an imported function declaration cannot specify native linkage");
                        }
                    }

                    Out.Signature.Kind = TypeSyntaxKind::Function;
                    Out.Signature.Offset = Out.Offset;
                    parseOptionalEnum(Out.Signature.ABI.ABI, ABIKind::Ink, detail::ABIEntries);
                    parseOptionalEnum(Out.Signature.ABI.Convention, CallingConvention::Default, detail::ConventionEntries);
                    Out.Signature.Element = std::make_unique<TypeNode>();
                    if (!parseType(*Out.Signature.Element) || !parseReference(Out.Id) || !parseOptionalName(Out.Name) || !consume("(") || !parseParameterList(Out.Signature, true) || !consume(")"))
                    {
                        return false;
                    }
                    if (!Out.ImportId.empty() && Out.Name.empty())
                    {
                        return Input.fail(Out.Offset, "an imported function declaration requires a non-empty symbol name");
                    }
                    if (!HasBody)
                    {
                        return true;
                    }
                    if (!consume("{"))
                    {
                        return false;
                    }
                    while (!peek("}"))
                    {
                        BlockNode Block;
                        if (!parseBlock(Block))
                        {
                            return false;
                        }
                        Out.Blocks.push_back(std::move(Block));
                    }
                    if (Out.Blocks.empty())
                    {
                        return Input.fail(Out.Offset, "a function definition must contain a basic block");
                    }
                    return consume("}");
                }

                bool parseRecordAfterTypeKeyword(RecordNode& Out)
                {
                    if (!parseRequiredEnum(Out.IsObject, detail::RecordKindEntries, "expected 'struct' or 'class'"))
                    {
                        return false;
                    }
                    if (consumeIf(detail::BorrowedKeyword))
                    {
                        if (consumeIf(detail::FromKeyword))
                        {
                            return Input.failCurrent("borrowed records cannot specify an import source");
                        }
                        Out.Registered = false;
                    }
                    else if (consumeIf(detail::FromKeyword))
                    {
                        Out.ImportOffset = Input.current().Offset;
                        if (!parseImportReference(Out.ImportId))
                        {
                            return false;
                        }
                        if (consumeIf(detail::BorrowedKeyword))
                        {
                            return Input.failCurrent("imported records cannot be marked borrowed");
                        }
                    }
                    if (!parseOptionalName(Out.Name))
                    {
                        return false;
                    }
                    if (!Out.ImportId.empty() && Out.Name.empty())
                    {
                        return Input.fail(Out.Offset, "an imported record requires a non-empty symbol name");
                    }
                    if (consumeIf(detail::ExtendsKeyword))
                    {
                        if (!consume("("))
                        {
                            return false;
                        }
                        if (!consumeIf(")"))
                        {
                            for (;;)
                            {
                                TypeNode Base;
                                if (!parseType(Base))
                                {
                                    return false;
                                }
                                Out.Bases.push_back(std::move(Base));
                                if (consumeIf(")"))
                                {
                                    break;
                                }
                                if (!consume(",") || peek(")"))
                                {
                                    return Input.failCurrent("invalid record base list");
                                }
                            }
                        }
                    }
                    if (!consume("{"))
                    {
                        return false;
                    }
                    while (consumeIf(detail::FieldKeyword))
                    {
                        FieldNode FieldValue;
                        FieldValue.Offset = Input.current().Offset;
                        if (!parseRequiredEnum(FieldValue.Visibility, detail::VisibilityEntries, "expected 'private' or 'public'") || !parseRequiredEnum(FieldValue.Kind, detail::MemberKindEntries, "expected 'instance' or 'static'") || !parseType(FieldValue.Type) || !parseOptionalName(FieldValue.Name))
                        {
                            return false;
                        }
                        Out.Fields.push_back(std::move(FieldValue));
                    }
                    while (consumeIf(detail::MethodKeyword))
                    {
                        MethodNode MethodValue;
                        MethodValue.Offset = Input.current().Offset;
                        if (!parseRequiredEnum(MethodValue.Kind, detail::MemberKindEntries, "expected 'instance' or 'static'") || !parseFunction(MethodValue.Function, true))
                        {
                            return false;
                        }
                        Out.Methods.push_back(std::move(MethodValue));
                    }
                    return consume("}");
                }

                bool parseGlobalAfterEquals(GlobalNode& Out)
                {
                    if (!parseRequiredEnum(Out.Visibility, detail::VisibilityEntries, "expected 'private' or 'public'"))
                    {
                        return false;
                    }
                    bool SawExplicitLinkage = false;
                    if (Input.current().Kind == TokenKind::Word)
                    {
                        if (const auto Parsed = detail::parseEnum(Input.current().Text, detail::LinkageEntries))
                        {
                            Out.Linkage = *Parsed;
                            SawExplicitLinkage = true;
                            Input.advance();
                        }
                    }
                    if (consumeIf(detail::FromKeyword))
                    {
                        if (SawExplicitLinkage)
                        {
                            return Input.failCurrent("an imported global variable cannot specify native linkage");
                        }
                        if (Out.Visibility != SymbolVisibility::Public)
                        {
                            return Input.fail(Out.Offset, "an imported global variable must have public visibility");
                        }
                        if (Out.Name.empty())
                        {
                            return Input.fail(Out.Offset, "an imported global variable requires a non-empty symbol name");
                        }
                        Out.ImportOffset = Input.current().Offset;
                        if (!parseImportReference(Out.ImportId))
                        {
                            return false;
                        }
                        if (Input.current().Kind == TokenKind::Word && detail::parseEnum(Input.current().Text, detail::LinkageEntries).has_value())
                        {
                            return Input.failCurrent("an imported global variable cannot specify native linkage");
                        }
                    }
                    return consume(detail::GlobalKeyword) && parseType(Out.StoredType);
                }

                bool parseGlobal(GlobalNode& Out)
                {
                    Out = {};
                    Out.Offset = Input.current().Offset;
                    return parseReference(Out.Id) && parseOptionalName(Out.Name) && consume("=") && parseGlobalAfterEquals(Out);
                }

                bool parseImport(ImportNode& Out)
                {
                    Out = {};
                    Out.Offset = Input.current().Offset;
                    if (!consume(detail::ImportKeyword) || !parseImportReference(Out.Id) || !consume("="))
                    {
                        return false;
                    }
                    Out.PathOffset = Input.current().Offset;
                    return parseQuoted(Out.Path);
                }

                bool parseModule()
                {
                    if (!consume(detail::ModulePathKeyword) || !consume("="))
                    {
                        return false;
                    }
                    ModulePathOffset = Input.current().Offset;
                    if (!parseQuoted(ModulePathText))
                    {
                        return false;
                    }
                    while (!Input.hasError() && Input.current().Kind != TokenKind::End)
                    {
                        if (peek(detail::ModulePathKeyword))
                        {
                            return Input.failCurrent("duplicate or misplaced 'module_path' header");
                        }
                        if (peek(detail::ImportKeyword))
                        {
                            ImportNode ImportValue;
                            if (!parseImport(ImportValue))
                            {
                                return false;
                            }
                            ParsedImports.push_back(std::move(ImportValue));
                        }
                        else if (Input.current().Kind == TokenKind::Word && detail::parseEnum(Input.current().Text, detail::FunctionKindEntries).has_value())
                        {
                            FunctionNode FunctionValue;
                            if (!parseFunction(FunctionValue))
                            {
                                return false;
                            }
                            ParsedFunctions.push_back(std::move(FunctionValue));
                        }
                        else if (Input.current().Kind == TokenKind::Reference && Input.peekNext().Kind == TokenKind::Punctuation && Input.peekNext().Text == "=")
                        {
                            const std::size_t Offset = Input.current().Offset;
                            std::string Id;
                            if (!parseReference(Id) || !consume("="))
                            {
                                return false;
                            }
                            if (consumeIf(detail::TypeKeyword))
                            {
                                RecordNode Record;
                                Record.Id = std::move(Id);
                                Record.Offset = Offset;
                                if (!parseRecordAfterTypeKeyword(Record))
                                {
                                    return false;
                                }
                                ParsedRecords.push_back(std::move(Record));
                            }
                            else
                            {
                                GlobalNode Global;
                                Global.Id = std::move(Id);
                                Global.Offset = Offset;
                                if (!parseGlobalAfterEquals(Global))
                                {
                                    return false;
                                }
                                ParsedGlobals.push_back(std::move(Global));
                            }
                        }
                        else
                        {
                            GlobalNode Global;
                            if (!parseGlobal(Global))
                            {
                                return false;
                            }
                            ParsedGlobals.push_back(std::move(Global));
                        }
                    }
                    return !Input.hasError();
                }

                bool internName(std::string_view Text, std::size_t Offset, core::Name& Out)
                {
                    Out = core::NamePool::instance().intern(Text);
                    if (!Text.empty() && !Out.valid())
                    {
                        return Input.fail(Offset, "name pool capacity exhausted");
                    }
                    return true;
                }

                bool addDefinition(std::string_view Id, DefinitionKind Kind, std::size_t Offset, const InstructionNode* Inst = nullptr)
                {
                    const char Prefix = (Kind == DefinitionKind::Function || Kind == DefinitionKind::Global) ? '@' : '%';
                    if (Id.size() < 2 || Id.front() != Prefix)
                    {
                        return Input.fail(Offset, "definition has an empty ID or an invalid ID prefix");
                    }
                    if (!Definitions.emplace(Id, DefinitionEntry{Kind, Inst}).second)
                    {
                        return Input.fail(Offset, "textual IDs must be unique throughout the module");
                    }
                    return true;
                }

                bool indexFunctionDefinitions(const FunctionNode& Node)
                {
                    if (!addDefinition(Node.Id, DefinitionKind::Function, Node.Offset))
                    {
                        return false;
                    }
                    for (const ParameterNode& Param : Node.Signature.Parameters)
                    {
                        if (!addDefinition(Param.ArgumentId, DefinitionKind::Argument, Param.Offset))
                        {
                            return false;
                        }
                    }
                    for (const BlockNode& Block : Node.Blocks)
                    {
                        if (!addDefinition(Block.Id, DefinitionKind::Block, Block.Offset))
                        {
                            return false;
                        }
                        bool SawNonPhi = false;
                        for (std::size_t Index = 0; Index < Block.Instructions.size(); ++Index)
                        {
                            const InstructionNode& Inst = Block.Instructions[Index];
                            if (Inst.Kind == InstructionKind::Phi)
                            {
                                if (SawNonPhi)
                                {
                                    return Input.fail(Inst.Offset, "phi instructions must form a prefix at the start of a block");
                                }
                            }
                            else
                            {
                                SawNonPhi = true;
                            }
                            if (Instruction::isTerminatorKind(Inst.Kind) && Index + 1 != Block.Instructions.size())
                            {
                                return Input.fail(Inst.Offset, "a terminator must be the last instruction in its block");
                            }
                            if (!Inst.Id.empty() && !addDefinition(Inst.Id, DefinitionKind::Instruction, Inst.Offset, &Inst))
                            {
                                return false;
                            }
                        }
                        if (Block.Instructions.empty() || !Instruction::isTerminatorKind(Block.Instructions.back().Kind))
                        {
                            return Input.fail(Block.Offset, "a basic block must end in a terminator");
                        }
                    }
                    return true;
                }

                bool indexDefinitions()
                {
                    for (const RecordNode& Record : ParsedRecords)
                    {
                        if (!addDefinition(Record.Id, DefinitionKind::Record, Record.Offset))
                        {
                            return false;
                        }
                        for (const MethodNode& MethodValue : Record.Methods)
                        {
                            if (!indexFunctionDefinitions(MethodValue.Function))
                            {
                                return false;
                            }
                        }
                    }
                    for (const GlobalNode& Global : ParsedGlobals)
                    {
                        if (!addDefinition(Global.Id, DefinitionKind::Global, Global.Offset))
                        {
                            return false;
                        }
                    }
                    for (const FunctionNode& FunctionValue : ParsedFunctions)
                    {
                        if (!indexFunctionDefinitions(FunctionValue))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                const Value* resolveType(const TypeNode& Node, bool AllowInstructions, std::size_t Depth = 0)
                {
                    if (Depth >= detail::MaxTypeExpressionDepth)
                    {
                        Input.fail(Node.Offset, "type dependency nesting exceeds the supported depth of 128");
                        return nullptr;
                    }
                    TypePool& Types = Owned->types();
                    const Value* Result = nullptr;
                    switch (Node.Kind)
                    {
                    case TypeSyntaxKind::Reference:
                        {
                            const auto Def = Definitions.find(Node.Reference);
                            if (Def == Definitions.end())
                            {
                                Input.fail(Node.Offset, "reference names an undefined textual ID");
                                return nullptr;
                            }
                            if (Def->second.Kind == DefinitionKind::Block)
                            {
                                Input.fail(Node.Offset, "a basic block ID cannot be used as a type");
                                return nullptr;
                            }
                            if (!AllowInstructions && Def->second.Kind == DefinitionKind::Instruction)
                            {
                                Input.fail(Node.Offset, "declaration type metadata cannot depend on a function-body instruction");
                                return nullptr;
                            }
                            const auto Found = Values.find(Node.Reference);
                            if (Found == Values.end())
                            {
                                return nullptr;
                            }
                            Result = Found->second;
                            break;
                        }
                    case TypeSyntaxKind::Meta:
                        Result = &Types.getMetaType();
                        break;
                    case TypeSyntaxKind::Bool:
                        Result = &Types.getBoolType();
                        break;
                    case TypeSyntaxKind::Void:
                        Result = &Types.getVoidType();
                        break;
                    case TypeSyntaxKind::Integer:
                        Result = &Types.getIntegerType(static_cast<IntegerType::Width>(Node.Width), Node.Signed ? IntegerType::Signedness::Signed : IntegerType::Signedness::Unsigned);
                        break;
                    case TypeSyntaxKind::Float:
                        Result = &Types.getFloatType(static_cast<FloatType::Width>(Node.Width));
                        break;
                    case TypeSyntaxKind::Pointer:
                        {
                            const Value* Pointee = resolveType(*Node.Element, AllowInstructions, Depth + 1);
                            if (!Pointee)
                            {
                                return nullptr;
                            }
                            Result = &Types.getPointerType(*Pointee);
                            break;
                        }
                    case TypeSyntaxKind::Array:
                        {
                            const Value* Element = resolveType(*Node.Element, AllowInstructions, Depth + 1);
                            if (!Element)
                            {
                                return nullptr;
                            }
                            if (VoidType::classof(Element))
                            {
                                Input.fail(Node.Offset, "array element type must not be void");
                                return nullptr;
                            }
                            Result = &Types.getArrayType(*Element, Node.Count);
                            break;
                        }
                    case TypeSyntaxKind::Function:
                        {
                            const Value* ReturnType = resolveType(*Node.Element, AllowInstructions, Depth + 1);
                            if (!ReturnType)
                            {
                                return nullptr;
                            }
                            std::vector<Parameter> Parameters;
                            Parameters.reserve(Node.Parameters.size());
                            for (const ParameterNode& Param : Node.Parameters)
                            {
                                const Value* ParamType = resolveType(*Param.Type, AllowInstructions, Depth + 1);
                                if (!ParamType)
                                {
                                    return nullptr;
                                }
                                if (VoidType::classof(ParamType))
                                {
                                    Input.fail(Param.Offset, "fixed parameter type must not be void");
                                    return nullptr;
                                }
                                Parameters.emplace_back(*ParamType, Param.Kind);
                            }
                            Result = &Types.getFunctionType(*ReturnType, Parameters, Node.ABI, Node.Variadic);
                            break;
                        }
                    }
                    if (Result && (!Result->isTypeValue() || (Result->valueKind() != ValueKind::Type && Result->valueKind() != ValueKind::Argument && Result->valueKind() != ValueKind::Instruction)))
                    {
                        Input.fail(Node.Offset, "type position requires a concrete type or a type-valued argument/instruction");
                        return nullptr;
                    }
                    return Result;
                }

                const FunctionType* resolveFunctionSignature(const FunctionNode& Node)
                {
                    const Value* ReturnType = resolveType(*Node.Signature.Element, false);
                    if (!ReturnType)
                    {
                        return nullptr;
                    }
                    std::vector<Parameter> Parameters;
                    Parameters.reserve(Node.Signature.Parameters.size());
                    for (const ParameterNode& Param : Node.Signature.Parameters)
                    {
                        const Value* ParamType = resolveType(*Param.Type, false);
                        if (!ParamType)
                        {
                            return nullptr;
                        }
                        if (VoidType::classof(ParamType))
                        {
                            Input.fail(Param.Offset, "fixed parameter type must not be void");
                            return nullptr;
                        }
                        Parameters.emplace_back(*ParamType, Param.Kind);
                    }
                    return &Owned->types().getFunctionType(*ReturnType, Parameters, Node.Signature.ABI, Node.Signature.Variadic);
                }

                const Value* resolveInstructionResultType(const InstructionNode& Node)
                {
                    const Value* TypeValue = resolveType(Node.ResultType, true);
                    if (!TypeValue)
                    {
                        return nullptr;
                    }
                    if (Node.Kind == InstructionKind::Alloca)
                    {
                        if (VoidType::classof(TypeValue))
                        {
                            Input.fail(Node.Offset, "alloca cannot allocate void");
                            return nullptr;
                        }
                        return &Owned->types().getPointerType(*TypeValue);
                    }
                    return TypeValue;
                }

                const Value* resolveOperandValue(const TypedOperand& Node)
                {
                    const Value* ExpectedType = resolveType(Node.Type, true);
                    if (!ExpectedType)
                    {
                        return nullptr;
                    }
                    ConstantPool& Constants = Owned->constants();
                    const Value* Result = nullptr;
                    switch (Node.Kind)
                    {
                    case OperandValueKind::Reference:
                        {
                            const auto Def = Definitions.find(Node.Text);
                            if (Def == Definitions.end())
                            {
                                Input.fail(Node.Offset, "reference names an undefined textual ID");
                                return nullptr;
                            }
                            if (Def->second.Kind == DefinitionKind::Block)
                            {
                                Input.fail(Node.Offset, "a basic block ID cannot be used as a value");
                                return nullptr;
                            }
                            const auto Found = Values.find(Node.Text);
                            if (Found == Values.end())
                            {
                                return nullptr;
                            }
                            Result = Found->second;
                            break;
                        }
                    case OperandValueKind::Unsigned:
                        if (IntegerType::classof(ExpectedType))
                        {
                            Result = Constants.getInteger(static_cast<const IntegerType&>(*ExpectedType), Node.Bits);
                        }
                        else if (FloatType::classof(ExpectedType))
                        {
                            Result = Constants.getFloatBits(static_cast<const FloatType&>(*ExpectedType), Node.Bits);
                        }
                        else
                        {
                            Input.fail(Node.Offset, "numeric constant requires an integer or floating-point type");
                            return nullptr;
                        }
                        if (!Result)
                        {
                            Input.fail(Node.Offset, "numeric constant has bits outside its declared width");
                            return nullptr;
                        }
                        break;
                    case OperandValueKind::ByteString:
                        Result = &Constants.getString(Node.Text);
                        break;
                    case OperandValueKind::TypeValue:
                        Result = resolveType(*Node.TypeLiteral, true);
                        if (!Result)
                        {
                            return nullptr;
                        }
                        break;
                    }
                    if (Result && &Result->getType() != ExpectedType)
                    {
                        Input.fail(Node.Offset, "operand value type does not match its declared type");
                        return nullptr;
                    }
                    return Result;
                }

                const Value* acquireOperand(const TypedOperand& Node)
                {
                    const Value* Resolved = resolveOperandValue(Node);
                    if (Resolved || Input.hasError())
                    {
                        return Resolved;
                    }
                    const Value* ExpectedType = resolveType(Node.Type, true);
                    if (!ExpectedType)
                    {
                        return nullptr;
                    }
                    const auto Def = Definitions.find(Node.Text);
                    if (Def == Definitions.end() || Def->second.Kind != DefinitionKind::Instruction || !Def->second.Instruction)
                    {
                        Input.fail(Node.Offset, "operand does not refer to an available value");
                        return nullptr;
                    }
                    const Value* InstructionType = resolveInstructionResultType(*Def->second.Instruction);
                    if (!InstructionType)
                    {
                        return nullptr;
                    }
                    if (InstructionType != ExpectedType)
                    {
                        Input.fail(Node.Offset, "operand value type does not match its declared type");
                        return nullptr;
                    }
                    auto Pending = std::make_unique<PendingValue>(*InstructionType);
                    const Value* Result = Pending.get();
                    PendingValues.push_back(std::move(Pending));
                    return Result;
                }

                const Value* acquireCallee(std::string_view CalleeId, std::size_t Offset)
                {
                    const auto Def = Definitions.find(CalleeId);
                    if (Def == Definitions.end())
                    {
                        Input.fail(Offset, "callee names an undefined textual ID");
                        return nullptr;
                    }
                    if (Def->second.Kind == DefinitionKind::Block)
                    {
                        Input.fail(Offset, "a basic block ID cannot be used as a callee");
                        return nullptr;
                    }
                    if (const auto Found = Values.find(CalleeId); Found != Values.end())
                    {
                        return Found->second;
                    }
                    if (Def->second.Kind != DefinitionKind::Instruction || !Def->second.Instruction)
                    {
                        Input.fail(Offset, "callee does not refer to an available value");
                        return nullptr;
                    }
                    const Value* InstructionType = resolveInstructionResultType(*Def->second.Instruction);
                    if (!InstructionType)
                    {
                        return nullptr;
                    }
                    auto Pending = std::make_unique<PendingValue>(*InstructionType);
                    const Value* Result = Pending.get();
                    PendingValues.push_back(std::move(Pending));
                    return Result;
                }

                BasicBlock* lookupBlock(std::string_view Id, const Function& Owner, std::size_t Offset)
                {
                    const auto Found = Blocks.find(Id);
                    if (Found == Blocks.end())
                    {
                        Input.fail(Offset, "branch or phi target does not name a basic block");
                        return nullptr;
                    }
                    if (Found->second->parent() != &Owner)
                    {
                        Input.fail(Offset, "branch or phi target belongs to another function");
                        return nullptr;
                    }
                    return Found->second;
                }

                bool precedingFunctionReady(std::size_t Index) const noexcept
                {
                    const FunctionBuildState& Current = Functions[Index];
                    for (std::size_t Previous = 0; Previous < Index; ++Previous)
                    {
                        if (Functions[Previous].Record == Current.Record && !Functions[Previous].Value)
                        {
                            return false;
                        }
                    }
                    return true;
                }

                bool instantiateFunction(FunctionBuildState& State, const FunctionType& Signature)
                {
                    const FunctionNode& Node = *State.Syntax;
                    core::Name FunctionName = core::NAME_None;
                    if (!internName(Node.Name, Node.Offset, FunctionName))
                    {
                        return false;
                    }
                    if (State.Record)
                    {
                        State.Value = &State.Record->addMethod(Signature, FunctionName, State.Kind, Node.Visibility, Node.Linkage).function();
                    }
                    else if (!Node.ImportId.empty())
                    {
                        const auto FoundImport = Imports.find(Node.ImportId);
                        if (FoundImport == Imports.end())
                        {
                            return Input.fail(Node.ImportOffset, "reference names an undefined module import ID");
                        }
                        State.Value = &Owned->module().declareImportedFunction(*FoundImport->second, Signature, FunctionName);
                    }
                    else
                    {
                        State.Value = &Owned->module().createFunction(Signature, FunctionName, Node.Visibility, Node.Linkage);
                    }
                    Values.emplace(Node.Id, State.Value);
                    for (std::size_t Index = 0; Index < Node.Signature.Parameters.size(); ++Index)
                    {
                        const ParameterNode& Param = Node.Signature.Parameters[Index];
                        core::Name ArgName = core::NAME_None;
                        if (!internName(Param.ArgumentName, Param.Offset, ArgName))
                        {
                            return false;
                        }
                        Argument& Arg = State.Value->argument(Index);
                        Arg.setName(ArgName);
                        Values.emplace(Param.ArgumentId, &Arg);
                    }
                    for (const BlockNode& BlockSyntax : Node.Blocks)
                    {
                        core::Name BlockName = core::NAME_None;
                        if (!internName(BlockSyntax.Name, BlockSyntax.Offset, BlockName))
                        {
                            return false;
                        }
                        BasicBlock& Block = State.Value->createBasicBlock(BlockName);
                        Blocks.emplace(BlockSyntax.Id, &Block);
                        BlockStates.push_back({&BlockSyntax, State.Value, &Block});
                    }
                    return true;
                }

                Instruction* instantiateInstruction(const InstructionNode& Node, BlockBuildState& State)
                {
                    const Value* ResultType = resolveInstructionResultType(Node);
                    if (!ResultType)
                    {
                        return nullptr;
                    }
                    InstructionBuilder& Builder = Owned->context().builder();
                    Builder.setInsertPoint(*State.Value);
                    switch (Node.Kind)
                    {
                    case InstructionKind::Add:
                        {
                            const Value* LHS = acquireOperand(Node.Operands[0]);
                            const Value* RHS = acquireOperand(Node.Operands[1]);
                            if (!LHS || !RHS)
                            {
                                return nullptr;
                            }
                            if ((!IntegerType::classof(ResultType) && !FloatType::classof(ResultType)) || &LHS->getType() != ResultType || &RHS->getType() != ResultType)
                            {
                                Input.fail(Node.Offset, "add requires matching integer or floating-point operand and result types");
                                return nullptr;
                            }
                            return &Builder.createAdd(*LHS, *RHS);
                        }
                    case InstructionKind::Alloca:
                        return &Builder.createAlloca(static_cast<const PointerType&>(*ResultType).pointeeType());
                    case InstructionKind::Load:
                        {
                            const Value* Source = acquireOperand(Node.Operands[0]);
                            if (!Source)
                            {
                                return nullptr;
                            }
                            if (!PointerType::classof(&Source->getType()) || VoidType::classof(ResultType) || &static_cast<const PointerType&>(Source->getType()).pointeeType() != ResultType)
                            {
                                Input.fail(Node.Offset, "load result type must match its pointer source's non-void pointee type");
                                return nullptr;
                            }
                            return &Builder.createLoad(*Source);
                        }
                    case InstructionKind::Store:
                        {
                            const Value* Destination = acquireOperand(Node.Operands[0]);
                            const Value* Source = acquireOperand(Node.Operands[1]);
                            if (!Destination || !Source)
                            {
                                return nullptr;
                            }
                            if (!PointerType::classof(&Destination->getType()) || &static_cast<const PointerType&>(Destination->getType()).pointeeType() != &Source->getType())
                            {
                                Input.fail(Node.Offset, "store requires a pointer to its source's exact type");
                                return nullptr;
                            }
                            return &Builder.createStore(*Destination, *Source);
                        }
                    case InstructionKind::Call:
                        {
                            const Value* Callee = acquireCallee(Node.CalleeId, Node.CalleeOffset);
                            if (!Callee)
                            {
                                return nullptr;
                            }
                            if (!FunctionType::classof(&Callee->getType()))
                            {
                                Input.fail(Node.Offset, "call callee must have a function type");
                                return nullptr;
                            }
                            const auto& Signature = static_cast<const FunctionType&>(Callee->getType());
                            const auto Parameters = Signature.parameters();
                            const std::size_t ArgumentCount = Node.Operands.size();
                            if (&Signature.returnType() != ResultType || ArgumentCount < Parameters.size() || (!Signature.isVariadic() && ArgumentCount != Parameters.size()))
                            {
                                Input.fail(Node.Offset, "call result type or argument count does not match its callee signature");
                                return nullptr;
                            }
                            std::vector<ValueRef> Arguments;
                            Arguments.reserve(ArgumentCount);
                            for (std::size_t Index = 0; Index < ArgumentCount; ++Index)
                            {
                                const Value* Arg = acquireOperand(Node.Operands[Index]);
                                if (!Arg)
                                {
                                    return nullptr;
                                }
                                if (VoidType::classof(&Arg->getType()) || (Index < Parameters.size() && &Arg->getType() != &Parameters[Index].type()))
                                {
                                    Input.fail(Node.Offset, "call arguments must be non-void and match the fixed parameter types");
                                    return nullptr;
                                }
                                Arguments.emplace_back(*Arg);
                            }
                            return &Builder.createCall(*Callee, Arguments);
                        }
                    case InstructionKind::Phi:
                        if (VoidType::classof(ResultType))
                        {
                            Input.fail(Node.Offset, "phi result type must not be void");
                            return nullptr;
                        }
                        return &Builder.createPhi(*ResultType);
                    case InstructionKind::Return:
                        {
                            const Value& ReturnType = State.Owner->signature().returnType();
                            const bool IsVoid = VoidType::classof(&ReturnType);
                            if (Node.Operands.size() != (IsVoid ? 0U : 1U))
                            {
                                Input.fail(Node.Offset, "return operand does not match function return type");
                                return nullptr;
                            }
                            if (IsVoid)
                            {
                                return &Builder.createReturn();
                            }
                            const Value* Returned = acquireOperand(Node.Operands[0]);
                            if (!Returned)
                            {
                                return nullptr;
                            }
                            if (&Returned->getType() != &ReturnType)
                            {
                                Input.fail(Node.Offset, "return value type does not match the function signature");
                                return nullptr;
                            }
                            return &Builder.createReturn(*Returned);
                        }
                    case InstructionKind::Branch:
                        {
                            BasicBlock* Target = lookupBlock(Node.Targets[0], *State.Owner, Node.Offset);
                            return Target ? &Builder.createBranch(*Target) : nullptr;
                        }
                    case InstructionKind::CondBranch:
                        {
                            const Value* Condition = acquireOperand(Node.Operands[0]);
                            BasicBlock* TrueTarget = lookupBlock(Node.Targets[0], *State.Owner, Node.Offset);
                            BasicBlock* FalseTarget = lookupBlock(Node.Targets[1], *State.Owner, Node.Offset);
                            if (!Condition || !TrueTarget || !FalseTarget)
                            {
                                return nullptr;
                            }
                            if (!BoolType::classof(&Condition->getType()))
                            {
                                Input.fail(Node.Offset, "conditional branch requires a boolean condition");
                                return nullptr;
                            }
                            return &Builder.createCondBranch(*Condition, *TrueTarget, *FalseTarget);
                        }
                    }
                    return nullptr;
                }

                bool buildModuleGraph()
                {
                    ModulePath ParsedPath;
                    if (!ModulePathText.empty())
                    {
                        ParsedPath = ModulePath::parse(ModulePathText);
                        if (!ParsedPath.valid())
                        {
                            return Input.fail(ModulePathOffset, "module_path must be a dot-separated list of non-empty identifiers");
                        }
                    }
                    Owned = std::make_unique<ParsedModule>(std::move(ParsedPath));
                    if (!indexDefinitions())
                    {
                        return false;
                    }

                    TypePool& Types = Owned->types();
                    Module& ModuleValue = Owned->module();
                    for (const ImportNode& Node : ParsedImports)
                    {
                        if (Imports.contains(Node.Id))
                        {
                            return Input.fail(Node.Offset, "module import IDs must be unique throughout the module");
                        }
                        ModulePath Path = ModulePath::parse(Node.Path);
                        if (!Path.valid())
                        {
                            return Input.fail(Node.PathOffset, "module import path must be a dot-separated list of non-empty identifiers");
                        }
                        if (ModuleValue.findImport(Path) != nullptr)
                        {
                            return Input.fail(Node.PathOffset, "module import path is duplicated");
                        }
                        ModuleImport& Created = ModuleValue.addImport(std::move(Path));
                        Imports.emplace(Node.Id, &Created);
                    }
                    for (const RecordNode& Node : ParsedRecords)
                    {
                        core::Name RecordName = core::NAME_None;
                        if (!internName(Node.Name, Node.Offset, RecordName))
                        {
                            return false;
                        }
                        const ModuleImport* ImportSource = nullptr;
                        if (!Node.ImportId.empty())
                        {
                            const auto FoundImport = Imports.find(Node.ImportId);
                            if (FoundImport == Imports.end())
                            {
                                return Input.fail(Node.ImportOffset, "reference names an undefined module import ID");
                            }
                            ImportSource = FoundImport->second;
                        }
                        RecordType* Record = nullptr;
                        if (Node.IsObject)
                        {
                            ObjectType& Object = Types.createObjectType(RecordName);
                            Record = &Object;
                            if (ImportSource)
                            {
                                ModuleValue.addImportedObjectType(*ImportSource, Object);
                            }
                            else if (Node.Registered)
                            {
                                ModuleValue.addObjectType(Object);
                            }
                        }
                        else
                        {
                            StructType& Struct = Types.createStructType(RecordName);
                            Record = &Struct;
                            if (ImportSource)
                            {
                                ModuleValue.addImportedStructType(*ImportSource, Struct);
                            }
                            else if (Node.Registered)
                            {
                                ModuleValue.addStructType(Struct);
                            }
                        }
                        Values.emplace(Node.Id, Record);
                        Records.emplace(&Node, Record);
                        for (const MethodNode& MethodValue : Node.Methods)
                        {
                            Functions.push_back({&MethodValue.Function, Record, MethodValue.Kind});
                        }
                    }
                    for (const FunctionNode& Node : ParsedFunctions)
                    {
                        Functions.push_back({&Node});
                    }

                    std::size_t RemainingDeclarations = Functions.size() + ParsedGlobals.size();
                    std::size_t NextGlobal = 0;
                    while (RemainingDeclarations != 0)
                    {
                        const std::size_t Previous = RemainingDeclarations;
                        for (std::size_t Index = 0; Index < Functions.size(); ++Index)
                        {
                            FunctionBuildState& State = Functions[Index];
                            if (State.Value || !precedingFunctionReady(Index))
                            {
                                continue;
                            }
                            const FunctionType* Signature = resolveFunctionSignature(*State.Syntax);
                            if (!Signature)
                            {
                                if (Input.hasError())
                                {
                                    return false;
                                }
                                continue;
                            }
                            if (!instantiateFunction(State, *Signature))
                            {
                                return false;
                            }
                            --RemainingDeclarations;
                        }
                        while (NextGlobal < ParsedGlobals.size())
                        {
                            const GlobalNode& Node = ParsedGlobals[NextGlobal];
                            const Value* StoredType = resolveType(Node.StoredType, false);
                            if (!StoredType)
                            {
                                if (Input.hasError())
                                {
                                    return false;
                                }
                                break;
                            }
                            if (VoidType::classof(StoredType))
                            {
                                return Input.fail(Node.StoredType.Offset, "a global variable stored type must not be void");
                            }
                            core::Name GlobalName = core::NAME_None;
                            if (!internName(Node.Name, Node.Offset, GlobalName))
                            {
                                return false;
                            }
                            GlobalVariable* Global = nullptr;
                            if (!Node.ImportId.empty())
                            {
                                const auto FoundImport = Imports.find(Node.ImportId);
                                if (FoundImport == Imports.end())
                                {
                                    return Input.fail(Node.ImportOffset, "reference names an undefined module import ID");
                                }
                                Global = &ModuleValue.declareImportedGlobal(*FoundImport->second, Types.getPointerType(*StoredType), GlobalName);
                            }
                            else
                            {
                                Global = &ModuleValue.createGlobalVariable(Types.getPointerType(*StoredType), GlobalName, Node.Visibility, Node.Linkage);
                            }
                            Values.emplace(Node.Id, Global);
                            ++NextGlobal;
                            --RemainingDeclarations;
                        }
                        if (RemainingDeclarations == Previous)
                        {
                            return Input.fail(0, "declaration dependencies cannot be resolved without changing declaration order");
                        }
                    }

                    for (const RecordNode& Node : ParsedRecords)
                    {
                        RecordType& Record = *Records.at(&Node);
                        for (const TypeNode& BaseNode : Node.Bases)
                        {
                            const Value* Base = resolveType(BaseNode, false);
                            if (!Base)
                            {
                                return Input.hasError() ? false : Input.fail(BaseNode.Offset, "record base dependency is unresolved");
                            }
                            if (Type::classof(Base) && !RecordType::classof(Base))
                            {
                                return Input.fail(BaseNode.Offset, "a concrete base type must be a struct or object");
                            }
                            Record.addBase(*Base);
                        }
                        for (const FieldNode& FieldSyntax : Node.Fields)
                        {
                            const Value* FieldType = resolveType(FieldSyntax.Type, false);
                            if (!FieldType)
                            {
                                return Input.hasError() ? false : Input.fail(FieldSyntax.Type.Offset, "record field type dependency is unresolved");
                            }
                            if (VoidType::classof(FieldType))
                            {
                                return Input.fail(FieldSyntax.Type.Offset, "field type must not be void");
                            }
                            core::Name FieldName = core::NAME_None;
                            if (!internName(FieldSyntax.Name, FieldSyntax.Offset, FieldName))
                            {
                                return false;
                            }
                            Record.addField(FieldName, *FieldType, FieldSyntax.Kind, FieldSyntax.Visibility);
                        }
                    }

                    std::size_t RemainingInstructions = 0;
                    for (const BlockBuildState& State : BlockStates)
                    {
                        RemainingInstructions += State.Syntax->Instructions.size();
                    }
                    while (RemainingInstructions != 0)
                    {
                        const std::size_t Previous = RemainingInstructions;
                        for (BlockBuildState& State : BlockStates)
                        {
                            while (State.NextInstruction < State.Syntax->Instructions.size())
                            {
                                const InstructionNode& Node = State.Syntax->Instructions[State.NextInstruction];
                                Instruction* Created = instantiateInstruction(Node, State);
                                if (!Created)
                                {
                                    if (Input.hasError())
                                    {
                                        return false;
                                    }
                                    break;
                                }
                                if (!Node.Id.empty())
                                {
                                    Values.emplace(Node.Id, Created);
                                }
                                InstructionStates.push_back({&Node, Created});
                                ++State.NextInstruction;
                                --RemainingInstructions;
                            }
                        }
                        if (RemainingInstructions == Previous)
                        {
                            return Input.fail(0, "instruction type dependencies contain a cycle or cannot be resolved");
                        }
                    }

                    for (const InstructionBuildState& State : InstructionStates)
                    {
                        const InstructionNode& Node = *State.Syntax;
                        if (Node.Kind == InstructionKind::Alloca || Node.Kind == InstructionKind::Branch)
                        {
                            continue;
                        }
                        if (Node.Kind == InstructionKind::Phi)
                        {
                            auto& Phi = static_cast<PhiInstruction&>(*State.Value);
                            for (const IncomingNode& Incoming : Node.Incoming)
                            {
                                const Value* IncomingValue = resolveOperandValue(Incoming.Value);
                                BasicBlock* Predecessor = lookupBlock(Incoming.BlockId, *State.Value->parent()->parent(), Incoming.Offset);
                                if (!IncomingValue || !Predecessor)
                                {
                                    return Input.hasError() ? false : Input.fail(Incoming.Offset, "phi incoming value is unresolved");
                                }
                                Phi.addIncoming(*IncomingValue, *Predecessor);
                            }
                            continue;
                        }
                        if (Node.Kind == InstructionKind::Call)
                        {
                            const Value* Callee = Values.contains(Node.CalleeId) ? Values.at(Node.CalleeId) : nullptr;
                            if (!Callee || !State.Value->setOperand(0, *Callee))
                            {
                                return Input.fail(Node.CalleeOffset, "call callee remains unresolved");
                            }
                            for (std::size_t Index = 0; Index < Node.Operands.size(); ++Index)
                            {
                                const Value* Arg = resolveOperandValue(Node.Operands[Index]);
                                if (!Arg || !State.Value->setOperand(Index + 1, *Arg))
                                {
                                    return Input.hasError() ? false : Input.fail(Node.Operands[Index].Offset, "call argument remains unresolved");
                                }
                            }
                            continue;
                        }
                        for (std::size_t Index = 0; Index < Node.Operands.size(); ++Index)
                        {
                            const Value* Operand = resolveOperandValue(Node.Operands[Index]);
                            if (!Operand || !State.Value->setOperand(Index, *Operand))
                            {
                                return Input.hasError() ? false : Input.fail(Node.Operands[Index].Offset, "instruction operand remains unresolved");
                            }
                        }
                    }

                    PendingValues.clear();
                    std::string VerificationDescription;
                    if (!verifyModule(ModuleValue, &VerificationDescription))
                    {
                        return Input.fail(0, VerificationDescription);
                    }
                    Owned->context().builder().clearInsertPoint();
                    return true;
                }

                Lexer Input;
                std::size_t TypeParseDepth = 0;
                std::string ModulePathText = {};
                std::size_t ModulePathOffset = 0;
                std::vector<ImportNode> ParsedImports = {};
                std::vector<RecordNode> ParsedRecords = {};
                std::vector<GlobalNode> ParsedGlobals = {};
                std::vector<FunctionNode> ParsedFunctions = {};
                std::unique_ptr<ParsedModule> Owned = nullptr;
                std::unordered_map<std::string_view, DefinitionEntry> Definitions = {};
                std::unordered_map<std::string_view, const ModuleImport*> Imports = {};
                std::unordered_map<std::string_view, const Value*> Values = {};
                std::unordered_map<std::string_view, BasicBlock*> Blocks = {};
                std::unordered_map<const RecordNode*, RecordType*> Records = {};
                std::vector<FunctionBuildState> Functions = {};
                std::vector<BlockBuildState> BlockStates = {};
                std::vector<InstructionBuildState> InstructionStates = {};
                std::vector<std::unique_ptr<PendingValue>> PendingValues = {};
        };
    } // namespace

    std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> readModuleText(std::string_view Text)
    {
        return ModuleTextReader(Text).read();
    }
} // namespace ink::ir
