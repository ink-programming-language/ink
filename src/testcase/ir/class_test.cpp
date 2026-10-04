#include "ink/ir/ir_builder.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/module/module_serialization.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // Nominal definitions retain distinct identity and can be completed only once without partial mutation on failure.
  TEST(IRClassTest, CompletesNominalTypesTransactionally)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *First = Builder.createClassType(Context.namePool().intern("Pair"));
    const auto *Second = Builder.createClassType(Context.namePool().intern("Pair"));
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const ClassField Fields[] = {{Context.namePool().intern("X"), Integer, VisibilityKind::Private}};
    EXPECT_NE(First, Second);
    EXPECT_FALSE(First->isComplete());
    EXPECT_EQ(Builder.createDetachedAllocaInstruction(*First), nullptr);
    EXPECT_FALSE(Builder.defineClassType(*First, Fields, {}));
    EXPECT_FALSE(First->isComplete());
    const auto Revision = Context.revision();
    ASSERT_TRUE(Builder.defineClassType(*First, Fields, "module::Pair"));
    EXPECT_GT(Context.revision(), Revision);
    EXPECT_EQ(First->identity(), "module::Pair");
    EXPECT_EQ(First->fields()[0].Visibility, VisibilityKind::Private);
    EXPECT_FALSE(Builder.defineClassType(*First, {}, "changed"));
    EXPECT_EQ(First->fields().size(), 1U);
    EXPECT_NE(Builder.createDetachedAllocaInstruction(*First), nullptr);
    EXPECT_FALSE(Second->isComplete());
  }

  // Fields reject duplicates, foreign contexts, void and direct or indirect infinite value layouts.
  TEST(IRClassTest, RejectsInvalidFieldsAndValueCycles)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Foreign(Compilation);
    IRBuilder Builder(Context);
    const auto *A = Builder.createClassType(Context.namePool().intern("A"));
    const auto *B = Builder.createClassType(Context.namePool().intern("B"));
    const auto Name = Context.namePool().intern("Value");
    const auto &Void = Context.typePool().getType<TypeKind::Void>();
    const auto &Bool = Context.typePool().getType<TypeKind::Bool>();
    const ClassField Duplicate[] = {{Name, &Bool}, {Name, &Bool}};
    const ClassField Invalid[] = {{Name, &Void}};
    const ClassField ForeignField[] = {{Name, &Foreign.typePool().getType<TypeKind::Bool>()}};
    const ClassField Self[] = {{Name, A}};
    EXPECT_FALSE(Builder.defineClassType(*A, Duplicate, "A"));
    EXPECT_FALSE(Builder.defineClassType(*A, Invalid, "A"));
    EXPECT_FALSE(Builder.defineClassType(*A, ForeignField, "A"));
    EXPECT_FALSE(Builder.defineClassType(*A, Self, "A"));
    const ClassField AFields[] = {{Name, B}};
    const ClassField BFields[] = {{Name, A}};
    ASSERT_TRUE(Builder.defineClassType(*A, AFields, "A"));
    EXPECT_FALSE(computeTypeLayout(*A, Compilation.targetContext()));
    EXPECT_FALSE(Builder.defineClassType(*B, BFields, "B"));
    EXPECT_FALSE(B->isComplete());
  }

  // Pointer recursion is finite and target layouts use declared alignment instead of the compiler host representation.
  TEST(IRClassTest, ComputesRecursivePointerLayoutsForBothTargets)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Node = Builder.createClassType(Context.namePool().intern("Node"));
    const auto *Pointer = Context.typePool().getType<TypeKind::Pointer>(*Node, AccessKind::ReadWrite);
    const auto *Byte = Context.typePool().getType<TypeKind::Integer>(8, false);
    const auto *Short = Context.typePool().getType<TypeKind::Integer>(16, false);
    const ClassField Fields[] = {
        {Context.namePool().intern("Tag"), Byte},
        {Context.namePool().intern("Next"), Pointer},
        {Context.namePool().intern("Count"), Short},
    };
    ASSERT_TRUE(Builder.defineClassType(*Node, Fields, "example::Node"));
    const auto Small = computeTypeLayout(*Node, core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian));
    const auto Large = computeTypeLayout(*Node, core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::BigEndian));
    ASSERT_TRUE(Small);
    ASSERT_TRUE(Large);
    EXPECT_EQ(Small->FieldOffsets, (std::vector<std::uint64_t>{0, 4, 8}));
    EXPECT_EQ(Small->Size, 12U);
    EXPECT_EQ(Large->FieldOffsets, (std::vector<std::uint64_t>{0, 8, 16}));
    EXPECT_EQ(Large->Size, 24U);
    EXPECT_EQ(Large->Stride, 24U);
    EXPECT_EQ(Large->Alignment, 8U);
  }

  // Empty classes have distinct storage while oversized arrays of classes fail target-size checks.
  TEST(IRClassTest, HandlesEmptyAndOverflowingLayouts)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Empty = Builder.createClassType(Context.namePool().intern("Empty"));
    ASSERT_TRUE(Builder.defineClassType(*Empty, {}, "Empty"));
    ASSERT_TRUE(computeTypeLayout(*Empty, Compilation.targetContext()));
    EXPECT_EQ(computeTypeLayout(*Empty, Compilation.targetContext())->Size, 1U);
    const auto *Huge = Context.typePool().getType<TypeKind::Array>(*Empty, UINT64_MAX);
    EXPECT_FALSE(computeTypeLayout(*Huge, core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian)));
    const auto *Outer = Context.typePool().getType<TypeKind::Array>(*Huge, 2);
    EXPECT_FALSE(computeTypeLayout(*Outer, core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian)));
  }

  // Constructing values checks exact nominal field types, and field projections preserve the original pointer's access.
  TEST(IRClassTest, BuildsTypedFieldValuesAndPlaces)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Class = Builder.createClassType(Context.namePool().intern("C"));
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const ClassField Fields[] = {{Context.namePool().intern("Value"), Integer}};
    ASSERT_TRUE(Builder.defineClassType(*Class, Fields, "C"));
    const auto *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    const Value *Arguments[] = {One};
    auto Value = Builder.createDetachedClassInstruction(*Class, Arguments);
    ASSERT_NE(Value, nullptr);
    EXPECT_EQ(Builder.createDetachedClassInstruction(*Class, {}), nullptr);
    EXPECT_EQ(Builder.createDetachedFieldExtractInstruction(*Value, 1), nullptr);
    auto Extract = Builder.createDetachedFieldExtractInstruction(*Value, 0);
    ASSERT_NE(Extract, nullptr);
    EXPECT_EQ(&Extract->type(), Integer);
    const auto *ReadOnly = Context.typePool().getType<TypeKind::Pointer>(*Class, AccessKind::ReadOnly);
    const Type *Parameters[] = {ReadOnly};
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), Parameters);
    auto Function = Builder.createFunction(Context.namePool().intern("read"), *Signature);
    auto Address = Builder.createDetachedFieldPointerInstruction(*Function->parameters()[0], 0);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(static_cast<const PointerType &>(Address->type()).access(), AccessKind::ReadOnly);
    EXPECT_NE(Builder.createDetachedLoadInstruction(*Address), nullptr);
    EXPECT_EQ(Builder.createDetachedStoreInstruction(*Address, *One), nullptr);
  }

  // Class constants are interned by nominal type and ordered fields without merging structurally identical classes.
  TEST(IRClassTest, InternsClassConstantsByNominalIdentity)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *A = Builder.createClassType(Context.namePool().intern("A"));
    const auto *B = Builder.createClassType(Context.namePool().intern("B"));
    const auto &Bool = Context.typePool().getType<TypeKind::Bool>();
    const ClassField Fields[] = {{Context.namePool().intern("Flag"), &Bool}};
    ASSERT_TRUE(Builder.defineClassType(*A, Fields, "A"));
    ASSERT_TRUE(Builder.defineClassType(*B, Fields, "B"));
    const Constant *Values[] = {&Context.constantPool().getBoolConstant(true)};
    const auto *First = Context.constantPool().getClassConstant(*A, Values);
    ASSERT_NE(First, nullptr);
    EXPECT_EQ(Context.constantPool().getClassConstant(*A, Values), First);
    EXPECT_NE(Context.constantPool().getClassConstant(*B, Values), First);
    EXPECT_EQ(Context.constantPool().getClassConstant(*A, {}), nullptr);
    EXPECT_TRUE(Context.constantPool().owns(*First));
    EXPECT_TRUE(Constant::classof(First));
  }

  // Both archive formats retain class definitions, recursive pointers, constants and field value/place operations.
  TEST(IRClassTest, RoundTripsDefinitionsConstantsAndInstructions)
  {
    constexpr std::string_view Text = R"(ink-ir 6
module @Classes {
  type !c = class "Pair" identity "Classes::Pair" { public "X": i32, private "Flag": bool }
  type !node = class "Node" identity "Classes::Node" { public "Next": ptr<rw, !node> }
  declare void @accept(ptr<rw, !node> %node)
  define !c @constant() {
  constant_entry:
    ret !c {7, true}
  }
  define i32 @run() {
  run_entry:
    %value = class.value !c {7, true}
    %storage = alloca !c
    store !c %value, ptr<rw, !c> %storage
    %address = field.pointer ptr<rw, i32>, 0, ptr<rw, !c> %storage
    %flag = field.extract bool, 1, !c %value
    %result = load i32, ptr<rw, i32> %address
    ret i32 %result
  }
}
)";
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, Text);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (const bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Restored(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Restored, Encoded.Bytes) : deserializeModuleText(Restored, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto Reencoded = Binary ? serializeModuleBinary(*Decoded.ModuleValue) : serializeModuleText(*Decoded.ModuleValue);
      ASSERT_TRUE(Reencoded.succeeded()) << Reencoded.Message;
      EXPECT_EQ(Reencoded.Bytes, Encoded.Bytes);
    }
  }

  // Text class definitions charge metadata fields and combined names against the same limits as binary records.
  TEST(IRClassTest, EnforcesClassArchiveLimits)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    ModuleArchiveLimits Limits;
    Limits.MaxFields = 1;
    const auto Metadata = deserializeModuleText(Context, "ink-ir 6 module @M { type !c = class \"C\" identity \"C\" {} }", Limits);
    EXPECT_EQ(Metadata.Status, ModuleArchiveStatus::LimitExceeded);
    Limits.MaxFields = 4;
    const auto Fields = deserializeModuleText(Context, "ink-ir 6 module @M { type !c = class \"C\" identity \"C\" { public \"X\": i32 } }", Limits);
    EXPECT_EQ(Fields.Status, ModuleArchiveStatus::LimitExceeded);
    Limits.MaxFields = 10;
    Limits.MaxStringBytes = 8;
    const auto Names = deserializeModuleText(Context, "ink-ir 6 module @M { type !c = class \"CC\" identity \"CC\" { public \"XXXXX\": i32 } }", Limits);
    EXPECT_EQ(Names.Status, ModuleArchiveStatus::LimitExceeded);
    EXPECT_TRUE(Context.modules().empty());
  }

  // Malformed archives cannot publish duplicate fields, infinite layouts, wrong field indices or incomplete constructions.
  TEST(IRClassTest, RejectsMalformedClassArchives)
  {
    constexpr std::string_view Bodies[] = {
        "type !c = class \"C\" identity \"C\" { public \"X\": i32, public \"X\": i32 }",
        "type !c = class \"C\" identity \"C\" { public \"X\": !c }",
        "type !c = class \"C\" identity \"C\" { public \"X\": void }",
        "type !c = class \"C\" identity \"\" { }",
        "type !c = class \"C\" identity \"C\" { public \"X\": i32 } define i32 @f() { entry: %v = field.extract i32, 1, !c {1} ret i32 %v }",
        "type !c = class \"C\" identity \"C\" { public \"X\": i32 } define !c @f() { entry: %v = class.value !c {} ret !c %v }",
    };
    for (const auto Body : Bodies)
    {
      core::CompilationContext Compilation;
      IRContext Context(Compilation);
      const auto Result = deserializeModuleText(Context, "ink-ir 6 module @Invalid { " + std::string(Body) + " }");
      EXPECT_FALSE(Result.succeeded()) << Body;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Context.modules().empty());
    }
  }
} // namespace ink::ir::test
