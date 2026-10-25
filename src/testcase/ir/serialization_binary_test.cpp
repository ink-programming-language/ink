#include "ink/ir/serialization/serialization.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/module/module_manager.h"

#include <gtest/gtest.h>

#include <array>
#include <limits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    core::Name name(std::string_view Text)
    {
      return core::NamePool::instance().intern(Text);
    }

    void set32(std::vector<std::byte> &Bytes, std::size_t Offset, std::uint32_t Value)
    {
      for (unsigned Index = 0; Index < 4; ++Index)
      {
        Bytes[Offset + Index] = static_cast<std::byte>((Value >> (Index * 8U)) & 255U);
      }
    }

    void set64(std::vector<std::byte> &Bytes, std::size_t Offset, std::uint64_t Value)
    {
      for (unsigned Index = 0; Index < 8; ++Index)
      {
        Bytes[Offset + Index] = static_cast<std::byte>((Value >> (Index * 8U)) & 255U);
      }
    }

    std::uint64_t get64(const std::vector<std::byte> &Bytes, std::size_t Offset)
    {
      std::uint64_t Result = 0;
      for (unsigned Index = 0; Index < 8; ++Index)
      {
        Result |= std::uint64_t(std::to_integer<unsigned char>(Bytes[Offset + Index])) << (Index * 8U);
      }
      return Result;
    }
  } // namespace

  // A decoded module borrows the supplied context and can be adopted by that context's manager.
  TEST(IRSerializationBinaryTest, ReadsIntoExternalContextAndPreservesCanonicalBytes)
  {
    IRContext SourceContext(Target);
    Module Source(SourceContext, ModuleID{{name("serialization"), name("binary")}});
    const auto *Integer = SourceContext.typePool().get<IntegerType>(32, true);
    auto *Callable = Source.createGlobalFunction(*SourceContext.typePool().get<FunctionType>(Integer, {Integer}), name("negate"));
    ASSERT_NE(Callable, nullptr);
    Callable->parameter(0)->setName(name("value"));
    auto *Entry = Callable->createBlock(name("entry"));
    IRBuilder Builder(SourceContext);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Negated = Builder.createNeg(*Callable->parameter(0));
    ASSERT_NE(Negated, nullptr);
    ASSERT_NE(Builder.createReturn(*Negated), nullptr);
    ASSERT_TRUE(Source.addExport({name("negate")}, *Callable));

    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    EXPECT_EQ(Bytes->at(0), std::byte{'I'});
    IRContext DestinationContext(Target);
    ModuleManager Manager(DestinationContext);
    auto Loaded = readBinary(DestinationContext, *Bytes);
    ASSERT_TRUE(Loaded) << Loaded.error().Message;
    EXPECT_EQ(&(*Loaded)->irContext(), &DestinationContext);
    EXPECT_EQ((*Loaded)->globalFunction(0)->parameter(0)->type(), DestinationContext.typePool().get<IntegerType>(32, true));
    EXPECT_EQ((*Loaded)->globalFunction(0)->parameter(0)->name(), name("value"));
    auto Again = writeBinary(**Loaded);
    ASSERT_TRUE(Again) << Again.error().Message;
    EXPECT_EQ(*Again, *Bytes);
    auto Adopted = Manager.adoptModule(*Loaded);
    ASSERT_TRUE(Adopted);
    EXPECT_EQ(Manager.moduleCount(), 1U);
    EXPECT_EQ(*Loaded, nullptr);
  }

  // Recursive nominal fields, method dispatch, frozen metadata and lifecycle entry points survive both codecs.
  TEST(IRSerializationBinaryTest, PreservesNominalMetadataAndExactConstantsAcrossCodecs)
  {
    IRContext SourceContext(Target);
    Module Source(SourceContext);
    auto &Types = SourceContext.typePool();
    auto &Constants = SourceContext.constantPool();
    auto *Node = Source.createStructType(16, 8, name("Node"));
    auto *OtherNode = Source.createStructType(16, 8, name("Node"));
    ASSERT_NE(Node, nullptr);
    ASSERT_NE(OtherNode, nullptr);
    const auto *NodePointer = Types.get<PointerType>(Types.get(Node, false));
    ASSERT_TRUE(Node->addField(Field(NodePointer, name("next"), 0, Constants.get<NullPointerConstant>())));
    const auto *Integer = Types.get<IntegerType>(64, false);
    const auto *Wide = Types.get<IntegerType>(129, false);
    const std::array<std::uint64_t, 3> Words = {0x123456789abcdef0ULL, std::numeric_limits<std::uint64_t>::max(), 1};
    auto WideBits = core::IntegerBits::fromWords(129, Words);
    ASSERT_TRUE(WideBits);
    const auto *WideConstant = Constants.get<IntegerConstant>(Wide, *WideBits);
    ASSERT_NE(WideConstant, nullptr);
    auto *WideGlobal = Source.createGlobalVariable(*Wide, name("wide"), false);
    ASSERT_NE(WideGlobal, nullptr);
    ASSERT_TRUE(WideGlobal->setInitializer(WideConstant));
    const auto *BitsType = Types.get<FloatType>(64);
    auto NaNBits = core::IntegerBits::fromUnsigned(64, 0x7ff8123456789abcULL);
    ASSERT_TRUE(NaNBits);
    auto *NaNGlobal = Source.createGlobalVariable(*BitsType, name("nan"));
    ASSERT_NE(NaNGlobal, nullptr);
    ASSERT_TRUE(NaNGlobal->setInitializer(Constants.get<FloatConstant>(BitsType, *NaNBits)));
    ASSERT_TRUE(Node->addField(Field(Integer, name("value"), 8, Constants.get<IntegerConstant>(Integer, std::uint64_t(42)))));

    const auto *VoidSignature = Types.get<FunctionType>(Types.get<VoidType>());
    auto MethodFunction = std::make_unique<Function>(*VoidSignature, name("visit"));
    IRBuilder Builder(SourceContext);
    ASSERT_TRUE(Builder.setInsertPoint(*MethodFunction->createBlock()));
    ASSERT_NE(Builder.createReturn(), nullptr);
    ASSERT_TRUE(Node->addMethod(Method(std::move(MethodFunction), {MethodDispatch::Virtual, true, false, 3})));
    Node->freeze();
    auto *Initializer = Source.createGlobalFunction(*VoidSignature, name("init"));
    ASSERT_TRUE(Builder.setInsertPoint(*Initializer->createBlock()));
    ASSERT_NE(Builder.createReturn(), nullptr);
    ASSERT_TRUE(Source.setInitializationFunction(Initializer));
    ASSERT_TRUE(Source.setDeinitializationFunction(Initializer));
    const auto *Text = Constants.get<StringConstant>(std::string("hello\0world", 11));
    ASSERT_NE(Text, nullptr);
    const auto *Storage = Source.getOrCreateStringStorage(*Text);
    ASSERT_NE(Storage, nullptr);
    Source.imports().push_back({{ModuleID{{name("library")}}, {name("entry")}}, VoidSignature});
    ASSERT_TRUE(Source.addExport({name("Node")}, *Node));
    ASSERT_TRUE(Source.addExport({name("Node")}, *OtherNode));

    auto Binary = writeBinary(Source);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext BinaryContext(Target);
    auto BinaryModule = readBinary(BinaryContext, *Binary);
    ASSERT_TRUE(BinaryModule) << BinaryModule.error().Message;
    EXPECT_NE((*BinaryModule)->structType(0), (*BinaryModule)->structType(1));
    EXPECT_TRUE((*BinaryModule)->structType(0)->isFrozen());
    EXPECT_FALSE((*BinaryModule)->structType(1)->isFrozen());
    EXPECT_EQ((*BinaryModule)->structType(0)->method(0)->virtualSlot(), 3U);
    EXPECT_EQ((*BinaryModule)->initializationFunction(), (*BinaryModule)->globalFunction(0));
    EXPECT_EQ((*BinaryModule)->deinitializationFunction(), (*BinaryModule)->globalFunction(0));
    const auto *LoadedWide = static_cast<const IntegerConstant *>((*BinaryModule)->globalVariable(0)->initializer());
    EXPECT_EQ(LoadedWide->bits(), *WideBits);
    const auto *LoadedNaN = static_cast<const FloatConstant *>((*BinaryModule)->globalVariable(1)->initializer());
    EXPECT_EQ(LoadedNaN->bits(), 0x7ff8123456789abcULL);
    const auto *LoadedString = static_cast<const StringConstant *>((*BinaryModule)->globalVariable(2)->initializer());
    EXPECT_EQ((*BinaryModule)->getOrCreateStringStorage(*LoadedString), (*BinaryModule)->globalVariable(2));
    auto TextIR = writeText(**BinaryModule);
    ASSERT_TRUE(TextIR) << TextIR.error().Message;
    IRContext TextContext(Target);
    auto TextModule = readText(TextContext, *TextIR);
    ASSERT_TRUE(TextModule) << TextModule.error().Message;
    auto BinaryAgain = writeBinary(**TextModule);
    ASSERT_TRUE(BinaryAgain) << BinaryAgain.error().Message;
    EXPECT_EQ(*BinaryAgain, *Binary);
  }

  // Every supported executable opcode retains its operands and block order, including forward block references.
  TEST(IRSerializationBinaryTest, RoundTripsMemoryCallsComparisonsAndControlFlow)
  {
    IRContext Context(Target);
    Module Source(Context);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<IntegerType>(32, true);
    const auto *Signature = Types.get<FunctionType>(Integer, {Integer});
    auto *Callee = Source.createGlobalFunction(*Signature, name("external"), SymbolLinkage::Import, LanguageLinkage::C, FunctionVisibility::Private);
    auto *FunctionValue = Source.createGlobalFunction(*Signature, name("run"));
    ASSERT_NE(Callee, nullptr);
    ASSERT_NE(FunctionValue, nullptr);
    auto *Entry = FunctionValue->createBlock(name("entry"));
    auto *Exit = FunctionValue->createBlock(name("exit"));
    auto *Middle = FunctionValue->createBlock(name("middle"));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Slot = Builder.createAlloca(*Integer, false);
    ASSERT_NE(Slot, nullptr);
    Slot->setName(name("saved"));
    ASSERT_NE(Builder.createInitialize(*FunctionValue->parameter(0), *Slot), nullptr);
    const auto *Zero = Context.constantPool().get<IntegerConstant>(Integer, std::int64_t(0));
    auto *Address = Builder.createGetElementPtr(*Slot->allocatedType(), *Slot, {Zero}, true);
    ASSERT_NE(Address, nullptr);
    auto *Loaded = Builder.createLoad(*Integer, *Address);
    ASSERT_NE(Loaded, nullptr);
    auto *Negative = Builder.createNeg(*Loaded);
    ASSERT_NE(Negative, nullptr);
    auto *Condition = Builder.createICmp(ICmpPredicate::SignedLessThan, *Loaded, *Zero);
    ASSERT_NE(Condition, nullptr);
    ASSERT_NE(Builder.createConditionalBranch(*Condition, *Middle, *Exit), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Middle));
    ASSERT_NE(Builder.createCall(*Callee, {Argument(*Negative, name("value"))}), nullptr);
    ASSERT_NE(Builder.createBranch(*Exit), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Exit));
    ASSERT_NE(Builder.createReturn(*Loaded), nullptr);

    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    IRContext Destination(Target);
    auto Result = readBinary(Destination, *Bytes);
    ASSERT_TRUE(Result) << Result.error().Message;
    const auto *ResultFunction = (*Result)->globalFunction(1);
    EXPECT_EQ(ResultFunction->block(1)->name(), name("exit"));
    EXPECT_EQ(ResultFunction->block(2)->name(), name("middle"));
    const auto *ResultSlot = static_cast<const AllocaInstruction *>(ResultFunction->block(0)->instruction(0));
    EXPECT_TRUE(ResultSlot->isInitialized());
    EXPECT_EQ(ResultSlot->name(), name("saved"));
    EXPECT_FALSE(ResultSlot->allocatedType()->isMutable());
    auto Text = writeText(**Result);
    ASSERT_TRUE(Text) << Text.error().Message;
    IRContext TextContext(Target);
    auto TextResult = readText(TextContext, *Text);
    ASSERT_TRUE(TextResult) << TextResult.error().Message;
    auto Again = writeBinary(**TextResult);
    ASSERT_TRUE(Again) << Again.error().Message;
    EXPECT_EQ(*Again, *Bytes);
  }

  // Each proper prefix is rejected without poisoning the external context for a later valid load.
  TEST(IRSerializationBinaryTest, RejectsEveryTruncatedPrefixAndCanRetry)
  {
    IRContext Context(Target);
    Module Source(Context);
    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    IRContext Destination(Target);
    for (std::size_t Size = 0; Size < Bytes->size(); ++Size)
    {
      EXPECT_FALSE(readBinary(Destination, std::span<const std::byte>(*Bytes).first(Size))) << Size;
    }
    auto Result = readBinary(Destination, *Bytes);
    ASSERT_TRUE(Result) << Result.error().Message;
  }

  // Binary framing validates versions, flags, section boundaries, record kinds, and caller resource limits.
  TEST(IRSerializationBinaryTest, RejectsCorruptIndexesVersionsAndLimits)
  {
    IRContext Context(Target);
    Module Source(Context);
    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    IRContext Destination(Target);
    auto Changed = *Bytes;
    Changed[0] = std::byte{'X'};
    auto Result = readBinary(Destination, Changed);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::InvalidMagic);
    Changed = *Bytes;
    set32(Changed, 8, 999);
    Result = readBinary(Destination, Changed);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::UnsupportedVersion);
    Changed = *Bytes;
    set32(Changed, 12, 1);
    EXPECT_FALSE(readBinary(Destination, Changed));
    Changed = *Bytes;
    set64(Changed, 40, std::numeric_limits<std::uint64_t>::max());
    EXPECT_FALSE(readBinary(Destination, Changed));
    Changed = *Bytes;
    const auto RecordsOffset = static_cast<std::size_t>(get64(Changed, 104));
    set32(Changed, RecordsOffset, 0xffffffffU);
    Result = readBinary(Destination, Changed);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::UnsupportedKind);
    IRReadOptions Limits;
    Limits.MaxRecords = 0;
    Result = readBinary(Destination, *Bytes, Limits);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::ResourceLimit);
    Limits = {};
    Limits.MaxBytes = Bytes->size() - 1;
    EXPECT_FALSE(readBinary(Destination, *Bytes, Limits));
    IRBinaryWriteOptions WriteLimits;
    WriteLimits.MaxBytes = Bytes->size() - 1;
    EXPECT_FALSE(writeBinary(Source, WriteLimits));
  }

  // Files are portable across host encoding but cannot silently change the module's target layout.
  TEST(IRSerializationBinaryTest, RejectsTargetMismatchBeforeConstructingIR)
  {
    IRContext Context(Target);
    Module Source(Context);
    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    IRContext Different(target::TargetContext(target::PointerWidth::Bits32, target::ByteOrder::BigEndian));
    auto Result = readBinary(Different, *Bytes);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::TargetMismatch);
    auto Changed = *Bytes;
    const auto TargetOffset = static_cast<std::size_t>(get64(Changed, 40));
    set64(Changed, TargetOffset + 16, 128);
    Result = readBinary(Context, Changed);
    ASSERT_FALSE(Result);
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::TargetMismatch);
  }

  // Long straight-line SSA chains use iterative dependency resolution independently of type nesting limits.
  TEST(IRSerializationBinaryTest, DecodesLongSSAChainWithoutRecursiveStackGrowth)
  {
    IRContext Context(Target);
    Module Source(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    auto *Callable = Source.createGlobalFunction(*Context.typePool().get<FunctionType>(Integer, {Integer}));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Callable->createBlock()));
    const Value *Current = Callable->parameter(0);
    for (std::size_t Index = 0; Index < 2048; ++Index)
    {
      Current = Builder.createNeg(*Current);
      ASSERT_NE(Current, nullptr);
    }
    ASSERT_NE(Builder.createReturn(*Current), nullptr);
    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    IRContext Destination(Target);
    IRReadOptions Options;
    Options.MaxDepth = 8;
    auto Result = readBinary(Destination, *Bytes, Options);
    ASSERT_TRUE(Result) << Result.error().Message;
    EXPECT_EQ((*Result)->globalFunction(0)->block(0)->instructionCount(), 2049U);
  }

  // Aggregate sharing, nonruntime type exports, variadic signatures and a big-endian target retain their identities.
  TEST(IRSerializationBinaryTest, PreservesAggregateConstantsAndExportedTypeFamilies)
  {
    const target::TargetContext OtherTarget(target::PointerWidth::Bits32, target::ByteOrder::BigEndian);
    IRContext Context(OtherTarget);
    Module Source(Context);
    auto &Types = Context.typePool();
    auto &Constants = Context.constantPool();
    const auto *Bool = Types.get<BoolType>();
    const auto *Array = Types.get<ArrayType>(Bool, 3);
    const auto *True = Constants.get<BoolConstant>(true);
    const auto *Aggregate = Constants.get<ArrayConstant>(Array, {True, Constants.get<BoolConstant>(false), True});
    ASSERT_NE(Aggregate, nullptr);
    auto *Global = Source.createGlobalVariable(*Array, name("flags"));
    ASSERT_NE(Global, nullptr);
    ASSERT_TRUE(Global->setInitializer(Aggregate));
    const auto *Float = Types.get<FloatType>(32);
    auto ZeroBits = core::IntegerBits::fromUnsigned(32, 0x80000000U);
    ASSERT_TRUE(ZeroBits);
    auto *FloatGlobal = Source.createGlobalVariable(*Float, name("negative_zero"));
    ASSERT_NE(FloatGlobal, nullptr);
    ASSERT_TRUE(FloatGlobal->setInitializer(Constants.get<FloatConstant>(Float, *ZeroBits)));
    const auto *Signature = Types.get<FunctionType>(Types.get<VoidType>(), {FunctionTypeParameter(Bool, true)}, false);
    ASSERT_NE(Source.createGlobalFunction(*Signature, name("variadic")), nullptr);
    ASSERT_TRUE(Source.addExport({name("Meta")}, *Types.get<MetaType>()));
    ASSERT_TRUE(Source.addExport({name("Variadic")}, *Types.get<VariadicType>()));
    ASSERT_TRUE(Source.addExport({name("Reference")}, *Types.get<ReferenceType>(Bool, false)));
    ASSERT_TRUE(Source.addExport({name("Class")}, *Source.createClassType(4, 4, name("Class"))));
    ASSERT_TRUE(Source.addExport({name("Interface")}, *Source.createInterfaceType(4, 4, name("Interface"))));
    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    IRContext Destination(OtherTarget);
    auto Result = readBinary(Destination, *Bytes);
    ASSERT_TRUE(Result) << Result.error().Message;
    const auto *LoadedArray = static_cast<const ArrayConstant *>((*Result)->globalVariable(0)->initializer());
    EXPECT_EQ(LoadedArray->elements()[0], LoadedArray->elements()[2]);
    EXPECT_EQ(LoadedArray->elements()[0], Destination.constantPool().get<BoolConstant>(true));
    EXPECT_EQ(static_cast<const FloatConstant *>((*Result)->globalVariable(1)->initializer())->bits(), 0x80000000U);
    EXPECT_TRUE((*Result)->globalFunction(0)->parameter(0)->isVariadic());
    EXPECT_FALSE((*Result)->globalFunction(0)->functionType()->isMutable());
    auto Text = writeText(**Result);
    ASSERT_TRUE(Text) << Text.error().Message;
    IRContext TextContext(OtherTarget);
    auto TextResult = readText(TextContext, *Text);
    ASSERT_TRUE(TextResult) << TextResult.error().Message;
    auto Again = writeBinary(**TextResult);
    ASSERT_TRUE(Again) << Again.error().Message;
    EXPECT_EQ(*Again, *Bytes);
  }

  // Single-byte corruption throughout a file either yields valid verified IR or a recoverable error.
  TEST(IRSerializationBinaryTest, HandlesSingleByteMutationsWithoutAssertions)
  {
    IRContext Context(Target);
    Module Source(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    auto *Callable = Source.createGlobalFunction(*Context.typePool().get<FunctionType>(Integer, {Integer}), name("mutation"));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Callable->createBlock()));
    ASSERT_NE(Builder.createReturn(*Callable->parameter(0)), nullptr);
    auto Bytes = writeBinary(Source);
    ASSERT_TRUE(Bytes) << Bytes.error().Message;
    auto Changed = *Bytes;
    for (std::size_t Index = 0; Index < Changed.size(); ++Index)
    {
      Changed[Index] ^= std::byte{0x80};
      IRContext Destination(Target);
      auto Result = readBinary(Destination, Changed);
      if (Result)
      {
        EXPECT_TRUE(IRVerifier::verify(**Result)) << Index;
      }
      Changed[Index] ^= std::byte{0x80};
    }
  }
} // namespace ink::ir::test
