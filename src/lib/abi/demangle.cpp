#include "ink/abi/name_mangling.h"

#include "codec.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::abi
{
  namespace
  {
    class Demangler final
    {
      public:
        explicit Demangler(std::string_view Input) noexcept
            : Input(Input)
        {
        }

        DemangleResult run()
        {
          if (!parsePrefix())
          {
            return failure();
          }
          if (atEnd())
          {
            fail(DemangleErrorKind::UnexpectedEnd);
            return failure();
          }

          const char EntityKind = Input[Position++];
          if (EntityKind == 'F' || EntityKind == 'C' || EntityKind == 'D')
          {
            FunctionIdentity Identity;
            Identity.Kind = EntityKind == 'F' ? FunctionKind::Function : EntityKind == 'C' ? FunctionKind::Constructor : FunctionKind::Destructor;
            parseScope(Identity.Path, 0);
            parseClosedInstanceKey(Identity.Instance, 0);
            consume('P');
            std::size_t ParameterCount = 0;
            parseCount(ParameterCount);
            for (std::size_t Index = 0; Index < ParameterCount && !failed(); ++Index)
            {
              Type Parameter;
              if (parseType(Parameter, 0))
              {
                Identity.Parameters.push_back(std::move(Parameter));
              }
            }
            consume('Q');
            parseType(Identity.Result, 0);
            if (failed())
            {
              return failure();
            }
            if (!atEnd())
            {
              fail(DemangleErrorKind::TrailingCharacters);
              return failure();
            }
            return DemangleResult::success(LinkageIdentity(std::move(Identity)));
          }

          if (EntityKind == 'G')
          {
            GlobalIdentity Identity;
            parseScope(Identity.Path, 0);
            if (!failed() && !atEnd())
            {
              if (Input[Position] == 'W')
              {
                Identity.Mutability = GlobalMutability::Mutable;
                ++Position;
              }
              else if (Input[Position] == 'R')
              {
                Identity.Mutability = GlobalMutability::Immutable;
                ++Position;
              }
              else
              {
                fail(DemangleErrorKind::UnexpectedCharacter);
              }
            }
            else if (!failed())
            {
              fail(DemangleErrorKind::UnexpectedEnd);
            }
            consume('Y');
            parseType(Identity.ValueType, 0);
            if (failed())
            {
              return failure();
            }
            if (!atEnd())
            {
              fail(DemangleErrorKind::TrailingCharacters);
              return failure();
            }
            return DemangleResult::success(LinkageIdentity(std::move(Identity)));
          }

          fail(DemangleErrorKind::UnexpectedCharacter, Position - 1);
          return failure();
        }

      private:
        bool parsePrefix()
        {
          static constexpr std::string_view Prefix = "_INK";
          for (std::size_t Index = 0; Index < Prefix.size(); ++Index)
          {
            if (Position >= Input.size() || Input[Position] != Prefix[Index])
            {
              fail(DemangleErrorKind::InvalidPrefix);
              return false;
            }
            ++Position;
          }
          if (atEnd() || Input[Position] != '1')
          {
            fail(DemangleErrorKind::UnsupportedVersion);
            return false;
          }
          ++Position;
          return true;
        }

        bool atEnd() const noexcept
        {
          return Position >= Input.size();
        }

        bool failed() const noexcept
        {
          return Error != DemangleErrorKind::None;
        }

        void fail(DemangleErrorKind Failure) noexcept
        {
          fail(Failure, Position);
        }

        void fail(DemangleErrorKind Failure, std::size_t Offset) noexcept
        {
          if (!failed())
          {
            Error = Failure;
            ErrorPosition = Offset;
          }
        }

        bool consume(char Expected)
        {
          if (failed())
          {
            return false;
          }
          if (atEnd())
          {
            fail(DemangleErrorKind::UnexpectedEnd);
            return false;
          }
          if (Input[Position] != Expected)
          {
            fail(DemangleErrorKind::UnexpectedCharacter);
            return false;
          }
          ++Position;
          return true;
        }

        template <typename Unsigned>
        bool parseUnsigned(Unsigned &Value)
        {
          static_assert(std::numeric_limits<Unsigned>::is_integer && !std::numeric_limits<Unsigned>::is_signed, "parseUnsigned requires an unsigned integer type");
          if (failed())
          {
            return false;
          }
          if (atEnd() || Input[Position] < '0' || Input[Position] > '9')
          {
            fail(DemangleErrorKind::NonCanonicalNumber);
            return false;
          }
          const std::size_t Start = Position;
          Value = 0;
          while (!atEnd() && Input[Position] >= '0' && Input[Position] <= '9')
          {
            const Unsigned Digit = static_cast<Unsigned>(Input[Position] - '0');
            if (Value > (std::numeric_limits<Unsigned>::max() - Digit) / 10)
            {
              fail(DemangleErrorKind::NumberOverflow);
              return false;
            }
            Value = static_cast<Unsigned>(Value * 10 + Digit);
            ++Position;
          }
          if (Position - Start > 1 && Input[Start] == '0')
          {
            fail(DemangleErrorKind::NonCanonicalNumber, Start);
            return false;
          }
          return true;
        }

        bool parseCount(std::size_t &Count)
        {
          return parseUnsigned(Count) && consume('_');
        }

        bool parseNameComponent(std::string &Name)
        {
          if (!consume('U'))
          {
            return false;
          }
          std::size_t ByteCount = 0;
          if (!parseUnsigned(ByteCount) || !consume('_'))
          {
            return false;
          }
          if (ByteCount == 0)
          {
            fail(DemangleErrorKind::EmptyNameComponent);
            return false;
          }
          if (ByteCount > std::numeric_limits<std::size_t>::max() / 2)
          {
            fail(DemangleErrorKind::InvalidNameLength);
            return false;
          }
          const std::size_t HexLength = ByteCount * 2;
          if (Input.size() - Position < HexLength)
          {
            fail(DemangleErrorKind::InvalidNameLength);
            return false;
          }

          const std::size_t HexStart = Position;
          Name.clear();
          Name.reserve(ByteCount);
          for (std::size_t Index = 0; Index < ByteCount; ++Index)
          {
            const std::uint8_t High = detail::uppercaseHexadecimalValue(Input[Position]);
            const std::uint8_t Low = detail::uppercaseHexadecimalValue(Input[Position + 1]);
            if (High == 0xFF || Low == 0xFF)
            {
              fail(DemangleErrorKind::InvalidHexadecimal, High == 0xFF ? Position : Position + 1);
              return false;
            }
            Name.push_back(static_cast<char>((High << 4U) | Low));
            Position += 2;
          }

          switch (detail::validateName(Name))
          {
            case detail::NameValidationResult::Valid:
              return true;
            case detail::NameValidationResult::Empty:
              fail(DemangleErrorKind::EmptyNameComponent, HexStart);
              return false;
            case detail::NameValidationResult::InvalidUtf8:
              fail(DemangleErrorKind::InvalidUtf8, HexStart);
              return false;
            case detail::NameValidationResult::NotNormalized:
              fail(DemangleErrorKind::NonNormalizedName, HexStart);
              return false;
            case detail::NameValidationResult::NormalizationFailed:
              fail(DemangleErrorKind::UnicodeNormalizationFailed, HexStart);
              return false;
          }
          return false;
        }

        bool parseScope(Scope &Path, std::size_t Depth)
        {
          if (!consume('K'))
          {
            return false;
          }
          std::size_t PackageCount = 0;
          if (!parseCount(PackageCount))
          {
            return false;
          }
          if (PackageCount == 0)
          {
            fail(DemangleErrorKind::MissingPackagePath);
            return false;
          }
          for (std::size_t Index = 0; Index < PackageCount && !failed(); ++Index)
          {
            std::string Package;
            if (parseNameComponent(Package))
            {
              Path.PackagePath.push_back(std::move(Package));
            }
          }
          if (!consume('M') || !parseNameComponent(Path.Module) || !consume('O'))
          {
            return false;
          }
          std::size_t OwnerCount = 0;
          if (!parseCount(OwnerCount))
          {
            return false;
          }
          for (std::size_t Index = 0; Index < OwnerCount && !failed(); ++Index)
          {
            OwnerType Owner;
            if (!consume('T') || !parseNameComponent(Owner.Name) || !parseClosedInstanceKey(Owner.Instance, Depth))
            {
              return false;
            }
            Path.EnclosingTypes.push_back(std::move(Owner));
          }
          return consume('N') && parseNameComponent(Path.DeclarationName);
        }

        bool parseClosedInstanceKey(ClosedInstanceKey &Key, std::size_t Depth)
        {
          if (!consume('X'))
          {
            return false;
          }
          std::size_t ArgumentCount = 0;
          if (!parseCount(ArgumentCount))
          {
            return false;
          }
          for (std::size_t Index = 0; Index < ArgumentCount && !failed(); ++Index)
          {
            if (atEnd())
            {
              fail(DemangleErrorKind::UnexpectedEnd);
              return false;
            }
            const char ArgumentKind = Input[Position++];
            Type ArgumentType;
            if (ArgumentKind != 'T' && ArgumentKind != 'V')
            {
              fail(DemangleErrorKind::UnexpectedCharacter, Position - 1);
              return false;
            }
            if (!parseType(ArgumentType, Depth))
            {
              return false;
            }
            if (ArgumentKind == 'T')
            {
              Key.Arguments.push_back(ClosedArgument::type(std::move(ArgumentType)));
              continue;
            }

            const std::size_t DigitCount = detail::canonicalBitHexDigitCount(ArgumentType);
            if (DigitCount == 0)
            {
              fail(DemangleErrorKind::UnsupportedValueArgumentType);
              return false;
            }
            if (!consume('_'))
            {
              return false;
            }
            if (Input.size() - Position < DigitCount)
            {
              fail(DemangleErrorKind::InvalidCanonicalBits);
              return false;
            }
            const std::size_t BitsStart = Position;
            for (std::size_t Digit = 0; Digit < DigitCount; ++Digit)
            {
              if (detail::uppercaseHexadecimalValue(Input[Position + Digit]) == 0xFF)
              {
                fail(DemangleErrorKind::InvalidCanonicalBits, Position + Digit);
                return false;
              }
            }
            Position += DigitCount;
            std::string Bits(Input.substr(BitsStart, DigitCount));
            Key.Arguments.push_back(ClosedArgument::value(std::move(ArgumentType), std::move(Bits)));
          }
          return !failed();
        }

        bool parseType(Type &Result, std::size_t Depth)
        {
          if (failed())
          {
            return false;
          }
          if (Depth >= NameManglingNestingLimit)
          {
            fail(DemangleErrorKind::NestingLimitExceeded);
            return false;
          }
          if (atEnd())
          {
            fail(DemangleErrorKind::UnexpectedEnd);
            return false;
          }

          const char Kind = Input[Position++];
          switch (Kind)
          {
            case 'v':
              Result = Type::voidType();
              return true;
            case 'b':
              Result = Type::boolType();
              return true;
            case 'y':
              Result = Type::byteType();
              return true;
            case 'z':
              Result = Type::pointerSizeType();
              return true;
            case 'h':
              Result = Type::f16Type();
              return true;
            case 'f':
              Result = Type::f32Type();
              return true;
            case 'd':
              Result = Type::f64Type();
              return true;
            case 'i':
              if (Input.size() - (Position - 1) < 3)
              {
                fail(DemangleErrorKind::UnexpectedEnd, Position - 1);
                return false;
              }
              if (Input.substr(Position - 1, 3) != "i32")
              {
                fail(DemangleErrorKind::InvalidType, Position - 1);
                return false;
              }
              Position += 2;
              Result = Type::i32Type();
              return true;
            case 'k':
            case 'p':
            case 'r':
            case 's':
            {
              Type Element;
              if (!parseType(Element, Depth + 1))
              {
                return false;
              }
              Result = Kind == 'k' ? Type::constType(std::move(Element)) : Kind == 'p' ? Type::pointerType(std::move(Element)) : Kind == 'r' ? Type::referenceType(std::move(Element)) : Type::sliceType(std::move(Element));
              return true;
            }
            case 'a':
            {
              std::uint64_t Length = 0;
              if (!parseUnsigned(Length) || !consume('_'))
              {
                return false;
              }
              Type Element;
              if (!parseType(Element, Depth + 1))
              {
                return false;
              }
              Result = Type::arrayType(Length, std::move(Element));
              return true;
            }
            case 't':
            {
              std::size_t ElementCount = 0;
              if (!parseCount(ElementCount))
              {
                return false;
              }
              std::vector<Type> Elements;
              for (std::size_t Index = 0; Index < ElementCount && !failed(); ++Index)
              {
                Type Element;
                if (parseType(Element, Depth + 1))
                {
                  Elements.push_back(std::move(Element));
                }
              }
              if (failed())
              {
                return false;
              }
              Result = Type::tupleType(std::move(Elements));
              return true;
            }
            case 'n':
            {
              NominalIdentity Identity;
              if (!parseScope(Identity.Path, Depth + 1) || !parseClosedInstanceKey(Identity.Instance, Depth + 1))
              {
                return false;
              }
              Result = Type::nominalType(std::move(Identity));
              return true;
            }
            default:
              fail(DemangleErrorKind::InvalidType, Position - 1);
              return false;
          }
        }

        DemangleResult failure() const noexcept
        {
          return DemangleResult::failure(Error, ErrorPosition);
        }

        std::string_view Input;
        std::size_t Position = 0;
        DemangleErrorKind Error = DemangleErrorKind::None;
        std::size_t ErrorPosition = 0;
    };
  } // namespace

  DemangleResult demangle(std::string_view Name)
  {
    Demangler Decoder(Name);
    return Decoder.run();
  }
} // namespace ink::abi
