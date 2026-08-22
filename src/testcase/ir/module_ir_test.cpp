#include "ink/ir/analysis/verifier.h"
#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"

#include "ir_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace ink::ir
{
  namespace
  {
    using ModuleIrTestContext = test::IRTestContext;
    using ink::test::hasDiagnostic;

    // Verifies that serialized module identities require a root package, non-empty identifier segments, and canonical dot separators.
    TEST(ModuleNameTest, ValidatesCanonicalModuleNames)
    {
      EXPECT_TRUE(isValidModuleName("game.main"));
      EXPECT_TRUE(isValidModuleName("game.graphics.window"));
      EXPECT_TRUE(isValidModuleName("_game._window2"));
      EXPECT_TRUE(isValidModuleName(u8"\u5E94\u7528.\u4E3B\u00B7\u6A21\u57572"));
      EXPECT_FALSE(isValidModuleName(""));
      EXPECT_FALSE(isValidModuleName("main"));
      EXPECT_FALSE(isValidModuleName(".main"));
      EXPECT_FALSE(isValidModuleName("game."));
      EXPECT_FALSE(isValidModuleName("game..main"));
      EXPECT_FALSE(isValidModuleName("game.$main"));
      EXPECT_FALSE(isValidModuleName(u8"game.\u00B7window"));
    }

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
        "module application.main\n"
        "initializer @init\n"
        "finalizer @fini\n"
        "\n"
        "@answer = global constant i32\n"
        "\n"
        "@counter = global mutable i32\n"
        "\n"
        "define void @init() {\n"
        "entry:\n"
        "  import dependency.runtime\n"
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

    const std::string ImportedReferenceText =
        "inkir 1\n"
        "module application.main\n"
        "initializer @init\n"
        "\n"
        "declare import global constant i32 @dependency.answer from module dependency.api, symbol @answer\n"
        "\n"
        "declare import void @dependency.hook() from module dependency.api, symbol @hook\n"
        "\n"
        "define void @init() {\n"
        "entry:\n"
        "  import dependency.api\n"
        "  call void @dependency.hook()\n"
        "  %0 = load i32, const byte* @dependency.answer\n"
        "  ret void\n"
        "}\n";

    // Verifies that byte-constant, function, and global IDs remain distinct strongly typed values after module identity moves to names.
    TEST(ModuleIrIdTest, EncapsulatesValueIds)
    {
      constexpr ByteConstantId Constant{4};
      constexpr FunctionId Function{5};
      constexpr GlobalId Global{6};

      static_assert(Constant.valid());
      static_assert(Function.valid());
      static_assert(Global.valid());
      static_assert(Function == FunctionId{5});
      static_assert(Global == GlobalId{6});
    }

    // Verifies that modules resolve function and global names to their stable typed IDs and report absent symbols.
    TEST(ModuleIrLookupTest, FindsNamedSymbols)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);
      Function First(Context.IR.getType(TypeKind::Void));
      First.Name = "first";
      ModuleValue.Functions.push_back(std::move(First));
      Function Second(Context.IR.getType(TypeKind::Void));
      Second.Name = "second";
      ModuleValue.Functions.push_back(std::move(Second));
      GlobalVariable Answer;
      Answer.Name = "answer";
      Answer.ValueType = &Context.IR.getType(TypeKind::I32);
      ModuleValue.Globals.push_back(std::move(Answer));

      ASSERT_TRUE(ModuleValue.findFunction("second").has_value());
      EXPECT_EQ(*ModuleValue.findFunction("second"), FunctionId{1});
      EXPECT_FALSE(ModuleValue.findFunction("missing").has_value());
      ASSERT_TRUE(ModuleValue.findGlobal("answer").has_value());
      EXPECT_EQ(*ModuleValue.findGlobal("answer"), GlobalId{0});
      EXPECT_FALSE(ModuleValue.findGlobal("missing").has_value());
    }

    // Verifies that module identity, typed globals, lifecycle metadata, global addresses, and initializer imports survive canonical text round-tripping.
    TEST(ModuleIrSerializationTest, RoundTripsModuleLifecycleAndGlobals)
    {
      ModuleIrTestContext Context;
      DeserializeResult Parsed = deserialize(Context.IR, ModuleText);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      const Module &ModuleValue = *Parsed.module();
      ASSERT_TRUE(ModuleValue.Name.has_value());
      EXPECT_EQ(*ModuleValue.Name, "application.main");
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
      EXPECT_EQ(static_cast<const ImportInstruction &>(Import).Module, "dependency.runtime");
      const Instruction &Store = *ModuleValue.Functions[0].Blocks[0].Instructions[1];
      ASSERT_EQ(Store.kind(), InstructionKind::Store);
      EXPECT_EQ(static_cast<const StoreInstruction &>(Store).Pointer->kind(), ValueKind::GlobalVariableAddressOperand);

      SerializeResult Serialized = serialize(Context.IR, ModuleValue);
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), ModuleText);
    }

    // Verifies that named imported function and global declarations round-trip while their uses remain ordinary local symbol references.
    TEST(ModuleIrSerializationTest, RoundTripsImportedFunctionAndGlobalReferences)
    {
      ModuleIrTestContext Context;
      DeserializeResult Parsed = deserialize(Context.IR, ImportedReferenceText);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->Globals.size(), 1U);
      const GlobalVariable &ImportedGlobal = Parsed.module()->Globals[0];
      EXPECT_EQ(ImportedGlobal.Kind, GlobalVariableKind::Imported);
      EXPECT_EQ(ImportedGlobal.Name, "dependency.answer");
      ASSERT_TRUE(ImportedGlobal.Import.has_value());
      EXPECT_EQ(ImportedGlobal.Import->Module, "dependency.api");
      EXPECT_EQ(ImportedGlobal.Import->Symbol, "answer");
      EXPECT_FALSE(ImportedGlobal.Mutable);
      ASSERT_EQ(Parsed.module()->Functions.size(), 2U);
      const Function &Import = Parsed.module()->Functions[0];
      EXPECT_EQ(Import.Kind, FunctionKind::Imported);
      EXPECT_EQ(Import.Name, "dependency.hook");
      ASSERT_TRUE(Import.Import.has_value());
      EXPECT_EQ(Import.Import->Module, "dependency.api");
      EXPECT_EQ(Import.Import->Symbol, "hook");
      const Function &Initializer = Parsed.module()->Functions[1];
      const auto &Call = static_cast<const CallInstruction &>(*Initializer.Blocks[0].Instructions[1]);
      EXPECT_EQ(Call.Callee, FunctionId{0});
      const auto &Load = static_cast<const LoadInstruction &>(*Initializer.Blocks[0].Instructions[2]);
      const auto &Address = static_cast<const GlobalVariableAddressOperand &>(*Load.Pointer);
      EXPECT_EQ(Address.global(), GlobalId{0});

      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), ImportedReferenceText);
    }

    // Verifies that call accepts only a declared local symbol and no longer embeds a module/function numeric pair.
    TEST(ModuleIrSerializationTest, RejectsLegacyQualifiedCallSyntax)
    {
      ModuleIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "module application.main\n"
          "\n"
          "define void @main() {\n"
          "entry:\n"
          "  call void module(2, 0)()\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Parsed = deserialize(Context.IR, Text);

      ASSERT_FALSE(Parsed.succeeded());
      ASSERT_EQ(Context.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Context.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::IrExpected);
    }

    // Verifies that global operands no longer accept an embedded module/global numeric pair.
    TEST(ModuleIrSerializationTest, RejectsLegacyQualifiedGlobalSyntax)
    {
      ModuleIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "module application.main\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = load i32, const byte* global(2, 0)\n"
          "  ret i32 %0\n"
          "}\n";

      const DeserializeResult Parsed = deserialize(Context.IR, Text);

      ASSERT_FALSE(Parsed.succeeded());
      ASSERT_EQ(Context.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Context.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::IrExpectedOperand);
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

    // Verifies that numeric module identities are rejected everywhere now that canonical module names are serialized directly.
    TEST(ModuleIrSerializationTest, RejectsNumericModuleIdentities)
    {
      ModuleIrTestContext Context;
      const std::vector<std::string> Texts = {
          "inkir 1\nmodule 1\n",
          "inkir 1\nmodule application.main\ninitializer @init\ndefine void @init() {\nentry:\n  import 2\n  ret void\n}\n",
          "inkir 1\nmodule application.main\ndeclare import void @dependency.run() from module 2, symbol @run\n",
          "inkir 1\nmodule application.main\ndeclare import global constant i32 @dependency.answer from module 2, symbol @answer\n",
      };

      for (const std::string &Text : Texts)
      {
        SCOPED_TRACE(Text);
        Context.Diagnostics.clear();
        const DeserializeResult Parsed = deserialize(Context.IR, Text);
        ASSERT_FALSE(Parsed.succeeded());
        ASSERT_EQ(Context.Diagnostics.diagnostics().size(), 1U);
        EXPECT_EQ(Context.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::IrExpected);
      }
    }

    // Verifies that import remains non-terminating and is valid in an ordinary function for runtime module loading.
    TEST(ModuleIrVerifierTest, AcceptsImportInOrdinaryFunction)
    {
      ModuleIrTestContext Context;
      Module ModuleValue = makeVoidFunctionModule(Context.IR, std::make_unique<ImportInstruction>("dependency.runtime"));

      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
      EXPECT_FALSE(isTerminator(InstructionKind::Import));
    }

    // Verifies that finalization cannot start a new dynamic module load while the loader is shutting down.
    TEST(ModuleIrVerifierTest, RejectsImportInFinalizer)
    {
      ModuleIrTestContext Context;
      Module ModuleValue = makeVoidFunctionModule(Context.IR, std::make_unique<ImportInstruction>("dependency.runtime"));
      ModuleValue.Finalizer = FunctionId{0};

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
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

    // Verifies that imported functions require the Ink convention, no body, a different valid module, and a valid target symbol name.
    TEST(ModuleIrVerifierTest, RejectsInvalidImportedFunctionMetadata)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);
      ModuleValue.Name = "application.main";
      Function Import(Context.IR.getType(TypeKind::Void));
      Import.Name = "dependency.run";
      Import.Kind = FunctionKind::Imported;
      Import.Convention = CallingConvention::C;
      Import.Import = ImportInfo{"application.main", ink::ir::Name{}};
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Import.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Import));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedFunctionWrongCallingConvention));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedFunctionHasBasicBlocks));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedFunctionInvalidModule));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedFunctionInvalidTargetName));
    }

    // Verifies that imported globals must name a valid symbol in a different valid module.
    TEST(ModuleIrVerifierTest, RejectsInvalidImportedGlobalMetadata)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);
      ModuleValue.Name = "application.main";
      GlobalVariable Import;
      Import.Name = "dependency.answer";
      Import.ValueType = &Context.IR.getType(TypeKind::I32);
      Import.Kind = GlobalVariableKind::Imported;
      Import.Import = ImportInfo{"application.main", ink::ir::Name{}};
      ModuleValue.Globals.push_back(std::move(Import));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedGlobalInvalidModule));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedGlobalInvalidTargetName));
    }

    // Verifies that imported functions and globals cannot omit their complete import information.
    TEST(ModuleIrVerifierTest, RejectsMissingImportInfo)
    {
      ModuleIrTestContext Context;
      Module ModuleValue(Context.IR);
      ModuleValue.Name = "application.main";
      Function ImportedFunction(Context.IR.getType(TypeKind::Void));
      ImportedFunction.Name = "dependency.run";
      ImportedFunction.Kind = FunctionKind::Imported;
      ModuleValue.Functions.push_back(std::move(ImportedFunction));
      GlobalVariable ImportedGlobal;
      ImportedGlobal.Name = "dependency.answer";
      ImportedGlobal.ValueType = &Context.IR.getType(TypeKind::I32);
      ImportedGlobal.Kind = GlobalVariableKind::Imported;
      ModuleValue.Globals.push_back(std::move(ImportedGlobal));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedFunctionInvalidModule));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedFunctionInvalidTargetName));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedGlobalInvalidModule));
      EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrImportedGlobalInvalidTargetName));
    }

    // Verifies that definitions reject import information that would otherwise be discarded during serialization.
    TEST(ModuleIrVerifierTest, RejectsImportInfoOnDefinitions)
    {
      ModuleIrTestContext Context;
      Module ModuleValue = makeVoidFunctionModule(Context.IR, std::make_unique<ImportInstruction>("dependency.runtime"));
      ModuleValue.Functions[0].Import = ImportInfo{"dependency.api", "run"};
      GlobalVariable Global;
      Global.Name = "answer";
      Global.ValueType = &Context.IR.getType(TypeKind::I32);
      Global.Import = ImportInfo{"dependency.api", "answer"};
      ModuleValue.Globals.push_back(std::move(Global));

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
      ModuleValue.Functions[0].Import.reset();
      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
      ModuleValue.Globals[0].Import.reset();
      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
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
      Store->StoredValue = Context.IR.constantPool().getIntegerConstant(I32Type, 1);
      Store->Pointer = std::make_unique<GlobalVariableAddressOperand>(BytePointerType, GlobalId{0});
      Module ModuleValue = makeVoidFunctionModule(Context.IR, std::move(Store));
      ModuleValue.Globals.push_back({"value", &I32Type, false});

      EXPECT_FALSE(verify(Context.IR, ModuleValue).succeeded());
      ModuleValue.Initializer = FunctionId{0};
      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
    }
  } // namespace
} // namespace ink::ir
