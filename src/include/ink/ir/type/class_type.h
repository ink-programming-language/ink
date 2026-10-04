#ifndef INK_IR_CLASS_TYPE_H
#define INK_IR_CLASS_TYPE_H

#include "ink/ir/type/user_defined_type.h"

#include <span>
#include <string>
#include <vector>

namespace ink::ir
{
  class Function;

  struct ClassField
  {
      Name FieldName;
      const Type *FieldType = nullptr;
      VisibilityKind Visibility = VisibilityKind::Public;
      const Function *Initializer = nullptr;
  };

  class ClassType final : public UserDefinedType
  {
    public:
      std::span<const ClassField> fields() const noexcept
      {
        return Fields;
      }

      std::span<const Function *const> methods() const noexcept
      {
        return Methods;
      }

      bool isComplete() const noexcept
      {
        return Complete;
      }

      std::string_view identity() const noexcept
      {
        return Identity;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ClassType;
      }

    private:
      ClassType(IRContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::ClassType, TypeKind::Class, MetaType, TypeName)
      {
      }

      std::vector<ClassField> Fields;
      std::vector<const Function *> Methods;
      std::string Identity;
      bool Complete = false;

      friend class TypePool;
      friend class IRBuilder;
      friend class Function;
  };
} // namespace ink::ir

#endif
