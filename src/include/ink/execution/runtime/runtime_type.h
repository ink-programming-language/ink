#ifndef INK_EXECUTION_RUNTIME_RUNTIME_TYPE_H
#define INK_EXECUTION_RUNTIME_RUNTIME_TYPE_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace ink::execution
{
  using RuntimeTypeId = std::uint32_t;
  using FunctionId = std::uint32_t;
  using SignatureId = RuntimeTypeId;
  inline constexpr RuntimeTypeId InvalidRuntimeType = std::numeric_limits<RuntimeTypeId>::max();
  inline constexpr FunctionId InvalidFunction = std::numeric_limits<FunctionId>::max();

  struct RuntimeTypeDomain final
  {
  };

  enum class RuntimeKind
  {
    Invalid,
    Void,
    Boolean,
    Integer,
    Float,
    String,
    Pointer,
    Function,
    Array,
  };

  // These owned descriptors contain representation and identity, never IR nodes.
  struct StorageLayout
  {
      std::shared_ptr<const RuntimeTypeDomain> Domain;
      RuntimeTypeId Type = InvalidRuntimeType;
      RuntimeKind Kind = RuntimeKind::Invalid;
      std::uint32_t BitWidth = 0;
      bool Signed = false;
      std::size_t Size = 0;
      std::size_t Alignment = 1;
      bool Native = false;
      RuntimeTypeId Pointee = InvalidRuntimeType;
      bool Writable = false;
      RuntimeTypeId ReturnType = InvalidRuntimeType;
      std::vector<RuntimeTypeId> Parameters;
      RuntimeTypeId ElementType = InvalidRuntimeType;
      std::uint64_t ElementCount = 0;
      std::shared_ptr<const StorageLayout> ElementLayout;
  };

  class RuntimeTypeTable final
  {
    public:
      RuntimeTypeTable() = default;
      RuntimeTypeTable(const RuntimeTypeTable &) = delete;
      RuntimeTypeTable &operator=(const RuntimeTypeTable &) = delete;
      RuntimeTypeTable(RuntimeTypeTable &&) = delete;
      RuntimeTypeTable &operator=(RuntimeTypeTable &&) = delete;

      const StorageLayout *get(RuntimeTypeId Type) const noexcept;
      std::size_t size() const noexcept;
      RuntimeTypeId append(StorageLayout Layout);

      const std::shared_ptr<const RuntimeTypeDomain> &domain() const noexcept
      {
        return Domain;
      }

    private:
      std::shared_ptr<const RuntimeTypeDomain> Domain = std::make_shared<const RuntimeTypeDomain>();
      std::deque<StorageLayout> Layouts;
  };

  struct RuntimeFunctionDescriptor
  {
      FunctionId Id = InvalidFunction;
      RuntimeTypeId Signature = InvalidRuntimeType;
      std::string Symbol;
      // External selects host lookup; C ABI definitions still execute their bytecode bodies.
      bool External = false;
      bool NativeAbi = false;
      bool Supported = false;
      bool CAbi = false;
      // Only the defining object publishes this raw native symbol name.
      bool Exported = false;
  };
} // namespace ink::execution

#endif
