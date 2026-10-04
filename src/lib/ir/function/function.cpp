#include "ink/ir/function/function.h"
#include "ink/ir/type/class_type.h"

#include <algorithm>

namespace ink::ir
{
  Function::~Function()
  {
    if (ClassOwner)
    {
      auto &Class = const_cast<ClassType &>(*ClassOwner);
      std::erase(Class.Methods, this);
      if (InitializerField < Class.Fields.size() && Class.Fields[InitializerField].Initializer == this)
      {
        Class.Fields[InitializerField].Initializer = nullptr;
      }
    }
  }
} // namespace ink::ir
