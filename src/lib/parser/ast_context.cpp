#include "ink/parser/ast_context.h"
#include <algorithm>
namespace ink::parser
{
  ASTContext::~ASTContext()
  {
    rollback({0, 0, 0, 0});
  }
  void *ASTContext::allocate(std::size_t Size, std::size_t Alignment)
  {
    std::size_t Offset = Blocks.empty() ? 0 : (Blocks.back().Used + Alignment - 1) & ~(Alignment - 1);
    if (Blocks.empty() || Blocks.back().Alignment < Alignment || Offset > Blocks.back().Size || Size > Blocks.back().Size - Offset)
    {
      const std::size_t Capacity = std::max<std::size_t>(65536, Size);
      Alignment = std::max(Alignment, alignof(std::max_align_t));
      Blocks.push_back({::operator new(Capacity, std::align_val_t(Alignment)), Capacity, Alignment, 0});
      Offset = 0;
    }
    Block &Storage = Blocks.back();
    Bytes += Offset - Storage.Used + Size;
    Storage.Used = Offset + Size;
    return static_cast<std::byte *>(Storage.Data) + Offset;
  }
  ASTCheckpoint ASTContext::checkpoint() const noexcept
  {
    return {Blocks.size(), Blocks.empty() ? 0 : Blocks.back().Used, Destructors.size(), Bytes};
  }
  void ASTContext::rollback(ASTCheckpoint Checkpoint)
  {
    assert(Checkpoint.Blocks <= Blocks.size() && Checkpoint.Destructors <= Destructors.size());
    while (Destructors.size() > Checkpoint.Destructors)
    {
      const Destructor Entry = Destructors.back();
      Destructors.pop_back();
      Entry.Run(Entry.Data, Entry.Count);
    }
    while (Blocks.size() > Checkpoint.Blocks)
    {
      ::operator delete(Blocks.back().Data, std::align_val_t(Blocks.back().Alignment));
      Blocks.pop_back();
    }
    if (!Blocks.empty())
    {
      Blocks.back().Used = Checkpoint.Offset;
    }
    Bytes = Checkpoint.Bytes;
  }
} // namespace ink::parser
