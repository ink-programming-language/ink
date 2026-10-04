#include "ink/abi/name_mangling.h"
#include "ink/ir/linkage.h"

#include <gtest/gtest.h>
#include <algorithm>

namespace ink::abi::test
{
  namespace
  {
    ModuleIdentity module()
    {
      return {{"org.example", {"demo"}, "1.0.0", {}}, {"math"}};
    }

    Record function(const ModuleIdentity &Module, std::string_view Name, std::vector<Record> Parameters = {}, Record Return = {'i', "32"})
    {
      const auto P = record('L', Parameters);
      const auto Receiver = Record{'A', "n"};
      const auto Decl = ir::declarationRecord(Module, {}, 'f', Name, record('H', {Receiver, P}));
      return record('F', {Decl, {'X', {}}, record('S', {{'B', "i"}, {'C', "c"}, Receiver, P, record('Q', {Return})})});
    }

    Record generic(Record Pattern, Record Closed, Record Schema = {'T', {}}, Record Argument = record('T', {{'i', "32"}}))
    {
      auto F = *childRecords(function(module(), "f", {Closed}));
      auto D = *childRecords(F[0]);
      D[5] = record('G', {Schema});
      D[6] = record('H', {{'A', "n"}, record('L', {Pattern})});
      F[0] = record('R', D);
      F[1] = record('X', {Argument});
      return record('F', F);
    }
  } // namespace

  // The published example is a golden vector, including ASCII payload lengths and escaping.
  TEST(NameManglingTest, MatchesPublishedAddSymbol)
  {
    const auto Result = mangle(function(module(), "add", {{'i', "32"}, {'i', "32"}}));
    ASSERT_TRUE(Result) << Result.Error;
    EXPECT_EQ(Result.Name, "_INK2F139_R94_P42_N13_org_2EexampleL7_N4_demoN9_1_2E0_2E0L0_M7_N4_mathO0_K1_fN3_addG0_H18_A1_nL10_i2_32i2_32X0_S34_B1_iC1_cA1_nL10_i2_32i2_32Q5_i2_32");
    const auto Parsed = demangle(Result.Name);
    ASSERT_TRUE(Parsed) << Parsed.Error;
    EXPECT_EQ(mangle(*Parsed.Identity).Name, Result.Name);
  }

  // Package authority, revision, variants and path segmentation independently distinguish symbols.
  TEST(NameManglingTest, SeparatesPackageAndModuleComponents)
  {
    auto M = module();
    const auto Baseline = mangle(function(M, "f")).Name;
    M.Package.Authority = "another.example";
    EXPECT_NE(mangle(function(M, "f")).Name, Baseline);
    M = module();
    M.Package.Revision = "2.0.0";
    EXPECT_NE(mangle(function(M, "f")).Name, Baseline);
    M = module();
    M.Package.Variant = {{"feature", "enabled"}, {"cfg", "checked"}};
    const auto First = mangle(function(M, "f"));
    std::reverse(M.Package.Variant.begin(), M.Package.Variant.end());
    EXPECT_EQ(mangle(function(M, "f")).Name, First.Name);
    M.Package.Variant.push_back({"cfg", "other"});
    EXPECT_FALSE(mangle(function(M, "f")));
    M = module();
    M.Path = {"a.b"};
    const auto Single = mangle(function(M, "f")).Name;
    M.Path = {"a", "b"};
    EXPECT_NE(mangle(function(M, "f")).Name, Single);
  }

  // Identifiers use UTF-8 bytes, NFC and one canonical escape spelling, including literal underscores.
  TEST(NameManglingTest, CanonicalUnicodeNames)
  {
    EXPECT_EQ(encodeRecord(nameRecord("a_b")), "N5_a_5Fb");
    EXPECT_EQ(encodeRecord(nameRecord("中")), "N9__E4_B8_AD");
    EXPECT_TRUE(mangle(function(module(), "中")));
    EXPECT_FALSE(mangle(function(module(), "e\xCC\x81")));
    EXPECT_FALSE(mangle(function(module(), std::string("a\0b", 3))));
    EXPECT_FALSE(decodeName({'N', "_61"}));
    EXPECT_FALSE(decodeName({'N', "_c3_A9"}));
    EXPECT_FALSE(decodeName({'N', "_FF"}));
  }

  // Every concrete primitive, qualifier, array length and function type is reversible without target layout.
  TEST(NameManglingTest, EncodesConcreteTypeAlgebra)
  {
    const Record Types[] = {
        {'v', {}},
        {'b', {}},
        {'i', "257"},
        {'u', "1"},
        {'f', "16"},
        {'f', "32"},
        {'f', "64"},
        {'p', "r" + encodeRecord({'i', "32"})},
        {'p', "w" + encodeRecord({'i', "32"})},
        {'r', "w" + encodeRecord({'i', "32"})},
        {'s', "r" + encodeRecord({'u', "8"})},
        record('a', {{'D', "0"}, {'i', "32"}}),
        record('a', {{'D', "18446744073709551615"}, {'b', {}}}),
        record('q', {record('L', {{'i', "32"}}), record('Q', {{'b', {}}})}),
    };
    for (const auto &Type : Types)
    {
      const auto Result = mangle(record('T', {Type}));
      ASSERT_TRUE(Result) << encodeRecord(Type) << ": " << Result.Error;
      const auto Parsed = demangle(Result.Name);
      ASSERT_TRUE(Parsed);
      EXPECT_EQ(*Parsed.Identity, record('T', {Type}));
    }
    EXPECT_FALSE(mangle(record('T', {{'i', "0"}})));
    EXPECT_FALSE(mangle(record('T', {{'f', "80"}})));
    EXPECT_FALSE(mangle(record('T', {{'m', {}}})));
    EXPECT_FALSE(mangle(record('T', {record('t', {{'i', "32"}})})));
  }

  // Open generic overload patterns remain distinct when both instances have the same closed signature.
  TEST(NameManglingTest, KeepsGenericDeclarationPattern)
  {
    const auto Variable = record('g', {{'D', "0"}, {'D', "0"}});
    const auto Dependent = mangle(generic(Variable, {'i', "32"}));
    const auto Fixed = mangle(generic({'i', "32"}, {'i', "32"}));
    ASSERT_TRUE(Dependent) << Dependent.Error;
    ASSERT_TRUE(Fixed) << Fixed.Error;
    EXPECT_NE(Dependent.Name, Fixed.Name);
    EXPECT_FALSE(mangle(generic(Variable, {'b', {}})));
    EXPECT_FALSE(mangle(generic(record('g', {{'D', "0"}, {'D', "1"}}), {'i', "32"})));
    EXPECT_FALSE(mangle(generic(record('g', {{'D', "1"}, {'D', "0"}}), {'i', "32"})));
  }

  // Array value parameters use typed exact bits and reject negative, mismatched or unbound lengths.
  TEST(NameManglingTest, SubstitutesTypedArrayLength)
  {
    const auto Variable = record('g', {{'D', "0"}, {'D', "0"}});
    const auto Pattern = record('a', {Variable, {'u', "8"}});
    const auto Closed = record('a', {{'D', "3"}, {'u', "8"}});
    const auto Schema = record('V', {{'u', "32"}});
    EXPECT_TRUE(mangle(generic(Pattern, Closed, Schema, record('V', {{'u', "32"}, {'B', "00000003"}}))));
    EXPECT_FALSE(mangle(generic(Pattern, Closed, Schema, record('V', {{'i', "32"}, {'B', "00000003"}}))));
    EXPECT_FALSE(mangle(generic(Pattern, Closed, Schema, record('V', {{'u', "32"}, {'B', "00000004"}}))));
    EXPECT_FALSE(mangle(generic(Pattern, Closed, Schema, record('V', {{'u', "32"}, {'B', "3"}}))));
    EXPECT_FALSE(mangle(generic(Pattern, Closed, record('V', {{'i', "32"}}), record('V', {{'i', "32"}, {'B', "FFFFFFFF"}}))));
    EXPECT_TRUE(mangle(generic(Pattern, Closed, record('V', {{'u', "128"}}), record('V', {{'u', "128"}, {'B', "00000000000000000000000000000003"}}))));
  }

  // Floating constants preserve signed zero and NaN payloads; integer high unused bits and lowercase are noncanonical.
  TEST(NameManglingTest, PreservesExactConstantBits)
  {
    const auto Encode = [](std::string Bits)
    {
      return mangle(generic({'i', "32"}, {'i', "32"}, record('V', {{'f', "32"}}), record('V', {{'f', "32"}, {'B', std::move(Bits)}})));
    };
    EXPECT_NE(Encode("00000000").Name, Encode("80000000").Name);
    const auto A = Encode("7FC00001");
    const auto B = Encode("7FC00002");
    ASSERT_TRUE(A);
    ASSERT_TRUE(B);
    EXPECT_NE(A.Name, B.Name);
    EXPECT_FALSE(Encode("7fc00001"));
    EXPECT_FALSE(mangle(generic({'i', "32"}, {'i', "32"}, record('V', {{'u', "3"}}), record('V', {{'u', "3"}, {'B', "F"}}))));
  }

  // Return types belong to link signatures; changing the result must not alter declaration overload identity.
  TEST(NameManglingTest, ReturnTypeDoesNotDefineOverload)
  {
    const auto A = function(module(), "f", {}, {'i', "32"});
    const auto B = function(module(), "f", {}, {'b', {}});
    EXPECT_EQ(childRecords(A)->front(), childRecords(B)->front());
    EXPECT_NE(mangle(A).Name, mangle(B).Name);
    auto F = *childRecords(A);
    auto S = *childRecords(F[2]);
    S[3] = record('L', {{'i', "32"}});
    F[2] = record('S', S);
    EXPECT_FALSE(mangle(record('F', F)));
  }

  // Truncation, overflowing lengths, noncanonical numbers, wrong versions and appended records are rejected.
  TEST(NameManglingTest, RejectsMalformedWireAndHonorsBudgets)
  {
    const auto Good = mangle(function(module(), "f")).Name;
    for (std::size_t Size = 0; Size < Good.size(); ++Size)
    {
      EXPECT_FALSE(demangle(std::string_view(Good).substr(0, Size))) << Size;
    }
    EXPECT_FALSE(demangle(Good + "L0_"));
    EXPECT_FALSE(demangle("_INK1" + Good.substr(5)));
    EXPECT_FALSE(demangle("_INK2F01_a"));
    EXPECT_FALSE(demangle("_INK2F18446744073709551616_"));
    EXPECT_FALSE(demangle("_INK2T5_i02_32"));
    EXPECT_FALSE(demangle(Good, {Good.size() - 1, 128, 65536}));
    EXPECT_FALSE(demangle(Good, {1024 * 1024, 1, 65536}));
    EXPECT_FALSE(demangle(Good, {1024 * 1024, 128, 1}));
    EXPECT_TRUE(demangle(Good, {Good.size(), 128, 65536}));
  }

  // Reflection getter identities carry the complete package and module and decode without filesystem context.
  TEST(NameManglingTest, RoundTripsModuleIdentity)
  {
    const auto M = module();
    const auto Encoded = mangle(record('J', {packageRecord(M.Package), moduleRecord(M)}));
    ASSERT_TRUE(Encoded);
    const auto Decoded = demangle(Encoded.Name);
    ASSERT_TRUE(Decoded);
    EXPECT_EQ(moduleIdentity(*Decoded.Identity), M);
  }
} // namespace ink::abi::test
