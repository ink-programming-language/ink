#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(!std::is_copy_constructible_v<Module>);
  static_assert(!std::is_copy_assignable_v<Module>);
  static_assert(!std::is_move_constructible_v<Module>);
  static_assert(!std::is_move_assignable_v<Module>);
  static_assert(std::is_base_of_v<Value, Module>);
  static_assert(std::is_same_v<decltype(std::declval<Module &>().moduleID()), const ModuleID &>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().moduleID()), const ModuleID &>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().classes()), const std::vector<std::unique_ptr<ClassType>> &>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().interfaces()), const std::vector<std::unique_ptr<InterfaceType>> &>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().structs()), const std::vector<std::unique_ptr<StructType>> &>);

  // Modules share Value identity without becoming typed operands, other value categories or owned declarations of themselves.
  TEST(IRModuleTest, ExposesUntypedValueIdentityWithoutChangingOwnership)
  {
    IRContext Context(Target);
    Module Owner(Context);
    Module Other(Context);
    const Value &ModuleValue = Owner;
    EXPECT_EQ(ModuleValue.kind(), ValueKind::Module);
    EXPECT_EQ(valueKindName(ModuleValue.kind()), "module");
    EXPECT_EQ(ModuleValue.type(), nullptr);
    EXPECT_TRUE(Module::classof(&ModuleValue));
    EXPECT_FALSE(Module::classof(nullptr));
    EXPECT_FALSE(Module::classof(Owner.typePool().getInt32Type()));
    EXPECT_FALSE(Type::classof(&ModuleValue));
    EXPECT_FALSE(Constant::classof(&ModuleValue));
    EXPECT_FALSE(Instruction::classof(&ModuleValue));
    EXPECT_FALSE(Function::classof(&ModuleValue));
    EXPECT_FALSE(BasicBlock::classof(&ModuleValue));
    EXPECT_FALSE(Owner.owns(&ModuleValue));
    EXPECT_FALSE(Owner.contains(&ModuleValue));
    EXPECT_FALSE(Owner.owns(&Other));
    EXPECT_FALSE(Owner.contains(&Other));

    const auto *Signature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    auto *Callable = Owner.createGlobalFunction(*Signature);
    ASSERT_NE(Callable, nullptr);
    EXPECT_FALSE(Module::classof(Callable));
    EXPECT_TRUE(Owner.owns(Callable));
    EXPECT_TRUE(Owner.contains(Callable));
  }

  // Modules start with empty declaration lists and either an empty identifier or the supplied root-level module identifier.
  TEST(IRModuleTest, EmptyModulesPreserveOptionalModuleID)
  {
    IRContext UnnamedContext(Target);
    Module Unnamed(UnnamedContext);
    const Module &ReadOnly = Unnamed;
    EXPECT_EQ(Unnamed.moduleID(), ModuleID{});
    EXPECT_EQ(ReadOnly.moduleID(), ModuleID{});
    EXPECT_TRUE(Unnamed.globalVariables().empty());
    EXPECT_EQ(Unnamed.globalVariableCount(), 0U);
    EXPECT_EQ(Unnamed.globalVariable(0), nullptr);
    EXPECT_EQ(ReadOnly.globalVariable(0), nullptr);
    EXPECT_TRUE(Unnamed.globalFunctions().empty());
    EXPECT_EQ(Unnamed.globalFunctionCount(), 0U);
    EXPECT_EQ(Unnamed.globalFunction(0), nullptr);
    EXPECT_EQ(ReadOnly.globalFunction(0), nullptr);
    EXPECT_TRUE(Unnamed.classes().empty());
    EXPECT_EQ(Unnamed.classCount(), 0U);
    EXPECT_EQ(ReadOnly.classType(0), nullptr);
    EXPECT_TRUE(Unnamed.interfaces().empty());
    EXPECT_EQ(Unnamed.interfaceCount(), 0U);
    EXPECT_EQ(ReadOnly.interfaceType(0), nullptr);
    EXPECT_TRUE(Unnamed.structs().empty());
    EXPECT_EQ(Unnamed.structCount(), 0U);
    EXPECT_EQ(ReadOnly.structType(0), nullptr);
    EXPECT_FALSE(Unnamed.owns(nullptr));
    EXPECT_FALSE(Unnamed.contains(nullptr));

    const core::Name ModuleName = core::NamePool::instance().intern("Empty");
    const ModuleID ID{{ModuleName}};
    IRContext NamedContext(Target);
    const Module Named(NamedContext, ID);
    EXPECT_EQ(Named.moduleID(), ID);
  }

  // Construction copies the full identifier; source changes, other modules and source destruction cannot change the owned value.
  TEST(IRModuleTest, OwnsIndependentModuleIDs)
  {
    auto &Names = core::NamePool::instance();
    const core::Name ModuleName = Names.intern("material");
    const core::Name RootName = Names.intern("game");
    const core::Name PackageName = Names.intern("render");
    const core::Name ChildName = Names.intern("detail");
    const ModuleID Expected{{RootName, PackageName, ModuleName}};
    const ModuleID ChildID{{RootName, PackageName, ChildName, ModuleName}};
    IRContext OwnerContext(Target);
    std::unique_ptr<Module> Owner;
    {
      ModuleID Source = Expected;
      Owner = std::make_unique<Module>(OwnerContext, Source);
      IRContext OtherContext(Target);
      Module Other(OtherContext, Source);
      EXPECT_EQ(Owner->moduleID(), Other.moduleID());
      Source.Components.clear();
      EXPECT_EQ(Other.moduleID(), Expected);
      EXPECT_EQ(Owner->moduleID(), Expected);
    }
    const Module &ReadOnly = *Owner;
    EXPECT_EQ(ReadOnly.moduleID(), Expected);
    ModuleID Source = ChildID;
    IRContext MovedContext(Target);
    Module Moved(MovedContext, std::move(Source));
    EXPECT_EQ(Moved.moduleID(), ChildID);
    EXPECT_NE(Moved.moduleID(), ReadOnly.moduleID());
    EXPECT_EQ(ReadOnly.moduleID(), Expected);
  }

  // Creation and ownership transfer preserve insertion order and entity addresses as both lists grow.
  TEST(IRModuleTest, OwnsCreatedAndAppendedDeclarationsAtStableAddresses)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto Signature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name VariableName = core::NamePool::instance().intern("IRModuleTest.Variable");
    const core::Name FunctionName = core::NamePool::instance().intern("IRModuleTest.Function");
    const Module &ReadOnly = Owner;
    auto *CreatedVariable = Owner.createGlobalVariable(*Owner.typePool().getBoolType(), VariableName);
    auto *CreatedFunction = Owner.createGlobalFunction(*Signature, FunctionName);
    ASSERT_NE(CreatedVariable, nullptr);
    ASSERT_NE(CreatedFunction, nullptr);
    EXPECT_EQ(CreatedVariable->name(), VariableName);
    EXPECT_EQ(CreatedVariable->type(), Owner.typePool().getBoolType());
    EXPECT_EQ(CreatedFunction->name(), FunctionName);
    EXPECT_EQ(CreatedFunction->functionType(), Signature);

    auto AppendedVariable = std::make_unique<GlobalVariable>(*Owner.typePool().getInt32Type());
    auto AppendedFunction = std::make_unique<Function>(*Signature);
    auto *VariableAddress = AppendedVariable.get();
    auto *FunctionAddress = AppendedFunction.get();
    EXPECT_EQ(Owner.appendGlobalVariable(std::move(AppendedVariable)), VariableAddress);
    EXPECT_EQ(Owner.appendGlobalFunction(std::move(AppendedFunction)), FunctionAddress);
    EXPECT_EQ(AppendedVariable, nullptr);
    EXPECT_EQ(AppendedFunction, nullptr);
    EXPECT_EQ(Owner.appendGlobalVariable(nullptr), nullptr);
    EXPECT_EQ(Owner.appendGlobalFunction(nullptr), nullptr);
    EXPECT_EQ(Owner.globalVariableCount(), 2U);
    EXPECT_EQ(Owner.globalFunctionCount(), 2U);

    for (std::size_t Index = 0; Index < 64; ++Index)
    {
      ASSERT_NE(Owner.createGlobalVariable(*Owner.typePool().getBoolType()), nullptr);
      ASSERT_NE(Owner.createGlobalFunction(*Signature), nullptr);
    }

    EXPECT_EQ(Owner.globalVariableCount(), 66U);
    EXPECT_EQ(Owner.globalFunctionCount(), 66U);
    EXPECT_EQ(Owner.globalVariables()[0].get(), CreatedVariable);
    EXPECT_EQ(Owner.globalVariables()[1].get(), VariableAddress);
    EXPECT_EQ(Owner.globalFunctions()[0].get(), CreatedFunction);
    EXPECT_EQ(Owner.globalFunctions()[1].get(), FunctionAddress);
    EXPECT_EQ(Owner.globalVariable(0), CreatedVariable);
    EXPECT_EQ(ReadOnly.globalVariable(1), VariableAddress);
    EXPECT_EQ(Owner.globalFunction(0), CreatedFunction);
    EXPECT_EQ(ReadOnly.globalFunction(1), FunctionAddress);
    EXPECT_TRUE(Owner.owns(CreatedVariable));
    EXPECT_TRUE(Owner.owns(VariableAddress));
    EXPECT_TRUE(Owner.owns(CreatedFunction));
    EXPECT_TRUE(Owner.owns(FunctionAddress));
    EXPECT_TRUE(Owner.contains(CreatedVariable));
    EXPECT_TRUE(Owner.contains(VariableAddress));
    EXPECT_TRUE(Owner.contains(CreatedFunction));
    EXPECT_TRUE(Owner.contains(FunctionAddress));
    for (const std::size_t Index : {std::size_t{66}, std::numeric_limits<std::size_t>::max()})
    {
      EXPECT_EQ(Owner.globalVariable(Index), nullptr);
      EXPECT_EQ(ReadOnly.globalVariable(Index), nullptr);
      EXPECT_EQ(Owner.globalFunction(Index), nullptr);
      EXPECT_EQ(ReadOnly.globalFunction(Index), nullptr);
    }
  }

  // Function creation forwards signature, name, symbol linkage, language linkage and visibility.
  TEST(IRModuleTest, FunctionCreationPreservesExplicitMetadata)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto Signature = Owner.typePool().getFunctionType(Owner.typePool().getInt32Type(), {Owner.typePool().getBoolType()});
    ASSERT_NE(Signature, nullptr);
    const core::Name FunctionName = core::NamePool::instance().intern("IRModuleTest.ForeignFunction");
    auto *Callable = Owner.createGlobalFunction(*Signature, FunctionName, SymbolLinkage::Import, LanguageLinkage::C, FunctionVisibility::Private);
    ASSERT_NE(Callable, nullptr);
    EXPECT_EQ(Callable->functionType(), Signature);
    EXPECT_EQ(Callable->name(), FunctionName);
    EXPECT_EQ(Callable->symbolLinkage(), SymbolLinkage::Import);
    EXPECT_EQ(Callable->languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(Callable->visibility(), FunctionVisibility::Private);
    EXPECT_EQ(Callable->parameterCount(), 1U);
    EXPECT_TRUE(Callable->isDeclaration());
  }

  // Optional lifecycle entries independently select, replace and clear owned declarations before their bodies exist.
  TEST(IRModuleTest, LifecycleEntriesIndependentlySelectOwnedFunctions)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto Signature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    const Module &ReadOnly = Owner;
    EXPECT_EQ(Owner.initializationFunction(), nullptr);
    EXPECT_EQ(Owner.deinitializationFunction(), nullptr);
    EXPECT_EQ(ReadOnly.initializationFunction(), nullptr);
    EXPECT_EQ(ReadOnly.deinitializationFunction(), nullptr);
    EXPECT_TRUE(Owner.setInitializationFunction(nullptr));
    EXPECT_TRUE(Owner.setDeinitializationFunction(nullptr));

    auto *First = Owner.createGlobalFunction(*Signature);
    auto *Second = Owner.appendGlobalFunction(std::make_unique<Function>(*Signature, core::NAME_None, SymbolLinkage::Import, LanguageLinkage::C, FunctionVisibility::Private));
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_TRUE(First->isDeclaration());
    ASSERT_TRUE(Second->isDeclaration());
    EXPECT_TRUE(Owner.setInitializationFunction(First));
    EXPECT_TRUE(Owner.setDeinitializationFunction(First));
    EXPECT_EQ(ReadOnly.initializationFunction(), First);
    EXPECT_EQ(ReadOnly.deinitializationFunction(), First);
    EXPECT_TRUE(Owner.setInitializationFunction(Second));
    EXPECT_EQ(Owner.initializationFunction(), Second);
    EXPECT_EQ(Owner.deinitializationFunction(), First);
    EXPECT_TRUE(Owner.setDeinitializationFunction(Second));
    EXPECT_TRUE(Owner.setInitializationFunction(nullptr));
    EXPECT_EQ(Owner.initializationFunction(), nullptr);
    EXPECT_EQ(Owner.deinitializationFunction(), Second);
    EXPECT_TRUE(Owner.setInitializationFunction(First));
    EXPECT_TRUE(Owner.setDeinitializationFunction(nullptr));
    EXPECT_EQ(Owner.initializationFunction(), First);
    EXPECT_EQ(Owner.deinitializationFunction(), nullptr);
    EXPECT_EQ(Owner.globalFunctionCount(), 2U);
    ASSERT_NE(First->createBlock(), nullptr);
    EXPECT_FALSE(Owner.initializationFunction()->isDeclaration());
  }

  // Foreign, detached, nominal member and incompatible functions cannot replace an already selected lifecycle entry.
  TEST(IRModuleTest, LifecycleEntriesRejectNonGlobalAndIncompatibleFunctions)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto Signature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType());
    const auto ReturningSignature = Owner.typePool().getFunctionType(Owner.typePool().getInt32Type());
    const auto ParameterSignature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType(), {Owner.typePool().getBoolType()});
    const auto EmptyStruct = Owner.createStructType(0, 1);
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ReturningSignature, nullptr);
    ASSERT_NE(ParameterSignature, nullptr);
    ASSERT_NE(EmptyStruct, nullptr);
    const auto EmptyStructSignature = Owner.typePool().getFunctionType(EmptyStruct);
    ASSERT_NE(EmptyStructSignature, nullptr);
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    auto *Selected = Owner.createGlobalFunction(*Signature);
    auto *Foreign = Other.createGlobalFunction(*Other.typePool().getFunctionType(Other.typePool().getVoidType()));
    auto *Returning = Owner.createGlobalFunction(*ReturningSignature);
    auto *Parameterized = Owner.createGlobalFunction(*ParameterSignature);
    auto *ReturningEmptyStruct = Owner.createGlobalFunction(*EmptyStructSignature);
    ASSERT_NE(Selected, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_NE(Returning, nullptr);
    ASSERT_NE(Parameterized, nullptr);
    ASSERT_NE(ReturningEmptyStruct, nullptr);
    Function Detached(*Signature);
    const auto ClassValue = Owner.createClassType(8, 4);
    ASSERT_NE(ClassValue, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("IRModuleTest.LifecycleMember");
    auto Member = std::make_unique<Function>(*Signature, MemberName);
    auto *MemberAddress = Member.get();
    ASSERT_TRUE(ClassValue->addMethod(Method(std::move(Member))));
    ASSERT_TRUE(Owner.setInitializationFunction(Selected));
    ASSERT_TRUE(Owner.setDeinitializationFunction(Selected));
    Function *Rejected[] = {
        Foreign,
        &Detached,
        MemberAddress,
        Returning,
        Parameterized,
        ReturningEmptyStruct,
    };
    for (Function *Candidate : Rejected)
    {
      EXPECT_FALSE(Owner.setInitializationFunction(Candidate));
      EXPECT_FALSE(Owner.setDeinitializationFunction(Candidate));
      EXPECT_EQ(Owner.initializationFunction(), Selected);
      EXPECT_EQ(Owner.deinitializationFunction(), Selected);
    }
  }

  // Nominal creation immediately owns distinct identities and preserves insertion order and addresses as lists grow.
  TEST(IRModuleTest, OwnsCreatedNominalTypesAtStableAddresses)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto FirstClass = Owner.createClassType(16, 8);
    const auto FirstInterface = Owner.createInterfaceType(16, 8);
    const auto FirstStruct = Owner.createStructType(8, 4);
    const auto SecondClass = Owner.createClassType(16, 8);
    const auto SecondInterface = Owner.createInterfaceType(16, 8);
    const auto SecondStruct = Owner.createStructType(8, 4);
    ASSERT_NE(FirstClass, nullptr);
    ASSERT_NE(FirstInterface, nullptr);
    ASSERT_NE(FirstStruct, nullptr);
    ASSERT_NE(SecondClass, nullptr);
    ASSERT_NE(SecondInterface, nullptr);
    ASSERT_NE(SecondStruct, nullptr);
    const Module &ReadOnly = Owner;
    EXPECT_EQ(Owner.classCount(), 2U);
    EXPECT_EQ(Owner.interfaceCount(), 2U);
    EXPECT_EQ(Owner.structCount(), 2U);

    for (std::size_t Index = 0; Index < 64; ++Index)
    {
      const auto NewClass = Owner.createClassType(16, 8);
      const auto NewInterface = Owner.createInterfaceType(16, 8);
      const auto NewStruct = Owner.createStructType(8, 4);
      ASSERT_NE(NewClass, nullptr);
      ASSERT_NE(NewInterface, nullptr);
      ASSERT_NE(NewStruct, nullptr);
    }

    EXPECT_EQ(Owner.classCount(), 66U);
    EXPECT_EQ(Owner.interfaceCount(), 66U);
    EXPECT_EQ(Owner.structCount(), 66U);
    EXPECT_EQ(Owner.classes()[0].get(), FirstClass);
    EXPECT_EQ(Owner.classes()[1].get(), SecondClass);
    EXPECT_EQ(Owner.interfaces()[0].get(), FirstInterface);
    EXPECT_EQ(Owner.interfaces()[1].get(), SecondInterface);
    EXPECT_EQ(Owner.structs()[0].get(), FirstStruct);
    EXPECT_EQ(Owner.structs()[1].get(), SecondStruct);
    EXPECT_EQ(ReadOnly.classType(0), FirstClass);
    EXPECT_EQ(ReadOnly.classType(1), SecondClass);
    EXPECT_EQ(ReadOnly.interfaceType(0), FirstInterface);
    EXPECT_EQ(ReadOnly.interfaceType(1), SecondInterface);
    EXPECT_EQ(ReadOnly.structType(0), FirstStruct);
    EXPECT_EQ(ReadOnly.structType(1), SecondStruct);
    const Value *Created[] = {
        FirstClass,
        SecondClass,
        FirstInterface,
        SecondInterface,
        FirstStruct,
        SecondStruct,
    };
    for (const Value *Candidate : Created)
    {
      EXPECT_TRUE(Owner.owns(Candidate));
      EXPECT_TRUE(Owner.contains(Candidate));
    }
    for (const std::size_t Index : {std::size_t{66}, std::numeric_limits<std::size_t>::max()})
    {
      EXPECT_EQ(Owner.classType(Index), nullptr);
      EXPECT_EQ(ReadOnly.classType(Index), nullptr);
      EXPECT_EQ(Owner.interfaceType(Index), nullptr);
      EXPECT_EQ(ReadOnly.interfaceType(Index), nullptr);
      EXPECT_EQ(Owner.structType(Index), nullptr);
      EXPECT_EQ(ReadOnly.structType(Index), nullptr);
    }
  }

  // Invalid nominal layouts fail without inserting declarations or disturbing previously created identities.
  TEST(IRModuleTest, RejectedNominalCreationPreservesOwnedLists)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto *Class = Owner.createClassType(8, 8);
    const auto *Interface = Owner.createInterfaceType(8, 8);
    const auto *Struct = Owner.createStructType(8, 8);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Interface, nullptr);
    ASSERT_NE(Struct, nullptr);
    for (const std::uint64_t Alignment : {0U, 3U})
    {
      EXPECT_EQ(Owner.createClassType(8, Alignment), nullptr);
      EXPECT_EQ(Owner.createInterfaceType(8, Alignment), nullptr);
      EXPECT_EQ(Owner.createStructType(8, Alignment), nullptr);
      ASSERT_EQ(Owner.classCount(), 1U);
      ASSERT_EQ(Owner.interfaceCount(), 1U);
      ASSERT_EQ(Owner.structCount(), 1U);
      EXPECT_EQ(Owner.classType(0), Class);
      EXPECT_EQ(Owner.interfaceType(0), Interface);
      EXPECT_EQ(Owner.structType(0), Struct);
      EXPECT_TRUE(Owner.ownsType(Class));
      EXPECT_TRUE(Owner.ownsType(Interface));
      EXPECT_TRUE(Owner.ownsType(Struct));
    }
  }

  // Membership excludes foreign and detached functions, foreign types, constants and nested values.
  TEST(IRModuleTest, RejectsForeignDetachedAndNestedValues)
  {
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    auto &Constants = Other.constantPool();
    const auto Signature = Other.typePool().getFunctionType(Other.typePool().getVoidType(), {Other.typePool().getBoolType()});
    const auto ClassLayout = Other.createClassType(16, 8);
    const auto InterfaceLayout = Other.createInterfaceType(16, 8);
    const auto StructLayout = Other.createStructType(8, 4);
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ClassLayout, nullptr);
    ASSERT_NE(InterfaceLayout, nullptr);
    ASSERT_NE(StructLayout, nullptr);
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    const auto LocalSignature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType(), {Owner.typePool().getBoolType()});
    ASSERT_NE(LocalSignature, nullptr);
    auto *Callable = Owner.createGlobalFunction(*LocalSignature);
    auto *ForeignFunction = Other.createGlobalFunction(*Signature);
    auto *ForeignVariable = Other.createGlobalVariable(*Other.typePool().getBoolType());
    ASSERT_NE(Callable, nullptr);
    ASSERT_NE(ForeignFunction, nullptr);
    ASSERT_NE(ForeignVariable, nullptr);
    auto *ParameterValue = Callable->parameter(0);
    auto *Block = Callable->createBlock();
    ASSERT_NE(ParameterValue, nullptr);
    ASSERT_NE(Block, nullptr);
    Function Detached(*LocalSignature);
    const Value *Rejected[] = {
        ForeignFunction,
        ForeignVariable,
        &Detached,
        Signature,
        ClassLayout,
        InterfaceLayout,
        StructLayout,
        Constants.getBoolConstant(true),
        Owner.constantPool().getBoolConstant(true),
        ParameterValue,
        Block,
    };
    for (const Value *Candidate : Rejected)
    {
      ASSERT_NE(Candidate, nullptr);
      EXPECT_FALSE(Owner.owns(Candidate));
      EXPECT_FALSE(Owner.contains(Candidate));
    }
  }

  // Module-owned nominal types own their overloaded methods independently of the module's top-level function list.
  TEST(IRModuleTest, OwnedClassMembersRetainNamesAndOverloadSignatures)
  {
    const core::Name ModuleName = core::NamePool::instance().intern("math");
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext, {{ModuleName}});
    const core::Name ClassName = core::NamePool::instance().intern("Point");
    const auto ClassLayout = Owner.createClassType(8, 4, ClassName);
    ASSERT_NE(ClassLayout, nullptr);
    const auto ReceiverType = Owner.typePool().getPointerType(ClassLayout);
    ASSERT_NE(ReceiverType, nullptr);
    const auto IntegerSignature = Owner.typePool().getFunctionType(Owner.typePool().getInt32Type(), {ReceiverType, Owner.typePool().getInt32Type()});
    const auto BooleanSignature = Owner.typePool().getFunctionType(Owner.typePool().getInt32Type(), {ReceiverType, Owner.typePool().getBoolType()});
    ASSERT_NE(IntegerSignature, nullptr);
    ASSERT_NE(BooleanSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    auto IntegerCallable = std::make_unique<Function>(*IntegerSignature, MemberName);
    auto BooleanCallable = std::make_unique<Function>(*BooleanSignature, MemberName);
    const auto *IntegerAddress = IntegerCallable.get();
    const auto *BooleanAddress = BooleanCallable.get();
    ASSERT_TRUE(ClassLayout->addMethod(Method(std::move(IntegerCallable))));
    ASSERT_TRUE(ClassLayout->addMethod(Method(std::move(BooleanCallable))));
    EXPECT_EQ(IntegerCallable, nullptr);
    EXPECT_EQ(BooleanCallable, nullptr);

    const Module &ReadOnly = Owner;
    const auto *OwnedClass = ReadOnly.classType(0);
    ASSERT_NE(OwnedClass, nullptr);
    EXPECT_EQ(OwnedClass->name(), ClassName);
    EXPECT_EQ(OwnedClass, ClassLayout);
    const auto Overloads = OwnedClass->findMethods(MemberName);
    ASSERT_EQ(Overloads.size(), 2U);
    EXPECT_EQ(Overloads[0]->name(), MemberName);
    EXPECT_EQ(Overloads[1]->name(), MemberName);
    EXPECT_EQ(Overloads[0]->function(), IntegerAddress);
    EXPECT_EQ(Overloads[1]->function(), BooleanAddress);
    EXPECT_FALSE(ReadOnly.owns(Overloads[0]->function()));
    EXPECT_FALSE(ReadOnly.owns(Overloads[1]->function()));
    EXPECT_FALSE(ReadOnly.contains(Overloads[0]->function()));
    EXPECT_FALSE(ReadOnly.contains(Overloads[1]->function()));
    EXPECT_EQ(ReadOnly.globalFunctionCount(), 0U);
    EXPECT_EQ(Overloads[0]->functionType(), IntegerSignature);
    EXPECT_EQ(Overloads[1]->functionType(), BooleanSignature);
    ASSERT_EQ(OwnedClass->methods().size(), 2U);
    EXPECT_EQ(OwnedClass->methods()[0].name(), MemberName);
    EXPECT_EQ(OwnedClass->methods()[0].functionType(), IntegerSignature);
    EXPECT_EQ(OwnedClass->methods()[1].name(), MemberName);
    EXPECT_EQ(OwnedClass->methods()[1].functionType(), BooleanSignature);
    EXPECT_TRUE(OwnedClass->findMethods(core::NAME_None).empty());
  }
} // namespace ink::ir::test
