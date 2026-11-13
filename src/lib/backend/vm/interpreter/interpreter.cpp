#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/config/config_manager.h"

#include <utility>

namespace ink::vm
{
  class Interpreter::LoadTransaction
  {
    public:
      explicit LoadTransaction(Interpreter &Owner)
          : Owner(Owner),
            SavedCodeSegment(std::move(Owner.CodeSegment)),
            SavedExportSymbolMap(std::move(Owner.ExportSymbolMap)),
            SavedNextLibraryHandle(Owner.NextLibraryHandle)
      {
        // Keep the original storage intact so failure also restores borrowed addresses and capacity.
        Owner.CodeSegment.reserve(SavedCodeSegment.capacity());
        Owner.CodeSegment.assign(SavedCodeSegment.begin(), SavedCodeSegment.end());
        Owner.ExportSymbolMap = SavedExportSymbolMap;
      }

      ~LoadTransaction()
      {
        if (Committed)
        {
          return;
        }
        Owner.CodeSegment.swap(SavedCodeSegment);
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

      LoadTransaction(const LoadTransaction &) = delete;
      LoadTransaction &operator=(const LoadTransaction &) = delete;
      LoadTransaction(LoadTransaction &&) = delete;
      LoadTransaction &operator=(LoadTransaction &&) = delete;

      void commit() noexcept
      {
        Committed = true;
      }

    private:
      Interpreter &Owner;
      std::vector<std::uint8_t> SavedCodeSegment;
      std::unordered_map<core::Name, std::uint64_t> SavedExportSymbolMap;
      std::uint32_t SavedNextLibraryHandle;
      bool Committed = false;
  };

  Interpreter::Interpreter() noexcept
      : Loader(*this)
  {
    CodeSegment.reserve(core::ConfigManager::getSize<core::ConfigKind::VMInitialCodeSegmentBytes>());
  }

  std::uint32_t Interpreter::loadLibrary(ExecutableFile Executable)
  {
    if (NextLibraryHandle == InvalidLibraryHandle)
    {
      return InvalidLibraryHandle;
    }
    LoadTransaction Transaction(*this);
    const std::uint32_t Handle = NextLibraryHandle++;
    auto &Library = Libraries.try_emplace(Handle, LibraryDescriptor{static_cast<std::uint64_t>(CodeSegment.size()), std::move(Executable)}).first->second;
    const auto &Image = Library.ExecutableFile.getImage();
    CodeSegment.insert(CodeSegment.end(), Image.begin(), Image.end());
    if (!Loader.loadLibrary(CodeSegment, Library.ImageBase, Library.ExecutableFile))
    {
      return InvalidLibraryHandle;
    }
    loadExportSymbol(Library);
    Transaction.commit();
    return Handle;
  }

  void Interpreter::loadExportSymbol(const LibraryDescriptor &Library)
  {
    for (const auto &Symbol : Library.ExecutableFile.getExportSymbols())
    {
      ExportSymbolMap.insert_or_assign(Symbol.Name, Library.ImageBase + Symbol.Offset);
    }
  }

  const ExecutableFile *Interpreter::findExecutable(std::uint32_t Handle) const noexcept
  {
    const auto Position = Libraries.find(Handle);
    return Position == Libraries.end() ? nullptr : &Position->second.ExecutableFile;
  }
} // namespace ink::vm
