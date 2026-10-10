#ifndef INK_IR_TYPE_STRUCT_TYPE_H
#define INK_IR_TYPE_STRUCT_TYPE_H

#include "ink/ir/type/type.h"

#include <memory>

namespace ink::ir
{
  // Carries an explicit completed layout in bytes; member declarations and
  // interface dispatch are supplied by later IR construction stages.
  class StructType final : public Type
  {
    public:
      // Alignment must be a nonzero power of two and divide Size; otherwise returns nullptr.
      static std::unique_ptr<StructType> create(std::uint64_t Size, std::uint64_t Alignment);

      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      StructType(std::uint64_t Size, std::uint64_t Alignment) noexcept;

      std::uint64_t Size = 0;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
