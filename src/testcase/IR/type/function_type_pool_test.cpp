#include "ink/core/name/name_pool.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace ink::ir::test
{
  static_assert(!std::is_copy_constructible_v<FunctionType>);
  static_assert(!std::is_move_constructible_v<FunctionType>);

  template <typename T>
  concept PoolAcceptsTemporaryFunctionReturn = requires(TypePool &Pool)
  {
    Pool.getFunctionType(std::declval<T &&>());
  };

  static_assert(!PoolAcceptsTemporaryFunctionReturn<IntegerType>);
  static_assert(!PoolAcceptsTemporaryFunctionReturn<const IntegerType>);

  struct CollidingFunctionHash
  {
    public:
      using is_transparent = void;

      template <typename T>
      std::size_t operator()(const T &) const noexcept
      {
        return 0;
      }
  };

  // Function identities include return type, ordered parameter types, and interned parameter names and kinds.
  TEST(IRTypePoolTest, UniquifiesFunctionsByCompleteParameterDescriptions)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType External(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    TypePool Pool;
    TypePool Other;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const auto &Bool = Pool.getBoolType();
    const Parameter Parameters[] = {
        Parameter(Integer, Names.intern("Count")),
        Parameter(Bool, Names.intern("Ready")),
    };
    const Parameter Equivalent[] = {
        Parameter(Integer, Names.intern("Count")),
        Parameter(Bool, Names.intern("Ready")),
    };
    const Parameter Renamed[] = {
        Parameter(Integer, Names.intern("OtherCount")),
        Parameter(Bool, Names.intern("Ready")),
    };
    const Parameter Reordered[] = {
        Parameter(Bool, Names.intern("Ready")),
        Parameter(Integer, Names.intern("Count")),
    };
    const Parameter OtherIdentity[] = {
        Parameter(External, Names.intern("Count")),
        Parameter(Bool, Names.intern("Ready")),
    };
    const Parameter Named[] = {
        Parameter(Integer, Names.intern("Count"), Parameter::Kind::Named),
        Parameter(Bool, Names.intern("Ready")),
    };
    const Parameter Variadic[] = {
        Parameter(Integer, Names.intern("Count"), Parameter::Kind::Variadic),
        Parameter(Bool, Names.intern("Ready")),
    };
    const auto &Function = Pool.getFunctionType(Integer, Parameters);
    EXPECT_EQ(&Function, &Pool.getFunctionType(Integer, Equivalent));
    EXPECT_NE(&Function, &Pool.getFunctionType(Bool, Parameters));
    EXPECT_NE(&Function, &Pool.getFunctionType(External, Parameters));
    EXPECT_NE(&Function, &Pool.getFunctionType(Integer, Renamed));
    EXPECT_NE(&Function, &Pool.getFunctionType(Integer, Reordered));
    EXPECT_NE(&Function, &Pool.getFunctionType(Integer, OtherIdentity));
    const auto &NamedFunction = Pool.getFunctionType(Integer, Named);
    const auto &VariadicFunction = Pool.getFunctionType(Integer, Variadic);
    EXPECT_NE(&Function, &NamedFunction);
    EXPECT_NE(&Function, &VariadicFunction);
    EXPECT_NE(&NamedFunction, &VariadicFunction);
    EXPECT_EQ(&NamedFunction, &Pool.getFunctionType(Integer, Named));
    EXPECT_EQ(&VariadicFunction, &Pool.getFunctionType(Integer, Variadic));
    EXPECT_EQ(NamedFunction.parameters()[0].kind(), Parameter::Kind::Named);
    EXPECT_EQ(VariadicFunction.parameters()[0].kind(), Parameter::Kind::Variadic);
    EXPECT_NE(&Function, &Pool.getFunctionType(Integer, std::span<const Parameter>(Parameters, 1)));
    EXPECT_NE(&Function, &Other.getFunctionType(Integer, Parameters));
    const auto &Empty = Pool.getFunctionType(Pool.getVoidType());
    EXPECT_EQ(&Empty, &Pool.getFunctionType(Pool.getVoidType(), {}));
    EXPECT_TRUE(Empty.parameters().empty());
    EXPECT_EQ(&Empty.returnType(), &Pool.getVoidType());
    EXPECT_EQ(&Pool.getReferenceType(Function).referencedType(), &Function);
  }

  // ABI and calling convention differences survive interning and remain visible in nested callback types.
  TEST(IRTypePoolTest, KeepsDistinctCallContractsInInternedCallbacks)
  {
    TypePool Pool;
    const auto &Void = Pool.getVoidType();
    const CallABI Contracts[] = {
        {},
        {ABIKind::C, CallingConvention::Default},
        {ABIKind::C, CallingConvention::CDecl},
        {ABIKind::C, CallingConvention::StdCall},
        {ABIKind::C, CallingConvention::FastCall},
        {ABIKind::C, CallingConvention::VectorCall},
    };
    for (const auto Contract : Contracts)
    {
      const auto &Callback = Pool.getFunctionType(Void, {}, Contract);
      const std::array Parameters{Parameter(Callback)};
      const auto &Consumer = Pool.getFunctionType(Void, Parameters);
      EXPECT_EQ(&Callback, &Pool.getFunctionType(Void, {}, Contract));
      EXPECT_EQ(Callback.callABI(), Contract);
      EXPECT_EQ(&Consumer.parameters()[0].type(), &Callback);
      EXPECT_EQ(&Pool.getReferenceType(Callback).referencedType(), &Callback);
      for (const auto OtherContract : Contracts)
      {
        if (Contract == OtherContract)
        {
          continue;
        }
        const auto &OtherCallback = Pool.getFunctionType(Void, {}, OtherContract);
        const std::array OtherParameters{Parameter(OtherCallback)};
        EXPECT_NE(&Callback, &OtherCallback);
        EXPECT_NE(&Consumer, &Pool.getFunctionType(Void, OtherParameters));
      }
    }
    EXPECT_EQ(&Pool.getFunctionType(Void), &Pool.getFunctionType(Void, {}, CallABI{}));
  }

  // Signature storage and interned names retain their identities while both the type and name pools grow.
  TEST(IRTypePoolTest, OwnsFunctionDescriptionsAndPreservesThemWhileGrowing)
  {
    auto &Names = core::NamePool::instance();
    TypePool Pool;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType *Function;
    {
      const std::array Parameters{Parameter(Integer, Names.intern(std::string("Value")))};
      Function = &Pool.getFunctionType(Integer, Parameters);
    }
    const core::Name OriginalName = Function->parameters()[0].name();
    const auto *NameData = Names.text(OriginalName).data();
    const auto OriginalHash = Function->hash();
    for (std::size_t Index = 0; Index < 512; ++Index)
    {
      const std::array Parameters{Parameter(Integer, Names.intern("Value" + std::to_string(Index)))};
      Pool.getFunctionType(Integer, Parameters);
    }
    const std::array Parameters{Parameter(Integer, Names.intern("Value"))};
    EXPECT_EQ(Function, &Pool.getFunctionType(Integer, Parameters));
    ASSERT_EQ(Function->parameters().size(), 1U);
    EXPECT_EQ(Function->parameters()[0].name(), Names.intern("Value"));
    EXPECT_EQ(Function->parameters()[0].name(), OriginalName);
    EXPECT_EQ(Names.text(Function->parameters()[0].name()).data(), NameData);
    EXPECT_EQ(Function->hash(), OriginalHash);
    EXPECT_EQ(&Function->parameters()[0].type(), &Integer);
  }

  // Transparent function keys retain exact equality and stable node addresses despite collisions and rehashing.
  TEST(IRTypePoolTest, FindsFunctionDescriptionsWithCollidingHashesAfterRehash)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const Parameter Parameters[] = {
        Parameter(Integer, Names.intern("Value")),
    };
    const Parameter Renamed[] = {
        Parameter(Integer, Names.intern("Other")),
    };
    const Parameter Named[] = {
        Parameter(Integer, Names.intern("Value"), Parameter::Kind::Named),
    };
    const Parameter Variadic[] = {
        Parameter(Integer, Names.intern("Value"), Parameter::Kind::Variadic),
    };
    std::unordered_set<FunctionType, CollidingFunctionHash, std::equal_to<>> Functions;
    const auto *Function = &*Functions.emplace(Integer, Parameters).first;
    const auto *OtherReturn = &*Functions.emplace(Bool, Parameters).first;
    const auto *OtherName = &*Functions.emplace(Integer, Renamed).first;
    const auto *OtherKind = &*Functions.emplace(Integer, Named).first;
    const auto *VariadicKind = &*Functions.emplace(Integer, Variadic).first;
    const auto *Empty = &*Functions.emplace(Integer).first;
    const CallABI CABI{ABIKind::C, CallingConvention::Default};
    const CallABI StdCallABI{ABIKind::C, CallingConvention::StdCall};
    const auto *CFunction = &*Functions.emplace(Integer, Parameters, CABI).first;
    const auto *StdCallFunction = &*Functions.emplace(Integer, Parameters, StdCallABI).first;
    EXPECT_EQ(Functions.size(), 8U);
    EXPECT_FALSE(Functions.emplace(Integer, Parameters).second);
    EXPECT_FALSE(Functions.emplace(Integer, Parameters, CABI).second);
    EXPECT_FALSE(Functions.emplace(Integer, Parameters, StdCallABI).second);
    const auto PreviousBucketCount = Functions.bucket_count();
    Functions.rehash(PreviousBucketCount * 2 + 1);
    EXPECT_GT(Functions.bucket_count(), PreviousBucketCount);
    const auto Found = Functions.find(FunctionType::Key{&Integer, Parameters});
    const auto FoundReturn = Functions.find(FunctionType::Key{&Bool, Parameters});
    const auto FoundName = Functions.find(FunctionType::Key{&Integer, Renamed});
    const auto FoundKind = Functions.find(FunctionType::Key{&Integer, Named});
    const auto FoundVariadic = Functions.find(FunctionType::Key{&Integer, Variadic});
    const auto FoundEmpty = Functions.find(FunctionType::Key{&Integer, {}});
    const auto FoundC = Functions.find(FunctionType::Key{&Integer, Parameters, CABI});
    const auto FoundStdCall = Functions.find(FunctionType::Key{&Integer, Parameters, StdCallABI});
    ASSERT_NE(Found, Functions.end());
    ASSERT_NE(FoundReturn, Functions.end());
    ASSERT_NE(FoundName, Functions.end());
    ASSERT_NE(FoundKind, Functions.end());
    ASSERT_NE(FoundVariadic, Functions.end());
    ASSERT_NE(FoundEmpty, Functions.end());
    ASSERT_NE(FoundC, Functions.end());
    ASSERT_NE(FoundStdCall, Functions.end());
    EXPECT_EQ(&*Found, Function);
    EXPECT_EQ(&*FoundReturn, OtherReturn);
    EXPECT_EQ(&*FoundName, OtherName);
    EXPECT_EQ(&*FoundKind, OtherKind);
    EXPECT_EQ(&*FoundVariadic, VariadicKind);
    EXPECT_EQ(&*FoundEmpty, Empty);
    EXPECT_EQ(&*FoundC, CFunction);
    EXPECT_EQ(&*FoundStdCall, StdCallFunction);
    EXPECT_EQ(Functions.find(FunctionType::Key{&Bool, Renamed}), Functions.end());
    const core::Hash<FunctionType> Hash;
    const std::equal_to<> Equal;
    EXPECT_EQ(Hash(*Function), Hash(FunctionType::Key{&Integer, Parameters}));
    EXPECT_EQ(Hash(*OtherKind), Hash(FunctionType::Key{&Integer, Named}));
    EXPECT_EQ(Hash(*VariadicKind), Hash(FunctionType::Key{&Integer, Variadic}));
    EXPECT_EQ(Hash(*Empty), Hash(FunctionType::Key{&Integer, {}}));
    EXPECT_EQ(Hash(*CFunction), Hash(FunctionType::Key{&Integer, Parameters, CABI}));
    EXPECT_EQ(Hash(*StdCallFunction), Hash(FunctionType::Key{&Integer, Parameters, StdCallABI}));
    EXPECT_TRUE(Equal(*Function, FunctionType::Key{&Integer, Parameters}));
    EXPECT_TRUE(Equal(FunctionType::Key{&Integer, Parameters}, *Function));
    EXPECT_FALSE(Equal(*Function, *OtherReturn));
    EXPECT_FALSE(Equal(*Function, *OtherName));
    EXPECT_FALSE(Equal(*Function, *OtherKind));
    EXPECT_FALSE(Equal(*Function, *VariadicKind));
    EXPECT_FALSE(Equal(*OtherKind, *VariadicKind));
    EXPECT_FALSE(Equal(*Function, *Empty));
    EXPECT_FALSE(Equal(*Function, *CFunction));
    EXPECT_FALSE(Equal(*CFunction, *StdCallFunction));
    EXPECT_FALSE(Equal(FunctionType::Key{&Integer, Renamed}, *Function));
  }
} // namespace ink::ir::test
