#include "ink/ir/ir_builder.h"
#include "ink/ir/analysis/type_layout.h"

#include <algorithm>
#include <unordered_set>

namespace ink::ir
{
  namespace
  {
    bool validFieldType(const Type &TypeValue, const ClassType &Defining, std::vector<const Type *> &Active, std::unordered_set<const Type *> &Validated)
    {
      if (Validated.contains(&TypeValue))
      {
        return true;
      }
      if (&TypeValue == &Defining || Active.size() >= 256 || std::find(Active.begin(), Active.end(), &TypeValue) != Active.end())
      {
        return false;
      }
      if (ArrayType::classof(&TypeValue))
      {
        Active.push_back(&TypeValue);
        const bool Valid = validFieldType(static_cast<const ArrayType &>(TypeValue).elementType(), Defining, Active, Validated);
        Active.pop_back();
        if (Valid)
        {
          Validated.insert(&TypeValue);
        }
        return Valid;
      }
      if (ClassType::classof(&TypeValue))
      {
        Active.push_back(&TypeValue);
        for (const ClassField &Field : static_cast<const ClassType &>(TypeValue).fields())
        {
          if (!validFieldType(*Field.FieldType, Defining, Active, Validated))
          {
            Active.pop_back();
            return false;
          }
        }
        Active.pop_back();
        Validated.insert(&TypeValue);
        return true;
      }
      switch (TypeValue.typeKind())
      {
      case TypeKind::Bool:
      case TypeKind::Integer:
      case TypeKind::Float:
      case TypeKind::Pointer:
      case TypeKind::Reference:
      case TypeKind::Slice:
        return true;
      default:
        return false;
      }
    }
  } // namespace

  bool IRBuilder::defineClassType(const ClassType &ValueType, std::span<const ClassField> Fields, std::string_view Identity)
  {
    if (&ValueType.context() != &Context || ValueType.isComplete() || Identity.empty())
    {
      return false;
    }
    std::vector<const Type *> Active;
    std::unordered_set<const Type *> Validated;
    std::unordered_set<Name> Names;
    for (const ClassField &Field : Fields)
    {
      if (!Context.namePool().contains(Field.FieldName) || !Names.insert(Field.FieldName).second || !Field.FieldType || &Field.FieldType->context() != &Context || (Field.Visibility != core::VisibilityKind::Public && Field.Visibility != core::VisibilityKind::Private) || !validFieldType(*Field.FieldType, ValueType, Active, Validated))
      {
        return false;
      }
    }
    auto &Definition = const_cast<ClassType &>(ValueType);
    Definition.Fields.assign(Fields.begin(), Fields.end());
    Definition.Identity = Identity;
    Definition.Complete = true;
    Context.notifyChanged();
    return true;
  }

  bool IRBuilder::registerClassType(Module &ModuleValue, const ClassType &Class)
  {
    if (&ModuleValue.context() != &Context || &Class.context() != &Context)
    {
      return false;
    }
    if (std::find(ModuleValue.ClassTypes.begin(), ModuleValue.ClassTypes.end(), &Class) == ModuleValue.ClassTypes.end())
    {
      ModuleValue.ClassTypes.push_back(&Class);
      Context.notifyChanged();
    }
    return true;
  }

  bool IRBuilder::setClassMethod(const ClassType &Class, Function &Method)
  {
    if (&Class.context() != &Context || &Method.context() != &Context || !Class.isComplete() || Method.ClassOwner || Method.functionType().parameterTypes().empty())
    {
      return false;
    }
    const auto *Receiver = Method.functionType().parameterTypes().front();
    if (!PointerType::classof(Receiver) || &static_cast<const PointerType &>(*Receiver).pointeeType() != &Class)
    {
      return false;
    }
    const_cast<ClassType &>(Class).Methods.push_back(&Method);
    Method.ClassOwner = &Class;
    Context.notifyChanged();
    return true;
  }

  bool IRBuilder::setFieldInitializer(const ClassType &Class, std::size_t Field, Function &Initializer)
  {
    if (&Class.context() != &Context || &Initializer.context() != &Context || Field >= Class.fields().size() || Class.fields()[Field].Initializer || Initializer.ClassOwner || !Initializer.functionType().parameterTypes().empty() || &Initializer.functionType().returnType() != Class.fields()[Field].FieldType)
    {
      return false;
    }
    const_cast<ClassType &>(Class).Fields[Field].Initializer = &Initializer;
    Initializer.ClassOwner = &Class;
    Initializer.InitializerField = Field;
    Context.notifyChanged();
    return true;
  }

  std::unique_ptr<ClassInstruction> IRBuilder::createDetachedClassInstruction(const ClassType &ValueType, std::span<const Value *const> Fields)
  {
    if (&ValueType.context() != &Context || !computeTypeLayout(ValueType, Context.compilationContext().targetContext()) || ValueType.fields().size() != Fields.size())
    {
      return nullptr;
    }
    for (std::size_t Index = 0; Index < Fields.size(); ++Index)
    {
      if (!Fields[Index] || &Fields[Index]->context() != &Context || &Fields[Index]->type() != ValueType.fields()[Index].FieldType)
      {
        return nullptr;
      }
    }
    return std::unique_ptr<ClassInstruction>(new ClassInstruction(ValueType, Fields));
  }

  std::unique_ptr<FieldExtractInstruction> IRBuilder::createDetachedFieldExtractInstruction(const Value &Object, std::size_t FieldIndex)
  {
    if (&Object.context() != &Context || !ClassType::classof(&Object.type()))
    {
      return nullptr;
    }
    const auto &Class = static_cast<const ClassType &>(Object.type());
    if (FieldIndex >= Class.fields().size() || !computeTypeLayout(Class, Context.compilationContext().targetContext()))
    {
      return nullptr;
    }
    return std::unique_ptr<FieldExtractInstruction>(new FieldExtractInstruction(Object, FieldIndex));
  }

  std::unique_ptr<FieldPointerInstruction> IRBuilder::createDetachedFieldPointerInstruction(const Value &Address, std::size_t FieldIndex)
  {
    if (&Address.context() != &Context || !PointerType::classof(&Address.type()))
    {
      return nullptr;
    }
    const auto &Pointer = static_cast<const PointerType &>(Address.type());
    if (!ClassType::classof(&Pointer.pointeeType()))
    {
      return nullptr;
    }
    const auto &Class = static_cast<const ClassType &>(Pointer.pointeeType());
    if (FieldIndex >= Class.fields().size() || !computeTypeLayout(Class, Context.compilationContext().targetContext()))
    {
      return nullptr;
    }
    const auto *Result = Context.typePool().getType<TypeKind::Pointer>(*Class.fields()[FieldIndex].FieldType, Pointer.access());
    return std::unique_ptr<FieldPointerInstruction>(new FieldPointerInstruction(*Result, Address, FieldIndex));
  }

  ClassInstruction *IRBuilder::createClassInstruction(const ClassType &ValueType, std::span<const Value *const> Fields)
  {
    return canInsert() ? insert(createDetachedClassInstruction(ValueType, Fields)) : nullptr;
  }

  FieldExtractInstruction *IRBuilder::createFieldExtractInstruction(const Value &Object, std::size_t FieldIndex)
  {
    return canInsert() ? insert(createDetachedFieldExtractInstruction(Object, FieldIndex)) : nullptr;
  }

  FieldPointerInstruction *IRBuilder::createFieldPointerInstruction(const Value &Address, std::size_t FieldIndex)
  {
    return canInsert() ? insert(createDetachedFieldPointerInstruction(Address, FieldIndex)) : nullptr;
  }
} // namespace ink::ir
