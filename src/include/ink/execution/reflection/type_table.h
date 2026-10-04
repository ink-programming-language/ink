#ifndef INK_EXECUTION_REFLECTION_TYPE_TABLE_H
#define INK_EXECUTION_REFLECTION_TYPE_TABLE_H

#include "ink/execution/reflection/type_desc.h"

#include <deque>
#include <unordered_map>

namespace ink::execution
{
  class RuntimeTypeTable final
  {
    public:
      RuntimeTypeTable() = default;
      RuntimeTypeTable(const RuntimeTypeTable &) = delete;
      RuntimeTypeTable &operator=(const RuntimeTypeTable &) = delete;
      RuntimeTypeTable(RuntimeTypeTable &&) = delete;
      RuntimeTypeTable &operator=(RuntimeTypeTable &&) = delete;

      const TypeDesc *get(RuntimeTypeId Type) const noexcept;
      std::size_t size() const noexcept;
      const TypeDesc *find(std::string_view Name) const noexcept;
      bool updateClass(RuntimeTypeId Type, std::shared_ptr<ClassDesc> Description);
      RuntimeTypeId append(TypeDesc Layout);
      RuntimeTypeId reserve();
      bool define(RuntimeTypeId Type, TypeDesc Layout);
      bool defineAll(std::vector<TypeDesc> Layouts, std::size_t MaxDepth = 256);

      const std::shared_ptr<const RuntimeTypeDomain> &domain() const noexcept
      {
        return Domain;
      }

    private:
      std::shared_ptr<const RuntimeTypeDomain> Domain = std::make_shared<const RuntimeTypeDomain>();
      std::deque<TypeDesc> Layouts;
      std::vector<std::shared_ptr<const TypeDesc>> OwnedLayouts;
      std::unordered_map<std::string, RuntimeTypeId> Names;
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
