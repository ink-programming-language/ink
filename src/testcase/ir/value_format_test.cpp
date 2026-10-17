#include "ink/ir/context.h"

// Internal formatter integration tests need a composite fixture because ordinary built-in values are formatting leaves.
#include "../../lib/ir/value_string_builder.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    constexpr ValueKind UnknownKind = static_cast<ValueKind>(std::numeric_limits<std::uint8_t>::max());

    class UnknownValue final : public Value
    {
      public:
        UnknownValue() noexcept
            : Value(UnknownKind)
        {
        }
    };

    class DescribedValue final : public Value
    {
      public:
        DescribedValue() noexcept
            : Value(UnknownKind)
        {
        }

        std::string toString() const override
        {
          return "custom-value";
        }
    };

    class CompositeValue final : public Value
    {
      public:
        explicit CompositeValue(std::vector<const Value *> Elements)
            : Value(UnknownKind),
              Elements(std::move(Elements))
        {
        }

        std::string toString() const override
        {
          return ValueStringBuilder::format(*this);
        }

      private:
        void appendToString(ValueStringBuilder &Builder) const override
        {
          Builder.append("[");
          Builder.pushText("]");
          for (std::size_t Index = Elements.size(); Index != 0; --Index)
          {
            Builder.pushValue(*Elements[Index - 1]);
            if (Index > 1)
            {
              Builder.pushText(", ");
            }
          }
        }

        std::vector<const Value *> Elements;
    };
  } // namespace

  // Ordinary composite values preserve nested type spellings, default kind descriptions and custom toString leaves in child order.
  TEST(IRValueFormatTest, FormatsCompositeValuesWithTypesAndOrdinaryLeaves)
  {
    IRContext Context(Target);
    const auto *Pointer = Context.typePool().getPointerType(Context.typePool().getInt32Type());
    ASSERT_NE(Pointer, nullptr);
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getBoolType(), {Pointer});
    ASSERT_NE(Signature, nullptr);
    const UnknownValue Unknown;
    const DescribedValue Custom;
    const CompositeValue Empty({});
    const CompositeValue Nested({Signature, &Unknown});
    const CompositeValue Composite({&Nested, &Custom, &Empty});
    const Value &ValueObject = Composite;
    EXPECT_EQ(ValueObject.toString(), "[[func(*i32): bool, unknown], custom-value, []]");
  }

  // Deep ordinary-value composition preserves the complete output without exhausting the formatting or destruction call stack.
  TEST(IRValueFormatTest, FormatsDeepCompositeValueChainsWithoutStackOverflow)
  {
    constexpr std::size_t Depth = 16384;
    const DescribedValue Leaf;
    const Value *ValueObject = &Leaf;
    std::vector<std::unique_ptr<CompositeValue>> Composites;
    Composites.reserve(Depth);
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Composites.push_back(std::make_unique<CompositeValue>(std::vector<const Value *>{ValueObject}));
      ValueObject = Composites.back().get();
    }
    EXPECT_EQ(ValueObject->toString(), std::string(Depth, '[') + "custom-value" + std::string(Depth, ']'));
  }
} // namespace ink::ir::test
