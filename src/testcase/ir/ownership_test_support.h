#ifndef INK_TESTCASE_IR_OWNERSHIP_TEST_SUPPORT_H
#define INK_TESTCASE_IR_OWNERSHIP_TEST_SUPPORT_H

#include <memory>
#include <vector>

namespace ink::ir::test
{
  template <typename ValueType>
  std::vector<ValueType *> borrowedPointers(const std::vector<std::unique_ptr<ValueType>> &Owners)
  {
    std::vector<ValueType *> Result;
    Result.reserve(Owners.size());
    for (const auto &Owner : Owners)
    {
      Result.push_back(Owner.get());
    }
    return Result;
  }
} // namespace ink::ir::test

#endif
