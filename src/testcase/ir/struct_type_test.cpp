#include "ink/core/context.h"
#include "ink/ir/analysis/verifier.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/module.h"
#include "ink/ir/model/struct_type.h"
#include "ink/ir/serialization.h"

#include "../diagnostic_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace ink::ir
{
  namespace
  {
    bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      return std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == Kind;
      });
    }

    void expectUserDefinedReflectArgument(const IRContext &Context, const Module &ModuleValue)
    {
      ASSERT_EQ(ModuleValue.StructTypes.size(), 3U);
      const StructType &LeafType = *ModuleValue.StructTypes[0];
      const StructType &DescriptorType = *ModuleValue.StructTypes[1];
      const StructType &RecordType = *ModuleValue.StructTypes[2];
      EXPECT_EQ(LeafType.name(), "ReflectLeaf");
      ASSERT_EQ(LeafType.fieldCount(), 2U);
      EXPECT_EQ(LeafType.field(0).name(), "Code");
      EXPECT_EQ(LeafType.field(1).name(), "Flags");
      EXPECT_EQ(DescriptorType.name(), "ReflectDescriptor");
      ASSERT_EQ(DescriptorType.fieldCount(), 2U);
      EXPECT_EQ(DescriptorType.field(0).name(), "Version");
      EXPECT_EQ(DescriptorType.field(1).name(), "Payload");
      EXPECT_EQ(DescriptorType.fieldType(1), &LeafType);
      ASSERT_EQ(RecordType.fieldCount(), 1U);
      const StructField &ReflectedField = RecordType.field(0);
      ASSERT_EQ(ReflectedField.attributes().size(), 1U);
      const Attribute &Reflect = ReflectedField.attributes()[0];
      ASSERT_EQ(Reflect.kind(), AttributeKind::Reflect);
      ASSERT_EQ(Reflect.arguments().size(), 1U);
      EXPECT_EQ(Reflect.arguments()[0].key(), "Descriptor");
      const Constant &DescriptorConstant = Reflect.arguments()[0].value();
      ASSERT_EQ(DescriptorConstant.kind(), ValueKind::AggregateConstant);
      EXPECT_EQ(&DescriptorConstant.type(), &DescriptorType);
      EXPECT_TRUE(Context.constantPool().owns(DescriptorConstant));
      const AggregateConstant &Descriptor = static_cast<const AggregateConstant &>(DescriptorConstant);
      ASSERT_EQ(Descriptor.elements().size(), 2U);
      const Constant &VersionConstant = Descriptor.elements()[0].get();
      ASSERT_EQ(VersionConstant.kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(static_cast<const IntegerConstant &>(VersionConstant).signedValue(), 7);
      EXPECT_TRUE(Context.constantPool().owns(VersionConstant));
      const Constant &LeafConstant = Descriptor.elements()[1].get();
      ASSERT_EQ(LeafConstant.kind(), ValueKind::AggregateConstant);
      EXPECT_EQ(&LeafConstant.type(), &LeafType);
      EXPECT_TRUE(Context.constantPool().owns(LeafConstant));
      const AggregateConstant &Leaf = static_cast<const AggregateConstant &>(LeafConstant);
      ASSERT_EQ(Leaf.elements().size(), 2U);
      ASSERT_EQ(Leaf.elements()[0].get().kind(), ValueKind::IntegerConstant);
      ASSERT_EQ(Leaf.elements()[1].get().kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(static_cast<const IntegerConstant &>(Leaf.elements()[0].get()).signedValue(), 42);
      EXPECT_EQ(static_cast<const IntegerConstant &>(Leaf.elements()[1].get()).unsignedValue(), 9U);
      EXPECT_TRUE(Context.constantPool().owns(Leaf.elements()[0].get()));
      EXPECT_TRUE(Context.constantPool().owns(Leaf.elements()[1].get()));
    }

    // Verifies that the standalone StructType model header exposes its Type inheritance, name, and ordered fields.
    TEST(StructTypeTest, ExposesNamedOrderedFieldsFromStandaloneModel)
    {
      static_assert(std::is_base_of_v<Type, StructType>);
      core::CompilationContext Compilation;
      IRContext Context(Compilation);
      const Type &ByteType = Context.getType(TypeKind::Byte);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const StructType &Record = Context.createStructType("Record", {&ByteType, &I32Type});

      EXPECT_EQ(Record.kind(), TypeKind::Struct);
      EXPECT_EQ(Record.name().text(), "Record");
      ASSERT_EQ(Record.fieldCount(), 2U);
      EXPECT_EQ(Record.fieldType(0), &ByteType);
      EXPECT_EQ(Record.fieldType(1), &I32Type);
    }

    // Verifies that a struct field directly retains its name, type, layout constraints, built-in attributes, and user-selected key-value argument names.
    TEST(StructTypeTest, StoresFieldAttributesWithCanonicalConstants)
    {
      core::CompilationContext Compilation;
      IRContext Context(Compilation);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const IntegerConstant &Level = Context.constantPool().getIntegerConstant(I32Type, 3);
      const IntegerConstant &Version = Context.constantPool().getIntegerConstant(I32Type, 1);
      std::vector<AttributeArgument> Arguments;
      Arguments.emplace_back("Level", Level);
      Arguments.emplace_back("UserSelectedKey", Version);
      std::vector<Attribute> Attributes;
      Attributes.emplace_back(AttributeKind::Reflect, std::move(Arguments));
      Attributes.emplace_back(AttributeKind::Stored);
      FieldLayoutConstraints FieldConstraints;
      FieldConstraints.ExplicitAlignment = 8;
      FieldConstraints.ExplicitOffset = 16;
      std::vector<StructField> Fields;
      Fields.emplace_back("Value", &I32Type, std::move(Attributes), FieldConstraints);
      StructLayoutConstraints StructConstraints;
      StructConstraints.ExplicitAlignment = 16;
      const StructType &Record = Context.createStructType("Record", std::move(Fields), StructConstraints);

      ASSERT_EQ(Record.fieldCount(), 1U);
      EXPECT_EQ(Record.name(), "Record");
      EXPECT_EQ(Record.layoutConstraints().ExplicitAlignment, 16U);
      const StructField &Field = Record.field(0);
      EXPECT_EQ(Field.name(), "Value");
      EXPECT_EQ(Field.type(), &I32Type);
      EXPECT_EQ(Field.layoutConstraints().ExplicitAlignment, 8U);
      EXPECT_EQ(Field.layoutConstraints().ExplicitOffset, 16U);
      ASSERT_EQ(Field.attributes().size(), 2U);
      EXPECT_EQ(Field.attributes()[0].kind(), AttributeKind::Reflect);
      EXPECT_EQ(Field.attributes()[1].kind(), AttributeKind::Stored);
      ASSERT_EQ(Field.attributes()[0].arguments().size(), 2U);
      EXPECT_EQ(Field.attributes()[0].arguments()[0].key(), "Level");
      EXPECT_EQ(&Field.attributes()[0].arguments()[0].value(), &Level);
      EXPECT_EQ(Field.attributes()[0].arguments()[1].key(), "UserSelectedKey");
      EXPECT_EQ(&Field.attributes()[0].arguments()[1].value(), &Version);
      EXPECT_STREQ(attributeKindName(AttributeKind::Reflect), "Reflect");
      EXPECT_STREQ(attributeKindSpelling(AttributeKind::Reflect), "reflect");
      EXPECT_EQ(attributeKindFromSpelling("stored"), AttributeKind::Stored);
      EXPECT_FALSE(attributeKindFromSpelling("userDefined").has_value());
    }

    // Verifies that field names, layout constraints, every existing scalar constant category, aggregate constants, and empty attributes survive InkIR text round trips.
    TEST(StructTypeSerializationTest, RoundTripsNamedAttributedFieldsAndLayoutConstraints)
    {
      core::CompilationContext FirstCompilation;
      IRContext FirstContext(FirstCompilation);
      const std::string Text =
          "inkir 1\n"
          "%Meta = type {i32}\n"
          "%Record = type align(16) pack(4) {Value: byte align(2) offset(0) [reflect(Level = i32 7, Label = const byte[] c\"ink\", Bits = f32 floatbits(f32,0x3F800000), Pointer = byte* null, Default = i32 zeroinitializer, Metadata = %Meta {i32 1}), stored]}\n";

      DeserializeResult FirstResult = deserialize(FirstContext, Text);

      ASSERT_TRUE(FirstResult.succeeded());
      ASSERT_TRUE(FirstResult.module().has_value());
      ASSERT_EQ(FirstResult.module()->StructTypes.size(), 2U);
      const StructType &Record = *FirstResult.module()->StructTypes[1];
      EXPECT_EQ(Record.layoutConstraints().ExplicitAlignment, 16U);
      EXPECT_EQ(Record.layoutConstraints().Packing, 4U);
      ASSERT_EQ(Record.fieldCount(), 1U);
      const StructField &Field = Record.field(0);
      EXPECT_EQ(Field.name(), "Value");
      EXPECT_EQ(Field.layoutConstraints().ExplicitAlignment, 2U);
      EXPECT_EQ(Field.layoutConstraints().ExplicitOffset, 0U);
      ASSERT_EQ(Field.attributes().size(), 2U);
      ASSERT_EQ(Field.attributes()[0].arguments().size(), 6U);
      EXPECT_EQ(Field.attributes()[0].arguments()[0].value().kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(Field.attributes()[0].arguments()[1].value().kind(), ValueKind::StringConstant);
      EXPECT_EQ(Field.attributes()[0].arguments()[2].value().kind(), ValueKind::FloatConstant);
      EXPECT_EQ(Field.attributes()[0].arguments()[3].value().kind(), ValueKind::NullConstant);
      EXPECT_EQ(Field.attributes()[0].arguments()[4].value().kind(), ValueKind::ZeroInitializer);
      EXPECT_EQ(Field.attributes()[0].arguments()[5].value().kind(), ValueKind::AggregateConstant);

      SerializeResult Serialized = serialize(FirstContext, *FirstResult.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      core::CompilationContext SecondCompilation;
      IRContext SecondContext(SecondCompilation);
      DeserializeResult SecondResult = deserialize(SecondContext, *Serialized.text());

      ASSERT_TRUE(SecondResult.succeeded());
      ASSERT_TRUE(SecondResult.module().has_value());
      ASSERT_EQ(SecondResult.module()->StructTypes.size(), 2U);
      const StructType &RoundTripped = *SecondResult.module()->StructTypes[1];
      ASSERT_EQ(RoundTripped.fieldCount(), 1U);
      EXPECT_EQ(RoundTripped.field(0).name(), "Value");
      ASSERT_EQ(RoundTripped.field(0).attributes().size(), 2U);
      EXPECT_EQ(RoundTripped.field(0).attributes()[0].arguments()[5].value().kind(), ValueKind::AggregateConstant);
    }

    // Verifies that reflect accepts a user-defined struct constant containing another user-defined struct and preserves every type and value binding across serialization.
    TEST(StructTypeSerializationTest, RoundTripsUserDefinedStructReflectArgument)
    {
      core::CompilationContext FirstCompilation;
      IRContext FirstContext(FirstCompilation);
      const std::string Text =
          "inkir 1\n"
          "%ReflectLeaf = type {Code: i32, Flags: byte}\n"
          "%ReflectDescriptor = type {Version: i32, Payload: %ReflectLeaf}\n"
          "%Record = type {Value: i32 [reflect(Descriptor = %ReflectDescriptor {i32 7, %ReflectLeaf {i32 42, byte 9}})]}\n";

      DeserializeResult FirstResult = deserialize(FirstContext, Text);

      ASSERT_TRUE(FirstResult.succeeded());
      ASSERT_TRUE(FirstResult.module().has_value());
      expectUserDefinedReflectArgument(FirstContext, *FirstResult.module());
      SerializeResult Serialized = serialize(FirstContext, *FirstResult.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      core::CompilationContext SecondCompilation;
      IRContext SecondContext(SecondCompilation);
      DeserializeResult SecondResult = deserialize(SecondContext, *Serialized.text());

      ASSERT_TRUE(SecondResult.succeeded());
      ASSERT_TRUE(SecondResult.module().has_value());
      expectUserDefinedReflectArgument(SecondContext, *SecondResult.module());
      const Constant &FirstDescriptor = FirstResult.module()->StructTypes[2]->field(0).attributes()[0].arguments()[0].value();
      const Constant &SecondDescriptor = SecondResult.module()->StructTypes[2]->field(0).attributes()[0].arguments()[0].value();
      EXPECT_NE(&FirstDescriptor, &SecondDescriptor);
      EXPECT_NE(&FirstDescriptor.type(), &SecondDescriptor.type());
      EXPECT_FALSE(constantsEqual(FirstDescriptor, SecondDescriptor));
    }

    // Verifies that the module verifier rejects invalid and duplicate field names, unknown built-in kinds, invalid or duplicate argument keys, and constants from another context.
    TEST(StructTypeVerifierTest, RejectsMalformedFieldAttributeMetadata)
    {
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      IRContext Context(Compilation);
      core::CompilationContext ForeignCompilation;
      IRContext ForeignContext(ForeignCompilation);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const IntegerConstant &ForeignValue = ForeignContext.constantPool().getIntegerConstant(I32Type, 1);
      std::vector<AttributeArgument> InvalidArguments;
      InvalidArguments.emplace_back("", ForeignValue);
      InvalidArguments.emplace_back("Duplicate", ForeignValue);
      InvalidArguments.emplace_back("Duplicate", ForeignValue);
      std::vector<Attribute> FirstAttributes;
      FirstAttributes.emplace_back(AttributeKind::Reflect, std::move(InvalidArguments));
      FirstAttributes.emplace_back(static_cast<AttributeKind>(static_cast<std::uint8_t>(AttributeKind::Count) + 1U));
      std::vector<StructField> Fields;
      Fields.emplace_back("1Invalid", &I32Type, std::move(FirstAttributes));
      Fields.emplace_back("Repeated", &I32Type);
      Fields.emplace_back("Repeated", &I32Type);
      StructLayoutConstraints InvalidLayoutConstraints;
      InvalidLayoutConstraints.Packing = 3;
      const StructType &Record = Context.createStructType("Record", std::move(Fields), InvalidLayoutConstraints);
      Module ModuleValue(Context);
      ModuleValue.StructTypes.push_back(&Record);

      const VerificationResult Result = verify(Context, ModuleValue);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrInvalidStructFieldName));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrDuplicateStructFieldName));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrUnknownStructFieldAttribute));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrInvalidStructFieldAttributeArgumentName));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrDuplicateStructFieldAttributeArgumentName));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrConstantPoolMismatch));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrInvalidStructLayoutConstraints));
    }

    // Verifies that the text parser rejects names outside the fixed built-in attribute registry instead of treating them as user-defined attributes.
    TEST(StructTypeSerializationTest, RejectsUnknownAttributeKinds)
    {
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      IRContext Context(Compilation);

      const DeserializeResult Result = deserialize(Context, "inkir 1\n%Record = type {Value: i32 [userDefined(Key = i32 1)]}\n");

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrExpected));
    }
  } // namespace
} // namespace ink::ir
