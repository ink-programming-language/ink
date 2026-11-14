#pragma once

#include "ink/backend/vm/interpreter/interpreter.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace ink::vm
{
  class Interpreter::LoadTransaction
  {
    public:
      explicit LoadTransaction(Interpreter &Owner);
      ~LoadTransaction();

      LoadTransaction(const LoadTransaction &) = delete;
      LoadTransaction &operator=(const LoadTransaction &) = delete;
      LoadTransaction(LoadTransaction &&) = delete;
      LoadTransaction &operator=(LoadTransaction &&) = delete;

      void commit() noexcept;

    private:
      Interpreter &Owner;
      std::size_t SavedCodeSize;
      std::unordered_map<core::Name, std::uint64_t> SavedExportSymbolMap;
      std::uint32_t SavedNextLibraryHandle;
      bool Committed = false;
  };
} // namespace ink::vm
