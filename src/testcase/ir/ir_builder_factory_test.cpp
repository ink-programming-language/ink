#include "ink/ir/ir_builder.h"
#include "ink/parser/parser.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // Non-instruction factories preserve an active anchor and place ownership outside the builder's lifetime.
  TEST(IRIRBuilderTest, NonInstructionFactoriesPreservePointAndOutliveBuilder)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func F[T: type](Value: T): T { return Value; } class Box[T: type] { field Item: T; };"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto &Root = *Parsed.Unit->root();
    const auto &FunctionAST = static_cast<const parser::FunctionDecl &>(*static_cast<const parser::DeclStmt *>(Root.statements()[0])->declaration());
    const auto &ClassAST = static_cast<const parser::ClassDecl &>(*static_cast<const parser::DeclStmt *>(Root.statements()[1])->declaration());
    IRContext Context(Compilation);
    std::unique_ptr<BasicBlock> DetachedBlock;
    Module *ModuleValue = nullptr;
    Function *FunctionValue = nullptr;
    ModuleDecl *ModuleDefinition = nullptr;
    FunctionDecl *FunctionDefinition = nullptr;
    ClassDecl *ClassDefinition = nullptr;
    const ClassType *Class = nullptr;
    const EnumType *Enum = nullptr;
    const InterfaceType *Interface = nullptr;
    {
      IRBuilder Builder(Context);
      DetachedBlock = Builder.createBasicBlock();
      ASSERT_NE(DetachedBlock, nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*DetachedBlock));
      AllocaInstruction *Anchor = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
      ASSERT_NE(Anchor, nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
      const Name ModuleName = Context.namePool().intern("Root");
      const Name FunctionName = Context.namePool().intern("F");
      const Name ClassName = Context.namePool().intern("Box");
      ModuleValue = Builder.createModule(ModuleName);
      ASSERT_NE(ModuleValue, nullptr);
      auto FunctionOwner = Builder.createFunction(FunctionName, *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
      ASSERT_NE(FunctionOwner, nullptr);
      FunctionValue = FunctionOwner.get();
      BasicBlock *Entry = Builder.createFunctionBody(*FunctionValue);
      BasicBlock *Tail = Builder.createBasicBlock(*FunctionValue);
      ASSERT_NE(Entry, nullptr);
      ASSERT_NE(Tail, nullptr);
      EXPECT_EQ(FunctionValue->entryBlock(), Entry);
      EXPECT_EQ(Entry->outer(), FunctionValue);
      EXPECT_EQ(Tail->outer(), FunctionValue);
      EXPECT_EQ(Builder.createFunctionBody(*FunctionValue), nullptr);
      ASSERT_TRUE(Builder.appendValue(ModuleValue->entryBlock(), std::move(FunctionOwner)));
      ModuleDefinition = Builder.createModuleDecl(*ModuleValue, Root);
      ASSERT_NE(ModuleDefinition, nullptr);
      FunctionDefinition = Builder.createFunctionDecl(*ModuleDefinition, FunctionName, FunctionAST);
      ClassDefinition = Builder.createClassDecl(*ModuleDefinition, ClassName, ClassAST);
      Class = Builder.createClassType(ClassName);
      Enum = Builder.createEnumType(ClassName);
      Interface = Builder.createInterfaceType(ClassName);
      ASSERT_NE(ModuleDefinition, nullptr);
      ASSERT_NE(FunctionDefinition, nullptr);
      ASSERT_NE(ClassDefinition, nullptr);
      ASSERT_NE(Class, nullptr);
      ASSERT_NE(Enum, nullptr);
      ASSERT_NE(Interface, nullptr);
      EXPECT_EQ(Builder.insertBlock(), DetachedBlock.get());
      EXPECT_EQ(Builder.saveInsertPoint().Before, Anchor);
      ASSERT_EQ(DetachedBlock->values().size(), 1U);
      EXPECT_EQ(DetachedBlock->values()[0].get(), Anchor);
    }
    ASSERT_EQ(Context.modules().size(), 1U);
    EXPECT_EQ(Context.modules()[0].get(), ModuleValue);
    EXPECT_EQ(ModuleValue->entryBlock().outer(), ModuleValue);
    ASSERT_EQ(ModuleValue->entryBlock().values().size(), 1U);
    EXPECT_EQ(ModuleValue->entryBlock().values()[0].get(), FunctionValue);
    EXPECT_EQ(FunctionValue->blocks().size(), 2U);
    EXPECT_EQ(DetachedBlock->outer(), nullptr);
    EXPECT_EQ(DetachedBlock->values().size(), 1U);
    EXPECT_EQ(ModuleValue->declarationRoot(), ModuleDefinition);
    EXPECT_EQ(&FunctionDefinition->module(), ModuleValue);
    EXPECT_EQ(&ClassDefinition->module(), ModuleValue);
    EXPECT_EQ(FunctionDefinition->parent(), ModuleDefinition);
    EXPECT_EQ(ClassDefinition->parent(), ModuleDefinition);
    EXPECT_EQ(&ModuleDefinition->ast(), &Root);
    EXPECT_EQ(&FunctionDefinition->ast(), &FunctionAST);
    EXPECT_EQ(&ClassDefinition->ast(), &ClassAST);
    for (const Type *Nominal : {static_cast<const Type *>(Class), static_cast<const Type *>(Enum), static_cast<const Type *>(Interface)})
    {
      EXPECT_EQ(&Nominal->context(), &Context);
      EXPECT_EQ(&Nominal->type(), &Context.typePool().getType<TypeKind::Meta>());
    }
  }

  // Detached creation accepts both unset and active points without inserting, while owners keep all six instructions alive.
  TEST(IRIRBuilderTest, DetachedInstructionFactoriesIgnoreInsertionPoint)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const IntegerType *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Integer, nullptr);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    ASSERT_NE(One, nullptr);
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("F"), *Context.typePool().getType<TypeKind::Function>(*Integer));
    ASSERT_NE(FunctionOwner, nullptr);
    auto Block = Builder.createBasicBlock();
    for (bool HasPoint : {false, true})
    {
      if (HasPoint)
      {
        ASSERT_TRUE(Builder.setInsertPoint(*Block));
      }
      auto Slot = Builder.createDetachedAllocaInstruction(*Integer);
      ASSERT_NE(Slot, nullptr);
      auto Store = Builder.createDetachedStoreInstruction(*Slot, *One);
      auto Load = Builder.createDetachedLoadInstruction(*Slot);
      auto Add = Builder.createDetachedAddInstruction(*One, *One);
      auto Call = Builder.createDetachedCallInstruction(*FunctionOwner);
      auto Return = Builder.createDetachedReturnInstruction(One);
      ASSERT_NE(Store, nullptr);
      ASSERT_NE(Load, nullptr);
      ASSERT_NE(Add, nullptr);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Return, nullptr);
      const Value *Instructions[] = {
          Slot.get(),
          Store.get(),
          Load.get(),
          Add.get(),
          Call.get(),
          Return.get(),
      };
      for (const Value *Instruction : Instructions)
      {
        EXPECT_EQ(Instruction->outer(), nullptr);
        EXPECT_EQ(&Instruction->context(), &Context);
      }
      EXPECT_EQ(Builder.insertBlock(), HasPoint ? Block.get() : nullptr);
      EXPECT_EQ(Builder.saveInsertPoint().Before, nullptr);
      EXPECT_TRUE(Block->values().empty());
    }
    Builder.clearInsertPoint();
  }
} // namespace ink::ir::test
