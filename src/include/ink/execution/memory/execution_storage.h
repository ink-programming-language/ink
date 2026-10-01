#ifndef INK_EXECUTION_MEMORY_EXECUTION_STORAGE_H
#define INK_EXECUTION_MEMORY_EXECUTION_STORAGE_H

#include "ink/execution/value/execution_value.h"

#include <string_view>
#include <utility>
#include <vector>

namespace ink::execution
{
  // Storage is owned exclusively by ExecutionHeap. Borrowed pointers obtained
  // from a storage reference are valid only until its explicit release.
  class ExecutionStorage : public ExecutionObject
  {
    protected:
      explicit ExecutionStorage(ExecutionObjectKind Kind) noexcept
          : ExecutionObject(Kind)
      {
      }
  };

  class ExecutionCell final : public ExecutionStorage
  {
    public:
      const ir::Type &type() const noexcept
      {
        return *ValueType;
      }

      const ExecutionValueRef &value() const noexcept
      {
        return Value;
      }

      bool writable() const noexcept
      {
        return Writable;
      }

      bool runtime() const noexcept
      {
        return Runtime;
      }

    private:
      ExecutionCell(const ir::Type &Type, bool Writable, ExecutionValueRef Initial, bool Runtime)
          : ExecutionStorage(ExecutionObjectKind::Cell),
            ValueType(&Type),
            Value(std::move(Initial)),
            Writable(Writable),
            Runtime(Runtime)
      {
      }

      const ir::Type *ValueType;
      ExecutionValueRef Value;
      bool Writable;
      bool Runtime;

      friend class ExecutionHeap;
  };

  // Fixed byte storage includes the optional trailing NUL byte. It never moves
  // while allocated; a language pointer contains only its non-owning identity.
  class ExecutionBuffer final : public ExecutionStorage
  {
    public:
      char *data() noexcept
      {
        return Bytes.data();
      }

      const char *data() const noexcept
      {
        return Bytes.data();
      }

      std::size_t size() const noexcept
      {
        return Bytes.size();
      }

    private:
      ExecutionBuffer(std::string_view Source, bool Terminate)
          : ExecutionStorage(ExecutionObjectKind::Buffer),
            Bytes(Source.begin(), Source.end())
      {
        if (Terminate)
        {
          Bytes.push_back('\0');
        }
      }

      std::vector<char> Bytes;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
