#ifndef INK_CORE_SINGLETON_H
#define INK_CORE_SINGLETON_H

namespace ink::core
{
  // Derive from Singleton<T> and make it a friend of T's private constructor/destructor.
  template <typename T>
  class Singleton
  {
    public:
      // Initialization is thread-safe; access to T's state requires its own synchronization.
      static T &instance()
      {
        static T Instance;
        return Instance;
      }

      Singleton(const Singleton &) = delete;
      Singleton &operator=(const Singleton &) = delete;
      Singleton(Singleton &&) = delete;
      Singleton &operator=(Singleton &&) = delete;

    protected:
      Singleton() noexcept = default;
      ~Singleton() = default;
  };
} // namespace ink::core

#endif
