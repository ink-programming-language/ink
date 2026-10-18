#ifndef INK_CORE_SUPPORT_RETURN_MACROS_H
#define INK_CORE_SUPPORT_RETURN_MACROS_H

// Evaluates Value once and evaluates ReturnValue only when returning; condition declarations retain their if scope.
#define RETURN_IF(Value, ReturnValue) \
  do \
  { \
    if (Value) \
    { \
      return (ReturnValue); \
    } \
  } while (false)

// Propagates a null result without adding a diagnostic or changing the variable's scope.
#define RETURN_IF_NULL(Value, FailureValue) \
  do \
  { \
    if ((Value) == nullptr) \
    { \
      return (FailureValue); \
    } \
  } while (false)

#endif
