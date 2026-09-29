#include "ink/ir/context.h"
#include "ink/ir/lifetime_observer.h"

#include <algorithm>

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

  void IRContext::notifyDestroyed(Value &Target) const noexcept
  {
    for (LifetimeObserver *Observer : Observers)
    {
      Observer->valueDestroyed(Target);
    }
  }

  void IRContext::notifyDestroyed(Decl &Target) const noexcept
  {
    for (LifetimeObserver *Observer : Observers)
    {
      Observer->declDestroyed(Target);
    }
  }
} // namespace ink::ir
