#include "ink/core/config.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/serialization/serialization.h"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <string>
#include <string_view>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    std::string makeTextIR(std::string_view Body)
    {
      std::string Text = "ink_ir ";
      Text.append(INK_IR_VERSION);
      Text.push_back('\n');
      Text.append(Body);
      return Text;
    }

    std::unique_ptr<Module> createNegationModule(IRContext &Context)
    {
      auto &Names = core::NamePool::instance();
      auto Result = std::make_unique<Module>(Context, ModuleID{{Names.intern("serialization"), Names.intern("text")}});
      const auto *Integer = Context.typePool().get<IntegerType>(32, true);
      const auto *Signature = Context.typePool().get<FunctionType>(Integer, {Integer});
      auto *Callable = Result->createGlobalFunction(*Signature, Names.intern("negate"));
      Callable->parameter(0)->setName(Names.intern("operand"));
      auto *Block = Callable->createBlock(Names.intern("entry"));
      IRBuilder Builder(Context);
      if (!Builder.setInsertPoint(*Block))
      {
        return {};
      }
      auto *Negation = Builder.createNeg(*Callable->parameter(0));
      if (!Negation || !Builder.createReturn(*Negation))
      {
        return {};
      }
      return Result;
    }

    bool replaceFirst(std::string &Text, std::string_view Original, std::string_view Replacement)
    {
      const auto Position = Text.find(Original);
      if (Position == std::string::npos)
      {
        return false;
      }
      Text.replace(Position, Original.size(), Replacement);
      return true;
    }
  } // namespace

  // Empty modules preserve their optional identifier and borrow the explicitly supplied context.
  TEST(IRTextSerializationTest, RoundTripsEmptyModuleIntoExternalContext)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    const auto Text = IRTextWriter::write(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_TRUE(Text->starts_with(makeTextIR({})));
    EXPECT_NE(Text->find("module []"), std::string::npos);
    const auto Loaded = IRTextReader::read(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    EXPECT_EQ(&(*Loaded)->irContext(), &DestinationContext);
    EXPECT_EQ((*Loaded)->moduleID(), Source.moduleID());
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Function signatures, block labels and typed operands replace flat records without changing value identity.
  TEST(IRTextSerializationTest, RoundTripsNamedNegationWithStructuredFunctionBody)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    const auto Source = createNegationModule(SourceContext);
    ASSERT_NE(Source, nullptr);
    const auto Text = writeText(*Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("define i32 @negate(i32 %operand)"), std::string::npos);
    EXPECT_NE(Text->find("entry:"), std::string::npos);
    EXPECT_NE(Text->find(" = neg i32 %operand"), std::string::npos);
    EXPECT_NE(Text->find("ret i32 %"), std::string::npos);
    EXPECT_EQ(Text->find(" = ret"), std::string::npos);
    EXPECT_EQ(Text->find("block ="), std::string::npos);
    EXPECT_EQ(Text->find("type.integer"), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 1U);
    const auto *Callable = (*Loaded)->globalFunction(0);
    EXPECT_EQ(core::NamePool::instance().text(Callable->name()), "negate");
    EXPECT_EQ(core::NamePool::instance().text(Callable->parameter(0)->name()), "operand");
    ASSERT_EQ(Callable->entryBlock()->instructionCount(), 2U);
    const auto *Negation = static_cast<const NegInstruction *>(Callable->entryBlock()->instruction(0));
    const auto *Return = static_cast<const ReturnInstruction *>(Callable->entryBlock()->instruction(1));
    EXPECT_EQ(Negation->operand(), Callable->parameter(0));
    EXPECT_EQ(Return->returnedValue(), Negation);
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Hand-written structured functions can reuse local names while a call keeps its callee and named argument.
  TEST(IRTextSerializationTest, ReadsStructuredGrammarWithIndependentFunctionScopes)
  {
    const auto Text = makeTextIR(R"(target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])
module ["demo"]
{
  define i32 @negate(i32 %value)
  {
  entry:
    %0 = neg i32 %value
    ret i32 %0
  }
  define i32 @forward(i32 %value)
  {
  entry:
    %0 = call i32 @negate(i32 %value [name = "operand"])
    ret i32 %0
  }
}
)");
    IRContext Context(Target);
    const auto Loaded = readText(Context, Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 2U);
    const auto *First = (*Loaded)->globalFunction(0);
    const auto *Second = (*Loaded)->globalFunction(1);
    const auto *Negation = static_cast<const NegInstruction *>(First->entryBlock()->instruction(0));
    const auto *Call = static_cast<const CallInstruction *>(Second->entryBlock()->instruction(0));
    EXPECT_EQ(Negation->operand(), First->parameter(0));
    EXPECT_EQ(Call->callee(), First);
    ASSERT_NE(Call->argument(0), nullptr);
    EXPECT_EQ(Call->argument(0)->value(), Second->parameter(0));
    EXPECT_EQ(core::NamePool::instance().text(Call->argument(0)->name()), "operand");
    const auto Canonical = writeText(**Loaded);
    ASSERT_TRUE(Canonical.has_value()) << Canonical.error().Message;
    const auto Reloaded = readText(Context, *Canonical);
    ASSERT_TRUE(Reloaded.has_value()) << Reloaded.error().Message;
    const auto Rewritten = writeText(**Reloaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Canonical);
  }

  // Disambiguated symbols preserve duplicate function, parameter, block and alloca names alongside anonymous values.
  TEST(IRTextSerializationTest, PreservesDuplicateAndAnonymousNamesAcrossScopes)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    auto &Names = core::NamePool::instance();
    const auto SharedName = Names.intern("shared");
    const auto *Integer = SourceContext.typePool().get<IntegerType>(32, true);
    const auto *Signature = SourceContext.typePool().get<FunctionType>(Integer, {Integer, Integer});
    IRBuilder Builder(SourceContext);
    for (unsigned Index = 0; Index < 3; ++Index)
    {
      auto *Callable = Source.createGlobalFunction(*Signature, Index < 2 ? SharedName : core::NAME_None);
      ASSERT_NE(Callable, nullptr);
      Callable->parameter(0)->setName(SharedName);
      Callable->parameter(1)->setName(SharedName);
      auto *Entry = Callable->createBlock(SharedName);
      auto *Exit = Callable->createBlock(SharedName);
      ASSERT_TRUE(Builder.setInsertPoint(*Entry));
      auto *Slot = Builder.createAlloca(*Integer);
      ASSERT_NE(Slot, nullptr);
      Slot->setName(SharedName);
      auto *Negation = Builder.createNeg(*Callable->parameter(1));
      ASSERT_NE(Negation, nullptr);
      ASSERT_NE(Builder.createBranch(*Exit), nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Exit));
      auto *Return = Builder.createReturn(*Negation);
      ASSERT_NE(Return, nullptr);
    }
    const auto OriginalBytes = writeBinary(Source);
    ASSERT_TRUE(OriginalBytes.has_value()) << OriginalBytes.error().Message;
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_EQ(Text->find(" = ret"), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 3U);
    for (unsigned Index = 0; Index < 3; ++Index)
    {
      const auto *Callable = (*Loaded)->globalFunction(Index);
      EXPECT_EQ(Callable->name(), Index < 2 ? SharedName : core::NAME_None);
      EXPECT_EQ(Callable->parameter(0)->name(), SharedName);
      EXPECT_EQ(Callable->parameter(1)->name(), SharedName);
      ASSERT_EQ(Callable->blockCount(), 2U);
      EXPECT_EQ(Callable->block(0)->name(), SharedName);
      EXPECT_EQ(Callable->block(1)->name(), SharedName);
      const auto *Slot = static_cast<const AllocaInstruction *>(Callable->block(0)->instruction(0));
      const auto *Negation = static_cast<const NegInstruction *>(Callable->block(0)->instruction(1));
      const auto *Return = static_cast<const ReturnInstruction *>(Callable->block(1)->instruction(0));
      EXPECT_EQ(Slot->name(), SharedName);
      EXPECT_EQ(Negation->operand(), Callable->parameter(1));
      EXPECT_EQ(Return->returnedValue(), Negation);
    }
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
    const auto RestoredBytes = writeBinary(**Loaded);
    ASSERT_TRUE(RestoredBytes.has_value()) << RestoredBytes.error().Message;
    EXPECT_EQ(*RestoredBytes, *OriginalBytes);
  }

  // Prefix const qualifiers retain each pointer layer and quoted call argument names survive the text codec.
  TEST(IRTextSerializationTest, PreservesNestedConstPointersAndNamedCallArguments)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    auto &Names = core::NamePool::instance();
    auto &Types = SourceContext.typePool();
    const auto *Byte = Types.get<IntegerType>(8, false, false);
    const auto *Inner = Types.get<PointerType>(Byte, false);
    const auto *Outer = Types.get<PointerType>(Inner, false);
    const auto *Signature = Types.get<FunctionType>(Outer, {Outer});
    auto *Callee = Source.createGlobalFunction(*Signature, Names.intern("callee"));
    auto *Caller = Source.createGlobalFunction(*Signature, Names.intern("caller"));
    ASSERT_NE(Callee, nullptr);
    ASSERT_NE(Caller, nullptr);
    Caller->parameter(0)->setName(Names.intern("input value"));
    IRBuilder Builder(SourceContext);
    ASSERT_TRUE(Builder.setInsertPoint(*Caller->createBlock()));
    const auto ArgumentName = Names.intern("source \"bytes\"");
    auto *Call = Builder.createCall(*Callee, {Argument(*Caller->parameter(0), ArgumentName)});
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Builder.createReturn(*Call), nullptr);
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("const *const *const u8"), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 2U);
    const auto *RestoredCaller = (*Loaded)->globalFunction(1);
    const auto *RestoredOuter = static_cast<const PointerType *>(RestoredCaller->parameter(0)->type());
    const auto *RestoredInner = static_cast<const PointerType *>(RestoredOuter->pointeeType());
    EXPECT_FALSE(RestoredOuter->isMutable());
    EXPECT_FALSE(RestoredInner->isMutable());
    EXPECT_FALSE(RestoredInner->pointeeType()->isMutable());
    const auto *RestoredCall = static_cast<const CallInstruction *>(RestoredCaller->entryBlock()->instruction(0));
    EXPECT_EQ(RestoredCall->callee(), (*Loaded)->globalFunction(0));
    EXPECT_EQ(RestoredCall->argument(0)->value(), RestoredCaller->parameter(0));
    EXPECT_EQ(RestoredCall->argument(0)->name(), ArgumentName);
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Array GEPs infer const element storage without result arrows while function types keep their own return arrows.
  TEST(IRTextSerializationTest, InfersArrayGetElementPtrTypesAndPreservesFunctionTypeArrows)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    auto &Names = core::NamePool::instance();
    auto &Types = SourceContext.typePool();
    const auto *Byte = Types.get<IntegerType>(8, false);
    const auto *Index = Types.get<IntegerType>(32, false);
    const auto *Integer = Types.get<IntegerType>(32, true);
    const auto *Bytes = Types.get<ArrayType>(Types.get<ArrayType>(Byte, 4), 2, false);
    const auto *Callback = Types.get<PointerType>(Types.get<FunctionType>(Integer, {Integer}));
    const auto *Callbacks = Types.get<ArrayType>(Callback, 2, false);
    const auto *Signature = Types.get<FunctionType>(Types.get<VoidType>(), {Types.get<PointerType>(Bytes, false), Types.get<PointerType>(Callbacks)});
    auto *Callable = Source.createGlobalFunction(*Signature, Names.intern("select"));
    ASSERT_NE(Callable, nullptr);
    Callable->parameter(0)->setName(Names.intern("bytes"));
    Callable->parameter(1)->setName(Names.intern("callbacks"));
    IRBuilder Builder(SourceContext);
    ASSERT_TRUE(Builder.setInsertPoint(*Callable->createBlock(Names.intern("entry"))));
    const auto *Zero = SourceContext.constantPool().get<IntegerConstant>(Index, std::uint64_t(0));
    const auto *One = SourceContext.constantPool().get<IntegerConstant>(Index, std::uint64_t(1));
    const auto *Two = SourceContext.constantPool().get<IntegerConstant>(Index, std::uint64_t(2));
    ASSERT_NE(Builder.createGetElementPtr(*Bytes, *Callable->parameter(0), {Zero, One, Two}, true), nullptr);
    ASSERT_NE(Builder.createGetElementPtr(*Callbacks, *Callable->parameter(1), {Zero, One}), nullptr);
    ASSERT_NE(Builder.createReturn(), nullptr);
    const auto OriginalBytes = writeBinary(Source);
    ASSERT_TRUE(OriginalBytes.has_value()) << OriginalBytes.error().Message;
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("%0 = getelementptr const [2 x [4 x u8]], const *const [2 x [4 x u8]] %bytes, u32 0, u32 1, u32 2 [inbounds]\n"), std::string::npos);
    EXPECT_NE(Text->find("%1 = getelementptr const [2 x *fn(i32) -> i32], *const [2 x *fn(i32) -> i32] %callbacks, u32 0, u32 1\n"), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    const auto *Block = (*Loaded)->globalFunction(0)->entryBlock();
    ASSERT_EQ(Block->instructionCount(), 3U);
    const auto *ByteAddress = static_cast<const GetElementPtrInstruction *>(Block->instruction(0));
    const auto *CallbackAddress = static_cast<const GetElementPtrInstruction *>(Block->instruction(1));
    EXPECT_TRUE(ByteAddress->isInBounds());
    EXPECT_EQ(ByteAddress->type(), DestinationContext.typePool().get<PointerType>(DestinationContext.typePool().get<IntegerType>(8, false, false)));
    const auto *CallbackStorage = static_cast<const PointerType *>(CallbackAddress->type())->pointeeType();
    ASSERT_TRUE(PointerType::classof(CallbackStorage));
    EXPECT_FALSE(CallbackStorage->isMutable());
    const auto *CallbackSignature = static_cast<const PointerType *>(CallbackStorage)->pointeeType();
    ASSERT_TRUE(FunctionType::classof(CallbackSignature));
    EXPECT_TRUE(CallbackSignature->isMutable());
    const auto RestoredBytes = writeBinary(**Loaded);
    ASSERT_TRUE(RestoredBytes.has_value()) << RestoredBytes.error().Message;
    EXPECT_EQ(*RestoredBytes, *OriginalBytes);
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // A GEP can select an array field from a later nominal definition and propagate its enclosing const qualification.
  TEST(IRTextSerializationTest, InfersGetElementPtrTypeFromForwardNominalDefinition)
  {
    const auto Text = makeTextIR(R"(target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])
module []
{
  define *const u8 @select(*const %Container %base)
  {
  entry:
    %0 = getelementptr const %Container, *const %Container %base, u32 0, u32 1, u32 2
    ret *const u8 %0
  }
  %Container = type struct [size = 8, alignment = 4]
  {
    field i32 "count" [offset = 0] = none
    field [4 x u8] "data" [offset = 4] = none
  }
}
)");
    IRContext Context(Target);
    const auto Loaded = readText(Context, Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    const auto *Callable = (*Loaded)->globalFunction(0);
    const auto *Address = static_cast<const GetElementPtrInstruction *>(Callable->entryBlock()->instruction(0));
    EXPECT_EQ(Address->type(), Context.typePool().get<PointerType>(Context.typePool().get<IntegerType>(8, false, false)));
    const auto *Return = static_cast<const ReturnInstruction *>(Callable->entryBlock()->instruction(1));
    EXPECT_EQ(Return->returnedValue(), Address);
    const auto Canonical = writeText(**Loaded);
    ASSERT_TRUE(Canonical.has_value()) << Canonical.error().Message;
    EXPECT_EQ(Canonical->find("->"), std::string::npos);
    IRContext DestinationContext(Target);
    const auto Reloaded = readText(DestinationContext, *Canonical);
    ASSERT_TRUE(Reloaded.has_value()) << Reloaded.error().Message;
    const auto Rewritten = writeText(**Reloaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Canonical);
  }

  // GEP parsing rejects obsolete result arrows, noninteger indices and field selections without a valid constant index.
  TEST(IRTextSerializationTest, RejectsGetElementPtrResultArrowsAndInvalidIndexPaths)
  {
    const auto Prefix = makeTextIR("target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])\nmodule []\n{\n%S = type struct [size = 4, alignment = 4] { field i32 \"x\" [offset = 0] = none }\n");
    constexpr std::string_view InvalidBodies[] = {
        "define void @f(*i32 %p) { entry: %0 = getelementptr i32, *i32 %p, u32 0 -> *i32 ret void }",
        "define void @f(*i32 %p) { entry: %0 = getelementptr i32, *i32 %p, bool false ret void }",
        "define void @f(*i32 %p) { entry: %0 = getelementptr i32, *i32 %p, u32 0, u32 0 ret void }",
        "define void @f(*%S %p, u32 %index) { entry: %0 = getelementptr %S, *%S %p, u32 0, u32 %index ret void }",
        "define void @f(*%S %p) { entry: %0 = getelementptr %S, *%S %p, u32 0, u32 1 ret void }",
        "define void @f(*%S %p) { entry: %0 = getelementptr %S, *%S %p, u32 0, i32 -1 ret void }",
    };
    IRContext Context(Target);
    for (const auto Body : InvalidBodies)
    {
      SCOPED_TRACE(Body);
      const auto Text = std::string(Prefix) + std::string(Body) + "\n}\n";
      EXPECT_FALSE(readText(Context, Text).has_value());
    }
  }

  // Quoted numeric symbols stay named and escaped symbol bytes are distinct from anonymous numbering.
  TEST(IRTextSerializationTest, PreservesQuotedNumericAndEscapedSymbolNames)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    const auto Source = createNegationModule(SourceContext);
    ASSERT_NE(Source, nullptr);
    auto &Names = core::NamePool::instance();
    const auto FunctionName = Names.intern("0");
    const auto ParameterName = Names.intern("value \"with space\"");
    const auto BlockName = Names.intern("entry\\body");
    const auto EmbeddedNullName = Names.intern(std::string("\0000", 2));
    auto *Callable = Source->globalFunction(0);
    Callable->setName(FunctionName);
    Callable->parameter(0)->setName(ParameterName);
    Callable->entryBlock()->setName(BlockName);
    ASSERT_NE(Source->createGlobalFunction(*SourceContext.typePool().get<FunctionType>(SourceContext.typePool().get<VoidType>()), EmbeddedNullName), nullptr);
    const auto Text = writeText(*Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("@\"0\""), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 2U);
    const auto *Restored = (*Loaded)->globalFunction(0);
    EXPECT_EQ(Restored->name(), FunctionName);
    EXPECT_EQ(Restored->parameter(0)->name(), ParameterName);
    EXPECT_EQ(Restored->entryBlock()->name(), BlockName);
    EXPECT_EQ((*Loaded)->globalFunction(1)->name(), EmbeddedNullName);
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Byte escaping preserves embedded NUL, quotes, backslashes, control characters and multibyte UTF-8.
  TEST(IRTextSerializationTest, PreservesExactStringBytesAndCanonicalEscapes)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    const std::string Bytes("A\0\"\\\n\t\xE4\xBD\xA0", 9);
    const auto *Constant = SourceContext.constantPool().get<StringConstant>(Bytes);
    ASSERT_NE(Constant, nullptr);
    const auto *Storage = Source.getOrCreateStringStorage(*Constant);
    ASSERT_NE(Storage, nullptr);
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("c\"A\\00\\22\\5C\\0A\\09\\E4\\BD\\A0\""), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalVariableCount(), 1U);
    const auto *Restored = static_cast<const StringConstant *>((*Loaded)->globalVariable(0)->initializer());
    ASSERT_NE(Restored, nullptr);
    EXPECT_EQ(Restored->value(), Bytes);
    EXPECT_EQ((*Loaded)->getOrCreateStringStorage(*Restored), (*Loaded)->globalVariable(0));
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Wide integers and IEEE NaN payloads survive hexadecimal text without floating point conversion.
  TEST(IRTextSerializationTest, PreservesMaximumIntegerWordsAndFloatingBitPatterns)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    const auto *Integer = SourceContext.typePool().get<IntegerType>(128, false);
    const std::array<std::uint64_t, 2> Words{std::numeric_limits<std::uint64_t>::max(), 0xFEDCBA9876543210ULL};
    const auto IntegerBits = core::IntegerBits::fromWords(128, Words);
    ASSERT_TRUE(IntegerBits.has_value());
    auto *IntegerGlobal = Source.createGlobalVariable(*Integer);
    ASSERT_NE(IntegerGlobal, nullptr);
    ASSERT_TRUE(IntegerGlobal->setInitializer(SourceContext.constantPool().get<IntegerConstant>(Integer, *IntegerBits)));
    const auto *Float = SourceContext.typePool().get<FloatType>(64);
    const auto FloatBits = core::IntegerBits::fromUnsigned(64, 0xFFF8000000000123ULL);
    ASSERT_TRUE(FloatBits.has_value());
    auto *FloatGlobal = Source.createGlobalVariable(*Float);
    ASSERT_NE(FloatGlobal, nullptr);
    ASSERT_TRUE(FloatGlobal->setInitializer(SourceContext.constantPool().get<FloatConstant>(Float, *FloatBits)));
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("0x"), std::string::npos);
    EXPECT_NE(Text->find("bits("), std::string::npos);
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalVariableCount(), 2U);
    const auto *LoadedInteger = static_cast<const IntegerConstant *>((*Loaded)->globalVariable(0)->initializer());
    const auto *LoadedFloat = static_cast<const FloatConstant *>((*Loaded)->globalVariable(1)->initializer());
    ASSERT_NE(LoadedInteger, nullptr);
    ASSERT_NE(LoadedFloat, nullptr);
    EXPECT_EQ(LoadedInteger->bits(), *IntegerBits);
    EXPECT_EQ(LoadedFloat->bits(), 0xFFF8000000000123ULL);
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value());
    EXPECT_EQ(*Rewritten, *Text);
  }

  // A branch can reference a later block definition while block order and target identity remain unchanged.
  TEST(IRTextSerializationTest, ResolvesForwardBranchTargets)
  {
    IRContext SourceContext(Target);
    IRContext DestinationContext(Target);
    Module Source(SourceContext);
    auto *Callable = Source.createGlobalFunction(*SourceContext.typePool().get<FunctionType>(SourceContext.typePool().get<VoidType>()));
    auto *Entry = Callable->createBlock();
    auto *Exit = Callable->createBlock();
    IRBuilder Builder(SourceContext);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createBranch(*Exit), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Exit));
    ASSERT_NE(Builder.createReturn(), nullptr);
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    const auto Loaded = readText(DestinationContext, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    const auto *Restored = (*Loaded)->globalFunction(0);
    ASSERT_EQ(Restored->blockCount(), 2U);
    const auto *Branch = static_cast<const BranchInstruction *>(Restored->block(0)->instruction(0));
    EXPECT_EQ(Branch->target(), Restored->block(1));
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Semicolon comments and arbitrary whitespace outside strings normalize to the canonical output.
  TEST(IRTextSerializationTest, AcceptsCommentsAndWhitespace)
  {
    IRContext Context(Target);
    Module Source(Context);
    const auto Canonical = writeText(Source);
    ASSERT_TRUE(Canonical.has_value());
    std::string Decorated = "; leading comment\r\n\t";
    for (const char Character : *Canonical)
    {
      Decorated += Character;
      if (Character == '\n')
      {
        Decorated += "\t; comment between records\r\n  ";
      }
    }
    Decorated += "; trailing comment";
    const auto Loaded = readText(Context, Decorated);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value());
    EXPECT_EQ(*Rewritten, *Canonical);
  }

  // Every prefix ending before the closing module delimiter is rejected as incomplete input.
  TEST(IRTextSerializationTest, RejectsTruncatedDocuments)
  {
    IRContext Context(Target);
    const auto Source = createNegationModule(Context);
    ASSERT_NE(Source, nullptr);
    const auto Text = writeText(*Source);
    ASSERT_TRUE(Text.has_value());
    const auto ClosingBrace = Text->rfind('}');
    ASSERT_NE(ClosingBrace, std::string::npos);
    for (std::size_t Length = 0; Length <= ClosingBrace; ++Length)
    {
      SCOPED_TRACE(Length);
      const auto Result = readText(Context, std::string_view(*Text).substr(0, Length));
      EXPECT_FALSE(Result.has_value());
    }
  }

  // Unknown and incomplete hexadecimal escapes and raw control bytes produce recoverable syntax errors.
  TEST(IRTextSerializationTest, RejectsMalformedStringEscapes)
  {
    IRContext Context(Target);
    Module Source(Context, ModuleID{{core::NamePool::instance().intern("escape")}});
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value());
    const std::string_view InvalidValues[] = {
        "\"\\Q0\"",
        "\"\\0\"",
        "\"line\nfeed\"",
        "\"\\GG\"",
        "\"\\n\"",
        "\"\\t\"",
        "\"\\x41\"",
        "\"\\\\\"",
    };
    for (const auto Invalid : InvalidValues)
    {
      SCOPED_TRACE(Invalid);
      std::string Malformed = *Text;
      ASSERT_TRUE(replaceFirst(Malformed, "\"escape\"", Invalid));
      const auto Result = readText(Context, Malformed);
      ASSERT_FALSE(Result.has_value());
      EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::InvalidSyntax);
      EXPECT_GT(Result.error().Offset, 0U);
    }
  }

  // Numeric overflow, invalid scalar widths and missing references are rejected instead of reaching builder assertions.
  TEST(IRTextSerializationTest, RejectsInvalidNumbersTypesAndReferences)
  {
    IRContext Context(Target);
    const auto Source = createNegationModule(Context);
    ASSERT_NE(Source, nullptr);
    const auto Text = writeText(*Source);
    ASSERT_TRUE(Text.has_value());
    std::string Overflow = *Text;
    ASSERT_TRUE(replaceFirst(Overflow, "neg i32", "neg i18446744073709551616"));
    const auto OverflowResult = readText(Context, Overflow);
    ASSERT_FALSE(OverflowResult.has_value());
    EXPECT_EQ(OverflowResult.error().Code, IRSerializationErrorCode::InvalidSyntax);
    std::string Negative = *Text;
    ASSERT_TRUE(replaceFirst(Negative, "neg i32", "neg i-32"));
    const auto NegativeResult = readText(Context, Negative);
    ASSERT_FALSE(NegativeResult.has_value());
    EXPECT_EQ(NegativeResult.error().Code, IRSerializationErrorCode::InvalidSyntax);
    std::string InvalidWidth = *Text;
    ASSERT_TRUE(replaceFirst(InvalidWidth, "neg i32", "neg i0"));
    EXPECT_FALSE(readText(Context, InvalidWidth).has_value());
    std::string InvalidReference = *Text;
    ASSERT_TRUE(replaceFirst(InvalidReference, "neg i32 %operand", "neg i32 %missing"));
    const auto ReferenceResult = readText(Context, InvalidReference);
    ASSERT_FALSE(ReferenceResult.has_value());
    EXPECT_EQ(ReferenceResult.error().Code, IRSerializationErrorCode::InvalidReference);
    std::string MissingOperand = *Text;
    ASSERT_TRUE(replaceFirst(MissingOperand, "neg i32 %operand", "neg i32 none"));
    EXPECT_FALSE(readText(Context, MissingOperand).has_value());
  }

  // Duplicate local definitions, a global prefix on a local operand and unknown attributes are rejected.
  TEST(IRTextSerializationTest, RejectsAmbiguousDefinitionsAndAttributes)
  {
    IRContext Context(Target);
    const auto Source = createNegationModule(Context);
    ASSERT_NE(Source, nullptr);
    const auto Text = writeText(*Source);
    ASSERT_TRUE(Text.has_value());
    std::string Duplicate = *Text;
    const auto Negation = Duplicate.find(" = neg i32");
    ASSERT_NE(Negation, std::string::npos);
    const auto NameStart = Duplicate.rfind('%', Negation);
    ASSERT_NE(NameStart, std::string::npos);
    Duplicate.replace(NameStart, Negation - NameStart, "%operand");
    EXPECT_FALSE(readText(Context, Duplicate).has_value());
    std::string WrongPrefix = *Text;
    ASSERT_TRUE(replaceFirst(WrongPrefix, "neg i32 %operand", "neg i32 @operand"));
    EXPECT_FALSE(readText(Context, WrongPrefix).has_value());
    std::string UnknownField = *Text;
    ASSERT_TRUE(replaceFirst(UnknownField, "@negate(i32 %operand)", "@negate(i32 %operand) [unknown]"));
    EXPECT_FALSE(readText(Context, UnknownField).has_value());
    EXPECT_FALSE(readText(Context, *Text + "unexpected").has_value());
  }

  // Operand type annotations, declaration attributes and function-local references cannot override the represented IR.
  TEST(IRTextSerializationTest, RejectsConflictingAnnotationsAndCrossFunctionReferences)
  {
    const auto Prefix = makeTextIR("target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])\nmodule []\n{\n");
    constexpr std::string_view InvalidBodies[] = {
        "define i32 @f(i32 %x, i32 %x) { entry: ret i32 %x }",
        "define i32 @f(i32 %x) { entry: ret u32 %x }",
        "define i32 @f(i32 %x) { entry: %0 = neg u32 %x ret i32 %0 }",
        "define i32 @f(i32 %x) [private, private] { entry: ret i32 %x }",
        "define i32 @f(i32 %x) [slot = -1] { entry: ret i32 %x }",
        "define i32 @f(i32 %x) [slot = 18446744073709551615] { entry: ret i32 %x }",
        "define void @empty() {}",
        "define i32 @f(i32 %x) { entry: %0 = neg i32 %x [unknown] ret i32 %0 }",
        "define i32 @f(i32 %hidden) { entry: ret i32 %hidden } define i32 @g(i32 %x) { entry: ret i32 %hidden }",
        "@value = global *i32 = u32 1",
        "declare i32 @f() define i32 @g() { entry: call void @f() [result_type = i32] ret i32 0 }",
        "define u32 @f(*u32 %p) { entry: %0 = load i32, *u32 %p [result_type = u32] ret u32 %0 }",
        "define void @f() { entry: %0 = alloca i32 [result_type = *u32] ret void }",
    };
    IRContext Context(Target);
    for (const auto Body : InvalidBodies)
    {
      SCOPED_TRACE(Body);
      const auto Text = std::string(Prefix) + std::string(Body) + "\n}\n";
      const auto Result = readText(Context, Text);
      EXPECT_FALSE(Result.has_value());
    }
  }

  // Nominal layout and field offsets must be explicit instead of silently defaulting to an empty layout.
  TEST(IRTextSerializationTest, RejectsMissingNominalLayoutAttributes)
  {
    const auto Prefix = makeTextIR("target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])\nmodule []\n{\n");
    constexpr std::string_view InvalidBodies[] = {
        "%S = type struct {}",
        "%S = type struct [size = 0] {}",
        "%S = type struct [alignment = 1] {}",
        "%S = type struct [size = 4, alignment = 4] { field i32 \"value\" = none }",
    };
    IRContext Context(Target);
    for (const auto Body : InvalidBodies)
    {
      SCOPED_TRACE(Body);
      const auto Text = std::string(Prefix) + std::string(Body) + "\n}\n";
      const auto Result = readText(Context, Text);
      ASSERT_FALSE(Result.has_value());
      EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::InvalidSyntax);
    }
  }

  // Decimal magnitudes are bounded to 64 bits; wider values use hexadecimal without width-times-digit conversion work.
  TEST(IRTextSerializationTest, RejectsOverflowingDecimalMagnitudesAndAcceptsWideHexadecimal)
  {
    const auto Text = makeTextIR(R"(target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])
module []
{
  @wide = global *u128 = u128 0x10000000000000000
}
)");
    IRContext Context(Target);
    const auto Valid = readText(Context, Text);
    ASSERT_TRUE(Valid.has_value()) << Valid.error().Message;
    ASSERT_EQ((*Valid)->globalVariableCount(), 1U);
    const auto *Value = static_cast<const IntegerConstant *>((*Valid)->globalVariable(0)->initializer());
    ASSERT_EQ(Value->bits().words().size(), 2U);
    EXPECT_EQ(Value->bits().words()[0], 0U);
    EXPECT_EQ(Value->bits().words()[1], 1U);
    for (const auto Literal : {"18446744073709551616", "-18446744073709551616"})
    {
      SCOPED_TRACE(Literal);
      std::string Malformed(Text);
      ASSERT_TRUE(replaceFirst(Malformed, "0x10000000000000000", Literal));
      const auto Result = readText(Context, Malformed);
      ASSERT_FALSE(Result.has_value());
      EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::InvalidSyntax);
    }
  }

  // Decimal literals honor signedness and signed bounds while hexadecimal literals preserve complete bit patterns.
  TEST(IRTextSerializationTest, ValidatesDecimalSignednessAndHexadecimalBitPatterns)
  {
    const auto Prefix = makeTextIR("target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])\nmodule []\n{\n");
    constexpr std::string_view InvalidBodies[] = {
        "@value = global *u8 = u8 -1",
        "@value = global *i8 = i8 -129",
        "@value = global *i8 = i8 128",
        "@value = global *i64 = i64 9223372036854775808",
        "@value = global *i64 = i64 -9223372036854775809",
        "@value = global *i8 = i8 -0x1",
    };
    IRContext Context(Target);
    for (const auto Body : InvalidBodies)
    {
      SCOPED_TRACE(Body);
      const auto Text = std::string(Prefix) + std::string(Body) + "\n}\n";
      const auto Result = readText(Context, Text);
      ASSERT_FALSE(Result.has_value());
      EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::InvalidSyntax);
    }
    const auto ValidText = std::string(Prefix) + "@minimum = global *i8 = i8 -128\n@bits = global *i8 = i8 0xFF\n@maximum = global *u64 = u64 18446744073709551615\n}\n";
    const auto Loaded = readText(Context, ValidText);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalVariableCount(), 3U);
    EXPECT_EQ(static_cast<const IntegerConstant *>((*Loaded)->globalVariable(0)->initializer())->bits().tryGetInt64(), -128);
    EXPECT_EQ(static_cast<const IntegerConstant *>((*Loaded)->globalVariable(1)->initializer())->bits().tryGetInt64(), -1);
    EXPECT_EQ(static_cast<const IntegerConstant *>((*Loaded)->globalVariable(2)->initializer())->bits().tryGetUInt64(), std::numeric_limits<std::uint64_t>::max());
  }

  // Deep inline type expressions are rejected at the caller's nesting budget before recursive parsing can grow unchecked.
  TEST(IRTextSerializationTest, EnforcesNestingLimitForInlinePointerTypes)
  {
    const auto Text = makeTextIR(R"(target(pointer_bits = 64, byte_order = little, native_abi = 0, alignments = [1, 4, 8, 16, 8])
module []
{
  declare void @consume(********i32 %value)
}
)");
    IRContext Context(Target);
    IRReadOptions Options;
    Options.MaxDepth = 2;
    const auto Limited = readText(Context, Text, Options);
    ASSERT_FALSE(Limited.has_value());
    EXPECT_EQ(Limited.error().Code, IRSerializationErrorCode::ResourceLimit);
    const auto Loaded = readText(Context, Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 1U);
    EXPECT_EQ((*Loaded)->globalFunction(0)->parameter(0)->type()->toString(), "********i32");
  }

  // The configured three-part version is emitted exactly, and a mismatch in any component or target is rejected.
  TEST(IRTextSerializationTest, RejectsUnsupportedVersionAndMismatchedTarget)
  {
    IRContext Context(Target);
    Module Source(Context);
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value());
    const auto Header = makeTextIR({});
    EXPECT_TRUE(Text->starts_with(Header));
    constexpr std::string_view Version = INK_IR_VERSION;
    std::size_t Begin = 0;
    for (unsigned Component = 0; Component < 3; ++Component)
    {
      const auto Separator = Version.find('.', Begin);
      if (Component < 2)
      {
        ASSERT_NE(Separator, std::string_view::npos);
      }
      else
      {
        ASSERT_EQ(Separator, std::string_view::npos);
      }
      const auto End = Separator == std::string_view::npos ? Version.size() : Separator;
      const auto Part = Version.substr(Begin, End - Begin);
      ASSERT_FALSE(Part.empty());
      EXPECT_EQ(Part.find_first_not_of("0123456789"), std::string_view::npos);
      std::string OtherVersion(Version);
      OtherVersion.replace(Begin, Part.size(), Part == "0" ? "1" : "0");
      SCOPED_TRACE(OtherVersion);
      std::string UnsupportedVersion = *Text;
      ASSERT_TRUE(replaceFirst(UnsupportedVersion, Version, OtherVersion));
      const auto VersionResult = readText(Context, UnsupportedVersion);
      ASSERT_FALSE(VersionResult.has_value());
      EXPECT_EQ(VersionResult.error().Code, IRSerializationErrorCode::UnsupportedVersion);
      Begin = End + 1;
    }
    for (const auto UnsupportedHeader : {"ink_ir 1\n", "ink_ir 2\n", "ink_ir 99\n"})
    {
      SCOPED_TRACE(UnsupportedHeader);
      std::string UnsupportedVersion = *Text;
      ASSERT_TRUE(replaceFirst(UnsupportedVersion, Header, UnsupportedHeader));
      const auto VersionResult = readText(Context, UnsupportedVersion);
      ASSERT_FALSE(VersionResult.has_value());
      EXPECT_EQ(VersionResult.error().Code, IRSerializationErrorCode::UnsupportedVersion);
    }
    IRContext OtherTarget(target::TargetContext(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian));
    const auto TargetResult = readText(OtherTarget, *Text);
    ASSERT_FALSE(TargetResult.has_value());
    EXPECT_EQ(TargetResult.error().Code, IRSerializationErrorCode::TargetMismatch);
  }

  // Input, output, decoded strings, record counts and field counts respect the caller's resource limits.
  TEST(IRTextSerializationTest, EnforcesReadAndWriteResourceLimits)
  {
    IRContext Context(Target);
    Module Source(Context, ModuleID{{core::NamePool::instance().intern("limits")}});
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value());
    IRTextWriteOptions WriteOptions;
    WriteOptions.MaxBytes = Text->size() - 1;
    const auto WriteResult = writeText(Source, WriteOptions);
    ASSERT_FALSE(WriteResult.has_value());
    EXPECT_EQ(WriteResult.error().Code, IRSerializationErrorCode::ResourceLimit);
    for (unsigned Limit = 0; Limit < 4; ++Limit)
    {
      SCOPED_TRACE(Limit);
      IRReadOptions Options;
      if (Limit == 0)
      {
        Options.MaxBytes = Text->size() - 1;
      }
      else if (Limit == 1)
      {
        Options.MaxRecords = 0;
      }
      else if (Limit == 2)
      {
        Options.MaxOperands = 0;
      }
      else
      {
        Options.MaxStringBytes = 0;
      }
      const auto Result = readText(Context, *Text, Options);
      ASSERT_FALSE(Result.has_value());
      EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::ResourceLimit);
    }
  }
} // namespace ink::ir::test
