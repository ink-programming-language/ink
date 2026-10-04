#ifndef INK_EXECUTION_SUPPORT_RUNTIME_KIND_H
#define INK_EXECUTION_SUPPORT_RUNTIME_KIND_H

namespace ink::execution
{
  // Value category shared by semantic execution values, VM values and reflection metadata.
  enum class RuntimeKind
  {
    Invalid,
    Void,
    Boolean,
    Integer,
    Float,
    String,
    Pointer,
    Function,
    Array,
    Class,
  };
} // namespace ink::execution

#endif
