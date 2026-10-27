#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/serialization/serialization.h"

#include <gtest/gtest.h>

#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Every comparison predicate emits its specified assembly spelling and retains its native meaning through both codecs.
  TEST(IRSerializationSchemaTest, PreservesEveryComparisonPredicateAndSpelling)
  {
    struct PredicateCase
    {
        ICmpPredicate Predicate;
        std::string_view Spelling;
    };
    constexpr PredicateCase Cases[] = {
        {ICmpPredicate::Equal, "eq"},
        {ICmpPredicate::NotEqual, "ne"},
        {ICmpPredicate::UnsignedGreaterThan, "ugt"},
        {ICmpPredicate::UnsignedGreaterEqual, "uge"},
        {ICmpPredicate::UnsignedLessThan, "ult"},
        {ICmpPredicate::UnsignedLessEqual, "ule"},
        {ICmpPredicate::SignedGreaterThan, "sgt"},
        {ICmpPredicate::SignedGreaterEqual, "sge"},
        {ICmpPredicate::SignedLessThan, "slt"},
        {ICmpPredicate::SignedLessEqual, "sle"},
    };

    IRContext Context(Target);
    Module Source(Context);
    auto &Names = core::NamePool::instance();
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    const auto *Signature = Context.typePool().get<FunctionType>(Context.typePool().get<BoolType>(), {Integer, Integer});
    IRBuilder Builder(Context);
    for (const auto &Case : Cases)
    {
      auto *Callable = Source.createGlobalFunction(*Signature, Names.intern("compare_" + std::string(Case.Spelling)));
      ASSERT_NE(Callable, nullptr);
      Callable->parameter(0)->setName(Names.intern("left"));
      Callable->parameter(1)->setName(Names.intern("right"));
      ASSERT_TRUE(Builder.setInsertPoint(*Callable->createBlock()));
      auto *Comparison = Builder.createICmp(Case.Predicate, *Callable->parameter(0), *Callable->parameter(1));
      ASSERT_NE(Comparison, nullptr);
      ASSERT_NE(Builder.createReturn(*Comparison), nullptr);
    }

    const auto Text = writeText(Source);
    ASSERT_TRUE(Text) << Text.error().Message;
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Spelling);
      const auto FunctionStart = Text->find("define bool @compare_" + std::string(Case.Spelling) + "(");
      ASSERT_NE(FunctionStart, std::string::npos);
      const auto FunctionEnd = Text->find("\n  }", FunctionStart);
      ASSERT_NE(FunctionEnd, std::string::npos);
      const auto Comparison = Text->find("icmp " + std::string(Case.Spelling) + " i32 %left, i32 %right", FunctionStart);
      EXPECT_LT(Comparison, FunctionEnd);
    }
    const auto Binary = writeBinary(Source);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext TextContext(Target);
    IRContext BinaryContext(Target);
    const auto TextModule = readText(TextContext, *Text);
    ASSERT_TRUE(TextModule) << TextModule.error().Message;
    const auto BinaryModule = readBinary(BinaryContext, *Binary);
    ASSERT_TRUE(BinaryModule) << BinaryModule.error().Message;
    for (const Module *Loaded : {TextModule->get(), BinaryModule->get()})
    {
      ASSERT_EQ(Loaded->globalFunctionCount(), std::size(Cases));
      for (std::size_t Index = 0; Index < std::size(Cases); ++Index)
      {
        SCOPED_TRACE(Cases[Index].Spelling);
        const auto *Callable = Loaded->globalFunction(Index);
        ASSERT_EQ(Callable->blockCount(), 1U);
        ASSERT_EQ(Callable->entryBlock()->instructionCount(), 2U);
        const auto *Value = Callable->entryBlock()->instruction(0);
        ASSERT_TRUE(ICmpInstruction::classof(Value));
        const auto *Comparison = static_cast<const ICmpInstruction *>(Value);
        EXPECT_EQ(Comparison->predicate(), Cases[Index].Predicate);
        EXPECT_EQ(Comparison->left(), Callable->parameter(0));
        EXPECT_EQ(Comparison->right(), Callable->parameter(1));
      }
    }
  }

  // Linkage, language and visibility combinations retain independent fields and their canonical declaration syntax.
  TEST(IRSerializationSchemaTest, PreservesFunctionMetadataCombinationsAndSpelling)
  {
    struct LinkageCase
    {
        SymbolLinkage Value;
        std::string_view Prefix;
    };
    struct LanguageCase
    {
        LanguageLinkage Value;
        std::string_view Prefix;
    };
    struct VisibilityCase
    {
        FunctionVisibility Value;
        std::string_view Attributes;
    };
    constexpr LinkageCase Linkages[] = {
        {SymbolLinkage::Local, ""},
        {SymbolLinkage::Import, "import "},
        {SymbolLinkage::Export, "export "},
    };
    constexpr LanguageCase Languages[] = {
        {LanguageLinkage::Ink, ""},
        {LanguageLinkage::C, "\"C\" "},
    };
    constexpr VisibilityCase Visibilities[] = {
        {FunctionVisibility::Public, ""},
        {FunctionVisibility::Private, " [private]"},
    };

    for (const auto &Linkage : Linkages)
    {
      for (const auto &Language : Languages)
      {
        for (const auto &Visibility : Visibilities)
        {
          const std::string Expected = "declare " + std::string(Linkage.Prefix) + std::string(Language.Prefix) + "void @metadata()" + std::string(Visibility.Attributes) + "\n";
          SCOPED_TRACE(Expected);
          IRContext Context(Target);
          Module Source(Context);
          const auto *Signature = Context.typePool().get<FunctionType>(Context.typePool().get<VoidType>());
          ASSERT_NE(Source.createGlobalFunction(*Signature, core::NamePool::instance().intern("metadata"), Linkage.Value, Language.Value, Visibility.Value), nullptr);
          const auto Text = writeText(Source);
          ASSERT_TRUE(Text) << Text.error().Message;
          EXPECT_NE(Text->find(Expected), std::string::npos);
          const auto Binary = writeBinary(Source);
          ASSERT_TRUE(Binary) << Binary.error().Message;
          IRContext TextContext(Target);
          IRContext BinaryContext(Target);
          const auto TextModule = readText(TextContext, *Text);
          ASSERT_TRUE(TextModule) << TextModule.error().Message;
          const auto BinaryModule = readBinary(BinaryContext, *Binary);
          ASSERT_TRUE(BinaryModule) << BinaryModule.error().Message;
          for (const Module *Loaded : {TextModule->get(), BinaryModule->get()})
          {
            ASSERT_EQ(Loaded->globalFunctionCount(), 1U);
            const auto *Callable = Loaded->globalFunction(0);
            EXPECT_EQ(Callable->symbolLinkage(), Linkage.Value);
            EXPECT_EQ(Callable->languageLinkage(), Language.Value);
            EXPECT_EQ(Callable->visibility(), Visibility.Value);
          }
        }
      }
    }
  }

  // Direct, virtual and abstract methods keep dispatch, override, final and slot fields distinct across both codecs.
  TEST(IRSerializationSchemaTest, PreservesMethodDispatchMetadataAndSpelling)
  {
    struct DispatchCase
    {
        std::string_view Name;
        MethodVirtualInfo Info;
        std::string_view Attributes;
    };
    const DispatchCase Cases[] = {
        {"method_direct", {MethodDispatch::Direct, false, false, std::nullopt}, " [owner = %Owner]"},
        {"method_virtual", {MethodDispatch::Virtual, true, true, 5}, " [owner = %Owner, virtual, override, final, slot = 5]"},
        {"method_abstract", {MethodDispatch::Abstract, true, false, 9}, " [owner = %Owner, abstract, override, slot = 9]"},
    };

    IRContext Context(Target);
    Module Source(Context);
    auto &Names = core::NamePool::instance();
    auto *Owner = Source.createClassType(8, 8, Names.intern("Owner"));
    ASSERT_NE(Owner, nullptr);
    const auto *Signature = Context.typePool().get<FunctionType>(Context.typePool().get<VoidType>());
    for (const auto &Case : Cases)
    {
      ASSERT_TRUE(Owner->addMethod(Method(std::make_unique<Function>(*Signature, Names.intern(Case.Name)), Case.Info)));
    }
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text) << Text.error().Message;
    for (const auto &Case : Cases)
    {
      EXPECT_NE(Text->find("declare void @" + std::string(Case.Name) + "()" + std::string(Case.Attributes) + "\n"), std::string::npos) << Case.Name;
    }
    const auto Binary = writeBinary(Source);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext TextContext(Target);
    IRContext BinaryContext(Target);
    const auto TextModule = readText(TextContext, *Text);
    ASSERT_TRUE(TextModule) << TextModule.error().Message;
    const auto BinaryModule = readBinary(BinaryContext, *Binary);
    ASSERT_TRUE(BinaryModule) << BinaryModule.error().Message;
    for (const Module *Loaded : {TextModule->get(), BinaryModule->get()})
    {
      ASSERT_EQ(Loaded->classCount(), 1U);
      ASSERT_EQ(Loaded->classType(0)->methodCount(), std::size(Cases));
      for (std::size_t Index = 0; Index < std::size(Cases); ++Index)
      {
        SCOPED_TRACE(Cases[Index].Name);
        const auto &Info = Loaded->classType(0)->method(Index)->virtualInfo();
        EXPECT_EQ(Info.Dispatch, Cases[Index].Info.Dispatch);
        EXPECT_EQ(Info.Override, Cases[Index].Info.Override);
        EXPECT_EQ(Info.Final, Cases[Index].Info.Final);
        EXPECT_EQ(Info.Slot, Cases[Index].Info.Slot);
      }
    }
  }
} // namespace ink::ir::test
