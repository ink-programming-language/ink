#ifndef INK_EXECUTION_REFLECTION_REFLECTION_H
#define INK_EXECUTION_REFLECTION_REFLECTION_H

#include "ink/execution/memory/execution_memory_manager.h"

namespace ink::execution
{
  class ExecutionMachine;

  // Views borrow object bytes and retain their type table. External bytes must
  // remain alive; managed views additionally retain an expiring storage identity.
  struct ObjectView
  {
      const void *Data = nullptr;
      RuntimeTypeId Type = InvalidRuntimeType;
      std::shared_ptr<const RuntimeTypeTable> Types;
      bool Writable = false;
      ExecutionStorageRef Storage;
  };

  struct ObjectViewResult
  {
      ExecutionStatus Status = ExecutionStatus::InvalidArguments;
      ObjectView Value;

      explicit operator bool() const noexcept
      {
        return Status == ExecutionStatus::Success;
      }
  };

  struct ReflectedValue
  {
      RuntimeValue Value;
      std::shared_ptr<const RuntimeTypeDomain> Domain;
  };

  // The memory manager outlives this service, its constructed objects and views.
  // Construction returns a managed place; destroy lifecycle objects before releasing their storage.
  class Reflection final
  {
    public:
      Reflection(std::shared_ptr<const RuntimeTypeTable> Types, ExecutionMemoryManager &Memory, ExecutionMachine *Machine = nullptr);
      ReflectedValue value(RuntimeValue Value) const;
      ObjectViewResult view(ExecutionPlace Place) const;
      ObjectViewResult view(RuntimeTypeId Type, std::span<std::byte> Bytes) const;
      ObjectViewResult view(RuntimeTypeId Type, std::span<const std::byte> Bytes) const;
      ObjectViewResult field(const ObjectView &Object, std::string_view Name) const;
      ObjectViewResult element(const ObjectView &Object, std::size_t Index) const;
      RuntimeValueResult read(const ObjectView &Object) const;
      ExecutionStatus write(const ObjectView &Object, const ReflectedValue &Value);
      RuntimeValueResult getField(const ObjectView &Object, std::string_view Name) const;
      ExecutionStatus setField(const ObjectView &Object, std::string_view Name, const ReflectedValue &Value);
      RuntimeValueResult invoke(const ObjectView &Object, std::string_view Name, std::span<const ReflectedValue> Arguments = {});
      ExecutionPlaceResult construct(RuntimeTypeId Type, std::span<const ReflectedValue> Arguments = {});
      ExecutionStatus destroy(const ObjectView &Object);

    private:
      ExecutionStatus validate(const ObjectView &Object) const;
      ObjectViewResult subobject(const ObjectView &Object, RuntimeTypeId Type, std::size_t Offset) const;
      std::shared_ptr<const RuntimeTypeTable> Types;
      ExecutionMemoryManager &Memory;
      ExecutionMachine *Machine;
  };
} // namespace ink::execution

#endif
