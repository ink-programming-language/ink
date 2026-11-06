#pragma once

#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/void_type.h"

#include <cstdlib>
#include <type_traits>

namespace ink::ir
{
  // Dispatches only; the derived visitor must handle every concrete type explicitly.
  template <typename Derived, typename Result = void>
  class TypeVisitor
  {
    public:
      Result visit(Type &Value)
      {
        return visitImpl(Value);
      }

      Result visit(const Type &Value)
      {
        return visitImpl(Value);
      }

    private:
      template <typename Node>
      Result visitImpl(Node &Value)
      {
        switch (Value.getKind())
        {
#define TYPE(Kind, Class, Spelling) case TypeKind::Kind: return static_cast<Derived &>(*this).visit##Class(static_cast<std::conditional_t<std::is_const_v<Node>, const Class, Class> &>(Value));
#include "ink/ir/type/type.def"
#undef TYPE
        }
        std::abort();
      }
  };
} // namespace ink::ir
