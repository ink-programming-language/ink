#pragma once

namespace ink::vm
{
  class ExecutableEmitter;

  class FunctionEmitter
  {
    public:
      // The borrowed executable emitter must outlive this emitter.
      explicit FunctionEmitter(ExecutableEmitter &Emitter) noexcept;

    private:
      ExecutableEmitter &Emitter;
  };
} // namespace ink::vm
