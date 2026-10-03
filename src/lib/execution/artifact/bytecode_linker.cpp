#include "ink/execution/artifact/bytecode_linker.h"

#include "bytecode_internal.h"

#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace ink::execution
{
  namespace
  {
    struct Definition
    {
        std::size_t Object;
        const BytecodeSymbol *Symbol;
        FunctionId Linked;
    };

    struct ObjectMapping
    {
        std::vector<RuntimeTypeId> Types;
        std::unordered_map<FunctionId, FunctionId> Functions;
    };

    struct DefinitionGroup
    {
        std::optional<Definition> Shared;
        std::unordered_map<std::size_t, Definition> Private;
    };

    struct TypeOrigin
    {
        std::size_t Object;
        RuntimeTypeId Type;
    };

    bool visible(const Definition &Candidate, std::size_t Importer, std::span<const BytecodeArtifact *const> Objects)
    {
      switch (Candidate.Symbol->Visibility)
      {
      case BytecodeVisibility::Private:
        return Candidate.Object == Importer;
      case BytecodeVisibility::Module:
        return Objects[Candidate.Object]->ModuleName == Objects[Importer]->ModuleName;
      case BytecodeVisibility::Public:
        return true;
      }
      return false;
    }

    bool sameDeclaration(const BytecodeSymbolIdentity &Left, const BytecodeSymbolIdentity &Right)
    {
      return Left.Module == Right.Module && Left.Name == Right.Name && Left.GenericArguments == Right.GenericArguments;
    }

    BytecodeArtifactResult failed(BytecodeStatus Status, std::string Message)
    {
      return {Status, std::move(Message), nullptr};
    }

    std::string symbolName(const BytecodeSymbolIdentity &Identity)
    {
      return Identity.Module + "::" + Identity.Name;
    }

    RuntimeValue remapValue(const RuntimeValue &Value, const ObjectMapping &Mapping, const RuntimeTypeTable &Source)
    {
      const RuntimeTypeId Type = Mapping.Types[Value.Type];
      if (Value.Initialized && Value.kind() == RuntimeKind::Array)
      {
        std::vector<RuntimeValue> Elements;
        Elements.reserve(Value.array().size());
        for (const RuntimeValue &Element : Value.array())
        {
          Elements.push_back(remapValue(Element, Mapping, Source));
        }
        return RuntimeValue::fromArray(std::move(Elements), Type);
      }
      RuntimeValue Result = Value;
      if (Value.Initialized && Source.get(Value.Type)->Kind == RuntimeKind::Function)
      {
        Result.Bits = Mapping.Functions.find(static_cast<FunctionId>(Value.Bits))->second;
      }
      Result.Type = Type;
      return Result;
    }

    std::unique_ptr<ExecutableFunction> copyFunction(const ExecutableFunction &Source, const ObjectMapping &Mapping, const std::shared_ptr<const RuntimeTypeTable> &Types)
    {
      auto Function = std::make_unique<ExecutableFunction>(Source);
      Function->Id = Mapping.Functions.find(Source.Id)->second;
      Function->Signature = Mapping.Types[Source.Signature];
      Function->Layouts = Types;
      for (RuntimeTypeId &Type : Function->SlotTypes)
      {
        Type = Mapping.Types[Type];
      }
      for (RuntimeValue &Value : Function->InitialSlots)
      {
        Value = remapValue(Value, Mapping, *Source.Layouts);
      }
      for (ExecutionCallSite &Call : Function->Calls)
      {
        Call.Signature = Mapping.Types[Call.Signature];
        if (Call.Target != InvalidFunction)
        {
          Call.Target = Mapping.Functions.find(Call.Target)->second;
        }
      }
      for (BytecodeInstruction &Instruction : Function->Code)
      {
        const auto &Metadata = *bytecodeInstructionMetadata(Instruction.Code);
        for (std::size_t Index = 0; Index < Instruction.Operands.size(); ++Index)
        {
          if (Metadata.Operands[Index] == BytecodeOperandKind::Layout)
          {
            Instruction.Operands[Index] = Mapping.Types[Instruction.Operands[Index]];
          }
        }
      }
      return Function;
    }
  } // namespace

  BytecodeArtifactResult linkBytecodeArtifacts(std::span<const BytecodeArtifact *const> Objects, const BytecodeSymbolIdentity *Entry, BytecodeLimits Limits)
  {
    if (Objects.empty())
    {
      return failed(BytecodeStatus::InvalidInput, "Linking requires at least one bytecode object");
    }
    artifact_detail::Budget Usage(Limits);
    if (!Usage.records(Objects.size(), sizeof(ObjectMapping)))
    {
      return failed(BytecodeStatus::LimitExceeded, "Bytecode object count exceeds the configured limit");
    }
    if (Entry)
    {
      if (!Usage.string(Entry->Module) || !Usage.string(Entry->Name) || !Usage.string(Entry->Signature) || !Usage.records(Entry->GenericArguments.size(), sizeof(BytecodeGenericArgument)))
      {
        return failed(BytecodeStatus::LimitExceeded, "Bytecode entry identity exceeds the configured limit");
      }
      for (const BytecodeGenericArgument &Argument : Entry->GenericArguments)
      {
        if (!Usage.string(Argument.Type) || !Usage.string(Argument.Value))
        {
          return failed(BytecodeStatus::LimitExceeded, "Bytecode entry generic arguments exceed the configured limit");
        }
      }
    }
    for (const BytecodeArtifact *Object : Objects)
    {
      if (!Object || Object->Kind != BytecodeArtifactKind::Object)
      {
        return failed(BytecodeStatus::InvalidInput, "Only relocatable bytecode objects can be linked");
      }
      if (const auto Result = validateBytecodeArtifact(*Object, Limits); !Result)
      {
        return failed(Result.Status, Result.Message);
      }
      if (const auto Result = artifact_detail::accountArtifact(*Object, Usage); !Result)
      {
        return failed(Result.Status, Result.Message);
      }
    }
    std::unordered_map<std::string_view, Definition> NativeExports;
    for (std::size_t ObjectIndex = 0; ObjectIndex < Objects.size(); ++ObjectIndex)
    {
      const BytecodeArtifact &Object = *Objects[ObjectIndex];
      for (const BytecodeSymbol &Symbol : Object.Symbols)
      {
        const RuntimeFunctionDescriptor &Descriptor = Object.Image.Descriptors.find(Symbol.Function)->second;
        if (!Descriptor.Exported)
        {
          continue;
        }
        if (!Usage.allocation(1, sizeof(std::string_view) + sizeof(Definition) + sizeof(void *) * 5))
        {
          return failed(BytecodeStatus::LimitExceeded, "Native export index exceeds the allocation limit");
        }
        if (!NativeExports.emplace(Descriptor.Symbol, Definition{ObjectIndex, &Symbol, InvalidFunction}).second)
        {
          return failed(BytecodeStatus::DuplicateSymbol, "Multiple native exports of " + Descriptor.Symbol);
        }
      }
    }
    auto Linked = std::make_unique<BytecodeArtifact>();
    Linked->Kind = BytecodeArtifactKind::Executable;
    Linked->ModuleName = "<linked>";
    Linked->Target = Objects.front()->Target;
    std::vector<ObjectMapping> Mappings(Objects.size());
    std::vector<TypeOrigin> Origins;
    std::unordered_map<std::string, RuntimeTypeId> TypeIds;
    for (std::size_t ObjectIndex = 0; ObjectIndex < Objects.size(); ++ObjectIndex)
    {
      const BytecodeArtifact &Object = *Objects[ObjectIndex];
      std::vector<std::string> Identities;
      if (const auto Result = artifact_detail::typeIdentities(*Object.Image.Layouts, Limits, Identities, &Usage); !Result)
      {
        return failed(Result.Status, Result.Message);
      }
      auto &Mapping = Mappings[ObjectIndex];
      if (!Usage.allocation(Identities.size(), sizeof(RuntimeTypeId) + sizeof(TypeOrigin) + sizeof(StorageLayout) + sizeof(std::string) + sizeof(void *) * 6) || !Usage.allocation(Object.Symbols.size(), sizeof(Definition) * 2 + sizeof(RuntimeFunctionDescriptor) + sizeof(BytecodeSymbol) + sizeof(void *) * 12))
      {
        return failed(BytecodeStatus::LimitExceeded, "Linked bytecode indices exceed the allocation limit");
      }
      Mapping.Types.resize(Identities.size());
      for (std::size_t Index = 0; Index < Identities.size(); ++Index)
      {
        const auto Found = TypeIds.find(Identities[Index]);
        if (Found != TypeIds.end())
        {
          Mapping.Types[Index] = Found->second;
          continue;
        }
        if (Origins.size() >= InvalidRuntimeType || !Usage.string(Identities[Index]))
        {
          return failed(BytecodeStatus::LimitExceeded, "Linked bytecode types exceed the configured limit");
        }
        const RuntimeTypeId Id = static_cast<RuntimeTypeId>(Origins.size());
        Mapping.Types[Index] = Id;
        Origins.push_back({ObjectIndex, static_cast<RuntimeTypeId>(Index)});
        TypeIds.emplace(std::move(Identities[Index]), Id);
      }
    }
    // Assign all final IDs before materializing layouts, including forward references.
    auto Types = std::make_shared<RuntimeTypeTable>();
    for (const TypeOrigin &Origin : Origins)
    {
      StorageLayout Layout = *Objects[Origin.Object]->Image.Layouts->get(Origin.Type);
      const ObjectMapping &Mapping = Mappings[Origin.Object];
      if (Layout.Kind == RuntimeKind::Pointer)
      {
        Layout.Pointee = Mapping.Types[Layout.Pointee];
      }
      if (Layout.Kind == RuntimeKind::Array)
      {
        Layout.ElementType = Mapping.Types[Layout.ElementType];
        Layout.ElementLayout.reset();
      }
      if (Layout.Kind == RuntimeKind::Function)
      {
        Layout.ReturnType = Mapping.Types[Layout.ReturnType];
        for (RuntimeTypeId &Parameter : Layout.Parameters)
        {
          Parameter = Mapping.Types[Parameter];
        }
      }
      if (Types->append(std::move(Layout)) == InvalidRuntimeType)
      {
        return failed(BytecodeStatus::LimitExceeded, "Linked bytecode has too many runtime types");
      }
    }
    Linked->Image.Layouts = Types;
    std::unordered_map<std::string, DefinitionGroup> Definitions;
    std::vector<Definition> AllDefinitions;
    for (std::size_t ObjectIndex = 0; ObjectIndex < Objects.size(); ++ObjectIndex)
    {
      const BytecodeArtifact &Object = *Objects[ObjectIndex];
      for (const BytecodeSymbol &Symbol : Object.Symbols)
      {
        if (Symbol.Kind == BytecodeSymbolKind::Import || (Symbol.Kind == BytecodeSymbolKind::Native && NativeExports.contains(Object.Image.Descriptors.find(Symbol.Function)->second.Symbol)))
        {
          continue;
        }
        if (Linked->Symbols.size() >= InvalidFunction)
        {
          return failed(BytecodeStatus::LimitExceeded, "Linked bytecode has too many functions");
        }
        const FunctionId Id = static_cast<FunctionId>(Linked->Symbols.size());
        if (Symbol.Kind == BytecodeSymbolKind::Definition)
        {
          if (!artifact_detail::accountSymbolKey(Symbol.Identity, Usage))
          {
            return failed(BytecodeStatus::LimitExceeded, "Linked symbol identities exceed the allocation limit");
          }
          const std::string Key = bytecodeSymbolKey(Symbol.Identity);
          auto &Candidates = Definitions[Key];
          const Definition Defined{ObjectIndex, &Symbol, Id};
          if (Symbol.Visibility == BytecodeVisibility::Private)
          {
            Candidates.Private.emplace(ObjectIndex, Defined);
          }
          else
          {
            if (Candidates.Shared)
            {
              return failed(BytecodeStatus::DuplicateSymbol, "Multiple definitions of " + symbolName(Symbol.Identity));
            }
            Candidates.Shared = Defined;
          }
          AllDefinitions.push_back(Defined);
        }
        BytecodeSymbol Output = Symbol;
        Output.Function = Id;
        Linked->Symbols.push_back(std::move(Output));
        Mappings[ObjectIndex].Functions.emplace(Symbol.Function, Id);
        RuntimeFunctionDescriptor Descriptor = Object.Image.Descriptors.find(Symbol.Function)->second;
        Descriptor.Id = Id;
        Descriptor.Signature = Mappings[ObjectIndex].Types[Descriptor.Signature];
        Linked->Image.Descriptors.emplace(Id, std::move(Descriptor));
      }
    }
    for (std::size_t ObjectIndex = 0; ObjectIndex < Objects.size(); ++ObjectIndex)
    {
      for (const BytecodeSymbol &Symbol : Objects[ObjectIndex]->Symbols)
      {
        if (Symbol.Kind == BytecodeSymbolKind::Native)
        {
          const RuntimeFunctionDescriptor &Imported = Objects[ObjectIndex]->Image.Descriptors.find(Symbol.Function)->second;
          const auto Native = NativeExports.find(Imported.Symbol);
          if (Native == NativeExports.end())
          {
            continue;
          }
          const Definition &Defined = Native->second;
          const FunctionId Target = Mappings[Defined.Object].Functions.find(Defined.Symbol->Function)->second;
          const RuntimeFunctionDescriptor &Descriptor = Linked->Image.Descriptors.find(Target)->second;
          if (!Imported.CAbi || !Descriptor.CAbi || Mappings[ObjectIndex].Types[Imported.Signature] != Descriptor.Signature)
          {
            return failed(BytecodeStatus::SignatureMismatch, "Native import signature does not match the export of " + Imported.Symbol);
          }
          Mappings[ObjectIndex].Functions.emplace(Symbol.Function, Target);
          continue;
        }
        if (Symbol.Kind != BytecodeSymbolKind::Import)
        {
          continue;
        }
        if (!artifact_detail::accountSymbolKey(Symbol.Identity, Usage))
        {
          return failed(BytecodeStatus::LimitExceeded, "Import identities exceed the allocation limit");
        }
        const auto Found = Definitions.find(bytecodeSymbolKey(Symbol.Identity));
        const Definition *Resolved = nullptr;
        if (Found != Definitions.end())
        {
          const DefinitionGroup &Candidates = Found->second;
          if (const auto Local = Candidates.Private.find(ObjectIndex); Local != Candidates.Private.end())
          {
            Resolved = &Local->second;
          }
          else if (Candidates.Shared && visible(*Candidates.Shared, ObjectIndex, Objects))
          {
            Resolved = &*Candidates.Shared;
          }
        }
        if (!Resolved)
        {
          if (Found != Definitions.end())
          {
            return failed(BytecodeStatus::InvisibleSymbol, "Definition is not visible to module " + Objects[ObjectIndex]->ModuleName + ": " + symbolName(Symbol.Identity));
          }
          for (const Definition &Candidate : AllDefinitions)
          {
            if (visible(Candidate, ObjectIndex, Objects) && sameDeclaration(Candidate.Symbol->Identity, Symbol.Identity))
            {
              return failed(BytecodeStatus::SignatureMismatch, "Import signature does not match the definition of " + symbolName(Symbol.Identity));
            }
          }
          return failed(BytecodeStatus::MissingSymbol, "Unresolved bytecode import " + symbolName(Symbol.Identity));
        }
        const RuntimeFunctionDescriptor &Imported = Objects[ObjectIndex]->Image.Descriptors.find(Symbol.Function)->second;
        const RuntimeFunctionDescriptor &Defined = Linked->Image.Descriptors.find(Resolved->Linked)->second;
        if (Imported.CAbi != Defined.CAbi)
        {
          return failed(BytecodeStatus::SignatureMismatch, "Import ABI does not match the definition of " + symbolName(Symbol.Identity));
        }
        Mappings[ObjectIndex].Functions.emplace(Symbol.Function, Resolved->Linked);
      }
    }
    if (Entry)
    {
      if (!artifact_detail::accountSymbolKey(*Entry, Usage))
      {
        return failed(BytecodeStatus::LimitExceeded, "Entry identity exceeds the allocation limit");
      }
      const auto Found = Definitions.find(bytecodeSymbolKey(*Entry));
      if (Found != Definitions.end() && Found->second.Shared && Found->second.Shared->Symbol->Visibility == BytecodeVisibility::Public)
      {
        Linked->Entry = Found->second.Shared->Linked;
      }
      if (Linked->Entry == InvalidFunction)
      {
        return failed(Found == Definitions.end() ? BytecodeStatus::MissingSymbol : BytecodeStatus::InvisibleSymbol, "Bytecode entry must resolve to a public definition: " + symbolName(*Entry));
      }
    }
    for (std::size_t ObjectIndex = 0; ObjectIndex < Objects.size(); ++ObjectIndex)
    {
      for (const auto &[Id, Source] : Objects[ObjectIndex]->Image.Functions)
      {
        auto Function = copyFunction(*Source, Mappings[ObjectIndex], Types);
        const FunctionId LinkedId = Function->Id;
        Linked->Image.Functions.emplace(LinkedId, std::move(Function));
      }
    }
    if (const auto Result = validateBytecodeArtifact(*Linked, Limits); !Result)
    {
      return failed(Result.Status, Result.Message);
    }
    return {BytecodeStatus::Success, {}, std::move(Linked)};
  }
} // namespace ink::execution
