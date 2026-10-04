#ifndef INK_EXECUTION_SUPPORT_EXECUTION_OBJECT_H
#define INK_EXECUTION_SUPPORT_EXECUTION_OBJECT_H

namespace ink::execution
{
  enum class ExecutionObjectKind
  {
    VoidValue,
    BooleanValue,
    IntegerValue,
    FloatValue,
    StringValue,
    PointerValue,
    FunctionValue,
    ArrayValue,
    ClassValue,
    Cell,
    Buffer,
  };

  // Runtime objects have stable addresses. Values use RAII ownership; storage
  // has an explicit heap owner and is never kept alive by a language pointer.
  class ExecutionObject
  {
    public:
      virtual ~ExecutionObject() = default;
      ExecutionObject(const ExecutionObject &) = delete;
      ExecutionObject &operator=(const ExecutionObject &) = delete;
      ExecutionObject(ExecutionObject &&) = delete;
      ExecutionObject &operator=(ExecutionObject &&) = delete;

      ExecutionObjectKind objectKind() const noexcept
      {
        return Kind;
      }

    protected:
      explicit ExecutionObject(ExecutionObjectKind Kind) noexcept
          : Kind(Kind)
      {
      }

    private:
      ExecutionObjectKind Kind;
  };
} // namespace ink::execution

#endif
