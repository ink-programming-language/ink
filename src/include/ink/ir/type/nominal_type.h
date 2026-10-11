#ifndef INK_IR_TYPE_NOMINAL_TYPE_H
#define INK_IR_TYPE_NOMINAL_TYPE_H

#include "ink/core/name.h"
#include "ink/ir/function/method.h"
#include "ink/ir/type/type.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  class TypeContext;

  // A distinct declared type with a fixed layout and editable metadata until freeze().
  // Method signatures belong to this type's context; implementation bindings are not represented yet.
  class NominalType : public Type
  {
    public:
      ~NominalType() noexcept override = 0;

      std::uint64_t size() const noexcept final;
      std::uint64_t alignment() const noexcept final;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      bool setName(core::Name NewName) noexcept;

      const std::vector<Method> &methods() const noexcept;
      std::size_t methodCount() const noexcept;
      // Returns nullptr outside the list. Adding a method can invalidate pointers to earlier entries.
      const Method *method(std::size_t Index) const noexcept;
      // Rejects invalid metadata, foreign-context signatures and changes after freeze().
      // Entries retain registration order without imposing member identity or overload conflict rules.
      bool addMethod(Method Entry);
      // Replaces one entry before freeze(), for example after assigning a virtual table slot.
      // Out-of-range indices and invalid metadata are rejected without changing the existing entry.
      bool replaceMethod(std::size_t Index, Method Entry);
      // Matches source member names in registration order; pointers remain subject to list growth.
      std::vector<const Method *> findMethods(core::Name MemberName) const;

      bool isFrozen() const noexcept;
      void freeze() noexcept;
      static bool classof(const Value *Value) noexcept;

    protected:
      NominalType(ValueKind Kind, const TypeContext &Context, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept;

    private:
      bool validMethod(const Method &Entry) const noexcept;

      const TypeContext &Context;
      const std::uint64_t Size;
      const std::uint64_t Alignment;
      core::Name Name = core::NAME_None;
      std::vector<Method> Methods;
      bool Frozen = false;
  };
} // namespace ink::ir

#endif
