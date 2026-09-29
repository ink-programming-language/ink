#ifndef INK_IR_LIFETIME_OBSERVER_H
#define INK_IR_LIFETIME_OBSERVER_H

namespace ink::ir
{
  class IRContext;
  class Value;
  class Decl;

  // RAII subscription for borrowed indexes such as semantic name bindings.
  // Context must outlive the observer. Callbacks only remove borrowed references:
  // they must not register/destroy observers or IR objects, or inspect derived state.
  class LifetimeObserver
  {
    public:
      virtual ~LifetimeObserver();
      LifetimeObserver(const LifetimeObserver &) = delete;
      LifetimeObserver &operator=(const LifetimeObserver &) = delete;
      LifetimeObserver(LifetimeObserver &&) = delete;
      LifetimeObserver &operator=(LifetimeObserver &&) = delete;

    protected:
      explicit LifetimeObserver(IRContext &Context);
      virtual void valueDestroyed(Value &Target) noexcept = 0;
      virtual void declDestroyed(Decl &Target) noexcept = 0;

    private:
      IRContext &Context;

      friend class IRContext;
  };
} // namespace ink::ir

#endif
