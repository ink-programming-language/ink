#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <functional>
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

  // Function identities include return type, ordered borrowed parameter types, and parameter kinds.
  TEST(IRTypePoolTest, UniquifiesFunctionsByCompleteParameterDescriptions)
  {
    const IntegerType External(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    TypePool Pool;
    TypePool Other;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const auto &Bool = Pool.getBoolType();
    const Parameter Parameters[] = {
        Parameter(Integer),
        Parameter(Bool),
    };
    const Parameter Equivalent[] = {
        Parameter(Integer),
        Parameter(Bool),
    };
    const Parameter Reordered[] = {
        Parameter(Bool),
        Parameter(Integer),
    };
    const Parameter OtherIdentity[] = {
        Parameter(External),
        Parameter(Bool),
    };
    const Parameter Named[] = {
        Parameter(Integer, Parameter::Kind::Named),
        Parameter(Bool),
    };
    const Parameter Variadic[] = {
        Parameter(Integer, Parameter::Kind::Variadic),
        Parameter(Bool),
    };
    const auto &Function = Pool.getFunctionType(Integer, Parameters);
    EXPECT_EQ(&Function, &Pool.getFunctionType(Integer, Equivalent));
    EXPECT_NE(&Function, &Pool.getFunctionType(Bool, Parameters));
    EXPECT_NE(&Function, &Pool.getFunctionType(External, Parameters));
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
    EXPECT_EQ(&Pool.getPointerType(Function).pointeeType(), &Function);
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
      EXPECT_EQ(&Pool.getPointerType(Callback).pointeeType(), &Callback);
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

  // Interned signatures own metadata snapshots and preserve their storage and hashes as the composite caches grow.
  TEST(IRTypePoolTest, OwnsFunctionDescriptionsAndPreservesThemWhileGrowing)
  {
    TypePool Pool;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType *Function;
    {
      std::array Parameters{Parameter(Integer, Parameter::Kind::Named)};
      Function = &Pool.getFunctionType(Integer, Parameters);
      Parameters[0] = Parameter(Pool.getBoolType(), Parameter::Kind::Variadic);
    }
    const auto *ParameterData = Function->parameters().data();
    const auto OriginalHash = Function->hash();
    for (std::size_t Index = 0; Index < 512; ++Index)
    {
      const auto &Array = Pool.getArrayType(Integer, Index + 1);
      const std::array Parameters{Parameter(Array)};
      Pool.getFunctionType(Integer, Parameters);
    }
    const std::array Parameters{Parameter(Integer, Parameter::Kind::Named)};
    EXPECT_EQ(Function, &Pool.getFunctionType(Integer, Parameters));
    ASSERT_EQ(Function->parameters().size(), 1U);
    EXPECT_EQ(Function->parameters().data(), ParameterData);
    EXPECT_EQ(Function->parameters()[0].kind(), Parameter::Kind::Named);
    EXPECT_EQ(Function->hash(), OriginalHash);
    EXPECT_EQ(&Function->parameters()[0].type(), &Integer);
  }

  // Transparent function keys retain exact equality and stable node addresses despite collisions and rehashing.
  TEST(IRTypePoolTest, FindsFunctionDescriptionsWithCollidingHashesAfterRehash)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const Parameter Parameters[] = {
        Parameter(Integer),
    };
    const Parameter OtherParameters[] = {
        Parameter(Bool),
    };
    const Parameter Named[] = {
        Parameter(Integer, Parameter::Kind::Named),
    };
    const Parameter Variadic[] = {
        Parameter(Integer, Parameter::Kind::Variadic),
    };
    std::unordered_set<FunctionType, CollidingFunctionHash, std::equal_to<>> Functions;
    const auto *Function = &*Functions.emplace(Integer, Parameters).first;
    const auto *OtherReturn = &*Functions.emplace(Bool, Parameters).first;
    const auto *OtherType = &*Functions.emplace(Integer, OtherParameters).first;
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
    const auto FoundType = Functions.find(FunctionType::Key{&Integer, OtherParameters});
    const auto FoundKind = Functions.find(FunctionType::Key{&Integer, Named});
    const auto FoundVariadic = Functions.find(FunctionType::Key{&Integer, Variadic});
    const auto FoundEmpty = Functions.find(FunctionType::Key{&Integer, {}});
    const auto FoundC = Functions.find(FunctionType::Key{&Integer, Parameters, CABI});
    const auto FoundStdCall = Functions.find(FunctionType::Key{&Integer, Parameters, StdCallABI});
    ASSERT_NE(Found, Functions.end());
    ASSERT_NE(FoundReturn, Functions.end());
    ASSERT_NE(FoundType, Functions.end());
    ASSERT_NE(FoundKind, Functions.end());
    ASSERT_NE(FoundVariadic, Functions.end());
    ASSERT_NE(FoundEmpty, Functions.end());
    ASSERT_NE(FoundC, Functions.end());
    ASSERT_NE(FoundStdCall, Functions.end());
    EXPECT_EQ(&*Found, Function);
    EXPECT_EQ(&*FoundReturn, OtherReturn);
    EXPECT_EQ(&*FoundType, OtherType);
    EXPECT_EQ(&*FoundKind, OtherKind);
    EXPECT_EQ(&*FoundVariadic, VariadicKind);
    EXPECT_EQ(&*FoundEmpty, Empty);
    EXPECT_EQ(&*FoundC, CFunction);
    EXPECT_EQ(&*FoundStdCall, StdCallFunction);
    EXPECT_EQ(Functions.find(FunctionType::Key{&Bool, OtherParameters}), Functions.end());
    const core::Hash<FunctionType> Hash;
    const std::equal_to<> Equal;
    EXPECT_EQ(Hash(*Function), Hash(FunctionType::Key{&Integer, Parameters}));
    EXPECT_EQ(Hash(*OtherType), Hash(FunctionType::Key{&Integer, OtherParameters}));
    EXPECT_EQ(Hash(*OtherKind), Hash(FunctionType::Key{&Integer, Named}));
    EXPECT_EQ(Hash(*VariadicKind), Hash(FunctionType::Key{&Integer, Variadic}));
    EXPECT_EQ(Hash(*Empty), Hash(FunctionType::Key{&Integer, {}}));
    EXPECT_EQ(Hash(*CFunction), Hash(FunctionType::Key{&Integer, Parameters, CABI}));
    EXPECT_EQ(Hash(*StdCallFunction), Hash(FunctionType::Key{&Integer, Parameters, StdCallABI}));
    EXPECT_TRUE(Equal(*Function, FunctionType::Key{&Integer, Parameters}));
    EXPECT_TRUE(Equal(FunctionType::Key{&Integer, Parameters}, *Function));
    EXPECT_FALSE(Equal(*Function, *OtherReturn));
    EXPECT_FALSE(Equal(*Function, *OtherType));
    EXPECT_FALSE(Equal(*Function, *OtherKind));
    EXPECT_FALSE(Equal(*Function, *VariadicKind));
    EXPECT_FALSE(Equal(*OtherKind, *VariadicKind));
    EXPECT_FALSE(Equal(*Function, *Empty));
    EXPECT_FALSE(Equal(*Function, *CFunction));
    EXPECT_FALSE(Equal(*CFunction, *StdCallFunction));
    EXPECT_FALSE(Equal(FunctionType::Key{&Integer, OtherParameters}, *Function));
  }
} // namespace ink::ir::test
