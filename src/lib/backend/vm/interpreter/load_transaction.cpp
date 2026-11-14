#include "load_transaction.h"

#include <utility>

namespace ink::vm
{
  Interpreter::LoadTransaction::LoadTransaction(Interpreter &Owner)
      : Owner(Owner),
        SavedCodeSize(Owner.CodeSegmentSize),
        SavedExportSymbolMap(std::move(Owner.ExportSymbolMap)),
        SavedNextLibraryHandle(Owner.NextLibraryHandle)
  {
    Owner.ExportSymbolMap = SavedExportSymbolMap;
  }

  Interpreter::LoadTransaction::~LoadTransaction()
  {
    if (Committed)
    {
      return;
    }
    Owner.CodeSegmentSize = SavedCodeSize;
    Owner.ExportSymbolMap.swap(SavedExportSymbolMap);
    for (auto Position = Owner.Libraries.begin(); Position != Owner.Libraries.end();)
    {
      if (Position->first >= SavedNextLibraryHandle)
      {
        Position = Owner.Libraries.erase(Position);
      }
      else
      {
        ++Position;
      }
    }
    Owner.NextLibraryHandle = SavedNextLibraryHandle;
  }

  void Interpreter::LoadTransaction::commit() noexcept
  {
    Committed = true;
  }
} // namespace ink::vm
