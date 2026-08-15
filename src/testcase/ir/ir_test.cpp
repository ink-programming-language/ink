#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"
#include "ink/ir/verifier.h"

#include <gtest/gtest.h>

#include <cstddef>
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

    Module makeHelloWorldModule()
    {
      Module Result;
      Result.ByteConstants.push_back({"str.0", "Hello, world!\n"});

      Function WriteStdout;
      WriteStdout.Name = "ink_rt_write_stdout";
      WriteStdout.Kind = FunctionKind::External;
      WriteStdout.Convention = CallingConvention::C;
      WriteStdout.ResultType = TypeKind::I32;
      WriteStdout.ParameterTypes = {TypeKind::ConstBytePointer, TypeKind::PointerSize};
      WriteStdout.HasSideEffects = true;
      Result.Functions.push_back(std::move(WriteStdout));

      auto Call = std::make_unique<CallInstruction>();
      Call->Result = ValueId{0};
      Call->ResultType = TypeKind::I32;
      Call->Callee = FunctionId{0};
      Call->Arguments.push_back(std::make_unique<GlobalAddressOperand>(TypeKind::ConstBytePointer, GlobalId{0}, 0));
      Call->Arguments.push_back(std::make_unique<IntegerConstant>(TypeKind::PointerSize, 14));

      Function Main;
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
        "declare extern \"C\" i32 @ink_rt_write_stdout(const byte*, ptrsize) [sideeffect]\n"
        "\n"
        "define void @main() {\n"
        "entry:\n"
        "  %0 = call i32 @ink_rt_write_stdout(const byte* @str.0[0], ptrsize 14)\n"
        "  ret void\n"
        "}\n";

    // Verifies that IRContext composes around the compilation context and reuses its diagnostic engine.
    TEST(IrContextTest, SharesCompilationDiagnosticEngine)
    {
      TestContext Context;

      EXPECT_EQ(&Context.IR.compilationContext(), &Context.Compilation);
      EXPECT_EQ(&Context.IR.diagnosticEngine(), &Context.Compilation.diagnosticEngine());
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
      EXPECT_STREQ(valueKindName(ValueKind::IntegerConstant), "IntegerConstant");
      EXPECT_STREQ(valueKindName(ValueKind::ValueOperand), "ValueOperand");
      EXPECT_STREQ(valueKindName(ValueKind::GlobalAddressOperand), "GlobalAddressOperand");
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
      static_assert(std::is_abstract_v<Value>);
      static_assert(std::is_base_of_v<Value, IntegerConstant>);
      static_assert(std::is_base_of_v<Value, ValueOperand>);
      static_assert(std::is_base_of_v<Value, GlobalAddressOperand>);

      std::unique_ptr<Value> Constant = std::make_unique<IntegerConstant>(TypeKind::I32, 42);

      EXPECT_EQ(Constant->kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(Constant->type(), TypeKind::I32);
      EXPECT_EQ(static_cast<const IntegerConstant &>(*Constant).value(), 42);
    }

    // Verifies that concrete instructions are polymorphically owned and retain their registered instruction kinds.
    TEST(IrInstructionTest, OwnsConcreteInstructionsThroughAbstractBase)
    {
      static_assert(std::is_abstract_v<Instruction>);
      static_assert(std::is_base_of_v<Instruction, CallInstruction>);
      static_assert(std::is_base_of_v<Instruction, ReturnInstruction>);

      std::unique_ptr<Instruction> Call = std::make_unique<CallInstruction>();
      std::unique_ptr<Instruction> Return = std::make_unique<ReturnInstruction>();

      EXPECT_EQ(Call->kind(), InstructionKind::Call);
      EXPECT_EQ(Return->kind(), InstructionKind::Return);
    }

    // Verifies that the minimum extern-based Hello World module satisfies every current IR invariant.
    TEST(IrVerifierTest, AcceptsExternHelloWorldModule)
    {
      TestContext Context;
      const VerificationResult Result = verify(Context.IR, makeHelloWorldModule());

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.diagnostics().empty());
    }

    // Verifies the deterministic LLVM-style text form for a byte constant, extern call, and void return.
    TEST(IrSerializationTest, SerializesExternHelloWorldDeterministically)
    {
      TestContext Context;

      EXPECT_EQ(serializeSuccessfully(Context.IR, makeHelloWorldModule()), HelloWorldText);
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

    // Verifies that deserialization resolves function and global references declared later in the text.
    TEST(IrSerializationTest, ResolvesForwardGlobalAndExternReferences)
    {
      TestContext Context;
      const std::string ForwardReferenceText =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = call i32 @ink_rt_write_stdout(const byte* @str.0[0], ptrsize 14)\n"
          "  ret void\n"
          "}\n"
          "declare extern \"C\" i32 @ink_rt_write_stdout(const byte*, ptrsize) [sideeffect]\n"
          "@str.0 = private constant [14 x byte] c\"Hello, world!\\0A\"\n";

      DeserializeResult Result = deserialize(Context.IR, ForwardReferenceText);

      ASSERT_TRUE(Result.succeeded());
      EXPECT_EQ(serializeSuccessfully(Context.IR, *Result.module()), HelloWorldText);
    }

    // Verifies that byte serialization preserves embedded zero, quotes, backslashes, and non-ASCII bytes.
    TEST(IrSerializationTest, RoundTripsEveryEscapedByteShapeUsedByConstants)
    {
      TestContext Context;
      Module ModuleValue = makeHelloWorldModule();
      ModuleValue.ByteConstants[0].Data = std::string("\0\"\\\xC3\xA9", 5);
      static_cast<CallInstruction &>(*ModuleValue.Functions[1].Blocks[0].Instructions[0]).Arguments[1] = std::make_unique<IntegerConstant>(TypeKind::PointerSize, 5);

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
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidIrText);
      EXPECT_NE(formatMessage(Result.diagnostics()[0]).find("does not match"), std::string::npos);
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
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidIrText);
      EXPECT_EQ(Result.diagnostics()[0].Span, (core::SourceRange{8, 9}));
      EXPECT_EQ(Consumer.diagnostics(), Result.diagnostics());
    }

    // Verifies that recursive-descent failure propagates explicitly and retains the offending version token range.
    TEST(IrDeserializationTest, RejectsUnsupportedVersionWithoutExceptionControlFlow)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, "inkir 2\n");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Span, (core::SourceRange{6, 7}));
      EXPECT_NE(formatMessage(Result.diagnostics()[0]).find("unsupported InkIR format version"), std::string::npos);
    }

    // Verifies that unresolved external call targets are rejected during reference resolution.
    TEST(IrDeserializationTest, RejectsUnknownCallTarget)
    {
      TestContext Context;
      DeserializeResult Result = deserialize(Context.IR, "inkir 1\ndefine void @main() {\nentry:\n  call void @missing()\n  ret void\n}\n");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidIrText);
      EXPECT_NE(formatMessage(Result.diagnostics()[0]).find("unknown call target"), std::string::npos);
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
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidIrModule);
      EXPECT_NE(formatMessage(Result.diagnostics()[0]).find("global byte address"), std::string::npos);
    }

    // Verifies that a programmatically constructed block without a terminator is rejected and cannot be serialized.
    TEST(IrVerifierTest, RejectsBlockWithoutTerminator)
    {
      TestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      Module ModuleValue = makeHelloWorldModule();
      ModuleValue.Functions[1].Blocks[0].Instructions.pop_back();

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Consumer.diagnostics(), Result.diagnostics());
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidIrModule);
      EXPECT_NE(formatMessage(Result.diagnostics()[0]).find("does not end with a terminator"), std::string::npos);
      const SerializeResult Serialized = serialize(Context.IR, ModuleValue);
      EXPECT_FALSE(Serialized.succeeded());
      EXPECT_FALSE(Serialized.text().has_value());
      EXPECT_EQ(Serialized.diagnostics(), Result.diagnostics());
    }
  } // namespace
} // namespace ink::ir
