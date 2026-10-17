#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/type.h"

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    class TrackedInstruction final : public Instruction
    {
      public:
        explicit TrackedInstruction(bool &Destroyed) noexcept
            : Destroyed(Destroyed)
        {
        }

        ~TrackedInstruction() noexcept override
        {
          Destroyed = true;
        }

      private:
        bool &Destroyed;
    };
  } // namespace

  static_assert(std::is_abstract_v<Instruction>);
  static_assert(std::is_base_of_v<Value, Instruction>);
  static_assert(!std::is_base_of_v<Type, Instruction>);
  static_assert(std::has_virtual_destructor_v<Instruction>);
  static_assert(!std::is_copy_constructible_v<TrackedInstruction>);
  static_assert(!std::is_copy_assignable_v<TrackedInstruction>);
  static_assert(!std::is_move_constructible_v<TrackedInstruction>);
  static_assert(!std::is_move_assignable_v<TrackedInstruction>);

  // A newly constructed instruction has no block owner before it is appended.
  TEST(IRInstructionTest, StartsWithoutParentBlock)
  {
    bool Destroyed = false;
    const TrackedInstruction Operation(Destroyed);
    EXPECT_EQ(Operation.parentBlock(), nullptr);
  }

  // Destroying a concrete instruction through Value runs its derived destructor and preserves instruction identity until deletion.
  TEST(IRInstructionTest, OwnedValueDestroysConcreteInstruction)
  {
    bool Destroyed = false;
    std::unique_ptr<Value> Operation = std::make_unique<TrackedInstruction>(Destroyed);
    EXPECT_EQ(Operation->kind(), ValueKind::Instruction);
    EXPECT_TRUE(Instruction::classof(Operation.get()));
    EXPECT_FALSE(Type::classof(Operation.get()));
    EXPECT_FALSE(Destroyed);
    Operation.reset();
    EXPECT_TRUE(Destroyed);
  }
} // namespace ink::ir::test
