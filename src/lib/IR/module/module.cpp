#include "ink/ir/module/module.h"

namespace ink::ir
{
  Module::Module(core::Name Name) noexcept
      : Name(Name)
  {
  }

  void Module::addStructType(StructType &TypeValue)
  {
    StructTypes.push_back(&TypeValue);
  }

  void Module::addObjectType(ObjectType &TypeValue)
  {
    ObjectTypes.push_back(&TypeValue);
  }

  Function &Module::createFunction(ValueRef Signature, core::Name FunctionName, SymbolVisibility Visibility, NativeLinkage Linkage)
  {
    Function &Created = Functions.emplace_back(Signature, FunctionName, Visibility, Linkage);
    Created.Parent = this;
    return Created;
  }

  GlobalVariable &Module::createGlobalVariable(const PointerType &ResultType, core::Name GlobalName, SymbolVisibility Visibility, NativeLinkage Linkage)
  {
    GlobalVariable &Created = GlobalVariables.emplace_back(ResultType, GlobalName, Visibility, Linkage);
    Created.Parent = this;
    return Created;
  }
} // namespace ink::ir
