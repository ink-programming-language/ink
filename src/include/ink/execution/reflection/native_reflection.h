#ifndef INK_EXECUTION_REFLECTION_NATIVE_REFLECTION_H
#define INK_EXECUTION_REFLECTION_NATIVE_REFLECTION_H

#include "ink/execution/reflection/type_desc.h"
#include "ink/execution/support/execution_result.h"

#include <cstring>
#include <span>

namespace ink::execution
{
  // Relocatable, constant AOT counterparts of the owned descriptors. IDs are
  // local to a module table; Details points to the structure selected by Kind.
  // The getter uses the _INK2 J(Package, Module) identity; obtain it with ir::reflectionSymbol.
  using NativeThunk = void (*)(void *Result, const void *const *Arguments);

  struct NativeIntegerDesc
  {
      std::uint32_t BitWidth;
      std::uint32_t Signed;
  };

  struct NativeFloatDesc
  {
      std::uint32_t BitWidth;
  };

  struct NativePointerDesc
  {
      RuntimeTypeId Pointee;
      std::uint32_t Writable;
  };

  struct NativeFunctionDesc
  {
      RuntimeTypeId ReturnType;
      std::size_t ParameterCount;
      const RuntimeTypeId *Parameters;
  };

  struct NativeArrayDesc
  {
      RuntimeTypeId ElementType;
      std::uint64_t ElementCount;
  };

  struct NativeFieldDesc
  {
      const char *Name;
      RuntimeTypeId Type;
      std::size_t Offset;
      std::uint32_t Visibility;
      NativeThunk Initializer;
  };

  struct NativeMethodDesc
  {
      const char *Name;
      RuntimeTypeId Signature;
      std::uint32_t Visibility;
      std::uint32_t WritableReceiver;
      NativeThunk Invoke;
  };

  struct NativeClassDesc
  {
      std::size_t FieldCount;
      const NativeFieldDesc *Fields;
      std::size_t MethodCount;
      const NativeMethodDesc *Methods;
  };

  struct NativeTypeDesc
  {
      std::uint32_t Kind;
      const char *Name;
      std::size_t Size;
      std::size_t Alignment;
      const void *Details;
  };

  struct NativeModuleDesc
  {
      std::uint32_t Version;
      const char *Name;
      std::size_t TypeCount;
      const NativeTypeDesc *Types;
  };

  struct NativeObjectView
  {
      const NativeModuleDesc *Module = nullptr;
      RuntimeTypeId Type = InvalidRuntimeType;
      const void *Data = nullptr;
      std::size_t Size = 0;
      bool Writable = false;
  };

  inline const NativeTypeDesc *nativeType(const NativeObjectView &View)
  {
    return View.Module && View.Module->Version == 1 && View.Type < View.Module->TypeCount && View.Data && View.Size >= View.Module->Types[View.Type].Size ? &View.Module->Types[View.Type] : nullptr;
  }

  inline RuntimeTypeId findNativeType(const NativeModuleDesc &Module, std::string_view Name)
  {
    if (Module.Version == 1 && !Name.empty())
    {
      for (std::size_t Index = 0; Index < Module.TypeCount; ++Index)
      {
        if (Name == Module.Types[Index].Name)
        {
          return static_cast<RuntimeTypeId>(Index);
        }
      }
    }
    return InvalidRuntimeType;
  }

  inline ExecutionStatus nativeField(const NativeObjectView &Object, std::string_view Name, NativeObjectView &Result)
  {
    const auto *Type = nativeType(Object);
    if (!Type || Type->Kind != static_cast<unsigned>(RuntimeKind::Class))
    {
      return ExecutionStatus::TypeMismatch;
    }
    const auto &Class = *static_cast<const NativeClassDesc *>(Type->Details);
    for (std::size_t Index = 0; Index < Class.FieldCount; ++Index)
    {
      const auto &Field = Class.Fields[Index];
      if (Name != Field.Name)
      {
        continue;
      }
      if (Field.Visibility != static_cast<unsigned>(core::VisibilityKind::Public))
      {
        return ExecutionStatus::AccessDenied;
      }
      Result = {Object.Module, Field.Type, static_cast<const std::byte *>(Object.Data) + Field.Offset, Object.Module->Types[Field.Type].Size, Object.Writable};
      return ExecutionStatus::Success;
    }
    return ExecutionStatus::UnknownBinding;
  }

  inline ExecutionStatus nativeElement(const NativeObjectView &Object, std::size_t Index, NativeObjectView &Result)
  {
    const auto *Type = nativeType(Object);
    if (!Type || Type->Kind != static_cast<unsigned>(RuntimeKind::Array))
    {
      return ExecutionStatus::TypeMismatch;
    }
    const auto &Array = *static_cast<const NativeArrayDesc *>(Type->Details);
    if (Index >= Array.ElementCount)
    {
      return ExecutionStatus::IndexOutOfBounds;
    }
    const auto Size = Object.Module->Types[Array.ElementType].Size;
    Result = {Object.Module, Array.ElementType, static_cast<const std::byte *>(Object.Data) + Index * Size, Size, Object.Writable};
    return ExecutionStatus::Success;
  }

  // Native views borrow all referenced memory, including string payloads. The
  // caller keeps those payloads alive for as long as a copied value uses them.
  inline ExecutionStatus nativeWrite(const NativeObjectView &Destination, const NativeObjectView &Source)
  {
    if (!nativeType(Destination) || !nativeType(Source))
    {
      return ExecutionStatus::InvalidPlace;
    }
    if (!Destination.Writable)
    {
      return ExecutionStatus::ReadOnly;
    }
    if (Destination.Module != Source.Module)
    {
      return ExecutionStatus::ForeignContext;
    }
    if (Destination.Type != Source.Type)
    {
      return ExecutionStatus::TypeMismatch;
    }
    std::memmove(const_cast<void *>(Destination.Data), Source.Data, nativeType(Destination)->Size);
    return ExecutionStatus::Success;
  }

  inline ExecutionStatus nativeInvoke(const NativeObjectView &Object, std::string_view Name, std::span<const NativeObjectView> Arguments, const NativeObjectView &Result)
  {
    const auto *Type = nativeType(Object);
    if (!Type || Type->Kind != static_cast<unsigned>(RuntimeKind::Class))
    {
      return ExecutionStatus::TypeMismatch;
    }
    const auto &Class = *static_cast<const NativeClassDesc *>(Type->Details);
    for (const auto &Argument : Arguments)
    {
      if (!nativeType(Argument))
      {
        return ExecutionStatus::InvalidPlace;
      }
      if (Argument.Module != Object.Module)
      {
        return ExecutionStatus::ForeignContext;
      }
    }
    const NativeMethodDesc *Selected = nullptr;
    bool Named = false;
    bool Accessible = false;
    for (std::size_t Index = 0; Index < Class.MethodCount; ++Index)
    {
      const auto &Method = Class.Methods[Index];
      if (Name != Method.Name)
      {
        continue;
      }
      Named = true;
      if (Method.Visibility != static_cast<unsigned>(core::VisibilityKind::Public))
      {
        continue;
      }
      Accessible = true;
      const auto &Signature = *static_cast<const NativeFunctionDesc *>(Object.Module->Types[Method.Signature].Details);
      bool Matches = Signature.ParameterCount == Arguments.size() + 1;
      for (std::size_t Argument = 0; Matches && Argument < Arguments.size(); ++Argument)
      {
        Matches = nativeType(Arguments[Argument]) && Arguments[Argument].Module == Object.Module && Arguments[Argument].Type == Signature.Parameters[Argument + 1];
      }
      if (Matches)
      {
        if (Selected)
        {
          return ExecutionStatus::AmbiguousMember;
        }
        Selected = &Method;
      }
    }
    if (!Selected)
    {
      return Named ? Accessible ? ExecutionStatus::InvalidArguments : ExecutionStatus::AccessDenied : ExecutionStatus::UnknownBinding;
    }
    if (!Selected->Invoke)
    {
      return ExecutionStatus::MissingBody;
    }
    if (Selected->WritableReceiver && !Object.Writable)
    {
      return ExecutionStatus::ReadOnly;
    }
    const auto &Signature = *static_cast<const NativeFunctionDesc *>(Object.Module->Types[Selected->Signature].Details);
    const bool Void = Object.Module->Types[Signature.ReturnType].Kind == static_cast<unsigned>(RuntimeKind::Void);
    if (!Void && (!nativeType(Result) || Result.Module != Object.Module || Result.Type != Signature.ReturnType || !Result.Writable))
    {
      return ExecutionStatus::TypeMismatch;
    }
    const void *Receiver = Object.Data;
    std::vector<const void *> Values{&Receiver};
    for (const auto &Argument : Arguments)
    {
      Values.push_back(Argument.Data);
    }
    Selected->Invoke(Void ? nullptr : const_cast<void *>(Result.Data), Values.data());
    return ExecutionStatus::Success;
  }

  inline ExecutionStatus nativeConstruct(const NativeObjectView &Object, std::span<const NativeObjectView> Arguments = {})
  {
    const auto *Type = nativeType(Object);
    if (!Type || Type->Kind != static_cast<unsigned>(RuntimeKind::Class) || !Object.Writable)
    {
      return ExecutionStatus::InvalidArguments;
    }
    const auto &Class = *static_cast<const NativeClassDesc *>(Type->Details);
    for (std::size_t Index = 0; Index < Class.MethodCount; ++Index)
    {
      if (std::string_view(Class.Methods[Index].Name) == "__init__")
      {
        return nativeInvoke(Object, "__init__", Arguments, {});
      }
    }
    for (std::size_t Index = 0; Index < Class.MethodCount; ++Index)
    {
      if (std::string_view(Class.Methods[Index].Name) == "__del__")
      {
        return ExecutionStatus::InvalidArguments;
      }
    }
    // Hand-built aggregate IR without lifecycle methods retains its explicit field API.
    if (Arguments.size() > Class.FieldCount)
    {
      return ExecutionStatus::InvalidArguments;
    }
    for (std::size_t Index = 0; Index < Class.FieldCount; ++Index)
    {
      const auto &Field = Class.Fields[Index];
      if (Index < Arguments.size())
      {
        if (!nativeType(Arguments[Index]) || Arguments[Index].Module != Object.Module || Arguments[Index].Type != Field.Type || Field.Visibility != static_cast<unsigned>(core::VisibilityKind::Public))
        {
          return ExecutionStatus::InvalidArguments;
        }
      }
      else if (!Field.Initializer)
      {
        return ExecutionStatus::InvalidArguments;
      }
    }
    std::vector<std::vector<std::byte>> Snapshots;
    for (const auto &Argument : Arguments)
    {
      const auto *Begin = static_cast<const std::byte *>(Argument.Data);
      Snapshots.emplace_back(Begin, Begin + nativeType(Argument)->Size);
    }
    auto *Bytes = static_cast<std::byte *>(const_cast<void *>(Object.Data));
    for (std::size_t Index = 0; Index < Class.FieldCount; ++Index)
    {
      const auto &Field = Class.Fields[Index];
      if (Index < Arguments.size())
      {
        std::memmove(Bytes + Field.Offset, Snapshots[Index].data(), Object.Module->Types[Field.Type].Size);
      }
      else
      {
        Field.Initializer(Bytes + Field.Offset, nullptr);
      }
    }
    return ExecutionStatus::Success;
  }

  inline ExecutionStatus nativeDestroy(const NativeObjectView &Object)
  {
    return nativeInvoke(Object, "__del__", {}, {});
  }
} // namespace ink::execution

#endif
