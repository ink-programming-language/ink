#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"
#include "ink/ir/verifier.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct ModuleIrTestContext
    {
      core::CompilationContext Compilation;
      IRContext IR{Compilation};
    };

    Module makeVoidFunctionModule(IRContext &Context, std::unique_ptr<Instruction> InstructionValue)
    {
      Module ModuleValue(Context);
      const Type &VoidType = Context.getType(TypeKind::Void);
      Function FunctionValue(VoidType);
      FunctionValue.Name = "function";
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(InstructionValue));
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      FunctionValue.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(FunctionValue));
      return ModuleValue;
    }

    const std::string ModuleText =
        "inkir 1\n"
        "module 7\n"
        "initializer @init\n"
        "finalizer @fini\n"
        "\n"
        "@answer = global constant i32\n"
        "\n"
        "@counter = global mutable i32\n"
        "\n"
        "define void @init() {\n"
        "entry:\n"
        "  import 9\n"
        "  store i32 42, byte* @answer\n"
        "  store i32 0, byte* @counter\n"
        "  ret void\n"
        "}\n"
        "\n"
        "define void @fini() {\n"
        "entry:\n"
        "  ret void\n"
        "}\n"
        "\n"
        "define i32 @read_answer() {\n"
        "entry:\n"
        "  %0 = load i32, const byte* @answer\n"
        "  ret i32 %0\n"
        "}\n";

    const std::string QualifiedReferenceText =
        "inkir 1\n"
        "module 1\n"
        "initializer @init\n"
        "\n"
        "define void @init() {\n"
        "entry:\n"
        "  import 2\n"
        "  call void module(2, 3)()\n"
        "  %0 = load i32, const byte* global(2, 4)\n"
        "  ret void\n"
        "}\n";

    // Verifies that module, byte-constant, function, and global IDs remain distinct and that qualified references preserve both components.
    TEST(ModuleIrIdTest, EncapsulatesModuleQualifiedReferences)
    {
      constexpr ModuleId Module{3};
      constexpr ByteConstantId Constant{4};
      constexpr FunctionRef Function{Module, FunctionId{5}};
      constexpr GlobalRef Global{Module, GlobalId{6}};

      static_assert(!std::is_convertible_v<std::size_t, ModuleId>);
      static_assert(!std::is_convertible_v<std::size_t, ByteConstantId>);
      static_assert(Module.valid());
      static_assert(Constant.valid());
      static_assert(Function.valid());
      static_assert(Function.isQualified());
      static_assert(Global.valid());
      static_assert(Global.isQualified());
      static_assert(Function.Module == Module);
      static_assert(Function.Function == FunctionId{5});
      static_assert(Global.Module == Module);
      static_assert(Global.Global == GlobalId{6});
      static_assert(!FunctionRef{FunctionId{0}}.isQualified());
      static_assert(!GlobalRef{GlobalId{0}}.isQualified());
    }

    // Verifies that module identity, typed globals, lifecycle metadata, global addresses, and initializer imports survive canonical text round-tripping.
    TEST(ModuleIrSerializationTest, RoundTripsModuleLifecycleAndGlobals)
    {
      ModuleIrTestContext Context;
      DeserializeResult Parsed = deserialize(Context.IR, ModuleText);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      const Module &ModuleValue = *Parsed.module();
      EXPECT_EQ(ModuleValue.Id, ModuleId{7});
      ASSERT_EQ(ModuleValue.Globals.size(), 2U);
      EXPECT_EQ(ModuleValue.Globals[0].Name, "answer");
      EXPECT_EQ(ModuleValue.Globals[0].ValueType, &Context.IR.getType(TypeKind::I32));
      EXPECT_FALSE(ModuleValue.Globals[0].Mutable);
      EXPECT_TRUE(ModuleValue.Globals[1].Mutable);
      ASSERT_TRUE(ModuleValue.Initializer.has_value());
      ASSERT_TRUE(ModuleValue.Finalizer.has_value());
      EXPECT_EQ(*ModuleValue.Initializer, FunctionId{0});
      EXPECT_EQ(*ModuleValue.Finalizer, FunctionId{1});
      const Instruction &Import = *ModuleValue.Functions[0].Blocks[0].Instructions[0];
      ASSERT_EQ(Import.kind(), InstructionKind::Import);
      EXPECT_EQ(static_cast<const ImportInstruction &>(Import).Module, ModuleId{9});
      const Instruction &Store = *ModuleValue.Functions[0].Blocks[0].Instructions[1];
      ASSERT_EQ(Store.kind(), InstructionKind::Store);
      EXPECT_EQ(static_cast<const StoreInstruction &>(Store).Pointer->kind(), ValueKind::GlobalVariableAddressOperand);

      SerializeResult Serialized = serialize(Context.IR, ModuleValue);
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), ModuleText);
    }

    // Verifies that numeric module-qualified function and global references round-trip without requiring the referenced module image.
    TEST(ModuleIrSerializationTest, RoundTripsQualifiedReferences)
    {
      ModuleIrTestContext Context;
      DeserializeResult Parsed = deserialize(Context.IR, QualifiedReferenceText);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      const Function &Initializer = Parsed.module()->Functions[0];
      const auto &Call = static_cast<const CallInstruction &>(*Initializer.Blocks[0].Instructions[1]);
      EXPECT_EQ(Call.Callee, (FunctionRef{ModuleId{2}, FunctionId{3}}));
      const auto &Load = static_cast<const LoadInstruction &>(*Initializer.Blocks[0].Instructions[2]);
      const auto &Address = static_cast<const GlobalVariableAddressOperand &>(*Load.Pointer);
      EXPECT_EQ(Address.global(), (GlobalRef{ModuleId{2}, GlobalId{4}}));

      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), QualifiedReferenceText);
    }

    // Verifies that serialization preserves FunctionId values when external declarations and definitions are interleaved programmatically.
    TEST(ModuleIrSerializationTest, PreservesInterleavedFunctionOrder)
    {
      ModuleIrTestContext Context;
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      Module ModuleValue(Context.IR);
      Function First(VoidType);
      First.Name = "first";
      BasicBlock FirstEntry;
      FirstEntry.Name = "entry";
      FirstEntry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      First.Blocks.push_back(std::move(FirstEntry));
      ModuleValue.Functions.push_back(std::move(First));
      Function External(VoidType);
      External.Name = "external";
      External.Kind = FunctionKind::External;
      External.Convention = CallingConvention::C;
      ModuleValue.Functions.push_back(std::move(External));
      Function Last(VoidType);
      Last.Name = "last";
      BasicBlock LastEntry;
      LastEntry.Name = "entry";
      LastEntry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Last.Blocks.push_back(std::move(LastEntry));
      ModuleValue.Functions.push_back(std::move(Last));
      ModuleValue.Initializer = FunctionId{2};

      const SerializeResult Serialized = serialize(Context.IR, ModuleValue);
      ASSERT_TRUE(Serialized.succeeded());
      const DeserializeResult Parsed = deserialize(Context.IR, *Serialized.text());

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->Functions.size(), 3U);
      EXPECT_EQ(Parsed.module()->Functions[0].Name, "first");
      EXPECT_EQ(Parsed.module()->Functions[1].Name, "external");
      EXPECT_EQ(Parsed.module()->Functions[2].Name, "last");
      EXPECT_EQ(Parsed.module()->Initializer, FunctionId{2});
    }

    // Verifies that the reserved InvalidId value is rejected in every explicit textual module-qualified ID position.
    TEST(ModuleIrSerializationTest, RejectsReservedInvalidIds)
    {
      ModuleIrTestContext Context;
      const std::string Invalid = std::to_string(std::numeric_limits<std::size_t>::max());
      const std::vector<std::string> Texts = {
          "inkir 1\nmodule " + Invalid + "\n",
          "inkir 1\nmodule 1\ninitializer @init\ndefine void @init() {\nentry:\n  import " + Invalid + "\n  ret void\n}\n",
          "inkir 1\nmodule 1\ndefine void @main() {\nentry:\n  call void module(" + Invalid + ", 0)()\n  ret void\n}\n",
          "inkir 1\nmodule 1\ndefine void @main() {\nentry:\n  call void module(2, " + Invalid + ")()\n  ret void\n}\n",
          "inkir 1\nmodule 1\ndefine i32 @main() {\nentry:\n  %0 = load i32, const byte* global(" + Invalid + ", 0)\n  ret i32 %0\n}\n",
          "inkir 1\nmodule 1\ndefine i32 @main() {\nentry:\n  %0 = load i32, const byte* global(2, " + Invalid + ")\n  ret i32 %0\n}\n",
      };

      for (const std::string &Text : Texts)
      {
        SCOPED_TRACE(Text);
        const DeserializeResult Parsed = deserialize(Context.IR, Text);
        ASSERT_FALSE(Parsed.succeeded());
        ASSERT_EQ(Parsed.diagnostics().size(), 1U);
        EXPECT_EQ(Parsed.diagnostics()[0].Kind, core::DiagnosticKind::IrNumericValueOutOfRange);
      }
    }

    // Verifies that import remains a non-terminating instruction but is rejected outside the module initializer.
    TEST(ModuleIrVerifierTest, RejectsImportOutsideInitializer)
    {
      ModuleIrTestContext Context;
      Module ModuleValue = makeVoidFunctionModule(Context.IR, std::make_unique<ImportInstruction>(ModuleId{1}));

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
      EXPECT_FALSE(isTerminator(InstructionKind::Import));
    }

    // Verifies that adding module runtime metadata does not invalidate an otherwise empty IR module.
    TEST(ModuleIrVerifierTest, AcceptsEmptyModule)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);

      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
    }

    // Verifies that lifecycle metadata accepts only local void functions without parameters.
    TEST(ModuleIrVerifierTest, RejectsInvalidInitializerSignature)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);
      Function Initializer(Context.IR.getType(TypeKind::I32));
      Initializer.Name = "init";
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Initializer.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Initializer));
      ModuleValue.Initializer = FunctionId{0};

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
    }

    // Verifies that ordinary IR calls cannot directly invoke lifecycle functions reserved for the module loader.
    TEST(ModuleIrVerifierTest, RejectsDirectInitializerCall)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      Function Initializer(VoidType);
      Initializer.Name = "init";
      BasicBlock InitializerEntry;
      InitializerEntry.Name = "entry";
      InitializerEntry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Initializer.Blocks.push_back(std::move(InitializerEntry));
      ModuleValue.Functions.push_back(std::move(Initializer));
      auto Call = std::make_unique<CallInstruction>(VoidType);
      Call->Callee = FunctionId{0};
      Function Main(VoidType);
      Main.Name = "main";
      BasicBlock MainEntry;
      MainEntry.Name = "entry";
      MainEntry.Instructions.push_back(std::move(Call));
      MainEntry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Main.Blocks.push_back(std::move(MainEntry));
      ModuleValue.Functions.push_back(std::move(Main));
      ModuleValue.Initializer = FunctionId{0};

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
    }

    // Verifies that immutable globals expose mutable storage only to their owning module initializer.
    TEST(ModuleIrVerifierTest, RestrictsImmutableGlobalMutationToInitializer)
    {
      ModuleIrTestContext Context;
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &BytePointerType = Context.IR.getType(TypeKind::BytePointer);
      auto Store = std::make_unique<StoreInstruction>();
      Store->StoredValue = std::make_unique<IntegerConstant>(I32Type, 1);
      Store->Pointer = std::make_unique<GlobalVariableAddressOperand>(BytePointerType, GlobalRef{GlobalId{0}});
      Module ModuleValue = makeVoidFunctionModule(Context.IR, std::move(Store));
      ModuleValue.Globals.push_back({"value", &I32Type, false});

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
      ModuleValue.Initializer = FunctionId{0};
      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
    }
  } // namespace
} // namespace ink::ir
