#include "abi_test_support.h"

#include <gtest/gtest.h>

#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ink::abi
{
  namespace
  {
    using namespace test;

    // Verifies every Ink linkage name printed as a complete golden vector in the name-mangling specification.
    TEST(NameMangleTest, MatchesSpecificationGoldenVectors)
    {
      const FunctionIdentity Main = makeFunction(makeScope({"game"}, "main", "main"), {}, Type::i32Type());
      const FunctionIdentity ConvertI32 = makeFunction(makeScope({"math"}, "ops", "convert"), {Type::i32Type()}, Type::f64Type());
      const FunctionIdentity ConvertF64 = makeFunction(makeScope({"math"}, "ops", "convert"), {Type::f64Type()}, Type::i32Type());
      ClosedInstanceKey ValueOneKey;
      ValueOneKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "00000001"));
      ClosedInstanceKey ValueTwoKey;
      ValueTwoKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "00000002"));
      const FunctionIdentity ValueOne = makeFunction(makeScope({"game"}, "main", "value"), {}, Type::i32Type(), std::move(ValueOneKey));
      const FunctionIdentity ValueTwo = makeFunction(makeScope({"game"}, "main", "value"), {}, Type::i32Type(), std::move(ValueTwoKey));
      const GlobalIdentity Counter = makeGlobal(makeScope({"game"}, "state", "counter"), Type::i32Type());
      const FunctionIdentity ImportedAnswer = makeFunction(makeScope({"library"}, "math", "answer"), {Type::i32Type()}, Type::i32Type());

      EXPECT_EQ(mangleOrReport(Main), "_INK1FK1_U4_67616D65MU4_6D61696EO0_NU4_6D61696EX0_P0_Qi32");
      EXPECT_EQ(mangleOrReport(ConvertI32), "_INK1FK1_U4_6D617468MU3_6F7073O0_NU7_636F6E76657274X0_P1_i32Qd");
      EXPECT_EQ(mangleOrReport(ConvertF64), "_INK1FK1_U4_6D617468MU3_6F7073O0_NU7_636F6E76657274X0_P1_dQi32");
      EXPECT_EQ(mangleOrReport(ValueOne), "_INK1FK1_U4_67616D65MU4_6D61696EO0_NU5_76616C7565X1_Vi32_00000001P0_Qi32");
      EXPECT_EQ(mangleOrReport(ValueTwo), "_INK1FK1_U4_67616D65MU4_6D61696EO0_NU5_76616C7565X1_Vi32_00000002P0_Qi32");
      EXPECT_EQ(mangleOrReport(Counter), "_INK1GK1_U4_67616D65MU5_7374617465O0_NU7_636F756E746572WYi32");
      EXPECT_EQ(mangleOrReport(ImportedAnswer), "_INK1FK1_U7_6C696272617279MU4_6D617468O0_NU6_616E73776572X0_P1_i32Qi32");
    }

    // Verifies that UTF-8 byte counts and uppercase hexadecimal bytes preserve NFC package and declaration names exactly.
    TEST(NameMangleTest, EncodesUnicodeNamesByUtf8ByteCount)
    {
      const FunctionIdentity Identity = makeFunction(makeScope({u8"café"}, "app", u8"玩家"));

      EXPECT_EQ(mangleOrReport(Identity), "_INK1FK1_U5_636166C3A9MU3_617070O0_NU6_E78EA9E5AEB6X0_P0_Qv");
      expectRoundTrip(LinkageIdentity(Identity));
    }

    // Verifies all primitive tags and every recursive type production, including const ordering, arrays, tuples, and nominal identities.
    TEST(NameMangleTest, EncodesPrimitiveAndCompoundTypesCompositionally)
    {
      const NominalIdentity PlayerIdentity{makeScope({"game"}, "window", "Player"), {}};
      const Type Player = Type::nominalType(PlayerIdentity);
      std::vector<Type> Parameters;
      Parameters.push_back(Type::voidType());
      Parameters.push_back(Type::boolType());
      Parameters.push_back(Type::byteType());
      Parameters.push_back(Type::i32Type());
      Parameters.push_back(Type::pointerSizeType());
      Parameters.push_back(Type::f16Type());
      Parameters.push_back(Type::f32Type());
      Parameters.push_back(Type::f64Type());
      Parameters.push_back(Type::constType(Type::byteType()));
      Parameters.push_back(Type::pointerType(Type::constType(Type::byteType())));
      Parameters.push_back(Type::constType(Type::pointerType(Type::byteType())));
      Parameters.push_back(Type::referenceType(Type::i32Type()));
      Parameters.push_back(Type::sliceType(Type::constType(Type::byteType())));
      Parameters.push_back(Type::arrayType(12, Type::f64Type()));
      Parameters.push_back(Type::tupleType({Type::boolType(), Type::pointerType(Type::i32Type()), Type::tupleType({})}));
      Parameters.push_back(Player);
      const FunctionIdentity Identity = makeFunction(makeScope({"types"}, "demo", "all"), std::move(Parameters), Player);

      EXPECT_EQ(mangleOrReport(Identity), "_INK1FK1_U5_7479706573MU4_64656D6FO0_NU3_616C6CX0_P16_vbyi32zhfdkypkykpyri32skya12_dt3_bpi32t0_nK1_U4_67616D65MU6_77696E646F77O0_NU6_506C61796572X0_QnK1_U4_67616D65MU6_77696E646F77O0_NU6_506C61796572X0_");
      expectRoundTrip(LinkageIdentity(Identity));
    }

    // Verifies outer-to-inner owner order and independent owner/function closed keys with mixed type and i32 value arguments.
    TEST(NameMangleTest, EncodesOwnersAndClosedInstanceKeysWithoutCollisions)
    {
      ClosedInstanceKey OuterKey;
      OuterKey.Arguments.push_back(ClosedArgument::type(Type::i32Type()));
      ClosedInstanceKey InnerKey;
      InnerKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "FFFFFFFF"));
      ClosedInstanceKey FunctionKey;
      FunctionKey.Arguments.push_back(ClosedArgument::type(Type::pointerType(Type::i32Type())));
      FunctionKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "00000001"));
      std::vector<OwnerType> Owners;
      Owners.push_back({"Outer", std::move(OuterKey)});
      Owners.push_back({"Inner", std::move(InnerKey)});
      const FunctionIdentity Identity = makeFunction(makeScope({"pkg", "sub"}, "mod", "call", std::move(Owners)), {}, Type::voidType(), std::move(FunctionKey));

      EXPECT_EQ(mangleOrReport(Identity), "_INK1FK2_U3_706B67U3_737562MU3_6D6F64O2_TU5_4F75746572X1_Ti32TU5_496E6E6572X1_Vi32_FFFFFFFFNU4_63616C6CX2_Tpi32Vi32_00000001P0_Qv");
      expectRoundTrip(LinkageIdentity(Identity));
    }

    // Verifies entity kind, parameter type, result type, and global mutability each contribute independently to the final symbol.
    TEST(NameMangleTest, DistinguishesAbiRelevantIdentityFields)
    {
      const Scope Path = makeScope({"pkg"}, "mod", "item");
      ClosedInstanceKey OwnerKey;
      OwnerKey.Arguments.push_back(ClosedArgument::type(Type::i32Type()));
      ClosedInstanceKey FunctionKey;
      FunctionKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "00000001"));
      std::set<std::string> Names;
      Names.insert(mangleOrReport(makeFunction(Path, {}, Type::i32Type(), {}, FunctionKind::Function)));
      Names.insert(mangleOrReport(makeFunction(Path, {}, Type::i32Type(), {}, FunctionKind::Constructor)));
      Names.insert(mangleOrReport(makeFunction(Path, {}, Type::i32Type(), {}, FunctionKind::Destructor)));
      Names.insert(mangleOrReport(makeFunction(Path, {Type::i32Type()}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(Path, {}, Type::f64Type())));
      Names.insert(mangleOrReport(makeFunction(makeScope({"other"}, "mod", "item"), {}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(makeScope({"pkg"}, "other", "item"), {}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(makeScope({"pkg"}, "mod", "other"), {}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(makeScope({"pkg"}, "mod", "item", {{"Owner", {}}}), {}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(makeScope({"pkg"}, "mod", "item", {{"Owner", std::move(OwnerKey)}}), {}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(Path, {}, Type::i32Type(), std::move(FunctionKey))));
      Names.insert(mangleOrReport(makeFunction(Path, {Type::i32Type(), Type::f64Type()}, Type::i32Type())));
      Names.insert(mangleOrReport(makeFunction(Path, {Type::f64Type(), Type::i32Type()}, Type::i32Type())));
      Names.insert(mangleOrReport(makeGlobal(Path, Type::i32Type(), GlobalMutability::Mutable)));
      Names.insert(mangleOrReport(makeGlobal(Path, Type::i32Type(), GlobalMutability::Immutable)));
      Names.insert(mangleOrReport(makeGlobal(Path, Type::f64Type(), GlobalMutability::Mutable)));

      EXPECT_EQ(Names.size(), 16U);
    }

    // Verifies mutable, readonly, and absent member receivers remain distinct while the shared owner path identifies the member declaration.
    TEST(NameMangleTest, PreservesCanonicalReceiverTypes)
    {
      const Scope PlayerPath = makeScope({"game"}, "window", "Player");
      const Type Player = Type::nominalType({PlayerPath, {}});
      const Scope MethodPath = makeScope({"game"}, "window", "take_damage", {{"Player", {}}});
      const FunctionIdentity Mutable = makeFunction(MethodPath, {Type::pointerType(Player), Type::i32Type()});
      const FunctionIdentity Readonly = makeFunction(MethodPath, {Type::pointerType(Type::constType(Player)), Type::i32Type()});
      const FunctionIdentity Static = makeFunction(MethodPath, {Type::i32Type()});

      EXPECT_NE(mangleOrReport(Mutable), mangleOrReport(Readonly));
      EXPECT_NE(mangleOrReport(Mutable), mangleOrReport(Static));
      EXPECT_NE(mangleOrReport(Readonly), mangleOrReport(Static));
      expectRoundTrip(LinkageIdentity(Mutable));
      expectRoundTrip(LinkageIdentity(Readonly));
      expectRoundTrip(LinkageIdentity(Static));
    }

    // Verifies missing, empty, malformed UTF-8, and non-NFC declaration path components are rejected before producing a partial symbol.
    TEST(NameMangleTest, RejectsInvalidDeclarationPathComponents)
    {
      FunctionIdentity MissingPackage = makeFunction(makeScope({}, "mod", "name"));
      FunctionIdentity EmptyPackage = makeFunction(makeScope({""}, "mod", "name"));
      FunctionIdentity EmptyModule = makeFunction(makeScope({"pkg"}, "", "name"));
      FunctionIdentity EmptyOwner = makeFunction(makeScope({"pkg"}, "mod", "name", {{"", {}}}));
      FunctionIdentity EmptyDeclaration = makeFunction(makeScope({"pkg"}, "mod", ""));
      FunctionIdentity InvalidUtf8 = makeFunction(makeScope({std::string("\xC0\x80", 2)}, "mod", "name"));
      FunctionIdentity NonNfc = makeFunction(makeScope({u8"cafe\u0301"}, "mod", "name"));

      const MangleResult MissingPackageResult = mangle(MissingPackage);
      EXPECT_FALSE(MissingPackageResult.succeeded());
      EXPECT_FALSE(MissingPackageResult.mangledName().has_value());
      EXPECT_EQ(MissingPackageResult.error(), MangleErrorKind::MissingPackagePath);
      EXPECT_EQ(mangle(EmptyPackage).error(), MangleErrorKind::EmptyNameComponent);
      EXPECT_EQ(mangle(EmptyModule).error(), MangleErrorKind::EmptyNameComponent);
      EXPECT_EQ(mangle(EmptyOwner).error(), MangleErrorKind::EmptyNameComponent);
      EXPECT_EQ(mangle(EmptyDeclaration).error(), MangleErrorKind::EmptyNameComponent);
      EXPECT_EQ(mangle(InvalidUtf8).error(), MangleErrorKind::InvalidUtf8);
      EXPECT_EQ(mangle(NonNfc).error(), MangleErrorKind::NonNormalizedName);
    }

    // Verifies ABI version 1 accepts fixed-width byte/i32 bits and rejects undefined value types or noncanonical bit text.
    TEST(NameMangleTest, RejectsUnsupportedOrNoncanonicalClosedValues)
    {
      ClosedInstanceKey ByteKey;
      ByteKey.Arguments.push_back(ClosedArgument::value(Type::byteType(), "FF"));
      ClosedInstanceKey FloatingKey;
      FloatingKey.Arguments.push_back(ClosedArgument::value(Type::f32Type(), "00000000"));
      ClosedInstanceKey BooleanKey;
      BooleanKey.Arguments.push_back(ClosedArgument::value(Type::boolType(), "1"));
      ClosedInstanceKey LowercaseKey;
      LowercaseKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "ffffffff"));
      ClosedInstanceKey ShortKey;
      ShortKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "0000000"));
      ClosedInstanceKey LongKey;
      LongKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "000000000"));
      ClosedInstanceKey NonHexKey;
      NonHexKey.Arguments.push_back(ClosedArgument::value(Type::i32Type(), "0000000G"));
      const FunctionIdentity ByteValue = makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::byteType(), std::move(ByteKey));

      EXPECT_EQ(mangleOrReport(ByteValue), "_INK1FK1_U3_706B67MU3_6D6F64O0_NU5_76616C7565X1_Vy_FFP0_Qy");
      expectRoundTrip(LinkageIdentity(ByteValue));
      EXPECT_EQ(mangle(makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::i32Type(), std::move(FloatingKey))).error(), MangleErrorKind::UnsupportedValueArgumentType);
      EXPECT_EQ(mangle(makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::boolType(), std::move(BooleanKey))).error(), MangleErrorKind::UnsupportedValueArgumentType);
      EXPECT_EQ(mangle(makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::i32Type(), std::move(LowercaseKey))).error(), MangleErrorKind::InvalidCanonicalBits);
      EXPECT_EQ(mangle(makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::i32Type(), std::move(ShortKey))).error(), MangleErrorKind::InvalidCanonicalBits);
      EXPECT_EQ(mangle(makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::i32Type(), std::move(LongKey))).error(), MangleErrorKind::InvalidCanonicalBits);
      EXPECT_EQ(mangle(makeFunction(makeScope({"pkg"}, "mod", "value"), {}, Type::i32Type(), std::move(NonHexKey))).error(), MangleErrorKind::InvalidCanonicalBits);
    }

    // Verifies invalid public enum values fail explicitly and moved-from Type values remain safely reusable rather than retaining broken recursive shape.
    TEST(NameMangleTest, HandlesInvalidEnumsAndMovedFromTypesSafely)
    {
      FunctionIdentity InvalidFunction = makeFunction(makeScope({"pkg"}, "mod", "function"));
      InvalidFunction.Kind = static_cast<FunctionKind>(255);
      GlobalIdentity InvalidGlobal = makeGlobal(makeScope({"pkg"}, "mod", "global"), Type::i32Type());
      InvalidGlobal.Mutability = static_cast<GlobalMutability>(255);
      Type Source = Type::pointerType(Type::byteType());
      Type Destination = std::move(Source);
      const MangleResult MovedFromResult = mangle(makeFunction(makeScope({"pkg"}, "mod", "source"), {}, Source));

      EXPECT_EQ(mangle(InvalidFunction).error(), MangleErrorKind::InvalidFunctionKind);
      EXPECT_EQ(mangle(InvalidGlobal).error(), MangleErrorKind::InvalidGlobalMutability);
      EXPECT_TRUE(MovedFromResult.succeeded()) << mangleErrorKindName(MovedFromResult.error());
      EXPECT_EQ(mangleOrReport(makeFunction(makeScope({"pkg"}, "mod", "destination"), {}, Destination)), "_INK1FK1_U3_706B67MU3_6D6F64O0_NU11_64657374696E6174696F6EX0_P0_Qpy");
    }

    // Verifies the documented recursion guard accepts the deepest supported type and rejects one additional pointer layer safely.
    TEST(NameMangleTest, EnforcesTypeNestingLimit)
    {
      Type DeepestAccepted = Type::byteType();
      for (std::size_t Depth = 1; Depth < NameManglingNestingLimit; ++Depth)
      {
        DeepestAccepted = Type::pointerType(std::move(DeepestAccepted));
      }
      const MangleResult Accepted = mangle(makeFunction(makeScope({"pkg"}, "mod", "deep"), {}, DeepestAccepted));
      const Type TooDeep = Type::pointerType(std::move(DeepestAccepted));
      const MangleResult Rejected = mangle(makeFunction(makeScope({"pkg"}, "mod", "deep"), {}, TooDeep));
      ClosedInstanceKey AcceptedFunctionKey;
      AcceptedFunctionKey.Arguments.push_back(ClosedArgument::type(TooDeep.elementType() == nullptr ? Type::voidType() : *TooDeep.elementType()));
      ClosedInstanceKey RejectedFunctionKey;
      RejectedFunctionKey.Arguments.push_back(ClosedArgument::type(TooDeep));
      const MangleResult AcceptedFunctionInstance = mangle(makeFunction(makeScope({"pkg"}, "mod", "function_key"), {}, Type::voidType(), std::move(AcceptedFunctionKey)));
      const MangleResult RejectedFunctionInstance = mangle(makeFunction(makeScope({"pkg"}, "mod", "function_key"), {}, Type::voidType(), std::move(RejectedFunctionKey)));
      Type NominalArgument = Type::byteType();
      for (std::size_t Depth = 2; Depth < NameManglingNestingLimit; ++Depth)
      {
        NominalArgument = Type::pointerType(std::move(NominalArgument));
      }
      ClosedInstanceKey AcceptedNominalKey;
      AcceptedNominalKey.Arguments.push_back(ClosedArgument::type(NominalArgument));
      ClosedInstanceKey RejectedNominalKey;
      RejectedNominalKey.Arguments.push_back(ClosedArgument::type(Type::pointerType(std::move(NominalArgument))));
      const Type AcceptedNominal = Type::nominalType({makeScope({"pkg"}, "mod", "Type"), std::move(AcceptedNominalKey)});
      const Type RejectedNominal = Type::nominalType({makeScope({"pkg"}, "mod", "Type"), std::move(RejectedNominalKey)});
      const MangleResult AcceptedNominalInstance = mangle(makeFunction(makeScope({"pkg"}, "mod", "nominal_key"), {}, AcceptedNominal));
      const MangleResult RejectedNominalInstance = mangle(makeFunction(makeScope({"pkg"}, "mod", "nominal_key"), {}, RejectedNominal));

      EXPECT_TRUE(Accepted.succeeded()) << mangleErrorKindName(Accepted.error());
      EXPECT_FALSE(Rejected.succeeded());
      EXPECT_EQ(Rejected.error(), MangleErrorKind::NestingLimitExceeded);
      EXPECT_TRUE(AcceptedFunctionInstance.succeeded()) << mangleErrorKindName(AcceptedFunctionInstance.error());
      EXPECT_EQ(RejectedFunctionInstance.error(), MangleErrorKind::NestingLimitExceeded);
      EXPECT_TRUE(AcceptedNominalInstance.succeeded()) << mangleErrorKindName(AcceptedNominalInstance.error());
      EXPECT_EQ(RejectedNominalInstance.error(), MangleErrorKind::NestingLimitExceeded);
    }
  } // namespace
} // namespace ink::abi
