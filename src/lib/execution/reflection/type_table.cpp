#include "ink/core/object_layout.h"
#include "ink/execution/reflection/type_table.h"

#include <algorithm>
#include <utility>

namespace ink::execution
{
  const TypeDesc *RuntimeTypeTable::get(RuntimeTypeId Type) const noexcept
  {
    return Type < Layouts.size() ? &Layouts[Type] : nullptr;
  }

  std::size_t RuntimeTypeTable::size() const noexcept
  {
    return Layouts.size();
  }

  RuntimeTypeId RuntimeTypeTable::reserve()
  {
    if (Layouts.size() >= InvalidRuntimeType)
    {
      return InvalidRuntimeType;
    }
    TypeDesc Layout;
    Layout.Domain = Domain;
    Layout.Type = static_cast<RuntimeTypeId>(Layouts.size());
    Layouts.push_back(std::move(Layout));
    OwnedLayouts.push_back(nullptr);
    return Layouts.back().Type;
  }

  bool RuntimeTypeTable::define(RuntimeTypeId Type, TypeDesc Layout)
  {
    if (!Layout.validDetails() || Type >= Layouts.size() || Layouts[Type].Kind != RuntimeKind::Invalid || ((Layout.Kind == RuntimeKind::Array || Layout.Kind == RuntimeKind::Class) && (Layout.Alignment == 0 || (Layout.Alignment & (Layout.Alignment - 1)) != 0)))
    {
      return false;
    }
    if (Layout.Kind == RuntimeKind::Array)
    {
      const TypeDesc *Element = get(Layout.arrayDesc().ElementType);
      if (!Element || Element->Kind == RuntimeKind::Invalid || Element->Kind == RuntimeKind::Void || Layout.arrayDesc().ElementType == Type)
      {
        return false;
      }
      if (Layout.arrayDesc().ElementCount > std::numeric_limits<std::size_t>::max() || (Element->Size != 0 && Layout.arrayDesc().ElementCount > std::numeric_limits<std::size_t>::max() / Element->Size) || Layout.Size != Element->Size * static_cast<std::size_t>(Layout.arrayDesc().ElementCount) || Layout.Alignment != Element->Alignment || Layout.Native != Element->Native)
      {
        return false;
      }
      Layout.editArray().ElementLayout = OwnedLayouts[Layout.arrayDesc().ElementType];
    }
    if (Layout.Kind == RuntimeKind::Class)
    {
      if (!Layout.validDetails() || Layout.classDesc().NominalIdentity.empty() || Layout.classDesc().NominalIdentity.find('\0') != std::string::npos)
      {
        return false;
      }
      auto Class = std::make_shared<ClassDesc>(Layout.classDesc());
      core::ObjectLayoutBuilder Object(std::numeric_limits<std::size_t>::max());
      bool Native = true;
      for (std::size_t Index = 0; Index < Layout.classDesc().Fields.size(); ++Index)
      {
        const TypeDesc *Field = get(Layout.classDesc().Fields[Index].Type);
        if (!Field || Field->Kind == RuntimeKind::Invalid || Field->Kind == RuntimeKind::Void || Layout.classDesc().Fields[Index].Type == Type || Field->Alignment == 0 || (Field->Alignment & (Field->Alignment - 1)) != 0)
        {
          return false;
        }
        const auto Offset = Object.append(Field->Size, Field->Alignment);
        if (!Offset || Layout.classDesc().Fields[Index].Offset != *Offset)
        {
          return false;
        }
        Native = Native && Field->Native;
        Class->Fields[Index].Layout = OwnedLayouts[Layout.classDesc().Fields[Index].Type];
      }
      if (!Object.size() || Layout.Size != *Object.size() || Layout.Alignment != Object.alignment() || Layout.Native != Native)
      {
        return false;
      }
      Layout.setDetails(std::move(Class));
    }
    Layout.Domain = Domain;
    Layout.Type = Type;
    if (Layout.Kind == RuntimeKind::Class)
    {
      Layout.Name = Layout.classDesc().NominalIdentity;
    }
    if (!Layout.Name.empty())
    {
      const auto [Entry, Inserted] = Names.emplace(Layout.Name, Type);
      if (!Inserted && Entry->second != Type)
      {
        Entry->second = InvalidRuntimeType;
      }
    }
    Layouts[Type] = std::move(Layout);
    OwnedLayouts[Type] = std::make_shared<TypeDesc>(Layouts[Type]);
    return true;
  }

  RuntimeTypeId RuntimeTypeTable::append(TypeDesc Layout)
  {
    const RuntimeTypeId Type = reserve();
    if (Type == InvalidRuntimeType)
    {
      return Type;
    }
    if (!define(Type, std::move(Layout)))
    {
      Layouts.pop_back();
      OwnedLayouts.pop_back();
      return InvalidRuntimeType;
    }
    return Type;
  }

  bool RuntimeTypeTable::defineAll(std::vector<TypeDesc> Values, std::size_t MaxDepth)
  {
    if (!Layouts.empty() || Values.size() >= InvalidRuntimeType)
    {
      return false;
    }
    for (const auto &Layout : Values)
    {
      if (!Layout.validDetails())
      {
        return false;
      }
    }
    for (std::size_t Index = 0; Index < Values.size(); ++Index)
    {
      reserve();
    }
    std::vector<std::uint8_t> States(Values.size());
    std::vector<std::size_t> Depths(Values.size());
    struct Visit
    {
        RuntimeTypeId Type;
        std::size_t Child = 0;
    };
    std::vector<Visit> Stack;
    for (std::size_t Root = 0; Root < Values.size(); ++Root)
    {
      if (States[Root] == 2)
      {
        continue;
      }
      States[Root] = 1;
      Stack.push_back({static_cast<RuntimeTypeId>(Root)});
      while (!Stack.empty())
      {
        if (Stack.size() > MaxDepth)
        {
          return false;
        }
        Visit &Current = Stack.back();
        const TypeDesc &Layout = Values[Current.Type];
        const std::size_t Count = Layout.Kind == RuntimeKind::Class ? Layout.classDesc().Fields.size() : Layout.Kind == RuntimeKind::Array ? 1 : 0;
        if (Current.Child < Count)
        {
          const RuntimeTypeId Child = Layout.Kind == RuntimeKind::Class ? Layout.classDesc().Fields[Current.Child++].Type : (++Current.Child, Layout.arrayDesc().ElementType);
          if (Child >= Values.size() || States[Child] == 1)
          {
            return false;
          }
          if (States[Child] == 0)
          {
            States[Child] = 1;
            Stack.push_back({Child});
          }
          continue;
        }
        const RuntimeTypeId Type = Current.Type;
        std::size_t Depth = 1;
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          const RuntimeTypeId Child = Layout.Kind == RuntimeKind::Class ? Layout.classDesc().Fields[Index].Type : Layout.arrayDesc().ElementType;
          if (Depths[Child] >= MaxDepth)
          {
            return false;
          }
          Depth = std::max(Depth, Depths[Child] + 1);
        }
        Depths[Type] = Depth;
        if (!define(Type, std::move(Values[Type])))
        {
          return false;
        }
        States[Type] = 2;
        Stack.pop_back();
      }
    }
    return true;
  }
} // namespace ink::execution
