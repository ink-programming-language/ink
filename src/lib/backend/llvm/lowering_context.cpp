#include "lowering_context.h"

#include "ink/ir/type/pointer_type.h"
#include "ink/ir/linkage.h"
#include "ink/ir/type/class_type.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Constants.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <utility>
#include <algorithm>

namespace ink::backend::llvm
{
  LoweringContext::LoweringContext(::llvm::LLVMContext &Context, const ir::Module &Source, ::llvm::Module &Module, std::string &Error)
      : Context(Context),
        Source(Source),
        Module(Module),
        Error(Error),
        SizeType(::llvm::IntegerType::get(Context, static_cast<unsigned>(Source.context().compilationContext().targetContext().pointerWidth()))),
        PointerType(::llvm::PointerType::getUnqual(Context))
  {
  }

  bool LoweringContext::fail(std::string Message)
  {
    if (Error.empty())
    {
      Error = std::move(Message);
    }
    return false;
  }

  std::optional<ir::TypeLayout> LoweringContext::layout(const ir::Type &Type)
  {
    const auto Result = ir::computeTypeLayout(Type, Source.context().compilationContext().targetContext());
    if (!Result)
    {
      fail("AOT type has no finite supported target layout");
      return std::nullopt;
    }
    // Memory scalar types must match the target, including arbitrary-width
    // integer allocation sizes. Aggregate offsets come from ObjectLayoutBuilder.
    if (Type.typeKind() != ir::TypeKind::Class && Type.typeKind() != ir::TypeKind::Array)
    {
      ::llvm::Type *Storage = Type.typeKind() == ir::TypeKind::Bool ? ::llvm::Type::getInt8Ty(Context) : lowerType(Type);
      const auto &Native = Module.getDataLayout();
      if (!Storage || Native.getTypeAllocSize(Storage) != Result->Size || Native.getABITypeAlign(Storage).value() != Result->Alignment)
      {
        fail("LLVM scalar storage layout differs from the Ink target ABI");
        return std::nullopt;
      }
    }
    return Result;
  }

  ::llvm::FunctionCallee LoweringContext::helper(::llvm::StringRef Name, ::llvm::Type *Result, ::llvm::ArrayRef<::llvm::Type *> Parameters)
  {
    return Module.getOrInsertFunction(Name, ::llvm::FunctionType::get(Result, Parameters, false));
  }

  void LoweringContext::collect(const ir::BasicBlock &Block)
  {
    for (const auto &Value : Block.values())
    {
      if (ir::Module::classof(Value.get()))
      {
        collect(static_cast<const ir::Module &>(*Value).entryBlock());
      }
      else if (ir::Function::classof(Value.get()))
      {
        const auto &Function = static_cast<const ir::Function &>(*Value);
        SourceFunctions.push_back(&Function);
        for (const auto &Body : Function.blocks())
        {
          collect(*Body);
        }
      }
    }
  }

  bool LoweringContext::declareFunctions()
  {
    std::unordered_map<std::string, const ir::Function *> Exports;
    for (const ir::Function *Function : SourceFunctions)
    {
      if (Function->isNativeExport())
      {
        const std::string Name(Source.context().namePool().text(Function->name()));
        if (!Function->hasBody())
        {
          return fail("AOT native export requires a function body: " + Name);
        }
        if (!Exports.emplace(Name, Function).second)
        {
          return fail("Duplicate AOT native export: " + Name);
        }
      }
    }
    for (const ir::Function *Function : SourceFunctions)
    {
      const std::string Name(Source.context().namePool().text(Function->name()));
      if ((Function->isNativeImport() || Function->isNativeExport()) && Function->languageLinkage() != ir::LanguageLinkage::C)
      {
        return fail("AOT native symbols require C language linkage: " + Name);
      }
      if ((Function->isNativeImport() || Function->isNativeExport()) && (Name.starts_with("ink_aot_") || Name.starts_with("_INK") || Name == "main"))
      {
        return fail("AOT reserves this native runtime symbol: " + Name);
      }
      if ((Function->hasBody() || Function->isNativeImport()) && Function->callingConvention() != ir::CallingConvention::C)
      {
        return fail("AOT currently supports only the C calling convention");
      }
      for (const auto &Parameter : Function->parameters())
      {
        if (Parameter->parameterKind() == ir::ParameterKind::Variadic)
        {
          return fail("AOT variadic functions are not supported yet");
        }
      }
      if (Function->isNativeImport())
      {
        const auto Found = Exports.find(Name);
        if (Found != Exports.end())
        {
          if (&Found->second->functionType() != &Function->functionType())
          {
            return fail("Native import signature does not match its Ink definition: " + Name);
          }
          NativeDefinitions.emplace(Function, Found->second);
          continue;
        }
      }
      if (!Function->hasBody() && !Function->isNativeImport())
      {
        continue;
      }
      const bool Native = Function->isNativeImport();
      ::llvm::FunctionType *Signature = signature(Function->functionType(), Native);
      if (!Signature)
      {
        return false;
      }
      const auto Mangled = Native ? abi::MangleResult{Name, {}} : ir::functionSymbol(*Function);
      if (!Mangled)
      {
        return fail(Mangled.Error + ": " + Name);
      }
      const std::string &Symbol = Mangled.Name;
      if (Native)
      {
        if (::llvm::Function *Existing = Module.getFunction(Symbol))
        {
          if (Existing->getFunctionType() != Signature)
          {
            return fail("Conflicting AOT native declarations: " + Symbol);
          }
          Functions.emplace(Function, Existing);
          continue;
        }
      }
      if (!Native && Module.getNamedValue(Symbol))
      {
        return fail("Duplicate Ink linkage identity: " + Symbol);
      }
      auto *Target = ::llvm::Function::Create(Signature, ::llvm::GlobalValue::ExternalLinkage, Symbol, Module);
      if (!Native && Function->visibility() == core::VisibilityKind::Private)
      {
        Target->setVisibility(::llvm::GlobalValue::HiddenVisibility);
      }
      Functions.emplace(Function, Target);
    }
    for (const auto &[Imported, Definition] : NativeDefinitions)
    {
      Functions.emplace(Imported, Functions.at(Definition));
    }
    return true;
  }

  bool LoweringContext::lower(const ir::Function *Entry)
  {
    for (const auto &Root : Source.context().modules())
    {
      collect(Root->entryBlock());
      for (const auto *Class : Root->classTypes())
      {
        if (Class->isComplete() && !recordClassABI(*Class))
        {
          return false;
        }
      }
    }
    if (!declareFunctions())
    {
      return false;
    }
    for (const ir::Function *Function : SourceFunctions)
    {
      if (Function->hasBody() && !lowerFunction(*Function))
      {
        return false;
      }
    }
    if (!lowerNativeExports() || !lowerReflection() || (Entry && !lowerEntry(*Entry)))
    {
      return false;
    }
    emitABIMetadata();
    return true;
  }

  bool LoweringContext::recordClassABI(const ir::ClassType &Class)
  {
    const auto Nominal = ir::typeRecord(Class);
    const auto Layout = layout(Class);
    if (!Nominal || !Layout)
    {
      return fail("AOT class requires a canonical nominal identity and finite layout");
    }
    std::vector<abi::Record> Fields;
    for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
    {
      const auto &Field = Class.fields()[Index];
      const auto Type = ir::typeRecord(*Field.FieldType);
      if (!Type)
      {
        return fail("AOT class field has no stable type identity");
      }
      Fields.push_back(abi::record('E', {abi::nameRecord(Class.context().namePool().text(Field.FieldName)), *Type, {'D', std::to_string(Layout->FieldOffsets[Index])}, {'D', std::to_string(static_cast<unsigned>(Field.Visibility))}, {'D', Field.Initializer ? "1" : "0"}}));
    }
    std::vector<std::string> Methods;
    for (const auto *Method : Class.methods())
    {
      const auto Identity = ir::functionSymbol(*Method);
      if (!Identity)
      {
        return fail(Identity.Error);
      }
      Methods.push_back(Identity.Name + ":" + std::to_string(static_cast<unsigned>(Method->visibility())));
    }
    std::sort(Methods.begin(), Methods.end());
    std::vector<abi::Record> MethodRecords;
    for (const auto &Method : Methods)
    {
      MethodRecords.push_back({'F', Method});
    }
    const auto Definition = abi::encodeRecord(abi::record('L', {{'D', std::to_string(Layout->Size)}, {'D', std::to_string(Layout->Alignment)}, abi::record('L', Fields), abi::record('L', MethodRecords)}));
    const auto [Found, Inserted] = ClassDefinitions.emplace(Class.identity(), Definition);
    return Inserted || Found->second == Definition || fail("Conflicting AOT definitions of nominal type: " + std::string(Class.identity()));
  }

  void LoweringContext::emitABIMetadata()
  {
    Module.addModuleFlag(::llvm::Module::Error, "ink.abi.version", 1);
    Module.addModuleFlag(::llvm::Module::Error, "ink.mangling.version", 2);
    std::vector<std::pair<std::string, std::string>> Definitions(ClassDefinitions.begin(), ClassDefinitions.end());
    for (const auto &[SourceFunction, Target] : Functions)
    {
      if (!SourceFunction->isNativeImport())
      {
        std::string Signature;
        ::llvm::raw_string_ostream Stream(Signature);
        Target->getFunctionType()->print(Stream);
        if (SourceFunction->isNativeExport())
        {
          Definitions.emplace_back(SourceFunction->context().namePool().text(SourceFunction->name()), Signature);
        }
        Definitions.emplace_back(Target->getName().str(), std::move(Signature));
      }
    }
    std::sort(Definitions.begin(), Definitions.end());
    std::vector<abi::Record> Entries;
    for (const auto &[Name, Definition] : Definitions)
    {
      Entries.push_back(abi::record('E', {{'N', Name}, {'D', Definition}}));
      Module.getOrInsertNamedMetadata("ink.abi.definitions")->addOperand(::llvm::MDNode::get(Context, {::llvm::MDString::get(Context, Name), ::llvm::MDString::get(Context, Definition)}));
    }
    const std::string Manifest = "INKABI1" + abi::encodeRecord(abi::record('L', {{'T', Module.getTargetTriple().str()}, {'D', Module.getDataLayoutStr()}, abi::record('L', Entries)}));
    auto *Bytes = ::llvm::ConstantDataArray::getString(Context, Manifest, false);
    auto *Metadata = new ::llvm::GlobalVariable(Module, Bytes->getType(), true, ::llvm::GlobalValue::PrivateLinkage, Bytes, "ink.abi");
    Metadata->setSection(".inkabi");
    ::llvm::appendToCompilerUsed(Module, {Metadata});
  }

  bool LoweringContext::lowerNativeExports()
  {
    for (const ir::Function *SourceFunction : SourceFunctions)
    {
      if (!SourceFunction->isNativeExport())
      {
        continue;
      }
      const auto &Signature = SourceFunction->functionType();
      if (!nativeType(Signature.returnType(), true))
      {
        return fail("AOT native exports require scalar or raw pointer results");
      }
      for (const ir::Type *Parameter : Signature.parameterTypes())
      {
        if (!nativeType(*Parameter))
        {
          return fail("AOT native exports require scalar or raw pointer parameters");
        }
      }
      ::llvm::Function *Internal = Functions.at(SourceFunction);
      const std::string Name(Source.context().namePool().text(SourceFunction->name()));
      auto *Export = ::llvm::Function::Create(Internal->getFunctionType(), ::llvm::GlobalValue::ExternalLinkage, Name, Module);
      ::llvm::IRBuilder<> Builder(::llvm::BasicBlock::Create(Context, "entry", Export));
      std::vector<::llvm::Value *> Arguments;
      for (::llvm::Argument &Argument : Export->args())
      {
        Arguments.push_back(&Argument);
      }
      ::llvm::Value *Result = Builder.CreateCall(Internal, Arguments);
      if (Internal->getReturnType()->isVoidTy())
      {
        Builder.CreateRetVoid();
      }
      else
      {
        Builder.CreateRet(Result);
      }
    }
    return true;
  }

  bool LoweringContext::lowerEntry(const ir::Function &Entry)
  {
    const auto Found = Functions.find(&Entry);
    if (Found == Functions.end() || !Entry.parameters().empty() || Entry.isNativeImport())
    {
      return fail("AOT entry must be a defined function without parameters");
    }
    ::llvm::Type *ReturnType = Found->second->getReturnType();
    if (!ReturnType->isVoidTy() && !ReturnType->isIntegerTy(32))
    {
      return fail("AOT entry must return void or i32");
    }
    auto *Main = ::llvm::Function::Create(::llvm::FunctionType::get(::llvm::Type::getInt32Ty(Context), false), ::llvm::GlobalValue::ExternalLinkage, "main", Module);
    ::llvm::IRBuilder<> Builder(::llvm::BasicBlock::Create(Context, "entry", Main));
    ::llvm::Value *Result = Builder.CreateCall(Found->second);
    if (ReturnType->isVoidTy())
    {
      Result = Builder.getInt32(0);
    }
    Builder.CreateRet(Result);
    return true;
  }
} // namespace ink::backend::llvm
