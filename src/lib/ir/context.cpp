#include "ink/ir/context.h"
#include "ink/ir/lifetime_observer.h"

#include <algorithm>
#include <limits>

namespace ink::ir
{
  LifetimeObserver::LifetimeObserver(IRContext &Context)
      : Context(Context)
  {
    Context.Observers.push_back(this);
  }

  LifetimeObserver::~LifetimeObserver()
  {
    std::erase(Context.Observers, this);
  }

  void IRContext::notifyChanged() const noexcept
  {
    if (Revision == std::numeric_limits<std::uint64_t>::max())
    {
      PANIC("IR context revision exhausted");
    }
    ++Revision;
  }

  void IRContext::notifyDestroyed(Value &Target) const noexcept
  {
    notifyChanged();
    for (LifetimeObserver *Observer : Observers)
    {
      Observer->valueDestroyed(Target);
    }
  }

  void IRContext::notifyDestroyed(Decl &Target) const noexcept
  {
    notifyChanged();
    for (LifetimeObserver *Observer : Observers)
    {
      Observer->declDestroyed(Target);
    }
  }
} // namespace ink::ir
