#include "ink/core/target_context.h"
#include "ink/core/object_layout.h"
#include "ink/abi/name_mangling.h"
#include "bytecode_internal.h"

#include "ink/execution/bytecode/execution_compiler.h"

#include <algorithm>
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
      return static_cast<std::size_t>(core::TargetContext::native().integerAlignment(Width));
    }

    bool validLayout(const TypeDesc &Layout, const RuntimeTypeTable &Types)
    {
      if (!Layout.validDetails() || Layout.Domain != Types.domain())
      {
        return false;
      }
      switch (Layout.Kind)
      {
      case RuntimeKind::Void:
        return Layout.bitWidth() == 0 && Layout.Size == 0 && Layout.Alignment == 1 && !Layout.Native;
      case RuntimeKind::Boolean:
        return Layout.bitWidth() == 1 && Layout.Size == sizeof(bool) && Layout.Alignment == alignof(bool) && Layout.Native;
      case RuntimeKind::Integer:
        return Layout.bitWidth() != 0 && Layout.Size == (((static_cast<std::uint64_t>(Layout.bitWidth()) + 7) / 8 + integerAlignment(Layout.bitWidth()) - 1) & ~(static_cast<std::uint64_t>(integerAlignment(Layout.bitWidth())) - 1)) && Layout.Alignment == integerAlignment(Layout.bitWidth()) && Layout.Native == nativeInteger(Layout.bitWidth());
      case RuntimeKind::Float:
        return (Layout.bitWidth() == 16 || Layout.bitWidth() == 32 || Layout.bitWidth() == 64) && Layout.Size == Layout.bitWidth() / 8 && Layout.Alignment == core::TargetContext::native().floatAlignment(Layout.bitWidth()) && Layout.Native == (Layout.bitWidth() != 16);
      case RuntimeKind::String:
        return Layout.bitWidth() == 0 && Layout.Size == sizeof(void *) * 2 && Layout.Alignment == sizeof(void *) && !Layout.Native;
      case RuntimeKind::Array:
      {
        const TypeDesc *Element = Types.get(Layout.arrayDesc().ElementType);
        return Element && Element->Kind != RuntimeKind::Void && Element->Kind != RuntimeKind::Invalid && Layout.arrayDesc().ElementLayout && Layout.arrayDesc().ElementLayout->Domain == Types.domain() && Layout.arrayDesc().ElementLayout->Type == Layout.arrayDesc().ElementType && Layout.bitWidth() == 0 && (!Element->Size || Layout.arrayDesc().ElementCount <= std::numeric_limits<std::size_t>::max() / Element->Size) && Layout.Size == Element->Size * Layout.arrayDesc().ElementCount && Layout.Alignment == Element->Alignment && Layout.Native == Element->Native;
      }
      case RuntimeKind::Class:
      {
        const auto Identity = abi::demangle(Layout.classDesc().NominalIdentity);
        const auto Nominal = Identity ? abi::childRecords(*Identity.Identity) : std::nullopt;
        if (!Nominal || Identity.Identity->Tag != 'T' || Nominal->size() != 1 || Nominal->front().Tag != 'c' || Layout.bitWidth())
        {
          return false;
        }
        core::ObjectLayoutBuilder Object(std::numeric_limits<std::size_t>::max());
        bool Native = true;
        for (std::size_t Index = 0; Index < Layout.classDesc().Fields.size(); ++Index)
        {
          if (!validName(Layout.classDesc().Fields[Index].Name))
          {
            return false;
          }
          const TypeDesc *Field = Types.get(Layout.classDesc().Fields[Index].Type);
          const auto &Owned = Layout.classDesc().Fields[Index].Layout;
          if (!Field || Field->Kind == RuntimeKind::Invalid || Field->Kind == RuntimeKind::Void || !Owned || Owned->Domain != Types.domain() || Owned->Type != Field->Type || Owned->Size != Field->Size || Owned->Alignment != Field->Alignment || !Field->Alignment)
          {
            return false;
          }
          const auto Offset = Object.append(Field->Size, Field->Alignment);
          if (!Offset || Layout.classDesc().Fields[Index].Offset != *Offset)
          {
            return false;
          }
          Native = Native && Field->Native;
        }
        return Object.size() && Layout.Size == *Object.size() && Layout.Alignment == Object.alignment() && Layout.Native == Native;
      }
      case RuntimeKind::Pointer:
        return Layout.bitWidth() == 0 && Layout.Size == sizeof(void *) && Layout.Alignment == sizeof(void *) && !Layout.Native && Types.get(Layout.pointerDesc().Pointee);
      case RuntimeKind::Function:
        if (Layout.bitWidth() != 0 || Layout.Size != sizeof(void *) || Layout.Alignment != sizeof(void *) || Layout.Native || !Types.get(Layout.functionDesc().ReturnType))
        {
          return false;
        }
        for (RuntimeTypeId Parameter : Layout.functionDesc().Parameters)
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

    bool validReflection(const TypeDesc &Layout, const ExecutionImage &Image)
    {
      if (Layout.Kind != RuntimeKind::Class)
      {
        return Layout.Name.find('\0') == std::string::npos;
      }
      if (Layout.Name != Layout.classDesc().NominalIdentity)
      {
        return false;
      }
      const auto &Types = *Image.Layouts;
      std::unordered_set<std::string> Names;
      for (const auto &Field : Layout.classDesc().Fields)
      {
        if (!Names.insert(Field.Name).second || Field.Visibility > core::VisibilityKind::Private)
        {
          return false;
        }
        if (Field.Initializer != InvalidFunction)
        {
          const auto Found = Image.Descriptors.find(Field.Initializer);
          const auto *Signature = Found == Image.Descriptors.end() ? nullptr : Types.get(Found->second.Signature);
          if (!Signature || Signature->Kind != RuntimeKind::Function || Signature->functionDesc().ReturnType != Field.Type || !Signature->functionDesc().Parameters.empty())
          {
            return false;
          }
        }
      }
      Names.clear();
      for (const auto &Method : Layout.classDesc().Methods)
      {
        const auto *Signature = Types.get(Method.Signature);
        if (!validName(Method.Name) || Method.Visibility > core::VisibilityKind::Private || !Names.insert(Method.Name + ":" + std::to_string(Method.Signature)).second || !Signature || Signature->Kind != RuntimeKind::Function || Signature->functionDesc().Parameters.empty())
        {
          return false;
        }
        const auto *Receiver = Types.get(Signature->functionDesc().Parameters.front());
        if (!Receiver || Receiver->Kind != RuntimeKind::Pointer || Receiver->pointerDesc().Pointee != Layout.Type || Receiver->pointerDesc().Writable != Method.WritableReceiver)
        {
          return false;
        }
        const auto Found = Image.Descriptors.find(Method.Function);
        if (Method.Function == InvalidFunction ? Method.Visibility != core::VisibilityKind::Private : Found == Image.Descriptors.end() || Found->second.Signature != Method.Signature)
        {
          return false;
        }
      }
      return true;
    }

    bool validAggregateDepth(const RuntimeTypeTable &Types, std::size_t Maximum)
    {
      if (Maximum == 0)
      {
        return false;
      }
      struct Visit
      {
          RuntimeTypeId Type;
          std::size_t Child = 0;
      };
      std::vector<std::uint8_t> States(Types.size());
      std::vector<std::size_t> Depths(Types.size());
      std::vector<Visit> Stack;
      for (std::size_t Root = 0; Root < Types.size(); ++Root)
      {
        if (States[Root] == 2)
        {
          continue;
        }
        States[Root] = 1;
        Stack.push_back({static_cast<RuntimeTypeId>(Root)});
        while (!Stack.empty())
        {
          if (Stack.size() > Maximum)
          {
            return false;
          }
          Visit &Current = Stack.back();
          const TypeDesc &Layout = *Types.get(Current.Type);
          const std::size_t Count = Layout.Kind == RuntimeKind::Class ? Layout.classDesc().Fields.size() : Layout.Kind == RuntimeKind::Array ? 1 : 0;
          if (Current.Child < Count)
          {
            const RuntimeTypeId Child = Layout.Kind == RuntimeKind::Class ? Layout.classDesc().Fields[Current.Child++].Type : (++Current.Child, Layout.arrayDesc().ElementType);
            if (!Types.get(Child) || States[Child] == 1)
            {
              return false;
            }
            if (States[Child] == 0)
            {
              States[Child] = 1;
              Stack.push_back({Child});
            }
            continue;
          }
          std::size_t Depth = 1;
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            const RuntimeTypeId Child = Layout.Kind == RuntimeKind::Class ? Layout.classDesc().Fields[Index].Type : Layout.arrayDesc().ElementType;
            if (Depths[Child] >= Maximum)
            {
              return false;
            }
            Depth = std::max(Depth, Depths[Child] + 1);
          }
          Depths[Current.Type] = Depth;
          States[Current.Type] = 2;
          Stack.pop_back();
        }
      }
      return true;
    }

    bool nativeType(const TypeDesc &Layout, bool Return)
    {
      return (Return && Layout.Kind == RuntimeKind::Void) || Layout.Kind == RuntimeKind::Boolean || (Layout.Kind == RuntimeKind::Integer && nativeInteger(Layout.bitWidth())) || (Layout.Kind == RuntimeKind::Float && (Layout.bitWidth() == 32 || Layout.bitWidth() == 64)) || Layout.Kind == RuntimeKind::Pointer;
    }

    bool nativeSignature(const TypeDesc &Signature, const RuntimeTypeTable &Types)
    {
      if (!nativeType(*Types.get(Signature.functionDesc().ReturnType), true) || Signature.functionDesc().Parameters.size() > std::numeric_limits<unsigned int>::max())
      {
        return false;
      }
      for (RuntimeTypeId Parameter : Signature.functionDesc().Parameters)
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
      return Character >= '0' && Character <= '9' ? Character - '0' : Character >= 'A' && Character <= 'F' ? Character - 'A' + 10 : -1;
    }

    bool genericConstant(const BytecodeGenericArgument &Argument, const TypeDesc &Type)
    {
      if (Type.Kind == RuntimeKind::Boolean)
      {
        return Argument.Value == "0" || Argument.Value == "1";
      }
      if (Type.Kind != RuntimeKind::Integer && Type.Kind != RuntimeKind::Float && Type.Kind != RuntimeKind::String)
      {
        return false;
      }
      if (Type.Kind == RuntimeKind::String ? Argument.Value.size() % 2 != 0 : Argument.Value.size() != (static_cast<std::uint64_t>(Type.bitWidth()) + 3) / 4)
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
      return Type.Kind == RuntimeKind::String || Type.bitWidth() % 4 == 0 || static_cast<unsigned>(hexDigit(Argument.Value.front())) < (1U << (Type.bitWidth() % 4));
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
        if (Type.Kind == RuntimeKind::Array || Type.Kind == RuntimeKind::Class)
        {
          for (const RuntimeValue &Element : Value.aggregate())
          {
            Pending.push_back(&Element);
          }
        }
      }
      // Call records are validated even when no instruction happens to reference them.
      for (const ExecutionCallSite &Call : Function.Calls)
      {
        const TypeDesc *Signature = Types.get(Call.Signature);
        if (!Signature || Signature->Kind != RuntimeKind::Function || Call.Arguments.size() != Signature->functionDesc().Parameters.size())
        {
          return {BytecodeStatus::InvalidImage, "Bytecode call record has an invalid signature"};
        }
        for (std::size_t Index = 0; Index < Call.Arguments.size(); ++Index)
        {
          if (Call.Arguments[Index] >= Function.SlotTypes.size() || Function.SlotTypes[Call.Arguments[Index]] != Signature->functionDesc().Parameters[Index])
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
        if (!Usage.records(Types.size(), sizeof(TypeDesc) * 2 + sizeof(void *) * 2))
        {
          return {BytecodeStatus::LimitExceeded, "Bytecode type table exceeds the configured storage limits"};
        }
        for (std::size_t Index = 0; Index < Types.size(); ++Index)
        {
          const TypeDesc &Layout = *Types.get(static_cast<RuntimeTypeId>(Index));
          if (!Usage.string(Layout.Name) || !Usage.records(Layout.classDesc().Methods.size(), sizeof(MethodDesc)) || !Usage.records(Layout.functionDesc().Parameters.size(), sizeof(RuntimeTypeId)) || !Usage.string(Layout.classDesc().NominalIdentity) || !Usage.records(Layout.classDesc().Fields.size(), sizeof(RuntimeTypeId) + sizeof(std::size_t) + sizeof(TypeDesc) + sizeof(void *) * 2))
          {
            return {BytecodeStatus::LimitExceeded, "Bytecode signature parameters exceed the configured storage limits"};
          }
          for (const MethodDesc &Method : Layout.classDesc().Methods)
          {
            if (!Usage.string(Method.Name))
            {
              return {BytecodeStatus::LimitExceeded, "Bytecode method names exceed the string limit"};
            }
          }
          for (const FieldDesc &Field : Layout.classDesc().Fields)
          {
            if (!Usage.string(Field.Name))
            {
              return {BytecodeStatus::LimitExceeded, "Bytecode field names exceed the configured string limit"};
            }
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
        if (!Usage.string(Symbol.Identity.Module) || !Usage.string(Symbol.Identity.Name) || !Usage.string(Symbol.Identity.Signature) || !Usage.string(Symbol.Identity.LinkName) || !Usage.records(Symbol.Identity.GenericArguments.size(), sizeof(BytecodeGenericArgument)))
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
            if (Current.kind() == RuntimeKind::Array || Current.kind() == RuntimeKind::Class)
            {
              if (!Usage.records(Current.aggregate().size(), sizeof(RuntimeValue) + sizeof(RuntimeValue *)))
              {
                return {BytecodeStatus::LimitExceeded, "Bytecode array constants exceed the configured storage limits"};
              }
              for (const RuntimeValue &Element : Current.aggregate())
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
      const TypeDesc &Layout = *Types.get(static_cast<RuntimeTypeId>(Index));
      if (Layout.Type != Index || !validLayout(Layout, Types) || !validReflection(Layout, Artifact.Image))
      {
        return {BytecodeStatus::InvalidImage, "Bytecode layout does not match its runtime kind or native target"};
      }
      if (Layout.Size > Limits.MaxAllocationBytes)
      {
        return {BytecodeStatus::LimitExceeded, "Bytecode layout exceeds the allocation limit"};
      }
    }
    if (!Usage.allocation(Types.size(), sizeof(std::size_t) * 3 + sizeof(std::uint8_t)) || !validAggregateDepth(Types, Limits.MaxTypeDepth))
    {
      return {BytecodeStatus::LimitExceeded, "Aggregate layouts exceed the nesting or allocation limit"};
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
      const auto [Existing, Inserted] = IdentityTypes.emplace(Identities[Index], static_cast<RuntimeTypeId>(Index));
      const TypeDesc &Layout = *Types.get(static_cast<RuntimeTypeId>(Index));
      if (!Inserted && Layout.Kind == RuntimeKind::Class)
      {
        const TypeDesc &Previous = *Types.get(Existing->second);
        if (Layout.Size != Previous.Size || Layout.Alignment != Previous.Alignment || Layout.Native != Previous.Native || Layout.classDesc().Fields.size() != Previous.classDesc().Fields.size() || Layout.classDesc().Methods.size() != Previous.classDesc().Methods.size())
        {
          return {BytecodeStatus::InvalidImage, "Conflicting definitions of the same nominal class"};
        }
        for (std::size_t Field = 0; Field < Layout.classDesc().Fields.size(); ++Field)
        {
          const auto &CurrentField = Layout.classDesc().Fields[Field];
          const auto &PreviousField = Previous.classDesc().Fields[Field];
          if (CurrentField.Name != PreviousField.Name || CurrentField.Offset != PreviousField.Offset || CurrentField.Visibility != PreviousField.Visibility || CurrentField.Initializer != PreviousField.Initializer || Identities[CurrentField.Type] != Identities[PreviousField.Type])
          {
            return {BytecodeStatus::InvalidImage, "Conflicting field types of the same nominal class"};
          }
        }
        for (std::size_t MethodIndex = 0; MethodIndex < Layout.classDesc().Methods.size(); ++MethodIndex)
        {
          const auto &Method = Layout.classDesc().Methods[MethodIndex];
          const auto &PreviousMethod = Previous.classDesc().Methods[MethodIndex];
          if (Method.Name != PreviousMethod.Name || Method.Visibility != PreviousMethod.Visibility || Method.WritableReceiver != PreviousMethod.WritableReceiver || Method.Function != PreviousMethod.Function || Identities[Method.Signature] != Identities[PreviousMethod.Signature])
          {
            return {BytecodeStatus::InvalidImage, "Conflicting methods of the same nominal class"};
          }
        }
      }
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
      const TypeDesc *Signature = Types.get(Descriptor.Signature);
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
      if ((Descriptor.External || Descriptor.Exported) && (!validName(Descriptor.Symbol) || Descriptor.Symbol.starts_with("_INK")))
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
      if (Artifact.Kind == BytecodeArtifactKind::Object && Symbol.Kind == BytecodeSymbolKind::Definition && Symbol.Identity.Module != Artifact.ModuleName && Symbol.Visibility != BytecodeVisibility::Private)
      {
        return {BytecodeStatus::InvalidImage, "Bytecode definition belongs to a different module"};
      }
      for (const BytecodeGenericArgument &Argument : Symbol.Identity.GenericArguments)
      {
        const auto Type = IdentityTypes.find(Argument.Type);
        if (Type == IdentityTypes.end() || Argument.Kind > core::GenericArgumentKind::Value || (Argument.Kind == core::GenericArgumentKind::Type ? !Argument.Value.empty() : !genericConstant(Argument, *Types.get(Type->second))))
        {
          return {BytecodeStatus::InvalidImage, "Bytecode generic argument is not a canonical typed value"};
        }
      }
      if (!artifact_detail::validSymbolLinkage(Symbol, Descriptor, Types, IdentityTypes))
      {
        return {BytecodeStatus::SignatureMismatch, "Bytecode link identity differs from its logical signature or generic arguments"};
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
