#include "ink/core/name_pool.h"
#include "ink/ir/constant/constant_context.h"
#include "ink/ir/function/method.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class ModuleBodyInstruction final : public Instruction
    {
      public:
        explicit ModuleBodyInstruction(bool &Destroyed) noexcept
            : Destroyed(Destroyed)
        {
        }

        ~ModuleBodyInstruction() noexcept override
        {
          Destroyed = true;
        }

      private:
        bool &Destroyed;
    };
  } // namespace

  static_assert(!std::is_copy_constructible_v<Module>);
  static_assert(!std::is_copy_assignable_v<Module>);
  static_assert(!std::is_move_constructible_v<Module>);
  static_assert(!std::is_move_assignable_v<Module>);

  // Named and unnamed modules begin with empty declaration lists and reject every indexed access.
  TEST(IRModuleTest, EmptyModulesPreserveOptionalCanonicalName)
  {
    Module Unnamed;
    const Module &ReadOnly = Unnamed;
    EXPECT_EQ(Unnamed.name(), core::NAME_None);
    EXPECT_FALSE(Unnamed.hasName());
    EXPECT_TRUE(Unnamed.globalVariables().empty());
    EXPECT_EQ(Unnamed.globalVariableCount(), 0U);
    EXPECT_EQ(Unnamed.globalVariable(0), nullptr);
    EXPECT_EQ(ReadOnly.globalVariable(0), nullptr);
    EXPECT_TRUE(Unnamed.functions().empty());
    EXPECT_EQ(Unnamed.functionCount(), 0U);
    EXPECT_EQ(Unnamed.function(0), nullptr);
    EXPECT_EQ(ReadOnly.function(0), nullptr);
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

    const core::Name ModuleName = core::NamePool::instance().intern("IRModuleTest.Empty");
    const Module Named(ModuleName);
    EXPECT_TRUE(Named.hasName());
    EXPECT_EQ(Named.name(), ModuleName);
  }

  // Creation and ownership transfer preserve insertion order and entity addresses as both lists grow.
  TEST(IRModuleTest, OwnsCreatedAndAppendedDeclarationsAtStableAddresses)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name VariableName = core::NamePool::instance().intern("IRModuleTest.Variable");
    const core::Name FunctionName = core::NamePool::instance().intern("IRModuleTest.Function");
    Module Owner;
    const Module &ReadOnly = Owner;
    auto *CreatedVariable = Owner.createGlobalVariable(*Types.getBoolType(), VariableName);
    auto *CreatedFunction = Owner.createFunction(*Signature, FunctionName);
    ASSERT_NE(CreatedVariable, nullptr);
    ASSERT_NE(CreatedFunction, nullptr);
    EXPECT_EQ(CreatedVariable->name(), VariableName);
    EXPECT_EQ(CreatedVariable->type(), Types.getBoolType());
    EXPECT_EQ(CreatedFunction->name(), FunctionName);
    EXPECT_EQ(CreatedFunction->functionType(), Signature);

    auto AppendedVariable = std::make_unique<GlobalVariable>(*Types.getInt32Type());
    auto AppendedFunction = std::make_unique<Function>(*Signature);
    auto *VariableAddress = AppendedVariable.get();
    auto *FunctionAddress = AppendedFunction.get();
    EXPECT_EQ(Owner.appendGlobalVariable(std::move(AppendedVariable)), VariableAddress);
    EXPECT_EQ(Owner.appendFunction(std::move(AppendedFunction)), FunctionAddress);
    EXPECT_EQ(AppendedVariable, nullptr);
    EXPECT_EQ(AppendedFunction, nullptr);
    EXPECT_EQ(Owner.appendGlobalVariable(nullptr), nullptr);
    EXPECT_EQ(Owner.appendFunction(nullptr), nullptr);
    EXPECT_EQ(Owner.globalVariableCount(), 2U);
    EXPECT_EQ(Owner.functionCount(), 2U);

    for (std::size_t Index = 0; Index < 64; ++Index)
    {
      ASSERT_NE(Owner.createGlobalVariable(*Types.getBoolType()), nullptr);
      ASSERT_NE(Owner.createFunction(*Signature), nullptr);
    }

    EXPECT_EQ(Owner.globalVariableCount(), 66U);
    EXPECT_EQ(Owner.functionCount(), 66U);
    EXPECT_EQ(Owner.globalVariables()[0].get(), CreatedVariable);
    EXPECT_EQ(Owner.globalVariables()[1].get(), VariableAddress);
    EXPECT_EQ(Owner.functions()[0].get(), CreatedFunction);
    EXPECT_EQ(Owner.functions()[1].get(), FunctionAddress);
    EXPECT_EQ(Owner.globalVariable(0), CreatedVariable);
    EXPECT_EQ(ReadOnly.globalVariable(1), VariableAddress);
    EXPECT_EQ(Owner.function(0), CreatedFunction);
    EXPECT_EQ(ReadOnly.function(1), FunctionAddress);
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
      EXPECT_EQ(Owner.function(Index), nullptr);
      EXPECT_EQ(ReadOnly.function(Index), nullptr);
    }
  }

  // Function creation forwards signature, name, symbol linkage, language linkage and visibility.
  TEST(IRModuleTest, FunctionCreationPreservesExplicitMetadata)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getInt32Type(), {Types.getBoolType()});
    ASSERT_NE(Signature, nullptr);
    const core::Name FunctionName = core::NamePool::instance().intern("IRModuleTest.ForeignFunction");
    Module Owner;
    auto *Callable = Owner.createFunction(*Signature, FunctionName, SymbolLinkage::Import, LanguageLinkage::C, FunctionVisibility::Private);
    ASSERT_NE(Callable, nullptr);
    EXPECT_EQ(Callable->functionType(), Signature);
    EXPECT_EQ(Callable->name(), FunctionName);
    EXPECT_EQ(Callable->symbolLinkage(), SymbolLinkage::Import);
    EXPECT_EQ(Callable->languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(Callable->visibility(), FunctionVisibility::Private);
    EXPECT_EQ(Callable->parameterCount(), 1U);
    EXPECT_TRUE(Callable->isDeclaration());
  }

  // Destroying a module releases the body instructions of both created and transferred functions.
  TEST(IRModuleTest, DestructionReleasesOwnedFunctionBodies)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    bool CreatedBodyDestroyed = false;
    bool TransferredBodyDestroyed = false;
    {
      Module Owner;
      auto *Created = Owner.createFunction(*Signature);
      ASSERT_NE(Created, nullptr);
      auto *CreatedBlock = Created->createBlock();
      ASSERT_NE(CreatedBlock, nullptr);
      ASSERT_NE(CreatedBlock->appendInstruction(std::make_unique<ModuleBodyInstruction>(CreatedBodyDestroyed)), nullptr);

      auto Transferred = std::make_unique<Function>(*Signature);
      auto *TransferredBlock = Transferred->createBlock();
      ASSERT_NE(TransferredBlock, nullptr);
      ASSERT_NE(TransferredBlock->appendInstruction(std::make_unique<ModuleBodyInstruction>(TransferredBodyDestroyed)), nullptr);
      ASSERT_NE(Owner.appendFunction(std::move(Transferred)), nullptr);
      EXPECT_FALSE(CreatedBodyDestroyed);
      EXPECT_FALSE(TransferredBodyDestroyed);
    }
    EXPECT_TRUE(CreatedBodyDestroyed);
    EXPECT_TRUE(TransferredBodyDestroyed);
  }

  // Nominal type registration borrows distinct identities, rejects duplicates and preserves insertion order as lists grow.
  TEST(IRModuleTest, RegistersBorrowedNominalTypesAtStableAddresses)
  {
    TypeContext Types(Target);
    const auto FirstClass = Types.createClassType(16, 8);
    const auto FirstInterface = Types.createInterfaceType(16, 8);
    const auto FirstStruct = Types.createStructType(8, 4);
    const auto SecondClass = Types.createClassType(16, 8);
    const auto SecondInterface = Types.createInterfaceType(16, 8);
    const auto SecondStruct = Types.createStructType(8, 4);
    ASSERT_NE(FirstClass, nullptr);
    ASSERT_NE(FirstInterface, nullptr);
    ASSERT_NE(FirstStruct, nullptr);
    ASSERT_NE(SecondClass, nullptr);
    ASSERT_NE(SecondInterface, nullptr);
    ASSERT_NE(SecondStruct, nullptr);
    Module Owner;
    const Module &ReadOnly = Owner;
    ASSERT_TRUE(Owner.addClass(*FirstClass));
    ASSERT_TRUE(Owner.addInterface(*FirstInterface));
    ASSERT_TRUE(Owner.addStruct(*FirstStruct));
    ASSERT_TRUE(Owner.addClass(*SecondClass));
    ASSERT_TRUE(Owner.addInterface(*SecondInterface));
    ASSERT_TRUE(Owner.addStruct(*SecondStruct));
    EXPECT_FALSE(Owner.addClass(*FirstClass));
    EXPECT_FALSE(Owner.addInterface(*FirstInterface));
    EXPECT_FALSE(Owner.addStruct(*FirstStruct));
    EXPECT_EQ(Owner.classCount(), 2U);
    EXPECT_EQ(Owner.interfaceCount(), 2U);
    EXPECT_EQ(Owner.structCount(), 2U);

    for (std::size_t Index = 0; Index < 64; ++Index)
    {
      const auto NewClass = Types.createClassType(16, 8);
      const auto NewInterface = Types.createInterfaceType(16, 8);
      const auto NewStruct = Types.createStructType(8, 4);
      ASSERT_NE(NewClass, nullptr);
      ASSERT_NE(NewInterface, nullptr);
      ASSERT_NE(NewStruct, nullptr);
      ASSERT_TRUE(Owner.addClass(*NewClass));
      ASSERT_TRUE(Owner.addInterface(*NewInterface));
      ASSERT_TRUE(Owner.addStruct(*NewStruct));
    }

    EXPECT_EQ(Owner.classCount(), 66U);
    EXPECT_EQ(Owner.interfaceCount(), 66U);
    EXPECT_EQ(Owner.structCount(), 66U);
    EXPECT_EQ(Owner.classes()[0], FirstClass);
    EXPECT_EQ(Owner.classes()[1], SecondClass);
    EXPECT_EQ(Owner.interfaces()[0], FirstInterface);
    EXPECT_EQ(Owner.interfaces()[1], SecondInterface);
    EXPECT_EQ(Owner.structs()[0], FirstStruct);
    EXPECT_EQ(Owner.structs()[1], SecondStruct);
    EXPECT_EQ(ReadOnly.classType(0), FirstClass);
    EXPECT_EQ(ReadOnly.classType(1), SecondClass);
    EXPECT_EQ(ReadOnly.interfaceType(0), FirstInterface);
    EXPECT_EQ(ReadOnly.interfaceType(1), SecondInterface);
    EXPECT_EQ(ReadOnly.structType(0), FirstStruct);
    EXPECT_EQ(ReadOnly.structType(1), SecondStruct);
    const Value *Registered[] = {
        FirstClass,
        SecondClass,
        FirstInterface,
        SecondInterface,
        FirstStruct,
        SecondStruct,
    };
    for (const Value *Candidate : Registered)
    {
      EXPECT_FALSE(Owner.owns(Candidate));
      EXPECT_TRUE(Owner.contains(Candidate));
    }
    for (const std::size_t Index : {std::size_t{66}, std::numeric_limits<std::size_t>::max()})
    {
      EXPECT_EQ(ReadOnly.classType(Index), nullptr);
      EXPECT_EQ(ReadOnly.interfaceType(Index), nullptr);
      EXPECT_EQ(ReadOnly.structType(Index), nullptr);
    }
  }

  // Membership excludes foreign and detached functions, unregistered types, constants and nested values.
  TEST(IRModuleTest, RejectsUnregisteredValues)
  {
    TypeContext Types(Target);
    ConstantContext Constants(Types);
    const auto Signature = Types.getFunctionType(Types.getVoidType(), {Types.getBoolType()});
    const auto ClassLayout = Types.createClassType(16, 8);
    const auto InterfaceLayout = Types.createInterfaceType(16, 8);
    const auto StructLayout = Types.createStructType(8, 4);
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ClassLayout, nullptr);
    ASSERT_NE(InterfaceLayout, nullptr);
    ASSERT_NE(StructLayout, nullptr);
    Module Owner;
    Module Other;
    auto *Callable = Owner.createFunction(*Signature);
    auto *ForeignFunction = Other.createFunction(*Signature);
    auto *ForeignVariable = Other.createGlobalVariable(*Types.getBoolType());
    ASSERT_NE(Callable, nullptr);
    ASSERT_NE(ForeignFunction, nullptr);
    ASSERT_NE(ForeignVariable, nullptr);
    ASSERT_TRUE(Other.addClass(*ClassLayout));
    ASSERT_TRUE(Other.addInterface(*InterfaceLayout));
    ASSERT_TRUE(Other.addStruct(*StructLayout));
    auto *ParameterValue = Callable->parameter(0);
    auto *Block = Callable->createBlock();
    ASSERT_NE(ParameterValue, nullptr);
    ASSERT_NE(Block, nullptr);
    Function Detached(*Signature);
    const Value *Rejected[] = {
        ForeignFunction,
        ForeignVariable,
        &Detached,
        Signature,
        ClassLayout,
        InterfaceLayout,
        StructLayout,
        Constants.getBoolConstant(true),
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

  // Registered nominal types preserve source member names and distinct overload signatures.
  TEST(IRModuleTest, RegisteredClassMembersRetainNamesAndOverloadSignatures)
  {
    TypeContext Types(Target);
    const core::Name ClassName = core::NamePool::instance().intern("Point");
    const auto ClassLayout = Types.createClassType(8, 4, ClassName);
    ASSERT_NE(ClassLayout, nullptr);
    const auto ReceiverType = Types.getPointerType(ClassLayout);
    ASSERT_NE(ReceiverType, nullptr);
    const auto IntegerSignature = Types.getFunctionType(Types.getInt32Type(), {ReceiverType, Types.getInt32Type()});
    const auto BooleanSignature = Types.getFunctionType(Types.getInt32Type(), {ReceiverType, Types.getBoolType()});
    ASSERT_NE(IntegerSignature, nullptr);
    ASSERT_NE(BooleanSignature, nullptr);
    const core::Name ModuleName = core::NamePool::instance().intern("IRModuleTest.math");
    const core::Name MemberName = core::NamePool::instance().intern("method");
    Module Owner(ModuleName);
    ASSERT_TRUE(Owner.addClass(*ClassLayout));
    ASSERT_TRUE(ClassLayout->addMethod(Method(MemberName, *IntegerSignature)));
    ASSERT_TRUE(ClassLayout->addMethod(Method(MemberName, *BooleanSignature)));

    const Module &ReadOnly = Owner;
    const auto *ImportedClass = ReadOnly.classType(0);
    ASSERT_NE(ImportedClass, nullptr);
    EXPECT_EQ(ImportedClass->name(), ClassName);
    EXPECT_EQ(ImportedClass, ClassLayout);
    const auto Overloads = ImportedClass->findMethods(MemberName);
    ASSERT_EQ(Overloads.size(), 2U);
    EXPECT_EQ(Overloads[0]->name(), MemberName);
    EXPECT_EQ(Overloads[1]->name(), MemberName);
    EXPECT_EQ(Overloads[0]->functionType(), IntegerSignature);
    EXPECT_EQ(Overloads[1]->functionType(), BooleanSignature);
    ASSERT_EQ(ImportedClass->methods().size(), 2U);
    EXPECT_EQ(ImportedClass->methods()[0].name(), MemberName);
    EXPECT_EQ(ImportedClass->methods()[0].functionType(), IntegerSignature);
    EXPECT_EQ(ImportedClass->methods()[1].name(), MemberName);
    EXPECT_EQ(ImportedClass->methods()[1].functionType(), BooleanSignature);
    EXPECT_TRUE(ImportedClass->findMethods(core::NAME_None).empty());
  }

  // Types and their method metadata remain usable after a registering module is destroyed.
  TEST(IRModuleTest, DestroyingModulePreservesBorrowedNominalTypes)
  {
    TypeContext Types(Target);
    const auto ClassValue = Types.createClassType(8, 4);
    const auto InterfaceValue = Types.createInterfaceType(8, 4);
    const auto StructValue = Types.createStructType(8, 4);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(ClassValue, nullptr);
    ASSERT_NE(InterfaceValue, nullptr);
    ASSERT_NE(StructValue, nullptr);
    ASSERT_NE(Signature, nullptr);
    const core::Name ModuleName = core::NamePool::instance().intern("IRModuleTest.Lifetime");
    const core::Name MemberName = core::NamePool::instance().intern("execute");
    {
      Module Owner(ModuleName);
      ASSERT_TRUE(Owner.addClass(*ClassValue));
      ASSERT_TRUE(Owner.addInterface(*InterfaceValue));
      ASSERT_TRUE(Owner.addStruct(*StructValue));
      ASSERT_TRUE(ClassValue->addMethod(Method(MemberName, *Signature)));
    }
    EXPECT_TRUE(Types.owns(ClassValue));
    EXPECT_TRUE(Types.owns(InterfaceValue));
    EXPECT_TRUE(Types.owns(StructValue));
    EXPECT_EQ(ClassValue->size(), 8U);
    EXPECT_EQ(InterfaceValue->size(), 8U);
    EXPECT_EQ(StructValue->size(), 8U);
    const auto Methods = ClassValue->findMethods(MemberName);
    ASSERT_EQ(Methods.size(), 1U);
    EXPECT_EQ(Methods[0]->name(), MemberName);
    EXPECT_EQ(Methods[0]->functionType(), Signature);
  }
} // namespace ink::ir::test
