#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(std::is_base_of_v<Value, Function>);
  static_assert(!std::is_base_of_v<Type, Function>);
  static_assert(!std::is_copy_constructible_v<Function>);
  static_assert(!std::is_copy_assignable_v<Function>);
  static_assert(!std::is_move_constructible_v<Function>);
  static_assert(!std::is_move_assignable_v<Function>);

  // An empty signature creates a public local declaration with no parameters or blocks.
  TEST(IRFunctionTest, EmptySignatureCreatesUnnamedDeclaration)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const Function &ReadOnly = Callable;
    EXPECT_EQ(Callable.functionType(), Signature);
    EXPECT_EQ(Callable.name(), core::NAME_None);
    EXPECT_FALSE(Callable.hasName());
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Local);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::Ink);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Public);
    EXPECT_TRUE(Callable.parameters().empty());
    EXPECT_EQ(Callable.parameterCount(), 0U);
    EXPECT_EQ(Callable.parameter(0), nullptr);
    EXPECT_EQ(ReadOnly.parameter(0), nullptr);
    EXPECT_TRUE(Callable.blocks().empty());
    EXPECT_EQ(Callable.blockCount(), 0U);
    EXPECT_EQ(Callable.block(0), nullptr);
    EXPECT_EQ(ReadOnly.block(0), nullptr);
    EXPECT_EQ(Callable.entryBlock(), nullptr);
    EXPECT_EQ(ReadOnly.entryBlock(), nullptr);
    EXPECT_TRUE(Callable.isDeclaration());
  }

  // A function entity, its signature and its parameters have distinct classifications through Value.
  TEST(IRFunctionTest, FunctionEntityDoesNotClassifyAsSignatureOrConstant)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::BoolType>(), {Types.typePool().get<ink::ir::BoolType>()});
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const Value *Entity = &Callable;
    const Value *SignatureValue = Signature;
    const Value *ParameterValue = Callable.parameter(0);
    ASSERT_NE(ParameterValue, nullptr);
    EXPECT_EQ(Entity->kind(), ValueKind::Function);
    EXPECT_TRUE(Function::classof(Entity));
    EXPECT_FALSE(FunctionType::classof(Entity));
    EXPECT_FALSE(Type::classof(Entity));
    EXPECT_FALSE(Constant::classof(Entity));
    EXPECT_FALSE(Parameter::classof(Entity));
    EXPECT_TRUE(Type::classof(SignatureValue));
    EXPECT_TRUE(FunctionType::classof(SignatureValue));
    EXPECT_FALSE(Function::classof(SignatureValue));
    EXPECT_FALSE(Function::classof(ParameterValue));
    EXPECT_FALSE(Function::classof(nullptr));
  }

  // Symbol linkage, language linkage and visibility change independently, including a private C export.
  TEST(IRFunctionTest, UpdatesIndependentLinkageAndVisibility)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name OriginalName = core::NamePool::instance().intern("IRFunctionTest.Original");
    const core::Name NewName = core::NamePool::instance().intern("IRFunctionTest.Renamed");
    ASSERT_TRUE(OriginalName.valid());
    ASSERT_TRUE(NewName.valid());
    Function Callable(*Signature, OriginalName, SymbolLinkage::Import, LanguageLinkage::C, FunctionVisibility::Private);
    EXPECT_EQ(Callable.name(), OriginalName);
    EXPECT_TRUE(Callable.hasName());
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Import);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Private);
    Callable.setName(NewName);
    EXPECT_EQ(Callable.name(), NewName);
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Import);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Private);
    Callable.setSymbolLinkage(SymbolLinkage::Export);
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Export);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Private);
    Callable.setLanguageLinkage(LanguageLinkage::Ink);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::Ink);
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Export);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Private);
    Callable.setVisibility(FunctionVisibility::Public);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Public);
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Export);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::Ink);
    EXPECT_EQ(Callable.name(), NewName);
    EXPECT_EQ(Callable.functionType(), Signature);
    EXPECT_TRUE(Callable.isDeclaration());
    Callable.setSymbolLinkage(SymbolLinkage::Local);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Public);
    Callable.setVisibility(FunctionVisibility::Private);
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Local);
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::Ink);
    Callable.setName(core::NAME_None);
    EXPECT_FALSE(Callable.hasName());
    EXPECT_EQ(Callable.languageLinkage(), LanguageLinkage::Ink);
    EXPECT_EQ(Callable.symbolLinkage(), SymbolLinkage::Local);
    EXPECT_EQ(Callable.visibility(), FunctionVisibility::Private);
  }

  // An Ink import keeps its own name and declaration state independently of an exported function's body.
  TEST(IRFunctionTest, InkImportKeepsIndependentNameLinkageAndBody)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Signature, nullptr);
    const core::Name ProviderName = core::NamePool::instance().intern("IRFunctionTest.Provider");
    const core::Name AliasName = core::NamePool::instance().intern("IRFunctionTest.ImportAlias");
    ASSERT_TRUE(ProviderName.valid());
    ASSERT_TRUE(AliasName.valid());
    ASSERT_NE(ProviderName, AliasName);
    Function Provider(*Signature, ProviderName, SymbolLinkage::Export);
    ASSERT_NE(Provider.createBlock(), nullptr);
    Function Imported(*Signature, AliasName, SymbolLinkage::Import, LanguageLinkage::Ink);
    EXPECT_EQ(Imported.symbolLinkage(), SymbolLinkage::Import);
    EXPECT_EQ(Imported.languageLinkage(), LanguageLinkage::Ink);
    EXPECT_EQ(Imported.visibility(), FunctionVisibility::Public);
    EXPECT_TRUE(Imported.isDeclaration());
    EXPECT_TRUE(Imported.blocks().empty());
    EXPECT_FALSE(Provider.isDeclaration());
    EXPECT_EQ(Provider.symbolLinkage(), SymbolLinkage::Export);
    EXPECT_EQ(Provider.visibility(), FunctionVisibility::Public);
    EXPECT_NE(Imported.name(), Provider.name());
    Imported.setName(ProviderName);
    EXPECT_EQ(Imported.name(), ProviderName);
    EXPECT_EQ(Imported.symbolLinkage(), SymbolLinkage::Import);
    EXPECT_TRUE(Imported.isDeclaration());
  }

  // Adding blocks creates a definition while preserving insertion order, parent pointers and earlier addresses.
  TEST(IRFunctionTest, OwnsStableBlocksAndTransitionsToDefinition)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const Function &ReadOnly = Callable;
    const core::Name EntryName = core::NamePool::instance().intern("IRFunctionTest.Entry");
    ASSERT_TRUE(EntryName.valid());
    BasicBlock Detached;
    const BasicBlock &ReadOnlyDetached = Detached;
    EXPECT_EQ(Detached.function(), nullptr);
    EXPECT_EQ(ReadOnlyDetached.function(), nullptr);
    auto *Entry = Callable.createBlock(EntryName);
    ASSERT_NE(Entry, nullptr);
    EXPECT_FALSE(Callable.isDeclaration());
    EXPECT_EQ(Entry->name(), EntryName);
    EXPECT_EQ(Entry->function(), &Callable);
    ASSERT_NE(ReadOnly.entryBlock(), nullptr);
    EXPECT_EQ(ReadOnly.entryBlock()->function(), &ReadOnly);
    for (std::size_t Index = 1; Index < 65; ++Index)
    {
      auto *Block = Callable.createBlock();
      ASSERT_NE(Block, nullptr);
      EXPECT_EQ(Block->function(), &Callable);
      EXPECT_EQ(Callable.block(Index), Block);
      EXPECT_EQ(ReadOnly.block(Index), Block);
    }
    EXPECT_EQ(Callable.blockCount(), 65U);
    EXPECT_EQ(Callable.blocks().front().get(), Entry);
    EXPECT_EQ(Callable.entryBlock(), Entry);
    EXPECT_EQ(ReadOnly.entryBlock(), Entry);
    EXPECT_EQ(Callable.block(0), Entry);
    EXPECT_EQ(ReadOnly.block(0), Entry);
    EXPECT_EQ(Callable.block(Callable.blockCount()), nullptr);
    EXPECT_EQ(ReadOnly.block(ReadOnly.blockCount()), nullptr);
  }
} // namespace ink::ir::test
