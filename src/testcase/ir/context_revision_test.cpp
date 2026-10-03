#include "ink/ir/ir_builder.h"
#include "ink/ir/module/module_serialization.h"
#include "ink/parser/parser.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // A cached function body is invalidated by block creation, insertion, removal and reinsertion.
  TEST(IRContextRevisionTest, FunctionStructureEditsAdvanceRevision)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto &Void = Context.typePool().getType<TypeKind::Void>();
    const auto &Bool = Context.typePool().getType<TypeKind::Bool>();
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("main"), *Context.typePool().getType<TypeKind::Function>(Void));
    ASSERT_NE(FunctionOwner, nullptr);
    std::uint64_t Previous = Context.revision();
    BasicBlock *Entry = Builder.createFunctionBody(*FunctionOwner);
    ASSERT_NE(Entry, nullptr);
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    ASSERT_NE(Builder.createBasicBlock(*FunctionOwner), nullptr);
    EXPECT_GT(Context.revision(), Previous);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Return = Builder.createReturnInstruction();
    ASSERT_NE(Return, nullptr);
    auto Slot = Builder.createDetachedAllocaInstruction(Bool);
    Value *SlotPointer = Slot.get();
    Previous = Context.revision();
    ASSERT_TRUE(Builder.insertValue(*Entry, std::move(Slot), Return));
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    auto Detached = Builder.removeValue(*Entry, *SlotPointer);
    ASSERT_NE(Detached, nullptr);
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    ASSERT_TRUE(Builder.insertValue(*Entry, std::move(Detached), Return));
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    ASSERT_TRUE(Builder.eraseValue(*Entry, *SlotPointer));
    EXPECT_GT(Context.revision(), Previous);
  }

  // Moving a root module out of and back into its context invalidates cached ownership assumptions.
  TEST(IRContextRevisionTest, ModuleOwnershipEditsAdvanceRevision)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    std::uint64_t Previous = Context.revision();
    Module *ModuleValue = Builder.createModule(Context.namePool().intern("main"));
    ASSERT_NE(ModuleValue, nullptr);
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    auto Owner = Builder.removeModule(*ModuleValue);
    ASSERT_NE(Owner, nullptr);
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    ASSERT_TRUE(Builder.appendModule(std::move(Owner)));
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    ASSERT_TRUE(Builder.eraseModule(*ModuleValue));
    EXPECT_GT(Context.revision(), Previous);
  }

  // Destruction invalidates detached function identities before an allocator can reuse their addresses.
  TEST(IRContextRevisionTest, DetachedDestructionInvalidatesBorrowedIdentities)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto &Void = Context.typePool().getType<TypeKind::Void>();
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("main"), *Context.typePool().getType<TypeKind::Function>(Void));
    ASSERT_NE(FunctionOwner, nullptr);
    ASSERT_NE(Builder.createFunctionBody(*FunctionOwner), nullptr);
    const std::uint64_t Previous = Context.revision();
    FunctionOwner.reset();
    EXPECT_GT(Context.revision(), Previous);
  }

  // Interning immutable constants during compile-time execution must not invalidate executable code.
  TEST(IRContextRevisionTest, ImmutablePoolGrowthAndReadsPreserveRevision)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const std::uint64_t Previous = Context.revision();
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Integer, nullptr);
    const auto *ConstantValue = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 42));
    ASSERT_NE(ConstantValue, nullptr);
    EXPECT_EQ(Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 42)), ConstantValue);
    EXPECT_TRUE(Context.namePool().intern("new_name").valid());
    EXPECT_TRUE(Context.modules().empty());
    EXPECT_EQ(Context.revision(), Previous);
  }

  // Rejected ownership edits preserve both the caller's owner and the existing code revision.
  TEST(IRContextRevisionTest, RejectedEditsPreserveRevision)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Other(Compilation);
    IRBuilder Builder(Context);
    IRBuilder OtherBuilder(Other);
    auto LocalBlock = Builder.createBasicBlock();
    auto ForeignBlock = OtherBuilder.createBasicBlock();
    auto Slot = Builder.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Slot, nullptr);
    const std::uint64_t Previous = Context.revision();
    const std::uint64_t OtherPrevious = Other.revision();
    EXPECT_FALSE(Builder.appendValue(*ForeignBlock, std::move(Slot)));
    EXPECT_NE(Slot, nullptr);
    EXPECT_EQ(Builder.removeValue(*LocalBlock, *Slot), nullptr);
    EXPECT_EQ(Context.revision(), Previous);
    EXPECT_EQ(Other.revision(), OtherPrevious);
    Slot.reset();
    EXPECT_GT(Context.revision(), Previous);
    EXPECT_EQ(Other.revision(), OtherPrevious);
  }

  // Declaration tree growth is observable even when its parent module already exists.
  TEST(IRContextRevisionTest, DeclarationAttachmentsAdvanceRevision)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Identity[T: type](Value: T): T { return Value; } class Box[T: type] { field Item: T; };"));
    ASSERT_TRUE(Parsed.succeeded());
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    Module *ModuleValue = Builder.createModule(Context.namePool().intern("main"));
    ASSERT_NE(ModuleValue, nullptr);
    std::uint64_t Previous = Context.revision();
    ModuleDecl *Root = Builder.createModuleDecl(*ModuleValue, *Parsed.Unit->root());
    ASSERT_NE(Root, nullptr);
    EXPECT_GT(Context.revision(), Previous);
    const auto *FunctionStatement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0]);
    const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(FunctionStatement->declaration());
    Previous = Context.revision();
    ASSERT_NE(Builder.createFunctionDecl(*Root, Context.namePool().intern("Identity"), *FunctionAST), nullptr);
    EXPECT_GT(Context.revision(), Previous);
    const auto *ClassStatement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[1]);
    const auto *ClassAST = static_cast<const parser::ClassDecl *>(ClassStatement->declaration());
    Previous = Context.revision();
    ASSERT_NE(Builder.createClassDecl(*Root, Context.namePool().intern("Box"), *ClassAST), nullptr);
    EXPECT_GT(Context.revision(), Previous);
  }

  // Both archive formats publish new roots through the same cache invalidation boundary.
  TEST(IRContextRevisionTest, DeserializationAdvancesRevisionAndSerializationPreservesIt)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    Module *ModuleValue = Builder.createModule(Context.namePool().intern("main"));
    ASSERT_NE(ModuleValue, nullptr);
    const auto &Void = Context.typePool().getType<TypeKind::Void>();
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("run"), *Context.typePool().getType<TypeKind::Function>(Void));
    ASSERT_NE(FunctionOwner, nullptr);
    BasicBlock *Entry = Builder.createFunctionBody(*FunctionOwner);
    ASSERT_NE(Entry, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createReturnInstruction(), nullptr);
    ASSERT_TRUE(Builder.appendValue(ModuleValue->entryBlock(), std::move(FunctionOwner)));
    std::uint64_t Previous = Context.revision();
    const auto Text = serializeModuleText(*ModuleValue);
    const auto Binary = serializeModuleBinary(*ModuleValue);
    ASSERT_TRUE(Text.succeeded()) << Text.Message;
    ASSERT_TRUE(Binary.succeeded()) << Binary.Message;
    EXPECT_EQ(Context.revision(), Previous);
    const auto TextRestored = deserializeModuleText(Context, Text.Bytes);
    ASSERT_TRUE(TextRestored.succeeded()) << TextRestored.Message;
    EXPECT_GT(Context.revision(), Previous);
    Previous = Context.revision();
    const auto BinaryRestored = deserializeModuleBinary(Context, Binary.Bytes);
    ASSERT_TRUE(BinaryRestored.succeeded()) << BinaryRestored.Message;
    EXPECT_GT(Context.revision(), Previous);
    EXPECT_EQ(Context.modules().size(), 3U);
  }
} // namespace ink::ir::test
