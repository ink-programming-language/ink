#include "lowering_context.h"
#include "ink/ir/linkage.h"

#include "ink/execution/support/execution_result.h"
#include "ink/core/diagnostic.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/array_element_pointer_instruction.h"
#include "ink/ir/instruction/array_extract_instruction.h"
#include "ink/ir/instruction/array_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/c_string_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/class_instruction.h"
#include "ink/ir/instruction/compare_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/field_extract_instruction.h"
#include "ink/ir/instruction/field_pointer_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/logical_and_instruction.h"
#include "ink/ir/instruction/logical_not_instruction.h"
#include "ink/ir/instruction/logical_or_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/type/class_type.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>

#include <algorithm>
#include <limits>

namespace ink::backend::llvm
{
  namespace
  {
    using execution::ExecutionStatus;

    class FunctionLowering final
    {
      public:
        FunctionLowering(LoweringContext &Context, const ir::Function &Source, ::llvm::Function &Target)
            : Context(Context),
              Source(Source),
              Target(Target),
              Builder(Context.Context)
        {
        }

        bool lower();
        bool lowerReflection();

      private:
        bool instruction(const ir::Value &Instruction);
        ::llvm::Value *value(const ir::Value &Value);
        void require(::llvm::Value *Condition, ExecutionStatus Status);
        ::llvm::Value *memoryLoad(const ir::Type &Type, ::llvm::Value *Address);
        bool memoryStore(const ir::Type &Type, ::llvm::Value *Value, ::llvm::Value *Address);
        ::llvm::Value *offset(::llvm::Value *Address, std::uint64_t Bytes);
        ::llvm::Value *checkedIndex(const ir::Value &Index, std::uint64_t Count);
        bool call(const ir::CallInstruction &Call, ::llvm::Value *&Result);
        bool hotEntry();

        LoweringContext &Context;
        const ir::Function &Source;
        ::llvm::Function &Target;
        ::llvm::IRBuilder<> Builder;
        std::unordered_map<const ir::BasicBlock *, ::llvm::BasicBlock *> Blocks;
        std::unordered_map<const ir::Value *, ::llvm::Value *> Values;
        std::unordered_map<const ir::Value *, ::llvm::Value *> Initialized;
    };

    void FunctionLowering::require(::llvm::Value *Condition, ExecutionStatus Status)
    {
      auto *Continue = ::llvm::BasicBlock::Create(Context.Context, "valid", &Target);
      auto *Failed = ::llvm::BasicBlock::Create(Context.Context, "invalid", &Target);
      Builder.CreateCondBr(Condition, Continue, Failed);
      Builder.SetInsertPoint(Failed);
      const core::DiagnosticKind Kind = Status == ExecutionStatus::IndexOutOfBounds ? core::DiagnosticKind::ExecutionIndexOutOfBounds : Status == ExecutionStatus::TypeMismatch ? core::DiagnosticKind::ExecutionTypeMismatch : core::DiagnosticKind::ExecutionRuntimeValue;
      const std::string Message = "inkc: error[" + std::string(core::diagnosticCode(Kind)) + "]: AOT execution failed: " + std::string(core::diagnosticDefaultMessage(Kind)) + "\n";
      auto *Panic = ::llvm::cast<::llvm::Function>(Context.helper("ink_aot_panic", Builder.getVoidTy(), {Context.PointerType, Context.SizeType}).getCallee());
      Panic->addFnAttr(::llvm::Attribute::NoReturn);
      Panic->addFnAttr(::llvm::Attribute::Cold);
      // Preserve completed C I/O before the OS-only panic terminates the process.
      Builder.CreateCall(Context.helper("fflush", Builder.getInt32Ty(), {Context.PointerType}), {::llvm::ConstantPointerNull::get(Context.PointerType)});
      Builder.CreateCall(Panic, {Builder.CreateGlobalString(Message, "diagnostic"), ::llvm::ConstantInt::get(Context.SizeType, Message.size())});
      Builder.CreateUnreachable();
      Builder.SetInsertPoint(Continue);
    }

    bool FunctionLowering::lower()
    {
      auto *Entry = ::llvm::BasicBlock::Create(Context.Context, "prologue", &Target);
      Builder.SetInsertPoint(Entry);
      for (const auto &Block : Source.blocks())
      {
        if (!Block->terminator())
        {
          return Context.fail("AOT function contains an unterminated block");
        }
        Blocks.emplace(Block.get(), ::llvm::BasicBlock::Create(Context.Context, "block", &Target));
        for (const auto &Value : Block->values())
        {
          if (Value->type().typeKind() == ir::TypeKind::Void || ir::Function::classof(Value.get()) || ir::Module::classof(Value.get()))
          {
            continue;
          }
          ::llvm::Type *Type = Context.lowerType(Value->type());
          if (!Type)
          {
            return false;
          }
          Values.emplace(Value.get(), Builder.CreateAlloca(Type, nullptr, "value"));
          auto *Flag = Builder.CreateAlloca(Builder.getInt1Ty(), nullptr, "initialized");
          Builder.CreateStore(Builder.getFalse(), Flag);
          Initialized.emplace(Value.get(), Flag);
        }
      }
      std::size_t Index = 0;
      for (::llvm::Argument &Argument : Target.args())
      {
        if (Source.parameters()[Index]->parameterKind() == ir::ParameterKind::Variadic)
        {
          return Context.fail("AOT does not support variadic Ink functions");
        }
        Values.emplace(Source.parameters()[Index++].get(), &Argument);
      }
      for (const auto &Parameter : Source.parameters())
      {
        if (!Values.at(Parameter.get()))
        {
          return false;
        }
      }
      Builder.CreateBr(Blocks.at(Source.entryBlock()));
      for (const auto &Block : Source.blocks())
      {
        Builder.SetInsertPoint(Blocks.at(Block.get()));
        for (const auto &Value : Block->values())
        {
          if (!instruction(*Value))
          {
            return false;
          }
        }
      }
      return hotEntry();
    }

    bool FunctionLowering::hotEntry()
    {
      const auto Found = Context.HotFunctions.find(&Source);
      if (Found == Context.HotFunctions.end())
      {
        return true;
      }
      Target.addFnAttr(::llvm::Attribute::NoInline);
      auto Leave = Context.helper("ink_hybrid_leave", Builder.getVoidTy(), {});
      for (auto &Block : Target)
      {
        if (::llvm::isa<::llvm::ReturnInst>(Block.getTerminator()))
        {
          Builder.SetInsertPoint(Block.getTerminator());
          Builder.CreateCall(Leave);
        }
      }
      auto *Original = &Target.getEntryBlock();
      auto *Entry = ::llvm::BasicBlock::Create(Context.Context, "hot.entry", &Target, Original);
      auto *Patched = ::llvm::BasicBlock::Create(Context.Context, "hot.patch", &Target);
      Builder.SetInsertPoint(Entry);
      auto *Patch = Builder.CreateCall(Context.helper("ink_hybrid_enter", Context.PointerType, {Context.PointerType, Builder.getInt32Ty()}), {Context.HybridModule, Builder.getInt32(Found->second)});
      Builder.CreateCondBr(Builder.CreateIsNotNull(Patch), Patched, Original);
      Builder.SetInsertPoint(Patched);
      auto *Addresses = Builder.CreateAlloca(Context.PointerType, ::llvm::ConstantInt::get(Context.SizeType, std::max<std::size_t>(Source.parameters().size(), 1)), "hot.arguments");
      std::size_t Index = 0;
      for (auto &Argument : Target.args())
      {
        const auto &Type = Source.parameters()[Index]->type();
        const auto Layout = Context.layout(Type);
        if (!Layout)
        {
          return false;
        }
        auto *Storage = Builder.CreateAlloca(Builder.getInt8Ty(), ::llvm::ConstantInt::get(Context.SizeType, std::max<std::uint64_t>(Layout->Size, 1)), "hot.argument");
        Storage->setAlignment(::llvm::Align(Layout->Alignment));
        if (!memoryStore(Type, &Argument, Storage))
        {
          return false;
        }
        Builder.CreateStore(Storage, Builder.CreateGEP(Context.PointerType, Addresses, ::llvm::ConstantInt::get(Context.SizeType, Index++)));
      }
      const auto &Return = Source.functionType().returnType();
      ::llvm::Value *Storage = ::llvm::ConstantPointerNull::get(Context.PointerType);
      if (Return.typeKind() != ir::TypeKind::Void)
      {
        const auto Layout = Context.layout(Return);
        if (!Layout)
        {
          return false;
        }
        auto *Buffer = Builder.CreateAlloca(Builder.getInt8Ty(), ::llvm::ConstantInt::get(Context.SizeType, std::max<std::uint64_t>(Layout->Size, 1)), "hot.result");
        Buffer->setAlignment(::llvm::Align(Layout->Alignment));
        Storage = Buffer;
      }
      auto *Status = Builder.CreateCall(Context.helper("ink_hybrid_invoke", Builder.getInt32Ty(), {Context.PointerType, Context.PointerType, Context.PointerType}), {Patch, Storage, Addresses});
      auto *Succeeded = ::llvm::BasicBlock::Create(Context.Context, "hot.return", &Target);
      auto *Failed = ::llvm::BasicBlock::Create(Context.Context, "hot.failed", &Target);
      Builder.CreateCondBr(Builder.CreateICmpEQ(Status, Builder.getInt32(0)), Succeeded, Failed);
      Builder.SetInsertPoint(Failed);
      Builder.CreateCall(Context.helper("ink_hybrid_panic", Builder.getVoidTy(), {}));
      Builder.CreateUnreachable();
      Builder.SetInsertPoint(Succeeded);
      ::llvm::Value *Result = Return.typeKind() == ir::TypeKind::Void ? nullptr : memoryLoad(Return, Storage);
      if (!Result && Return.typeKind() != ir::TypeKind::Void)
      {
        return false;
      }
      Builder.CreateCall(Leave);
      if (Result)
      {
        Builder.CreateRet(Result);
      }
      else
      {
        Builder.CreateRetVoid();
      }
      return true;
    }

    bool FunctionLowering::lowerReflection()
    {
      Builder.SetInsertPoint(::llvm::BasicBlock::Create(Context.Context, "entry", &Target));
      std::vector<::llvm::Value *> Arguments;
      for (std::size_t Index = 0; Index < Source.parameters().size(); ++Index)
      {
        auto *Slot = Builder.CreateGEP(Context.PointerType, Target.getArg(1), ::llvm::ConstantInt::get(Context.SizeType, Index));
        auto *Address = Builder.CreateLoad(Context.PointerType, Slot);
        auto *Argument = memoryLoad(Source.parameters()[Index]->type(), Address);
        if (!Argument)
        {
          return false;
        }
        Arguments.push_back(Argument);
      }
      auto *Result = Builder.CreateCall(Context.Functions.at(&Source), Arguments);
      if (Source.functionType().returnType().typeKind() != ir::TypeKind::Void && !memoryStore(Source.functionType().returnType(), Result, Target.getArg(0)))
      {
        return false;
      }
      Builder.CreateRetVoid();
      return true;
    }

    ::llvm::Value *FunctionLowering::value(const ir::Value &Value)
    {
      if (ir::Constant::classof(&Value) || ir::Function::classof(&Value))
      {
        return Context.constant(Value);
      }
      const auto Found = Values.find(&Value);
      if (Found == Values.end())
      {
        Context.fail("AOT encountered an unresolved instruction operand");
        return nullptr;
      }
      if (ir::FunctionParameter::classof(&Value))
      {
        return Found->second;
      }
      require(Builder.CreateLoad(Builder.getInt1Ty(), Initialized.at(&Value)), ExecutionStatus::RuntimeValue);
      return Builder.CreateLoad(Context.lowerType(Value.type()), Found->second);
    }

    ::llvm::Value *FunctionLowering::offset(::llvm::Value *Address, std::uint64_t Bytes)
    {
      return Builder.CreateGEP(Builder.getInt8Ty(), Address, ::llvm::ConstantInt::get(Context.SizeType, Bytes));
    }

    ::llvm::Value *FunctionLowering::memoryLoad(const ir::Type &Type, ::llvm::Value *Address)
    {
      ::llvm::Type *TargetType = Context.lowerType(Type);
      if (!TargetType)
      {
        return nullptr;
      }
      if (Type.typeKind() == ir::TypeKind::Bool)
      {
        ::llvm::Value *Bits = Builder.CreateAlignedLoad(Builder.getInt8Ty(), Address, ::llvm::Align(1));
        require(Builder.CreateICmpULE(Bits, Builder.getInt8(1)), ExecutionStatus::TypeMismatch);
        return Builder.CreateTrunc(Bits, Builder.getInt1Ty());
      }
      if (Type.typeKind() == ir::TypeKind::Class)
      {
        const auto &Class = static_cast<const ir::ClassType &>(Type);
        const auto Layout = Context.layout(Type);
        if (!Layout)
        {
          return nullptr;
        }
        ::llvm::Value *Result = ::llvm::Constant::getNullValue(TargetType);
        for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
        {
          ::llvm::Value *Field = memoryLoad(*Class.fields()[Index].FieldType, offset(Address, Layout->FieldOffsets[Index]));
          if (!Field)
          {
            return nullptr;
          }
          Result = Builder.CreateInsertValue(Result, Field, {static_cast<unsigned>(Index)});
        }
        return Result;
      }
      if (Type.typeKind() == ir::TypeKind::Array)
      {
        const auto &Array = static_cast<const ir::ArrayType &>(Type);
        const auto Layout = Context.layout(Array.elementType());
        if (!Layout)
        {
          return nullptr;
        }
        ::llvm::Value *Result = ::llvm::Constant::getNullValue(TargetType);
        for (std::uint64_t Index = 0; Index < Array.elementCount(); ++Index)
        {
          ::llvm::Value *Element = memoryLoad(Array.elementType(), offset(Address, Layout->Stride * Index));
          if (!Element)
          {
            return nullptr;
          }
          Result = Builder.CreateInsertValue(Result, Element, {static_cast<unsigned>(Index)});
        }
        return Result;
      }
      return Builder.CreateAlignedLoad(TargetType, Address, ::llvm::Align(1));
    }

    bool FunctionLowering::memoryStore(const ir::Type &Type, ::llvm::Value *Value, ::llvm::Value *Address)
    {
      if (Type.typeKind() == ir::TypeKind::Bool)
      {
        Builder.CreateAlignedStore(Builder.CreateZExt(Value, Builder.getInt8Ty()), Address, ::llvm::Align(1));
        return true;
      }
      if (Type.typeKind() == ir::TypeKind::Class)
      {
        const auto &Class = static_cast<const ir::ClassType &>(Type);
        const auto Layout = Context.layout(Type);
        if (!Layout)
        {
          return false;
        }
        for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
        {
          if (!memoryStore(*Class.fields()[Index].FieldType, Builder.CreateExtractValue(Value, {static_cast<unsigned>(Index)}), offset(Address, Layout->FieldOffsets[Index])))
          {
            return false;
          }
        }
        return true;
      }
      if (Type.typeKind() == ir::TypeKind::Array)
      {
        const auto &Array = static_cast<const ir::ArrayType &>(Type);
        const auto Layout = Context.layout(Array.elementType());
        if (!Layout)
        {
          return false;
        }
        for (std::uint64_t Index = 0; Index < Array.elementCount(); ++Index)
        {
          if (!memoryStore(Array.elementType(), Builder.CreateExtractValue(Value, {static_cast<unsigned>(Index)}), offset(Address, Layout->Stride * Index)))
          {
            return false;
          }
        }
        return true;
      }
      Builder.CreateAlignedStore(Value, Address, ::llvm::Align(1));
      return true;
    }

    ::llvm::Value *FunctionLowering::checkedIndex(const ir::Value &Index, std::uint64_t Count)
    {
      ::llvm::Value *Result = value(Index);
      if (!Result || !Result->getType()->isIntegerTy())
      {
        Context.fail("AOT array index must be an integer");
        return nullptr;
      }
      const auto Width = Result->getType()->getIntegerBitWidth();
      if (static_cast<const ir::IntegerType &>(Index.type()).isSigned())
      {
        require(Builder.CreateICmpSGE(Result, ::llvm::ConstantInt::get(Result->getType(), 0)), ExecutionStatus::IndexOutOfBounds);
      }
      // Compare before truncating wide indices so negative and overflowing values cannot wrap.
      const auto CompareWidth = std::max<unsigned>(Width, Context.SizeType->getBitWidth());
      auto *CompareType = ::llvm::IntegerType::get(Context.Context, CompareWidth);
      ::llvm::Value *Extended = Builder.CreateZExtOrTrunc(Result, CompareType);
      require(Builder.CreateICmpULT(Extended, ::llvm::ConstantInt::get(CompareType, Count)), ExecutionStatus::IndexOutOfBounds);
      return Builder.CreateZExtOrTrunc(Result, Context.SizeType);
    }

    bool FunctionLowering::call(const ir::CallInstruction &Call, ::llvm::Value *&Result)
    {
      const ir::Function *Direct = Call.directCallee();
      const bool Native = Direct && Direct->isNativeImport() && !Context.NativeDefinitions.contains(Direct);
      ::llvm::FunctionType *Signature = Context.signature(Call.functionType(), Native);
      ::llvm::Value *Callee = value(Call.callee());
      if (!Signature || !Callee)
      {
        return false;
      }
      if (!Direct)
      {
        return Context.fail("AOT indirect calls are not yet supported");
      }
      std::vector<::llvm::Value *> Arguments;
      for (std::size_t Index = 0; Index < Call.arguments().size(); ++Index)
      {
        const ir::Value &Argument = *Call.arguments()[Index];
        const ir::Type &Parameter = *Call.functionType().parameterTypes()[Index];
        ::llvm::Value *Item = value(Argument);
        if (!Item)
        {
          return false;
        }
        if (Native && Parameter.typeKind() == ir::TypeKind::Pointer)
        {
          if (Argument.type().typeKind() == ir::TypeKind::Slice)
          {
            if (!ir::StringConstant::classof(&Argument) || !static_cast<const ir::StringConstant &>(Argument).tryGetCString())
            {
              return Context.fail("AOT implicit C string conversion requires a NUL-free string constant");
            }
            Item = Builder.CreateExtractValue(Item, {0});
          }
        }
        Arguments.push_back(Item);
      }
      Result = Builder.CreateCall(Signature, Callee, Arguments);
      return true;
    }

    bool FunctionLowering::instruction(const ir::Value &Instruction)
    {
      ::llvm::Value *Result = nullptr;
      switch (Instruction.kind())
      {
        case ir::ValueKind::Function:
        case ir::ValueKind::Module:
          return true;
        case ir::ValueKind::AllocaInstruction:
        {
          const auto Layout = Context.layout(static_cast<const ir::AllocaInstruction &>(Instruction).allocatedType());
          if (!Layout)
          {
            return false;
          }
          auto *Storage = Builder.CreateAlloca(Builder.getInt8Ty(), ::llvm::ConstantInt::get(Context.SizeType, std::max<std::uint64_t>(Layout->Size, 1)), "object");
          Storage->setAlignment(::llvm::Align(Layout->Alignment));
          Result = Storage;
          break;
        }
        case ir::ValueKind::CStringInstruction:
        {
          const auto &String = static_cast<const ir::CStringInstruction &>(Instruction).source();
          ::llvm::Value *Constant = Context.constant(String);
          if (!Constant)
          {
            return false;
          }
          auto *Size = ::llvm::ConstantInt::get(Context.SizeType, String.value().size() + 1);
          Result = Builder.CreateAlloca(Builder.getInt8Ty(), Size, "cstring");
          Builder.CreateMemCpy(Result, ::llvm::Align(1), Builder.CreateExtractValue(Constant, {0}), ::llvm::Align(1), Size);
          break;
        }
        case ir::ValueKind::LoadInstruction:
        {
          const auto &Load = static_cast<const ir::LoadInstruction &>(Instruction);
          ::llvm::Value *Pointer = value(Load.address());
          const auto Layout = Context.layout(Load.type());
          if (!Pointer || !Layout)
          {
            return false;
          }
          Result = memoryLoad(Load.type(), Pointer);
          if (!Result)
          {
            return false;
          }
          break;
        }
        case ir::ValueKind::StoreInstruction:
        {
          const auto &Store = static_cast<const ir::StoreInstruction &>(Instruction);
          ::llvm::Value *Pointer = value(Store.address());
          ::llvm::Value *Stored = value(Store.storedValue());
          const auto Layout = Context.layout(Store.storedValue().type());
          if (!Pointer || !Layout || !Stored)
          {
            return false;
          }
          if (static_cast<const ir::PointerType &>(Store.address().type()).access() != ir::AccessKind::ReadWrite)
          {
            return Context.fail("AOT cannot store through a read-only pointer");
          }
          if (!memoryStore(Store.storedValue().type(), Stored, Pointer))
          {
            return false;
          }
          return true;
        }
        case ir::ValueKind::ClassInstruction:
        {
          const auto &Class = static_cast<const ir::ClassInstruction &>(Instruction);
          ::llvm::Type *Type = Context.lowerType(Class.type());
          if (!Type)
          {
            return false;
          }
          Result = ::llvm::Constant::getNullValue(Type);
          for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
          {
            ::llvm::Value *Field = value(*Class.fields()[Index]);
            if (!Field)
            {
              return false;
            }
            Result = Builder.CreateInsertValue(Result, Field, {static_cast<unsigned>(Index)});
          }
          break;
        }
        case ir::ValueKind::FieldExtractInstruction:
        {
          const auto &Extract = static_cast<const ir::FieldExtractInstruction &>(Instruction);
          ::llvm::Value *Object = value(Extract.object());
          if (!Object)
          {
            return false;
          }
          Result = Builder.CreateExtractValue(Object, {static_cast<unsigned>(Extract.fieldIndex())});
          break;
        }
        case ir::ValueKind::FieldPointerInstruction:
        {
          const auto &Field = static_cast<const ir::FieldPointerInstruction &>(Instruction);
          const ir::Type &ObjectType = static_cast<const ir::PointerType &>(Field.address().type()).pointeeType();
          const auto Layout = Context.layout(ObjectType);
          const auto FieldLayout = Context.layout(static_cast<const ir::PointerType &>(Field.type()).pointeeType());
          ::llvm::Value *Pointer = value(Field.address());
          if (!Layout || !FieldLayout || !Pointer || Field.fieldIndex() >= Layout->FieldOffsets.size())
          {
            return Context.fail("AOT field address has an invalid layout or index");
          }
          Result = offset(Pointer, Layout->FieldOffsets[Field.fieldIndex()]);
          break;
        }
        case ir::ValueKind::ArrayInstruction:
        {
          const auto &Array = static_cast<const ir::ArrayInstruction &>(Instruction);
          ::llvm::Type *Type = Context.lowerType(Array.type());
          if (!Type)
          {
            return false;
          }
          Result = ::llvm::Constant::getNullValue(Type);
          for (std::uint64_t Index = 0; Index < Array.arrayType().elementCount(); ++Index)
          {
            const ir::Value &Element = *Array.elements()[Array.repeated() ? 0 : Index];
            ::llvm::Value *Item = value(Element);
            if (!Item)
            {
              return false;
            }
            Result = Builder.CreateInsertValue(Result, Item, {static_cast<unsigned>(Index)});
          }
          break;
        }
        case ir::ValueKind::ArrayElementPointerInstruction:
        {
          const auto &Element = static_cast<const ir::ArrayElementPointerInstruction &>(Instruction);
          const auto &Array = static_cast<const ir::ArrayType &>(static_cast<const ir::PointerType &>(Element.address().type()).pointeeType());
          const auto Layout = Context.layout(Array.elementType());
          ::llvm::Value *Pointer = value(Element.address());
          ::llvm::Value *Index = checkedIndex(Element.index(), Array.elementCount());
          if (!Layout || !Pointer || !Index)
          {
            return false;
          }
          Result = Builder.CreateGEP(Builder.getInt8Ty(), Pointer, Builder.CreateMul(Index, ::llvm::ConstantInt::get(Context.SizeType, Layout->Stride)));
          break;
        }
        case ir::ValueKind::ArrayExtractInstruction:
        {
          const auto &Extract = static_cast<const ir::ArrayExtractInstruction &>(Instruction);
          const auto &Array = static_cast<const ir::ArrayType &>(Extract.array().type());
          ::llvm::Value *Object = value(Extract.array());
          ::llvm::Value *Index = checkedIndex(Extract.index(), Array.elementCount());
          if (!Index || !Object)
          {
            return false;
          }
          ::llvm::IRBuilder<> Prologue(&Target.getEntryBlock(), Target.getEntryBlock().begin());
          auto *Temporary = Prologue.CreateAlloca(Object->getType());
          Builder.CreateStore(Object, Temporary);
          ::llvm::Value *Address = Builder.CreateGEP(Object->getType(), Temporary, {Builder.getInt32(0), Index});
          Result = Builder.CreateLoad(Context.lowerType(Extract.type()), Address);
          break;
        }
        case ir::ValueKind::AddInstruction:
        case ir::ValueKind::LogicalAndInstruction:
        case ir::ValueKind::LogicalOrInstruction:
        {
          const ir::Value *LeftSource = nullptr;
          const ir::Value *RightSource = nullptr;
          if (Instruction.kind() == ir::ValueKind::AddInstruction)
          {
            const auto &Add = static_cast<const ir::AddInstruction &>(Instruction);
            LeftSource = &Add.left();
            RightSource = &Add.right();
          }
          else if (Instruction.kind() == ir::ValueKind::LogicalAndInstruction)
          {
            const auto &And = static_cast<const ir::LogicalAndInstruction &>(Instruction);
            LeftSource = &And.left();
            RightSource = &And.right();
          }
          else
          {
            const auto &Or = static_cast<const ir::LogicalOrInstruction &>(Instruction);
            LeftSource = &Or.left();
            RightSource = &Or.right();
          }
          ::llvm::Value *Left = value(*LeftSource);
          ::llvm::Value *Right = value(*RightSource);
          if (!Left || !Right)
          {
            return false;
          }
          Result = Instruction.kind() == ir::ValueKind::AddInstruction ? Builder.CreateAdd(Left, Right) : Instruction.kind() == ir::ValueKind::LogicalAndInstruction ? Builder.CreateAnd(Left, Right) : Builder.CreateOr(Left, Right);
          break;
        }
        case ir::ValueKind::LogicalNotInstruction:
        {
          ::llvm::Value *Operand = value(static_cast<const ir::LogicalNotInstruction &>(Instruction).operand());
          if (!Operand)
          {
            return false;
          }
          Result = Builder.CreateNot(Operand);
          break;
        }
        case ir::ValueKind::CompareInstruction:
        {
          const auto &Compare = static_cast<const ir::CompareInstruction &>(Instruction);
          ::llvm::Value *Left = value(Compare.left());
          ::llvm::Value *Right = value(Compare.right());
          if (!Left || !Right)
          {
            return false;
          }
          const bool Signed = Compare.left().type().typeKind() == ir::TypeKind::Integer && static_cast<const ir::IntegerType &>(Compare.left().type()).isSigned();
          ::llvm::CmpInst::Predicate Predicate = ::llvm::CmpInst::ICMP_EQ;
          switch (Compare.predicate())
          {
            case core::ComparisonPredicate::Equal:
              Predicate = ::llvm::CmpInst::ICMP_EQ;
              break;
            case core::ComparisonPredicate::NotEqual:
              Predicate = ::llvm::CmpInst::ICMP_NE;
              break;
            case core::ComparisonPredicate::Less:
              Predicate = Signed ? ::llvm::CmpInst::ICMP_SLT : ::llvm::CmpInst::ICMP_ULT;
              break;
            case core::ComparisonPredicate::LessEqual:
              Predicate = Signed ? ::llvm::CmpInst::ICMP_SLE : ::llvm::CmpInst::ICMP_ULE;
              break;
            case core::ComparisonPredicate::Greater:
              Predicate = Signed ? ::llvm::CmpInst::ICMP_SGT : ::llvm::CmpInst::ICMP_UGT;
              break;
            case core::ComparisonPredicate::GreaterEqual:
              Predicate = Signed ? ::llvm::CmpInst::ICMP_SGE : ::llvm::CmpInst::ICMP_UGE;
              break;
          }
          Result = Builder.CreateICmp(Predicate, Left, Right);
          break;
        }
        case ir::ValueKind::CallInstruction:
          if (!call(static_cast<const ir::CallInstruction &>(Instruction), Result))
          {
            return false;
          }
          break;
        case ir::ValueKind::ReturnInstruction:
        {
          const ir::Value *Returned = static_cast<const ir::ReturnInstruction &>(Instruction).returnedValue();
          ::llvm::Value *ReturnValue = Returned ? value(*Returned) : nullptr;
          if (Returned && !ReturnValue)
          {
            return false;
          }
          if (ReturnValue)
          {
            Builder.CreateRet(ReturnValue);
          }
          else
          {
            Builder.CreateRetVoid();
          }
          return true;
        }
        case ir::ValueKind::BranchInstruction:
          Builder.CreateBr(Blocks.at(&static_cast<const ir::BranchInstruction &>(Instruction).target()));
          return true;
        case ir::ValueKind::ConditionalBranchInstruction:
        {
          const auto &Branch = static_cast<const ir::ConditionalBranchInstruction &>(Instruction);
          ::llvm::Value *Condition = value(Branch.condition());
          if (!Condition)
          {
            return false;
          }
          Builder.CreateCondBr(Condition, Blocks.at(&Branch.trueTarget()), Blocks.at(&Branch.falseTarget()));
          return true;
        }
        default:
          return Context.fail("AOT encountered an unsupported instruction kind");
      }
      if (Instruction.type().typeKind() != ir::TypeKind::Void)
      {
        if (!Result)
        {
          return Context.fail("AOT instruction did not produce its declared value");
        }
        Builder.CreateStore(Result, Values.at(&Instruction));
        Builder.CreateStore(Builder.getTrue(), Initialized.at(&Instruction));
      }
      return true;
    }
  } // namespace

  bool LoweringContext::lowerFunction(const ir::Function &Function)
  {
    return FunctionLowering(*this, Function, *Functions.at(&Function)).lower();
  }
  ::llvm::Function *LoweringContext::reflectionThunk(const ir::Function &Function)
  {
    if (!Functions.contains(&Function))
    {
      return nullptr;
    }
    auto *Signature = ::llvm::FunctionType::get(::llvm::Type::getVoidTy(Context), {PointerType, PointerType}, false);
    const auto Mangled = ir::reflectionThunkSymbol(Function);
    if (!Mangled)
    {
      fail(Mangled.Error);
      return nullptr;
    }
    auto *Thunk = ::llvm::Function::Create(Signature, ::llvm::GlobalValue::PrivateLinkage, Mangled.Name, Module);
    return FunctionLowering(*this, Function, *Thunk).lowerReflection() ? Thunk : nullptr;
  }
} // namespace ink::backend::llvm
