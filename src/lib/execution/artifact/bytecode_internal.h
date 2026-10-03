#ifndef INK_EXECUTION_ARTIFACT_BYTECODE_INTERNAL_H
#define INK_EXECUTION_ARTIFACT_BYTECODE_INTERNAL_H

#include "ink/execution/artifact/bytecode_artifact.h"

#include <string_view>

namespace ink::execution::artifact_detail
{
  class Budget
  {
    public:
      explicit Budget(BytecodeLimits Limits)
          : Limits(Limits)
      {
      }

      bool records(std::size_t Count, std::size_t Size)
      {
        return add(Records, Count, Limits.MaxRecords) && (Size == 0 || Count <= Limits.MaxAllocationBytes / Size) && add(Allocation, Count * Size, Limits.MaxAllocationBytes);
      }

      bool bytes(std::size_t Count)
      {
        return add(Bytes, Count, Limits.MaxBytes) && add(Allocation, Count, Limits.MaxAllocationBytes);
      }

      bool allocation(std::size_t Count, std::size_t Size = 1)
      {
        return (Size == 0 || Count <= Limits.MaxAllocationBytes / Size) && add(Allocation, Count * Size, Limits.MaxAllocationBytes);
      }

      bool string(std::string_view Value)
      {
        return add(Strings, Value.size(), Limits.MaxStringBytes) && bytes(Value.size());
      }

    private:
      static bool add(std::size_t &Current, std::size_t Amount, std::size_t Maximum)
      {
        if (Current > Maximum || Amount > Maximum - Current)
        {
          return false;
        }
        Current += Amount;
        return true;
      }

      BytecodeLimits Limits;
      std::size_t Records = 0;
      std::size_t Bytes = 0;
      std::size_t Strings = 0;
      std::size_t Allocation = 0;
  };

  BytecodeResult accountArtifact(const BytecodeArtifact &Artifact, Budget &Usage);
  bool accountSymbolKey(const BytecodeSymbolIdentity &Identity, Budget &Usage);
  BytecodeResult typeIdentities(const RuntimeTypeTable &Types, BytecodeLimits Limits, std::vector<std::string> &Identities, Budget *SharedUsage = nullptr);
} // namespace ink::execution::artifact_detail

#endif
