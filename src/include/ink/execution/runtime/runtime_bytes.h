#ifndef INK_EXECUTION_RUNTIME_RUNTIME_BYTES_H
#define INK_EXECUTION_RUNTIME_RUNTIME_BYTES_H

#include "ink/core/core_define.h"
#include "ink/execution/runtime/runtime_type.h"
#include "ink/execution/value/execution_integer.h"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>
#include <memory>
#include <new>

namespace ink::execution
{
  struct RuntimeBytesDeleter
  {
      std::align_val_t Alignment = std::align_val_t{alignof(std::max_align_t)};

      void operator()(std::byte *Data) const noexcept
      {
        ::operator delete[](Data, Alignment);
      }
  };

  using RuntimeBytes = std::unique_ptr<std::byte[], RuntimeBytesDeleter>;

  inline RuntimeBytes allocateRuntimeBytes(std::size_t Size, std::size_t Alignment)
  {
    const auto Align = std::align_val_t{std::max(Alignment, alignof(std::max_align_t))};
    RuntimeBytes Result(static_cast<std::byte *>(::operator new[](std::max<std::size_t>(Size, 1), Align)), RuntimeBytesDeleter{Align});
    std::memset(Result.get(), 0, std::max<std::size_t>(Size, 1));
    return Result;
  }

  // A slot is a view into its frame's native-layout bytes. Metadata never occurs
  // inside the language value; aggregate fields are addressed by shared layouts.
  struct RuntimeSlot
  {
      std::byte *Data = nullptr;
      const TypeDesc *Layout = nullptr;
      bool Initialized = false;

      FORCE_INLINE std::uint64_t bits() const noexcept
      {
        const std::size_t Width = Layout->Kind == RuntimeKind::Integer ? (static_cast<std::size_t>(Layout->bitWidth()) + 7) / 8 : Layout->Size;
        assert(Width <= sizeof(std::uint64_t));
        std::uint64_t Result = 0;
        auto *Bytes = reinterpret_cast<std::byte *>(&Result);
        std::memcpy(Bytes + (std::endian::native == std::endian::big ? sizeof(Result) - Width : 0), Data, Width);
        return Result;
      }

      FORCE_INLINE void setBits(std::uint64_t Bits) noexcept
      {
        const std::size_t Width = Layout->Kind == RuntimeKind::Integer ? (static_cast<std::size_t>(Layout->bitWidth()) + 7) / 8 : Layout->Size;
        assert(Width <= sizeof(Bits));
        const auto *Bytes = reinterpret_cast<const std::byte *>(&Bits);
        std::memset(Data, 0, Layout->Size);
        std::memcpy(Data, Bytes + (std::endian::native == std::endian::big ? sizeof(Bits) - Width : 0), Width);
        Initialized = true;
      }

      FORCE_INLINE void *pointer() const noexcept
      {
        void *Address;
        std::memcpy(&Address, Data, sizeof(Address));
        return Address;
      }

      FORCE_INLINE void setPointer(void *Address) noexcept
      {
        std::memcpy(Data, &Address, sizeof(Address));
        Initialized = true;
      }

      FORCE_INLINE void copyFrom(const void *Source) noexcept
      {
        std::memmove(Data, Source, Layout->Size);
        Initialized = true;
      }

      ExecutionInteger integer() const
      {
        if (Layout->bitWidth() <= 64)
        {
          return ExecutionInteger(Layout->bitWidth(), bits());
        }
        std::vector<std::uint64_t> Words((static_cast<std::size_t>(Layout->bitWidth()) + 63) / 64);
        const std::size_t Count = (static_cast<std::size_t>(Layout->bitWidth()) + 7) / 8;
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          const std::size_t Byte = std::endian::native == std::endian::little ? Index : Count - 1 - Index;
          Words[Index / 8] |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(Data[Byte])) << ((Index % 8) * 8);
        }
        return ExecutionInteger(ir::IntegerBits(Layout->bitWidth(), Words));
      }

      void setInteger(const ExecutionInteger &Value)
      {
        if (Layout->bitWidth() <= 64)
        {
          setBits(Value.lowWord());
          return;
        }
        const auto Bits = Value.bits();
        const std::size_t Count = (static_cast<std::size_t>(Layout->bitWidth()) + 7) / 8;
        std::memset(Data, 0, Layout->Size);
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          const std::size_t Byte = std::endian::native == std::endian::little ? Index : Count - 1 - Index;
          Data[Byte] = static_cast<std::byte>(Bits.words()[Index / 8] >> ((Index % 8) * 8));
        }
        Initialized = true;
      }
  };
} // namespace ink::execution

#endif
