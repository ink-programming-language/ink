#include "ink/ir/type/nominal_type/nominal_type.h"

#include "ink/ir/type/type_context.h"

#include <utility>

namespace ink::ir
{
  NominalType::NominalType(ValueKind Kind, const TypeContext &Context, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept
      : Type(Kind),
        Context(Context),
        Size(Size),
        Alignment(Alignment),
        Name(Name)
  {
  }

  NominalType::~NominalType() noexcept = default;

  std::uint64_t NominalType::size() const noexcept
  {
    return Size;
  }

  std::uint64_t NominalType::alignment() const noexcept
  {
    return Alignment;
  }

  core::Name NominalType::name() const noexcept
  {
    return Name;
  }

  bool NominalType::hasName() const noexcept
  {
    return Name.valid();
  }

  bool NominalType::setName(core::Name NewName) noexcept
  {
    if (Frozen)
    {
      return false;
    }
    Name = NewName;
    return true;
  }

  const std::vector<Field> &NominalType::fields() const noexcept
  {
    return Fields;
  }

  std::size_t NominalType::fieldCount() const noexcept
  {
    return Fields.size();
  }

  const Field *NominalType::field(std::size_t Index) const noexcept
  {
    return Index < Fields.size() ? &Fields[Index] : nullptr;
  }

  bool NominalType::addField(Field Entry)
  {
    if (!validField(Entry))
    {
      return false;
    }
    Fields.push_back(Entry);
    return true;
  }

  bool NominalType::replaceField(std::size_t Index, Field Entry)
  {
    if (Index >= Fields.size() || !validField(Entry))
    {
      return false;
    }
    Fields[Index] = Entry;
    return true;
  }

  bool NominalType::validField(const Field &Entry) const noexcept
  {
    return !Frozen && Context.owns(Entry.type()) && (!Entry.hasInitializer() || Context.constantPool().owns(Entry.initializer())) && Entry.valid() && Entry.offset() <= Size && Entry.type()->size() <= Size - Entry.offset();
  }

  std::vector<const Field *> NominalType::findFields(core::Name MemberName) const
  {
    std::vector<const Field *> Matches;
    if (!MemberName.valid())
    {
      return Matches;
    }
    for (const Field &Entry : Fields)
    {
      if (Entry.name() == MemberName)
      {
        Matches.push_back(&Entry);
      }
    }
    return Matches;
  }

  const std::vector<Method> &NominalType::methods() const noexcept
  {
    return Methods;
  }

  std::size_t NominalType::methodCount() const noexcept
  {
    return Methods.size();
  }

  const Method *NominalType::method(std::size_t Index) const noexcept
  {
    return Index < Methods.size() ? &Methods[Index] : nullptr;
  }

  bool NominalType::addMethod(Method &&Entry)
  {
    if (!validMethod(Entry))
    {
      return false;
    }
    Methods.push_back(std::move(Entry));
    return true;
  }

  bool NominalType::replaceMethod(std::size_t Index, Method &&Entry)
  {
    if (Index >= Methods.size() || !validMethod(Entry))
    {
      return false;
    }
    Methods[Index] = std::move(Entry);
    return true;
  }

  bool NominalType::validMethod(const Method &Entry) const noexcept
  {
    return !Frozen && Entry.valid() && Context.owns(Entry.functionType());
  }

  std::vector<const Method *> NominalType::findMethods(core::Name MemberName) const
  {
    std::vector<const Method *> Matches;
    if (!MemberName.valid())
    {
      return Matches;
    }
    for (const Method &Entry : Methods)
    {
      if (Entry.name() == MemberName)
      {
        Matches.push_back(&Entry);
      }
    }
    return Matches;
  }

  bool NominalType::isFrozen() const noexcept
  {
    return Frozen;
  }

  void NominalType::freeze() noexcept
  {
    Frozen = true;
  }

  bool NominalType::classof(const Value *Value) noexcept
  {
    return Value && (Value->kind() == ValueKind::Class || Value->kind() == ValueKind::Interface || Value->kind() == ValueKind::Struct);
  }
} // namespace ink::ir
