#include "ink/execution/artifact/bytecode_builder.h"
#include "bytecode_internal.h"

#include "ink/execution/bytecode/execution_compiler.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/linkage.h"

#include <unordered_set>
#include <utility>

namespace ink::execution
{
  BytecodeArtifactResult buildBytecodeObject(std::string_view ModuleName, SemanticValueBridge &Bridge, std::span<const BytecodeFunctionInput> Functions, BytecodeLimits Limits)
  {
    if (ModuleName.empty())
    {
      return {BytecodeStatus::InvalidInput, "A bytecode object requires a module name"};
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
    std::vector<BytecodeFunctionInput> ReflectedFunctions(Functions.begin(), Functions.end());
    std::unordered_set<const ir::Function *> Registered;
    std::unordered_set<const ir::Module *> DefiningModules;
    const auto Owner = [](const ir::Function &Function) -> const ir::Module *
    {
      for (const ir::Value *Value = Function.outer(); Value; Value = Value->outer())
      {
        if (ir::Module::classof(Value))
        {
          return static_cast<const ir::Module *>(Value);
        }
      }
      return nullptr;
    };
    for (const auto &Input : Functions)
    {
      if (!Input.Function)
      {
        return {BytecodeStatus::InvalidInput, "Missing function declaration"};
      }
      Registered.insert(Input.Function);
      if (Input.Kind == BytecodeSymbolKind::Definition)
      {
        DefiningModules.insert(Owner(*Input.Function));
      }
      Bridge.lowerFunction(*Input.Function);
    }
    Bridge.retainReflectionTypes();
    const auto RegisterReflection = [&](FunctionId Id)
    {
      const ir::Function *Function = Bridge.sourceFunction(Id);
      if (!Function || Registered.contains(Function))
      {
        return;
      }
      const ir::Module *Module = Owner(*Function);
      const bool Local = DefiningModules.contains(Module);
      if (!Local && Function->visibility() == core::VisibilityKind::Private)
      {
        return;
      }
      Registered.insert(Function);
      BytecodeFunctionInput Input;
      Input.Function = Function;
      Input.Identity.Module = Local ? std::string(ModuleName) : Module ? std::string(Module->context().namePool().text(Module->name())) : std::string(ModuleName);
      Input.Identity.Name = Function->context().namePool().text(Function->name());
      Input.Kind = Function->isNativeImport() ? BytecodeSymbolKind::Native : Local ? BytecodeSymbolKind::Definition : BytecodeSymbolKind::Import;
      Input.Visibility = Function->visibility() == core::VisibilityKind::Private ? BytecodeVisibility::Private : BytecodeVisibility::Public;
      ReflectedFunctions.push_back(std::move(Input));
    };
    for (std::size_t Index = 0; Index < Bridge.types()->size(); ++Index)
    {
      const auto &Layout = *Bridge.types()->get(static_cast<RuntimeTypeId>(Index));
      if (Layout.Kind != RuntimeKind::Class)
      {
        continue;
      }
      for (const auto &Field : Layout.classDesc().Fields)
      {
        RegisterReflection(Field.Initializer);
      }
      for (const auto &Method : Layout.classDesc().Methods)
      {
        RegisterReflection(Method.Function);
      }
    }
    Functions = ReflectedFunctions;
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
      const auto *Function = Bridge.sourceFunction(Symbol.Function);
      if (Symbol.Kind == BytecodeSymbolKind::Native)
      {
        if (!Symbol.Identity.LinkName.empty() && Symbol.Identity.LinkName != Artifact->Image.Descriptors.at(Symbol.Function).Symbol)
        {
          return {BytecodeStatus::InvalidInput, "Native link name differs from its external symbol"};
        }
        Symbol.Identity.LinkName = Artifact->Image.Descriptors.at(Symbol.Function).Symbol;
        continue;
      }
      abi::ModuleIdentity Detached;
      Detached.Path = abi::modulePath(Symbol.Identity.Module);
      auto Linkage = Function ? ir::functionRecord(*Function, &Detached, Symbol.Identity.Name) : std::nullopt;
      if (!Linkage)
      {
        return {BytecodeStatus::InvalidInput, "Bytecode function has no supported Ink linkage identity"};
      }
      if (!Symbol.Identity.GenericArguments.empty())
      {
        auto F = abi::childRecords(*Linkage);
        auto D = F && F->size() == 3 && Linkage->Tag == 'F' ? abi::childRecords((*F)[0]) : std::nullopt;
        if (!D || D->size() != 7)
        {
          return {BytecodeStatus::InvalidInput, "Generic arguments require a function declaration"};
        }
        std::vector<abi::Record> Parameters;
        std::vector<abi::Record> Arguments;
        for (const auto &Argument : Symbol.Identity.GenericArguments)
        {
          const auto Found = std::find(TypeIdentities.begin(), TypeIdentities.end(), Argument.Type);
          const auto *Source = Found == TypeIdentities.end() ? nullptr : Bridge.sourceType(static_cast<RuntimeTypeId>(Found - TypeIdentities.begin()));
          const auto ValueType = Source ? ir::typeRecord(*Source) : std::nullopt;
          if (!ValueType)
          {
            return {BytecodeStatus::InvalidInput, "Generic argument has no canonical IR type"};
          }
          if (Argument.Kind == core::GenericArgumentKind::Type)
          {
            Parameters.push_back({'T', {}});
            Arguments.push_back(abi::record('T', {*ValueType}));
          }
          else
          {
            Parameters.push_back(abi::record('V', {*ValueType}));
            Arguments.push_back(abi::record('V', {*ValueType, {'B', Argument.Value}}));
          }
        }
        (*D)[5] = abi::record('G', Parameters);
        for (const auto &Input : Functions)
        {
          if (Input.Function == Function && Input.OverloadPattern)
          {
            (*D)[6] = *Input.OverloadPattern;
          }
        }
        (*F)[0] = abi::record('R', *D);
        (*F)[1] = abi::record('X', Arguments);
        Linkage = abi::record('F', *F);
      }
      const auto Mangled = abi::mangle(*Linkage);
      if (!Mangled || (!Symbol.Identity.LinkName.empty() && Symbol.Identity.LinkName != Mangled.Name))
      {
        return {BytecodeStatus::InvalidInput, "Bytecode link name does not match its canonical IR identity"};
      }
      Symbol.Identity.LinkName = Mangled.Name;
    }
    // Freeze the type domain as well as code; later use of the compiler bridge must
    // not append types to an artifact already handed to a caller.
    auto Types = std::make_shared<RuntimeTypeTable>();
    std::vector<TypeDesc> Layouts;
    Layouts.reserve(Artifact->Image.Layouts->size());
    for (std::size_t Index = 0; Index < Artifact->Image.Layouts->size(); ++Index)
    {
      TypeDesc Layout = *Artifact->Image.Layouts->get(static_cast<RuntimeTypeId>(Index));
      if (Layout.Kind == RuntimeKind::Class)
      {
        for (auto &Method : Layout.editClass().Methods)
        {
          if (Method.Visibility == core::VisibilityKind::Private && !Artifact->Image.Descriptors.contains(Method.Function))
          {
            Method.Function = InvalidFunction;
          }
        }
      }
      Layouts.push_back(std::move(Layout));
    }
    if (!Types->defineAll(std::move(Layouts), Limits.MaxTypeDepth))
    {
      return {BytecodeStatus::InvalidImage, "Could not freeze bytecode aggregate layouts"};
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
