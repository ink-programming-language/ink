#ifndef INK_IR_TYPE_VALUE_H
#define INK_IR_TYPE_VALUE_H

#include <cstdint>
#include <string>
#include <string_view>

namespace ink::ir
{
  class Type;
  class ValueStringBuilder;

  enum class ValueKind : std::uint8_t
  {
#define INK_VALUE(Kind, Name, FixedSize) Kind,
#include "ink/ir/type/Value.def"
#undef INK_VALUE
  };

  std::string_view valueKindName(ValueKind Kind) noexcept;

  class Value
  {
    public:
      virtual ~Value() noexcept = 0;

      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;

      ValueKind kind() const noexcept;
      // Returns a borrowed recorded type, or nullptr for an untyped value. Does not convert or validate the value.
      // Modules and basic blocks use the default nullptr result; type descriptors report their metatype and functions their full signature.
      virtual const Type *type() const noexcept;
      // Type descriptors report their represented object's top-level mutability; ordinary SSA values return false.
      // Write permission through a pointer is described by its pointee type, not by the pointer value.
      virtual bool isMutable() const noexcept;
      // Returns a short diagnostic description of this object, not its type or a complete IR dump.
      // The default implementation returns the value kind name.
      virtual std::string toString() const;

    protected:
      explicit Value(ValueKind Kind) noexcept;

    private:
      friend class ValueStringBuilder;

      // Composite values schedule their children here so formatting does not recurse through toString().
      // Other values use their virtual toString() implementation as a leaf description.
      virtual void appendToString(ValueStringBuilder &Builder) const;

      ValueKind Kind = ValueKind::Bool;
  };
} // namespace ink::ir

#endif
