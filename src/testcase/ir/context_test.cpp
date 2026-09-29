#include "ink/ir/ir_builder.h"
#include "ink/ir/lifetime_observer.h"
#include "ink/parser/parser.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  namespace
  {
    class DestructionObserver final : public LifetimeObserver
    {
      public:
        explicit DestructionObserver(IRContext &Context)
            : LifetimeObserver(Context)
        {
        }

        unsigned Values = 0;
        unsigned Declarations = 0;

      private:
        void valueDestroyed(Value &) noexcept override
        {
          ++Values;
        }

        void declDestroyed(Decl &) noexcept override
        {
          ++Declarations;
        }
    };
  } // namespace

  // Erasing an IR module reports its declaration and value tree without a semantic context.
  TEST(IRContextTest, ModuleDestructionNotifiesObservers)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Parsed.succeeded());
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    DestructionObserver First(Context);
    DestructionObserver Second(Context);
    Module *Owner = Builder.createModule(Context.namePool().intern("main"));
    ASSERT_NE(Owner, nullptr);
    ASSERT_NE(Builder.createModuleDecl(*Owner, *Parsed.Unit->root()), nullptr);
    ASSERT_TRUE(Builder.eraseModule(*Owner));
    EXPECT_EQ(First.Values, 2U);
    EXPECT_EQ(First.Declarations, 1U);
    EXPECT_EQ(Second.Values, First.Values);
    EXPECT_EQ(Second.Declarations, First.Declarations);
  }

  // Destroying one observer unregisters it while preserving the remaining subscriber.
  TEST(IRContextTest, ObserverCanUnregisterBeforeNodesAreDestroyed)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    DestructionObserver Remaining(Context);
    {
      DestructionObserver Temporary(Context);
      auto Block = Builder.createBasicBlock();
      ASSERT_NE(Block, nullptr);
      Block.reset();
      EXPECT_EQ(Temporary.Values, 1U);
    }
    auto Block = Builder.createBasicBlock();
    ASSERT_NE(Block, nullptr);
    Block.reset();
    EXPECT_EQ(Remaining.Values, 2U);
  }

  // Observers receive only notifications from their own IR context.
  TEST(IRContextTest, DestructionNotificationsStayInTheirContext)
  {
    core::CompilationContext Compilation;
    IRContext First(Compilation);
    IRContext Second(Compilation);
    DestructionObserver Observer(First);
    IRBuilder Builder(Second);
    auto Block = Builder.createBasicBlock();
    ASSERT_NE(Block, nullptr);
    Block.reset();
    EXPECT_EQ(Observer.Values, 0U);
    EXPECT_EQ(Observer.Declarations, 0U);
  }
} // namespace ink::ir::test
