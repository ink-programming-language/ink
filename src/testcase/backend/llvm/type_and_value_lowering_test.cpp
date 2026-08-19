#include "ink/backend/llvm/llvm_backend.h"
#include "ink/core/context.h"
#include "ink/core/target_context.h"
#include "ink/ir/model/context.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ink::backend::llvm
{
  namespace
  {
    struct LLVMBackendTypeAndValueTestContext
    {
        LLVMBackendTypeAndValueTestContext()
            : Compilation(),
              IR(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        explicit LLVMBackendTypeAndValueTestContext(core::TargetContext Target)
            : Compilation(Target),
              IR(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ~LLVMBackendTypeAndValueTestContext()
        {
          Compilation.diagnosticEngine().removeConsumer(Diagnostics);
        }

        core::CompilationContext Compilation;
        ir::IRContext IR;
        ::llvm::LLVMContext LLVM;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    std::string firstDiagnosticMessage(const std::vector<core::Diagnostic> &Diagnostics)
    {
      return Diagnostics.empty() ? std::string{} : core::DiagnosticFormatter().format(Diagnostics.front()).Message;
    }

    std::unique_ptr<::llvm::Module> lowerSuccessfully(LLVMBackendTypeAndValueTestContext &Context, std::string_view Text)
    {
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      if (!Parsed.succeeded() || !Parsed.module().has_value())
      {
        ADD_FAILURE() << "InkIR deserialization failed with " << Context.Diagnostics.diagnostics().size() << " diagnostic(s)";
        return nullptr;
      }

      LoweringResult Lowered = lowerToLLVMIR(Context.LLVM, *Parsed.module());
      if (!Lowered.succeeded() || Lowered.module() == nullptr)
      {
        ADD_FAILURE() << "LLVM lowering failed: " << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
        return nullptr;
      }

      std::unique_ptr<::llvm::Module> Target = Lowered.takeModule();
      if (Target == nullptr)
      {
        ADD_FAILURE() << "successful LLVM lowering returned no module";
        return nullptr;
      }

      std::string VerificationMessage;
      ::llvm::raw_string_ostream VerificationStream(VerificationMessage);
      const bool Broken = ::llvm::verifyModule(*Target, &VerificationStream);
      VerificationStream.flush();
      EXPECT_FALSE(Broken) << VerificationMessage;
      return Target;
    }

    const ::llvm::Value *returnedValue(const ::llvm::Module &ModuleValue, const char *FunctionName)
    {
      const ::llvm::Function *FunctionValue = ModuleValue.getFunction(FunctionName);
      if (FunctionValue == nullptr)
      {
        ADD_FAILURE() << "missing lowered function @" << FunctionName;
        return nullptr;
      }
      if (FunctionValue->empty())
      {
        ADD_FAILURE() << "lowered function @" << FunctionName << " has no body";
        return nullptr;
      }
      const auto *Return = ::llvm::dyn_cast_or_null<::llvm::ReturnInst>(FunctionValue->getEntryBlock().getTerminator());
      if (Return == nullptr)
      {
        ADD_FAILURE() << "lowered function @" << FunctionName << " has no return terminator";
        return nullptr;
      }
      return Return->getReturnValue();
    }

    void expectFloatingReturnBits(const ::llvm::Module &ModuleValue, const char *FunctionName, std::uint64_t ExpectedBits)
    {
      const auto *Constant = ::llvm::dyn_cast_or_null<::llvm::ConstantFP>(returnedValue(ModuleValue, FunctionName));
      ASSERT_NE(Constant, nullptr);
      EXPECT_EQ(Constant->getValueAPF().bitcastToAPInt().getZExtValue(), ExpectedBits);
    }

    // Verifies that every built-in InkIR type and a named struct become the corresponding LLVM scalar, pointer, slice, or aggregate type.
    TEST(LLVMBackendTypeLoweringTest, LowersBuiltInAndNamedTypes)
    {
      LLVMBackendTypeAndValueTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "%Record = type {Flag: bool, Octet: byte, Number: i32, Half: f16, Single: f32, Double: f64, MutablePointer: byte*, ConstantPointer: const byte*}\n"
          "define void @void_type() {\n"
          "entry:\n"
          "  ret void\n"
          "}\n"
          "define bool @bool_type() {\n"
          "entry:\n"
          "  ret bool 1\n"
          "}\n"
          "define byte @byte_type() {\n"
          "entry:\n"
          "  ret byte 255\n"
          "}\n"
          "define i32 @i32_type() {\n"
          "entry:\n"
          "  ret i32 -7\n"
          "}\n"
          "define ptrsize @ptrsize_type() {\n"
          "entry:\n"
          "  ret ptrsize 42\n"
          "}\n"
          "define byte* @byte_pointer_type() {\n"
          "entry:\n"
          "  ret byte* null\n"
          "}\n"
          "define const byte* @const_byte_pointer_type() {\n"
          "entry:\n"
          "  ret const byte* null\n"
          "}\n"
          "define f16 @f16_type() {\n"
          "entry:\n"
          "  ret f16 floatbits(f16,0x3C00)\n"
          "}\n"
          "define f32 @f32_type() {\n"
          "entry:\n"
          "  ret f32 floatbits(f32,0x3F800000)\n"
          "}\n"
          "define f64 @f64_type() {\n"
          "entry:\n"
          "  ret f64 floatbits(f64,0x3FF0000000000000)\n"
          "}\n"
          "define ptrsize @slice_types(byte[] %0, const byte[] %1) {\n"
          "entry:\n"
          "  %2 = slice.length byte[] %0\n"
          "  %3 = slice.length const byte[] %1\n"
          "  %4 = add ptrsize %2, ptrsize %3\n"
          "  ret ptrsize %4\n"
          "}\n"
          "define %Record @record_type() {\n"
          "entry:\n"
          "  ret %Record zeroinitializer\n"
          "}\n";

      std::unique_ptr<::llvm::Module> Target = lowerSuccessfully(Context, Text);

      ASSERT_NE(Target, nullptr);
      const ::llvm::Function *VoidFunction = Target->getFunction("void_type");
      const ::llvm::Function *BoolFunction = Target->getFunction("bool_type");
      const ::llvm::Function *ByteFunction = Target->getFunction("byte_type");
      const ::llvm::Function *I32Function = Target->getFunction("i32_type");
      const ::llvm::Function *PointerSizeFunction = Target->getFunction("ptrsize_type");
      const ::llvm::Function *MutablePointerFunction = Target->getFunction("byte_pointer_type");
      const ::llvm::Function *ConstPointerFunction = Target->getFunction("const_byte_pointer_type");
      const ::llvm::Function *HalfFunction = Target->getFunction("f16_type");
      const ::llvm::Function *FloatFunction = Target->getFunction("f32_type");
      const ::llvm::Function *DoubleFunction = Target->getFunction("f64_type");
      const ::llvm::Function *SliceFunction = Target->getFunction("slice_types");
      const ::llvm::Function *RecordFunction = Target->getFunction("record_type");
      ASSERT_NE(VoidFunction, nullptr);
      ASSERT_NE(BoolFunction, nullptr);
      ASSERT_NE(ByteFunction, nullptr);
      ASSERT_NE(I32Function, nullptr);
      ASSERT_NE(PointerSizeFunction, nullptr);
      ASSERT_NE(MutablePointerFunction, nullptr);
      ASSERT_NE(ConstPointerFunction, nullptr);
      ASSERT_NE(HalfFunction, nullptr);
      ASSERT_NE(FloatFunction, nullptr);
      ASSERT_NE(DoubleFunction, nullptr);
      ASSERT_NE(SliceFunction, nullptr);
      ASSERT_NE(RecordFunction, nullptr);
      EXPECT_TRUE(VoidFunction->getReturnType()->isVoidTy());
      EXPECT_TRUE(BoolFunction->getReturnType()->isIntegerTy(1));
      EXPECT_TRUE(ByteFunction->getReturnType()->isIntegerTy(8));
      EXPECT_TRUE(I32Function->getReturnType()->isIntegerTy(32));
      EXPECT_TRUE(PointerSizeFunction->getReturnType()->isIntegerTy(64));
      EXPECT_TRUE(MutablePointerFunction->getReturnType()->isPointerTy());
      EXPECT_TRUE(ConstPointerFunction->getReturnType()->isPointerTy());
      EXPECT_EQ(MutablePointerFunction->getReturnType(), ConstPointerFunction->getReturnType());
      EXPECT_TRUE(HalfFunction->getReturnType()->isHalfTy());
      EXPECT_TRUE(FloatFunction->getReturnType()->isFloatTy());
      EXPECT_TRUE(DoubleFunction->getReturnType()->isDoubleTy());
      ASSERT_EQ(SliceFunction->arg_size(), 2U);
      const auto *MutableSliceType = ::llvm::dyn_cast<::llvm::StructType>(SliceFunction->getFunctionType()->getParamType(0));
      const auto *ConstSliceType = ::llvm::dyn_cast<::llvm::StructType>(SliceFunction->getFunctionType()->getParamType(1));
      ASSERT_NE(MutableSliceType, nullptr);
      ASSERT_NE(ConstSliceType, nullptr);
      EXPECT_EQ(MutableSliceType, ConstSliceType);
      ASSERT_EQ(MutableSliceType->getNumElements(), 2U);
      EXPECT_TRUE(MutableSliceType->getElementType(0)->isPointerTy());
      EXPECT_TRUE(MutableSliceType->getElementType(1)->isIntegerTy(64));
      const auto *RecordType = ::llvm::dyn_cast<::llvm::StructType>(RecordFunction->getReturnType());
      ASSERT_NE(RecordType, nullptr);
      EXPECT_TRUE(RecordType->hasName());
      EXPECT_EQ(RecordType->getName(), "Record");
      ASSERT_EQ(RecordType->getNumElements(), 8U);
      EXPECT_TRUE(RecordType->getElementType(0)->isIntegerTy(1));
      EXPECT_TRUE(RecordType->getElementType(1)->isIntegerTy(8));
      EXPECT_TRUE(RecordType->getElementType(2)->isIntegerTy(32));
      EXPECT_TRUE(RecordType->getElementType(3)->isHalfTy());
      EXPECT_TRUE(RecordType->getElementType(4)->isFloatTy());
      EXPECT_TRUE(RecordType->getElementType(5)->isDoubleTy());
      EXPECT_TRUE(RecordType->getElementType(6)->isPointerTy());
      EXPECT_TRUE(RecordType->getElementType(7)->isPointerTy());
    }

    // Verifies that the configured target byte order and pointer width drive both the LLVM DataLayout and ptrsize lowering independently of the host.
    TEST(LLVMBackendTargetLoweringTest, PropagatesTargetDataLayout)
    {
      LLVMBackendTypeAndValueTestContext Little32(core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian));
      LLVMBackendTypeAndValueTestContext Big64(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::BigEndian));
      const std::string Text =
          "inkir 1\n"
          "%PointerRecord = type {Tag: byte, Value: ptrsize}\n"
          "define ptrsize @pointer_size() {\n"
          "entry:\n"
          "  ret ptrsize 1\n"
          "}\n"
          "define %PointerRecord @pointer_record() {\n"
          "entry:\n"
          "  ret %PointerRecord zeroinitializer\n"
          "}\n";

      std::unique_ptr<::llvm::Module> LittleTarget = lowerSuccessfully(Little32, Text);
      std::unique_ptr<::llvm::Module> BigTarget = lowerSuccessfully(Big64, Text);

      ASSERT_NE(LittleTarget, nullptr);
      ASSERT_NE(BigTarget, nullptr);
      EXPECT_TRUE(LittleTarget->getDataLayout().isLittleEndian());
      EXPECT_TRUE(BigTarget->getDataLayout().isBigEndian());
      EXPECT_EQ(LittleTarget->getDataLayout().getPointerSizeInBits(), 32U);
      EXPECT_EQ(BigTarget->getDataLayout().getPointerSizeInBits(), 64U);
      EXPECT_EQ(LittleTarget->getDataLayoutStr(), "e-p:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f16:16:16-f32:32:32-f64:64:64");
      EXPECT_EQ(BigTarget->getDataLayoutStr(), "E-p:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f16:16:16-f32:32:32-f64:64:64");
      ASSERT_NE(LittleTarget->getFunction("pointer_size"), nullptr);
      ASSERT_NE(BigTarget->getFunction("pointer_size"), nullptr);
      ASSERT_NE(LittleTarget->getFunction("pointer_record"), nullptr);
      ASSERT_NE(BigTarget->getFunction("pointer_record"), nullptr);
      EXPECT_TRUE(LittleTarget->getFunction("pointer_size")->getReturnType()->isIntegerTy(32));
      EXPECT_TRUE(BigTarget->getFunction("pointer_size")->getReturnType()->isIntegerTy(64));
      auto *LittleRecord = ::llvm::dyn_cast<::llvm::StructType>(LittleTarget->getFunction("pointer_record")->getReturnType());
      auto *BigRecord = ::llvm::dyn_cast<::llvm::StructType>(BigTarget->getFunction("pointer_record")->getReturnType());
      ASSERT_NE(LittleRecord, nullptr);
      ASSERT_NE(BigRecord, nullptr);
      EXPECT_FALSE(LittleRecord->isPacked());
      EXPECT_FALSE(BigRecord->isPacked());
      EXPECT_EQ(LittleTarget->getDataLayout().getABITypeAlign(LittleRecord).value(), 4U);
      EXPECT_EQ(BigTarget->getDataLayout().getABITypeAlign(BigRecord).value(), 8U);
    }

    // Verifies that byte constants preserve every byte and storage property while mutable and immutable InkIR globals receive zero initializers, linkage, writable initialization storage, and ABI alignment.
    TEST(LLVMBackendValueLoweringTest, LowersByteConstantsAndGlobals)
    {
      LLVMBackendTypeAndValueTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "@blob = private constant [5 x byte] c\"\\00A\\22\\5C\\FF\"\n"
          "@counter = global mutable i32\n"
          "@readonly = global constant byte\n";

      std::unique_ptr<::llvm::Module> Target = lowerSuccessfully(Context, Text);

      ASSERT_NE(Target, nullptr);
      const ::llvm::GlobalVariable *Blob = Target->getNamedGlobal("blob");
      const ::llvm::GlobalVariable *Counter = Target->getNamedGlobal("counter");
      const ::llvm::GlobalVariable *Readonly = Target->getNamedGlobal("readonly");
      ASSERT_NE(Blob, nullptr);
      ASSERT_NE(Counter, nullptr);
      ASSERT_NE(Readonly, nullptr);
      EXPECT_TRUE(Blob->isConstant());
      EXPECT_TRUE(Blob->hasPrivateLinkage());
      EXPECT_EQ(Blob->getUnnamedAddr(), ::llvm::GlobalValue::UnnamedAddr::None);
      ASSERT_TRUE(Blob->getAlign().has_value());
      EXPECT_EQ(Blob->getAlign()->value(), 1U);
      const auto *BlobType = ::llvm::dyn_cast<::llvm::ArrayType>(Blob->getValueType());
      const auto *BlobData = ::llvm::dyn_cast<::llvm::ConstantDataArray>(Blob->getInitializer());
      ASSERT_NE(BlobType, nullptr);
      ASSERT_NE(BlobData, nullptr);
      EXPECT_TRUE(BlobType->getElementType()->isIntegerTy(8));
      EXPECT_EQ(BlobType->getNumElements(), 5U);
      const std::string ExpectedBytes("\0A\"\\\xFF", 5);
      EXPECT_TRUE(BlobData->getAsString() == ::llvm::StringRef(ExpectedBytes.data(), ExpectedBytes.size()));
      EXPECT_FALSE(Counter->isConstant());
      EXPECT_TRUE(Counter->hasExternalLinkage());
      ASSERT_NE(Counter->getInitializer(), nullptr);
      EXPECT_TRUE(Counter->getInitializer()->isNullValue());
      ASSERT_TRUE(Counter->getAlign().has_value());
      EXPECT_EQ(Counter->getAlign()->value(), 4U);
      // Ink immutable globals can be assigned by the module initializer, so they remain writable LLVM storage.
      EXPECT_FALSE(Readonly->isConstant());
      EXPECT_TRUE(Readonly->hasExternalLinkage());
      ASSERT_NE(Readonly->getInitializer(), nullptr);
      EXPECT_TRUE(Readonly->getInitializer()->isNullValue());
      ASSERT_TRUE(Readonly->getAlign().has_value());
      EXPECT_EQ(Readonly->getAlign()->value(), 1U);
    }

    // Verifies that byte-constant offsets become constant LLVM GEPs while global-variable addresses directly reference their lowered LLVM globals.
    TEST(LLVMBackendValueLoweringTest, LowersByteConstantAndGlobalVariableAddresses)
    {
      LLVMBackendTypeAndValueTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "@blob = private constant [4 x byte] c\"\\10\\20\\30\\40\"\n"
          "@counter = global mutable i32\n"
          "define const byte* @byte_address() {\n"
          "entry:\n"
          "  ret const byte* @blob[2]\n"
          "}\n"
          "define byte* @global_address() {\n"
          "entry:\n"
          "  ret byte* @counter\n"
          "}\n";

      std::unique_ptr<::llvm::Module> Target = lowerSuccessfully(Context, Text);

      ASSERT_NE(Target, nullptr);
      const ::llvm::GlobalVariable *Blob = Target->getNamedGlobal("blob");
      const ::llvm::GlobalVariable *Counter = Target->getNamedGlobal("counter");
      ASSERT_NE(Blob, nullptr);
      ASSERT_NE(Counter, nullptr);
      const auto *ByteAddress = ::llvm::dyn_cast_or_null<::llvm::ConstantExpr>(returnedValue(*Target, "byte_address"));
      ASSERT_NE(ByteAddress, nullptr);
      EXPECT_EQ(ByteAddress->getOpcode(), ::llvm::Instruction::GetElementPtr);
      ASSERT_EQ(ByteAddress->getNumOperands(), 3U);
      EXPECT_EQ(ByteAddress->getOperand(0), Blob);
      const auto *ArrayIndex = ::llvm::dyn_cast<::llvm::ConstantInt>(ByteAddress->getOperand(1));
      const auto *ByteOffset = ::llvm::dyn_cast<::llvm::ConstantInt>(ByteAddress->getOperand(2));
      ASSERT_NE(ArrayIndex, nullptr);
      ASSERT_NE(ByteOffset, nullptr);
      EXPECT_TRUE(ArrayIndex->isZero());
      EXPECT_TRUE(ArrayIndex->getType()->isIntegerTy(32));
      EXPECT_EQ(ByteOffset->getZExtValue(), 2U);
      EXPECT_TRUE(ByteOffset->getType()->isIntegerTy(64));
      EXPECT_EQ(returnedValue(*Target, "global_address"), Counter);
    }

    // Verifies exact lowering of integer, floating-point, null, zero, string, and nested aggregate constants, including preserved floating payload bits.
    TEST(LLVMBackendValueLoweringTest, LowersEveryConstantKind)
    {
      LLVMBackendTypeAndValueTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {Bits: f32, Pointer: const byte*}\n"
          "%Outer = type {Count: i32, Payload: %Inner}\n"
          "define bool @bool_constant() {\n"
          "entry:\n"
          "  ret bool 1\n"
          "}\n"
          "define byte @byte_constant() {\n"
          "entry:\n"
          "  ret byte 255\n"
          "}\n"
          "define i32 @integer_constant() {\n"
          "entry:\n"
          "  ret i32 -7\n"
          "}\n"
          "define ptrsize @pointer_size_constant() {\n"
          "entry:\n"
          "  ret ptrsize 4294967297\n"
          "}\n"
          "define f16 @half_constant() {\n"
          "entry:\n"
          "  ret f16 floatbits(f16,0xBC00)\n"
          "}\n"
          "define f32 @float_constant() {\n"
          "entry:\n"
          "  ret f32 floatbits(f32,0x7FC00042)\n"
          "}\n"
          "define f64 @double_constant() {\n"
          "entry:\n"
          "  ret f64 floatbits(f64,0xFFF0000000000000)\n"
          "}\n"
          "define byte* @mutable_null() {\n"
          "entry:\n"
          "  ret byte* null\n"
          "}\n"
          "define const byte* @constant_null() {\n"
          "entry:\n"
          "  ret const byte* null\n"
          "}\n"
          "define i32 @integer_zero() {\n"
          "entry:\n"
          "  ret i32 zeroinitializer\n"
          "}\n"
          "define %Outer @aggregate_zero() {\n"
          "entry:\n"
          "  ret %Outer zeroinitializer\n"
          "}\n"
          "define ptrsize @string_length() {\n"
          "entry:\n"
          "  %0 = slice.length const byte[] c\"\\00ink\\FF\"\n"
          "  ret ptrsize %0\n"
          "}\n"
          "define %Outer @aggregate_constant() {\n"
          "entry:\n"
          "  ret %Outer {i32 -9, %Inner {f32 floatbits(f32,0x80000000), const byte* null}}\n"
          "}\n";

      std::unique_ptr<::llvm::Module> Target = lowerSuccessfully(Context, Text);

      ASSERT_NE(Target, nullptr);
      const auto *BoolConstant = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(returnedValue(*Target, "bool_constant"));
      const auto *ByteConstant = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(returnedValue(*Target, "byte_constant"));
      const auto *IntegerConstant = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(returnedValue(*Target, "integer_constant"));
      const auto *PointerSizeConstant = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(returnedValue(*Target, "pointer_size_constant"));
      ASSERT_NE(BoolConstant, nullptr);
      ASSERT_NE(ByteConstant, nullptr);
      ASSERT_NE(IntegerConstant, nullptr);
      ASSERT_NE(PointerSizeConstant, nullptr);
      EXPECT_TRUE(BoolConstant->isOne());
      EXPECT_EQ(ByteConstant->getZExtValue(), 255U);
      EXPECT_EQ(IntegerConstant->getSExtValue(), -7);
      EXPECT_EQ(PointerSizeConstant->getZExtValue(), 4294967297ULL);
      expectFloatingReturnBits(*Target, "half_constant", 0xBC00U);
      expectFloatingReturnBits(*Target, "float_constant", 0x7FC00042U);
      expectFloatingReturnBits(*Target, "double_constant", 0xFFF0000000000000ULL);
      EXPECT_NE(::llvm::dyn_cast_or_null<::llvm::ConstantPointerNull>(returnedValue(*Target, "mutable_null")), nullptr);
      EXPECT_NE(::llvm::dyn_cast_or_null<::llvm::ConstantPointerNull>(returnedValue(*Target, "constant_null")), nullptr);
      const auto *IntegerZero = ::llvm::dyn_cast_or_null<::llvm::Constant>(returnedValue(*Target, "integer_zero"));
      const auto *AggregateZero = ::llvm::dyn_cast_or_null<::llvm::Constant>(returnedValue(*Target, "aggregate_zero"));
      ASSERT_NE(IntegerZero, nullptr);
      ASSERT_NE(AggregateZero, nullptr);
      EXPECT_TRUE(IntegerZero->isNullValue());
      EXPECT_TRUE(AggregateZero->isNullValue());
      const ::llvm::GlobalVariable *StringStorage = Target->getNamedGlobal(".ink.string.0");
      ASSERT_NE(StringStorage, nullptr);
      EXPECT_TRUE(StringStorage->isConstant());
      EXPECT_TRUE(StringStorage->hasPrivateLinkage());
      EXPECT_EQ(StringStorage->getUnnamedAddr(), ::llvm::GlobalValue::UnnamedAddr::None);
      const auto *StringData = ::llvm::dyn_cast<::llvm::ConstantDataArray>(StringStorage->getInitializer());
      ASSERT_NE(StringData, nullptr);
      const std::string ExpectedString("\0ink\xFF", 5);
      EXPECT_TRUE(StringData->getAsString() == ::llvm::StringRef(ExpectedString.data(), ExpectedString.size()));
      const auto *StringLength = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(returnedValue(*Target, "string_length"));
      ASSERT_NE(StringLength, nullptr);
      EXPECT_EQ(StringLength->getZExtValue(), 5U);
      const auto *OuterConstant = ::llvm::dyn_cast_or_null<::llvm::Constant>(returnedValue(*Target, "aggregate_constant"));
      ASSERT_NE(OuterConstant, nullptr);
      const auto *Count = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(OuterConstant->getAggregateElement(0U));
      const ::llvm::Constant *InnerConstant = OuterConstant->getAggregateElement(1U);
      ASSERT_NE(Count, nullptr);
      ASSERT_NE(InnerConstant, nullptr);
      EXPECT_EQ(Count->getSExtValue(), -9);
      const auto *InnerFloat = ::llvm::dyn_cast_or_null<::llvm::ConstantFP>(InnerConstant->getAggregateElement(0U));
      const auto *InnerPointer = ::llvm::dyn_cast_or_null<::llvm::ConstantPointerNull>(InnerConstant->getAggregateElement(1U));
      ASSERT_NE(InnerFloat, nullptr);
      ASSERT_NE(InnerPointer, nullptr);
      EXPECT_EQ(InnerFloat->getValueAPF().bitcastToAPInt().getZExtValue(), 0x80000000U);
    }

    // Verifies that packed and explicitly offset InkIR structs acquire exact physical padding, field indices, stride, global alignment, and aggregate constant placement in LLVM IR.
    TEST(LLVMBackendTypeLoweringTest, PreservesCustomStructLayoutsAndAggregatePlacement)
    {
      LLVMBackendTypeAndValueTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "%Packed = type pack(1) {Tag: byte, Value: i32}\n"
          "%Explicit = type align(16) {Tag: byte, Value: i32 align(8) offset(8)}\n"
          "@layout_storage = global constant %Explicit\n"
          "define %Packed @packed_constant() {\n"
          "entry:\n"
          "  ret %Packed {byte 7, i32 42}\n"
          "}\n"
          "define %Explicit @explicit_constant() {\n"
          "entry:\n"
          "  ret %Explicit {byte 9, i32 100}\n"
          "}\n";

      std::unique_ptr<::llvm::Module> Target = lowerSuccessfully(Context, Text);

      ASSERT_NE(Target, nullptr);
      ::llvm::Function *PackedFunction = Target->getFunction("packed_constant");
      ::llvm::Function *ExplicitFunction = Target->getFunction("explicit_constant");
      ASSERT_NE(PackedFunction, nullptr);
      ASSERT_NE(ExplicitFunction, nullptr);
      auto *PackedType = ::llvm::dyn_cast<::llvm::StructType>(PackedFunction->getReturnType());
      auto *ExplicitType = ::llvm::dyn_cast<::llvm::StructType>(ExplicitFunction->getReturnType());
      ASSERT_NE(PackedType, nullptr);
      ASSERT_NE(ExplicitType, nullptr);
      EXPECT_TRUE(PackedType->isPacked());
      ASSERT_EQ(PackedType->getNumElements(), 2U);
      const ::llvm::StructLayout *PackedLayout = Target->getDataLayout().getStructLayout(PackedType);
      EXPECT_EQ(PackedLayout->getElementOffset(0).getFixedValue(), 0U);
      EXPECT_EQ(PackedLayout->getElementOffset(1).getFixedValue(), 1U);
      EXPECT_EQ(PackedLayout->getSizeInBytes().getFixedValue(), 5U);
      EXPECT_EQ(PackedLayout->getAlignment().value(), 1U);
      EXPECT_FALSE(ExplicitType->isPacked());
      ASSERT_EQ(ExplicitType->getNumElements(), 2U);
      auto *ExplicitPayload = ::llvm::dyn_cast<::llvm::StructType>(ExplicitType->getElementType(0));
      const auto *AlignmentMarker = ::llvm::dyn_cast<::llvm::ArrayType>(ExplicitType->getElementType(1));
      ASSERT_NE(ExplicitPayload, nullptr);
      ASSERT_NE(AlignmentMarker, nullptr);
      EXPECT_TRUE(ExplicitPayload->isPacked());
      ASSERT_EQ(ExplicitPayload->getNumElements(), 4U);
      EXPECT_EQ(AlignmentMarker->getNumElements(), 0U);
      EXPECT_EQ(Target->getDataLayout().getABITypeAlign(AlignmentMarker->getElementType()).value(), 16U);
      const auto *InternalPadding = ::llvm::dyn_cast<::llvm::ArrayType>(ExplicitPayload->getElementType(1));
      const auto *TailPadding = ::llvm::dyn_cast<::llvm::ArrayType>(ExplicitPayload->getElementType(3));
      ASSERT_NE(InternalPadding, nullptr);
      ASSERT_NE(TailPadding, nullptr);
      EXPECT_TRUE(InternalPadding->getElementType()->isIntegerTy(8));
      EXPECT_EQ(InternalPadding->getNumElements(), 7U);
      EXPECT_TRUE(ExplicitPayload->getElementType(2)->isIntegerTy(32));
      EXPECT_TRUE(TailPadding->getElementType()->isIntegerTy(8));
      EXPECT_EQ(TailPadding->getNumElements(), 4U);
      const ::llvm::StructLayout *ExplicitPayloadLayout = Target->getDataLayout().getStructLayout(ExplicitPayload);
      EXPECT_EQ(ExplicitPayloadLayout->getElementOffset(0).getFixedValue(), 0U);
      EXPECT_EQ(ExplicitPayloadLayout->getElementOffset(1).getFixedValue(), 1U);
      EXPECT_EQ(ExplicitPayloadLayout->getElementOffset(2).getFixedValue(), 8U);
      EXPECT_EQ(ExplicitPayloadLayout->getElementOffset(3).getFixedValue(), 12U);
      const ::llvm::StructLayout *ExplicitLayout = Target->getDataLayout().getStructLayout(ExplicitType);
      EXPECT_EQ(ExplicitLayout->getSizeInBytes().getFixedValue(), 16U);
      EXPECT_EQ(ExplicitLayout->getAlignment().value(), 16U);
      const ::llvm::GlobalVariable *Storage = Target->getNamedGlobal("layout_storage");
      ASSERT_NE(Storage, nullptr);
      EXPECT_EQ(Storage->getValueType(), ExplicitType);
      EXPECT_FALSE(Storage->isConstant());
      ASSERT_TRUE(Storage->getAlign().has_value());
      EXPECT_EQ(Storage->getAlign()->value(), 16U);
      const auto *PackedConstant = ::llvm::dyn_cast_or_null<::llvm::Constant>(returnedValue(*Target, "packed_constant"));
      const auto *ExplicitConstant = ::llvm::dyn_cast_or_null<::llvm::Constant>(returnedValue(*Target, "explicit_constant"));
      ASSERT_NE(PackedConstant, nullptr);
      ASSERT_NE(ExplicitConstant, nullptr);
      const auto *PackedTag = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(PackedConstant->getAggregateElement(0U));
      const auto *PackedValue = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(PackedConstant->getAggregateElement(1U));
      ASSERT_NE(PackedTag, nullptr);
      ASSERT_NE(PackedValue, nullptr);
      EXPECT_EQ(PackedTag->getZExtValue(), 7U);
      EXPECT_EQ(PackedValue->getSExtValue(), 42);
      const ::llvm::Constant *ExplicitPayloadConstant = ExplicitConstant->getAggregateElement(0U);
      const ::llvm::Constant *ExplicitAlignmentMarker = ExplicitConstant->getAggregateElement(1U);
      ASSERT_NE(ExplicitPayloadConstant, nullptr);
      ASSERT_NE(ExplicitAlignmentMarker, nullptr);
      const auto *ExplicitTag = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(ExplicitPayloadConstant->getAggregateElement(0U));
      const ::llvm::Constant *ExplicitInternalPadding = ExplicitPayloadConstant->getAggregateElement(1U);
      const auto *ExplicitValue = ::llvm::dyn_cast_or_null<::llvm::ConstantInt>(ExplicitPayloadConstant->getAggregateElement(2U));
      const ::llvm::Constant *ExplicitTailPadding = ExplicitPayloadConstant->getAggregateElement(3U);
      ASSERT_NE(ExplicitTag, nullptr);
      ASSERT_NE(ExplicitInternalPadding, nullptr);
      ASSERT_NE(ExplicitValue, nullptr);
      ASSERT_NE(ExplicitTailPadding, nullptr);
      EXPECT_EQ(ExplicitTag->getZExtValue(), 9U);
      EXPECT_TRUE(ExplicitInternalPadding->isNullValue());
      EXPECT_EQ(ExplicitValue->getSExtValue(), 100);
      EXPECT_TRUE(ExplicitTailPadding->isNullValue());
      EXPECT_TRUE(ExplicitAlignmentMarker->isNullValue());
    }
  } // namespace
} // namespace ink::backend::llvm
