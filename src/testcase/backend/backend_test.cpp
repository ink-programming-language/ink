#include "ink/backend/backend.h"
#include "ink/frontend/compiler.h"
#include "ink/ir/ir.h"
#include "ink/target/target_context.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ink::backend
{
  namespace
  {
    target::TargetKey hostTargetKey()
    {
      const target::TargetContext Host = target::TargetContext::host();
      return Host.key();
    }

    ir::VerifiedClosedModule close(ir::IrBuilder &Builder, target::TargetKey Key = hostTargetKey())
    {
      ir::IrStagedVerificationResult Staged = ir::verifyStaged(Builder.finish());
      if (!Staged.succeeded())
      {
        throw std::logic_error("backend test fixture did not produce verified staged InkIR");
      }
      ir::IrClosedVerificationResult Closed = ir::closeAndVerify(Staged.takeVerified(), std::move(Key));
      if (!Closed.succeeded())
      {
        throw std::logic_error("backend test fixture did not produce verified closed InkIR");
      }
      return Closed.takeVerified();
    }

    std::filesystem::path unusedObjectPath()
    {
      static std::atomic<std::uint64_t> Counter{0};
      const std::uint64_t Time = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
      for (std::uint64_t Attempt = 0; Attempt < 100; ++Attempt)
      {
        const std::filesystem::path Candidate = std::filesystem::temp_directory_path() / ("ink-backend-" + std::to_string(Time) + "-" + std::to_string(Counter.fetch_add(1)) + ".obj");
        if (!std::filesystem::exists(Candidate))
        {
          return Candidate;
        }
      }
      throw std::runtime_error("could not reserve an unused backend test object path");
    }

    ir::VerifiedClosedModule arithmeticModule()
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
      const ir::IrTypeId I64 = Builder.integerType(64, ir::IrSignedness::Signed);
      const ir::IrTypeId IdentitySignature = Builder.functionType({I32}, I32);
      const ir::IrFunctionId Identity = Builder.addFunction("identity", IdentitySignature, ir::IrFunctionKind::External);
      const ir::IrTypeId Signature = Builder.functionType({I32}, I32);
      const ir::IrFunctionId Function = Builder.addFunction("calculate", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function, {I32});
      const ir::IrBuiltBlock TrueBlock = Builder.addBlock(Function, {I32});
      const ir::IrBuiltBlock FalseBlock = Builder.addBlock(Function, {I32});
      const ir::IrBuiltBlock MergeBlock = Builder.addBlock(Function, {I32});
      const ir::IrConstantId TwoConstant = Builder.integerConstant(I32, 2);
      const ir::IrValueId Two = Builder.createIntegerConstant(Entry.Block, TwoConstant);
      const ir::IrValueId Place = Builder.createAlloca(Entry.Block, I32, ir::IrPlaceAccess::ReadWrite);
      Builder.createStore(Entry.Block, Place, Entry.Arguments[0]);
      const ir::IrValueId Loaded = Builder.createLoad(Entry.Block, Place);
      const ir::IrValueId Sum = Builder.createIntegerBinary(Entry.Block, ir::IrOpcode::IntAdd, Loaded, Two);
      const ir::IrValueId Product = Builder.createIntegerBinary(Entry.Block, ir::IrOpcode::IntMul, Sum, Two);
      const ir::IrValueId Negated = Builder.createIntegerNegate(Entry.Block, Product);
      Builder.createIntegerCast(Entry.Block, Negated, I64);
      const ir::IrValueId Condition = Builder.createIntegerCompare(Entry.Block, ir::IrComparePredicate::SignedGreater, Sum, Loaded);
      Builder.createConditionalBranch(Entry.Block, Condition, TrueBlock.Block, {Sum}, FalseBlock.Block, {Loaded});
      const ir::IrBuiltOperation Call = Builder.createDirectCall(TrueBlock.Block, Identity, {TrueBlock.Arguments[0]});
      Builder.createBranch(TrueBlock.Block, MergeBlock.Block, {Call.Results[0]});
      const ir::IrValueId Difference = Builder.createIntegerBinary(FalseBlock.Block, ir::IrOpcode::IntSub, FalseBlock.Arguments[0], Two);
      Builder.createBranch(FalseBlock.Block, MergeBlock.Block, {Difference});
      Builder.createReturn(MergeBlock.Block, MergeBlock.Arguments[0]);
      return close(Builder);
    }

    ir::VerifiedClosedModule voidModule(target::TargetKey Key, std::string Name)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId Signature = Builder.functionType({}, std::nullopt);
      const ir::IrFunctionId Function = Builder.addFunction(std::move(Name), Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function);
      Builder.createReturn(Entry.Block);
      return close(Builder, std::move(Key));
    }

    // Verifies LLVM lowering uses synthetic entry edges and PHIs while preserving memory, calls, comparisons, signed extension, and wrapping integer arithmetic without poison flags.
    TEST(BackendLlvmTextTest, LowersFirstSliceControlFlowAndMemory)
    {
      const ir::VerifiedClosedModule Closed = arithmeticModule();
      BackendResult<std::string> Result = emitLlvmText(Closed);

      ASSERT_TRUE(Result.succeeded()) << Result.error().Message;
      const std::string &Text = Result.value();
      EXPECT_NE(Text.find("target datalayout ="), std::string::npos);
      EXPECT_NE(Text.find("target triple = \"" + hostTargetKey().Triple + "\""), std::string::npos);
      EXPECT_NE(Text.find("phi i32"), std::string::npos);
      EXPECT_NE(Text.find("alloca i32"), std::string::npos);
      EXPECT_NE(Text.find("load i32"), std::string::npos);
      EXPECT_NE(Text.find("store i32"), std::string::npos);
      EXPECT_NE(Text.find("call i32 @identity"), std::string::npos);
      EXPECT_NE(Text.find("sext i32"), std::string::npos);
      EXPECT_NE(Text.find("icmp sgt i32"), std::string::npos);
      EXPECT_EQ(Text.find(" nsw "), std::string::npos);
      EXPECT_EQ(Text.find(" nuw "), std::string::npos);
    }

    // Verifies two conditional edges targeting the same InkIR block retain distinct arguments through separate LLVM edge blocks and verifier-valid PHI predecessors.
    TEST(BackendLlvmTextTest, SplitsSameTargetConditionalEdgesForPhiArguments)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId Bool = Builder.boolType();
      const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
      const ir::IrTypeId Signature = Builder.functionType({Bool}, I32);
      const ir::IrFunctionId Function = Builder.addFunction("select_same_target", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function, {Bool});
      const ir::IrBuiltBlock Merge = Builder.addBlock(Function, {I32});
      const ir::IrValueId TrueValue = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 42));
      const ir::IrValueId FalseValue = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 7));
      Builder.createConditionalBranch(Entry.Block, Entry.Arguments[0], Merge.Block, {TrueValue}, Merge.Block, {FalseValue});
      Builder.createReturn(Merge.Block, Merge.Arguments[0]);
      const ir::VerifiedClosedModule Closed = close(Builder);

      BackendResult<std::string> Result = emitLlvmText(Closed);

      ASSERT_TRUE(Result.succeeded()) << Result.error().Message;
      EXPECT_NE(Result.value().find("edge"), std::string::npos);
      EXPECT_NE(Result.value().find("phi i32"), std::string::npos);
      EXPECT_NE(Result.value().find("[ 42, %edge"), std::string::npos);
      EXPECT_NE(Result.value().find("[ 7, %edge"), std::string::npos);
    }

    // Verifies unit remains an empty-aggregate value while void has no value and never lowers to a noreturn void function followed by LLVM unreachable.
    TEST(BackendLlvmTextTest, DistinguishesUnitVoidAndNever)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId Unit = Builder.unitType();
      const ir::IrTypeId Never = Builder.neverType();
      const ir::IrTypeId UnitSignature = Builder.functionType({}, Unit);
      const ir::IrTypeId VoidSignature = Builder.functionType({}, std::nullopt);
      const ir::IrTypeId NeverSignature = Builder.functionType({}, Never);
      const ir::IrFunctionId ExternalUnit = Builder.addFunction("external_unit", UnitSignature, ir::IrFunctionKind::External);
      const ir::IrFunctionId ExternalVoid = Builder.addFunction("external_void", VoidSignature, ir::IrFunctionKind::External);
      const ir::IrFunctionId Abort = Builder.addFunction("abort_now", NeverSignature, ir::IrFunctionKind::External);
      const ir::IrFunctionId UnitWrapper = Builder.addFunction("unit_wrapper", UnitSignature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock UnitEntry = Builder.addBlock(UnitWrapper);
      const ir::IrBuiltOperation UnitCall = Builder.createDirectCall(UnitEntry.Block, ExternalUnit, {});
      Builder.createReturn(UnitEntry.Block, UnitCall.Results[0]);
      const ir::IrFunctionId VoidWrapper = Builder.addFunction("void_wrapper", VoidSignature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock VoidEntry = Builder.addBlock(VoidWrapper);
      Builder.createDirectCall(VoidEntry.Block, ExternalVoid, {});
      Builder.createReturn(VoidEntry.Block);
      const ir::IrFunctionId NeverWrapper = Builder.addFunction("never_wrapper", NeverSignature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock NeverEntry = Builder.addBlock(NeverWrapper);
      Builder.createDirectCall(NeverEntry.Block, Abort, {});
      Builder.createUnreachable(NeverEntry.Block);
      const ir::VerifiedClosedModule Closed = close(Builder);

      BackendResult<std::string> Result = emitLlvmText(Closed);

      ASSERT_TRUE(Result.succeeded()) << Result.error().Message;
      const std::string &Text = Result.value();
      EXPECT_NE(Text.find("declare {} @external_unit()"), std::string::npos);
      EXPECT_NE(Text.find("declare void @external_void()"), std::string::npos);
      EXPECT_NE(Text.find("declare void @abort_now()"), std::string::npos);
      EXPECT_NE(Text.find("call {} @external_unit()"), std::string::npos);
      EXPECT_NE(Text.find("call void @external_void()"), std::string::npos);
      EXPECT_NE(Text.find("call void @abort_now()"), std::string::npos);
      EXPECT_NE(Text.find("unreachable"), std::string::npos);
      EXPECT_NE(Text.find("noreturn"), std::string::npos);
    }

    // Verifies rt.trap becomes the LLVM trap intrinsic and a terminating unreachable instruction before module verification.
    TEST(BackendLlvmTextTest, LowersRuntimeTrapIntrinsic)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId Signature = Builder.functionType({}, std::nullopt);
      const ir::IrFunctionId Function = Builder.addFunction("trap_wrapper", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function);
      Builder.createTrap(Entry.Block, ir::IrTrapKind::User);
      const ir::VerifiedClosedModule Closed = close(Builder);

      BackendResult<std::string> Result = emitLlvmText(Closed);

      ASSERT_TRUE(Result.succeeded()) << Result.error().Message;
      EXPECT_NE(Result.value().find("call void @llvm.trap()"), std::string::npos);
      EXPECT_NE(Result.value().find("unreachable"), std::string::npos);
    }

    // Verifies AOT runtime support defines a static stdout external with embedded bytes and the host platform write primitive.
    TEST(BackendLlvmTextTest, DefinesStaticOutputRuntimeFunction)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
      const ir::IrTypeId OutputSignature = Builder.functionType({}, I32);
      const ir::IrFunctionId Output = Builder.addFunction("ink_test_stdout", OutputSignature, ir::IrFunctionKind::External);
      const ir::IrFunctionId Main = Builder.addFunction("main", OutputSignature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
      Builder.createDirectCall(Entry.Block, Output, {});
      const ir::IrValueId Zero = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 0));
      Builder.createReturn(Entry.Block, Zero);
      const ir::VerifiedClosedModule Closed = close(Builder);
      AotRuntimeSupport Support;
      Support.StaticOutputFunctions.push_back({"ink_test_stdout", "Hello\n"});

      BackendResult<std::string> Result = emitLlvmText(Closed, Support);

      ASSERT_TRUE(Result.succeeded()) << Result.error().Message;
      EXPECT_NE(Result.value().find("define i32 @ink_test_stdout()"), std::string::npos);
      EXPECT_NE(Result.value().find("private constant [6 x i8] c\"Hello\\0A\""), std::string::npos);
#ifdef _WIN32
      EXPECT_NE(Result.value().find("@GetStdHandle"), std::string::npos);
      EXPECT_NE(Result.value().find("@WriteFile"), std::string::npos);
#else
      EXPECT_NE(Result.value().find("@write"), std::string::npos);
#endif
    }

    // Verifies native object emission publishes a nonempty x86-64 COFF object only after successful target and LLVM verification.
    TEST(BackendObjectTest, EmitsNativeCoffObject)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
      const ir::IrTypeId Signature = Builder.functionType({}, I32);
      const ir::IrFunctionId Function = Builder.addFunction("ink_object_entry", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function);
      const ir::IrConstantId AnswerConstant = Builder.integerConstant(I32, 42);
      const ir::IrValueId Answer = Builder.createIntegerConstant(Entry.Block, AnswerConstant);
      Builder.createReturn(Entry.Block, Answer);
      const ir::VerifiedClosedModule Closed = close(Builder);
      const std::filesystem::path Output = unusedObjectPath();

      BackendResult<void> Result = emitObject(Closed, Output);

      ASSERT_TRUE(Result.succeeded()) << Result.error().Message;
      ASSERT_TRUE(std::filesystem::is_regular_file(Output));
      EXPECT_GT(std::filesystem::file_size(Output), 0U);
#if defined(_WIN32) && (defined(_M_X64) || defined(__x86_64__))
      {
        std::ifstream Stream(Output, std::ios::binary);
        const std::vector<unsigned char> Bytes((std::istreambuf_iterator<char>(Stream)), std::istreambuf_iterator<char>());
        ASSERT_GE(Bytes.size(), 2U);
        EXPECT_EQ(Bytes[0], 0x64U);
        EXPECT_EQ(Bytes[1], 0x86U);
      }
#endif
      EXPECT_TRUE(std::filesystem::remove(Output));
    }

    // Verifies object publication refuses an existing destination and preserves its bytes instead of replacing or truncating it.
    TEST(BackendObjectTest, PreservesExistingOutput)
    {
      const ir::VerifiedClosedModule Closed = voidModule(hostTargetKey(), "preserve_existing_output");
      const std::filesystem::path Output = unusedObjectPath();
      const std::string Sentinel = "existing-output-must-survive";
      {
        std::ofstream Stream(Output, std::ios::binary);
        ASSERT_TRUE(Stream.is_open());
        Stream.write(Sentinel.data(), static_cast<std::streamsize>(Sentinel.size()));
        ASSERT_TRUE(Stream.good());
      }

      BackendResult<void> Result = emitObject(Closed, Output);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_EQ(Result.error().Code, BackendErrorCode::OutputAlreadyExists);
      std::string Contents;
      {
        std::ifstream Stream(Output, std::ios::binary);
        ASSERT_TRUE(Stream.is_open());
        Contents.assign(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
      }
      EXPECT_EQ(Contents, Sentinel);
      EXPECT_TRUE(std::filesystem::remove(Output));
    }

    // Verifies a minimal source file crosses Compiler parsing, Sema, InkIR verification and closure before both LLVM text and a native object are emitted.
    TEST(BackendPipelineTest, EmitsCompilerProducedClosedModule)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId File = Session.addSource("backend-pipeline.ink", "func main() -> i32 { return 42; }");
      frontend::Compiler Compiler(Session);
      frontend::ClosedCompilationResult Compiled = Compiler.compile(File);

      ASSERT_TRUE(Compiled.succeeded()) << (Compiled.Issues.empty() ? "" : Compiled.Issues.front().Message);
      ASSERT_TRUE(Compiled.Module.has_value());
      BackendResult<std::string> Text = emitLlvmText(*Compiled.Module);
      ASSERT_TRUE(Text.succeeded()) << Text.error().Message;
      EXPECT_NE(Text.value().find("define i32 @main()"), std::string::npos);
      EXPECT_NE(Text.value().find("ret i32 42"), std::string::npos);

      const std::filesystem::path Output = unusedObjectPath();
      BackendResult<void> Object = emitObject(*Compiled.Module, Output);
      ASSERT_TRUE(Object.succeeded()) << Object.error().Message;
      ASSERT_TRUE(std::filesystem::is_regular_file(Output));
      EXPECT_GT(std::filesystem::file_size(Output), 0U);
      EXPECT_TRUE(std::filesystem::remove(Output));
    }

    // Verifies the reserved source-level trap declaration crosses Compiler lowering and becomes llvm.trap without leaving an external trap symbol in LLVM IR.
    TEST(BackendPipelineTest, LowersCompilerTrapIntrinsicToLlvmTrap)
    {
      frontend::CompilationSession Session;
      const core::SourceFileId File = Session.addSource("backend-trap.ink", "func trap() -> never; func main() -> never { trap(); }");
      frontend::Compiler Compiler(Session);
      frontend::ClosedCompilationResult Compiled = Compiler.compile(File);

      ASSERT_TRUE(Compiled.succeeded()) << (Compiled.Issues.empty() ? "" : Compiled.Issues.front().Message);
      ASSERT_TRUE(Compiled.Module.has_value());
      BackendResult<std::string> Text = emitLlvmText(*Compiled.Module);
      ASSERT_TRUE(Text.succeeded()) << Text.error().Message;
      EXPECT_NE(Text.value().find("call void @llvm.trap()"), std::string::npos);
      EXPECT_EQ(Text.value().find("declare void @trap"), std::string::npos);
    }

    // Verifies a non-native target key is rejected during preflight without creating either the requested output or a published partial object.
    TEST(BackendTargetTest, RejectsNonNativeTargetWithoutOutput)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId Signature = Builder.functionType({}, std::nullopt);
      const ir::IrFunctionId Function = Builder.addFunction("foreign", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function);
      Builder.createReturn(Entry.Block);
      target::TargetKey Foreign = hostTargetKey();
      Foreign.Triple = Foreign.Triple.find("aarch64") == std::string::npos ? "aarch64-pc-windows-msvc" : "x86_64-pc-windows-msvc";
      const ir::VerifiedClosedModule Closed = close(Builder, Foreign);
      const std::filesystem::path Output = unusedObjectPath();

      BackendResult<void> Result = emitObject(Closed, Output);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_EQ(Result.error().Code, BackendErrorCode::NonNativeTarget);
      EXPECT_FALSE(std::filesystem::exists(Output));
    }

    // Verifies native preflight rejects an unknown CPU, an unknown signed feature, and a feature without the required sign before creating any object path.
    TEST(BackendTargetTest, RejectsInvalidCpuAndFeatureStringsWithoutOutput)
    {
      target::TargetKey InvalidCpuKey = hostTargetKey();
      InvalidCpuKey.Cpu = "ink-cpu-that-does-not-exist";
      const ir::VerifiedClosedModule InvalidCpuModule = voidModule(InvalidCpuKey, "invalid_cpu");
      const std::filesystem::path InvalidCpuOutput = unusedObjectPath();
      BackendResult<void> InvalidCpuResult = emitObject(InvalidCpuModule, InvalidCpuOutput);
      ASSERT_FALSE(InvalidCpuResult.succeeded());
      EXPECT_EQ(InvalidCpuResult.error().Code, BackendErrorCode::InvalidTarget);
      EXPECT_FALSE(std::filesystem::exists(InvalidCpuOutput));

      target::TargetKey UnknownFeatureKey = hostTargetKey();
      UnknownFeatureKey.Features = "+ink-feature-that-does-not-exist";
      const ir::VerifiedClosedModule UnknownFeatureModule = voidModule(UnknownFeatureKey, "unknown_feature");
      const std::filesystem::path UnknownFeatureOutput = unusedObjectPath();
      BackendResult<void> UnknownFeatureResult = emitObject(UnknownFeatureModule, UnknownFeatureOutput);
      ASSERT_FALSE(UnknownFeatureResult.succeeded());
      EXPECT_EQ(UnknownFeatureResult.error().Code, BackendErrorCode::InvalidTarget);
      EXPECT_FALSE(std::filesystem::exists(UnknownFeatureOutput));

      target::TargetKey MalformedFeatureKey = hostTargetKey();
      MalformedFeatureKey.Features = "sse2";
      const ir::VerifiedClosedModule MalformedFeatureModule = voidModule(MalformedFeatureKey, "malformed_feature");
      const std::filesystem::path MalformedFeatureOutput = unusedObjectPath();
      BackendResult<void> MalformedFeatureResult = emitObject(MalformedFeatureModule, MalformedFeatureOutput);
      ASSERT_FALSE(MalformedFeatureResult.succeeded());
      EXPECT_EQ(MalformedFeatureResult.error().Code, BackendErrorCode::InvalidTarget);
      EXPECT_FALSE(std::filesystem::exists(MalformedFeatureOutput));
    }

    // Verifies target-dependent PDB division is reported as unsupported instead of silently inheriting LLVM division trap and overflow semantics.
    TEST(BackendOpcodeTest, RejectsPdbDivisionExplicitly)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
      const ir::IrTypeId Signature = Builder.functionType({I32}, I32);
      const ir::IrFunctionId Function = Builder.addFunction("divide", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Function, {I32});
      const ir::IrConstantId TwoConstant = Builder.integerConstant(I32, 2);
      const ir::IrValueId Two = Builder.createIntegerConstant(Entry.Block, TwoConstant);
      const ir::IrValueId Quotient = Builder.createIntegerBinary(Entry.Block, ir::IrOpcode::IntSignedDiv, Entry.Arguments[0], Two);
      Builder.createReturn(Entry.Block, Quotient);
      const ir::VerifiedClosedModule Closed = close(Builder);

      BackendResult<std::string> Result = emitLlvmText(Closed);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_EQ(Result.error().Code, BackendErrorCode::UnsupportedOpcode);
      EXPECT_EQ(Result.error().Operation.value(), 1U);
    }
  } // namespace
} // namespace ink::backend
