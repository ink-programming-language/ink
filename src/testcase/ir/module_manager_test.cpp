#include "ink/core/name/name_pool.h"
#include "ink/ir/context.h"
#include "ink/ir/module/module.h"
#include "ink/ir/module/module_manager.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    ModuleID moduleID(std::initializer_list<std::string_view> Components)
    {
      ModuleID Result;
      for (const auto Component : Components)
      {
        Result.Components.push_back(core::NamePool::instance().intern(Component));
      }
      return Result;
    }
  } // namespace

  static_assert(!std::is_default_constructible_v<ModuleManager>);
  static_assert(!std::is_copy_constructible_v<ModuleManager>);
  static_assert(!std::is_copy_assignable_v<ModuleManager>);
  static_assert(!std::is_move_constructible_v<ModuleManager>);
  static_assert(!std::is_move_assignable_v<ModuleManager>);
  static_assert(!std::is_constructible_v<ModuleManager, IRContext &&>);
  static_assert(std::is_same_v<decltype(std::declval<ModuleManager &>().irContext()), IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<const ModuleManager &>().irContext()), const IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<ModuleManager &>().findModule(std::declval<const ModuleID &>())), Module *>);
  static_assert(std::is_same_v<decltype(std::declval<const ModuleManager &>().findModule(std::declval<const ModuleID &>())), const Module *>);
  static_assert(std::is_same_v<decltype(std::declval<const ModuleManager &>().resolveSymbol(std::declval<const SymbolRef &>())), std::expected<const SymbolExport *, ModuleError>>);

  // Empty managers borrow their context and return null for absent or invalid module IDs.
  TEST(IRModuleManagerTest, EmptyManagerExposesBorrowedContextAndNullableQueries)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    const ModuleManager &ReadOnly = Manager;
    EXPECT_EQ(&Manager.irContext(), &Context);
    EXPECT_EQ(&ReadOnly.irContext(), &Context);
    EXPECT_EQ(Manager.moduleCount(), 0U);
    EXPECT_EQ(Manager.findModule(moduleID({"missing"})), nullptr);
    EXPECT_EQ(ReadOnly.findModule(moduleID({"missing"})), nullptr);
    EXPECT_EQ(Manager.findModule({}), nullptr);
  }

  // Creation shares one context and compares independently constructed module IDs by value.
  TEST(IRModuleManagerTest, CreatesNamedModulesWithSharedContextAndValueLookup)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto First = Manager.createModule(moduleID({"package", "first"}));
    auto Second = Manager.createModule(moduleID({"package", "second"}));
    ASSERT_TRUE(First.has_value());
    ASSERT_TRUE(Second.has_value());
    const ModuleManager &ReadOnly = Manager;
    EXPECT_EQ(Manager.moduleCount(), 2U);
    EXPECT_NE(*First, *Second);
    EXPECT_EQ(&(*First)->irContext(), &Context);
    EXPECT_EQ(&(*Second)->typePool(), &Context.typePool());
    EXPECT_EQ(Manager.findModule(moduleID({"package", "first"})), *First);
    EXPECT_EQ(ReadOnly.findModule(moduleID({"package", "second"})), *Second);
    EXPECT_EQ(Manager.findModule(moduleID({"first", "package"})), nullptr);
    EXPECT_EQ(Manager.findModule(moduleID({"first"})), nullptr);
  }

  // Duplicate creation never replaces the registered module or its existing declarations.
  TEST(IRModuleManagerTest, RejectsDuplicateCreationWithoutReplacingExistingModule)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Original = Manager.createModule(moduleID({"package", "module"}));
    ASSERT_TRUE(Original.has_value());
    auto *Variable = (*Original)->createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    auto Duplicate = Manager.createModule(moduleID({"package", "module"}));
    ASSERT_FALSE(Duplicate.has_value());
    EXPECT_EQ(Duplicate.error(), ModuleError::DuplicateModule);
    EXPECT_EQ(Manager.moduleCount(), 1U);
    EXPECT_EQ(Manager.findModule(moduleID({"package", "module"})), *Original);
    EXPECT_EQ((*Original)->globalVariable(0), Variable);
  }

  // Empty IDs and invalid name components are rejected without consuming a module slot.
  TEST(IRModuleManagerTest, RejectsInvalidModuleIDsWithoutChangingTheCollection)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Empty = Manager.createModule({});
    ASSERT_FALSE(Empty.has_value());
    EXPECT_EQ(Empty.error(), ModuleError::InvalidModuleID);
    auto InvalidID = moduleID({"package", "module"});
    InvalidID.Components[1] = core::NAME_None;
    auto Invalid = Manager.createModule(InvalidID);
    ASSERT_FALSE(Invalid.has_value());
    EXPECT_EQ(Invalid.error(), ModuleError::InvalidModuleID);
    EXPECT_EQ(Manager.findModule(InvalidID), nullptr);
    EXPECT_EQ(Manager.moduleCount(), 0U);
    auto Valid = Manager.createModule(moduleID({"package", "module"}));
    ASSERT_TRUE(Valid.has_value());
    EXPECT_EQ(Manager.findModule(moduleID({"package", "module"})), *Valid);
  }

  // Adoption transfers the original object and declarations instead of cloning or rebuilding them.
  TEST(IRModuleManagerTest, AdoptsExistingModuleAndPreservesObjectIdentity)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Owned = std::make_unique<Module>(Context, moduleID({"existing"}));
    Module *Original = Owned.get();
    auto *Function = Owned->createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto Adopted = Manager.adoptModule(Owned);
    ASSERT_TRUE(Adopted.has_value());
    EXPECT_EQ(Owned, nullptr);
    EXPECT_EQ(*Adopted, Original);
    EXPECT_EQ(Manager.findModule(moduleID({"existing"})), Original);
    EXPECT_EQ(Original->globalFunction(0), Function);
    EXPECT_EQ(Manager.moduleCount(), 1U);
  }

  // Null adoption and invalid module identity report separate errors while retaining caller ownership on failure.
  TEST(IRModuleManagerTest, RejectsNullAndUnnamedAdoptionWithoutConsumingOwnership)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    std::unique_ptr<Module> Empty;
    auto Null = Manager.adoptModule(Empty);
    ASSERT_FALSE(Null.has_value());
    EXPECT_EQ(Null.error(), ModuleError::InvalidModule);
    EXPECT_EQ(Empty, nullptr);
    auto Owned = std::make_unique<Module>(Context);
    Module *Original = Owned.get();
    auto Unnamed = Manager.adoptModule(Owned);
    ASSERT_FALSE(Unnamed.has_value());
    EXPECT_EQ(Unnamed.error(), ModuleError::InvalidModuleID);
    EXPECT_EQ(Owned.get(), Original);
    EXPECT_EQ(Manager.moduleCount(), 0U);
    EXPECT_EQ(Owned->moduleID(), ModuleID{});
  }

  // Failed duplicate adoption retains both objects and permits transfer to another manager sharing the same context.
  TEST(IRModuleManagerTest, DuplicateAdoptionPreservesOwnershipForAnotherManager)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Existing = Manager.createModule(moduleID({"duplicate"}));
    ASSERT_TRUE(Existing.has_value());
    auto Owned = std::make_unique<Module>(Context, moduleID({"duplicate"}));
    Module *Original = Owned.get();
    auto Duplicate = Manager.adoptModule(Owned);
    ASSERT_FALSE(Duplicate.has_value());
    EXPECT_EQ(Duplicate.error(), ModuleError::DuplicateModule);
    EXPECT_EQ(Owned.get(), Original);
    EXPECT_EQ(Manager.findModule(moduleID({"duplicate"})), *Existing);
    EXPECT_EQ(Manager.moduleCount(), 1U);
    ModuleManager OtherManager(Context);
    auto Accepted = OtherManager.adoptModule(Owned);
    ASSERT_TRUE(Accepted.has_value());
    EXPECT_EQ(*Accepted, Original);
    EXPECT_EQ(Owned, nullptr);
    EXPECT_EQ(OtherManager.findModule(moduleID({"duplicate"})), Original);
    EXPECT_EQ(Manager.findModule(moduleID({"duplicate"})), *Existing);
  }

  // Matching target layouts do not permit moving a module between different IR contexts.
  TEST(IRModuleManagerTest, RejectsForeignContextAndPreservesModuleForItsOwnManager)
  {
    IRContext Context(Target);
    IRContext OtherContext(Target);
    ModuleManager Manager(Context);
    ModuleManager OtherManager(OtherContext);
    auto Owned = std::make_unique<Module>(OtherContext, moduleID({"foreign"}));
    Module *Original = Owned.get();
    auto Rejected = Manager.adoptModule(Owned);
    ASSERT_FALSE(Rejected.has_value());
    EXPECT_EQ(Rejected.error(), ModuleError::ContextMismatch);
    EXPECT_EQ(Owned.get(), Original);
    EXPECT_EQ(Manager.moduleCount(), 0U);
    auto Accepted = OtherManager.adoptModule(Owned);
    ASSERT_TRUE(Accepted.has_value());
    EXPECT_EQ(*Accepted, Original);
    EXPECT_EQ(Owned, nullptr);
    EXPECT_EQ(OtherManager.findModule(moduleID({"foreign"})), Original);
  }

  // Creation and adoption retain independent ID values when the caller changes the source identifiers.
  TEST(IRModuleManagerTest, RegisteredIDsRemainIndependentOfCallerValues)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto CreatedID = moduleID({"created"});
    auto AdoptedID = moduleID({"adopted"});
    auto Created = Manager.createModule(CreatedID);
    ASSERT_TRUE(Created.has_value());
    auto Owned = std::make_unique<Module>(Context, AdoptedID);
    auto Adopted = Manager.adoptModule(Owned);
    ASSERT_TRUE(Adopted.has_value());
    CreatedID.Components.clear();
    AdoptedID.Components[0] = core::NamePool::instance().intern("replacement");
    EXPECT_EQ((*Created)->moduleID(), moduleID({"created"}));
    EXPECT_EQ((*Adopted)->moduleID(), moduleID({"adopted"}));
    EXPECT_EQ(Manager.findModule(moduleID({"created"})), *Created);
    EXPECT_EQ(Manager.findModule(moduleID({"adopted"})), *Adopted);
    EXPECT_EQ(Manager.findModule(moduleID({"replacement"})), nullptr);
  }

  // Adding many modules preserves every previously returned pointer and its ID lookup.
  TEST(IRModuleManagerTest, CollectionGrowthPreservesModulePointersAndLookup)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto First = Manager.createModule(moduleID({"first"}));
    ASSERT_TRUE(First.has_value());
    Module *Original = *First;
    std::array<Module *, 128> AddedModules{};
    for (std::size_t Index = 0; Index < AddedModules.size(); ++Index)
    {
      const std::string Name = "module" + std::to_string(Index);
      auto Added = Manager.createModule(moduleID({"growth", Name}));
      ASSERT_TRUE(Added.has_value());
      AddedModules[Index] = *Added;
      EXPECT_EQ(Manager.findModule(moduleID({"growth", Name})), *Added);
      EXPECT_EQ(Manager.findModule(moduleID({"first"})), Original);
    }
    for (std::size_t Index = 0; Index < AddedModules.size(); ++Index)
    {
      const std::string Name = "module" + std::to_string(Index);
      EXPECT_EQ(Manager.findModule(moduleID({"growth", Name})), AddedModules[Index]);
    }
    EXPECT_EQ(Manager.moduleCount(), AddedModules.size() + 1);
  }

  // Resolving invalid references distinguishes malformed IDs, missing modules and missing exports.
  TEST(IRModuleManagerTest, ResolutionReportsInvalidAndMissingReferenceComponents)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    const SymbolId Symbol{core::NamePool::instance().intern("function")};
    auto EmptyModule = Manager.resolveSymbol({{}, Symbol});
    ASSERT_FALSE(EmptyModule.has_value());
    EXPECT_EQ(EmptyModule.error(), ModuleError::InvalidModuleID);
    auto InvalidID = moduleID({"invalid"});
    InvalidID.Components.push_back(core::NAME_None);
    auto InvalidModule = Manager.resolveSymbol({InvalidID, Symbol});
    ASSERT_FALSE(InvalidModule.has_value());
    EXPECT_EQ(InvalidModule.error(), ModuleError::InvalidModuleID);
    auto InvalidSymbol = Manager.resolveSymbol({moduleID({"missing"}), {core::NAME_None}});
    ASSERT_FALSE(InvalidSymbol.has_value());
    EXPECT_EQ(InvalidSymbol.error(), ModuleError::InvalidSymbolID);
    auto MissingModule = Manager.resolveSymbol({moduleID({"missing"}), Symbol});
    ASSERT_FALSE(MissingModule.has_value());
    EXPECT_EQ(MissingModule.error(), ModuleError::ModuleNotFound);
    auto Created = Manager.createModule(moduleID({"present"}));
    ASSERT_TRUE(Created.has_value());
    auto MissingExport = Manager.resolveSymbol({moduleID({"present"}), Symbol});
    ASSERT_FALSE(MissingExport.has_value());
    EXPECT_EQ(MissingExport.error(), ModuleError::SymbolNotFound);
  }

  // Resolution uses explicit exports rather than scanning function names, visibility or linkage flags.
  TEST(IRModuleManagerTest, NamedFunctionsRemainUnresolvedUntilExplicitlyExported)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Created = Manager.createModule(moduleID({"owner"}));
    ASSERT_TRUE(Created.has_value());
    const SymbolId PublicSymbol{core::NamePool::instance().intern("publicFunction")};
    const SymbolId PrivateSymbol{core::NamePool::instance().intern("privateFunction")};
    auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType());
    ASSERT_NE((*Created)->createGlobalFunction(*Signature, PublicSymbol.SymbolName, SymbolLinkage::Export), nullptr);
    ASSERT_NE((*Created)->createGlobalFunction(*Signature, PrivateSymbol.SymbolName, SymbolLinkage::Local, LanguageLinkage::Ink, FunctionVisibility::Private), nullptr);
    auto Public = Manager.resolveSymbol({moduleID({"owner"}), PublicSymbol});
    auto Private = Manager.resolveSymbol({moduleID({"owner"}), PrivateSymbol});
    ASSERT_FALSE(Public.has_value());
    ASSERT_FALSE(Private.has_value());
    EXPECT_EQ(Public.error(), ModuleError::SymbolNotFound);
    EXPECT_EQ(Private.error(), ModuleError::SymbolNotFound);
  }

  // Explicit exports added after registration immediately become resolvable without stale manager caches.
  TEST(IRModuleManagerTest, ResolvesExportsAddedAfterRegistration)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Created = Manager.createModule(moduleID({"owner"}));
    ASSERT_TRUE(Created.has_value());
    const SymbolId Symbol{core::NamePool::instance().intern("api")};
    const SymbolRef Reference{moduleID({"owner"}), Symbol};
    auto Missing = Manager.resolveSymbol(Reference);
    ASSERT_FALSE(Missing.has_value());
    EXPECT_EQ(Missing.error(), ModuleError::SymbolNotFound);
    auto *Function = (*Created)->createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE((*Created)->addExport(Symbol, *Function).has_value());
    const ModuleManager &ReadOnly = Manager;
    auto Resolved = ReadOnly.resolveSymbol(Reference);
    ASSERT_TRUE(Resolved.has_value());
    EXPECT_EQ((*Resolved)->Symbol.SymbolName, Symbol.SymbolName);
    ASSERT_EQ((*Resolved)->Entries.size(), 1U);
    EXPECT_EQ((*Resolved)->Entries[0].Declaration, Function);
    EXPECT_EQ((*Resolved)->Entries[0].SymbolType, Function->type());
  }

  // A registered export name resolves every overload in insertion order as additional signatures are exported.
  TEST(IRModuleManagerTest, ResolvesOverloadGroupUpdatedAfterRegistration)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Created = Manager.createModule(moduleID({"overloads"}));
    ASSERT_TRUE(Created.has_value());
    const SymbolId Symbol{core::NamePool::instance().intern("convert")};
    const SymbolRef Reference{moduleID({"overloads"}), Symbol};
    auto *Integer = Context.typePool().getInt32Type();
    auto *Float = Context.typePool().getFloat32Type();
    auto *IntegerSignature = Context.typePool().getFunctionType(Integer, {Integer});
    auto *FloatSignature = Context.typePool().getFunctionType(Float, {Float});
    auto *IntegerFunction = (*Created)->createGlobalFunction(*IntegerSignature, Symbol.SymbolName);
    auto *FloatFunction = (*Created)->createGlobalFunction(*FloatSignature, Symbol.SymbolName);
    ASSERT_NE(IntegerFunction, nullptr);
    ASSERT_NE(FloatFunction, nullptr);
    ASSERT_TRUE((*Created)->addExport(Symbol, *IntegerFunction).has_value());
    auto Initial = Manager.resolveSymbol(Reference);
    ASSERT_TRUE(Initial.has_value());
    ASSERT_EQ((*Initial)->Entries.size(), 1U);
    EXPECT_EQ((*Initial)->Entries[0].Declaration, IntegerFunction);
    ASSERT_TRUE((*Created)->addExport(Symbol, *FloatFunction).has_value());
    auto Expanded = Manager.resolveSymbol(Reference);
    ASSERT_TRUE(Expanded.has_value());
    EXPECT_EQ((*Expanded)->Symbol.SymbolName, Symbol.SymbolName);
    ASSERT_EQ((*Expanded)->Entries.size(), 2U);
    EXPECT_EQ((*Expanded)->Entries[0].Declaration, IntegerFunction);
    EXPECT_EQ((*Expanded)->Entries[1].Declaration, FloatFunction);
    EXPECT_EQ((*Expanded)->Entries[0].SymbolType, IntegerSignature);
    EXPECT_EQ((*Expanded)->Entries[1].SymbolType, FloatSignature);
    const SymbolId OtherSymbol{core::NamePool::instance().intern("integerConvert")};
    ASSERT_TRUE((*Created)->addExport(OtherSymbol, *IntegerFunction).has_value());
    auto Refreshed = Manager.resolveSymbol(Reference);
    ASSERT_TRUE(Refreshed.has_value());
    ASSERT_EQ((*Refreshed)->Entries.size(), 2U);
    EXPECT_EQ((*Refreshed)->Entries[0].Declaration, IntegerFunction);
    EXPECT_EQ((*Refreshed)->Entries[1].Declaration, FloatFunction);
    EXPECT_EQ((*Refreshed)->Entries[0].SymbolType, IntegerSignature);
    EXPECT_EQ((*Refreshed)->Entries[1].SymbolType, FloatSignature);
  }

  // Adopting a module retains all overload candidates and their corresponding signatures in the existing export group.
  TEST(IRModuleManagerTest, AdoptionPreservesEveryOverloadCandidate)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    auto Owned = std::make_unique<Module>(Context, moduleID({"adoptedOverloads"}));
    const SymbolId Symbol{core::NamePool::instance().intern("process")};
    auto *Integer = Context.typePool().getInt32Type();
    auto *Float = Context.typePool().getFloat64Type();
    auto *IntegerSignature = Context.typePool().getFunctionType(Context.typePool().getVoidType(), {Integer});
    auto *FloatSignature = Context.typePool().getFunctionType(Context.typePool().getVoidType(), {Float});
    auto *IntegerFunction = Owned->createGlobalFunction(*IntegerSignature, Symbol.SymbolName);
    auto *FloatFunction = Owned->createGlobalFunction(*FloatSignature, Symbol.SymbolName);
    ASSERT_NE(IntegerFunction, nullptr);
    ASSERT_NE(FloatFunction, nullptr);
    ASSERT_TRUE(Owned->addExport(Symbol, *IntegerFunction).has_value());
    ASSERT_TRUE(Owned->addExport(Symbol, *FloatFunction).has_value());
    auto Adopted = Manager.adoptModule(Owned);
    ASSERT_TRUE(Adopted.has_value());
    EXPECT_EQ(Owned, nullptr);
    auto Resolved = Manager.resolveSymbol({moduleID({"adoptedOverloads"}), Symbol});
    ASSERT_TRUE(Resolved.has_value());
    ASSERT_EQ((*Resolved)->Entries.size(), 2U);
    EXPECT_EQ((*Resolved)->Entries[0].Declaration, IntegerFunction);
    EXPECT_EQ((*Resolved)->Entries[1].Declaration, FloatFunction);
    EXPECT_EQ((*Resolved)->Entries[0].SymbolType, IntegerSignature);
    EXPECT_EQ((*Resolved)->Entries[1].SymbolType, FloatSignature);
  }

  // Identical symbol names in different modules resolve to their own declarations, including preexisting adopted exports.
  TEST(IRModuleManagerTest, QualifiedResolutionSeparatesModulesAndPreservesAdoptedExports)
  {
    IRContext Context(Target);
    ModuleManager Manager(Context);
    const SymbolId Symbol{core::NamePool::instance().intern("shared")};
    auto First = Manager.createModule(moduleID({"first"}));
    ASSERT_TRUE(First.has_value());
    auto *Function = (*First)->createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE((*First)->addExport(Symbol, *Function).has_value());
    auto Owned = std::make_unique<Module>(Context, moduleID({"second"}));
    auto *Variable = Owned->createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    ASSERT_TRUE(Owned->addExport(Symbol, *Variable).has_value());
    auto Second = Manager.adoptModule(Owned);
    ASSERT_TRUE(Second.has_value());
    auto FirstSymbol = Manager.resolveSymbol({moduleID({"first"}), Symbol});
    auto SecondSymbol = Manager.resolveSymbol({moduleID({"second"}), Symbol});
    ASSERT_TRUE(FirstSymbol.has_value());
    ASSERT_TRUE(SecondSymbol.has_value());
    ASSERT_EQ((*FirstSymbol)->Entries.size(), 1U);
    ASSERT_EQ((*SecondSymbol)->Entries.size(), 1U);
    EXPECT_EQ((*FirstSymbol)->Entries[0].Declaration, Function);
    EXPECT_EQ((*FirstSymbol)->Entries[0].SymbolType, Function->type());
    EXPECT_EQ((*SecondSymbol)->Entries[0].Declaration, Variable);
    EXPECT_EQ((*SecondSymbol)->Entries[0].SymbolType, Variable->type());
  }

  // Manager destruction releases every created and adopted module and unregisters their nominal types before the context dies.
  TEST(IRModuleManagerTest, DestructionReleasesOwnedNominalTypesAndAllowsNewSession)
  {
    IRContext Context(Target);
    std::array<const Type *, 4> ReleasedTypes{};
    {
      ModuleManager Manager(Context);
      auto FirstCreated = Manager.createModule(moduleID({"firstOwner"}));
      auto SecondCreated = Manager.createModule(moduleID({"secondOwner"}));
      ASSERT_TRUE(FirstCreated.has_value());
      ASSERT_TRUE(SecondCreated.has_value());
      ReleasedTypes[0] = (*FirstCreated)->createStructType(4, 4);
      ReleasedTypes[1] = (*SecondCreated)->createClassType(8, 8);
      auto FirstOwned = std::make_unique<Module>(Context, moduleID({"firstAdopted"}));
      auto SecondOwned = std::make_unique<Module>(Context, moduleID({"secondAdopted"}));
      ReleasedTypes[2] = FirstOwned->createInterfaceType(8, 8);
      ReleasedTypes[3] = SecondOwned->createStructType(16, 8);
      ASSERT_TRUE(Manager.adoptModule(FirstOwned).has_value());
      ASSERT_TRUE(Manager.adoptModule(SecondOwned).has_value());
      EXPECT_EQ(FirstOwned, nullptr);
      EXPECT_EQ(SecondOwned, nullptr);
      EXPECT_EQ(Manager.moduleCount(), 4U);
      for (const Type *TypeValue : ReleasedTypes)
      {
        ASSERT_NE(TypeValue, nullptr);
        EXPECT_TRUE(Context.containsType(TypeValue));
      }
    }
    for (const Type *TypeValue : ReleasedTypes)
    {
      EXPECT_FALSE(Context.containsType(TypeValue));
    }
    ModuleManager Next(Context);
    auto Recreated = Next.createModule(moduleID({"firstOwner"}));
    ASSERT_TRUE(Recreated.has_value());
    EXPECT_EQ(Next.moduleCount(), 1U);
  }
} // namespace ink::ir::test
