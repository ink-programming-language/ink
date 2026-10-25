#ifndef INK_IR_SERIALIZATION_LOAD_TRANSACTION_H
#define INK_IR_SERIALIZATION_LOAD_TRANSACTION_H

#include <cstddef>

namespace ink::ir
{
  class IRContext;

  namespace detail
  {
    // Restores pooled IR when a decoder rejects its private, unpublished module.
    // Declare the pending module owner before this guard so rollback runs before
    // module destruction. After rollback that module may only be destroyed: its
    // borrowed pool objects have expired. Current module destructors do not read
    // those objects, and unregister nominal declarations after clearing bodies.
    // No newly created object may escape to existing IR before commit(). All
    // context access requires external synchronization throughout the transaction.
    class IRLoadTransaction final
    {
      public:
        explicit IRLoadTransaction(IRContext &Context) noexcept;
        ~IRLoadTransaction() noexcept;

        IRLoadTransaction(const IRLoadTransaction &) = delete;
        IRLoadTransaction &operator=(const IRLoadTransaction &) = delete;
        IRLoadTransaction(IRLoadTransaction &&) = delete;
        IRLoadTransaction &operator=(IRLoadTransaction &&) = delete;

        void commit() noexcept;

      private:
        IRContext &Context;
        std::size_t TypeCount;
        std::size_t ConstantCount;
        bool Committed = false;
    };
  } // namespace detail
} // namespace ink::ir

#endif
