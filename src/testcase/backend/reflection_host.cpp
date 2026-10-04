#include "ink/execution/reflection/native_reflection.h"

#include <cstdint>

using namespace ink::execution;

extern "C" const NativeModuleDesc *_INK2J49_P31_N5_localL12_N9_anonymousN1_0L0_M10_N7_program();

namespace
{
  std::int32_t Initializers = 0;
}

extern "C" std::int32_t inkReflectionNext()
{
  return ++Initializers;
}

extern "C" std::int32_t inkReflectionProbe()
{
  const auto *Module = _INK2J49_P31_N5_localL12_N9_anonymousN1_0L0_M10_N7_program();
  if (!Module || Module->Version != 1)
  {
    return 1;
  }
  RuntimeTypeId Counter = InvalidRuntimeType;
  bool RetainedUnused = false;
  for (std::size_t Index = 0; Index < Module->TypeCount; ++Index)
  {
    if (Module->Types[Index].Kind == static_cast<unsigned>(RuntimeKind::Class))
    {
      const auto &Class = *static_cast<const NativeClassDesc *>(Module->Types[Index].Details);
      if (Class.FieldCount && std::string_view(Class.Fields[0].Name) == "UnusedCode")
      {
        RetainedUnused = findNativeType(*Module, Module->Types[Index].Name) == Index;
      }
      else if (Class.FieldCount && std::string_view(Class.Fields[0].Name) == "Value")
      {
        Counter = findNativeType(*Module, Module->Types[Index].Name);
      }
    }
  }
  if (!RetainedUnused || Counter == InvalidRuntimeType)
  {
    return 12;
  }
  alignas(16) std::byte Bytes[128]{};
  alignas(16) std::byte CopyBytes[128]{};
  NativeObjectView Object{Module, Counter, Bytes, sizeof(Bytes), true};
  NativeObjectView Copy{Module, Counter, CopyBytes, sizeof(CopyBytes), true};
  Initializers = 37;
  if (nativeConstruct(Object) != ExecutionStatus::Success || nativeConstruct(Copy) != ExecutionStatus::Success || Initializers != 39)
  {
    return 2;
  }
  NativeObjectView Field;
  if (nativeField(Object, "Secret", Field) != ExecutionStatus::AccessDenied || nativeField(Object, "Value", Field) != ExecutionStatus::Success)
  {
    return 3;
  }
  std::int32_t Value = 0;
  std::memcpy(&Value, Field.Data, sizeof(Value));
  if (Value != 38)
  {
    return 4;
  }
  NativeObjectView Result{Module, findNativeType(*Module, "i32"), &Value, sizeof(Value), true};
  if (nativeInvoke(Object, "advance", {}, Result) != ExecutionStatus::Success || Value != 40)
  {
    return 5;
  }
  if (nativeInvoke(Object, "copy", {}, Copy) != ExecutionStatus::Success || nativeInvoke(Copy, "advance", {}, Result) != ExecutionStatus::Success || Value != 42)
  {
    return 6;
  }
  Object.Writable = false;
  if (nativeInvoke(Object, "advance", {}, Result) != ExecutionStatus::ReadOnly)
  {
    return 7;
  }
  if (nativeField(Copy, "Flag", Field) != ExecutionStatus::Success)
  {
    return 8;
  }
  std::uint8_t Flag = 0;
  std::memcpy(&Flag, Field.Data, sizeof(Flag));
  if (Flag != 1 || nativeField(Copy, "Flags", Field) != ExecutionStatus::Success)
  {
    return 9;
  }
  NativeObjectView Element;
  if (nativeElement(Field, 1, Element) != ExecutionStatus::Success || nativeElement(Field, 2, Element) != ExecutionStatus::IndexOutOfBounds)
  {
    return 10;
  }
  std::memcpy(&Flag, Element.Data, sizeof(Flag));
  Object.Writable = true;
  if (nativeDestroy(Object) != ExecutionStatus::Success || nativeDestroy(Copy) != ExecutionStatus::Success)
  {
    return 13;
  }
  return Flag == 0 ? 0 : 11;
}
