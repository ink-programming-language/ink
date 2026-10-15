#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/param/argument.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class BorrowedInstruction final : public Instruction
    {
      public:
        explicit BorrowedInstruction(bool &Destroyed) noexcept
            : Destroyed(Destroyed)
        {
        }

        ~BorrowedInstruction() noexcept override
        {
          Destroyed = true;
        }

      private:
        bool &Destroyed;
    };
  } // namespace

  static_assert(!std::is_base_of_v<Value, Argument>);
  static_assert(std::is_copy_constructible_v<Argument>);
  static_assert(std::is_copy_assignable_v<Argument>);
  static_assert(!std::is_default_constructible_v<Argument>);
  static_assert(!std::is_convertible_v<const Value &, Argument>);

  // Actual arguments borrow constants, instruction results and formal parameters without changing their Value identities.
  TEST(IRArgumentTest, ReferencesConstantInstructionAndParameterValues)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const auto Literal = Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 42);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()});
    ASSERT_NE(Literal, nullptr);
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const auto *Formal = Callable.parameter(0);
    ASSERT_NE(Formal, nullptr);
    bool Destroyed = false;
    BorrowedInstruction Operation(Destroyed);
    const Argument ConstantArgument(*Literal);
    const Argument InstructionArgument(Operation);
    const Argument ParameterArgument(*Formal);
    EXPECT_FALSE(ConstantArgument.hasName());
    EXPECT_EQ(ConstantArgument.name(), core::NAME_None);
    EXPECT_EQ(ConstantArgument.value(), Literal);
    EXPECT_EQ(InstructionArgument.value(), &Operation);
    EXPECT_EQ(ParameterArgument.value(), Formal);
    EXPECT_TRUE(Constant::classof(ConstantArgument.value()));
    EXPECT_TRUE(Instruction::classof(InstructionArgument.value()));
    EXPECT_TRUE(Parameter::classof(ParameterArgument.value()));
    EXPECT_FALSE(Destroyed);
  }

  // Separate calls supply different actual values while the callee retains the same formal parameter identity.
  TEST(IRArgumentTest, DifferentActualListsShareCalleeParameters)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()});
    const auto FirstLiteral = Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 1);
    const auto SecondLiteral = Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 2);
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(FirstLiteral, nullptr);
    ASSERT_NE(SecondLiteral, nullptr);
    Function Callee(*Signature);
    const auto *Formal = Callee.parameter(0);
    ASSERT_NE(Formal, nullptr);
    const std::vector<Argument> FirstCall = {Argument(*FirstLiteral)};
    const std::vector<Argument> SecondCall = {Argument(*SecondLiteral)};
    EXPECT_NE(FirstCall[0].value(), SecondCall[0].value());
    EXPECT_EQ(FirstCall[0].value(), FirstLiteral);
    EXPECT_EQ(SecondCall[0].value(), SecondLiteral);
    EXPECT_EQ(Callee.parameter(0), Formal);
    EXPECT_EQ(Formal->function(), &Callee);
    EXPECT_EQ(Callee.parameterCount(), 1U);
    EXPECT_EQ(Callee.functionType(), Signature);
  }

  // Renaming actual arguments is independent of the names on their referenced formal parameter and function values.
  TEST(IRArgumentTest, LocalNamesDoNotFollowReferencedValueNames)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()});
    ASSERT_NE(Signature, nullptr);
    const core::Name FunctionName = core::NamePool::instance().intern("IRArgumentTest.Callee");
    const core::Name FormalName = core::NamePool::instance().intern("IRArgumentTest.Formal");
    const core::Name ActualName = core::NamePool::instance().intern("IRArgumentTest.Actual");
    ASSERT_TRUE(FunctionName.valid());
    ASSERT_TRUE(FormalName.valid());
    ASSERT_TRUE(ActualName.valid());
    Function Callable(*Signature, FunctionName);
    auto *Formal = Callable.parameter(0);
    ASSERT_NE(Formal, nullptr);
    Formal->setName(FormalName);
    Argument ParameterArgument(*Formal, ActualName);
    Argument FunctionArgument(Callable);
    EXPECT_TRUE(ParameterArgument.hasName());
    EXPECT_EQ(ParameterArgument.name(), ActualName);
    EXPECT_FALSE(FunctionArgument.hasName());
    FunctionArgument.setName(ActualName);
    EXPECT_EQ(Formal->name(), FormalName);
    EXPECT_EQ(Callable.name(), FunctionName);
    Formal->setName(core::NAME_None);
    Callable.setName(core::NAME_None);
    EXPECT_EQ(ParameterArgument.name(), ActualName);
    EXPECT_EQ(FunctionArgument.name(), ActualName);
    ParameterArgument.setName(core::NAME_None);
    EXPECT_FALSE(ParameterArgument.hasName());
    EXPECT_EQ(FunctionArgument.name(), ActualName);
    EXPECT_EQ(ParameterArgument.value(), Formal);
    EXPECT_EQ(FunctionArgument.value(), &Callable);
  }

  // Copies share borrowed values but keep independent local names, and destroying the wrappers leaves the value owners intact.
  TEST(IRArgumentTest, CopiesDoNotOwnReferencedValues)
  {
    const core::Name OriginalName = core::NamePool::instance().intern("IRArgumentTest.Original");
    const core::Name CopyName = core::NamePool::instance().intern("IRArgumentTest.Copy");
    ASSERT_TRUE(OriginalName.valid());
    ASSERT_TRUE(CopyName.valid());
    bool FirstDestroyed = false;
    bool SecondDestroyed = false;
    auto First = std::make_unique<BorrowedInstruction>(FirstDestroyed);
    auto Second = std::make_unique<BorrowedInstruction>(SecondDestroyed);
    {
      const Argument Original(*First, OriginalName);
      Argument Copy = Original;
      Argument Assigned(*Second);
      EXPECT_EQ(Assigned.value(), Second.get());
      Assigned = Copy;
      EXPECT_EQ(Copy.value(), Original.value());
      EXPECT_EQ(Assigned.value(), First.get());
      EXPECT_EQ(Copy.name(), OriginalName);
      EXPECT_EQ(Assigned.name(), OriginalName);
      Copy.setName(CopyName);
      Assigned.setName(core::NAME_None);
      EXPECT_EQ(Original.name(), OriginalName);
      EXPECT_EQ(Copy.name(), CopyName);
      EXPECT_FALSE(Assigned.hasName());
      EXPECT_EQ(Copy.value(), First.get());
      EXPECT_EQ(Assigned.value(), First.get());
    }
    EXPECT_FALSE(FirstDestroyed);
    EXPECT_FALSE(SecondDestroyed);
    First.reset();
    Second.reset();
    EXPECT_TRUE(FirstDestroyed);
    EXPECT_TRUE(SecondDestroyed);
  }
} // namespace ink::ir::test
