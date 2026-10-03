#include "bytecode_internal.h"

#include "ink/execution/bytecode/execution_compiler.h"

#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace ink::execution
{
  namespace
  {
    bool validName(std::string_view Name)
    {
      return !Name.empty() && Name.find('\0') == std::string_view::npos;
    }

    bool nativeInteger(std::uint32_t Width)
    {
      return Width == 8 || Width == 16 || Width == 32 || Width == 64;
    }

    std::size_t integerAlignment(std::uint32_t Width)
    {
      switch (Width)
      {
      case 8:
        return alignof(std::uint8_t);
      case 16:
        return alignof(std::uint16_t);
      case 32:
        return alignof(std::uint32_t);
      case 64:
        return alignof(std::uint64_t);
      default:
        return 1;
      }
    }

    bool validLayout(const StorageLayout &Layout, const RuntimeTypeTable &Types)
    {
      if (Layout.Kind != RuntimeKind::Array && (Layout.ElementType != InvalidRuntimeType || Layout.ElementCount || Layout.ElementLayout))
      {
        return false;
      }
      if (Layout.Domain != Types.domain() || (Layout.Kind != RuntimeKind::Integer && Layout.Signed) || (Layout.Kind != RuntimeKind::Pointer && (Layout.Pointee != InvalidRuntimeType || Layout.Writable)) || (Layout.Kind != RuntimeKind::Function && (Layout.ReturnType != InvalidRuntimeType || !Layout.Parameters.empty())))
      {
        return false;
      }
      switch (Layout.Kind)
      {
      case RuntimeKind::Void:
        return Layout.BitWidth == 0 && Layout.Size == 0 && Layout.Alignment == 1 && !Layout.Native;
      case RuntimeKind::Boolean:
        return Layout.BitWidth == 1 && Layout.Size == sizeof(bool) && Layout.Alignment == alignof(bool) && Layout.Native;
      case RuntimeKind::Integer:
        return Layout.BitWidth != 0 && Layout.Size == (static_cast<std::uint64_t>(Layout.BitWidth) + 7) / 8 && Layout.Alignment == integerAlignment(Layout.BitWidth) && Layout.Native == nativeInteger(Layout.BitWidth);
      case RuntimeKind::Float:
        return (Layout.BitWidth == 16 || Layout.BitWidth == 32 || Layout.BitWidth == 64) && Layout.Size == Layout.BitWidth / 8 && Layout.Alignment == (Layout.BitWidth == 32 ? alignof(float) : Layout.BitWidth == 64 ? alignof(double) : 1) && Layout.Native == (Layout.BitWidth != 16);
      case RuntimeKind::String:
        return Layout.BitWidth == 0 && Layout.Size == sizeof(void *) * 2 && Layout.Alignment == sizeof(void *) && !Layout.Native;
      case RuntimeKind::Array:
      {
        const StorageLayout *Element = Types.get(Layout.ElementType);
        return Element && Layout.ElementType < Layout.Type && Element->Kind != RuntimeKind::Void && Element->Kind != RuntimeKind::Invalid && Layout.ElementLayout && Layout.ElementLayout->Domain == Types.domain() && Layout.ElementLayout->Type == Layout.ElementType && Layout.BitWidth == 0 && (!Element->Size || Layout.ElementCount <= std::numeric_limits<std::size_t>::max() / Element->Size) && Layout.Size == Element->Size * Layout.ElementCount && Layout.Alignment == Element->Alignment && Layout.Native == Element->Native;
      }
      case RuntimeKind::Pointer:
        return Layout.BitWidth == 0 && Layout.Size == sizeof(void *) && Layout.Alignment == sizeof(void *) && !Layout.Native && Types.get(Layout.Pointee);
      case RuntimeKind::Function:
        if (Layout.BitWidth != 0 || Layout.Size != sizeof(FunctionId) || Layout.Alignment != alignof(FunctionId) || Layout.Native || !Types.get(Layout.ReturnType))
        {
          return false;
        }
        for (RuntimeTypeId Parameter : Layout.Parameters)
        {
          const auto *Type = Types.get(Parameter);
          if (!Type || Type->Kind == RuntimeKind::Void)
          {
            return false;
          }
        }
        return true;
      case RuntimeKind::Invalid:
        return false;
      }
      return false;
    }

    bool nativeType(const StorageLayout &Layout, bool Return)
    {
      return (Return && Layout.Kind == RuntimeKind::Void) || Layout.Kind == RuntimeKind::Boolean || (Layout.Kind == RuntimeKind::Integer && nativeInteger(Layout.BitWidth)) || (Layout.Kind == RuntimeKind::Float && (Layout.BitWidth == 32 || Layout.BitWidth == 64)) || Layout.Kind == RuntimeKind::Pointer;
    }

    bool nativeSignature(const StorageLayout &Signature, const RuntimeTypeTable &Types)
    {
      if (!nativeType(*Types.get(Signature.ReturnType), true) || Signature.Parameters.size() > std::numeric_limits<unsigned int>::max())
      {
        return false;
      }
      for (RuntimeTypeId Parameter : Signature.Parameters)
      {
        if (!nativeType(*Types.get(Parameter), false))
        {
          return false;
        }
      }
      return true;
    }

    int hexDigit(char Character)
    {
      return Character >= '0' && Character <= '9' ? Character - '0' : Character >= 'a' && Character <= 'f' ? Character - 'a' + 10 : -1;
    }

    bool genericConstant(const BytecodeGenericArgument &Argument, const StorageLayout &Type)
    {
      if (Type.Kind == RuntimeKind::Boolean)
      {
        return Argument.Value == "0" || Argument.Value == "1";
      }
      if (Type.Kind != RuntimeKind::Integer && Type.Kind != RuntimeKind::Float && Type.Kind != RuntimeKind::String)
      {
        return false;
      }
      if (Type.Kind == RuntimeKind::String ? Argument.Value.size() % 2 != 0 : Argument.Value.size() != (static_cast<std::uint64_t>(Type.BitWidth) + 3) / 4)
      {
        return false;
      }
      for (char Character : Argument.Value)
      {
        if (hexDigit(Character) < 0)
        {
          return false;
        }
      }
      return Type.Kind == RuntimeKind::String || Type.BitWidth % 4 == 0 || static_cast<unsigned>(hexDigit(Argument.Value.front())) < (1U << (Type.BitWidth % 4));
    }

    BytecodeResult validateFunction(const ExecutableFunction &Function, const BytecodeArtifact &Artifact)
    {
      if (Function.Layouts != Artifact.Image.Layouts || ExecutionCompiler{}.verify(Function) != ExecutionStatus::Success)
      {
        return {BytecodeStatus::InvalidImage, "Bytecode function failed instruction or frame validation"};
      }
      const RuntimeTypeTable &Types = *Artifact.Image.Layouts;
      std::vector<const RuntimeValue *> Pending;
      for (const RuntimeValue &Value : Function.InitialSlots)
      {
        Pending.push_back(&Value);
      }
      while (!Pending.empty())
      {
        const RuntimeValue &Value = *Pending.back();
        Pending.pop_back();
        if (!Value.Initialized)
        {
          continue;
        }
        const auto &Type = *Types.get(Value.Type);
        if (Value.Object && Value.Bits != 0)
        {
          return {BytecodeStatus::InvalidImage, "Bytecode constant payload contains noncanonical scalar bits"};
        }
        if (Type.Kind == RuntimeKind::Pointer && Value.pointer().kind() != ExecutionPointer::Kind::Null)
        {
          return {BytecodeStatus::UnsupportedConstant, "Only null pointer constants can be stored in bytecode artifacts"};
        }
        if (Type.Kind == RuntimeKind::Function)
        {
          const auto Found = Artifact.Image.Descriptors.find(static_cast<FunctionId>(Value.Bits));
          if (Found == Artifact.Image.Descriptors.end() || Found->second.Signature != Value.Type)
          {
            return {BytecodeStatus::InvalidImage, "Bytecode function constant refers to a missing or incompatible function"};
          }
        }
        if (Type.Kind == RuntimeKind::Array)
        {
          for (const RuntimeValue &Element : Value.array())
          {
            Pending.push_back(&Element);
          }
        }
      }
      // Call records are validated even when no instruction happens to reference them.
      for (const ExecutionCallSite &Call : Function.Calls)
      {
        const StorageLayout *Signature = Types.get(Call.Signature);
        if (!Signature || Signature->Kind != RuntimeKind::Function || Call.Arguments.size() != Signature->Parameters.size())
        {
          return {BytecodeStatus::InvalidImage, "Bytecode call record has an invalid signature"};
        }
        for (std::size_t Index = 0; Index < Call.Arguments.size(); ++Index)
        {
          if (Call.Arguments[Index] >= Function.SlotTypes.size() || Function.SlotTypes[Call.Arguments[Index]] != Signature->Parameters[Index])
          {
            return {BytecodeStatus::InvalidImage, "Bytecode call record has an invalid argument slot"};
          }
        }
        if (Call.Target == InvalidFunction)
        {
          if (Call.CalleeSlot >= Function.SlotTypes.size() || Function.SlotTypes[Call.CalleeSlot] != Call.Signature)
          {
            return {BytecodeStatus::InvalidImage, "Bytecode indirect call record has an invalid callee slot"};
          }
        }
        else
        {
          const auto Found = Artifact.Image.Descriptors.find(Call.Target);
          if (Call.CalleeSlot != InvalidSlot || Found == Artifact.Image.Descriptors.end() || Found->second.Signature != Call.Signature)
          {
            return {BytecodeStatus::InvalidImage, "Bytecode direct call record refers to a missing or incompatible function"};
          }
        }
      }
      return {};
    }
  } // namespace

  namespace artifact_detail
  {
    BytecodeResult accountArtifact(const BytecodeArtifact &Artifact, Budget &Usage)
    {
      if (!Usage.string(Artifact.ModuleName) || !Usage.string(Artifact.Target) || !Usage.records(Artifact.Symbols.size(), sizeof(BytecodeSymbol)) || !Usage.records(Artifact.Image.Descriptors.size(), sizeof(RuntimeFunctionDescriptor)) || !Usage.records(Artifact.Image.Functions.size(), sizeof(ExecutableFunction)))
      {
        return {BytecodeStatus::LimitExceeded, "Bytecode artifact exceeds the configured storage limits"};
      }
      if (Artifact.Image.Layouts)
      {
        const auto &Types = *Artifact.Image.Layouts;
        if (!Usage.records(Types.size(), sizeof(StorageLayout)))
        {
          return {BytecodeStatus::LimitExceeded, "Bytecode type table exceeds the configured storage limits"};
        }
        for (std::size_t Index = 0; Index < Types.size(); ++Index)
        {
          if (!Usage.records(Types.get(static_cast<RuntimeTypeId>(Index))->Parameters.size(), sizeof(RuntimeTypeId)))
          {
            return {BytecodeStatus::LimitExceeded, "Bytecode signature parameters exceed the configured storage limits"};
          }
        }
      }
      for (const auto &[Id, Descriptor] : Artifact.Image.Descriptors)
      {
        if (!Usage.string(Descriptor.Symbol))
        {
          return {BytecodeStatus::LimitExceeded, "Bytecode native symbol names exceed the string limit"};
        }
      }
      for (const BytecodeSymbol &Symbol : Artifact.Symbols)
      {
        if (!Usage.string(Symbol.Identity.Module) || !Usage.string(Symbol.Identity.Name) || !Usage.string(Symbol.Identity.Signature) || !Usage.records(Symbol.Identity.GenericArguments.size(), sizeof(BytecodeGenericArgument)))
        {
          return {BytecodeStatus::LimitExceeded, "Bytecode symbols exceed the configured storage limits"};
        }
        for (const BytecodeGenericArgument &Argument : Symbol.Identity.GenericArguments)
        {
          if (!Usage.string(Argument.Type) || !Usage.string(Argument.Value))
          {
            return {BytecodeStatus::LimitExceeded, "Bytecode generic arguments exceed the string limit"};
          }
        }
      }
      for (const auto &[Id, Function] : Artifact.Image.Functions)
      {
        if (!Function)
        {
          return {BytecodeStatus::InvalidImage, "Bytecode function table contains an empty body"};
        }
        if (!Usage.records(Function->Code.size(), sizeof(BytecodeInstruction)) || !Usage.records(Function->SlotTypes.size(), sizeof(RuntimeTypeId)) || !Usage.records(Function->InitialSlots.size(), sizeof(RuntimeValue)) || !Usage.records(Function->Calls.size(), sizeof(ExecutionCallSite)) || !Usage.bytes(Function->ConstantData.size()))
        {
          return {BytecodeStatus::LimitExceeded, "Bytecode function exceeds the configured storage limits"};
        }
        for (const ExecutionCallSite &Call : Function->Calls)
        {
          if (!Usage.records(Call.Arguments.size(), sizeof(SlotId)))
          {
            return {BytecodeStatus::LimitExceeded, "Bytecode call arguments exceed the configured storage limits"};
          }
        }
        for (const RuntimeValue &Value : Function->InitialSlots)
        {
          std::vector<const RuntimeValue *> Pending = {&Value};
          while (!Pending.empty())
          {
            const RuntimeValue &Current = *Pending.back();
            Pending.pop_back();
            if (Current.kind() == RuntimeKind::String && !Usage.string(Current.string()))
            {
              return {BytecodeStatus::LimitExceeded, "Bytecode string constants exceed the configured storage limits"};
            }
            if (Current.kind() == RuntimeKind::Integer && !Usage.records((static_cast<std::uint64_t>(Current.integer().bitWidth()) + 63) / 64, sizeof(std::uint64_t)))
            {
              return {BytecodeStatus::LimitExceeded, "Bytecode integer constants exceed the configured storage limits"};
            }
            if (Current.kind() == RuntimeKind::Array)
            {
              if (!Usage.records(Current.array().size(), sizeof(RuntimeValue) + sizeof(RuntimeValue *)))
              {
                return {BytecodeStatus::LimitExceeded, "Bytecode array constants exceed the configured storage limits"};
              }
              for (const RuntimeValue &Element : Current.array())
              {
                Pending.push_back(&Element);
              }
            }
          }
        }
      }
      return {};
    }
  } // namespace artifact_detail

  BytecodeResult validateBytecodeArtifact(const BytecodeArtifact &Artifact, BytecodeLimits Limits)
  {
    artifact_detail::Budget Usage(Limits);
    if (const auto Result = artifact_detail::accountArtifact(Artifact, Usage); !Result)
    {
      return Result;
    }
    if (Artifact.Kind != BytecodeArtifactKind::Object && Artifact.Kind != BytecodeArtifactKind::Executable)
    {
      return {BytecodeStatus::InvalidInput, "Unknown bytecode artifact kind"};
    }
    const std::string NativeTarget = nativeBytecodeTarget();
    if (NativeTarget.empty() || Artifact.Target != NativeTarget)
    {
      return {BytecodeStatus::IncompatibleTarget, "Bytecode target does not match this runtime"};
    }
    if (!validName(Artifact.ModuleName) || !Artifact.Image.Layouts || Artifact.Symbols.size() != Artifact.Image.Descriptors.size() || (Artifact.Kind == BytecodeArtifactKind::Object && Artifact.Entry != InvalidFunction))
    {
      return {BytecodeStatus::InvalidImage, "Bytecode artifact has invalid module, entry, type table or symbol counts"};
    }
    const RuntimeTypeTable &Types = *Artifact.Image.Layouts;
    for (std::size_t Index = 0; Index < Types.size(); ++Index)
    {
      const StorageLayout &Layout = *Types.get(static_cast<RuntimeTypeId>(Index));
      if (Layout.Type != Index || !validLayout(Layout, Types))
      {
        return {BytecodeStatus::InvalidImage, "Bytecode layout does not match its runtime kind or native target"};
      }
      if (Layout.Size > Limits.MaxAllocationBytes)
      {
        return {BytecodeStatus::LimitExceeded, "Bytecode layout exceeds the allocation limit"};
      }
    }
    std::vector<std::string> Identities;
    if (const auto Result = artifact_detail::typeIdentities(Types, Limits, Identities, &Usage); !Result)
    {
      return Result;
    }
    std::unordered_map<std::string, RuntimeTypeId> IdentityTypes;
    if (!Usage.allocation(Identities.size(), sizeof(std::string) + sizeof(RuntimeTypeId) + sizeof(void *) * 6) || !Usage.allocation(Artifact.Symbols.size(), sizeof(FunctionId) + sizeof(BytecodeSymbol *) + sizeof(std::string) + sizeof(std::string_view) + sizeof(void *) * 15))
    {
      return {BytecodeStatus::LimitExceeded, "Bytecode validation indices exceed the allocation limit"};
    }
    for (std::size_t Index = 0; Index < Identities.size(); ++Index)
    {
      if (!Usage.allocation(Identities[Index].size()))
      {
        return {BytecodeStatus::LimitExceeded, "Bytecode validation identities exceed the allocation limit"};
      }
      IdentityTypes.emplace(Identities[Index], static_cast<RuntimeTypeId>(Index));
    }
    std::unordered_map<FunctionId, const BytecodeSymbol *> Symbols;
    std::unordered_set<std::string> Definitions;
    std::unordered_set<std::string_view> NativeExports;
    for (const BytecodeSymbol &Symbol : Artifact.Symbols)
    {
      const auto Found = Artifact.Image.Descriptors.find(Symbol.Function);
      if (Symbol.Function == InvalidFunction || Found == Artifact.Image.Descriptors.end() || Found->second.Id != Symbol.Function || !Symbols.emplace(Symbol.Function, &Symbol).second || Symbol.Kind > BytecodeSymbolKind::Native || Symbol.Visibility > BytecodeVisibility::Public || !validName(Symbol.Identity.Module) || !validName(Symbol.Identity.Name))
      {
        return {BytecodeStatus::InvalidImage, "Bytecode symbol has an invalid identity or descriptor"};
      }
      const RuntimeFunctionDescriptor &Descriptor = Found->second;
      const StorageLayout *Signature = Types.get(Descriptor.Signature);
      if (!Signature || Signature->Kind != RuntimeKind::Function || Symbol.Identity.Signature != Identities[Descriptor.Signature])
      {
        return {BytecodeStatus::SignatureMismatch, "Bytecode symbol signature differs from its function descriptor"};
      }
      const auto Body = Artifact.Image.Functions.find(Symbol.Function);
      if ((Symbol.Kind == BytecodeSymbolKind::Definition) != (Body != Artifact.Image.Functions.end()) || (Symbol.Kind == BytecodeSymbolKind::Native) != Descriptor.External || (!Descriptor.CAbi && (Descriptor.External || Descriptor.Exported || Descriptor.Supported)) || (Descriptor.Exported && Symbol.Kind != BytecodeSymbolKind::Definition))
      {
        return {BytecodeStatus::InvalidImage, "Bytecode symbol kind does not match its body or native flags"};
      }
      if (Descriptor.CAbi && (!Descriptor.NativeAbi || !Descriptor.Supported || !nativeSignature(*Signature, Types)))
      {
        return {BytecodeStatus::InvalidImage, "Bytecode function has an unsupported C ABI signature"};
      }
      if ((Descriptor.External || Descriptor.Exported) && !validName(Descriptor.Symbol))
      {
        return {BytecodeStatus::InvalidImage, "Bytecode native import or export has an invalid symbol name"};
      }
      if (Descriptor.Exported && !NativeExports.insert(Descriptor.Symbol).second)
      {
        return {BytecodeStatus::DuplicateSymbol, "Bytecode artifact contains duplicate native exports: " + Descriptor.Symbol};
      }
      if (Artifact.Kind == BytecodeArtifactKind::Executable && Symbol.Kind == BytecodeSymbolKind::Import)
      {
        return {BytecodeStatus::MissingSymbol, "Executable bytecode cannot contain unresolved Ink imports"};
      }
      if (Artifact.Kind == BytecodeArtifactKind::Object && Symbol.Kind == BytecodeSymbolKind::Definition && Symbol.Identity.Module != Artifact.ModuleName)
      {
        return {BytecodeStatus::InvalidImage, "Bytecode definition belongs to a different module"};
      }
      for (const BytecodeGenericArgument &Argument : Symbol.Identity.GenericArguments)
      {
        const auto Type = IdentityTypes.find(Argument.Type);
        if (Type == IdentityTypes.end() || Argument.Kind > BytecodeGenericArgumentKind::Constant || (Argument.Kind == BytecodeGenericArgumentKind::Type ? !Argument.Value.empty() : !genericConstant(Argument, *Types.get(Type->second))))
        {
          return {BytecodeStatus::InvalidImage, "Bytecode generic argument is not a canonical typed value"};
        }
      }
      if (Symbol.Kind == BytecodeSymbolKind::Definition && (Artifact.Kind == BytecodeArtifactKind::Object || Symbol.Visibility != BytecodeVisibility::Private))
      {
        if (!artifact_detail::accountSymbolKey(Symbol.Identity, Usage))
        {
          return {BytecodeStatus::LimitExceeded, "Bytecode definition keys exceed the allocation limit"};
        }
        if (!Definitions.insert(bytecodeSymbolKey(Symbol.Identity)).second)
        {
          return {BytecodeStatus::DuplicateSymbol, "Bytecode artifact contains duplicate definitions"};
        }
      }
    }
    for (const auto &[Id, Function] : Artifact.Image.Functions)
    {
      const auto Symbol = Symbols.find(Id);
      const auto Descriptor = Artifact.Image.Descriptors.find(Id);
      if (Id != Function->Id || Symbol == Symbols.end() || Symbol->second->Kind != BytecodeSymbolKind::Definition || Descriptor == Artifact.Image.Descriptors.end() || Descriptor->second.Signature != Function->Signature)
      {
        return {BytecodeStatus::InvalidImage, "Bytecode function body does not match its definition"};
      }
      if (!Usage.allocation(Function->SlotTypes.size(), sizeof(bool)) || !Usage.allocation(Function->LocalStorageCount, sizeof(SlotId)))
      {
        return {BytecodeStatus::LimitExceeded, "Bytecode verifier workspace exceeds the allocation limit"};
      }
      if (const auto Result = validateFunction(*Function, Artifact); !Result)
      {
        return Result;
      }
    }
    if (Artifact.Entry != InvalidFunction)
    {
      const auto Found = Symbols.find(Artifact.Entry);
      if (Found == Symbols.end() || Found->second->Kind != BytecodeSymbolKind::Definition || Found->second->Visibility != BytecodeVisibility::Public)
      {
        return {BytecodeStatus::InvalidImage, "Bytecode entry must name a public definition"};
      }
    }
    return {};
  }
} // namespace ink::execution
