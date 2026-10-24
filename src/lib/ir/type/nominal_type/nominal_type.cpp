#include "ink/ir/type/nominal_type/nominal_type.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module.h"

#include <utility>

namespace ink::ir
{
  NominalType::NominalType(ValueKind Kind, const Module &Owner, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept
      : Type(Kind, *Owner.typePool().get<MetaType>()),
        Declaration(this),
        Owner(Owner),
        Size(Size),
        Alignment(Alignment),
        Name(Name)
  {
  }

  NominalType::NominalType(const NominalType &Declaration, bool Mutable) noexcept
      : Type(Declaration.kind(), *Declaration.Owner.typePool().get<MetaType>(), Mutable),
        Declaration(Declaration.declarationType()),
        Owner(Declaration.Owner),
        Size(Declaration.size()),
        Alignment(Declaration.alignment())
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

  std::string NominalType::toString() const
  {
    const auto NameText = core::NamePool::instance().text(name());
    return NameText.empty() ? Type::toString() : std::string(isMutable() ? "" : "const ") + std::string(NameText);
  }

  const NominalType *NominalType::declarationType() const noexcept
  {
    return Declaration;
  }

  core::Name NominalType::name() const noexcept
  {
    return Declaration->Name;
  }

  bool NominalType::hasName() const noexcept
  {
    return name().valid();
  }

  bool NominalType::setName(core::Name NewName) noexcept
  {
    if (Declaration != this || Frozen)
    {
      return false;
    }
    Name = NewName;
    return true;
  }

  const std::vector<Field> &NominalType::fields() const noexcept
  {
    return Declaration->Fields;
  }

  std::size_t NominalType::fieldCount() const noexcept
  {
    return fields().size();
  }

  const Field *NominalType::field(std::size_t Index) const noexcept
  {
    return Index < fields().size() ? &fields()[Index] : nullptr;
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
    return Declaration == this && !Frozen && Owner.ownsType(Entry.type()) && (!Entry.hasInitializer() || Owner.constantPool().owns(Entry.initializer())) && Entry.valid() && Entry.offset() <= Size && Entry.type()->size() <= Size - Entry.offset();
  }

  std::vector<const Field *> NominalType::findFields(core::Name MemberName) const
  {
    std::vector<const Field *> Matches;
    if (!MemberName.valid())
    {
      return Matches;
    }
    for (const Field &Entry : fields())
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
    return Declaration->Methods;
  }

  std::size_t NominalType::methodCount() const noexcept
  {
    return methods().size();
  }

  const Method *NominalType::method(std::size_t Index) const noexcept
  {
    return Index < methods().size() ? &methods()[Index] : nullptr;
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
    return Declaration == this && !Frozen && Entry.valid() && Owner.ownsType(Entry.functionType());
  }

  std::vector<const Method *> NominalType::findMethods(core::Name MemberName) const
  {
    std::vector<const Method *> Matches;
    if (!MemberName.valid())
    {
      return Matches;
    }
    for (const Method &Entry : methods())
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
    return Declaration->Frozen;
  }

  void NominalType::freeze() noexcept
  {
    if (Declaration == this)
    {
      Frozen = true;
    }
  }

  bool NominalType::classof(const Value *Value) noexcept
  {
    return Value && (Value->kind() == ValueKind::Class || Value->kind() == ValueKind::Interface || Value->kind() == ValueKind::Struct);
  }
} // namespace ink::ir
