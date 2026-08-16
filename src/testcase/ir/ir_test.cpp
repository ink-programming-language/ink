#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"
#include "ink/ir/verifier.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace ink::ir
{
  namespace
  {
    struct TestContext
    {
        core::CompilationContext Compilation;
        IRContext IR{Compilation};
    };

    std::string formatMessage(const core::Diagnostic &DiagnosticEntry)
    {
      return core::DiagnosticFormatter().format(DiagnosticEntry).Message;
    }

    std::string serializeSuccessfully(IRContext &Context, const Module &ModuleValue)
    {
      SerializeResult Result = serialize(Context, ModuleValue);
      if (!Result.succeeded())
      {
        ADD_FAILURE() << "expected InkIR serialization to succeed";
        return {};
      }
      return std::move(*Result.text());
    }

    Module makeHelloWorldModule(IRContext &Context)
    {
      const Type &VoidType = Context.getType(TypeKind::Void);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const Type &PointerSizeType = Context.getType(TypeKind::PointerSize);
      const Type &ConstBytePointerType = Context.getType(TypeKind::ConstBytePointer);
      Module Result(Context);
      Result.ByteConstants.push_back({"str.0", "Hello, world!\n"});

      Function WriteStdout(I32Type);
      WriteStdout.Name = "write";
      WriteStdout.Kind = FunctionKind::External;
      WriteStdout.Convention = CallingConvention::C;
      WriteStdout.ParameterTypes = {&I32Type, &ConstBytePointerType, &PointerSizeType};
      WriteStdout.HasSideEffects = true;
      Result.Functions.push_back(std::move(WriteStdout));

      auto Call = std::make_unique<CallInstruction>(I32Type);
      Call->Result = ValueId{0};
      Call->Callee = FunctionId{0};
      Call->Arguments.push_back(std::make_unique<IntegerConstant>(I32Type, 1));
      Call->Arguments.push_back(std::make_unique<GlobalAddressOperand>(ConstBytePointerType, GlobalId{0}, 0));
      Call->Arguments.push_back(std::make_unique<IntegerConstant>(PointerSizeType, 14));

      Function Main(VoidType);
      Main.Name = "main";
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(Call));
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Main.Blocks.push_back(std::move(Entry));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    const std::string HelloWorldText =
        "inkir 1\n"
        "\n"
        "@str.0 = private constant [14 x byte] c\"Hello, world!\\0A\"\n"
        "\n"
        "declare extern \"C\" i32 @write(i32, const byte*, ptrsize) [sideeffect]\n"
        "\n"
        "define void @main() {\n"
        "entry:\n"
        "  %0 = call i32 @write(i32 1, const byte* @str.0[0], ptrsize 14)\n"
        "  ret void\n"
        "}\n";

    const std::string NativeIoText =
        "inkir 1\n"
        "\n"
        "declare extern \"C\" i32 @read(i32, byte*, ptrsize) [sideeffect]\n"
        "\n"
        "declare extern \"C\" i32 @write(i32, const byte*, ptrsize) [sideeffect]\n";

    const std::string StructText =
        "inkir 1\n"
        "\n"
        "%Pair = type {i32, i32}\n"
        "\n"
        "define i32 @sum_second() {\n"
        "entry:\n"
        "  %0 = insertvalue %Pair zeroinitializer, i32 20, 0\n"
        "  %1 = insertvalue %Pair %0, i32 22, 1\n"
        "  %2 = extractvalue %Pair %1, 1\n"
        "  ret i32 %2\n"
        "}\n";

    // Verifies that IRContext composes around the compilation context and reuses its diagnostic engine.
    TEST(IrContextTest, SharesCompilationDiagnosticEngine)
    {
      TestContext Context;

      EXPECT_EQ(&Context.IR.compilationContext(), &Context.Compilation);
      EXPECT_EQ(&Context.IR.diagnosticEngine(), &Context.Compilation.diagnosticEngine());
    }

    // Verifies that IRContext owns one stable Type object for each primitive TypeKind.
    TEST(IrContextTest, OwnsCanonicalPrimitiveTypes)
    {
      TestContext Context;
      const Type &FirstI32 = Context.IR.getType(TypeKind::I32);
      const Type &SecondI32 = Context.IR.getType(TypeKind::I32);
      const Type &VoidType = Context.IR.getType(TypeKind::Void);

      static_assert(!std::is_abstract_v<Type>);
      EXPECT_EQ(&FirstI32, &SecondI32);
      EXPECT_NE(&FirstI32, &VoidType);
      EXPECT_EQ(FirstI32.kind(), TypeKind::I32);
      EXPECT_EQ(VoidType.kind(), TypeKind::Void);
    }

    // Verifies that IRContext owns distinct named struct types even when their field lists are identical.
    TEST(IrContextTest, OwnsDistinctNamedStructTypes)
    {
      TestContext Context;
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const StructType &First = Context.IR.createStructType("First", {&I32Type});
      const StructType &Second = Context.IR.createStructType("Second", {&I32Type});

      EXPECT_NE(&First, &Second);
      EXPECT_EQ(First.kind(), TypeKind::Struct);
      EXPECT_EQ(First.name(), "First");
      ASSERT_EQ(First.fieldTypes().size(), 1u);
      EXPECT_EQ(First.fieldTypes()[0], &I32Type);
    }

    // Verifies that all instruction metadata comes from the centralized IR definition table.
    TEST(IrDefinitionTest, ExposesRegisteredInstructionMetadata)
    {
      EXPECT_STREQ(instructionKindName(InstructionKind::Call), "Call");
      EXPECT_STREQ(instructionMnemonic(InstructionKind::Call), "call");
      EXPECT_FALSE(isTerminator(InstructionKind::Call));
      EXPECT_STREQ(instructionKindName(InstructionKind::Return), "Return");
      EXPECT_STREQ(instructionMnemonic(InstructionKind::Return), "ret");
      EXPECT_TRUE(isTerminator(InstructionKind::Return));
      EXPECT_STREQ(instructionMnemonic(InstructionKind::InsertValue), "insertvalue");
      EXPECT_STREQ(instructionMnemonic(InstructionKind::ExtractValue), "extractvalue");
      EXPECT_STREQ(valueKindName(ValueKind::IntegerConstant), "IntegerConstant");
      EXPECT_STREQ(valueKindName(ValueKind::ValueOperand), "ValueOperand");
      EXPECT_STREQ(valueKindName(ValueKind::GlobalAddressOperand), "GlobalAddressOperand");
      EXPECT_STREQ(valueKindName(ValueKind::ZeroInitializer), "ZeroInitializer");
    }

    // Verifies that strong IDs expose their index without allowing direct mutation of the stored representation.
    TEST(IrValueTest, EncapsulatesStrongIds)
    {
      constexpr GlobalId Global{3};
      constexpr FunctionId Function{4};
      constexpr ValueId Value{5};

      static_assert(Global.valid());
      static_assert(Function.valid());
      static_assert(Value.valid());
      static_assert(!std::is_aggregate_v<GlobalId>);
      static_assert(!std::is_aggregate_v<FunctionId>);
      static_assert(!std::is_aggregate_v<ValueId>);
      static_assert(!std::is_convertible_v<std::size_t, GlobalId>);
      static_assert(!std::is_convertible_v<std::size_t, FunctionId>);
      static_assert(!std::is_convertible_v<std::size_t, ValueId>);
      static_assert(Global.value() == 3);
      static_assert(Function.value() == 4);
      static_assert(Value.value() == 5);
      static_assert(!GlobalId{}.valid());
      static_assert(!FunctionId{}.valid());
      static_assert(!ValueId{}.valid());
    }

    // Verifies that every operand value carries its type through the abstract Value base class.
    TEST(IrValueTest, OwnsTypedOperandsThroughValueBase)
    {
      TestContext Context;
      static_assert(std::is_abstract_v<Value>);
      static_assert(std::is_base_of_v<Value, IntegerConstant>);
      static_assert(std::is_base_of_v<Value, ValueOperand>);
      static_assert(std::is_base_of_v<Value, GlobalAddressOperand>);

      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      std::unique_ptr<Value> Constant = std::make_unique<IntegerConstant>(I32Type, 42);

      EXPECT_EQ(Constant->kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(&Constant->type(), &I32Type);
      EXPECT_EQ(Constant->type().kind(), TypeKind::I32);
      EXPECT_EQ(static_cast<const IntegerConstant &>(*Constant).value(), 42);
    }

    // Verifies that concrete instructions are polymorphically owned and retain their registered instruction kinds.
    TEST(IrInstructionTest, OwnsConcreteInstructionsThroughAbstractBase)
    {
      TestContext Context;
      static_assert(std::is_abstract_v<Instruction>);
      static_assert(std::is_base_of_v<Instruction, CallInstruction>);
      static_assert(std::is_base_of_v<Instruction, ReturnInstruction>);

      std::unique_ptr<Instruction> Call = std::make_unique<CallInstruction>(Context.IR.getType(TypeKind::Void));
      std::unique_ptr<Instruction> Return = std::make_unique<ReturnInstruction>();

      EXPECT_EQ(Call->kind(), InstructionKind::Call);
      EXPECT_EQ(Return->kind(), InstructionKind::Return);
    }

    // Verifies that the minimum extern-based Hello World module satisfies every current IR invariant.
    TEST(IrVerifierTest, AcceptsExternHelloWorldModule)
    {
      TestContext Context;
      const VerificationResult Result = verify(Context.IR, makeHelloWorldModule(Context.IR));

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.diagnostics().empty());
    }

    // Verifies that a function without parameters cannot skip SSA value zero for its first instruction result.
    TEST(IrVerifierTest, RejectsSkippedFirstSsaResultWithoutParameters)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      CallInstruction &Call = static_cast<CallInstruction &>(*ModuleValue.Functions[1].Blocks[0].Instructions[0]);
      Call.Result = ValueId{1};

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrNonConsecutiveSsaResult>({}, "call", "main", std::uint64_t{0}, std::uint64_t{1});
      EXPECT_EQ(Result.diagnostics()[0].Kind, Expected.Kind);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, Expected.Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::InternalCompilerError);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "call in function @main defines SSA value %1; expected %0");
    }

    // Verifies that the first instruction result after one parameter cannot skip the next SSA value one.
    TEST(IrVerifierTest, RejectsSkippedFirstSsaResultAfterParameter)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      Function &Main = ModuleValue.Functions[1];
      Main.ParameterTypes.push_back(&Context.IR.getType(TypeKind::I32));
      CallInstruction &Call = static_cast<CallInstruction &>(*Main.Blocks[0].Instructions[0]);
      Call.Result = ValueId{2};

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrNonConsecutiveSsaResult>({}, "call", "main", std::uint64_t{1}, std::uint64_t{2});
      EXPECT_EQ(Result.diagnostics()[0].Kind, Expected.Kind);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, Expected.Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::InternalCompilerError);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "call in function @main defines SSA value %2; expected %1");
    }

    // Verifies that a huge valid SSA result after a parameter is rejected as non-consecutive without indexing storage by that value.
    TEST(IrVerifierTest, RejectsHugeFirstSsaResultAfterParameter)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      Function &Main = ModuleValue.Functions[1];
      Main.ParameterTypes.push_back(&Context.IR.getType(TypeKind::I32));
      CallInstruction &Call = static_cast<CallInstruction &>(*Main.Blocks[0].Instructions[0]);
      Call.Result = ValueId{InvalidId - 1};

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrNonConsecutiveSsaResult>({}, "call", "main", std::uint64_t{1}, static_cast<std::uint64_t>(InvalidId - 1));
      EXPECT_EQ(Result.diagnostics()[0].Kind, Expected.Kind);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, Expected.Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::InternalCompilerError);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "call in function @main defines SSA value %" + std::to_string(InvalidId - 1) + "; expected %1");
    }

    // Verifies that a parameter at SSA value zero followed by an instruction result at value one remains valid.
    TEST(IrVerifierTest, AcceptsConsecutiveFirstSsaResultAfterParameter)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      Function &Main = ModuleValue.Functions[1];
      Main.ParameterTypes.push_back(&Context.IR.getType(TypeKind::I32));
      CallInstruction &Call = static_cast<CallInstruction &>(*Main.Blocks[0].Instructions[0]);
      Call.Result = ValueId{1};

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.diagnostics().empty());
    }

    // Verifies that a gap after an already valid instruction result reports the next missing SSA value rather than only validating the first result.
    TEST(IrVerifierTest, RejectsGapAfterFirstSsaResult)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      Function &Main = ModuleValue.Functions[1];
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &PointerSizeType = Context.IR.getType(TypeKind::PointerSize);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      auto SecondCall = std::make_unique<CallInstruction>(I32Type);
      SecondCall->Result = ValueId{2};
      SecondCall->Callee = FunctionId{0};
      SecondCall->Arguments.push_back(std::make_unique<IntegerConstant>(I32Type, 1));
      SecondCall->Arguments.push_back(std::make_unique<GlobalAddressOperand>(ConstBytePointerType, GlobalId{0}, 0));
      SecondCall->Arguments.push_back(std::make_unique<IntegerConstant>(PointerSizeType, 14));
      Main.Blocks[0].Instructions.insert(Main.Blocks[0].Instructions.end() - 1, std::move(SecondCall));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrNonConsecutiveSsaResult>({}, "call", "main", std::uint64_t{1}, std::uint64_t{2});
      EXPECT_EQ(Result.diagnostics()[0].Kind, Expected.Kind);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, Expected.Arguments);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "call in function @main defines SSA value %2; expected %1");
    }

    // Verifies the deterministic LLVM-style text form for a byte constant, extern call, and void return.
    TEST(IrSerializationTest, SerializesExternHelloWorldDeterministically)
    {
      TestContext Context;

      EXPECT_EQ(serializeSuccessfully(Context.IR, makeHelloWorldModule(Context.IR)), HelloWorldText);
    }

    // Verifies that serialized Hello World IR round-trips without changing its canonical text.
    TEST(IrSerializationTest, RoundTripsExternHelloWorld)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, HelloWorldText);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      EXPECT_TRUE(Result.diagnostics().empty());
      EXPECT_EQ(serializeSuccessfully(Context.IR, *Result.module()), HelloWorldText);
    }

    // Verifies that mutable byte pointers in native read and write declarations survive canonical InkIR serialization and deserialization.
    TEST(IrSerializationTest, RoundTripsNativeIoDeclarations)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, NativeIoText);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      ASSERT_EQ(Result.module()->Functions.size(), 2u);
      const Function &Read = Result.module()->Functions[0];
      const Function &Write = Result.module()->Functions[1];
      EXPECT_EQ(Read.Convention, CallingConvention::C);
      ASSERT_EQ(Read.ParameterTypes.size(), 3u);
      EXPECT_EQ(Read.ParameterTypes[1]->kind(), TypeKind::BytePointer);
      EXPECT_EQ(Write.Convention, CallingConvention::C);
      ASSERT_EQ(Write.ParameterTypes.size(), 3u);
      EXPECT_EQ(Write.ParameterTypes[1]->kind(), TypeKind::ConstBytePointer);
      EXPECT_EQ(serializeSuccessfully(Context.IR, *Result.module()), NativeIoText);
    }

    // Verifies that named struct declarations and aggregate SSA instructions round-trip through canonical InkIR text.
    TEST(IrSerializationTest, RoundTripsNamedStructAndAggregateInstructions)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, StructText);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      ASSERT_EQ(Result.module()->StructTypes.size(), 1u);
      EXPECT_EQ(Result.module()->StructTypes[0]->name(), "Pair");
      EXPECT_EQ(serializeSuccessfully(Context.IR, *Result.module()), StructText);
    }

    // Verifies that deserialization resolves function and global references declared later in the text.
    TEST(IrSerializationTest, ResolvesForwardGlobalAndExternReferences)
    {
      TestContext Context;
      const std::string ForwardReferenceText =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = call i32 @write(i32 1, const byte* @str.0[0], ptrsize 14)\n"
          "  ret void\n"
          "}\n"
          "declare extern \"C\" i32 @write(i32, const byte*, ptrsize) [sideeffect]\n"
          "@str.0 = private constant [14 x byte] c\"Hello, world!\\0A\"\n";

      DeserializeResult Result = deserialize(Context.IR, ForwardReferenceText);

      ASSERT_TRUE(Result.succeeded());
      EXPECT_EQ(serializeSuccessfully(Context.IR, *Result.module()), HelloWorldText);
    }

    // Verifies that byte serialization preserves embedded zero, quotes, backslashes, and non-ASCII bytes.
    TEST(IrSerializationTest, RoundTripsEveryEscapedByteShapeUsedByConstants)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      ModuleValue.ByteConstants[0].Data = std::string("\0\"\\\xC3\xA9", 5);
      static_cast<CallInstruction &>(*ModuleValue.Functions[1].Blocks[0].Instructions[0]).Arguments[2] = std::make_unique<IntegerConstant>(Context.IR.getType(TypeKind::PointerSize), 5);

      const std::string Text = serializeSuccessfully(Context.IR, ModuleValue);
      DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      EXPECT_EQ(Result.module()->ByteConstants[0].Data, ModuleValue.ByteConstants[0].Data);
      EXPECT_EQ(serializeSuccessfully(Context.IR, *Result.module()), Text);
    }

    // Verifies that the textual array size must match the decoded byte-string payload.
    TEST(IrDeserializationTest, RejectsMismatchedByteConstantSize)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, "inkir 1\n@value = private constant [2 x byte] c\"x\"\n");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrByteConstantSizeMismatch>({35, 36}, std::uint64_t{2}, std::uint64_t{1});
      EXPECT_EQ(Result.diagnostics()[0], Expected);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "declared byte constant size 2 does not match decoded string length 1");
    }

    // Verifies that the InkIR lexer returns a located Core diagnostic and publishes the same value through IRContext.
    TEST(IrDeserializationTest, ReportsLocatedLexerDiagnosticThroughIrContext)
    {
      TestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      DeserializeResult Result = deserialize(Context.IR, "inkir 1\n?");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrUnexpectedCharacter>({8, 9}, "?");
      EXPECT_EQ(Result.diagnostics()[0], Expected);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "unexpected character '?'");
      EXPECT_EQ(Consumer.diagnostics(), Result.diagnostics());
    }

    // Verifies that recursive-descent failure propagates explicitly and retains the offending version token range.
    TEST(IrDeserializationTest, RejectsUnsupportedVersionWithoutExceptionControlFlow)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, "inkir 2\n");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic Expected = core::makeDiagnostic<core::DiagnosticKind::IrUnsupportedFormatVersion>({6, 7}, std::uint64_t{2}, std::uint64_t{1});
      EXPECT_EQ(Result.diagnostics()[0], Expected);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "unsupported InkIR format version 2; expected 1");
    }

    // Verifies that unresolved external call targets are rejected during reference resolution.
    TEST(IrDeserializationTest, RejectsUnknownCallTarget)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, "inkir 1\ndefine void @main() {\nentry:\n  call void @missing()\n  ret void\n}\n");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrUnknownCallTarget);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, core::makeDiagnostic<core::DiagnosticKind::IrUnknownCallTarget>({}, "missing").Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "unknown call target @missing");
    }

    // Verifies that type annotations on global-address operands are checked against the extern signature.
    TEST(IrDeserializationTest, RejectsMismatchedExternArgumentType)
    {
      TestContext Context;
      std::string InvalidText = HelloWorldText;
      const std::size_t TypeOffset = InvalidText.find("const byte* @str.0[0]");
      ASSERT_NE(TypeOffset, std::string::npos);
      InvalidText.replace(TypeOffset, std::string("const byte*").size(), "ptrsize");

      DeserializeResult Result = deserialize(Context.IR, InvalidText);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_FALSE(Result.diagnostics().empty());
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrGlobalAddressWrongType);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, core::makeDiagnostic<core::DiagnosticKind::IrGlobalAddressWrongType>({}, "main").Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "global byte address in function @main must have type 'const byte*'");
    }

    // Verifies that immutable global byte storage cannot be presented as a mutable byte pointer to a native read call.
    TEST(IrDeserializationTest, RejectsMutableAddressOfGlobalByteConstant)
    {
      TestContext Context;
      const std::string InvalidText =
          "inkir 1\n"
          "@data = private constant [1 x byte] c\"x\"\n"
          "declare extern \"C\" i32 @read(i32, byte*, ptrsize) [sideeffect]\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = call i32 @read(i32 0, byte* @data[0], ptrsize 1)\n"
          "  ret i32 %0\n"
          "}\n";

      DeserializeResult Result = deserialize(Context.IR, InvalidText);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrGlobalAddressWrongType);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, core::makeDiagnostic<core::DiagnosticKind::IrGlobalAddressWrongType>({}, "main").Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::User);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "global byte address in function @main must have type 'const byte*'");
    }

    // Verifies that a programmatically constructed block without a terminator is rejected and cannot be serialized.
    TEST(IrVerifierTest, RejectsBlockWithoutTerminator)
    {
      TestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      Module ModuleValue = makeHelloWorldModule(Context.IR);
      ModuleValue.Functions[1].Blocks[0].Instructions.pop_back();

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Consumer.diagnostics(), Result.diagnostics());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrBlockMissingTerminator);
      EXPECT_EQ(Result.diagnostics()[0].Arguments, core::makeDiagnostic<core::DiagnosticKind::IrBlockMissingTerminator>({}, "main", "entry").Arguments);
      EXPECT_EQ(Result.diagnostics()[0].classification(), core::DiagnosticClass::InternalCompilerError);
      EXPECT_EQ(formatMessage(Result.diagnostics()[0]), "basic block entry in function @main does not end with a terminator");
      const SerializeResult Serialized = serialize(Context.IR, ModuleValue);
      EXPECT_FALSE(Serialized.succeeded());
      EXPECT_FALSE(Serialized.text().has_value());
      EXPECT_EQ(Serialized.diagnostics(), Result.diagnostics());
    }
  } // namespace
} // namespace ink::ir
