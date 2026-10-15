#ifndef INK_SEMANTIC_ANALYZER_BLOCK_DEPTH_GUARD_H
#define INK_SEMANTIC_ANALYZER_BLOCK_DEPTH_GUARD_H

#include <cstddef>

namespace ink::semantic
{
  // Counts one recursive analysis level and restores the caller's depth on every exit path.
  class BlockDepthGuard final
  {
    public:
      explicit BlockDepthGuard(std::size_t &Depth) noexcept
          : Depth(Depth),
            SavedDepth(Depth)
      {
        ++Depth;
      }

      ~BlockDepthGuard() noexcept
      {
        Depth = SavedDepth;
      }

      BlockDepthGuard(const BlockDepthGuard &) = delete;
      BlockDepthGuard &operator=(const BlockDepthGuard &) = delete;
      BlockDepthGuard(BlockDepthGuard &&) = delete;
      BlockDepthGuard &operator=(BlockDepthGuard &&) = delete;

    private:
      std::size_t &Depth;
      std::size_t SavedDepth;
  };
} // namespace ink::semantic

#endif
