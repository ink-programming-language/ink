#pragma once

namespace ink::vm
{
  class ExecutableFile;
  class Interpreter;

  class Loader
  {
    public:
      // The owning interpreter must outlive this loader.
      explicit Loader(vm::Interpreter &Interpreter) noexcept;

      // Loading is not implemented yet; the executable is owned by the interpreter.
      void loadLibrary(ExecutableFile &Executable);

    private:
      vm::Interpreter &Interpreter;
  };
} // namespace ink::vm
