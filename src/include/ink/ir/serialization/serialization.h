#ifndef INK_IR_SERIALIZATION_SERIALIZATION_H
#define INK_IR_SERIALIZATION_SERIALIZATION_H

#include "ink/ir/module/module.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ink::ir
{
  enum class IRSerializationErrorCode : std::uint8_t
  {
    InvalidMagic,
    UnsupportedVersion,
    UnsupportedFeature,
    TargetMismatch,
    TruncatedInput,
    InvalidSyntax,
    InvalidReference,
    InvalidIR,
    UnsupportedKind,
    ResourceLimit,
  };

  // Offset is a byte position in the input when available, otherwise zero.
  // This is a recoverable codec error; formatting and process output belong to the caller.
  struct IRSerializationError
  {
      IRSerializationErrorCode Code = IRSerializationErrorCode::InvalidIR;
      std::size_t Offset = 0;
      std::string Message;
  };

  template <typename T>
  using IRSerializationResult = std::expected<T, IRSerializationError>;

  struct IRReadOptions
  {
      std::size_t MaxBytes = 256U * 1024U * 1024U;
      std::size_t MaxRecords = 4U * 1024U * 1024U;
      std::size_t MaxOperands = 16U * 1024U * 1024U;
      std::size_t MaxStringBytes = 64U * 1024U * 1024U;
      std::size_t MaxDepth = 256;
  };

  struct IRTextWriteOptions
  {
      std::size_t MaxBytes = 256U * 1024U * 1024U;
  };

  struct IRBinaryWriteOptions
  {
      std::size_t MaxBytes = 256U * 1024U * 1024U;
  };

  class IRTextWriter final
  {
    public:
      // Writes structured assembly with the configured INK_IR_VERSION header.
      static IRSerializationResult<std::string> write(const Module &ModuleValue, const IRTextWriteOptions &Options = {});
  };

  class IRTextReader final
  {
    public:
      // Reads structured assembly matching the configured INK_IR_VERSION.
      static IRSerializationResult<std::unique_ptr<Module>> read(IRContext &Context, std::string_view Text, const IRReadOptions &Options = {});
  };

  class IRBinaryWriter final
  {
    public:
      static IRSerializationResult<std::vector<std::byte>> write(const Module &ModuleValue, const IRBinaryWriteOptions &Options = {});
  };

  class IRBinaryReader final
  {
    public:
      static IRSerializationResult<std::unique_ptr<Module>> read(IRContext &Context, std::span<const std::byte> Bytes, const IRReadOptions &Options = {});
  };

  class IRVerifier final
  {
    public:
      static IRSerializationResult<void> verify(const Module &ModuleValue);
  };

  IRSerializationResult<std::string> writeText(const Module &ModuleValue, const IRTextWriteOptions &Options = {});
  IRSerializationResult<std::vector<std::byte>> writeBinary(const Module &ModuleValue, const IRBinaryWriteOptions &Options = {});

  // Context must outlive the returned module and every user of its pooled objects.
  // Loading requires exclusive access to Context. A failed load rolls back newly pooled IR.
  // Successful modules follow the normal IRContext lifetime contract; module unloading is unsupported.
  IRSerializationResult<std::unique_ptr<Module>> readText(IRContext &Context, std::string_view Text, const IRReadOptions &Options = {});
  IRSerializationResult<std::unique_ptr<Module>> readBinary(IRContext &Context, std::span<const std::byte> Bytes, const IRReadOptions &Options = {});
} // namespace ink::ir

#endif
