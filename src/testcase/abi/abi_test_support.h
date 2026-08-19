#ifndef INK_TESTCASE_ABI_TEST_SUPPORT_H
#define INK_TESTCASE_ABI_TEST_SUPPORT_H

#include "ink/abi/name_mangling.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

namespace ink::abi::test
{
  inline Scope makeScope(std::vector<std::string> PackagePath, std::string Module, std::string DeclarationName, std::vector<OwnerType> EnclosingTypes = {})
  {
    return {std::move(PackagePath), std::move(Module), std::move(EnclosingTypes), std::move(DeclarationName)};
  }

  inline FunctionIdentity makeFunction(Scope Path, std::vector<Type> Parameters = {}, Type Result = Type::voidType(), ClosedInstanceKey Instance = {}, FunctionKind Kind = FunctionKind::Function)
  {
    return {Kind, std::move(Path), std::move(Instance), std::move(Parameters), std::move(Result)};
  }

  inline GlobalIdentity makeGlobal(Scope Path, Type ValueType, GlobalMutability Mutability = GlobalMutability::Mutable)
  {
    return {std::move(Path), Mutability, std::move(ValueType)};
  }

  inline std::string mangleOrReport(const LinkageIdentity &Identity)
  {
    const MangleResult Result = mangle(Identity);
    if (!Result.succeeded())
    {
      ADD_FAILURE() << "mangle failed: " << mangleErrorKindName(Result.error());
      return {};
    }
    return *Result.mangledName();
  }

  inline void expectRoundTrip(const LinkageIdentity &Identity)
  {
    const MangleResult Encoded = mangle(Identity);
    ASSERT_TRUE(Encoded.succeeded()) << mangleErrorKindName(Encoded.error());
    const DemangleResult Decoded = demangle(*Encoded.mangledName());
    ASSERT_TRUE(Decoded.succeeded()) << demangleErrorKindName(Decoded.error()) << " at " << Decoded.errorOffset();
    EXPECT_EQ(*Decoded.identity(), Identity);
    const MangleResult Reencoded = mangle(*Decoded.identity());
    ASSERT_TRUE(Reencoded.succeeded()) << mangleErrorKindName(Reencoded.error());
    EXPECT_EQ(*Reencoded.mangledName(), *Encoded.mangledName());
  }
} // namespace ink::abi::test

#endif
