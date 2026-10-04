#include "ink/ir/module/module_serialization.h"
#include "ink/ir/ir_builder.h"
#include "ink/parser/ast_walker.h"

#include <gtest/gtest.h>
#include <llvm/Support/Endian.h>

#include <array>

namespace ink::ir::test
{
  namespace
  {
    constexpr std::string_view EmptyModuleText = R"(ink-ir 7
module @"Example" {
}
)";

    Module *makeModule(IRContext &Context)
    {
      IRBuilder Builder(Context);
      auto &Types = Context.typePool();
      auto &Constants = Context.constantPool();
      auto &Names = Context.namePool();
      auto *Root = Builder.createModule(Names.intern("Example\"\\\n\t\r\x01\xe4\xb8\xad"));
      const auto *Integer = Types.getType<TypeKind::Integer>(129, true);
      const auto *Unsigned = Types.getType<TypeKind::Integer>(8, false);
      const auto *Slice = Types.getType<TypeKind::Slice>(*Unsigned, AccessKind::ReadOnly);
      const auto *Signature = Types.getType<TypeKind::Function>(*Integer, std::array<const Type *, 1>{Integer});
      const auto *MainSignature = Types.getType<TypeKind::Function>(*Integer, std::array<const Type *, 3>{Integer, Signature, Slice});
      const ParameterKind Kinds[] = {ParameterKind::Positional, ParameterKind::Named, ParameterKind::Variadic};
      const Name ParameterNames[] = {Names.intern("Input"), Names.intern("Callback"), {}};
      auto MainOwner = Builder.createFunction(Names.intern("Main"), *MainSignature, Kinds, ParameterNames, CallingConvention::Fast, LanguageLinkage::Ink);
      auto TargetOwner = Builder.createFunction(Names.intern("Target"), *Signature, {}, {}, CallingConvention::Cold, LanguageLinkage::C);
      auto *Main = MainOwner.get();
      auto *Target = TargetOwner.get();
      if (!Main || !Builder.setFunctionVisibility(*Main, core::VisibilityKind::Private))
      {
        return nullptr;
      }
      if (!Builder.appendValue(Root->entryBlock(), std::move(MainOwner)) || !Builder.appendValue(Root->entryBlock(), std::move(TargetOwner)))
      {
        return nullptr;
      }
      auto *Entry = Builder.createBasicBlock(*Main);
      Builder.setInsertPoint(*Entry);
      const std::uint64_t Words[] = {UINT64_MAX, 0x123456789abcdef0, 1};
      const auto *Wide = Constants.getIntegerConstant(*Integer, IntegerBits(129, Words));
      const auto *Slot = Builder.createAllocaInstruction(*Integer);
      const auto *Store = Builder.createStoreInstruction(*Slot, *Main->parameters()[0]);
      const auto *Load = Builder.createLoadInstruction(*Slot);
      const auto *Sum = Builder.createAddInstruction(*Load, *Wide);
      const auto *Direct = Builder.createCallInstruction(*Target, std::array<const Value *, 1>{Sum});
      const auto *Indirect = Builder.createCallInstruction(*Main->parameters()[1], std::array<const Value *, 1>{Direct});
      const auto *String = Constants.getStringConstant(*Slice, "hello\n\"\\\xe4\xb8\xad");
      const auto *CString = Builder.createCStringInstruction(*String);
      const auto *Float16 = Types.getType<TypeKind::Float>(16);
      const auto *Float32 = Types.getType<TypeKind::Float>(32);
      const auto *Float64 = Types.getType<TypeKind::Float>(64);
      const auto *Class = Types.createClassType(Names.intern("SameName"));
      const auto *OtherClass = Types.createClassType(Names.intern("SameName"));
      const auto *Enum = Types.createEnumType(Names.intern("Enum"));
      const auto *Interface = Types.createInterfaceType(Names.intern("Interface"));
      const auto *Array = Types.getType<TypeKind::Array>(*Unsigned, UINT64_MAX);
      const auto *Pointer = Types.getType<TypeKind::Pointer>(*Class, AccessKind::ReadOnly);
      const auto *Reference = Types.getType<TypeKind::Reference>(*Enum, AccessKind::ReadWrite);
      const auto *WritableSlice = Types.getType<TypeKind::Slice>(*Unsigned, AccessKind::ReadWrite);
      const auto *NulString = Constants.getStringConstant(*Slice, std::string_view("a\0b", 3));
      const Value *Arguments[] = {
          Constants.getFloatConstant(*Float16, FloatBits(16, 0x8000)),
          Constants.getFloatConstant(*Float32, FloatBits(32, 0x7fc12345)),
          Constants.getFloatConstant(*Float64, FloatBits(64, 0xfff0000000000000)),
          &Constants.getBoolConstant(true),
          &Constants.getBoolConstant(false),
          NulString,
          Class,
          OtherClass,
          Enum,
          Interface,
          Array,
          Pointer,
          Reference,
          WritableSlice,
          &Types.getType<TypeKind::Meta>(),
      };
      std::vector<const Type *> ArgumentTypes;
      for (const auto *Argument : Arguments)
      {
        ArgumentTypes.push_back(&Argument->type());
      }
      const auto *SinkSignature = Types.getType<TypeKind::Function>(Types.getType<TypeKind::Void>(), ArgumentTypes);
      auto SinkOwner = Builder.createFunction(Names.intern("Sink"), *SinkSignature);
      const auto *Sink = SinkOwner.get();
      if (!Builder.appendValue(Root->entryBlock(), std::move(SinkOwner)))
      {
        return nullptr;
      }
      const auto *SinkCall = Builder.createCallInstruction(*Sink, Arguments);
      const auto *Return = Builder.createReturnInstruction(Indirect);
      auto *SecondBlock = Builder.createBasicBlock(*Main);
      Builder.setInsertPoint(*SecondBlock);
      const auto *SecondReturn = Builder.createReturnInstruction(Main->parameters()[0].get());
      // Target's recursive call refers to its own already-created function identity.
      Builder.setInsertPoint(*Builder.createBasicBlock(*Target));
      const auto *Recursive = Builder.createCallInstruction(*Target, std::array<const Value *, 1>{Target->parameters()[0].get()});
      const auto *RecursiveReturn = Builder.createReturnInstruction(Recursive);
      auto *Nested = Builder.createModule(Names.intern("Nested"));
      auto NestedOwner = Builder.removeModule(*Nested);
      if (!Builder.appendValue(Root->entryBlock(), std::move(NestedOwner)) || !Builder.appendValue(Nested->entryBlock(), Builder.createBasicBlock()))
      {
        return nullptr;
      }
      return Store && CString && SinkCall && Return && SecondReturn && RecursiveReturn ? Root : nullptr;
    }

    void checkModule(const Module &Root)
    {
      const auto &Values = Root.entryBlock().values();
      ASSERT_EQ(Values.size(), 4U);
      const auto &Main = static_cast<const Function &>(*Values[0]);
      const auto &Target = static_cast<const Function &>(*Values[1]);
      EXPECT_EQ(Main.callingConvention(), CallingConvention::Fast);
      EXPECT_EQ(Target.callingConvention(), CallingConvention::Cold);
      EXPECT_EQ(Target.languageLinkage(), LanguageLinkage::C);
      EXPECT_EQ(Main.visibility(), core::VisibilityKind::Private);
      EXPECT_EQ(Target.visibility(), core::VisibilityKind::Public);
      ASSERT_EQ(Main.parameters().size(), 3U);
      EXPECT_EQ(Main.parameters()[1]->parameterKind(), ParameterKind::Named);
      EXPECT_EQ(Main.parameters()[2]->parameterKind(), ParameterKind::Variadic);
      EXPECT_FALSE(Main.parameters()[2]->name().valid());
      EXPECT_EQ(Root.context().namePool().text(Main.parameters()[0]->name()), "Input");
      ASSERT_EQ(Main.blocks().size(), 2U);
      EXPECT_EQ(Main.blocks()[1]->outer(), &Main);
      const auto &Instructions = Main.entryBlock()->values();
      ASSERT_EQ(Instructions.size(), 9U);
      const auto &Store = static_cast<const StoreInstruction &>(*Instructions[1]);
      EXPECT_EQ(&Store.address(), Instructions[0].get());
      EXPECT_EQ(&Store.storedValue(), Main.parameters()[0].get());
      const auto &Sum = static_cast<const AddInstruction &>(*Instructions[3]);
      const auto &Wide = static_cast<const IntegerConstant &>(Sum.right());
      EXPECT_EQ(Wide.value().bitWidth(), 129U);
      ASSERT_EQ(Wide.value().words().size(), 3U);
      EXPECT_EQ(Wide.value().words()[0], UINT64_MAX);
      EXPECT_EQ(Wide.value().words()[1], 0x123456789abcdef0ULL);
      EXPECT_EQ(Wide.value().words()[2], 1U);
      EXPECT_EQ(static_cast<const CallInstruction &>(*Instructions[4]).directCallee(), &Target);
      EXPECT_EQ(static_cast<const CallInstruction &>(*Instructions[5]).indirectCallee(), Main.parameters()[1].get());
      const auto &Sink = static_cast<const CallInstruction &>(*Instructions[7]);
      EXPECT_EQ(static_cast<const FloatConstant *>(Sink.arguments()[0])->value().bits(), 0x8000U);
      EXPECT_EQ(static_cast<const FloatConstant *>(Sink.arguments()[1])->value().bits(), 0x7fc12345U);
      EXPECT_EQ(static_cast<const FloatConstant *>(Sink.arguments()[2])->value().bits(), 0xfff0000000000000ULL);
      EXPECT_EQ(static_cast<const StringConstant *>(Sink.arguments()[5])->value(), std::string_view("a\0b", 3));
      EXPECT_NE(Sink.arguments()[6], Sink.arguments()[7]);
      EXPECT_EQ(static_cast<const UserDefinedType *>(Sink.arguments()[6])->name(), static_cast<const UserDefinedType *>(Sink.arguments()[7])->name());
      EXPECT_EQ(static_cast<const ReturnInstruction &>(*Instructions[8]).returnedValue(), Instructions[5].get());
      EXPECT_EQ(static_cast<const CallInstruction &>(*Target.entryBlock()->values()[0]).directCallee(), &Target);
      const auto &Nested = static_cast<const Module &>(*Values[3]);
      EXPECT_EQ(Nested.outer(), &Root.entryBlock());
      EXPECT_EQ(Nested.entryBlock().values()[0]->outer(), &Nested.entryBlock());
    }
  } // namespace

  // Both encodings preserve every supported kind, exact constant bits, aliases, metadata and nested ownership.
  TEST(IRModuleSerializationTest, CompleteGraphRoundTripsThroughFreshContexts)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto *Root = makeModule(Source);
    ASSERT_NE(Root, nullptr);
    for (bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Root) : serializeModuleText(*Root);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      Destination.namePool().intern("Unrelated existing name");
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes) : deserializeModuleText(Destination, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      ASSERT_EQ(Destination.modules().size(), 1U);
      EXPECT_EQ(Decoded.ModuleValue, Destination.modules()[0].get());
      EXPECT_EQ(&Decoded.ModuleValue->context(), &Destination);
      EXPECT_EQ(Decoded.ModuleValue->declarationRoot(), nullptr);
      checkModule(*Decoded.ModuleValue);
      const auto Reencoded = Binary ? serializeModuleBinary(*Decoded.ModuleValue) : serializeModuleText(*Decoded.ModuleValue);
      ASSERT_TRUE(Reencoded.succeeded()) << Reencoded.Message;
      EXPECT_EQ(Encoded.Bytes, Reencoded.Bytes);
    }
  }

  // The documented text records can be authored manually and edited without any binary payload.
  TEST(IRModuleSerializationTest, ReadsHandwrittenTextAndPreservesEditedNames)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    auto Text = std::string(EmptyModuleText);
    Text.replace(Text.find("Example"), 7, "Edited\\x00Name");
    const auto Result = deserializeModuleText(Context, Text);
    ASSERT_TRUE(Result.succeeded()) << Result.Message;
    EXPECT_EQ(Context.namePool().text(Result.ModuleValue->name()), std::string_view("Edited\0Name", 11));
    EXPECT_TRUE(Result.ModuleValue->entryBlock().values().empty());
    const auto Encoded = serializeModuleText(*Result.ModuleValue);
    ASSERT_TRUE(Encoded.succeeded());
    EXPECT_NE(Encoded.Bytes.find("Edited\\x00Name"), std::string::npos);
  }

  // The documented addOne example fixes readable assembly and verifies a nonempty module's text round trip.
  TEST(IRModuleSerializationTest, SerializesDocumentedFunctionExample)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Example"));
    ASSERT_NE(Root, nullptr);
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Integer, nullptr);
    const Type *ParameterTypes[] = {Integer};
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(*Integer, ParameterTypes);
    ASSERT_NE(Signature, nullptr);
    const Name ParameterNames[] = {Context.namePool().intern("x")};
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("addOne"), *Signature, {}, ParameterNames);
    auto *FunctionValue = FunctionOwner.get();
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_TRUE(Builder.appendValue(Root->entryBlock(), std::move(FunctionOwner)));
    auto *Body = Builder.createFunctionBody(*FunctionValue);
    ASSERT_NE(Body, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Body));
    const auto *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    ASSERT_NE(One, nullptr);
    const auto *Sum = Builder.createAddInstruction(*FunctionValue->parameters()[0], *One);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Builder.createReturnInstruction(Sum), nullptr);
    constexpr std::string_view Expected = R"(ink-ir 7
module @Example {
  define i32 @addOne(i32 %x) {
  entry:
    %v0 = add i32 %x, 1
    ret i32 %v0
  }
}
)";
    const auto Encoded = serializeModuleText(*Root);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    EXPECT_EQ(Encoded.Bytes, Expected);
    IRContext Destination(Compilation);
    const auto Decoded = deserializeModuleText(Destination, Expected);
    ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
    EXPECT_EQ(serializeModuleText(*Decoded.ModuleValue).Bytes, Encoded.Bytes);
  }

  // Explicit public/private text annotations and default public visibility survive both module archive formats.
  TEST(IRModuleSerializationTest, PreservesFunctionVisibility)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, "ink-ir 7 module @Visibility { declare void @Default() declare void @Public() visibility public declare void @Private() visibility private }");
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (bool Binary : {false, true})
    {
      const auto Saved = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
      IRContext Destination(Compilation);
      const auto Loaded = Binary ? deserializeModuleBinary(Destination, Saved.Bytes) : deserializeModuleText(Destination, Saved.Bytes);
      ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
      const auto &Values = Loaded.ModuleValue->entryBlock().values();
      ASSERT_EQ(Values.size(), 3U);
      EXPECT_EQ(static_cast<const Function &>(*Values[0]).visibility(), core::VisibilityKind::Public);
      EXPECT_EQ(static_cast<const Function &>(*Values[1]).visibility(), core::VisibilityKind::Public);
      EXPECT_EQ(static_cast<const Function &>(*Values[2]).visibility(), core::VisibilityKind::Private);
      const auto Canonical = serializeModuleText(*Loaded.ModuleValue);
      ASSERT_TRUE(Canonical.succeeded()) << Canonical.Message;
      EXPECT_NE(Canonical.Bytes.find("@Private() visibility private"), std::string::npos);
      EXPECT_EQ(Canonical.Bytes.find("visibility public"), std::string::npos);
    }
  }

  // Unknown or missing visibility fields and old formats are rejected before a restored module can be published.
  TEST(IRModuleSerializationTest, RejectsMalformedVisibilityAndLegacyVersions)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, "ink-ir 7 module @Visibility { declare void @Private() visibility private }");
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Saved = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
    std::size_t Offset = 16;
    bool Found = false;
    while (Offset < Saved.Bytes.size())
    {
      const char *Header = Saved.Bytes.data() + Offset;
      const auto Fields = llvm::support::endian::read32le(Header + 12);
      const auto TextSize = llvm::support::endian::read64le(Header + 16);
      if (llvm::support::endian::read32le(Header) == 36)
      {
        ASSERT_EQ(Fields, 4U);
        auto Invalid = Saved.Bytes;
        llvm::support::endian::write64le(Invalid.data() + Offset + 48, 99);
        EXPECT_EQ(deserializeModuleBinary(Context, Invalid).Status, core::ArchiveStatus::InvalidArchive);
        Invalid = Saved.Bytes;
        llvm::support::endian::write32le(Invalid.data() + Offset + 12, 3);
        Invalid.erase(Offset + 48, 8);
        EXPECT_EQ(deserializeModuleBinary(Context, Invalid).Status, core::ArchiveStatus::InvalidArchive);
        Found = true;
        break;
      }
      Offset += 32 + static_cast<std::size_t>(Fields) * 8 + static_cast<std::size_t>(TextSize) + static_cast<std::size_t>(llvm::support::endian::read64le(Header + 24));
    }
    ASSERT_TRUE(Found);
    auto Legacy = Saved.Bytes;
    llvm::support::endian::write32le(Legacy.data() + 4, 2);
    EXPECT_EQ(deserializeModuleBinary(Context, Legacy).Status, core::ArchiveStatus::UnsupportedVersion);
    EXPECT_EQ(deserializeModuleText(Context, "ink-ir 2 module @Legacy {}").Status, core::ArchiveStatus::UnsupportedVersion);
    llvm::support::endian::write32le(Legacy.data() + 4, 3);
    EXPECT_EQ(deserializeModuleBinary(Context, Legacy).Status, core::ArchiveStatus::UnsupportedVersion);
    EXPECT_EQ(deserializeModuleText(Context, "ink-ir 3 module @Legacy {}").Status, core::ArchiveStatus::UnsupportedVersion);
    for (std::string_view Visibility : {"unknown", "private visibility public"})
    {
      const auto Invalid = deserializeModuleText(Context, "ink-ir 7 module @Invalid { declare void @f() visibility " + std::string(Visibility) + " }");
      EXPECT_EQ(Invalid.Status, core::ArchiveStatus::InvalidArchive);
    }
    EXPECT_EQ(Context.modules().size(), 1U);
  }

  // Native direction round-trips independently of source visibility and C ABI callbacks in both archive formats.
  TEST(IRModuleSerializationTest, PreservesNativeBindingsAndPrivateExports)
  {
    constexpr std::string_view Text = R"(ink-ir 7
module @Native {
  declare void @Imported() linkage c visibility private binding import
  define void @Callback() linkage c binding local {
  callback_entry:
    ret void
  }
  define void @Exported() linkage c visibility private binding export {
  export_entry:
    call void @Callback()
    ret void
  }
}
)";
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, Text);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (bool Binary : {false, true})
    {
      const auto Saved = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
      IRContext Destination(Compilation);
      const auto Loaded = Binary ? deserializeModuleBinary(Destination, Saved.Bytes) : deserializeModuleText(Destination, Saved.Bytes);
      ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
      const auto &Values = Loaded.ModuleValue->entryBlock().values();
      ASSERT_EQ(Values.size(), 3U);
      const auto &Imported = static_cast<const Function &>(*Values[0]);
      const auto &Callback = static_cast<const Function &>(*Values[1]);
      const auto &Exported = static_cast<const Function &>(*Values[2]);
      EXPECT_TRUE(Imported.isNativeImport());
      EXPECT_FALSE(Imported.hasBody());
      EXPECT_EQ(Imported.visibility(), core::VisibilityKind::Private);
      EXPECT_EQ(Callback.binding(), core::FunctionBinding::Local);
      EXPECT_EQ(Callback.languageLinkage(), LanguageLinkage::C);
      EXPECT_TRUE(Callback.hasBody());
      EXPECT_TRUE(Exported.isNativeExport());
      EXPECT_TRUE(Exported.hasBody());
      EXPECT_EQ(Exported.visibility(), core::VisibilityKind::Private);
      const auto Canonical = serializeModuleText(*Loaded.ModuleValue);
      ASSERT_TRUE(Canonical.succeeded()) << Canonical.Message;
      EXPECT_NE(Canonical.Bytes.find("visibility private binding import"), std::string::npos);
      EXPECT_NE(Canonical.Bytes.find("visibility private binding export"), std::string::npos);
      EXPECT_EQ(Canonical.Bytes.find("binding local"), std::string::npos);
      EXPECT_EQ(Canonical.Bytes, serializeModuleText(*Parsed.ModuleValue).Bytes);
    }
  }

  // Archives reject native definitions with the wrong direction, ABI, missing body or unknown binding value.
  TEST(IRModuleSerializationTest, RejectsInvalidNativeBindingMetadata)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    constexpr std::string_view InvalidFunctions[] = {
        "declare void @f() binding import",
        "declare void @f() cc fast linkage c binding import",
        "declare void @f() linkage c binding export",
        "declare void @f() linkage c binding unknown",
        "declare void @f() linkage c binding import binding local",
        "define void @f() linkage c binding import {}",
        "define void @f() linkage c binding import { entry: ret void }",
        "define void @f() linkage c binding export {}",
        "define void @f() binding export { entry: ret void }",
    };
    for (std::string_view FunctionText : InvalidFunctions)
    {
      EXPECT_EQ(deserializeModuleText(Context, "ink-ir 7 module @Invalid { " + std::string(FunctionText) + " }").Status, core::ArchiveStatus::InvalidArchive) << FunctionText;
    }
    EXPECT_TRUE(Context.modules().empty());
    const auto Parsed = deserializeModuleText(Context, "ink-ir 7 module @Native { declare void @Imported() linkage c binding import }");
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Saved = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
    std::size_t Offset = 16;
    bool Found = false;
    while (Offset < Saved.Bytes.size())
    {
      const char *Header = Saved.Bytes.data() + Offset;
      const auto Fields = llvm::support::endian::read32le(Header + 12);
      const auto TextSize = llvm::support::endian::read64le(Header + 16);
      if (llvm::support::endian::read32le(Header) == 36)
      {
        ASSERT_EQ(Fields, 4U);
        auto Invalid = Saved.Bytes;
        llvm::support::endian::write64le(Invalid.data() + Offset + 56, 99);
        EXPECT_EQ(deserializeModuleBinary(Context, Invalid).Status, core::ArchiveStatus::InvalidArchive);
        Invalid = Saved.Bytes;
        llvm::support::endian::write64le(Invalid.data() + Offset + 56, 2);
        EXPECT_EQ(deserializeModuleBinary(Context, Invalid).Status, core::ArchiveStatus::InvalidArchive);
        Invalid = Saved.Bytes;
        llvm::support::endian::write64le(Invalid.data() + Offset + 40, 0);
        EXPECT_EQ(deserializeModuleBinary(Context, Invalid).Status, core::ArchiveStatus::InvalidArchive);
        Found = true;
        break;
      }
      Offset += 32 + static_cast<std::size_t>(Fields) * 8 + static_cast<std::size_t>(TextSize) + static_cast<std::size_t>(llvm::support::endian::read64le(Header + 24));
    }
    ASSERT_TRUE(Found);
    EXPECT_EQ(Context.modules().size(), 1U);
  }

  // An export under construction cannot be archived until its implementation is attached.
  TEST(IRModuleSerializationTest, RejectsIncompleteNativeExport)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    Module *ModuleValue = Builder.createModule(Context.namePool().intern("Native"));
    ASSERT_NE(ModuleValue, nullptr);
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("Exported"), *Signature, {}, {}, CallingConvention::C, LanguageLinkage::C, core::FunctionBinding::Export);
    ASSERT_NE(FunctionOwner, nullptr);
    Function *Exported = FunctionOwner.get();
    ASSERT_TRUE(Builder.appendValue(ModuleValue->entryBlock(), std::move(FunctionOwner)));
    EXPECT_EQ(serializeModuleText(*ModuleValue).Status, core::ArchiveStatus::InvalidInput);
    EXPECT_EQ(serializeModuleBinary(*ModuleValue).Status, core::ArchiveStatus::InvalidInput);
    BasicBlock *Body = Builder.createFunctionBody(*Exported);
    ASSERT_NE(Body, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Body));
    ASSERT_NE(Builder.createReturnInstruction(), nullptr);
    EXPECT_TRUE(serializeModuleText(*ModuleValue).succeeded());
    EXPECT_TRUE(serializeModuleBinary(*ModuleValue).succeeded());
  }

  // Forward operands and functions resolve after parsing, while comments and quoted symbols remain editable.
  TEST(IRModuleSerializationTest, ReadsForwardReferencesAndComments)
  {
    constexpr std::string_view Text = R"(ink-ir 7
; A hand-written module needs no object table.
module @Example {
  define i32 @main(i32 %x) {
  entry:
    %sum = add i32 %later, 1
    %later = call i32 @"callee with spaces"(i32 %x)
    ret i32 %sum
  }
  declare i32 @"callee with spaces"(i32 %arg name "")
}
)";
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, Text);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto &FunctionValue = static_cast<const Function &>(*Parsed.ModuleValue->entryBlock().values()[0]);
    const auto &Values = FunctionValue.blocks()[0]->values();
    ASSERT_EQ(Values.size(), 3U);
    EXPECT_EQ(&static_cast<const AddInstruction &>(*Values[0]).left(), Values[1].get());
    const auto Encoded = serializeModuleText(*Parsed.ModuleValue);
    ASSERT_TRUE(Encoded.succeeded());
    EXPECT_NE(Encoded.Bytes.find("call i32 @\"callee with spaces\""), std::string::npos);
    IRContext Destination(Compilation);
    EXPECT_TRUE(deserializeModuleText(Destination, Encoded.Bytes).succeeded());
    const auto End = Text.find_last_of('}');
    for (std::size_t Length = 0; Length <= End; ++Length)
    {
      EXPECT_FALSE(deserializeModuleText(Destination, Text.substr(0, Length)).succeeded()) << Length;
      EXPECT_EQ(Destination.modules().size(), 1U);
    }
  }

  // Generated symbol suffixes preserve original duplicate function and parameter names.
  TEST(IRModuleSerializationTest, PreservesDuplicateAndUnnamedSymbols)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Same"));
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const Type *Types[] = {Integer, Integer};
    const Name Names[] = {Context.namePool().intern("x"), Context.namePool().intern("x")};
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(*Integer, Types);
    for (auto Name : {"Same", "Same.1", "Same"})
    {
      ASSERT_TRUE(Builder.appendValue(Root->entryBlock(), Builder.createFunction(Context.namePool().intern(Name), *Signature, {}, Names)));
    }
    const auto Encoded = serializeModuleText(*Root);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    IRContext Destination(Compilation);
    const auto Decoded = deserializeModuleText(Destination, Encoded.Bytes);
    ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
    EXPECT_EQ(serializeModuleBinary(*Decoded.ModuleValue).Bytes, serializeModuleBinary(*Root).Bytes);
    EXPECT_EQ(serializeModuleText(*Decoded.ModuleValue).Bytes, Encoded.Bytes);
  }

  // Equivalent structural aliases compare restored types rather than incidental archive IDs.
  TEST(IRModuleSerializationTest, ResolvesEquivalentTypeAliases)
  {
    constexpr std::string_view Text = R"(ink-ir 7
module @Aliases {
  type !left = i32
  type !right = i32
  type !box = class "Box"
  type !sameBox = !box
  define !left @identity(!left %x) {
  entry:
    ret !right %x
  }
  declare void @boxes(!box %first, !sameBox %second)
}
)";
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Result = deserializeModuleText(Context, Text);
    ASSERT_TRUE(Result.succeeded()) << Result.Message;
    EXPECT_NE(serializeModuleText(*Result.ModuleValue).Bytes.find("ret i32 %x"), std::string::npos);
    const auto &Boxes = static_cast<const Function &>(*Result.ModuleValue->entryBlock().values()[1]);
    EXPECT_EQ(&Boxes.parameters()[0]->type(), &Boxes.parameters()[1]->type());
    EXPECT_EQ(serializeModuleText(*Result.ModuleValue).Bytes.find("class \"Box\""), serializeModuleText(*Result.ModuleValue).Bytes.rfind("class \"Box\""));
  }

  // Repeated aliases must charge copied field storage rather than bypassing the allocation budget.
  TEST(IRModuleSerializationTest, ChargesRepeatedTypeAliasFields)
  {
    std::string Text = "ink-ir 7 module @Budget { type !large = fn(";
    for (unsigned Index = 0; Index < 256; ++Index)
    {
      if (Index)
      {
        Text += ", ";
      }
      Text += "i32";
    }
    Text += ") -> void ";
    for (unsigned Index = 0; Index < 32; ++Index)
    {
      Text += "type !alias" + std::to_string(Index) + " = !large ";
    }
    Text += "}";
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    ModuleArchiveLimits Limits;
    Limits.MaxAllocationBytes = 128 * 1024;
    const auto Result = deserializeModuleText(Context, Text, Limits);
    EXPECT_EQ(Result.Status, core::ArchiveStatus::LimitExceeded) << Result.Message;
    EXPECT_TRUE(Context.modules().empty());
  }

  // Long operand chains use iterative restoration and do not consume the structural nesting budget.
  TEST(IRModuleSerializationTest, RestoresLongOperandChainsInBothFormats)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Chain"));
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(*Integer);
    auto Owner = Builder.createFunction(Context.namePool().intern("sum"), *Signature);
    auto *FunctionValue = Owner.get();
    ASSERT_TRUE(Builder.appendValue(Root->entryBlock(), std::move(Owner)));
    ASSERT_TRUE(Builder.setInsertPoint(*Builder.createFunctionBody(*FunctionValue)));
    const auto *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    const Value *Current = One;
    for (unsigned Index = 0; Index < 4096; ++Index)
    {
      Current = Builder.createAddInstruction(*Current, *One);
      ASSERT_NE(Current, nullptr);
    }
    ASSERT_NE(Builder.createReturnInstruction(Current), nullptr);
    ModuleArchiveLimits Limits;
    Limits.MaxNestingDepth = 8;
    for (bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Root, Limits) : serializeModuleText(*Root, Limits);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes, Limits) : deserializeModuleText(Destination, Encoded.Bytes, Limits);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto &Function = static_cast<const ir::Function &>(*Decoded.ModuleValue->entryBlock().values()[0]);
      EXPECT_EQ(Function.blocks()[0]->values().size(), 4097U);
      EXPECT_EQ(serializeModuleBinary(*Decoded.ModuleValue).Bytes, serializeModuleBinary(*Root).Bytes);
    }
  }

  // Pool insertion order and unrelated context objects must not alter either canonical archive.
  TEST(IRModuleSerializationTest, SerializationIsIndependentOfPoolHistory)
  {
    core::CompilationContext Compilation;
    IRContext First(Compilation);
    IRContext Second(Compilation);
    Second.namePool().intern("Unrelated");
    Second.typePool().getType<TypeKind::Integer>(23, false);
    Second.typePool().createClassType(Second.namePool().intern("UnrelatedType"));
    const auto *Left = makeModule(First);
    const auto *Right = makeModule(Second);
    ASSERT_NE(Left, nullptr);
    ASSERT_NE(Right, nullptr);
    EXPECT_EQ(serializeModuleText(*Left).Bytes, serializeModuleText(*Right).Bytes);
    EXPECT_EQ(serializeModuleBinary(*Left).Bytes, serializeModuleBinary(*Right).Bytes);
  }

  // Archives own all required spellings and constant bytes after the source context is destroyed.
  TEST(IRModuleSerializationTest, RestorationDoesNotBorrowSourceStorage)
  {
    core::CompilationContext Compilation;
    std::string Bytes;
    {
      IRContext Source(Compilation);
      const auto *Root = makeModule(Source);
      ASSERT_NE(Root, nullptr);
      auto Encoded = serializeModuleBinary(*Root);
      ASSERT_TRUE(Encoded.succeeded());
      Bytes = std::move(Encoded.Bytes);
    }
    IRContext Destination(Compilation);
    auto Decoded = deserializeModuleBinary(Destination, Bytes);
    ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
    Bytes.assign(Bytes.size(), '\0');
    checkModule(*Decoded.ModuleValue);
  }

  // References to detached or other-module functions fail explicitly instead of creating dangling IDs.
  TEST(IRModuleSerializationTest, RejectsOperandsOutsideTheModuleTree)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Root"));
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    auto External = Builder.createFunction(Context.namePool().intern("External"), *Signature);
    ASSERT_TRUE(Builder.appendValue(Root->entryBlock(), Builder.createDetachedCallInstruction(*External)));
    for (const auto &Result : {serializeModuleText(*Root), serializeModuleBinary(*Root)})
    {
      EXPECT_EQ(Result.Status, core::ArchiveStatus::InvalidInput);
      EXPECT_TRUE(Result.Bytes.empty());
      EXPECT_FALSE(Result.Message.empty());
    }
  }

  // Truncation at every byte, including record and raw payload boundaries, never publishes a partial module.
  TEST(IRModuleSerializationTest, RejectsEveryBinaryTruncation)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, EmptyModuleText);
    ASSERT_TRUE(Parsed.succeeded());
    const auto Archive = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Archive.succeeded());
    EXPECT_EQ(Archive.Bytes.substr(0, 4), "IIRB");
    for (std::size_t Length = 0; Length < Archive.Bytes.size(); ++Length)
    {
      const auto Result = deserializeModuleBinary(Context, std::string_view(Archive.Bytes).substr(0, Length));
      EXPECT_FALSE(Result.succeeded()) << Length;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_EQ(Context.modules().size(), 1U);
    }
    auto Trailing = Archive.Bytes + std::string(4, '\0');
    EXPECT_FALSE(deserializeModuleBinary(Context, Trailing).succeeded());
  }

  // Fixed-width headers, lengths, flags and IDs are validated before allocation or publication.
  TEST(IRModuleSerializationTest, RejectsMalformedBinaryRecords)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Root = Builder.createModule(Context.namePool().intern("Root"));
    const auto Archive = serializeModuleBinary(*Root);
    ASSERT_TRUE(Archive.succeeded());
    const auto Check32 = [&](std::size_t Offset, std::uint32_t Value, core::ArchiveStatus Status)
    {
      auto Bytes = Archive.Bytes;
      llvm::support::endian::write32le(Bytes.data() + Offset, Value);
      EXPECT_EQ(deserializeModuleBinary(Context, Bytes).Status, Status);
      EXPECT_EQ(Context.modules().size(), 1U);
    };
    Check32(4, 999, core::ArchiveStatus::UnsupportedVersion);
    Check32(8, UINT32_MAX, core::ArchiveStatus::LimitExceeded);
    Check32(12, 1, core::ArchiveStatus::InvalidArchive);
    Check32(16, 999, core::ArchiveStatus::InvalidArchive);
    Check32(20, UINT32_MAX, core::ArchiveStatus::InvalidArchive);
    Check32(24, 1, core::ArchiveStatus::InvalidArchive);
    Check32(28, UINT32_MAX, core::ArchiveStatus::InvalidArchive);
    auto Bytes = Archive.Bytes;
    llvm::support::endian::write64le(Bytes.data() + 32, UINT64_MAX);
    EXPECT_EQ(deserializeModuleBinary(Context, Bytes).Status, core::ArchiveStatus::InvalidArchive);
    EXPECT_EQ(Context.modules().size(), 1U);
  }

  // Invalid syntax, escapes, symbols and trailing input leave existing modules intact.
  TEST(IRModuleSerializationTest, RejectsMalformedTextWithoutChangingExistingModules)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Original = deserializeModuleText(Context, EmptyModuleText);
    ASSERT_TRUE(Original.succeeded()) << Original.Message;
    const std::string_view Invalid[] = {
        "other 2 module @A {}",
        "ink-ir 7 module @A {",
        "ink-ir 7 module @A {} junk",
        "ink-ir 7 module @\"Bad\\qEscape\" {}",
        "ink-ir 7 module @\"Bad\\xzzEscape\" {}",
        "ink-ir 7 module @\"Unclosed {}",
        "ink-ir 7 module @A { unknown }",
        "ink-ir 7 module @A { declare void @f() declare void @f() }",
        "ink-ir 7 module @A { define i32 @f() { ^entry: ret i32 %missing } }",
        "ink-ir 7 module @A { define i32 @f(i32 %x, i32 %x) { ^entry: ret i32 %x } }",
    };
    for (auto Text : Invalid)
    {
      const auto Result = deserializeModuleText(Context, Text);
      EXPECT_EQ(Result.Status, core::ArchiveStatus::InvalidArchive) << Text;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_EQ(Context.modules().size(), 1U);
      EXPECT_EQ(Context.modules()[0].get(), Original.ModuleValue);
    }
    EXPECT_EQ(deserializeModuleText(Context, "ink-ir 999").Status, core::ArchiveStatus::UnsupportedVersion);
  }

  // Each configurable budget is enforced for writers and readers with no partial archive or root.
  TEST(IRModuleSerializationTest, EnforcesResourceBudgets)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto *Root = makeModule(Context);
    ASSERT_NE(Root, nullptr);
    const auto Text = serializeModuleText(*Root);
    const auto Binary = serializeModuleBinary(*Root);
    ASSERT_TRUE(Text.succeeded());
    ASSERT_TRUE(Binary.succeeded());
    std::array<ModuleArchiveLimits, 6> Limits;
    Limits[0].MaxArchiveBytes = 1;
    Limits[1].MaxObjects = 1;
    Limits[2].MaxFields = 0;
    Limits[3].MaxStringBytes = 0;
    Limits[4].MaxAllocationBytes = 0;
    Limits[5].MaxNestingDepth = 1;
    for (const auto &Limit : Limits)
    {
      for (const auto &Result : {serializeModuleText(*Root, Limit), serializeModuleBinary(*Root, Limit)})
      {
        EXPECT_EQ(Result.Status, core::ArchiveStatus::LimitExceeded) << Result.Message;
        EXPECT_TRUE(Result.Bytes.empty());
      }
      for (const auto &Result : {deserializeModuleText(Context, Text.Bytes, Limit), deserializeModuleBinary(Context, Binary.Bytes, Limit)})
      {
        EXPECT_EQ(Result.Status, core::ArchiveStatus::LimitExceeded) << Result.Message;
        EXPECT_EQ(Result.ModuleValue, nullptr);
        EXPECT_EQ(Context.modules().size(), 1U);
      }
    }
  }

  // Assembly cannot construct cyclic types, overflowing constants or ill-typed instructions.
  TEST(IRModuleSerializationTest, ValidatesGraphAndInstructionSemantics)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const std::string_view Bodies[] = {
        "type !a = ptr<ro, !a>",
        "type !a = ptr<ro, !b> type !b = ptr<ro, !a>",
        "declare i0 @f()",
        "declare i4294967296 @f()",
        "declare f80 @f()",
        "type !a = ptr<invalid, i32>",
        "declare !missing @f()",
        "define i1 @f() { ^entry: ret i1 2 }",
        "define u8 @f() { ^entry: ret u8 -1 }",
        "define f16 @f() { ^entry: ret f16 bits(0x10000) }",
        "define i32 @f(i64 %x) { ^entry: %a = add i32 %x, 1 ret i32 %a }",
        "define i32 @f() { ^entry: %a = load i32, i32 1 ret i32 %a }",
        "define void @f() { ^entry: ret i32 1 }",
        "define void @f() { ^entry: call i32 @f() ret void }",
        "declare void @f() cc invalid",
        "declare void @f() linkage invalid",
    };
    for (auto Body : Bodies)
    {
      const auto Result = deserializeModuleText(Context, "ink-ir 7 module @Root { " + std::string(Body) + " }");
      EXPECT_EQ(Result.Status, core::ArchiveStatus::InvalidArchive) << Body << ": " << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Context.modules().empty());
    }
  }

  // Appending a value after a terminator fails during attachment and rolls back the detached tree.
  TEST(IRModuleSerializationTest, RollsBackAfterLateAttachmentFailure)
  {
    constexpr std::string_view Text = R"(ink-ir 7
module @Root {
  define void @Main() {
  ^entry:
    ret void
    call void @Main()
  }
}
)";
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Result = deserializeModuleText(Context, Text);
    EXPECT_EQ(Result.Status, core::ArchiveStatus::InvalidArchive);
    EXPECT_EQ(Result.ModuleValue, nullptr);
    EXPECT_TRUE(Context.modules().empty());
    EXPECT_NE(Result.Message.find("placement"), std::string::npos);
  }

  // Generic trees retain full AST snapshots, shared node identity and independent source units in both formats.
  TEST(IRModuleSerializationTest, GenericDeclarationsAndASTsSurviveSourceDestruction)
  {
    for (bool Binary : {false, true})
    {
      core::CompilationContext Compilation;
      std::string Bytes;
      std::string ASTDump;
      {
        core::FrontendContext Frontend(Compilation);
        auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "class Box[T: type] { field Item: T; }; func Identity[T: type](Value: T): T { return Value; }"));
        auto Additional = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Nested[T: type](Value: T): T { return Value; }"));
        ASSERT_TRUE(Parsed.succeeded());
        ASSERT_TRUE(Additional.succeeded());
        ASTDump = parser::dumpAST(*Parsed.Unit);
        const auto *ClassAST = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0])->declaration());
        const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[1])->declaration());
        const auto *NestedAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Additional.Unit->root()->statements()[0])->declaration());
        IRContext Source(Compilation);
        IRBuilder Builder(Source);
        auto *Root = makeModule(Source);
        ASSERT_NE(Root, nullptr);
        auto *DeclRoot = Builder.createModuleDecl(*Root, *Parsed.Unit->root());
        ASSERT_NE(DeclRoot, nullptr);
        auto *Class = Builder.createClassDecl(*DeclRoot, Source.namePool().intern("Box"), *ClassAST);
        ASSERT_NE(Class, nullptr);
        ASSERT_NE(Builder.createFunctionDecl(*Class, Source.namePool().intern("Nested"), *NestedAST), nullptr);
        ASSERT_NE(Builder.createFunctionDecl(*DeclRoot, Source.namePool().intern("Identity"), *FunctionAST), nullptr);
        ASSERT_NE(Builder.createFunctionDecl(*DeclRoot, Source.namePool().intern("Alias"), *FunctionAST), nullptr);
        auto *NestedModule = static_cast<Module *>(Root->entryBlock().values()[3].get());
        auto *NestedRoot = Builder.createModuleDecl(*NestedModule, *Parsed.Unit->root());
        ASSERT_NE(NestedRoot, nullptr);
        ASSERT_NE(Builder.createClassDecl(*NestedRoot, Source.namePool().intern("SharedBox"), *ClassAST), nullptr);
        const parser::ParseResult *Sources[] = {&Parsed, &Additional};
        auto Encoded = Binary ? serializeModuleBinary(*Root, {}, Sources) : serializeModuleText(*Root, {}, Sources);
        ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
        Bytes = std::move(Encoded.Bytes);
      }
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Bytes) : deserializeModuleText(Destination, Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      auto *Root = Decoded.ModuleValue;
      checkModule(*Root);
      ASSERT_EQ(Root->archivedASTs().size(), 2U);
      EXPECT_EQ(parser::dumpAST(*Root->archivedASTs()[0]->Unit), ASTDump);
      const auto *DeclRoot = Root->declarationRoot();
      ASSERT_NE(DeclRoot, nullptr);
      ASSERT_EQ(DeclRoot->children().size(), 3U);
      const auto &Class = *DeclRoot->children()[0];
      EXPECT_TRUE(ClassDecl::classof(&Class));
      EXPECT_EQ(Class.parent(), DeclRoot);
      EXPECT_EQ(&Class.module(), Root);
      ASSERT_EQ(Class.children().size(), 1U);
      EXPECT_TRUE(FunctionDecl::classof(Class.children()[0].get()));
      EXPECT_EQ(Class.children()[0]->parent(), &Class);
      EXPECT_EQ(&DeclRoot->children()[1]->ast(), &DeclRoot->children()[2]->ast());
      EXPECT_EQ(static_cast<const FunctionDecl &>(*DeclRoot->children()[1]).ast().genericParameters().size(), 1U);
      auto *NestedModule = static_cast<Module *>(Root->entryBlock().values()[3].get());
      ASSERT_NE(NestedModule->declarationRoot(), nullptr);
      EXPECT_EQ(&NestedModule->declarationRoot()->ast(), &DeclRoot->ast());
      EXPECT_EQ(&NestedModule->declarationRoot()->children()[0]->ast(), &Class.ast());
      const auto Reencoded = Binary ? serializeModuleBinary(*Root) : serializeModuleText(*Root);
      ASSERT_TRUE(Reencoded.succeeded()) << Reencoded.Message;
      EXPECT_EQ(Reencoded.Bytes, Bytes);
      const auto Converted = Binary ? serializeModuleText(*Root) : serializeModuleBinary(*Root);
      ASSERT_TRUE(Converted.succeeded()) << Converted.Message;
      IRContext OtherFormat(Compilation);
      const auto Other = Binary ? deserializeModuleText(OtherFormat, Converted.Bytes) : deserializeModuleBinary(OtherFormat, Converted.Bytes);
      ASSERT_TRUE(Other.succeeded()) << Other.Message;
      EXPECT_EQ(parser::dumpAST(*Other.ModuleValue->archivedASTs()[0]->Unit), ASTDump);
      IRBuilder Builder(Destination);
      auto Detached = Builder.removeValue(Root->entryBlock(), *NestedModule);
      ASSERT_NE(Detached, nullptr);
      ASSERT_TRUE(Builder.eraseModule(*Root));
      // The retained snapshot stays usable after destroying the original restored root.
      ASSERT_EQ(NestedModule->archivedASTs().size(), 1U);
      EXPECT_EQ(parser::dumpAST(*NestedModule->archivedASTs()[0]->Unit), ASTDump);
      EXPECT_EQ(NestedModule->declarationRoot()->children()[0]->ast().getKind(), parser::ASTKind::ClassDecl);
      EXPECT_TRUE(serializeModuleBinary(*NestedModule).succeeded());
    }
  }

  // Missing source owners are explicit errors; invalid nested AST bytes return status without an ICE.
  TEST(IRModuleSerializationTest, ValidatesDeclarationSourcesAndEmbeddedASTArchives)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Identity[T: type](Value: T): T { return Value; }"));
    ASSERT_TRUE(Parsed.succeeded());
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Generic"));
    ASSERT_NE(Builder.createModuleDecl(*Root, *Parsed.Unit->root()), nullptr);
    EXPECT_EQ(serializeModuleText(*Root).Status, core::ArchiveStatus::InvalidInput);
    EXPECT_EQ(serializeModuleBinary(*Root).Status, core::ArchiveStatus::InvalidInput);
    const parser::ParseResult *Sources[] = {&Parsed};
    auto Encoded = serializeModuleText(*Root, {}, Sources);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    const auto Signature = Encoded.Bytes.find("ModuleAST");
    ASSERT_NE(Signature, std::string::npos);
    Encoded.Bytes.replace(Signature, 9, "UnknownAST");
    auto Invalid = deserializeModuleText(Context, Encoded.Bytes);
    EXPECT_EQ(Invalid.Status, core::ArchiveStatus::InvalidArchive);
    EXPECT_EQ(Invalid.ModuleValue, nullptr);
    EXPECT_EQ(Context.modules().size(), 1U);
    ModuleArchiveLimits Limits;
    Limits.AST.MaxNodes = 1;
    EXPECT_EQ(serializeModuleBinary(*Root, Limits, Sources).Status, core::ArchiveStatus::LimitExceeded);
    auto Binary = serializeModuleBinary(*Root, {}, Sources);
    ASSERT_TRUE(Binary.succeeded());
    EXPECT_EQ(deserializeModuleBinary(Context, Binary.Bytes, Limits).Status, core::ArchiveStatus::LimitExceeded);
    EXPECT_EQ(Context.modules().size(), 1U);
  }

  // The composable AST APIs preserve parse metadata and return failures for every truncated byte prefix.
  TEST(IRModuleSerializationTest, EmbeddedASTUsesNonfatalErrorsAndPreservesParseState)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Identity[T: type](Value: T): T { return Value; }"));
    ASSERT_TRUE(Parsed.succeeded());
    Parsed.Status = parser::ParseStatus::Cancelled;
    Parsed.HasSyntaxErrors = true;
    auto AST = parser::trySerializeAST(Parsed);
    ASSERT_TRUE(AST.succeeded());
    for (std::size_t Length = 0; Length < AST.Bytes.size(); ++Length)
    {
      const auto Result = parser::tryDeserializeAST(Frontend, std::string_view(AST.Bytes).substr(0, Length));
      EXPECT_FALSE(Result.succeeded()) << Length;
      EXPECT_EQ(Result.Parsed.Unit, nullptr);
    }
    EXPECT_EQ(parser::trySerializeAST({}).Status, core::ArchiveStatus::InvalidInput);
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Interrupted"));
    ASSERT_NE(Builder.createModuleDecl(*Root, *Parsed.Unit->root()), nullptr);
    const parser::ParseResult *Sources[] = {&Parsed};
    const auto Encoded = serializeModuleBinary(*Root, {}, Sources);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    const auto Decoded = deserializeModuleBinary(Context, Encoded.Bytes);
    ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
    ASSERT_EQ(Decoded.ModuleValue->archivedASTs().size(), 1U);
    EXPECT_EQ(Decoded.ModuleValue->archivedASTs()[0]->Status, parser::ParseStatus::Cancelled);
    EXPECT_TRUE(Decoded.ModuleValue->archivedASTs()[0]->HasSyntaxErrors);
  }
} // namespace ink::ir::test
