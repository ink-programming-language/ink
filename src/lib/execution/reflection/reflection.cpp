#include "ink/execution/reflection/reflection.h"
#include "ink/execution/engine/execution_machine.h"

namespace ink::execution
{
  Reflection::Reflection(std::shared_ptr<const RuntimeTypeTable> Types, ExecutionMemoryManager &Memory, ExecutionMachine *Machine)
      : Types(std::move(Types)),
        Memory(Memory),
        Machine(Machine)
  {
  }

  ReflectedValue Reflection::value(RuntimeValue Value) const
  {
    return {std::move(Value), Types ? Types->domain() : nullptr};
  }

  ObjectViewResult Reflection::view(ExecutionPlace Place) const
  {
    if (const auto Status = Place.status(); Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    const auto *Cell = Place.storage().cell();
    if (!Types || Cell->layout().Domain != Types->domain() || !Memory.owns(Place.storage()))
    {
      return {ExecutionStatus::ForeignContext};
    }
    return {ExecutionStatus::Success, {Cell->data(), Cell->type(), Types, Cell->writable(), Place.storage()}};
  }

  ObjectViewResult Reflection::view(RuntimeTypeId Type, std::span<const std::byte> Bytes) const
  {
    const auto *Layout = Types ? Types->get(Type) : nullptr;
    if (!Layout || Layout->Kind == RuntimeKind::Invalid || !Bytes.data() || Bytes.size() < Layout->Size)
    {
      return {ExecutionStatus::InvalidPlace};
    }
    return {ExecutionStatus::Success, {Bytes.data(), Type, Types, false}};
  }

  ObjectViewResult Reflection::view(RuntimeTypeId Type, std::span<std::byte> Bytes) const
  {
    auto Result = view(Type, std::span<const std::byte>(Bytes));
    Result.Value.Writable = static_cast<bool>(Result);
    return Result;
  }

  ExecutionStatus Reflection::validate(const ObjectView &Object) const
  {
    if (!Types || !Object.Types || Object.Types->domain() != Types->domain())
    {
      return ExecutionStatus::ForeignContext;
    }
    if (!Object.Data || !Types->get(Object.Type))
    {
      return ExecutionStatus::InvalidPlace;
    }
    if (Object.Storage.hasIdentity())
    {
      return Object.Storage.status();
    }
    return ExecutionStatus::Success;
  }

  ObjectViewResult Reflection::subobject(const ObjectView &Object, RuntimeTypeId Type, std::size_t Offset) const
  {
    ObjectView Result = Object;
    Result.Data = static_cast<const std::byte *>(Object.Data) + Offset;
    Result.Type = Type;
    return {ExecutionStatus::Success, std::move(Result)};
  }

  ObjectViewResult Reflection::field(const ObjectView &Object, std::string_view Name) const
  {
    if (const auto Status = validate(Object); Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    const auto &Layout = *Types->get(Object.Type);
    if (Layout.Kind != RuntimeKind::Class)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    const auto *Field = Layout.classDesc().findField(Name);
    if (!Field)
    {
      return {ExecutionStatus::UnknownBinding};
    }
    if (Field->Visibility != core::VisibilityKind::Public)
    {
      return {ExecutionStatus::AccessDenied};
    }
    return subobject(Object, Field->Type, Field->Offset);
  }

  ObjectViewResult Reflection::element(const ObjectView &Object, std::size_t Index) const
  {
    if (const auto Status = validate(Object); Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    const auto &Layout = *Types->get(Object.Type);
    if (Layout.Kind != RuntimeKind::Array)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    const auto &Array = Layout.arrayDesc();
    if (Index >= Array.ElementCount)
    {
      return {ExecutionStatus::IndexOutOfBounds};
    }
    return subobject(Object, Array.ElementType, Index * Array.ElementLayout->Size);
  }

  RuntimeValueResult Reflection::read(const ObjectView &Object) const
  {
    if (const auto Status = validate(Object); Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    return Memory.loadPointer(ExecutionPointer::fromNative(const_cast<void *>(Object.Data)), *Types->get(Object.Type));
  }

  ExecutionStatus Reflection::write(const ObjectView &Object, const ReflectedValue &Value)
  {
    if (const auto Status = validate(Object); Status != ExecutionStatus::Success)
    {
      return Status;
    }
    if (!Object.Writable)
    {
      return ExecutionStatus::ReadOnly;
    }
    if (Value.Domain != Types->domain())
    {
      return ExecutionStatus::ForeignContext;
    }
    return Memory.storePointer(ExecutionPointer::fromNative(const_cast<void *>(Object.Data)), *Types->get(Object.Type), Value.Value);
  }

  RuntimeValueResult Reflection::getField(const ObjectView &Object, std::string_view Name) const
  {
    const auto Field = field(Object, Name);
    return Field ? read(Field.Value) : RuntimeValueResult{Field.Status};
  }

  ExecutionStatus Reflection::setField(const ObjectView &Object, std::string_view Name, const ReflectedValue &Value)
  {
    const auto Field = field(Object, Name);
    return Field ? write(Field.Value, Value) : Field.Status;
  }

  RuntimeValueResult Reflection::invoke(const ObjectView &Object, std::string_view Name, std::span<const ReflectedValue> Arguments)
  {
    if (const auto Status = validate(Object); Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    if (Types->get(Object.Type)->Kind != RuntimeKind::Class)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    const MethodDesc *Selected = nullptr;
    bool Named = false;
    bool Accessible = false;
    for (const auto &Method : Types->get(Object.Type)->classDesc().Methods)
    {
      if (Method.Name != Name)
      {
        continue;
      }
      Named = true;
      if (Method.Visibility != core::VisibilityKind::Public)
      {
        continue;
      }
      Accessible = true;
      const auto *Signature = Types->get(Method.Signature);
      if (!Signature || Signature->Kind != RuntimeKind::Function || Signature->functionDesc().Parameters.size() != Arguments.size() + 1)
      {
        continue;
      }
      bool Matches = true;
      for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
      {
        if (Arguments[Index].Domain != Types->domain())
        {
          return {ExecutionStatus::ForeignContext};
        }
        Matches = Matches && Arguments[Index].Value.Initialized && Arguments[Index].Value.Type == Signature->functionDesc().Parameters[Index + 1];
      }
      if (Matches)
      {
        if (Selected)
        {
          return {ExecutionStatus::AmbiguousMember};
        }
        Selected = &Method;
      }
    }
    if (!Selected)
    {
      return {Named ? Accessible ? ExecutionStatus::InvalidArguments : ExecutionStatus::AccessDenied : ExecutionStatus::UnknownBinding};
    }
    if (Selected->WritableReceiver && !Object.Writable)
    {
      return {ExecutionStatus::ReadOnly};
    }
    if (!Machine)
    {
      return {ExecutionStatus::MissingBody};
    }
    if (!Machine->types() || Machine->types()->domain() != Types->domain())
    {
      return {ExecutionStatus::ForeignContext};
    }
    const auto &Signature = Types->get(Selected->Signature)->functionDesc();
    std::vector<RuntimeValue> Values;
    Values.push_back(RuntimeValue::fromPointer(ExecutionPointer::fromNative(const_cast<void *>(Object.Data)), Signature.Parameters.front()));
    for (const auto &Argument : Arguments)
    {
      Values.push_back(Argument.Value);
    }
    return Machine->execute(Selected->Function, Values);
  }

  ExecutionPlaceResult Reflection::construct(RuntimeTypeId Type, std::span<const ReflectedValue> Arguments)
  {
    const auto *Layout = Types ? Types->get(Type) : nullptr;
    if (!Layout || Layout->Kind != RuntimeKind::Class)
    {
      return {ExecutionStatus::InvalidArguments};
    }
    bool HasDestructor = false;
    for (const auto &Method : Layout->classDesc().Methods)
    {
      HasDestructor = HasDestructor || Method.Name == "__del__";
      if (Method.Name != "__init__")
      {
        continue;
      }
      const auto Place = Memory.allocateCell(*Layout);
      if (!Place)
      {
        return Place;
      }
      const auto Object = view(Place.Place);
      const auto Result = Object ? invoke(Object.Value, "__init__", Arguments) : RuntimeValueResult{Object.Status};
      if (!Result)
      {
        Memory.release(Place.Place);
        return {Result.Status};
      }
      return Place;
    }
    // Hand-built aggregate IR without lifecycle methods retains its explicit field API.
    if (HasDestructor || Arguments.size() > Layout->classDesc().Fields.size())
    {
      return {ExecutionStatus::InvalidArguments};
    }
    if (Machine && (!Machine->types() || Machine->types()->domain() != Types->domain()))
    {
      return {ExecutionStatus::ForeignContext};
    }
    const auto &Fields = Layout->classDesc().Fields;
    // Validate every supplied/missing argument before running initializer effects.
    for (std::size_t Index = 0; Index < Fields.size(); ++Index)
    {
      if (Index < Arguments.size())
      {
        if (Arguments[Index].Domain != Types->domain())
        {
          return {ExecutionStatus::ForeignContext};
        }
        if (Fields[Index].Visibility != core::VisibilityKind::Public)
        {
          return {ExecutionStatus::AccessDenied};
        }
        if (validateStorageValue(*Fields[Index].Layout, Arguments[Index].Value) != ExecutionStatus::Success)
        {
          return {ExecutionStatus::TypeMismatch};
        }
      }
      else if (Fields[Index].Initializer == InvalidFunction || !Machine)
      {
        return {ExecutionStatus::InvalidArguments};
      }
    }
    std::vector<RuntimeValue> Values;
    for (std::size_t Index = 0; Index < Fields.size(); ++Index)
    {
      if (Index < Arguments.size())
      {
        Values.push_back(Arguments[Index].Value);
      }
      else
      {
        auto Initial = Machine->execute(Fields[Index].Initializer, {});
        if (!Initial)
        {
          return {Initial.Status};
        }
        Values.push_back(std::move(Initial.Value));
      }
    }
    return Memory.allocateCell(*Layout, true, RuntimeValue::fromClass(std::move(Values), Type));
  }
  ExecutionStatus Reflection::destroy(const ObjectView &Object)
  {
    return invoke(Object, "__del__").Status;
  }
} // namespace ink::execution
