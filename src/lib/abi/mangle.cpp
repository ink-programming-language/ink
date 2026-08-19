#include "ink/abi/name_mangling.h"

#include "codec.h"

#include <cstdint>
#include <string>
#include <utility>

namespace ink::abi
{
  namespace
  {
    class Mangler final
    {
      public:
        MangleResult mangleFunction(const FunctionIdentity &Identity)
        {
          Output = "_INK1";
          switch (Identity.Kind)
          {
            case FunctionKind::Function:
              Output.push_back('F');
              break;
            case FunctionKind::Constructor:
              Output.push_back('C');
              break;
            case FunctionKind::Destructor:
              Output.push_back('D');
              break;
            default:
              fail(MangleErrorKind::InvalidFunctionKind);
              break;
          }
          appendScope(Identity.Path, 0);
          appendClosedInstanceKey(Identity.Instance, 0);
          Output.push_back('P');
          detail::appendUnsigned(Output, static_cast<std::uint64_t>(Identity.Parameters.size()));
          Output.push_back('_');
          for (const Type &Parameter : Identity.Parameters)
          {
            appendType(Parameter, 0);
          }
          Output.push_back('Q');
          appendType(Identity.Result, 0);
          return finish();
        }

        MangleResult mangleGlobal(const GlobalIdentity &Identity)
        {
          Output = "_INK1G";
          appendScope(Identity.Path, 0);
          switch (Identity.Mutability)
          {
            case GlobalMutability::Mutable:
              Output.push_back('W');
              break;
            case GlobalMutability::Immutable:
              Output.push_back('R');
              break;
            default:
              fail(MangleErrorKind::InvalidGlobalMutability);
              break;
          }
          Output.push_back('Y');
          appendType(Identity.ValueType, 0);
          return finish();
        }

      private:
        void fail(MangleErrorKind Failure) noexcept
        {
          if (Error == MangleErrorKind::None)
          {
            Error = Failure;
          }
        }

        void appendNameComponent(std::string_view Name)
        {
          if (Error != MangleErrorKind::None)
          {
            return;
          }
          switch (detail::validateName(Name))
          {
            case detail::NameValidationResult::Valid:
              break;
            case detail::NameValidationResult::Empty:
              fail(MangleErrorKind::EmptyNameComponent);
              return;
            case detail::NameValidationResult::InvalidUtf8:
              fail(MangleErrorKind::InvalidUtf8);
              return;
            case detail::NameValidationResult::NotNormalized:
              fail(MangleErrorKind::NonNormalizedName);
              return;
            case detail::NameValidationResult::NormalizationFailed:
              fail(MangleErrorKind::UnicodeNormalizationFailed);
              return;
          }
          Output.push_back('U');
          detail::appendUnsigned(Output, static_cast<std::uint64_t>(Name.size()));
          Output.push_back('_');
          detail::appendUpperHexadecimal(Output, Name);
        }

        void appendScope(const Scope &Path, std::size_t Depth)
        {
          if (Error != MangleErrorKind::None)
          {
            return;
          }
          if (Path.PackagePath.empty())
          {
            fail(MangleErrorKind::MissingPackagePath);
            return;
          }
          Output.push_back('K');
          detail::appendUnsigned(Output, static_cast<std::uint64_t>(Path.PackagePath.size()));
          Output.push_back('_');
          for (const std::string &Package : Path.PackagePath)
          {
            appendNameComponent(Package);
          }
          Output.push_back('M');
          appendNameComponent(Path.Module);
          Output.push_back('O');
          detail::appendUnsigned(Output, static_cast<std::uint64_t>(Path.EnclosingTypes.size()));
          Output.push_back('_');
          for (const OwnerType &Owner : Path.EnclosingTypes)
          {
            Output.push_back('T');
            appendNameComponent(Owner.Name);
            appendClosedInstanceKey(Owner.Instance, Depth);
          }
          Output.push_back('N');
          appendNameComponent(Path.DeclarationName);
        }

        void appendClosedInstanceKey(const ClosedInstanceKey &Key, std::size_t Depth)
        {
          if (Error != MangleErrorKind::None)
          {
            return;
          }
          Output.push_back('X');
          detail::appendUnsigned(Output, static_cast<std::uint64_t>(Key.Arguments.size()));
          Output.push_back('_');
          for (const ClosedArgument &Argument : Key.Arguments)
          {
            if (Argument.kind() == ClosedArgumentKind::Type)
            {
              Output.push_back('T');
              appendType(Argument.argumentType(), Depth);
              continue;
            }
            if (detail::canonicalBitHexDigitCount(Argument.argumentType()) == 0)
            {
              fail(MangleErrorKind::UnsupportedValueArgumentType);
              return;
            }
            if (!detail::validateCanonicalBits(Argument.argumentType(), Argument.canonicalBits()))
            {
              fail(MangleErrorKind::InvalidCanonicalBits);
              return;
            }
            Output.push_back('V');
            appendType(Argument.argumentType(), Depth);
            Output.push_back('_');
            Output.append(Argument.canonicalBits());
          }
        }

        void appendType(const Type &TypeValue, std::size_t Depth)
        {
          if (Error != MangleErrorKind::None)
          {
            return;
          }
          if (Depth >= NameManglingNestingLimit)
          {
            fail(MangleErrorKind::NestingLimitExceeded);
            return;
          }
          switch (TypeValue.kind())
          {
            case TypeKind::Void:
              Output.push_back('v');
              return;
            case TypeKind::Bool:
              Output.push_back('b');
              return;
            case TypeKind::Byte:
              Output.push_back('y');
              return;
            case TypeKind::I32:
              Output.append("i32");
              return;
            case TypeKind::PointerSize:
              Output.push_back('z');
              return;
            case TypeKind::F16:
              Output.push_back('h');
              return;
            case TypeKind::F32:
              Output.push_back('f');
              return;
            case TypeKind::F64:
              Output.push_back('d');
              return;
            case TypeKind::Const:
              Output.push_back('k');
              appendType(*TypeValue.elementType(), Depth + 1);
              return;
            case TypeKind::Pointer:
              Output.push_back('p');
              appendType(*TypeValue.elementType(), Depth + 1);
              return;
            case TypeKind::Reference:
              Output.push_back('r');
              appendType(*TypeValue.elementType(), Depth + 1);
              return;
            case TypeKind::Slice:
              Output.push_back('s');
              appendType(*TypeValue.elementType(), Depth + 1);
              return;
            case TypeKind::Array:
              Output.push_back('a');
              detail::appendUnsigned(Output, TypeValue.arrayLength());
              Output.push_back('_');
              appendType(*TypeValue.elementType(), Depth + 1);
              return;
            case TypeKind::Tuple:
              Output.push_back('t');
              detail::appendUnsigned(Output, static_cast<std::uint64_t>(TypeValue.elements().size()));
              Output.push_back('_');
              for (const Type &Element : TypeValue.elements())
              {
                appendType(Element, Depth + 1);
              }
              return;
            case TypeKind::Nominal:
              Output.push_back('n');
              appendScope(TypeValue.nominalIdentity()->Path, Depth + 1);
              appendClosedInstanceKey(TypeValue.nominalIdentity()->Instance, Depth + 1);
              return;
          }
        }

        MangleResult finish()
        {
          return Error == MangleErrorKind::None ? MangleResult::success(std::move(Output)) : MangleResult::failure(Error);
        }

        std::string Output;
        MangleErrorKind Error = MangleErrorKind::None;
    };
  } // namespace

  MangleResult mangle(const FunctionIdentity &Identity)
  {
    Mangler Encoder;
    return Encoder.mangleFunction(Identity);
  }

  MangleResult mangle(const GlobalIdentity &Identity)
  {
    Mangler Encoder;
    return Encoder.mangleGlobal(Identity);
  }

  MangleResult mangle(const LinkageIdentity &Identity)
  {
    if (const FunctionIdentity *Function = std::get_if<FunctionIdentity>(&Identity))
    {
      return mangle(*Function);
    }
    return mangle(*std::get_if<GlobalIdentity>(&Identity));
  }
} // namespace ink::abi
