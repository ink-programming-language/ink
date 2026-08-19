#include "abi_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

namespace ink::abi
{
  namespace
  {
    using namespace test;

    // Verifies every specification golden vector decodes successfully and re-encodes to the identical canonical byte string.
    TEST(NameDemangleTest, CanonicallyDecodesSpecificationGoldenVectors)
    {
      const std::string Names[] = {
          "_INK1FK1_U4_67616D65MU4_6D61696EO0_NU4_6D61696EX0_P0_Qi32",
          "_INK1FK1_U4_6D617468MU3_6F7073O0_NU7_636F6E76657274X0_P1_i32Qd",
          "_INK1FK1_U4_6D617468MU3_6F7073O0_NU7_636F6E76657274X0_P1_dQi32",
          "_INK1FK1_U4_67616D65MU4_6D61696EO0_NU5_76616C7565X1_Vi32_00000001P0_Qi32",
          "_INK1FK1_U4_67616D65MU4_6D61696EO0_NU5_76616C7565X1_Vi32_00000002P0_Qi32",
          "_INK1GK1_U4_67616D65MU5_7374617465O0_NU7_636F756E746572WYi32",
          "_INK1FK1_U7_6C696272617279MU4_6D617468O0_NU6_616E73776572X0_P1_i32Qi32",
      };

      for (const std::string &Name : Names)
      {
        const DemangleResult Decoded = demangle(Name);
        ASSERT_TRUE(Decoded.succeeded()) << Name << ": " << demangleErrorKindName(Decoded.error()) << " at " << Decoded.errorOffset();
        const MangleResult Reencoded = mangle(*Decoded.identity());
        ASSERT_TRUE(Reencoded.succeeded()) << Name << ": " << mangleErrorKindName(Reencoded.error());
        EXPECT_EQ(*Reencoded.mangledName(), Name);
      }
    }

    // Verifies demangling restores NFC Unicode components as structured package, module, and declaration fields rather than a dotted string.
    TEST(NameDemangleTest, RestoresUnicodeDeclarationPath)
    {
      const DemangleResult Result = demangle("_INK1FK1_U5_636166C3A9MU3_617070O0_NU6_E78EA9E5AEB6X0_P0_Qv");

      ASSERT_TRUE(Result.succeeded()) << demangleErrorKindName(Result.error());
      const FunctionIdentity *Function = std::get_if<FunctionIdentity>(&*Result.identity());
      ASSERT_NE(Function, nullptr);
      EXPECT_EQ(Function->Path.PackagePath, (std::vector<std::string>{u8"café"}));
      EXPECT_EQ(Function->Path.Module, "app");
      EXPECT_TRUE(Function->Path.EnclosingTypes.empty());
      EXPECT_EQ(Function->Path.DeclarationName, u8"玩家");
      EXPECT_TRUE(Function->Instance.Arguments.empty());
      EXPECT_TRUE(Function->Parameters.empty());
      EXPECT_EQ(Function->Result, Type::voidType());
    }

    // Verifies demangling reconstructs nested owners and preserves their distinct type/value instance keys in outer-to-inner order.
    TEST(NameDemangleTest, RestoresOwnerAndFunctionInstanceKeys)
    {
      const std::string Name = "_INK1FK2_U3_706B67U3_737562MU3_6D6F64O2_TU5_4F75746572X1_Ti32TU5_496E6E6572X1_Vi32_FFFFFFFFNU4_63616C6CX2_Tpi32Vi32_00000001P0_Qv";
      const DemangleResult Result = demangle(Name);

      ASSERT_TRUE(Result.succeeded()) << demangleErrorKindName(Result.error()) << " at " << Result.errorOffset();
      const FunctionIdentity *Function = std::get_if<FunctionIdentity>(&*Result.identity());
      ASSERT_NE(Function, nullptr);
      ASSERT_EQ(Function->Path.PackagePath, (std::vector<std::string>{"pkg", "sub"}));
      ASSERT_EQ(Function->Path.EnclosingTypes.size(), 2U);
      EXPECT_EQ(Function->Path.EnclosingTypes[0].Name, "Outer");
      ASSERT_EQ(Function->Path.EnclosingTypes[0].Instance.Arguments.size(), 1U);
      EXPECT_EQ(Function->Path.EnclosingTypes[0].Instance.Arguments[0], ClosedArgument::type(Type::i32Type()));
      EXPECT_EQ(Function->Path.EnclosingTypes[1].Name, "Inner");
      ASSERT_EQ(Function->Path.EnclosingTypes[1].Instance.Arguments.size(), 1U);
      EXPECT_EQ(Function->Path.EnclosingTypes[1].Instance.Arguments[0], ClosedArgument::value(Type::i32Type(), "FFFFFFFF"));
      ASSERT_EQ(Function->Instance.Arguments.size(), 2U);
      EXPECT_EQ(Function->Instance.Arguments[0], ClosedArgument::type(Type::pointerType(Type::i32Type())));
      EXPECT_EQ(Function->Instance.Arguments[1], ClosedArgument::value(Type::i32Type(), "00000001"));
    }

    // Verifies all function entity tags and both global mutability tags survive a complete structured round trip.
    TEST(NameDemangleTest, RestoresEntityKindsAndGlobalMutability)
    {
      const Scope Path = makeScope({"pkg"}, "module", "symbol");
      const LinkageIdentity Identities[] = {
          makeFunction(Path, {}, Type::voidType(), {}, FunctionKind::Function),
          makeFunction(Path, {Type::pointerType(Type::byteType())}, Type::voidType(), {}, FunctionKind::Constructor),
          makeFunction(Path, {Type::pointerType(Type::byteType())}, Type::voidType(), {}, FunctionKind::Destructor),
          makeGlobal(Path, Type::i32Type(), GlobalMutability::Mutable),
          makeGlobal(Path, Type::i32Type(), GlobalMutability::Immutable),
      };

      for (const LinkageIdentity &Identity : Identities)
      {
        expectRoundTrip(Identity);
      }
    }

    // Verifies a generic nominal type recursively restores its own scope, owner keys, and closed type argument.
    TEST(NameDemangleTest, RestoresRecursiveNominalIdentity)
    {
      ClosedInstanceKey OwnerKey;
      OwnerKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "00000002"));
      ClosedInstanceKey TypeKey;
      TypeKey.Arguments.push_back(ClosedArgument::type(Type::arrayType(4, Type::byteType())));
      const NominalIdentity Nominal{makeScope({"collections"}, "box", "Entry", {{"Box", std::move(OwnerKey)}}), std::move(TypeKey)};
      const FunctionIdentity Expected = makeFunction(makeScope({"collections"}, "box", "get"), {Type::nominalType(Nominal)}, Type::tupleType({Type::boolType(), Type::nominalType(Nominal)}));
      const std::string Name = mangleOrReport(Expected);

      const DemangleResult Decoded = demangle(Name);
      ASSERT_TRUE(Decoded.succeeded()) << demangleErrorKindName(Decoded.error()) << " at " << Decoded.errorOffset();
      EXPECT_EQ(*Decoded.identity(), LinkageIdentity(Expected));
    }
  } // namespace
} // namespace ink::abi
