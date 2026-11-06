#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/struct_type.h"

#include <cstddef>
#include <iterator>
#include <list>
#include <ranges>
#include <type_traits>

namespace ink::ir
{
  // Registers borrowed record definitions and owns globals and functions in insertion order.
  // Appending preserves declaration addresses and registration iterators; registered types retain their external owner.
  // All borrowed nodes must remain alive while the declaration graph is used.
  // Destruction does not access borrowed nodes; stop using the graph when any external owner begins destruction.
  class Module final
  {
    private:
      template <typename Record, bool IsConst>
      class RecordIterator
      {
        public:
          using value_type = Record;
          using difference_type = std::ptrdiff_t;
          using pointer = std::conditional_t<IsConst, const Record *, Record *>;
          using reference = std::conditional_t<IsConst, const Record &, Record &>;
          using iterator_category = std::bidirectional_iterator_tag;
          using iterator_concept = std::bidirectional_iterator_tag;

          RecordIterator() noexcept = default;

          template <bool OtherIsConst>
          RecordIterator(const RecordIterator<Record, OtherIsConst> &Other) noexcept requires (IsConst && !OtherIsConst)
              : Position(Other.Position)
          {
          }

          reference operator*() const noexcept
          {
            return **Position;
          }

          pointer operator->() const noexcept
          {
            return *Position;
          }

          RecordIterator &operator++() noexcept
          {
            ++Position;
            return *this;
          }

          RecordIterator operator++(int) noexcept
          {
            RecordIterator Previous = *this;
            ++*this;
            return Previous;
          }

          RecordIterator &operator--() noexcept
          {
            --Position;
            return *this;
          }

          RecordIterator operator--(int) noexcept
          {
            RecordIterator Previous = *this;
            --*this;
            return Previous;
          }

          friend bool operator==(const RecordIterator &, const RecordIterator &) noexcept = default;

        private:
          friend class Module;
          template <typename, bool>
          friend class RecordIterator;

          using StorageIterator = typename std::list<Record *>::const_iterator;

          explicit RecordIterator(StorageIterator Position) noexcept
              : Position(Position)
          {
          }

          StorageIterator Position = {};
      };

    public:
      explicit Module(core::Name Name = core::NAME_None) noexcept;

      Module(const Module &) = delete;
      Module &operator=(const Module &) = delete;
      Module(Module &&) = delete;
      Module &operator=(Module &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      // Registers existing nominal types without transferring ownership or validating duplicate definitions.
      void addStructType(StructType &TypeValue);
      void addObjectType(ObjectType &TypeValue);
      Function &createFunction(ValueRef Signature, core::Name FunctionName = core::NAME_None, SymbolVisibility Visibility = SymbolVisibility::Private, NativeLinkage Linkage = NativeLinkage::None);
      GlobalVariable &createGlobalVariable(const PointerType &ResultType, core::Name GlobalName = core::NAME_None, SymbolVisibility Visibility = SymbolVisibility::Private, NativeLinkage Linkage = NativeLinkage::None);
      GlobalVariable &createGlobalVariable(const PointerType &&, core::Name = core::NAME_None, SymbolVisibility = SymbolVisibility::Private, NativeLinkage = NativeLinkage::None) = delete;

      auto structTypes(this auto &&Self) noexcept
      {
        using Iterator = RecordIterator<StructType, std::is_const_v<std::remove_reference_t<decltype(Self)>>>;
        return std::ranges::subrange(Iterator(Self.StructTypes.cbegin()), Iterator(Self.StructTypes.cend()));
      }

      auto objectTypes(this auto &&Self) noexcept
      {
        using Iterator = RecordIterator<ObjectType, std::is_const_v<std::remove_reference_t<decltype(Self)>>>;
        return std::ranges::subrange(Iterator(Self.ObjectTypes.cbegin()), Iterator(Self.ObjectTypes.cend()));
      }

      auto functions(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Functions.begin(), Self.Functions.end());
      }

      auto globalVariables(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.GlobalVariables.begin(), Self.GlobalVariables.end());
      }

      std::size_t structTypeCount() const noexcept
      {
        return StructTypes.size();
      }

      std::size_t objectTypeCount() const noexcept
      {
        return ObjectTypes.size();
      }

      std::size_t functionCount() const noexcept
      {
        return Functions.size();
      }

      std::size_t globalVariableCount() const noexcept
      {
        return GlobalVariables.size();
      }

    private:
      core::Name Name = core::NAME_None;
      std::list<StructType *> StructTypes = {};
      std::list<ObjectType *> ObjectTypes = {};
      std::list<GlobalVariable> GlobalVariables = {};
      std::list<Function> Functions = {};
  };
} // namespace ink::ir
