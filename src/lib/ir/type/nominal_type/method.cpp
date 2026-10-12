#include "ink/ir/type/nominal_type/method.h"

#include <utility>

namespace ink::ir
{
  Method::Method(std::unique_ptr<Function> Callable, MethodVirtualInfo VirtualInfo) noexcept
      : Callable(std::move(Callable)),
        VirtualInfo(VirtualInfo)
  {
  }

  Method::~Method() noexcept = default;

  Method::Method(Method &&) noexcept = default;

  Method &Method::operator=(Method &&) noexcept = default;

  Function *Method::function() noexcept
  {
    return Callable.get();
  }

  const Function *Method::function() const noexcept
  {
    return Callable.get();
  }

  core::Name Method::name() const noexcept
  {
    return Callable ? Callable->name() : core::NAME_None;
  }

  const FunctionType *Method::functionType() const noexcept
  {
    return Callable ? Callable->functionType() : nullptr;
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
    if (!Callable || !Callable->hasName())
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
