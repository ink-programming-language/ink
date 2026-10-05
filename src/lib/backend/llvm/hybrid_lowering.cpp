#include "lowering_context.h"
#include "ink/execution/artifact/bytecode_builder.h"
#include "ink/execution/hybrid/archive.h"
#include "ink/execution/hybrid/runtime.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>

namespace ink::backend::llvm
{
  bool LoweringContext::prepareHybrid(const ir::Function *Entry)
  {
    using namespace execution;
    SemanticValueBridge Bridge(const_cast<ir::IRContext &>(Source.context()));
    Bridge.exchangeNativeImportResolution(false);
    std::vector<BytecodeFunctionInput> Inputs;
    std::unordered_set<std::string> MatchedModules;
    for (const auto *Function : SourceFunctions)
    {
      const ir::Module *Owner = nullptr;
      for (const ir::Value *Outer = Function->outer(); Outer; Outer = Outer->outer())
      {
        if (ir::Module::classof(Outer))
        {
          Owner = static_cast<const ir::Module *>(Outer);
          break;
        }
      }
      if (!Owner || (!Function->hasBody() && !Function->isNativeImport()))
      {
        continue;
      }
      const std::string ModuleName(Source.context().namePool().text(Owner->name()));
      const bool Selected = HotModules.empty() || std::find(HotModules.begin(), HotModules.end(), ModuleName) != HotModules.end();
      if (Selected)
      {
        MatchedModules.insert(ModuleName);
      }
      BytecodeFunctionInput Input;
      Input.Function = Function;
      Input.Kind = Function->isNativeImport() ? BytecodeSymbolKind::Native : BytecodeSymbolKind::Import;
      Input.Identity.Module = Function->isNativeImport() ? "C" : ModuleName;
      Input.Visibility = Function->visibility() == core::VisibilityKind::Public ? BytecodeVisibility::Public : BytecodeVisibility::Private;
      Inputs.push_back(std::move(Input));
      const auto Id = Bridge.lowerFunction(*Function);
      HybridSources.emplace(Id, Function);
      if (Selected && Function->hasBody() && Function != Entry && Source.context().namePool().text(Function->name()) != "main")
      {
        HotFunctions.emplace(Function, Id);
      }
    }
    for (const auto &Name : HotModules)
    {
      if (!MatchedModules.contains(Name))
      {
        return fail("Unknown hot reload module: " + Name);
      }
    }
    auto Built = buildBytecodeObject(Source.context().namePool().text(Source.name()), Bridge, Inputs);
    if (!Built)
    {
      return fail(Built.Message);
    }
    HybridMetadata = std::move(Built.Artifact);
    auto *Type = ::llvm::StructType::get(Context, {::llvm::Type::getInt32Ty(Context), PointerType, SizeType, PointerType, SizeType});
    if (Module.getDataLayout().getTypeAllocSize(Type) != sizeof(InkHybridModule))
    {
      return fail("Hybrid module descriptor differs from the host ABI");
    }
    HybridModule = new ::llvm::GlobalVariable(Module, Type, true, ::llvm::GlobalValue::PrivateLinkage, nullptr, "ink.hybrid.module");
    return true;
  }

  bool LoweringContext::finishHybrid()
  {
    using namespace execution;
    auto *Word = ::llvm::Type::getInt32Ty(Context);
    auto *FunctionType = ::llvm::StructType::get(Context, {Word, Word, PointerType});
    if (Module.getDataLayout().getTypeAllocSize(FunctionType) != sizeof(InkHybridFunction))
    {
      return fail("Hybrid function descriptor differs from the host ABI");
    }
    std::vector<::llvm::Constant *> Entries;
    for (const auto &Symbol : HybridMetadata->Symbols)
    {
      const auto Found = HybridSources.find(Symbol.Function);
      if (Found == HybridSources.end() || (!Found->second->hasBody() && !Found->second->isNativeImport()))
      {
        return fail("Hybrid native manifest contains a missing body");
      }
      auto *Thunk = reflectionThunk(*Found->second);
      if (!Thunk)
      {
        return false;
      }
      const unsigned Flags = (HotFunctions.contains(Found->second) ? 1U : 0U) | (Found->second->isNativeExport() ? 2U : 0U) | (Found->second->isNativeImport() ? 4U : 0U);
      Entries.push_back(::llvm::ConstantStruct::get(FunctionType, {::llvm::ConstantInt::get(Word, Symbol.Function), ::llvm::ConstantInt::get(Word, Flags), Thunk}));
    }
    auto *ArrayType = ::llvm::ArrayType::get(FunctionType, Entries.size());
    auto *FunctionTable = new ::llvm::GlobalVariable(Module, ArrayType, true, ::llvm::GlobalValue::PrivateLinkage, ::llvm::ConstantArray::get(ArrayType, Entries), "ink.hybrid.functions");
    std::string Code;
    ::llvm::raw_string_ostream Stream(Code);
    Module.print(Stream, nullptr);
    auto Serialized = serializeHybridArtifact(HybridArtifactKind::Base, hybridBuildId(Code), *HybridMetadata);
    if (!Serialized)
    {
      return fail(Serialized.Message);
    }
    HybridManifest = std::move(Serialized.Bytes);
    auto *Bytes = ::llvm::ConstantDataArray::getString(Context, ::llvm::StringRef(HybridManifest.data(), HybridManifest.size()), false);
    auto *Data = new ::llvm::GlobalVariable(Module, Bytes->getType(), true, ::llvm::GlobalValue::PrivateLinkage, Bytes, "ink.hybrid.manifest");
    HybridModule->setInitializer(::llvm::ConstantStruct::get(static_cast<::llvm::StructType *>(HybridModule->getValueType()), {::llvm::ConstantInt::get(Word, 1), Data, ::llvm::ConstantInt::get(SizeType, HybridManifest.size()), FunctionTable, ::llvm::ConstantInt::get(SizeType, Entries.size())}));
    if (Module.getNamedValue("ink_hybrid_module"))
    {
      return fail("ink_hybrid_module is reserved for the generated hybrid module getter");
    }
    auto *Getter = ::llvm::Function::Create(::llvm::FunctionType::get(PointerType, false), ::llvm::GlobalValue::ExternalLinkage, "ink_hybrid_module", Module);
    ::llvm::IRBuilder<> Builder(::llvm::BasicBlock::Create(Context, "entry", Getter));
    Builder.CreateRet(HybridModule);
    return true;
  }
} // namespace ink::backend::llvm
