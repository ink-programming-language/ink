#include "ink/abi/name_mangling.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>

namespace ink::abi
{
  namespace
  {
    void expectFailure(std::string_view Name, DemangleErrorKind Error)
    {
      const DemangleResult Result = demangle(Name);
      EXPECT_FALSE(Result.succeeded()) << Name;
      EXPECT_FALSE(Result.identity().has_value()) << Name;
      EXPECT_EQ(Result.error(), Error) << Name << " failed as " << demangleErrorKindName(Result.error()) << " at " << Result.errorOffset();
      EXPECT_LE(Result.errorOffset(), Name.size()) << Name;
    }

    // Verifies ordinary C names, malformed reserved prefixes, unsupported ABI versions, and unknown entity tags are never accepted as Ink symbols.
    TEST(NameDemangleFailureTest, RejectsInvalidPrefixesVersionsAndEntities)
    {
      expectFailure("", DemangleErrorKind::InvalidPrefix);
      expectFailure("write", DemangleErrorKind::InvalidPrefix);
      expectFailure("_ink1F", DemangleErrorKind::InvalidPrefix);
      expectFailure("_INK", DemangleErrorKind::UnsupportedVersion);
      expectFailure("_INK0F", DemangleErrorKind::UnsupportedVersion);
      expectFailure("_INK2F", DemangleErrorKind::UnsupportedVersion);
      expectFailure("_INK1", DemangleErrorKind::UnexpectedEnd);
      expectFailure("_INK1Z", DemangleErrorKind::UnexpectedCharacter);
      EXPECT_EQ(demangle("_INK2F").errorOffset(), 4U);
      EXPECT_EQ(demangle("_INK1Z").errorOffset(), 5U);
    }

    // Verifies every proper prefix of a valid recursive symbol reports failure instead of returning a partially decoded identity.
    TEST(NameDemangleFailureTest, RejectsEveryTruncationOfValidSymbol)
    {
      const std::string Name = "_INK1FK1_U3_706B67MU3_6D6F64O1_TU5_4F776E6572X1_Ta4_i32NU4_63616C6CX1_Tt2_bpi32P2_skyri32Qt2_ynK1_U3_706B67MU3_6D6F64O0_NU4_54797065X0_";
      ASSERT_TRUE(demangle(Name).succeeded());

      for (std::size_t Length = 0; Length < Name.size(); ++Length)
      {
        EXPECT_FALSE(demangle(std::string_view(Name).substr(0, Length)).succeeded()) << "accepted truncation at " << Length;
      }
    }

    // Verifies name components reject empty data, lowercase or invalid hex, length mismatch, malformed UTF-8, and decomposed non-NFC text.
    TEST(NameDemangleFailureTest, RejectsMalformedNameComponents)
    {
      const std::string Tail = "MU1_6DO0_NU1_66X0_P0_Qv";
      expectFailure("_INK1FK1_U0_" + Tail, DemangleErrorKind::EmptyNameComponent);
      expectFailure("_INK1FK1_U1_6a" + Tail, DemangleErrorKind::InvalidHexadecimal);
      expectFailure("_INK1FK1_U1_G1" + Tail, DemangleErrorKind::InvalidHexadecimal);
      expectFailure("_INK1FK1_U2_61" + Tail, DemangleErrorKind::InvalidHexadecimal);
      expectFailure("_INK1FK1_U2_C080" + Tail, DemangleErrorKind::InvalidUtf8);
      expectFailure("_INK1FK1_U1_C3" + Tail, DemangleErrorKind::InvalidUtf8);
      expectFailure("_INK1FK1_U3_EDA080" + Tail, DemangleErrorKind::InvalidUtf8);
      expectFailure("_INK1FK1_U4_F4908080" + Tail, DemangleErrorKind::InvalidUtf8);
      expectFailure("_INK1FK1_U3_65CC81" + Tail, DemangleErrorKind::NonNormalizedName);
      expectFailure("_INK1FK1_U18446744073709551616_61" + Tail, DemangleErrorKind::NumberOverflow);
      EXPECT_EQ(demangle("_INK1FK1_U1_6a" + Tail).errorOffset(), 13U);
    }

    // Verifies all decimal fields use minimal unsigned decimal syntax and detect arithmetic overflow without exception-based conversion.
    TEST(NameDemangleFailureTest, RejectsNoncanonicalOrOverflowingNumbers)
    {
      expectFailure("_INK1FK01_U1_61MU1_6DO0_NU1_66X0_P0_Qv", DemangleErrorKind::NonCanonicalNumber);
      expectFailure("_INK1FK+1_U1_61MU1_6DO0_NU1_66X0_P0_Qv", DemangleErrorKind::NonCanonicalNumber);
      expectFailure("_INK1FK_U1_61MU1_6DO0_NU1_66X0_P0_Qv", DemangleErrorKind::NonCanonicalNumber);
      expectFailure("_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Qa01_y", DemangleErrorKind::NonCanonicalNumber);
      expectFailure("_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Qa18446744073709551616_y", DemangleErrorKind::NumberOverflow);
      expectFailure("_INK1FK18446744073709551616_", DemangleErrorKind::NumberOverflow);
    }

    // Verifies package, owner, instance, parameter, and tuple counts must match the exact number of following productions.
    TEST(NameDemangleFailureTest, RejectsMismatchedProductionCounts)
    {
      expectFailure("_INK1FK0_MU1_6DO0_NU1_66X0_P0_Qv", DemangleErrorKind::MissingPackagePath);
      expectFailure("_INK1FK2_U1_61MU1_6DO0_NU1_66X0_P0_Qv", DemangleErrorKind::UnexpectedCharacter);
      expectFailure("_INK1FK1_U1_61MU1_6DO1_NU1_66X0_P0_Qv", DemangleErrorKind::UnexpectedCharacter);
      expectFailure("_INK1FK1_U1_61MU1_6DO0_NU1_66X1_P0_Qv", DemangleErrorKind::UnexpectedCharacter);
      expectFailure("_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P1_Qv", DemangleErrorKind::InvalidType);
      expectFailure("_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Qt2_b", DemangleErrorKind::UnexpectedEnd);
      expectFailure("_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Qt1_by", DemangleErrorKind::TrailingCharacters);
    }

    // Verifies unknown, malformed, and incomplete type productions fail at the type boundary for parameters, results, arrays, and nominal types.
    TEST(NameDemangleFailureTest, RejectsUnknownOrIncompleteTypes)
    {
      const std::string Prefix = "_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Q";
      expectFailure(Prefix + "x", DemangleErrorKind::InvalidType);
      expectFailure(Prefix + "i64", DemangleErrorKind::InvalidType);
      expectFailure(Prefix + "p", DemangleErrorKind::UnexpectedEnd);
      expectFailure(Prefix + "a1_", DemangleErrorKind::UnexpectedEnd);
      expectFailure(Prefix + "a_", DemangleErrorKind::NonCanonicalNumber);
      expectFailure(Prefix + "t2_b", DemangleErrorKind::UnexpectedEnd);
      expectFailure(Prefix + "nK1_U1_61MU1_6DO0_NU1_54", DemangleErrorKind::UnexpectedEnd);
    }

    // Verifies closed values require a defined fixed-width integer type, an underscore delimiter, and the exact uppercase hexadecimal width.
    TEST(NameDemangleFailureTest, RejectsMalformedOrUnsupportedClosedValues)
    {
      const std::string Prefix = "_INK1FK1_U1_61MU1_6DO0_NU1_66X1_";
      const std::string Tail = "P0_Qv";
      expectFailure(Prefix + "Z" + Tail, DemangleErrorKind::UnexpectedCharacter);
      expectFailure(Prefix + "Vf_00000000" + Tail, DemangleErrorKind::UnsupportedValueArgumentType);
      expectFailure(Prefix + "Vz_00000000" + Tail, DemangleErrorKind::UnsupportedValueArgumentType);
      expectFailure(Prefix + "Vb_1" + Tail, DemangleErrorKind::UnsupportedValueArgumentType);
      expectFailure(Prefix + "Vy_F" + Tail, DemangleErrorKind::InvalidCanonicalBits);
      expectFailure(Prefix + "Vi32X00000001" + Tail, DemangleErrorKind::UnexpectedCharacter);
      expectFailure(Prefix + "Vi32_0000000" + Tail, DemangleErrorKind::InvalidCanonicalBits);
      expectFailure(Prefix + "Vi32_0000000G" + Tail, DemangleErrorKind::InvalidCanonicalBits);
      expectFailure(Prefix + "Vi32_ffffffff" + Tail, DemangleErrorKind::InvalidCanonicalBits);
      expectFailure(Prefix + "Vi32_000000001" + Tail, DemangleErrorKind::UnexpectedCharacter);
    }

    // Verifies canonical function and global encodings reject any otherwise unconsumed suffix, including embedded NUL bytes.
    TEST(NameDemangleFailureTest, RejectsTrailingCharacters)
    {
      const std::string Function = "_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Qv";
      const std::string Global = "_INK1GK1_U1_61MU1_6DO0_NU1_67WYi32";
      std::string WithNull = Function;
      WithNull.push_back('\0');

      expectFailure(Function + "x", DemangleErrorKind::TrailingCharacters);
      expectFailure(Global + "0", DemangleErrorKind::TrailingCharacters);
      expectFailure(WithNull, DemangleErrorKind::TrailingCharacters);
      EXPECT_EQ(demangle(Function + "x").errorOffset(), Function.size());
    }

    // Verifies global-specific mutability, separator, and value-type productions each diagnose their own missing or invalid tag.
    TEST(NameDemangleFailureTest, RejectsMalformedGlobalSuffixes)
    {
      const std::string Prefix = "_INK1GK1_U1_70MU1_6DO0_NU1_67";

      expectFailure(Prefix, DemangleErrorKind::UnexpectedEnd);
      expectFailure(Prefix + "ZYi32", DemangleErrorKind::UnexpectedCharacter);
      expectFailure(Prefix + "WZi32", DemangleErrorKind::UnexpectedCharacter);
      expectFailure(Prefix + "WY", DemangleErrorKind::UnexpectedEnd);
      expectFailure(Prefix + "WYx", DemangleErrorKind::InvalidType);
    }

    // Verifies deeply recursive types and enormous declared counts terminate with explicit errors rather than exhausting the call stack or preallocating by count.
    TEST(NameDemangleFailureTest, EnforcesResourceBounds)
    {
      const std::string Prefix = "_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_Q";
      std::string DeepestAccepted(NameManglingNestingLimit - 1, 'p');
      DeepestAccepted.push_back('y');
      std::string DeepType(NameManglingNestingLimit, 'p');
      DeepType.push_back('y');
      std::string NominalArgumentAccepted(NameManglingNestingLimit - 2, 'p');
      NominalArgumentAccepted.push_back('y');
      std::string NominalArgumentRejected(NameManglingNestingLimit - 1, 'p');
      NominalArgumentRejected.push_back('y');
      const std::string FunctionKeyPrefix = "_INK1FK1_U1_61MU1_6DO0_NU1_66X1_T";
      const std::string FunctionKeySuffix = "P0_Qv";
      const std::string NominalKeyPrefix = "_INK1FK1_U1_61MU1_6DO0_NU1_66X0_P0_QnK1_U1_61MU1_6DO0_NU1_54X1_T";

      EXPECT_TRUE(demangle(Prefix + DeepestAccepted).succeeded());
      expectFailure(Prefix + DeepType, DemangleErrorKind::NestingLimitExceeded);
      EXPECT_TRUE(demangle(FunctionKeyPrefix + DeepestAccepted + FunctionKeySuffix).succeeded());
      expectFailure(FunctionKeyPrefix + DeepType + FunctionKeySuffix, DemangleErrorKind::NestingLimitExceeded);
      EXPECT_TRUE(demangle(NominalKeyPrefix + NominalArgumentAccepted).succeeded());
      expectFailure(NominalKeyPrefix + NominalArgumentRejected, DemangleErrorKind::NestingLimitExceeded);
      expectFailure("_INK1FK999999999999999999_", DemangleErrorKind::UnexpectedEnd);
      expectFailure("_INK1FK1_U999999999999999999_", DemangleErrorKind::InvalidNameLength);
    }

    // Verifies every possible single-byte entity discriminator on an otherwise truncated symbol fails deterministically without reading past the input.
    TEST(NameDemangleFailureTest, RejectsArbitraryEntityBytesSafely)
    {
      for (unsigned int Byte = 0; Byte <= std::numeric_limits<unsigned char>::max(); ++Byte)
      {
        std::string Input = "_INK1";
        Input.push_back(static_cast<char>(Byte));
        EXPECT_FALSE(demangle(Input).succeeded()) << "accepted entity byte " << Byte;
      }
    }
  } // namespace
} // namespace ink::abi
