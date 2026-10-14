#ifndef INK_IR_TYPE_NOMINAL_TYPE_NOMINAL_TYPE_H
#define INK_IR_TYPE_NOMINAL_TYPE_NOMINAL_TYPE_H

#include "ink/core/name/name.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/ir/type/type.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  class Module;

  // A distinct declared type with a fixed layout and editable metadata until freeze().
  // Owns field metadata and methods with their functions; field initializers and referenced types belong to its module.
  // freeze() prevents changes to type metadata but does not freeze function names or bodies.
  class NominalType : public Type
  {
    public:
      ~NominalType() noexcept override = 0;

      std::uint64_t size() const noexcept final;
      std::uint64_t alignment() const noexcept final;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      bool setName(core::Name NewName) noexcept;

      const std::vector<Field> &fields() const noexcept;
      std::size_t fieldCount() const noexcept;
      // Returns nullptr outside the list. Adding a field can invalidate pointers to earlier entries.
      const Field *field(std::size_t Index) const noexcept;
      // Rejects invalid metadata, foreign types or initializers, out-of-bounds storage and changes after freeze().
      // Explicit offsets never change the fixed layout; overlap, packing and name conflicts are left to later stages.
      bool addField(Field Entry);
      // Rejection preserves the existing list, including for out-of-range indices.
      bool replaceField(std::size_t Index, Field Entry);
      // Matches names in registration order; returned pointers remain subject to list growth.
      std::vector<const Field *> findFields(core::Name MemberName) const;

      const std::vector<Method> &methods() const noexcept;
      std::size_t methodCount() const noexcept;
      // Returns nullptr outside the list. Adding a method can invalidate pointers to earlier entries.
      const Method *method(std::size_t Index) const noexcept;
      // Transfers ownership on success; rejection leaves Entry and the existing list unchanged.
      // Rejects invalid metadata, foreign-module signatures and changes after freeze().
      // Entries retain registration order without imposing member identity or overload conflict rules.
      bool addMethod(Method &&Entry);
      // Replaces an owned method before freeze(), destroying its previous function on success.
      // Out-of-range indices and invalid metadata are rejected without changing either entry.
      bool replaceMethod(std::size_t Index, Method &&Entry);
      // Matches current function names in registration order; renaming a function changes these lookup results.
      // Returned pointers remain subject to list growth.
      std::vector<const Method *> findMethods(core::Name MemberName) const;

      bool isFrozen() const noexcept;
      void freeze() noexcept;
      static bool classof(const Value *Value) noexcept;

    protected:
      NominalType(ValueKind Kind, const Module &Owner, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept;

    private:
      friend class Module;

      bool validField(const Field &Entry) const noexcept;
      bool validMethod(const Method &Entry) const noexcept;

      const Module &Owner;
      const std::uint64_t Size;
      const std::uint64_t Alignment;
      core::Name Name = core::NAME_None;
      std::vector<Field> Fields;
      std::vector<Method> Methods;
      bool Frozen = false;
  };
} // namespace ink::ir

#endif
