#ifndef INK_BACKEND_RESULT_H
#define INK_BACKEND_RESULT_H

#include "ink/ir/ids.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace ink::backend
{
  enum class BackendErrorCode : std::uint8_t
  {
    InvalidTarget,
    NonNativeTarget,
    TargetMachineUnavailable,
    TargetMismatch,
    InvalidIr,
    UnsupportedType,
    UnsupportedOpcode,
    LlvmVerificationFailed,
    OutputAlreadyExists,
    ObjectEmissionFailed,
    IoError,
  };

  const char *backendErrorCodeName(BackendErrorCode Code) noexcept;

  struct BackendError
  {
    BackendErrorCode Code = BackendErrorCode::InvalidIr;
    std::string Message;
    ir::IrFunctionId Function;
    ir::IrBlockId Block;
    ir::IrOperationId Operation;
  };

  template <typename T>
  class BackendResult
  {
  public:
    static BackendResult success(T Value)
    {
      return BackendResult(Storage(std::in_place_index<0>, std::move(Value)));
    }

    static BackendResult failure(BackendError Error)
    {
      return BackendResult(Storage(std::in_place_index<1>, std::move(Error)));
    }

    bool succeeded() const noexcept
    {
      return StorageValue.index() == 0;
    }

    explicit operator bool() const noexcept
    {
      return succeeded();
    }

    const T &value() const
    {
      if (!succeeded())
      {
        throw std::logic_error("backend result does not contain a value");
      }
      return std::get<0>(StorageValue);
    }

    T takeValue()
    {
      if (!succeeded())
      {
        throw std::logic_error("backend result does not contain a value");
      }
      return std::move(std::get<0>(StorageValue));
    }

    const BackendError &error() const
    {
      if (succeeded())
      {
        throw std::logic_error("successful backend result does not contain an error");
      }
      return std::get<1>(StorageValue);
    }

  private:
    using Storage = std::variant<T, BackendError>;

    explicit BackendResult(Storage StorageValue) : StorageValue(std::move(StorageValue))
    {
    }

    Storage StorageValue;
  };

  template <>
  class BackendResult<void>
  {
  public:
    static BackendResult success()
    {
      return BackendResult(std::nullopt);
    }

    static BackendResult failure(BackendError Error)
    {
      return BackendResult(std::move(Error));
    }

    bool succeeded() const noexcept
    {
      return !ErrorValue.has_value();
    }

    explicit operator bool() const noexcept
    {
      return succeeded();
    }

    const BackendError &error() const
    {
      if (succeeded())
      {
        throw std::logic_error("successful backend result does not contain an error");
      }
      return *ErrorValue;
    }

  private:
    explicit BackendResult(std::optional<BackendError> ErrorValue) : ErrorValue(std::move(ErrorValue))
    {
    }

    std::optional<BackendError> ErrorValue;
  };
} // namespace ink::backend

#endif
