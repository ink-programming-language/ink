#include "ink/core/name/name_pool.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/function/parameter.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_nothrow_constructible_v<Module>);
  static_assert(!std::is_copy_constructible_v<Module> && !std::is_copy_assignable_v<Module>);
  static_assert(!std::is_move_constructible_v<Module> && !std::is_move_assignable_v<Module>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<Module &>().functions())>, Function &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const Module &>().functions())>, const Function &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<Module &>().globalVariables())>, GlobalVariable &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const Module &>().globalVariables())>, const GlobalVariable &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<Module &>().structTypes())>, StructType &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const Module &>().structTypes())>, const StructType &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<Module &>().objectTypes())>, ObjectType &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const Module &>().objectTypes())>, const ObjectType &>);
  static_assert(std::ranges::bidirectional_range<decltype(std::declval<Module &>().structTypes())>);
  static_assert(std::ranges::bidirectional_range<decltype(std::declval<const Module &>().objectTypes())>);
  static_assert(std::is_same_v<decltype(std::declval<Function &>().parent()), Module *>);
  static_assert(std::is_same_v<decltype(std::declval<const Function &>().parent()), const Module *>);

  template <typename ResultType>
  concept ModuleAcceptsGlobalResult = requires(Module &Value, ResultType &&TypeValue)
  {
    Value.createGlobalVariable(std::forward<ResultType>(TypeValue));
  };

  template <typename ResultType>
  concept ModuleAcceptsModifiedGlobalResult = requires(Module &Value, ResultType &&TypeValue)
  {
    Value.createGlobalVariable(std::forward<ResultType>(TypeValue), core::NAME_None, SymbolVisibility::Public, NativeLinkage::Export);
  };

  template <typename SignatureType>
  concept ModuleAcceptsFunctionSignature = requires(Module &Value, SignatureType &&Signature)
  {
    Value.createFunction(std::forward<SignatureType>(Signature));
  };

  template <typename Record>
  concept ModuleAcceptsStructRegistration = requires(Module &Value, Record &&TypeValue)
  {
    Value.addStructType(std::forward<Record>(TypeValue));
  };

  template <typename Record>
  concept ModuleAcceptsObjectRegistration = requires(Module &Value, Record &&TypeValue)
  {
    Value.addObjectType(std::forward<Record>(TypeValue));
  };

  static_assert(ModuleAcceptsGlobalResult<PointerType &> && ModuleAcceptsGlobalResult<const PointerType &>);
  static_assert(!ModuleAcceptsGlobalResult<PointerType> && !ModuleAcceptsGlobalResult<const PointerType>);
  static_assert(!ModuleAcceptsModifiedGlobalResult<PointerType> && !ModuleAcceptsModifiedGlobalResult<const PointerType>);
  static_assert(!ModuleAcceptsGlobalResult<IntegerType &>);
  static_assert(ModuleAcceptsFunctionSignature<const FunctionType &>);
  static_assert(!ModuleAcceptsFunctionSignature<FunctionType> && !ModuleAcceptsFunctionSignature<const FunctionType>);
  static_assert(ModuleAcceptsStructRegistration<StructType &> && !ModuleAcceptsStructRegistration<StructType>);
  static_assert(ModuleAcceptsObjectRegistration<ObjectType &> && !ModuleAcceptsObjectRegistration<ObjectType>);

  // Module creation preserves order in all four collections while forwarding symbol attributes and parent links for declarations.
  TEST(IRModuleTest, OwnsOrderedDeclarationsAndForwardsTheirSymbolAttributes)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    const PointerType &GlobalType = Types.getPointerType(Types.getBoolType());
    const core::Name ModuleName = Names.intern("application");
    const core::Name SharedName = Names.intern("declaration");
    Module ModuleValue(ModuleName);
    EXPECT_EQ(ModuleValue.name(), ModuleName);
    EXPECT_EQ(ModuleValue.functionCount(), 0U);
    EXPECT_EQ(ModuleValue.globalVariableCount(), 0U);
    EXPECT_EQ(ModuleValue.structTypeCount(), 0U);
    EXPECT_EQ(ModuleValue.objectTypeCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(ModuleValue.functions()));
    EXPECT_TRUE(std::ranges::empty(ModuleValue.globalVariables()));
    EXPECT_TRUE(std::ranges::empty(ModuleValue.structTypes()));
    EXPECT_TRUE(std::ranges::empty(ModuleValue.objectTypes()));
    Function &FirstFunction = ModuleValue.createFunction(Signature);
    Function &SecondFunction = ModuleValue.createFunction(Signature, SharedName, SymbolVisibility::Public, NativeLinkage::Import);
    GlobalVariable &FirstGlobal = ModuleValue.createGlobalVariable(GlobalType);
    GlobalVariable &SecondGlobal = ModuleValue.createGlobalVariable(GlobalType, SharedName, SymbolVisibility::Public, NativeLinkage::Export);
    StructType &FirstStruct = Types.createStructType();
    StructType &SecondStruct = Types.createStructType();
    ObjectType &FirstObject = Types.createObjectType();
    ObjectType &SecondObject = Types.createObjectType();
    EXPECT_EQ(ModuleValue.structTypeCount(), 0U);
    EXPECT_EQ(ModuleValue.objectTypeCount(), 0U);
    ModuleValue.addStructType(FirstStruct);
    ModuleValue.addStructType(SecondStruct);
    ModuleValue.addObjectType(FirstObject);
    ModuleValue.addObjectType(SecondObject);
    EXPECT_EQ(FirstFunction.name(), core::NAME_None);
    EXPECT_EQ(FirstFunction.visibility(), SymbolVisibility::Private);
    EXPECT_EQ(FirstFunction.nativeLinkage(), NativeLinkage::None);
    EXPECT_EQ(SecondFunction.name(), SharedName);
    EXPECT_EQ(SecondFunction.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(SecondFunction.nativeLinkage(), NativeLinkage::Import);
    EXPECT_EQ(&SecondFunction.signature(), &Signature);
    EXPECT_EQ(FirstGlobal.name(), core::NAME_None);
    EXPECT_EQ(FirstGlobal.visibility(), SymbolVisibility::Private);
    EXPECT_EQ(FirstGlobal.nativeLinkage(), NativeLinkage::None);
    EXPECT_EQ(SecondGlobal.name(), SharedName);
    EXPECT_EQ(SecondGlobal.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(SecondGlobal.nativeLinkage(), NativeLinkage::Export);
    EXPECT_EQ(&SecondGlobal.getType(), &GlobalType);
    ASSERT_EQ(ModuleValue.functionCount(), 2U);
    ASSERT_EQ(ModuleValue.globalVariableCount(), 2U);
    ASSERT_EQ(ModuleValue.structTypeCount(), 2U);
    ASSERT_EQ(ModuleValue.objectTypeCount(), 2U);
    auto FunctionPosition = ModuleValue.functions().begin();
    EXPECT_EQ(&*FunctionPosition++, &FirstFunction);
    EXPECT_EQ(&*FunctionPosition++, &SecondFunction);
    EXPECT_EQ(FunctionPosition, ModuleValue.functions().end());
    auto GlobalPosition = ModuleValue.globalVariables().begin();
    EXPECT_EQ(&*GlobalPosition++, &FirstGlobal);
    EXPECT_EQ(&*GlobalPosition++, &SecondGlobal);
    EXPECT_EQ(GlobalPosition, ModuleValue.globalVariables().end());
    const Module &ConstModule = ModuleValue;
    auto StructPosition = ConstModule.structTypes().begin();
    EXPECT_EQ(&*StructPosition++, &FirstStruct);
    EXPECT_EQ(&*StructPosition++, &SecondStruct);
    EXPECT_EQ(StructPosition, ConstModule.structTypes().end());
    auto ObjectPosition = ConstModule.objectTypes().begin();
    EXPECT_EQ(&*ObjectPosition++, &FirstObject);
    EXPECT_EQ(&*ObjectPosition++, &SecondObject);
    EXPECT_EQ(ObjectPosition, ConstModule.objectTypes().end());
    for (const Function &FunctionValue : ConstModule.functions())
    {
      EXPECT_EQ(FunctionValue.parent(), &ConstModule);
    }
    for (const GlobalVariable &Global : ConstModule.globalVariables())
    {
      EXPECT_EQ(Global.parent(), &ConstModule);
    }
    EXPECT_EQ(FirstFunction.parent(), &ModuleValue);
    EXPECT_EQ(FirstGlobal.parent(), &ModuleValue);
  }

  // Growing type registrations and declarations preserves pooled record identities, saved iterators, and cross-collection operands.
  TEST(IRModuleTest, PreservesDeclarationAndOperandIdentityWhileAllCollectionsGrow)
  {
    TypePool Types;
    Module ModuleValue;
    EXPECT_EQ(ModuleValue.name(), core::NAME_None);
    StructType &FirstStruct = Types.createStructType();
    ObjectType &FirstObject = Types.createObjectType();
    ModuleValue.addStructType(FirstStruct);
    ModuleValue.addObjectType(FirstObject);
    const FunctionType &Signature = Types.getFunctionType(FirstStruct);
    const PointerType &GlobalType = Types.getPointerType(FirstStruct);
    FirstStruct.addField(core::NAME_None, Types.getPointerType(FirstObject));
    FirstObject.addField(core::NAME_None, GlobalType);
    Function &FirstFunction = ModuleValue.createFunction(Signature);
    GlobalVariable &FirstGlobal = ModuleValue.createGlobalVariable(GlobalType);
    auto FunctionPosition = ModuleValue.functions().begin();
    auto GlobalPosition = ModuleValue.globalVariables().begin();
    auto StructPosition = ModuleValue.structTypes().begin();
    auto ObjectPosition = ModuleValue.objectTypes().begin();
    BasicBlock &Entry = FirstFunction.createBasicBlock();
    LoadInstruction *Load = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Entry);
      Load = &Builder.createLoad(FirstGlobal);
    }
    for (unsigned Index = 0; Index < 128; ++Index)
    {
      ModuleValue.createFunction(Signature);
      ModuleValue.createGlobalVariable(GlobalType);
      ModuleValue.addStructType(Types.createStructType());
      ModuleValue.addObjectType(Types.createObjectType());
    }
    EXPECT_EQ(ModuleValue.functionCount(), 129U);
    EXPECT_EQ(ModuleValue.globalVariableCount(), 129U);
    EXPECT_EQ(ModuleValue.structTypeCount(), 129U);
    EXPECT_EQ(ModuleValue.objectTypeCount(), 129U);
    EXPECT_EQ(&*FunctionPosition, &FirstFunction);
    EXPECT_EQ(&*GlobalPosition, &FirstGlobal);
    EXPECT_EQ(&*StructPosition, &FirstStruct);
    EXPECT_EQ(&*ObjectPosition, &FirstObject);
    EXPECT_EQ(&*ModuleValue.functions().begin(), &FirstFunction);
    EXPECT_EQ(&*ModuleValue.globalVariables().begin(), &FirstGlobal);
    EXPECT_EQ(&*ModuleValue.structTypes().begin(), &FirstStruct);
    EXPECT_EQ(&*ModuleValue.objectTypes().begin(), &FirstObject);
    EXPECT_EQ(FirstFunction.parent(), &ModuleValue);
    EXPECT_EQ(FirstGlobal.parent(), &ModuleValue);
    EXPECT_EQ(Entry.parent(), &FirstFunction);
    EXPECT_EQ(Load->parent(), &Entry);
    EXPECT_EQ(&Load->getSource(), &FirstGlobal);
    EXPECT_EQ(&Load->getType(), &FirstStruct);
    EXPECT_EQ(&FirstFunction.signature().returnType(), &FirstStruct);
    EXPECT_EQ(&FirstGlobal.valueType(), &FirstStruct);
    EXPECT_EQ(&Types.getPointerType(FirstStruct), &GlobalType);
    ASSERT_EQ(FirstStruct.fields().size(), 1U);
    ASSERT_EQ(FirstObject.fields().size(), 1U);
    EXPECT_EQ(&FirstStruct.fields()[0].type(), &Types.getPointerType(FirstObject));
    EXPECT_EQ(&FirstObject.fields()[0].type(), &GlobalType);
    EXPECT_EQ(&*FirstFunction.basicBlocks().begin(), &Entry);
    ++FunctionPosition;
    ++GlobalPosition;
    EXPECT_EQ(FunctionPosition->parent(), &ModuleValue);
    EXPECT_EQ(GlobalPosition->parent(), &ModuleValue);
    ++StructPosition;
    ++ObjectPosition;
    EXPECT_NE(&*StructPosition, &FirstStruct);
    EXPECT_NE(&*ObjectPosition, &FirstObject);
    --StructPosition;
    --ObjectPosition;
    EXPECT_EQ(&*StructPosition, &FirstStruct);
    EXPECT_EQ(&*ObjectPosition, &FirstObject);
  }

  // Independent modules share borrowed canonical types while destroying one leaves the other module and the external pool usable.
  TEST(IRModuleTest, KeepsSharedTypeOwnershipOutsideIndependentModules)
  {
    TypePool Types;
    const FunctionType &Signature = Types.getFunctionType(Types.getBoolType());
    const PointerType &GlobalType = Types.getPointerType(Types.getBoolType());
    Module SurvivingModule;
    Function &SurvivingFunction = SurvivingModule.createFunction(Signature);
    GlobalVariable &SurvivingGlobal = SurvivingModule.createGlobalVariable(GlobalType);
    {
      Module TemporaryModule;
      Function &TemporaryFunction = TemporaryModule.createFunction(Signature);
      GlobalVariable &TemporaryGlobal = TemporaryModule.createGlobalVariable(GlobalType);
      EXPECT_NE(&TemporaryFunction, &SurvivingFunction);
      EXPECT_NE(&TemporaryGlobal, &SurvivingGlobal);
      EXPECT_EQ(&TemporaryFunction.signature(), &SurvivingFunction.signature());
      EXPECT_EQ(&TemporaryGlobal.getType(), &SurvivingGlobal.getType());
      EXPECT_EQ(TemporaryFunction.parent(), &TemporaryModule);
      EXPECT_EQ(TemporaryGlobal.parent(), &TemporaryModule);
    }
    EXPECT_EQ(&Types.getFunctionType(Types.getBoolType()), &Signature);
    EXPECT_EQ(&Types.getPointerType(Types.getBoolType()), &GlobalType);
    EXPECT_EQ(SurvivingFunction.parent(), &SurvivingModule);
    EXPECT_EQ(SurvivingGlobal.parent(), &SurvivingModule);
    BasicBlock &Entry = SurvivingFunction.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    LoadInstruction *Load = &Builder.createLoad(SurvivingGlobal);
    ReturnInstruction *Return = &Builder.createReturn(*Load);
    EXPECT_EQ(&Load->getSource(), &SurvivingGlobal);
    EXPECT_EQ(&Load->getType(), &Types.getBoolType());
    EXPECT_EQ(Return->getReturnValue(), Load);
  }

  // Registered pooled records can form recursive field graphs and remain exact operands for globals, signatures, and loads.
  TEST(IRModuleTest, UsesRegisteredRecordsAcrossFieldsCanonicalTypesAndFunctionBodies)
  {
    TypePool Types;
    Module ModuleValue;
    core::NamePool &Names = core::NamePool::instance();
    StructType &Payload = Types.createStructType();
    ObjectType &Owner = Types.createObjectType();
    ModuleValue.addStructType(Payload);
    ModuleValue.addObjectType(Owner);
    const PointerType &PayloadPointer = Types.getPointerType(Payload);
    const PointerType &OwnerPointer = Types.getPointerType(Owner);
    Payload.addField(Names.intern("owner"), OwnerPointer);
    Owner.addField(Names.intern("payload"), Payload);
    Owner.addField(Names.intern("next"), OwnerPointer);
    const std::array Parameters{Parameter(OwnerPointer)};
    const FunctionType &Signature = Types.getFunctionType(Payload, Parameters);
    Function &FunctionValue = ModuleValue.createFunction(Signature);
    GlobalVariable &PayloadGlobal = ModuleValue.createGlobalVariable(PayloadPointer);
    GlobalVariable &OwnerGlobal = ModuleValue.createGlobalVariable(OwnerPointer);
    Function &MethodFunction = Owner.addMethod(Signature, Names.intern("method")).function();
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    LoadInstruction *LoadedPayload = &Builder.createLoad(PayloadGlobal);
    LoadInstruction *LoadedOwner = &Builder.createLoad(OwnerGlobal);
    ReturnInstruction *Return = &Builder.createReturn(*LoadedPayload);
    ASSERT_EQ(Payload.fields().size(), 1U);
    ASSERT_EQ(Owner.fields().size(), 2U);
    EXPECT_EQ(&Payload.fields()[0].type(), &OwnerPointer);
    EXPECT_EQ(&Owner.fields()[0].type(), &Payload);
    EXPECT_EQ(&Owner.fields()[1].type(), &OwnerPointer);
    EXPECT_EQ(&Types.getPointerType(Payload), &PayloadPointer);
    EXPECT_EQ(&Types.getPointerType(Owner), &OwnerPointer);
    EXPECT_EQ(&FunctionValue.signature(), &Types.getFunctionType(Payload, Parameters));
    EXPECT_EQ(&Signature.returnType(), &Payload);
    ASSERT_EQ(Signature.parameters().size(), 1U);
    EXPECT_EQ(&Signature.parameters()[0].type(), &OwnerPointer);
    EXPECT_EQ(&PayloadGlobal.valueType(), &Payload);
    EXPECT_EQ(&OwnerGlobal.valueType(), &Owner);
    EXPECT_EQ(&LoadedPayload->getType(), &Payload);
    EXPECT_EQ(&LoadedOwner->getType(), &Owner);
    EXPECT_EQ(Return->getReturnValue(), LoadedPayload);
    ASSERT_EQ(Owner.methods().size(), 1U);
    EXPECT_EQ(&Owner.methods()[0].function(), &MethodFunction);
    EXPECT_NE(&MethodFunction, &FunctionValue);
    EXPECT_EQ(MethodFunction.parent(), nullptr);
  }

  // Destroying a registry module leaves pooled records, their owned method bodies, and derived type caches usable.
  TEST(IRModuleTest, KeepsPooledRecordsAndOwnedMethodBodiesAfterRegistryDestruction)
  {
    TypePool Types;
    StructType &Record = Types.createStructType();
    ObjectType &Object = Types.createObjectType();
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType &Signature = Types.getFunctionType(Int32);
    const PointerType &RecordPointer = Types.getPointerType(Record);
    const PointerType &ObjectPointer = Types.getPointerType(Object);
    Record.addField(core::NAME_None, ObjectPointer);
    Object.addField(core::NAME_None, RecordPointer);
    Function &Implementation = Record.addMethod(Signature, core::NAME_None, MemberKind::Static, SymbolVisibility::Public, NativeLinkage::Export).function();
    BasicBlock &Entry = Implementation.createBasicBlock();
    LoadInstruction *Load = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      Module Registry;
      Registry.addStructType(Record);
      Registry.addObjectType(Object);
      Function &FreeFunction = Registry.createFunction(Signature);
      Registry.createGlobalVariable(RecordPointer);
      EXPECT_NE(&Implementation, &FreeFunction);
      EXPECT_EQ(Implementation.parent(), nullptr);
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Entry);
      AllocaInstruction *Slot = &Builder.createAlloca(Int32);
      Load = &Builder.createLoad(*Slot);
      Return = &Builder.createReturn(*Load);
    }
    EXPECT_EQ(&Types.getPointerType(Record), &RecordPointer);
    EXPECT_EQ(&Types.getPointerType(Object), &ObjectPointer);
    EXPECT_EQ(&Types.getFunctionType(Int32), &Signature);
    ASSERT_EQ(Record.methods().size(), 1U);
    EXPECT_EQ(&Record.methods()[0].function(), &Implementation);
    EXPECT_EQ(Implementation.parent(), nullptr);
    EXPECT_EQ(Implementation.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(Implementation.nativeLinkage(), NativeLinkage::Export);
    EXPECT_EQ(&Implementation.signature(), &Signature);
    EXPECT_EQ(Return->getReturnValue(), Load);
    EXPECT_EQ(Entry.parent(), &Implementation);
    InstructionBuilder Builder(Types);
    ASSERT_TRUE(Builder.setInsertPoint(*Return));
    AddInstruction *Sum = &Builder.createAdd(*Load, *Load);
    EXPECT_EQ(&Sum->getLHS(), Load);
    EXPECT_EQ(&Sum->getRHS(), Load);
    EXPECT_EQ(Entry.instructionCount(), 4U);
    Module AnotherRegistry;
    AnotherRegistry.addStructType(Record);
    AnotherRegistry.addObjectType(Object);
    EXPECT_EQ(&*AnotherRegistry.structTypes().begin(), &Record);
    EXPECT_EQ(&*AnotherRegistry.objectTypes().begin(), &Object);
  }
} // namespace ink::ir::test
