#include "ink/execution/artifact/bytecode_builder.h"
#include "bytecode_internal.h"

#include "ink/execution/bytecode/execution_compiler.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <unordered_set>
#include <utility>

namespace ink::execution
{
  BytecodeArtifactResult buildBytecodeObject(std::string_view ModuleName, SemanticValueBridge &Bridge, std::span<const BytecodeFunctionInput> Functions, BytecodeLimits Limits)
  {
    if (ModuleName.empty() || Functions.empty())
    {
      return {BytecodeStatus::InvalidInput, "A bytecode object requires a module name and function metadata"};
    }
    if (Functions.size() > Limits.MaxRecords || ModuleName.size() > Limits.MaxStringBytes)
    {
      return {BytecodeStatus::LimitExceeded, "Bytecode object metadata exceeds its configured limit"};
    }
    struct ResolutionGuard
    {
        SemanticValueBridge &Bridge;
        bool Previous;

        ~ResolutionGuard()
        {
          Bridge.exchangeNativeImportResolution(Previous);
        }
    };
    ResolutionGuard Resolution{Bridge, Bridge.exchangeNativeImportResolution(false)};
    auto Artifact = std::make_unique<BytecodeArtifact>();
    Artifact->ModuleName = ModuleName;
    Artifact->Target = nativeBytecodeTarget();
    Artifact->Image.Layouts = Bridge.types();
    std::unordered_set<const ir::Function *> Seen;
    for (const BytecodeFunctionInput &Input : Functions)
    {
      if (!Input.Function || !Seen.insert(Input.Function).second)
      {
        return {BytecodeStatus::InvalidInput, "Each function requires exactly one symbol declaration"};
      }
      const ir::Function &Function = *Input.Function;
      if (!Function.context().compilationContext().targetContext().isNativeAbiCompatible())
      {
        return {BytecodeStatus::IncompatibleTarget, "Bytecode objects require the native target ABI"};
      }
      if (Function.callingConvention() != ir::CallingConvention::C)
      {
        return {BytecodeStatus::InvalidInput, "Bytecode objects support only the C calling convention"};
      }
      const bool Native = Function.isNativeImport();
      if (Native != (Input.Kind == BytecodeSymbolKind::Native) || (Input.Kind == BytecodeSymbolKind::Definition && !Function.hasBody()) || (Input.Kind == BytecodeSymbolKind::Native && Function.hasBody()))
      {
        return {BytecodeStatus::InvalidInput, "Symbol declaration does not match its function body or binding"};
      }
      const FunctionId Id = Bridge.lowerFunction(Function);
      const RuntimeFunctionDescriptor *Descriptor = Bridge.functionDescriptor(Id);
      if (Id == InvalidFunction || !Descriptor)
      {
        return {BytecodeStatus::InvalidInput, "Function belongs to another compilation context"};
      }
      BytecodeSymbol Symbol;
      Symbol.Function = Id;
      Symbol.Identity = Input.Identity;
      Symbol.Kind = Input.Kind;
      Symbol.Visibility = Input.Visibility;
      if (Symbol.Identity.Module.empty())
      {
        Symbol.Identity.Module = Native ? "C" : Artifact->ModuleName;
      }
      if (Symbol.Identity.Name.empty())
      {
        Symbol.Identity.Name = Function.context().namePool().text(Function.name());
      }
      RuntimeFunctionDescriptor Output = *Descriptor;
      Output.Exported = Function.isNativeExport() && Input.Kind == BytecodeSymbolKind::Definition;
      Artifact->Image.Descriptors.emplace(Id, std::move(Output));
      Artifact->Symbols.push_back(std::move(Symbol));
    }
    for (const BytecodeFunctionInput &Input : Functions)
    {
      if (Input.Kind != BytecodeSymbolKind::Definition)
      {
        continue;
      }
      ExecutionCompilationResult Compiled = ExecutionCompiler{}.compile(*Input.Function, Bridge);
      if (!Compiled)
      {
        return {BytecodeStatus::InvalidImage, "Could not lower a function into bytecode"};
      }
      const FunctionId Id = Compiled.Function->Id;
      Artifact->Image.Functions.emplace(Id, std::move(Compiled.Function));
    }
    std::vector<std::string> TypeIdentities;
    if (const BytecodeResult Result = artifact_detail::typeIdentities(*Artifact->Image.Layouts, Limits, TypeIdentities); !Result)
    {
      return {Result.Status, Result.Message};
    }
    for (BytecodeSymbol &Symbol : Artifact->Symbols)
    {
      const RuntimeTypeId Type = Artifact->Image.Descriptors.find(Symbol.Function)->second.Signature;
      const std::string &Signature = TypeIdentities[Type];
      if (!Symbol.Identity.Signature.empty() && Symbol.Identity.Signature != Signature)
      {
        return {BytecodeStatus::SignatureMismatch, "Symbol signature does not match its function type"};
      }
      Symbol.Identity.Signature = Signature;
    }
    // Freeze the type domain as well as code; later use of the compiler bridge must
    // not append types to an artifact already handed to a caller.
    auto Types = std::make_shared<RuntimeTypeTable>();
    for (std::size_t Index = 0; Index < Artifact->Image.Layouts->size(); ++Index)
    {
      Types->append(*Artifact->Image.Layouts->get(static_cast<RuntimeTypeId>(Index)));
    }
    Artifact->Image.Layouts = Types;
    for (auto &[Id, Function] : Artifact->Image.Functions)
    {
      Function->Layouts = Types;
    }
    const BytecodeResult Validated = validateBytecodeArtifact(*Artifact, Limits);
    if (!Validated)
    {
      return {Validated.Status, Validated.Message};
    }
    return {BytecodeStatus::Success, {}, std::move(Artifact)};
  }
} // namespace ink::execution
