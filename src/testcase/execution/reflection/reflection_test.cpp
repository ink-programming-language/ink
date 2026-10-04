#include "../artifact/artifact_test_support.h"
#include "ink/execution/reflection/reflection.h"
#include "ink/ir/module/module_serialization.h"

#include <gtest/gtest.h>

#ifdef _WIN32
#define INK_REFLECTION_EXPORT extern "C" __declspec(dllexport)
#else
#define INK_REFLECTION_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace
{
  std::int32_t InitializerCalls = 0;
}

INK_REFLECTION_EXPORT std::int32_t inkTestReflectionNext() noexcept
{
  return ++InitializerCalls;
}

#undef INK_REFLECTION_EXPORT

namespace ink::execution::test
{
  namespace
  {
    struct ReflectionProgram : ArtifactContext
    {
        const ir::ClassType *Class = nullptr;
        ir::Function *Next = nullptr;
        ir::Function *Initial = nullptr;
        ir::Function *Secret = nullptr;
        ir::Function *Add = nullptr;

        bool build()
        {
          Class = Builder.createClassType(Context.namePool().intern("Counter"));
          const ir::ClassField Fields[] = {
              {Context.namePool().intern("Value"), &Int32},
              {Context.namePool().intern("Secret"), &Int32, ir::VisibilityKind::Private},
          };
          if (!Builder.defineClassType(*Class, Fields, "reflection::Counter"))
          {
            return false;
          }
          Next = function("inkTestReflectionNext", Int32, {}, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
          Initial = function("Counter.__default_0", Int32);
          if (!begin(*Initial) || !Builder.createReturnInstruction(Builder.createCallInstruction(*Next)) || !Builder.setFieldInitializer(*Class, 0, *Initial))
          {
            return false;
          }
          Secret = function("Counter.__default_1", Int32);
          if (!begin(*Secret) || !Builder.createReturnInstruction(&integer(7)) || !Builder.setFieldInitializer(*Class, 1, *Secret))
          {
            return false;
          }
          const ir::Type *Parameters[] = {Context.typePool().getType<ir::TypeKind::Pointer>(*Class, ir::AccessKind::ReadWrite), &Int32};
          Add = function("Counter.add", Int32, Parameters);
          if (!begin(*Add))
          {
            return false;
          }
          const auto *Field = Builder.createFieldPointerInstruction(*Add->parameters()[0], 0);
          const auto *Sum = Builder.createAddInstruction(*Builder.createLoadInstruction(*Field), *Add->parameters()[1]);
          return Builder.createStoreInstruction(*Field, *Sum) && Builder.createReturnInstruction(Sum) && Builder.setClassMethod(*Class, *Add);
        }

        BytecodeArtifactResult artifact()
        {
          const BytecodeFunctionInput Inputs[] = {input(*Add, "reflection", "add"), input(*Next, "C", "inkTestReflectionNext", BytecodeSymbolKind::Native)};
          return buildBytecodeObject("reflection", Bridge, Inputs);
        }
    };
  } // namespace

  // Reflection executes defaults for each object, invokes methods, and enforces visibility, domains and writable views.
  TEST(ReflectionTest, ConstructsAndInvokesWithCheckedFieldAccess)
  {
    ReflectionProgram Test;
    ASSERT_TRUE(Test.build());
    const auto Type = Test.Bridge.lowerType(*Test.Class);
    ASSERT_NE(Type, InvalidRuntimeType);
    EXPECT_EQ(Test.Bridge.types()->find("reflection::Counter")->Type, Type);
    EXPECT_EQ(Test.Bridge.types()->find("i32")->Type, Test.Bridge.lowerType(Test.Int32));
    ExecutionEngine Engine(Test.Context);
    ExecutionLinker Linker(Test.Bridge);
    ExecutionMachine Machine(Engine, Linker);
    auto &Memory = Engine.heap().memoryManager();
    Reflection Reflect(Test.Bridge.types(), Memory, &Machine);
    InitializerCalls = 40;
    const auto First = Reflect.construct(Type);
    const auto Second = Reflect.construct(Type);
    ASSERT_TRUE(First) << static_cast<unsigned>(First.Status);
    ASSERT_TRUE(Second) << static_cast<unsigned>(Second.Status);
    EXPECT_EQ(InitializerCalls, 42);
    const auto Object = Reflect.view(First.Place);
    ASSERT_TRUE(Object);
    EXPECT_EQ(Reflect.getField(Object.Value, "Value").Value.Bits, 41U);
    EXPECT_EQ(Reflect.getField(Reflect.view(Second.Place).Value, "Value").Value.Bits, 42U);
    EXPECT_EQ(Reflect.getField(Object.Value, "Secret").Status, ExecutionStatus::AccessDenied);
    EXPECT_EQ(Reflect.getField(Object.Value, "Missing").Status, ExecutionStatus::UnknownBinding);
    const auto One = Reflect.value(RuntimeValue::fromBits(1, Test.Bridge.lowerType(Test.Int32)));
    const ReflectedValue Arguments[] = {One};
    EXPECT_EQ(Reflect.invoke(Object.Value, "add", Arguments).Value.Bits, 42U);
    auto ReadOnly = Object.Value;
    ReadOnly.Writable = false;
    EXPECT_EQ(Reflect.setField(ReadOnly, "Value", One), ExecutionStatus::ReadOnly);
    EXPECT_EQ(Reflect.invoke(ReadOnly, "add", Arguments).Status, ExecutionStatus::ReadOnly);
    auto Foreign = One;
    Foreign.Domain = std::make_shared<RuntimeTypeDomain>();
    EXPECT_EQ(Reflect.setField(Object.Value, "Value", Foreign), ExecutionStatus::ForeignContext);
    const ReflectedValue PrivateArguments[] = {One, One};
    EXPECT_EQ(Reflect.construct(Type, PrivateArguments).Status, ExecutionStatus::AccessDenied);
    EXPECT_EQ(InitializerCalls, 42);
    EXPECT_EQ(Memory.release(First.Place), ExecutionStatus::Success);
    EXPECT_EQ(Reflect.getField(Object.Value, "Value").Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Memory.release(Second.Place), ExecutionStatus::Success);
  }

  // Frozen reflection metadata survives IR destruction, bytecode serialization and function/type ID remapping by the linker.
  TEST(ReflectionTest, RetainsCallableMetadataThroughArchiveAndLink)
  {
    std::unique_ptr<BytecodeArtifact> Object;
    {
      ReflectionProgram Test;
      ASSERT_TRUE(Test.build());
      auto Built = Test.artifact();
      ASSERT_TRUE(Built) << Built.Message;
      const auto Bytes = serializeBytecodeArtifact(*Built.Artifact);
      ASSERT_TRUE(Bytes) << Bytes.Message;
      auto Loaded = deserializeBytecodeArtifact(Bytes.Bytes);
      ASSERT_TRUE(Loaded) << Loaded.Message;
      Object = std::move(Loaded.Artifact);
    }
    auto Padding = constantObject("padding", "padding", 0);
    ASSERT_TRUE(Padding);
    const BytecodeArtifact *Objects[] = {Padding.Artifact.get(), Object.get()};
    auto Linked = linkBytecodeArtifacts(Objects);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Types = Linked.Artifact->Image.Layouts;
    const auto *Class = Types->find("reflection::Counter");
    ASSERT_NE(Class, nullptr);
    ASSERT_EQ(Class->classDesc().Methods.size(), 1U);
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionEngine Engine(Context);
    ExecutionLinker Linker(std::move(Linked.Artifact->Image));
    ExecutionMachine Machine(Engine, Linker);
    Reflection Reflect(Types, Engine.heap().memoryManager(), &Machine);
    InitializerCalls = 10;
    auto Instance = Reflect.construct(Class->Type);
    ASSERT_TRUE(Instance) << static_cast<unsigned>(Instance.Status);
    auto View = Reflect.view(Instance.Place);
    const ReflectedValue Arguments[] = {Reflect.value(RuntimeValue::fromBits(31, Types->find("i32")->Type))};
    EXPECT_EQ(Reflect.invoke(View.Value, "add", Arguments).Value.Bits, 42U);
    EXPECT_EQ(Engine.heap().memoryManager().release(Instance.Place), ExecutionStatus::Success);
  }

  // A serialized image cannot redirect an initializer to a function with arguments or a method to the wrong receiver type.
  TEST(ReflectionTest, RejectsCorruptCallableMetadata)
  {
    ReflectionProgram Test;
    ASSERT_TRUE(Test.build());
    auto Object = Test.artifact();
    ASSERT_TRUE(Object) << Object.Message;
    auto Types = std::const_pointer_cast<RuntimeTypeTable>(Object.Artifact->Image.Layouts);
    const auto *Class = Types->find("reflection::Counter");
    auto Description = std::make_shared<ClassDesc>(Class->classDesc());
    Description->Fields[0].Initializer = Description->Methods[0].Function;
    ASSERT_TRUE(Types->updateClass(Class->Type, Description));
    EXPECT_FALSE(validateBytecodeArtifact(*Object.Artifact));
    EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
  }

  // IR text and binary retain method ownership and default initializer associations without the semantic AST.
  TEST(ReflectionTest, RoundTripsIRMemberAssociations)
  {
    ReflectionProgram Test;
    ASSERT_TRUE(Test.build());
    auto *Module = Test.Builder.createModule(Test.Context.namePool().intern("reflection"));
    ASSERT_TRUE(Test.Builder.registerClassType(*Module, *Test.Class));
    for (auto &Function : Test.Functions)
    {
      ASSERT_TRUE(Test.Builder.appendValue(Module->entryBlock(), std::move(Function)));
    }
    for (bool Binary : {false, true})
    {
      auto Encoded = Binary ? ir::serializeModuleBinary(*Module) : ir::serializeModuleText(*Module);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      auto Decoded = Binary ? ir::deserializeModuleBinary(Context, Encoded.Bytes) : ir::deserializeModuleText(Context, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      ASSERT_EQ(Decoded.ModuleValue->classTypes().size(), 1U);
      SemanticValueBridge Bridge(Context);
      for (const auto &Value : Decoded.ModuleValue->entryBlock().values())
      {
        if (ir::Function::classof(Value.get()))
        {
          Bridge.lowerFunction(static_cast<const ir::Function &>(*Value));
        }
      }
      const auto *Class = Bridge.types()->find("reflection::Counter");
      ASSERT_NE(Class, nullptr);
      EXPECT_EQ(Class->classDesc().Fields[1].Visibility, MemberVisibility::Private);
      EXPECT_NE(Class->classDesc().Fields[0].Initializer, InvalidFunction);
      ASSERT_EQ(Class->classDesc().Methods.size(), 1U);
      EXPECT_EQ(Class->classDesc().Methods[0].Name, "add");
    }
  }

  // A module containing only an otherwise unreferenced class preserves its type registry through IR and bytecode archives.
  TEST(ReflectionTest, RetainsUnreferencedClassRoots)
  {
    ArtifactContext Test;
    auto *Module = Test.Builder.createModule(Test.Context.namePool().intern("types"));
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Unused"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Code"), &Test.Int32}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "types::Unused"));
    ASSERT_TRUE(Test.Builder.registerClassType(*Module, *Class));
    ASSERT_TRUE(Test.Builder.registerClassType(*Module, *Class));
    ASSERT_EQ(Module->classTypes().size(), 1U);
    for (bool Binary : {false, true})
    {
      auto Encoded = Binary ? ir::serializeModuleBinary(*Module) : ir::serializeModuleText(*Module);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      auto Decoded = Binary ? ir::deserializeModuleBinary(Context, Encoded.Bytes) : ir::deserializeModuleText(Context, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      ASSERT_EQ(Decoded.ModuleValue->classTypes().size(), 1U);
      SemanticValueBridge Bridge(Context);
      auto Built = buildBytecodeObject("types", Bridge, {});
      ASSERT_TRUE(Built) << Built.Message;
      auto Bytes = serializeBytecodeArtifact(*Built.Artifact);
      ASSERT_TRUE(Bytes) << Bytes.Message;
      auto Loaded = deserializeBytecodeArtifact(Bytes.Bytes);
      ASSERT_TRUE(Loaded) << Loaded.Message;
      const auto *Description = Loaded.Artifact->Image.Layouts->find("types::Unused");
      ASSERT_NE(Description, nullptr);
      ASSERT_EQ(Description->classDesc().Fields.size(), 1U);
      EXPECT_EQ(Description->classDesc().Fields[0].Name, "Code");
    }
  }

  // Array and string field views retain payloads, reject out-of-range elements, and expire with their containing object.
  TEST(ReflectionTest, ProjectsArrayAndStringFields)
  {
    ArtifactContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *Strings = Test.Context.typePool().getType<ir::TypeKind::Array>(*String, 2);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Texts"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Items"), Strings}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "reflection::Texts"));
    const auto Type = Test.Bridge.lowerType(*Class);
    const auto StringType = Test.Bridge.lowerType(*String);
    const auto ArrayType = Test.Bridge.lowerType(*Strings);
    ExecutionMemoryManager Memory;
    Reflection Reflect(Test.Bridge.types(), Memory);
    const ReflectedValue Arguments[] = {Reflect.value(RuntimeValue::fromArray({RuntimeValue::fromString("first", StringType), RuntimeValue::fromString("second", StringType)}, ArrayType))};
    const auto Object = Reflect.construct(Type, Arguments);
    ASSERT_TRUE(Object);
    const auto Items = Reflect.field(Reflect.view(Object.Place).Value, "Items");
    ASSERT_TRUE(Items);
    const auto Element = Reflect.element(Items.Value, 1);
    ASSERT_TRUE(Element);
    EXPECT_EQ(Reflect.element(Items.Value, 2).Status, ExecutionStatus::IndexOutOfBounds);
    const auto Snapshot = Reflect.read(Element.Value);
    EXPECT_EQ(Reflect.write(Element.Value, Reflect.value(RuntimeValue::fromString("replacement", StringType))), ExecutionStatus::Success);
    EXPECT_EQ(Reflect.read(Element.Value).Value.string(), "replacement");
    EXPECT_EQ(Snapshot.Value.string(), "second");
    EXPECT_EQ(Memory.release(Object.Place), ExecutionStatus::Success);
    EXPECT_EQ(Reflect.read(Element.Value).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Snapshot.Value.string(), "second");
  }

  // Methods with the same name resolve by exact reflected argument types and reject mismatched arity.
  TEST(ReflectionTest, ResolvesMethodOverloads)
  {
    ReflectionProgram Test;
    ASSERT_TRUE(Test.build());
    const ir::Type *Parameters[] = {Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Test.Class, ir::AccessKind::ReadWrite), &Test.Bool};
    auto *Overload = Test.function("Counter.add", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Overload));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(99)), nullptr);
    ASSERT_TRUE(Test.Builder.setClassMethod(*Test.Class, *Overload));
    const auto Type = Test.Bridge.lowerType(*Test.Class);
    ExecutionEngine Engine(Test.Context);
    ExecutionLinker Linker(Test.Bridge);
    ExecutionMachine Machine(Engine, Linker);
    auto &Memory = Engine.heap().memoryManager();
    Reflection Reflect(Test.Bridge.types(), Memory, &Machine);
    InitializerCalls = 0;
    const auto Object = Reflect.construct(Type);
    ASSERT_TRUE(Object);
    const auto View = Reflect.view(Object.Place);
    const ReflectedValue Integer[] = {Reflect.value(RuntimeValue::fromBits(41, Test.Bridge.lowerType(Test.Int32)))};
    const ReflectedValue Boolean[] = {Reflect.value(RuntimeValue::fromBits(1, Test.Bridge.lowerType(Test.Bool)))};
    EXPECT_EQ(Reflect.invoke(View.Value, "add", Integer).Value.Bits, 42U);
    EXPECT_EQ(Reflect.invoke(View.Value, "add", Boolean).Value.Bits, 99U);
    EXPECT_EQ(Reflect.invoke(View.Value, "add", {}).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Memory.release(Object.Place), ExecutionStatus::Success);
  }

  // Duplicate nominal class definitions cannot disagree on field access permissions even when their storage layouts match.
  TEST(ReflectionTest, RejectsConflictingReflectedVisibility)
  {
    ArtifactContext Test;
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Record"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Value"), &Test.Int32}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "reflection::Record"));
    Test.Bridge.lowerType(*Class);
    auto Object = buildBytecodeObject("reflection", Test.Bridge, {});
    ASSERT_TRUE(Object);
    auto Types = std::const_pointer_cast<RuntimeTypeTable>(Object.Artifact->Image.Layouts);
    const auto Duplicate = Types->append(*Types->find("reflection::Record"));
    ASSERT_NE(Duplicate, InvalidRuntimeType);
    ASSERT_TRUE(validateBytecodeArtifact(*Object.Artifact));
    auto Description = std::make_shared<ClassDesc>(Types->get(Duplicate)->classDesc());
    Description->Fields[0].Visibility = MemberVisibility::Private;
    ASSERT_TRUE(Types->updateClass(Duplicate, std::move(Description)));
    EXPECT_FALSE(validateBytecodeArtifact(*Object.Artifact));
  }

  // Destroying a detached method or initializer removes its borrowed IR association before reflection resynchronizes.
  TEST(ReflectionTest, RemovesDestroyedMemberAssociations)
  {
    ReflectionProgram Test;
    ASSERT_TRUE(Test.build());
    const auto Type = Test.Bridge.lowerType(*Test.Class);
    ASSERT_EQ(Test.Bridge.types()->get(Type)->classDesc().Methods.size(), 1U);
    for (auto &Function : Test.Functions)
    {
      if (Function.get() == Test.Add || Function.get() == Test.Initial)
      {
        Function.reset();
      }
    }
    EXPECT_TRUE(Test.Class->methods().empty());
    EXPECT_EQ(Test.Class->fields()[0].Initializer, nullptr);
    Test.Bridge.synchronizeReflection();
    EXPECT_TRUE(Test.Bridge.types()->get(Type)->classDesc().Methods.empty());
    EXPECT_EQ(Test.Bridge.types()->get(Type)->classDesc().Fields[0].Initializer, InvalidFunction);
  }
} // namespace ink::execution::test
