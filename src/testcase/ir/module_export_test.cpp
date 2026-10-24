#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    SymbolId symbol(std::string_view Name)
    {
      return {core::NamePool::instance().intern(Name)};
    }

    void expectExport(const Module &Owner, const SymbolId &ID, std::initializer_list<const Value *> Declarations)
    {
      const auto *Export = Owner.findExport(ID);
      ASSERT_NE(Export, nullptr);
      EXPECT_EQ(Export->Symbol.SymbolName, ID.SymbolName);
      ASSERT_EQ(Export->Entries.size(), Declarations.size());
      std::size_t Index = 0;
      for (const Value *Declaration : Declarations)
      {
        EXPECT_EQ(Export->Entries[Index].Declaration, Declaration);
        EXPECT_EQ(Export->Entries[Index].SymbolType, Declaration->type());
        ++Index;
      }
    }
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<Module &>().exports()), const std::vector<SymbolExport> &>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().exports()), const std::vector<SymbolExport> &>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().findExport(std::declval<const SymbolId &>())), const SymbolExport *>);

  // Explicit exports retain function, global, nominal type and shared pooled type declarations in entries containing their corresponding types.
  TEST(IRModuleExportTest, RecordsOwnedDeclarationsAndSharedTypes)
  {
    IRContext Context(Target);
    Module Owner(Context);
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    auto *Variable = Owner.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, true));
    auto *Nominal = Owner.createStructType(8, 8);
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    ASSERT_NE(Callable, nullptr);
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Nominal, nullptr);

    const SymbolId FunctionID = symbol("IRModuleExportTest.Function");
    const SymbolId GlobalID = symbol("IRModuleExportTest.Global");
    const SymbolId NominalID = symbol("IRModuleExportTest.Nominal");
    const SymbolId IntegerID = symbol("IRModuleExportTest.Integer");
    ASSERT_TRUE(Owner.addExport(FunctionID, *Callable));
    ASSERT_TRUE(Owner.addExport(GlobalID, *Variable));
    ASSERT_TRUE(Owner.addExport(NominalID, *Nominal));
    ASSERT_TRUE(Owner.addExport(IntegerID, *Integer));

    const Module &ReadOnly = Owner;
    expectExport(ReadOnly, FunctionID, {Callable});
    expectExport(ReadOnly, GlobalID, {Variable});
    expectExport(ReadOnly, NominalID, {Nominal});
    expectExport(ReadOnly, IntegerID, {Integer});
    ASSERT_EQ(ReadOnly.exports().size(), 4U);
    EXPECT_EQ(ReadOnly.findExport(FunctionID), &ReadOnly.exports()[0]);
    EXPECT_EQ(ReadOnly.findExport(GlobalID), &ReadOnly.exports()[1]);
    EXPECT_EQ(ReadOnly.findExport(NominalID), &ReadOnly.exports()[2]);
    EXPECT_EQ(ReadOnly.findExport(IntegerID), &ReadOnly.exports()[3]);
  }

  // Function names and linkage never implicitly publish an export, and explicit registration leaves those properties unchanged.
  TEST(IRModuleExportTest, UsesExplicitRegistrationWithoutChangingFunctionProperties)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const SymbolId SourceName = symbol("IRModuleExportTest.SourceName");
    const SymbolId ExportID = symbol("IRModuleExportTest.CanonicalID");
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()), SourceName.SymbolName, SymbolLinkage::Export, LanguageLinkage::C, FunctionVisibility::Private);
    ASSERT_NE(Callable, nullptr);
    EXPECT_EQ(Owner.findExport(SourceName), nullptr);
    EXPECT_TRUE(Owner.exports().empty());

    ASSERT_TRUE(Owner.addExport(ExportID, *Callable));
    EXPECT_EQ(Owner.findExport(SourceName), nullptr);
    expectExport(Owner, ExportID, {Callable});
    EXPECT_EQ(Callable->name(), SourceName.SymbolName);
    EXPECT_EQ(Callable->symbolLinkage(), SymbolLinkage::Export);
    EXPECT_EQ(Callable->languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(Callable->visibility(), FunctionVisibility::Private);

    Callable->setName(core::NamePool::instance().intern("IRModuleExportTest.Renamed"));
    expectExport(Owner, ExportID, {Callable});
  }

  // Invalid identities fail explicitly without adding records or changing an existing group's entries.
  TEST(IRModuleExportTest, RejectsInvalidSymbolWithoutChangingExports)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Boolean = Context.typePool().get<ink::ir::BoolType>();
    const SymbolId ValidID = symbol("IRModuleExportTest.Valid");
    ASSERT_TRUE(Owner.addExport(ValidID, *Integer));
    ASSERT_TRUE(Owner.addExport(ValidID, *Boolean));

    const auto Result = Owner.addExport({}, *Integer);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error(), ModuleError::InvalidSymbolID);
    EXPECT_EQ(Owner.exports().size(), 1U);
    expectExport(Owner, ValidID, {Integer, Boolean});
    EXPECT_EQ(Owner.findExport({}), nullptr);
    EXPECT_EQ(Owner.findExport(symbol("IRModuleExportTest.Missing")), nullptr);
  }

  // Foreign declarations and non-declaration values cannot create a group or alter the entries in an existing group.
  TEST(IRModuleExportTest, RejectsForeignAndNonDeclarationValues)
  {
    IRContext Context(Target);
    IRContext ForeignContext(Target);
    Module Owner(Context);
    Module Other(Context);
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Integer, {Integer}));
    auto *OtherCallable = Other.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    auto *OtherVariable = Other.createGlobalVariable(*Integer);
    auto *OtherNominal = Other.createStructType(8, 8);
    const auto *Constant = Context.constantPool().get<ink::ir::BoolConstant>(true);
    ASSERT_NE(Callable, nullptr);
    ASSERT_NE(OtherCallable, nullptr);
    ASSERT_NE(OtherVariable, nullptr);
    ASSERT_NE(OtherNominal, nullptr);
    ASSERT_NE(Callable->parameter(0), nullptr);
    ASSERT_NE(Constant, nullptr);
    const SymbolId ExistingID = symbol("IRModuleExportTest.Existing");
    const SymbolId AbsentID = symbol("IRModuleExportTest.Rejected");
    ASSERT_TRUE(Owner.addExport(ExistingID, *Integer));
    ASSERT_TRUE(Owner.addExport(ExistingID, *Callable));
    const Value *Rejected[] = {
        OtherCallable,
        OtherVariable,
        OtherNominal,
        ForeignContext.typePool().get<ink::ir::IntegerType>(32, true),
        Callable->parameter(0),
        Constant,
        &Owner,
    };
    for (const Value *Declaration : Rejected)
    {
      const auto ExistingResult = Owner.addExport(ExistingID, *Declaration);
      ASSERT_FALSE(ExistingResult);
      EXPECT_EQ(ExistingResult.error(), ModuleError::InvalidExport);
      const auto AbsentResult = Owner.addExport(AbsentID, *Declaration);
      ASSERT_FALSE(AbsentResult);
      EXPECT_EQ(AbsentResult.error(), ModuleError::InvalidExport);
      EXPECT_EQ(Owner.exports().size(), 1U);
      expectExport(Owner, ExistingID, {Integer, Callable});
      EXPECT_EQ(Owner.findExport(AbsentID), nullptr);
    }
  }

  // One exported name groups distinct function declarations in order, including declarations that share an identical signature.
  TEST(IRModuleExportTest, GroupsOverloadsWithoutPerformingSemanticUniquenessChecks)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const SymbolId ExportID = symbol("IRModuleExportTest.Overloaded");
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Byte = Context.typePool().get<ink::ir::IntegerType>(8, false);
    const auto *FirstSignature = Context.typePool().get<ink::ir::FunctionType>(Integer, {Integer});
    auto *First = Owner.createGlobalFunction(*FirstSignature, ExportID.SymbolName);
    auto *Second = Owner.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Integer, {Byte}), ExportID.SymbolName);
    auto *SameSignature = Owner.createGlobalFunction(*FirstSignature, ExportID.SymbolName);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(SameSignature, nullptr);
    ASSERT_NE(First, SameSignature);
    ASSERT_TRUE(Owner.addExport(ExportID, *First));
    ASSERT_TRUE(Owner.addExport(ExportID, *Second));
    ASSERT_TRUE(Owner.addExport(ExportID, *SameSignature));

    EXPECT_EQ(Owner.exports().size(), 1U);
    expectExport(Owner, ExportID, {First, Second, SameSignature});
    EXPECT_EQ(First->symbolLinkage(), SymbolLinkage::Local);
    EXPECT_EQ(Second->symbolLinkage(), SymbolLinkage::Local);
    EXPECT_EQ(SameSignature->symbolLinkage(), SymbolLinkage::Local);
  }

  // Re-registering any declaration already present in a multi-candidate group preserves every entry and its insertion order.
  TEST(IRModuleExportTest, RejectsDuplicateDeclarationWithoutChangingCandidateEntries)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const SymbolId ExportID = symbol("IRModuleExportTest.Duplicate");
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Boolean = Context.typePool().get<ink::ir::BoolType>();
    ASSERT_TRUE(Owner.addExport(ExportID, *Integer));
    ASSERT_TRUE(Owner.addExport(ExportID, *Boolean));

    const Value *Repeated[] = {
        Integer,
        Boolean,
    };
    for (const Value *Declaration : Repeated)
    {
      const auto Result = Owner.addExport(ExportID, *Declaration);
      ASSERT_FALSE(Result);
      EXPECT_EQ(Result.error(), ModuleError::DuplicateExport);
      EXPECT_EQ(Owner.exports().size(), 1U);
      expectExport(Owner, ExportID, {Integer, Boolean});
    }
  }

  // Separate exported names may refer to the same declaration, and adding another alias candidate leaves the original group unchanged.
  TEST(IRModuleExportTest, AllowsAliasesToShareDeclarations)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const SymbolId OriginalID = symbol("IRModuleExportTest.Original");
    const SymbolId AliasID = symbol("IRModuleExportTest.Alias");
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Boolean = Context.typePool().get<ink::ir::BoolType>();
    ASSERT_TRUE(Owner.addExport(OriginalID, *Integer));
    ASSERT_TRUE(Owner.addExport(AliasID, *Integer));
    ASSERT_TRUE(Owner.addExport(AliasID, *Boolean));
    EXPECT_EQ(Owner.exports().size(), 2U);
    expectExport(Owner, OriginalID, {Integer});
    expectExport(Owner, AliasID, {Integer, Boolean});
  }
} // namespace ink::ir::test
