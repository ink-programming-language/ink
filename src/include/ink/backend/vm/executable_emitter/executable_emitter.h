#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/core/name/name.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
  class FunctionEmitter;
  class MachineModule;
  class AddressOperand;
  class RegisterOperand;
  class ImmediateOperand;
  enum class RegisterId : std::uint8_t;

  class ExecutableEmitter
  {
    public:
      // The borrowed diagnostic engine must outlive this emitter.
      explicit ExecutableEmitter(const core::DiagnosticEngine &Diagnostics) noexcept;

      [[nodiscard]] ExecutableFile emit(const MachineModule &Module);

      // Borrows the current unbound image; further emission may change its contents.
      const ExecutableFile &getExecutable() const noexcept
      {
        return Executable;
      }

      // Rebuilds the symbol map from all functions and globals, using zero offsets until image layout is known.
      void scanUnResolvedSymbol(const MachineModule &Module);

      // Appends Count bytes from a readable source outside the image; zero Count permits a null source.
      void pad(const void *Src, std::size_t Count);

      // Appends Count zero bytes to the image.
      void padZero(std::size_t Count);

      // Appends aligned global storage and resolves each global to its image offset.
      void padGlobals(const MachineModule &Module);

      // Finalizes forward references after all globals and functions are laid out; every referenced VM symbol must have an image offset.
      void resolveStaticRelocations();

      // Appends a register identifier and its signed byte offset without dereferencing memory.
      void padOperand(const RegisterOperand &Operand);

      // Appends a numeric immediate or a 64-bit symbol address placeholder with a loading relocation; direct-call targets use opcode-specific encoding.
      void padOperand(const ImmediateOperand &Operand, std::uint32_t Width);

      // Appends a nine-byte address operand; unresolved VM symbols reserve a zero displacement and queue a static relocation.
      void padOperand(const AddressOperand &Operand, std::uint64_t InstructionOffset);

    private:
      friend class FunctionEmitter;

      enum class StaticRelocationKind : std::uint8_t
      {
        PCRelative,
        ImageBaseAddress,
      };

      struct StaticRelocationSymbol
      {
          std::uint64_t ImageOffset = 0;
          core::Name Name = core::NAME_None;
          std::uint64_t InstructionOffset = 0;
          std::int64_t Addend = 0;
          StaticRelocationKind Kind = StaticRelocationKind::PCRelative;
      };

      void padInteger(std::uint64_t Bits, std::uint32_t Width);
      void padRegister(RegisterId Register, std::int64_t ByteOffset);
      std::uint64_t resolveSymbolOffset(std::uint64_t SymbolOffset, std::int64_t Addend) const;
      std::uint64_t resolvePCRelativeOffset(std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset) const;
      std::uint32_t getOrAddNativeSymbol(const DynamicImportSymbol &Import);
      void padUnresolvedSymbol(core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset);
      void padPCRelativeSymbol(std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset);
      void padNativeSymbol(const DynamicImportSymbol &Import, std::int64_t Addend);

      const core::DiagnosticEngine &Diagnostics;
      ExecutableFile Executable;
      std::unordered_map<core::Name, std::uint64_t> UnresolvedSymbol;
      std::unordered_map<core::Name, std::uint64_t> ResolvedSymbol;
      std::unordered_map<core::Name, DynamicImportSymbol> NativeSymbols;
      std::vector<StaticRelocationSymbol> PendingStaticRelocationSymbol;
  };
} // namespace ink::vm
