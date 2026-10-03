#include "../execution/ffi/external_call_test_support.h"

#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/execution/support/execution_diagnostic.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/parser/parser.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <string>
#include <string_view>

#if defined(_WIN32) || defined(__linux__)
namespace ink::semantic::test
{
  using namespace ink::ir;
  using execution::test::NativePipe;
  using execution::test::writeDeclaration;
  using execution::test::writeReturnType;
  using execution::test::WriteReturnWidth;
  using execution::test::WriteSymbol;

  namespace
  {
    class ExternalAnalysis final
    {
      public:
        explicit ExternalAnalysis(std::string_view Source, core::TargetContext Target = core::TargetContext::native())
            : Compilation(Target),
              Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        Module *analyze()
        {
          return Analyzer{}.analyze(Context, Parsed);
        }

        const Value *returnedValue(Module &Owner, std::string_view Name)
        {
          const auto *Binding = NameResolver(Context).lookupMember(Owner, Context.namePool().find(Name));
          if (!Binding || Binding->targets().size() != 1 || !Function::classof(Binding->targets().front()))
          {
            return nullptr;
          }
          const auto &Function = static_cast<const ir::Function &>(*Binding->targets().front());
          if (!Function.entryBlock())
          {
            return nullptr;
          }
          for (const auto &Instruction : Function.entryBlock()->values())
          {
            if (ReturnInstruction::classof(Instruction.get()))
            {
              return static_cast<const ReturnInstruction &>(*Instruction).returnedValue();
            }
          }
          return nullptr;
        }

        void expectReturn(Module &Owner, std::string_view Name, std::uint64_t Expected, std::uint32_t Width = WriteReturnWidth)
        {
          SCOPED_TRACE(Name);
          const auto *Binding = NameResolver(Context).lookupMember(Owner, Context.namePool().find(Name));
          ASSERT_NE(Binding, nullptr);
          ASSERT_EQ(Binding->targets().size(), 1U);
          ASSERT_TRUE(Function::classof(Binding->targets().front()));
          const auto &Function = static_cast<const ir::Function &>(*Binding->targets().front());
          ASSERT_NE(Function.entryBlock(), nullptr);
          const Value *Returned = nullptr;
          for (const auto &Instruction : Function.entryBlock()->values())
          {
            if (ReturnInstruction::classof(Instruction.get()))
            {
              Returned = static_cast<const ReturnInstruction &>(*Instruction).returnedValue();
            }
          }
          ASSERT_TRUE(IntegerConstant::classof(Returned));
          const auto &Bits = static_cast<const IntegerConstant &>(*Returned).value();
          EXPECT_EQ(Bits.bitWidth(), Width);
          ASSERT_EQ(Bits.words().size(), 1U);
          EXPECT_EQ(Bits.words().front(), Expected);
        }

        void expectExecutionFailure(execution::ExecutionStatus Status)
        {
          ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
          const core::Diagnostic &Diagnostic = Diagnostics.diagnostics().front();
          const auto Expected = execution::makeExecutionDiagnostic(Status, {}, {}, "compile-time execution failed");
          ASSERT_TRUE(Expected);
          EXPECT_EQ(Diagnostic.Kind, Expected->Kind);
          EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostic).Message, core::DiagnosticFormatter{}.format(*Expected).Message);
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };
  } // namespace

  // Ink source reaches the host write symbol and emits the exact Chinese UTF-8 and embedded NUL bytes to a private pipe.
  TEST(SemanticExternalCallTest, WritesUtf8AndEmbeddedNullBytesFromInkSource)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    const std::string Source = writeDeclaration() + "comptime var Written: " + writeReturnType() + " = " + std::string(WriteSymbol) + "(" + std::to_string(Pipe.writer()) + ", \"中文\\0尾\", 10); func Read(): " + writeReturnType() + " { return comptime Written; }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Read", 10);
    std::string Expected = "\xe4\xb8\xad\xe6\x96\x87";
    Expected.push_back('\0');
    Expected += "\xe5\xb0\xbe";
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, Expected);
  }

  // A source-level void-pointer declaration accepts a zero-byte request without producing host output.
  TEST(SemanticExternalCallTest, SupportsZeroLengthWriteThroughVoidPointer)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    const std::string Source = writeDeclaration(true) + "comptime var Written: " + writeReturnType() + " = " + std::string(WriteSymbol) + "(" + std::to_string(Pipe.writer()) + ", \"\", 0); func Read(): " + writeReturnType() + " { return comptime Written; }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Read", 0);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_TRUE(Output.empty());
  }

  // A missing host symbol produces a specific compile-time diagnostic rather than a missing Ink body error.
  TEST(SemanticExternalCallTest, DiagnosesMissingProcessSymbol)
  {
    ExternalAnalysis Input("import \"C\" func InkMissingExternalSymbol94c8f17e(X: i32): i32; comptime InkMissingExternalSymbol94c8f17e(1);");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    Input.expectExecutionFailure(execution::ExecutionStatus::SymbolNotFound);
  }

  // Nested compile-time calls execute stored IR write instructions separately and return the sum of actual byte counts.
  TEST(SemanticExternalCallTest, ExecutesNativeWritesInsideNestedCompileTimeFunctions)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    std::string Source = writeDeclaration();
    Source += "comptime func Emit(Fd: i32): " + writeReturnType() + " { return " + std::string(WriteSymbol) + "(Fd, \"层\", 3); }\n";
    Source += "comptime func Twice(Fd: i32): " + writeReturnType() + " { var First = Emit(Fd); var Second = Emit(Fd); return First + Second; }\n";
    Source += "comptime var Written = Twice(" + std::to_string(Pipe.writer()) + "); func Read(): " + writeReturnType() + " { return comptime Written; }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Read", 6);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "\xe5\xb1\x82\xe5\xb1\x82");
  }

  // Reusing a compile-time initializer result in several functions does not repeat its irreversible native write.
  TEST(SemanticExternalCallTest, DoesNotReplayNativeInitializerWhenItsResultIsReused)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    std::string Source = writeDeclaration();
    Source += "comptime var Written = " + std::string(WriteSymbol) + "(" + std::to_string(Pipe.writer()) + ", \"X\", 1);\n";
    Source += "func First(): " + writeReturnType() + " { return comptime Written; }\n";
    Source += "func Second(): " + writeReturnType() + " { return comptime Written; }\n";
    Source += "func Twice(): " + writeReturnType() + " { return comptime(Written + Written); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "First", 1);
    Input.expectReturn(*Result, "Second", 1);
    Input.expectReturn(*Result, "Twice", 2);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "X");
  }

  // An explicit compile-time branch is removed during function lowering before its native call can execute.
  TEST(SemanticExternalCallTest, SkipsNativeWriteInUnselectedCompileTimeBranch)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    std::string Source = writeDeclaration();
    Source += "comptime func Maybe(Fd: i32): " + writeReturnType() + " { comptime if (false) { return " + std::string(WriteSymbol) + "(Fd, \"X\", 1); } return 0; }\n";
    Source += "func Read(): " + writeReturnType() + " { return comptime Maybe(" + std::to_string(Pipe.writer()) + "); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Read", 0);
    const auto *Binding = NameResolver(Input.Context).lookupMember(*Result, Input.Context.namePool().find("Maybe"));
    ASSERT_NE(Binding, nullptr);
    ASSERT_EQ(Binding->targets().size(), 1U);
    ASSERT_TRUE(Function::classof(Binding->targets().front()));
    EXPECT_TRUE(static_cast<const Function *>(Binding->targets().front())->hasBody());
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_TRUE(Output.empty());
  }

  // Explicit cross-target compilation reports a host ABI error before any external function can run.
  TEST(SemanticExternalCallTest, RejectsHostCallsForNonNativeCompilationTargets)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    const auto Native = core::TargetContext::native();
    ExternalAnalysis Input(writeDeclaration() + "comptime " + std::string(WriteSymbol) + "(" + std::to_string(Pipe.writer()) + ", \"A\", 1);", core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-E0016\\]");
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_TRUE(Output.empty());
  }

  // Distinct C library names resolve through the same bridge and retain their actual argument and return signatures.
  TEST(SemanticExternalCallTest, CallsAbsStrlenAndStrcmpFromSource)
  {
    const std::uint32_t SizeWidth = sizeof(std::size_t) * 8;
    std::string Source = "import \"C\" func abs(Value: i32): i32; import \"C\" func strlen(Value: *u8): u" + std::to_string(SizeWidth) + "; import \"C\" func strcmp(Left: *u8, Right: *u8): i32;\n";
    Source += "comptime const Text = \"中文\";\n";
    Source += "func Absolute(): i32 { return comptime abs(-21); }\n";
    Source += "func Length(): u" + std::to_string(SizeWidth) + " { return comptime strlen(Text); }\n";
    Source += "func Equal(): i32 { return comptime strcmp(\"same\", \"same\"); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Absolute", 21, 32);
    Input.expectReturn(*Result, "Length", 6, SizeWidth);
    Input.expectReturn(*Result, "Equal", 0, 32);
  }

  // Exported functions with no arguments and void results execute from Ink without requiring a special host adapter.
  TEST(SemanticExternalCallTest, CallsExportedZeroArgumentAndVoidFunctions)
  {
    const std::string Source = "import \"C\" func inkTestExternalZero(): i32; import \"C\" func inkTestExternalSet(Value: i32): void; import \"C\" func inkTestExternalGet(): i32; comptime inkTestExternalSet(4567); func Zero(): i32 { return comptime inkTestExternalZero(); } func Observed(): i32 { return comptime inkTestExternalGet(); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Zero", 37, 32);
    Input.expectReturn(*Result, "Observed", 4567, 32);
  }

  // Signed and unsigned integer arguments preserve every supported width and high bits through source-level calls.
  TEST(SemanticExternalCallTest, PreservesMixedIntegerWidthsAcrossNativeCalls)
  {
    std::string Source = "import \"C\" func inkTestExternalMix(Small: i8, Medium: u16, Signed: i32, Wide: u64): i64;\n";
    Source += "import \"C\" func inkTestExternalNarrow(Byte: u8, Half: i16, Word: u32, Wide: i64): u64;\n";
    Source += "func Mixed(): i64 { return comptime inkTestExternalMix(-7, 60000, -100000, 17293822569102704640); }\n";
    Source += "func Narrow(): u64 { return comptime inkTestExternalNarrow(255, -30000, 4000000000, -9999999983); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Mixed", static_cast<std::uint64_t>(-39992), 64);
    Input.expectReturn(*Result, "Narrow", 4000000272ULL, 64);
  }

  // Ten integer arguments verify native stack arguments in addition to the ABI's argument registers.
  TEST(SemanticExternalCallTest, PreservesArgumentsBeyondNativeRegisterCapacity)
  {
    const std::string Source = "import \"C\" func inkTestExternalMany(A: i32, B: i32, C: i32, D: i32, E: i32, F: i32, G: i32, H: i32, I: i32, J: i32): i64; func Read(): i64 { return comptime inkTestExternalMany(1, 2, 3, 4, 5, 6, 7, 8, 9, 10); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "Read", 385, 64);
  }

  // A native boolean return becomes an Ink boolean constant rather than an integer ABI carrier.
  TEST(SemanticExternalCallTest, PreservesBooleanNativeReturnType)
  {
    ExternalAnalysis Input("import \"C\" func inkTestExternalNot(Value: bool): bool; func Read(): bool { return comptime inkTestExternalNot(false); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Value *Returned = Input.returnedValue(*Result, "Read");
    ASSERT_TRUE(BoolConstant::classof(Returned));
    EXPECT_TRUE(static_cast<const BoolConstant &>(*Returned).value());
  }

  // Native float returns feed a mixed-precision call and survive storage in a compile-time variable before runtime lowering.
  TEST(SemanticExternalCallTest, PreservesFloatingValuesAcrossNestedNativeCalls)
  {
    const std::string Source = "import \"C\" func inkTestExternalFloatSeed32(): f32; import \"C\" func inkTestExternalFloatSeed64(): f64; import \"C\" func inkTestExternalFloat(Left: f32, Right: f64): f64; comptime var Stored = inkTestExternalFloat(inkTestExternalFloatSeed32(), inkTestExternalFloatSeed64()); func Read(): f64 { return comptime Stored; }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Value *Returned = Input.returnedValue(*Result, "Read");
    ASSERT_TRUE(FloatConstant::classof(Returned));
    EXPECT_EQ(static_cast<const FloatConstant &>(*Returned).value(), FloatBits(64, std::bit_cast<std::uint64_t>(5.25)));
  }

  // Native pointer arguments preserve embedded NUL bytes, add a terminator, and isolate writes from interned source strings.
  TEST(SemanticExternalCallTest, IsolatesNativeStringMutationAndPreservesEmbeddedNulls)
  {
    std::string Source = "import \"C\" func inkTestExternalByte(Buffer: *u8, Index: u32): u32; import \"C\" func inkTestExternalMutate(Buffer: *u8): u32;\n";
    Source += "comptime const Text = \"A\\0B\"; comptime const MutableText = \"ABC\";\n";
    Source += "func NullByte(): u32 { return comptime inkTestExternalByte(Text, 1); }\n";
    Source += "func TailByte(): u32 { return comptime inkTestExternalByte(Text, 2); }\n";
    Source += "func Terminator(): u32 { return comptime inkTestExternalByte(Text, 3); }\n";
    Source += "func Mutated(): u32 { return comptime inkTestExternalMutate(MutableText); }\n";
    Source += "func Original(): u32 { return comptime inkTestExternalByte(MutableText, 0); }";
    ExternalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectReturn(*Result, "NullByte", 0, 32);
    Input.expectReturn(*Result, "TailByte", 'B', 32);
    Input.expectReturn(*Result, "Terminator", 0, 32);
    Input.expectReturn(*Result, "Mutated", 'Z', 32);
    Input.expectReturn(*Result, "Original", 'A', 32);
  }

  // Unsupported integer and floating ABI types fail before the resolved host function can be called.
  TEST(SemanticExternalCallTest, RejectsUnsupportedAbiTypesBeforeNativeExecution)
  {
    const std::string_view Sources[] = {
        "import \"C\" func inkTestExternalZero(): i128; comptime inkTestExternalZero();",
        "import \"C\" func inkTestExternalZero(): f16; comptime inkTestExternalZero();",
    };
    for (std::string_view Source : Sources)
    {
      SCOPED_TRACE(Source);
      ExternalAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      Input.expectExecutionFailure(execution::ExecutionStatus::UnsupportedExternalSignature);
    }
  }
} // namespace ink::semantic::test
#endif
