#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    class UnknownTypeValue final : public Value
    {
      public:
        explicit UnknownTypeValue(ValueRef ValueType) noexcept
            : Value(ValueType)
        {
        }
    };
  } // namespace

  // Empty modules, module-owned declarations and globals, and complete record method bodies all pass verification.
  TEST(IRModuleVerifierTest, AcceptsDeclarationsGlobalsAndCompleteMethodBodies)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    Module Empty;
    verifyModule(Empty, Engine);
    Module ModuleValue;
    StructType Record;
    ObjectType Object;
    ModuleValue.addStructType(Record);
    ModuleValue.addObjectType(Object);
    static_cast<void>(ModuleValue.createFunction(Types.getFunctionType(Types.getVoidType())));
    static_cast<void>(ModuleValue.createGlobalVariable(Types.getPointerType(Types.getBoolType())));
    Method &RecordMethod = Record.addMethod(Types.getFunctionType(Types.getVoidType()));
    Method &ObjectMethod = Object.addMethod(Types.getFunctionType(Types.getVoidType()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(RecordMethod.function().createBasicBlock());
    static_cast<void>(Builder.createReturn());
    Builder.setInsertPoint(ObjectMethod.function().createBasicBlock());
    static_cast<void>(Builder.createReturn());
    verifyModule(ModuleValue, Engine);
    EXPECT_EQ(ModuleValue.functionCount(), 1U);
  }

  // Recursive pointer graphs terminate type traversal, while object references and static fields do not create instance layout cycles.
  TEST(IRModuleVerifierTest, AcceptsPointerRecursionObjectReferencesAndStaticSelfFields)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    StructType First;
    StructType Second;
    ObjectType Object;
    First.addField(core::NAME_None, Types.getPointerType(Second));
    Second.addField(core::NAME_None, Types.getPointerType(First));
    First.addField(core::NAME_None, First, MemberKind::Static);
    Object.addField(core::NAME_None, Object);
    Module ModuleValue;
    ModuleValue.addStructType(First);
    ModuleValue.addStructType(Second);
    ModuleValue.addObjectType(Object);
    verifyModule(ModuleValue, Engine);
    EXPECT_EQ(ModuleValue.structTypeCount(), 2U);
  }

  // Type metadata can borrow an external type-valued argument without treating it as a foreign runtime data operand.
  TEST(IRModuleVerifierTest, AcceptsBorrowedGenericArgumentsInTypeMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    Function TypeParameters(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    const Argument &T = TypeParameters.argument(0);
    StructType Record;
    Record.addField(core::NAME_None, T);
    Record.addField(core::NAME_None, Types.getArrayType(T, 4));
    const FunctionType Signature(T, std::array{Parameter(T)});
    Module ModuleValue;
    ModuleValue.addStructType(Record);
    static_cast<void>(ModuleValue.createFunction(Signature));
    static_cast<void>(ModuleValue.createGlobalVariable(Types.getPointerType(T)));
    verifyModule(ModuleValue, Engine);
    EXPECT_EQ(ModuleValue.globalVariableCount(), 1U);
  }

  // Registered struct methods are verified recursively even though their function objects are not in the module function list.
  TEST(IRModuleVerifierDeathTest, ReportsIncompleteStructMethodBodies)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    StructType Record;
    Method &MethodValue = Record.addMethod(Types.getFunctionType(Types.getVoidType()));
    static_cast<void>(MethodValue.function().createBasicBlock());
    Module ModuleValue;
    ModuleValue.addStructType(Record);
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001.*basic block is missing a terminator");
  }

  // Registered object methods cannot hide an incomplete body behind reference-semantics record metadata.
  TEST(IRModuleVerifierDeathTest, ReportsIncompleteObjectMethodBodies)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    ObjectType Object;
    Method &MethodValue = Object.addMethod(Types.getFunctionType(Types.getVoidType()));
    static_cast<void>(MethodValue.function().createBasicBlock());
    Module ModuleValue;
    ModuleValue.addObjectType(Object);
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001.*basic block is missing a terminator");
  }

  // Moving a method out of a registered record leaves a safely detectable empty slot that verification reports as an ICE.
  TEST(IRModuleVerifierDeathTest, ReportsMovedFromMethodsWithoutDereferencingAnEmptyFunction)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    StructType Record;
    Method &Stored = Record.addMethod(Types.getFunctionType(Types.getVoidType()));
    Method Moved(std::move(Stored));
    EXPECT_FALSE(Stored.hasFunction());
    EXPECT_TRUE(Moved.hasFunction());
    Module ModuleValue;
    ModuleValue.addStructType(Record);
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // A runtime integer constant cannot occupy a record field's type metadata position.
  TEST(IRModuleVerifierDeathTest, ReportsNonTypeFieldMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerConstant Value(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 1);
    StructType Record;
    Record.addField(core::NAME_None, Value);
    Module ModuleValue;
    ModuleValue.addStructType(Record);
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Directly constructed array types still have their borrowed element metadata checked during module verification.
  TEST(IRModuleVerifierDeathTest, ReportsNonTypeArrayElementMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerConstant Value(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 1);
    const ArrayType Array(Value, 4);
    StructType Record;
    Record.addField(core::NAME_None, Array);
    Module ModuleValue;
    ModuleValue.addStructType(Record);
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // A concrete pointer wrapper cannot turn an ordinary runtime value into valid global storage type metadata.
  TEST(IRModuleVerifierDeathTest, ReportsNonTypePointerPointeeMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerConstant Value(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 1);
    const PointerType Pointer(Value);
    Module ModuleValue;
    static_cast<void>(ModuleValue.createGlobalVariable(Pointer));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Function return metadata must be a type value even when the function has no body to inspect.
  TEST(IRModuleVerifierDeathTest, ReportsNonTypeFunctionReturnMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerConstant Value(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 1);
    const FunctionType Signature(Value);
    Module ModuleValue;
    static_cast<void>(ModuleValue.createFunction(Signature));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Function parameter descriptions cannot borrow a runtime constant in place of a type value.
  TEST(IRModuleVerifierDeathTest, ReportsNonTypeFunctionParameterMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerConstant Value(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 1);
    const FunctionType Signature(Types.getVoidType(), std::array{Parameter(Value)});
    Module ModuleValue;
    static_cast<void>(ModuleValue.createFunction(Signature));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Unknown value subclasses cannot masquerade as accepted symbolic types solely by borrowing the canonical metatype.
  TEST(IRModuleVerifierDeathTest, ReportsUnknownTypeValueKinds)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const UnknownTypeValue Unknown(Types.getMetaType());
    const PointerType Pointer(Unknown);
    Module ModuleValue;
    static_cast<void>(ModuleValue.createGlobalVariable(Pointer));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Stored void pointers are valid, while void itself cannot be global storage, a field, an array element, or a parameter type.
  TEST(IRModuleVerifierDeathTest, ReportsVoidTypesInStoredAndParameterPositions)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    Module PointerModule;
    static_cast<void>(PointerModule.createGlobalVariable(Types.getPointerType(Types.getPointerType(Types.getVoidType()))));
    verifyModule(PointerModule, Engine);
    Module VoidStorageModule;
    static_cast<void>(VoidStorageModule.createGlobalVariable(Types.getPointerType(Types.getVoidType())));
    EXPECT_DEATH(verifyModule(VoidStorageModule, Engine), "INK-I0001");
    StructType Record;
    Record.addField(core::NAME_None, Types.getVoidType());
    Module FieldModule;
    FieldModule.addStructType(Record);
    EXPECT_DEATH(verifyModule(FieldModule, Engine), "INK-I0001");
    const ArrayType Array(Types.getVoidType(), 4);
    StructType ArrayRecord;
    ArrayRecord.addField(core::NAME_None, Array);
    Module ArrayModule;
    ArrayModule.addStructType(ArrayRecord);
    EXPECT_DEATH(verifyModule(ArrayModule, Engine), "INK-I0001");
    const FunctionType Signature(Types.getVoidType(), std::array{Parameter(Types.getVoidType())});
    Module ParameterModule;
    static_cast<void>(ParameterModule.createFunction(Signature));
    EXPECT_DEATH(verifyModule(ParameterModule, Engine), "INK-I0001");
  }

  // Instance fields cannot create direct or array-mediated by-value layout cycles between struct definitions.
  TEST(IRModuleVerifierDeathTest, ReportsDirectAndArrayMediatedStructLayoutCycles)
  {
    const core::DiagnosticEngine Engine;
    StructType Direct;
    Direct.addField(core::NAME_None, Direct);
    Module DirectModule;
    DirectModule.addStructType(Direct);
    EXPECT_DEATH(verifyModule(DirectModule, Engine), "INK-I0001");
    StructType Element;
    const ArrayType Array(Element, 1);
    Element.addField(core::NAME_None, Array);
    Module ArrayModule;
    ArrayModule.addStructType(Element);
    EXPECT_DEATH(verifyModule(ArrayModule, Engine), "INK-I0001");
  }

  // Nominal base graphs must reject cycles independently of pointer-mediated field recursion.
  TEST(IRModuleVerifierDeathTest, ReportsInheritanceCycles)
  {
    const core::DiagnosticEngine Engine;
    StructType First;
    StructType Second;
    First.addBase(Second);
    Second.addBase(First);
    Module ModuleValue;
    ModuleValue.addStructType(First);
    ModuleValue.addStructType(Second);
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Unsupported signedness enumerators must not silently behave as unsigned integer metadata.
  TEST(IRModuleVerifierDeathTest, ReportsInvalidIntegerSignednessEnumerators)
  {
    const core::DiagnosticEngine Engine;
    const IntegerType Invalid(IntegerType::Width::Bits32, static_cast<IntegerType::Signedness>(255));
    const PointerType Pointer(Invalid);
    Module ModuleValue;
    static_cast<void>(ModuleValue.createGlobalVariable(Pointer));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // Module declaration types cannot hide detached instruction results inside pointer metadata for globals or registered fields.
  TEST(IRModuleVerifierDeathTest, ReportsInstructionDependenciesInGlobalAndRecordMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const PhiInstruction Detached(Types.getMetaType());
    const PointerType &Nested = Types.getPointerType(Detached);
    Module GlobalModule;
    static_cast<void>(GlobalModule.createGlobalVariable(Types.getPointerType(Nested)));
    EXPECT_DEATH(verifyModule(GlobalModule, Engine), "INK-I0001");
    StructType Record;
    Record.addField(core::NAME_None, Nested);
    Module RecordModule;
    RecordModule.addStructType(Record);
    EXPECT_DEATH(verifyModule(RecordModule, Engine), "INK-I0001");
  }
} // namespace ink::ir::test
