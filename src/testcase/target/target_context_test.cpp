#include "ink/target/target_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>

namespace ink::target
{
  namespace
  {
    // Verifies every target endianness value has a stable name and invalid values use the conservative unknown fallback.
    TEST(TargetEndiannessTest, NamesEveryValueAndInvalidFallback)
    {
      EXPECT_STREQ(targetEndiannessName(TargetEndianness::Unknown), "unknown");
      EXPECT_STREQ(targetEndiannessName(TargetEndianness::Little), "little");
      EXPECT_STREQ(targetEndiannessName(TargetEndianness::Big), "big");
      EXPECT_STREQ(targetEndiannessName(static_cast<TargetEndianness>(255)), "unknown");
    }

    // Verifies a target key has an unambiguous deterministic identity even when string fields contain separators.
    TEST(TargetKeyTest, ProducesCanonicalLengthDelimitedIdentity)
    {
      const TargetKey Key{"x86_64-pc-windows-msvc", "cpu;variant", "+sse2,-avx", 64, TargetEndianness::Little};

      EXPECT_TRUE(Key.isValid());
      EXPECT_EQ(Key.canonicalString(), "triple=22:x86_64-pc-windows-msvc;cpu=11:cpu;variant;features=10:+sse2,-avx;pointer=64;endianness=little;");
    }

    // Verifies target-key validity accepts every supported pointer-width and endianness combination while rejecting all incomplete layouts.
    TEST(TargetKeyTest, ValidatesEverySupportedLayoutCombination)
    {
      EXPECT_TRUE((TargetKey{"target", "", "", 32, TargetEndianness::Little}).isValid());
      EXPECT_TRUE((TargetKey{"target", "", "", 32, TargetEndianness::Big}).isValid());
      EXPECT_TRUE((TargetKey{"target", "", "", 64, TargetEndianness::Little}).isValid());
      EXPECT_TRUE((TargetKey{"target", "", "", 64, TargetEndianness::Big}).isValid());
      EXPECT_FALSE((TargetKey{"", "", "", 32, TargetEndianness::Little}).isValid());
      EXPECT_FALSE((TargetKey{"target", "", "", 16, TargetEndianness::Little}).isValid());
      EXPECT_FALSE((TargetKey{"target", "", "", 128, TargetEndianness::Little}).isValid());
      EXPECT_FALSE((TargetKey{"target", "", "", 32, TargetEndianness::Unknown}).isValid());
      EXPECT_FALSE((TargetKey{"target", "", "", 32, static_cast<TargetEndianness>(255)}).isValid());
    }

    // Verifies target-key equality observes every identity field and inequality remains the exact complement of equality.
    TEST(TargetKeyTest, ComparesEveryIdentityField)
    {
      const TargetKey Base{"x86_64-unknown-linux-gnu", "generic", "+sse2", 64, TargetEndianness::Little};
      const TargetKey Equal = Base;
      TargetKey Different = Base;

      EXPECT_EQ(Base, Equal);
      EXPECT_FALSE(Base != Equal);
      Different.Triple = "aarch64-unknown-linux-gnu";
      EXPECT_NE(Base, Different);
      Different = Base;
      Different.Cpu = "x86-64-v3";
      EXPECT_NE(Base, Different);
      Different = Base;
      Different.Features = "+sse2,+avx";
      EXPECT_NE(Base, Different);
      Different = Base;
      Different.PointerBitWidth = 32;
      EXPECT_NE(Base, Different);
      Different = Base;
      Different.Endianness = TargetEndianness::Big;
      EXPECT_NE(Base, Different);
    }

    // Verifies target contexts reject incomplete identities and out-of-domain endianness values instead of silently inheriting layout facts.
    TEST(TargetContextTest, RejectsInvalidTargetKeys)
    {
      EXPECT_THROW(TargetContext({"", "generic", "", 64, TargetEndianness::Little}), std::invalid_argument);
      EXPECT_THROW(TargetContext({"x86_64-unknown", "generic", "", 0, TargetEndianness::Little}), std::invalid_argument);
      EXPECT_THROW(TargetContext({"x86_64-unknown", "generic", "", 64, TargetEndianness::Unknown}), std::invalid_argument);
      const TargetKey OutOfDomain{"x86_64-unknown", "generic", "", 64, static_cast<TargetEndianness>(255)};
      EXPECT_FALSE(OutOfDomain.isValid());
      EXPECT_THROW(TargetContext{OutOfDomain}, std::invalid_argument);
    }

    // Verifies the host target reports a stable layout key and only the integer widths supported by the first slice.
    TEST(TargetContextTest, DescribesHostFirstSliceCapabilities)
    {
      const TargetContext Host = TargetContext::host();

      EXPECT_TRUE(Host.key().isValid());
      EXPECT_EQ(Host.pointerBitWidth(), sizeof(void *) * 8U);
      EXPECT_NE(Host.endianness(), TargetEndianness::Unknown);
      EXPECT_TRUE(Host.supportsIntegerBitWidth(1));
      EXPECT_TRUE(Host.supportsIntegerBitWidth(32));
      EXPECT_TRUE(Host.supportsIntegerBitWidth(64));
      EXPECT_FALSE(Host.supportsIntegerBitWidth(24));
    }

    // Verifies an explicit target context preserves its key and reports exactly the integer widths supported by the first slice.
    TEST(TargetContextTest, PreservesExplicitLayoutAndReportsExactIntegerWidths)
    {
      const TargetKey Key{"powerpc-unknown-none", "generic", "", 32, TargetEndianness::Big};
      const TargetContext Context(Key);
      constexpr std::uint32_t SupportedWidths[] = {
          1,
          8,
          16,
          32,
          64,
      };
      constexpr std::uint32_t UnsupportedWidths[] = {
          0,
          2,
          7,
          24,
          63,
          65,
          128,
      };

      EXPECT_EQ(Context.key(), Key);
      EXPECT_EQ(Context.pointerBitWidth(), 32U);
      EXPECT_EQ(Context.endianness(), TargetEndianness::Big);
      for (const std::uint32_t BitWidth : SupportedWidths)
      {
        EXPECT_TRUE(Context.supportsIntegerBitWidth(BitWidth));
      }
      for (const std::uint32_t BitWidth : UnsupportedWidths)
      {
        EXPECT_FALSE(Context.supportsIntegerBitWidth(BitWidth));
      }
    }
  } // namespace
} // namespace ink::target
