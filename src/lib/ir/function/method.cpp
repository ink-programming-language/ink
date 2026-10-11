#include "ink/ir/function/method.h"

namespace ink::ir
{
  Method::Method(core::Name Name, const FunctionType &Signature, MethodVirtualInfo VirtualInfo) noexcept
      : Name(Name),
        Signature(&Signature),
        VirtualInfo(VirtualInfo)
  {
  }

  core::Name Method::name() const noexcept
  {
    return Name;
  }

  const FunctionType *Method::functionType() const noexcept
  {
    return Signature;
  }

  const MethodVirtualInfo &Method::virtualInfo() const noexcept
  {
    return VirtualInfo;
  }

  bool Method::isVirtual() const noexcept
  {
    return VirtualInfo.Dispatch == MethodDispatch::Virtual || VirtualInfo.Dispatch == MethodDispatch::Abstract;
  }

  bool Method::isAbstract() const noexcept
  {
    return VirtualInfo.Dispatch == MethodDispatch::Abstract;
  }

  bool Method::isOverride() const noexcept
  {
    return VirtualInfo.Override;
  }

  bool Method::isFinal() const noexcept
  {
    return VirtualInfo.Final;
  }

  std::optional<std::uint32_t> Method::virtualSlot() const noexcept
  {
    return VirtualInfo.Slot;
  }

  bool Method::valid() const noexcept
  {
    if (!Name.valid())
    {
      return false;
    }

    switch (VirtualInfo.Dispatch)
    {
      case MethodDispatch::Direct:
        return !VirtualInfo.Override && !VirtualInfo.Final && !VirtualInfo.Slot.has_value();
      case MethodDispatch::Virtual:
        return true;
      case MethodDispatch::Abstract:
        return !VirtualInfo.Final;
      default:
        return false;
    }
  }
} // namespace ink::ir
