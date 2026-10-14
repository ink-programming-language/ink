#ifndef INK_IR_TYPE_NOMINAL_TYPE_METHOD_H
#define INK_IR_TYPE_NOMINAL_TYPE_METHOD_H

#include "ink/core/name/name.h"
#include "ink/ir/function/function.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace ink::ir
{
  // Abstract methods participate in virtual dispatch and require an implementation in a concrete type.
  enum class MethodDispatch : std::uint8_t
  {
    Direct,
    Virtual,
    Abstract,
  };

  struct MethodVirtualInfo
  {
      MethodDispatch Dispatch = MethodDispatch::Direct;
      bool Override = false;
      bool Final = false;
      // An optional zero-based virtual table index, not a byte offset; absence means unassigned.
      // Slot allocation, table construction and dispatch lowering are handled by later stages.
      std::optional<std::uint32_t> Slot;
  };

  // Exclusively owns a member function and its dispatch properties; the signature remains borrowed.
  // Moving a method transfers ownership without changing the function, parameter or block addresses.
  class Method final
  {
    public:
      explicit Method(std::unique_ptr<Function> Callable, MethodVirtualInfo VirtualInfo = {}) noexcept;
      ~Method() noexcept;

      Method(const Method &) = delete;
      Method &operator=(const Method &) = delete;
      Method(Method &&) noexcept;
      Method &operator=(Method &&) noexcept;

      // Empty and moved-from methods have no function or type, report NAME_None, and are invalid.
      auto *function(this auto &&Self) noexcept
      {
        return Self.Callable ? &std::forward_like<decltype(Self) &>(*Self.Callable) : nullptr;
      }

      core::Name name() const noexcept;
      const FunctionType *functionType() const noexcept;
      const MethodVirtualInfo &virtualInfo() const noexcept;
      bool isVirtual() const noexcept;
      bool isAbstract() const noexcept;
      bool isOverride() const noexcept;
      bool isFinal() const noexcept;
      std::optional<std::uint32_t> virtualSlot() const noexcept;

      // Checks the function's current name and local dispatch consistency, without resolving inheritance or virtual tables.
      bool valid() const noexcept;

    private:
      std::unique_ptr<Function> Callable;
      MethodVirtualInfo VirtualInfo;
  };
} // namespace ink::ir

#endif
