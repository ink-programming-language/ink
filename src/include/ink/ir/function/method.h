#ifndef INK_IR_FUNCTION_METHOD_H
#define INK_IR_FUNCTION_METHOD_H

#include "ink/core/name.h"
#include "ink/ir/type/function_type.h"

#include <cstdint>
#include <optional>

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
      // An optional resolved virtual table slot; zero is a valid slot and absence means unassigned.
      std::optional<std::uint32_t> Slot;
  };

  // Describes a source member's name, signature and dispatch properties without an implementation binding.
  // Only the signature is borrowed and must outlive this metadata; no Function or Module is retained.
  class Method final
  {
    public:
      Method(core::Name Name, const FunctionType &Signature, MethodVirtualInfo VirtualInfo = {}) noexcept;

      core::Name name() const noexcept;
      const FunctionType *functionType() const noexcept;
      const MethodVirtualInfo &virtualInfo() const noexcept;
      bool isVirtual() const noexcept;
      bool isAbstract() const noexcept;
      bool isOverride() const noexcept;
      bool isFinal() const noexcept;
      std::optional<std::uint32_t> virtualSlot() const noexcept;

      // Checks the source name and local dispatch consistency, without resolving inheritance or virtual tables.
      bool valid() const noexcept;

    private:
      core::Name Name;
      const FunctionType *Signature;
      MethodVirtualInfo VirtualInfo;
  };
} // namespace ink::ir

#endif
